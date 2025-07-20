#include <windows.h>
#include <iostream>
#include <cstdio>
#include <vector>
#include <string>

static HANDLE openMouseClass()
{
    for (int i = 0; i < 10; ++i) {
        char path[64];
        std::snprintf(path, sizeof(path), "\\\\.\\MouseClass%d", i);
        HANDLE h = CreateFileA(path,
                               GENERIC_READ,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               NULL,
                               OPEN_EXISTING,
                               0,
                               NULL);
        if (h != INVALID_HANDLE_VALUE) {
            std::cout << "[+] Opened " << path << std::endl;
            return h;
        }
    }
    return INVALID_HANDLE_VALUE;
}

int main(int argc, char* argv[])
{
    // Optional log file
    FILE* flog = NULL;
    if (argc >= 2) {
        flog = std::fopen(argv[1], "wb");
        if (!flog) {
            std::cerr << "[-] Could not open log file " << argv[1] << " for writing.\n";
        } else {
            std::cout << "[+] Logging to " << argv[1] << std::endl;
        }
    }

    HANDLE hMouse = openMouseClass();
    if (hMouse == INVALID_HANDLE_VALUE) {
        std::cerr << "[-] Failed to open any MouseClass device. Error: " << GetLastError() << std::endl;
        std::cerr << "    Tip: Run as Administrator.\n";
        return 1;
    }

    std::cout << "=== ALPS PS/2 Raw Packet Logger ===\n"
                 "Move finger(s) on touchpad.\n"
                 "Press Ctrl+C to stop.\n\n";

    BYTE buffer[32];           // Some drivers send >3 bytes; ALPS often 6–9
    DWORD bytesRead = 0;

    while (true) {
        BOOL ok = ReadFile(hMouse, buffer, sizeof(buffer), &bytesRead, NULL);
        if (!ok) {
            DWORD err = GetLastError();
            if (err == ERROR_OPERATION_ABORTED) break;
            std::cerr << "[-] ReadFile failed. Error: " << err << std::endl;
            Sleep(50);
            continue;
        }
        if (bytesRead == 0) continue;

        std::cout << "Packet:";
        for (DWORD i = 0; i < bytesRead; ++i) {
            std::printf(" %02X", buffer[i]);
        }
        std::cout << std::endl;

        if (flog) {
            for (DWORD i = 0; i < bytesRead; ++i)
                std::fprintf(flog, "%02X ", buffer[i]);
            std::fprintf(flog, "\n");
            std::fflush(flog);
        }
    }

    if (flog) std::fclose(flog);
    CloseHandle(hMouse);
    return 0;
}
