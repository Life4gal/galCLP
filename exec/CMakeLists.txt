cmake_minimum_required(VERSION 3.12...3.17)

project(
		galCLPTest
		LANGUAGES CXX
)

set(
		test-source
		test/test.cpp
)

add_executable(
		${PROJECT_NAME}
		${test-source}
		main.cpp
)

set_target_properties(
		${PROJECT_NAME}
		PROPERTIES CXX_STANDARD 17
		OUTPUT_NAME "${PROJECT_NAME}"
)

include(../cmake/CPM.cmake)

include(cmake/google-test.cmake)
target_link_libraries(
		${PROJECT_NAME}
		PRIVATE
		gtest_main
)

CPMAddPackage(
		NAME galCLP
		SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..
)

target_link_libraries(
		${PROJECT_NAME}
		PRIVATE
		galCLP::galCLP
)
