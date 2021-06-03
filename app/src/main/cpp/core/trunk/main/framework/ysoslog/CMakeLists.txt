##最低版本号要求##
cmake_minimum_required(VERSION 2.8)

##库文件的默认输出路径，这里设置为工程目录下的lib目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
        ${PROJECT_SOURCE_DIR}/android/${ANDROID_ABI}/libs)

##project info##
SET(PACKAGE_NAME ysoslog)

aux_source_directory(./src SRC_LIST)

##add inlude include_directories and link_libraries ##
include_directories(. ${PROJECT_SOURCE_DIR}/thirdparty/include)

#1. 添加自己的so库test-lib
add_library( # Sets the name of the library.
        ${PACKAGE_NAME}

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        #src/ysos.cpp
        ${SRC_LIST})

#2.添加第三方库
add_library(log4cplus SHARED IMPORTED)
#3.添加库的路径
set_target_properties(log4cplus
        PROPERTIES IMPORTED_LOCATION
        ${PROJECT_SOURCE_DIR}/thirdparty/lib/log4cplus/${ANDROID_ABI}/liblog4cplus.so)

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log )
#2.添加链接
target_link_libraries( # Specifies the target library.
        ${PACKAGE_NAME}
        #ysos
        log4cplus

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib} )