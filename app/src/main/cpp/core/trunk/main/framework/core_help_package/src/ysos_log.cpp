/**
  *@file log.cpp
  *@brief Definition of log functions
  *@version 0.1
  *@author XuLanyue
  *@date Created on: 2016-11-05 14:00:00
  *@copyright Copyright © 2016 YunShen Technology. All rights reserved.
    
  */

//  howto:
//  从日志的配置文件中, 获取有哪些日志(目录及文件)需要创建.
//  创建一个列表, 记录已有哪些日志被创建.
//  未包含在日志配置文件中的日志按默认设置创建和维护.
//  统计目录中日志文件的数量, 将多余的文件清理掉.
//  Channel,
//  Target,
//  Destination,
//  FileName="ysos_%Y%m%d%_%H%M%S_%03N.log"
//  Filter="%Channel% contains \"ysos\" and %Severity% > info "
//  Format
//  Asynchronous=false
//  AutoFlush=true
//  Append=true
//  RotationInterval=86400
//  RotationTimePoint="00:00:00"
//  RotationSize=10485760
//  MaxFiles=3
//  ScanForFiles="All"
//  details请设计为一个json value.

#include "../../../public/include/core_help_package/ysos_log.h"

#include <string.h>

#include <boost/make_shared.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/thread.hpp>
#include <boost/locale/generator.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/detail/config.hpp>
#include <boost/log/detail/thread_id.hpp>
#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/channel_feature.hpp>
#include <boost/log/sources/channel_logger.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/severity_feature.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/formatter_parser.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/settings.hpp>
#include <boost/log/utility/setup/from_stream.hpp>

namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;
namespace attrs = boost::log::attributes;
namespace ysos {

void BuildSinkSettingsString(
  std::string &sink_settings_string,
  const std::string &sink_name,
  bool auto_flush,
  logging::trivial::severity_level level);

/// 静态全局变量初始化
bool Log::initialized_ = false;
bool Log::initialized_ever_ = false;
LogPtr Log::s_log_ = NULL;
std::string Log::locale_ = "chs";

Log::Log(const std::string &class_name) {
  console_log_enabled_ = false;
  global_log_level_ = trivial::info;
  console_log_level_ = trivial::info;
  log_filter_mode_ = Log::LOG_FILTER_MODE_RELEASE;
  default_formatter_ =
  expr::format("{\"mdl\":\"%1%\",\"ts\":\"%2%\",\"ut\":\"%3%\",\"pid\":\"%4%\",\"tid\":\"%5%\",\"lvl\":\"%6%\",\"msg\":{%7%}}")
  % expr::attr<std::string>("Channel")
  % expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
  % expr::format_date_time< attrs::timer::value_type >("Uptime", "%O:%M:%S.%f")
  % expr::attr<attrs::current_process_id::value_type >("ProcessID")
  % expr::attr<attrs::current_thread_id::value_type >("ThreadID")
  % expr::attr<logging::trivial::severity_level>("Severity")
  % expr::smessage;

  console_log_minimized_ = true;
  temp_path_ = "./temp/";
  ini_file_path_ = "./";
  console_logger_ = NULL;
}

Log::~Log() {
  Clear();
  delete console_logger_;
  console_logger_ = NULL;
}

/// 初始化日志
/// ini_file_path应以字符'\\'或'/'结尾
void Log::Init(LogFilterMode log_filter_mode, const std::string& ini_file_path) {
  bool init_completed = false;
  do {
    if (initialized_) {
      break;
    }

    /* !!!影响CRT流处理慎用!!! */
    //  所在区域
//    std::locale::global(std::locale(locale_));

    //  sink_tdapp->locked_backend()->set_file_collector(sinks::file::make_collector(
    //    keywords::target = "./log/",                  //  目标文件夹
    //    keywords::max_size = 16 * 1024 * 1024,        //  所有日志加起来的最大大小,
    //    keywords::min_free_space = 100 * 1024 * 1024  //  最低磁盘空间限制
    //    ));

    log_filter_mode_ = log_filter_mode;
    ini_file_path_ = ini_file_path;

    if (!boost::filesystem::exists(temp_path_)
      || !boost::filesystem::is_directory(temp_path_)) {
      boost::filesystem::create_directories(temp_path_);
    }

    if (ini_file_path_.length() > 0
      && (!boost::filesystem::exists(ini_file_path_)
      || !boost::filesystem::is_directory(ini_file_path_))) {
        boost::filesystem::create_directories(ini_file_path_);
    }

    //  确保日志目录存在
    //  统计日志文件的数量

    ////////////////////////////////////////////////////////////////////////////
    //  添加属性
    logging::add_common_attributes();
    logging::core::get()->add_global_attribute("ProcessID", attrs::current_process_id());
    logging::core::get()->add_global_attribute("ThreadID", attrs::current_thread_id());
    logging::core::get()->add_global_attribute("Uptime", attrs::timer());
    //  logging::core::get()->add_thread_attribute("Scope", attrs::named_scope());
    logging::core::get()->add_global_attribute("Scope", attrs::named_scope());
    logging::register_simple_formatter_factory<trivial::severity_level, char>("Severity");
    logging::register_simple_filter_factory<trivial::severity_level, char>("Severity");

    if (log_filter_mode == LOG_FILTER_MODE_TRACE) {
      global_log_level_ = trivial::trace;
    } else if (log_filter_mode == LOG_FILTER_MODE_DEBUG) {
      global_log_level_ = trivial::debug;
    }
    SetGlobalLogLevel(global_log_level_);

    //  属性必须提前注册好才读配置
    std::string ini_file_name = "log_config.ini";
    std::string ini_path_file_name = ini_file_path_ + ini_file_name;
    if (boost::filesystem::exists(ini_path_file_name) && boost::filesystem::is_regular_file(ini_path_file_name)) {
      {
        std::ifstream ini_file(ini_path_file_name);
        AddEachChannelInfoInConfig(ini_file);
      }
      {
        std::ifstream ini_file(ini_path_file_name);
        logging::init_from_stream(ini_file);
      }
    }

    if (!initialized_ever_ && !InitConsoleSink()) {
      initialized_ever_ = true;
      break;
    }

    Enable();

    initialized_ = true;
    init_completed = true;
  } while (false);
  if (!initialized_ && !init_completed) {
    //
  }
}

/// 反初始化日志, 将清理相关资源. 在显式初始化与隐式初始化之间切换时应调用.
void Log::Uninit() {
  Disable();
  logging::core::get()->remove_all_sinks();
  Clear();
  initialized_ = false;
}

/// 获取进程唯一的实例指针.
const LogPtr Log::Instance(void) {
  if (NULL == s_log_) {
    s_log_ = LogPtr(new Log());
    if (s_log_ && !s_log_->initialized_) {
      s_log_->Init(s_log_->log_filter_mode_, s_log_->ini_file_path_);
    }
  } else if (!s_log_->initialized_) {
      s_log_->Init(s_log_->log_filter_mode_, s_log_->ini_file_path_);
  }

  return s_log_;
}

/// 获取已初始化的Logger实例指针.
const LogPtr Log::GetInitializedLogger() {
  if (!Instance()->initialized_) {
    Instance()->Init(Instance()->log_filter_mode_, Instance()->ini_file_path_);
  }
  return Instance();
}

/// 设置所在区域, 如"chs".
void Log::SetLocale(const std::string &locale) {
  if (locale.length() > 0) {
    locale_ = locale;
  }
}

/// 使能日志
void Log::Enable() {
  logging::core::get()->set_logging_enabled(true);
}

/// 除能日志
void Log::Disable() {
  //  logging::core::get()->flush();
  logging::core::get()->set_logging_enabled(false);
}

/// 获取Channel名对应的Logger.
ysos_channel_log_type& Log::GetChannelLogger(const std::string &channel_name) {
  std::string sink_name = channel_name;
  boost::algorithm::to_lower(sink_name);
  //  std::transform(sink_name.begin(), sink_name.end(), sink_name.begin(), ::tolower);
  boost::mutex::scoped_lock lock(channel_map_mutex_);
  ysos_channel_name_ptr_map_type::iterator it = ysos_channel_map_.find(sink_name);
  if (it != ysos_channel_map_.end()) {
    return *(it->second);
  } else {

    std::string sink_settings_string = "";
    bool auto_flush = true; //  false;
    logging::trivial::severity_level level = global_log_level_;
    ysos::BuildSinkSettingsString(sink_settings_string, channel_name, auto_flush, level);

    std::string temp_log_config_file_name = temp_path_ + "log_config_";
    temp_log_config_file_name += channel_name;
    temp_log_config_file_name += ".ini";
    {
      std::ofstream temp_sink_ini_file(temp_log_config_file_name, std::ios::binary);
      temp_sink_ini_file.write(sink_settings_string.c_str(), sink_settings_string.length());
      temp_sink_ini_file.flush();
      temp_sink_ini_file.close();
    }
    {
      std::ifstream temp_sink_ini_file(temp_log_config_file_name);
      logging::init_from_stream(temp_sink_ini_file);
    }
    ysos_channel_log_type* backend_ptr = new ysos_channel_log_type(keywords::channel = sink_name);
    ysos_channel_map_.insert(std::make_pair(channel_name, backend_ptr));
    return *backend_ptr;
  }
}

/// 获取控制台Logger.
ysos_console_log_type& Log::GetConsoleLogger() {
  static ysos_console_log_type& return_value = *console_logger_;
  static ysos_console_log_type* return_value_ptr = console_logger_;
  if (return_value_ptr != console_logger_) {
    return_value = *console_logger_;
    return_value_ptr = console_logger_;
  }
  return return_value;
}

/// 对channel做设置,如:其单个日志文件的最大尺寸,该channel对应的文件数量的最大值,是否宽字符等.
/// 若channel_name为空,则对每个channel做同样的设置.
void Log::SetChannelInfo(const std::string &channel_name, const std::string &channel_info) {
  //
}

/// 设置全局日志级别,包括控制台的.
/// 这将使得仅级别不低于level的记录才可能输出.
void Log::SetGlobalLogLevel(trivial::severity_level level) {
  global_log_level_ = level;
  logging::core::get()->set_filter(expr::attr<trivial::severity_level>("Severity") >= level);
}

/// 设置控制台日志级别.
void Log::SetConsoleLogLevel(trivial::severity_level level) {
  console_log_level_ = level;
  EnableConsoleLog(console_log_enabled_);
}

/// 使能控制台日志.
bool Log::EnableConsoleLog(bool console_log_enabled) {
  bool return_value = false;
  static bool doing = false;
  if (!doing)
  {
    doing = true;

    if (console_log_enabled) {
      sink_console_->reset_filter();
      if (console_log_level_ != trivial::trace) {
        //  sink_console_->set_filter((expr::attr<trivial::severity_level>("Severity") >= console_log_level_));
        sink_console_->set_filter((!console_log_minimized_
          || (console_log_minimized_ && (expr::attr<std::string>("Channel") == "")))
          && expr::attr<trivial::severity_level>("Severity") >= console_log_level_);
      }
      sink_console_->set_formatter(ysos::Log::default_formatter_);
    } else if (console_log_enabled_ != console_log_enabled) {
      sink_console_->reset_filter();
      sink_console_->set_filter((expr::attr<trivial::severity_level>("Severity") > trivial::fatal));
      sink_console_->set_formatter(ysos::Log::default_formatter_);
    }

    console_log_enabled_ = console_log_enabled;
    return_value = true;
    doing = false;
  }

  return return_value;
}

/// 设置控制台日志最小化(控制台仅显示来源于控制台的日志).
void Log::MinimizeConsoleLog(bool console_log_minimized) {
  console_log_minimized_ = console_log_minimized;
}

void Log::Clear() {
  ClearChannelMap();
  //  s_log_ = NULL;
}

void Log::ClearChannelMap() {
  while (ysos_channel_map_.size() != 0) {
    ysos_channel_log_type* channel_ptr = ysos_channel_map_.begin()->second;
    ysos_channel_map_.erase(ysos_channel_map_.begin());
    delete channel_ptr;
  }
}

bool Log::InitConsoleSink() {
  bool return_value = false;
  do {
    if (!console_logger_) {
      console_logger_ = new ysos_console_log_type;
    }
    if (!console_logger_) {
      break;
    }

    boost::shared_ptr< std::basic_ostream< char > > stream_ptr(
      &std::clog, boost::null_deleter());
    if (!stream_ptr) {
      break;
    }
    boost::shared_ptr< backend_t > backend_ptr = boost::make_shared< backend_t >();
    if (!backend_ptr) {
      break;
    }
    backend_ptr->auto_flush(true);
    backend_ptr->add_stream(stream_ptr);
    sink_console_ = boost::make_shared< sink_t >(backend_ptr);
    if (!sink_console_) {
      break;
    }

    sink_console_->set_formatter(default_formatter_);

    /// 为了使得在文件里的日志不在控制台上呈现, 设此过滤器.
    sink_console_->set_filter(
      (!console_log_minimized_
      || (console_log_minimized_ &&
      (expr::attr<std::string>("Channel") == "")))
      && expr::attr<trivial::severity_level>("Severity") >= console_log_level_);

    logging::core::get()->add_sink(sink_console_);
    return_value = true;
  } while (false);
  return return_value;
}

template< typename CharT >
void Log::AddEachChannelInfoInConfig(std::basic_istream< CharT >& strm) {
  typedef CharT char_type;
  typedef std::basic_string< char_type > string_type;

  if (!strm.good())
    BOOST_THROW_EXCEPTION(std::invalid_argument("The input stream for parsing settings is not valid"));
#define TAG_SINK_BGN  "[Sinks."
#define TAG_SINK_END  "]"
#define TAG_TARGET_BGN  "Target=\""
#define TAG_TARGET_END  "\""
  string_type line;
  boost::mutex::scoped_lock lock(channel_map_mutex_);
  while (!strm.eof()) {
    std::getline(strm, line);
    boost::algorithm::trim(line);
    if (boost::algorithm::starts_with(line, TAG_SINK_BGN) && boost::algorithm::ends_with(line, TAG_SINK_END)) {
      std::string channel_name = line.substr(strlen(TAG_SINK_BGN), line.length() - strlen(TAG_SINK_BGN) - strlen(TAG_SINK_END));
      std::string sink_name = channel_name;
      boost::algorithm::to_lower(sink_name);
      ysos_channel_name_ptr_map_type::iterator it = ysos_channel_map_.find(sink_name);
      if (it == ysos_channel_map_.end()) {
        ysos_channel_log_type* backend_ptr = new ysos_channel_log_type(keywords::channel = sink_name);
        ysos_channel_map_.insert(std::make_pair(channel_name, backend_ptr));
      }
    }

    //  日志的路径:
    //  Target="./log/"
    if (boost::algorithm::starts_with(line, TAG_TARGET_BGN) && boost::algorithm::ends_with(line, TAG_TARGET_END)) {
      std::string path = line.substr(strlen(TAG_TARGET_BGN), line.length() - strlen(TAG_TARGET_BGN) - strlen(TAG_TARGET_END));

      if (path.length() > 0
        && (!boost::filesystem::exists(path)
        || !boost::filesystem::is_directory(path))) {
          boost::filesystem::create_directories(path);
      }
    }

    line.clear();
  }
}

void GetCorrespondingLogLevelString(
  std::string& level_name,
  logging::trivial::severity_level level) {
    static const char* const str[] = {
      "trace",
      "debug",
      "info",
      "warning",
      "error",
      "fatal"
    };
    if (level >= trivial::trace
      && static_cast<std::size_t>(level - trivial::trace) < (sizeof(str) / sizeof(*str))) {
        level_name = str[level - trivial::trace];
    } else {
      level_name = str[trivial::info - trivial::trace];
    }
}

void BuildSinkSettingsString(
  std::string &sink_settings_string,
  const std::string &sink_name,
  bool auto_flush,
  logging::trivial::severity_level level) {

  std::string level_name = "";
  GetCorrespondingLogLevelString(level_name, level);

  sink_settings_string = "[Sinks.";
  sink_settings_string += sink_name;
  sink_settings_string += "]\r\n";

  sink_settings_string += "Target=\"./log/\"\r\n";

  sink_settings_string += "Destination=TextFile\r\n";

  sink_settings_string += "FileName=\"";
  sink_settings_string += sink_name;
  sink_settings_string += "_%Y%m%d_%H%M%S_%03N.log\"\r\n";  //  "_%Y%m%d_%H%M%S%f_%03N.log\"\r\n";

  sink_settings_string += "Filter=\"%Channel% contains \\\"";
  sink_settings_string += sink_name;
  sink_settings_string += "\\\" and %Severity% >= ";
  sink_settings_string += level_name;
  sink_settings_string += "\"\r\n";

  sink_settings_string += "Format=\"{";
  sink_settings_string += "\\\"mdl\\\":\\\"%Channel%\\\",";
  sink_settings_string += "\\\"ts\\\":\\\"%TimeStamp%\\\",";
  sink_settings_string += "\\\"ut\\\":\\\"%Uptime%\\\",";
  sink_settings_string += "\\\"pid\\\":\\\"%ProcessID%\\\",";
  sink_settings_string += "\\\"tid\\\":\\\"%ThreadID%\\\",";
  sink_settings_string += "\\\"lvl\\\":\\\"%Severity%\\\",";
  sink_settings_string += "\\\"msg\\\":{%Message%}";
  sink_settings_string += "}\"\r\n";

  sink_settings_string += "Asynchronous=false\r\n";

  sink_settings_string += "AutoFlush=";
  sink_settings_string += (auto_flush ? "true" : "false");
  sink_settings_string += "\r\n";

  sink_settings_string += "Append=true\r\n";

  sink_settings_string += "RotationInterval=86400\r\n";

  sink_settings_string += "RotationTimePoint=\"00:00:00\"\r\n";

  sink_settings_string += "RotationSize=10485760\r\n";

  sink_settings_string += "MaxFiles=3\r\n";

  sink_settings_string += "ScanForFiles=\"All\"\r\n";
}

} //  namespace ysos
