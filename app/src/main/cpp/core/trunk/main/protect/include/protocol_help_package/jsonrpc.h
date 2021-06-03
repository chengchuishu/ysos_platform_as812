/*
# jsonrpc.h
# Definition of jsonrpc
# Created on: 2016-09-19 16:01:03
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20160919, created by JinChengZhe
*/

#ifndef PHP_JSON_RPC_H_
#define PHP_JSON_RPC_H_

/// ThirdParty Headers
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
/// Platform Headers
#include "../../../public/include/sys_interface_package/common.h"
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../public/include/sys_interface_package/connectioninterface.h"
#include "../../../public/include/sys_interface_package/protocolinterface.h"
#include "../../../public/include/sys_interface_package/rpcinterface.h"

namespace ysos {

/**
*@brief JsonRPC的具体实现  // NOLINT
*/
class RPCCallbackInterface;
class BaseRPCCallbackImpl;
class JsonRPCConnectionCallbackImpl;
typedef boost::shared_ptr<JsonRPCConnectionCallbackImpl> JsonRPCConnectionCallbackImplPtr;

class JsonRPC;
typedef boost::shared_ptr<JsonRPC> JsonRPCPtr;
class YSOS_EXPORT JsonRPC : public RPCInterface, public BaseInterfaceImpl {
  DECLARE_CREATEINSTANCE(JsonRPC);
  DISALLOW_COPY_AND_ASSIGN(JsonRPC);
  DECLARE_PROTECT_CONSTRUCTOR(JsonRPC);

 public:
  /**
  *@brief 重新定义unique_lock  // NOLINT
  */
  typedef boost::unique_lock<boost::mutex> UniqueLock;
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
  *@brief JSonRPC析构函数  // NOLINT
  */
  virtual ~JsonRPC();
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
  *@brief 请求服务回调函数  // NOLINT
  *@param session_id[Input]： 平台返回的session id  // NOLINT
  *@param service_name[Input]： 服务名  // NOLINT
  *@param service_param[Input]： 服务参数  // NOLINT
  *@return： 无  // NOLINT
  */
  void HandleDoService(const std::string& session_id, const std::string& service_name, const std::string& service_param);
  /**
  *@brief 回复请求服务回调函数  // NOLINT
  *@param session_id[Input]： 平台返回的session id  // NOLINT
  *@param service_name[Input]： 服务名  // NOLINT
  *@param result[Input]： 服务处理结果  // NOLINT
  *@return： 无  // NOLINT
  */
  void HandleOnDispatchMessage(const std::string& session_id, const std::string& service_name, const std::string& result);
  /**
  *@brief 错误处理回调函数  // NOLINT
  *@param session_id[Input]： 平台返回的session id  // NOLINT
  *@param error_param[Input]： 错误信息  // NOLINT
  *@return： 无  // NOLINT
  */
  void HandleError(const std::string& session_id, const std::string& error_param);
  /**
  *@brief 获取流水号  // NOLINT
  *@param serial_number[Input]： 流水号  // NOLINT
  *@return： 无  // NOLINT
  */
  void GetSerialNumber(std::string& serial_number);
  /**
  *@brief 初始化模块参数  // NOLINT
  *@param params[Input]： 初始化参数指针  // NOLINT
  *@return： 无  // NOLINT
  */
  int InitOpenParam(void* params);
  /**
  *@brief 获取模块初始化状态值  // NOLINT
  *@return： 模块初始化状态值  // NOLINT
  */
  bool GetInitStatus() {
    BoostSharedLock lock(shared_mutex_);
    return init_status_;
  }
  /**
  *@brief 设置模块初始化状态值  // NOLINT
  *@param status[Input]： 模块初始化状态值  // NOLINT
  *@return： 无  // NOLINT
  */
  void SetInitStatus(bool status) {
    BoostLockGuard lock(shared_mutex_);
    init_status_ = status;
  }
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
  BaseRPCCallbackImpl* base_rpc_callback_impl_ptr_;  ///< rpc callback指针  // NOLINT
  ConnectionInterfacePtr connection_ptr_;  ///< connection指针  // NOLINT
  JsonRPCConnectionCallbackImplPtr json_rpc_connection_callback_impl_ptr_;  ///< connection callback指针  // NOLINT

  boost::mutex mutex_lock_;  ///< 互斥锁  // NOLINT
  mutable BoostSharedMutex shared_mutex_;  ///< 读写共享锁  // NOLINT
  boost::condition_variable_any condition_vraiable_any_init_;  ///< 初始化用信号量，用于同步操作  // NOLINT
  volatile bool init_status_;  ///< 初始化状态值  // NOLINT
  volatile UINT32 running_status_;  ///< 运行状态标记符  // NOLINT

  BufferPoolInterfacePtr buffer_pool_ptr_;  ///< 内存池指针  // NOLINT
  ProtocolInterfacePtr protocol_ptr_;  ///< 协议接口指针  // NOLINT
  std::string session_id_;  ///< session id，客户端模式下，每次连接服务器将自动返回一个session id/服务器模式下，每次连接将自动生成session id  // NOLINT
  static UINT64 serial_number_;  ///< 流水号  // NOLINT
};

}

#endif
