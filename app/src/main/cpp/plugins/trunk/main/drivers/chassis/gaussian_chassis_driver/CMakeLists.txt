##最低版本号要求##
cmake_minimum_required (VERSION 2.8)

##project info##
SET(PACKAGE_NAME gaussian_chassis_driver)

#set compile flags
#添加-fPIC编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

##Enable C++11##
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

##LIB_DIR##
#set(LIB_DIR "/usr/local/lib")
set(BASE_LIB_DIR ${PROJECT_SOURCE_DIR}/thirdparty/lib)
set(PUB_LIB_DIR ${PROJECT_SOURCE_DIR}/linux/lib)
set(EXT_LIB_DIR ${PROJECT_SOURCE_DIR}/linux/extern_lib)

##add inlude include_directories and link_libraries ##
#include_directories(. /usr/local/include ./public/include)
include_directories(. ${PROJECT_SOURCE_DIR}/thirdparty/include ${PROJECT_SOURCE_DIR}/plugins/thirdparty/websocketpp-0.7.0)

##add link_libraries_directories ##
link_directories(${BASE_LIB_DIR}/log4cplus ${BASE_LIB_DIR}/curl ${PUB_LIB_DIR} ${EXT_LIB_DIR})

#链接库
link_libraries(log4cplus curl sys_framework_package os_hal_package core_help_package httpclient)

##查找当前目录下的所有源文件##
##并将名称保存到DIR_SRCS变量##
aux_source_directory(./src SRC_LIST)

##指定生成目标##生成动态库或静态库(第1个参数指定库的名字；第2个参数决定是动态还是静态，如果没有就默认静态；第3个参数指定生成库的源文件)
add_library(${PACKAGE_NAME}_shared SHARED ${SRC_LIST})
#add_library(${PACKAGE_NAME}_static STATIC ${SRC_LIST})
##设置输出的名称，还有其它功能，如设置库的版本号等等
set_target_properties(${PACKAGE_NAME}_shared PROPERTIES OUTPUT_NAME ${PACKAGE_NAME})
#set_target_properties(${PACKAGE_NAME}_static PROPERTIES OUTPUT_NAME ${PACKAGE_NAME})
##库文件的默认输出路径，这里设置为工程目录下的lib目录
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/linux/plugins)