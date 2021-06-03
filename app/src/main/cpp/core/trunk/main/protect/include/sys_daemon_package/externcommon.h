/**
*# externcommon.h
*# Definition of externcommon
*# Created on: 2017-03-15 09:47:38
*# Original author: jinchengzhe
*# Copyright (c) 2016 YunShen Technology. All rights reserved.
*# 
*# version 0.1, 20170315, created by JinChengZhe
*/

#ifndef SPS_EXTERN_COMMON_H_
#define SPS_EXTERN_COMMON_H_


/**
 *@brief Windows平台常用头文件及预定义
 */
#ifdef _WIN32
#pragma warning(disable: 4995)
#include <Windows.h>
#include <process.h>
#include <tchar.h>
#include <strsafe.h>
#pragma comment(lib, "advapi32.lib")
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include <memory>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#else
#pragma warning(disable: 4995)

/**--need add for linux--**/
#if !defined(_MSC_VER)
#include <tr1/memory>
#include <tr1/functional>
#endif

#include <string.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include <boost/filesystem.hpp>
#include <sys/time.h>
#include <codecvt>
#include <pthread.h>
#endif
/**
 *@brief 重定义线程创建API
 */
namespace ysos {

namespace extern_common {


#ifdef _WIN32
#pragma region 
/**
 *@brief 服务进程pipe消息命令
 */
enum {
  WIN_SVC_PIPE_CMD_KILL_PROCESSES = 1000,
  WIN_SVC_PIPE_CMD_EXECUTE_PROCESSES,
};

/**
 *@brief 通过pipe委托服务进程，杀死或者运行指定进程json报文格式
 */
/*
{
"processes": [
"ysos.exe",
"ysos_update.exe"
]
}
*/

/**
 *@brief 通过pipe委托服务进程，杀死指定进程或者运指定进程，处理返回json报文格式
 */
/*
{
"status_code": "0",
"result": "Success",
"detail": [
"ysos.exe",
"ysos_update.exe"
]
}
*/
//////////////////////////////////////////////////////////////////////////

/**
 *@brief 管道服务器名称
 */
#ifdef _YSOS_UNICODE
static const TCHAR* g_ysos_service_pipe_server_name = _T("\\\\.\\pipe\\{1F68526E-0311-49E3-ABAD-01C3DFC63A32}");
#else
static const char* g_ysos_service_pipe_server_name = "\\\\.\\pipe\\{1F68526E-0311-49E3-ABAD-01C3DFC63A32}";
#endif
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
typedef unsigned(__stdcall *PTHREAD_START)(void *);
#define YSOSBEGINTHREADEX(psa, cbStackSize, pfnStartAddr, pvParam, dwCreateFlags, pdwThreadId) \
    ((HANDLE)_beginthreadex((void*)(psa), \
    (unsigned)(cbStackSize), \
    (PTHREAD_START) (pfnStartAddr), \
    (void*)(pvParam), \
    (unsigned)(dwCreateFlags), \
    (unsigned*)(pdwThreadId)))
//////////////////////////////////////////////////////////////////////////

/**
 *@brief 用于线程间加锁
 */
//////////////////////////////////////////////////////////////////////////
class CYSOSLock {
 public:
  CYSOSLock() {
    InitializeCriticalSection(&critical_section_);
  }

  ~CYSOSLock() {
    DeleteCriticalSection(&critical_section_);
  }

  void Lock() {
    EnterCriticalSection(&critical_section_);
  }
  void Unlock() {
    LeaveCriticalSection(&critical_section_);
  }
 private:
  CYSOSLock& operator= (const CYSOSLock&);
  CYSOSLock(const CYSOSLock&);

