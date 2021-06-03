/*
# update.h
# Definition of Update
# Created on: 2017-10-12 22:07:57
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20171012, created by JinChengZhe
*/

#ifndef SUP_UPDATE_H_
#define SUP_UPDATE_H_

/// C++ Standard Headers
#include <map>
/// ThirdParty Headers
#include <boost/thread.hpp>  // NOLINT
#include <boost/function.hpp>  // NOLINT
#include <boost/shared_ptr.hpp>  // NOLINT
#include <boost/thread/shared_mutex.hpp>
/// Platform Headers
#include <sys_interface_package/common.h>
#include <core_help_package/singletontemplate.h>
#include <sys_daemon_package/externinterface.h>
#include <sys_daemon_package/externutility.h>
#include <sys_update_package/updatehelper.h>
#include <sys_framework_package/platformrpcentity.h>
#include <core_help_package/jsonutility.h>
#if _WIN32
#include <Windows.h>
#include <tchar.h>
#endif

using namespace ysos::extern_interface;
using namespace ysos::extern_utility;

namespace ysos {

namespace sys_update_package {
//////////////////////////////////////////////////////////////////////////
/**
*@brief ConcreteInvoker 类声明
*/
class ConcreteInvoker : public Invoker {
  DISALLOW_COPY_AND_ASSIGN(ConcreteInvoker)

 public:
  ConcreteInvoker(void);
  virtual ~ConcreteInvoker(void);

  virtual int SetCommand(const CommandPtr& command_ptr);
  virtual int ExecuteCommand(const CommandPtr& command_ptr);

 protected:
  virtual int SetCommandEx(const CommandPtr& command_ptr);
  virtual int CancelCommand(const CommandPtr& command_ptr);

 private:
  std::vector<CommandPtr> command_list_;
};
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////
/**
*@brief ConcreteObserver 类声明
*/
class ConcreteObserver : public Observer, public Invoker, public boost::enable_shared_from_this<ConcreteObserver> {
  DISALLOW_COPY_AND_ASSIGN(ConcreteObserver)

 public:
  typedef boost::shared_mutex BoostSharedMutex;
  typedef boost::shared_lock<BoostSharedMutex> BoostSharedLock;
  typedef boost::lock_guard<BoostSharedMutex> BoostLockGuard;

  typedef JobQueue<ExternMessage> SubjectStateQueue;
  typedef boost::shared_ptr<Worker<SubjectStateQueue>> SubjectStateWorkerPtr;

  ConcreteObserver(const SubjectPtr& subject_ptr);
  virtual ~ConcreteObserver(void);
  /**
  *@brief 更新  // NOLINT
  *@param message[Input]： 消息  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  virtual int Update(const ExternMessage& message);
  virtual int SetCommand(const CommandPtr& command_ptr);
  virtual int ExecuteCommand(const ExternMessage& message);
  virtual int Notify(const ExternMessage& message);

 protected:
  /**
  *@brief 初始化  // NOLINT
  *@param 无  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  virtual int Init(void);
  /**
  *@brief 启动通知者  // NOLINT
  *@param 无  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  virtual int Start(void);
  /**
  *@brief 关闭通知者  // NOLINT
  *@param 无  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  virtual int Stop(void);
  /**
  *@brief 处理消息  // NOLINT
  *@param message[Input]： 消息  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleService(const ExternMessage& message);
  /**
  *@brief 处理消息  // NOLINT
  *@param message[Input]： 消息  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleEvent(const ExternMessage& message);
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
  /**
  *@brief 处理消息  // NOLINT
  *@param message[Input]： 消息  // NOLINT
  *@return： 成功返回true，失败返回false  // NOLINT
  */
  bool ProcessSubjectState(const ExternMessage& message);

 protected:
  mutable BoostSharedMutex shared_mutex_;  ///< 共享锁
  CommandPtr command_ptr_;

 private:
  volatile int running_status_;  ///< 是否是运行状态 0 表示初始化状态 1 表示运行状态 2表示停止状态  // NOLINT
  SubjectStateQueue subject_state_queue_;  ///< 通知者状态信息队列  // NOLINT
  SubjectStateWorkerPtr subject_state_worker_ptr_;  ///< 通知者状态信息队列处理函数  // NOLINT
};
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
*@brief Communicator 类声明
*/
class Communicator : public ConcreteObserver {
  DISALLOW_COPY_AND_ASSIGN(Communicator)

