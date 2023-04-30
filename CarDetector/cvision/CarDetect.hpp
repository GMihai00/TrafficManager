#pragma once
#ifndef CARDETECT_CVISION_CARDETECT_HPP
#define CARDETECT_CVISION_CARDETECT_HPP

#include <future>
#include <queue>
#include <map>
#include <filesystem>
#include "Utils.hpp"
#include "opencv2/objdetect.hpp"
#include "MovingObjectGroup.hpp"
#include "utile/ThreadSafeQueue.hpp"
#include <opencv2/dnn.hpp>

#include "utile/IPCDataTypes.hpp"
#include "../tensorflow/TensorflowClient.hpp"

namespace cvision
{
	class CarDetect
	{
	private:
		const ipc::utile::IP_ADRESS TENSORFLOW_SERVER_HOST = "127.0.0.1";
		const ipc::utile::PORT TENSORFLOW_SERVER_PORT = 8000;

		common::utile::ThreadSafeQueue<std::pair<uint32_t, cv::Mat>> taskQueue_;
		std::map<uint32_t, std::future<uint8_t>> carCountTaskMap_;
		tensorflow::TensorflowClient tensorflowClient_;

		std::thread threadDetect_;
		std::mutex mutexDetect_;
		std::condition_variable condVarDetect_;
		std::atomic<bool> shuttingDown_ = false;
		std::atomic<bool> isRunning_ = false;
		std::atomic<bool> shoudlClearAllTasks_ = false;
		std::future<uint8_t> getCarsPresentInImage(const cv::Mat& image);
	public:
		CarDetect();
		~CarDetect();

		void loadTask(const uint32_t id ,const const cv::Mat& image);
		std::map<uint32_t, uint8_t> waitForFinish();
		bool startDetecting();
		bool isRunning() const;
		void stopDetecting();
	};
	} // namespace cvision

#endif // #CARDETECT_CVISION_CARDETECT_HPP