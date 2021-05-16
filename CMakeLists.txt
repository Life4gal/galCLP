cmake_minimum_required(VERSION 3.13...3.20)

project(
		galCLP
		VERSION 0.0.1
		LANGUAGES CXX
)

set(
		galCLPSource
		include/command_line_parser.hpp
)

add_library(
		${PROJECT_NAME} INTERFACE
)

target_include_directories(
		${PROJECT_NAME} INTERFACE
		${galCLPSource}
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
)

target_compile_features(
		${PROJECT_NAME} INTERFACE
		cxx_std_17
)

include(cmake/CPM.cmake)
string(TOLOWER ${PROJECT_NAME}/version.h VERSION_HEADER_LOCATION)
CPMAddPackage("gh:TheLartians/PackageProject.cmake#master")

packageProject(
		NAME ${PROJECT_NAME}
		VERSION ${PROJECT_VERSION}
		NAMESPACE ${PROJECT_NAME}
		BINARY_DIR ${PROJECT_BINARY_DIR}
		INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include
		INCLUDE_DESTINATION include/${PROJECT_NAME}-${PROJECT_VERSION}
		VERSION_HEADER "${VERSION_HEADER_LOCATION}"
		DEPENDENCIES "${PROJECT_NAME} "
)
