#!/bin/sh

NCPU="$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)"

if [ ! -e build ];then
	mkdir build
fi

cp ./src/evergrande/fq_srv.sh ./build
cp -rfd ./3rdparty/opencv-4.0.0 ./build/

if [ ! -e log ];then
	mkdir log
fi

cd build
cmake -DEXECUTABLE_OUTPUT_PATH:PATH=$(pwd) \
	-DCMAKE_BUILD_TYPE=RELEASE \
	-DCMAKE_INSTALL_PREFIX=/usr/local \
	-DBUILD_JASPER=ON \
	-DBUILD_JAVA=OFF \
	-DBUILD_JPEG=ON \
	-DBUILD_PERF_TESTS=OFF \
	-DBUILD_PNG=ON \
	-DBUILD_PROTOBUF=ON \
	-DBUILD_SHARED_LIBS=NO \
	-DBUILD_TESTS=OFF \
	-DBUILD_TIFF=ON \
	-DBUILD_ZLIB=ON \
	-DBUILD_WEBP=ON \
	-DBUILD_opencv_apps=OFF \
	-DBUILD_opencv_core=ON \
	-DBUILD_opencv_calib3d=ON \
	-DBUILD_opencv_dnn=ON \
	-DBUILD_opencv_features2d=ON \
	-DBUILD_opencv_flann=ON \
	-DBUILD_opencv_gapi=OFF \
	-DBUILD_opencv_highgui=ON \
	-DBUILD_opencv_imgcodecs=ON \
	-DBUILD_opencv_imgproc=ON \
	-DBUILD_opencv_java_bindings_generator=OFF \
	-DBUILD_opencv_js=OFF \
	-DBUILD_opencv_ml=ON \
	-DBUILD_opencv_objdetect=OFF \
	-DBUILD_opencv_photo=OFF \
	-DBUILD_opencv_python2=OFF \
	-DBUILD_opencv_python3=OFF \
	-DBUILD_opencv_python_bindings_generator=OFF \
	-DBUILD_opencv_stitching=OFF \
	-DBUILD_opencv_ts=OFF \
	-DBUILD_opencv_video=OFF \
	-DBUILD_opencv_videoio=OFF \
	-DLIB_NO_GUI_SUPPORT=ON \
	-DWITH_GTK=OFF \
	-DWITH_GTK_2_X=OFF \
	-DJSONCPP_WITH_POST_BUILD_UNITTEST=OFF \
	-DJSONCPP_WITH_TESTS=OFF \
	..
	
make -j $NCPU

