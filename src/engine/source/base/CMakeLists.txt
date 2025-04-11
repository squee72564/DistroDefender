########################
# BASE
########################

# Defs
set(SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(INC_DIR ${CMAKE_CURRENT_LIST_DIR}/include)


add_library(base STATIC
    ${SRC_DIR}/logger.cpp
)

target_include_directories(base
    PUBLIC
    ${INC_DIR}

    PRIVATE
    ${INC_DIR}
    ${SRC_DIR}
)

target_link_libraries(base
    PUBLIC
    spdlog::spdlog
)

# Tests

if(ENGINE_BUILD_TEST)

    set(TEST_DIR ${CMAKE_CURRENT_LIST_DIR}/test)
    set(TEST_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/test/src)
    set(UNIT_SRC_DIR ${TEST_SRC_DIR}/unit)

    add_library(base_test INTERFACE)
    target_include_directories(base_test INTERFACE ${TEST_DIR}/include)
    target_link_libraries(base_test INTERFACE base GTest::gmock)
    add_library(base::test ALIAS base_test)

    add_executable(base_utest
        ${UNIT_SRC_DIR}/error_test.cpp
    )

    target_include_directories(base_utest
        PRIVATE
        ${TEST_SRC_DIR}
        ${SRC_DIR}
        ${INC_DIR}/base
    )

    target_link_libraries(base_utest
        PUBLIC
        base::test

        PRIVATE
        base
        GTest::gtest_main
    )

    gtest_discover_tests(base_utest)

    # This needs to be instantiated along because the logger is
    # instantiated on tests

    add_executable(base_logger_utest
        ${UNIT_SRC_DIR}/logger_test.cpp
    )

    target_include_directories(base_logger_utest
        PRIVATE
        ${UNIT_SRC_DIR}
        ${INC_DIR}/base
    )

    target_link_libraries(base_logger_utest
        PRIVATE
        base
        GTest::gtest_main
    )

    gtest_discover_tests(base_logger_utest)

    endif(ENGINE_BUILD_TEST)

