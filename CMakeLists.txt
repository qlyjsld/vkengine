# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

# set the project name
project(vk_engine)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add source files
file(GLOB SOURCE_FILES 
	"vk_engine/src/headers/*.h"
	"vk_engine/src/*.cpp")

# add the executable
add_executable(vk_engine ${SOURCE_FILES})

# Vulkan
find_package(Vulkan REQUIRED FATAL_ERROR)
target_link_libraries(vk_engine Vulkan::Vulkan)

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory("Libraries/glfw-3.3.2")

target_link_libraries(vk_engine glfw)

# glm
set(glm_DIR "Libraries/glm/cmake/glm")
find_package(glm REQUIRED FATAL_ERROR)

# Define the include DIRs
include_directories(
	"vk_engine/src/headers"
	"Libraries/glfw-3.3.2/include"
	"Libraries/glm"
	${Vulkan_INCLUDE_DIRS}
)

# Copy resources
FILE(COPY vk_engine/shaders DESTINATION "${CMAKE_BINARY_DIR}")
