# read :    https://de.slideshare.net/DanielPfeifer1/cmake-48475415
#           https://rix0r.nl/blog/2015/08/13/cmake-guide/

cmake_minimum_required(VERSION 3.9 FATAL_ERROR) #3.9 required to find boost 1.65


project(tests LANGUAGES CXX)



### SOURCES


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
add_executable(${PROJECT_NAME}  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
								${CMAKE_CURRENT_SOURCE_DIR}/src/test_messaging.cpp
								${CMAKE_CURRENT_SOURCE_DIR}/src/test_library.hpp
								${CMAKE_CURRENT_SOURCE_DIR}/src/test_library.cpp
)


### COMPILER OPTIONS

# common
#set_property(GLOBAL PROPERTY CXX_STANDARD 11)
#set(COMPILE_OPTIONS_MSVC "/W4" "$<$<CONFIG:Debug>:/MTd>" "$<$<CONFIG:Release>:/MT>" "$<$<CONFIG:Release>:/WX>")
#set(COMPILE_OPTIONS_UNIX "-std=c++11" "-Wall"  "$<$<CONFIG:Release>:-Werror>")


if(MSVC)
	target_compile_options(
    ${PROJECT_NAME} PRIVATE
		${COMPILE_OPTIONS_MSVC}
)
else()
	target_compile_options(
		${PROJECT_NAME} PRIVATE
		${COMPILE_OPTIONS_UNIX}
)
endif(MSVC)

target_include_directories(${PROJECT_NAME} 	PRIVATE	${CMAKE_CURRENT_SOURCE_DIR}/include
													${SRC_DIR_LIB}
                                                    ${Boost_INCLUDE_DIRS}
										    PUBLIC	${INCLUDE_DIR}
)


### LINK
target_link_libraries(${PROJECT_NAME}   ${LIB_NAME}
                                        ${CMAKE_THREAD_LIBS_INIT}
                                        ${Boost_LIBRARIES}
)

#### TEST
add_test(NAME TEST COMMAND ${PROJECT_NAME} -s WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
