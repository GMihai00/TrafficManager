#include "GLFWWindowManager.hpp"

#include "DrawHelpers.hpp"

namespace model
{
	static void inputK(GLFWwindow* window, int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GL_TRUE);
	}

	void GLFWWindowManager::draw_junction()
	{
		//UP LEFT
		draw_dotted_line(Point(-1.0f, 0.5f), Point(-0.5f, 0.5f));
		draw_dotted_line(Point(-0.5f, 0.5f), Point(-0.5f, 1.0f));

		//UP RIGHT
		draw_dotted_line(Point(1.0f, 0.5f), Point(0.5f, 0.5f));
		draw_dotted_line(Point(0.5f, 0.5f), Point(0.5f, 1.0f));
		
		//DOWN RIGHT
		draw_dotted_line(Point(1.0f, -0.5f), Point(0.5f, -0.5f));
		draw_dotted_line(Point(0.5f, -0.5f), Point(0.5f, -1.0f));

		//DOWN LEFT
		draw_dotted_line(Point(-1.0f, -0.5f), Point(-0.5f, -0.5f));
		draw_dotted_line(Point(-0.5f, -0.5f), Point(-0.5f, -1.0f));

		draw_traffic_lane_line(Point(-1.0f, 0.0f), Point(1.0f, 0.0f), 0.035f, COLOR_WHITE);
		draw_traffic_lane_line(Point(0.0f, -1.0f), Point(0.0f, 1.0f), 0.035f, COLOR_WHITE);

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
			
			//empty other queue and start drawing
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
} // namespace model