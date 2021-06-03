/*
# connectioncallbackinterface.h
# Definition of connectioncallbackinterface
# Created on: 2016-06-03 16:43:58
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20160912, created by JinChengZhe
*/
#ifndef CONNECTION_CALLBACK_INTERFACE_H        //NOLINT
#define CONNECTION_CALLBACK_INTERFACE_H        //NOLINT

/// Platform Headers
#include "../../../public/include/sys_interface_package/common.h"
#include "../../../public/include/sys_interface_package/transportcallbackinterface.h"
#include "../../../public/include/sys_interface_package/baseinterface.h"
#include <boost/shared_ptr.hpp>

namespace ysos {
/**
 *@brief ConnectionCallbackInterface的具体实现  // NOLINT
 */
class ConnectionCallbackInterface;
typedef boost::shared_ptr<ConnectionCallbackInterface> ConnectionCallbackInterfacePtr;

class YSOS_EXPORT ConnectionCallbackInterface: virtual public BaseInterface {
 public:
  /**
  *@brief Connection模式  // NOLINT
  */
  enum ConnectionMode {
    CLIENT,              ///< 客户端，负责向服务器请求连接
    SERVER,              ///< 服务器端，启动Listen，等待客户端端连接请求
    SERVERTERMINAL,      ///< 客户端请求后，服务器端会产生一个端点与客户端端通信
    ConnectionModeEnd
  };

  /**
  *@brief 析构函数  // NOLINT
  *@param 无  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual ~ConnectionCallbackInterface() {}
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
  *@brief 获取是否封装数据  // NOLINT
  *@return： 返回封装数据标记  // NOLINT
  */
  virtual bool GetEnableWrap(void) = 0;
  /**
  *@brief 设置是否封装数据  // NOLINT
  *@param enable_wrap[Input]： 封装数据标记符  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void SetEnableWrap(const bool enable_wrap) = 0;
  /**
  *@brief 获取Transport指针  // NOLINT
  *@return： 返回Transport的指针  // NOLINT
  */
  virtual TransportCallbackInterfacePtr GetTransportCallbackPtr(void) = 0;
  /**
  *@brief 设置Transport指针  // NOLINT
  *@param transport_ptr[Input]： Transport指针  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void SetTransportCallbackPtr(const TransportCallbackInterfacePtr& transport_callback_ptr) = 0;
  /**
  *@brief 获取Buffer的长度  // NOLINT
  *@return： 返回Buffer的长度  // NOLINT
  */
  virtual unsigned int GetBufferLength(void) = 0;
  /**
  *@brief 设置Buffer的长度  // NOLINT
  *@param buffer_length[Input]： buffer的长度  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void SetBufferLength(unsigned int buffer_length) = 0;
  /**
  *@brief 获取Buffer的个数  // NOLINT
  *@return： Buffer的个数  // NOLINT
  */
  virtual unsigned int GetBufferNumber(void) = 0;
  /**
  *@brief 设置Buffer的个数  // NOLINT
  *@param buffer_number[Input]： buffer的个数  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void SetBufferNumber(unsigned int buffer_number) = 0;
  /**
  *@brief 接收连接回调函数  // NOLINT
  *@param error_code[Input]： 系统错误码  // NOLINT
  *@param ConnectionInterfacePtr[Input]： connection指针  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleAccept(const boost::system::error_code&, const ConnectionInterfacePtr&) = 0;
  /**
  *@brief 连接回调函数  // NOLINT
  *@param error_code[Input]： 系统错误码  // NOLINT
  *@param ConnectionInterfacePtr[Input]： connection指针  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleConnect(const boost::system::error_code&, const ConnectionInterfacePtr&) = 0;
  /**
  *@brief 断开回调函数  // NOLINT
  *@param error_code[Input]： 系统错误码  // NOLINT
  *@param ConnectionInterfacePtr[Input]： connection指针  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleDisconnect(const boost::system::error_code&, const ConnectionInterfacePtr&) = 0;
  /**
  *@brief 读取回调函数  // NOLINT
  *@param buffer_ptr[Input]： 读取buffer指针  // NOLINT
  *@param ConnectionInterfacePtr[Input]： connection指针  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleRead(const BufferInterfacePtr&, const ConnectionInterfacePtr&) = 0;
  /**
  *@brief 写入回调函数  // NOLINT
  *@param buffer_ptr[Input]： 写入buffer指针  // NOLINT
  *@param ConnectionInterfacePtr[Input]： connection指针  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleWrite(const BufferInterfacePtr&, const ConnectionInterfacePtr&) = 0;
  /**
  *@brief 错误处理回调函数  // NOLINT
  *@param error_code[Input]： 系统错误码  // NOLINT
  *@param ConnectionInterfacePtr[Input]： connection指针  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleError(const boost::system::error_code&, const UINT32, const ConnectionInterfacePtr&) = 0;
};

}

#endif
