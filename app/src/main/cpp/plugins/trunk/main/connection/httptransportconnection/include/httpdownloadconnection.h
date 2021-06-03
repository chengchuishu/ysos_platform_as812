/*
# HttpDownloadConnectionImpl.h
# Definition of HttpDownloadConnectionImpl
# Created on: 2017-1-20 14:00:00
# Original author: CaiYanli
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
*/

#ifndef  HTTP_DOWN_LOAD_CONNECTION_IMPL_H
#define HTTP_DOWN_LOAD_CONNECTION_IMPL_H


/// ThirdParty Headers
#include <boost/shared_ptr.hpp>  // NOLINT
#include <curl/curl.h>  // NOLINT
/// Platform Headers
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/common.h"  // NOLINT
#include "../../../../../../core/trunk/main/public/include/core_help_package/baseinterfaceimpl.h"  // NOLINT
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/connectioninterface.h"  // NOLINT
#include "../../../../../../core/trunk/main/public/include/os_hal_package/threadpool.h"  // NOLINT


using namespace boost::threadpool;  // NOLINT


namespace ysos {
/**
*@brief HttpDownloadConnectionImpl的具体实现  // NOLINT
*/
class HttpDownloadConnectionImpl;
typedef boost::shared_ptr<HttpDownloadConnectionImpl> httpdownloadconnectionImplPtr;

class YSOS_EXPORT HttpDownloadConnectionImpl : public ConnectionInterface, public BaseInterfaceImpl {
  DECLARE_CREATEINSTANCE(HttpDownloadConnectionImpl);
  DISALLOW_COPY_AND_ASSIGN(HttpDownloadConnectionImpl);
  DECLARE_PROTECT_CONSTRUCTOR(HttpDownloadConnectionImpl);

 public:
  enum { TIME_OUT = 600, MIN_PROGRESS_INTERVAL = 3 };

  ///< 进度条
  struct MyProgress {
    double lastruntime;
    CURL *curl;
  };

 public:
  /**
  *@brief 析构函数  // NOLINT
  *@param ： 无  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual ~HttpDownloadConnectionImpl();

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

  /**
  *@brief 下载文件，多线程方式，支持断点续传、以二进制形式对文件进行操作。  // NOLINT
  *@param url[Input]： 下载文件地址。	 // NOLINT
  *@param file_name[Input]： 存放路径及文件名。	// NOLINT
  *@return： 成功返回0，失败返回相应错误值	。	// NOLINT
  */
  int DownThread(const char *url, const char *file_name);

  /**
  *@brief 线程池boost::bind的回调函数。  // NOLINT
  *@param url[Input]： 下载文件地址。	 // NOLINT
  *@param file_name[Input]： 存放路径及文件名。  // NOLINT
  *@return： 成功返回CURLE_OK，失败返回相应错误值	。	// NOLINT
  */
  unsigned long DownLoad(const char *url, const char *file_name);

  /**
  *@brief 获取服务器文件大小，从http头部获取文件size。  // NOLINT
  *@param url： 下载文件地址。  // NOLINT
  *@return： 返回服务器文件大小。  // NOLINT
  */
  double GetRemoteFileLen(const char *url);

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
  *@brief PROGRESSFUNCTION回调函数的实现。设置下载进度。  // NOLINT
  *@param clientp： 要获取数据的地址。  // NOLINT
  *@param dltotal： 要下载的总字节大小。  // NOLINT
  *@param dlnow： 已经下载的字节大小。  // NOLINT
  *@param ultotal： 要上传的总字节大小。  // NOLINT
   *@param ulnow： 已经上传的字节大小。  // NOLINT
  *@return： 成功返回0，或者出错返回-1。  // NOLINT
  */
  static int ProgressFunc(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);

  /**
  *@brief HEADERFUNCTION回调函数的实现，从http头部获取文件size。  // NOLINT
  *@param ptr： 要获取数据的地址。  // NOLINT
  *@param size： 要写入内容的单字节数。  // NOLINT
  *@param nmemb： 要进行写入size字节的数据项的个数。  // NOLINT
  *@param userdata： 目标文件指针。  // NOLINT
  *@return： 返回实际写入的数据量大小。  // NOLINT
  */
  static size_t GetRemoteFileLenFunc(void *ptr, size_t size, size_t nmemb, void *userdata) ;

 private:
  std::string local_filepath_;  ///< 本地文件路径
  std::string http_filepath_;  ///< 服务器文件路径

  bool is_use_thread_;  ///< 是否使用多线程

  static long  local_flie_len_;  ///< 已下载的字节大小。
  pool  thread_pool_;  ///< Boost线程池
};

}  //  end of namespace ysos

#endif
