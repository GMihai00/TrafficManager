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

#define COLOR_BLACK RGBColor{0.0f, 0.0f, 0.0f}
#define COLOR_WHITE RGBColor{255.0f, 255.0f, 255.0f}
#define COLOR_RED RGBColor{255.0f, 0.0f, 0.0f}
#define COLOR_LIGHT_BLUE RGBColor{0.0f, 255.0f, 255.0f}