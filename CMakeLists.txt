cmake_minimum_required(VERSION 3.15)
project(lilds)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files
set(SOURCES
    # App
    src/app/main.cpp

    # Misc
    src/helpers.cpp
    src/math.cpp

    # Core
    src/core/ds.cpp
    src/core/arm/interpreter/arm32.cpp
    src/core/arm/interpreter/alu.cpp
    src/core/arm/interpreter/mul.cpp
    src/core/arm/interpreter/condition_codes.cpp
    src/core/arm/interpreter/decode.cpp
    src/core/arm/arm.cpp
    src/core/bus/bus.cpp
)

# Create the executable
add_executable(lilds ${SOURCES})