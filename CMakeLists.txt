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
        src/structs/common.hpp
        src/enums/surface.hpp
        src/face-accessors.hpp
        src/face-accessors.cpp
        src/enums/lump.hpp
        src/enums/props.hpp
        src/structs/headers.hpp
        src/structs/geometry.hpp
        src/structs/brushes.hpp
        src/structs/textures.hpp
        src/structs/displacements.hpp
        src/structs/detail-props.hpp
        src/structs/static-props.hpp
        src/structs/models.hpp
        src/displacement-accessors.hpp
        src/displacement-accessors.cpp
        src/prop-accessors.hpp
        src/prop-accessors.cpp
)

target_include_directories(
        BSPParser PRIVATE
        "src"
)
