#include "RSA.hpp"

#include <vector>

#pragma warning(disable : 4996)

namespace security
{
    RSA* RSAWrapper::generateKeyPair(std::string& publicKey)
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

        privateKeyBio = BIO_new(BIO_s_mem());
        if (PEM_write_bio_RSAPrivateKey(privateKeyBio, rsaKeyPair, nullptr, nullptr, 0, nullptr, nullptr) != 1)
        {
            std::cerr << "Failed to write RSA private key." << std::endl;
            RSA_free(rsaKeyPair);
            BIO_free_all(publicKeyBio);
            BIO_free_all(privateKeyBio);
            return nullptr;
        }

        // Clean up
        BN_free(bne);
        BIO_free_all(publicKeyBio);
        BIO_free_all(privateKeyBio);

        return rsaKeyPair;
    }

    RSA* RSAWrapper::readPublicKeyFromString(const std::string& publicKeyStr)
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

    RSAWrapper::RSAWrapper()
    {
        m_keyPair = generateKeyPair(m_publicKeyStr);
        if (!m_keyPair)
        {
            throw std::runtime_error("Failed to generate RSA key pair.");
        }
        m_isPrivateKeyAvailable = true;
    }

    RSAWrapper::RSAWrapper(const std::string& publicKeyStr) : m_publicKeyStr(publicKeyStr)
    {
        // in this scenario private key will be missing from pair
        m_keyPair = readPublicKeyFromString(m_publicKeyStr);
        if (!m_keyPair)
        {
            throw std::runtime_error("Failed to acquire public key");
        }
    }

    RSAWrapper::~RSAWrapper() noexcept
    {
        RSA_free(m_keyPair);
    }

    std::optional<std::string> RSAWrapper::encryptMessage(const std::string& message)
    {
        int encryptedSize = RSA_size(m_keyPair);
        std::vector<unsigned char> encrypted(encryptedSize);

        int result = RSA_public_encrypt(
            static_cast<int>(message.size()),
            reinterpret_cast<const unsigned char*>(message.c_str()),
            encrypted.data(),
            m_keyPair,
            RSA_PKCS1_PADDING
        );

        if (result == -1)
        {
            std::cerr << "Failed to encrypt the message." << std::endl;
            return std::nullopt;
        }

        return std::string(encrypted.begin(), encrypted.begin() + result);
    }

    std::optional<std::string> RSAWrapper::decryptMessage(const std::string& encryptedMessage)
    {
        if (!m_isPrivateKeyAvailable)
        {
            std::cerr << "Private key not present";
            return std::nullopt;
        }

        int decryptedSize = RSA_size(m_keyPair);
        std::vector<unsigned char> decrypted(decryptedSize);

        auto result = RSA_private_decrypt(
            static_cast<int>(encryptedMessage.size()),
            reinterpret_cast<const unsigned char*>(encryptedMessage.c_str()),
            decrypted.data(),
            m_keyPair,
            RSA_PKCS1_PADDING
        );

        if (result == -1)
        {
            std::cerr << "Failed to decrypt the message." << std::endl;
            return std::nullopt;
        }

        return std::string(decrypted.begin(), decrypted.begin() + result);
    }

    std::string RSAWrapper::getPublicKeyAsString()
    {
        return m_publicKeyStr;
    }

} // namespace security

#pragma warning(default : 4496)



