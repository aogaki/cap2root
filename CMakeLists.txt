cmake_minimum_required(VERSION 3.15)
project(cap2root)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find ROOT
find_package(ROOT REQUIRED COMPONENTS RIO Tree)
include(${ROOT_USE_FILE})

# Find Cap'n Proto
find_package(PkgConfig REQUIRED)
pkg_check_modules(CAPNP REQUIRED capnp)

include_directories(${CAPNP_INCLUDE_DIRS})
link_directories(${CAPNP_LIBRARY_DIRS})

# Find capnp compiler
find_program(CAPNP_EXECUTABLE capnp)
find_program(CAPNPC_CXX_EXECUTABLE capnpc-c++)

if(NOT CAPNP_EXECUTABLE)
    message(FATAL_ERROR "Could not locate capnp executable")
endif()
if(NOT CAPNPC_CXX_EXECUTABLE)
    message(FATAL_ERROR "Could not locate capnpc-c++ executable")
endif()

# Generate Cap'n Proto C++ code
set(CAPNP_SCHEMA ${CMAKE_CURRENT_SOURCE_DIR}/eventProto.capnp)
set(CAPNP_SRCS ${CMAKE_CURRENT_BINARY_DIR}/eventProto.capnp.c++)
set(CAPNP_HDRS ${CMAKE_CURRENT_BINARY_DIR}/eventProto.capnp.h)

add_custom_command(
    OUTPUT ${CAPNP_SRCS} ${CAPNP_HDRS}
    COMMAND ${CAPNP_EXECUTABLE} compile
        -o${CAPNPC_CXX_EXECUTABLE}:${CMAKE_CURRENT_BINARY_DIR}
        --src-prefix=${CMAKE_CURRENT_SOURCE_DIR}
        ${CAPNP_SCHEMA}
    DEPENDS ${CAPNP_SCHEMA}
    COMMENT "Generating C++ code from Cap'n Proto schema"
)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Main converter executable
add_executable(cap2root
    src/main.cpp
    src/CapnpReader.cpp
    src/RootWriter.cpp
    ${CAPNP_SRCS}
)
target_link_libraries(cap2root
    ${ROOT_LIBRARIES}
    ${CAPNP_LIBRARIES}
)

# Cap'n Proto dump utility
add_executable(capdump
    src/capdump.cpp
    src/CapnpReader.cpp
    src/RootWriter.cpp
    ${CAPNP_SRCS}
)
target_link_libraries(capdump
    ${ROOT_LIBRARIES}
    ${CAPNP_LIBRARIES}
)

# Test file structure utility
add_executable(test_file_structure
    src/test_file_structure.cpp
    ${CAPNP_SRCS}
)
target_link_libraries(test_file_structure
    ${CAPNP_LIBRARIES}
)

# Test PlainData structure
add_executable(test_plain_data
    src/test_plain_data.cpp
    ${CAPNP_SRCS}
)
target_link_libraries(test_plain_data
    ${CAPNP_LIBRARIES}
)

# Tests
enable_testing()
add_executable(test_converter
    tests/test_main.cpp
    tests/test_reader.cpp
    tests/test_writer.cpp
    src/CapnpReader.cpp
    src/RootWriter.cpp
    ${CAPNP_SRCS}
)
target_link_libraries(test_converter
    ${ROOT_LIBRARIES}
    ${CAPNP_LIBRARIES}
)
add_test(NAME converter_tests COMMAND test_converter)

# Check positions utility
add_executable(check_positions
    src/check_positions.cpp
    ${CAPNP_SRCS}
)
target_link_libraries(check_positions
    ${CAPNP_LIBRARIES}
)

# Test correct format
add_executable(test_correct_format
    src/test_correct_format.cpp
    ${CAPNP_SRCS}
)
target_link_libraries(test_correct_format
    ${CAPNP_LIBRARIES}
)
