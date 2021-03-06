cmake_minimum_required(VERSION 3.1)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}"
    "/usr/local/share/GLEW/cmake/Modules"
    "/usr/local/lib/cmake/glfw3/"
    "/usr/local/lib/x86_64-linux-gnu/cmake/glm/"
    )

set (CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(tut03)

# project files
aux_source_directory(src SRC_LIST)
file(GLOB_RECURSE HEADERS "include/*.h*")

find_package(glm)
find_package(GLEW)
find_package(glfw3)
find_package(OpenGL REQUIRED)

include_directories("${GLM_INCLUDE_DIRS}")
include_directories("${OPENGL_INCLUDE_DIR}")
include_directories("${GLEW_INCLUDE_DIR}")

# output and linker
add_executable(${PROJECT_NAME} ${SRC_LIST} ${HEADERS})

target_link_libraries(${PROJECT_NAME}
    ${GLEW_LIBRARY}
    glfw
    ${OPENGL_gl_LIBRARY}
)

