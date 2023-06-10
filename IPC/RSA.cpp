#include "RSA.hpp"

#pragma warning(disable : 4996)

namespace security
{
    namespace details
    {
        // Function to generate RSA key pair and return the public key
        RSA* generateKeyPair(std::string& publicKey, std::string& privateKey)
        {
            RSA* rsaKeyPair = nullptr;
            BIGNUM* bne = nullptr;
            BIO* privateKeyBio = nullptr;
            BIO* publicKeyBio = nullptr;

            // Generate RSA key pair
            bne = BN_new();
            if (BN_set_word(bne, RSA_F4) != 1)
            {
                std::cerr << "Failed to set RSA exponent." << std::endl;
                return nullptr;
            }

            rsaKeyPair = RSA_new();
            if (RSA_generate_key_ex(rsaKeyPair, 2048, bne, nullptr) != 1)
            {
                std::cerr << "Failed to generate RSA key pair." << std::endl;
                RSA_free(rsaKeyPair);
                return nullptr;
            }

            // Get public key in PEM format
            publicKeyBio = BIO_new(BIO_s_mem());
            if (PEM_write_bio_RSAPublicKey(publicKeyBio, rsaKeyPair) != 1)
            {
                std::cerr << "Failed to write RSA public key." << std::endl;
                RSA_free(rsaKeyPair);
                BIO_free_all(publicKeyBio);
                return nullptr;
            }

            // Read public key from BIO into a string
            char* publicKeyBuffer = nullptr;
            long publicKeySize = BIO_get_mem_data(publicKeyBio, &publicKeyBuffer);
            publicKey.assign(publicKeyBuffer, publicKeySize);

            // Get private key in PEM format
            privateKeyBio = BIO_new(BIO_s_mem());
            if (PEM_write_bio_RSAPrivateKey(privateKeyBio, rsaKeyPair, nullptr, nullptr, 0, nullptr, nullptr) != 1)
            {
                std::cerr << "Failed to write RSA private key." << std::endl;
                RSA_free(rsaKeyPair);
                BIO_free_all(publicKeyBio);
                BIO_free_all(privateKeyBio);
                return nullptr;
            }

            // Read private key from BIO into a string
            char* privateKeyBuffer = nullptr;
            long privateKeySize = BIO_get_mem_data(privateKeyBio, &privateKeyBuffer);
            privateKey.assign(privateKeyBuffer, privateKeySize);

            // Clean up
            BN_free(bne);
            BIO_free_all(publicKeyBio);
            BIO_free_all(privateKeyBio);

            return rsaKeyPair;
        }

        // Function to read a public key from a string
        RSA* readPublicKeyFromString(const std::string& publicKeyStr)
        {
            RSA* rsaPublicKey = nullptr;
            BIO* publicKeyBio = BIO_new_mem_buf(publicKeyStr.c_str(), -1);

            if (!publicKeyBio)
            {
                std::cerr << "Failed to create BIO for public key." << std::endl;
                return nullptr;
            }

            rsaPublicKey = PEM_read_bio_RSAPublicKey(publicKeyBio, nullptr, nullptr, nullptr);

            if (!rsaPublicKey)
            {
                std::cerr << "Failed to read public key from string." << std::endl;
                BIO_free_all(publicKeyBio);
                return nullptr;
            }

            BIO_free_all(publicKeyBio);

            return rsaPublicKey;
        }

        // Function to encrypt a message using the public key
        std::string encryptMessage(const std::string& message, RSA* publicKey)
        {
            int encryptedSize = RSA_size(publicKey);
            std::vector<unsigned char> encrypted(encryptedSize);

            int result = RSA_public_encrypt(
                static_cast<int>(message.size()),
                reinterpret_cast<const unsigned char*>(message.c_str()),
                encrypted.data(),
                publicKey,
                RSA_PKCS1_PADDING
            );

            if (result == -1)
            {
                std::cerr << "Failed to encrypt the message." << std::endl;
                return "";
            }

            return std::string(encrypted.begin(), encrypted.begin() + result);
        }

        // Function to decrypt a message using the private key
        std::string decryptMessage(const std::string& encryptedMessage, RSA* privateKey)
        {
            int decryptedSize = RSA_size(privateKey);
            std::vector<unsigned char> decrypted(decryptedSize);

            int result = RSA_private_decrypt(
                static_cast<int>(encryptedMessage.size()),
                reinterpret_cast<const unsigned char*>(encryptedMessage.c_str()),
                decrypted.data(),
                privateKey,
                RSA_PKCS1_PADDING
            );

            if (result == -1)
            {
                std::cerr << "Failed to decrypt the message." << std::endl;
                return "";
            }

            return std::string(decrypted.begin(), decrypted.begin() + result);
        }
    } // namespace details

    // std::string publicKeyStr;
    // std::string privateKeyStr;
    // RSA* publicKey
    // RSA* privateKey 
    // de salvat in doua clase
    void a(){
        std::string publicKeyStr;
        std::string privateKeyStr;
        RSA* publicKey = details::generateKeyPair(publicKeyStr, privateKeyStr);
        if (!publicKey)
        {
            std::cerr << "Failed to generate RSA key pair." << std::endl;
            return 1;
        }

        // Encrypt a message using the public key
        std::string message = "Hello, RSA!";
        std::string encryptedMessage = details::encryptMessage(message, publicKey);

        // Display the public key and encrypted message
        std::cout << "Public Key:\n" << publicKeyStr << std::endl;
        std::cout << "Encrypted Message: " << encryptedMessage << std::endl;

        // Load the private key from memory
        BIO* privateKeyBio = BIO_new_mem_buf(privateKeyStr.c_str(), -1);
        RSA* privateKey = PEM_read_bio_RSAPrivateKey(privateKeyBio, nullptr, nullptr, nullptr);
        BIO_free(privateKeyBio);

        if (!privateKey)
        {
            std::cerr << "Failed to load private key." << std::endl;
            RSA_free(publicKey);
            return 1;
        }

        // Decrypt the message using the private key
        std::string decryptedMessage = details::decryptMessage(encryptedMessage, privateKey);

        // Display the decrypted message
        std::cout << "Decrypted Message: " << decryptedMessage << std::endl;

        // Clean up
        RSA_free(publicKey);
        RSA_free(privateKey);
    }
} // namespace security


#pragma warning(default : 4496)



