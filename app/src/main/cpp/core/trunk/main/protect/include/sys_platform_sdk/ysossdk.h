/*
# ysossdk.h
# Definition of YSOSSDK
# Created on: 2016-09-29 09:23:33
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20160929, created by JinChengZhe
*/

#ifndef SPS_YSOS_SDK_H
#define SPS_YSOS_SDK_H

/// ThirdParty Headers
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
/// Platform Headers
#include "../../../public/include/sys_interface_package/common.h"
#include "../../../public/include/sys_interface_package/ysossdkinterface.h"
#include "../../../protect/include/core_help_package/singletontemplate.h"

using namespace ysos::ysos_sdk;

namespace ysos {

/**
*@brief YSOS SDK 具体类实现  // NOLINT
*/
class RPCInterface;
class YSOSSDKRPCCallbackImpl;
class YSOS_SDK_EXPORT YSOSSDK {
  DISALLOW_COPY_AND_ASSIGN(YSOSSDK)
  DECLARE_PROTECT_CONSTRUCTOR(YSOSSDK)
  DECLARE_SINGLETON_VARIABLE(YSOSSDK)

 public:
  /**
  *@brief 重新定义读写共享锁  // NOLINT
  */
  typedef boost::shared_mutex BoostSharedMutex;
  typedef boost::shared_lock<BoostSharedMutex> BoostSharedLock;
  typedef boost::lock_guard<BoostSharedMutex> BoostLockGuard;
  /**
  *@brief 模块运行状态  // NOLINT
  */
  enum {
    IS_INITIAL = 0,  ///< 初始状态  // NOLINT
    IS_RUNNING = 1,  ///< 运行状态  // NOLINT
    IS_STOPPED = 2,  ///< 停止状态  // NOLINT
  };

  virtual ~YSOSSDK();
  /**
  *@brief 打开并初始化传模块  // NOLINT
  *@param param[Input]： 输入的配置参数  // NOLINT
  *@param session_id[Input]： 平台返回的session id  // NOLINT
  *@return： 成功返回0，失败返回相应错误值-1  // NOLINT
  */
  int Open(void* params, char* session_id = 0);
  /**
  *@brief 关闭并释放模块  // NOLINT
  *@param session_id[Input]： 平台返回的session id  // NOLINT
  *@return： 成功返回0，失败返回相应错误值-1  // NOLINT
  */
  int Close(const char* session_id = 0);
  /**
  *@brief 发送请求服务  // NOLINT
  *@param session_id[Input]： 平台返回的session id  // NOLINT
  *@param service_name[Input]： 服务名  // NOLINT
  *@param service_param[Input]： 服务参数  // NOLINT
  *@return： 成功返回0，失败返回相应错误值-1  // NOLINT
  */
  int DoService(const char* session_id, const char* service_name, const char* service_param);
  /**
  *@brief 回复请求服务  // NOLINT
  *@param session_id[Input]： 平台返回的session id  // NOLINT
  *@param service_name[Input]： 服务名  // NOLINT
  *@param result[Input]： 服务处理结果  // NOLINT
  *@return： 成功返回0，失败返回相应错误值-1  // NOLINT
  */
  int OnDispatchMessage(const char* session_id, const char* service_name, const char* result);
  /**
  *@brief 设置YSOSSDK log file path
  *@param log_file_path_ptr[Input]： 日志文件路径
  *@return： 成功返回true，失败返回相应错误值false
  */
  bool InitYSOSSDKLogger(const char* log_file_path_ptr);
 private:
  /**
  *@brief 请求服务回调函数  // NOLINT
  *@param session_id[Input]： 平台返回的session id  // NOLINT
  *@param service_name[Input]： 服务名  // NOLINT
  *@param service_param[Input]： 服务参数  // NOLINT
  *@return： 无
  */
  void HandleDoService(const char* session_id, const char* service_name, const char* service_param);
  /**
  *@brief 回复请求服务回调函数  // NOLINT
  *@param session_id[Input]： 平台返回的session id  // NOLINT
  *@param service_name[Input]： 服务名  // NOLINT
  *@param result[Input]： 服务处理结果  // NOLINT
  *@return： 无  // NOLINT
  */
  void HandleOnDispatchMessage(const char* session_id, const char* service_name, const char* result);
  /**
  *@brief 错误处理回调函数  // NOLINT
  *@param session_id[Input]： 平台返回的session id  // NOLINT
  *@param error_param[Input]： 错误信息  // NOLINT
  *@return： 无  // NOLINT
  */
  void HandleError(const char* session_id, const char* error_param);
  /**
  *@brief 尝试重新连接  // NOLINT
  *@param param[Input]： 输入的配置参数  // NOLINT
  *@return： 无  // NOLINT
  */
  void RetryConnect(void* params);
  /**
  *@brief 获取模块运行状态值  // NOLINT
  *@return： 模块运行状态值  // NOLINT
  */
  UINT32 GetRunningStatus() {
    BoostSharedLock lock(shared_mutex_);
    return running_status_;
  }
  /**
  *@brief 设置模块运行状态值  // NOLINT
  *@param status[Input]： 模块运行状态值  // NOLINT
  *@return： 无  // NOLINT
  */
  void SetRunningStatus(const UINT32 status) {
    BoostLockGuard lock(shared_mutex_);
    running_status_ = status;
  }

 private:
  OpenParams* open_params_ptr_;  ///< 打开SDK配置参数指针  // NOLINT
  RPCInterface* rpc_ptr_;  ///< rpc 指针  // NOLINT
  YSOSSDKRPCCallbackImpl* ysos_sdk_rpc_callback_impl_ptr_;  ///< ysos rpc callback 指针  // NOLINT
  std::pair<std::string, std::string>* retry_connect_params_ptr_;  ///< 重连参数  // NOLINT
  unsigned reconnect_amount_counter_;  ///< 最大重连次数计数器  // NOLINT
  mutable BoostSharedMutex shared_mutex_;  ///< 读写共享锁  // NOLINT
  volatile UINT32 running_status_;  ///< 运行状态标记符  // NOLINT
};

}

#define GetYSOSSDK ysos::YSOSSDK::Instance

#endif
