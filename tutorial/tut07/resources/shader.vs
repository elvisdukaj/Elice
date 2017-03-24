#version 330
layout (location = 0) in vec3 Position;
out vec4 color;

uniform mat4 world;

void main()
{
    gl_Position = world * vec4(Position, 1.0);
    color = vec4(clamp(Position, 0.0, 1.0), 1.0f);
}
