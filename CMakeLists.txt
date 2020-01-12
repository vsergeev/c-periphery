cmake_minimum_required(VERSION 2.6)
project(c-periphery C)

include(CheckIncludeFiles)

CHECK_INCLUDE_FILES(linux/gpio.h HAVE_LINUX_HEADERS)
if(NOT HAVE_LINUX_HEADERS)
    message("c-periphery needs kernel headers")
    return()
endif()


set(VERSION "2.1.0")
set(SOVERSION ${VERSION})

# Define COMMIT_ID
execute_process(
    COMMAND git describe --abbrev --always --tags --dirty
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_VARIABLE COMMIT_ID
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)

add_definitions(-DPERIPHERY_VERSION_COMMIT="${COMMIT_ID}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -pedantic -Wall -Wextra -Wno-unused-parameter -fPIC")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

file(
    GLOB_RECURSE
    c_pheriphery_SOURCES
    src/*.c
)

file(
    GLOB_RECURSE
    c_pheriphery_HEADERS
    src/*.h
)

# Build shared library
add_library(periphery ${c_pheriphery_SOURCES} ${c_pheriphery_HEADERS})
set_target_properties(periphery PROPERTIES SOVERSION ${VERSION})

# Install shared library and headers
install(TARGETS periphery DESTINATION lib)
install(FILES ${c_pheriphery_HEADERS} DESTINATION include/${PROJECT_NAME})

# Build all tests contains in tests directory
file(
    GLOB_RECURSE
    c_pheriphery_TESTS
    tests/*.c
)

foreach(TEST_FILE ${c_pheriphery_TESTS})
    get_filename_component(PROGRAM ${TEST_FILE} NAME_WE)
    add_executable(${PROGRAM} ${TEST_FILE})
    target_link_libraries(${PROGRAM} periphery pthread)
    set(PROGRAMS ${PROGRAMS} ${PROGRAM})
endforeach()

add_custom_target(tests DEPENDS periphery ${PROGRAM_TESTS}
COMMENT "Generating Test c-periphery API"
VERBATIM)
