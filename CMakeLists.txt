cmake_minimum_required(VERSION 3.15)
project(lilds)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files
set(SOURCES
    src/app/main.cpp
    src/helpers.cpp

    src/core/arm/arm7.cpp
)

# Create the executable
add_executable(lilds ${SOURCES})