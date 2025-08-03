#include <windows.h>
#include <iostream>

int main(int argc, char* argv[]) {
    // Check if a command string was provided, this is required. 
    // Recommended to try "cmd /c start calc.exe" to test this.
    if (argc < 2) {
        std::cerr << "Usage : simple-atom-writer.exe \"command which is less than 255 characters\"" << std::endl;
        return 1;
    }
    // Build the command string from command line arguments. 
    std::string command;
    for (int i = 1; i < argc; ++i) {
        command += argv[i];
        if (i < argc - 1) {
            command += " ";
        }
    }

    // Limit the command length to be less than 255 as that's the size of an Atom.
    if (command.length() > 255) {
        std::cerr << "Error: Command exceeds maximum length of 255 characters which fits in an Atom." << std::endl;
        return 1;
    }

    // Add the string to the Global Atom Table
    ATOM atom = AddAtom(command.c_str());
    if (atom == 0) {
        std::cerr << "AddAtom failed: " << GetLastError() << std::endl;
        return 1;
    }

    std::cout << "Stored command in Atom Table." << std::endl;
    std::cout << "Atom ID: " << atom << std::endl;

    return 0;
}
