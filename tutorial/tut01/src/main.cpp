#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <boost/scope_exit.hpp>

using namespace std;

void InitializeWindow()
{
    if (!glfwInit())
        throw std::runtime_error{"Unable to init GLFW"};

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

void InitGrapics()
{
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
        throw std::runtime_error{"Unable to init GLEW"};
}

void QueryVRAM()
{
    bool hasNVXMemInfo = glewGetExtension("GL_NVX_gpu_memory_info");
    bool hasATIMemInfo = glewGetExtension("GL_ATI_meminfo");

    if (!(hasNVXMemInfo || hasATIMemInfo))
    {
        cerr << "Cannot querry VRAM info" << endl;
        return;
    }

    if (hasNVXMemInfo)
    {
        int VRAMDedicated, VRAMDTotalAviable, VRAMCurrentAviable, evictionCount, evicedMemory;

        glGetIntegerv(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX,         &VRAMDedicated);
        glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX,   &VRAMDTotalAviable);
        glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &VRAMCurrentAviable);
        glGetIntegerv(GL_GPU_MEMORY_INFO_EVICTION_COUNT_NVX,           &evictionCount);
        glGetIntegerv(GL_GPU_MEMORY_INFO_EVICTED_MEMORY_NVX,           &evicedMemory);

        cout << "GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX         " << VRAMDedicated      << '\n'
             << "GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX         " << VRAMDTotalAviable  << '\n'
             << "GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX " << VRAMCurrentAviable << '\n'
             << "GPU_MEMORY_INFO_EVICTION_COUNT_NVX           " << evictionCount      << '\n'
             << "GPU_MEMORY_INFO_EVICTED_MEMORY_NVX           " << evicedMemory       << '\n'
             << endl;
    }
}

GLuint AllocateVRAM(size_t bytes)
{
    vector<uint8_t> arrayBuffer(bytes);
    GLuint buffer;
    glGenBuffers(1, &buffer);

    if (buffer == 0)
        throw std::runtime_error{"Unable to create buffer"};

    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, arrayBuffer.size(), arrayBuffer.data(), GL_DYNAMIC_READ);

    return buffer;
}

int main(int , char **)
{
    try
    {
        const size_t bytes = 1000 * 1024 * 1024;

        BOOST_SCOPE_EXIT(void)
        {
            glfwTerminate();
        } BOOST_SCOPE_EXIT_END

        InitializeWindow();

        GLFWwindow* window = glfwCreateWindow(800, 600, "GPU Mem Fill", nullptr, nullptr);

        glfwMakeContextCurrent(window);
        InitGrapics();

        cout
             << "Vendor:             " << glGetString(GL_VENDOR) << '\n'
             << "Version:            " << glGetString(GL_VERSION) << '\n'
             << "Renderer:           " << glGetString(GL_RENDERER) << '\n'
             << "GLSL Version:       " << glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n'
             << endl;

        cout << "allocating 100 MByte" << endl;
        auto vab = AllocateVRAM(1000 * 1024 * 1024);
        cout << "Vidoe Memory Allocated, Press [Enter] to continue" << endl;

        string line;
        getline(cin, line);

        glDeleteBuffers(1, &vab);
    }
    catch(const exception& exc)
    {
        cerr << exc.what() << endl;
    }

    return EXIT_SUCCESS;
}
