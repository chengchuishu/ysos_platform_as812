/*
# processwrapper.cpp
# Definition of processwrapper
# Created on: 2017-03-15 15:45:53
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20170315, created by JinChengZhe
*/

/// Private Headers
#include "../../../protect/include/sys_daemon_package/processwrapper.h"
#include <map>
#include <algorithm>
#include <string>
/// Windows Standard Headers
#ifdef _WIN32
#include <atlstr.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include <ShellAPI.h>
#pragma comment(lib, "Shell32.lib")
#include <WtsApi32.h>
#pragma comment(lib, "Wtsapi32.lib")
#include <Userenv.h>
#pragma comment(lib, "Userenv.lib")
#include <Shlobj.h>
#pragma comment(lib, "Shell32.lib")
#include <Sddl.h>
#pragma comment(lib, "Advapi32.lib")
#include <psapi.h>
#pragma comment(lib,"psapi.lib")
#else
#include <boost/function.hpp>

#include <libgen.h>
#include <unistd.h>
#endif


namespace ysos {

namespace process_wrapper {

YSOS_IMPLIMENT_SINGLETON(ProcessWrapper)
ProcessWrapper::ProcessWrapper() {
}

ProcessWrapper::~ProcessWrapper() {
}

#ifdef _WIN32
bool ProcessWrapper::GetProcssInfos(std::vector<PROCESSENTRY32> &process_info) {
  process_info.clear();

  bool result = FALSE;

  HANDLE snapshot_handle = INVALID_HANDLE_VALUE;

  do {
    snapshot_handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE == snapshot_handle) {
      break;
    }

    PROCESSENTRY32 process_entry;
    process_entry.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(snapshot_handle, &process_entry)) {
      break;
    }

    do {
      process_info.push_back(process_entry);
    } while (Process32Next(snapshot_handle, &process_entry));

    result = TRUE;
  } while (0);

  if (INVALID_HANDLE_VALUE != snapshot_handle) {
    CloseHandle(snapshot_handle);
  }

  return result;
}

int ProcessWrapper::ExecuteProcess(
  LPCTSTR process_file_path_ptr,
  LPCTSTR command_line_ptr,
  LPTSTR expand_environment_string_ptr,
  bool hide_window,
  LONG delay_time,
  bool use_shell,
  HANDLE* process_handle_ptr) {
  int result = -1;

  EnablePrivilege(SE_DEBUG_NAME, TRUE);

  do {
    if (NULL == process_file_path_ptr ||0 == lstrlen(process_file_path_ptr)) {
#ifdef _YSOS_UNICODE
      PrintDS(_T("ProcessWrapper"), _T("process_file_path_ptr is NULL"));
#else
      PrintDSA("ProcessWrapper", "process_file_path_ptr is NULL");
#endif
      break;
    }

    if (!PathFileExists(process_file_path_ptr)) {
#ifdef _YSOS_UNICODE
      PrintDS(_T("ProcessWrapper"), _T("There is not the file.[%s]"), process_file_path_ptr);
#else
      PrintDSA("ProcessWrapper", "There is not the file.[%s]", process_file_path_ptr);
#endif
      break;
    }

    PTSTR process_name = PathFindFileName(process_file_path_ptr);
    if (NULL == process_name ||0 == lstrlen(process_name)) {
#ifdef _YSOS_UNICODE
      PrintDS(_T("ProcessWrapper"), _T("process_name is empty."));
#else
      PrintDSA("ProcessWrapper", "process_name is empty.");
#endif
      break;
    }

    if (TRUE == IsRunningProcess(process_name)) {
#ifdef _YSOS_UNICODE
      PrintDS(_T("ProcessWrapper"), _T("The process is already running.[%s]"), process_name);
#else
      PrintDSA("ProcessWrapper", "The process is already running.[%s]", process_name);
#endif
  
      result = 0;
      break;
    }

    if (0 != delay_time) {
#ifdef _YSOS_UNICODE
      PrintDS(_T("ProcessWrapper"), _T("delay_time[%d]"), delay_time);
#else
      PrintDSA("ProcessWrapper", "delay_time[%d]", delay_time);
#endif
      Sleep(delay_time*1000);
    }

    if (0 == ExecuteProcessAsLogonAccountW(
          process_file_path_ptr,
          command_line_ptr,
          expand_environment_string_ptr,
          hide_window,
          process_handle_ptr)) {
#ifdef _YSOS_UNICODE
      PrintDS(_T("ProcessWrapper"), _T("The function[ExecuteProcessAsLogonAccount()] succeed."));
#else
      PrintDSA("ProcessWrapper", "The function[ExecuteProcessAsLogonAccount()] succeed.");
#endif
      result = 0;
      break;
    }

    if (TRUE == use_shell) {
      if (0 != ExecuteProcessWithShell(
            process_file_path_ptr,
            command_line_ptr,
            expand_environment_string_ptr,
            hide_window,
            FALSE,
            process_handle_ptr)) {
#ifdef _YSOS_UNICODE
        PrintDS(_T("ProcessWrapper"), _T("The function[ExecuteProcessWithShell()] fails."));
#else
        PrintDSA("ProcessWrapper", "The function[ExecuteProcessWithShell()] fails.");
#endif
        break;
      }
    }

    result = 0;
  } while (0);

  EnablePrivilege(SE_DEBUG_NAME, FALSE);

  return result;
}