 public:
  Communicator(const SubjectPtr& subject_ptr);
  virtual ~Communicator(void);

 protected:
  /**
  *@brief 初始化  // NOLINT
  *@param 无  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  virtual int Init(void);
  /**
  *@brief 启动通知者  // NOLINT
  *@param 无  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  virtual int Start(void);
  /**
  *@brief 关闭通知者  // NOLINT
  *@param 无  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  virtual int Stop(void);
  /**
  *@brief 处理消息  // NOLINT
  *@param message[Input]： 消息  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleService(const ExternMessage& message);
  /**
  *@brief 处理消息  // NOLINT
  *@param message[Input]： 消息  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleEvent(const ExternMessage& message);

 private:
  UPCommunicationPtr upcommunication_ptr_;
  CmdCheckNewVersionPtr cmd_check_new_version_ptr_;
  CmdGetCurrentVersionPtr cmd_get_current_version_ptr_;
  CmdGetAllAvailableVersionPtr cmd_get_all_available_version_ptr_;
};
typedef boost::shared_ptr<Communicator> CommunicatorPtr;
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
*@brief Downloader 类声明
*/
class Downloader : public ConcreteObserver {
  DISALLOW_COPY_AND_ASSIGN(Downloader)

 public:
  typedef UHDownload::FileInfo ModuleInfo;
  typedef UHDownload::DownloadFiles DownloadModules;
  typedef UHDownload::RspDownloadFiles RspDownloadModules;

  Downloader(const SubjectPtr& subject_ptr);
  virtual ~Downloader(void);

 protected:
  /**
  *@brief 初始化  // NOLINT
  *@param 无  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  virtual int Init(void);
  /**
  *@brief 启动通知者  // NOLINT
  *@param 无  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  virtual int Start(void);
  /**
  *@brief 关闭通知者  // NOLINT
  *@param 无  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  virtual int Stop(void);
  /**
  *@brief 处理消息  // NOLINT
  *@param message[Input]： 消息  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleService(const ExternMessage& message);
  /**
  *@brief 处理消息  // NOLINT
  *@param message[Input]： 消息  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleEvent(const ExternMessage& message);

 private:
  UPDownloadPtr updownload_ptr_;
  CmdDownloadFilePtr cmd_download_file_ptr_;
};
typedef boost::shared_ptr<Downloader> DownloaderPtr;
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
*@brief Updater 类声明
*/
class Updater : public ConcreteObserver {
  DISALLOW_COPY_AND_ASSIGN(Updater)

 public:
  typedef UHExecuteCommand::ECProcessDetailInfo ProcessDetailInfo;
  typedef UHExecuteCommand::ECProcessInfo ProcessInfo;

  typedef struct PipeRequestMsg {
    std::vector<std::string> detail_;

    PipeRequestMsg(void) {}
    ~PipeRequestMsg(void) {
      detail_.clear();
    }

    void Clear(void);
    int FromString(const std::string& from_string);
    int ToString(std::string& to_string);
  } *PipeRequestMsgPtr;

  Updater(const SubjectPtr& subject_ptr);
  virtual ~Updater(void);

 protected:
  /**
  *@brief 初始化  // NOLINT
  *@param 无  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  virtual int Init(void);
  /**
  *@brief 启动通知者  // NOLINT
  *@param 无  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  virtual int Start(void);
  /**
  *@brief 关闭通知者  // NOLINT
  *@param 无  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  virtual int Stop(void);
  /**
  *@brief 处理消息  // NOLINT
  *@param message[Input]： 消息  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleService(const ExternMessage& message);
  /**
  *@brief 处理消息  // NOLINT
  *@param message[Input]： 消息  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleEvent(const ExternMessage& message);

 private:
  UPExecuteCommandPtr excute_command_ptr_;
  CmdKillProcessPtr cmd_kill_process_ptr_;
  std::string install_file_path_;  ///< 安装文件路径
};
typedef boost::shared_ptr<Updater> UpdaterPtr;
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
*@brief Rollback 类声明
*/
class Rollback : public ConcreteObserver {
  DISALLOW_COPY_AND_ASSIGN(Rollback)

 public:
  Rollback(const SubjectPtr& subject_ptr);
  virtual ~Rollback(void);

