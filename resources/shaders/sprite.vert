#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 projection;
uniform vec4 uvRect;

void main()
{
    TexCoords = uvRect.xy + aTexCoords * uvRect.zw;
    gl_Position = projection * model * vec4(aPos, 0.0, 1.0);
}
