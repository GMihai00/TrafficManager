#include "RSA.hpp"

#include <vector>
#include <random>
#include <sstream>

namespace security
{
    namespace RSA
    {
        namespace details
        {
            constexpr uint32_t MAXSIZE = (uint32_t)1e6;

            std::vector<uint32_t> generate_prime_numbers()
            {
                std::vector<bool> ciur(MAXSIZE + 1);
                std::fill(ciur.begin(), ciur.end(), false);

                std::vector<uint32_t> rez;

                for (uint32_t i = 1; ((i * i) << 1) + (i << 1) <= MAXSIZE; i += 1) {
                    if (ciur[i] == false) {
                        for (uint32_t j = ((i * i) << 1) + (i << 1); (j << 1) + 1 <= MAXSIZE; j += (i << 1) + 1) {
                            ciur[j] = true;
                        }
                    }
                }
                for (uint32_t i = 1; 2 * i + 1 <= MAXSIZE; ++i)
                    if (ciur[i] == false) rez.push_back(2 * i + 1);

                return rez;
            }

            static std::vector<uint32_t> prime_numbers = generate_prime_numbers();

            uint32_t lgput(uint32_t nr, uint32_t power, uint32_t mod)
            {
                uint32_t rez = 1;
                while (power > 0)
                {
                    if (power & 1)
                    {
                        rez *= nr;
                        rez %= mod;
                        power--;
                    }
                    nr *= nr;
                    nr %= mod;
                    power >>= 1;
                }
                return rez;
            }

            uint32_t inversModularPrimeNumber(uint32_t nr, uint32_t mod)
            {
                return lgput(nr, nr - 2, mod);
            }

            size_t getRandomNumber(size_t min, size_t max) {
                std::random_device rd;  
                std::mt19937 gen(rd()); 

                std::uniform_int_distribution<size_t> distribution(min, max);

                return distribution(gen);
            }
 
        }

        Key::Key(const uint32_t power, const uint32_t modulo) : m_power(power), m_modulo(modulo) {}

        uint32_t Key::encrypt(const uint32_t nr)
        {
            return details::lgput(nr, m_modulo, m_power);
        }

        std::string Key::encrypt(const std::string& text)
        {
            std::stringstream ss;

            for (const auto& chr : text)
            {
                ss << encrypt(static_cast<uint32_t>(chr));
            }

            return ss.str();
        }

        PublicKey::PublicKey(const uint32_t power = 1, const uint32_t modulo = 1) : Key(power, modulo) {}

        std::pair<uint32_t, uint32_t> PublicKey::getKeyNumericValues()
        {
            return { m_power, m_modulo };
        }

        void PublicKey::setKeyNumericValues(const std::pair<uint32_t, uint32_t>& values)
        {
            m_power = values.first;
            m_modulo = values.second;
        }


        std::pair<KeyPtr, KeyPtr> generateKeyPair()
        {
            uint32_t primenr1 = details::prime_numbers[details::getRandomNumber(0, (details::prime_numbers.size() - 1) / 2)];
            uint32_t primenr2 = details::prime_numbers[details::getRandomNumber(0, (details::prime_numbers.size() - 1) / 2)];
            uint32_t mod = primenr1 * primenr2;
            uint32_t euler_product = (primenr1 - 1) * (primenr2 - 1);

            auto low = std::lower_bound(details::prime_numbers.begin(), details::prime_numbers.end(), euler_product - 2);
            if (low == details::prime_numbers.end())
                low = details::prime_numbers.end() - 1;

            auto e = details::prime_numbers[details::getRandomNumber(2, low - details::prime_numbers.begin())];
            auto d = details::inversModularPrimeNumber(e, mod);

            auto publicKey = std::make_shared<PublicKey>( e, mod );
            auto privateKey = std::make_shared<Key>( d, mod );

            return { publicKey, privateKey };
        }
    }
}