/*
# basetransportcallbackimpl.h
# Definition of basetransportcallbackimpl
# Created on: 2016-06-03 16:43:58
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20160912, created by JinChengZhe
*/

#ifndef OHP_BASE_TRANSPORT_CALLBACK_IMPL_H        //NOLINT
#define OHP_BASE_TRANSPORT_CALLBACK_IMPL_H        //NOLINT

/// ThirdParty Headers
#include <boost/function.hpp>  // NOLINT
#include <boost/shared_ptr.hpp>  // NOLINT
/// Platform Headers
#include "../../../public/include/sys_interface_package/common.h"  // NOLINT
#include "../../../public/include/sys_interface_package/transportinterface.h"  // NOLINT
#include "../../../public/include/sys_interface_package/transportcallbackinterface.h"  // NOLINT
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"  // NOLINT

namespace ysos {
/**
 *@brief BaseTransportCallbackImpl的具体实现  // NOLINT
 */
class BaseTransportCallbackImpl;
class ConnectionCallbackInterface;
class ConnectionInterface;
class CommonConnectionImpl;
typedef boost::shared_ptr<BaseTransportCallbackImpl> BaseTransportCallbackImplPtr;

class YSOS_EXPORT BaseTransportCallbackImpl: public TransportCallbackInterface, public BaseInterfaceImpl {
  DECLARE_CREATEINSTANCE(BaseTransportCallbackImpl);
  DISALLOW_COPY_AND_ASSIGN(BaseTransportCallbackImpl);
  DECLARE_PROTECT_CONSTRUCTOR(BaseTransportCallbackImpl);

 public:
  virtual ~BaseTransportCallbackImpl();
  /**
  *@brief 获取模式  // NOLINT
  *@return： 返回模式  // NOLINT
  */
  virtual UINT32 GetMode(void);
  /**
  *@brief 设置模式  // NOLINT
  *@param mode[Input]： 模式  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void SetMode(const UINT32 mode);
  /**
  *@brief 获取地址  // NOLINT
  *@return： 返回地址  // NOLINT
  */
  virtual std::string GetAddress(void);
  /**
  *@brief 设置地址  // NOLINT
  *@param address[Input]： 地址  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void SetAddress(const std::string &address);
  /**
  *@brief 获取端口  // NOLINT
  *@return： 返回端口  // NOLINT
  */
  virtual short GetPort(void);
  /**
  *@brief 设置端口  // NOLINT
  *@param port[Input]： 端口  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void SetPort(const short port);
  /**
  *@brief 获取是否可读  // NOLINT
  *@return： 返回可读标记  // NOLINT
  */
  virtual bool GetEnableRead(void);
  /**
  *@brief 设置可读标记  // NOLINT
  *@param enable_read[Input]： 可读标记  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void SetEnableRead(const bool enable_read);
  /**
  *@brief 获取是否可写  // NOLINT
  *@return： 返回可写标记  // NOLINT
  */
  virtual bool GetEnableWrite(void);
  /**
  *@brief 设置可写标记  // NOLINT
  *@param enable_write[Input]： 可写标记  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void SetEnableWrite(const bool enable_write);
  /**
  *@brief 获取Connection指针  // NOLINT
  *@return： Connection指针  // NOLINT
  */
  virtual ConnectionInterface* GetConnectionPtr(void);
  /**
  *@brief 设置Connection指针  // NOLINT
  *@param connection_ptr[Input]： Connection指针  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void SetConnectionPtr(ConnectionInterface* connection_ptr);
  /**
  *@brief 接收连接回调函数  // NOLINT
  *@param error_code[Input]： 系统错误码  // NOLINT
  *@param transport_ptr[Input]： transport指针  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleAccept(const boost::system::error_code& error_code, const TransportInterfacePtr& transport_ptr);
  /**
  *@brief 连接回调函数  // NOLINT
  *@param error_code[Input]： 系统错误码  // NOLINT
  *@param transport_ptr[Input]： transport指针  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleConnect(const boost::system::error_code& error_code, const TransportInterfacePtr& transport_ptr);
  /**
  *@brief 断开回调函数  // NOLINT
  *@param error_code[Input]： 系统错误码  // NOLINT
  *@param transport_ptr[Input]： transport指针  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleDisconnect(const boost::system::error_code& error_code, const TransportInterfacePtr& transport_ptr);
  /**
  *@brief 读取回调函数  // NOLINT
  *@param error_code[Input]： 系统错误码  // NOLINT
  *@param bytes_transferred[Input]： 读取字节数  // NOLINT
  *@param buffer_ptr[Input]： 读取buffer指针  // NOLINT
  *@param transport_ptr[Input]： transport指针  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleRead(const boost::system::error_code& error_code, const std::size_t length, const BufferInterfacePtr& buffer_ptr, const TransportInterfacePtr& transport_ptr);
  /**
  *@brief 写入回调函数  // NOLINT
  *@param error_code[Input]： 系统错误码  // NOLINT
  *@param bytes_transferred[Input]： 写入字节数  // NOLINT
  *@param buffer_ptr[Input]： 写入buffer指针  // NOLINT
  *@param transport_ptr[Input]： transport指针  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleWrite(const boost::system::error_code& error_code, const std::size_t length, const BufferInterfacePtr& buffer_ptr, const TransportInterfacePtr& transport_ptr);
  /**
  *@brief 错误处理回调函数  // NOLINT
  *@param error_code[Input]： 系统错误码  // NOLINT
  *@param transport_ptr[Input]： transport指针  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleError(const boost::system::error_code& error_code, const UINT32 callback_type, const TransportInterfacePtr& transport_ptr);

 private:
  UINT32 mode_;  ///< 模式
  std::string address_;  ///< 地址
  short port_;  ///< 端口
  bool enable_write_;  ///< 写入标记符
  bool enable_read_;  ///< 读取标记符
  CommonConnectionImpl *connection_ptr_;  ///< Connection指针
};

}

#endif
