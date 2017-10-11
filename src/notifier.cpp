/*
 * Copyright 2016 Red Hat, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "notifier.hpp"

#include <cstring>
#include <string>

#define UNICODE
#define _UNICODE
#include <windows.h>
#include <shellapi.h>
#include <commctrl.h>

// http://svn.wxwidgets.org/viewvc/wx/wxWidgets/trunk/src/msw/msgdlg.cpp?r1=70409&r2=70408&pathrev=70409
#ifndef TDF_SIZE_TO_CONTENT
#define TDF_SIZE_TO_CONTENT 0x1000000
#endif

#include "jsonio.hpp"
#include "JsonRecord.hpp"
#include "Version.hpp"
#include "platform.hpp"
#include "Tracer.hpp"
#include "utils.hpp"

namespace { // anonymous

namespace ch = checker;

enum State { STATE_STANDBY, STATE_UPDATE };
const UINT NOTIFIER_ICON_UID = 1;
const std::wstring NOTIFIER_WINDOW_CLASS = L"e48e2be7-451e-48e5-8889-8c97c1485340";
const UINT WMAPP_NOTIFYCALLBACK = WM_APP + 1;
const size_t NOTIFIER_MAX_RC_LEN = 1 << 12;
const size_t NOTIFIER_MAX_INPUT_JSON_LEN = 1 << 15;
HINSTANCE NOTIFIER_HANDLE_INSTANCE = NULL;
// loaded on startup
std::wstring NOTIFIER_BALLOON_TEXT;
std::wstring NOTIFIER_UPDATE_HEADER;
std::wstring NOTIFIER_UPDATE_TEXT;
std::wstring NOTIFIER_BMP_ICON_PATH;
// we should be fine without sync in STA mode
State NOTIFIER_STATE = STATE_STANDBY;
// tracing
ch::Tracer TRACER;


std::wstring load_resource_string(UINT id) {
    std::wstring str;
    str.resize(NOTIFIER_MAX_RC_LEN);
    int loaded = LoadStringW(NOTIFIER_HANDLE_INSTANCE, id, &str.front(), static_cast<int>(str.length()));
    if (loaded > 0) {
        str.resize(loaded);
        return str;
    } else {
        TRACER.trace("error loading resource, uid: [" + ch::utils::to_string(id) + "]");
        return L"ERROR_LOAD_RESOURCE";
    }
}

void dump_trace() {
    if (!TRACER.is_enabled()) {
        return;
    }
    try {
        std::string appdatadir = ch::platform::get_userdata_directory();
        std::string vendorname = ch::utils::narrow(load_resource_string(IDS_VENDOR_DIRNAME));
        std::string appname = ch::utils::narrow(load_resource_string(IDS_APP_DIRNAME));
        std::string path = appdatadir + vendorname + "/" + appname + "/update/trace.json";
        ch::write_to_file(TRACER.get_json(), path);
    } catch(...) {
        // quiet
    }
}

bool load_input_json() {
    try {
        // load shipped version number
        std::wstring vnumwstr = load_resource_string(IDS_SHIPPED_VERSION_NUMBER);
        std::string vnumstr = ch::utils::narrow(vnumwstr);
        TRACER.trace("shipped version extracted, version_number: [" + vnumstr + "]");
        TRACER.trace("EVENT_SHIPPEDVERSION " + vnumstr);

        // find out path
        std::string appdatadir = ch::platform::get_userdata_directory();
        std::string vendorname = ch::utils::narrow(load_resource_string(IDS_VENDOR_DIRNAME));
        std::string appname = ch::utils::narrow(load_resource_string(IDS_APP_DIRNAME));
        std::string path = appdatadir + vendorname + "/" + appname + "/update/version.json";
        TRACER.trace("loading version from file, path: [" + path + "]");
        TRACER.trace("EVENT_LOCALPATH " + path);

        // load json
        ch::JsonRecord json = ch::read_from_file(path, NOTIFIER_MAX_INPUT_JSON_LEN);
        ch::Version ver(json);
        TRACER.trace("version loaded successfully");
        TRACER.trace("version contents, balloon_text: [" + ver.ui_balloon_text + "]");
        TRACER.trace("EVENT_CONTENTBALLOON " + ver.ui_balloon_text);
        NOTIFIER_BALLOON_TEXT = ch::utils::widen(ver.ui_balloon_text);
        TRACER.trace("version contents, update_header: [" + ver.ui_update_header + "]");
        TRACER.trace("EVENT_CONTENTUPDATEHEADER " + ver.ui_update_header);
        NOTIFIER_UPDATE_HEADER = ch::utils::widen(ver.ui_update_header);
        TRACER.trace("version contents, update_text: [" + ver.ui_update_text + "]");
        TRACER.trace("EVENT_CONTENTUPDATETEXT " + ver.ui_update_text);
        NOTIFIER_UPDATE_TEXT = ch::utils::widen(ver.ui_update_text);
        TRACER.trace("version contents, version_number: [" + ch::utils::to_string(ver.version_number) + "]");
        TRACER.trace("EVENT_CONTENTVERSION " + ch::utils::to_string(ver.version_number));

        // find out icon path
        std::string exepath = ch::platform::current_executable_path();
        std::string exedir = ch::utils::strip_filename(exepath);
        std::string iconpath = exedir + "icon.bmp";
        NOTIFIER_BMP_ICON_PATH = ch::utils::widen(iconpath);
        TRACER.trace("balloon icon resolved, path: [" + iconpath + "]");

        // check wherher version updated
        uint64_t vnum = ch::utils::parse_uint64(vnumstr);
        return ver.version_number > vnum;
    } catch (const std::exception& e) {
        TRACER.trace(std::string() + "ERROR: " + e.what());
        return false;
    }
}

bool add_notification(HWND hwnd) {
    TRACER.trace("is due to add notification");
    NOTIFYICONDATA nid;
    memset(&nid, '\0', sizeof(NOTIFYICONDATA));
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = NOTIFIER_ICON_UID;
    nid.uFlags = NIF_INFO | NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP;
    nid.uCallbackMessage = WMAPP_NOTIFYCALLBACK;

    // tray icon from embedded ico
    HRESULT err_icon = LoadIconMetric(NOTIFIER_HANDLE_INSTANCE, MAKEINTRESOURCE(IDI_NOTIFICATIONICON), LIM_SMALL, &nid.hIcon);
    if (S_OK != err_icon) {
        TRACER.trace("'LoadIconMetric' fail, return: [" + ch::utils::to_string(err_icon) + "]");
        return false;
    }

    // balloon icon from file
    nid.hBalloonIcon = static_cast<HICON>(LoadImageW(NULL, NOTIFIER_BMP_ICON_PATH.c_str(), IMAGE_BITMAP, 128, 128, LR_LOADFROMFILE));
    if (NULL == nid.hBalloonIcon) {
        TRACER.trace("'LoadImageW' fail, error: [" + ch::utils::errcode_to_string(GetLastError()) + "]");
        return false;
    }
    nid.dwInfoFlags = NIIF_USER | NIIF_NOSOUND | NIIF_RESPECT_QUIET_TIME;

    // tooltip text
    int err_tooltip = LoadStringW(NOTIFIER_HANDLE_INSTANCE, IDS_TOOLTIP, nid.szTip, ARRAYSIZE(nid.szTip));
    if (0 == err_tooltip) {
        TRACER.trace("'LoadStringW' for tooltip fail, error: [" + ch::utils::errcode_to_string(GetLastError()) + "]");
        return false;
    }

    // balloon header, static
    int err_title = LoadStringW(NOTIFIER_HANDLE_INSTANCE, IDS_BALLOON_TITLE, nid.szInfoTitle, ARRAYSIZE(nid.szInfoTitle));
    if (0 == err_title) {
        TRACER.trace("'LoadStringW' for title fail, error: [" + ch::utils::errcode_to_string(GetLastError()) + "]");
        return false;
    }

    // balloon text, from version file
    errno_t err_info = wcscpy_s(nid.szInfo, ARRAYSIZE(nid.szInfo), NOTIFIER_BALLOON_TEXT.c_str());
    if (0 != err_info) {
        return false;
    }

    // show
    BOOL success = Shell_NotifyIcon(NIM_ADD, &nid);
    if (success) {
        nid.uVersion = NOTIFYICON_VERSION_4;
        bool res = 0 != Shell_NotifyIcon(NIM_SETVERSION, &nid);
        TRACER.trace("'Shell_NotifyIconW' version, result: [" + ch::utils::to_string(res) + "]");
        return res;
    } else {
        TRACER.trace("'Shell_NotifyIconW' main fail");
        return false;
    }
}

bool delete_notification(HWND hwnd) {
    TRACER.trace("is due to delete notification");
    NOTIFYICONDATA nid;
    memset(&nid, '\0', sizeof(NOTIFYICONDATA));
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = NOTIFIER_ICON_UID;
    bool res = 0 != Shell_NotifyIcon(NIM_DELETE, &nid);
    TRACER.trace("'Shell_NotifyIconW' delete, result: [" + ch::utils::to_string(res) + "]");
    return res;
}

HRESULT CALLBACK link_clicked_callback(HWND hwnd, UINT uNotification, WPARAM /* wParam */, LPARAM lParam, LONG_PTR /* dwRefData */) {
    if (TDN_HYPERLINK_CLICKED != uNotification) {
        TRACER.trace("non-proceed dialog event received, code: [" + ch::utils::to_string(uNotification) + "]");
        return S_OK;
    }
    TRACER.trace("update proceed selected");
    HINSTANCE res = ShellExecuteW(NULL, NULL, reinterpret_cast<LPCTSTR> (lParam), NULL, NULL, SW_SHOW);
    int intres = reinterpret_cast<int> (res);
    bool success = intres > 32;
    if (!success) {
        TRACER.trace("'ShellExecuteW' fail, error: [" + ch::utils::to_string(intres) + "]");
        std::wstring title = load_resource_string(IDS_BROWSER_ERROR_TITLE);
        std::wstring text = load_resource_string(IDS_BROWSER_ERROR_TEXT);
        TaskDialog(hwnd, NOTIFIER_HANDLE_INSTANCE, title.c_str(), text.c_str(), L"", TDCBF_CLOSE_BUTTON, TD_ERROR_ICON, NULL);
    }
    DestroyWindow(hwnd);
    return S_OK;
}

