/**
 *@file connectioninterface.h
 *@brief Definition of connection
 *@version 0.1
 *@author jinchengzhe
 *@date Created on: 2016-05-30 10:37:55
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef CONNECTION_INTERFACE_H  //NOLINT
#define CONNECTION_INTERFACE_H  //NOLINT

#include "../../../public/include/sys_interface_package/bufferpool.h"
#include "../../../public/include/sys_interface_package/baseinterface.h"
#include <boost/shared_ptr.hpp>

namespace ysos {
class BaseInterface;

/**
 *@brief 传输数据接口，主要用于进程间通讯或者远程通讯，内部的物理接口采用Socket或者进程间通讯机制实现
 */
class YSOS_EXPORT ConnectionInterface : virtual public BaseInterface {
  IMPLEMENT_PUBLIC_EMPTY_DESTRUCTOR(ConnectionInterface)
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
    CONNECTION_INTERFACE_PROPERTY_END
  };

  typedef struct YSOS_EXPORT ConnectionErrorCode {
    int boost_asio_error_code;
    int ysos_error_code;

    ConnectionErrorCode() :
      boost_asio_error_code(0),
      ysos_error_code(0) {
    }

    ~ConnectionErrorCode() {
      boost_asio_error_code = 0;
      ysos_error_code = 0;
    }
  } *ConnectionErrorCodePtr;

  /**
   *@brief 读取数据函数  // NOLINT
   *@param input_buffer_ptr[Input]： 读取数据的缓冲，内部包含实际读取数据长度  // NOLINT
   *@param input_length[Input]： 缓冲长度  // NOLINT
   *@param output_buffer_ptr[Out]： 读取数据的缓冲，内部包含实际读取数据长度  // NOLINT
   *@param out_length[Out]： 缓冲长度  // NOLINT
   *@param context_ptr[Input]： 传输接口的上下文，与具体实现密切相关  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Read(BufferInterfacePtr input_buffer_ptr, int input_length, void* context_ptr = NULL) = 0;

  /**
  *@brief 写入数据函数  // NOLINT
  *@param input_buffer_ptr[Input]： 读取数据的缓冲，内部包含实际读取数据长度  // NOLINT
  *@param input_length[Input]： 缓冲长度  // NOLINT
  *@param output_buffer_ptr[Out]： 读取数据的缓冲，内部包含实际读取数据长度  // NOLINT
  *@param out_length[Out]： 缓冲长度  // NOLINT
  *@param context_ptr[Input]： 传输接口的上下文，与具体实现密切相关  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int Write(BufferInterfacePtr input_buffer_ptr, int input_length, void* context_ptr = NULL) = 0;

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
  *@brief 是否封装数据  // NOLINT
  *@param is_enable[Input]： 是否封装开关量  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void EnableWrap(bool is_enable = true) = 0;

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
  virtual void Close(void *param=NULL) = 0;
};

typedef boost::shared_ptr<ConnectionInterface> ConnectionInterfacePtr;

} // namespace ysos

#endif  // TRANSPORT_INTERFACE_H  //NOLINT