 protected:
  /**
  *@brief 处理消息  // NOLINT
  *@param message[Input]： 消息  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleService(const ExternMessage& message);
  /**
  *@brief 处理消息  // NOLINT
  *@param message[Input]： 消息  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleEvent(const ExternMessage& message);
};
typedef boost::shared_ptr<Rollback> RollbackPtr;
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////
/**
 *@brief ConcreteSubject 类声明  // NOLINT
 */
class ConcreteSubject : public Subject {
  DISALLOW_COPY_AND_ASSIGN(ConcreteSubject)

 public:
  typedef boost::shared_mutex BoostSharedMutex;
  typedef boost::shared_lock<BoostSharedMutex> BoostSharedLock;
  typedef boost::lock_guard<BoostSharedMutex> BoostLockGuard;

  typedef JobQueue<ExternMessage> SubjectStateQueue;
  typedef boost::shared_ptr<Worker<SubjectStateQueue>> SubjectStateWorkerPtr;
  typedef boost::function<void(std::string& event_content)> EventHandler;

  ConcreteSubject(void);
  virtual ~ConcreteSubject(void);
  /**
  *@brief 增加观察者  // NOLINT
  *@param observer_ptr[Input]： 观察者指针  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  virtual int Attach(const ObserverPtr& observer_ptr);
  /**
  *@brief 删除观察者  // NOLINT
  *@param observer_ptr[Input]： 观察者指针  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  virtual int Detach(const ObserverPtr& observer_ptr);
  /**
  *@brief 通知观察者  // NOLINT
  *@param message[Input]： 消息  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  virtual int Notify(const ExternMessage& message);
  /**
  *@brief 初始化  // NOLINT
  *@param event_handler[Input]： 回掉函数  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  virtual int Init(const EventHandler& event_handler = NULL);
  /**
  *@brief 启动通知者  // NOLINT
  *@param 无  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  virtual int Start(void);
  /**
  *@brief 关闭通知者  // NOLINT
  *@param 无  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  virtual int Stop(void);

 protected:
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
  /**
  *@brief 获取初始化状态  // NOLINT
  *@param 无 // NOLINT
  *@return： 未初始化返回IS_UNINITIALIZED，初始化返回IS_INITIALIZED  // NOLINT
  */
  int GetInitStatus() {
    BoostSharedLock lock(shared_mutex_);
    return init_status_;
  }
  /**
  *@brief 设置初始化状态  // NOLINT
  *@param status[Input]： 状态  // NOLINT
  *@return： 无  // NOLINT
  */
  void SetInitStatus(const UINT32 status = 0) {
    BoostLockGuard lock(shared_mutex_);
    init_status_ = status;
  }
  /**
  *@brief 消息处理  // NOLINT
  *@param message[Input]： 消息处理结果  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  virtual void HandleEvent(const ExternMessage& message);

 private:
  /**
  *@brief 处理消息  // NOLINT
  *@param message[Input]： 消息  // NOLINT
  *@return： 成功返回true，失败返回false  // NOLINT
  */
  bool ProcessSubjectState(const ExternMessage& message);

 protected:
  mutable BoostSharedMutex shared_mutex_;  ///< 共享锁
  EventHandler event_handler_;  ///< 事件回调函数

 private:
  volatile int running_status_;  ///< 是否是运行状态 0 表示初始化状态 1 表示运行状态 2表示停止状态  // NOLINT
  volatile int init_status_;  ///< 是否是已初始化状态 0 表示未初始化状态 1 表示初始化状态  // NOLINT
  SubjectStateQueue subject_state_queue_;  ///< 通知者状态信息队列  // NOLINT
  SubjectStateWorkerPtr subject_state_worker_ptr_;  ///< 通知者状态信息队列处理函数  // NOLINT
};
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
 *@brief Update 类声明  // NOLINT
 */
class Update;
typedef boost::shared_ptr<Update> UpdatePtr;
class Update : public ConcreteSubject, public boost::enable_shared_from_this<Update> {
  DISALLOW_COPY_AND_ASSIGN(Update)
  DECLARE_PROTECT_CONSTRUCTOR(Update)
  DECLARE_SINGLETON_VARIABLE(Update)

