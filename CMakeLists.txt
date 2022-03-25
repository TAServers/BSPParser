
cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)

project("BSPParser")
add_executable(
	${PROJECT_NAME}
	"BSPParser.cpp"
	"FileFormat/Parser.cpp"
)
