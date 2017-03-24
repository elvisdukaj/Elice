#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <string>
#include <stdexcept>
#include <array>
#include <sstream>
#include <iostream>

using namespace std;

static int g_world_uniform_loc;

SDL_Window* InitializeWindow(int width, int height, const std::string& title)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        throw runtime_error{"Unable to init SDL2"};

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,
                        SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    auto window = SDL_CreateWindow(title.c_str(),
                                   SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                   width, height,
                                   SDL_WINDOW_OPENGL);

    if (!window)
        throw runtime_error{"Unable to create GLFW Window"};


    return window;
}

void InitGrapics(SDL_Window* window)
{
    auto context = SDL_GL_CreateContext(window);
    if (!context)
        throw std::runtime_error{"Unable to create gl context"};

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
        throw runtime_error{"Unable to init GLEW"};

    int width, height;
    SDL_GetWindowSize(window, &width, &height);

    glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
    glViewport(0, 0, width, height);
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

uniform mat4 world;

void main()
{
    gl_Position = world * vec4(Position, 1.0);
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

    g_world_uniform_loc = glGetUniformLocation(gpuProg, "world");

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
    auto world = glm::translate(glm::mat4(1.0f), glm::vec3(sin(scale), 0.0f, 0.0f));
    glUniformMatrix4fv(g_world_uniform_loc, 1, GL_FALSE, glm::value_ptr(world));

    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableVertexAttribArray(0);
}

bool HandleWindowsInput()
{
    auto mustQuit = false;
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                mustQuit = true;
                break;
            }
        break;

        case SDL_QUIT:
            mustQuit = true;
            break;
        }
    }
    return mustQuit;
}

int main(int , char **)
{
    try
    {
        auto window = InitializeWindow(800, 600, "Tutorial 05 - Move Triangle - SDL2");

        InitGrapics(window);

        cout << "Vendor:       " << glGetString(GL_VENDOR)   << '\n'
             << "Version:      " << glGetString(GL_VERSION)  << '\n'
             << "Renderer:     " << glGetString(GL_RENDERER) << '\n'
             << endl;

        auto triangleBufferVBO = CreateTriangleBuffer();
        auto triangleBufferVAO = CreateTriangleVertexArrayObject(triangleBufferVBO);
        auto gpuProg = CreateGPUProgram();

        while(!HandleWindowsInput())
        {
            glClear(GL_COLOR_BUFFER_BIT);

            glUseProgram(gpuProg);
            draw(triangleBufferVAO);
            glUseProgram(0);

            SDL_GL_SwapWindow(window);
        }

        glDeleteBuffers(1, &triangleBufferVAO);
        glDeleteProgram(gpuProg);

        SDL_DestroyWindow(window);
        SDL_Quit();
    }
    catch(const exception& exc)
    {
        SDL_Quit();
        cerr << exc.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
