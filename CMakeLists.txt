cmake_minimum_required(VERSION 3.21)
project(AxiomEngine)

# Set the output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Winvalid-pch")
endif()

# Different settings for Debug and Release
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  message("Building in Debug mode")
  add_definitions(-DDEBUG_MODE)
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
  message("Building in Release mode")
endif()


# Add the submodules' directories
add_subdirectory(lib/glfw EXCLUDE_FROM_ALL)
#add_subdirectory(lib/etc2comp EXCLUDE_FROM_ALL)
add_subdirectory(lib/flecs EXCLUDE_FROM_ALL)
add_subdirectory(lib/optick EXCLUDE_FROM_ALL)
add_subdirectory(lib/taskflow EXCLUDE_FROM_ALL)
add_subdirectory(lib/vulkan-headers EXCLUDE_FROM_ALL)
if (WIN32)
   set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
endif()
add_subdirectory(lib/volk EXCLUDE_FROM_ALL) 
add_subdirectory(lib/tinyxml2 EXCLUDE_FROM_ALL)

set(XXHASH_BUILD_ENABLE_INLINE_API ON) #optional
set(XXHASH_BUILD_XXHSUM OFF) #optional
add_subdirectory(lib/xxHash/cmake_unofficial EXCLUDE_FROM_ALL)

add_subdirectory(lib/glslang)
add_subdirectory(lib/assimp)

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
    lib/volk
    lib/stb
    lib/taskflow
    lib/tinyxml2
    lib/xxHash
    lib/glslang
    lib/assimp/include
    lib/imgui
    lib/imgui/backends
)

# Add source files and header files
file(GLOB SYSTEMS_SOURCE
    start/flecs-world.cpp
    start/helpers.cpp
    systems/*.cpp
    systems/core/*.cpp
    systems/render/*.cpp
    systems/scene/*.cpp
)
file(GLOB SYSTEMS_HEADERS
    start/flecs-world.h
    start/helpers.h
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
file(GLOB EDITOR_SOURCES
    editor/*.cpp
    lib/imgui/*.cpp
    lib/imgui/backends/imgui_impl_vulkan.cpp
    lib/imgui/backends/imgui_impl_glfw.cpp
)

file (GLOB_RECURSE EDITOR_HEADERS
    editor/*.h
    lib/imgui/*.h
    lib/imgui/backends/imgui_impl_vulkan.h
)

# Create the library
add_library(AxiomEngineLib STATIC ${COMPONENTS} ${SYSTEMS_SOURCE} ${SYSTEMS_HEADERS})
#add_library(AxiomEditorLib STATIC ${EDITOR_SOURCES} ${EDITOR_HEADERS})

# Link the submodules' libraries to the library
target_link_libraries(AxiomEngineLib glfw ${GLFW_LIBRARIES})
target_link_libraries(AxiomEngineLib flecs_static)
target_link_libraries(AxiomEngineLib OptickCore)
target_link_libraries(AxiomEngineLib Vulkan::Headers)
target_link_libraries(AxiomEngineLib ${OPENGL_LIBRARIES})
#target_link_libraries(AxiomEngineLib etc2comp)
target_link_libraries(AxiomEngineLib tinyxml2)
target_link_libraries(AxiomEngineLib xxhash)
target_link_libraries(AxiomEngineLib glslang SPIRV)
target_link_libraries(AxiomEngineLib assimp)


# Link the include libs
target_include_directories(AxiomEngineLib PRIVATE lib/taskflow)
target_include_directories(AxiomEngineLib PRIVATE lib/volk)
#target_include_directories(AxiomEngineLib PRIVATE ${VULKAN_INCLUDE_DIR})


target_compile_options(flecs PUBLIC -w)  # -w suppresses all warnings in GCC/Clang


# Define DEBUG_MODE for debug configuration
target_compile_definitions(AxiomEngineLib PRIVATE $<$<CONFIG:Debug>:DEBUG_MODE>)

# Create the executable
add_executable(AxiomEngine start/main.cpp)
target_precompile_headers(AxiomEngine PRIVATE start/pch.h)
target_link_libraries(AxiomEngine AxiomEngineLib)

#add_executable(AxiomEditor editor/editor-main.cpp ${EDITOR_SOURCES} ${EDITOR_HEADERS})
#target_link_libraries(AxiomEditor PUBLIC AxiomEngineLib)