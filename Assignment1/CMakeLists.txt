cmake_minimum_required(VERSION 3.3)
project(Assignment1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    include/Angel.h
    include/CheckError.h
    include/InitShader.cpp
    include/mat.h
    include/vec.h
    FruitTetris.cpp
    fshader.glsl
    Makefile
    vshader.glsl)

add_executable(Assignment1 ${SOURCE_FILES})