int ProcessWrapper::ExecuteProcessWithShell(
  LPCTSTR process_file_path_ptr,
  LPCTSTR command_line_ptr,
  LPTSTR expand_environment_string_ptr,
  bool hide_window,
  bool as_admin,
  HANDLE* process_handle_ptr) {
  int result = -1;

  do {
    if (NULL == process_file_path_ptr ||
        0 == lstrlen(process_file_path_ptr)) {
      break;
    }

    if (!PathFileExists(process_file_path_ptr)) {
      break;
    }

    PTSTR process_name = PathFindFileName(process_file_path_ptr);
    if (NULL == process_name ||
        0 == lstrlen(process_name)
       ) {
      break;
    }

    if (TRUE == IsRunningProcess(process_name)) {
      result = 0;
      break;
    }

//     if (S_OK != CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)) {
//       result = GetLastError();
//       break;
//     }

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    SHELLEXECUTEINFO shell_execute_info;
    ZeroMemory(&shell_execute_info, sizeof(shell_execute_info));
    shell_execute_info.cbSize = sizeof(SHELLEXECUTEINFOW);
    shell_execute_info.hwnd = NULL;
    shell_execute_info.fMask = SEE_MASK_FLAG_DDEWAIT|SEE_MASK_NOCLOSEPROCESS;

    /// 以管理员权限运行进程，但是会弹警告框
    if (TRUE == as_admin) {
      shell_execute_info.lpVerb = _TEXT("runas");
    } else {
      shell_execute_info.lpVerb = _TEXT("open");
    }

    shell_execute_info.lpFile = process_file_path_ptr;
    shell_execute_info.lpParameters = command_line_ptr;
    if (hide_window) {
      shell_execute_info.nShow = SW_HIDE;
    } else {
      shell_execute_info.nShow = SW_SHOW;
    }

    if (NULL != expand_environment_string_ptr) {
      shell_execute_info.lpDirectory = expand_environment_string_ptr;
    }

    if (!ShellExecuteEx(&shell_execute_info)) {
      result = GetLastError();
      CoUninitialize();
      break;
    }

    if (shell_execute_info.hInstApp <= (HINSTANCE)32) {
      result = GetLastError();
      CoUninitialize();
      break;
    }

    if (NULL != process_handle_ptr) {
      *process_handle_ptr = shell_execute_info.hProcess;
    }

    CoUninitialize();

    result = 0;
  } while (0);

  return result;
}

#ifdef UNICODE
std::string TcharToString(TCHAR *STR)
{
  int iLen = WideCharToMultiByte(CP_ACP, 0,STR, -1, NULL, 0, NULL, NULL);
  char* chRtn =new char[iLen*sizeof(char)];
  if( chRtn == NULL) return "";
  WideCharToMultiByte(CP_ACP, 0, STR, -1, chRtn, iLen, NULL, NULL);
  std::string str(chRtn);
  delete [] chRtn;
  chRtn = NULL;

  return str;
}
#endif

static std::string GetProcessUserAccount(DWORD process_id) {
  //DWORD dwProcessId = 348;
  HANDLE hProcess2 = OpenProcess(PROCESS_QUERY_INFORMATION , FALSE, process_id);
  HANDLE hToken;
  if (NULL != hProcess2)
  {
    bool bTokenOK = OpenProcessToken(hProcess2, TOKEN_QUERY, &hToken);
    //wcout << L"OpenProcess is successful." << endl;
    if (bTokenOK)
    {
      //wcout << L"OpenProcessToken is successful." << endl;

      DWORD dwSize;
      bool bGetTokenOK = GetTokenInformation(hToken, TokenUser, NULL, 0, &dwSize);
      if (!bGetTokenOK)
      {
        DWORD dwResult = GetLastError();
        if ( dwResult != ERROR_INSUFFICIENT_BUFFER)
        {
          //fprintf(stderr, "GetTokenInformation 失败，GetLastError()=%d\n", GetLastError());
          return "";
        }
      }

      PTOKEN_USER  pTokenUser = (PTOKEN_USER)new BYTE[dwSize];
      bGetTokenOK = GetTokenInformation(hToken, TokenUser, pTokenUser, dwSize, &dwSize);
      if(bGetTokenOK)
      {
        bool bLookupSid;
        SID_NAME_USE snu;
        TCHAR szUserName[1024];
        int nLen = sizeof(szUserName)/sizeof(TCHAR);
        DWORD dwUserSize = sizeof(szUserName)/sizeof(TCHAR);
        TCHAR szDomain[1024];
        DWORD cbDomain = sizeof(szDomain)/sizeof(TCHAR);
        bLookupSid = LookupAccountSid(NULL, pTokenUser->User.Sid, 
          szUserName, &dwUserSize, 
          szDomain, &cbDomain, &snu);

        if(!bLookupSid)
        {
          DWORD dwRet = ::GetLastError();
          //printf("LookupAccountsid faild...GetLastError() = %d\n", dwRet);
		  return "";
        }
#ifdef UNICODE
		std::string strUserName = TcharToString(szUserName);
		return strUserName;
#else
		return szUserName;
#endif
      }
    }
  }

  return "";
}

bool ProcessWrapper::IsRunningProcess(LPCTSTR process_name_ptr) {
  bool result = FALSE;
  bool kill_process = FALSE;

  HANDLE snapshot_handle = INVALID_HANDLE_VALUE;

  do {

    if (NULL == process_name_ptr) {
      break;
    }

    snapshot_handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE == snapshot_handle) {
      break;
    }

    PROCESSENTRY32 process_entry;
    process_entry.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(snapshot_handle, &process_entry)) {
      break;
    }

    do {
      if (0 == lstrcmpi(process_entry.szExeFile, process_name_ptr)) {
        result = TRUE;
        break;
      }

    } while (Process32Next(snapshot_handle, &process_entry));

    if(TRUE == result) {
      std::string process_account = GetProcessUserAccount(process_entry.th32ProcessID);
      if (process_account == "SYSTEM" || process_account == "system") {
        kill_process = TRUE;
      }
    }

  } while (0);

  if (INVALID_HANDLE_VALUE != snapshot_handle) {
    CloseHandle(snapshot_handle);
  }

  if(TRUE == kill_process) {
    KillProcess(process_name_ptr);
  }

  return result;
}

bool ProcessWrapper::IsRuningProcessEx(LPCTSTR process_file_path_ptr) {

  bool result = FALSE;

  do {
    if (!PathFileExists(process_file_path_ptr)) {
      break;
    }

    PTSTR process_name = PathFindFileName(process_file_path_ptr);
    if (NULL == process_name ||0 == lstrlen(process_name)) {
      break;
    }

    if (TRUE != IsRunningProcess(process_name)) {
      break;
    }

    result =TRUE;
  } while (0);

  return result;
}

int ProcessWrapper::KillProcess(LPCTSTR process_name_ptr) {
  int result = 0;

  HANDLE snapshot_handle = INVALID_HANDLE_VALUE;

  do {

    if (NULL == process_name_ptr) {
      result = -1;
      break;
    }

    snapshot_handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE == snapshot_handle) {
      result = GetLastError();
      break;
    }

    PROCESSENTRY32 process_entry;
    process_entry.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(snapshot_handle, &process_entry)) {
      result = GetLastError();
      break;
    }

    HANDLE process_handle = INVALID_HANDLE_VALUE;
    do {
      if (0 == lstrcmpi(process_entry.szExeFile, process_name_ptr)) {

        process_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_entry.th32ProcessID);
        if (NULL == process_handle) {
          result = GetLastError();
          continue;
        }

        if (TRUE != TerminateProcess(process_handle, 1)) {
          result = GetLastError();
        }
      }

    } while (Process32Next(snapshot_handle, &process_entry));

    if (NULL != process_handle) {
      CloseHandle(process_handle);
    }

  } while (0);

  if (INVALID_HANDLE_VALUE != snapshot_handle) {
    CloseHandle(snapshot_handle);
  }

  return result;
}

