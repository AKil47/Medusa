cmake_minimum_required(VERSION 3.10)
project(medusa_native)

set(CMAKE_CXX_STANDARD 17)

# Set output directory to bin/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../bin)

# MSDetours library setup
set(DETOURS_ROOT ${CMAKE_SOURCE_DIR}/../third_party/Detours)
add_custom_target(build_detours
    COMMAND nmake
    WORKING_DIRECTORY ${DETOURS_ROOT}/src
    COMMENT "Building MSDetours library"
)
set(DETOURS_LIB_DIR ${DETOURS_ROOT}/lib.X64)
set(DETOURS_INCLUDE_DIR ${DETOURS_ROOT}/include)

# Controller DLL
add_library(medusa_controller SHARED controller/controller.cpp)
target_link_libraries(medusa_controller user32 advapi32 psapi)

# Hook DLL
add_library(medusa_hook SHARED hook/hook.cpp)
target_include_directories(medusa_hook PRIVATE ${DETOURS_INCLUDE_DIR})
target_link_libraries(medusa_hook ${DETOURS_LIB_DIR}/detours.lib user32 kernel32 advapi32 psapi)
add_dependencies(medusa_hook build_detours)

# Logger executable
add_executable(medusa_log logger/logger.cpp)
