####Make最低版本号要求##
##cmake_minimum_required(VERSION 2.8)
####project info##
##project(plugins)

##add connection cpp src --lib ##
#add_subdirectory(connection)

##add protocols cpp src --lib ##
add_subdirectory(protocols)

##add commons cpp src --lib ##
#add_subdirectory(commons)

##add callbacks cpp src --lib ##
add_subdirectory(callbacks)

##add modules cpp src --lib ##
add_subdirectory(modules)

##add drivers cpp src --lib ##
#add_subdirectory(drivers)

##add_subdirectory(tests)