/**
  *@file log.h
  *@brief Definition of log functions
  *@version 0.1
  *@author XuLanyue
  *@date Created on: 2016-11-05 10:00:00
  *@copyright Copyright © 2016 YunShen Technology. All rights reserved.
    
  *@howto
    Usage:
    1.确认写日志的权限,如通常不应写到系统盘;应加到可信站点.
    2.包含本头文件:
    #include "../../../public/include/core_help_package/ysos_log.h"
    根据实际情况可能要修改包含路径
    3.要在显式初始化与隐式初始化之间切换,请先调用YSOS_LOG_UNINIT();或者,
    确定不需要写日志了,请调用YSOS_LOG_UNINIT();以显式清理资源(请慎用.调用进程正常退出时会隐式清理相关资源).
    4.要做显式的设置调用YSOS_LOG_SET_XXX宏:
    YSOS_LOG_SET_LOCALE
    YSOS_LOG_SET_CHANNEL_INFO
    YSOS_LOG_SET_GLOBAL_LOG_LEVEL
    YSOS_LOG_SET_CONSOLE_LOG_LEVEL
    YSOS_LOG_ENABLE_CONSOLE_LOG
    YSOS_LOG_MINIMIZE_CONSOLE_LOG
    5.要做显式的初始化,请调用宏YSOS_LOG_INIT(ysos::Log::LOG_FILTER_MODE_RELEASE,
      "D:/"     //  日志配置文件的路径
      );
    日志配置文件说明
# must be ansi/ascii format.  //应为ansi/ascii格式

[Core]  //全局信息
DisableLogging=false  //true默认除能日志,false默认使能日志
Filter="%Severity% >= info"  //日志过滤级别

[Sinks.ysos]  //Channel "ysos"
Target="./log/"  //日志文件存储目录
Destination=TextFile  //日志目标为文本文件
FileName="ysos_%Y%m%d_%H%M%S_%03N.log"  //日志文件格式
Wide=false  //true对应宽字符,false对应ansi/ascii
Filter="%Channel% contains \"ysos\" and %Severity% >= info"  //过滤设置
Format="{\"mdl\":\"%Channel%\",\"ts\":\"%TimeStamp%\",\"ut\":\"%Uptime%\",\"pid\":\"%ProcessID%\",\"tid\":\"%ThreadID%\",\"lvl\":\"%Severity%\",\"msg\":{%Message%}}"  //格式设置
LocalAddress=127.0.0.1:6002  //本地ip:port
TargetAddress=127.0.0.1:6001  //目的ip:port
Asynchronous=false  //true对应异步,false对应同步
AutoFlush=false  //true对应每写一条日志都会自动冲洗缓冲区,会影响性能
Append=true  //true对应日志记录追加到文件尾
RotationInterval=86400  //以秒为单位的日志旋转的时间间隔
RotationTimePoint="00:00:00"  //日志旋转时刻
RotationSize=10485760  //日志旋转的尺寸
MaxFiles=3  //该日志文件的数量
ScanForFiles="All"  //在日志文件存储目录下扫描所有文件

    6.要实时除能写日志,请调用宏YSOS_LOG_DISABLE();要实时使能写日志,请调用宏YSOS_LOG_ENABLE();
    7.写日志.
    写控制台日志, 请使用TDLOG_CONSOLE_XXX宏, 如: TDLOG_CONSOLE_TRACE << 
    TDLOG_CONSOLE_TRACE << "Message with level Trace";
    TDLOG_CONSOLE_DEBUG << "Message with level Debug";
    TDLOG_CONSOLE_INFO << "Message with level Info";
    TDLOG_CONSOLE_WARNING << "Message with level Warning";
    TDLOG_CONSOLE_ERROR << "Message with level Error";
    TDLOG_CONSOLE_FATAL << "Message with level Fatal";
    写模块的日志, 请使用TDLOG_MODULE_XXX宏:
    TDLOG_MODULE_TRACE("module_name")
    TDLOG_MODULE_DEBUG("module_name")
    TDLOG_MODULE_INFO("module_name")
    TDLOG_MODULE_WARNING("module_name")
    TDLOG_MODULE_ERROR("module_name")
    TDLOG_MODULE_FATAL("module_name")

    @todo
    15.支持在日志初始化时设置日志是否为AutoFlush的.
    16.若某模块的本地日志文件数量超过指定值,则清除多出的日志文件.
    17.支持在日志初始化时设置日志是否写到远程服务器(TargetAddress).
    //指定日志存储服务器的IP和Port.//TargetAddress
    18.支持在日志初始化时设置字符是否为宽的(Wide).
    19.支持初始化时设置某个日志的全部属性.
    //在日志中带上终端的IP和Port.//LocalAddress
    YSOS_LOG_SET_CHANNEL_INFO
    (SetChannelInfo)
    parse the ini file.
    total file.
    rebuild ini file to apply explicit settings.
    如果测试表明单例有问题,那么可以考虑使用平台提供的单例模板.

  */

