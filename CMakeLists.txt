cmake_minimum_required(VERSION 3.20)

project("BSPParser" CXX)
set(CMAKE_CXX_STANDARD 17)

add_library(
	${PROJECT_NAME}
	"BSPParser.cpp"

	"FileFormat/Parser.cpp"
	"FileFormat/Vector.cpp"

	"Displacements/Positions.cpp"
	"Displacements/TBNGen.cpp"
	"Displacements/NormalBlending.cpp"
	"Displacements/SubEdgeIterator.cpp"
)

target_include_directories(
	${PROJECT_NAME} PRIVATE

	${PROJECT_SOURCE_DIR}
)
