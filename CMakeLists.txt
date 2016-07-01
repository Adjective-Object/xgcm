cmake_minimum_required(VERSION 2.8.4)
project(xgcm)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    example/sample
    example/sample.xgcm
    example/sample_xgcmrc
    example/sample_xgcmrc_2
    ini/ini.c
    ini/ini.h
    Makefile
    README.md
    README.md.xgcm
    simple_hmap.c
    simple_hmap.h
    simple_ll.c
    simple_ll.h
    string_buffer.c
    string_buffer.h
    utils.c
    utils.h
    xgcm.c
    xgcm_conf.c
    xgcm_conf.h
    xgcm_parser.c
    xgcm_parser.h
    xgcm_traversal.c
    xgcm_traversal.h)

add_executable(xgcm ${SOURCE_FILES})