/**
  *@file jsonrpcconnectioncallbackimpl.h
  *@brief Implement of jsonrpcconnectioncallbackimpl
  *@version 0.1
  *@author jinchengzhe
  *@date Created on: 2016-09-24 10:30:00
  *@copyright Copyright © 2016 YunShen Technology. All rights reserved.
  * 
  */
#ifndef PHP_JSON_RPC_CONNECTION_CALLBACK_IMPL_H        //NOLINT
#define PHP_JSON_RPC_CONNECTION_CALLBACK_IMPL_H        //NOLINT

/// ThirdParty Headers
#include <boost/function.hpp>  // NOLINT
#include <boost/shared_ptr.hpp>  // NOLINT
/// Platform Headers
#include "../../../public/include/sys_interface_package/common.h"
#include "../../../protect/include/protocol_help_package/commonconnectioncallbackimpl.h"
#include "../../../public/include/sys_interface_package/protocolinterface.h"

namespace ysos {
/**
 *@brief JsonRPCConnectionCallbackImpl的具体实现
 */
class JsonRPCConnectionCallbackImpl;
typedef boost::shared_ptr<JsonRPCConnectionCallbackImpl> JsonRPCConnectionCallbackImplPtr;
class YSOS_EXPORT JsonRPCConnectionCallbackImpl: public CommonConnectionCallbackImpl {
  DECLARE_CREATEINSTANCE(JsonRPCConnectionCallbackImpl);
  DISALLOW_COPY_AND_ASSIGN(JsonRPCConnectionCallbackImpl);
  DECLARE_PROTECT_CONSTRUCTOR(JsonRPCConnectionCallbackImpl);

 public:
  /**
  *@brief 定义函数指针  // NOLINT
  */
  typedef boost::function<void(const std::string& session_id, const std::string& error_param)> ErrorHandler;
  typedef boost::function<void(const std::string& session_id, const std::string& service_name, const std::string& service_param)> DoServiceHandler;
  typedef boost::function<void(const std::string& session_id, const std::string& service_name, const std::string& result)> OnDispatchMessageHandler;

  virtual ~JsonRPCConnectionCallbackImpl();
  /**
  *@brief 初始化参数函数  // NOLINT
  *@param params[Input]： 初始化参数  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int Initialize(void *param=NULL);
  /**
  *@brief 接收连接回调函数  // NOLINT
  *@param error_code[Input]： 系统错误码  // NOLINT
  *@param connection_ptr[Input]： connection指针  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleAccept(const boost::system::error_code& error_code, const ConnectionInterfacePtr& connection_ptr);
  /**
  *@brief 连接回调函数  // NOLINT
  *@param error_code[Input]： 系统错误码  // NOLINT
  *@param connection_ptr[Input]： connection指针  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleConnect(const boost::system::error_code& error_code, const ConnectionInterfacePtr& connection_ptr);
  /**
  *@brief 断开回调函数  // NOLINT
  *@param error_code[Input]： 系统错误码  // NOLINT
  *@param connection_ptr[Input]： connection指针  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleDisconnect(const boost::system::error_code& error_code, const ConnectionInterfacePtr& connection_ptr);
  /**
  *@brief 读取回调函数  // NOLINT
  *@param buffer_ptr[Input]： 读取buffer指针  // NOLINT
  *@param connection_ptr[Input]： connection指针  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleRead(const BufferInterfacePtr& buffer_ptr, const ConnectionInterfacePtr& connection_ptr);
  /**
  *@brief 写入回调函数  // NOLINT
  *@param buffer_ptr[Input]： 写入buffer指针  // NOLINT
  *@param connection_ptr[Input]： connection指针  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleWrite(const BufferInterfacePtr& buffer_ptr, const ConnectionInterfacePtr& connection_ptr);
  /**
  *@brief 错误处理回调函数  // NOLINT
  *@param error_code[Input]： 系统错误码  // NOLINT
  *@param callback_type[Input]： 回调函数类型  // NOLINT
  *@param connection_ptr[Input]： connection指针  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleError(const boost::system::error_code& error_code, const UINT32 callback_type, const ConnectionInterfacePtr& connection_ptr);

 public:
  /**
  *@brief 设置错误处理函数指针  // NOLINT
  *@param error_handler[Input]： 错误函数指针  // NOLINT
  *@return： 无  // NOLINT
  */
  void SetErrorHandler(const ErrorHandler error_handler);
  /**
  *@brief 获取错误处理函数指针  // NOLINT
  *@return： 成功返回错误处理函数指针，失败返回NULL  // NOLINT
  */
  ErrorHandler GetErrorHandler(void);
  /**
  *@brief 设置DoService函数指针  // NOLINT
  *@param do_service_handler[Input]： DoService函数指针  // NOLINT
  *@return： 无  // NOLINT
  */
  void SetDoServiceHandler(const DoServiceHandler do_service_handler);
  /**
  *@brief 获取DoService函数指针  // NOLINT
  *@return： 成功返回DoService函数指针，失败返回NULL  // NOLINT
  */
  DoServiceHandler GetDoServiceHandler(void);
  /**
  *@brief 设置OnDispatchMessage函数指针  // NOLINT
  *@param on_dispatch_message_handler[Input]： OnDispatchMessage函数指针  // NOLINT
  *@return： 无  // NOLINT
  */
  void SetOnDispatchMessageHandler(const OnDispatchMessageHandler on_dispatch_message_handler);
  /**
  *@brief 获取OnDispatchMessage函数指针  // NOLINT
  *@return： 成功返回OnDispatchMessage函数指针，失败返回NULL  // NOLINT
  */
  OnDispatchMessageHandler GetOnDispatchMessageHandler(void);

 private:
  ProtocolInterfacePtr protocol_ptr_;  ///< 协议指针
  ErrorHandler error_handler_;  ///< 错误处理函数指针
  DoServiceHandler do_service_handler_;  ///< DoService函数指针
  OnDispatchMessageHandler on_dispatch_message_handler_;  ///< OnDispatchMessage函数指针
};

}

#endif  //  PHP_JSON_RPC_CONNECTION_CALLBACK_IMPL_H
