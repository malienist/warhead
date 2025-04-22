#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>

int main() {
    const char* command = "cmd /c start calc.exe";  // can replace with "notepad", "mspaint", "echo hi", etc.

    printf("[*] Storing command in Local Atom Table: \"%s\"\n", command);

    ATOM atom = AddAtomA(command);
    if (atom == 0) {
        printf("[!] Failed to add atom. Error: %lu\n", GetLastError());
        return 1;
    }

    char buffer[256] = { 0 };
    if (GetAtomNameA(atom, buffer, sizeof(buffer)) == 0) {
        printf("[!] Failed to retrieve atom. Error: %lu\n", GetLastError());
        return 1;
    }

    printf("[+] Retrieved Atom String: \"%s\"\n", buffer);

    // Execute command using WinExec (simpler than CreateProcess)
    printf("[*] Executing command via WinExec...\n");
    UINT result = WinExec(buffer, SW_SHOW);
    if (result > 31) {
        printf("[+] Command executed successfully.\n");
    }
    else {
        printf("[!] WinExec failed. Code: %u\n", result);
    }

    return 0;
}
