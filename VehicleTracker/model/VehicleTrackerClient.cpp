#include "VehicleTrackerClient.hpp"

#include <chrono>
#include <exception>

#include "utile/ConfigHelpers.hpp"

namespace model
{
	void VehicleTrackerClient::process()
	{
		while (!shuttingDown_)
		{
			std::unique_lock<std::mutex> ulock(mutexProcess_);
			if (!shouldPause_ && !shuttingDown_)
				condVarProcess_.wait(ulock, [&] { return (shouldPause_ == false || shuttingDown_); });

			if (shuttingDown_)
			{
				continue;
			}

			while (!shouldPause_)
			{
				if (lastVisitedProxys_.empty())
				{
					throw std::runtime_error("Failed to find a valid junction, client is out of reach");
				}

				LOG_DBG << lastVisitedProxys_.top().first << lastVisitedProxys_.top().second;
				
				if (!isConnected() && !connect(lastVisitedProxys_.top().first, lastVisitedProxys_.top().second))
				{
					lastVisitedProxys_.pop();
					continue;
				}

				if (!queryProxy())
				{
					if (lastVisitedProxys_.size() != 1)
					{
						lastVisitedProxys_.pop();
						disconnect();
					}
					continue;
				}

				disconnect();
				
				if (!connect(nextJunction_->getIpAdress(), nextJunction_->getPort()))
				{
					LOG_ERR << "FAILED TO COMMUNICATE WITH JUNCTION";
					continue;
				}

				if (!notifyJunction())
				{
					LOG_ERR << "FAILED TO COMMUNICATE WITH JUNCTION";
					continue;
				}

				waitToPassJunction();

				disconnect();
			}

			if (isConnected())
				disconnect();
		}
	}

	VehicleTrackerClient::VehicleTrackerClient(const std::string& pathConfigFile, std::istream& inputStream) :
		ipc::net::Client<ipc::VehicleDetectionMessages>(),
		gpsAdapter_(inputStream),
		lastVisitedProxys_(common::utile::getLastVisitedProxys(pathConfigFile))
	{
		if (lastVisitedProxys_.empty())
			lastVisitedProxys_.push({ ipc::utile::G_PROXY_IP, ipc::utile::G_PROXY_PORT });

		threadProcess_ = std::thread(std::bind(&VehicleTrackerClient::process, this));
	}

	VehicleTrackerClient::~VehicleTrackerClient()
	{
		stop();
		if (threadProcess_.joinable())
			threadProcess_.join();
	}

	bool VehicleTrackerClient::start()
	{
		std::unique_lock<std::mutex> ulock(mutexProcess_);
		shouldPause_ = false;
		gpsAdapter_.start();
		condVarProcess_.notify_one();
		return true;
	}

	void VehicleTrackerClient::pause()
	{
		shouldPause_ = true;
		gpsAdapter_.pause();
	}

	void VehicleTrackerClient::stop()
	{
		shuttingDown_ = true;
		gpsAdapter_.stop();
		condVarProcess_.notify_one();
	}

	bool VehicleTrackerClient::queryProxy()
	{
		isRedirected_ = false;
		if (!isConnected()) { return false; }

		auto start = gpsAdapter_.getCurrentCoordinates();
		auto current = gpsAdapter_.getCurrentCoordinates();

		if (!(start.has_value() && current.has_value()))
			return false;

		auto direction = calculateDirection(start.value(), current.value());
		if (!direction.has_value()) { return false; }
		followedLane_ = direction.value();

		auto msgId = messageIdProvider_.provideId(ipc::VehicleDetectionMessages::VDB);
		ipc::net::Message<ipc::VehicleDetectionMessages> message;
		message.header.id = msgId;
		message.header.type = ipc::VehicleDetectionMessages::VDB;
		message << start.value().latitude << start.value().longitude << current.value().latitude << current.value().longitude;
		send(message);

		// WAIT FOR RESPONSE
		if (!waitForAnswear(3000))
		{
			return false;
		}

		// GET MESSAGE AND CHECK TYPE
		auto answear = getLastUnreadAnswear();
		if (!answear.has_value()) { return false; }

		// IF THIS FAILS IT MEANS THAT YOU ARE UNABLE TO BE REDIRECTED TO A JUNCTION FOR SOME KIND OF REASON
		// JUST HALT THE EXECUTION OF THE PROGRAM AND RETURN ERR EXIT CODE, NO REASON TO MOVE FORWARD
		if (!handleProxyAnswear(answear.value().first.msg)) 
		{ 
			return false;
		}

		if (isRedirected_)
		{
			return queryProxy();
		}
		return true;
	}

