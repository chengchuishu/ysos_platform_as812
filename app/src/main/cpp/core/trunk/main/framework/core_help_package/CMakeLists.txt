##最低版本号要求##
cmake_minimum_required (VERSION 2.8)

#------------------------------------------[0]----------------------------------------------------#
##库文件的默认输出路径，这里设置为工程目录下的lib目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
        ${PROJECT_SOURCE_DIR}/android/${ANDROID_ABI}/libs)
#-------------------------------------------------------------------------------------------------#

##project info##
SET(PACKAGE_NAME core_help_package)

#set compile flags
#添加-fPIC编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

## add compile params  ##
#### -DDEBUG  open   assert alias debug   ####
#### -DNDEBUG ignore assert alias release ####
add_definitions(-DNDEBUG)

##Enable C++11##
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
##LIB_DIR##
#set(LIB_DIR "/usr/local/lib")
set(BASE_LIB_DIR ${PROJECT_SOURCE_DIR}/thirdparty/lib)

add_definitions(-DBOOST_LOG_DYN_LINK)

##add inlude include_directories and link_libraries ##
include_directories(. ${PROJECT_SOURCE_DIR}/thirdparty/include)

#------------------------------------------[1]----------------------------------------------------#
####添加第三方库 json boost_chrono boost_filesystem boost_log_setup boost_locale boost_log log4cplus tinyxml2)
add_library(json SHARED IMPORTED)
add_library(boost_chrono SHARED IMPORTED)
add_library(boost_filesystem SHARED IMPORTED)
add_library(boost_log_setup SHARED IMPORTED)
add_library(boost_locale SHARED IMPORTED)
add_library(boost_log SHARED IMPORTED)
add_library(boost_thread SHARED IMPORTED)
add_library(boost_system SHARED IMPORTED)
add_library(log4cplus SHARED IMPORTED)
add_library(tinyxml2 SHARED IMPORTED)
#3.添加库的路径
set_target_properties(json
        PROPERTIES IMPORTED_LOCATION
        ${PROJECT_SOURCE_DIR}/thirdparty/lib/json/${ANDROID_ABI}/libjson.so)
set_target_properties(boost_chrono
        PROPERTIES IMPORTED_LOCATION
        ${PROJECT_SOURCE_DIR}/thirdparty/lib/boost/${ANDROID_ABI}/libboost_chrono.so)
set_target_properties(boost_filesystem
        PROPERTIES IMPORTED_LOCATION
        ${PROJECT_SOURCE_DIR}/thirdparty/lib/boost/${ANDROID_ABI}/libboost_filesystem.so)
set_target_properties(boost_log_setup
        PROPERTIES IMPORTED_LOCATION
        ${PROJECT_SOURCE_DIR}/thirdparty/lib/boost/${ANDROID_ABI}/libboost_log_setup.so)
set_target_properties(boost_locale
        PROPERTIES IMPORTED_LOCATION
        ${PROJECT_SOURCE_DIR}/thirdparty/lib/boost/${ANDROID_ABI}/libboost_locale.so)
set_target_properties(boost_log
        PROPERTIES IMPORTED_LOCATION
        ${PROJECT_SOURCE_DIR}/thirdparty/lib/boost/${ANDROID_ABI}/libboost_log.so)
set_target_properties(boost_thread
        PROPERTIES IMPORTED_LOCATION
        ${PROJECT_SOURCE_DIR}/thirdparty/lib/boost/${ANDROID_ABI}/libboost_thread.so)
set_target_properties(boost_system
        PROPERTIES IMPORTED_LOCATION
        ${PROJECT_SOURCE_DIR}/thirdparty/lib/boost/${ANDROID_ABI}/libboost_system.so)
set_target_properties(log4cplus
        PROPERTIES IMPORTED_LOCATION
        ${PROJECT_SOURCE_DIR}/thirdparty/lib/log4cplus/${ANDROID_ABI}/liblog4cplus.so)
set_target_properties(tinyxml2
        PROPERTIES IMPORTED_LOCATION
        ${PROJECT_SOURCE_DIR}/thirdparty/lib/tinyxml2/${ANDROID_ABI}/libtinyxml2.so)
#-------------------------------------------------------------------------------------------------#

##查找当前目录下的所有源文件##
##并将名称保存到DIR_SRCS变量##
aux_source_directory(./src SRC_LIST)

#------------------------------------------[2]----------------------------------------------------#
####添加自己的so库test-lib
add_library( # Sets the name of the library.
        ${PACKAGE_NAME}

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        #src/ysos.cpp
        ${SRC_LIST})

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log )
#####添加链接
target_link_libraries( # Specifies the target library.
        ${PACKAGE_NAME}
        json
        boost_chrono
        boost_filesystem
        boost_log_setup
        boost_locale
        boost_log
        boost_thread
        boost_system
        log4cplus
        tinyxml2

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib} )
#-------------------------------------------------------------------------------------------------#