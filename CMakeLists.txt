cmake_minimum_required(VERSION 3.2)
project(AxiomEngine)

#Add source files
file(GLOB SOURCE_FILES src/*.cpp)

#Add header files
file(GLOB HEADER_FILES include/*.h)

#Add Executable
add_executable(AxiomEngine ${HEADER_FILES} ${SOURCE_FILES})

# Set o utpout directory
set(CMAKE_RUTIME_OUTPUT_DIRECTORY bin/)

