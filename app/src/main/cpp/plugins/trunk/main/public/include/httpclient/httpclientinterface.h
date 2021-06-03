#ifndef YSOS_HTTP_CLIENT_INTERFACE_H_
#define YSOS_HTTP_CLIENT_INTERFACE_H_

#include <string>

namespace ysos {

#ifdef _YSOS_DLL_EXPORTING
#define YSOS_EXPORT __declspec(dllexport)
#else
#define YSOS_EXPORT /*__declspec(dllimport)*/  //add for linux
#endif

enum HTTP_ACTION { HTTP_POST=0, HTTP_PUSH, HTTP_PATCH, HTTP_GET, HTTP_DELETE, HTTP_ACTION_END };

/**
  * @brief htttp client的接口
  */
class YSOS_EXPORT HttpClientInterface {
 public:
  HttpClientInterface(const std::string &object_name="HttpClientInterface") {};
  virtual ~HttpClientInterface() {};

  /**
    * @brief 设置代理信息
    * @param proxy_server_ip_port       代理服务器ip和端口
    * @param proxy_server_user_password 代理服务器用户和密码
    * @return 无
    */
  virtual void SetProxyInfo(const std::string &proxy_server_ip_port, const std::string &proxy_server_user_password = "") = 0;

  /**
    * @brief 设置Htttp的头部内容
    * @param key key值
    * @param value value值
    * @return 无
    */
  virtual void SetHttpHeader(const std::string &key, const std::string &value) = 0;
  /**
    * @brief 设置Htttp的Url
    * @param url http server的Url地址
    * @return 无
    */
  virtual void SetHttpUrl(const std::string &url) = 0;
  /**
    * @brief 设置Htttp的Content
    * @param content http的请求内容
    * @param content_len http的请求内容的长度
    * @return 无
    */
  virtual void SetHttpContent(const char *content, const int content_len) = 0;
  /**
    * @brief 是否支持ssl协议
    * @param is_ssl 是否支持ssl协议，默认不支持（false）
    * @return 无
    */
  virtual void EnableSsl(bool is_ssl=false) = 0;
  /**
    * @brief 设置超时值
    * @param connect_timeout 连接超时值，单位秒
    * @param read_timeout 读取操作超时值，单位秒
    * @return 无
    */
  virtual void SetTimeOut(const int connect_timeout=1, const int read_timeout=1) = 0;
  /**
    * @brief 发送Htttp请求
    * @param response 请求完成后，http server返回的结果
    * @param action 请求的类型
    * @return Response的返回码，-1代表请求失败
    */
  virtual int Request(std::string &response, HTTP_ACTION action=HTTP_POST) = 0;
};
}
#endif
