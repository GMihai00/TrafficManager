#pragma once
#ifndef UTILE_DBWRAPPER_HPP
#define UTILE_DBWRAPPER_HPP

#include <memory>
#include <exception>

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
	constexpr auto SERVER = "tcp://localhost:3306/traffic_manager";
	constexpr auto USERNAME = "root"; // I KNOW I NEED TO HIDE THIS THING
	constexpr auto PASSWORD = "Calebbb1234567890*"; // I KNOW I NEED TO HIDE THIS THING
	// SERVER, USERNAME AND PASSWORD WILL BE PASSED using CMD or with a config file, for now left it like this just cause it's easier to test

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
		db::ProxyPtr findProxyById(const uint32_t id) noexcept;
		std::vector<db::ProxyPtr> findProxyChildren(const db::ProxyPtr& proxy) noexcept;
		std::vector<db::JunctionPtr> findJunctionsWithinBoundingRect(const db::BoundingRectPtr& boundingRect) noexcept;
		db::JunctionPtr findLeastLoadedProxyThatCoversLocation(const GeoCoordinate<DecimalCoordinate>& point, const LANE& direction) noexcept;
	public:
		DBWrapper(const std::string server = SERVER, const std::string username = USERNAME, const std::string password = PASSWORD);
		~DBWrapper() noexcept = default;

		db::JunctionPtr getNextJunction(const GeoCoordinate<DecimalCoordinate>& pointA, const LANE& direction) noexcept;
		bool updateProxyLoad(const db::ProxyPtr proxy, bool connecting) noexcept;
	};
} // namespace utile
#endif // #UTILE_DBWRAPPER_HPP