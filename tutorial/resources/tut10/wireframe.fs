#version 330

in vec3 vsColor;
out vec4 fragmentColor;

void main()
{
    fragmentColor = vec4(1.0 - vsColor.z, 1.0 - vsColor.z, 1.0 - vsColor.z, 1.0f);
}
