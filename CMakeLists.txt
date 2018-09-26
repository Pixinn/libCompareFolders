# read :    https://de.slideshare.net/DanielPfeifer1/cmake-48475415
#           https://rix0r.nl/blog/2015/08/13/cmake-guide/

cmake_minimum_required(VERSION 3.9 FATAL_ERROR) #3.9 required to find boost 1.65

project(compare_folders VERSION 0.1 LANGUAGES CXX)

### FIND THE LIBRARIES
# Win32 only: Where to install and search for packages. Would default to "Program Files"
if(CMAKE_SYSTEM MATCHES "Windows") 
	if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
		if("$ENV{CMAKE_PACKAGES_DIR}" STREQUAL "")
			message(FATAL_ERROR "\n!!!!!!!!! FATAL ERROR !!!!!!!!!\nPlease define CMAKE_INSTALL_PREFIX or set a CMAKE_PACKAGES_DIR environment variable.\nDelete CMakeCache.txt then proceed.\n!!!!!!!!!")
		else()
            set(CMAKE_INSTALL_PREFIX $ENV{CMAKE_PACKAGES_DIR} CACHE PATH "..." FORCE)             
		endif()
  endif()
  set(CMAKE_PREFIX_PATH ${CMAKE_INSTALL_PREFIX})
endif(CMAKE_SYSTEM MATCHES "Windows")
include(GNUInstallDirs)

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
# my libraries
find_package(cryptopp REQUIRED)
find_package(rlutil REQUIRED)

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
						${SRC_DIR_LIB}/TDequeConcurrent.hpp
						${SRC_DIR_LIB}/CProxyLogger.hpp
                        ${INCLUDE_DIR}/CompareFolders.hpp
)

# applications
set(APP_COMPAREFOLDERS_NAME "compare_folders")
set(APP_SCANFOLDER_NAME "scan_folder")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(SRC_DIR_APPS ${CMAKE_SOURCE_DIR}/src/apps)

add_executable(${APP_COMPAREFOLDERS_NAME}  ${SRC_DIR_APPS}/${APP_COMPAREFOLDERS_NAME}/main.cpp)
add_executable(${APP_SCANFOLDER_NAME}      ${SRC_DIR_APPS}/${APP_SCANFOLDER_NAME}/main.cpp)

### COMPILER OPTIONS

# common
set_property(GLOBAL PROPERTY CXX_STANDARD 14)
set(COMPILE_OPTIONS_MSVC "/W4" "/MP" "$<$<CONFIG:Debug>:/MTd>" "$<$<CONFIG:Release>:/MT>" "$<$<CONFIG:Release>:/WX>")
set(COMPILE_OPTIONS_UNIX "-std=c++14" "-Wall"  "$<$<CONFIG:Release>:-Werror>" "-Wno-missing-braces") #"missing-braces" only emitted by Xcode (deprecated in GCC)

# library
target_include_directories(${LIB_NAME} 	PRIVATE	${Boost_INCLUDE_DIRS}
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
target_include_directories(${APP_COMPAREFOLDERS_NAME} 	PRIVATE	
													    ${SRC_DIR_APPS}/common
                                                        ${INCLUDE_DIR}
)
target_include_directories(${APP_SCANFOLDER_NAME} 	PRIVATE
                                                        ${SRC_DIR_APPS}/common
													    ${INCLUDE_DIR}
)

### LINK

# library
target_link_libraries(${LIB_NAME} ${Boost_LIBRARIES} cryptopp-static)

# applications
target_link_libraries(${APP_COMPAREFOLDERS_NAME} ${LIB_NAME} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(${APP_SCANFOLDER_NAME} ${LIB_NAME} ${CMAKE_THREAD_LIBS_INIT})

### TEST
enable_testing()
add_subdirectory(test)
