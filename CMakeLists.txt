cmake_minimum_required(VERSION 2.8.4)
project(xgcm)
include(FindPkgConfig)
find_package(PkgConfig REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(LIB_SRCS
    ini.c
    ini.h
    README.md
    README.md.xgcm
    simple_ll.c
    simple_ll.h
    string_buffer.c
    string_buffer.h
    utils.c
    utils.h
    xgcm_conf.c
    xgcm_conf.h
    xgcm_lua.c
    xgcm_lua.h
    xgcm_parser.c
    xgcm_parser.h
    xgcm_traversal.c
    xgcm_traversal.h)
add_library(XGCM_LIB OBJECT ${LIB_SRCS})

add_executable(xgcm xgcm.c $<TARGET_OBJECTS:XGCM_LIB>)

pkg_search_module(LUA REQUIRED lua)

target_link_libraries(xgcm ${LUA_LIBRARIES})
include_directories(xgcm PUBLIC ${LUA_INCLUDE_DIRS})
target_compile_options(xgcm PUBLIC ${LUA_CFLAGS_OTHER})

