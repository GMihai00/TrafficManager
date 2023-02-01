#pragma once
#pragma once
#ifndef MODEL_CVISION_MOVINGOBJECTGROUP_HPP
#define MODEL_CVISION_MOVINGOBJECTGROUP_HPP

#include <iostream>
#include <list> 
#include <vector>
#include <memory>
#include <mutex>
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
		cv::Point sumCenterPoz_;
		bool objFoundInFrame_;
		size_t nrFramesWithoutMatch_;
		cv::Point futurePosition_;
		std::mutex mutexGroup_;
		size_t nrFramesWithoutBeeingCar_;
		size_t nrCars_;
		std::atomic<size_t> cntLock = 0;

		void predictNextPosition();
	public:
		MovingObjectGroup();
		MovingObjectGroup(const MovingObjectGroup& cpy);
		~MovingObjectGroup() = default;

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
		void updateCarState(size_t nrCars);
		size_t nrCars();
	};
	typedef std::shared_ptr<MovingObjectGroup> MovingObjectGroupPtr;
} // namespace cvision

#endif // #MODEL_CVISION_MOVINGOBJECTGROUP_HPP