	bool VehicleTrackerClient::handleProxyAnswear(ipc::net::Message<ipc::VehicleDetectionMessages>& msg)
	{
		ipc::net::Message<ipc::VehicleDetectionMessages> msgcpy = msg;

		try
		{
			ipc::net::ProxyReply reply{ msgcpy };
			return setupData(reply);
		}
		catch (const std::runtime_error&)
		{
		}

		try
		{
			ipc::net::ProxyRedirect redirect{ msg };
			LOG_DBG << "Redirected";
			return switchConnectionToRedirectedProxy(redirect);
		}
		catch (const std::runtime_error&)
		{
			// IF WE REACHED THIS POINT WE JUST GOT A NACK MESSAGE
			// FOR NOW NO NEED TO REPARSE IT TO SEE IF IT IS NACK OR NOT
			LOG_DBG << "Message rejected";
			return false;
		}
	}

	bool VehicleTrackerClient::setupData(ipc::net::ProxyReply& reply)
	{
		if (!reply.isApproved()) { return false; }

		auto junctionIpAndPort = reply.getServerIPAdressAndPort();
		nextJunction_ = std::make_shared<common::db::Junction>(junctionIpAndPort.first, junctionIpAndPort.second, reply.getServerCoordinates());
		isEmergency_ = reply.isEmergency();

		return true;
	}

	bool VehicleTrackerClient::switchConnectionToRedirectedProxy(ipc::net::ProxyRedirect& redirect)
	{
		isRedirected_ = true;
		disconnect();
		auto proxyIpAndPort = redirect.getServerIPAdressAndPort();
		lastVisitedProxys_.push({ proxyIpAndPort.first, proxyIpAndPort.second });
		return connect(proxyIpAndPort.first, proxyIpAndPort.second);
	}

	bool VehicleTrackerClient::notifyJunction()
	{
		if (!isConnected())
		{
			return false;
		}

		auto msgId = messageIdProvider_.provideId(ipc::VehicleDetectionMessages::VDB);
		ipc::net::Message<ipc::VehicleDetectionMessages> message;
		message.header.id = msgId;
		message.header.type = ipc::VehicleDetectionMessages::VDB;
		message << followedLane_;
		send(message);

		if (!waitForAnswear(1000))
		{
			LOG_ERR << "Timeout recieving answear from the junction";
			return false;
		}

		auto answear = getLastUnreadAnswear();
		if (!answear.has_value()) { return false; }

		return answear->first.msg.header.type == ipc::VehicleDetectionMessages::ACK;
	}
	
	// AICI PASSED SI IS PASSING SOMEHOW IS FAILING ??? to check if still valid
	void VehicleTrackerClient::waitToPassJunction()
	{
		auto pointA = gpsAdapter_.getCurrentCoordinates();
		auto pointB = gpsAdapter_.getCurrentCoordinates();
		while (pointA.has_value() && pointB.has_value() 
			&& !nextJunction_->passedJunction(pointA.value(), pointB.value()) 
			&& nextJunction_->isPassing(pointA.value(), pointB.value()))
		{
			pointA = pointB;
			pointB = gpsAdapter_.getCurrentCoordinates();
		}
	}

	bool VehicleTrackerClient::saveDataToJson()
	{
		return common::utile::saveVTConfig(lastVisitedProxys_);
	}
}