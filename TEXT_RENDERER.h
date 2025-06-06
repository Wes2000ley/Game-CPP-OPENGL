/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <map>
#include <memory>

//#include <glad/glad.h>
#include <glm/glm.hpp>

#include "TEXTURE.h"
#include "shader.h"


/// Holds all state information relevant to a character as loaded using FreeType
struct Character {
	unsigned int TextureID;   // ID handle of the glyph texture
	glm::ivec2   Size;        // Size of glyph
	glm::ivec2   Bearing;     // Offset from baseline to left/top of glyph
	unsigned int Advance;     // Horizontal offset to advance to next glyph
};



// A renderer class for rendering text displayed by a font loaded using the
// FreeType library. A single font is loaded, processed into a list of Character
// items for later rendering.
class TextRenderer
{
public:
	// holds a list of pre-compiled Characters
	std::map<char, Character> Characters;
	// shader used for text rendering
	std::shared_ptr<Shader> TextShader;
	// constructor
	TextRenderer(unsigned int width, unsigned int height);
	// pre-compiles a list of characters from the given font
	void Load(std::string font, unsigned int fontSize);
	// renders a string of text using the precompiled list of characters
	void RenderText(std::string text, float x, float y, float scale, glm::vec3 color, const glm::mat4& projection);

	glm::vec4 MeasureRenderedTextBounds(const std::string& text, float x, float y, float scale) const;
	static TextRenderer& LoadTextRenderer(const std::string& name, unsigned int width, unsigned int height);
	static TextRenderer& GetTextRenderer(const std::string& name);
	float MeasureTextWidth(const std::string& text, float scale);
	float GetLineHeight(float scale) const {
		return (Ascent - Descent) * scale;
	}
    unsigned int GetFontSize() const { return FontSize; }
	unsigned int FontSize;
private:
	// render state
	unsigned int VAO{}, VBO{};
	int Ascent;
	int Descent;




};
float MeasureTextWidth(const std::string& text, float scale);


#endif
