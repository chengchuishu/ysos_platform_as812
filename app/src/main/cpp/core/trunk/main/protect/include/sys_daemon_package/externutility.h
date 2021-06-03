/*
# externutility.h
# Definition of extern utilities
# Created on: 2017-09-23 14:38:55
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20170923, created by JinChengZhe
*/

#ifndef SPS_EXTERN_UTILITY_H_
#define SPS_EXTERN_UTILITY_H_

#if _WIN32
  #include <Windows.h>
  #include <string>
  #include <tchar.h>
#else
  #include <string>
  typedef int                 BOOL;
  typedef unsigned long       DWORD;
  typedef const char *LPCSTR;
  #define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)

  #ifndef NULL
    #ifdef __cplusplus
      #define NULL    0
    #else
      #define NULL    ((void *)0)
    #endif
  #endif

  #ifndef FALSE
    #define FALSE               0
  #endif

  #ifndef TRUE
    #define TRUE                1
  #endif

#endif


namespace ysos {

namespace extern_utility {
#if _WIN32
class FileLoader {
 public:
  FileLoader();
  ~FileLoader(void);

#if _YSOS_UNICODE
  BOOL OpenFile(LPCTSTR file_path, const DWORD file_size = 0);
#else
  BOOL OpenFile(LPCSTR file_path, const DWORD file_size= 0);
#endif

  BOOL CloseFile(BOOL is_resized = FALSE);
  BOOL ResizeFile(const DWORD file_data_size);
  void* GetFileData(void);

 private:
  FileLoader(const FileLoader&);
  FileLoader& operator=(const FileLoader&);

  HANDLE file_handle_;
  HANDLE file_map_handle_;
  void* file_data_;
#ifdef _YSOS_UNICODE
  std::wstring file_path_;
#else
  std::string file_path_;
#endif
  DWORD file_size_;

  BOOL is_opened_;
};
#else
class FileLoader {
 public:
  FileLoader();
  ~FileLoader(void);

#if _YSOS_UNICODE
  BOOL OpenFile(LPCTSTR file_path, const DWORD file_size = 0);
#else
  BOOL OpenFile(LPCSTR file_path, const DWORD file_size= 0);
#endif

  BOOL CloseFile(BOOL is_resized = FALSE);
  BOOL ResizeFile(const DWORD file_data_size);
  char* GetFileData();
  DWORD file_size_;
  
  
 private:
  FileLoader(const FileLoader&);
  FileLoader& operator=(const FileLoader&);

  int file_handle_;
  //HANDLE file_map_handle_;
  char* file_data_; //update for linux
#ifdef _YSOS_UNICODE
  std::wstring file_path_;
#else
  std::string file_path_;
#endif
  

  BOOL is_opened_;
};
#endif

}

}

#endif