  CRITICAL_SECTION critical_section_;
};
//////////////////////////////////////////////////////////////////////////

/**
 *@brief 用于DebugView中打印Log
 */
//////////////////////////////////////////////////////////////////////////
inline void PrintDS(LPWSTR pszTag, LPWSTR pszFormat, ...) {
  va_list argList;
  va_start(argList, pszFormat);

  WCHAR szBuffer[1024*20] = {0,};
  int nResult = -1;

  if (
    pszTag != NULL &&
    wcslen(pszTag) != 0
  ) {
    nResult = swprintf_s(szBuffer, _countof(szBuffer), L"###[%s]###", pszTag);
    if (nResult == -1) {
      nResult = 0;
    }
  } else {
    nResult = swprintf_s(szBuffer, _countof(szBuffer), L"****");
    if (nResult == -1) {
      nResult = 0;
    }
  }

  vswprintf_s(szBuffer + nResult, _countof(szBuffer) - nResult, pszFormat, argList);

  OutputDebugStringW(szBuffer);

  va_end(argList);

  return;
}

inline void PrintDSA(LPCSTR pszTag, LPCSTR pszFormat, ...) {
  va_list argList;
  va_start(argList, pszFormat);

  char szBuffer[1024*20] = {0,};
  int nResult = -1;

  if (
    pszTag != NULL &&
    strlen(pszTag) != 0
  ) {
    nResult = sprintf_s(szBuffer, _countof(szBuffer), "###[%s]###", pszTag);
    if (nResult == -1) {
      nResult = 0;
    }
  } else {
    nResult = sprintf_s(szBuffer, _countof(szBuffer), "****");
    if (nResult == -1) {
      nResult = 0;
    }
  }

  vsprintf_s(szBuffer + nResult, _countof(szBuffer) - nResult, pszFormat, argList);

  OutputDebugStringA(szBuffer);

  va_end(argList);

  return;
}
//////////////////////////////////////////////////////////////////////////

/**
 *@brief 用于实现单例
 */
//////////////////////////////////////////////////////////////////////////
#define YSOS_DECLARE_SINGLETON(CLASS_NAME) \
    public: \
      static const std::tr1::shared_ptr<CLASS_NAME> GetInstancePtr(void); \
      class Delete##CLASS_NAME##SingletonFunction { \
      public: \
        void operator() (CLASS_NAME* object_ptr) { \
          if (NULL != object_ptr) { \
            delete object_ptr; \
            object_ptr = NULL; \
          } \
        } \
      }; \
      friend class Delete##CLASS_NAME##SingletonFunction; \
    private: \
      static std::tr1::shared_ptr<CLASS_NAME> the_instance_ptr_;

#define YSOS_IMPLIMENT_SINGLETON(CLASS_NAME) \
    std::tr1::shared_ptr<CLASS_NAME> CLASS_NAME::the_instance_ptr_ = NULL; \
    const std::tr1::shared_ptr<CLASS_NAME> CLASS_NAME::GetInstancePtr(void) { \
        if (NULL == the_instance_ptr_) { \
            static CYSOSLock lock; \
            lock.Lock(); \
            if (NULL == the_instance_ptr_) { \
                Delete##CLASS_NAME##SingletonFunction delete_function; \
                the_instance_ptr_ = std::tr1::shared_ptr<CLASS_NAME>(new CLASS_NAME(), delete_function); \
            } \
            lock.Unlock(); \
        } \
        return the_instance_ptr_; \
    }
//////////////////////////////////////////////////////////////////////////

/**
 *@brief 字符集转换
 */
//////////////////////////////////////////////////////////////////////////
enum EncodingType {
  ASCII,   ///<  ascii, gb2312. etc  //  NOLINT
  UTF8,    ///<  utf-8 //  NOLINT
  UTF16,   ///<  unicode //  NOLINT
  EncodingTypeEnd
};

inline EncodingType GetStringEncoding(const unsigned char *character_ptr) {
  if (NULL == character_ptr) {
    return EncodingTypeEnd;
  }

  if (*character_ptr == 0xff) {
    if (*(++character_ptr) == 0xfe) {
      return UTF16;
    }
  } else if (*character_ptr == 0xef) {
    if (*(++character_ptr) == 0xbb) {
      if (*(++character_ptr) == 0xbf) {
        return UTF8;
      }
    }
  }

  return ASCII;
}

inline EncodingType GetStringEncoding(std::ifstream &file_stream) {
  const int encode_tpe_len = 3;
  unsigned char encode_type[encode_tpe_len] = {0};

  file_stream.seekg(0, file_stream.beg);
  file_stream.read((char*)encode_type, encode_tpe_len);

  return GetStringEncoding(encode_type);
}

inline std::wstring AcsiiToUnicode(const std::string& data) {
  int widesize = MultiByteToWideChar(CP_ACP,0,data.c_str(),data.size(), NULL, 0);
  if (widesize == ERROR_NO_UNICODE_TRANSLATION)  {
    return std::wstring();
  }
  if (widesize == 0) {
    return std::wstring();
  }
  std::vector<wchar_t> resultstring(widesize+1);
  int convresult = MultiByteToWideChar(CP_ACP, 0,data.c_str(),data.size(), &resultstring[0], widesize);
  if (convresult != widesize) {
    return std::wstring();
  }
  return std::wstring(&resultstring[0]);
}

inline std::string UnicodeToAcsii(const std::wstring& data) {
  int asciisize = ::WideCharToMultiByte(CP_OEMCP,0,data.c_str(),data.size(), NULL, 0, NULL, NULL);
  if (asciisize == ERROR_NO_UNICODE_TRANSLATION) {
    return std::string("");
  }
  if (asciisize == 0) {
    return std::string("");
  }
  std::vector<char> resultstring(asciisize+1);
  int convresult =::WideCharToMultiByte(CP_OEMCP,0,data.c_str(),data.size(),&resultstring[0], asciisize, NULL, NULL);
  if (convresult != asciisize) {
    return std::string("");
  }
  return std::string(&resultstring[0]);
}

inline std::wstring Utf8ToUnicode(const std::string& data) {
  int widesize = ::MultiByteToWideChar(CP_UTF8, 0, data.c_str(),data.size(), NULL, 0);
  if (widesize == ERROR_NO_UNICODE_TRANSLATION) {
    return std::wstring();
  }
  if (widesize == 0) {
    return std::wstring();
  }
  std::vector<wchar_t> resultstring(widesize+1);
  int convresult = ::MultiByteToWideChar(CP_UTF8, 0, data.c_str(),data.size(), &resultstring[0], widesize);
  if (convresult != widesize) {
    return std::wstring();
  }
  return std::wstring(&resultstring[0]);
}

inline std::string Utf8ToAscii(const std::string &data) {
  //先把 utf8 转为 unicode
  std::wstring wstr = Utf8ToUnicode(data);
  //最后把 unicode 转为 ascii
  return UnicodeToAcsii(wstr);
}

inline std::string ReadAllDataFromFile(const std::string &file_name) {
  std::ifstream in_ifile;
  in_ifile.open(file_name.c_str(), std::ios_base::in|std::ios_base::binary);
  if (!in_ifile.good()) {
    return "";
  }

  EncodingType type = GetStringEncoding(in_ifile);
  if (EncodingTypeEnd == type) {
    return "";
  }

  int start_pos = 0, end_len = 1;
  if (UTF16 == type) {
    start_pos = 2;
    end_len = 2;
  } else if (UTF8 == type) {
    start_pos = 3;
  }

  in_ifile.seekg(0, in_ifile.end);
  int length = static_cast<int>(in_ifile.tellg()) - start_pos;
  in_ifile.seekg(start_pos, in_ifile.beg);

  char * buffer = new char [length+1];
  std::memset(buffer, 0, length+1);
  in_ifile.read(buffer,length);
  if (in_ifile) {

  } else {

  }
  in_ifile.close();

  std::string result_str;

  if (UTF8 == type) { ///< utf-8 //  NOLINT
    result_str = Utf8ToAscii(buffer);
  } else if (UTF16 == type) { ///<  utf-16 //  NOLINT
    result_str = UnicodeToAcsii((wchar_t*)buffer);
  } else { ///<  gbk  //  NOLINT
    result_str = buffer;
  }
  delete []buffer;

  return result_str;
}

inline int MDDirectoryIsExists(LPCTSTR directory) {
  int result = 0;

  do {
    if (NULL == directory ||
        MAX_PATH < _tcslen(directory)) {
      result = -1;
      break;
    }

    DWORD file_attribute = GetFileAttributes(directory);
    if ((0xFFFFFFFF != file_attribute) &&
        (FILE_ATTRIBUTE_DIRECTORY & file_attribute)) {
      result = 1;
    }
  } while (0);

  return result;
}

inline DWORD MDCreateDirectory(LPCTSTR directory) {
  DWORD result = 0;

  do {
    if (NULL == directory ||
        MAX_PATH < _tcslen(directory)) {
      result = 1;
      printf("MiniDump::MDCreateDirectory[Fail][The parameter is invalid][%d]\n", result);
      break;
    }

    TCHAR buffer[MAX_PATH] = {0,};
    if (TRUE != SUCCEEDED(StringCchCopy(buffer, MAX_PATH, directory))) {
      result = GetLastError();
      printf("MiniDump::MDCreateDirectory[Fail][The function[StringCchCopy()] is failed][%d]\n", result);
      break;
    }

    LPTSTR pch = buffer;
    LPTSTR slash = NULL;

    do {
      /// 检测反斜杠，获取每个子路径文件夹
      slash = _tcschr(pch, '\\');
      if (NULL != slash) {
        *slash = '\0';
      }

      /// 不是root路径，检测子路径
      if (NULL == _tcschr(pch, ':')) {
        /// 检测文件夹是否存在
        result = MDDirectoryIsExists(buffer);
        if (0 == result) {
          /// 创建文件夹，函数执行失败跳出
          if (TRUE != CreateDirectory(buffer, NULL)) {
            result = GetLastError();
            printf("MiniDump::MDCreateDirectory[Fail][The function[CreateDirectory()] is failed][%d]", result);
            break;
          }
        } else {
          /// 检测文件夹函数执行失败，跳出
          if (-1 == result) {
            printf("MiniDump::MDCreateDirectory[Fail][The function[MDDirectoryIsExists()] is failed][%d]", result);
            result = 2;
            break;
          }
          /// 检测文件夹已存在，复位
          result = 0;
        }
      }

      if (NULL != slash) {
        *slash = '\\';
      }

      pch = slash + 1;
    } while (NULL != slash);
  } while (0);

  return result;
}

#pragma endregion

//////////////////////////////////////////////////////////////////////////

#else
/**
 *@brief 其他平台常用头文件及预定义，后期实现
 */

/**
 *@brief 重定义线程创建API
 */

/**
 *@brief 服务进程pipe消息命令
 */
enum {
  WIN_SVC_PIPE_CMD_KILL_PROCESSES = 1000,
  WIN_SVC_PIPE_CMD_EXECUTE_PROCESSES,
};

/**
 *@brief 通过pipe委托服务进程，杀死或者运行指定进程json报文格式
 */
/*
{
"processes": [
"ysos.exe",
"ysos_update.exe"
]
}
*/

/**
 *@brief 通过pipe委托服务进程，杀死指定进程或者运指定进程，处理返回json报文格式
 */
/*
{
"status_code": "0",
"result": "Success",
"detail": [
"ysos.exe",
"ysos_update.exe"
]
}
*/
//////////////////////////////////////////////////////////////////////////

/**
 *@brief 管道服务器名称
 */
#ifdef _YSOS_UNICODE
static const TCHAR* g_ysos_service_pipe_server_name = _T("\\\\.\\pipe\\{1F68526E-0311-49E3-ABAD-01C3DFC63A32}");
#else
static const char* g_ysos_service_pipe_server_name = "\\\\.\\pipe\\{1F68526E-0311-49E3-ABAD-01C3DFC63A32}";
#endif
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
typedef unsigned(/*__stdcall*/ *PTHREAD_START)(void *);
#define YSOSBEGINTHREADEX(psa, cbStackSize, pfnStartAddr, pvParam, dwCreateFlags, pdwThreadId) \
    ((void*)_beginthreadex((void*)(psa), \
    (unsigned)(cbStackSize), \
    (PTHREAD_START) (pfnStartAddr), \
    (void*)(pvParam), \
    (unsigned)(dwCreateFlags), \
    (unsigned*)(pdwThreadId)))
//////////////////////////////////////////////////////////////////////////

/**
 *@brief 用于线程间加锁
 */
//////////////////////////////////////////////////////////////////////////
class CYSOSLock {
 public:
  CYSOSLock() {
    // 
    #ifdef _WIN32
      InitializeCriticalSection(&critical_section_);
    #else
      pthread_mutex_init(&mutex, NULL);  //初始化锁 mutex==1
    #endif
  }

