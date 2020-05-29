cmake_minimum_required(VERSION 2.6)
project(periphery C)

# Check for Linux kernel headers
include(CheckIncludeFiles)
CHECK_INCLUDE_FILES(linux/gpio.h HAVE_LINUX_HEADERS)
if(NOT HAVE_LINUX_HEADERS)
    message(FATAL_ERROR "c-periphery needs Linux kernel headers")
endif()

# Library version
set(VERSION "2.2.0")
set(SOVERSION ${VERSION})

# Glob sources, headers, tests
file(GLOB_RECURSE periphery_SOURCES src/*.c)
file(GLOB_RECURSE periphery_HEADERS src/*.h)
file(GLOB_RECURSE periphery_TESTS tests/*.c)

# Expose git commit id into COMMIT_ID variable
execute_process(
    COMMAND git describe --abbrev --always --tags --dirty
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_VARIABLE COMMIT_ID
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)

# Define C flags and include directories
add_definitions(-DPERIPHERY_VERSION_COMMIT="${COMMIT_ID}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -pedantic -Wall -Wextra -Wno-unused-parameter -fPIC")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Declare library target
add_library(periphery ${periphery_SOURCES} ${periphery_HEADERS})
set_target_properties(periphery PROPERTIES SOVERSION ${VERSION})

include(GNUInstallDirs)

# Generate pkg-config pc file
configure_file(${CMAKE_SOURCE_DIR}/src/libperiphery.pc.in ${CMAKE_BINARY_DIR}/libperiphery.pc @ONLY)

# Declare install targets
install(TARGETS periphery DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${periphery_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})
install(FILES ${CMAKE_BINARY_DIR}/libperiphery.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

# Declare tests targets
foreach(TEST_SOURCE ${periphery_TESTS})
    get_filename_component(TEST_PROGRAM ${TEST_SOURCE} NAME_WE)
    add_executable(${TEST_PROGRAM} ${TEST_SOURCE})
    target_link_libraries(${TEST_PROGRAM} periphery pthread)
    set(TEST_PROGRAMS ${TEST_PROGRAMS} ${TEST_PROGRAM})
endforeach()
add_custom_target(tests DEPENDS periphery ${TEST_PROGRAMS})
