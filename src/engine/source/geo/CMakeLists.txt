set(SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(INC_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(IFACE_DIR ${CMAKE_CURRENT_LIST_DIR}/interface)

add_library(geo_igeo INTERFACE)
target_include_directories(geo_igeo INTERFACE ${IFACE_DIR})
target_link_libraries(geo_igeo INTERFACE base)
add_library(geo::igeo ALIAS geo_igeo)

set(SRCS
    ${SRC_DIR}/manager.cpp
    ${SRC_DIR}/downloader.cpp
    ${SRC_DIR}/locator.cpp
)

set(PRIVATE_LINKS
    cpr::cpr
    maxminddb::maxminddb
)

set(PUBLIC_LINKS
    geo::igeo
    store::istore
)

# Patch the maxminddb target
set_target_properties(maxminddb::maxminddb
  PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ""
)

add_library(geo
      ${SRCS}
)
target_include_directories(geo
    PUBLIC
    ${INC_DIR}

    PRIVATE
    ${SRC_DIR}
    ${INC_DIR}/geo
)
target_link_libraries(geo
    PUBLIC
    ${PUBLIC_LINKS}

    PRIVATE
    ${PRIVATE_LINKS}
)
