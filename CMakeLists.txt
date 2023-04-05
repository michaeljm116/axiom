cmake_minimum_required(VERSION 3.14)
project(AxiomEngine)

# Set the output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Find OpenGL package
find_package(OpenGL REQUIRED)

# Add the submodules' directories
add_subdirectory(lib/glfw EXCLUDE_FROM_ALL)
#add_subdirectory(lib/etc2comp EXCLUDE_FROM_ALL)
add_subdirectory(lib/flecs EXCLUDE_FROM_ALL)
add_subdirectory(lib/optick EXCLUDE_FROM_ALL)
add_subdirectory(lib/taskflow EXCLUDE_FROM_ALL)
add_subdirectory(lib/googletest EXCLUDE_FROM_ALL)
add_subdirectory(lib/vulkan-headers EXCLUDE_FROM_ALL)

# Include the submodules' header files
include_directories(lib/glm)
include_directories(lib/stb)

# Add source files
file(GLOB SOURCES src/*.cpp)

# Add header files
file(GLOB HEADERS include/*.h)

# Create the library
add_library(AxiomEngineLib STATIC ${HEADERS} ${SOURCES})

# Link the submodules' libraries to the library
target_link_libraries(AxiomEngineLib glfw ${GLFW_LIBRARIES})
#target_link_libraries(AxiomEngineLib etc2comp)
target_link_libraries(AxiomEngineLib flecs_static)
target_link_libraries(AxiomEngineLib OptickCore)
target_link_libraries(AxiomEngineLib Vulkan::Headers)
target_link_libraries(AxiomEngineLib ${OPENGL_LIBRARIES})

# link the include libs
target_include_directories(AxiomEngineLib PRIVATE lib/taskflow)


# Create the executable
add_executable(AxiomEngine src/main.cpp)
target_link_libraries(AxiomEngine AxiomEngineLib)

# Google Test integration
file(GLOB TEST_SOURCES "tests/*.cpp")
add_executable(tests ${TEST_SOURCES})
target_link_libraries(tests gtest gtest_main AxiomEngineLib)