/*
# platformrpc.h
# Definition of PlatformRPC
# Created on: 2016-09-29 09:23:33
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20160929, created by JinChengZhe
*/

#ifndef SFP_PLATFORM_RPC_H
#define SFP_PLATFORM_RPC_H

/// ThirdParty Headers
#include <boost/shared_ptr.hpp>  // NOLINT
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
/// Platform Headers
#include "../../../public/include/sys_interface_package/common.h"
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../public/include/sys_interface_package/rpcinterface.h"

namespace ysos {

/**
*@brief PlatformRPC的具体实现  // NOLINT
*/
class JsonRPC;
class BaseRPCCallbackImpl;

class PlatformRPC;
typedef boost::shared_ptr<PlatformRPC> PlatformRPCPtr;
class YSOS_EXPORT PlatformRPC : public RPCInterface, public BaseInterfaceImpl {
  DECLARE_CREATEINSTANCE(PlatformRPC);
  DISALLOW_COPY_AND_ASSIGN(PlatformRPC);
  DECLARE_PROTECT_CONSTRUCTOR(PlatformRPC);

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

  /**
  *@brief PlatformRPC析构函数  // NOLINT
  */
  virtual ~PlatformRPC();
  /**
  *@brief 打开并初始化传模块  // NOLINT
  *@param params[Input]： RPCCallback指针  // NOLINT
  *@param session_id_Ptr[Input]： 平台返回的session id指针  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  int Open(RPCCallbackInterface* params, std::string* session_id_Ptr = NULL);
  /**
  *@brief 关闭并释放模块  // NOLINT
  *@param session_id_ptr[Input]： 平台返回的session id指针  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  int Close(const std::string* session_id_ptr = NULL);
  /**
  *@brief 发送请求服务  // NOLINT
  *@param session_id[Input]： 平台返回的session id  // NOLINT
  *@param service_name[Input]： 服务名  // NOLINT
  *@param service_param[Input]： 服务参数  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  int DoService(const std::string& session_id, const std::string& service_name, const std::string& service_param);
  /**
  *@brief 回复请求服务  // NOLINT
  *@param session_id[Input]： 平台返回的session id  // NOLINT
  *@param service_name[Input]： 服务名  // NOLINT
  *@param result[Input]： 服务处理结果  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  int OnDispatchMessage(const std::string& session_id, const std::string& service_name, const std::string& result);

 private:
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
  JsonRPC* json_rpc_ptr_;  ///< json rpc指针，内部调用jsonrpc底层类  // NOLINT
  BaseRPCCallbackImpl* base_rpc_callback_impl_ptr_;  ///< rpc callback指针，用于open参数  // NOLINT
  mutable BoostSharedMutex shared_mutex_;  ///< 读写共享锁  // NOLINT
  volatile UINT32 running_status_;  ///< 运行状态标记符  // NOLINT
};

}

#endif
