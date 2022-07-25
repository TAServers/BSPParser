cmake_minimum_required(VERSION 3.20)

project("BSPParser" CXX)
set(CMAKE_CXX_STANDARD 17)

add_library(
	${PROJECT_NAME}
	"BSPParser.cpp"
	"FileFormat/Parser.cpp"
)
