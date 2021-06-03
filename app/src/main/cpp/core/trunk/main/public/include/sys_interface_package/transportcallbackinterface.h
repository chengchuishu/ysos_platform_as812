/*
# transportcallbackinterface.h
# Definition of transportcallbackinterface
# Created on: 2016-06-03 16:43:58
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20160912, created by JinChengZhe
*/

#ifndef TRANSPORT_CALLBACK_INTERFACE_H        //NOLINT
#define TRANSPORT_CALLBACK_INTERFACE_H        //NOLINT

/// ThirdParty Headers
#include <boost/function.hpp>  // NOLINT
#include <boost/shared_ptr.hpp>  // NOLINT
/// Platform Headers
#include "../../../public/include/sys_interface_package/common.h"  // NOLINT
#include "../../../public/include/sys_interface_package/baseinterface.h"
#include "../../../public/include/sys_interface_package/transportinterface.h"  // NOLINT
#include "../../../public/include/sys_interface_package/connectioninterface.h"  // NOLINT

namespace ysos {
/**
 *@brief TransportCallbackInterface的具体实现  // NOLINT
 */
class TransportCallbackInterface;
typedef boost::shared_ptr<TransportCallbackInterface> TransportCallbackInterfacePtr;

class YSOS_EXPORT TransportCallbackInterface: virtual public BaseInterface {
 public:
  /**
  *@brief Transport模式  // NOLINT
  */
  enum TransportMode {
    CLIENT,                ///< 客户端，负责向服务器请求连接
    SERVER,                ///< 服务器端，启动Listen，等待客户端端连接请求
    SERVERTERMINAL,        ///< 客户端请求后，服务器端会产生一个端点与客户端端通信
    TransportModeModeEnd
  };

  virtual ~TransportCallbackInterface() {}
  /**
  *@brief 获取模式  // NOLINT
  *@return： 返回模式  // NOLINT
  */
  virtual UINT32 GetMode(void) = 0;
  /**
  *@brief 设置模式  // NOLINT
  *@param mode[Input]： 模式  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void SetMode(const UINT32 mode) = 0;
  /**
  *@brief 获取地址  // NOLINT
  *@return： 返回地址  // NOLINT
  */
  virtual std::string GetAddress(void) = 0;
  /**
  *@brief 设置地址  // NOLINT
  *@param address[Input]： 地址  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void SetAddress(const std::string &address) = 0;
  /**
  *@brief 获取端口  // NOLINT
  *@return： 返回端口  // NOLINT
  */
  virtual short GetPort(void) = 0;
  /**
  *@brief 设置端口  // NOLINT
  *@param port[Input]： 端口  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void SetPort(const short port) = 0;
  /**
  *@brief 获取是否可读  // NOLINT
  *@return： 返回可读标记  // NOLINT
  */
  virtual bool GetEnableRead(void) = 0;
  /**
  *@brief 设置可读标记  // NOLINT
  *@param enable_read[Input]： 可读标记  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void SetEnableRead(const bool enable_read) = 0;
  /**
  *@brief 获取是否可写  // NOLINT
  *@return： 返回可写标记  // NOLINT
  */
  virtual bool GetEnableWrite(void) = 0;
  /**
  *@brief 设置可写标记  // NOLINT
  *@param enable_write[Input]： 可写标记  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void SetEnableWrite(const bool enable_write) = 0;
  /**
  *@brief 获取Connection指针  // NOLINT
  *@return： Connection指针  // NOLINT
  */
  virtual ConnectionInterface* GetConnectionPtr(void) = 0;
  /**
  *@brief 设置Connection指针  // NOLINT
  *@param connection_ptr[Input]： Connection指针  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void SetConnectionPtr(ConnectionInterface* connection_ptr) = 0;
  /**
  *@brief 接收连接回调函数  // NOLINT
  *@param error_code[Input]： 系统错误码  // NOLINT
  *@param transport_ptr[Input]： transport指针  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleAccept(const boost::system::error_code&, const TransportInterfacePtr&) = 0;
  /**
  *@brief 连接回调函数  // NOLINT
  *@param error_code[Input]： 系统错误码  // NOLINT
  *@param transport_ptr[Input]： transport指针  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleConnect(const boost::system::error_code&, const TransportInterfacePtr&) = 0;
  /**
  *@brief 断开回调函数  // NOLINT
  *@param error_code[Input]： 系统错误码  // NOLINT
  *@param transport_ptr[Input]： transport指针  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleDisconnect(const boost::system::error_code&, const TransportInterfacePtr&) = 0;
  /**
  *@brief 读取回调函数  // NOLINT
  *@param error_code[Input]： 系统错误码  // NOLINT
  *@param bytes_transferred[Input]： 读取字节数  // NOLINT
  *@param buffer_ptr[Input]： 读取buffer指针  // NOLINT
  *@param transport_ptr[Input]： transport指针  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleRead(const boost::system::error_code&, const std::size_t, const BufferInterfacePtr&, const TransportInterfacePtr&) = 0;
  /**
  *@brief 写入回调函数  // NOLINT
  *@param error_code[Input]： 系统错误码  // NOLINT
  *@param bytes_transferred[Input]： 写入字节数  // NOLINT
  *@param buffer_ptr[Input]： 写入buffer指针  // NOLINT
  *@param transport_ptr[Input]： transport指针  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleWrite(const boost::system::error_code&, const std::size_t, const BufferInterfacePtr&, const TransportInterfacePtr&) = 0;
  /**
  *@brief 错误处理回调函数  // NOLINT
  *@param error_code[Input]： 系统错误码  // NOLINT
  *@param transport_ptr[Input]： transport指针  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleError(const boost::system::error_code&, const UINT32, const TransportInterfacePtr&) = 0;
};

}

#endif
