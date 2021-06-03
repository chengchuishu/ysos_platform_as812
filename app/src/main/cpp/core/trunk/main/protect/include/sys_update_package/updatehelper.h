/*
# updatehelper.h
# Definition of UpdateHelper
# Created on: 2017-10-12 22:01:13
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20171012, created by JinChengZhe
*/

#ifndef SUP_UPDATE_HELPER_H_
#define SUP_UPDATE_HELPER_H_

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
#include <sys_interface_package/ysossdkinterface.h>
#include <sys_daemon_package/externinterface.h>
#include <httpclient/httpclient.h>
#include <httpdownload/httpdownload.h>
#if _WIN32
#include <Windows.h>
#include <tchar.h>
#endif

using namespace ysos::extern_interface;

namespace ysos {

namespace sys_update_package {
//////////////////////////////////////////////////////////////////////////
/**
*@brief ConcreteCommand类声明
*/
class ConcreteCommand : public Command {
  DISALLOW_COPY_AND_ASSIGN(ConcreteCommand)

 public:
  typedef boost::function<void(const ExternMessage& message)> ResponseMessageHandler;
  ConcreteCommand(const InvokerPtr invoker_ptr, const ReceiverPtr& receiver_ptr);
  virtual ~ConcreteCommand(void);

  virtual int Execute(const ExternMessage& message);

 protected:
  virtual int Init(void);
  virtual void HandleResponseCommandMessage(const ExternMessage& message);
};
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
*@brief CmdCheckNewVersion类声明
*/
class CmdCheckNewVersion : public ConcreteCommand {
  DISALLOW_COPY_AND_ASSIGN(CmdCheckNewVersion)

 public:
  CmdCheckNewVersion(const InvokerPtr invoker_ptr, const ReceiverPtr& receiver_ptr);
  virtual ~CmdCheckNewVersion(void);

  virtual int Execute(const ExternMessage& message);
};
typedef boost::shared_ptr<CmdCheckNewVersion> CmdCheckNewVersionPtr;
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
*@brief CmdGetCurrentVersion类声明
*/
class CmdGetCurrentVersion : public ConcreteCommand {
  DISALLOW_COPY_AND_ASSIGN(CmdGetCurrentVersion)

 public:
  CmdGetCurrentVersion(const InvokerPtr invoker_ptr, const ReceiverPtr& receiver_ptr);
  virtual ~CmdGetCurrentVersion(void);

  virtual int Execute(const ExternMessage& message);
};
typedef boost::shared_ptr<CmdGetCurrentVersion> CmdGetCurrentVersionPtr;
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
*@brief CmdGetAllAvailableVersion类声明
*/
class CmdGetAllAvailableVersion : public ConcreteCommand {
  DISALLOW_COPY_AND_ASSIGN(CmdGetAllAvailableVersion)

 public:
  CmdGetAllAvailableVersion(const InvokerPtr invoker_ptr, const ReceiverPtr& receiver_ptr);
  virtual ~CmdGetAllAvailableVersion(void);

  virtual int Execute(const ExternMessage& message);
};
typedef boost::shared_ptr<CmdGetAllAvailableVersion> CmdGetAllAvailableVersionPtr;
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
*@brief CmdDownloadFile类声明
*/
class CmdDownloadFile : public ConcreteCommand {
  DISALLOW_COPY_AND_ASSIGN(CmdDownloadFile)

 public:
  CmdDownloadFile(const InvokerPtr invoker_ptr, const ReceiverPtr& receiver_ptr);
  virtual ~CmdDownloadFile(void);

  virtual int Execute(const ExternMessage& message);
};
typedef boost::shared_ptr<CmdDownloadFile> CmdDownloadFilePtr;
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
*@brief CmdKillProcess类声明
*/
class CmdKillProcess : public ConcreteCommand {
  DISALLOW_COPY_AND_ASSIGN(CmdKillProcess)

 public:
  CmdKillProcess(const InvokerPtr invoker_ptr, const ReceiverPtr& receiver_ptr);
  virtual ~CmdKillProcess(void);

