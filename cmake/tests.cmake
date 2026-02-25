# Unit tests building

if(VIT_BUILD_TESTING)
    set(BUILD_TESTING ON)
    enable_testing()
endif(VIT_BUILD_TESTING)

function(build_tests TARGET)
    if(NOT VIT_BUILD_TESTING)
        return()
    endif(NOT VIT_BUILD_TESTING)

    message(STATUS "Building unit tests for ${PROJECT_NAME}")
    set(TEST_NAME ${TARGET}_Test)
    add_executable(${TEST_NAME} ${ARGN})
    target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests/)
    target_link_libraries(${TEST_NAME} PRIVATE Catch2::Catch2WithMain ${TARGET})
    target_disable_rtti(${TEST_NAME})
    add_dependencies(tests ${TEST_NAME})

    add_custom_command(
        TARGET ${TEST_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:${TEST_NAME}> $<TARGET_RUNTIME_DLLS:${TEST_NAME}>
        COMMAND_EXPAND_LISTS
    )

    if(DEFINED ENV{CI})
        catch_discover_tests(${TEST_NAME} TEST_SPEC "~[no_ci]")
    else()
        catch_discover_tests(${TEST_NAME})
    endif()
endfunction()
