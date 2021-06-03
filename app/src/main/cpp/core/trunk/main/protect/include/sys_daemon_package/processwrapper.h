/*
# processwrapper.h
# Definition of processwrapper
# Created on: 2017-03-15 15:45:53
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20170315, created by JinChengZhe
*/

#ifndef SPS_PROCESS_WRAPPER_H_
#define SPS_PROCESS_WRAPPER_H_

/// C++ Standard Headers
#include <vector>
/// ThirdParty Headers
#include "../../../protect/include/sys_daemon_package/externcommon.h"
/// Windows Standard Headers
#ifdef _WIN32

// = 0x0600 for VISTA level from sdkddkver.h
#if (defined(_WIN32_WINNT) && (_WIN32_WINNT < _WIN32_WINNT_VISTA))
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include <TlHelp32.h>
#else
typedef char* LPTSTR;
typedef const char* LPCTSTR,PCTSTR;
typedef long LONG;
typedef unsigned long ULONG;
typedef void* HANDLE;
//#define MAX_PATH 260;
//#include <tr1/memory>
#endif

using namespace ysos::extern_common;


namespace ysos {

namespace process_wrapper {

/**
*@brief ProcessWrapper的具体实现，封装了进程相关函数  // NOLINT
*/
class ProcessWrapper {
  YSOS_DECLARE_SINGLETON(ProcessWrapper)

 public:
  /**
  *@brief 析构函数  // NOLINT
  *@param 无  // NOLINT
  *@return： 无  // NOLINT
  */
  ~ProcessWrapper();

