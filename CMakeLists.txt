cmake_minimum_required(VERSION 3.15)
project(lilds)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files
set(SOURCES
    # App
    src/app/main.cpp
    src/app/frontend/frontend.cpp

    # Misc
    src/helpers.cpp
    src/math.cpp

    # Core
    src/core/ds.cpp
    src/core/arm/interpreter/arm32.cpp
    src/core/arm/interpreter/condition_codes.cpp
    src/core/arm/interpreter/decode.cpp
    src/core/arm/arm.cpp
    src/core/bus/vram_banks.cpp
    src/core/bus/bus.cpp
    src/core/bus/load_rom.cpp
    src/core/joypad/joypad.cpp
)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Address Sanitizer
# add_compile_options(-fsanitize=address)
# add_link_options(-fsanitize=address)

# Create the executable
add_executable(lilds ${SOURCES})
target_link_libraries(lilds ${SDL2_LIBRARIES})