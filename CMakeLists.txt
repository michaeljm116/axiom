cmake_minimum_required(VERSION 3.14)
project(AxiomEngine)

# Set the output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Winvalid-pch")

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
if (WIN32)
   set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
endif()
add_subdirectory(lib/volk EXCLUDE_FROM_ALL) 

# Include the submodules' header files
include_directories(
    start
    systems
    components
    systems/render
    systems/core
    systems/scene
    components/render
    components/core
    components/scene
)
include_directories(
    lib/glm 
    lib/glfw/include
    lib/stb
    lib/imgui
    lib/volk
)


# Add source files and header files
file(GLOB SYSTEMS_SOURCE
    systems/*.cpp
    systems/core/*.cpp
    systems/render/*.cpp
    systems/scene/*.cpp
)
file(GLOB SYSTEMS_HEADERS
    systems/*.h 
    systems/core/*.h 
    systems/render/*.h
    systems/scene/*.h
)
file(GLOB COMPONENTS 
    components/*.h 
    components/render/*.h
    components/core/*.h
    components/scene/*.h
)

# Create the library
add_library(AxiomEngineLib STATIC ${COMPONENTS} ${SYSTEMS_SOURCE} ${SYSTEMS_HEADERS})

# Link the submodules' libraries to the library
target_link_libraries(AxiomEngineLib glfw ${GLFW_LIBRARIES})
target_link_libraries(AxiomEngineLib flecs_static)
target_link_libraries(AxiomEngineLib OptickCore)
target_link_libraries(AxiomEngineLib Vulkan::Headers)
target_link_libraries(AxiomEngineLib ${OPENGL_LIBRARIES})
#target_link_libraries(AxiomEngineLib etc2comp)


# link the include libs
target_include_directories(AxiomEngineLib PRIVATE lib/taskflow)
target_include_directories(AxiomEngineLib PRIVATE lib/volk)

# Create the executable
add_executable(AxiomEngine start/main.cpp)
target_precompile_headers(AxiomEngine PRIVATE start/pch.h)
target_link_libraries(AxiomEngine AxiomEngineLib)