  ~CYSOSLock() {
    #ifdef _WIN32
      DeleteCriticalSection(&critical_section_);
    #else
      pthread_mutex_destroy(&mutex);  //销毁锁
    #endif
  }

  void Lock() {
    #ifdef _WIN32
      EnterCriticalSection(&critical_section_);
    #else
      pthread_mutex_lock(&mutex); //加锁
    #endif
  }
  void Unlock() {
    #ifdef _WIN32
      LeaveCriticalSection(&critical_section_);
    #else
      pthread_mutex_unlock(&mutex); //解锁
    #endif
  }
 private:
  CYSOSLock& operator= (const CYSOSLock&);
  CYSOSLock(const CYSOSLock&);

  #ifdef _WIN32
    CRITICAL_SECTION critical_section_;
  #else
    pthread_mutex_t mutex;      //定义锁
  #endif
};
//////////////////////////////////////////////////////////////////////////

/**
 *@brief 用于DebugView中打印Log
 */
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

/**
 *@brief 用于实现单例
 */
//////////////////////////////////////////////////////////////////////////
#define YSOS_DECLARE_SINGLETON(CLASS_NAME) \
    public: \
      static const std::tr1::shared_ptr<CLASS_NAME> GetInstancePtr(void); \
      class Delete##CLASS_NAME##SingletonFunction { \
      public: \
        void operator() (CLASS_NAME* object_ptr) { \
          if (NULL != object_ptr) { \
            delete object_ptr; \
            object_ptr = NULL; \
          } \
        } \
      }; \
      friend class Delete##CLASS_NAME##SingletonFunction; \
    private: \
      static std::tr1::shared_ptr<CLASS_NAME> the_instance_ptr_;

