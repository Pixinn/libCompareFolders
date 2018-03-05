# read :    https://de.slideshare.net/DanielPfeifer1/cmake-48475415
#           https://rix0r.nl/blog/2015/08/13/cmake-guide/

cmake_minimum_required(VERSION 3.9 FATAL_ERROR) #3.9 required to find boost 1.65
 

project(compare_folders VERSION 0.1 LANGUAGES CXX)

### FIND THE LIBRARIES

# threads
find_package (Threads REQUIRED)
# boost
set(BOOST_ROOT $ENV{BOOST_DIR})
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME ON)
find_package(Boost 1.65.0 COMPONENTS filesystem system REQUIRED)
if(NOT Boost_FOUND)
	message(FATAL_ERROR "\nBoost is mandatory.\nTry to export the root directory of your Boost installation as \"BOOST_DIR\".")
endif()
# cryptopp
set(CRYTOPP_LIB_DIR ${CMAKE_SOURCE_DIR}/submodules/cryptopp)


### SOURCES

# library
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(SRC_DIR_LIB ${CMAKE_SOURCE_DIR}/src/lib)
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(LIB_NAME "CompareFolders")
add_library(${LIB_NAME} ${SRC_DIR_LIB}/CCollectionInfo.hpp
                        ${SRC_DIR_LIB}/CCollectionInfo.cpp
                        ${SRC_DIR_LIB}/CFactoryInfo.hpp
                        ${SRC_DIR_LIB}/CFactoryInfo.cpp
						${SRC_DIR_LIB}/CompareFolders.cpp
                        ${SRC_DIR_LIB}/Utilities.cpp
                        ${INCLUDE_DIR}/CompareFolders.hpp)

# applications
set(APP_COMPAREFOLDERS_NAME "compare_folders")
set(APP_SCANFOLDER_NAME "scan_folder")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(SRC_DIR_APPS ${CMAKE_SOURCE_DIR}/src/apps)
add_executable(${APP_COMPAREFOLDERS_NAME}  ${SRC_DIR_APPS}/${APP_COMPAREFOLDERS_NAME}/main.cpp)
add_executable(${APP_SCANFOLDER_NAME}      ${SRC_DIR_APPS}/${APP_SCANFOLDER_NAME}/main.cpp)

### COMPILER OPTIONS

# common
set_property(GLOBAL PROPERTY CXX_STANDARD 11)
set(COMPILE_OPTIONS_MSVC "/W4" "/MP" "$<$<CONFIG:Debug>:/MTd>" "$<$<CONFIG:Release>:/MT>" "$<$<CONFIG:Release>:/WX>")
set(COMPILE_OPTIONS_UNIX "-std=c++11" "-Wall"  "$<$<CONFIG:Release>:-Werror>" "-Wno-missing-braces") #"missing-braces" only emitted by Xcode (deprecated in GCC)

# library
target_include_directories(${LIB_NAME} 	PRIVATE	${Boost_INCLUDE_DIRS}
                                                ${CMAKE_SOURCE_DIR}/submodules
                                                ${SRC_DIR_LIB}
										PUBLIC	${INCLUDE_DIR}
)
if(MSVC)
	target_compile_options(
        ${LIB_NAME} PRIVATE
        ${COMPILE_OPTIONS_MSVC}
)
else()
	target_compile_options(
		${LIB_NAME} PRIVATE
		${COMPILE_OPTIONS_UNIX}
)
endif(MSVC)

# applications
if(MSVC)
	target_compile_options(
        ${APP_COMPAREFOLDERS_NAME} PRIVATE
		${COMPILE_OPTIONS_MSVC}
    )
    target_compile_options(
        ${APP_SCANFOLDER_NAME} PRIVATE
		${COMPILE_OPTIONS_MSVC}
    )
else()
	target_compile_options(
		${APP_COMPAREFOLDERS_NAME} PRIVATE
		${COMPILE_OPTIONS_UNIX}
    )
    target_compile_options(
		${APP_SCANFOLDER_NAME} PRIVATE
		${COMPILE_OPTIONS_UNIX}
    )
endif(MSVC)
target_include_directories(${APP_COMPAREFOLDERS_NAME} 	PRIVATE	${Boost_INCLUDE_DIRS} 			        # to be removed
													    ${CMAKE_SOURCE_DIR}/submodules	                # to be removed
													    ${SRC_DIR_APPS}/${APP_COMPAREFOLDERS_NAME}/	    # to be removed
													    ${SRC_DIR_APPS}/common
                                                        ${INCLUDE_DIR}
)
target_include_directories(${APP_SCANFOLDER_NAME} 	PRIVATE	${Boost_INCLUDE_DIRS} 			        # to be removed
													    ${CMAKE_SOURCE_DIR}/submodules	                # to be removed
													    ${SRC_DIR_APPS}/${APP_SCANFOLDER_NAME}/	    # to be removed
                                                        ${SRC_DIR_APPS}/common
													    ${INCLUDE_DIR}
)

### LINK

# library
target_link_libraries(${LIB_NAME} ${Boost_LIBRARIES})
if(MSVC)
    target_link_libraries(${LIB_NAME}
                            debug       ${CRYTOPP_LIB_DIR}/Win32/Output/Debug/cryptlib.lib
                            optimized   ${CRYTOPP_LIB_DIR}/Win32/Output/Release/cryptlib.lib)
else()
    target_link_libraries(${LIB_NAME} ${CRYTOPP_LIB_DIR}/libcryptopp.a)
endif()

# applications
target_link_libraries(${APP_COMPAREFOLDERS_NAME} ${LIB_NAME} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(${APP_SCANFOLDER_NAME} ${LIB_NAME} ${CMAKE_THREAD_LIBS_INIT})

### TEST
enable_testing()
add_subdirectory(test)
