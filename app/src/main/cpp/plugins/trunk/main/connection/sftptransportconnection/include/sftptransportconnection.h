/*
# sftptransportconnectionImpl.h
# Definition of sftptransportconnectionImpl
# Created on: 2017-1-18 14:00:00
# Original author: CaiYanli
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
*/

#ifndef  SFTP_TRANSPORT_CONNECTION_IMPL_H
#define SFTP_TRANSPORT_CONNECTION_IMPL_H

/// ThirdParty Headers
#include <boost/shared_ptr.hpp>  // NOLINT
#include <curl/curl.h>
/// Platform Headers
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/common.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/connectioninterface.h"


namespace ysos {
/**
*@brief sftptransportconnectionImpl的具体实现  // NOLINT
*/
class sftptransportconnectionImpl;
typedef boost::shared_ptr<sftptransportconnectionImpl> sftptransportconnectionImplPtr;

class YSOS_EXPORT sftptransportconnectionImpl : public ConnectionInterface, public BaseInterfaceImpl {
  DECLARE_CREATEINSTANCE(sftptransportconnectionImpl);
  DISALLOW_COPY_AND_ASSIGN(sftptransportconnectionImpl);
  DECLARE_PROTECT_CONSTRUCTOR(sftptransportconnectionImpl);

  public:
  /**
  *@brief 析构函数  // NOLINT
  *@param ： 无  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual ~sftptransportconnectionImpl();

  /**
  *@brief 打开并初始化类  // NOLINT
  *@param params[Input]： 初始化参数，函数内部以TeamParamsImpl格式强制转换使用  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int Open(void *params);

  /**
   *@brief 关闭类  // NOLINT
   *@param params[Input]： 当为Server时，param若不为NULL，则是自己建立的一个连接端点，要关闭了  // NOLINT
   *@return： 无  // NOLINT
   */
  virtual void Close(void *param=NULL);

  /**
   *@brief 读取数据函数  // NOLINT
   *@param input_buffer_ptr[Input]： 读取数据的缓冲，内部包含实际读取数据长度  // NOLINT
   *@param input_length[Input]： 缓冲长度  // NOLINT
   *@param output_buffer_ptr[Out]： 读取数据的缓冲，内部包含实际读取数据长度  // NOLINT
   *@param out_length[Out]： 缓冲长度  // NOLINT
   *@param context_ptr[Input]： 传输接口的上下文，与具体实现密切相关  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Read(BufferInterfacePtr input_buffer_ptr, int input_length, void* context_ptr = NULL);

  /**
  *@brief 写入数据函数  // NOLINT
  *@param input_buffer_ptr[Input]： 读取数据的缓冲，内部包含实际读取数据长度  // NOLINT
  *@param input_length[Input]： 缓冲长度  // NOLINT
  *@param output_buffer_ptr[Out]： 读取数据的缓冲，内部包含实际读取数据长度  // NOLINT
  *@param out_length[Out]： 缓冲长度  // NOLINT
  *@param context_ptr[Input]： 传输接口的上下文，与具体实现密切相关  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int Write(BufferInterfacePtr input_buffer_ptr, int input_length, void* context_ptr = NULL);

  /**
    *@brief 是否允许写入  // NOLINT
    *@param is_enable[Input]： 允许写入开关量  // NOLINT
    *@return： 无  // NOLINT
  */
  virtual void EnableWrite(bool is_enable = true);

  /**
    *@brief 是否允许读出  // NOLINT
    *@param is_enable[Input]： 允许读出开关量  // NOLINT
    *@return： 无  // NOLINT
  */
  virtual void EnableRead(bool is_enable = true);

  /**
  *@brief 是否封装数据  // NOLINT
  *@param is_enable[Input]： 是否封装开关量  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void EnableWrap(bool is_enable = true);

 public:
  enum { TIME_OUT = 600};

  /**
  *@brief READFUNCTION回调函数的实现。  // NOLINT
  *@param ptr： 是要获取数据的地址。  // NOLINT
  *@param size： 要读入内容的单字节数。  // NOLINT
  *@param nmemb： 要进行读入size字节的数据项的个数。  // NOLINT
  *@param stream： 目标文件指针。  // NOLINT
  *@return： 返回实际读入的数据项个数count。  // NOLINT
  */
  static  size_t  read_callback(void *ptr, size_t size, size_t nmemb, FILE *stream);

  /**
  *@brief WRITEFUNCTION回调函数的实现。  // NOLINT
  *@param ptr： 是要获取数据的地址。  // NOLINT
  *@param size： 要写入内容的单字节数。  // NOLINT
  *@param nmemb： 要进行写入size字节的数据项的个数。  // NOLINT
  *@param stream： 目标文件指针。  // NOLINT
  *@return： 返回实际写入的数据项个数count。  // NOLINT
  */
  static  size_t  write_callback(void *ptr, size_t size, size_t nmemb, void *stream);

  /**
  *@brief 获取文件大小  // NOLINT
  *@param file_name[Input]： 下载文件名  // NOLINT
  *@return： 返回文件大小  // NOLINT
  */
  static  curl_off_t  get_file_size(FILE *file);

 private:
  bool enable_write_;  ///< 下载标记符
  bool enable_read_;  ///< 上传标记符

  std::string local_filepath_;
  std::string sftp_filepath_;
  int sftp_port_;
  std::string user_pwd_;
};

}  //  end of namespace ysos

#endif