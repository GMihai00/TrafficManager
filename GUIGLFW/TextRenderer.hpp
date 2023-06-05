#pragma once

#include <filesystem>
#include <memory>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <GLFW/glfw3.h>
#include "GraphicsDataTypes.hpp"

namespace model
{
	class TextRenderer
	{
		FT_Library m_ft;
		FT_Face m_face;
		int m_dpi;
		int m_fontSize;

	public:
		TextRenderer() = delete;
		TextRenderer(const TextRenderer& obj) = delete;
		TextRenderer(const TextRenderer&& obj) = delete;
		TextRenderer(const std::filesystem::path& fontPath, int fontSize = 10, int dpi = 300);
		~TextRenderer() noexcept;

		bool writeText(const std::string& text, GLfloat x, GLfloat y, const RGBColor& color = COLOR_WHITE);
		void setFontSize(const int fontSize);
	};
	typedef std::unique_ptr<TextRenderer> TextRendererPtr;

} // namespace model 