int ProcessWrapper::KillProcessEx(LPCTSTR process_file_path_ptr) {
  int result = -1;

  do {
    if (!PathFileExists(process_file_path_ptr)) {
      break;
    }

    PTSTR process_name = PathFindFileName(process_file_path_ptr);
    if (
      NULL == process_name ||
      0 == lstrlen(process_name)
    ) {
      break;
    }

    result = KillProcess(process_name);
    if (0 != result) {
      break;
    }

    result =0;
  } while (0);

  return result;
}

int ProcessWrapper::GetProcessPrivilege() {

  int result = -1;

  HANDLE process_token_handle = NULL;
  do {
    TCHAR user_name[MAX_PATH] = {0};
    DWORD user_name_size = MAX_PATH;
    if (TRUE != GetUserName(user_name, &user_name_size)) {
      result = GetLastError();
      break;
    }

    if (TRUE != OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &process_token_handle)) {
      result = GetLastError();
      break;
    }

    TOKEN_ELEVATION_TYPE token_elevation_type;
    DWORD return_length = 0;
    if (TRUE == GetTokenInformation(
          process_token_handle,
          TokenElevationType,
          &token_elevation_type,
          sizeof(TOKEN_ELEVATION_TYPE),
          &return_length)) {

      BYTE admin_sid[SECURITY_MAX_SID_SIZE];
      return_length = SECURITY_MAX_SID_SIZE;

      if (TRUE != CreateWellKnownSid(WinBuiltinAdministratorsSid, NULL, &admin_sid, &return_length)) {
        result = GetLastError();
        break;
      }

      bool is_admin = FALSE;
      if (token_elevation_type == TokenElevationTypeLimited) {
        HANDLE unfiltered_token_handle = NULL;
        if (TRUE != GetTokenInformation(process_token_handle, TokenLinkedToken, (void *)&unfiltered_token_handle, sizeof(HANDLE), &return_length)) {
          result = GetLastError();
          break;
        }

        if (TRUE != CheckTokenMembership(unfiltered_token_handle, &admin_sid, &is_admin)) {
          CloseHandle(unfiltered_token_handle);
          result = GetLastError();
          break;
        }

        result = 0;

        CloseHandle(unfiltered_token_handle);
      } else {
        is_admin = IsUserAnAdmin();
        result = 0;
      }
      /// for debug
#if 1
      if (is_admin) {
#ifdef _YSOS_UNICODE
        PrintDS(_T("ProcessWrapper"), _T("[%s]属于管理员群"), user_name);
#else
        PrintDSA("ProcessWrapper", "[%s]属于管理员群", user_name);
#endif
      } else {
#ifdef _YSOS_UNICODE
        PrintDS(_T("ProcessWrapper"), _T("[%s]不属于管理员群"), user_name);
#else
        PrintDSA("ProcessWrapper", "[%s]不属于管理员群", user_name);
#endif
      }

      switch (token_elevation_type) {
      case TokenElevationTypeDefault:
#ifdef _YSOS_UNICODE
        PrintDS(_T("ProcessWrapper"), _T("基本用户或者UAC功能已关闭"));
#else
        PrintDSA("ProcessWrapper", "基本用户或者UAC功能已关闭");
#endif
        break;
      case TokenElevationTypeFull:
#ifdef _YSOS_UNICODE
        PrintDS(_T("ProcessWrapper"), _T("权限提升"));
#else
        PrintDSA("ProcessWrapper", "权限提升");
#endif
        break;
      case TokenElevationTypeLimited:
#ifdef _YSOS_UNICODE
        PrintDS(_T("ProcessWrapper"), _T("以有限权限运行"));
#else
        PrintDSA("ProcessWrapper", "以有限权限运行");
#endif
        break;
      default:
        break;
      }
    }
#endif
  } while (0);

  if (NULL != process_token_handle) {
    CloseHandle(process_token_handle);
    process_token_handle = NULL;
  }

  return result;
}

