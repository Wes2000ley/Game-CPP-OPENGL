
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <freetype2/ft2build.h>
#include <glad/glad.h>

#include FT_FREETYPE_H

#include "text_renderer.h"
#include "resource_manager.h"


TextRenderer::TextRenderer(unsigned int width, unsigned int height)
{
    // load and configure shader
    this->TextShader = ResourceManager::LoadShader("resources/shaders/text.vert", "resources/shaders/text.frag", nullptr, "text");
    this->TextShader->SetInteger("text", 0);
    // configure VAO/VBO for texture quads
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);
    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void TextRenderer::Load(std::string font, unsigned int fontSize)
{
    // first clear the previously loaded Characters
    this->Characters.clear();
    // then initialize and load the FreeType library
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) // all functions return a value different than 0 whenever an error occurred
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
    // load font as face
    FT_Face face;
    if (FT_New_Face(ft, font.c_str(), 0, &face))
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
    // set size to load glyphs as
    FT_Set_Pixel_Sizes(face, 0, fontSize);
    // disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    // then for the first 128 ASCII characters, pre-load/compile their characters and store them
    for (GLubyte c = 0; c < 128; c++) // lol see what I did there
    {
        // load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
            );
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x)
        };
        Characters.insert(std::pair<char, Character>(c, character));
    }

    this->FontSize = fontSize;
    this->Ascent = face->size->metrics.ascender >> 6;
    this->Descent = face->size->metrics.descender >> 6;
    glBindTexture(GL_TEXTURE_2D, 0);
    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void TextRenderer::RenderText(std::string text, float x, float y, float scale, glm::vec3 color, const glm::mat4& projection)
{
    // activate corresponding render state
    this->TextShader->Use();
    this->TextShader->SetMatrix4("projection", projection); // ✅ Set here instead
    this->TextShader->SetVector3f("textColor", color);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(this->VAO);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y + (this->Characters['H'].Bearing.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 0.0f },
            { xpos,     ypos,       0.0f, 0.0f },

            { xpos,     ypos + h,   0.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 0.0f }
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (1/64th times 2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

float TextRenderer::MeasureTextWidth(const std::string& text, float scale) {
    float width = 0.0f;
    for (char c : text) {
        auto it = Characters.find(c);
        if (it != Characters.end())
            width += (it->second.Advance >> 6) * scale;  // 1/64th pixel units → pixels
    }
    return width;
}
glm::vec4 TextRenderer::MeasureRenderedTextBounds(const std::string& text, float x, float y, float scale) const // Add const
{
    // ... existing implementation ...
    float minX = x, maxX = x; // Initialize minX/maxX with the initial x
    float minY = y, maxY = y; // Initialize minY/maxY with the initial y
    // (or use FLT_MAX/FLT_MIN if text could be empty and render nothing)
    bool firstChar = true;


    // Store the initial pen position before iterating
    float currentPenX = x;

    for (char c_char : text) // Use char c_char to avoid conflict with RenderText's c iterator
    {
        auto it = Characters.find(c_char);
        if (it == Characters.end()) continue;

        const Character& ch = it->second;

        // xpos and ypos are the top-left of the glyph's bitmap
        float xpos = currentPenX + ch.Bearing.x * scale;
        float ypos = y + (Characters.at('H').Bearing.y - ch.Bearing.y) * scale; // Assuming 'H' exists and y is "top of H" line

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        if (firstChar) {
            minX = xpos;
            maxX = xpos + w;
            minY = ypos;
            maxY = ypos + h;
            firstChar = false;
        } else {
            minX = std::min(minX, xpos);
            maxX = std::max(maxX, xpos + w);
            minY = std::min(minY, ypos); // Smallest y-coordinate (topmost)
            maxY = std::max(maxY, ypos + h); // Largest y-coordinate + h (bottommost)
        }
        // Advance pen position for the next character
        currentPenX += (ch.Advance >> 6) * scale;
    }

    if (firstChar) { // Text was empty or all chars not found
        return glm::vec4(x, y, 0.0f, 0.0f);
    }

    return glm::vec4(minX, minY, maxX - minX, maxY - minY); // (actual_left, actual_top, actual_width, actual_height)
}
