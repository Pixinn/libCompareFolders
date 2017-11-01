# read :    https://de.slideshare.net/DanielPfeifer1/cmake-48475415
#           https://rix0r.nl/blog/2015/08/13/cmake-guide/

cmake_minimum_required(VERSION 3.4 FATAL_ERROR)


project(compare_folders VERSION 0.1 LANGUAGES CXX)

### FIND THE LIBRARIES

# boost
set(BOOST_ROOT $ENV{BOOST_DIR})
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME ON)
find_package(Boost COMPONENTS filesystem REQUIRED)
if(NOT Boost_FOUND)
	message(FATAL_ERROR "\nBoost is mandatory.\nTry to export the root directory of your Boost installation as \"BOOST_DIR\".")
endif()
# cryptopp
set(CRYTOPP_LIB_DIR ${CMAKE_SOURCE_DIR}/submodules/cryptopp)


### SOURCES

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin) 
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
add_executable(${PROJECT_NAME}  ${SRC_DIR}/main.cpp
                                ${SRC_DIR}/CCollectionHash.hpp
                                ${SRC_DIR}/CCollectionHash.cpp
                                ${SRC_DIR}/CFactoryHashes.hpp
                                ${SRC_DIR}/CFactoryHashes.cpp
                                ${SRC_DIR}/Exceptions.hpp)
                                

### COMPILER OPTIONS

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)

if(MSVC)
	target_compile_options(
    ${PROJECT_NAME} PRIVATE 
    "/W4"
    "$<$<CONFIG:Debug>:/MTd>"
    "$<$<CONFIG:Release>:/MT>"
)
endif(MSVC)


target_include_directories(${PROJECT_NAME} PRIVATE  ${Boost_INCLUDE_DIRS}
                                                    ${CMAKE_SOURCE_DIR}/submodules
                                                    ${CMAKE_CURRENT_SOURCE_DIR}/src) 
                                
### LINK         
                      
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
if(MSVC)
    target_link_libraries(${PROJECT_NAME} 
                            debug       ${CRYTOPP_LIB_DIR}/Win32/Output/Debug/cryptlib.lib
                            optimized   ${CRYTOPP_LIB_DIR}/Win32/Output/Release/cryptlib.lib)
else()
    target_link_libraries(${PROJECT_NAME} ${CRYTOPP_LIB_DIR}/libcryptopp.a)
endif()
