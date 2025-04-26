cmake_minimum_required(VERSION 3.15)
project(AlgorithmPlayground)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Specify source and include directories
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

# Add include directories
include_directories(${INCLUDE_DIR})  # Ensures include/ headers are found

# Collect all source files
file(GLOB SOURCES "${SRC_DIR}/*.cpp")

# Create executable
add_executable(AlgorithmPlayground ${SOURCES})

# Set MSVC specific compiler flags
if (MSVC)
    target_compile_options(AlgorithmPlayground PRIVATE /W4 /WX)
    # Increase default stack size (reserve 2 MB)
    set(CMAKE_EXE_LINKER_FLAGS
        "${CMAKE_EXE_LINKER_FLAGS} /STACK:2000000")
endif()

# Set output directory
set_target_properties(AlgorithmPlayground PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
