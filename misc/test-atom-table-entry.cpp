#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>

int main() {
    const char* testString = "HelloWarHead123";

    printf("[*] Adding string to Local Atom Table: \"%s\"\n", testString);

    // Add the string to the local atom table
    ATOM atom = AddAtomA(testString);
    if (atom == 0) {
        printf("[!] Failed to add atom. Error: %lu\n", GetLastError());
        return 1;
    }

    printf("[+] Atom added successfully.\n");
    printf("    Atom ID: 0x%04X (%u)\n", atom, atom);

    // Retrieve the string back from the atom ID
    char buffer[256] = {0};
    UINT result = GetAtomNameA(atom, buffer, sizeof(buffer));
    if (result == 0) {
        printf("[!] Failed to retrieve atom string. Error: %lu\n", GetLastError());
        return 1;
    }

    printf("[+] Retrieved Atom String: \"%s\"\n", buffer);

    // Optional: Delete the atom (not necessary for local atoms, but good hygiene)
    DeleteAtom(atom);
    printf("[*] Atom deleted.\n");

    return 0;
}
