cmake_minimum_required(VERSION 4.0)
project(binary64_multiplier_simulator)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

set(SOURCES
        main.cpp
        src/FloatData.cpp
        src/Decomposer.cpp
        src/Multiplier.cpp
        src/Simulator.cpp
        src/Menu.cpp
)

add_executable(binary64_multiplier_simulator ${SOURCES})
