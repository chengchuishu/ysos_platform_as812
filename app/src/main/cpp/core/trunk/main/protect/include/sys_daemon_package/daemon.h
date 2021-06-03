/*
# daemon.h
# Definition of Daemon
# Created on: 2017-03-20 15:53:55
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20170320, created by JinChengZhe
*/

#ifndef SDP_DAEMON_H_
#define SDP_DAEMON_H_

/// C++ Standard Headers
#include <map>
/// ThirdParty Headers
#include <boost/thread.hpp>  // NOLINT
#include <boost/function.hpp>  // NOLINT
#include <boost/shared_ptr.hpp>  // NOLINT
#include <boost/thread/shared_mutex.hpp>
/// Platform Headers
#include "../../../public/include/sys_interface_package/common.h"
#include "../../../protect/include/core_help_package/singletontemplate.h"
#include "../../../protect/include/sys_daemon_package/externinterface.h"
#include "../../../protect/include/sys_daemon_package/flashserver.h"
#if _WIN32
#include <Windows.h>
#include <tchar.h>
#endif

using namespace ysos::extern_interface;

namespace ysos {

namespace sys_daemon_package {
/**
*@brief ConnectionListener的具体实现  // NOLINT
*/
class ConnectionListener : public Colleague, public boost::enable_shared_from_this<ConnectionListener> {
 public:
  /**
  *@brief 构造函数  // NOLINT
  *@param mediator[Input]： 中介者指针  // NOLINT
  *@return： 无  // NOLINT
  */
  ConnectionListener(const MediatorPtr& mediator_ptr);
  /**
  *@brief 析构函数  // NOLINT
  *@param 无  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual ~ConnectionListener(void);
  /**
  *@brief 通过中介者发送消息给协同类  // NOLINT
  *@param message[Input]： 消息  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int Send(const ExternMessage& message);
  /**
  *@brief 处理接收到中介者中转的消息  // NOLINT
  *@param message[Input]： 消息  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int Notify(const ExternMessage& message);

 private:
  /**
  *@brief 数值参数  // NOLINT
  */
  enum {
    IS_INITIAL = 0,  ///< 初始状态  // NOLINT
    IS_RUNNING = 1,  ///< 运行状态  // NOLINT
    IS_STOPPED = 2,  ///< 停止状态  // NOLINT
    SERVER_PORT = 6003,  ///< 服务器端口  // NOLINT
  };

  /**
  *@brief 重定义  // NOLINT
  */
  typedef boost::shared_mutex BoostSharedMutex;  ///< 共享mutex  // NOLINT
  typedef boost::shared_lock<BoostSharedMutex> BoostSharedLock;  ///< 共享锁  // NOLINT
  typedef boost::lock_guard<BoostSharedMutex> BoostLockGuard;  ///< 互斥锁  // NOLINT
  typedef std::map<std::string/*session id*/, std::string /*应用名称*/> AppSessionIDMap;  ///< 应用和session id对应表  // NOLINT
  typedef std::map<std::string, std::string>::iterator AppSessionIDMapItr;  ///< 应用和session id对应表迭代器  // NOLINT

