/*
# HttpClientConnection.h
# Definition of HttpDownloadConnectionImpl
# Created on: 2017-1-20 14:00:00
# Original author: CaiYanli
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
*/

#ifndef PLUGIN_HTTP_CLIENT_CONNECTION_H_
#define PLUGIN_HTTP_CLIENT_CONNECTION_H_


/// ThirdParty Headers
#include <boost/shared_ptr.hpp>  // NOLINT
#include <curl/curl.h>  // NOLINT
/// Platform Headers
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/common.h"  // NOLINT
#include "../../../../../../core/trunk/main/public/include/core_help_package/baseinterfaceimpl.h"  // NOLINT
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/connectioninterface.h"  // NOLINT

namespace ysos {
/**
*@brief HttpDownloadConnectionImpl的具体实现  // NOLINT
*/
class HttpClientConnection;
typedef boost::shared_ptr<HttpClientConnection> HttpClientConnectionPtr;

class YSOS_EXPORT HttpClientConnection : public ConnectionInterface, public BaseInterfaceImpl, public boost::enable_shared_from_this<HttpClientConnection> {
  DECLARE_CREATEINSTANCE(HttpClientConnection);
  DISALLOW_COPY_AND_ASSIGN(HttpClientConnection);
  DECLARE_PROTECT_CONSTRUCTOR(HttpClientConnection);

 public:
  /**
  *@brief 析构函数  // NOLINT
  *@param ： 无  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual ~HttpClientConnection();

  /**
  *@brief 打开并初始化类  // NOLINT
  *@param params[Input]： uri，std::string *  // NOLINT
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
   *@param context_ptr[Input]： 传输接口的上下文，与具体实现密切相关  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Read(BufferInterfacePtr input_buffer_ptr, int input_length, void* context_ptr = NULL);

  /**
  *@brief 写入数据函数  // NOLINT
  *@param input_buffer_ptr[Input]： 读取数据的缓冲，内部包含实际读取数据长度  // NOLINT
  *@param input_length[Input]： 缓冲长度 (大于0时有效；否则使用input_buffer_ptr里的实际长度)  // NOLINT
  *@param context_ptr[Input]： 若不为空，表示服务器返回的数据，类型BufferInterfacePtr*  // NOLINT
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

 private:
  /**
  *@brief HEADERFUNCTION回调函数的实现。  // NOLINT
  *@param ptr： 是要获取数据的地址。  // NOLINT
  *@param size： 要写入内容的单字节数。  // NOLINT
  *@param nmemb： 要进行写入size字节的数据项的个数。  // NOLINT
  *@param userdata： 目标文件指针。  // NOLINT
  *@return： 返回实际写入的数据项个数count。  // NOLINT
  */
  static size_t HeaderFunc(char *ptr, size_t size, size_t nmemb, void *userdata);

  /**
  *@brief WRITEFUNCTION回调函数的实现。  // NOLINT
  *@param ptr： 是要获取数据的地址。  // NOLINT
  *@param size： 要写入内容的单字节数。  // NOLINT
  *@param nmemb： 要进行写入size字节的数据项的个数。  // NOLINT
  *@param userdata： 目标文件指针。  // NOLINT
  *@return： 返回实际写入的数据项个数count。  // NOLINT
  */
  static size_t WriteFunc(char *ptr, size_t size, size_t nmemb, void *userdata);

  /**
  *@brief READFUNCTION回调函数的实现。  // NOLINT
  *@param ptr： 是要获取数据的地址。  // NOLINT
  *@param size： 要读入内容的单字节数。  // NOLINT
  *@param nmemb： 要进行写入size字节的数据项的个数。  // NOLINT
  *@param userdata： 目标文件指针。  // NOLINT
  *@return： 返回实际读入的数据项个数count。  // NOLINT
  */
  static size_t ReadFunc(char *ptr, size_t size, size_t nmemb, void *userdata);

 protected:
  CURL *curl_ptr_;
  curl_slist *curl_headers_;
};

}  //  end of namespace ysos

#endif
