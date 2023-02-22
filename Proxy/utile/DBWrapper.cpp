#include "DBWrapper.hpp"
#include <assert.h>

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
		uint32_t id = queryResult->getInt("id");
		GeoCoordinate<DecimalCoordinate> boundSW{};
		boundSW.latitude = queryResult->getDouble("sw_latitude");
		boundSW.longitude = queryResult->getDouble("sw_longitude");
		GeoCoordinate<DecimalCoordinate> boundNE{};
		boundNE.latitude = queryResult->getDouble("ne_latitude");
		boundNE.longitude = queryResult->getDouble("ne_longitude");

		return std::make_shared<db::BoundingRect>(id, boundSW, boundNE);
	}

	db::ProxyPtr DBWrapper::buildProxyFromQueryResult(const std::shared_ptr<sql::ResultSet>& queryResult) noexcept
	{
		std::string ipAdress = queryResult->getString("ip_adress");
		uint16_t port = queryResult->getInt("port");
		uint32_t load = queryResult->getInt("load");
		uint32_t boudingRectId = queryResult->getInt("bounding_rect_id");
		db::BoundingRectPtr coveredArea = findBoundingRectById(boudingRectId);

		auto proxy = std::make_shared<db::Proxy>(ipAdress, port, load, coveredArea);
		return proxy;
	}

	db::JunctionPtr DBWrapper::buildJunctionFromQueryResult(const std::shared_ptr<sql::ResultSet>& queryResult) noexcept
	{
		std::string ipAdress = queryResult->getString("ip_adress");
		uint16_t port = queryResult->getInt("port");
		uint32_t boudingRectId = queryResult->getInt("bounding_rect_id");
		db::BoundingRectPtr coveredArea = findBoundingRectById(boudingRectId);

		return std::make_shared<db::Junction>(ipAdress, port, coveredArea);
	}

	db::BoundingRectPtr DBWrapper::findBoundingRectById(const uint32_t id) noexcept
	{
		try
		{
			std::shared_ptr<sql::PreparedStatement> preparedStatement(connection_->prepareStatement("SELECT * FROM bounding_rect WHERE id = ?;"));
			preparedStatement->setInt(1, id);
			std::shared_ptr<sql::ResultSet> queryResult(preparedStatement->executeQuery());

			if (queryResult->next())
			{
				return buildBoundingRectFromQueryResult(queryResult);
			}
		}
		catch (const sql::SQLException& err)
		{
			LOG_WARN << "Failed to find BoundingRect with id: " << id << " err: " << err.what();
			return nullptr;
		}
	}

	std::vector<db::ProxyPtr> DBWrapper::getAllProxys() noexcept
	{
		try
		{
			std::vector<db::ProxyPtr> proxys;

			std::shared_ptr<sql::PreparedStatement> preparedStatement(connection_->prepareStatement("SELECT * FROM proxy;"));
			std::shared_ptr<sql::ResultSet> queryResult(preparedStatement->executeQuery());

			while (queryResult->next())
			{
				proxys.push_back(std::move(buildProxyFromQueryResult(queryResult)));
			}
		}
		catch (const sql::SQLException& err)
		{
			LOG_WARN << "Failed to find Proxys err: " << err.what();
			return {};
		}
	}

	std::vector<db::JunctionPtr> DBWrapper::getAllJunctions() noexcept
	{
		try
		{
			std::vector<db::JunctionPtr> junctions;

			std::shared_ptr<sql::PreparedStatement> preparedStatement(connection_->prepareStatement("SELECT * FROM junction;"));
			std::shared_ptr<sql::ResultSet> queryResult(preparedStatement->executeQuery());

			while (queryResult->next())
			{
				junctions.push_back(std::move(buildJunctionFromQueryResult(queryResult)));
			}
		}
		catch (const sql::SQLException& err)
		{
			LOG_WARN << "Failed to find Junctions err: " << err.what();
			return {};
		}
	}

	db::ProxyPtr DBWrapper::findLeastLoadedProxyThatCoversLocation(const GeoCoordinate<DecimalCoordinate>& point) noexcept
	{
		try
		{
			std::shared_ptr<sql::PreparedStatement> preparedStatement(connection_->prepareStatement(
				"SELECT * FROM proxy WHERE bouding_rect_id IN (SELECT id from bounding_rect WHERE ? <= ne_latitude AND  ? <= ne_longitude AND sw_latitude <= ? AND sw_longitude <= ? ) ORDER BY load;"));
			preparedStatement->setInt(1, point.latitude);
			preparedStatement->setInt(2, point.longitude);
			preparedStatement->setInt(3, point.latitude);
			preparedStatement->setInt(4, point.longitude);

			std::shared_ptr<sql::ResultSet> queryResult(preparedStatement->executeQuery());
			if (queryResult->next())
			{
				return buildProxyFromQueryResult(queryResult);
			}

			return nullptr;
		}
		catch (const sql::SQLException& err)
		{
			LOG_WARN << "Failed to find Proxy that covers location err: " << err.what();
			return nullptr;
		}
	}

	db::ProxyPtr DBWrapper::findClosestProxyToPoint(const GeoCoordinate<DecimalCoordinate>& point) noexcept
	{
		try
		{
			
			return nullptr;
		}
		catch (const sql::SQLException& err)
		{
			LOG_WARN << "Failed to find closest proxy err: " << err.what();
			return nullptr;
		}
	}

	bool DBWrapper::updateProxyLoad(const db::ProxyPtr proxy, bool connecting) noexcept
	{
		assert(proxy);
		try
		{
			std::shared_ptr<sql::PreparedStatement> preparedStatement(connection_->prepareStatement("UPDATE proxy SET load = ? WHERE ip_adress = ? AMD port = ?;"));
			preparedStatement->setInt(1, proxy->updateLoad(connecting));
			preparedStatement->setString(2, proxy->getIpAdress());
			preparedStatement->setInt(3, proxy->getPort());
			preparedStatement->executeQuery();
			return true;
		}
		catch (const sql::SQLException& err)
		{
			LOG_WARN << "Failed to update Junction load err: " << err.what();
			proxy->updateLoad(!connecting);
			return false;
		}
	}

} // namespace utile
