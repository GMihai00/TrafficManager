#include "finally.h"

namespace common
{
	namespace utile
	{

		finally::finally(const std::function<void()>&finallAction) : 
			m_finallAction(finallAction) {};

		finally::~finally() noexcept
		{
			if (m_finallAction)
				m_finallAction();
		}
		
	} // namespace utile
} // namespace common