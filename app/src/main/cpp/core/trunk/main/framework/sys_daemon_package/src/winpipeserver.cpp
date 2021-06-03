/*
# winpipeserver.h
# Definition of WinPipeServer
# Created on: 2017-11-26 18:21:48
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20171126, created by JinChengZhe
*/

/// Private Headers
#include "../../../protect/include/sys_daemon_package/winpipeserver.h"

/// Windows Platform Headers
#ifdef _WIN32
#include <tchar.h>   //C++ Standard Headers
#include <strsafe.h>
#include <atlconv.h>
#include <basetsd.h>
#else
#include <stdio.h>
#include <string.h>
#endif
using namespace ysos::extern_common;



namespace ysos {

namespace win_pipe {

YSOS_IMPLIMENT_SINGLETON(WinPipeServer)
WinPipeServer::WinPipeServer(void)
  : running_status_(IS_INITIAL),
    pipe_message_handlerA_(NULL),
    thread_handle_(NULL),
    thread_id_(0) {

}

WinPipeServer::~WinPipeServer(void) {

}

/**
 * 
 **/
void WinPipeServer::PrintDSA(std::string naspace, std::string content) 
{
  printf("%s::%s\n", naspace.c_str(), content.c_str());
  return;
}

std::string WinPipeServer::GeneratePipeNameA(std::string& guid) {
  DWORD sid = 9999;
  //ProcessIdToSessionId(GetCurrentProcessId(), &sid);
  sid = getpid(); //add for linux
  char pipe_name[MAX_PATH] = {0,};
  //StringCchPrintfA(pipe_name, MAX_PATH, "\\\\.\\pipe\\%s-S%d", guid, sid);
  sprintf(pipe_name, "\\\\.\\pipe\\%s-S%ld", guid.c_str(), sid);// add for linux
  return std::string(pipe_name);
}

std::wstring WinPipeServer::GeneratePipeNameW(std::wstring& guid) {
  DWORD sid = 9999;
  //ProcessIdToSessionId(GetCurrentProcessId(), &sid);
  sid = getpid(); //add for linux
  WCHAR pipe_name[MAX_PATH] = {0,};
  //StringCchPrintfW(pipe_name, MAX_PATH, L"\\\\.\\pipe\\%s-S%d", guid, sid);
  swprintf(pipe_name, MAX_PATH, L"\\\\.\\pipe\\%s-S%d", guid, sid); //add for linux
  return std::wstring(pipe_name);
}

int WinPipeServer::StartA(const std::string& pipe_name, const PipeMessageHandlerA& pipe_message_handler) {
  PrintDSA("WinPipeServer", "WinPipeServer::StartA[Enter]");
  int result = 0;

  do {
    if (IS_RUNNING == running_status_) {
      PrintDSA("WinPipeServer", "WinPipeServer::StartA[Check Point]WinPipeServer is already running");
      break;
    }

    PrintDSA("WinPipeServer", "WinPipeServer::StartA[Check Point][0]");

    if (true == pipe_name.empty()) {
      PrintDSA("WinPipeServer", "WinPipeServer::StartA[Fail][pipe_name is empty]");
      result = -1;
      break;
    }

    pipe_nameA_ = pipe_name;

    PrintDSA("WinPipeServer::WinPipeServer::StartA[Check Point][pipe_nameA_][%s]", pipe_nameA_.c_str());

    if (NULL != pipe_message_handler) {
      PrintDSA("WinPipeServer", "WinPipeServer::StartA[Check Point][pipe_message_handler is not NULL]");
      pipe_message_handlerA_ = pipe_message_handler;
    }

    PrintDSA("WinPipeServer", "WinPipeServer::StartA[Check Point][1]");

    /// for debug
    //  Sleep(20000);
    //TODO: add for linux
    /* old
    thread_handle_ = YSOSBEGINTHREADEX(NULL, 0, &WinPipeServer::PipeThreadProcA, NULL, 0, &thread_id_);
    if (NULL == thread_handle_) {
      PrintDSA("WinPipeServer", "WinPipeServer::StartA[Fail][The function[YSOSBEGINTHREADEX()] is failed][%d]GetLastError()");
      result = -2;
      break;
    }
    */
    running_status_ = IS_RUNNING;
    PrintDSA("WinPipeServer", "WinPipeServer::StartA[Check Point][End]");
  } while (0);

  PrintDSA("WinPipeServer", "WinPipeServer::StartA[Exit]");
  return result;
}

int WinPipeServer::StartW(const std::wstring& pipe_name, const PipeMessageHandlerW& pipe_message_handler) {
  PrintDSA("WinPipeServer", "WinPipeServer::StartW[Enter]");
  int result = 0;

  do {
  } while (0);

  PrintDSA("WinPipeServer", "WinPipeServer::StartW[Exit]");
  return result;
}

int WinPipeServer::Stop(void) {
  PrintDSA("WinPipeServer", "WinPipeServer::Stop[Enter]");
  int result = 0;

  do {
    if (IS_STOPPED == running_status_) {
      PrintDSA("WinPipeServer", "WinPipeServer::Stop[Check Point]WinPipeServer is already stopped");
      break;
    }

    PrintDSA("WinPipeServer", "WinPipeServer::Stop[Check Point][0]");

    running_status_ = IS_STOPPED;

    //::Sleep(2000);
    boost::this_thread::sleep(boost::posix_time::seconds(2));//add for linux

    PrintDSA("WinPipeServer", "WinPipeServer::Stop[Check Point][1]");

    if (NULL != thread_handle_) {
      PrintDSA("WinPipeServer", "WinPipeServer::Stop[Check Point][thread_handle_ is not NULL]");
      //CloseHandle(thread_handle_);
      close((long)thread_handle_); //add for linux
      thread_handle_ = NULL;
    }

    PrintDSA("WinPipeServer", "WinPipeServer::Stop[Check Point][End]");
  } while (0);

  PrintDSA("WinPipeServer", "WinPipeServer::Stop[Exit]");
  return result;
}

unsigned /*WINAPI*/ WinPipeServer::PipeThreadProcA(LPVOID param_ptr) {
  //PrintDSA("WinPipeServer", "WinPipeServer::PipeThreadProcA[Enter]");
  printf("WinPipeServer::WinPipeServer::PipeThreadProcA[Enter]");

  bool is_connected = false;
  HANDLE pipe_handler = /*INVALID_HANDLE_VALUE*/NULL, heap_handler = NULL;
  char *request_heap_ptr = nullptr, *reply_heap_ptr = nullptr;
  /*
  for (;;) {
    /// 判断对象全局指针是否为空
    if (nullptr == the_instance_ptr_) {
      PrintDSA("WinPipeServer", "WinPipeServer::PipeThreadProcA[Fail][this_ptr_ is NULL]");
      break;
    }

    BYTE sd[SECURITY_DESCRIPTOR_MIN_LENGTH];

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = &sd;

    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&sd, TRUE, (PACL)0, FALSE);

    /// 创建管道
    pipe_handler = CreateNamedPipeA(
                     the_instance_ptr_->pipe_nameA_.c_str(),                // pipe name
                     PIPE_ACCESS_DUPLEX,                            // read/write access
                     PIPE_TYPE_MESSAGE |                            // message type pipe
                     PIPE_READMODE_MESSAGE |                        // message-read mode
                     PIPE_WAIT,                                     // blocking mode
                     PIPE_UNLIMITED_INSTANCES,                      // max. instances
                     g_buffer_size,                                 // output buffer size
                     g_buffer_size,                                 // input buffer size
                     0,                                             // client time-out
                     &sa);                                         // default security attribute
    if (pipe_handler == INVALID_HANDLE_VALUE) {
      PrintDSA("WinPipeServer", "WinPipeServer::PipeThreadProcA[Fail][The function[CreateNamedPipeA()] is failed][%d]", GetLastError());
      break;
    }

    /// 等待客户端连接
    is_connected = ConnectNamedPipe(pipe_handler, NULL) ?
                   TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

    if (TRUE == is_connected) {
      /// 连接成功，读写数据
      PrintDSA("WinPipeServer", "WinPipeServer::PipeThreadProcA[Check Pint][pipe is connected]");

      /// 获取进程heap
      heap_handler = GetProcessHeap();
      if (NULL == heap_handler) {
        PrintDSA("WinPipeServer", "WinPipeServer::PipeThreadProcA[Fail][heap_handler is NULL]");
        break;
      }

      /// 请求消息heap
      request_heap_ptr = (char*)HeapAlloc(heap_handler, 0, g_buffer_size*sizeof(char));
      if (NULL == request_heap_ptr) {
        PrintDSA("WinPipeServer", "WinPipeServer::PipeThreadProcA[Fail][request_heap_ptr is NULL]");
        break;
      }

      /// 返回消息heap
      reply_heap_ptr = (char*)HeapAlloc(heap_handler, 0, g_buffer_size*sizeof(char));
      if (NULL == reply_heap_ptr) {
        PrintDSA("WinPipeServer", "WinPipeServer::PipeThreadProcA[Fail][reply_heap_ptr is NULL]");
        break;
      }

      /// 调用客户端连接发送消息处理函数
      int result = the_instance_ptr_->HandlePipeConnectionMessageA(pipe_handler, request_heap_ptr, reply_heap_ptr);

      /// 释放内存资源
      if (INVALID_HANDLE_VALUE != pipe_handler) {
        PrintDSA("WinPipeServer", "WinPipeServer::PipeThreadProcA[Check Pint][pipe_handler is not INVALID_HANDLE_VALUE]");
        FlushFileBuffers(pipe_handler);
        DisconnectNamedPipe(pipe_handler);
        CloseHandle(pipe_handler);
        pipe_handler = NULL;
      }

      if (NULL != request_heap_ptr) {
        PrintDSA("WinPipeServer", "WinPipeServer::PipeThreadProcA[Check Pint][request_heap_ptr is not NULL]");
        HeapFree(heap_handler, 0, request_heap_ptr);
        request_heap_ptr = nullptr;
      }

      if (NULL != reply_heap_ptr) {
        PrintDSA("WinPipeServer", "WinPipeServer::PipeThreadProcA[Check Pint][reply_heap_ptr is not NULL]");
        HeapFree(heap_handler, 0, reply_heap_ptr);
        reply_heap_ptr = nullptr;
      }

      /// pipe客户端断开连接或者读取错误，但大部分是pipe客户端断开，即客户端主动调用Stop
      if (CMD_PIPE_SKIP == result) {
        PrintDSA("WinPipeServer", "WinPipeServer::PipeThreadProcA[Fail][result is CMD_PIPE_SKIP, so to go on]");
        continue;
      } else if (CMD_STOP_PIPE == result) {
        PrintDSA("WinPipeServer", "WinPipeServer::PipeThreadProcA[Fail][result is CMD_STOP_PIPE, so to close pipe server]");
        /// 回掉上层pipe消息处理函数
        if (NULL != the_instance_ptr_->pipe_message_handlerA_) {
          PrintDSA("WinPipeServer", "WinPipeServer::PipeThreadProcA[Check Point][Call pipe_message_handlerA_()]");
          /// 自定义返回数据
          DWORD request_cmd = CMD_STOP_PIPE, reply_cmd_result = 0;
          std::string request_msg = "Stop Pipe Server", reply_msg;
          the_instance_ptr_->pipe_message_handlerA_(request_cmd, request_msg, reply_cmd_result, reply_msg);
        }
        break;
      } else {
        PrintDSA("WinPipeServer", "WinPipeServer::PipeThreadProcA[Fail][The function[the_instance_ptr_->HandlePipeConnectionMessageA()] is failed][%d]", result);
        break;
      }
    } else {
      /// 连接失败，关闭管道
      PrintDSA("WinPipeServer", "WinPipeServer::PipeThreadProcA[Check Pint][pipe is not connected]");
      CloseHandle(pipe_handler);
      pipe_handler = NULL;
    }
  }

  /// 释放内存资源
  if (INVALID_HANDLE_VALUE != pipe_handler) {
    PrintDSA("WinPipeServer", "WinPipeServer::PipeThreadProcA[Check Pint][pipe_handler is not INVALID_HANDLE_VALUE]");
    FlushFileBuffers(pipe_handler);
    DisconnectNamedPipe(pipe_handler);
    CloseHandle(pipe_handler);
    pipe_handler = NULL;
  }

  if (NULL != request_heap_ptr) {
    PrintDSA("WinPipeServer", "WinPipeServer::PipeThreadProcA[Check Pint][request_heap_ptr is not NULL]");
    HeapFree(heap_handler, 0, request_heap_ptr);
    request_heap_ptr = nullptr;
  }

  if (NULL != reply_heap_ptr) {
    PrintDSA("WinPipeServer", "WinPipeServer::PipeThreadProcA[Check Pint][reply_heap_ptr is not NULL]");
    HeapFree(heap_handler, 0, reply_heap_ptr);
    reply_heap_ptr = nullptr;
  }
  */
  printf("WinPipeServer::WinPipeServer::PipeThreadProcA[Exit]");
  return 0;
}

int WinPipeServer::HandlePipeConnectionMessageA(HANDLE pipe_handler, char* request_heap_ptr, char* reply_heap_ptr) {
  PrintDSA("WinPipeServer", "WinPipeServer::HandlePipeConnectionMessageA[Enter]");
  int result = 0;

  do {
    if (/*INVALID_HANDLE_VALUE*/NULL == pipe_handler ||
        NULL == request_heap_ptr ||
        NULL == reply_heap_ptr) {
      PrintDSA("WinPipeServer", "WinPipeServer::HandlePipeConnectionMessageA[Fail][pipe_handler/request_heap_ptr/reply_heap_ptr is invalid]");
      result = CMD_STOP_PIPE;  ///< 参数不正确，停止处理pipe
      break;
    }

    DWORD bytes_read = 0, reply_bytes = 0, byte_written = 0, reply_cmd_result = 0;
    bool is_success = false;
    PipeHeader request_pipe_header, reply_pipe_header;
    std::string reply_msg;

    for (;;) {
      if (IS_STOPPED == running_status_) {
        PrintDSA("WinPipeServer", "WinPipeServer::HandlePipeConnectionMessageA[Check Point]WinPipeServer is stopped");
        result = CMD_STOP_PIPE;  ///< 主动关闭pipe
        break;
      }

      /// 读取Pipe信息
      bytes_read = 0;
      memset(request_heap_ptr, 0, g_buffer_size*sizeof(char));
      /* add for linux
      is_success = ReadFile(
                     pipe_handler,                  // handle to pipe
                     request_heap_ptr,              // buffer to receive data
                     g_buffer_size*sizeof(char),    // size of buffer
                     &bytes_read,                   // number of bytes read
                     NULL);                         // not overlapped I/O
      */
      //TODO: add for linux
      if (!is_success || bytes_read == 0) {
        PrintDSA("WinPipeServer", "WinPipeServer::HandlePipeConnectionMessageA[Fail][The function[ReadFile()] is failed][GetLastError()]");
        //if (GetLastError() == ERROR_BROKEN_PIPE) {
        //TODO: add for linux
        char * err;
        perror(err);
        if (err) { //end TODO
          PrintDSA("WinPipeServer", "WinPipeServer::HandlePipeConnectionMessageA[Fail][client disconnected]");
        } else {
          PrintDSA("WinPipeServer", "WinPipeServer::HandlePipeConnectionMessageA[Fail][ReadFile failed]");
        }
        result = CMD_PIPE_SKIP;
        break;
      }

      /// 解析pipe消息头
      request_pipe_header.Clear();
      memcpy(&request_pipe_header, request_heap_ptr, sizeof(PipeHeader));

      /// 比较签名，签名不匹配则跳过
      if (0 != strncmp(request_pipe_header.sig_, g_signatrue, g_signature_length)) {
        PrintDSA("WinPipeServer", "WinPipeServer::HandlePipeConnectionMessageA[Fail][The signature is not matched]");
        continue;
      }

      reply_cmd_result = 0;
      reply_msg.clear();

      /// 回掉上层pipe消息处理函数
      if (NULL != pipe_message_handlerA_) {
        PrintDSA("WinPipeServer", "WinPipeServer::HandlePipeConnectionMessageA[Check Point][Call pipe_message_handlerA_()]");
        pipe_message_handlerA_(request_pipe_header.cmd_, std::string(request_heap_ptr+sizeof(PipeHeader)), reply_cmd_result, reply_msg);
      } else {
        PrintDSA("WinPipeServer", "WinPipeServer::HandlePipeConnectionMessageA[Check Point][pipe_message_handlerA_ is NULL]");
        /// 自定义返回数据
        reply_cmd_result = 0;
        reply_msg = "success";
      }

      /// 拼装返回消息
      reply_pipe_header.Clear();
      memcpy(reply_pipe_header.sig_, g_signatrue, g_signature_length);
      reply_pipe_header.cmd_ = request_pipe_header.cmd_;
      reply_pipe_header.cmd_result_ = reply_cmd_result;
      reply_pipe_header.msg_len_ = reply_msg.length();

      memset(reply_heap_ptr, 0, g_buffer_size*sizeof(char));
      memcpy(reply_heap_ptr, &reply_pipe_header, sizeof(PipeHeader));
      memcpy(reply_heap_ptr+sizeof(PipeHeader), reply_msg.c_str(), reply_msg.length());

      reply_bytes = sizeof(PipeHeader) + reply_msg.length() + 1;

      /// 返回pipe消息
      byte_written = 0;
      /* add for linux
      is_success = WriteFile(
                     pipe_handler,              // handle to pipe
                     reply_heap_ptr,            // buffer to write from
                     reply_bytes,               // number of bytes to write
                     &byte_written,             // number of bytes written
                     NULL);                     // not overlapped I/O
      */
      //TODO: add for linux
      if (!is_success || reply_bytes != byte_written) {
        PrintDSA("WinPipeServer", "WinPipeServer::HandlePipeConnectionMessageA[Fail][The function[WriteFile()] is failed][%d=GetLastError]");
        result = CMD_STOP_PIPE;
        break;
      }
    }  /// end of for
  } while (0);

  PrintDSA("WinPipeServer", "WinPipeServer::HandlePipeConnectionMessageA[Exit]");
  return result;
}

int WinPipeServer::HandlePipeConnectionMessageW(HANDLE pipe_handler, WCHAR* request_heap_ptr, WCHAR* reply_heap_ptr) {
  PrintDSA("WinPipeServer", "WinPipeServer::HandlePipeConnectionMessageW[Enter]");
  int result = 0;

  do {
  } while (0);

  PrintDSA("WinPipeServer", "WinPipeServer::HandlePipeConnectionMessageW[Exit]");
  return result;
}

}

}
