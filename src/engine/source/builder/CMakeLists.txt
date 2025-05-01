set(IFACE_DIR ${CMAKE_CURRENT_LIST_DIR}/interface)
set(SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(INC_DIR ${CMAKE_CURRENT_LIST_DIR}/include)

## Interface
add_library(builder_ibuilder INTERFACE)
target_include_directories(builder_ibuilder INTERFACE ${IFACE_DIR})
target_link_libraries(builder_ibuilder INTERFACE base)
add_library(builder::ibuilder ALIAS builder_ibuilder)

if(ENGINE_BUILD_TEST)

set(TEST_MOCK_DIR ${CMAKE_CURRENT_LIST_DIR}/test/mocks)

## Mocks
add_library(builder_mocks INTERFACE)
target_include_directories(builder_mocks INTERFACE ${TEST_MOCK_DIR})
target_link_libraries(builder_mocks INTERFACE GTest::gmock builder::ibuilder)
add_library(builder::mocks ALIAS builder_mocks)

endif(ENGINE_BUILD_TEST)
