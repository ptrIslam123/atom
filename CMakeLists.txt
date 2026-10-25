cmake_minimum_required(VERSION 3.24)

project(atom
    VERSION 0.1.0
    DESCRIPTION "System library"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTS    "Build unit tests"                 ON)
option(BUILD_EXAMPLES "Build examples"                   ON)
option(SANITIZE       "Enable AddressSanitizer and UBSan" OFF)
option(COVERAGE       "Enable gcov coverage instrumentation" OFF)
option(WERROR         "Treat warnings as errors"         OFF)
option(BUILD_DOCS     "Build API documentation (needs Doxygen)" OFF)
option(FORMAT_TARGETS "Add clang-format targets (needs clang-format)" ON)

# Default to a release build in single-config generators.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        Debug Release RelWithDebInfo MinSizeRel)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Atom has no external runtime dependencies; GoogleTest is fetched only for tests.
find_package(Threads REQUIRED)

if(BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
        URL_HASH SHA256=f179ec217f9b3b3f3c6e8b02d3e7eda997b49e4ce26d6b235c9053bec9c0bf9f
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

file(GLOB_RECURSE ATOM_SRC  CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE ATOM_HDRS CONFIGURE_DEPENDS "include/*.h")

add_library(${PROJECT_NAME} ${ATOM_SRC} ${ATOM_HDRS})
add_library("${PROJECT_NAME}::${PROJECT_NAME}" ALIAS ${PROJECT_NAME})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads)

target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    $<$<BOOL:${WERROR}>:-Werror>
)

if(SANITIZE)
    target_compile_options(${PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:-fsanitize=address,undefined>
        $<BUILD_INTERFACE:-fno-omit-frame-pointer>
    )
    target_link_options(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:-fsanitize=address,undefined>)
endif()

# gcov instrumentation for the library only. Consumers such as unit-tests link
# the gcov runtime, but their own sources are not instrumented: test/example
# coverage is excluded from the report. BUILD_INTERFACE keeps the flags out of
# the installed target.
if(COVERAGE)
    target_compile_options(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:--coverage>)
    target_link_options(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:--coverage>)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

install(TARGETS ${PROJECT_NAME}
    EXPORT "${PROJECT_NAME}Targets"
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(FILES
    LICENSE
    README.md
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
)

install(EXPORT "${PROJECT_NAME}Targets"
    FILE "${PROJECT_NAME}Targets.cmake"
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/Config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/ConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake
)

if(BUILD_TESTS)
    enable_testing()

    file(GLOB_RECURSE UNIT_TEST_SRC CONFIGURE_DEPENDS "tests/unit/*.cpp")

    add_executable(unit-tests ${UNIT_TEST_SRC})
    target_link_libraries(unit-tests PRIVATE "${PROJECT_NAME}::${PROJECT_NAME}" GTest::gtest_main)
    set_target_properties(unit-tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
    )

    include(GoogleTest)
    gtest_discover_tests(unit-tests)
endif()
