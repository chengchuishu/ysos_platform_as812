/*
# sftptransportconnectionImpl.h
# Definition of sftptransportconnectionImpl
# Created on: 2017-1-18 14:00:00
# Original author: CaiYanli
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
*/

/// Private Headers
#include "../include/sftptransportconnection.h"
/// ThirdParty Headers
#include <boost/bind.hpp>
/// Platform Headers
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"



namespace ysos {

struct FtpFile {
  const char *filename;
  FILE *stream;
};

sftptransportconnectionImpl::sftptransportconnectionImpl(const std::string &strClassName /* =sftptransportconnectionImpl */) : BaseInterfaceImpl(strClassName) {

}

sftptransportconnectionImpl::~sftptransportconnectionImpl() {
  enable_read_ = false;
  enable_write_ = false;

  local_filepath_ = "";
  sftp_filepath_ = "";
  sftp_port_ = 0;
  user_pwd_ = "";
}

int sftptransportconnectionImpl::Open(void *params) {

  int result = YSOS_ERROR_FAILED;

  //todo:
  local_filepath_ = "";//"D:\\棠棣宣传片.mp4";
  sftp_filepath_ = "";//"sftp://103.21.116.236:1222/home/loguser/upload/棠棣宣传片.mp4";
  sftp_port_ = 1222;
  user_pwd_ = "loguser:loguserxyz";

  return YSOS_ERROR_SUCCESS;
}

void sftptransportconnectionImpl::Close(void *param) {

  local_filepath_ = "";
  sftp_filepath_ = "";
  sftp_port_ = 0;
  user_pwd_ = "";

  return;
}

int sftptransportconnectionImpl::Read(BufferInterfacePtr input_buffer_ptr, int input_length, void* context_ptr) {

  CURL *curl = NULL;
  CURLcode res = CURLE_OK;

  const char *sourcePath = local_filepath_.c_str();
  const char *sftpPath = sftp_filepath_.c_str();
  FILE* file = fopen(sourcePath, "rb");
  if (NULL == file) {
    printf("the file is not exist!\n");
    return -1;
  }

  // 初始化libcurl
  res = curl_global_init(CURL_GLOBAL_ALL);
  if (CURLE_OK != res) {
    printf("init libcurl failed.\n");
    curl_global_cleanup();
    return -1;
  }

  // 获取easy handle
  curl = curl_easy_init();
  if (NULL == curl) {
    printf("get a easy handle failed.\n");
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    return -1;
  }

  // 设置上传文件路径名、端口
  res = curl_easy_setopt(curl, CURLOPT_URL, sftpPath);
  res = curl_easy_setopt(curl, CURLOPT_PORT, sftp_port_);

  // 设置回调函数：保存读取到的数据
  res = curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
  res = curl_easy_setopt(curl, CURLOPT_READDATA, file);

  // 设置要上传的文件的大小
  res = curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, get_file_size(file));

  // 设置响应超时，以秒为单位
  res = curl_easy_setopt(curl, CURLOPT_FTP_RESPONSE_TIMEOUT, TIME_OUT);

  // SSH身份验证方式
  res = curl_easy_setopt(curl, CURLOPT_USERPWD, user_pwd_.c_str());
  res = curl_easy_setopt(curl, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_PASSWORD);
  //res =curl_easy_setopt(curl, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_AGENT);

  //在服务器上创建缺失目录
  res = curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);

  //upload操作
  res = curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

  // 执行数据请求
  res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform[%d] error./n", res);
  }

  fclose(file);

  // 释放资源
  curl_easy_cleanup(curl);
  curl_global_cleanup();

//  YSOS_LOG << "sftptransportconnectionImpl::Read[Exit]" << std::endl;
  printf("sftptransportconnectionImpl::Read[Exit]\n");

  return res;
}

int sftptransportconnectionImpl::Write(BufferInterfacePtr input_buffer_ptr, int input_length, void* context_ptr) {
  CURL *curl = NULL;
  CURLcode res = CURLE_OK;

  struct FtpFile ftpfile= {
    sftp_filepath_.c_str(),
    NULL
  };

  // 初始化libcurl
  res = curl_global_init(CURL_GLOBAL_DEFAULT);
  if (CURLE_OK != res) {
    printf("init libcurl failed.\n");
    curl_global_cleanup();
    return -1;
  }

  // 获取easy handle
  curl = curl_easy_init();
  if (NULL == curl) {
    printf("get a easy handle failed.\n");
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    return -1;
  }

  // 设置下载源文件路径名、端口
  res = curl_easy_setopt(curl, CURLOPT_URL, local_filepath_.c_str());
  res = curl_easy_setopt(curl, CURLOPT_PORT, sftp_port_);

  // 设置连接超时，单位秒
  res = curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 0);

  // 设置回调函数：保存接收到的数据
  res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ftpfile);

  // 设置SSH认证
  res = curl_easy_setopt(curl, CURLOPT_USERPWD, user_pwd_.c_str());
  res = curl_easy_setopt(curl, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_PASSWORD);

  // 设置输出响应
  res = curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

  // 执行数据请求
  res = curl_easy_perform(curl);
  if (CURLE_OK != res) {
    fprintf(stderr, "curl told us [%d] error./n", res);
  }

  // 关闭本地文件
  if (NULL != ftpfile.stream) {
    fclose(ftpfile.stream);
  }

  // 释放资源
  curl_easy_cleanup(curl);
  curl_global_cleanup();

//  YSOS_LOG << "sftptransportconnectionImpl::Read[Exit]" << std::endl;
  printf("sftptransportconnectionImpl::Read[Exit]\n");

  return res;
}

void sftptransportconnectionImpl::EnableWrite(bool is_enable) {
  enable_write_ = is_enable;
  enable_read_ = !is_enable;

  return;
}


void sftptransportconnectionImpl::EnableRead(bool is_enable /*= true*/) {
  enable_read_ = is_enable;
  enable_write_ = !is_enable;

  return;
}

void sftptransportconnectionImpl::EnableWrap(bool is_enable) {
  return;
}

size_t sftptransportconnectionImpl::read_callback(void *ptr, size_t size, size_t nmemb, FILE *stream) {
  size_t retcode;
  curl_off_t nread;

  retcode = fread(ptr, size, nmemb, stream);
  //fprintf(stderr, "*** size %d, nmemb %d bytes\n", size, nmemb);
  nread = (curl_off_t)retcode;
  fprintf(stderr, "*** We read %" CURL_FORMAT_CURL_OFF_T " bytes from file\n", nread);

  return retcode;
}

size_t sftptransportconnectionImpl::write_callback(void *ptr, size_t size, size_t nmemb, void *stream) {
  struct FtpFile *out=(struct FtpFile *)stream;
  if (out && !out->stream) {
    out->stream=fopen(out->filename, "wb");
    if (!out->stream)
      return -1;
  }

  return fwrite(ptr, size, nmemb, out->stream);
}

curl_off_t sftptransportconnectionImpl::get_file_size(FILE *file) {
  curl_off_t size = 0;

  fseek(file, 0L, SEEK_END);
  size = (curl_off_t) ftell(file);
  fseek(file, 0L, SEEK_SET);

  return size;
}

}