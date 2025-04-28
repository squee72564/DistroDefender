set(SCHEMAS_INC_DIR ${ENGINE_SOURCE_DIR}/schemas/include)

add_library(schemas INTERFACE)

target_include_directories(schemas
    INTERFACE
    ${SCHEMAS_INC_DIR}
)
