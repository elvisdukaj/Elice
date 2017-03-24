cmake_minimum_required(VERSION 3.1)

set(CMAKE_MODULE_PATH
    "/usr/local/share/GLEW/cmake/Modules"
    "/usr/local/lib/x86_64-linux-gnu/cmake/glm/"
    "/usr/lib/x86_64-linux-gnu/cmake/SDL2/sdl2-config.cmake"
    )

set (CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(tut10)

# project files
aux_source_directory(src SRC_LIST)
file(GLOB_RECURSE HEADERS "include/*.h*")

find_package(glm REQUIRED)
find_package(GLEW REQUIRED)
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

include_directories("${GLM_INCLUDE_DIRS}")
include_directories("${GLEW_INCLUDE_DIR}")
include_directories("${SDL2_INCLUDE_DIR}")
include_directories("${OPENGL_INCLUDE_DIR}")
INCLUDE_DIRECTORIES(include)

# output and linker
add_executable(${PROJECT_NAME} ${SRC_LIST} ${HEADERS})

target_link_libraries(${PROJECT_NAME}
    ${GLEW_LIBRARY}
    ${SDL2_LIBRARY}
    ${OPENGL_gl_LIBRARY}
)

