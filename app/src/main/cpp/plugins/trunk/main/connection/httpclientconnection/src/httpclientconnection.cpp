/*
# HttpDownloadConnectionImpl.h
# Definition of HttpDownloadConnectionImpl
# Created on: 2017-1-20 14:00:00
# Original author: CaiYanli
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
*/

/// Private Headers
#include "../include/httpclientconnection.h"
#include <sys/stat.h>
/// ThirdParty Headers
/// Platform Headers
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"

namespace ysos {

static log4cplus::Logger s_logtag;

HttpClientConnection::HttpClientConnection(const std::string &strClassName)
  : BaseInterfaceImpl(strClassName)
  , curl_ptr_(NULL)
  , curl_headers_(NULL)
{
  logger_ = GetUtility()->GetLogger();
  s_logtag = GetUtility()->GetLogger();
}

HttpClientConnection::~HttpClientConnection() {
  if(NULL != curl_headers_) {
    curl_slist_free_all(curl_headers_);
  }

  if(NULL != curl_ptr_) {
    curl_easy_cleanup(curl_ptr_);
  }
}

int HttpClientConnection::Open(void *params) {
  curl_ptr_ = curl_easy_init();
  if (NULL == curl_ptr_) {
    YSOS_LOG_ERROR("open connection failed: " << logic_name_);
  }

  std::string *uri = (std::string *)params;
  curl_easy_setopt(curl_ptr_, CURLOPT_URL, uri->c_str());
  // curl_easy_setopt(curl_ptr_, CURLOPT_HTTPHEADER, curl_headers_);
  // curl_easy_setopt(curl_ptr_, CURLOPT_HEADERFUNCTION, &HttpClientConnection::HeaderFunc);
  // curl_easy_setopt(curl_ptr_, CURLOPT_HEADERDATA, NULL);

  /// 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作
  /// 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出
  curl_easy_setopt(curl_ptr_, CURLOPT_NOSIGNAL, 1);
  curl_easy_setopt(curl_ptr_, CURLOPT_CONNECTTIMEOUT, 10);
  curl_easy_setopt(curl_ptr_, CURLOPT_TIMEOUT, 10);

  return YSOS_ERROR_SUCCESS;
}

void HttpClientConnection::Close(void *param) {
  if(NULL != curl_headers_) {
    curl_slist_free_all(curl_headers_);
    curl_headers_ = NULL;
  }

  if(NULL != curl_ptr_) {
    curl_easy_cleanup(curl_ptr_);
    curl_ptr_ = NULL;
  }
}

int HttpClientConnection::Read(BufferInterfacePtr input_buffer_ptr, int input_length, void* context_ptr) {
  int rst = YSOS_ERROR_FAILED;

  GetBufferUtility()->SetBufferLength(input_buffer_ptr, 0);
  curl_easy_setopt(curl_ptr_, CURLOPT_WRITEFUNCTION, &HttpClientConnection::WriteFunc);
  curl_easy_setopt(curl_ptr_, CURLOPT_WRITEDATA, &input_buffer_ptr);

  try {
    CURLcode curl_code = curl_easy_perform(curl_ptr_);
    if(CURLE_OK != curl_code){

    }
    rst = YSOS_ERROR_SUCCESS;
  }catch (...) {

  }

  return rst;
}

int HttpClientConnection::Write(BufferInterfacePtr input_buffer_ptr, int input_length, void* context_ptr) {
  int rst = YSOS_ERROR_FAILED;

  UINT8* bufferptr = NULL;
  UINT32 bufferlength = 0;
  UINT32 bufferprefix= 0;
  input_buffer_ptr->GetBufferAndLength(&bufferptr, &bufferlength);
  input_buffer_ptr->GetPrefixLength(&bufferprefix);

  if (input_length<=0 || input_length>bufferlength) {
    input_length = bufferlength;
  }

  curl_easy_setopt(curl_ptr_, CURLOPT_POST,1);
  curl_easy_setopt(curl_ptr_, CURLOPT_POSTFIELDSIZE, input_length);
  curl_easy_setopt(curl_ptr_, CURLOPT_READFUNCTION, &HttpClientConnection::ReadFunc);
  curl_easy_setopt(curl_ptr_, CURLOPT_READDATA, &input_buffer_ptr);

  if (context_ptr != NULL) {
    GetBufferUtility()->SetBufferLength(*reinterpret_cast<BufferInterfacePtr*>(context_ptr), 0);
    curl_easy_setopt(curl_ptr_, CURLOPT_WRITEFUNCTION, &HttpClientConnection::WriteFunc);
    curl_easy_setopt(curl_ptr_, CURLOPT_WRITEDATA, context_ptr);
  }
  else {
    curl_easy_setopt(curl_ptr_, CURLOPT_WRITEFUNCTION, NULL);
    curl_easy_setopt(curl_ptr_, CURLOPT_WRITEDATA, NULL);
  }

  try {
    CURLcode curl_code = curl_easy_perform(curl_ptr_);
    if(CURLE_OK != curl_code){

    }
    rst = YSOS_ERROR_SUCCESS;
  }catch (...) {

  }

  // revert the length
  input_buffer_ptr->SetLength(bufferlength, bufferprefix);

  return rst;

}

void HttpClientConnection::EnableWrite(bool is_enable) {
  return;
}

void HttpClientConnection::EnableRead(bool is_enable /*= true*/) {
  return;
}

void HttpClientConnection::EnableWrap(bool is_enable) {
  return;
}

size_t HttpClientConnection::HeaderFunc(char *ptr, size_t size, size_t nmemb, void *userdata) {
  return size*nmemb;
}

size_t HttpClientConnection::WriteFunc(char *ptr, size_t size, size_t nmemb, void *userdata) {
  if (ptr == NULL) return 0;
  size *= nmemb;

  BufferInterfacePtr* buff = reinterpret_cast<BufferInterfacePtr*>(userdata);
  if (NULL != buff) {
    UINT8* bufferptr = NULL;
    UINT32 bufferlength = 0;
    UINT32 buffermaxlength = 0;
    (*buff)->GetBufferAndLength(&bufferptr, &bufferlength);
    (*buff)->GetMaxLength(&buffermaxlength);
    if (NULL != bufferptr) {
      if (buffermaxlength > bufferlength+size) {
        memcpy((char*)bufferptr+bufferlength, ptr, size);
        bufferlength += size;
        // add '\0' at the end
        bufferptr[bufferlength] = '\0';
        // but '\0' is not counted in the bufferlength
        GetBufferUtility()->SetBufferLength(*buff, bufferlength);
      }
      else {
        YSOS_LOG_ERROR_CUSTOM(s_logtag, "buffer size is not enough "<<buffermaxlength<<" "<<(bufferlength+size));
      }
    }
  }

  return size;
}

size_t HttpClientConnection::ReadFunc(char *ptr, size_t size, size_t nmemb, void *userdata) {
  if (ptr == NULL) return 0;
  size *= nmemb;

  BufferInterfacePtr* buff = reinterpret_cast<BufferInterfacePtr*>(userdata);
  if (NULL != buff) {
    UINT8* bufferptr = NULL;
    UINT32 bufferlength = 0;
    UINT32 bufferprefix= 0;
    (*buff)->GetBufferAndLength(&bufferptr, &bufferlength);
    (*buff)->GetPrefixLength(&bufferprefix);
    if (NULL != bufferptr) {
      if (size > bufferlength) {
        size = bufferlength;
      }
      memcpy(ptr, (char*)bufferptr, size);
      bufferlength -= size;
      bufferprefix += size;
      (*buff)->SetLength(bufferlength,bufferprefix);
    }
  }

  return size;
}

}
