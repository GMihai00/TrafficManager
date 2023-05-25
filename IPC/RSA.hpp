#pragma once

#include <algorithm>

namespace security
{
	namespace RSA
	{
		class Key
		{
			uint32_t m_power;
			uint32_t m_modulo;
		public:
			Key() = delete;
			Key(uint32_t power, uint32_t modulo);

			uint32_t encrypt(const uint32_t nr, const Key& key);
		};

		std::pair<Key, Key> generateKeyPair();
	}
}