  /**
  *@brief 通讯回调函数声明  // NOLINT
  */
  void UserHandleDoService(const char* session_id, const char* service_name, const char* service_param);
  void UserHandleOnDispatchMessage(const char* session_id, const char* service_name, const char* result);
  void UserHandleError(const char* session_id, const char* error_param);
  /**
  *@brief 初始化  // NOLINT
  *@param 无  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int Init(void);
  /**
  *@brief 设置YSOS Log文件路径函数  // NOLINT
  *@param log_properties_file_path[Out]： 生成的日志配置文件路径
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int SetYSOSLogProperty(std::string& log_properties_file_path);
  /**
  *@brief 获取平台日志配置文件路径
  *@param log_template_properties_file_path[Input]： 日志模板配置文件路径
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int GetYSOSLogTemplatePropertiesFilePath(std::string& log_template_properties_file_path);
  /**
  *@brief 生成平台日志模块配置文件路径和文件
  *@param log_template_properties_file_path[Input]： 日志模板配置文件路径
  *@param log_properties_file_path[Out]： 生成的日志配置文件路径
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int CreateYSOSLogPropertiesFile(const std::string& log_template_properties_file_path, std::string& log_properties_file_path);
  /**
  *@brief 设置YSOS 环境变量函数  // NOLINT
  *@param 无  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int SetYSOSEnvironmentVariable();
  /**
  *@brief 获取日志文件中替换的字符  // NOLINT
  *@param alternate_string[Input]： 替换的字符
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int GetYSOSLogPropertiesAlternateString(std::string& alternate_string);
  /**
  *@brief 开启服务器  // NOLINT
  *@param 无 // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  int RunServer(void);
  /**
  *@brief 关闭服务器  // NOLINT
  *@param 无 // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  int StopServer(void);
  /**
  *@brief 启动  // NOLINT
  *@param 无  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int Start(void);
  /**
  *@brief 关闭  // NOLINT
  *@param terminate_all[Input]： 是否杀死所有守护的进程  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int Stop(void);
  /**
  *@brief 获取app名  // NOLINT
  *@param session_id[Input]： session id  // NOLINT
  *@return： 成功返回应用名，失败返回空  // NOLINT
  */
  std::string GetAppName(const std::string& session_id);
  /**
  *@brief 获取app session id  // NOLINT
  *@param app_name[Input]： app名  // NOLINT
  *@return： 成功返回应用名，失败返回空  // NOLINT
  */
  std::string GetAppSessionID(const std::string& app_name);
  /**
  *@brief 处理平台进程请求逻辑  // NOLINT
  *@param session_id[Input]： session id  // NOLINT
  *@param service_name[Input]： 服务名  // NOLINT
  *@param service_param[Input]： 服务参数  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int HandleDoService4YSOS(const std::string& session_id, const std::string& service_name, const std::string& service_param);
  /**
  *@brief 处理更新进程请求逻辑  // NOLINT
  *@param session_id[Input]： session id  // NOLINT
  *@param service_name[Input]： 服务名  // NOLINT
  *@param service_param[Input]： 服务参数  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int HandleDoService4Update(const std::string& session_id, const std::string& service_name, const std::string& service_param);
  /**
  *@brief 向平台进程请求服务  // NOLINT
  *@param service_name[Input]： 服务名  // NOLINT
  *@param service_param[Input]： 服务参数  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int DoService4YSOS(const std::string& service_name, const std::string& service_param);
  /**
  *@brief 向更新进程请求服务  // NOLINT
  *@param service_name[Input]： 服务名  // NOLINT
  *@param service_param[Input]： 服务参数  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int DoService4Update(const std::string& service_name, const std::string& service_param);
  /**
  *@brief 向指定进程请求服务  // NOLINT
  *@param app_name[Input]： 应用名  // NOLINT
  *@param service_name[Input]： 服务名  // NOLINT
  *@param service_param[Input]： 服务参数  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int DoService(const std::string& app_name, const std::string& service_name, const std::string& service_param);
  /**
  *@brief 响应平台进程请求服务处理  // NOLINT
  *@param session_id[Input]： session id  // NOLINT
  *@param service_name[Input]： 服务名  // NOLINT
  *@param result[Input]： 处理结果  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int HandleOnDispatchMessage4YSOS(const std::string& session_id, const std::string& service_name, const std::string& result);
  /**
  *@brief 响应更新进程请求服务处理  // NOLINT
  *@param session_id[Input]： session id  // NOLINT
  *@param service_name[Input]： 服务名  // NOLINT
  *@param result[Input]： 处理结果  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int HandleOnDispatchMessage4Update(const std::string& session_id, const std::string& service_name, const std::string& result);
  /**
  *@brief 响应平台进程请求服务  // NOLINT
  *@param service_name[Input]： 服务名  // NOLINT
  *@param result[Input]： 处理结果  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int OnDispatchMessage4YSOS(const std::string& service_name, const std::string& result);
  /**
  *@brief 响应更新进程请求服务  // NOLINT
  *@param service_name[Input]： 服务名  // NOLINT
  *@param result[Input]： 处理结果  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int OnDispatchMessage4Update(const std::string& service_name, const std::string& result);
  /**
  *@brief 响应进程请求服务  // NOLINT
  *@param app_name[Input]： 应用名  // NOLINT
  *@param service_name[Input]： 服务名  // NOLINT
  *@param result[Input]： 处理结果  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int OnDispatchMessage(const std::string& app_name, const std::string& service_name, const std::string& result);
  /**
  *@brief 平台进程错误处理回调函数  // NOLINT
  *@param session_id[Input]： 平台返回的session id  // NOLINT
  *@param error_param[Input]： 错误信息  // NOLINT
  *@return： 无  // NOLINT
  */
  int HandleError4YSOS(const std::string& session_id, const std::string& error_param);
  /**
  *@brief 更新进程错误处理回调函数  // NOLINT
  *@param session_id[Input]： 平台返回的session id  // NOLINT
  *@param error_param[Input]： 错误信息  // NOLINT
  *@return： 无  // NOLINT
  */
  int HandleError4Update(const std::string& session_id, const std::string& error_param);
  /**
  *@brief 获取运行状态  // NOLINT
  *@param 无 // NOLINT
  *@return： 运行返回IS_RUNNING，否则返回IS_STOPPED  // NOLINT
  */
  int GetRunningStatus() {
    BoostSharedLock lock(shared_mutex_);
    return running_status_;
  }
  /**
  *@brief 设置运行状态  // NOLINT
  *@param status[Input]： 状态  // NOLINT
  *@return： 无  // NOLINT
  */
  void SetRunningStatus(const UINT32 status = 0) {
    BoostLockGuard lock(shared_mutex_);
    running_status_ = status;
  }

