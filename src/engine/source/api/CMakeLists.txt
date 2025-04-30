add_subdirectory(adapter)
add_subdirectory(catalog)

add_library(api INTERFACE)

target_include_directories(api INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(api INTERFACE
    api::catalog
)
