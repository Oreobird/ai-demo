1.安装环境依赖库，需下载好opencv源码压缩包opencv-4.0.0.zip，在aivideo目录下，执行
	./setup.sh
2.编译C++源代码，在obj_detect目录下，执行
	./build.sh
3.运行服务：
	cd ./build
	./od_srv -s ../conf/object_detect.xml
	

