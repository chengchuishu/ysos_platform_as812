/*
# winpipeclient.h
# Definition of WinPipeClient
# Created on: 2017-11-26 18:21:48
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20171126, created by JinChengZhe
*/

#ifndef SPS_WIN_PIPE_CLIENT_H_
#define SPS_WIN_PIPE_CLIENT_H_

/// Platform Headers
#include "../../../protect/include/sys_daemon_package/winpipeinterface.h"
/// Windows Standard Headers
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace ysos::extern_common;

namespace ysos {

namespace win_pipe {

class WinPipeClient : public WinPipeInterface {
  YSOS_DECLARE_SINGLETON(WinPipeClient)

 public:
  virtual ~WinPipeClient(void);

  int StartA(const std::string& pipe_name, const PipeMessageHandlerA& pipe_message_handler);
  int StartW(const std::wstring& pipe_name, const PipeMessageHandlerW& pipe_message_handler);
  int Stop(void);

  int SendA(DWORD cmd, std::string& msg, DWORD* reply_cmd_result, std::string& reply_msg);
  int SendW(DWORD cmd, std::wstring& msg, DWORD* reply_cmd_result, std::wstring& reply_msg);

 private:
  WinPipeClient(const WinPipeClient&);
  WinPipeClient& operator=(const WinPipeClient);
  WinPipeClient(void);

  volatile UINT32 running_status_;  ///< 是否是运行状态 1 表示运行 0 表示没运行  // NOLINT

  std::string pipe_nameA_;
  std::wstring pipe_nameW_;

  //void* pipe_handler_;  //add for linux
  int pipe_handler_;
};

}

}

#define GetWinPipeClient ysos::win_pipe::WinPipeClient::GetInstancePtr

#endif
