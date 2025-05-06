set(SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(INC_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(IFACE_DIR ${CMAKE_CURRENT_LIST_DIR}/interface)

add_library(archiver_iface INTERFACE)
target_include_directories(archiver_iface INTERFACE ${IFACE_DIR})
target_link_libraries(archiver_iface INTERFACE base)
add_library(archiver::iface ALIAS archiver_iface)

add_library(archiver STATIC ${SRC_DIR}/archiver.cpp)
target_include_directories(archiver PUBLIC ${INC_DIR})
target_link_libraries(archiver PUBLIC archiver_iface)

if(ENGINE_BUILD_TEST)

set(TEST_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/test/src)
set(TEST_MOCK_DIR ${CMAKE_CURRENT_LIST_DIR}/test/mocks)
set(UNIT_SRC_DIR ${TEST_SRC_DIR}/unit)

add_library(archiver_mocks INTERFACE)
target_include_directories(archiver_mocks INTERFACE ${TEST_MOCK_DIR})
target_link_libraries(archiver_mocks INTERFACE archiver::iface)
add_library(archiver::mocks ALIAS archiver_mocks)

add_executable(archiver_utest
    ${UNIT_SRC_DIR}/archiver_test.cpp
)

target_link_libraries(archiver_utest
    PRIVATE
    archiver
    GTest::gtest_main
    archiver::mocks
)

gtest_discover_tests(archiver_utest)

endif(ENGINE_BUILD_TEST)