 public:
  /**
  *@brief 数值参数  // NOLINT
  */
  enum {
    PROP_UI_MODE = 1,  ///< 显示模式 0表示无窗口无托盘图标 1表示窗口模式 2表示窗口托盘模式(暂不支持)  // NOLINT
    PROP_ORG_ID,  ///< 机构号  // NOLINT
    PROP_TERM_ID,  ///< 终端号  // NOLINT
    SUP_PROP_CURRENT_VERSION,  ///< 当前版本  // NOLINT
    SUP_PROP_LAST_VERSION,  ///< 最新版本  // NOLINT
    PROP_CHECK_NEW_VERSION_URL,  ///< 监测是否需要更新信息服务器地址  // NOLINT
    PROP_GET_ALL_AVAILABLE_VERSION_URL,  ///< 获取所有可用版本信息服务器地址  // NOLINT
    PROP_UPLOAD_CURRENT_MODE_VERSION_URL,  ///< 上传当前模块版本服务器地址  // NOLINT
    PROP_UPLOAD_UPDATE_STATUS_VERSION_URL,  ///< 上传当前更新状态服务器地址  // NOLINT
    PROP_UPDATE_SETTINGS,  ///< 设置更新模式  // NOLINT
    SUP_PROP_LOCAL_AVAILABLE_ROLLBACK_VERSION,  ///< 本地可回退版本信息  // NOLINT
    PROP_DOWNLOAD_PROGRESS_CALLBACK,  ///< 下载进度回调函数  // NOLINT
    PROP_CONFIG,  ///< 所有配置  // NOLINT
    PROP_CMD_CHECK_NEW_VERSION,  ///< 获取最新版本命令  // NOLINT
    PROP_CMD_DOWNLOAD_NEW_VERSION,  ///< 下载最新版本命令  // NOLINT
    PROP_CMD_UPDATE_NEW_VERSION,  ///< 更新最新版本命令  // NOLINT
    PROP_CMD_GET_ALL_AVAILABLE_VERSION,  ///< 获取所有可更新版本命令  // NOLINT
    PROP_CMD_SAVE_CONFIG,  ///< 保存配置文件命令  // NOLINT
    SERVER_PORT = 6003,  ///< 服务器端口  // NOLINT
  };

  /**
   *@brief LocalAppInfo的结构体声明  // NOLINT
   */
  typedef struct LocalAppInfo {
    std::string terminal_id_;
    std::string organization_id_;
    std::string tag_no_;

    int FromeString(const std::string& from_string);

    void Clear(void) {
      terminal_id_.clear();
      organization_id_.clear();
      tag_no_.clear();
    }
  } *LocalAppInfoPtr;

  /**
   *@brief UpdateServerURLInfo的结构体声明  // NOLINT
   */
  typedef struct UpdateConfigInfo {
    unsigned ui_mode_;
    std::string org_id_;
    std::string term_id_;
    std::string current_version_;
    std::string last_version_;
    std::string check_new_version_url_;  ///< 检测是否需要更新服务器地址
    std::string get_all_available_version_url_;  ///< 获取所有可用版本服务器地址
    std::string upload_current_module_version_url_;  /// 上传当前模块版本服务器地址
    std::string upload_update_status_url_;  /// 上传当前更新状态服务器地址
    unsigned update_settings_;
    std::string local_available_rollback_version_;
    std::string ini_org_id_;
    std::string ini_term_id_;

    UpdateConfigInfo(void)
      : ui_mode_(0),
        update_settings_(0) {
    }

    ~UpdateConfigInfo(void) {
      ui_mode_ = 0;
      org_id_.clear();
      term_id_.clear();
      current_version_.clear();
      last_version_.clear();
      check_new_version_url_.clear();
      get_all_available_version_url_.clear();
      upload_current_module_version_url_.clear();
      upload_update_status_url_.clear();
      update_settings_ = 0;
      local_available_rollback_version_.clear();
      ini_org_id_.clear();
      ini_term_id_.clear();
    }

    void Clear(void) {
      ui_mode_ = 0;
      org_id_.clear();
      term_id_.clear();
      current_version_.clear();
      last_version_.clear();
      check_new_version_url_.clear();
      get_all_available_version_url_.clear();
      upload_current_module_version_url_.clear();
      upload_update_status_url_.clear();
      update_settings_ = 0;
      local_available_rollback_version_.clear();
      ini_org_id_.clear();
      ini_term_id_.clear();
    }
  } *UpdateConfigInfoPtr;


  /**
   *@brief UpdateModuleInfoHeader的结构体声明  // NOLINT
   */
  typedef struct UpdateModuleInfoHeader {
    std::string code_;
    std::string rsp_msg_;

    int FromString(const std::string& from_string);
    void Clear(void);
  } *UpdateModuleInfoHeaderPtr;

