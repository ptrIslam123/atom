cmake_minimum_required(VERSION 3.18)

project(ciengine)

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)
option(BUILD_EXAMPLES "Build the examples" ON)
option(BUILD_TOOLS "Build the tools" ON)
option(BUILD_TESTS "Build the tests" ON)

find_package(GTest REQUIRED)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -g") # enable thread sanitize

function(target_builder TARGET_NAME SRCS HDRS HDRS_DIR LIBS LIBS_DIR OUTPUT_DIR)
    add_executable(${TARGET_NAME} ${SRCS} ${HDRS})

    target_link_libraries(${TARGET_NAME} PUBLIC ${LIBS})

    target_include_directories(${TARGET_NAME} PUBLIC ${HDRS_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include)

    target_link_directories(${TARGET_NAME} PUBLIC ${LIBS_DIR})

    set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DIR}")
endfunction(target_builder)

set(CMAKE_CXX_STANDARD 20)

#******************************************************* Building engine library *******************************************************#
set(LIB_NAME ${PROJECT_NAME}_core)
set(LIB_DIRS ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include)
message(STATUS "CIENGINE_DIRS=${LIB_DIRS}")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

set(LIB_HDRS 

)

set(LIB_SRCS 

)

message(STATUS "LIB_INSTALL_DIR=${CMAKE_BINARY_DIR}/lib")

if(BUILD_SHARED_LIBS)
    message(STATUS "BUILD_TYPE=DYNAMIC_LIBRARY")

    #add_library(${LIB_NAME} SHARED ${LIB_HDRS} ${LIB_SRCS})
else()
    message(STATUS "BUILD_TYPE=STATIC_LIBRARY")

    #add_library(${LIB_NAME} STATIC ${LIB_HDRS} ${LIB_SRCS})
endif()

#target_include_directories(${LIB_NAME} PUBLIC ${LIB_DIRS})

#set_target_properties(${LIB_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})


#******************************************************* Build example dir *******************************************************#
if(BUILD_EXAMPLES)
    message(STATUS "BUILD_EXAMPLES=ON")

    target_builder("sync-usage" "examples/sync_usage.cpp" "" "" "" "" "examples")
    target_builder("rc-usage" "examples/rc_usage.cpp" "" "" "" "" "examples")
endif()

#******************************************************* Build tools dir *******************************************************#
if(BUILD_TOOLS)
    message(STATUS "BUILD_TOOLS=ON")
endif()

if(BUILD_TESTS)
    message(STATUS "BUILD_TESTS=${CMAKE_BINARY_DIR}")

    file(GLOB_RECURSE TEST_SRC "${CMAKE_SOURCE_DIR}/test/units/*.cpp")
    set(TEST_LIBS "${GTEST_BOTH_LIBRARIES}" "pthread")
    set(TEST_INCLUDE_DIRS "${CMAKE_SOURCE_DIR} ${GTEST_INCLUDE_DIRS}")

    message(STATUS "test_src=${TEST_SRC}")

    target_builder("unit_tests" "${TEST_SRC}" "" "${TEST_INCLUDE_DIRS}" "${TEST_LIBS}" "" "test")

endif()
