#pragma once
#include <GLFW/glfw3.h>

struct Point
{
	GLfloat m_oX;
	GLfloat m_oY;
	Point() = delete;
	Point(GLfloat oX, GLfloat oY) : m_oX(oX), m_oY(oY) {}
};

struct RGBColor
{
	GLfloat m_red;
	GLfloat m_green;
	GLfloat m_blue;
	RGBColor() = delete;
	RGBColor(GLfloat red, GLfloat green, GLfloat blue) : m_red(red), m_green(green), m_blue(blue) {}
};

#ifndef COLOR_BLACK
#define COLOR_BLACK RGBColor{0.0f, 0.0f, 0.0f}
#endif // !COLOR_BLACK

#ifndef COLOR_WHITE
#define COLOR_WHITE RGBColor{255.0f, 255.0f, 255.0f}
#endif // !COLOR_WHITE

#ifndef COLOR_RED
#define COLOR_RED RGBColor{255.0f, 0.0f, 0.0f}
#endif // !COLOR_RED

#ifndef COLOR_LIGHT_BLUE
#define COLOR_LIGHT_BLUE RGBColor{0.0f, 255.0f, 255.0f}
#endif  // !COLOR_LIGHT_BLUE

#ifndef COLOR_GREEN
#define COLOR_GREEN RGBColor{0.0f, 255.0f, 0.0f}
#endif // !COLOR_GREEN


