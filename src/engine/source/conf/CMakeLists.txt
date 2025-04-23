set(SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(INC_DIR ${CMAKE_CURRENT_LIST_DIR}/include)

add_library(conf STATIC
    ${SRC_DIR}/conf.cpp
    ${SRC_DIR}/confLoader.cpp
)

target_include_directories(conf
    PUBLIC
    ${INC_DIR}
)

target_link_libraries(conf
    PUBLIC
    base
)

## TESTS

if(ENGINE_BUILD_TEST)

set(TEST_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/test/src)
set(UNIT_SRC_DIR ${TEST_SRC_DIR}/unit)
set(COMPONENT_SRC_DIR ${TEST_SRC_DIR}/component)

#add_executable(conf_utest
#    ${UNIT_SRC_DIR}/conf_test.cpp
#    ${UNIT_SRC_DIR}/unitconf_test.cpp
#)
#target_link_libraries(conf_utest
#    GTest::gtest_main
#    GTest::gmock
#    conf
#)
#
#gtest_discover_tests(conf_utest)
#
add_executable(conf_ctest
    ${COMPONENT_SRC_DIR}/confLoader_test.cpp
)

target_link_libraries(conf_ctest
    PRIVATE
    conf
    GTest::gtest_main
    httpserver
)

gtest_discover_tests(conf_ctest)

endif(ENGINE_BUILD_TEST)
