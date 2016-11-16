
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

enum State { STATE_STANDBY, STATE_ABOUT, STATE_UPDATE };
class __declspec(uuid("7cf53058-6728-45bc-a0e6-1ed7629708bc")) NOTIFIER_ICON;
const std::wstring NOTIFIER_WINDOW_CLASS = L"e48e2be7-451e-48e5-8889-8c97c1485340";
const UINT WMAPP_NOTIFYCALLBACK = WM_APP + 1;
const size_t NOTIFIER_MAX_RC_LEN = 1 << 12;
const size_t NOTIFIER_MAX_INPUT_JSON_LEN = 1 << 15;
HINSTANCE NOTIFIER_HANDLE_INSTANCE = NULL;
// loaded on startup
std::wstring NOTIFIER_BALLOON_TEXT;
std::wstring NOTIFIER_UPDATE_HEADER;
std::wstring NOTIFIER_UPDATE_TEXT;
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

BOOL load_input_json() {
    try {
        std::string exepath = ch::platform::current_executable_path();
        std::string dirpath = ch::utils::strip_filename(exepath);
        std::string path = dirpath + "version.json";
        ch::JsonRecord json = ch::read_from_file(path, NOTIFIER_MAX_INPUT_JSON_LEN);
        ch::Version ver(json);
        NOTIFIER_BALLOON_TEXT = ch::utils::widen(ver.ui_balloon_text);
        NOTIFIER_UPDATE_HEADER = ch::utils::widen(ver.ui_update_header);
        NOTIFIER_UPDATE_TEXT = ch::utils::widen(ver.ui_update_text);
        std::wstring vnumwstr = load_resource_string(IDS_SHIPPED_VERSION_NUMBER);
        std::string vnumstr = ch::utils::narrow(vnumwstr);
        uint32_t vnum = ch::utils::parse_uint32(vnumstr);
        return ver.version_number > vnum;
    } catch (const std::exception& e) {
        return false;
    }
}

BOOL add_notification(HWND hwnd) {
    NOTIFYICONDATA nid;
    memset(&nid, '\0', sizeof(NOTIFYICONDATA));
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uFlags = NIF_INFO | NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP | NIF_GUID;
    nid.guidItem = __uuidof(NOTIFIER_ICON);
    nid.uCallbackMessage = WMAPP_NOTIFYCALLBACK;
    LoadIconMetric(NOTIFIER_HANDLE_INSTANCE, MAKEINTRESOURCE(IDI_NOTIFICATIONICON), LIM_SMALL, &nid.hIcon);
    LoadStringW(NOTIFIER_HANDLE_INSTANCE, IDS_TOOLTIP, nid.szTip, ARRAYSIZE(nid.szTip));
    nid.dwInfoFlags = NIIF_INFO | NIIF_NOSOUND | NIIF_RESPECT_QUIET_TIME;
    LoadStringW(NOTIFIER_HANDLE_INSTANCE, IDS_BALLOON_TITLE, nid.szInfoTitle, ARRAYSIZE(nid.szInfoTitle));
    wcscpy_s(nid.szInfo, ARRAYSIZE(nid.szInfo), NOTIFIER_BALLOON_TEXT.c_str());

    Shell_NotifyIcon(NIM_ADD, &nid);
    nid.uVersion = NOTIFYICON_VERSION_4;
    return Shell_NotifyIcon(NIM_SETVERSION, &nid);
}

BOOL delete_notification() {
    NOTIFYICONDATA nid;
    memset(&nid, '\0', sizeof(NOTIFYICONDATA));
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.uFlags = NIF_GUID;
    nid.guidItem = __uuidof(NOTIFIER_ICON);
    return Shell_NotifyIcon(NIM_DELETE, &nid);
}

BOOL restore_tooltip() {
    NOTIFYICONDATA nid;
    memset(&nid, '\0', sizeof(NOTIFYICONDATA));
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.uFlags = NIF_SHOWTIP | NIF_GUID;
    nid.guidItem = __uuidof(NOTIFIER_ICON);
    return Shell_NotifyIcon(NIM_MODIFY, &nid);
}

