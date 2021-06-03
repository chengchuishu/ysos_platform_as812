/*
# commonconnectioncallbackimpl.h
# Definition of commonconnectioncallbackimpl
# Created on: 2016-06-03 16:43:58
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20160912, created by JinChengZhe
*/
#ifndef OHP_COMMON_CONNECTION_CALLBACK_IMPL_H
#define OHP_COMMON_CONNECTION_CALLBACK_IMPL_H


/// ThirdParty Headers
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
/// Platform Headers
#include "../../../public/include/sys_interface_package/common.h"
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../public/include/sys_interface_package/transportinterface.h"
#include "../../../public/include/sys_interface_package/connectioncallbackinterface.h"

namespace ysos {

/**
 *@brief CommonConnectionCallbackImpl的具体实现  // NOLINT
 */
class CommonConnectionCallbackImpl;
typedef boost::shared_ptr<CommonConnectionCallbackImpl> CommonConnectionCallbackImplPtr;
class YSOS_EXPORT CommonConnectionCallbackImpl: public ConnectionCallbackInterface, public BaseInterfaceImpl {
  DECLARE_CREATEINSTANCE(CommonConnectionCallbackImpl);
  DISALLOW_COPY_AND_ASSIGN(CommonConnectionCallbackImpl);
  DECLARE_PROTECT_CONSTRUCTOR(CommonConnectionCallbackImpl);

 public:
  typedef struct YSOS_EXPORT ConnectionCallbackErrorParam {
    int error_code;
    std::string callback_type;
    std::string description;

    ConnectionCallbackErrorParam() :
      error_code(0),
      callback_type(""),
      description("") {
    }

    ~ConnectionCallbackErrorParam() {
      error_code = 0;
      callback_type.clear();
      description.clear();
    }

    bool ToString(std::string& to_string);
    bool FromString(const std::string& from_string);
  } *ErrorParamPtr;

  /**
  *@brief 析构函数  // NOLINT
  *@param 无  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual ~CommonConnectionCallbackImpl();
  /**
  *@brief 初始化参数函数  // NOLINT
  *@param params[Input]： 初始化参数  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int Initialize(void *param=NULL);
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
  *@brief 获取是否封装数据  // NOLINT
  *@return： 返回封装数据标记  // NOLINT
  */
  virtual bool GetEnableWrap(void);
  /**
  *@brief 设置是否封装数据  // NOLINT
  *@param enable_wrap[Input]： 封装数据标记符  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void SetEnableWrap(const bool enable_wrap);
  /**
  *@brief 获取Transport指针  // NOLINT
  *@return： 返回Transport的指针  // NOLINT
  */
  virtual TransportCallbackInterfacePtr GetTransportCallbackPtr(void);
  /**
  *@brief 设置Transport指针  // NOLINT
  *@param transport_ptr[Input]： Transport指针  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void SetTransportCallbackPtr(const TransportCallbackInterfacePtr& transport_callback_ptr);
  /**
  *@brief 获取Buffer的长度  // NOLINT
  *@return： 返回Buffer的长度  // NOLINT
  */
  virtual unsigned int GetBufferLength(void);
  /**
  *@brief 设置Buffer的长度  // NOLINT
  *@param buffer_length[Input]： buffer的长度  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void SetBufferLength(unsigned int buffer_length);
  /**
  *@brief 获取Buffer的个数  // NOLINT
  *@return： Buffer的个数  // NOLINT
  */
  virtual unsigned int GetBufferNumber(void);
  /**
  *@brief 设置Buffer的个数  // NOLINT
  *@param buffer_number[Input]： buffer的个数  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void SetBufferNumber(unsigned int buffer_number);
  /**
  *@brief 接收连接回调函数  // NOLINT
  *@param error_code[Input]： 系统错误码  // NOLINT
  *@param ConnectionInterfacePtr[Input]： connection指针  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleAccept(const boost::system::error_code& error_code, const ConnectionInterfacePtr& connection_ptr);
  /**
  *@brief 连接回调函数  // NOLINT
  *@param error_code[Input]： 系统错误码  // NOLINT
  *@param ConnectionInterfacePtr[Input]： connection指针  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleConnect(const boost::system::error_code& error_code, const ConnectionInterfacePtr& connection_ptr);
  /**
  *@brief 断开回调函数  // NOLINT
  *@param error_code[Input]： 系统错误码  // NOLINT
  *@param ConnectionInterfacePtr[Input]： connection指针  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleDisconnect(const boost::system::error_code& error_code, const ConnectionInterfacePtr& connection_ptr);
  /**
  *@brief 读取回调函数  // NOLINT
  *@param buffer_ptr[Input]： 读取buffer指针  // NOLINT
  *@param ConnectionInterfacePtr[Input]： connection指针  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleRead(const BufferInterfacePtr& buffer_ptr, const ConnectionInterfacePtr& connection_ptr);
  /**
  *@brief 写入回调函数  // NOLINT
  *@param buffer_ptr[Input]： 写入buffer指针  // NOLINT
  *@param ConnectionInterfacePtr[Input]： connection指针  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleWrite(const BufferInterfacePtr& buffer_ptr, const ConnectionInterfacePtr& connection_ptr);
  /**
  *@brief 错误处理回调函数  // NOLINT
  *@param error_code[Input]： 系统错误码  // NOLINT
  *@param ConnectionInterfacePtr[Input]： connection指针  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleError(const boost::system::error_code& error_code, const UINT32 callback_type, const ConnectionInterfacePtr& connection_ptr);

 private:
  TransportCallbackInterfacePtr transport_callback_ptr_;  ///< transport callback指针

  unsigned int buffer_length_;  ///< buffer的长度
  unsigned int buffer_number_;  ///< buffer的个数

  UINT32 mode_;  ///< 模式
  std::string address_;  ///< 地址
  short port_;  ///< 端口

  bool enable_write_;  ///< 写入标记符
  bool enable_read_;  ///< 读取标记符
  bool enable_wrap_;  ///< 解封标记符
};

}

#endif
