cmake_minimum_required(VERSION 3.4 FATAL_ERROR)

set(BOOST_ROOT $ENV{BOOST_DIR})
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME ON)
find_package(Boost)
if(NOT Boost_FOUND)
	message(FATAL_ERROR "Boost is mandatory")
endif()

project(compare_folder)

if(MSVC)
	set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG} "/MTd")
	set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_DEBUG} "/MT")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
add_executable(${PROJECT_NAME} ${SRC_DIR}/main.cpp)
