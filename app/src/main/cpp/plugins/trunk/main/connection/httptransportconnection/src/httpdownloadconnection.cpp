/*
# HttpDownloadConnectionImpl.h
# Definition of HttpDownloadConnectionImpl
# Created on: 2017-1-20 14:00:00
# Original author: CaiYanli
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
*/

/// Private Headers
#include "../include/httpdownloadconnection.h"
#include <sys/stat.h>
/// ThirdParty Headers
/// Platform Headers
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"



namespace ysos {

const int FILE_EXIST = 200;

long HttpDownloadConnectionImpl::local_flie_len_ = -1;

HttpDownloadConnectionImpl::HttpDownloadConnectionImpl(const std::string &strClassName) : BaseInterfaceImpl(strClassName) {

  local_filepath_ = "";
  http_filepath_ = "";
  is_use_thread_ = false;
}

HttpDownloadConnectionImpl::~HttpDownloadConnectionImpl() {

  local_filepath_ = "";
  http_filepath_ = "";
  is_use_thread_ = false;
}

int HttpDownloadConnectionImpl::Open(void *params) {

  int result = YSOS_ERROR_FAILED;

  //todo:
  local_filepath_ = "";//"D:\\607000063577356.mp4";
  http_filepath_ = "";//"http://103.21.116.236:8080/tdopm/upload/201607/607000063577356.mp4";

  is_use_thread_ = false;

  return YSOS_ERROR_SUCCESS;
}

void HttpDownloadConnectionImpl::Close(void *param) {

  local_filepath_ = "";
  http_filepath_ = "";

  is_use_thread_ = false;

  return;
}

int HttpDownloadConnectionImpl::Read(BufferInterfacePtr input_buffer_ptr, int input_length, void* context_ptr) {

  return CURLE_OK;
}

int HttpDownloadConnectionImpl::Write(BufferInterfacePtr input_buffer_ptr, int input_length, void* context_ptr) {

  if (is_use_thread_) {
    return DownThread(http_filepath_.c_str(), local_filepath_.c_str());
  } else {
    return DownLoad(http_filepath_.c_str(), local_filepath_.c_str());
  }
}

void HttpDownloadConnectionImpl::EnableWrite(bool is_enable) {

  return;
}

void HttpDownloadConnectionImpl::EnableRead(bool is_enable /*= true*/) {

  return;
}

void HttpDownloadConnectionImpl::EnableWrap(bool is_enable) {

  return;
}

size_t HttpDownloadConnectionImpl::GetRemoteFileLenFunc(void *ptr, size_t size, size_t nmemb, void *stream) {

  long len = 0;
  int result = sscanf((char *)ptr, "Content-Length: %ld\n", &len);
  if (result && NULL != stream) /* Microsoft: we don't read the specs */
    *((long *) stream) = len;

  return size * nmemb;
}

double HttpDownloadConnectionImpl::GetRemoteFileLen(const char *url) {

  double len= 0.0;

  CURL *handle = NULL;
  CURLcode return_code = CURLE_OK;

  handle = curl_easy_init();
  if (NULL == handle) {
    printf("get a easy handle failed.\n");
    curl_easy_cleanup(handle);
    return len;
  }

  // ??????url
  return_code = curl_easy_setopt(handle, CURLOPT_URL, url);

  //?????????header???
  return_code = curl_easy_setopt(handle, CURLOPT_HEADER, 1);

  //?????????body
  return_code = curl_easy_setopt(handle, CURLOPT_NOBODY, 1);

  //??????http??????????????????
  return_code = curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, GetRemoteFileLenFunc);

  // ??????????????????
  return_code = curl_easy_perform(handle);
  if (return_code == CURLE_OK) {
    if (CURLE_OK == curl_easy_getinfo(handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &len)) {
      ;
    } else {
      printf("curl_easy_getinfo failed!\n");
    }
  } else {
    len= -1;
  }

  curl_easy_cleanup(handle);

  return len;
}

int HttpDownloadConnectionImpl::ProgressFunc(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow) {
  struct MyProgress *myp = (struct MyProgress *)clientp;
  CURL *curl = myp->curl;
  double curtime = 0;

  int retcode = curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &curtime);

  if ((curtime - myp->lastruntime) >= MIN_PROGRESS_INTERVAL) {
    myp->lastruntime = curtime;

    if (dltotal > 0) {
      double fRate= (dlnow + local_flie_len_)/(dltotal+local_flie_len_);
      int nRate = (int)(fRate*100);
      printf("TOTAL TIME:%f, POGRESS RATE:%d%s\n", curtime, nRate, "%");
    }
  }

  return 0;
}

