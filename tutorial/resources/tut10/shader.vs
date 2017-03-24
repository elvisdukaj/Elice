#version 330

uniform mat4 world;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

out vec4 vsColor;

void main()
{
    gl_Position = world * vec4(position, 1.0);
    vsColor = vec4(color, 1.0);
}
