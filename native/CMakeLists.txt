cmake_minimum_required(VERSION 3.10)
project(medusa_native)

set(CMAKE_CXX_STANDARD 17)

# Set output directory to bin/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../bin)

# Controller DLL
add_library(medusa_controller SHARED controller/controller.cpp)

# Hook DLL
add_library(medusa_hook SHARED hook/hook.cpp)

# Logger executable
add_executable(medusa_log logger/logger.cpp)