size_t HttpDownloadConnectionImpl::WriteFunc(char *ptr, size_t size, size_t nmemb, void *userdata) {
  //std::cout << (char *)buffer << std::endl;

  FILE *file = (FILE *)userdata;
  if (NULL != file) {
    size_t return_size = fwrite(ptr, size, nmemb, file);
    return return_size;
  }

  return -1;
}

int HttpDownloadConnectionImpl::DownThread(const char *url, const char *file_name) {

  thread_pool_.schedule(boost::bind(&HttpDownloadConnectionImpl::DownLoad, this, url, file_name));

  size_t active_threads = thread_pool_.active();
  size_t pending_threads = thread_pool_.pending();
  size_t total_threads = thread_pool_.size();

  thread_pool_.size_controller().resize(6);
  thread_pool_.wait();

  return 0;
}

unsigned long HttpDownloadConnectionImpl::DownLoad(const char *url, const char *file_name) {

  CURL *easy_handle = NULL;
  CURLcode return_code = CURLE_OK;

  // ????????????length
  curl_off_t local_file_length_ = -1 ;
  struct stat file_info;
  int use_resume = 0;

  // ????????????????????????
  if (stat(file_name, &file_info) == 0) {
    local_file_length_ =  file_info.st_size;
    use_resume  = 1;
  }

  // ???????????????????????????
  double remote_file_length = GetRemoteFileLen(url);
  if (remote_file_length > 0 && remote_file_length == local_file_length_) {
    printf("file is exist success! file_name : [%s]\n", file_name);

    return CURLE_OK;
  }

  // ?????????????????????????????????
  local_flie_len_ = use_resume ? local_file_length_:0;

  // ???????????????????????????????????????????????????????????????
  FILE *file = fopen(file_name, "ab+");
  if (NULL == file) {
    printf("the file is not exist!\n");
    return -1;
  }

  // ?????????libcurl
  return_code = curl_global_init(CURL_GLOBAL_ALL);
  if (CURLE_OK != return_code) {
    printf("init libcurl failed.\n");
    curl_global_cleanup();
    return -1;
  }

  // ??????easy handle
  easy_handle = curl_easy_init();
  if (NULL == easy_handle) {
    printf("get a easy handle failed.\n");
    curl_easy_cleanup(easy_handle);
    curl_global_cleanup();
    return -1;
  }

  //????????????struct
  struct MyProgress prog;
  prog.lastruntime = 0;
  prog.curl = easy_handle;

  // ??????easy handle??????
  return_code = curl_easy_setopt(easy_handle, CURLOPT_URL, url);

  // ??????????????????????????????
  return_code = curl_easy_setopt(easy_handle, CURLOPT_CONNECTTIMEOUT, 0);

  // ??????????????????????????????libcurl
  return_code = curl_easy_setopt(easy_handle, CURLOPT_RESUME_FROM_LARGE, use_resume ? local_file_length_:0);

  //?????????????????????????????????????????????
  return_code = curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, &WriteFunc);
  return_code = curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, file);

  //????????????????????????????????????????????????????????????libcurl????????????
  return_code = curl_easy_setopt(easy_handle, CURLOPT_NOPROGRESS, 0L);
  return_code = curl_easy_setopt(easy_handle, CURLOPT_PROGRESSFUNCTION, ProgressFunc);
  return_code = curl_easy_setopt(easy_handle, CURLOPT_PROGRESSDATA, &prog);

  //??????????????????http??????????????????/????????????????????????
  return_code = curl_easy_setopt(easy_handle, CURLOPT_VERBOSE, 1L);

  // ??????????????????
  return_code = curl_easy_perform(easy_handle);

  //?????????????????????http???????????????????????????????????????200???400??????????????????????????????????????????404?????????
  int retcode = 0;
  return_code = curl_easy_getinfo(easy_handle, CURLINFO_RESPONSE_CODE , &retcode);
  if (CURLE_OK == return_code && FILE_EXIST == retcode) {
    double length = 0;
    return_code = curl_easy_getinfo(easy_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD , &length);

    printf("file download success! file_name : [%s]\n", file_name);
  } else {
    printf("curl_easy_perform[%d] error\n", return_code);
  }

  fclose(file);

  // ????????????
  curl_easy_cleanup(easy_handle);
  curl_global_cleanup();

  return CURLE_OK;
}

}
