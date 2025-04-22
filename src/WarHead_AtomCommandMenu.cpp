#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <string.h>

void execute_command_from_atom(const char* command) {
    printf("[*] Storing command in Atom Table: \"%s\"\n", command);
    ATOM atom = AddAtomA(command);
    if (atom == 0) {
        printf("[!] Failed to add atom. Error: %lu\n", GetLastError());
        return;
    }

    char buffer[256] = { 0 };
    if (GetAtomNameA(atom, buffer, sizeof(buffer)) == 0) {
        printf("[!] Failed to retrieve atom. Error: %lu\n", GetLastError());
        return;
    }

    printf("[+] Retrieved Atom String: \"%s\"\n", buffer);
    printf("[*] Executing via WinExec...\n");

    UINT result = WinExec(buffer, SW_SHOW);
    if (result > 31) {
        printf("[+] Command executed successfully.\n");
    }
    else {
        printf("[!] WinExec failed. Code: %u\n", result);
    }

    DeleteAtom(atom);
}

int main() {
    char input[256] = { 0 };
    int choice;

    while (1) {
        printf("\n=== WarHead Atom Command Launcher ===\n");
        printf("1. Launch calc.exe\n");
        printf("2. Enter your own command\n");
        printf("3. Exit\n");
        printf("Select an option: ");
        fgets(input, sizeof(input), stdin);
        choice = atoi(input);

        switch (choice) {
        case 1:
            execute_command_from_atom("cmd /c start calc.exe");
            break;
        case 2:
            printf("Enter your command: ");
            fgets(input, sizeof(input), stdin);
            input[strcspn(input, "\n")] = 0;  // remove newline
            if (strlen(input) > 0) {
                execute_command_from_atom(input);
            }
            break;
        case 3:
            printf("Exiting...\n");
            return 0;
        default:
            printf("[!] Invalid option. Try again.\n");
        }
    }
}