int ProcessWrapper::ExecuteProcessAsLogonAccountW(
  LPCTSTR process_file_path_ptr,
  LPCTSTR command_line_ptr,
  LPTSTR expand_environment_string_ptr,
  bool hide_window,
  HANDLE* process_handle_ptr) {
  PrintDSA("ProcessWrapper", "ProcessWrapper::ExecuteProcessAsLogonAccount[Enter]");

  int result = -1;

  HANDLE active_user_tocken_handle = NULL;  ///< 当前活动用户的token句柄
  HANDLE dummy_active_user_tocken_handle = NULL;  ///< 复制的当前活动用户的token句柄
  LPVOID  active_user_environment_block_ptr = NULL;  ///< 复制的当前活动用户的环境信息

  do {
    if (NULL == process_file_path_ptr ||
        0 == _tcslen(process_file_path_ptr)) {
      PrintDS(L"ProcessWrapper", L"ProcessWrapper::ExecuteProcessAsLogonAccount[process_file_path_ptr is empty]");
      break;
    }

    PrintDS(L"ProcessWrapper", L"ProcessWrapper::ExecuteProcessAsLogonAccount[Check Point][0]");

    if (!PathFileExists(process_file_path_ptr)) {
      break;
    }

    PTSTR process_name = PathFindFileName(process_file_path_ptr);
    if (NULL == process_name ||
        0 == lstrlen(process_name)) {
      break;
    }

    PrintDS(L"ProcessWrapper", L"ProcessWrapper::ExecuteProcessAsLogonAccount[Check Point][1]");

    if (TRUE == IsRunningProcess(process_name)) {
      result = 0;
      break;
    }

#ifndef _YSOS_UNICODE
    std::wstring process_file_path, command_line_string, expand_environment_string;
    process_file_path = AcsiiToUnicode(std::string(process_file_path_ptr));
    PrintDS(L"ProcessWrapper", L"ProcessWrapper::ExecuteProcessAsLogonAccount[process_file_path][%s]", process_file_path.c_str());

    if (NULL != command_line_ptr &&
        0 != _tcslen(command_line_ptr)) {
      PrintDSA("ProcessWrapper", "ProcessWrapper::ExecuteProcessAsLogonAccount[command_line_ptr][%s]", command_line_ptr);
      command_line_string = AcsiiToUnicode(std::string(command_line_ptr));
      PrintDS(L"ProcessWrapper", L"ProcessWrapper::ExecuteProcessAsLogonAccount[command_line_ptr][%s]", command_line_string.c_str());
    }

    PrintDS(L"ProcessWrapper", L"ProcessWrapper::ExecuteProcessAsLogonAccount[Check Point][2]");

    if (NULL != expand_environment_string_ptr &&
        0 != _tcslen(expand_environment_string_ptr)) {
      PrintDSA("ProcessWrapper", "ProcessWrapper::ExecuteProcessAsLogonAccount[expand_environment_string_ptr][%s]", expand_environment_string_ptr);
      expand_environment_string = AcsiiToUnicode(std::string(expand_environment_string_ptr));
      PrintDS(L"ProcessWrapper", L"ProcessWrapper::ExecuteProcessAsLogonAccount[expand_environment_string][%s]", expand_environment_string.c_str());
    }
#endif // _YSOS_UNICODE

    PrintDS(L"ProcessWrapper", L"ProcessWrapper::ExecuteProcessAsLogonAccount[Check Point][3]");

    /// 获取当前活动用户的session id
    DWORD active_user_session_id = WTSGetActiveConsoleSessionId();
    if (0xFFFFFFFF == active_user_session_id) {
      result = GetLastError();
      PrintDS(L"ProcessWrapper", L"ProcessWrapper::ExecuteProcessAsLogonAccount[Fail][The function[WTSGetActiveConsoleSessionId()] is failed][%d]", result);
      break;
    }

    PrintDS(L"ProcessWrapper", L"ProcessWrapper::ExecuteProcessAsLogonAccount[active_user_session_id][%d]", active_user_session_id);

    /// 获取当前活动用户的token句柄
    if (FALSE == WTSQueryUserToken(active_user_session_id, &active_user_tocken_handle)) {
      result = GetLastError();
      PrintDS(L"ProcessWrapper", L"ProcessWrapper::ExecuteProcessAsLogonAccount[Fail][The function[WTSQueryUserToken()] is failed][%d]", result);
      break;
    }

    PrintDS(L"ProcessWrapper", L"ProcessWrapper::ExecuteProcessAsLogonAccount[Check Point][4]");

    /// 复制当前活动用户的token句柄
    if (FALSE == DuplicateTokenEx(active_user_tocken_handle, TOKEN_ALL_ACCESS, NULL, SecurityIdentification, TokenPrimary, &dummy_active_user_tocken_handle)) {
      result = GetLastError();
      PrintDS(L"ProcessWrapper", L"ProcessWrapper::ExecuteProcessAsLogonAccount[Fail][The function[DuplicateTokenEx()] is failed][%d]", result);
      break;
    }

    PrintDS(L"ProcessWrapper", L"ProcessWrapper::ExecuteProcessAsLogonAccount[Check Point][5]");

    /// 复制当前活动用户的环境信息
    if (FALSE == CreateEnvironmentBlock(&active_user_environment_block_ptr, dummy_active_user_tocken_handle, FALSE)) {
      result = GetLastError();
      PrintDS(L"ProcessWrapper", L"ProcessWrapper::ExecuteProcessAsLogonAccount[Fail][The function[CreateEnvironmentBlock()] is failed][%d]", result);
      break;
    }

    PrintDS(L"ProcessWrapper", L"ProcessWrapper::ExecuteProcessAsLogonAccount[Check Point][6]");

    /// 扩展当前活动用户的环境信息
    std::wstring new_user_environment_strings;
#ifndef _YSOS_UNICODE
    if (true != expand_environment_string.empty()) {
      new_user_environment_strings = CustomizeUserEnvironmentStringW(active_user_environment_block_ptr, const_cast<LPWSTR>(expand_environment_string.c_str()));
#else
    if (NULL != expand_environment_string_ptr &&
        0 != _tcslen(expand_environment_string_ptr)) {
      new_user_environment_strings = CustomizeUserEnvironmentStringW(active_user_environment_block_ptr, expand_environment_string_ptr);
#endif
      PrintDS(L"ProcessWrapper", L"new_user_environment_strings[%s]", new_user_environment_strings.c_str());
    }

    LPVOID new_active_user_environment_block_ptr = reinterpret_cast<LPVOID>(const_cast<LPWSTR>(new_user_environment_strings.c_str()));

    /// 打印活动用户的环境信息(for Debug dump input)
    LPWSTR ptr = static_cast<LPWSTR>(new_active_user_environment_block_ptr);
    while (*ptr) {
      PrintDS(L"ProcessWrapper", L"ProcessWrapper::ExecuteProcessAsLogonAccount[new_active_user_environment_block_ptr][%s]", ptr);
      ptr += wcslen(ptr) + 1;
    }

    /// 设置进程启动参数信息
    STARTUPINFOW startup_info;
    ZeroMemory(&startup_info, sizeof(STARTUPINFO));
    startup_info.cb = sizeof(STARTUPINFO);
    startup_info.lpDesktop = L"winsta0\\default";
    startup_info.dwFlags = STARTF_USESHOWWINDOW;

    if (hide_window) {
      PrintDS(L"ProcessWrapper", L"hide window");
      startup_info.wShowWindow = SW_HIDE;
    } else {
      PrintDS(L"ProcessWrapper", L"show window");
      startup_info.wShowWindow = SW_SHOW;
    }

    DWORD creation_falgs = NORMAL_PRIORITY_CLASS|CREATE_UNICODE_ENVIRONMENT|CREATE_NEW_CONSOLE;

#ifndef _YSOS_UNICODE
    WCHAR command_line[MAX_PATH*2] = {0,};
    if (true != command_line_string.empty()) {
      StringCchPrintfW(command_line, MAX_PATH*2, L"\"%s\" %s", process_file_path.c_str(), command_line_string.c_str());
    } else {
      StringCchCopyW(command_line, MAX_PATH*2, process_file_path.c_str());
    }
#else
    TCHAR command_line[MAX_PATH*2] = {0,};
    if (NULL != command_line_ptr &&
        0 != _tcslen(command_line_ptr)) {
      StringCchPrintf(command_line, MAX_PATH*2, _T("\"%s\" %s"), process_file_path_ptr, command_line_ptr);
    } else {
      StringCchCopy(command_line, MAX_PATH*2, process_file_path_ptr);
    }
#endif
    PrintDS(L"ProcessWrapper", L"command_line[%s]", command_line);

    PROCESS_INFORMATION process_information;
    ZeroMemory(&process_information, sizeof(PROCESS_INFORMATION));

    /// 以当前活动用户启动进程
#ifndef _YSOS_UNICODE
    if (FALSE == CreateProcessAsUserW(
          dummy_active_user_tocken_handle,
          process_file_path.c_str(),
          command_line,
          NULL,
          NULL,
          FALSE,
          creation_falgs,
          new_active_user_environment_block_ptr,
          NULL,
          &startup_info,
          &process_information)) {
      result = GetLastError();
      PrintDS(L"ProcessWrapper", L"ProcessWrapper::ExecuteProcessAsLogonAccount[Fail][The function[CreateProcessAsUser()] is failed][%d]", result);
      break;
    }
#else
    if (FALSE == CreateProcessAsUser(
          dummy_active_user_tocken_handle,
          process_file_path_ptr,
          command_line,
          NULL,
          NULL,
          FALSE,
          creation_falgs,
          new_active_user_environment_block_ptr,
          NULL,
          &startup_info,
          &process_information)) {
      result = GetLastError();
      PrintDS(_T("ProcessWrapper"), _T("ProcessWrapper::ExecuteProcessAsLogonAccount[Fail][The function[CreateProcessAsUser()] is failed][%d]"), result);
      break;
    }
#endif
    PrintDS(L"ProcessWrapper", L"ProcessWrapper::ExecuteProcessAsLogonAccount[Check Point][7]");

    if (INVALID_HANDLE_VALUE != process_information.hProcess) {
      PrintDS(L"ProcessWrapper", L"CloseHandle[process_information.hProcess]");
      if (NULL != process_handle_ptr) {
        *process_handle_ptr = process_information.hProcess;
      }
      CloseHandle(process_information.hProcess);
    }

    PrintDS(L"ProcessWrapper", L"ProcessWrapper::ExecuteProcessAsLogonAccount[Check Point][8]");

    if (INVALID_HANDLE_VALUE != process_information.hThread) {
      PrintDS(L"ProcessWrapper", L"CloseHandle[process_information.hThread]");
      CloseHandle(process_information.hThread);
    }

    PrintDS(L"ProcessWrapper", L"ProcessWrapper::ExecuteProcessAsLogonAccount[Check Point][End]");
    result = 0;
  }
  while (0);

  if (NULL != active_user_environment_block_ptr) {
    PrintDS(L"ProcessWrapper", L"DestroyEnvironmentBlock[active_user_environment_block_ptr]");
    DestroyEnvironmentBlock(active_user_environment_block_ptr);
  }

  if (NULL != active_user_environment_block_ptr) {
    PrintDS(L"ProcessWrapper", L"CloseHandle[active_user_tocken_handle]");
    CloseHandle(active_user_tocken_handle);
  }

  if (NULL != dummy_active_user_tocken_handle) {
    PrintDS(L"ProcessWrapper", L"CloseHandle[dummy_active_user_tocken_handle]");
    CloseHandle(dummy_active_user_tocken_handle);
  }

  PrintDS(L"ProcessWrapper", L"ProcessWrapper::ExecuteProcessAsLogonAccount[Exit]");
  return result;
}

std::wstring ProcessWrapper::CustomizeUserEnvironmentStringW(LPVOID user_environment_block, LPVOID expand_environment_strings) {
  PrintDS(L"ProcessWrapper", L"ProcessWrapper::CustomizeUserEnvironmentStringW[Enter]");
  std::wstring result;

  do {
    if (NULL == user_environment_block ||
        NULL == expand_environment_strings) {
      PrintDS(L"ProcessWrapper", L"ProcessWrapper::CustomizeUserEnvironmentStringW[Fail][user_environment_block or expand_environment_strings is NULL]");
      break;
    }

    PrintDS(L"ProcessWrapper", L"ProcessWrapper::CustomizeUserEnvironmentStringW[Check Point][0]");

    /// 打印活动用户的环境信息(for Debug dump input)，注意活动用户环境变量本身是unicode
    LPWSTR ptr = static_cast<LPWSTR>(user_environment_block);
    while (*ptr) {
      PrintDS(L"ProcessWrapper", L"ProcessWrapper::CustomizeUserEnvironmentStringW[user_environment_block][%s]", ptr);
      ptr += wcslen(ptr) + 1;
    }

    /// 打印要扩展用户的环境信息字符串(for Debug dump input)
    LPWSTR expand_environment_ptr = static_cast<LPWSTR>(expand_environment_strings);
    PrintDS(L"ProcessWrapper", L"ProcessWrapper::CustomizeUserEnvironmentStringW[expand_environment_strings][%s]", expand_environment_ptr);

    /// 解析活动用户的环境信息字符串
    typedef std::pair<std::wstring, std::wstring> EnvironmentStringPair;
    std::map<std::wstring, std::wstring> user_environment_block_map;
    typedef std::map<std::wstring, std::wstring>::iterator UserEnvironmentBlockMapItr;
    std::basic_string<wchar_t>::size_type index_ch;

    EnvironmentStringPair environment_string_pair;  ///< 解析的环境变量字符串内容，以key和value分开

    ptr = static_cast<LPWSTR>(user_environment_block);
    std::wstring temp_string;
    while (*ptr) {
      /// 每回都重置字符串
      temp_string.clear();
      temp_string = ptr;

      PrintDS(L"ProcessWrapper", L"ProcessWrapper::CustomizeUserEnvironmentStringW[temp_string][%s]", temp_string.c_str());

      /// 以=号为分割符，解析后放入map
      index_ch = temp_string.find(L"=");
      if (std::wstring::npos == index_ch) {
        PrintDS(L"ProcessWrapper", L"ProcessWrapper::CustomizeUserEnvironmentStringW[Fail][It can't find =][%s]", ptr);
        /// 解析下一行
        ptr += wcslen(ptr) + 1;
        continue;
      }

      environment_string_pair.first = temp_string.substr(0, index_ch+1);
      environment_string_pair.second = temp_string.substr(index_ch+1, wcslen(ptr));

      PrintDS(L"ProcessWrapper", L"ProcessWrapper::CustomizeUserEnvironmentStringW[environment_string_pair.first][%s]", environment_string_pair.first.c_str());
      PrintDS(L"ProcessWrapper", L"ProcessWrapper::CustomizeUserEnvironmentStringW[environment_string_pair.second][%s]", environment_string_pair.second.c_str());

      user_environment_block_map.insert(environment_string_pair);

      /// 解析下一行
      ptr += wcslen(ptr) + 1;
    }

    if (true == user_environment_block_map.empty()) {
      PrintDS(L"ProcessWrapper", L"ProcessWrapper::CustomizeUserEnvironmentStringW[Fail][user_environment_block_map is empty]");
      break;
    }

    /// 打印解析的内容(for Debug dump input)
    for (UserEnvironmentBlockMapItr itr = user_environment_block_map.begin();
         itr != user_environment_block_map.end();
         ++itr) {
      PrintDS(L"ProcessWrapper", L"ProcessWrapper::CustomizeUserEnvironmentStringW[itr][%s][%s]", itr->first.c_str(), itr->second.c_str());
    }

    /// 查找系统path
    UserEnvironmentBlockMapItr itr = user_environment_block_map.find(L"Path=");
    if (user_environment_block_map.end() == itr) {
      PrintDS(L"ProcessWrapper", L"ProcessWrapper::CustomizeUserEnvironmentStringW[Fail][It can't find [Path=]]");
      break;
    }

    PrintDS(L"ProcessWrapper", L"ProcessWrapper::CustomizeUserEnvironmentStringW[It find Path=][%s]", itr->second.c_str());

    /// 扩展系统path
    (itr->second).append(L";");
    (itr->second).append(static_cast<LPWSTR>(expand_environment_strings));
    PrintDS(L"ProcessWrapper", L"ProcessWrapper::CustomizeUserEnvironmentStringW[Expand Path=][%s]", itr->second.c_str());

    /// 重新组合用户系统变量信息字符串
    temp_string.clear();
    for (UserEnvironmentBlockMapItr itr = user_environment_block_map.begin();
         itr != user_environment_block_map.end();
         ++itr) {
      PrintDS(L"ProcessWrapper", L"ProcessWrapper::CustomizeUserEnvironmentStringW[itr->first][%s]", itr->first.c_str());
      PrintDS(L"ProcessWrapper", L"ProcessWrapper::CustomizeUserEnvironmentStringW[itr->second][%s]", itr->second.c_str());

      temp_string += itr->first;
      temp_string += itr->second;
      temp_string += L'\0';

      PrintDS(L"ProcessWrapper", L"ProcessWrapper::CustomizeUserEnvironmentStringW[temp_string][%s]", temp_string.c_str());
    }

    temp_string += L'\0';

    PrintDS(L"ProcessWrapper", L"ProcessWrapper::CustomizeUserEnvironmentStringW[temp_string][%s]", temp_string.c_str());

    /// 打印重新组合的用户的环境信息(for Debug dump input)
    ptr = const_cast<LPWSTR>(temp_string.c_str());
    while (*ptr) {
      PrintDS(L"ProcessWrapper", L"ProcessWrapper::CustomizeUserEnvironmentStringW[new user_environment_block][%s]", ptr);
      ptr += wcslen(ptr) + 1;
    }

    PrintDS(L"ProcessWrapper", L"ProcessWrapper::CustomizeUserEnvironmentStringW[Check Point][End]");
    result = temp_string;
  } while (0);

  PrintDS(L"ProcessWrapper", L"ProcessWrapper::CustomizeUserEnvironmentStringW[Exit]");
  return std::wstring(result);
}

int ProcessWrapper::GetProcessID(LPCTSTR process_name_ptr, std::vector<DWORD> &process_ids) {
  int result = -1;

  HANDLE snapshot_handle = INVALID_HANDLE_VALUE;
  do {

    if (NULL == process_name_ptr) {
      break;
    }

    process_ids.clear();

    snapshot_handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE == snapshot_handle) {
      result = GetLastError();
      break;
    }

    PROCESSENTRY32 process_entry;
    process_entry.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(snapshot_handle, &process_entry)) {
      result = GetLastError();
      break;
    }

    do {

      if (0 == lstrcmpi(process_entry.szExeFile, process_name_ptr)) {
        process_ids.push_back(process_entry.th32ProcessID);
      }

    } while (Process32Next(snapshot_handle, &process_entry));

    result = 0;
  } while (0);

  if (INVALID_HANDLE_VALUE != snapshot_handle) {
    CloseHandle(snapshot_handle);
  }

  return result;
}

