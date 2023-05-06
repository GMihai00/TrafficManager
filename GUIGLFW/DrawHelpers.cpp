#include "DrawHelpers.hpp"
#include <iostream>
#include <cmath>

void draw_square(const Point& bl_point, const GLfloat& edge, const RGBColor& color)
{
	Point points[] = { {0.0f, 0.0f}, {0.0f, edge}, {edge, edge}, {edge, 0.0f} };

	glBegin(GL_POLYGON);
	glColor3f(color.m_red, color.m_green, color.m_blue);
	for (auto i = 0; i < 4; i++)
	{
		glVertex2f(bl_point.m_oX + points[i].m_oX, bl_point.m_oY + points[i].m_oY);
	}
	glEnd();
}

void draw_rect(const Point& bl_point, const GLfloat& height, const GLfloat& width, const RGBColor& color)
{
	Point points[] = { {0.0f, 0.0f}, {0.0f, height}, {width, height}, {width, 0.0f} };

	glBegin(GL_POLYGON);
	glColor3f(color.m_red, color.m_green, color.m_blue);
	for (auto i = 0; i < 4; i++)
	{
		glVertex2f(bl_point.m_oX + points[i].m_oX, bl_point.m_oY + points[i].m_oY);
	}
	glEnd();
}

void draw_dotted_line(const Point& start_point, const Point& end_point, const GLfloat& dot_size, const RGBColor& color)
{
    GLfloat distance = std::sqrt(std::pow(end_point.m_oX - start_point.m_oX, 2.0f) + std::pow(end_point.m_oY - start_point.m_oY, 2.0f));
	
    GLint num_dots = static_cast<GLint>(std::ceil(distance / dot_size));

    GLfloat dot_distance = distance / num_dots;

    GLfloat dx = (end_point.m_oX - start_point.m_oX) / distance;
    GLfloat dy = (end_point.m_oY - start_point.m_oY) / distance;

    glColor3f(color.m_red, color.m_green, color.m_blue);
    glBegin(GL_POINTS);
    for (int i = 0; i < num_dots; ++i) {
        GLfloat x = start_point.m_oX + i * dot_distance * dx;
        GLfloat y = start_point.m_oY + i * dot_distance * dy;
        glVertex2f(x, y);
    }
    glEnd();
}

void draw_traffic_lane_line(const Point& start_point, const Point& end_point, const GLfloat& line_thickness, const RGBColor& color = COLOR_WHITE)
{
	Point current_point = start_point;

	if (end_point.m_oX == start_point.m_oX)
	{
		GLfloat distance = std::abs(end_point.m_oY - start_point.m_oY);
		GLfloat rect_width = line_thickness;
		GLfloat rect_height = line_thickness * 1.5f;

		auto number_dots = distance / (rect_height * 2);

		for (int i = 0; i < number_dots; i++)
		{
			draw_rect(current_point, rect_height, rect_width, color);
			current_point.m_oY += rect_height * 2;
		}
	}
	else if (end_point.m_oY == start_point.m_oY)
	{
		GLfloat distance = std::abs(end_point.m_oX - start_point.m_oX);
		GLfloat rect_height = line_thickness;
		GLfloat rect_width = line_thickness * 2;

		auto number_dots = distance / (rect_width * 2);

		for (int i = 0; i < number_dots; i++)
		{
			draw_rect(current_point, rect_height, rect_width, color);
			current_point.m_oX += rect_width * 2;
		}
	}
	else
	{
		std::cerr << "Invalid data passed to the function, points should form a perpendicular line on the oX/oY axis";
	}
}