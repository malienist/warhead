#include <windows.h>
#include <iostream>

int main() {
    const char* command = "cmd /c start calc.exe";

    // Add the string to the Global Atom Table
    ATOM atom = GlobalAddAtomA(command);
    if (atom == 0) {
        std::cerr << "GlobalAddAtom failed: " << GetLastError() << std::endl;
        return 1;
    }

    std::cout << "Stored command in Atom Table." << std::endl;
    std::cout << "Atom ID: " << atom << std::endl;

    return 0;
}
