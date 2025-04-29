//This stores the cmd into the atom, retrieves and then executes - no shellcode involved. 

#include <windows.h>
#include <iostream>

int main() {
    // Step 1: Add command to Atom Table
    const char* command = "cmd /c start calc.exe";
    ATOM atom = AddAtomA(command);
    if (!atom) {
        std::cerr << "[-] AddAtomA failed: " << GetLastError() << std::endl;
        return 1;
    }
    std::cout << "[+] Atom created with ID: 0x" << std::hex << atom << std::endl;

    // Step 2: Retrieve command from Atom Table
    char buffer[256] = { 0 };
    if (GetAtomNameA(atom, buffer, sizeof(buffer)) == 0) {
        std::cerr << "[-] GetAtomNameA failed: " << GetLastError() << std::endl;
        return 1;
    }
    std::cout << "[+] Retrieved command: " << buffer << std::endl;

    // Step 3: Launch it directly
    UINT result = WinExec(buffer, SW_SHOWNORMAL);
    if (result <= 31) {
        std::cerr << "[-] WinExec failed with code: " << result << std::endl;
        return 1;
    }

    std::cout << "[+] Successfully launched command from Atom Table!" << std::endl;
    return 0;
}
