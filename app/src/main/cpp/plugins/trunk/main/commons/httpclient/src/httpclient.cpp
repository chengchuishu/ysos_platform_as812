#include "../../../public/include/httpclient/httpclient.h"
#include "../../../public/include/httpclient/basehttpclientimpl.h"


namespace ysos {

HttpClient::HttpClient(const std::string &object_name/* ="BaseHttpClientImpl" */) {
  http_impl_ = new BaseHttpClientImpl(object_name + "_impl");
}

HttpClient::~HttpClient() {
  if(NULL != http_impl_) {
    delete http_impl_;
	http_impl_ = NULL;
  }
}

void HttpClient::SetProxyInfo(const std::string &proxy_server_ip_port, const std::string &proxy_server_user_password) {
  if(http_impl_)http_impl_->SetProxyInfo(proxy_server_ip_port, proxy_server_user_password);
}

void HttpClient::SetHttpHeader(const std::string &key, const std::string &value) {
  if(http_impl_)http_impl_->SetHttpHeader(key, value);
}

void HttpClient::SetHttpUrl(const std::string &url) {
  if(http_impl_)http_impl_->SetHttpUrl(url);
}

void HttpClient::SetHttpContent(const char *content, const int content_len) {
  if(http_impl_)http_impl_->SetHttpContent(content, content_len);
}

void HttpClient::EnableSsl(bool is_ssl) {
  if(http_impl_)http_impl_->EnableSsl(is_ssl);
}

void HttpClient::SetTimeOut(const int connect_timeout, const int read_timeout) {
  if(http_impl_)http_impl_->SetTimeOut(connect_timeout, read_timeout);
}

int HttpClient::Request(std::string &response, HTTP_ACTION action) {
  if(http_impl_ == NULL) return -1;
  return http_impl_->Request(response, action);
}
}