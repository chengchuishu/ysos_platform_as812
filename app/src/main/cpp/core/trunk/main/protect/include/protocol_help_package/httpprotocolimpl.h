/*
# httpprotocolimpl.h
# Definition of HTTPProtocolImpl
# Created on: 2016-04-26 13:18:58
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20160912, created by JinChengZhe
*/
#ifndef PHP_HTTP_PROTOCOL_IMPL_H_  // NOLINT
#define PHP_HTTP_PROTOCOL_IMPL_H_  // NOLINT

/// C++ Standard Headers
#include <map>  // NOLINT
#include <utility>  // NOLINT
/// Platform Headers
#include "../../../public/include/sys_interface_package/common.h"  // NOLINT
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"  // NOLINT
#include "../../../public/include/sys_interface_package/protocolinterface.h"  // NOLINT

namespace ysos {
extern const char* g_request_header_tag_method;
extern const char* g_request_header_tag_request_url;
extern const char* g_request_header_tag_http_version;
extern const char* g_response_header_tag_status_code;
extern const char* g_response_header_tag_reason_phrase;

/**
 *@brief HTTPProtocolImpl的具体实现  // NOLINT
 */
class HTTPProtocolImpl;
typedef boost::shared_ptr<HTTPProtocolImpl> HTTPProtocolImplPtr;
class YSOS_EXPORT HTTPProtocolImpl : public ProtocolInterface, public BaseInterfaceImpl {
  DISALLOW_COPY_AND_ASSIGN(HTTPProtocolImpl)  // 禁止拷贝和复制
  DECLARE_PROTECT_CONSTRUCTOR(HTTPProtocolImpl)  // 构造函数保护
  DECLARE_CREATEINSTANCE(HTTPProtocolImpl)  // 定义类的全局静态创建函数

 public:
  /**
  *@brief 定义用于解析的数据结构  // NOLINT
  */
  typedef std::pair<std::string, std::string> HTTPProtocolParamPair;
  typedef std::map<std::string, std::string> HTTPProtocolParams;
  typedef std::map<std::string, std::string>::iterator HTTPProtocolParamsIterator;

  typedef struct FormMessageContext {
    bool use_default_header;
    std::string url;
    HTTPProtocolParams header_info;
    HTTPProtocolParams data_info;

    FormMessageContext() : use_default_header(false), url("") {
      header_info.clear();
      data_info.clear();
    }

    ~FormMessageContext() {
      use_default_header = false;
      url = "";
      header_info.clear();
      data_info.clear();
    }
  } FMC, *FormMessageContextPtr;

 public:
  /**
  *@brief  析构函数  // NOLINT
  *@param  无  // NOLINT
  *@return:  无  // NOLINT
  */
  virtual ~HTTPProtocolImpl();

  /**
  *@brief 解析HTTP响应信息  // NOLINT
  *@param in_buffer[IN]:  响应信息  // NOLINT
  *@param out_buffer[IN]:  响应数据（去除头信息）  // NOLINT
  *@param format_id[IN]:  请求方法属性，只支持GET和POST  // NOLINT
  *@param context_ptr[IN/OUT]:  头信息（以teamparamsimpl数据格式返回）  // NOLINT
  *@return:  返回 0 成功，返回 -1 或者 大于 0 失败，返回大于 0 表示out_buffe所需要的内存大小  // NOLINT
  */
  virtual int ParseMessage(BufferInterfacePtr in_buffer, BufferInterfacePtr out_buffer, ProtocolFormatId format_id, void *context_ptr);

  /**
  *@brief 生成HTTP请求信息  // NOLINT
  *@param in_buffer[IN]:  URL和Port信息，URL中必须加http://或者https://，例如http://www.baidu.com:80  // NOLINT
  *@param out_buffe[IN]:  请求头信息  // NOLINT
  *@param format_id[IN]:  请求方法属性，只支持GET和POST  // NOLINT
  *@param context_ptr[IN]:  需要添加到头信息的属性值（以teamparamsimpl数据格式输入）  // NOLINT
  *@return:  返回 0 成功，返回 -1 或者大于 0 失败，返回大于 0 表示out_buffe所需要的内存大小  // NOLINT
  */
  virtual int FormMessage(BufferInterfacePtr in_buffer, BufferInterfacePtr out_buffer, ProtocolFormatId format_id, void *context_ptr);

 private:
  /**
  *@brief  空实现函数  // NOLINT
  */
  int GetProperty(int type_id, void *type) {
    return YSOS_ERROR_SUCCESS;
  }
  int SetProperty(int type_id, void *type) {
    return YSOS_ERROR_SUCCESS;
  }
  int GetLength(BufferInterfacePtr in_buffer, ProtocolFormatId format_id) {
    return 0;
  }

  /**
  *@brief 解析HTPPURL  // NOLINT
  *@param url[IN]:  需解析的URL  // NOLINT
  *@param protocol[OUT]:  protocol  // NOLINT
  *@param domain[OUT]:  domain  // NOLINT
  *@param port[OUT]:  port  // NOLINT
  *@param path[OUT]:  path  // NOLINT
  *@param query[OUT]:  query  // NOLINT
  *@param fragment[OUT]:  fragment  // NOLINT
  *@return:  无返回值 // NOLINT
  */
  void ParseURL(const std::string url, std::string& protocol, std::string& domain, std::string& port, std::string& path, std::string& query, std::string& fragment);
};

}

#endif // SFP_YSOS_HTTP_PROTOCOL_IMPL_H_  // NOLINT
