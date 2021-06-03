#ifndef YSOS_HTTP_CLIENT_H_
#define YSOS_HTTP_CLIENT_H_

#include <string>
#include "httpclientinterface.h"

namespace ysos {
/**
  * @brief htttp client的通用类
  */
class YSOS_EXPORT HttpClient: public HttpClientInterface {
 public:
  HttpClient(const std::string &object_name="HttpClient");
  virtual ~HttpClient();

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
    * @param connect_timeout 握手连接超时值（只包含握手超时，不包含传输数据），单位秒
    * @param read_timeout 连接总超时值（从连接开始到结束，大于connect_timeout），单位秒
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

 private:
  HttpClient(const HttpClient &);
  HttpClient& operator=(const HttpClient&);

 protected:
  HttpClientInterface *http_impl_;            ///< http的执行指针
};
}
#endif
