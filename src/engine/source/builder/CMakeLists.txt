set(IFACE_DIR ${CMAKE_CURRENT_LIST_DIR}/interface)
set(SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(INC_DIR ${CMAKE_CURRENT_LIST_DIR}/include)

## Interface
add_library(builder_ibuilder INTERFACE)
target_include_directories(builder_ibuilder INTERFACE ${IFACE_DIR})
target_link_libraries(builder_ibuilder INTERFACE base)
add_library(builder::ibuilder ALIAS builder_ibuilder)