void show_context_menu(HWND hwnd, int x, int y) {
    HMENU hMenu = LoadMenu(NOTIFIER_HANDLE_INSTANCE, MAKEINTRESOURCE(IDC_CONTEXTMENU));
    if (hMenu) {
        HMENU hSubMenu = GetSubMenu(hMenu, 0);
        if (hSubMenu) {
            // our window must be foreground before calling TrackPopupMenu or the menu will not disappear when the user clicks away
            SetForegroundWindow(hwnd);
            // respect menu drop alignment
            UINT uFlags = TPM_RIGHTBUTTON;
            if (0 != GetSystemMetrics(SM_MENUDROPALIGNMENT)) {
                uFlags |= TPM_RIGHTALIGN;
            } else {
                uFlags |= TPM_LEFTALIGN;
            }
            TrackPopupMenuEx(hSubMenu, uFlags, x, y, hwnd, NULL);
        }
        DestroyMenu(hMenu);
    }
}

void open_browser() {
    std::wstring url = load_resource_string(IDS_BROWSER_URL);
    ShellExecuteW(NULL, NULL, url.c_str(), NULL, NULL, SW_SHOW);
}

void show_about_dialog(HWND hwnd) {
    if (STATE_STANDBY != NOTIFIER_STATE) {
        return;
    }
    NOTIFIER_STATE = STATE_ABOUT;
    std::wstring title = load_resource_string(IDS_ABOUT_TITLE);
    std::wstring header = load_resource_string(IDS_ABOUT_HEADER);
    std::wstring text = load_resource_string(IDS_ABOUT_TEXT);
    TaskDialog(hwnd, NOTIFIER_HANDLE_INSTANCE, title.c_str(), header.c_str(), text.c_str(),
            TDCBF_CLOSE_BUTTON, MAKEINTRESOURCE(IDI_NOTIFICATIONICON), NULL);
    NOTIFIER_STATE = STATE_STANDBY;
}

void show_update_dialog(HWND hwnd) {
    if (STATE_STANDBY != NOTIFIER_STATE) {
        return;
    }
    NOTIFIER_STATE = STATE_UPDATE;
    int chosen = 0;
    std::wstring title = load_resource_string(IDS_UPDATE_TITLE);
    TaskDialog(hwnd, NOTIFIER_HANDLE_INSTANCE, title.c_str(), NOTIFIER_UPDATE_HEADER.c_str(),
            NOTIFIER_UPDATE_TEXT.c_str(), TDCBF_YES_BUTTON | TDCBF_CANCEL_BUTTON,
            MAKEINTRESOURCE(IDI_NOTIFICATIONICON), &chosen);
    if (IDYES == chosen) {
        open_browser();
    }
    DestroyWindow(hwnd);
}

LRESULT CALLBACK window_callback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        if (!add_notification(hwnd)) {
            return -1;
        }
        break;
    case WM_COMMAND: {
            int const wmId = LOWORD(wParam);
            switch (wmId) {
            case IDM_DOWNLOAD:
                open_browser();
                DestroyWindow(hwnd);
                break;
            case IDM_ABOUT:
                show_about_dialog(hwnd);
                break;
            case IDM_CANCEL:
                DestroyWindow(hwnd);
                break;
            default:
                return DefWindowProc(hwnd, message, wParam, lParam);
            }
        }
        break;

    case WMAPP_NOTIFYCALLBACK:
        switch (LOWORD(lParam)) {
        case NIN_SELECT:
            show_update_dialog(hwnd);
            break;
        case NIN_BALLOONTIMEOUT:
            restore_tooltip();
            break;

        case NIN_BALLOONUSERCLICK:
            restore_tooltip();
            show_update_dialog(hwnd);
            break;

        case WM_CONTEXTMENU:
            show_context_menu(hwnd, LOWORD(wParam), HIWORD(wParam));
            break;
        }
        break;

    case WM_TIMER:
        break;
    case WM_DESTROY:
        delete_notification();
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

} // namespace

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR /*lpCmdLine*/, int /* nCmdShow */) {
    // fill globals
    NOTIFIER_HANDLE_INSTANCE = hInstance;
    if (!(load_input_json())) {
        return 1;
    }
    // create ui
    WNDCLASSEX wcex;
    memset(&wcex, '\0', sizeof(wcex));
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.lpfnWndProc = window_callback;
    wcex.hInstance = NOTIFIER_HANDLE_INSTANCE;
    wcex.hIcon = LoadIcon(NOTIFIER_HANDLE_INSTANCE, MAKEINTRESOURCE(IDI_NOTIFICATIONICON));
    wcex.lpszClassName = NOTIFIER_WINDOW_CLASS.c_str();
    RegisterClassExW(&wcex);
    HWND hwnd = CreateWindowExW(0, NOTIFIER_WINDOW_CLASS.c_str(), NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);
    if (hwnd) {
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return 0;
}