ULONG ProcessWrapper::GetProcessIDWithHWnd(HWND wnd_handle) {
  ULONG process_id = 0;
  GetWindowThreadProcessId(wnd_handle, &process_id);
  return process_id;
}

HWND ProcessWrapper::GetProcessHWndWithID(const DWORD process_id) {
  HWND result = NULL;

  do {
    HWND wnd_handle = FindWindow(NULL, NULL);
    while (NULL != wnd_handle) {
      if (NULL == GetParent(wnd_handle)) {
        if (process_id == GetProcessIDWithHWnd(wnd_handle)) {
          result = wnd_handle;
          break;
        }
      }

      wnd_handle = GetWindow(wnd_handle, GW_HWNDNEXT);
    }

  } while (0);

  return result;
}

HANDLE ProcessWrapper::GetProcessHandleWithID(const DWORD process_id) {
  HANDLE result = NULL;

  do {
    if (0 == process_id) {
      break;
    }

    HWND hwnd = ::GetTopWindow(NULL);

    DWORD received_process_id = 0, retrieved_process_id =0;
    while (hwnd) {
      received_process_id = 0;
      retrieved_process_id = GetWindowThreadProcessId(hwnd, &received_process_id);
      if (retrieved_process_id != 0) {
        if (received_process_id == process_id) {
          result = hwnd;
          break;
        }
      }
      hwnd=::GetNextWindow(hwnd, GW_HWNDNEXT);
    }
  } while (0);

  return result;
}

