cmake_minimum_required(VERSION 3.14.0)
project(
    oops
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CXX_FLAGS "-Wall -stdlib=libc++")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
)

file(GLOB_RECURSE GLAD_SOURCES libs/glad/src/*.c)
add_executable(oops main.cpp ${SOURCES})
find_package(OpenGL REQUIRED)
target_link_libraries(oops PRIVATE ${OPENGL_LIBRARIES} ${OPENGL_INCLUDE_DIR})

find_package(OpenGL REQUIRED)
target_link_libraries(oops PRIVATE OpenGL::GL)

find_package(GLEW REQUIRED)
target_link_libraries(oops PRIVATE GLEW::GLEW)

# this is heuristically generated, and may not be correct
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(oops PRIVATE glfw)

find_package(glm CONFIG REQUIRED)
target_link_libraries(oops PRIVATE glm::glm)

# Or use the header-only version
find_package(glm CONFIG REQUIRED)
target_link_libraries(oops PRIVATE glm::glm-header-only "-framework Cocoa" "-framework OpenGL" "-framework IOKit")