#define YSOS_IMPLIMENT_SINGLETON(CLASS_NAME) \
    std::tr1::shared_ptr<CLASS_NAME> CLASS_NAME::the_instance_ptr_; /*= NULL;**/ \
    const std::tr1::shared_ptr<CLASS_NAME> CLASS_NAME::GetInstancePtr(void) { \
        if (NULL == the_instance_ptr_) { \
            static CYSOSLock lock; \
            lock.Lock(); \
            if (NULL == the_instance_ptr_) { \
                Delete##CLASS_NAME##SingletonFunction delete_function; \
                the_instance_ptr_ = std::tr1::shared_ptr<CLASS_NAME>(new CLASS_NAME(), delete_function); \
            } \
            lock.Unlock(); \
        } \
        return the_instance_ptr_; \
    }
//////////////////////////////////////////////////////////////////////////

/**
 *@brief 字符集转换
 */
//////////////////////////////////////////////////////////////////////////
enum EncodingType {
  ASCII,   ///<  ascii, gb2312. etc  //  NOLINT
  UTF8,    ///<  utf-8 //  NOLINT
  UTF16,   ///<  unicode //  NOLINT
  EncodingTypeEnd
};

inline EncodingType GetStringEncoding(const unsigned char *character_ptr) {
  if (NULL == character_ptr) {
    return EncodingTypeEnd;
  }

  if (*character_ptr == 0xff) {
    if (*(++character_ptr) == 0xfe) {
      return UTF16;
    }
  } else if (*character_ptr == 0xef) {
    if (*(++character_ptr) == 0xbb) {
      if (*(++character_ptr) == 0xbf) {
        return UTF8;
      }
    }
  }

  return ASCII;
}

