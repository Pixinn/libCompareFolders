cmake_minimum_required(VERSION 3.4 FATAL_ERROR)

### FORCE THE RUNTIME ON MSVC
if(MSVC)
  set(CMAKE_USER_MAKE_RULES_OVERRIDE
     ${CMAKE_CURRENT_SOURCE_DIR}/c_flag_overrides.cmake)
  set(CMAKE_USER_MAKE_RULES_OVERRIDE_CXX
     ${CMAKE_CURRENT_SOURCE_DIR}/cxx_flag_overrides.cmake)
endif()

project(compare_folder)

### FIND THE LIBRARIES
set(BOOST_ROOT $ENV{BOOST_DIR})
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME ON)
find_package(Boost COMPONENTS filesystem REQUIRED)
if(NOT Boost_FOUND)
	message(FATAL_ERROR "\nBoost is mandatory.\nTry to export the root directory of your Boost installation as \"BOOST_DIR\".")
endif()
set(CRYTOPP_LIB ${CMAKE_SOURCE_DIR}/submodules/cryptopp/libcryptopp.a)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

### COMPILER OPTIONS

if(MSVC)
	set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
	set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_DEBUG})
else()
  if( APPLE )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++14 -stdlib=libc++" )
    set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++" )
    set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -stdlib=libc++" )
    set( CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -stdlib=libc++" )
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -march=native")
  endif( APPLE )
endif()

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/submodules)

### SOURCES

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
add_executable(${PROJECT_NAME}  ${SRC_DIR}/main.cpp
                                ${SRC_DIR}/CCollectionHash.hpp
                                ${SRC_DIR}/CCollectionHash.cpp
                                ${SRC_DIR}/CFactoryHashes.hpp
                                ${SRC_DIR}/CFactoryHashes.cpp
                                ${SRC_DIR}/Exceptions.hpp)
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${CRYTOPP_LIB})