void show_update_dialog(HWND hwnd) {
    if (STATE_STANDBY != NOTIFIER_STATE) {
        TRACER.trace("additional update dialog prevented");
        return;
    }
    NOTIFIER_STATE = STATE_UPDATE;
    TASKDIALOGCONFIG cf;
    memset(&cf, '\0', sizeof(TASKDIALOGCONFIG));
    cf.cbSize = sizeof(TASKDIALOGCONFIG);
    cf.hwndParent = hwnd;
    cf.dwFlags = TDF_ENABLE_HYPERLINKS | TDF_EXPAND_FOOTER_AREA | TDF_ALLOW_DIALOG_CANCELLATION | TDF_SIZE_TO_CONTENT;
    cf.hInstance = NOTIFIER_HANDLE_INSTANCE;
    std::wstring url = load_resource_string(IDS_BROWSER_URL);
    std::wstring link = std::wstring() + L"<a href=\"" + url + L"\">" + url + L"</a>";
    cf.pszFooter = link.c_str();
    cf.pfCallback = link_clicked_callback;
    std::wstring title = load_resource_string(IDS_UPDATE_TITLE);
    cf.pszWindowTitle = title.c_str();
    cf.pszMainIcon = MAKEINTRESOURCE(IDI_NOTIFICATIONICON);
    cf.pszMainInstruction = NOTIFIER_UPDATE_HEADER.c_str();
    cf.pszFooterIcon = MAKEINTRESOURCE(IDI_NOTIFICATIONICON); 
    cf.pszExpandedInformation = NOTIFIER_UPDATE_TEXT.c_str();
    std::wstring proceed = load_resource_string(IDS_UPDATE_PROCEED);
    cf.pszExpandedControlText = proceed.c_str();
    cf.cxWidth = 0;
    cf.dwCommonButtons = TDCBF_CANCEL_BUTTON;
    TRACER.trace("is due to show update dialog");
    HRESULT res = TaskDialogIndirect(&cf, NULL, NULL, NULL);
    TRACER.trace("update dialog closed, result: [" + ch::utils::to_string(res) + "]");
    DestroyWindow(hwnd);
}

