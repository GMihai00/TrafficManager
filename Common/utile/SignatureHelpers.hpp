#pragma once
#ifndef COMMON_UTILE_SIGNATUREHELPERS_HPP
#define COMMON_UTILE_SIGNATUREHELPERS_HPP

#include <string>
// #include <WinTrust.h> //MAYBE LOOK INTO THIS IT COULD WORK

namespace common
{
	namespace utile
	{
		std::wstring ReadSignature();

	} // namespace utile
} // namespace common
#endif // #COMMON_UTILE_SIGNATUREHELPERS_HPP