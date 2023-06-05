#include <iostream>
#include <exception>
#include "TextRenderer.hpp"

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

namespace model
{
    TextRenderer::TextRenderer(const std::filesystem::path& fontPath, int fontSize, int dpi) :
        m_fontSize{ fontSize },
        m_dpi{ dpi }
    {
        std::error_code ec;
        if (!std::filesystem::is_regular_file(fontPath, ec))
            throw std::runtime_error("Invalid font file");

        if (ec)
            throw std::runtime_error("Invalid font file err= " + ec.value());

        if (FT_Init_FreeType(&m_ft))
            throw std::runtime_error("Could not init FreeType Library");

        if (FT_New_Face(m_ft, fontPath.string().c_str(), 0, &m_face))
            throw std::runtime_error("Failed to load font");

    }

    TextRenderer::~TextRenderer() noexcept
    {
        FT_Done_Face(m_face);
        FT_Done_FreeType(m_ft);
    }

    bool TextRenderer::writeText(const std::string& text, GLfloat x, GLfloat y, const RGBColor& color)
    {
        if (FT_Set_Char_Size(m_face, 0, m_fontSize * 64, m_dpi, m_dpi))
        {
            std::cout << "Failed to set text size";
            return false;
        }

        GLuint* textures = (GLuint*)malloc(sizeof(GLuint) * text.size());

        if (textures == NULL)
        {
            return false;
        }

        glGenTextures(static_cast<GLsizei>(text.size()), textures);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, 800, 0, 800, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glColor3f(color.m_red, color.m_green, color.m_blue);

        FT_GlyphSlot gliphSlot = m_face->glyph;

        if (!gliphSlot)
        {
            return false;
        }

        glEnable(GL_TEXTURE_2D);

        GLuint tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        auto scale = 1.0;

        for (const auto& chr : text)
        {
            if (FT_Load_Char(m_face, chr, FT_LOAD_RENDER))
                continue;

            glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, gliphSlot->bitmap.width,
                gliphSlot->bitmap.rows, 0, GL_ALPHA, GL_UNSIGNED_BYTE,
                gliphSlot->bitmap.buffer);

            glBegin(GL_QUADS);

            glTexCoord2f(0, 1);
            glVertex2f(x, -(GLfloat)gliphSlot->bitmap.rows + y);

            glTexCoord2f(1, 1);
            glVertex2f(gliphSlot->bitmap.width + x, -(GLfloat)gliphSlot->bitmap.rows + y);

            glTexCoord2f(1, 0);
            glVertex2f(gliphSlot->bitmap.width + x, y);

            glTexCoord2f(0, 0);
            glVertex2f(x, y);

            glEnd();

            x += static_cast<int>((gliphSlot->advance.x >> 6) * scale);
        }

        glDeleteTextures(1, &tex);
        glDisable(GL_BLEND);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-1, 1, -1, 1, -1, 1);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    }

    void TextRenderer::setFontSize(const int fontSize)
    {
        m_fontSize = fontSize;
    }

} // namespace model
