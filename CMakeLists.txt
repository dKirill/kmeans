cmake_minimum_required(VERSION 3.7)
project(kmeans CXX)

# set flags
set(CMAKE_CXX_FLAGS "-std=c++1z -Werror -Wall -Wextra -Wpedantic -Wcast-align -Wcast-qual -Wconversion -Wctor-dtor-privacy -Wenum-compare -Wfloat-equal -Wnon-virtual-dtor -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wsign-conversion -Wsign-promo -Wno-unused-function")

# create interface lib
set(KMEANS_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/kmeans)
set(HEADERS 
	${KMEANS_INCLUDE_DIR}/kmeans.hpp 
	${KMEANS_INCLUDE_DIR}/ctpl_stl.h)
add_library(kmeans INTERFACE)
target_sources(kmeans INTERFACE ${HEADERS})
source_group("Public headers" FILES ${HEADERS})
target_include_directories(kmeans INTERFACE ${KMEANS_INCLUDE_DIR})

# Prepare "Catch" unit test library for other executables
set(CATCH_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/extern/Catch2/single_include)
add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})

# Add test executable
set(TEST_SOURCES Test.cpp)
add_executable(tests ${TEST_SOURCES})
target_link_libraries(tests PRIVATE Catch kmeans)

# Run test automatically as post build step
add_custom_command(
	TARGET tests
	COMMENT "Run kmeans tests"
	POST_BUILD 
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	COMMAND tests
)

# install
install(DIRECTORY ${KMEANS_INCLUDE_DIR}
	PUBLIC_HEADER DESTINATION include
)