  virtual int Execute(const ExternMessage& message);
};
typedef boost::shared_ptr<CmdKillProcess> CmdKillProcessPtr;
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////
/**
*@brief ConcreteReceiver 类声明
*/
class ConcreteReceiver : public Receiver {
  DISALLOW_COPY_AND_ASSIGN(ConcreteReceiver)

 public:
  typedef boost::shared_mutex BoostSharedMutex;
  typedef boost::shared_lock<BoostSharedMutex> BoostSharedLock;
  typedef boost::lock_guard<BoostSharedMutex> BoostLockGuard;

  typedef JobQueue<ExternMessage> CommandMessageQueue;
  typedef boost::shared_ptr<Worker<CommandMessageQueue>> CommandMessageQueueWorkerPtr;

  ConcreteReceiver(void);
  virtual ~ConcreteReceiver(void);

  virtual int Action(const ExternMessage& input_message, ExternMessage& output_message);
  virtual int Action(const ExternMessage& message, const ResponseMessageHandler& response_message_handler);

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
  *@return： 成功返回true，失败返回false  // NOLINT
  */
  virtual void HandleCommandMessage(const ExternMessage& message);
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
  bool ProcessCommadMessage(const ExternMessage& message);

 protected:
  mutable BoostSharedMutex shared_mutex_;  ///< 共享锁
  CommandMessageQueue commad_message_queue_;
  CommandMessageQueueWorkerPtr command_message_queue_woker_ptr_;
  ResponseMessageHandler response_message_handler_;

 private:
  volatile int running_status_;  ///< 是否是运行状态 0 表示初始化状态 1 表示运行状态 2表示停止状态  // NOLINT
};
typedef boost::shared_ptr<ConcreteReceiver> ConcreteReceiverPtr;
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
*@brief UHCommunication 类声明
*/
class UHCommunication : public ConcreteReceiver {
  DISALLOW_COPY_AND_ASSIGN(UHCommunication)

 public:
  typedef boost::shared_ptr<HttpClient> HTTPClientPtr;
  UHCommunication(void);
  virtual ~UHCommunication(void);

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
  *@return： 成功返回true，失败返回false  // NOLINT
  */
  virtual void HandleCommandMessage(const ExternMessage& message);

 private:
  HTTPClientPtr http_client_ptr_;
};
typedef boost::shared_ptr<UHCommunication> UPCommunicationPtr;
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
*@brief UHDownload 类声明
*/
class UHDownload : public ConcreteReceiver {
  DISALLOW_COPY_AND_ASSIGN(UHDownload)

 public:
  enum {
    RETRY_DOWNLOAD_MAX_TIMES = 3,
  };

  /**
  *@brief FileInfo的结构体声明  // NOLINT
  */
  typedef struct FileInfo {
    std::string server_file_path_;
    std::string server_file_size_;
    std::string server_file_md5_;
    std::string local_file_path_;

    void Clear(void);
  } *FileInfoPtr;

  /**
  *@brief DownloadModuleInfo的结构体声明  // NOLINT
  */
  typedef struct DownloadFiles {
    std::string result_;
    std::string response_message_;
    std::vector<FileInfo> files_;

    int ToString(std::string& to_string);
    int FromString(const std::string& from_string);
    void Clear(void);
  } *DownloadFilesPtr;

  /**
  *@brief DownloadProgress的结构体声明  // NOLINT
  */
  typedef struct DownloadProgress {
    std::string remote_file_path_;  ///< 文件远程路径
    std::string local_file_path_;  ///< 文件本地路径
    std::string total_time_;  ///< 已下载花费的总时间，单位为秒
    std::string progress_rate_;  ///< 已下载文件百分比，单位为%
    std::string required_time_;  ///< 下载所需时间，单位为秒
    std::string average_download_speed_;  ///< 平均下载速度，单位为千字节/秒
    std::string total_file_size_;  ///< 文件总大小，单位为千字节
    std::string download_file_size_;  ///< 已下载文件大小，单位为千字节

    int ToString(std::string& to_string);
    int FromString(const std::string& from_string);
    void Clear(void);
  };

