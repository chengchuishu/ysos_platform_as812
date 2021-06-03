/**
 *@file BufferImpl.h
 *@brief Definition of BufferImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef YSOS_LOG_H_  // NOLINT
#define YSOS_LOG_H_  // NOLINT

//#include "../../../public/include/ysoslog/ysoslog.h"

/// STL Headers
#ifdef _WIN32
#include <tchar.h>
#else
#endif

#include <string>
/// 3rdparty Headers
#include <log4cplus/loggingmacros.h>

namespace ysos {

#ifdef _YSOS_DLL_EXPORTING
#define YSOS_EXPORT __declspec(dllexport)
#else
#define YSOS_EXPORT /*__declspec(dllimport)*/ // update for linux
#endif

namespace log {

/**
  *@brief  初始化全局log4plus配置文件 //  NOLINT
  *@param log_properties_file_path log配置文件路径   //  NOLINT
  *@return 成功返回true，失败返回false  //  NOLINT
  */
#ifdef _UNICODE

bool YSOS_EXPORT InitLogger(const std::wstring &log_properties_file_path);
bool YSOS_EXPORT InitLoggerExt(const std::wstring& filename, const std::wstring &logger_name=_T(""), long maxFileSize = 10*1024*1024, int maxBackupIndex = 5, bool immediateFlush = true, bool createDirs = true);
log4cplus::Logger YSOS_EXPORT GetLogger(const std::wstring &logger_name=_T(""));


#define  YSOS_LOG_INFO_DEFAULT(msg)    LOG4CPLUS_INFO(log4cplus::Logger::getInstance(_T("ysos")), msg)
#define  YSOS_LOG_WARN_DEFAULT(msg)    LOG4CPLUS_WARN(log4cplus::Logger::getInstance(_T("ysos")), msg)
#define  YSOS_LOG_DEBUG_DEFAULT(msg)   LOG4CPLUS_DEBUG(log4cplus::Logger::getInstance(_T("ysos")), msg)
#define  YSOS_LOG_ERROR_DEFAULT(msg)   LOG4CPLUS_ERROR(log4cplus::Logger::getInstance(_T("ysos")), msg)
#define  YSOS_LOG_FATAL_DEFAULT(msg)   LOG4CPLUS_FATAL(log4cplus::Logger::getInstance(_T("ysos")), msg)
//
#define  YSOS_LOG_INFO(msg)   LOG4CPLUS_INFO(logger_, msg)
#define  YSOS_LOG_WARN(msg)   LOG4CPLUS_WARN(logger_, msg)
#define  YSOS_LOG_DEBUG(msg)   LOG4CPLUS_DEBUG(logger_, msg)
#define  YSOS_LOG_ERROR(msg)   LOG4CPLUS_ERROR(logger_, msg)
#define  YSOS_LOG_FATAL(msg)   LOG4CPLUS_FATAL(logger_, msg)
//
#define  YSOS_LOG_INFO_CUSTOM(logger, msg)   LOG4CPLUS_INFO(logger, msg)
#define  YSOS_LOG_WARN_CUSTOM(logger, msg)   LOG4CPLUS_WARN(logger, msg)
#define  YSOS_LOG_DEBUG_CUSTOM(logger, msg)   LOG4CPLUS_DEBUG(logger, msg)
#define  YSOS_LOG_ERROR_CUSTOM(logger, msg)   LOG4CPLUS_ERROR(logger, msg)
#define  YSOS_LOG_FATAL_CUSTOM(logger, msg)   LOG4CPLUS_FATAL(logger, msg)

#else

bool YSOS_EXPORT InitLogger(const std::string &log_properties_file_path);
bool YSOS_EXPORT InitLoggerExt(const std::string& filename, const std::string &logger_name="", long maxFileSize = 10*1024*1024, int maxBackupIndex = 5, bool immediateFlush = true, bool createDirs = true);
log4cplus::Logger YSOS_EXPORT GetLogger(const std::string &logger_name="");


#define  YSOS_LOG_INFO_DEFAULT(msg)    LOG4CPLUS_INFO(log4cplus::Logger::getInstance("ysos"), msg)
#define  YSOS_LOG_WARN_DEFAULT(msg)    LOG4CPLUS_WARN(log4cplus::Logger::getInstance("ysos"), msg)
#define  YSOS_LOG_DEBUG_DEFAULT(msg)   LOG4CPLUS_DEBUG(log4cplus::Logger::getInstance("ysos"), msg)
#define  YSOS_LOG_ERROR_DEFAULT(msg)   LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("ysos"), msg)
#define  YSOS_LOG_FATAL_DEFAULT(msg)   LOG4CPLUS_FATAL(log4cplus::Logger::getInstance("ysos"), msg)
//
#define  YSOS_LOG_INFO(msg)   LOG4CPLUS_INFO(logger_, msg)
#define  YSOS_LOG_WARN(msg)   LOG4CPLUS_WARN(logger_, msg)
#define  YSOS_LOG_DEBUG(msg)   LOG4CPLUS_DEBUG(logger_, msg)
#define  YSOS_LOG_ERROR(msg)   LOG4CPLUS_ERROR(logger_, msg)
#define  YSOS_LOG_FATAL(msg)   LOG4CPLUS_FATAL(logger_, msg)
//
#define  YSOS_LOG_INFO_CUSTOM(logger, msg)   LOG4CPLUS_INFO(logger, msg)
#define  YSOS_LOG_WARN_CUSTOM(logger, msg)   LOG4CPLUS_WARN(logger, msg)
#define  YSOS_LOG_DEBUG_CUSTOM(logger, msg)   LOG4CPLUS_DEBUG(logger, msg)
#define  YSOS_LOG_ERROR_CUSTOM(logger, msg)   LOG4CPLUS_ERROR(logger, msg)
#define  YSOS_LOG_FATAL_CUSTOM(logger, msg)   LOG4CPLUS_FATAL(logger, msg)

#endif // _UNICODE

}
}
#endif  // CHP_LOG_H_  // NOLINT