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
#include <fstream>
#include <algorithm>

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

struct TriangleBuffers {
    GLuint vao;
    GLuint vbo;
    GLuint ibo;
    int count;
};

TriangleBuffers CreateTriangleBuffer()
{
    enum {vbo, ibo};
    array<GLuint, 2> buffers = {0};

    array<glm::vec3, 4> vertexes =
    {
        glm::vec3{-1.0f, -1.0f,  0.0f},
        glm::vec3{ 0.0f, -1.0f,  1.0f},
        glm::vec3{ 1.0f, -1.0f,  0.0f},
        glm::vec3{ 0.0f,  1.0f,  0.0f},
    };

    glGenBuffers(buffers.size(), buffers.data());

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    if (any_of(begin(buffers), end(buffers), [](GLuint buff) { return buff == 0; } ))
        throw runtime_error{"Unable to create Buffer"};

    glBindBuffer(GL_ARRAY_BUFFER, buffers[vbo]);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(vertexes),
                 vertexes.data(),
                 GL_STATIC_DRAW
                 );

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
                0, 3, GL_FLOAT, GL_FALSE,
                0, nullptr
                );

    constexpr auto indeces_count = 12;
    array<GLuint, indeces_count> indeces = {
        0, 3, 1,
        1, 3, 2,
        2, 3, 0,
        0, 1, 2
    };

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[ibo]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(indeces),
                 indeces.data(),
                 GL_STATIC_DRAW
                 );
    return TriangleBuffers{vao, buffers[vbo], buffers[ibo], indeces.size()};
}

void drawTriangle(const TriangleBuffers triangle)
{
    static float scale = 0.0f;

    glBindVertexArray(triangle.vao);

    scale += 0.01f;
    const auto scaled = sin(scale);
    auto world = glm::scale(glm::mat4(1.0f), glm::vec3(scaled, scaled, scaled));
    glUniformMatrix4fv(g_world_uniform_loc, 1, GL_FALSE, glm::value_ptr(world));

    glDrawElements(GL_TRIANGLES, triangle.count, GL_UNSIGNED_INT, nullptr);
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

GLuint CreateShaderFromFile(GLenum type, const std::string& filename)
{
    std::ifstream file(filename);

    if (!file)
        throw std::invalid_argument{"The specified file doesn't exists"};

    std::string source;

    file.seekg(0, std::ios::end);
    source.reserve(file.tellg());
    file.seekg(0, std::ios::beg);

    source.assign(std::istreambuf_iterator<char>{file},
                  std::istreambuf_iterator<char>{ });

    return CreateShaderFromSource(type, source);
}

GLuint CreateTriangleGPUProgram()
{
    auto vsObj = CreateShaderFromFile(GL_VERTEX_SHADER, "../tut08/resources/shader.vs");
    auto fsObj = CreateShaderFromFile(GL_FRAGMENT_SHADER, "../tut08/resources/shader.fs");

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

int main(int, char **)
{
    try
    {
        auto window = InitializeWindow(800, 600, "Tutorial 08 - Indexed Triangle - SDL2");
        InitGrapics(window);

        cout << "Vendor:       " << glGetString(GL_VENDOR)   << '\n'
             << "Version:      " << glGetString(GL_VERSION)  << '\n'
             << "Renderer:     " << glGetString(GL_RENDERER) << '\n'
             << endl;

        auto triangle = CreateTriangleBuffer();
        auto gpuProg = CreateTriangleGPUProgram();

        while(!HandleWindowsInput())
        {
            glClear(GL_COLOR_BUFFER_BIT);

            glUseProgram(gpuProg);
            drawTriangle(triangle);
            glUseProgram(0);

            SDL_GL_SwapWindow(window);
        }

        glDeleteBuffers(1, &triangle.ibo);
        glDeleteBuffers(1, &triangle.vbo);
        glDeleteVertexArrays(1, &triangle.vao);
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
