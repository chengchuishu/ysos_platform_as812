/*
# baserpccallbackimpl.h
# Definition of BaseRPCCallbackImpl
# Created on: 2016-09-29 09:28:02
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20160929, created by JinChengZhe
*/

#ifndef PHP_BASE_RPC_CALLBACK_IMPL_H
#define PHP_BASE_RPC_CALLBACK_IMPL_H

/// ThirdParty Headers
#include <boost/function.hpp>  // NOLINT
#include <boost/shared_ptr.hpp>  // NOLINT
/// Platform Headers
#include "../../../public/include/sys_interface_package/common.h"
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../public/include/sys_interface_package/rpccallbackinterface.h"

namespace ysos {

#define BASE_RPC_CALLBACK_ADDRESS                   "127.0.0.1"
#define BASE_RPC_CALLBACK_PORT                      6002
#define BASE_RPC_CALLBACK_MAX_RECONNECT_AMOUNT      3
#define BASE_RPC_CALLBACK_RECONNECT_TIME            60
#define BASE_RPC_CALLBACK_BUFFER_LENGTH             8192


/**
 *@brief BaseRPCCallbackImpl的具体实现
 */
class BaseRPCCallbackImpl;
typedef boost::shared_ptr<BaseRPCCallbackImpl> BaseRPCCallbackImplPtr;

class YSOS_EXPORT BaseRPCCallbackImpl : public RPCCallbackInterface, public BaseInterfaceImpl {
  DECLARE_CREATEINSTANCE(BaseRPCCallbackImpl);
  DISALLOW_COPY_AND_ASSIGN(BaseRPCCallbackImpl);
  DECLARE_PROTECT_CONSTRUCTOR(BaseRPCCallbackImpl);

 public:
  typedef struct YSOS_EXPORT RPCCallbackErrorParam {
    int error_code;
    std::string callback_type;
    std::string description;

    RPCCallbackErrorParam() :
      error_code(0),
      callback_type(""),
      description("") {
    }

    ~RPCCallbackErrorParam() {
      error_code = 0;
      callback_type.clear();
      description.clear();
    }

    bool ToString(std::string& to_string);
    bool FromString(const std::string& from_string);
  } *ErrorParamPtr;

  virtual ~BaseRPCCallbackImpl();

  /// Attribute Function
  /**
   *@brief 获取RPC的类型
   *@return 返回RPC的类型
   */
  virtual UINT32 GetMode(void);
  /**
   *@brief 设置RPC的类型
   *@param mode RPC的类型
   *@return 无
   */
  virtual void SetMode(const UINT32 mode);
  /**
   *@brief 获取RPC的地址
   *@return 返回RPC的地址
   */
  virtual std::string GetAddress(void);
  /**
   *@brief 设置RPC的地址
   *@param address RPC的地址
   *@return 无
   */
  virtual void SetAddress(const std::string &address);
  /**
   *@brief 获取RPC的端口
   *@return 返回RPC的端口
   */
  virtual UINT32 GetPort(void);
  /**
   *@brief 设置RPC的端口
   *@param port RPC的端口
   *@return 无
   */
  virtual void SetPort(const UINT32 port);
  /**
  *@brief 获取RPC的最大连接次数
  *@return 返回RPC的端口
  */
  virtual UINT32 GetMaxReconnectAmount(void);
  /**
   *@brief 设置RPC的最大连接次数
   *@param max_reconnect_amount RPC的最大连接次数
   *@return 无
   */
  virtual void SetMaxReconnectAmount(const UINT32 max_reconnect_amount);
  /**
   *@brief 获取RPC的重连时间
   *@return 返回RPC的重连时间
   */
  virtual UINT32 GetReconnectTime(void);
  /**
   *@brief 设置RPC的重连时间
   *@param reconnect_time RPC的重连时间
   *@return 无
   */
  virtual void SetReconnectTime(const UINT32 reconnect_time);
  /**
  *@brief 获取RPC的读写buffer大小
  *@return 返回RPC的读写buffer大小
  */
  virtual UINT32 GetBufferLength(void);
  /**
   *@brief 设置RPC的读写buffer大小
   *@param reconnect_time RPC的读写buffer大小
   *@return 无
   */
  virtual void SetBufferLength(const UINT32 buffer_length);
  /**
  *@brief 获取RPC应用程序的名字
  *@return 返回RPC应用程序的名字
  */
  virtual std::string GetApplicationName(void);
  /**
   *@brief 设置RPC应用程序的名字
   *@param application_name RPC应用程序的名字
   *@return 无
   */
  virtual void SetAplicationName(const std::string &application_name);
  /**
  *@brief 获取RPC应用程序所要用的策略的名字
  *@return 返回RPC应用程序所要用的策略的名字
  */
  virtual std::string GetStrategyName(void);
  /**
   *@brief 设置RPC应用程序所要用的策略的名字
   *@param strategy_name RPC应用程序所要用的策略的名字
   *@return 无
   */
  virtual void SetStrategyName(const std::string &strategy_name);
  /**
  *@brief 设置RPC应用程序错误回调函数
  *@param error_handler RPC应用程序错误回调函数
  *@return 无
  */
  virtual void SetErrorHandler(const ErrorHandler error_handler);

  /// Handle Function
  /**
   *@brief 设置RPC的DoService回调函数
   *@param session_id 该连接的SessionID
   *@param service_name 服务名
   *@param service_param 服务的参数
   *@return 无
   */
  virtual void HandleDoService(const std::string& session_id, const std::string& service_name, const std::string& service_param);
  /**
   *@brief 设置RPC的OnDispatchMessage回调函数
   *@param session_id 该连接的SessionID
   *@param service_name 服务名
   *@param service_param 服务的参数
   *@return 无
   */
  virtual void HandleOnDispatchMessage(const std::string& session_id, const std::string& service_name, const std::string& result);
  /**
   *@brief 设置RPC的错误处理回调函数
   *@param session_id 该连接的SessionID
   *@param error_param 错误参数
   *@return 无
   */
  virtual void HandleError(const std::string& session_id, const std::string& error_param);

 private:
  UINT32 mode_;
  std::string address_;
  UINT32 port_;
  UINT32 max_reconnect_amount_;
  UINT32 reconnect_time_;
  UINT32 buffer_length_;
  std::string application_name_;
  std::string strategy_name_;
};

}

#endif
