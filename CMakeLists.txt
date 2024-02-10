cmake_minimum_required(VERSION 3.28.1)

project(Def13)

add_executable(${PROJECT_NAME}
	main.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES 
	RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_SOURCE_DIR}>
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED YES
)

set(CMAKE_CXX_FLAGS "-O3 -Wall -Wextra")

file(MAKE_DIRECTORY data/auth)