  /**
  *@brief 基于Windows平台常用进程相关函数实现，其他平台后期实现
  */
#ifdef _WIN32
  /**
  *@brief 获取系统进程列表  // NOLINT
  *@param process_info[Input]： 进程列表  // NOLINT
  *@return： 成功返回TRUE，失败返回FALSE  // NOLINT
  */
  bool GetProcssInfos(std::vector<PROCESSENTRY32> &process_info);
  /**
  *@brief 执行进程  // NOLINT
  *@param process_file_path_ptr[Input]：  执行文件路径  // NOLINT
  *@param command_line_ptr[Input]：  命令行参数  // NOLINT
  *@param expand_environment_string_ptr[Input]：  进程运行环境变量  // NOLINT
  *@param hide_window[Input]：  是否隐藏窗口  // NOLINT
  *@param delay_time[Input]：  延迟时间，单位秒，例如10表示10秒 // NOLINT
  *@param use_shell[Input]：  以当前用户运行进程失败后，是否使用shell运行进程 // NOLINT
  *@param process_handle_ptr[Output]：  进程句柄指针  // NOLINT
  *@return： 成功返回0，失败返回相应错误码  // NOLINT
  */
  int ExecuteProcess(
    LPCTSTR process_file_path_ptr,
    LPCTSTR command_line_ptr = NULL,
    LPTSTR expand_environment_string_ptr = NULL,
    bool hide_window = FALSE,
    LONG delay_time = 0,
    bool use_shell = FALSE,
    HANDLE* process_handle_ptr = NULL);
  /**
  *@brief 执行进程，使用Shell  // NOLINT
  *@param process_file_path_ptr[Input]：  执行文件路径  // NOLINT
  *@param command_line_ptr[Input]：  命令行参数  // NOLINT
  *@param expand_environment_string_ptr[Input]：  进程运行环境变量  // NOLINT
  *@param hide_window[Input]：  是否隐藏窗口  // NOLINT
  *@param as_admin[Input]：  是否以管理员权限运行  // NOLINT
  *@param process_handle_ptr[Output]：  进程句柄指针  // NOLINT
  *@return： 成功返回0，失败返回相应错误码  // NOLINT
  */
  int ExecuteProcessWithShell(
    LPCTSTR process_file_path_ptr,
    LPCTSTR command_line_ptr = NULL,
    LPTSTR expand_environment_string_ptr = NULL,
    bool hide_window = FALSE,
    bool as_admin = FALSE,
    HANDLE* process_handle_ptr = NULL);
  /**
  *@brief 判断是否是运行的进程  // NOLINT
  *@param process_name_ptr[Input]： 进程名称  // NOLINT
  *@return： 成功返回TRUE，失败返回FALSE  // NOLINT
  */
  bool IsRunningProcess(LPCTSTR process_name_ptr);
  /**
  *@brief 判断是否是运行的进程  // NOLINT
  *@param process_file_path_ptr[Input]： 进程执行文件路径  // NOLINT
  *@return： 成功返回TRUE，失败返回FALSE  // NOLINT
  */
  bool IsRuningProcessEx(LPCTSTR process_file_path_ptr);
  /**
  *@brief 终止进程  // NOLINT
  *@param process_name_ptr[Input]： 进程名称  // NOLINT
  *@return： 成功返回0，失败返回相应错误码  // NOLINT
  */
  int KillProcess(LPCTSTR process_name_ptr);
  /**
  *@brief 终止进程  // NOLINT
  *@param process_file_path_ptr[Input]： 进程执行文件路径  // NOLINT
  *@return： 成功返回0，失败返回相应错误码  // NOLINT
  */
  int KillProcessEx(LPCTSTR process_file_path_ptr);
  /**
  *@brief 执行进程  // NOLINT
  *@param process_file_path_ptr[Input]：  执行文件路径  // NOLINT
  *@param command_line_ptr[Input]：  命令行参数  // NOLINT
  *@param expand_environment_string_ptr[Input]：  需要扩展的环境变量字符串  // NOLINT
  *@param hide_window[Input]：  是否隐藏窗口  // NOLINT
  *@param process_handle_ptr[Output]：  进程句柄指针  // NOLIN
  *@return： 成功返回0，失败返回相应错误码  // NOLINT/ NOLINT
  */
  int ExecuteProcessAsLogonAccountW(
    LPCTSTR process_file_path_ptr,
    LPCTSTR command_line_ptr = NULL,
    LPTSTR expand_environment_string_ptr = NULL,
    bool hide_window = FALSE,
    HANDLE* process_handle_ptr = NULL);
  /**
  *@brief 获取进程ID  // NOLINT
  *@param process_name_ptr[Input]： 进程名称  // NOLINT
  *@param process_ids[Input]： 进程ID  // NOLINT
  *@return： 成功返回0，失败返回相应错误码  // NOLINT
  */
  int GetProcessID(LPCTSTR process_name_ptr, std::vector<DWORD> &process_ids);
  /**
  *@brief 通过进程HWND,获取进程ID  // NOLINT
  *@param process_id[Input]： 进程HWND  // NOLINT
  *@return： 成功返回进程id，失败返回0  // NOLINT
  */
  ULONG GetProcessIDWithHWnd(HWND wnd_handle);
  /**
  *@brief 通过进程ID,获取进程HWnd  // NOLINT
  *@param process_id[Input]： 进程ID  // NOLINT
  *@return： 成功返回进程HWnd，失败返回NULL  // NOLINT
  */
  HWND GetProcessHWndWithID(const DWORD process_id);
  /**
  *@brief 通过进程ID,获取进程句柄  // NOLINT
  *@param process_id[Input]： 进程ID  // NOLINT
  *@return： 成功返回进程句柄，失败返回NULL  // NOLINT
  */
  HANDLE GetProcessHandleWithID(const DWORD process_id);
  /**
  *@brief 通过进程名称,获取进程句柄  // NOLINT
  *@param process_name_ptr[Input]： 进程名称  // NOLINT
  *@param process_handles[Input]： 进程句柄  // NOLINT
  *@return： 成功返回0，失败返回相应错误码  // NOLINT
  */
  int GetProcessHandle(LPCTSTR process_name_ptr, std::vector<HANDLE> &process_handles);
  /**
  *@brief 通过进程ID,获取进程文件路径  // NOLINT
  *@param process_id[Input]： 进程ID  // NOLINT
  *@param process_path[Input]： 进程文件路径  // NOLINT
  *@param process_path_size[Input]： 进程文件路径缓冲大小  // NOLINT
  *@return： 成功返回0，失败返回相应错误码  // NOLINT
  */
  int GetProcessPathWithID(const DWORD process_id, LPTSTR process_path, DWORD process_path_size = MAX_PATH);
  /**
  *@brief 调整进程权限  // NOLINT
  *@param privilege_ptr[Input]： 进程权限，SE_DEBUG_NAME为运行进程最高权限  // NOLINT
  *@param enable[Input]： 是否设置权限  // NOLINT
  *@return： 成功返回TRUE，失败返回FALSE  // NOLINT
  */
  bool EnablePrivilege(PCTSTR privilege_ptr, bool enable);
  /**
  *@brief 获取进程权限  // NOLINT
  *@param 无  // NOLINT
  *@return： 成功返回0，失败返回相应错误码  // NOLINT
  */
  int GetProcessPrivilege();
  /**
  *@brief 定制用户环境变量字符串  // NOLINT
  *@param user_environment_block 用户环境变量字符串  // NOLINT
  *@param expand_environment_strings 需要扩展用户环境变量字符串  // NOLINT
  *@return： 成功返回最新的环境信息字符串指针，失败返回空字符串  // NOLINT
  */
  std::wstring ProcessWrapper::CustomizeUserEnvironmentStringW(LPVOID user_environment_block, LPVOID expand_environment_strings);

