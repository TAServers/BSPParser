cmake_minimum_required(VERSION 3.20)

project("BSPParser" CXX)
set(CMAKE_CXX_STANDARD 20)

add_library(BSPParser
        BSPParser.hpp
        src/bsp.cpp
        src/bsp.hpp
        src/helpers/check-bounds.hpp
        src/helpers/offset-data-view.cpp
        src/helpers/offset-data-view.hpp
)

target_include_directories(
        BSPParser PRIVATE
        "src"
)
