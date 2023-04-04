cmake_minimum_required(VERSION 3.14)
project(AxiomEngine)

# Set the output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Find OpenGL package
find_package(OpenGL REQUIRED)

# Add the submodules' directories
add_subdirectory(lib/glfw)
add_subdirectory(lib/etc2comp)
add_subdirectory(lib/flecs)
add_subdirectory(lib/optick)
add_subdirectory(lib/taskflow)
add_subdirectory(lib/googletest)
add_subdirectory(lib/vulkan-headers)

# Include the submodules' header files
include_directories(lib/glm)
include_directories(lib/stb)

# Your project's sources
file(GLOB SOURCES "src/*.cpp")

#Add source files
file(GLOB SOURCE_FILES src/*.cpp)

#Add header files
file(GLOB HEADER_FILES include/*.h)

#Add Executable
add_executable(AxiomEngine ${HEADER_FILES} ${SOURCE_FILES})

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link the submodules' libraries to your project
target_link_libraries(${PROJECT_NAME} glfw ${GLFW_LIBRARIES})
target_link_libraries(${PROJECT_NAME} etc2comp)
target_link_libraries(${PROJECT_NAME} flecs_static)
target_link_libraries(${PROJECT_NAME} OptickCore)
target_link_libraries(${PROJECT_NAME} Taskflow::Taskflow)
target_link_libraries(${PROJECT_NAME} Vulkan::Headers)
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})

# Google Test integration
file(GLOB TEST_SOURCES "tests/*.cpp")
add_executable(tests ${TEST_SOURCES})
target_link_libraries(tests gtest gtest_main ${PROJECT_NAME})