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

#include "VehicleTypes.hpp"
#include "GraphicsDataTypes.hpp"

namespace model
{
	class GLFWWindowManager
	{
	private:
		std::thread m_rendering_thread;
		std::queue<std::function<void()>> m_waing_queue;

		std::map<common::utile::LANE, bool> m_lane_alowence_map;

		GLFWwindow* m_window = nullptr;

		void draw_junction();

		void render(int window_weight, int window_height);

		void drawCar(const RGBColor& color,
			const common::utile::LANE lane,
			std::optional<Point> bl_point,
			const GLfloat& height,
			const GLfloat& width,
			GLfloat moving_rate);

	public:
		GLFWWindowManager(int window_weight = 1080, int window_height = 1080);
		virtual ~GLFWWindowManager() noexcept;

		void signalIncomingCar(const paint::VehicleTypes type,
			const common::utile::LANE lane,
			std::optional<Point> bl_point = std::nullopt,
			const GLfloat& height = 0.1,
			const GLfloat& width = 0.2,
			GLfloat moving_rate = 0.1);
		void changeTrafficLights(const std::set<common::utile::LANE>& green_light_lanes);
	};
} // namespace model
#endif // !GUI_GLFWWINDOWMANAGER_HPP