  /**
  *@brief 刷新任务栏托盘   // NOLINT
  *@param 无  // NOLINT
  *@return： 成功返回TRUE，失败返回FALSE  // NOLINT
  */
  bool RefreshTaskbarIcon();

#else
/**
  *@brief 获取系统进程列表  // NOLINT
  *@param process_info[Input]： 进程列表  // NOLINT
  *@return： 成功返回TRUE，失败返回FALSE  // NOLINT
  */
  bool GetProcssInfos(std::vector<long> &process_info);
  /**
  *@brief 执行进程  // NOLINT
  *@param process_file_path_ptr[Input]：  执行文件路径  // NOLINT
  *@param command_line_ptr[Input]：  命令行参数  // NOLINT
  *@param expand_environment_string_ptr[Input]：  进程运行环境变量  // NOLINT
  *@param hide_window[Input]：  是否隐藏窗口  // NOLINT
  *@param delay_time[Input]：  延迟时间，单位秒，例如10表示10秒 // NOLINT
  *@param use_shell[Input]：  以当前用户运行进程失败后，是否使用shell运行进程 // NOLINT
  *@param process_handle_ptr[Output]：  进程句柄指针  // NOLINT
  *@return： 成功返回0，失败返回相应错误码  // NOLINT
  */
  int ExecuteProcess(
    LPCTSTR process_file_path_ptr,
    LPCTSTR command_line_ptr = NULL,
    LPTSTR expand_environment_string_ptr = NULL,
    bool hide_window = false,
    LONG delay_time = 0,
    bool use_shell = false,
    HANDLE* process_handle_ptr = NULL);
  /**
  *@brief 执行进程，使用Shell  // NOLINT
  *@param process_file_path_ptr[Input]：  执行文件路径  // NOLINT
  *@param command_line_ptr[Input]：  命令行参数  // NOLINT
  *@param expand_environment_string_ptr[Input]：  进程运行环境变量  // NOLINT
  *@param hide_window[Input]：  是否隐藏窗口  // NOLINT
  *@param as_admin[Input]：  是否以管理员权限运行  // NOLINT
  *@param process_handle_ptr[Output]：  进程句柄指针  // NOLINT
  *@return： 成功返回0，失败返回相应错误码  // NOLINT
  */
  int ExecuteProcessWithShell(
    LPCTSTR process_file_path_ptr,
    LPCTSTR command_line_ptr = NULL,
    LPTSTR expand_environment_string_ptr = NULL,
    bool hide_window = false,
    bool as_admin = false,
    HANDLE* process_handle_ptr = NULL);
  /**
  *@brief 判断是否是运行的进程  // NOLINT
  *@param process_name_ptr[Input]： 进程名称  // NOLINT
  *@return： 成功返回TRUE，失败返回FALSE  // NOLINT
  */
  bool IsRunningProcess(LPCTSTR process_name_ptr);
  /**
  *@brief 判断是否是运行的进程  // NOLINT
  *@param process_file_path_ptr[Input]： 进程执行文件路径  // NOLINT
  *@return： 成功返回TRUE，失败返回FALSE  // NOLINT
  */
  bool IsRuningProcessEx(LPCTSTR process_file_path_ptr);
  /**
  *@brief 终止进程  // NOLINT
  *@param process_name_ptr[Input]： 进程名称  // NOLINT
  *@return： 成功返回0，失败返回相应错误码  // NOLINT
  */
  int KillProcess(LPCTSTR process_name_ptr);
  /**
  *@brief 终止进程  // NOLINT
  *@param process_file_path_ptr[Input]： 进程执行文件路径  // NOLINT
  *@return： 成功返回0，失败返回相应错误码  // NOLINT
  */
  int KillProcessEx(LPCTSTR process_file_path_ptr);
  /**
  *@brief 执行进程  // NOLINT
  *@param process_file_path_ptr[Input]：  执行文件路径  // NOLINT
  *@param command_line_ptr[Input]：  命令行参数  // NOLINT
  *@param expand_environment_string_ptr[Input]：  需要扩展的环境变量字符串  // NOLINT
  *@param hide_window[Input]：  是否隐藏窗口  // NOLINT
  *@param process_handle_ptr[Output]：  进程句柄指针  // NOLIN
  *@return： 成功返回0，失败返回相应错误码  // NOLINT/ NOLINT
  */
  int ExecuteProcessAsLogonAccountW(
    LPCTSTR process_file_path_ptr,
    LPCTSTR command_line_ptr = NULL,
    LPTSTR expand_environment_string_ptr = NULL,
    bool hide_window = false,
    HANDLE* process_handle_ptr = NULL);
  /**
  *@brief 获取进程ID  // NOLINT
  *@param process_name_ptr[Input]： 进程名称  // NOLINT
  *@param process_ids[Input]： 进程ID  // NOLINT
  *@return： 成功返回0，失败返回相应错误码  // NOLINT
  */
  int GetProcessID(LPCTSTR process_name_ptr, std::vector<unsigned long> &process_ids);
  /**
  *@brief 通过进程HWND,获取进程ID  // NOLINT
  *@param process_id[Input]： 进程HWND  // NOLINT
  *@return： 成功返回进程id，失败返回0  // NOLINT
  */
  //ULONG GetProcessIDWithHWnd(HWND wnd_handle);
  /**
  *@brief 通过进程ID,获取进程HWnd  // NOLINT
  *@param process_id[Input]： 进程ID  // NOLINT
  *@return： 成功返回进程HWnd，失败返回NULL  // NOLINT
  */
  //HWND GetProcessHWndWithID(const DWORD process_id);
  /**
  *@brief 通过进程ID,获取进程句柄  // NOLINT
  *@param process_id[Input]： 进程ID  // NOLINT
  *@return： 成功返回进程句柄，失败返回NULL  // NOLINT
  */
  HANDLE GetProcessHandleWithID(const unsigned long process_id);
  /**
  *@brief 通过进程名称,获取进程句柄  // NOLINT
  *@param process_name_ptr[Input]： 进程名称  // NOLINT
  *@param process_handles[Input]： 进程句柄  // NOLINT
  *@return： 成功返回0，失败返回相应错误码  // NOLINT
  */
  int GetProcessHandle(LPCTSTR process_name_ptr, std::vector<HANDLE> &process_handles);
  /**
  *@brief 通过进程ID,获取进程文件路径  // NOLINT
  *@param process_id[Input]： 进程ID  // NOLINT
  *@param process_path[Input]： 进程文件路径  // NOLINT
  *@param process_path_size[Input]： 进程文件路径缓冲大小  // NOLINT
  *@return： 成功返回0，失败返回相应错误码  // NOLINT
  */
  int GetProcessPathWithID(const unsigned long process_id, LPTSTR process_path, unsigned long process_path_size = 260);
  /**
  *@brief 调整进程权限  // NOLINT
  *@param privilege_ptr[Input]： 进程权限，SE_DEBUG_NAME为运行进程最高权限  // NOLINT
  *@param enable[Input]： 是否设置权限  // NOLINT
  *@return： 成功返回TRUE，失败返回FALSE  // NOLINT
  */
  bool EnablePrivilege(PCTSTR privilege_ptr, bool enable);
  /**
  *@brief 获取进程权限  // NOLINT
  *@param 无  // NOLINT
  *@return： 成功返回0，失败返回相应错误码  // NOLINT
  */
  int GetProcessPrivilege();
  /**
  *@brief 定制用户环境变量字符串  // NOLINT
  *@param user_environment_block 用户环境变量字符串  // NOLINT
  *@param expand_environment_strings 需要扩展用户环境变量字符串  // NOLINT
  *@return： 成功返回最新的环境信息字符串指针，失败返回空字符串  // NOLINT
  */
  //std::wstring ProcessWrapper::CustomizeUserEnvironmentStringW(void* user_environment_block, void* expand_environment_strings);

  /**
  *@brief 刷新任务栏托盘   // NOLINT
  *@param 无  // NOLINT
  *@return： 成功返回TRUE，失败返回FALSE  // NOLINT
  */
  bool RefreshTaskbarIcon();
  
#endif

 private:
  ProcessWrapper& operator= (const ProcessWrapper&);
  ProcessWrapper(const ProcessWrapper&);
  ProcessWrapper();
};

}

}

#define GetProcessWrapper ysos::process_wrapper::ProcessWrapper::GetInstancePtr

#endif