#pragma once
#ifndef UTILE_DBWRAPPER_HPP
#define UTILE_DBWRAPPER_HPP

#include <memory>

#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>

// TO ADD METHODS BUT FIRST NEED TO DEFINE DB
namespace utile
{
	class DBWrapper
	{
	private:
		std::unique_ptr<sql::Driver> driver_;
		std::unique_ptr<sql::Connection> connection_;
	public:
		DBWrapper() = delete;
		DBWrapper(const std::string server, const std::string username, const std::string password);
		~DBWrapper() noexcept = default;
	};
} // namespace utile
#endif // #UTILE_DBWRAPPER_HPP