#include "camera.h"
#include "pipeline.h"
#include "gpu.h"
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <array>
#include <fstream>

using namespace std;

float g_xAxis = 1.0f;
float g_yAxis = 0.0f;
float g_zAxis = 0.0f;

static int g_triangle_world_uniform_loc;

Camera g_mainCamera;

class TriangleProgram : public Program {
public:
    TriangleProgram(std::vector<Shader>&& shaders)
        : Program{move(shaders)}

    {
        enable();
        m_worldUniformLoc = glGetUniformLocation(m_program, "world");
        disable();
    }

    TriangleProgram(const Program&) = delete;
    TriangleProgram(Program&& rhs);

    void setModelViewProjection(const glm::mat4& mvp)
    {
        glUniformMatrix4fv(m_worldUniformLoc, 1, GL_FALSE, glm::value_ptr(mvp));
    }

private:
    GLuint m_worldUniformLoc;
};

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
                                   width,
                                   height,
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

    const float side = 2.0f;
//    g_mainCamera.ortho(-side, side, -side, side, -side, side);
    g_mainCamera.perspective(45.0f, float(width) / (float)height, 0.01f, 1000.0f);
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

        // front
        glm::vec3{-1.0f, -1.0f,  1.0f},
        glm::vec3{ 1.0f, -1.0f,  1.0f},
        glm::vec3{ 1.0f,  1.0f,  1.0f},
        glm::vec3{-1.0f,  1.0f,  1.0f},
        // back
        glm::vec3{-1.0f, -1.0f, -1.0f},
        glm::vec3{ 1.0f, -1.0f, -1.0f},
        glm::vec3{ 1.0f,  1.0f, -1.0f},
        glm::vec3{-1.0f,  1.0f, -1.0f},

        // colors
        glm::vec3{1.0f, 0.0f, 1.0f},
        glm::vec3{0.0f, 1.0f, 0.0f},
        glm::vec3{0.0f, 0.0f, 1.0f},
        glm::vec3{1.0f, 1.0f, 1.0f},

        glm::vec3{1.0f, 1.0f, 1.0f},
        glm::vec3{0.0f, 0.0f, 1.0f},
        glm::vec3{0.0f, 1.0f, 0.0f},
        glm::vec3{1.0f, 0.0f, 1.0f},
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
        // front
        0, 1, 2, 2, 3, 0,
        // top
        1, 5, 6, 6, 2, 1,
        // back
        7, 6, 5, 5, 4, 7,
        // bottom
        4, 0, 3, 3, 7, 4,
        // left
        4, 5, 1, 1, 0, 4,
        // right
        3, 2, 6, 6, 7, 3,
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
                0, (GLvoid*)(sizeof(glm::vec3) * 8)
                );

    return TriangleBuffers{vao, buffers[vbo], buffers[ibo], indeces.size()};
}

void drawTriangle(const TriangleBuffers triangle, TriangleProgram& gpuProg)
{
    gpuProg.enable();
    static float scale = 0.0f;

    Pipeline p;

    glBindVertexArray(triangle.vao);

    scale += 0.01f;
    const auto scaleFactor = sin(scale * 0.1f);
    p.scale(glm::vec3{scaleFactor, scaleFactor, scaleFactor});
    p.worldPos(glm::vec3{sin(scale), 0.0f, 0.0f});
    p.rotate(scale, glm::vec3(g_xAxis, g_yAxis, g_zAxis));

    const auto mvp = static_cast<glm::mat4>(g_mainCamera) * static_cast<glm::mat4>(p);
    gpuProg.setModelViewProjection(mvp);

    glDrawElements(GL_TRIANGLES, triangle.count, GL_UNSIGNED_INT, nullptr);

    gpuProg.disable();
}

TriangleProgram CreateTriangleGPUProgram()
{
    vector<Shader> shaders;
    shaders.emplace_back(ShaderType::vertex  , move(ifstream{"../../resources/tut10/shader.vs"}));
    shaders.emplace_back(ShaderType::fragment, move(ifstream{"../../resources/tut10/shader.fs"}));
    return TriangleProgram{move(shaders)};
}

bool HandleWindowsInput()
{
    auto mustQuit = false;
    SDL_Event event;

    static bool x = true;
    static bool y = false;
    static bool z = false;

    const auto STEP_SIZE = 0.02f;

    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_UP:
                g_mainCamera.forward(STEP_SIZE);
                break;

            case SDLK_DOWN:
                g_mainCamera.forward(-STEP_SIZE);
                break;

            case SDLK_LEFT:
                g_mainCamera.lateral(STEP_SIZE);
                break;

            case SDLK_RIGHT:
                g_mainCamera.lateral(-STEP_SIZE);
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
            }
            break;

        case SDL_KEYUP:
            switch (event.key.keysym.sym)
            {
            case SDLK_q:
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

    // at least one axis true
    if (!x && !y && !z)
        x = true;

    g_xAxis = x ? 1.0f : 0.0f;
    g_yAxis = y ? 1.0f : 0.0f;
    g_zAxis = z ? 1.0f : 0.0f;

    return mustQuit;
}

int main(int, char **)
{
    try
    {
        auto window = InitializeWindow(800, 600, "Tutorial 10 - Pipleine Triangle - SDL2");
        InitGrapics(window);

        cout << "Vendor:       " << glGetString(GL_VENDOR)   << '\n'
             << "Version:      " << glGetString(GL_VERSION)  << '\n'
             << "Renderer:     " << glGetString(GL_RENDERER) << '\n'
             << endl;

        auto triangle = CreateTriangleBuffer();
        TriangleProgram gpuProg = move(CreateTriangleGPUProgram());

        while(!HandleWindowsInput())
        {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            drawTriangle(triangle, gpuProg);
            SDL_GL_SwapWindow(window);
        }

        glDeleteBuffers(1, &triangle.ibo);
        glDeleteBuffers(1, &triangle.vbo);
        glDeleteVertexArrays(1, &triangle.vao);

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