inline EncodingType GetStringEncoding(std::ifstream &file_stream) {
  const int encode_tpe_len = 3;
  unsigned char encode_type[encode_tpe_len] = {0};

  file_stream.seekg(0, file_stream.beg);
  file_stream.read((char*)encode_type, encode_tpe_len);

  return GetStringEncoding(encode_type);
}

inline std::wstring AcsiiToUnicode(const std::string& data) {
  #ifdef _WIN32
    int widesize = MultiByteToWideChar(CP_ACP,0,data.c_str(),data.size(), NULL, 0);
    if (widesize == ERROR_NO_UNICODE_TRANSLATION)  {
      return std::wstring();
    }
    if (widesize == 0) {
      return std::wstring();
    }
    std::vector<wchar_t> resultstring(widesize+1);
    int convresult = MultiByteToWideChar(CP_ACP, 0,data.c_str(),data.size(), &resultstring[0], widesize);
    if (convresult != widesize) {
      return std::wstring();
    }
    return std::wstring(&resultstring[0]);
  #else
    std::wstring ret;
    std::mbstate_t state = {};
    const char* src = data.data();
    size_t len = std::mbsrtowcs(nullptr, &src, 0, &state);
    if (static_cast<size_t>(-1) != len) {
        std::unique_ptr< wchar_t[] > buff(new wchar_t[len + 1]);
        len = std::mbsrtowcs(buff.get(), &src, len, &state);
        if (static_cast<size_t>(-1) != len) {
            ret.assign(buff.get(), len);
        }
    }
    return ret;
    //return "";
  #endif
}

