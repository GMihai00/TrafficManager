#include "DBWrapper.hpp"

namespace utile
{
	DBWrapper::DBWrapper(const std::string server, const std::string username, const std::string password) :
		driver_(get_driver_instance()),
		connection_(driver_->connect(server, username, password))
	{
	}

} // namespace utile