#ifndef CHP_YSOS_LOG_H//NOLINT
#define CHP_YSOS_LOG_H//NOLINT

#include "boost/shared_ptr.hpp"
#include "boost/filesystem.hpp"
#include "boost/log/core.hpp"
#include "boost/log/trivial.hpp"
#include "boost/log/sinks.hpp"
#include "boost/log/sinks/sync_frontend.hpp"
#include "boost/log/sinks/text_file_backend.hpp"
#include "boost/log/sources/logger.hpp"
#include "boost/log/sources/severity_logger.hpp"
#include "boost/log/sources/severity_channel_logger.hpp"
#include "boost/log/support/date_time.hpp"
#include <cassert>
#include <string>
#include <fstream>      // NOLINT
#include <iostream>     // NOLINT

#if !defined(USING_YSOS_LOG_LIB)
#include "../../../public/include/sys_interface_package/common.h"
#else   // USING_YSOS_LOG_LIB
#ifndef YSOS_EXPORT
#define YSOS_EXPORT
#endif  // YSOS_EXPORT
#endif  // USING_YSOS_LOG_LIB

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace trivial = boost::log::trivial;
namespace sinks = boost::log::sinks;
namespace fs = boost::filesystem;

//  fs::path(__FILE__).string()
//  fs::path(__FILE__).filename()
#define YSOS_MAKE_SCOPE_LOG     "\"pos\":{\"file\":" << fs::path(__FILE__).filename() << ",\"func\":\"" << __FUNCTION__ << "\",\"line\":" << __LINE__ << "}" << ",\"dtls\":"

namespace ysos {
class Log;
typedef boost::shared_ptr<Log>  LogPtr;

typedef src::severity_channel_logger_mt<trivial::severity_level, std::string> ysos_channel_log_type;

typedef sinks::synchronous_sink<sinks::text_file_backend> TextSink;
typedef sinks::basic_text_ostream_backend< char > backend_t;
typedef sinks::synchronous_sink< backend_t > sink_t;
typedef src::severity_logger_mt<trivial::severity_level>  ysos_console_log_type;
typedef std::pair<std::string, ysos_channel_log_type*>    ysos_channel_name_ptr_pair_type;
typedef std::map<std::string, ysos_channel_log_type*>     ysos_channel_name_ptr_map_type;

class YSOS_EXPORT Log {
 public:

  /// 日志过滤模式
  enum LogFilterMode {
    LOG_FILTER_MODE_TRACE,      ///< 最详尽的日志
    LOG_FILTER_MODE_DEBUG,      ///< >= debug
    LOG_FILTER_MODE_RELEASE,    ///< >= info
  };

 private:
  Log(const Log&);
  Log& operator=(const Log&);
 protected:
  Log(const std::string &class_name = "Log");
 public:
  ~Log();

 public:
  /// 初始化日志
  /// ini_file_path应以字符'\\'或'/'结尾
  void Init(LogFilterMode log_filter_mode = LOG_FILTER_MODE_RELEASE, const std::string &ini_file_path = "./");

  /// 反初始化日志, 将清理相关资源. 在显式初始化与隐式初始化之间切换时应调用.
  void Uninit();

   /// 获取进程唯一的实例指针.
  static const LogPtr Instance(void);

   /// 获取已初始化的Logger实例指针.
  static const LogPtr GetInitializedLogger();

  /// 设置所在区域, 如"chs".
  static void SetLocale(const std::string &locale);

  /// 使能日志
  void Enable();

  /// 除能日志
  void Disable();

  /// 获取Channel名对应的Logger.
  ysos_channel_log_type& GetChannelLogger(const std::string &channel_name);

  /// 获取控制台Logger.
  ysos_console_log_type& GetConsoleLogger();

  /// 对channel做设置,如:其单个日志文件的最大尺寸,该channel对应的文件数量的最大值,是否宽字符等.
  /// 若channel_name为空,则对每个channel做同样的设置.
  void SetChannelInfo(const std::string &channel_name, const std::string &channel_info);

  /// 设置全局日志级别,包括控制台的.
  /// 这将使得仅级别不低于level的记录才可能输出.
  void SetGlobalLogLevel(trivial::severity_level level);

  /// 设置控制台日志级别.
  void SetConsoleLogLevel(trivial::severity_level level);

  /// 使能控制台日志.
  bool EnableConsoleLog(bool console_log_enabled = true);

  /// 设置控制台日志最小化(控制台仅显示来源于控制台的日志).
  void MinimizeConsoleLog(bool console_log_minimized = true);

 private:
  void ClearChannelMap();
  void Clear();
  bool InitConsoleSink();
  template< typename CharT > void AddEachChannelInfoInConfig(std::basic_istream< CharT >& strm);
  static LogPtr s_log_;
  static bool initialized_;
  static bool initialized_ever_;
  static std::string locale_;
  logging::formatter default_formatter_;
  ysos_channel_name_ptr_map_type ysos_channel_map_;
  boost::mutex channel_map_mutex_;
  bool console_log_enabled_;
  bool console_log_minimized_;
  trivial::severity_level global_log_level_;
  trivial::severity_level console_log_level_;
  LogFilterMode log_filter_mode_;
  std::string temp_path_;
  std::string ini_file_path_;
  boost::shared_ptr< sink_t > sink_console_;
  ysos_console_log_type* console_logger_;
};

} //  namespace ysos
#define GET_YSOS_LOGGER                 ysos::Log::Instance

