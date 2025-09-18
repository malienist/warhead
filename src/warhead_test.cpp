#include "warhead.h"
#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <stdexcept>

// Helper function to run a single test case
void runTest(const std::string& testName, void (*testFunction)()) {
    std::cout << "Running test: " << testName << "..." << std::endl;
    try {
        testFunction();
        std::cout << "Test '" << testName << "' PASSED." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test '" << testName << "' FAILED: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Test '" << testName << "' FAILED with unknown exception." << std::endl;
    }
}

// Test case for successful encryption, decryption, and deletion
void testEncryptionDecryption() {
    std::wstring originalCommand = L"notepad.exe";
    std::vector<ATOM> atoms;

    // Test encryption
    atoms = warhead::storeEncryptedCommandInAtoms(originalCommand);
    assert(!atoms.empty());

    // Test decryption
    std::wstring decryptedCommand = warhead::retrieveAndDecryptCommandFromAtoms(atoms);
    assert(decryptedCommand == originalCommand);

    // Test deletion
    warhead::deleteAtoms(atoms);
    assert(GlobalGetAtomNameW(atoms[0], nullptr, 0) == 0); // Check if atom is gone
}

// Test case for a command longer than 255 characters (multi-atom storage)
void testMultiAtomCommand() {
    std::wstring originalCommand(256, L'A'); // 256 'A's
    std::vector<ATOM> atoms;

    // Test multi-atom encryption
    atoms = warhead::storeEncryptedCommandInAtoms(originalCommand);
    assert(atoms.size() > 1);

    // Test multi-atom decryption
    std::wstring decryptedCommand = warhead::retrieveAndDecryptCommandFromAtoms(atoms);
    assert(decryptedCommand == originalCommand);
    
    // Test deletion
    warhead::deleteAtoms(atoms);
}

// Test case for handling empty command string
void testEmptyCommand() {
    std::wstring originalCommand = L"";
    std::vector<ATOM> atoms;

    // Test encryption with empty string
    atoms = warhead::storeEncryptedCommandInAtoms(originalCommand);
    assert(!atoms.empty()); // The IV and padding should still result in atoms

    // Test decryption with empty string
    std::wstring decryptedCommand = warhead::retrieveAndDecryptCommandFromAtoms(atoms);
    assert(decryptedCommand == originalCommand);

    // Test deletion
    warhead::deleteAtoms(atoms);
}

// Test case for handling invalid ATOM handles (e.g., trying to decrypt a non-existent atom)
void testInvalidAtoms() {
    // Create a vector with a known bad ATOM handle
    std::vector<ATOM> invalidAtoms = {0xFFFF}; // ATOMs are non-zero

    bool caughtException = false;
    try {
        warhead::retrieveAndDecryptCommandFromAtoms(invalidAtoms);
    } catch (const std::runtime_error& e) {
        caughtException = true;
    }
    assert(caughtException);
}

int main() {
    runTest("Encryption, Decryption, and Deletion", testEncryptionDecryption);
    runTest("Multi-Atom Command Handling", testMultiAtomCommand);
    runTest("Empty Command Handling", testEmptyCommand);
    runTest("Invalid Atom Handling", testInvalidAtoms);

    std::cout << "All tests completed." << std::endl;

    return 0;
}
