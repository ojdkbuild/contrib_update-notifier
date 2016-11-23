/*
 * Copyright 2016, akashche at redhat.com
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

#include "jsonio.hpp"
#include "JsonRecord.hpp"
#include "Version.hpp"
#include "platform.hpp"
#include "utils.hpp"

namespace { // anonymous

namespace ch = checker;

enum State { STATE_STANDBY, STATE_BALLOON, STATE_ABOUT, STATE_UPDATE, STATE_HOVERED, STATE_CLICKED };
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


std::wstring load_resource_string(UINT id) {
    std::wstring str;
    str.resize(NOTIFIER_MAX_RC_LEN);
    int loaded = LoadStringW(NOTIFIER_HANDLE_INSTANCE, id, &str.front(), str.length());
    if (loaded > 0) {
        str.resize(loaded);
        return str;
    } else {
        return L"ERROR_LOAD_RESOURCE";
    }
}

bool load_input_json() {
    try {
        std::string appdatadir = ch::platform::get_userdata_directory();
        std::string vendorname = ch::utils::narrow(load_resource_string(IDS_VENDOR_DIRNAME));
        std::string appname = ch::utils::narrow(load_resource_string(IDS_APP_DIRNAME));
        std::string path = appdatadir + vendorname + "/" + appname + "/version.json";
        ch::JsonRecord json = ch::read_from_file(path, NOTIFIER_MAX_INPUT_JSON_LEN);
        ch::Version ver(json);
        NOTIFIER_BALLOON_TEXT = ch::utils::widen(ver.ui_balloon_text);
        NOTIFIER_UPDATE_HEADER = ch::utils::widen(ver.ui_update_header);
        NOTIFIER_UPDATE_TEXT = ch::utils::widen(ver.ui_update_text);
        std::string exepath = ch::platform::current_executable_path();
        std::string exedir = ch::utils::strip_filename(exepath);
        NOTIFIER_BMP_ICON_PATH = ch::utils::widen(exedir + "icon.bmp");
        std::wstring vnumwstr = load_resource_string(IDS_SHIPPED_VERSION_NUMBER);
        std::string vnumstr = ch::utils::narrow(vnumwstr);
        uint32_t vnum = ch::utils::parse_uint32(vnumstr);
        return ver.version_number > vnum;
    } catch (const std::exception& e) {
        return false;
    }
}

bool add_notification(HWND hwnd) {
    NOTIFYICONDATA nid;
    memset(&nid, '\0', sizeof(NOTIFYICONDATA));
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = NOTIFIER_ICON_UID;
    nid.uFlags = NIF_INFO | NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP;
    nid.uCallbackMessage = WMAPP_NOTIFYCALLBACK;
    HRESULT err_icon = LoadIconMetric(NOTIFIER_HANDLE_INSTANCE, MAKEINTRESOURCE(IDI_NOTIFICATIONICON), LIM_SMALL, &nid.hIcon);
    if (S_OK != err_icon) {
        return false;
    }
    nid.hBalloonIcon = static_cast<HICON>(LoadImageW(NULL, NOTIFIER_BMP_ICON_PATH.c_str(), IMAGE_BITMAP, 128, 128, LR_LOADFROMFILE));
    if (NULL == nid.hBalloonIcon) {
        return false;
    }
    int err_tooltip = LoadStringW(NOTIFIER_HANDLE_INSTANCE, IDS_TOOLTIP, nid.szTip, ARRAYSIZE(nid.szTip));
    if (0 == err_tooltip) {
        return false;
    }
    nid.dwInfoFlags = NIIF_USER | NIIF_NOSOUND | NIIF_RESPECT_QUIET_TIME;
    int err_title = LoadStringW(NOTIFIER_HANDLE_INSTANCE, IDS_BALLOON_TITLE, nid.szInfoTitle, ARRAYSIZE(nid.szInfoTitle));
    if (0 == err_title) {
        return false;
    }
    errno_t err_info = wcscpy_s(nid.szInfo, ARRAYSIZE(nid.szInfo), NOTIFIER_BALLOON_TEXT.c_str());
    if (0 != err_info) {
        return false;
    }
    BOOL success = Shell_NotifyIcon(NIM_ADD, &nid);
    if (success) {
        nid.uVersion = NOTIFYICON_VERSION_4;
        return Shell_NotifyIcon(NIM_SETVERSION, &nid);
    } else {
        return false;
    }
}

bool delete_notification(HWND hwnd) {
    NOTIFYICONDATA nid;
    memset(&nid, '\0', sizeof(NOTIFYICONDATA));
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = NOTIFIER_ICON_UID;
    return Shell_NotifyIcon(NIM_DELETE, &nid);
}

bool show_context_menu(HWND hwnd, int x, int y) {
    HMENU menu = LoadMenu(NOTIFIER_HANDLE_INSTANCE, MAKEINTRESOURCE(IDC_CONTEXTMENU));
    if (NULL == menu) {
        return false;
    }
    HMENU submenu = GetSubMenu(menu, 0);
    if (NULL == submenu) {
        return false;
    }
    // our window must be foreground before calling TrackPopupMenu or the menu will not disappear when the user clicks away
    BOOL err_fg = SetForegroundWindow(hwnd);
    if (0 == err_fg) {
        return false;
    }
    // respect menu drop alignment
    UINT uFlags = TPM_RIGHTBUTTON;
    if (0 != GetSystemMetrics(SM_MENUDROPALIGNMENT)) {
        uFlags |= TPM_RIGHTALIGN;
    } else {
        uFlags |= TPM_LEFTALIGN;
    }
    BOOL err_track = TrackPopupMenuEx(submenu, uFlags, x, y, hwnd, NULL);
    if (0 == err_track) {
        return false;
    }
    return DestroyMenu(menu);
}

bool open_browser() {
    std::wstring url = load_resource_string(IDS_BROWSER_URL);
    HINSTANCE res = ShellExecuteW(NULL, NULL, url.c_str(), NULL, NULL, SW_SHOW);
    return reinterpret_cast<int>(res) > 32;
}

bool show_about_dialog(HWND hwnd) {
    if (STATE_UPDATE == NOTIFIER_STATE) {
        return true;
    }
    State prev = NOTIFIER_STATE;
    NOTIFIER_STATE = STATE_ABOUT;
    std::wstring title = load_resource_string(IDS_ABOUT_TITLE);
    std::wstring header = load_resource_string(IDS_ABOUT_HEADER);
    std::wstring text = load_resource_string(IDS_ABOUT_TEXT);
    HRESULT res = TaskDialog(hwnd, NOTIFIER_HANDLE_INSTANCE, title.c_str(), header.c_str(), text.c_str(),
            TDCBF_CLOSE_BUTTON, MAKEINTRESOURCE(IDI_NOTIFICATIONICON), NULL);
    NOTIFIER_STATE = prev;
    return S_OK == res;
}

void show_update_dialog(HWND hwnd) {
    NOTIFIER_STATE = STATE_UPDATE;
    int chosen = 0;
    std::wstring title = load_resource_string(IDS_UPDATE_TITLE);
    HRESULT res = TaskDialog(hwnd, NOTIFIER_HANDLE_INSTANCE, title.c_str(), NOTIFIER_UPDATE_HEADER.c_str(),
            NOTIFIER_UPDATE_TEXT.c_str(), TDCBF_YES_BUTTON | TDCBF_CANCEL_BUTTON,
            MAKEINTRESOURCE(IDI_NOTIFICATIONICON), &chosen);
    if (S_OK == res && IDYES == chosen) {
        open_browser();
    }
    DestroyWindow(hwnd);
}

bool restore_tooltip(HWND hwnd) {
    NOTIFYICONDATA nid;
    memset(&nid, '\0', sizeof(NOTIFYICONDATA));
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = NOTIFIER_ICON_UID;
    nid.uFlags = NIF_SHOWTIP;
    return Shell_NotifyIcon(NIM_MODIFY, &nid);
}

LRESULT CALLBACK window_callback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE: {
            bool success = add_notification(hwnd);
            if (!success) {
                return -1;
            }
            NOTIFIER_STATE = STATE_BALLOON;
        } break;
    case WM_COMMAND: {
            int const wmId = LOWORD(wParam);
            switch (wmId) {
            case IDM_DOWNLOAD:
                open_browser();
                DestroyWindow(hwnd);
                break;
            case IDM_ABOUT: {
                    bool success = show_about_dialog(hwnd);
                    if (!success) {
                        DestroyWindow(hwnd);
                    }
                }
                break;
            case IDM_CANCEL:
                DestroyWindow(hwnd);
                break;
            default:
                return DefWindowProc(hwnd, message, wParam, lParam);
            }
        } break;
    case WMAPP_NOTIFYCALLBACK:
        switch (LOWORD(lParam)) {
            case WM_MOUSEMOVE:
                if (STATE_BALLOON == NOTIFIER_STATE) {
                    NOTIFIER_STATE = STATE_HOVERED;
                }
                break;
            case NIN_SELECT:
                if (STATE_UPDATE != NOTIFIER_STATE && STATE_ABOUT != NOTIFIER_STATE) {
                    show_update_dialog(hwnd);
                }
                break;
            case NIN_BALLOONUSERCLICK:
                if (STATE_BALLOON == NOTIFIER_STATE || STATE_CLICKED == NOTIFIER_STATE) {
                    show_update_dialog(hwnd);
                } else if (STATE_HOVERED == NOTIFIER_STATE) {
                    NOTIFIER_STATE = STATE_CLICKED;
                }
                break;
            case NIN_BALLOONTIMEOUT:
                if (STATE_BALLOON == NOTIFIER_STATE) {
                    DestroyWindow(hwnd);
                }
                break;
            case WM_CONTEXTMENU:
                if (STATE_UPDATE != NOTIFIER_STATE && STATE_ABOUT != NOTIFIER_STATE) {
                    restore_tooltip(hwnd);
                    bool success = show_context_menu(hwnd, LOWORD(wParam), HIWORD(wParam));
                    if (!success) {
                        DestroyWindow(hwnd);
                    }
                }
                break;
        } break;
    case WM_DESTROY:
        delete_notification(hwnd);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

} // namespace

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR /*lpCmdLine*/, int /* nCmdShow */) {
    // check we are alone
    std::wstring mutex_uid = load_resource_string(IDS_INSTANCE_MUTEX_UUID);
    ch::utils::NamedMutex mutex(mutex_uid);
    if (mutex.already_taken()) {
        return 0;
    }
    // fill globals
    NOTIFIER_HANDLE_INSTANCE = hInstance;
    bool err_vcheck = load_input_json();
    if (!err_vcheck) {
        return 0; // err handling is too coarse here
    }
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
        return 1;
    }
    HWND hwnd = CreateWindowExW(0, NOTIFIER_WINDOW_CLASS.c_str(), NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);
    if (NULL == hwnd) {
        return 1;
    }
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

