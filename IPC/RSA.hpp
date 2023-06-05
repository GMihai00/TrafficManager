#pragma once

#include <algorithm>
#include <memory>
#include <string>

namespace security
{
	namespace RSA
	{
		class Key
		{
		protected:
			uint32_t m_power;
			uint32_t m_modulo;
		public:
			Key() = delete;
			Key(const uint32_t power, const uint32_t modulo);

			uint32_t encrypt(const uint32_t nr);
			std::string encrypt(const std::string& text);
			virtual ~Key() noexcept = default;
		};

		class PublicKey : public Key
		{
		public:
			PublicKey() = delete;
			PublicKey(const uint32_t power, const uint32_t modulo);
			std::pair<uint32_t, uint32_t> getKeyNumericValues();
			void setKeyNumericValues(const std::pair<uint32_t, uint32_t>& values);

			virtual ~PublicKey() noexcept = default;
		};

		typedef std::shared_ptr<Key> KeyPtr;

		std::pair<KeyPtr, KeyPtr> generateKeyPair();
	}
}