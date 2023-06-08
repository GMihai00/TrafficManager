#pragma once
#ifndef GUI_GLFWWINDOWMANAGER_HPP
#define GUI_GLFWWINDOWMANAGER_HPP

#include <queue>
#include <thread>
#include <mutex>
#include <functional>
#include <set>
#include <map>
#include <optional>

#include <GLFW/glfw3.h>

#include "utile/DataTypes.hpp"
#include "utile/Timer.hpp"

#include "VehicleTypes.hpp"
#include "GraphicsDataTypes.hpp"
#include "TextRenderer.hpp"
#include "DataCollector.hpp"

namespace model
{
	class GLFWWindowManager
	{
	private:
		std::thread m_rendering_thread;
		std::queue<std::function<void()>> m_waiting_queue;

		std::map<common::utile::LANE, bool> m_lane_alowence_map;

		std::mutex m_mutex;

		GLFWwindow* m_window = nullptr;

		TextRendererPtr m_textRendere;
		const GLfloat junction_edge_ = 0.5f;

		const std::map<common::utile::LANE, common::utile::TimerPtr>& laneToTimerMap_;

		std::map<paint::VehicleTypes, std::map<common::utile::LANE, uint16_t>> m_vehicleTypeToCarsWaiting;

		uint32_t m_id = 0;
		std::shared_ptr<common::data::DataCollector> m_dataCollector{ nullptr };

		void draw_junction();
		void display_time_left_for_lane(const common::utile::LANE lane, const uint16_t time_left);
		void display_time_left_timers();
		void display_cars_waiting_for_lane(const uint16_t nr_cars, const common::utile::LANE lane, const paint::VehicleTypes type);
		void display_cars_waiting();

		void render(int window_weight, int window_height);

		void drawCars(const paint::VehicleTypes type,
			const common::utile::LANE lane,
			std::optional<Point> bl_point,
			const GLfloat height,
			const GLfloat width,
			GLfloat moving_rate,
			const size_t nr_cars,
			const uint32_t id);
		bool isAboutToCrossTheJunction(const Point& bl_point, const GLfloat& height, const GLfloat& width, const common::utile::LANE lane);

	public:
		GLFWWindowManager(const std::map<common::utile::LANE, common::utile::TimerPtr>& laneToTimerMap,
			int window_weight = 1080, int window_height = 1080, 
			const std::filesystem::path& texturePath = std::filesystem::path("..\\resources\\Textures\\Arialn.ttf"));
		virtual ~GLFWWindowManager() noexcept;

		void signalIncomingCar(const paint::VehicleTypes type,
			const common::utile::LANE lane,
			const size_t nr_cars = 1,
			std::optional<Point> bl_point = std::nullopt,
			const GLfloat& height = 0.1,
			const GLfloat& width = 0.2,
			GLfloat moving_rate = 0.1);
		void changeTrafficLights(const std::set<common::utile::LANE>& green_light_lanes);
	};
} // namespace model
#endif // !GUI_GLFWWINDOWMANAGER_HPP
