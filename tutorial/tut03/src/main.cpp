#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <stdexcept>
#include <array>
#include <sstream>
#include <iostream>

using namespace std;

static int g_scale_uniform_loc;

GLFWwindow* InitializeWindow(int width, int height, const std::string& title)
{
    if (!glfwInit())
        throw runtime_error{"Unable to init GLFW"};

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(width, height,
                                          title.c_str(),
                                          nullptr, nullptr
                                          );

    if (!window)
        throw runtime_error{"Unable to create GLFW Window"};

    return window;
}

void InitGrapics(GLFWwindow* window)
{
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
        throw runtime_error{"Unable to init GLEW"};

    int width, height;
    glfwGetWindowSize(window, &width, &height);

    glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
    glViewport(0, 0, width, height);
}

void WindowsResize(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
}

void KeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void InitalizeEvents(GLFWwindow* window)
{
    glfwSetWindowSizeCallback(window, WindowsResize);
    glfwSetKeyCallback(window, KeyEvent);
}

GLuint CreateTriangleBuffer()
{
    array<glm::vec3, 3> vertexes =
    {
        glm::vec3{-1.0f, -1.0f,  0.0f},
        glm::vec3{ 1.0f, -1.0f,  0.0f},
        glm::vec3{ 0.0f,  1.0f,  0.0f},
    };

    GLuint triangleVertexBufferObject = 0;
    glGenBuffers(1, &triangleVertexBufferObject);

    if (triangleVertexBufferObject == 0)
        throw runtime_error{"Unable to create Buffer"};

    glBindBuffer(GL_ARRAY_BUFFER, triangleVertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(vertexes),
                 vertexes.data(),
                 GL_STATIC_DRAW
                 );

    return triangleVertexBufferObject;
}

GLuint CreateTriangleVertexArrayObject(GLuint triangleVBO)
{
    GLuint triangleVertexArrayObject;

    glGenVertexArrays(1, & triangleVertexArrayObject);
    glBindVertexArray(triangleVertexArrayObject);

    glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
                0, 3, GL_FLOAT, GL_FALSE,
                0, nullptr
                );

    return triangleVertexArrayObject;
}

GLuint CreateShaderFromSource(GLenum type, const std::string& source)
{
    GLuint shaderObj = glCreateShader(type);

    if (!shaderObj)
        throw runtime_error{"Unable to create the shader object"};

    const GLchar* sources[1] = { source.c_str() };
    GLint lengths[1] = {GLint(source.length())};
    glShaderSource(shaderObj, 1, sources, lengths);
    glCompileShader(shaderObj);

    GLint buildResult;
    glGetShaderiv(shaderObj, GL_COMPILE_STATUS, &buildResult);

    if (!buildResult)
    {
        GLint logLength;
        glGetShaderiv(shaderObj, GL_INFO_LOG_LENGTH, &logLength);

        string logMessage(logLength, ' ');
        glGetShaderInfoLog(shaderObj, logLength, nullptr, &logMessage[0]);

        throw runtime_error{logMessage};
    }

    return shaderObj;
}

GLuint CreateGPUProgram()
{
    std::string vsSrc = R"(#version 330
layout (location = 0) in vec3 Position;

uniform float scale_uniform;

void main()
{
    gl_Position = vec4(Position * scale_uniform, 1.0);
})";

    std::string fsSrc = R"(#version 330
out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0, 0.0, 1.0, 1.0);
})";

    auto vsObj = CreateShaderFromSource(GL_VERTEX_SHADER, vsSrc);
    auto fsObj = CreateShaderFromSource(GL_FRAGMENT_SHADER, fsSrc);

    GLuint gpuProg = glCreateProgram();
    if (!gpuProg)
        throw runtime_error{"Unable to create gpu program"};

    glAttachShader(gpuProg, vsObj);
    glAttachShader(gpuProg, fsObj);

    GLint linkResult;
    glLinkProgram(gpuProg);
    glGetProgramiv(gpuProg, GL_LINK_STATUS, &linkResult);
    if (!linkResult)
    {
        GLint logLength;
        glGetProgramiv(gpuProg, GL_INFO_LOG_LENGTH, &logLength);

        string logMessage(logLength, ' ');
        glGetProgramInfoLog(gpuProg, logLength, nullptr, &logMessage[0]);

        throw runtime_error{logMessage};
    }

    GLint validateResult;
    glGetProgramiv(gpuProg, GL_VALIDATE_STATUS, &validateResult);
    if (!linkResult)
    {
        GLint logLength;
        glGetProgramiv(gpuProg, GL_INFO_LOG_LENGTH, &logLength);

        string logMessage(logLength, ' ');
        glGetProgramInfoLog(gpuProg, logLength, nullptr, &logMessage[0]);

        throw runtime_error{logMessage};
    }

    g_scale_uniform_loc = glGetUniformLocation(gpuProg, "scale_uniform");

    if (glGetError() != GL_NO_ERROR)
        throw runtime_error{"Unable to get scale_uniform location"};

    return gpuProg;
}

void draw(GLuint triangleVAO)
{
    static float scale = 0.0f;

    glEnableVertexAttribArray(0);
    glBindVertexArray(triangleVAO);

    scale += 0.01f;
    glUniform1f(g_scale_uniform_loc, sin(scale));

    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableVertexAttribArray(0);
}

int main(int , char **)
{
    try
    {
        auto window = InitializeWindow(800, 600, "Tutorial 03 - Triangle");

        InitGrapics(window);
        InitalizeEvents(window);

        cout << "Vendor:       " << glGetString(GL_VENDOR)   << '\n'
             << "Version:      " << glGetString(GL_VERSION)  << '\n'
             << "Renderer:     " << glGetString(GL_RENDERER) << '\n'
             << endl;

        auto triangleBufferVBO = CreateTriangleBuffer();
        auto triangleBufferVAO = CreateTriangleVertexArrayObject(triangleBufferVBO);
        auto gpuProg = CreateGPUProgram();

        while(!glfwWindowShouldClose(window))
        {
            glClear(GL_COLOR_BUFFER_BIT);

            glUseProgram(gpuProg);
            draw(triangleBufferVAO);
            glUseProgram(0);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        glDeleteBuffers(1, &triangleBufferVAO);

        glfwDestroyWindow(window);
        glfwTerminate();
    }
    catch(const exception& exc)
    {
        glfwTerminate();
        cerr << exc.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