  /**
   *@brief UpdateModuleDetailInfo的结构体声明  // NOLINT
   */
  typedef struct UpdateModuleDetailInfo {
    std::string module_id_;
    std::string tag_no_;
    std::string english_name_;
    std::string chinese_name_;
    std::string version_;
    std::string file_path_;
    std::string modify_time_;
    std::string execute_;
    std::string exe_params_;
    std::string file_size_;
    std::string use_flag_;
    std::string use_flag_ch_;
    std::string field1_;
    std::string field2_;
    std::string md5_;

    UpdateModuleDetailInfo(void) {
    }

    ~UpdateModuleDetailInfo() {
      module_id_.clear();
      tag_no_.clear();
      english_name_.clear();
      chinese_name_.clear();
      version_.clear();
      file_path_.clear();
      modify_time_.clear();
      execute_.clear();
      exe_params_.clear();
      file_size_.clear();
      use_flag_.clear();
      use_flag_ch_.clear();
      field1_.clear();
      field2_.clear();
      md5_.clear();
    }

    void Clear(void);
  } *UpdateModuleDetailInfoPtr;

  /**
   *@brief UpdateModuleInfoBody的结构体声明  // NOLINT
   */
  typedef struct UpdateModuleInfoBody {
    std::string main_tag_;
    std::string result_;
    std::vector<UpdateModuleDetailInfo> modules_;

    int FromString(const std::string& from_string);
    void Clear(void);
  } *UpdateModuleInfoBodyPtr;

  /**
   *@brief UpdateModuleInfo的结构体声明  // NOLINT
   */
  typedef struct UpdateModuleInfo {
    UpdateModuleInfoHeader header_;
    UpdateModuleInfoBody body_;

    int FromString(const std::string& from_string);
    void Clear(void);
  } *UpdateModuleInfoPtr;

  /**
   *@brief AvailableUpdateModuleDetailInfo的结构体声明  // NOLINT
   */
  typedef struct AvailableUpdateModuleDetailInfo {
    std::string id_;
    std::string tag_;
    std::string times_;

    void Clear(void);
  } *AvailableUpdateModuleDetailInfoPtr;

  /**
   *@brief AvailableUpdateModuleInfoBody的结构体声明  // NOLINT
   */
  typedef struct AvailableUpdateModuleInfoBody {
    std::string result_;
    std::string specify_tag_no_;
    std::vector<AvailableUpdateModuleDetailInfo> tags_;
    JsonValue json_value_tags_;

    const JsonValue& GetTags(void) const;
    int FromString(const std::string& from_string);
    void Clear(void);
  } *AvailableUpdateModuleInfoBodyPtr;

  /**
   *@brief AvailableUpdateModuleInfo的结构体声明  // NOLINT
   */
  typedef struct AvailableUpdateModuleInfo {
    UpdateModuleInfoHeader header_;
    AvailableUpdateModuleInfoBody body_;

    int FromString(const std::string& from_string);
    void Clear(void);
  } *AvailableUpdateModuleInfoPtr;

  /**
  *@brief RspCheckNewVersionInfo的结构体声明  // NOLINT
  */
  typedef struct RspCheckNewVersionInfo {
    std::string result_;
    std::string response_message_;
    std::string need_update_;
    std::vector<UpdateModuleDetailInfo> modules_;

    int ToString(std::string& to_string);
    void Clear(void);
  } *RspCheckNewVersionInfoPtr;

  /**
  *@brief RspGetAllAvailableVersionInfo的结构体声明  // NOLINT
  */
  typedef struct RspGetAllAvailableVersionInfo {
    std::string result_;
    std::string response_message_;
    std::vector<AvailableUpdateModuleDetailInfo> tags_;

    int ToString(std::string& to_string);
    void Clear(void);
  } *RspGetAllAvailableVersionInfoPtr;

  /**
  *@brief RspUpdateNewVersionInfo的结构体声明  // NOLINT
  */
  typedef struct RspUpdateNewVersionInfo {
    std::string result_;
    std::string response_message_;

    int ToString(std::string& to_string);
    void Clear(void);
  } *RspUpdateNewVersionInfoPtr;