int ProcessWrapper::GetProcessHandle(LPCTSTR process_name_ptr, std::vector<HANDLE> &process_handles) {
  int result = -1;

  do {
    if (NULL == process_name_ptr) {
      break;
    }

    std::vector<DWORD> process_ids;
    if (0 != GetProcessID(process_name_ptr, process_ids)) {
      break;
    }

    process_handles.clear();
    for (unsigned ii = 0; ii < process_ids.size(); ++ii) {
      //HANDLE process_handle = GetProcessHandleWithID(process_ids[ii]);
      HANDLE process_handle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, process_ids[ii]);
      if (NULL != process_handle) {
        process_handles.push_back(process_handle);
      }

      if (NULL != process_handle) {
        if (TRUE != CloseHandle(process_handle)) {
          result = GetLastError();
        }
        process_handle = NULL;
      }
    }

    result = 0;
  } while (0);

  return result;
}

int ProcessWrapper::GetProcessPathWithID(const DWORD process_id, LPTSTR process_path, DWORD process_path_size) {
  int result = -1;

  HANDLE process_handle = NULL;
  do {
    if (0 == process_id || NULL == process_path) {
      break;
    }

    process_handle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, process_id);
    if (NULL == process_handle) {
      result = GetLastError();
      break;
    }

    HMODULE module_handle = NULL;
    DWORD cbneeded;
    if (TRUE != EnumProcessModules(process_handle, &module_handle, sizeof(HMODULE), &cbneeded)) {
      result = GetLastError();
      if (TRUE != QueryFullProcessImageName(process_handle, 0, process_path, &process_path_size)) {
        result = GetLastError();
        break;
      }
    } else {
      if (0 == GetModuleFileNameEx(process_handle, module_handle, process_path, process_path_size)) {
        result = GetLastError();
        break;
      }
    }

    result = 0;
  } while (0);

  if (NULL != process_handle) {
    if (TRUE !=CloseHandle(process_handle)) {
    }
    process_handle = NULL;
  }

  return result;
}

