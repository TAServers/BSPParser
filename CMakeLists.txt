cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 17)

project("BSPParser")
add_library(
	${PROJECT_NAME}
	"BSPParser.cpp"
	"FileFormat/Parser.cpp"
)
