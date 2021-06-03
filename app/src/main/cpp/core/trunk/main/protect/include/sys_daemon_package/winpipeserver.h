/*
# winpipeserver.h
# Definition of WinPipeServer
# Created on: 2017-11-26 18:21:48
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20171126, created by JinChengZhe
*/

#ifndef SPS_WIN_PIPE_SERVER_H_
#define SPS_WIN_PIPE_SERVER_H_

/// Platform Headers
#include "../../../protect/include/sys_daemon_package/winpipeinterface.h"
/// Windows Standard Headers
#ifdef _WIN32
#include <windows.h>
#else
#define MAX_PATH  260
typedef void  *HANDLE;
#define WINAPI   /*__stdcall*/
typedef void  *LPVOID;
typedef unsigned char  BYTE;

#ifndef _MAC
typedef wchar_t WCHAR;    // wc,   16-bit UNICODE character
#else
// some Macintosh compilers don't define wchar_t in a convenient location, or define it as a char
typedef unsigned short WCHAR;    // wc,   16-bit UNICODE character


#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <boost/thread.hpp>

#endif
namespace ysos {

namespace win_pipe {

class WinPipeServer : public WinPipeInterface {
  YSOS_DECLARE_SINGLETON(WinPipeServer)

 public:
  virtual ~WinPipeServer(void);

  std::string GeneratePipeNameA(std::string& guid);
  std::wstring GeneratePipeNameW(std::wstring& guid);

  int StartA(const std::string& pipe_name, const PipeMessageHandlerA& pipe_message_handler);
  int StartW(const std::wstring& pipe_name,const PipeMessageHandlerW& pipe_message_handler);
  int Stop(void);

 private:
  WinPipeServer(const WinPipeServer&);
  WinPipeServer& operator=(const WinPipeServer);
  WinPipeServer(void);

  static unsigned /*WINAPI*/ PipeThreadProcA(LPVOID param_ptr);
  static unsigned /*WINAPI*/ PipeThreadProcW(LPVOID param_ptr);

  int HandlePipeConnectionMessageA(HANDLE pipe_handler, char* request_heap_ptr, char* reply_heap_ptr);
  int HandlePipeConnectionMessageW(HANDLE pipe_handler, WCHAR* request_heap_ptr, WCHAR* reply_heap_ptr);

  void PrintDSA(std::string naspace, std::string content); //add for linux

 private:
  volatile UINT32 running_status_;  ///< 是否是运行状态 1 表示运行 0 表示没运行  // NOLINT

  std::string pipe_nameA_;
  std::wstring pipe_nameW_;

  PipeMessageHandlerA pipe_message_handlerA_;
  PipeMessageHandlerW pipe_message_handlerW_;

  HANDLE thread_handle_;
  //int thread_handle_; //add for linux
  unsigned thread_id_;
};

}

}

#define GetWinPipeServer ysos::win_pipe::WinPipeServer::GetInstancePtr

#endif