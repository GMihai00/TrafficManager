#pragma once
#ifndef UTILE_DBWRAPPER_HPP
#define UTILE_DBWRAPPER_HPP

#include <memory>
#include <exception>
#include <string>

#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>

#include "utile/GeoCoordinate.hpp"
#include "utile/Logger.hpp"
#include "db/BoundingRect.hpp"
#include "db/Junction.hpp"
#include "db/Proxy.hpp"

namespace utile
{
	struct DBConnectionData
	{
		std::string m_server;
		std::string m_username;
		std::string m_password;
		DBConnectionData() = delete;
		DBConnectionData(const std::string& server, const std::string& username, const std::string& password) :
			m_server(server),
			m_username(username),
			m_password(password)
		{}
	};

	using namespace common;
	using namespace common::utile;

	class DBWrapper
	{
	private:
		std::unique_ptr<sql::Connection> connection_;

		LOGGER("DBWrapper");

		db::ProxyPtr buildProxyFromQueryResult(const std::shared_ptr<sql::ResultSet>& queryResult) noexcept;
		db::BoundingRectPtr buildBoundingRectFromQueryResult(const std::shared_ptr<sql::ResultSet>& queryResult) noexcept;
		db::JunctionPtr buildJunctionFromQueryResult(const std::shared_ptr<sql::ResultSet>& queryResult) noexcept;

		db::BoundingRectPtr findBoundingRectById(const uint32_t id) noexcept;
	public:
		DBWrapper(DBConnectionData connectionData);
		~DBWrapper() noexcept = default;

		std::vector<db::ProxyPtr> getAllProxys() noexcept;
		std::vector<db::JunctionPtr> getAllJunctions() noexcept;
		db::ProxyPtr findLeastLoadedProxyThatCoversLocation(const GeoCoordinate<DecimalCoordinate>& point) noexcept;
		db::ProxyPtr findClosestProxyToPoint(const GeoCoordinate<DecimalCoordinate>& point) noexcept;
		bool updateProxyLoad(const db::ProxyPtr proxy, bool connecting) noexcept;
	};
} // namespace utile
#endif // #UTILE_DBWRAPPER_HPP