#pragma once

#include "GraphicsDataTypes.hpp"

void draw_square(const Point& bl_point, const GLfloat& edge, const RGBColor& color);
void draw_rect(const Point& bl_point, const GLfloat& height, const GLfloat& width, const RGBColor& color);
void draw_dotted_line(const Point& start_point, const Point& end_point, const GLfloat& dot_size = 0.01f, const RGBColor& color = COLOR_WHITE);
void draw_traffic_lane_line(const Point& start_point, const Point& end_point, const GLfloat& line_thickness = 0.05f, const RGBColor& color = COLOR_WHITE);