  /**
  *@brief 析构函数  // NOLINT
  *@param 无  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual ~Update(void);
  /**
  *@brief 初始化  // NOLINT
  *@param event_handler[Input]： 回掉函数  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  virtual int Init(const EventHandler& event_handler = NULL);
  /**
  *@brief 启动更新进程  // NOLINT
  *@param 无  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  virtual int Start(void);
  /**
  *@brief 关闭更新进程  // NOLINT
  *@param 无  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  virtual int Stop(void);
  /**
  *@brief 设置更新模块配置  // NOLINT
  *@param type[Input]： 参数类型  // NOLINT
  *@param value[Input]： 参数值  // NOLINT
  *@return： 成功返回0，失败返回相应错误值  // NOLINT
  */
  int SetProperty(const unsigned type, void* value);

  /**
  *@brief 获取更新模块配置  // NOLINT
  *@param type[Input]： 参数类型  // NOLINT
  *@param value[Input]： 参数值  // NOLINT
  *@return： 成功返回0，失败返回相应错误值  // NOLINT
  */
  int GetProperty(const unsigned type, void* value);

 protected:
  /**
  *@brief 消息处理  // NOLINT
  *@param message[Input]： 消息处理结果  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  virtual void HandleEvent(const ExternMessage& message);

 private:
  /**
  *@brief 通讯回调函数声明  // NOLINT
  */
  void UserHandleDoService(const char* session_id, const char* service_name, const char* service_param);
  void UserHandleOnDispatchMessage(const char* session_id, const char* service_name, const char* result);
  void UserHandleError(const char* session_id, const char* error_param);

  /**
  *@brief 加载更新配置文件  // NOLINT
  *@param 无
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int LoadConfig(void);
  /**
  *@brief 加载更新ini配置文件  // NOLINT
  *@param load_mode[In]： 加载模式，0 读取模式 1 写入模式
  *@param section[In]： ini section
  *@param key[In]： key
  *@param value[In]： key值
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int LoadConfigINI(int load_mode = 0, const std::string& section = "", const std::string& key = "", const std::string& value = "");
  /**
  *@brief 加载更新xml配置文件  // NOLINT
  *@param 无
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int LoadConfigXML(void);
  /**
  *@brief 解析平台xml配置文件  // NOLINT
  *@param xml_file_path[In]： xml文件路径
  *@param xml_tag_name[In]： xml tag名
  *@param xml_tag_value[Out]： xml tag值
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int ParseConfigXML4Data(const std::string& xml_file_path, const std::string& xml_tag_name, std::string& xml_tag_vale);
  /**
  *@brief 解析平台xml配置文件  // NOLINT
  *@param xml_file_path[In]： xml文件路径
  *@param xml_tag_name1[In]： xml tag名
  *@param xml_tag_name2[In]： xml tag名
  *@param xml_tag_value1[Out]： xml tag值
  *@param xml_tag_value2[Out]： xml tag值
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int ParseConfigXML4Conf(const std::string& xml_file_path, const std::string& xml_tag_name1, const std::string& xml_tag_name2, std::string& xml_tag_vale1, std::string& xml_tag_vale2);

  /**
  *@brief 打开通讯类  // NOLINT
  *@param 无 // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int ConnectYSOSDaemon();
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
  *@brief 请求事件通知服务
  *@param service_param[Input]： 结构化服务请求参数  // NOLINT
  *@return 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值  //NOLINT
  */
  int DoEventNotifyService(ReqEventNotifyServiceParam& service_param);
  /**
  *@brief 响应页面自定义事件服务
  *@param result[Input]： 处理结果结构化参数  // NOLINT
  *@return 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值  //NOLINT
  */
  virtual int DoCustomEventDispatch(ResCustomEventServiceParam& result);

 private:
  static char session_id_[YSOS_SDK_SESSION_ID_LENGTH];  ///< 通讯类返回的session id
  CommunicatorPtr communicator_ptr_;  ///< 通讯模块指针
  DownloaderPtr downloader_ptr_;  ///< 下载模块指针
  UpdaterPtr updater_ptr_;  ///< 更新模块指针
  RollbackPtr rollback_ptr_;  ///< 回滚模块指针
  UpdateConfigInfo update_config_info_;  ///< 更新配置信息
  LocalAppInfo local_app_info_;  ///< 本地应用信息
  std::string install_file_path_;  ///< 安装文件路径
};
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
}  /// namespace sys_update_package

}  /// namespace ysos

#define GetUpdate ysos::sys_update_package::Update::Instance

#endif
