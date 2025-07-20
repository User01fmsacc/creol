#include <windows.h>
#include <iostream>
#include <vector>
#include <string>

#define MAX_BUFFER_SIZE 64

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_INPUT) {
        UINT dwSize = 0;
        GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
        BYTE lpb[MAX_BUFFER_SIZE];

        if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
            return 0;

        RAWINPUT* raw = (RAWINPUT*)lpb;

        if (raw->header.dwType == RIM_TYPEMOUSE) {
            LONG x = raw->data.mouse.lLastX;
            LONG y = raw->data.mouse.lLastY;
            USHORT flags = raw->data.mouse.usButtonFlags;

            std::cout << "Mouse: X=" << x << " Y=" << y << " BtnFlags=" << flags << std::endl;
        }
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int main()
{
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "RawInputLogger";

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "RawInputLogger", "RawInput Logger", 0,
        0, 0, 100, 100, HWND_MESSAGE, NULL, wc.hInstance, NULL);

    RAWINPUTDEVICE rid;
    rid.usUsagePage = 0x01; // Generic desktop controls
    rid.usUsage = 0x02;     // Mouse
    rid.dwFlags = RIDEV_INPUTSINK;
    rid.hwndTarget = hwnd;

    if (!RegisterRawInputDevices(&rid, 1, sizeof(rid))) {
        std::cerr << "RegisterRawInputDevices failed." << std::endl;
        return 1;
    }

    std::cout << "✅ RawInput Logger Started — move touchpad or mouse.\n";
    std::cout << "Press Ctrl+C to exit.\n";

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
