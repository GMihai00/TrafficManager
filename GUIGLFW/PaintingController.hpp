#pragma once
#ifndef GUI_PAINTINCONTROLLER_HPP
#define GUI_PAINTINCONTROLLER_HPP

#include <map>
#include <set>
#include <memory>

#include "utile/DataTypes.hpp"

#include "GLFWWindowManager.hpp"

#include "VehicleTypes.hpp"

namespace model
{
	class PaintingController
	{
	private:
		std::shared_ptr<model::GLFWWindowManager> m_window_manager;
	
		void drawCar(const paint::VehicleTypes type, const common::utile::LANE& lane, const bool passing_junction);
		void changeTrafficLights(const std::set<common::utile::LANE>& green_light_lanes);
	};
} // namespace model
#endif // !GUI_PAINTINCONTROLLER_HPP