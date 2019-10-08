cmake_minimum_required(VERSION 2.8.4)

project(seeta_fa_lib)

# Build options
option(BUILD_FACE_QUALITY  "Set to ON to build examples"  ON)

# Use C++11
#set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -g")
message(STATUS "C++11 support has been enabled by default.")

find_package(Eigen3 REQUIRED)
message(STATUS "Eigen3 found: ${EIGEN3_FOUND}")
message(STATUS "Eigen3 include dir found at ${EIGEN3_INCLUDE_DIR}")
message(STATUS "Eigen3 version: ${EIGEN3_VERSION}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1")
set(CEREAL_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/3rdparty/cereal-1.1.1/include/")
set(BASE_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/3rdparty/base/")

include_directories(include
                    include/detect
                    include/alignment
                    include/evergrande
                    include/rcr
                    include/superviseddescent)

include_directories(${CEREAL_INCLUDE_DIR})
include_directories(${EIGEN3_INCLUDE_DIR})
include_directories(${BASE_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/3rdparty/tinyxml/)


set(detect_src_files
    src/detect/util/nms.cpp
    src/detect/util/image_pyramid.cpp
    src/detect/io/lab_boost_model_reader.cpp
    src/detect/io/surf_mlp_model_reader.cpp
    src/detect/feat/lab_feature_map.cpp
    src/detect/feat/surf_feature_map.cpp
    src/detect/classifier/lab_boosted_classifier.cpp
    src/detect/classifier/mlp.cpp
    src/detect/classifier/surf_mlp.cpp
    src/detect/face_detection.cpp
    src/detect/fust.cpp
    )

aux_source_directory(src/alignment alignment_src_files)

set(evg_srv_files
	src/evergrande/fq_quality.cpp
    src/evergrande/fq_emotion.cpp
    src/evergrande/fq_detector.cpp
    src/evergrande/fq_json.cpp
    src/evergrande/fq_occlusion.cpp
    src/evergrande/fq_codecs.cpp
	src/evergrande/fq_req_mgt.cpp 
	src/evergrande/fq_msg_oper.cpp  
	src/evergrande/fq_request.cpp 
	src/evergrande/fq_req_processor.cpp 
	src/evergrande/fq_req_tcp_event_handler.cpp 
	src/evergrande/fq_conf_mgt.cpp 
	src/evergrande/fq_detector_mgt.cpp
	src/evergrande/main.cpp 
	)

set(evg_cli_files
	src/evergrande/fq_codecs.cpp
	src/evergrande/test/fq_cli.cpp
	)

set(evg_test_files
    src/evergrande/fq_quality.cpp
    src/evergrande/fq_emotion.cpp
    src/evergrande/fq_detector.cpp
    src/evergrande/fq_json.cpp
    src/evergrande/fq_occlusion.cpp
    src/evergrande/test/fq_test.cpp
    )

set(evg_tool_files
	src/evergrande/tools/tree_engine.cpp
	)
	
# Build shared library
add_library(seeta_facedet_lib ${detect_src_files})
add_library(seeta_fa_lib ${alignment_src_files})
set(seeta_required_libs seeta_facedet_lib seeta_fa_lib)

aux_source_directory(${CMAKE_SOURCE_DIR}/3rdparty/base base_src_files)
add_library(base ${base_src_files})

aux_source_directory(${CMAKE_SOURCE_DIR}/3rdparty/tinyxml tinyxml_src_files)
add_library(tinyxml ${tinyxml_src_files})

set(install_dir
    /root/aivideo/
    )

set(opencv_libs 	
	opencv_core
	opencv_dnn
	opencv_imgproc
	opencv_imgcodecs
	opencv_ml
	opencv_calib3d
	opencv_highgui
	zlib
	libjasper
	libjpeg-turbo
	libpng
	libtiff
	IlmImf
	libprotobuf
	libwebp
	)

if (BUILD_FACE_QUALITY)
    message(STATUS "Build with face_quality.")
	
	include_directories(${CMAKE_SOURCE_DIR}/3rdparty/opencv-4.0.0/include)
	include_directories(${CMAKE_SOURCE_DIR}/3rdparty/dlib)	
	include_directories(${CMAKE_SOURCE_DIR}/3rdparty/jsoncpp-0.10.7/include)

	add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/opencv-4.0.0)
	add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/jsoncpp-0.10.7)
	add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/dlib)

	link_directories(${CMAKE_SOURCE_DIR}/build/3rdparty/jsoncpp-0.10.7/src/lib_json)
	list(APPEND seeta_required_libs ${opencv_libs})

	add_executable(fq_srv ${evg_srv_files})
	target_link_libraries(fq_srv ${seeta_required_libs} base tinyxml jsoncpp -lrt -lpthread dlib)

	add_executable(fq_cli ${evg_cli_files})
	target_link_libraries(fq_cli ${opencv_libs} jsoncpp)

	#add_executable(fq_test ${evg_test_files})
	#target_link_libraries(fq_test ${seeta_required_libs} base -lrt -lpthread jsoncpp dlib)

	#add_executable(tree_engine ${evg_tool_files})
	#target_link_libraries(tree_engine ${opencv_libs})

	install(DIRECTORY ${CMAKE_SOURCE_DIR}/model DESTINATION ${install_dir})
	install(DIRECTORY ${CMAKE_SOURCE_DIR}/conf DESTINATION ${install_dir})
	install(DIRECTORY ${CMAKE_SOURCE_DIR}/log DESTINATION ${install_dir})
	install(PROGRAMS ${CMAKE_SOURCE_DIR}/build/fq_srv DESTINATION ${install_dir}/build/)
	install(PROGRAMS ${CMAKE_SOURCE_DIR}/build/fq_srv.sh DESTINATION ${install_dir}/build/)

endif()
