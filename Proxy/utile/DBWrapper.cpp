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
		uint32_t id = queryResult->getInt("id");
		uint32_t load = queryResult->getInt("load");
		uint32_t boudingRectId = queryResult->getInt("bounding_rect_id");
		db::BoundingRectPtr coveredArea = findBoundingRectById(boudingRectId);

		auto proxy = std::make_shared<db::Proxy>(id, load, coveredArea);
		proxy->setMonitoredJunctions(findJunctionsCoveredByProxy(proxy));
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

	db::ProxyPtr DBWrapper::findProxyById(const uint32_t id) noexcept
	{
		try
		{
			std::shared_ptr<sql::PreparedStatement> preparedStatement(connection_->prepareStatement("SELECT * FROM proxy WHERE id = ?;"));
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

	std::vector<db::JunctionPtr> DBWrapper::findJunctionsCoveredByProxy(const db::ProxyPtr& proxy) noexcept
	{
		assert(proxy);
		auto boudingRect = proxy->getCoveredArea();
		assert(boudingRect);

		try
		{
			std::vector<db::JunctionPtr> junctions;
			auto bounds = boudingRect->getBounds();
			auto sw_longitude = bounds.first.longitude;
			auto sw_latitude = bounds.first.latitude;
			auto ne_longitude = bounds.second.longitude;
			auto ne_latitude = bounds.second.latitude;

			std::shared_ptr<sql::PreparedStatement> preparedStatement(connection_->prepareStatement(
				"SELECT * FROM junction WHERE bouding_rect_id NOT IN (SELECT id from bounding_rect WHERE ? > ne_latitude OR ? > ne_longitude OR sw_latitude > ? OR sw_longitude > ? );"));
			preparedStatement->setInt(1, sw_latitude);
			preparedStatement->setInt(2, sw_longitude);
			preparedStatement->setInt(3, ne_latitude);
			preparedStatement->setInt(4, ne_longitude);

			std::shared_ptr<sql::ResultSet> queryResult(preparedStatement->executeQuery());
			while (queryResult->next())
			{
				junctions.push_back(std::move(buildJunctionFromQueryResult(queryResult)));
			}
			return junctions;
		}
		catch (const sql::SQLException& err)
		{
			LOG_WARN << "Failed to find Junctions within BoundingRect err: " << err.what();
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

	// FOR NOW USING JUST DB TO TAKE NEXT JUNCTION, IT SHOULD BE TAKEN WITH THE USE OF THE PROXY, NOT LIKE THIS
	db::JunctionPtr DBWrapper::getNextJunction(const GeoCoordinate<DecimalCoordinate>& pointA, const LANE& direction) noexcept
	{
		try
		{
			auto proxy = findLeastLoadedProxyThatCoversLocation(pointA);
			for (const auto& junction : proxy->getMonitoredJunctions())
			{
				if (junction->isPassing(pointA, direction))
				{
					return junction;
				}
				
			}

			return nullptr;
		}
		catch (const sql::SQLException& err)
		{
			LOG_WARN << "Failed to get next Junction err: " << err.what();
			return nullptr;
		}
	}

	bool DBWrapper::updateProxyLoad(const db::ProxyPtr proxy, bool connecting) noexcept
	{
		assert(proxy);
		try
		{
			std::shared_ptr<sql::PreparedStatement> preparedStatement(connection_->prepareStatement("UPDATE proxy SET load = ? WHERE id = ?;"));
			preparedStatement->setInt(1, proxy->updateLoad(connecting));
			preparedStatement->setInt(2, proxy->getId());
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
