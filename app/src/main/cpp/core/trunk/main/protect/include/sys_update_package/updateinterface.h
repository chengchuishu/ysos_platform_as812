/*
# updateinterface.h
# Definition of updateinterface
# Created on: 2017-11-16 15:50:29
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20171116, created by JinChengZhe
*/

#ifndef SUP_UPDATE_INTERFACE_H_
#define SUP_UPDATE_INTERFACE_H_

#ifdef WIN32
#ifdef _YSOS_DLL_EXPORTING
#define YSOS_SDK_EXPORT __declspec(dllexport)
#else
#define YSOS_SDK_EXPORT __declspec(dllimport)
#endif
#define YSOS_SDK_CALLBACK __stdcall
#else
#define YSOS_SDK_CALLBACK __attribute__((__stdcall__))
#endif


/// C++ Standard Headers
#include <string>
#include <memory>
#include <functional>

namespace ysos {

namespace sys_update_package {

#ifdef __cplusplus
extern "C" {
#endif

enum {
  SUP_PROP_UI_MODE = 1,  ///< 显示模式 0表示无窗口无托盘图标 1表示窗口模式 2表示窗口托盘模式(暂不支持)  // NOLINT
  SUP_PROP_ORG_ID,  ///< 机构号  // NOLINT
  SUP_PROP_TERM_ID,  ///< 终端号  // NOLINT
  SUP_PROP_CURRENT_VERSION,  ///< 当前版本  // NOLINT
  SUP_PROP_LAST_VERSION,  ///< 最新版本  // NOLINT
  SUP_PROP_CHECK_NEW_VERSION_URL,  ///< 监测是否需要更新信息服务器地址  // NOLINT
  SUP_PROP_GET_ALL_AVAILABLE_VERSION_URL,  ///< 获取所有可用版本信息服务器地址  // NOLINT
  SUP_PROP_UPLOAD_CURRENT_MODE_VERSION_URL,  ///< 上传当前模块版本服务器地址  // NOLINT
  SUP_PROP_UPLOAD_UPDATE_STATUS_VERSION_URL,  ///< 上传当前更新状态服务器地址  // NOLINT
  SUP_PROP_UPDATE_SETTINGS,  ///< 设置更新模式  // NOLINT
  SUP_PROP_LOCAL_AVAILABLE_ROLLBACK_VERSION,  ///< 本地可回退版本信息  // NOLINT
  SUP_PROP_DOWNLOAD_PROGRESS_CALLBACK,  ///< 下载进度回调函数  // NOLINT
  SUP_PROP_CONFIG,  ///< 所有配置  // NOLINT
  SUP_PROP_CMD_CHECK_NEW_VERSION,  ///< 获取最新版本命令  // NOLINT
  SUP_PROP_CMD_DOWNLOAD_NEW_VERSION,  ///< 下载最新版本命令  // NOLINT
  SUP_PROP_CMD_UPDATE_NEW_VERSION,  ///< 更新最新版本命令  // NOLINT
  SUP_PROP_CMD_GET_ALL_AVAILABLE_VERSION,  ///< 获取所有可更新版本命令  // NOLINT
  SUP_PROP_CMD_SAVE_CONFIG,  ///< 保存配置文件命令  // NOLINT
};

typedef struct YSOSUpdateConfigInfo {
  unsigned ui_mode_;
  std::string org_id_;
  std::string term_id_;
  std::string current_version_;
  std::string last_version_;
  std::string check_new_version_url_;
  std::string get_all_available_version_url_;
  std::string upload_current_module_version_url_;
  std::string upload_update_status_url_;
  unsigned update_settings_;
  std::string local_available_rollback_version_;

  YSOSUpdateConfigInfo(void)
    : ui_mode_(0),
      update_settings_(0) {
  }

  ~YSOSUpdateConfigInfo(void) {
    ui_mode_ = 0;
    org_id_.clear();
    term_id_.clear();
    current_version_.clear();
    last_version_.clear();
    check_new_version_url_.clear();
    get_all_available_version_url_.clear();
    upload_current_module_version_url_.clear();
    upload_update_status_url_.clear();
    update_settings_ = 0;
    local_available_rollback_version_.clear();
  }

  void Clear(void) {
    ui_mode_ = 0;
    org_id_.clear();
    term_id_.clear();
    current_version_.clear();
    last_version_.clear();
    check_new_version_url_.clear();
    get_all_available_version_url_.clear();
    upload_current_module_version_url_.clear();
    upload_update_status_url_.clear();
    update_settings_ = 0;
    local_available_rollback_version_.clear();
  }
} *YSOSUpdateConfigInfoPtr;

typedef std::function<void(const std::string& event_content)> YSOSUpdateModuleEventHandler;
//////////////////////////////////////////////////////////////////////////
/*                          外部调用接口函数声明                        */
//////////////////////////////////////////////////////////////////////////
/**
*@brief 初始化更新模块  // NOLINT
*@param ysos_update_module_event_handler[Input]： 事件回掉函数  // NOLINT
*@return： 成功返回0，失败返回相应错误值  // NOLINT
*/
YSOS_SDK_EXPORT int InitYSOSUpdateModule(const YSOSUpdateModuleEventHandler& ysos_update_module_event_handler = NULL);
/**
*@brief 启动更新模块  // NOLINT
*@return： 成功返回0，失败返回相应错误值  // NOLINT
*/
YSOS_SDK_EXPORT int StartYSOSUpdateModule(void);

/**
*@brief 停止更新模块  // NOLINT
*@return： 成功返回0，失败返回相应错误值  // NOLINT
*/
YSOS_SDK_EXPORT int StopYSOSUpdateModule(void);

/**
*@brief 设置更新模块配置  // NOLINT
*@param type[Input]： 参数类型  // NOLINT
*@param value[Input]： 参数值  // NOLINT
*@return： 成功返回0，失败返回相应错误值  // NOLINT
*/
YSOS_SDK_EXPORT int SetYSOSUpdateProperty(const unsigned type, void* value);

/**
*@brief 获取更新模块配置  // NOLINT
*@param type[Input]： 参数类型  // NOLINT
*@param value[Input]： 参数值  // NOLINT
*@return： 成功返回0，失败返回相应错误值  // NOLINT
*/
YSOS_SDK_EXPORT int GetYSOSUpdateProperty(const unsigned type, void* value);

/* 外部调用接口函数指针声明 */
typedef int (*FP_InitYSOSUpdateModule)(const YSOSUpdateModuleEventHandler&);
typedef int (*FP_StartYSOSUpdateModule)(void);
typedef int (*FP_StopYSOSUpdateModule)(void);
typedef int (*FP_SetYSOSUpdateProperty)(const unsigned, void*);
typedef int (*FP_GetYSOSUpdateProperty)(const unsigned, void*);

#ifdef __cplusplus
}  /// end of extern "C"
#endif

}  /// end of namespace sys_update_package

}  /// end of namespace ysos
#endif