inline std::string UnicodeToAcsii(const std::wstring& data) {
  #ifdef _WIN32
    int asciisize = ::WideCharToMultiByte(CP_OEMCP,0,data.c_str(),data.size(), NULL, 0, NULL, NULL);
    if (asciisize == ERROR_NO_UNICODE_TRANSLATION) {
      return std::string("");
    }
    if (asciisize == 0) {
      return std::string("");
    }
    std::vector<char> resultstring(asciisize+1);
    int convresult =::WideCharToMultiByte(CP_OEMCP,0,data.c_str(),data.size(),&resultstring[0], asciisize, NULL, NULL);
    if (convresult != asciisize) {
      return std::string("");
    }
    return std::string(&resultstring[0]);
  #else
    std::string ret;
    std::mbstate_t state = {};
    const wchar_t* src = data.data();
    size_t len = std::wcsrtombs(nullptr, &src, 0, &state);
    if (static_cast<size_t>(-1) != len) {
        std::unique_ptr< char[] > buff(new char[len + 1]);
        len = std::wcsrtombs(buff.get(), &src, len, &state);
        if (static_cast<size_t>(-1) != len) {
            ret.assign(buff.get(), len);
        }
    }
    return ret;
  #endif
}

inline std::wstring Utf8ToUnicode(const std::string& data) {
  #ifdef _WIN32
    int widesize = ::MultiByteToWideChar(CP_UTF8, 0, data.c_str(),data.size(), NULL, 0);
    if (widesize == ERROR_NO_UNICODE_TRANSLATION) {
      return std::wstring();
    }
    if (widesize == 0) {
      return std::wstring();
    }
    std::vector<wchar_t> resultstring(widesize+1);
    int convresult = ::MultiByteToWideChar(CP_UTF8, 0, data.c_str(),data.size(), &resultstring[0], widesize);
    if (convresult != widesize) {
      return std::wstring();
    }
    return std::wstring(&resultstring[0]);
  #else
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.from_bytes(data);
    //return "";
  #endif
}

inline std::string Utf8ToAscii(const std::string &data) {
  //先把 utf8 转为 unicode
  std::wstring wstr = Utf8ToUnicode(data);
  //最后把 unicode 转为 ascii
  return UnicodeToAcsii(wstr);
}

