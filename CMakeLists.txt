cmake_minimum_required(VERSION 2.6)
project(periphery C)

option(BUILD_TESTS "Build periphery compile tests" ON)

# Check for Linux kernel headers
include(CheckIncludeFiles)
CHECK_INCLUDE_FILES(linux/gpio.h HAVE_LINUX_HEADERS)
if(HAVE_LINUX_HEADERS)
    set(PERIPHERY_GPIO_CDEV_SUPPORT 1)
else()
    message(WARNING "c-periphery will only support legacy Linux kernel GPIO interface. "linux/gpio.h" header file not detect")
endif()

# Library version
set(PERIPHERY_VERSION_MAJOR "2")
set(PERIPHERY_VERSION_MINOR "2")
set(PERIPHERY_VERSION_PATCH "1")
set(VERSION "${PERIPHERY_VERSION_MAJOR}.${PERIPHERY_VERSION_MINOR}.${PERIPHERY_VERSION_PATCH}")
set(SOVERSION ${VERSION})

# Expose git commit id into COMMIT_ID variable
execute_process(
    COMMAND git --git-dir="${CMAKE_CURRENT_SOURCE_DIR}" describe --abbrev --always --tags --dirty
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_VARIABLE COMMIT_ID
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)

# Set Periphery Version variable
if(COMMIT_ID)
    set(PERIPHERY_VERSION_COMMIT "${COMMIT_ID}")
else()
    set(PERIPHERY_VERSION_COMMIT "unknown")
endif()

# generate version file
configure_file(${CMAKE_SOURCE_DIR}/src/version.h.in ${CMAKE_SOURCE_DIR}/src/version.h)

# Glob sources, headers, tests
file(GLOB_RECURSE periphery_SOURCES src/*.c)
file(GLOB_RECURSE periphery_HEADERS src/*.h)

# Define C flags and include directories
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

# Build tests if enabled
#if(BUILD_TESTS)
    add_subdirectory(tests)
#endif()
