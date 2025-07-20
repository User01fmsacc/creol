#include <windows.h>
#include <iostream>

POINT cursorPos = {0};

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_INPUT) {
        UINT dwSize = 0;
        GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
        BYTE lpb[64];

        if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
            return 0;

        RAWINPUT* raw = (RAWINPUT*)lpb;

        if (raw->header.dwType == RIM_TYPEMOUSE) {
            LONG dx = raw->data.mouse.lLastX;
            LONG dy = raw->data.mouse.lLastY;

            if (GetCursorPos(&cursorPos)) {
                cursorPos.x += dx;
                cursorPos.y += dy;

                int screenX = GetSystemMetrics(SM_CXSCREEN);
                int screenY = GetSystemMetrics(SM_CYSCREEN);
                cursorPos.x = max(0, min(screenX - 1, cursorPos.x));
                cursorPos.y = max(0, min(screenY - 1, cursorPos.y));

                SetCursorPos(cursorPos.x, cursorPos.y);
            }
        }

        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int main() {
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "TouchScrollInjector";

    RegisterClass(&wc);
    HWND hwnd = CreateWindowEx(0, "TouchScrollInjector", "TouchScrollInjector", 0,
        0, 0, 100, 100, HWND_MESSAGE, NULL, wc.hInstance, NULL);

    RAWINPUTDEVICE rid;
    rid.usUsagePage = 0x01;
    rid.usUsage = 0x02;
    rid.dwFlags = RIDEV_INPUTSINK;
    rid.hwndTarget = hwnd;

    if (!RegisterRawInputDevices(&rid, 1, sizeof(rid))) {
        std::cerr << "RegisterRawInputDevices failed." << std::endl;
        return 1;
    }

    std::cout << "✅ TouchScrollInjector started.\nMove finger on touchpad to drag scroll.\n";

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