 private:
  volatile UINT32 running_status_;  ///< 是否是运行状态 1 表示运行 0 表示没运行  // NOLINT
  mutable BoostSharedMutex shared_mutex_;  ///< 共享锁
  static ConnectionListener* this_ptr_;  ///< 自身指针
  AppSessionIDMap app_session_id_map_;  ///< 应用和session id对应表  // NOLINT
};
typedef boost::shared_ptr<ConnectionListener> ConnectionListenerPtr;

//////////////////////////////////////////////////////////////////////////////////////////////////////////**/
/**
*@brief AppSessionIDMapValueFinder的具体实现，辅助类  // NOLINT
*/
class AppSessionIDMapValueFinder {
 public:
  AppSessionIDMapValueFinder(const std::string& app_name) : app_name_(app_name) {}

  bool operator()(const std::map<std::string, std::string>::value_type &pair) {
    return pair.second == app_name_;
  }

 private:
  const std::string& app_name_;
};

/**
 *@brief ProcessMonitor的具体实现  // NOLINT
 */
class ProcessMonitor : public Colleague, public boost::enable_shared_from_this<ProcessMonitor> {
 public:
  /**
  *@brief 构造函数  // NOLINT
  *@param mediator_ptr[Input]： 中介者指针  // NOLINT
  *@return： 无  // NOLINT
  */
  ProcessMonitor(const MediatorPtr& mediator_ptr);
  /**
  *@brief 析构函数  // NOLINT
  *@param 无  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual ~ProcessMonitor(void);
  /**
  *@brief 通过中介者发送消息给协同类  // NOLINT
  *@param message[Input]： 消息  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int Send(const ExternMessage& message);
  /**
  *@brief 处理接收到中介者中转的消息  // NOLINT
  *@param message[Input]： 消息  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int Notify(const ExternMessage& message);

 private:
  /**
  *@brief 进程信息结构体  // NOLINT
  */
  typedef struct ProcessInfo {
    std::string tag;  ///< 进程标识
    std::string dependency;  ///< 依赖性
    std::string path;  ///< 文件路径
    std::string command_line;  ///< 命令行参数
    std::string delay_time;  ///< 延迟时间
    std::string hide_window;  ///< 隐藏window框
    std::string call_times;  ///< 重新拉起次数 空值则无限拉起
    std::string type;  ///< 进程种类 -1 一般进程 0 ui进程 1主进程
    unsigned failed_call_times;  ///< 进程启动失败次数

    ProcessInfo()
      : delay_time("10"),
        hide_window("0"),
        type("-1"),
        failed_call_times(0) {
    }

    ~ProcessInfo() {
      tag.clear();
      dependency.clear();
      path.clear();
      command_line.clear();
      delay_time.clear();
      hide_window.clear();
      call_times.clear();
      type.clear();
      failed_call_times = 0;
    }

    ProcessInfo& operator=(const ProcessInfo& new_process_info) {
      if (this == &new_process_info) {
        return (*this);
      }

      this->tag = new_process_info.tag;
      this->dependency = new_process_info.dependency;
      this->path = new_process_info.path;
      this->command_line = new_process_info.command_line;
      this->delay_time = new_process_info.delay_time;
      this->hide_window = new_process_info.hide_window;
      this->call_times = new_process_info.call_times;
      this->type = new_process_info.type;
      this->failed_call_times = new_process_info.failed_call_times;

      return (*this);
    }

    void Clear() {
      tag.clear();
      dependency.clear();
      path.clear();
      command_line.clear();
      delay_time.clear();
      hide_window.clear();
      call_times.clear();
      type.clear();
      failed_call_times = 0;
    }
  } *ProcessInfoPtr;

