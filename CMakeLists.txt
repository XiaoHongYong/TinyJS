cmake_minimum_required(VERSION 3.15...3.28)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

set(CMAKE_CXX_FLAGS "-std=c++17")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(UT "unit test" OFF)
if (UT)
    add_definitions(-DUNIT_TEST)
endif(UT)

if (APPLE)
    add_definitions(-D_MAC_OS)
elseif (LINUX)
    add_definitions(-D_LINUX)
endif (APPLE)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

PROJECT(TinyJS)

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/../third-parties/googletest/include
    ${PROJECT_SOURCE_DIR}/../third-parties/http-parser
    ${PROJECT_SOURCE_DIR}/../third-parties/rapidjson
    # ${PROJECT_SOURCE_DIR}/../third-parties/glog
    ${PROJECT_SOURCE_DIR}/../third-parties/googletest
)

set(SRC )

if (UT)
    set(SRC
        ${SRC}
        ${PROJECT_SOURCE_DIR}/../third-parties/googletest/src/gtest-all.cc
    )
endif(UT)

set(SRC_PATHS
    ${PROJECT_SOURCE_DIR}/utils
    ${PROJECT_SOURCE_DIR}/strings
    ${PROJECT_SOURCE_DIR}/parser
    ${PROJECT_SOURCE_DIR}/objects
    ${PROJECT_SOURCE_DIR}/generated
    ${PROJECT_SOURCE_DIR}/interpreter
    ${PROJECT_SOURCE_DIR}/api-web
    ${PROJECT_SOURCE_DIR}/api-built-in
)

if (UT)
    set(SRC_PATHS ${SRC_PATHS}
        ${PROJECT_SOURCE_DIR}/unittest/objects
        ${PROJECT_SOURCE_DIR}/unittest/parser
        ${PROJECT_SOURCE_DIR}/unittest/utils
    )
endif(UT)

set(HEADER_FILES)

foreach(_PATH ${SRC_PATHS})
    aux_source_directory(${_PATH} SRC)

    file(GLOB _HEADERS_H "${_PATH}/*.h")
    file(GLOB _HEADERS_HPP "${_PATH}/*.hpp")
    set(HEADER_FILES ${HEADER_FILES} ${_HEADERS_H} ${_HEADERS_HPP})
endforeach()

list(REMOVE_ITEM SRC "${PROJECT_SOURCE_DIR}/main.cpp") # 此文件需要被排除
# message(SRC="${SRC}")

set(LINK_OPT_FRAME_WORK)
set(LINK_OPT_EXTRA)

if (APPLE)
    aux_source_directory(${PROJECT_SOURCE_DIR}/utils/mac SRC)
    set(LINK_OPT_EXTRA "pthread -static-libstdc++ -framework CoreFoundation" "-framework Cocoa")
elseif (LINUX)
    aux_source_directory(${PROJECT_SOURCE_DIR}/utils/linux SRC)
    set(LINK_OPT_EXTRA "pthread -static-libstdc++ ")
else ()
    aux_source_directory(${PROJECT_SOURCE_DIR}/utils/win32 SRC)
endif (APPLE)

# add_subdirectory(${PROJECT_SOURCE_DIR}/../third-parties/glog glog)

# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(SRC_TINY_JS
    ${SRC}
    ${PROJECT_SOURCE_DIR}/main.cpp
)
add_executable(${PROJECT_NAME} ${SRC_TINY_JS} ${HEADER_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LINK_OPT_EXTRA})

add_library(${PROJECT_NAME}_STATIC ${SRC} ${HEADER_FILES})
target_link_libraries(${PROJECT_NAME}_STATIC PRIVATE ${LINK_OPT_EXTRA})
