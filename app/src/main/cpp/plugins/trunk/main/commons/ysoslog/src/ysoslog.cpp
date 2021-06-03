
/**
 *@file BaseInterfaceImpl.cpp
 *@brief Definition of BaseInterfaceImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// Self Header
//#include <log4cplus/logger.h>
/// 3rdparty Headers
#include "../../../../../../thirdparty/include/log4cplus/configurator.h"
#include "../../../../../../thirdparty/include/log4cplus/fileappender.h"
#include "../../../../../../thirdparty/include/log4cplus/helpers/loglog.h"
#include "../../../../../../thirdparty/include/log4cplus/layout.h"

namespace ysos {

namespace log {

using namespace log4cplus;
using namespace log4cplus::helpers;

static log4cplus::ConfigureAndWatchThread* g_configure_and_watch_thread_ptr_ = NULL;

#ifdef _UNICODE

bool InitLogger(const std::wstring &log_properties_file_path) {
  if (true == log_properties_file_path.empty()) {
    return false;
  }

  g_configure_and_watch_thread_ptr_ = new log4cplus::ConfigureAndWatchThread(log_properties_file_path.c_str(), 30*1000);
  assert(NULL != g_configure_and_watch_thread_ptr_);
  // log4cplus::ConfigureAndWatchThread configureThread("log4cplus.properties", 5 * 1000);

  return true;
}

bool InitLoggerExt(const std::wstring& filename, const std::wstring &logger_name, long maxFileSize, int maxBackupIndex, bool immediateFlush, bool createDirs) {
  if (true == filename.empty()) {
    return false;
  }

  log4cplus::initialize();
  LogLog::getLogLog()->setInternalDebugging(true);
  try {
    /// create a new layout
    SharedObjectPtr<Appender> append(new RollingFileAppender(filename, maxFileSize, maxBackupIndex, immediateFlush, createDirs));
    /// create a new layout
    log4cplus::tstring pattern = LOG4CPLUS_TEXT("[%D{%Y-%m-%d %H:%M:%S:%Q} %p-%t] --- %m --- [%F:%M:%L] %n");
    std::auto_ptr<Layout> layout = std::auto_ptr<Layout>(new PatternLayout(pattern));

    std::wstring duplicated_append_name = logger_name;
    std::wstring duplicated_logger_name = _T("ysos.");
    if (!logger_name.empty()) {
      duplicated_append_name.append(_T("_append"));
      duplicated_logger_name.append(logger_name);
    } else {
      duplicated_append_name = _T("ysos_append");
      duplicated_logger_name = _T("ysos");
    }

    append->setName(duplicated_append_name);

    append->setLayout(layout);
    log4cplus::Logger::getInstance(duplicated_logger_name).addAppender(append);
  } catch (...) {
  }

  return true;
}

log4cplus::Logger GetLogger(const std::wstring &logger_name) {
  if (true == logger_name.empty()) {
    return log4cplus::Logger::getInstance(_T("ysos"));
  }

  std::wstring duplicated_logger_name = _T("ysos.");
  duplicated_logger_name.append(logger_name);
  return log4cplus::Logger::getInstance(duplicated_logger_name);
}

#else

bool InitLogger(const std::string &log_properties_file_path) {
  if (true == log_properties_file_path.empty()) {
    return false;
  }

  g_configure_and_watch_thread_ptr_ = new log4cplus::ConfigureAndWatchThread(log_properties_file_path.c_str(), 30*1000);
  assert(NULL != g_configure_and_watch_thread_ptr_);
  // log4cplus::ConfigureAndWatchThread configureThread("log4cplus.properties", 5 * 1000);

  return true;
}

bool InitLoggerExt(const std::string& filename, const std::string &logger_name, long maxFileSize, int maxBackupIndex, bool immediateFlush, bool createDirs) {
  if (true == filename.empty()) {
    return false;
  }

  log4cplus::initialize();
  LogLog::getLogLog()->setInternalDebugging(true);
  try {
    /// create a new layout
    SharedObjectPtr<Appender> append(new RollingFileAppender(filename, maxFileSize, maxBackupIndex, immediateFlush, createDirs));
    /// create a new layout
    log4cplus::tstring pattern = LOG4CPLUS_TEXT("[%D{%Y-%m-%d %H:%M:%S:%Q} %p-%t] --- %m --- [%F:%M:%L] %n");
    //std::auto_ptr<Layout> layout = std::auto_ptr<Layout>(new PatternLayout(pattern));
    //std::unique_ptr<log4cplus::Layout> layout = std::unique_ptr<log4cplus::Layout>(new log4cplus::PatternLayout(pattern));//add for linux
    std::string duplicated_append_name = logger_name;
    std::string duplicated_logger_name = "ysos.";
    if (!logger_name.empty()) {
      duplicated_append_name.append("_append");
      duplicated_logger_name.append(logger_name);
    } else {
      duplicated_append_name = "ysos_append";
      duplicated_logger_name = "ysos";
    }

    append->setName(duplicated_append_name);

    append->setLayout(std::unique_ptr<log4cplus::Layout>(new log4cplus::PatternLayout(pattern)));
    log4cplus::Logger::getInstance(duplicated_logger_name).addAppender(append);
  } catch (...) {
  }

  return true;
}

log4cplus::Logger GetLogger(const std::string &logger_name) {
  if (true == logger_name.empty()) {
    return log4cplus::Logger::getInstance("ysos");
  }

  std::string duplicated_logger_name = "ysos.";
  duplicated_logger_name.append(logger_name);
  return log4cplus::Logger::getInstance(duplicated_logger_name);
}

#endif // _UNICODE

}
}