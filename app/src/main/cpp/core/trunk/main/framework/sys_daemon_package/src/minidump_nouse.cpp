/*
# minidump.cpp
# Definition of MiniDump
# Created on: 2017-12-15 11:45:32
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20171215, created by JinChengZhe
*/
/*************************************** add no used for linux  ************************************
/// Private Headers
#include "../../../protect/include/sys_daemon_package/minidump.h"
#ifdef _WIN32
#include <dbghelp/DbgHelp.h>
#ifndef _WIN64
#pragma comment(lib, "DbgHelp_x86.Lib")
#else
#pragma comment(lib, "DbgHelp_x64.Lib")
#endif
#else

#endif

namespace ysos {

namespace mini_dump {

YSOS_IMPLIMENT_SINGLETON(MiniDump)

MiniDump::MiniDump(void) {

}

MiniDump::~MiniDump(void) {

}

#ifdef _UNICODE
int MiniDump::WriteDump(const HMODULE module_handle, const std::wstring& dump_file_path ) {
  return;
}
#else
int MiniDump::WriteDump(const HMODULE module_handle, const std::string& dump_file_path ) {
  int result = 0;

  do {
    /// 屏蔽系统错误弹框
    SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX|SEM_NOOPENFILEERRORBOX);

    /// 获取模块文件所在路径
    TCHAR module_file_name[MAX_PATH] = {0,};
    if (0 == GetModuleFileName(module_handle, module_file_name, MAX_PATH)) {
      result = GetLastError();
      PrintDSA("MiniDump", "MiniDump::WriteDump[Fail][The function[GetModuleFileName()] is failed][%d]", result);
      break;
    }

    /// 获取模块名
    TCHAR module_name[MAX_PATH] = {0,};
    if (TRUE != SUCCEEDED(StringCchCopy(module_name, MAX_PATH, PathFindFileName(module_file_name)))) {
      result = GetLastError();
      PrintDSA("MiniDump", "MiniDump::WriteDump[Fail][The function[StringCchCopy()] is failed][%d]", result);
      break;
    }

    /// 删除文件扩展名
    PathRemoveExtension(module_name);

    /// 设置模块名
    module_name_ = module_name;

    /// 如果没有指定dump文件路径，则在当前执行文件路径下生成dump文件夹保存dump
    if (true == dump_file_path.empty()) {
      if (TRUE != PathRemoveFileSpec(module_file_name)) {
        result = GetLastError();
        PrintDSA("MiniDump", "WriteDump[Fail][The function[PathRemoveFileSpec()] is failed][%d]", result);
        break;
      }
    }
    /// 如果指定dump文件夹，则按给定路径生成文件夹
    else {
      ZeroMemory(module_file_name, MAX_PATH);
      if (TRUE != SUCCEEDED(StringCchCopy(module_file_name, MAX_PATH, dump_file_path.c_str()))) {
        result = GetLastError();
        PrintDSA("MiniDump", "MiniDump::WriteDump[Fail][The function[StringCchCopy()] is failed][%d]", result);
        break;
      }
    }

    /// 设置dump文件路径
    if (TRUE != PathAppend(module_file_name, _T("dump"))) {
      result = GetLastError();
      PrintDSA("MiniDump", "MiniDump::WriteDump[Fail][The function[PathAppend()] is failed][%d]", result);
      break;
    }

    result = MDCreateDirectory(module_file_name);
    if (0 != result) {
      result = GetLastError();
      PrintDSA("MiniDump", "MiniDump::WriteDump[Fail][The function[MDCreateDirectory()] is failed][%d]", result);
      break;
    }

    dump_file_path_ = module_file_name;

    /// 设置exception filter
    SetUnhandledExceptionFilter(&MiniDump::HandleUnhandledException);
  } while (0);

  return result;
}
#endif

LONG WINAPI MiniDump::HandleUnhandledException(LPEXCEPTION_POINTERS exception_info_ptr) {
  LONG result = EXCEPTION_CONTINUE_SEARCH;

  do {
    /// 判断指针是否为空
    if (NULL == the_instance_ptr_) {
      PrintDSA("MiniDump", "MiniDump::HandleUnhandledException[Fail][the_instance_ptr_ is NULL]");
      break;
    }

    /// 获取本地时间
    SYSTEMTIME system_time = {0,};
    GetLocalTime(&system_time);

    /// 重组文件名
    TCHAR file_name[_MAX_FNAME] = {0,};
    if (TRUE != SUCCEEDED(StringCchPrintf(file_name,
                                          _MAX_FNAME,
                                          _T("%s__%4d%02d%02d_%02d%02d%02d.dmp"),
                                          the_instance_ptr_->module_name_.c_str(),
                                          system_time.wYear,
                                          system_time.wMonth,
                                          system_time.wDay,
                                          system_time.wHour,
                                          system_time.wMinute,
                                          system_time.wSecond))) {
      result = GetLastError();
      PrintDSA("MiniDump", "MiniDump::HandleUnhandledException[Fail][The function[StringCchPrintf()] is failed][%d]", result);
      result = EXCEPTION_CONTINUE_SEARCH;
      break;
    }

    /// 重组dump文件路径
    TCHAR dum_file_path[MAX_PATH] = {0,};
    if (TRUE != SUCCEEDED(StringCchCopy(dum_file_path, MAX_PATH, the_instance_ptr_->dump_file_path_.c_str()))) {
      result = GetLastError();
      PrintDSA("MiniDump", "MiniDump::HandleUnhandledException[Fail][The function[StringCchCopy()] is failed][%d]", result);
      result = EXCEPTION_CONTINUE_SEARCH;
      break;
    }

    if (TRUE != PathAppend(dum_file_path, file_name)) {
      result = GetLastError();
      PrintDSA("MiniDump", "MiniDump::HandleUnhandledException[Fail][The function[PathAppend()] is failed][%d]", result);
      result = EXCEPTION_CONTINUE_SEARCH;
      break;
    }

    /// 创建dump文件
    HANDLE file_handle = CreateFile(dum_file_path,
                                    GENERIC_WRITE,
                                    FILE_SHARE_WRITE,
                                    NULL,
                                    CREATE_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL);
    if (INVALID_HANDLE_VALUE == file_handle) {
      result = GetLastError();
      PrintDSA("MiniDump", "MiniDump::HandleUnhandledException[Fail][The function[CreateFile()] is failed][%d]", result);
      result = EXCEPTION_CONTINUE_SEARCH;
      break;
    }

    /// 写mini dump
    MINIDUMP_EXCEPTION_INFORMATION minidump_exception_information;
    minidump_exception_information.ThreadId = GetCurrentThreadId();
    minidump_exception_information.ExceptionPointers = exception_info_ptr;
    minidump_exception_information.ClientPointers = FALSE;

    if (TRUE != MiniDumpWriteDump(GetCurrentProcess(),
                                  GetCurrentProcessId(),
                                  file_handle,
                                  MiniDumpNormal,
                                  &minidump_exception_information,
                                  NULL,
                                  NULL)) {
      result = GetLastError();
      PrintDSA("MiniDump", "MiniDump::HandleUnhandledException[Fail][The function[MiniDumpWriteDump()] is failed][%d]", result);
      result = EXCEPTION_CONTINUE_SEARCH;
    } else {
      PrintDSA("MiniDump", "MiniDump::HandleUnhandledException[Success]");
      result = EXCEPTION_EXECUTE_HANDLER;
    }

    /// 关闭文件
    if (INVALID_HANDLE_VALUE != file_handle) {
      if (TRUE != CloseHandle(file_handle)) {
        result = GetLastError();
        PrintDSA("MiniDump", "MiniDump::HandleUnhandledException[Fail][The function[CloseHandle()] is failed][%d]", result);
        result = EXCEPTION_CONTINUE_SEARCH;
      }
    }
  } while (0);

  return result;
}

}

}

*************************************** add no used for linux  ************************************/