/*
# rpccallbackinterface.h
# Definition of PlatformRPCCallbackInterface
# Created on: 2016-09-29 09:32:52
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20160929, created by JinChengZhe
*/

#ifndef RPC_CALLBACK_INTERFACE_H
#define RPC_CALLBACK_INTERFACE_H

/// ThirdParty Headers
#include <boost/shared_ptr.hpp>  // NOLINT
#include <boost/function.hpp>  // NOLINT
/// Platform Headers
#include "../../../public/include/sys_interface_package/common.h"  // NOLINT
#include "../../../public/include/sys_interface_package/baseinterface.h"  // NOLINT

namespace ysos {
/**
*@brief RPCCallbackInterface的具体实现  // NOLINT
*/
class RPCCallbackInterface;
typedef boost::shared_ptr<RPCCallbackInterface > RPCCallbackInterfacePtr;

class YSOS_EXPORT RPCCallbackInterface: virtual public BaseInterface {
 public:
  enum RPCMode {
    CLIENT,
    SERVER,
    PlatformSDKModeEnd
  };

  typedef boost::function<void(const std::string&, const std::string&)> ErrorHandler;

  virtual ~RPCCallbackInterface() {};

  /// Attribute Function
  /**
   *@brief 获取RPC的类型
   *@return 返回RPC的类型
   */
  virtual UINT32 GetMode(void) = 0;
  /**
   *@brief 设置RPC的类型
   *@param mode RPC的类型
   *@return 无
   */
  virtual void SetMode(const UINT32 mode) = 0;
  /**
   *@brief 获取RPC的地址
   *@return 返回RPC的地址
   */
  virtual std::string GetAddress(void) = 0;
  /**
   *@brief 设置RPC的地址
   *@param address RPC的地址
   *@return 无
   */
  virtual void SetAddress(const std::string &address) = 0;
  /**
   *@brief 获取RPC的端口
   *@return 返回RPC的端口
   */
  virtual UINT32 GetPort(void) = 0;
  /**
   *@brief 设置RPC的端口
   *@param port RPC的端口
   *@return 无
   */
  virtual void SetPort(const UINT32 port) = 0;
  /**
  *@brief 获取RPC的最大连接次数
  *@return 返回RPC的端口
  */
  virtual UINT32 GetMaxReconnectAmount(void) = 0;
  /**
   *@brief 设置RPC的最大连接次数
   *@param max_reconnect_amount RPC的最大连接次数
   *@return 无
   */
  virtual void SetMaxReconnectAmount(const UINT32 max_reconnect_amount) = 0;
  /**
   *@brief 获取RPC的重连时间
   *@return 返回RPC的重连时间
   */
  virtual UINT32 GetReconnectTime(void) = 0;
  /**
   *@brief 设置RPC的重连时间
   *@param reconnect_time RPC的重连时间
   *@return 无
   */
  virtual void SetReconnectTime(const UINT32 reconnect_time) = 0;
  /**
  *@brief 获取RPC的读写buffer大小
  *@return 返回RPC的读写buffer大小
  */
  virtual UINT32 GetBufferLength(void) = 0;
  /**
   *@brief 设置RPC的读写buffer大小
   *@param reconnect_time RPC的读写buffer大小
   *@return 无
   */
  virtual void SetBufferLength(const UINT32 buffer_length) = 0;
  /**
  *@brief 获取RPC应用程序的名字
  *@return 返回RPC应用程序的名字
  */
  virtual std::string GetApplicationName(void) = 0;
  /**
   *@brief 设置RPC应用程序的名字
   *@param application_name RPC应用程序的名字
   *@return 无
   */
  virtual void SetAplicationName(const std::string &application_name) = 0;
  /**
  *@brief 获取RPC应用程序所要用的策略的名字
  *@return 返回RPC应用程序所要用的策略的名字
  */
  virtual std::string GetStrategyName(void) = 0;
  /**
   *@brief 设置RPC应用程序所要用的策略的名字
   *@param strategy_name RPC应用程序所要用的策略的名字
   *@return 无
   */
  virtual void SetStrategyName(const std::string &strategy_name) = 0;
  /**
  *@brief 设置RPC应用程序错误回调函数
  *@param error_handler RPC应用程序错误回调函数
  *@return 无
  */
  virtual void SetErrorHandler(const ErrorHandler error_handler) = 0;

  /// Handle Function
  /**
   *@brief 设置RPC的DoService回调函数
   *@param session_id 该连接的SessionID
   *@param service_name 服务名
   *@param service_param 服务的参数
   *@return 无
   */
  virtual void HandleDoService(const std::string& session_id, const std::string& service_name, const std::string& service_param) = 0;
  /**
   *@brief 设置RPC的OnDispatchMessage回调函数
   *@param session_id 该连接的SessionID
   *@param service_name 服务名
   *@param service_param 服务的参数
   *@return 无
   */
  virtual void HandleOnDispatchMessage(const std::string& session_id, const std::string& service_name, const std::string& result) = 0;
  /**
   *@brief 设置RPC的错误处理回调函数
   *@param session_id 该连接的SessionID
   *@param error_param 错误参数
   *@return 无
   */
  virtual void HandleError(const std::string& session_id, const std::string& error_param) = 0;

 protected:
  ErrorHandler error_handler_;  ///< RPC应用程序错误回调函数
};

}

#endif