bool ProcessWrapper::EnablePrivilege(PCTSTR privilege_ptr, bool enable) {
  /// Enabling the debug privilege allows the application to see
  /// information about service applications
  bool result = FALSE;    ///< Assume function fails
  HANDLE token_handle;

  /// Try to open this process's access token
  if (OpenProcessToken(
        GetCurrentProcess(),
        TOKEN_ADJUST_PRIVILEGES,
        &token_handle)) {
    /// Attempt to modify the given privilege
    TOKEN_PRIVILEGES token_privileges;
    token_privileges.PrivilegeCount = 1;
    LookupPrivilegeValue(NULL, privilege_ptr, &token_privileges.Privileges[0].Luid);
    token_privileges.Privileges[0].Attributes = enable ? SE_PRIVILEGE_ENABLED : 0;
    AdjustTokenPrivileges(token_handle, FALSE, &token_privileges, sizeof(token_privileges), NULL, NULL);
    result = (GetLastError() == ERROR_SUCCESS);

    /// Don't forget to close the token handle
    CloseHandle(token_handle);
  }
  return (result);
}

bool ProcessWrapper::RefreshTaskbarIcon()
{
#ifdef _YSOS_UNICODE
  //任务栏窗口   
  HWND hShellTrayWnd = ::FindWindow(L"Shell_TrayWnd",NULL);

  //任务栏右边托盘图标+时间区   
  HWND hTrayNotifyWnd = ::FindWindowEx(hShellTrayWnd,0,L"TrayNotifyWnd",NULL);  

  //不同系统可能有可能没有这层   
  HWND hSysPager = ::FindWindowEx(hTrayNotifyWnd,0,L"SysPager",NULL);  

  //托盘图标窗口   
  HWND hToolbarWindow32; 

  if (hSysPager) {  
    hToolbarWindow32 = ::FindWindowEx(hSysPager,0,L"ToolbarWindow32",NULL);  
  }  else  {  
    hToolbarWindow32 = ::FindWindowEx(hTrayNotifyWnd,0,L"ToolbarWindow32",NULL);  
  }
#else
  //任务栏窗口   
  HWND hShellTrayWnd = ::FindWindow("Shell_TrayWnd",NULL);

  //任务栏右边托盘图标+时间区   
  HWND hTrayNotifyWnd = ::FindWindowEx(hShellTrayWnd,0,"TrayNotifyWnd",NULL);  
  
  //不同系统可能有可能没有这层   
  HWND hSysPager = ::FindWindowEx(hTrayNotifyWnd,0,"SysPager",NULL);  
  
  //托盘图标窗口   
  HWND hToolbarWindow32; 

  if (hSysPager) {  
    hToolbarWindow32 = ::FindWindowEx(hSysPager,0,"ToolbarWindow32",NULL);  
  }  else  {  
    hToolbarWindow32 = ::FindWindowEx(hTrayNotifyWnd,0,"ToolbarWindow32",NULL);  
  } 
#endif

  if (hToolbarWindow32)  {  
    RECT r;  
    ::GetWindowRect(hToolbarWindow32,&r);  
    int width = r.right - r.left;  
    int height = r.bottom - r.top;  

    //从任务栏中间从左到右 MOUSEMOVE一遍，所有图标状态会被更新   
    for (int x = 1; x<width; x++) {  
	  ::SendMessage(hToolbarWindow32,WM_MOUSEMOVE,0,MAKELPARAM(x,height/2));  
    }  
  } 

  return TRUE;
}

#else

bool ProcessWrapper::GetProcssInfos(std::vector<long> &process_info) {
  process_info.clear();

  bool result = false;

  //TODO: add for linux

  return result;
}

int ProcessWrapper::ExecuteProcess(
  LPCTSTR process_file_path_ptr,
  LPCTSTR command_line_ptr,
  LPTSTR expand_environment_string_ptr,
  bool hide_window,
  LONG delay_time,
  bool use_shell,
  HANDLE* process_handle_ptr) {
  int result = -1;

  //TODO: add for linux
  do {
    if (NULL == process_file_path_ptr ||0 == strlen(process_file_path_ptr)) {
      printf("ProcessWrapper::process_file_path_ptr is NULL\n");
      break;
    }

    if (!boost::filesystem::exists(const_cast<char*>(process_file_path_ptr))) {
      printf("ProcessWrapper::There is not the file.[%s]\n", process_file_path_ptr);
      break;
    }

    char *process_name = basename(const_cast<char*>(process_file_path_ptr));
    if (NULL == process_name ||0 == strlen(process_name)) {
      printf("ProcessWrapper::process_name is empty.\n");
      break;
    }

    if (true == IsRunningProcess(process_name)) {
      printf("ProcessWrapper::The process is already running.[%s]\n", process_name);
  
      result = 0;
      break;
    }

    if (0 != delay_time) {
      printf("ProcessWrapper::delay_time[%ld]\n", delay_time);
#ifdef _WIN32
      Sleep(delay_time*1000);
#else
      sleep(delay_time);
      //boost::this_thread::sleep(boost::posix_time::seconds(2));
#endif
    }


    if (0 == ExecuteProcessAsLogonAccountW(
          process_file_path_ptr,
          command_line_ptr,
          expand_environment_string_ptr,
          hide_window,
          process_handle_ptr)) {
      printf("ProcessWrapper::The function[ExecuteProcessAsLogonAccount()] succeed.\n");
      result = 0;
      break;
    }
    
    if (true == use_shell) {
      if (0 != ExecuteProcessWithShell(
            process_file_path_ptr,
            command_line_ptr,
            expand_environment_string_ptr,
            hide_window,
            false,
            process_handle_ptr)) {
        printf("ProcessWrapper::The function[ExecuteProcessWithShell()] fails.\n");
        break;
      }
    }
    
    result = 0;
  } while (0);

  return result;
}

