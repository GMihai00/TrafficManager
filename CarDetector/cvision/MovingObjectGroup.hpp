#pragma once
#pragma once
#ifndef MODEL_CVISION_MOVINGOBJECTGROUP_HPP
#define MODEL_CVISION_MOVINGOBJECTGROUP_HPP

#include <iostream>
#include <list> 
#include <vector>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <boost/optional.hpp>
#include "MovingObject.hpp"

namespace cvision
{
	constexpr size_t MAX_FRAMES_WITHOUT_A_MATCH = 5;
	constexpr size_t MAX_OBJECTS_STORED = 5;
	class MovingObjectGroup {
	private:
		MovingObjPtrList movingObjectStates_;
		std::vector<cv::Point> centerPositions_;
		cv::Point sumCenterPoz_ = cv::Point{ 0 };
		bool objFoundInFrame_ = false;
		size_t nrFramesWithoutMatch_ = 0;
		cv::Point futurePosition_;
		std::shared_mutex mutexGroup_;
		size_t nrFramesWithoutBeeingCar_ = 0;
		uint8_t nrCars_ = 0;

		void predictNextPosition();
	public:
		void addMovingObject(const MovingObjectPtr& obj);
		void updateState(bool found);
		bool stillBeingTracked();

		boost::optional<double> getDiagonalSize();
		boost::optional<cv::Point> getCenterPosition(const size_t& index);
		boost::optional<cv::Point> getLastCenterPosition();
		size_t getNrOfMovingObjInGroup();
		cv::Point getFuturePosition();
		MovingObjectPtr getFirstState();
		MovingObjectPtr getLastState();
		cv::Mat getCroppedImage(cv::Mat img);
		void updateCarState(uint8_t nrCars);
		uint8_t nrCars();
	};
	typedef std::shared_ptr<MovingObjectGroup> MovingObjectGroupPtr;
	typedef std::list<MovingObjectGroupPtr> MovingObjGroupPtrList;
} // namespace cvision

#endif // #MODEL_CVISION_MOVINGOBJECTGROUP_HPP