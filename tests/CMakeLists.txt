file(GLOB_RECURSE periphery_TESTS *.c)

# Declare tests targets
foreach(TEST_SOURCE ${periphery_TESTS})
    if(NOT (TEST_SOURCE MATCHES ".*test_gpio.*") AND (NOT PERIPHERY_GPIO_CDEV_SUPPORT))
        get_filename_component(TEST_PROGRAM ${TEST_SOURCE} NAME_WE)
        add_executable(${TEST_PROGRAM} ${TEST_SOURCE})
        target_link_libraries(${TEST_PROGRAM} periphery pthread)
        set(TEST_PROGRAMS ${TEST_PROGRAMS} ${TEST_PROGRAM})
    endif()
endforeach()
add_custom_target(tests DEPENDS periphery ${TEST_PROGRAMS})