  /**
  *@brief FileDownloadProgress的结构体声明  // NOLINT
  */
  typedef struct FileDownloadProgress {
    std::string result_;
    std::string response_message_;
    DownloadProgress download_progress_;

    int ToString(std::string& to_string);
    int FromString(const std::string& from_string);
    void Clear(void);
  } *FileDownloadProgressPtr;

  typedef DownloadFiles RspDownloadFiles;
  typedef boost::shared_ptr<HTTPDownload> HTTPDownloadPtr;

  UHDownload(void);
  virtual ~UHDownload(void);

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
  *@return： 成功返回true，失败返回false  // NOLINT
  */
  virtual void HandleCommandMessage(const ExternMessage& message);

 private:
  void ProgressCallback(const std::string& remote_file_path/*文件远程路径*/,
                        const std::string& local_file_path/*文件本地路径*/,
                        const double& total_time/*已下载花费的总时间，单位为秒*/,
                        const double& progress_rate/*已下载文件百分比，单位为%*/,
                        const double& required_time/*下载所需时间，单位为秒*/,
                        const double& average_download_speed/*平均下载速度，单位为千字节/秒*/,
                        const double& total_file_size/*文件总大小，单位为千字节*/,
                        const double& download_file_size/*已下载文件大小，单位为千字节*/);

 private:
  DownloadFiles download_files_;
  HTTPDownloadPtr http_download_ptr_;
};
typedef boost::shared_ptr<UHDownload> UPDownloadPtr;
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
*@brief UHZip 类声明
*/
class UHZip : public ConcreteReceiver {
  DISALLOW_COPY_AND_ASSIGN(UHZip)

 public:
  UHZip(void);
  virtual ~UHZip(void);

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
  *@return： 成功返回true，失败返回false  // NOLINT
  */
  virtual void HandleCommandMessage(const ExternMessage& message);
};
typedef boost::shared_ptr<UHZip> UPZipPtr;
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
*@brief UHCopyFile 类声明
*/
class UHCopyFile : public ConcreteReceiver {
  DISALLOW_COPY_AND_ASSIGN(UHCopyFile)

 public:
  UHCopyFile(void);
  virtual ~UHCopyFile(void);

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
  *@return： 成功返回true，失败返回false  // NOLINT
  */
  virtual void HandleCommandMessage(const ExternMessage& message);
};
typedef boost::shared_ptr<UHCopyFile> UPCopyFilePtr;
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
*@brief UHCopySettings 类声明
*/
class UHCopySettings : public ConcreteReceiver {
  DISALLOW_COPY_AND_ASSIGN(UHCopySettings)

 public:
  UHCopySettings(void);
  virtual ~UHCopySettings(void);

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
  *@return： 成功返回true，失败返回false  // NOLINT
  */
  virtual void HandleCommandMessage(const ExternMessage& message);
};
typedef boost::shared_ptr<UHCopySettings> UPCopySettingsPtr;
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
*@brief UHExecuteCommand 类声明
*/
class UHExecuteCommand : public ConcreteReceiver {
  DISALLOW_COPY_AND_ASSIGN(UHExecuteCommand)

 public:
  UHExecuteCommand(void);
  virtual ~UHExecuteCommand(void);

  typedef struct ECProcessDetailInfo {
    std::string process_file_path_;
    std::string command_line_;
    std::string expand_environment_string_;
    std::string hide_window_;
    std::string delay_time_;

    void Clear(void);
  } *ECProcessDetailInfoPtr;

  typedef struct ECProcessInfo {
    std::vector<ECProcessDetailInfo> processes_;

    int ToString(std::string& to_string);
    int FromString(const std::string& from_string);
    void Clear(void);
  } *ECProcessInfoPtr;

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
  *@return： 成功返回true，失败返回false  // NOLINT
  */
  virtual void HandleCommandMessage(const ExternMessage& message);
};
typedef boost::shared_ptr<UHExecuteCommand> UPExecuteCommandPtr;
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
}  /// namespace sys_update_package

}  /// namespace ysos

#endif
