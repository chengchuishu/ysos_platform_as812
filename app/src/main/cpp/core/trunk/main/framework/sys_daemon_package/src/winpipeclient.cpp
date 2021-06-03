/*
# winpipeclient.cpp
# Definition of WinPipeClient
# Created on: 2017-11-26 18:21:48
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20171126, created by JinChengZhe
*/

/// Private Headers
#include "../../../protect/include/sys_daemon_package/winpipeclient.h"
//#include <tchar.h>
#ifdef _WIN32
/// Windows Platform Headers
#include <strsafe.h>
#include <atlconv.h>
#else
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <boost/thread.hpp>
#endif

using namespace ysos::extern_common;

namespace ysos {

namespace win_pipe {

YSOS_IMPLIMENT_SINGLETON(WinPipeClient)
WinPipeClient::WinPipeClient(void)
  : running_status_(IS_INITIAL),
    pipe_handler_(0) {

}

WinPipeClient::~WinPipeClient(void) {

}

int WinPipeClient::StartA(const std::string& pipe_name, const PipeMessageHandlerA& pipe_message_handler) {
  printf("WinPipeClient::WinPipeClient::StartA[Enter]\n");
  int result = 0;

  do {
    if (IS_RUNNING == running_status_) {
      printf("WinPipeClient::WinPipeClient::StartA[Check Point]WinPipeClient is already running\n");
      break;
    }

    printf("WinPipeClient::WinPipeClient::StartA[Check Point][0]\n");

    if (true == pipe_name.empty() || NULL != pipe_message_handler) {
      printf("WinPipeClient::WinPipeClient::StartA[Fail][pipe_name is empty or pipe_message_handler is not NULL]\n");
      result = -1;
      break;
    }

    printf("WinPipeClient::WinPipeClient::StartA[Check Point][1]\n");

    pipe_nameA_ = pipe_name;

    printf("WinPipeClient::WinPipeClient::StartA[Check Point][pipe_nameA_][%s]\n", pipe_nameA_.c_str());

    for (;;) {
      /// 连接pipe服务器
      //pipe_handler_ = CreateFileA( pipe_nameA_.c_str()/*pipe name*/,GENERIC_READ | GENERIC_WRITE/* read and write access*/,0/* no sharing*/,NULL/*default security attributes*/,OPEN_EXISTING/*opens existing pipe*/,0,NULL);// no template file
      int wfd = open(pipe_name.c_str(),O_WRONLY|O_NONBLOCK,0);
      /// 连接pipe服务器成功
      if ( wfd > 0 ) {
        printf("WinPipeClient::WinPipeClient::StartA[Check Point][Success]\n");
        break;
      }

      /// 无法连接pipe服务器
      const char* err;
      perror(err);
      if (NULL != err) {
        printf("WinPipeClient::WinPipeClient::StartA[Fail][The function[CreateFileA()] is failed][%s]\n", err);
        result = -2;
        break;
      }
      
      //TODO: add for linux
      boost::this_thread::sleep(boost::posix_time::seconds(2));
      /* /// pipe服务器忙，等待两秒后再次连接
      if (TRUE != WaitNamedPipeA(pipe_name.c_str(), 20000)) {
        printf("WinPipeClient::WinPipeClient::StartA[Fail][The function[WaitNamedPipeA()] is failed][%s]\n", "GetLastError()");
        result = -3;
        break;
      }
      */
    }
  } while (0);

  /// 设置运行标志位
  if (0 == result) {
    running_status_ = IS_RUNNING;
  }

  printf("WinPipeClient::WinPipeClient::StartA[Exit]\n");
  return result;
}

int WinPipeClient::StartW(const std::wstring& pipe_name, const PipeMessageHandlerW& pipe_message_handler) {
  int result = 0;

  do {
  } while (0);

  return result;
}

int WinPipeClient::Stop(void) {
  printf("WinPipeClient::WinPipeClient::Stop[Enter]\n");
  int result = 0;

  do {
    if (IS_STOPPED == running_status_) {
      printf("WinPipeClient::WinPipeClient::Stop[Check Point]WinPipeClient is already stopped\n");
      break;
    }

    printf("WinPipeClient::WinPipeClient::Stop[Check Point][0]\n");

    if (0 >= pipe_handler_) {
      printf("WinPipeClient::WinPipeClient::Stop[Check Point][pipe_handler_ is not INVALID_HANDLE_VALUE]\n");
      //CloseHandle(pipe_handler_);
      close(pipe_handler_);//add for linux
      pipe_handler_ = 0;//add for linux
    }

    printf("WinPipeClient::WinPipeClient::Stop[Check Point][1]\n");

    /// 设置运行标志位
    running_status_ = IS_STOPPED;
    printf("WinPipeClient::WinPipeClient::Stop[Check Point][End]\n");
  } while (0);

  printf("WinPipeClient::WinPipeClient::Stop[Exit]\n");
  return result;
}

int WinPipeClient::SendA(DWORD cmd, std::string& msg, DWORD* reply_cmd_result, std::string& reply_msg) {
  printf("WinPipeClient::WinPipeClient::SendA[Enter]\n");
  int result = 0;

  do {
    if (IS_RUNNING != running_status_) {
      printf("WinPipeClient::WinPipeClient::SendA[Fail][WinPipeClient is not running]\n");
      result = -1;
      break;
    }

    printf("WinPipeClient::WinPipeClient::SendA[Check Point][0]\n");

    if (0 >= pipe_handler_) {
      printf("WinPipeClient::WinPipeClient::SendA[Fail][pipe_handler_ is INVALID_HANDLE_VALUE]\n");
      result = -2;
      break;
    }

    printf("WinPipeClient::WinPipeClient::SendA[Check Point][1]\n");

    bool is_success = false;
    DWORD read_bytes = 0, write_bytes = 0, bytes_written, pipe_mode = 0;

    /// 连接pipe服务器成功，则切换为message-read模式
    //// pipe_mode = PIPE_READMODE_MESSAGE; //add for linux
    //// is_success = SetNamedPipeHandleState(pipe_handler_,/*/pipe handle*/&pipe_mode,/*/new pipe mode*/NULL,/*/don't set maximum bytes*/NULL);/*/don't set maximum time*/ //add for linux
    //TODO: add for lniux
    is_success = true;
    printf("WinPipeClient::WinPipeClient::SendA[Fail][The function[SetNamedPipeHandleState()][is_success] is true][true]\n");
    if (!is_success) {
      printf("WinPipeClient::WinPipeClient::SendA[Fail][The function[SetNamedPipeHandleState()] is failed][%s]\n", "GetLastError()");
      result = -3;
      break;
    }

    printf("WinPipeClient::WinPipeClient::SendA[Check Point][2]\n");

    /// 拼装发送数据
    PipeHeader request_pipe_header;
    memcpy(request_pipe_header.sig_, g_signatrue, g_signature_length);
    request_pipe_header.cmd_ = cmd;
    request_pipe_header.msg_len_ = msg.length();

    printf("WinPipeClient::WinPipeClient::SendA[Check Point][3]\n");

    char buffer[g_buffer_size] = {0,};
    memcpy(buffer, &request_pipe_header, sizeof(PipeHeader));
    memcpy(buffer+sizeof(PipeHeader), msg.c_str(), ((msg.length())*sizeof(char))) ;

    write_bytes = sizeof(PipeHeader) + (msg.length()+1)*sizeof(char);

    printf("WinPipeClient::WinPipeClient::SendA[Check Point][4]\n");

    /// 发送数据
    //is_success = WriteFile(pipe_handler_,/*pipe handle*/ buffer,/*//message*/write_bytes,/*// message length*/&bytes_written,/*// bytes written*/NULL);/*// not overlapped*/
    //TODO: add for linux
    long dwWritten = write(pipe_handler_, buffer, write_bytes );
    if(-1 == dwWritten || write_bytes != dwWritten)
    {
      is_success = false;
      printf(" 写入数据出错或无数据写入!!!!!");
      printf("WinPipeClient::WinPipeClient::SendA[Fail][The function[WriteFile()] is failed][%s]\n", "GetLastError()");
      result = -4;
      break;
    }
    if (!is_success) {
      printf("WinPipeClient::WinPipeClient::SendA[Fail][The function[WriteFile()] is failed][%s]\n", "GetLastError()");
      result = -4;
      break;
    }

    printf("WinPipeClient::WinPipeClient::SendA[Check Point][5]\n");

    /// 读取pipe服务器返回数据
    do {
      //is_success = ReadFile(pipe_handler_,/*/pipe handle*/ buffer,/*/buffer to receive reply*/g_buffer_size*sizeof(char),/*/size of buffer*/&read_bytes,/*/number of bytes read*/NULL);/*/not overlapped*/
      //TODO: add for linux
      char buf[g_buffer_size*sizeof(char)];
      printf("Please wait...\n");
      //while(1)
      {
        buf[0] = 0;
        //将文件描述符rfd中的内容放入缓冲区buf中
        ssize_t s = read(pipe_handler_,buf,read_bytes);
        if (s > 0) {
          //成功从管道中读取到数据
          buf[s-1] = 0;
          printf("[client]:> %s\n",buf);
        }
        else if(0 == s) {
          //管道中暂时没有数据
          printf("client quit, exit now!\n");
          is_success = true;
          break;
        }
        else {
          //读失败
          is_success = false;
          break;
        }
      }
      
      const char* err;
      perror(err);
      if (!is_success && err != NULL) {
        printf("WinPipeClient::WinPipeClient::SendA[Fail][The function[ReadFile()] is failed][%s]\n", err);
        break;
      }
    } while (!is_success);   // repeat loop if ERROR_MORE_DATA

    printf("WinPipeClient::WinPipeClient::SendA[Check Point][6]\n");

    if (!is_success) {
      printf("WinPipeClient::WinPipeClient::SendA[Fail][Can't read data from pipe]\n");
      result = -5;
      break;
    }

    printf("WinPipeClient::WinPipeClient::SendA[Check Point][7]\n");

    /// 解析pipe消息头
    PipeHeader reply_pipe_header;
    memcpy(&reply_pipe_header, buffer, sizeof(PipeHeader));

    printf("WinPipeClient::WinPipeClient::SendA[Check Point][8]\n");

    /// 比较签名
    if (0 != strncmp(reply_pipe_header.sig_, g_signatrue, g_signature_length)) {
      printf("WinPipeClient::WinPipeClient::SendA[Fail][The signature is not matched]\n");
      result = -6;
      break;
    }

    printf("WinPipeClient::WinPipeClient::SendA[Check Point][9]\n");

    /// 获取pipe服务器返回消息
    *reply_cmd_result = reply_pipe_header.cmd_result_;
    reply_msg = std::string(buffer+sizeof(PipeHeader));
    printf("WinPipeClient::WinPipeClient::SendA[Check Point][End]\n");
  } while (0);

  printf("WinPipeClient::WinPipeClient::SendA[Exit]\n");
  return result;
}

int WinPipeClient::SendW(DWORD cmd, std::wstring& msg, DWORD* reply_cmd_result, std::wstring& reply_msg) {
  int result = 0;

  do {
  } while (0);

  return result;
}

}

}
