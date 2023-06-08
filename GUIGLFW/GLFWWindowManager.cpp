#include "GLFWWindowManager.hpp"

#include <functional>

#include "DrawHelpers.hpp"


namespace model
{
	static void inputK(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GL_TRUE);
	}

	void GLFWWindowManager::draw_junction()
	{
		//UP LEFT
		draw_dotted_line(Point(-1.0f, junction_edge_ / 2), Point(-junction_edge_ / 2, junction_edge_ / 2));
		draw_dotted_line(Point(-junction_edge_ / 2, junction_edge_ / 2), Point(-junction_edge_ / 2, 1.0f));

		//UP RIGHT
		draw_dotted_line(Point(1.0f, junction_edge_ / 2), Point(junction_edge_ / 2, junction_edge_ / 2));
		draw_dotted_line(Point(junction_edge_ / 2, junction_edge_ / 2), Point(junction_edge_ / 2, 1.0f));
		
		//DOWN RIGHT
		draw_dotted_line(Point(1.0f, -junction_edge_ / 2), Point(junction_edge_ / 2, -junction_edge_ / 2));
		draw_dotted_line(Point(junction_edge_ / 2, -junction_edge_ / 2), Point(junction_edge_ / 2, -1.0f));

		//DOWN LEFT
		draw_dotted_line(Point(-1.0f, -junction_edge_ / 2), Point(-junction_edge_ / 2, -junction_edge_ / 2));
		draw_dotted_line(Point(-junction_edge_ / 2, -junction_edge_ / 2), Point(-junction_edge_ / 2, -1.0f));

		draw_traffic_lane_line(Point(-1.0f, 0.0f), Point(1.0f, 0.0f), 0.035f, COLOR_WHITE);
		draw_traffic_lane_line(Point(0.0f, -1.0f), Point(0.0f, 1.0f), 0.035f, COLOR_WHITE);

		// traffic lights
		const static std::vector<common::utile::LANE> all_lanes = 
		{
			common::utile::LANE::E,
			common::utile::LANE::W,
			common::utile::LANE::S,
			common::utile::LANE::N
		};

		const static std::map<common::utile::LANE, Point> semaphor_location_based_on_lane =
		{
			{common::utile::LANE::E, Point(0.4f, 0.4f)},
			{common::utile::LANE::W, Point(-0.4f, -0.4f)},
			{common::utile::LANE::S, Point(0.4f, -0.4f)},
			{common::utile::LANE::N, Point(-0.4f, 0.4f)}
		};

		for (const auto& lane : all_lanes)
		{
			RGBColor color = (m_lane_alowence_map[lane] ? COLOR_GREEN : COLOR_RED);

			if (auto it = semaphor_location_based_on_lane.find(lane); it == semaphor_location_based_on_lane.end())
				continue;
			else
				draw_circle(it->second, GLfloat(0.1), color);
		}
			
	}

	void GLFWWindowManager::display_time_left_for_lane(const common::utile::LANE lane, const uint16_t time_left)
	{
		if (!m_textRendere)
			return;

		RGBColor color = COLOR_GREEN;
		if (time_left == 0)
			color = COLOR_RED;

		switch (lane)
		{
		case common::utile::LANE::E:
			m_textRendere->writeText("TIMER E: " + std::to_string(time_left), 530., 780., color);
			break;
		case common::utile::LANE::W:
			m_textRendere->writeText("TIMER W: " + std::to_string(time_left), 20., 100., color);
			break;
		case common::utile::LANE::N:
			m_textRendere->writeText("TIMER N: " + std::to_string(time_left), 20., 780., color);
			break;
		case common::utile::LANE::S:
			m_textRendere->writeText("TIMER S: " + std::to_string(time_left), 530., 100., color);
			break;
		default:
			break;
		}
	}

	void GLFWWindowManager::display_cars_waiting_for_lane(const uint16_t nr_cars, const common::utile::LANE lane, const paint::VehicleTypes type)
	{
		if (!m_textRendere)
			return;

		uint16_t translateY = 0;
		std::string text;
		RGBColor color = COLOR_WHITE;
		switch (type)
		{
		case paint::VehicleTypes::NORMAL_VEHICLE:
			translateY = 0;
			color = COLOR_WHITE;
			text = "CARS" ": " + std::to_string(nr_cars);
			break;
		case paint::VehicleTypes::VT_VEHICLE:
			translateY = 40;
			color = COLOR_LIGHT_BLUE;
			text = "DSRC" ": " + std::to_string(nr_cars);
			break;
		case paint::VehicleTypes::EMERGENCY_VEHICLE:

			if (nr_cars == 0)
				return;

			color = COLOR_RED;
			translateY = 80;
			text = "EMERGENCY";
			break;
		default:
			break;
		}



		switch (lane)
		{
		case common::utile::LANE::E:
			m_textRendere->writeText(text, 532., 740. - translateY, color);
			break;
		case common::utile::LANE::W:
			m_textRendere->writeText(text, 22., 140. + translateY, color);
			break;
		case common::utile::LANE::N:
			m_textRendere->writeText(text, 22., 740. - translateY, color);
			break;
		case common::utile::LANE::S:
			m_textRendere->writeText(text, 532., 140. + translateY, color);
			break;
		default:
			break;
		}
	}

	void GLFWWindowManager::display_cars_waiting()
	{
		if (!m_textRendere)
			return;

		for (const auto& [type, map] : m_vehicleTypeToCarsWaiting)
		{
			for (const auto& [lane, nr_cars] : map)
			{
				display_cars_waiting_for_lane(nr_cars, lane, type);
			}
		}
	}

	void GLFWWindowManager::display_time_left_timers()
	{
		if (!m_textRendere)
			return;

		for (const auto& [lane, timer] : laneToTimerMap_)
		{
			display_time_left_for_lane(lane, timer->getTimeLeft());
		}
	}

	void GLFWWindowManager::render(int window_weight, int window_height)
	{
		if (!glfwInit()) { return; }

		m_window = glfwCreateWindow(window_weight, window_height, "Traffic", NULL, NULL);

		if (!m_window)
		{
			glfwTerminate();
			return;
		}

		glfwMakeContextCurrent(m_window);
		glfwSwapInterval(1);
		glfwSetKeyCallback(m_window, inputK);

		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_BLEND);
		glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);

		while (!glfwWindowShouldClose(m_window)) {
			glClear(GL_COLOR_BUFFER_BIT);

			draw_junction();
			display_time_left_timers();

			size_t numberOfElementToPop = 0;

			numberOfElementToPop = m_waiting_queue.size();

			if (numberOfElementToPop != 0)
			{
				while (!m_waiting_queue.empty() && --numberOfElementToPop)
				{
					std::function<void()> draw_function;
					{
						std::scoped_lock lock(m_mutex);
						draw_function = m_waiting_queue.front();
						m_waiting_queue.pop();
					}

					if (draw_function)
						draw_function();
				}
			}

			display_cars_waiting();

			glfwSwapBuffers(m_window);
			glfwPollEvents();
		}

		glfwDestroyWindow(m_window);
		glfwTerminate();
	}

	GLFWWindowManager::GLFWWindowManager(const std::map<common::utile::LANE, common::utile::TimerPtr>& laneToTimerMap, 
		int window_weight, int window_height, const std::filesystem::path& texturePath) :laneToTimerMap_(laneToTimerMap)
	{
		for (auto i = 0; i < 4; i++)
		{
			m_lane_alowence_map[(common::utile::LANE)i] = false;
		}

		try
		{
			m_textRendere = std::make_unique<TextRenderer>(texturePath);
		}
		catch (const std::exception& err)
		{
			std::cerr << err.what() << std::endl;
		}

		std::vector<paint::VehicleTypes> allVehicleTypes = {
			paint::VehicleTypes::EMERGENCY_VEHICLE, paint::VehicleTypes::NORMAL_VEHICLE, paint::VehicleTypes::VT_VEHICLE };

		std::vector<common::utile::LANE> allLanes = {
			common::utile::LANE::E,
			common::utile::LANE::W,
			common::utile::LANE::N,
			common::utile::LANE::S
		};

		for (const auto& vehicleType : allVehicleTypes)
		{
			std::map<common::utile::LANE, uint16_t> laneToCarCount;
			for (const auto& lane : allLanes)
			{
				laneToCarCount[lane] = 0;
			}

			m_vehicleTypeToCarsWaiting.emplace(vehicleType, std::move(laneToCarCount));
		}

		try
		{
			m_dataCollector = std::make_shared<common::data::DataCollector>();
		}
		catch (const std::exception& err)
		{
			m_dataCollector = nullptr;
			std::cerr << err.what() << std::endl;
		}

		m_rendering_thread = std::thread(std::bind(&GLFWWindowManager::render, this, window_weight, window_height));
	}

	GLFWWindowManager::~GLFWWindowManager()
	{
		if (m_window)
			glfwSetWindowShouldClose(m_window, GL_TRUE);

		if (m_rendering_thread.joinable())
			m_rendering_thread.join();
	}

	bool GLFWWindowManager::isAboutToCrossTheJunction(const Point& bl_point, const GLfloat& height, const GLfloat& width, const common::utile::LANE lane)
	{
		auto junction_border_NE = Point(junction_edge_ / 2, junction_edge_ / 2);
		auto junction_border_SE = Point(junction_edge_ / 2, -junction_edge_ / 2);
		auto junction_border_NW = Point(-junction_edge_ / 2, junction_edge_ / 2);
		auto junction_border_SW = Point(-junction_edge_ / 2, -junction_edge_ / 2);

		// from draw_rect method
		// Point points[] = { {0.0f, 0.0f}, {0.0f, height}, {width, height}, {width, 0.0f} };

		auto car_border_NE = Point(bl_point.m_oX + width, bl_point.m_oY + height);
		auto car_border_SE = Point(bl_point.m_oX + width, bl_point.m_oY);
		auto car_border_NW = Point(bl_point.m_oX, bl_point.m_oY + height);
		auto car_border_SW = Point(bl_point.m_oX, bl_point.m_oY);

		//std::cout << "Car OXSW: " << car_border_SW.m_oX << " OYSW: " << car_border_SW.m_oY 
		//	<< " OYNW: " << car_border_NW.m_oY <<  " OXNW: " << car_border_NW.m_oX
		//	<< " OYSE:" << car_border_SE.m_oY << " OXSE: " << car_border_SE.m_oX
		//	<< " OYNE:" << car_border_NE.m_oY << " OXNE: " << car_border_NE.m_oX
		//	<< std::endl;

		switch (lane)
		{
		case common::utile::LANE::E:
			// ox are valori pozitive
			return  (car_border_SW.m_oX <= junction_border_SE.m_oX) && (car_border_SE.m_oX > junction_border_SE.m_oX);
			break;
		case common::utile::LANE::W:
			// ox are valori negative 
			return (car_border_SE.m_oX >= junction_border_SW.m_oX) && (car_border_SW.m_oX < junction_border_SW.m_oX);
			break;
		case common::utile::LANE::N:
			// oy are valori pozitive
			return (car_border_SW.m_oY <= junction_border_NE.m_oY) && (car_border_NW.m_oY > junction_border_NE.m_oY);
			break;
		case common::utile::LANE::S:
			// oy are valori negative
			return (car_border_NW.m_oY >= junction_border_SW.m_oY) && (car_border_SW.m_oY < junction_border_SW.m_oY);
			break;
		default:
			return false;
		}
	}

	void GLFWWindowManager::drawCars(const paint::VehicleTypes type,
		const common::utile::LANE lane,
		std::optional<Point> bl_point,
		const GLfloat height, const GLfloat width,
		GLfloat moving_rate,
		const size_t nr_cars,
		const uint32_t id)
	{
		RGBColor color = COLOR_WHITE;

		switch (type)
		{
		case paint::VehicleTypes::EMERGENCY_VEHICLE:
			color = COLOR_RED;
			break;
		case paint::VehicleTypes::NORMAL_VEHICLE:
			color = COLOR_WHITE;
			break;
		case paint::VehicleTypes::VT_VEHICLE:
			color = COLOR_LIGHT_BLUE;
			break;
		default:
			return;
		}

		const static std::map<common::utile::LANE, Point> starting_lane_points =
		{
			{common::utile::LANE::E, Point(GLfloat(1.), GLfloat(0.1))},
			{common::utile::LANE::N, Point(GLfloat(-0.1), GLfloat(1.))},
			{common::utile::LANE::S, Point(GLfloat(0.1), GLfloat(-1.))},
			{common::utile::LANE::W, Point(GLfloat(- 1.), GLfloat(-0.1))}
		};

		const static std::map<common::utile::LANE, Point> ending_lane_points = 
		{
			{common::utile::LANE::E, Point(GLfloat(-1.), GLfloat(0.1))},
			{common::utile::LANE::N, Point(GLfloat(-0.1), GLfloat(-1.))},
			{common::utile::LANE::S, Point(GLfloat(0.1), GLfloat(1.))},
			{common::utile::LANE::W, Point(GLfloat(1.), GLfloat(-0.1))}
		};

		if (bl_point == std::nullopt)
		{
			if (auto it = starting_lane_points.find(lane); it != starting_lane_points.end())
			{
				m_vehicleTypeToCarsWaiting[type][lane]+=nr_cars;
				bl_point = it->second;

				if (m_dataCollector)
					m_dataCollector->registerCars(id, nr_cars);
			}
			else
				return;
		}
		else
		{
			if (auto it = ending_lane_points.find(lane); it == ending_lane_points.end() ||
				((std::abs(it->second.m_oX) < std::abs(bl_point.value().m_oX)) ||
					(std::abs(it->second.m_oY) < std::abs(bl_point.value().m_oY))))
			{
				m_vehicleTypeToCarsWaiting[type][lane]-=nr_cars;

				if (m_dataCollector)
					m_dataCollector->unregisterCars(id);

				return;
			}
		}

		// do the actual drawing
		const std::map<common::utile::LANE, std::pair<GLfloat, GLfloat>> rect_shape_based_on_lane =
		{
			{common::utile::LANE::E, {-height, width}},
			{common::utile::LANE::N, {width, -height}},
			{common::utile::LANE::S, {width, height}},
			{common::utile::LANE::W, {height, width}}
		};

		const std::map<common::utile::LANE, Point> moving_rate_base_on_lane =
		{
			{common::utile::LANE::E, Point(-moving_rate, 0.)},
			{common::utile::LANE::N, Point(0., -moving_rate)},
			{common::utile::LANE::S, Point(0., moving_rate)},
			{common::utile::LANE::W,  Point(moving_rate, 0.)}
		};

		if (auto it = rect_shape_based_on_lane.find(lane); it != rect_shape_based_on_lane.end())
		{
			if (!m_lane_alowence_map[lane] && isAboutToCrossTheJunction(bl_point.value(), it->second.first, it->second.second, lane))
			{
				if (auto it2 = moving_rate_base_on_lane.find(lane); it2 != moving_rate_base_on_lane.end() && bl_point.has_value())
				{
					Point oldPoint = bl_point.value();
					oldPoint.m_oX -= it2->second.m_oX;
					oldPoint.m_oY -= it2->second.m_oY;

					draw_rect(oldPoint, it->second.first, it->second.second, color);
				}
				m_waiting_queue.push(std::bind(&GLFWWindowManager::drawCars, this, type, lane, bl_point, height, width, moving_rate, nr_cars, id));
				return;
			}

			draw_rect(bl_point.value(), it->second.first, it->second.second, color);
		}
		else
			return;

		Point new_point = bl_point.value();
		if (auto it = moving_rate_base_on_lane.find(lane); it != moving_rate_base_on_lane.end() && bl_point.has_value())
		{
			new_point.m_oX += it->second.m_oX;
			new_point.m_oY += it->second.m_oY;
		}
		else
			return;

		m_waiting_queue.push(std::bind(&GLFWWindowManager::drawCars, this, type, lane, new_point, height, width, moving_rate, nr_cars, id));
	}

	void GLFWWindowManager::changeTrafficLights(const std::set<common::utile::LANE>& green_light_lanes)
	{
		for (auto i = 0; i < 4; i++)
		{
			m_lane_alowence_map[(common::utile::LANE)i] = false;
		}

		for (const auto& lane : green_light_lanes)
			m_lane_alowence_map[lane] = true;
	}

	void  GLFWWindowManager::signalIncomingCar(const paint::VehicleTypes type,
		const common::utile::LANE lane,
		const size_t nr_cars,
		std::optional<Point> bl_point,
		const GLfloat& height,
		const GLfloat& width,
		GLfloat moving_rate)
	{
		uint32_t id = ++m_id;
		m_waiting_queue.push(std::bind(&GLFWWindowManager::drawCars, this, type, lane, bl_point, height, width, moving_rate, nr_cars, id));
	}

} // namespace model