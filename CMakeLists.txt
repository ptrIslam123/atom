cmake_minimum_required(VERSION 3.18)

project(atom)

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)
option(BUILD_EXAMPLES "Build the examples" ON)
option(BUILD_TOOLS "Build the tools" ON)
option(BUILD_TESTS "Build the tests" ON)
option(ENABLE_TSAN "Build with TSAN" OFF)
option(ENABLE_ASAN "Build with ASAN" OFF)

find_package(GTest REQUIRED)

if(ENABLE_TSAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -g") # enable thread sanitize
endif()

if(ENABLE_ASAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g") # enable address sanitize
endif()

function(target_builder TARGET_NAME SRCS HDRS HDRS_DIR LIBS LIBS_DIR OUTPUT_DIR)
    add_executable(${TARGET_NAME} ${SRCS} ${HDRS})

    target_include_directories(${TARGET_NAME} PUBLIC ${HDRS_DIR} ${CURRENT_SOURCE_DIR} ${CURRENT_SOURCE_DIR}/include)

    target_link_directories(${TARGET_NAME} PUBLIC ${LIBS_DIR})

    target_link_libraries(${TARGET_NAME} PUBLIC ${LIBS})

    set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DIR}")
endfunction(target_builder)

set(CMAKE_CXX_STANDARD 20)

#******************************************************* Building engine library *******************************************************#
# Используем ${CMAKE_CURRENT_SOURCE_DIR} вместо ${CMAKE_SOURCE_DIR}
set(CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(LIB_NAME ${PROJECT_NAME}_core)
set(LIB_DIRS ${CURRENT_SOURCE_DIR} ${CURRENT_SOURCE_DIR}/include)
message(STATUS "CIENGINE_DIRS=${LIB_DIRS}")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

file(GLOB_RECURSE HDRS "${CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE SRCS "${CURRENT_SOURCE_DIR}/src/*.cpp")

set(LIB_HDRS 
${HDRS}
)

set(LIB_SRCS 
${SRCS}
)

message(STATUS "LIB_INSTALL_DIR=${CMAKE_BINARY_DIR}/lib")

if(BUILD_SHARED_LIBS)
    message(STATUS "BUILD_TYPE=DYNAMIC_LIBRARY")

    add_library(${LIB_NAME} SHARED ${LIB_HDRS} ${LIB_SRCS})
else()
    message(STATUS "BUILD_TYPE=STATIC_LIBRARY")

    add_library(${LIB_NAME} STATIC ${LIB_HDRS} ${LIB_SRCS})
endif()

target_include_directories(${LIB_NAME} PUBLIC ${LIB_DIRS})

set_target_properties(${LIB_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

#******************************************************* Build example dir *******************************************************#
if(BUILD_EXAMPLES)
    message(STATUS "BUILD_EXAMPLES=ON")

    target_builder("sync-usage" "examples/sync_usage.cpp" "" "" "${PROJECT_NAME}" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}" "examples")
    target_builder("rc-usage" "examples/rc_usage.cpp" "" "" "${PROJECT_NAME}" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}" "examples")
    target_builder("safe_mutex_usage" "examples/safe_mutex_usage.cpp" "" "" "${PROJECT_NAME}" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}" "examples")
endif()

#******************************************************* Build tools dir *******************************************************#
if(BUILD_TOOLS)
    message(STATUS "BUILD_TOOLS=ON")
endif()

if(BUILD_TESTS)
    message(STATUS "BUILD_TESTS=${CMAKE_BINARY_DIR}")

    file(GLOB_RECURSE TEST_SRC "${CURRENT_SOURCE_DIR}/test/units/*.cpp")
    set(TEST_LIBS "${GTEST_BOTH_LIBRARIES}" "pthread")
    set(TEST_INCLUDE_DIRS "${CURRENT_SOURCE_DIR} ${GTEST_INCLUDE_DIRS}")

    target_builder("unit_tests" "${TEST_SRC}" "" "${TEST_INCLUDE_DIRS}" "${TEST_LIBS};${PROJECT_NAME}" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}" "test")
endif()
