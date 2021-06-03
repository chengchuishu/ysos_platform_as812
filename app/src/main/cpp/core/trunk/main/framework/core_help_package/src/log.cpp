/**
 *@file BaseInterfaceImpl.cpp
 *@brief Definition of BaseInterfaceImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

///// Self Header
//#include "../../../protect/include/core_help_package/log.h"
////#include <log4cplus/logger.h>
///// 3rdparty Headers
//#include <log4cplus/configurator.h>
//
//namespace ysos {
//
//namespace log {
//static log4cplus::ConfigureAndWatchThread* g_configure_and_watch_thread_ptr_ = NULL;
//bool InitLogger(const std::string &log_properties_file_path) {
//  if (true == log_properties_file_path.empty()) {
//    return false;
//  }
//
//  g_configure_and_watch_thread_ptr_ = new log4cplus::ConfigureAndWatchThread(log_properties_file_path.c_str(), 30*1000);
//  assert(NULL != g_configure_and_watch_thread_ptr_);
//  // log4cplus::ConfigureAndWatchThread configureThread("log4cplus.properties", 5 * 1000);
//
//  return true;
//}
//
//}
//}
