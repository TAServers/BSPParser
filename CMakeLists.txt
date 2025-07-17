cmake_minimum_required(VERSION 3.20)

project("BSPParser" CXX)
set(CMAKE_CXX_STANDARD 20)

add_library(BSPParser
        BSPParser.hpp
        src/errors.hpp
        src/bsp.cpp
        src/bsp.hpp
        src/helpers/check-bounds.hpp
        src/helpers/offset-data-view.cpp
        src/helpers/offset-data-view.hpp
        src/structs/common.hpp
        src/enums/surface.hpp
        src/accessors/face-accessors.hpp
        src/accessors/face-accessors.cpp
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
        src/accessors/prop-accessors.hpp
        src/accessors/prop-accessors.cpp
        src/accessors/texture-accessors.hpp
        src/accessors/texture-accessors.cpp
        src/helpers/vector-maths.hpp
        src/structs/physics.hpp
        src/structs/zip.hpp
        src/helpers/zip.hpp
        src/helpers/zip.cpp
        src/enums/zip.hpp
        src/accessors/vertex.hpp
        src/accessors/face-triangulation.hpp
        src/accessors/face-triangulation.cpp
        src/accessors/displacements/indices.hpp
        src/accessors/displacements/vertices.hpp
        src/accessors/displacements/indices.cpp
        src/accessors/displacements/vertices.cpp
)