  /**
  *@brief 数值参数  // NOLINT
  */
  enum {
    IS_INITIAL = 0,  ///< 初始状态  // NOLINT
    IS_RUNNING = 1,  ///< 运行状态  // NOLINT
    IS_STOPPED = 2,  ///< 停止状态  // NOLINT
    MONITORING = 3,  ///< 监测进程状态  // NOLINT
    STOP_MONITRING = 4,   ///< 停止监测进程状态  // NOLINT
  };

  /**
  *@brief 重定义  // NOLINT
  */
  typedef boost::shared_mutex BoostSharedMutex;  ///< 共享mutex  // NOLINT
  typedef boost::shared_lock<BoostSharedMutex> BoostSharedLock;  ///< 共享锁  // NOLINT
  typedef boost::lock_guard<BoostSharedMutex> BoostLockGuard;  ///< 互斥锁  // NOLINT
  typedef std::map<std::string, ProcessInfo> ProcessInfoMap;  ///< 进程信息map  // NOLINT
  typedef std::map<std::string, ProcessInfo>::iterator ProcessInfoMapIterator;  ///< 进程信息map迭代器  // NOLINT

  int SetProcessInfoMap(const ProcessInfo& process_info);
  /**
  *@brief 初始化  // NOLINT
  *@param conf_file_path[Input]： 配置文件路径  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int Init(const std::string& conf_file_path = "");
  /**
  *@brief 解析配置文件  // NOLINT
  *@param conf_file_path[Input]： 配置文件路径  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int ParseConfFile(const std::string& conf_file_path = "");
  /**
  *@brief 启动进程线程函数  // NOLINT
  *@param 无  // NOLINT
  *@return： 无  // NOLINT
  */
  void RunMonitorThreades(void);
  /**
  *@brief 启动进程线程函数  // NOLINT
  *@param process_info[Input]： 进程信息  // NOLINT
  *@return： 无  // NOLINT
  */
  void RunProcess(ProcessInfo& process_info);
  /**
  *@brief 检测指定依赖进程是否已运行  // NOLINT
  *@param dependency_info[Input]： 依赖进程tag号字符串  // NOLINT
  *@param is_running[Output]： 依赖进程是否都已运行  // NOLINT
  *@param main_process_call_times[Output]： 主进程最大重运行次数  // NOLINT
  *@param main_process_failed_call_times[Output]： 主进程当前运行失败次数  // NOLINT
  *@param main_process_path[Output]： 主进程路径  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int CheckDependencyProcessRunningStatus(const std::string& dependency_info, bool& is_running, unsigned& main_process_call_times, unsigned& main_process_failed_call_times, std::string& main_process_path);
  /**
  *@brief 设置主进程失败运行次数  // NOLINT
  *@param failed_call_times[Input]： 需要设置的失败运行次数 // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int SetMainProcessFailedCallTimes(const unsigned failed_call_times);
  /**
  *@brief 监控线程函数  // NOLINT
  *@param 无 // NOLINT
  *@return： 无  // NOLINT
  */
  void MonitorProcess(void);
  /**
  *@brief 杀死进程  // NOLINT
  *@param process_name[Input]： 要杀死所的进程名  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int TerminateProcess(const std::string& process_name);
  /**
  *@brief 使用cmd.exe隐藏窗口方式运行进程  // NOLINT
  *@param params[Input]： 需要传的参数  // NOLINT
  *@param environment[Input]： 进程环境变量  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int ExecuteProcessWithCMD(const std::string& params, const std::string& environment = "");
  /**
  *@brief 启动  // NOLINT
  *@param 无  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int Start(void);
  /**
  *@brief 关闭  // NOLINT
  *@param terminate_all[Input]： 是否杀死所有守护的进程  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int Stop(void);
  /**
  *@brief 获取是否是停止监测进程  // NOLINT
  *@param 无 // NOLINT
  *@return： 运行返回MONITORING，否则返回STOP_MONITRING  // NOLINT
  */
  int GetMonitoringStatus() {
    BoostSharedLock lock(shared_mutex_);
    return monitoring_status_;
  }
  /**
  *@brief 设置是否是停止监测进程  // NOLINT
  *@param status[Input]： 状态  // NOLINT
  *@return： 无  // NOLINT
  */
  void SetMonitoringStatus(const UINT32 status = 0) {
    BoostLockGuard lock(shared_mutex_);
    monitoring_status_ = status;
  }
  /**
  *@brief 获取运行状态  // NOLINT
  *@param 无 // NOLINT
  *@return： 运行返回IS_RUNNING，否则返回IS_STOPPED  // NOLINT
  */
  int GetRunningStatus() {
    BoostSharedLock lock(shared_mutex_);
    return running_status_;
  }
  /**
  *@brief 设置运行状态  // NOLINT
  *@param status[Input]： 状态  // NOLINT
  *@return： 无  // NOLINT
  */
  void SetRunningStatus(const UINT32 status = 0) {
    BoostLockGuard lock(shared_mutex_);
    running_status_ = status;
  }
  std::string GetExpandEnvironmentVariableString(void);

