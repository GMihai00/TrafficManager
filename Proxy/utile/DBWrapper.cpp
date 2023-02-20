#include "DBWrapper.hpp"

// TO FINISH THIS
namespace utile
{
	DBWrapper::DBWrapper(const std::string server, const std::string username, const std::string password) :
		connection_(get_driver_instance()->connect(server, username, password))
	{
		LOG_INF << "Connected";
		connection_->setSchema("traffic_manager");
	}

	db::BoundingRectPtr DBWrapper::buildBoundingRectFromQueryResult(const std::shared_ptr<sql::ResultSet>& queryResult) noexcept
	{
		auto rez = std::make_shared<db::BoundingRect>();
		return rez;
	}

	db::ProxyPtr DBWrapper::buildProxyFromQueryResult(const std::shared_ptr<sql::ResultSet>& queryResult) noexcept
	{
		auto rez = std::make_shared<db::Proxy>();
		return rez;
	}

	db::JunctionPtr DBWrapper::buildJunctionFromQueryResult(const std::shared_ptr<sql::ResultSet>& queryResult) noexcept
	{
		auto rez = std::make_shared<db::Junction>();
		return rez;
	}

	db::BoundingRectPtr DBWrapper::findBoundingRectById(const uint32_t id) noexcept
	{
		try
		{

		}
		catch (const sql::SQLException& err)
		{
			LOG_WARN << "Failed to find BoundingRect with id: " << id << " err: " << err.what();
			return nullptr;
		}
	}

	db::ProxyPtr DBWrapper::findProxyById(const uint32_t id) noexcept
	{
		try
		{
			std::shared_ptr<sql::PreparedStatement> preparedStatement(connection_->prepareStatement("SELECT * FROM bounding_rect WHERE id = ?;"));
			preparedStatement->setInt(1, id);
			std::shared_ptr<sql::ResultSet> queryResult(preparedStatement->executeQuery());

			if (queryResult->next())
			{
				return buildProxyFromQueryResult(queryResult);
			}

			return nullptr;
		}
		catch (const sql::SQLException& err)
		{
			LOG_WARN << "Failed to find Proxy with id: " << id << " err: " << err.what();
			return nullptr;
		}
	}

	std::vector<db::ProxyPtr> DBWrapper::findProxyChildren(const db::ProxyPtr& proxy) noexcept
	{
		try
		{
			std::vector<db::ProxyPtr> children;

			return children;
		}
		catch (const sql::SQLException& err)
		{
			LOG_WARN << "Failed to find Proxy children err: " << err.what();
			return {};
		}
	}

	std::vector<db::JunctionPtr> DBWrapper::findJunctionsWithinBoundingRect(const db::BoundingRectPtr& boundingRect) noexcept
	{
		try
		{
			std::vector<db::JunctionPtr> junctions;

			return junctions;
		}
		catch (const sql::SQLException& err)
		{
			LOG_WARN << "Failed to find Junctions within BoundingRect err: " << err.what();
			return {};
		}
	}

	db::JunctionPtr DBWrapper::findLeastLoadedProxyThatCoversLocation(const GeoCoordinate<DecimalCoordinate>& point, const LANE& direction) noexcept
	{
		try
		{

		}
		catch (const sql::SQLException& err)
		{
			LOG_WARN << "Failed to find Proxy that covers location err: " << err.what();
			return nullptr;
		}
	}

	db::JunctionPtr DBWrapper::getNextJunction(const GeoCoordinate<DecimalCoordinate>& pointA, const LANE& direction) noexcept
	{
		try
		{

		}
		catch (const sql::SQLException& err)
		{
			LOG_WARN << "Failed to get next Junction err: " << err.what();
			return nullptr;
		}
	}

	bool DBWrapper::updateProxyLoad(const db::ProxyPtr proxy, bool connecting) noexcept
	{
		try
		{
			return true;
		}
		catch (const sql::SQLException& err)
		{
			LOG_WARN << "Failed to update Junction load err: " << err.what();
			return false;
		}
	}

} // namespace utile
