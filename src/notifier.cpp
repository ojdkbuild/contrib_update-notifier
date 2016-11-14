
#include <cstring>
#include <string>

#include "notifier.hpp"
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <shellapi.h>
#include <commctrl.h>
#include <strsafe.h>

#include "utils.hpp"

namespace utils = checker::utils;

const UINT WMAPP_NOTIFYCALLBACK = WM_APP + 1;
const std::wstring NOTIFIER_WINDOW_CLASS = utils::widen("notifier");
HINSTANCE NOTIFIER_HANDLE_INSTANCE = NULL;
class __declspec(uuid("7cf53058-6728-45bc-a0e6-1ed7629708bc")) NOTIFIER_ICON;

// Forward declarations of functions included in this code module:
LRESULT CALLBACK    window_callback(HWND, UINT, WPARAM, LPARAM);
void                show_context_menu(HWND hwnd, POINT pt);
BOOL                add_notification(HWND hwnd);
BOOL                delete_notification();

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR /*lpCmdLine*/, int /* nCmdShow */) {
    NOTIFIER_HANDLE_INSTANCE = hInstance;
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

BOOL add_notification(HWND hwnd) {
    NOTIFYICONDATA nid;
    memset(&nid, '\0', sizeof(NOTIFYICONDATA));
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uFlags = NIF_INFO | NIF_ICON | NIF_MESSAGE | NIF_SHOWTIP | NIF_GUID;
    nid.guidItem = __uuidof(NOTIFIER_ICON);
    nid.uCallbackMessage = WMAPP_NOTIFYCALLBACK;
    LoadIconMetric(NOTIFIER_HANDLE_INSTANCE, MAKEINTRESOURCE(IDI_NOTIFICATIONICON), LIM_SMALL, &nid.hIcon);
    nid.dwInfoFlags = NIIF_INFO | NIIF_NOSOUND | NIIF_RESPECT_QUIET_TIME;
    LoadString(NOTIFIER_HANDLE_INSTANCE, IDS_LOWINK_TITLE, nid.szInfoTitle, ARRAYSIZE(nid.szInfoTitle));
    LoadString(NOTIFIER_HANDLE_INSTANCE, IDS_LOWINK_TEXT, nid.szInfo, ARRAYSIZE(nid.szInfo));
    Shell_NotifyIcon(NIM_ADD, &nid);
    nid.uVersion = NOTIFYICON_VERSION_4;
    return Shell_NotifyIcon(NIM_SETVERSION, &nid);
}

BOOL delete_notification() {
    NOTIFYICONDATA nid;
    memset(&nid, '\0', sizeof(NOTIFYICONDATA));
    nid.uFlags = NIF_GUID;
    nid.guidItem = __uuidof(NOTIFIER_ICON);
    return Shell_NotifyIcon(NIM_DELETE, &nid);
}

BOOL show_balloon() {
    NOTIFYICONDATA nid;
    memset(&nid, '\0', sizeof(NOTIFYICONDATA));
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.uFlags = NIF_INFO | NIF_GUID;
    nid.guidItem = __uuidof(NOTIFIER_ICON);
    nid.dwInfoFlags = NIIF_INFO | NIIF_NOSOUND | NIIF_RESPECT_QUIET_TIME;
    LoadString(NOTIFIER_HANDLE_INSTANCE, IDS_LOWINK_TITLE, nid.szInfoTitle, ARRAYSIZE(nid.szInfoTitle));
    LoadString(NOTIFIER_HANDLE_INSTANCE, IDS_LOWINK_TEXT, nid.szInfo, ARRAYSIZE(nid.szInfo));
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
            if (GetSystemMetrics(SM_MENUDROPALIGNMENT) != 0) {
                uFlags |= TPM_RIGHTALIGN;
            }
            else {
                uFlags |= TPM_LEFTALIGN;
            }

            TrackPopupMenuEx(hSubMenu, uFlags, x, y, hwnd, NULL);
        }
        DestroyMenu(hMenu);
    }
}

LRESULT CALLBACK window_callback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        // add the notification icon
        if (!add_notification(hwnd)) {
            return -1;
        }
        break;
    case WM_COMMAND: {
            int const wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId) {
            case IDM_LOWINK:
                break;

            case IDM_NOINK:
                break;

            case IDM_PRINTJOB:
                break;

            case IDM_OPTIONS:
                // placeholder for an options dialog
                MessageBox(hwnd,  L"Display the options dialog here.", L"Options", MB_OK);
                break;

            case IDM_EXIT:
                DestroyWindow(hwnd);
                break;

            case IDM_FLYOUT:
                break;

            default:
                return DefWindowProc(hwnd, message, wParam, lParam);
            }
        }
        break;

    case WMAPP_NOTIFYCALLBACK:
        switch (LOWORD(lParam)) {
        case NIN_SELECT:
            show_balloon();
            break;
        case NIN_BALLOONTIMEOUT:
            break;

        case NIN_BALLOONUSERCLICK:
            // placeholder for the user clicking on the balloon.
            MessageBox(hwnd, L"The user clicked on the balloon.", L"User click", MB_OK);
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

