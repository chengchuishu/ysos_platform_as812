/*
# ysos.h
# Definition of PlatformRPC
# Created on: 2017-01-19 14:23:23
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20170119, created by JinChengZhe
*/

// ysos.h : main header file for the ysos application
//
#pragma once

//#ifndef __AFXWIN_H__
//#error "include 'stdafx.h' before including this file for PCH"
//#endif

/// Private Headers
/// Platform Headers
#include "../../../public/include/sys_interface_package/ysossdkinterface.h"
using namespace ysos::ysos_sdk;


typedef void *HANDLE;   //add for linux
typedef void *HMODULE;  //add for linux
typedef char TCHAR;     //add for linux
#define MAX_PATH  260   //add for linux 

/*
#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif
*/
#ifndef FALSE
#ifdef __cplusplus
#define FALSE   0       //add for linux
#else
#endif
#endif

#ifndef TRUE
#define TRUE    1       //add for linux
#endif

#ifndef NULL
#define NULL    0       //add for linux
#endif

#pragma region
#define WM_ICON_NOTIFY                        WM_USER+2013
#define WM_YSOS_RUN_STATUS                    WM_USER+2014

//#include <strsafe.h>
/// Platform Headers
/*
#ifdef _WIN32
//#include <ysoslog/ysoslog.h>
#else
#include "../../../public/include/ysoslog/ysoslog.h"
#endif
*/
#include "../../../public/include/ysoslog/ysoslog.h"
extern log4cplus::Logger logger_;

//#include <sys_daemon_package/externcommon.h>
//using namespace ysos::extern_common;

//#include <sys_interface_package/ysossdkinterface.h>
//using namespace ysos::ysos_sdk;

extern FP_OpenYSOSSDK g_fp_open_ysos_sdk;
extern FP_CloseYSOSSDK g_fp_close_ysos_sdk;
extern FP_InitYSOSSDKLogger g_fp_init_ysos_sdk_logger;
extern FP_SetYSOSSDKOpenParams g_fp_set_ysos_sdk_open_params;

#pragma endregion


// CysosApp:
// See ysos.cpp for the implementation of this class
//

class CysosApp {
 public:
  CysosApp();
 public:
  virtual ~CysosApp();

 private:
  /**
  *@brief 设置YSOS Log文件路径函数  // NOLINT
  *@param 无  // NOLINT
  *@return： 成功返回TRUE，失败返回FALSE  // NOLINT
  */
  int SetYSOSLogProperty();
  /**
  *@brief 获取平台日志配置文件路径
  *@param log_template_properties_file_path[Input]： 日志模板配置文件路径
  *@return： 成功返回TRUE，失败返回相应错误值FALSE
  */
  int GetYSOSLogTemplatePropertiesFilePath(std::string& log_template_properties_file_path);
  /**
  *@brief 生成平台日志模块配置文件路径和文件
  *@param log_template_properties_file_path[Input]： 日志模板配置文件路径
  *@param log_properties_file_path[Out]： 生成的日志模板配置文件路径
  *@return： 成功返回TRUE，失败返回相应错误值FALSE
  */
  int CreateYSOSLogPropertiesFile(const std::string& log_template_properties_file_path, std::string& log_properties_file_path);
  /**
  *@brief 设置YSOS 环境变量函数  // NOLINT
  *@param 无  // NOLINT
  *@return： 成功返回TRUE，失败返回FALSE  // NOLINT
  */
  int SetYSOSEnvironmentVariable();
  /**
  *@brief 加载dll，并设置函数接口  // NOLINT
  *@param 无  // NOLINT
  *@return： 成功返回TRUE，失败返回FALSE  // NOLINT
  */
  int LoadLibraryAndSetGetFunctionPtr();

  HANDLE mutex_handle_;  ///< mutext handle，防止程序重复运行
  HMODULE module_handle_;  ///< 动态加载dll的handle

// Overrides
 public:
  virtual int InitInstance();
  virtual int ExitInstance();
  virtual int  OnAutoRun();
  virtual void OnRealRun();
  virtual void OnHandStop();
  virtual void OnHandExit();

 private:
  //  设置ysos.exe运行相关属性值
        typedef struct YSOSConfInfo
        {
            bool auto_run;  ///< 设置是否自动启动平台

            YSOSConfInfo() :
              auto_run(true)
            { }

            ~YSOSConfInfo() {
                auto_run = false;
            }

        } *YSOSConfInfoPtr;

        /**
        *@brief 加载ysos.exe配置ini文件函数  // NOLINT
        *@param 无  // NOLINT
        *@return： 无  // NOLINT
        */
        bool LoadConfig();

        /**
        *@brief 启动平台函数  // NOLINT
        *@param 无  // NOLINT
        *@return： 成功返回TRUE，失败返回FALSE  // NOLINT
        */
        bool RunYSOSPlatform();

        /**
        *@brief 停止平台函数  // NOLINT
        *@param 无  // NOLINT
        *@return： 成功返回TRUE，失败返回FALSE  // NOLINT
        */
        bool StopYSOSPlatform();

        /**
        *@brief 通知守护进程函数  // NOLINT
        *@param 无  // NOLINT
        *@return： 成功返回TRUE，失败返回FALSE  // NOLINT
        */
        bool NotifyDaemonYSOSState();
   private:
        YSOSConfInfoPtr ysos_conf_info_ptr_;  ///< ysos.exe配置属性结构体指针
        
        bool is_run_;  ///< 启动平台标识符    
  

};

extern CysosApp theApp;
