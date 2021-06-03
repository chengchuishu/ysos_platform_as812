/*
# minidump.h
# Definition of MiniDump
# Created on: 2017-12-15 11:45:32
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20171215, created by JinChengZhe
*/

/*************************************** add no used for linux  ************************************
#ifndef SDP_DAEMON_H_
#define SDP_DAEMON_H_

/// ThirdParty Headers
#include "../../../protect/include/sys_daemon_package/externcommon.h"
using namespace ysos::extern_common;

namespace ysos {

namespace mini_dump {

class MiniDump {
  YSOS_DECLARE_SINGLETON(MiniDump)

 public:
  virtual ~MiniDump(void);
#ifdef _UNICODE
  int WriteDump(const HMODULE module_handle = NULL, const std::wstring& dump_file_path = _T(""));
#else
  int WriteDump(const HMODULE module_handle = NULL, const std::string& dump_file_path = _T(""));
#endif

 private:
  MiniDump(const MiniDump&);
  MiniDump& operator=(const MiniDump);
  MiniDump(void);

  static LONG WINAPI HandleUnhandledException(LPEXCEPTION_POINTERS exception_info_ptr);

#ifdef _UNICODE
  std::wstring dump_file_path_;
  std::wstring module_name_;
#else
  std::string dump_file_path_;
  std::string module_name_;
#endif
};

}

}

#define GetMiniDump ysos::mini_dump::MiniDump::GetInstancePtr

#endif
*************************************** add no used for linux  ************************************/