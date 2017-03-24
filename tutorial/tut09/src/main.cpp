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

float g_xAxis = 1.0f;
float g_yAxis = 0.0f;
float g_zAxis = 0.0f;

static int g_world_uniform_loc;
static bool g_wireframe = false;

enum class Wireframe {solid, wireframe, both};

Wireframe g_wireframeEnum;

Wireframe GetNextWireframeEnum(Wireframe w)
{
    switch (w) {
    case Wireframe::solid:
        return Wireframe::wireframe;
    case Wireframe::wireframe:
        return Wireframe::both;
    case Wireframe::both:
        return Wireframe::solid;
    default:
        return Wireframe::wireframe;
    }
}

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

void SetViewport(int width, int height)
{
    glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
    glViewport(0, 0, width, height);
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
    SetViewport(width, height);

    glEnable(GL_DEPTH_TEST);
}

struct TriangleBuffers {
    GLuint vao;
    GLuint vbo;
    GLuint ibo;
    size_t count;
};

TriangleBuffers CreateTriangleBuffer()
{
    enum {vbo, ibo};
    array<GLuint, 2> buffers = {0};

    std::vector<glm::vec3> vertexes =
    {
        // positions
        glm::vec3{-0.5f, -0.5f,  0.0f},
        glm::vec3{ 0.0f, -0.5f,  0.5f},
        glm::vec3{ 0.5f, -0.5f,  0.0f},
        glm::vec3{ 0.0f,  0.5f,  0.0f},
        // colors
        glm::vec3{1.0f, 0.0f, 1.0f},
        glm::vec3{0.0f, 1.0f, 0.0f},
        glm::vec3{0.0f, 0.0f, 1.0f},
        glm::vec3{1.0f, 1.0f, 0.0f},
    };

    glGenBuffers(buffers.size(), buffers.data());

    if (any_of(begin(buffers), end(buffers), [](GLuint buff) { return buff == 0; } ))
        throw runtime_error{"Unable to create Buffer"};

    glBindBuffer(GL_ARRAY_BUFFER, buffers[vbo]);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(vertexes[0]) * vertexes.size(),
                 vertexes.data(),
                 GL_STATIC_DRAW
                 );

    vector<int> indeces = {
        0, 3, 1,
        1, 3, 2,
        2, 3, 0,
        0, 1, 2,
        0
    };

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[ibo]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(indeces[0]) * indeces.size(),
                 indeces.data(),
                 GL_STATIC_DRAW
                 );

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[ibo]);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, buffers[vbo]);

    glVertexAttribPointer(
                0, 3, GL_FLOAT, GL_FALSE,
                0, nullptr
                );

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
                1, 3, GL_FLOAT, GL_FALSE,
                0, (GLvoid*)(sizeof(glm::vec3) * 4)
                );

    return TriangleBuffers{vao, buffers[vbo], buffers[ibo], indeces.size()};
}

void drawTriangle(const TriangleBuffers triangle)
{
    static float scale = 0.0f;

    glBindVertexArray(triangle.vao);

    scale += 0.01f;
    auto world = glm::rotate(glm::mat4(1.0f), scale, glm::vec3(g_xAxis, g_yAxis, g_zAxis));

    glUniformMatrix4fv(g_world_uniform_loc, 1, GL_FALSE, glm::value_ptr(world));

    if (g_wireframe)
        glDrawElements(GL_LINE_STRIP, triangle.count, GL_UNSIGNED_INT, nullptr);
    else
        glDrawElements(GL_TRIANGLE_STRIP, triangle.count - 1, GL_UNSIGNED_INT, nullptr);
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
    auto vsObj = CreateShaderFromFile(GL_VERTEX_SHADER, "../../resources/tut09/shader.vs");
    auto fsObj = CreateShaderFromFile(GL_FRAGMENT_SHADER, "../../resources/tut09/shader.fs");

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

GLuint CreateTriangleGPUWireframeProgram()
{
    auto vsObj = CreateShaderFromFile(GL_VERTEX_SHADER, "../../resources/tut09/wireframe.vs");
    auto fsObj = CreateShaderFromFile(GL_FRAGMENT_SHADER, "../../resources/tut09/wireframe.fs");

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

    static bool x = true;
    static bool y = false;
    static bool z = false;

    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_q:
            case SDLK_ESCAPE:
                mustQuit = true;
                break;

            case SDLK_x:
                x = !x;
                break;

            case SDLK_y:
                y = !y;
                break;

            case SDLK_z:
                z = !z;
                break;

            case SDLK_w:
                g_wireframe = !g_wireframe;
                g_wireframeEnum = GetNextWireframeEnum(g_wireframeEnum);
            }

            // at least one axis true
            if (!x && !y && !z)
                x = true;

            g_xAxis = x ? 1.0f : 0.0f;
            g_yAxis = y ? 1.0f : 0.0f;
            g_zAxis = z ? 1.0f : 0.0f;
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
        auto window = InitializeWindow(800, 600, "Tutorial 09 - Pipleine Triangle - SDL2");
        InitGrapics(window);

        cout << "Vendor:       " << glGetString(GL_VENDOR)   << '\n'
             << "Version:      " << glGetString(GL_VERSION)  << '\n'
             << "Renderer:     " << glGetString(GL_RENDERER) << '\n'
             << endl;

        auto triangle = CreateTriangleBuffer();
        auto gpuProg = CreateTriangleGPUProgram();
        auto wireframeProg = CreateTriangleGPUWireframeProgram();

        while(!HandleWindowsInput())
        {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            if (g_wireframe)
                glUseProgram(wireframeProg);
            else
                glUseProgram(gpuProg);

            drawTriangle(triangle);
            glUseProgram(0);

            SDL_GL_SwapWindow(window);
        }

        glDeleteBuffers(1, &triangle.ibo);
        glDeleteBuffers(1, &triangle.vbo);
        glDeleteVertexArrays(1, &triangle.vao);
        glDeleteProgram(gpuProg);
        glDeleteProgram(wireframeProg);

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