 private:
  volatile UINT32 running_status_;  ///< 是否是运行状态 1 表示运行 0 表示没运行  // NOLINT
  volatile UINT32 monitoring_status_;  ///< 是否是停止监测进程 1 表示停止监测 0 表示监测  // NOLINT
  mutable BoostSharedMutex shared_mutex_;  ///< 共享锁
  ProcessInfoMap process_info_map_;  ///< 需要监控的进程列表信息
  boost::thread monitor_thread_;  ///< 进程监控线程
  boost::thread_group run_process_thread_group_;  ///< 执行进程线程组

  bool    enable_flash_server_;     /// 使能Flash Server
  UINT32  flash_listen_port_;       /// Flash Server 监听端口,默认843
  std::string listen_address_;      /// Flash Server 监听端口,默认0.0.0.0
  Server      *flash_listen_server_; /// Flash Server

  bool    kill_explorer_;
  std::string kill_explorer_path_;
};
typedef boost::shared_ptr<ProcessMonitor> ProcessMonitorPtr;


/**
 *@brief Daemon的具体实现  // NOLINT
 */
class Daemon;
typedef boost::shared_ptr<Daemon> DaemonPtr;
class Daemon : public Mediator, public boost::enable_shared_from_this<Daemon> {
  DISALLOW_COPY_AND_ASSIGN(Daemon)
  DECLARE_PROTECT_CONSTRUCTOR(Daemon)
  DECLARE_SINGLETON_VARIABLE(Daemon)

