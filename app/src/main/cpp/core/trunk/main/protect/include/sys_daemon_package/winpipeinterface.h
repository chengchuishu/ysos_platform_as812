/*
# winpipeinterface.h
# Definition of WinPipeInterface
# Created on: 2017-11-26 18:21:48
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20171126, created by JinChengZhe
*/

#ifndef SPS_WIN_PIPE_INTERFACE_H_
#define SPS_WIN_PIPE_INTERFACE_H_

#pragma warning(disable: 4995)

/// C++ Standard Headers
#include <memory>
#include <functional>
#ifdef _WIN32
/// Windows Platform Headers
#include <Windows.h>
#else
#include <string.h>
typedef unsigned long ULONG;
typedef unsigned int  UINT32;
typedef unsigned long DWORD;
#endif
/// Platform Headers
#include "../../../protect/include/sys_daemon_package/externcommon.h"

namespace ysos {

namespace win_pipe {

static const unsigned g_signature_length = 4;
static const char* g_signatrue = "YSOS";
static const unsigned g_buffer_size = 4096;

#pragma pack(push, 1)
typedef struct PipeHeader {
  char sig_[g_signature_length];
  ULONG cmd_;
  ULONG cmd_result_;
  ULONG msg_len_;

  PipeHeader(void)
    : cmd_(0),
      cmd_result_(0),
      msg_len_(0) {
    memset(sig_, 0, 4*sizeof(char));
  }

  ~PipeHeader(void) {
    memset(sig_, 0, 4*sizeof(char));
    cmd_ = 0;
    cmd_result_ = 0;
    msg_len_ = 0;
  }

  void Clear(void) {
    memset(sig_, 0, 4*sizeof(char));
    cmd_ = 0;
    cmd_result_ = 0;
    msg_len_ = 0;
  }
} *PipeHeaderPtr;
#pragma pack(pop)

typedef std::function<void(const DWORD /*命令类型*/, const std::string& /*请求消息*/, DWORD& /*命令处理结果*/, std::string& /*返回消息*/)> PipeMessageHandlerA;
typedef std::function<void(const DWORD /*命令类型*/, const std::wstring& /*请求消息*/, DWORD& /*命令处理结果*/, std::wstring& /*返回消息*/)> PipeMessageHandlerW;

class WinPipeInterface {
 public:
  /**
  *@brief 数值参数  // NOLINT
  */
  enum {
    IS_INITIAL = 0,                     ///< 初始状态  // NOLINT
    IS_RUNNING = 1,                     ///< 运行状态  // NOLINT
    IS_STOPPED = 2,                     ///< 停止状态  // NOLINT
    CMD_STOP_PIPE = 100,                ///< 停止管道  // NOLINT
    CMD_PIPE_SKIP = 101,                ///< 继续处理管道  // NOLINT
    CMD_RESULT_FAILED = 1000,           ///< 命令执行成功  // NOLINT
    CMD_RESULT_SUCCESS = 1001,          ///< 命令执行失败  // NOLINT
    CMD_RESULT_NOT_SUPPORTED = 1002,    ///< 命令不支持  // NOLINT
  };

  virtual ~WinPipeInterface(void) {}

  virtual int StartA(const std::string& pipe_name, const PipeMessageHandlerA& pipe_message_handler) = 0;
  virtual int StartW(const std::wstring& pipe_name, const PipeMessageHandlerW& pipe_message_handler) = 0;

  virtual int Stop(void) = 0;
};

}

}

#endif