LRESULT CALLBACK window_callback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE: {
            TRACER.trace("'WM_CREATE' event received");
            bool success = add_notification(hwnd);
            if (!success) {
                return -1;
            }
        } break;
    case WMAPP_NOTIFYCALLBACK:
        switch (LOWORD(lParam)) {
            case NIN_SELECT:
                TRACER.trace("'NIN_SELECT' event received");
                show_update_dialog(hwnd);
                break;
            case NIN_BALLOONUSERCLICK:
                TRACER.trace("'NIN_BALLOONUSERCLICK' event received");
                show_update_dialog(hwnd);
                break;
            case NIN_BALLOONTIMEOUT:
                TRACER.trace("'NIN_BALLOONTIMEOUT' event received");
                if (STATE_STANDBY == NOTIFIER_STATE) {
                    TRACER.trace("'NIN_BALLOONTIMEOUT' proceed");
                    DestroyWindow(hwnd);
                }
                break;
            case WM_CONTEXTMENU:
                TRACER.trace("'WM_CONTEXTMENU' event received");
                show_update_dialog(hwnd);
                break;
        } break;
    case WM_DESTROY:
        TRACER.trace("'WM_DESTROY' event received");
        delete_notification(hwnd);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

} // namespace

namespace checker {

// no-op notifier-specific transform impl
json_t* download_manager_transform(json_t* json) {
    return json;
}

} // namespace

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR /*lpCmdLine*/, int /* nCmdShow */) {
    // init tracer
    std::wstring cline(GetCommandLineW());
    bool verbose = L'v' == cline[cline.length() - 1] && L'-' == cline[cline.length() - 2];
    TRACER.set_enabled(verbose);
    TRACER.trace("EVENT_PID " + ch::utils::to_string(ch::platform::current_pid()));
    TRACER.trace("tracer initialized");

    // check we are alone
    std::wstring mutex_uid = load_resource_string(IDS_INSTANCE_MUTEX_UUID);
    ch::utils::NamedMutex mutex(mutex_uid);
    if (mutex.already_taken()) {
        TRACER.trace("another instance of notifier is already running");
        dump_trace();
        return 0;
    }

    // fill globals
    NOTIFIER_HANDLE_INSTANCE = hInstance;
    bool err_vcheck = load_input_json();
    if (!err_vcheck) {
        TRACER.trace("'NOGO' mode exiting");
        TRACER.trace("EVENT_PROCEED 1");
        dump_trace();
        return 0; // err handling is too coarse here
    }
    TRACER.trace("'GO' mode proceeding");
    TRACER.trace("EVENT_PROCEED 0");

    // create ui
    WNDCLASSEX wcex;
    memset(&wcex, '\0', sizeof(wcex));
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.lpfnWndProc = window_callback;
    wcex.hInstance = NOTIFIER_HANDLE_INSTANCE;
    wcex.hIcon = LoadIcon(NOTIFIER_HANDLE_INSTANCE, MAKEINTRESOURCE(IDI_NOTIFICATIONICON));
    wcex.lpszClassName = NOTIFIER_WINDOW_CLASS.c_str();
    ATOM err_reg = RegisterClassExW(&wcex);
    if (0 == err_reg) {
        TRACER.trace("'RegisterClassExW' fail, error: [" + ch::utils::errcode_to_string(GetLastError()) + "]");
        dump_trace();
        return 1;
    }
    HWND hwnd = CreateWindowExW(0, NOTIFIER_WINDOW_CLASS.c_str(), NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);
    if (NULL == hwnd) {
        TRACER.trace("'CreateWindowExW' fail, error: [" + ch::utils::errcode_to_string(GetLastError()) + "]");
        dump_trace();
        return 1;
    }

    // message loop
    MSG msg;
    TRACER.trace("is due to start message loop");
    // dump early what we have, will be replaced on clean UI exit
    dump_trace();
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    dump_trace();
    return 0;
}
