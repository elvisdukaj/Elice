#version 330

uniform mat4 world;

layout(location = 0) in vec3 position;


void main()
{
    gl_Position = world * vec4(position, 1.0);
}
