#include "GLFWWindowManager.hpp"

#include "DrawHelpers.hpp"
#include <functional>

namespace model
{
	static void inputK(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GL_TRUE);
	}

	void GLFWWindowManager::draw_junction()
	{
		//UP LEFT
		draw_dotted_line(Point(-1.0f, 0.25f), Point(-0.25f, 0.25f));
		draw_dotted_line(Point(-0.25f, 0.25f), Point(-0.25f, 1.0f));

		//UP RIGHT
		draw_dotted_line(Point(1.0f, 0.25f), Point(0.25f, 0.25f));
		draw_dotted_line(Point(0.25f, 0.25f), Point(0.25f, 1.0f));
		
		//DOWN RIGHT
		draw_dotted_line(Point(1.0f, -0.25f), Point(0.25f, -0.25f));
		draw_dotted_line(Point(0.25f, -0.25f), Point(0.25f, -1.0f));

		//DOWN LEFT
		draw_dotted_line(Point(-1.0f, -0.25f), Point(-0.25f, -0.25f));
		draw_dotted_line(Point(-0.25f, -0.25f), Point(-0.25f, -1.0f));

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
				draw_circle(it->second, 0.1, color);
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

			// de vazut daca nu cumva is probleme pe multithread aici dar nu cred
			std::queue<std::function<void()>> m_draw_actions = std::move(m_waing_queue);

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

	GLFWWindowManager::GLFWWindowManager(int window_weight, int window_height)
	{
		m_rendering_thread = std::thread(std::bind(&GLFWWindowManager::render, this, window_weight, window_height));
	}

	GLFWWindowManager::~GLFWWindowManager()
	{
		if (m_window)
			glfwSetWindowShouldClose(m_window, GL_TRUE);

		if (m_rendering_thread.joinable())
			m_rendering_thread.join();
	}

	void GLFWWindowManager::drawCar(const RGBColor& color,
		const common::utile::LANE lane,
		std::optional<Point> bl_point,
		const GLfloat& height, const GLfloat& width,
		GLfloat moving_rate)
	{
		if (!m_lane_alowence_map[lane])
		{
			m_waing_queue.push(std::bind(&GLFWWindowManager::drawCar, this, color, lane, bl_point, height, width, moving_rate));
			return;
		}

		const static std::map<common::utile::LANE, Point> starting_lane_points =
		{
			{common::utile::LANE::E, Point(1., 0.)},
			{common::utile::LANE::N, Point(0., 1.)},
			{common::utile::LANE::S, Point(0., -1.)},
			{common::utile::LANE::W,  Point(-1., 0.)}
		};

		const static std::map<common::utile::LANE, Point> ending_lane_points = 
		{
			{common::utile::LANE::E, Point(-1., 0.)},
			{common::utile::LANE::N, Point(0., -1.)},
			{common::utile::LANE::S, Point(0., 1.)},
			{common::utile::LANE::W, Point(1., 0.)}
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
				((it->second.m_oX < bl_point.value().m_oX) || (it->second.m_oY < bl_point.value().m_oY)))
					return;
		}

		// do the actual drawing
		const std::map<common::utile::LANE, std::pair<GLfloat, GLfloat>> rect_shape_based_on_lane =
		{
			{common::utile::LANE::E, {height, -width}},
			{common::utile::LANE::N, {width, -height}},
			{common::utile::LANE::S, {width, height}},
			{common::utile::LANE::W, {height, width}}
		};

		if (auto it = rect_shape_based_on_lane.find(lane); it != rect_shape_based_on_lane.end())
		{
			draw_rect(bl_point.value(), it->second.first, it->second.second, color);
		}
		else
			return;

		const std::map<common::utile::LANE, Point> moving_rate_base_on_lane =
		{
			{common::utile::LANE::E, Point(-moving_rate, 0.)},
			{common::utile::LANE::N, Point(0., -moving_rate)},
			{common::utile::LANE::S, Point(0., -moving_rate)},
			{common::utile::LANE::W,  Point(moving_rate, 0.)}
		};

		if (auto it = moving_rate_base_on_lane.find(lane); it != moving_rate_base_on_lane.end() && bl_point.has_value())
		{
			bl_point->m_oX += it->second.m_oX;
			bl_point->m_oY += it->second.m_oY;
		}
		else
			return;

		m_waing_queue.push(std::bind(&GLFWWindowManager::drawCar, this, color, lane, bl_point, height, width, moving_rate));
	}

	void GLFWWindowManager::changeTrafficLights(const std::set<common::utile::LANE>& green_light_lanes)
	{
		m_lane_alowence_map.clear();

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