cmake_minimum_required(VERSION 3.14)
project(AxiomEngine)

# Add the submodules' include_directories
add_subdirectory(lib/glm)
add_subdirectory(lib/flecs)

# Include the submodule's header files
include_directories(lib/glm/include)
include_directories(lib/flecs/include)

#Add source files
file(GLOB SOURCE_FILES src/*.cpp)

#Add header files
file(GLOB HEADER_FILES include/*.h)

#Add Executable
add_executable(AxiomEngine ${HEADER_FILES} ${SOURCE_FILES})

# Set output directory
set(CMAKE_RUTIME_OUTPUT_DIRECTORY bin/)

target_link_libraries(${AxiomEngine} glm)
target_link_libraries(${AxiomEngine} flecs)