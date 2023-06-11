#pragma once

#include <iostream>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <memory>
#include <optional>

namespace security
{
	class RSAWrapper
	{
	private:
		std::string m_publicKeyStr;
		RSA* m_keyPair;
		bool m_isPrivateKeyAvailable = false;

		RSA* generateKeyPair(std::string& publicKey);
		RSA* readPublicKeyFromString(const std::string& publicKeyStr);
	public:
		RSAWrapper();
		RSAWrapper(const std::string& publicKeyStr);
		~RSAWrapper() noexcept;

		std::optional<std::string> encryptMessage(const std::string& message);
		std::optional<std::string> decryptMessage(const std::string& encryptedMessage);
		std::string getPublicKeyAsString();
	};
	typedef std::shared_ptr<RSAWrapper> RSAWrapperPtr;
}