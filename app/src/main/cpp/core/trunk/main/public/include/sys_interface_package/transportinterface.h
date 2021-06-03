/**
 *@file transportinterface.h
 *@brief Definition of transport
 *@version 0.1
 *@author Steven.Shi
 *@date Created on: 2016-04-13 08:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef TRANSPORT_INTERFACE_H  //NOLINT
#define TRANSPORT_INTERFACE_H  //NOLINT

#include "../../../public/include/sys_interface_package/common.h"
#include "../../../public/include/sys_interface_package/baseinterface.h"
#include "../../../public/include/sys_interface_package/bufferpool.h"
#include <boost/shared_ptr.hpp>

namespace ysos {
class BaseInterface;

/**
 *@brief 传输数据接口，主要用于进程间通讯或者远程通讯
   内部的物理接口采用Socket或者进程间通讯机制实现
 */
class YSOS_EXPORT TransportInterface : virtual public BaseInterface {
  IMPLEMENT_PUBLIC_EMPTY_DESTRUCTOR(TransportInterface)
 public:

  enum Properties {
    Information = 0,                        ///< Transport Information
    OnAccept,                               ///< On Transport Accept 异步回调，通过SetProperity设置
    OnConnect,                              ///< On Transport Connect 异步回调，通过SetProperity设置
    OnDisconnect,                           ///< On Transport Disconnect 异步回调，通过SetProperity设置
    OnRead,                                 ///< On Read 异步回调，通过SetProperity设置
    OnWrite,                                ///< On Write 异步回调，通过SetProperity设置
    Protocol,                               ///< The protocol needed by this transport
    Mode,                                   ///< 同步/异步模式，通过SetProperity设置
    TRANSPORT_INTERFACE_PROPERTY_END
  };

  /**
   *@brief 读取数据函数  // NOLINT
   *@param buffer_ptr[Input/Output]： 读取数据的缓冲，内部包含实际读取数据长度  // NOLINT
   *@param length[Input]： 缓冲长度，暂时无用  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Read(BufferInterfacePtr buffer_ptr, int length) = 0;

  /**
    *@brief 写入数据函数  // NOLINT
    *@param buffer_ptr[Input/Output]： 写入数据的缓冲，内部包含实际写入数据长度  // NOLINT
    *@param length[Input]： 缓冲长度，暂时无用  // NOLINT
    *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int Write(BufferInterfacePtr buffer_ptr, int length) = 0;

  /**
    *@brief 是否允许写入  // NOLINT
    *@param is_enable[Input]： 允许写入开关量  // NOLINT
    *@return： 无  // NOLINT
  */
  virtual void EnableWrite(bool is_enable = true) = 0;

  /**
    *@brief 是否允许读出  // NOLINT
    *@param is_enable[Input]： 允许读出开关量  // NOLINT
    *@return： 无  // NOLINT
  */
  virtual void EnableRead(bool is_enable = true) = 0;

  /**
  *@brief 打开并初始化传输模块  // NOLINT
  *@param param[Input]： 输入的配置参数  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int Open(void *params) = 0;

  /**
  *@brief 关闭传输模块  // NOLINT
  *@param ： 无  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void Close() = 0;
};

typedef boost::shared_ptr<TransportInterface> TransportInterfacePtr;

} // namespace ysos

#endif  // TRANSPORT_INTERFACE_H  //NOLINT
