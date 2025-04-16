
cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)

# Set C++ 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

project(
    distro-defender
    VERSION 0.1
    LANGUAGES CXX
)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-Wall -Werror -O3)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/W4 /WX /0x)
endif()

set(ENGINE_BIN_DIR ${PROJECT_SOURCE_DIR}/bin)
set(ENGINE_SOURCE_DIR ${PROJECT_SOURCE_DIR}/source)
#set(ENGINE_BENCHMARK_DIR ${PROJECT_SOURCE_DIR}/benchmark)
#set(ENGINE_DOC_DIR ${PROJECT_SOURCE_DIR}/docs)

# Options
option(ENGINE_BUILD_TEST "Generate tests" ON)
#option(ENGINE_BUILD_BENCHMARK "Generate benchmarks" ON)
#option(ENGINE_BUILD_DOCUMENTATION "Generate doxygen documentation" ON)

if(DEFINED VCPKG_TARGET_TRIPLET)
    message(STATUS "Using VCPKG triplet: ${VCPKG_TARGET_TRIPLET}")
else()
    message(
        WARNING
        "VCPKG_TARGET_TRIPLET is not defined. Make sure you are using the vcpkg toolchain."
    )

    message(FATAL_ERROR "VCPKG_TARGET_TRIPLET is not defined.")
endif()

############################################
# Dependencies
############################################

function(action package_name target_name)
    if (${package_name}_FOUND)
        if (NOT TARGET ${target_name})
            add_library(${target_name} INTERFACE IMPORTED)
            set_target_properties(${target_name} PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${${package_name}_INCLUDE_DIRS}"
            )
            message(STATUS "Created imported target ${target_name}")
        else()
            message(STATUS "Target ${target_name} already exists")
        endif()
    else()
        message(FATAL_ERROR "${package_name} not found!")
    endif()
endfunction()

function(find_and_create_imported_target package_name target_name)
    find_package(${package_name} CONFIG REQUIRED)
    action(${package_name} ${target_name})
endfunction()

function(find_and_create_imported_target_ex package_name target_name)
    find_package(${package_name} REQUIRED)
    action(${package_name} ${target_name})
endfunction()

# Build Test
if(ENGINE_BUILD_TEST)
    include(CTest)
    find_and_create_imported_target("GTest" "GTest::gtest")
    find_and_create_imported_target("GTest" "GTest::gtest_main")
    find_and_create_imported_target("GTest" "GTest::gmock")
    find_and_create_imported_target("GTest" "GTest::gmock_main")
    include(GoogleTest)
endif(ENGINE_BUILD_TEST)

find_and_create_imported_target("spdlog" "spdlog::spdlog")
find_and_create_imported_target("fmt" "fmt::fmt-header-only")
find_and_create_imported_target("httplib" "httplib::htttplib")

############################################
# TARGETS
############################################

add_executable(main ${ENGINE_SOURCE_DIR}/main.cpp)

add_subdirectory(${ENGINE_SOURCE_DIR}/base)
add_subdirectory(${ENGINE_SOURCE_DIR}/httpserver)

target_link_libraries(main
    base
    httpserver
)

# Build Benchmark
#if (ENGINE_BUILD_BENCHMARK)
#    add_subdirectory(${ENGINE_BENCHMARK_DIR})
#endif(ENGINE_BUILD_BENCHMARK)

# Create Custom Test Target
function(get_all_targets _result _dir)
    get_property(_subdirs DIRECTORY "${_dir}" PROPERTY SUBDIRECTORIES)
    foreach(_subdir IN LISTS _subdirs)
        get_all_targets(${_result} "${_subdir}")
    endforeach()

    get_directory_property(_sub_targets DIRECTORY "${_dir}" BUILDSYSTEM_TARGETS)
    set(${_result} ${${_result}} ${_sub_targets} PARENT_SCOPE)
endfunction()

# Create an empty list to store all targets
set(ALL_TARGETS)
set(test_targets_to_build)

get_all_targets(ALL_TARGETS ${CMAKE_CURRENT_SOURCE_DIR})

foreach(target ${ALL_TARGETS})
    # Print all targets
    # Test end with _test _ctest or _utest
    if(${target} MATCHES "_test$" OR ${target} MATCHES "_ctest$" OR ${target} MATCHES "_utest$")
        list(APPEND test_targets_to_build ${target})
        message(STATUS "Test Target: ${target}")
    endif()
endforeach()

add_custom_target(all_tests DEPENDS ${test_targets_to_build})

# Generate Docs
# TODO

# Custom clean targets
add_custom_target(clean_insource
    COMMAND rm -f "${CMAKE_SORUCE_DIR}/CMakeCache.txt"
    COMMAND rm -rf "${CMAKE_SORUCE_DIR}/CMakeFiles"
    COMMAND rm -f "${CMAKE_SORUCE_DIR}/cmake_install.cmake"
    COMMAND rm -f "${CMAKE_SORUCE_DIR}/Makefile"
    ERROR_QUIET )

add_custom_target( clean_build
    COMMAND rm -rf "${CMAKE_SOURCE_DIR}/build/*"
    ERROR QUIET )