inline std::string ReadAllDataFromFile(const std::string &file_name) {
  std::ifstream in_ifile;
  in_ifile.open(file_name.c_str(), std::ios_base::in|std::ios_base::binary);
  if (!in_ifile.good()) {
    return "";
  }

  EncodingType type = GetStringEncoding(in_ifile);
  if (EncodingTypeEnd == type) {
    return "";
  }

  int start_pos = 0, end_len = 1;
  if (UTF16 == type) {
    start_pos = 2;
    end_len = 2;
  } else if (UTF8 == type) {
    start_pos = 3;
  }

  in_ifile.seekg(0, in_ifile.end);
  int length = static_cast<int>(in_ifile.tellg()) - start_pos;
  in_ifile.seekg(start_pos, in_ifile.beg);

  char * buffer = new char [length+1];
  std::memset(buffer, 0, length+1);
  in_ifile.read(buffer,length);
  if (in_ifile) {

  } else {

  }
  in_ifile.close();

  std::string result_str;

  if (UTF8 == type) { ///< utf-8 //  NOLINT
    result_str = Utf8ToAscii(buffer);
  } else if (UTF16 == type) { ///<  utf-16 //  NOLINT
    result_str = UnicodeToAcsii((wchar_t*)buffer);
  } else { ///<  gbk  //  NOLINT
    result_str = buffer;
  }
  delete []buffer;

  return result_str;
}

inline int MDDirectoryIsExists(char* directory) {
  int result = 0;
  int MAX_PATH = 260;
  do {
    if (NULL == directory ||
        MAX_PATH < strlen(directory)) {
      result = -1;
      break;
    }
    
    boost::filesystem::path path_file(directory);
    if ( boost::filesystem::exists(path_file) && boost::filesystem::is_directory(path_file) ){
         printf("boost: path exist.\n");
         result = 1;
    } else {
         printf("boost: path not exist.\n");
    }
    
  } while (0);

  return result;
}

inline unsigned long MDCreateDirectory(char* directory) {
  unsigned long result = 0;
  int MAX_PATH = 260;
  do {
    if (NULL == directory ||
        MAX_PATH < strlen(directory)) {
      result = 1;
      printf("MiniDump::MDCreateDirectory[Fail][The parameter is invalid][%ld]\n", result);
      break;
    }

    char buffer[MAX_PATH] = {0,};
    if (NULL == (strncpy(buffer, directory, MAX_PATH))) {
      //result = GetLastError();// add for linux
      printf("MiniDump::MDCreateDirectory[Fail][The function[StringCchCopy()] is failed][%ld]\n", result);
      break;
    }

    char* pch = buffer;
    char* slash = NULL;

    do {
      /// 检测反斜杠，获取每个子路径文件夹
      slash = strchr(pch, '/');
      if (NULL != slash) {
        *slash = '\0';
      }

      /// 不是root路径，检测子路径
      if (NULL == strchr(pch, ':')) {
        /// 检测文件夹是否存在
        result = MDDirectoryIsExists(buffer);
        if (0 == result) {
          /// 创建文件夹，函数执行失败跳出
          if (true != (boost::filesystem::create_directory(buffer))) {
            //result = GetLastError(); // add for linux
            printf("MiniDump::MDCreateDirectory[Fail][The function[CreateDirectory()] is failed][%ld]\n", result);
            break;
          }
        } else {
          /// 检测文件夹函数执行失败，跳出
          if (-1 == result) {
            printf("MiniDump::MDCreateDirectory[Fail][The function[MDDirectoryIsExists()] is failed][%ld]\n", result);
            result = 2;
            break;
          }
          /// 检测文件夹已存在，复位
          result = 0;
        }
      }

      if (NULL != slash) {
        *slash = '\\';
      }

      pch = slash + 1;
    } while (NULL != slash);
  } while (0);

  return result;
}
//////////////////////////////////////////////////////////////////////////


}  //  namespace extern_common

}  //  namespace ysos

#endif

#endif

