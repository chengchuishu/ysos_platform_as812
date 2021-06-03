#ifndef YSOS_BASE_HTTP_CLIENT_IMPL_H_
#define YSOS_BASE_HTTP_CLIENT_IMPL_H_

extern "C" {
#include <curl/curl.h>
};
#include <string>
#include "httpclientinterface.h"

namespace ysos {

/**
  * @brief Http返回参数数据结构
  */
struct HttpResponse {
  char *content_;
  int   max_len_;
  int   len_;

  HttpResponse();
  ~HttpResponse();

  /**
    * @brief 重新分配内存
    */
  void Relocate(const int request_size);
  /**
    * @brief 复位
    */
  void Reset();
};

/**
  * @brief htttp client的通用类
  */
class BaseHttpClientImpl: public HttpClientInterface {
 public:
  BaseHttpClientImpl(const std::string &object_name="BaseHttpClientImpl");
  virtual ~BaseHttpClientImpl();

  /**
    * @brief 设置代理信息
    * @param proxy_server_ip_port       代理服务器ip和端口
    * @param proxy_server_user_password 代理服务器用户和密码
    * @return 无
    */
  virtual void SetProxyInfo(const std::string &proxy_server_ip_port, const std::string &proxy_server_user_password = "");

  /**
    * @brief 设置Htttp的头部内容
    * @param key key值
    * @param value value值
    * @return 无
    */
  virtual void SetHttpHeader(const std::string &key, const std::string &value);
  /**
    * @brief 设置Htttp的Url
    * @param url http server的Url地址
    * @return 无
    */
  virtual void SetHttpUrl(const std::string &url);
  /**
    * @brief 设置Htttp的Content
    * @param content http的请求内容
    * @param content_len http的请求内容的长度
    * @return 无
    */
  virtual void SetHttpContent(const char *content, const int content_len);
  /**
    * @brief 是否支持ssl协议
    * @param is_ssl 是否支持ssl协议，默认不支持（false）
    * @return 无
    */
  virtual void EnableSsl(bool is_ssl=false);
  /**
    * @brief 设置超时值
    * @param connect_timeout 连接超时值，单位秒
    * @param read_timeout 读取操作超时值，单位秒
    * @return 无
    */
  virtual void SetTimeOut(const int connect_timeout=1, const int read_timeout=1);
  /**
    * @brief 发送Htttp请求
    * @param response 请求完成后，http server返回的结果
    * @param action 请求的类型
    * @return Response的返回码，-1代表请求失败
    */
  virtual int Request(std::string &response, HTTP_ACTION action=HTTP_POST);

 protected:
  /**
    * @brief 设置Htttp请求的类型
    * @param action 请求的类型
    * @return 无
    */
  void SetRequestAction(HTTP_ACTION action);

 private:
  BaseHttpClientImpl(const BaseHttpClientImpl &);
  BaseHttpClientImpl& operator=(const BaseHttpClientImpl&);

 protected:
  CURL            *curl_ptr_;                ///< http的执行指针
  curl_slist      *curl_headers_;            ///< http的Header指针
  int              ret_code_;                ///< http的返回码
  std::string      url_;                     ///< http url
  const char      *request_;                 ///< http 请求的内容
  int              request_len_;             ///< http 请求的内容长度
  bool             is_ssl_;                  ///< 是否支持ssl
  HttpResponse       *response_;               ///< 存储Response的内容
  int               response_max_len_;       ///< 可支持的response内容最大长度
  std::string      *http_action_array_;      ///< http action类型组
  int               connect_timeout_;        ///< 连接超时值
  int               read_timeout_;           ///< 读取操作超时值
};
}
#endif
