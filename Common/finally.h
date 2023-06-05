#pragma once

#include <functional>

namespace common
{
	namespace utile
	{
		struct finally
		{
		private:
			std::function<void()> m_finallAction;
		public:
			finally() = delete;
			finally(const finally& obj) = delete;
			finally(const finally&& obj) = delete;
			finally(const std::function<void()>& finallAction);
			virtual ~finally() noexcept;
		};

	} // namespace utile
} // namespace common