 public:
  /**
  *@brief 数值参数  // NOLINT
  */
  enum {
    IS_INITIAL = 0,  ///< 初始状态  // NOLINT
    IS_RUNNING = 1,  ///< 运行状态  // NOLINT
    IS_STOPPED = 2,  ///< 停止状态  // NOLINT
    MSG_START = 101,  ///< 开启消息
    MSG_STOP,  ///< 关闭消息
    MSG_YSOS_IS_STARTED,  ///< 平台进程已开启
    MSG_KILL_PROCESS,  ///< 杀死指定的线程
  };

  typedef boost::shared_mutex BoostSharedMutex;
  typedef boost::shared_lock<BoostSharedMutex> BoostSharedLock;
  typedef boost::lock_guard<BoostSharedMutex> BoostLockGuard;

  /**
  *@brief 析构函数  // NOLINT
  *@param 无  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual ~Daemon(void);
  /**
  *@brief 设置监听连接类指针  // NOLINT
  *@param connection_listener_ptr[Input]： 监听连接类指针  // NOLINT
  *@return： 无  // NOLINT
  */
  void SetConnectionListener(const ConnectionListenerPtr& connection_listener_ptr) {
    connection_listener_ptr_ = connection_listener_ptr;
  }
  /**
  *@brief 设置监测进程类指针  // NOLINT
  *@param process_monitor_ptr[Input]： 监测进程类指针  // NOLINT
  *@return： 无  // NOLINT
  */
  void SetProcessMonitor(const ProcessMonitorPtr& process_monitor_ptr) {
    process_monitor_ptr_ = process_monitor_ptr;
  }
  /**
  *@brief 消息中转，监听连接类会发送给监测进程类，监测进程类会发送给监听连接类  // NOLINT
  *@param message[Input]： 消息  // NOLINT
  *@param colleague_ptr[Input]： 协同类指针  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int Send(const ExternMessage& message, const ColleaguePtr& colleague_ptr);
  /**
  *@brief 广播消息给所有协同类  // NOLINT
  *@param message[Input]： 消息  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int NotifyAll(const ExternMessage& message);
  /**
  *@brief 主动向协同类发送消息 // NOLINT
  *@param message[Input]： 消息  // NOLINT
  *@param colleague_ptr[Input]： 协同类指针  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int Notify(const ExternMessage& message, const ColleaguePtr& colleague_ptr);
  /**
  *@brief 启动守护进程  // NOLINT
  *@param 无  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int Start(void);
  /**
  *@brief 关闭守护进程  // NOLINT
  *@param terminate_all[Input]： 是否杀死所有守护的进程  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int Stop(bool terminate_all = false);

 private:
  /**
  *@brief 获取运行状态  // NOLINT
  *@param 无 // NOLINT
  *@return： 运行返回IS_RUNNING，否则返回IS_STOPPED  // NOLINT
  */
  int GetRunningStatus(void) {
    BoostSharedLock lock(shared_mutex_);
    return running_status_;
  }
  /**
  *@brief 设置运行状态  // NOLINT
  *@param status[Input]： 状态  // NOLINT
  *@return： 无  // NOLINT
  */
  void SetRunningStatus(const UINT32 status = 0) {
    BoostLockGuard lock(shared_mutex_);
    running_status_ = status;
  }

 private:
  volatile UINT32 running_status_;  ///< 是否是运行状态 1 表示运行 0 表示没运行  // NOLINT
  mutable BoostSharedMutex shared_mutex_;  ///< 共享锁  // NOLINT
  ConnectionListenerPtr connection_listener_ptr_;  ///< 监听连接类指针
  ProcessMonitorPtr process_monitor_ptr_;  ///< 监测进程类指针
};

}  /// namespace sys_daemon_package

}  /// namespace ysos

#define GetDaemon ysos::sys_daemon_package::Daemon::Instance

#endif
