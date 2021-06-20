# CMakeLists.txt
include(ExternalProject)
cmake_minimum_required(VERSION 3.10)

# set the project name
project(VkEngine)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add source files
file(GLOB SOURCE_FILES
	"VkEngine/Source/Main.cpp"
	"VkEngine/Source/VkEngine.h"
	"VkEngine/Source/VkEngine/Renderer/*.h"
	"VkEngine/Source/VkEngine/Renderer/*.cpp"
	"VkEngine/Source/VkEngine/Core/*.h"
	"VkEngine/Source/VkEngine/Core/*.cpp"
	"VkEngine/Source/VkEngine/Asset/*.cpp"
	"VkEngine/Source/VkEngine/Asset/*.h"
	"vendors/lz4-1.9.3/lib/lz4.c")

# add the executable
add_executable(VkEngine ${SOURCE_FILES})

# Add source files
file(GLOB SOURCE_FILES
	"VkEngine/Source/VkAsset.cpp"
	"VkEngine/Source/VkEngine/Asset/*.cpp"
	"VkEngine/Source/VkEngine/Asset/*.h"
	"vendors/lz4-1.9.3/lib/lz4.c")

# add the executable
add_executable(VkAsset ${SOURCE_FILES})

# Vulkan
find_package(Vulkan REQUIRED FATAL_ERROR)
target_link_libraries(VkEngine Vulkan::Vulkan)

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory("vendors/glfw-3.3.2")

target_link_libraries(VkEngine glfw)

# glm
set(glm_DIR "vendors/glm/cmake/glm")
find_package(glm REQUIRED FATAL_ERROR)

# spdlog
ExternalProject_Add(spdlog
    PREFIX spdlog
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/vendors/spdlog
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_INSTALL_PREFIX=${STAGING_DIR}
    -DSPDLOG_BUILD_SHARED=OFF
)

add_dependencies(VkEngine spdlog)

# Define the include DIRs
include_directories(
	"VkEngine/Source"
	"vendors/glfw-3.3.2/include"
	"vendors/glm"
	"vendors/vma"
	"vendors/stb_image"
	"vendors/spdlog/include"
	"vendors/tinyobjloader"
	"vendors/lz4-1.9.3/lib"
	"vendors/json"
	"vendors/VirtuosoConsole"
	${Vulkan_INCLUDE_DIRS}
)

# Copy resources
FILE(COPY VkEngine/Shaders DESTINATION "${CMAKE_BINARY_DIR}")
FILE(COPY VkEngine/Assets DESTINATION "${CMAKE_BINARY_DIR}")
