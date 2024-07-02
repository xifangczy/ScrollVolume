#include <windows.h>
#include <iostream>
#include <tchar.h>
#include <shellapi.h>
#include "resource.h"


bool isVolumeControlActive = false; // 激活滚轮调节音量
bool enableTaskbarVolumeControl = false;    // 激活任务栏调节音量
bool enableTrayIcon = true; // 启用托盘图标

// 定义全局变量
HHOOK mouseHook = NULL;
NOTIFYICONDATA nid = {};
HMENU hMenu = NULL;

UINT modKey = MOD_SHIFT | MOD_CONTROL | MOD_ALT; // 默认修饰键
UINT vkKey = 'V'; // 默认主键

// 检查窗口是否是任务栏或其子窗口
bool IsTaskbarWindow(HWND hWnd) {
    TCHAR className[256];
    while (hWnd != NULL) {
        GetClassName(hWnd, className, 256);
        if (_tcscmp(className, _T("Shell_TrayWnd")) == 0) {
            return true;
        }
        hWnd = GetParent(hWnd);
    }
    return false;
}

// 检查鼠标是否在任务栏上
bool IsMouseOnTaskbar() {
    if (!enableTaskbarVolumeControl) {
        return false;
    }
    POINT pt;
    GetCursorPos(&pt);
    HWND hWnd = WindowFromPoint(pt);
    return IsTaskbarWindow(hWnd);
}

// 鼠标钩子回调函数
LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        if (wParam == WM_MOUSEWHEEL) {
            MSLLHOOKSTRUCT* mouseStruct = (MSLLHOOKSTRUCT*)lParam;
            int zDelta = GET_WHEEL_DELTA_WPARAM(mouseStruct->mouseData);

            // 调节音量
            if (isVolumeControlActive || IsMouseOnTaskbar()) {
                if (zDelta > 0) {
                    keybd_event(VK_VOLUME_UP, 0, 0, 0);
                }
                else {
                    keybd_event(VK_VOLUME_DOWN, 0, 0, 0);
                }
                return true; // 截获消息，不传递给其他应用程序
            }
        }
    }
    return CallNextHookEx(mouseHook, nCode, wParam, lParam);
}

// 托盘图标
void UpdateTrayIcon() {
    wcscpy_s(nid.szTip, L"Scroll Volume");
    if (isVolumeControlActive) {
        wcscat_s(nid.szTip, L" (Active)");
    }
    if (enableTaskbarVolumeControl) {
        wcscat_s(nid.szTip, L" (Taskbar Control)");
    }
    Shell_NotifyIcon(NIM_MODIFY, &nid);
}
void AddTrayIcon(HWND hwnd, HINSTANCE hInstance) {
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_APP;
    nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    UpdateTrayIcon();
    Shell_NotifyIcon(NIM_ADD, &nid);
}
void RemoveTrayIcon(HWND hwnd) {
    Shell_NotifyIcon(NIM_DELETE, &nid);
}
// 右键菜单
void ShowContextMenu(HWND hwnd, POINT pt) {
    hMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING | (isVolumeControlActive ? MF_CHECKED : 0), 1, L"Activate Volume Control");
    AppendMenu(hMenu, MF_STRING | (enableTaskbarVolumeControl ? MF_CHECKED : 0), 2, L"Enable Taskbar Volume Control");
    AppendMenu(hMenu, MF_STRING, 3, L"Exit");
    SetForegroundWindow(hwnd);
    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
    DestroyMenu(hMenu);
}

// 窗口过程函数
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_HOTKEY:
        if (wParam == 1) {
            // 切换音量控制模式
            isVolumeControlActive = !isVolumeControlActive;
        }
        break;
    case WM_COMMAND:
            switch (LOWORD(wParam)) {
            case 1:
                isVolumeControlActive = !isVolumeControlActive;
                UpdateTrayIcon();
                break;
            case 2:
                enableTaskbarVolumeControl = !enableTaskbarVolumeControl;
                UpdateTrayIcon();
                break;
            case 3:
                PostQuitMessage(0);
                break;
            }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_APP:
        if (lParam == WM_RBUTTONUP) {
            POINT pt;
            GetCursorPos(&pt);
            ShowContextMenu(hwnd, pt);
        }
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

// 解析启动参数
void ParseArguments(LPSTR lpCmdLine) {
    std::string cmdLine(lpCmdLine);
    size_t pos = 0;
    std::string token;

    while ((pos = cmdLine.find(' ')) != std::string::npos || !cmdLine.empty()) {
        if (pos == std::string::npos) {
            token = cmdLine;
            cmdLine.clear();
        }
        else {
            token = cmdLine.substr(0, pos);
            cmdLine.erase(0, pos + 1);
        }

        if (token == "-hotkey") {
            pos = cmdLine.find(' ');
            std::string hotkeyStr = (pos == std::string::npos) ? cmdLine : cmdLine.substr(0, pos);
            cmdLine.erase(0, pos + 1);

            modKey = 0;
            size_t plusPos = 0;
            while ((plusPos = hotkeyStr.find('+')) != std::string::npos) {
                std::string modStr = hotkeyStr.substr(0, plusPos);
                hotkeyStr.erase(0, plusPos + 1);

                if (modStr == "SHIFT") modKey |= MOD_SHIFT;
                if (modStr == "CONTROL") modKey |= MOD_CONTROL;
                if (modStr == "ALT") modKey |= MOD_ALT;
                if (modStr == "WIN") modKey |= MOD_WIN;
            }
            vkKey = VkKeyScanA(hotkeyStr[0]);
        }
        else if (token == "-taskbar") {
            pos = cmdLine.find(' ');
            std::string taskbarStr = (pos == std::string::npos) ? cmdLine : cmdLine.substr(0, pos);
            cmdLine.erase(0, pos + 1);
            enableTaskbarVolumeControl = (taskbarStr == "on");
        }else if (token == "-trayicon") {
            pos = cmdLine.find(' ');
            std::string taskbarStr = (pos == std::string::npos) ? cmdLine : cmdLine.substr(0, pos);
            cmdLine.erase(0, pos + 1);
            enableTrayIcon = (taskbarStr == "on");
        }
    }
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // 解析启动参数
    ParseArguments(lpCmdLine);

    // 注册窗口类
    const wchar_t* className = L"ScrollVolumeClass";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = className;
    RegisterClass(&wc);

    // 创建隐形窗口
    HWND hwnd = CreateWindowEx(0, className, L"Scroll Volume", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL);

    if (enableTrayIcon) {
        // 添加托盘图标
        AddTrayIcon(hwnd, hInstance);
    }

    // 注册全局热键
    RegisterHotKey(hwnd, 1, modKey, vkKey);

    // 设置鼠标钩子
    mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, hInstance, 0);

    // 消息循环
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // 释放钩子
    UnhookWindowsHookEx(mouseHook);

    // 移除托盘图标
    RemoveTrayIcon(hwnd);

    return 0;
}