int ProcessWrapper::ExecuteProcessWithShell(
  LPCTSTR process_file_path_ptr,
  LPCTSTR command_line_ptr,
  LPTSTR expand_environment_string_ptr,
  bool hide_window,
  bool as_admin,
  HANDLE* process_handle_ptr) {
  int result = -1;
  
  //TODO: add for linux
  do
  {
    system(process_file_path_ptr);
    result = 0;
  } while (0);

  return result;
}

#ifdef UNICODE
std::string TcharToString(TCHAR *STR)
{
  int iLen = WideCharToMultiByte(CP_ACP, 0,STR, -1, NULL, 0, NULL, NULL);
  char* chRtn =new char[iLen*sizeof(char)];
  if( chRtn == NULL) return "";
  WideCharToMultiByte(CP_ACP, 0, STR, -1, chRtn, iLen, NULL, NULL);
  std::string str(chRtn);
  delete [] chRtn;
  chRtn = NULL;

  return str;
}
#endif

static std::string GetProcessUserAccount(unsigned long process_id) {
  //TODO: add for linux
  return "";
}

bool ProcessWrapper::IsRunningProcess(LPCTSTR process_name_ptr) {
  bool result = false;
  bool kill_process = false;
  //TODO: add for linux
  do {

    if (NULL == process_name_ptr) {
      break;
    }

    FILE* fp = NULL;
    int count = 0;
    int BUFSZ = 100;
    char buf[BUFSZ];
    char command[150];


    //system() 也可以检测进程是否存在，但非常耗费资源
    sprintf(command, "ps -C %s | wc -l", process_name_ptr );


    if((fp = popen(command,"r")) == NULL)
    {
        printf("(fp = popen(command,\"r\")) == NULL\n");
        return false;
    }

    if( (fgets(buf,BUFSZ,fp))!= NULL )
    {
        count = atoi(buf);
    }

    pclose(fp);
    fp = NULL;

    if((count - 1) == 0)
    {
        printf("%s not found\n", process_name_ptr);
        //可以用system()开启test
        result = false;
    }
    else
    {
        printf("process : %s total is %d\n", process_name_ptr, (count - 1));
        result = true;
    }
  } while (0); 
   
  /*
  if(true == kill_process) {
    KillProcess(process_name_ptr);
  }*/
  return result;
}

bool ProcessWrapper::IsRuningProcessEx(LPCTSTR process_file_path_ptr) {
  bool result = false;
  // TODO: add for linux
  do {
    if (!boost::filesystem::exists(const_cast<char*>(process_file_path_ptr))) {
      break;
    }
    // printf("IsRuningProcessEx::process_file_path_ptr --> %s\n", process_file_path_ptr);
    char *process_name = basename(const_cast<char*>(process_file_path_ptr));
    if (NULL == process_name ||0 == strlen(process_name)) {
      break;
    }
    // printf("IsRuningProcessEx::process_name --> %s\n", process_name);
    if (true != IsRunningProcess(process_name)) {
      break;
    }

    result = true;
  } while (0);
  return result;
}

int ProcessWrapper::KillProcess(LPCTSTR process_name_ptr) {
  int result = 0;
  //TODO: add for linux
  do {
    if (NULL == process_name_ptr) {
      result = -1;
      break;
    }


  } while (0);

  return result;
}

int ProcessWrapper::KillProcessEx(LPCTSTR process_file_path_ptr) {
  int result = -1;
  //TODO: add for linux
  do {
    if (!boost::filesystem::exists(const_cast<char*>(process_file_path_ptr))) {
      break;
    }

    char *process_name = basename(const_cast<char*>(process_file_path_ptr));
    if ( NULL == process_name || 0 == strlen(process_name))
    {
      break;
    }

    result = KillProcess(process_name);
    if (0 != result) {
      break;
    }

    result =0;
  } while (0);
  return result;
}

int ProcessWrapper::GetProcessPrivilege() {

  int result = -1;
  //TODO: add for linux
  return result;
}

int ProcessWrapper::ExecuteProcessAsLogonAccountW(
  LPCTSTR process_file_path_ptr,
  LPCTSTR command_line_ptr,
  LPTSTR expand_environment_string_ptr,
  bool hide_window,
  HANDLE* process_handle_ptr) {

  int result = -1;
  //TODO: add for linux
  do {

    if (!boost::filesystem::exists(const_cast<char*>(process_file_path_ptr))) {
      break;
    }

    char *process_name = basename(const_cast<char*>(process_file_path_ptr));
    if ( NULL == process_name || 0 == strlen(process_name))
    {
      break;
    }
   
    char* const argv_execv[] = { process_name, (char*)"", NULL };
    if(fork() == 0)
    {
      if(execv(process_file_path_ptr, argv_execv) < 0)
      {
        perror("Err on execv");
        break;
      }
      printf("execv process... success ... \n");
    }
    result = 0;
  } while (0);
  return result;
}


int ProcessWrapper::GetProcessID(LPCTSTR process_name_ptr, std::vector<unsigned long> &process_ids) {
  int result = -1;
  //TODO: add for linux
  return result;
}


HANDLE ProcessWrapper::GetProcessHandleWithID(const unsigned long process_id) {
  HANDLE result = NULL;

  do {
    if (0 == process_id) {
      break;
    }
    //TODO: add for linux
        
  } while (0);

  return result;
}

int ProcessWrapper::GetProcessHandle(LPCTSTR process_name_ptr, std::vector<HANDLE> &process_handles) {
  int result = -1;

  do {
    if (NULL == process_name_ptr) {
      break;
    }
    //TODO: add for linux

    result = 0;
  } while (0);

  return result;
}

int ProcessWrapper::GetProcessPathWithID(const unsigned long process_id, LPTSTR process_path, unsigned long process_path_size) {
  int result = -1;
  //TODO: add for linux
  return result;
}

bool ProcessWrapper::EnablePrivilege(PCTSTR privilege_ptr, bool enable) {
  /// Enabling the debug privilege allows the application to see
  /// information about service applications
  bool result = false;    ///< Assume function fails
  //TODO: add for linux
  return (true);
}

bool ProcessWrapper::RefreshTaskbarIcon()
{
  //TODO: add for linux
  return true;
}
#endif

}

}
