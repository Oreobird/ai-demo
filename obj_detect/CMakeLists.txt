cmake_minimum_required(VERSION 2.8.4)

project(object_detect)

# Build options
option(BUILD_OBJECT_DETECT "Set to ON to build examples"  ON)

# Use C++11
#set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")
message(STATUS "C++11 support has been enabled by default.")

include_directories(${CMAKE_SOURCE_DIR}/3rdparty/base/)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty/tinyxml/)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty/rapidjson/include/)
include_directories(src)
include_directories(src/test)

aux_source_directory(${CMAKE_SOURCE_DIR}/3rdparty/base base_src_files)
add_library(base ${base_src_files})

aux_source_directory(${CMAKE_SOURCE_DIR}/3rdparty/tinyxml tinyxml_src_files)
add_library(tinyxml ${tinyxml_src_files})

aux_source_directory(src evergrande_src_files)

set(tools_files
	src/obj_detector.cpp
	src/tools/object_detect_demo.cpp
	)

set(cli_files
	src/conf_mgt.cpp
	src/codecs.cpp
	src/protocol.cpp
	src/test/cli.cpp
	)
	
if (BUILD_OBJECT_DETECT)
    message(STATUS "Build with object_detect.")
    find_package(OpenCV)
    if (NOT OpenCV_FOUND)
        message(WARNING "OpenCV not found. Test will not be built.")
    else()
		include_directories(${Boost_INCLUDE_DIRS} build)
		include_directories(${OpenCV_INCLUDE_DIRS} build)
		set(required_libs ${OpenCV_LIBS})

		add_executable(demo ${tools_files})
		target_link_libraries(demo ${required_libs})

		add_executable(od_srv ${evergrande_src_files})
		target_link_libraries(od_srv ${required_libs} base -lrt -lpthread tinyxml)
		
		add_executable(cli ${cli_files})
		target_link_libraries(cli ${required_libs} base -lrt -lpthread tinyxml)
    endif()
endif()
