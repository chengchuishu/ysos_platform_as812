#include "../../../public/include/httpclient/basehttpclientimpl.h"

#ifdef _WIN32
#include <assert.h>
#else
#include <assert.h>
#include <string.h>
#include <sstream>
#endif

namespace ysos {

///////////////////////////////////////////////////////////////////////////////////////////////////
////////////             HttpResponse                  ///////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
HttpResponse::HttpResponse() {
  len_ = 0;
  max_len_ = 1024;
  content_ = new char[max_len_];
  if(content_ != NULL)memset(content_, 0, max_len_);
}

HttpResponse::~HttpResponse() {
  if(NULL != content_) {
    delete [] content_;
    content_ = NULL;
  }
}

void HttpResponse::Relocate(const int request_size) {
  if(content_ == NULL) return;
  char *value = content_;
  int allocate_size = max_len_ + request_size;

  /// 重新分配内存空间，并将原来的数据拷贝过来
  content_ = new char[allocate_size];
  if(content_ == NULL) {
    content_ = value;
    return;
  }
  
  #if _WIN32
    memcpy_s(content_, allocate_size,value, len_);
  #else
    memcpy(content_, value, len_);
  #endif  

  content_[len_] = '\0';
  max_len_ = allocate_size;

  if(value != NULL){
    delete [] value;
    value = NULL;
  }
}

void HttpResponse::Reset() {
  if(content_ != NULL)memset(content_, 0, max_len_);
  len_ = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
////////////             BaseHttpClientImpl            ///////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

static void PrintDSA(const char* pszTag, const char* pszFormat, ...) {
#ifdef _DEBUG
  va_list argList;
  va_start(argList, pszFormat);

  char szBuffer[1024*2] = {0};
  int nResult = -1;
  int nTagLen = strlen(pszTag);

  if (pszTag != NULL && nTagLen > 0 && nTagLen < 2040 ) {
    nResult = sprintf_s(szBuffer, _countof(szBuffer), "###[%s]###", pszTag);
    if (nResult == -1) {
      nResult = 0;
    }
  } else {
    nResult = sprintf_s(szBuffer, _countof(szBuffer), "****");
    if (nResult == -1) {
      nResult = 0;
    }
  }

  vsprintf_s(szBuffer + nResult, _countof(szBuffer) - nResult, pszFormat, argList);

  OutputDebugStringA(szBuffer);

  va_end(argList);
#endif

  return;
}

BaseHttpClientImpl::BaseHttpClientImpl(const std::string &object_name/* ="BaseHttpClientImpl" */)
  : curl_ptr_(NULL), curl_headers_(NULL), ret_code_(0), is_ssl_(false), connect_timeout_(30), read_timeout_(30) {
  curl_ptr_ = curl_easy_init();
  if (NULL == curl_ptr_) {
    printf("get HttpProtocol curl info failed !!!\n");
  }

  //request_ = new HttpParam();
  request_ = NULL;
  request_len_ = 0;
  response_ = new HttpResponse();

  http_action_array_ = new std::string[6];
  http_action_array_[0] = "POST";
  http_action_array_[1] = "PUSH";
  http_action_array_[2] = "PATCH";
  http_action_array_[3] = "GET";
  http_action_array_[4] = "DELETE";
  http_action_array_[5] = "";
}

BaseHttpClientImpl::~BaseHttpClientImpl() {
  if(curl_headers_)curl_slist_free_all(curl_headers_);
  if(curl_ptr_)curl_easy_cleanup(curl_ptr_);

  if(NULL != response_) {
    delete response_;
    request_ = NULL;
  }

  if(NULL != http_action_array_) {
    delete [] http_action_array_;
	http_action_array_ = NULL;
  }
}

void BaseHttpClientImpl::SetProxyInfo(const std::string &proxy_server_ip_port, const std::string &proxy_server_user_password) {
  if (proxy_server_ip_port.length() > 0) {
    PrintDSA("proxy server ip port: %s", proxy_server_ip_port.c_str());
    curl_easy_setopt(curl_ptr_, CURLOPT_PROXY, proxy_server_ip_port.c_str());  //10.99.60.201:8080
    if (proxy_server_user_password.length() > 0) {
      PrintDSA("proxy server user password: %s", proxy_server_user_password.c_str());
      curl_easy_setopt(curl_ptr_, CURLOPT_PROXYUSERPWD, proxy_server_user_password.c_str());  //user1:pwd
    }
  }
}

void BaseHttpClientImpl::SetHttpHeader(const std::string &key, const std::string &value) {
  std::string header_content = key + ":" + value;
  curl_headers_ = curl_slist_append(curl_headers_, header_content.c_str());
}

void BaseHttpClientImpl::SetHttpUrl(const std::string &url) {
  url_ = url;
}

void BaseHttpClientImpl::SetHttpContent(const char *content, const int content_len) {
  request_ = content;
  request_len_ = content_len;
}

void BaseHttpClientImpl::EnableSsl(bool is_ssl) {
  is_ssl_ = is_ssl;

  curl_easy_setopt(curl_ptr_, CURLOPT_SSL_VERIFYPEER, is_ssl_ ? 1L : 0L);
  curl_easy_setopt(curl_ptr_, CURLOPT_SSL_VERIFYHOST, is_ssl_ ? 1L : 0L);
}

void BaseHttpClientImpl::SetTimeOut(const int connect_timeout, const int read_timeout) {
  connect_timeout_ = connect_timeout;
  read_timeout_ = read_timeout;
}

void BaseHttpClientImpl::SetRequestAction(HTTP_ACTION action) {
  assert(HTTP_ACTION_END > action); //add for debug
  
  std::string &action_type = http_action_array_[action];
  curl_easy_setopt(curl_ptr_, CURLOPT_CUSTOMREQUEST, action_type.c_str());
}

static size_t http_header_callback(void *ptr, size_t size, size_t nmemb, void *data) {
  return size*nmemb;
}

static size_t http_read_callback(void *ptr, size_t size, size_t nmemb, void *data) {
  return size*nmemb;
}

static size_t http_write_callback(void *ptr, size_t size, size_t nmemb, void *data) {
  HttpResponse *response = reinterpret_cast<HttpResponse*>(data);
  assert(NULL != response);// for debug linux
  
  int request_size = size * nmemb;
  int left_size = response->max_len_ - response->len_;

  if(left_size < request_size) {
    response->Relocate(request_size);
  }

  if(response->content_ != NULL && request_size < response->max_len_ - response->len_ ) {
    char *p = response->content_ + response->len_;
    if(p!=NULL)memcpy(p, ptr, request_size);
    response->len_ += request_size;
    response->content_[response->len_] = '\0';
  }

  return size*nmemb;
}

int BaseHttpClientImpl::Request(std::string &response, HTTP_ACTION action) {
  if(NULL == curl_ptr_||NULL == response_) return -1;

#ifdef _DEBUG
  curl_easy_setopt(curl_ptr_, CURLOPT_VERBOSE, 1);
#endif

  curl_easy_setopt(curl_ptr_, CURLOPT_URL,url_.c_str());
  if(curl_headers_!= NULL)curl_easy_setopt(curl_ptr_, CURLOPT_HTTPHEADER, curl_headers_);

  response_->Reset();
  curl_easy_setopt(curl_ptr_, CURLOPT_WRITEDATA, response_);
  curl_easy_setopt(curl_ptr_,CURLOPT_WRITEFUNCTION, http_write_callback);

  curl_easy_setopt(curl_ptr_,CURLOPT_HEADERFUNCTION, http_header_callback);
  curl_easy_setopt(curl_ptr_, CURLOPT_HEADERDATA, NULL);

  curl_easy_setopt(curl_ptr_,CURLOPT_READFUNCTION, http_read_callback);
  curl_easy_setopt(curl_ptr_,CURLOPT_READDATA, NULL);

  /// 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作
  /// 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出
  curl_easy_setopt(curl_ptr_, CURLOPT_NOSIGNAL, 1);
  curl_easy_setopt(curl_ptr_, CURLOPT_CONNECTTIMEOUT, connect_timeout_);
  curl_easy_setopt(curl_ptr_, CURLOPT_TIMEOUT, read_timeout_);

  if (NULL != request_ && 0 != request_len_) {
    curl_easy_setopt(curl_ptr_, CURLOPT_POSTFIELDS, request_);
    curl_easy_setopt(curl_ptr_, CURLOPT_POSTFIELDSIZE, request_len_);
  }

  int ret_code = -1;

  try {
    CURLcode curl_code = curl_easy_perform(curl_ptr_);
    if (CURLE_OK == curl_code) {
      ret_code = 0;
      if(response_->content_ != NULL)response = response_->content_;
    } else {
      //ret_code = curl_code;
      curl_easy_getinfo(curl_ptr_, CURLINFO_RESPONSE_CODE, &ret_code);
    }
  } catch (...) {
    // Exception
  }

  return ret_code;
}

}