/*
# daemoninterface.h
# Definition of daemoninterface
# Created on: 2017-11-16 20:09:11
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20171116, created by JinChengZhe
*/

#ifndef SUP_DAEMON_INTERFACE_H_
#define SUP_DAEMON_INTERFACE_H_

#ifdef _WIN32
#ifdef _YSOS_DLL_EXPORTING
#define YSOS_SDK_EXPORT __declspec(dllexport)
#else
#define YSOS_SDK_EXPORT __declspec(dllimport)
#endif
#define YSOS_SDK_CALLBACK __stdcall
#else
#define YSOS_SDK_CALLBACK __attribute__((__stdcall__))
#define YSOS_SDK_EXPORT /*__declspec(dllimport)*/
#endif

namespace ysos {

namespace sys_daemon_package {

#ifdef __cplusplus
extern "C" {
#endif

//////////////////////////////////////////////////////////////////////////
/*                          外部调用接口函数声明                        */
//////////////////////////////////////////////////////////////////////////
/**
*@brief 启动守护模块
*@return： 无
*/
YSOS_SDK_EXPORT int StartYSOSDaemonModule(void);

/**
*@brief 停止守护模块
*@return： 无
*/
YSOS_SDK_EXPORT int StopYSOSDaemonModule(void);


/* 外部调用接口函数指针声明 */
typedef int (*FP_StartYSOSDaemonModule)(void);
typedef int (*FP_StopYSOSDaemonModule)(void);


#ifdef __cplusplus
}  /// end of extern "C"
#endif

}  /// end of namespace sys_daemon_package

}  /// end of namespace ysos
#endif
