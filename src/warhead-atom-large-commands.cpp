#include <iostream>
#include <string>
#include <vector>
#include <windows.h>

// Function to store a larger string in the global Atom table, elevated perms required.
// Splits into 255-character chunks if required.
std::vector<ATOM> storeCommandInAtoms(const std::wstring& command) {
    std::vector<ATOM> atomList;

    for (size_t i = 0; i < command.length(); i += 255) {
        std::wstring chunk = command.substr(i, 255);
        ATOM atom = GlobalAddAtomW(chunk.c_str());

        if (atom == 0) {
        std::wcerr << L"AtomError adding atom: " << GetLastError() << std::endl;
            exit(1);
        }
        else {
            atomList.push_back(atom);
            std::wcout << L"Atom added: " << atom << std::endl;
        }
    }

    return atomList;
}

// Retrieve the command strings from the atom table using a provided list of atoms.
std::wstring retrieveCommandFromAtoms(const std::vector<ATOM>& atoms) {
    std::wstring retrievedCommand;
    for (ATOM atom : atoms) {
        wchar_t* buffer = nullptr;
        int charCount = GlobalGetAtomNameW(atom, buffer, 0);

        if (charCount == 0) {
            std::wcerr << L"AtomError getting the atom name size: " << GetLastError() << std::endl;
            exit(1);
        }
        else {

            buffer = new wchar_t[charCount + 1];
            if (GlobalGetAtomNameW(atom, buffer, charCount + 1) != 0) {
                retrievedCommand += buffer;
            }
            else {
                std::wcerr << L"AtomError: getting atom name: " << GetLastError() << std::endl;
                exit(1);
            }
            delete[] buffer;
        }
    }
    return retrievedCommand;
}

int main() {
    std::wstring command;
    std::wcout << L"Enter the command: ";
    std::getline(std::wcin, command);

    std::vector<ATOM> atoms = storeCommandInAtoms(command);
    if (!atoms.empty()) {
        std::wstring retrievedCommand = retrieveCommandFromAtoms(atoms);
        std::wcout << L"Retrieved command: " << retrievedCommand << std::endl;

        int size_needed = WideCharToMultiByte(CP_ACP, 0, retrievedCommand.c_str(), -1, NULL, 0, NULL, NULL);
        std::string commandA(size_needed, 0);
        WideCharToMultiByte(CP_ACP, 0, retrievedCommand.c_str(), -1, &commandA[0], size_needed, NULL, NULL);
        UINT result = WinExec(commandA.c_str(), SW_SHOW);

        if (result < 32) {
            std::cerr << "AtomError executing the command: " << result << std::endl;
            exit(1);
        }

        for (ATOM atom : atoms) {
            GlobalDeleteAtom(atom);
        }
    }
}