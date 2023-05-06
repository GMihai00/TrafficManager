#pragma once
#ifndef GUI_GLFWWINDOWMANAGER_HPP
#define GUI_GLFWWINDOWMANAGER_HPP

#include <queue>
#include <thread>
#include <mutex>
#include <functional>

#include <GLFW/glfw3.h>

namespace model
{
	class GLFWWindowManager
	{
	private:
		std::thread m_rendering_thread;
		std::queue<std::function<void()>> m_waing_queue;

		GLFWwindow* m_window = nullptr;

		void draw_junction();

		void render(int window_weight, int window_height);
	public:
		GLFWWindowManager(int window_weight = 1280, int window_height = 1024);
		virtual ~GLFWWindowManager() noexcept;
	};
} // namespace model
#endif // !GUI_GLFWWINDOWMANAGER_HPP
