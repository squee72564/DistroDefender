set(SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(INC_DIR ${CMAKE_CURRENT_LIST_DIR}/include)

add_library(httpserver STATIC
    ${SRC_DIR}/server.cpp
)

target_include_directories(httpserver
    PUBLIC
    ${INC_DIR}

    PRIVATE
    ${SRC_DIR}
    ${INC_DIR}/httpserver
)

target_link_libraries(httpserver
    PUBLIC
    base
    httplib::httplib
)

if(ENGINE_BUILD_TEST)

set(TEST_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/test/src)
set(UNIT_SRC_DIR ${TEST_SRC_DIR}/unit)
#set(COMPONENT_SRC_DIR ${TEST_SRC_DIR}/component)

add_executable(httpserver_utest
    ${UNIT_SRC_DIR}/server_test.cpp
)

target_include_directories(httpserver_utest
    PRIVATE
    ${TEST_SRC_DIR}
)

target_link_libraries(httpserver_utest
    PRIVATE
    httpserver
    GTest::gtest_main
)

gtest_discover_tests(httpserver_utest)

#add_executable(httpserver_ctest
#    ${COMPONENT_SRC_DIR}/server_test.cpp
#    ${TEST_SRC_DIR}/generic_request.pb.cc
#)
#
#target_include_directories(httpserver_ctest
#    PRIVATE
#    ${TEST_SRC_DIR}
#)
#
#target_link_libraries(httpserver_ctest
#    PRIVATE
#    httpserver
#    GTest::gtest_main
#    base::test
#)
#
#gtest_discover_tests(httpserver_ctest)

endif()
