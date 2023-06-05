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

		// TO DO
		switch (lane)
		{
		case common::utile::LANE::E:
			m_textRendere->writeText("TIMER E: " + std::to_string(time_left), 530., 780.);
			break;
		case common::utile::LANE::W:
			m_textRendere->writeText("TIMER W: 120" + std::to_string(time_left), 20., 100.);
			break;
		case common::utile::LANE::N:
			m_textRendere->writeText("TIMER N: 120" + std::to_string(time_left), 20., 780.);
			break;
		case common::utile::LANE::S:
			m_textRendere->writeText("TIMER S: 120" + std::to_string(time_left), 530., 100.);
			break;
		default:
			break;
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

			std::unique_lock<std::mutex> lock(m_mutex);
			std::queue<std::function<void()>> m_draw_actions = std::move(m_waing_queue);
			lock.unlock();

			while (!m_draw_actions.empty())
			{
				std::function<void()> draw_function = m_draw_actions.front();

				if (draw_function)
					draw_function();

				m_draw_actions.pop();
			}

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

	void GLFWWindowManager::drawCar(const RGBColor& color,
		const common::utile::LANE lane,
		std::optional<Point> bl_point,
		const GLfloat& height, const GLfloat& width,
		GLfloat moving_rate)
	{
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
				bl_point = it->second;
			else
				return;
		}
		else
		{
			if (auto it = ending_lane_points.find(lane); it == ending_lane_points.end() ||
				((std::abs(it->second.m_oX) < std::abs(bl_point.value().m_oX)) ||
					(std::abs(it->second.m_oY) < std::abs(bl_point.value().m_oY))))
			{
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
				m_waing_queue.push(std::bind(&GLFWWindowManager::drawCar, this, color, lane, bl_point, height, width, moving_rate));
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

		m_waing_queue.push(std::bind(&GLFWWindowManager::drawCar, this, color, lane, new_point, height, width, moving_rate));
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
		std::optional<Point> bl_point,
		const GLfloat& height,
		const GLfloat& width,
		GLfloat moving_rate)
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

		m_waing_queue.push(std::bind(&GLFWWindowManager::drawCar, this, color, lane, bl_point, height, width, moving_rate));
	}

} // namespace model