#define YSOS_LOG_SET_LOCALE             ysos::Log::SetLocale
#define YSOS_LOG_INIT                   GET_YSOS_LOGGER()->Init
#define YSOS_LOG_ENABLE                 GET_YSOS_LOGGER()->Enable
#define YSOS_LOG_DISABLE                GET_YSOS_LOGGER()->Disable
#define YSOS_LOG_UNINIT                 GET_YSOS_LOGGER()->Uninit

#define YSOS_LOG_SET_CHANNEL_INFO       GET_YSOS_LOGGER()->SetChannelInfo
#define YSOS_LOG_SET_GLOBAL_LOG_LEVEL   GET_YSOS_LOGGER()->SetGlobalLogLevel
#define YSOS_LOG_SET_CONSOLE_LOG_LEVEL  GET_YSOS_LOGGER()->SetConsoleLogLevel
#define YSOS_LOG_ENABLE_CONSOLE_LOG     GET_YSOS_LOGGER()->EnableConsoleLog
#define YSOS_LOG_MINIMIZE_CONSOLE_LOG   GET_YSOS_LOGGER()->MinimizeConsoleLog

#define GET_INITIALIZED_LOGGER          ysos::Log::GetInitializedLogger
#define GET_TDLOG_MODULE(m)             GET_INITIALIZED_LOGGER()->GetChannelLogger(m)
#define GET_TDLOG_YSOS                  GET_INITIALIZED_LOGGER()->GetChannelLogger("ysos")
#define GET_TDLOG_DAEMON                GET_INITIALIZED_LOGGER()->GetChannelLogger("daemon")
#define GET_TDLOG_CONSOLE               GET_INITIALIZED_LOGGER()->GetConsoleLogger()

#define TDLOG_MODULE_TRACE(m)           BOOST_LOG_SEV(GET_TDLOG_MODULE(m), trivial::trace) << YSOS_MAKE_SCOPE_LOG
#define TDLOG_MODULE_DEBUG(m)           BOOST_LOG_SEV(GET_TDLOG_MODULE(m), trivial::debug) << YSOS_MAKE_SCOPE_LOG
#define TDLOG_MODULE_INFO(m)            BOOST_LOG_SEV(GET_TDLOG_MODULE(m), trivial::info) << YSOS_MAKE_SCOPE_LOG
#define TDLOG_MODULE_WARNING(m)         BOOST_LOG_SEV(GET_TDLOG_MODULE(m), trivial::warning) << YSOS_MAKE_SCOPE_LOG
#define TDLOG_MODULE_ERROR(m)           BOOST_LOG_SEV(GET_TDLOG_MODULE(m), trivial::error) << YSOS_MAKE_SCOPE_LOG
#define TDLOG_MODULE_FATAL(m)           BOOST_LOG_SEV(GET_TDLOG_MODULE(m), trivial::fatal) << YSOS_MAKE_SCOPE_LOG

#define TDLOG_CONSOLE_TRACE             BOOST_LOG_SEV(GET_TDLOG_CONSOLE, trivial::trace)
#define TDLOG_CONSOLE_DEBUG             BOOST_LOG_SEV(GET_TDLOG_CONSOLE, trivial::debug)
#define TDLOG_CONSOLE_INFO              BOOST_LOG_SEV(GET_TDLOG_CONSOLE, trivial::info)
#define TDLOG_CONSOLE_WARNING           BOOST_LOG_SEV(GET_TDLOG_CONSOLE, trivial::warning)
#define TDLOG_CONSOLE_ERROR             BOOST_LOG_SEV(GET_TDLOG_CONSOLE, trivial::error)
#define TDLOG_CONSOLE_FATAL             BOOST_LOG_SEV(GET_TDLOG_CONSOLE, trivial::fatal)

#define TDLOG_YSOS_TRACE                BOOST_LOG_SEV(GET_TDLOG_YSOS, trivial::trace)
#define TDLOG_YSOS_DEBUG                BOOST_LOG_SEV(GET_TDLOG_YSOS, trivial::debug)
#define TDLOG_YSOS_INFO                 BOOST_LOG_SEV(GET_TDLOG_YSOS, trivial::info)
#define TDLOG_YSOS_WARNING              BOOST_LOG_SEV(GET_TDLOG_YSOS, trivial::warning)
#define TDLOG_YSOS_ERROR                BOOST_LOG_SEV(GET_TDLOG_YSOS, trivial::error)
#define TDLOG_YSOS_FATAL                BOOST_LOG_SEV(GET_TDLOG_YSOS, trivial::fatal)

#endif  //  CHP_YSOS_LOG_H          //NOLINT

