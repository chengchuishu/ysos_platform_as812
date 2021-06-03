/*
# ysossdkinterface.h
# Definition of YSOSSDKInterface
# Created on: 2017-02-23 10:41:59
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20170223, created by JinChengZhe
*/

#ifndef SPS_YSOS_SDK_INTERFACE_H
#define SPS_YSOS_SDK_INTERFACE_H

/**--need add for linux--**/
#if !defined(_MSC_VER)
#include <tr1/memory>
#include <tr1/functional>
#endif

/// C++ Standard Headers
#include <string.h>
#include <memory>
#include <functional>


#ifdef WIN32
#ifdef _YSOS_DLL_EXPORTING
#define YSOS_SDK_EXPORT __declspec(dllexport)
#else
#define YSOS_SDK_EXPORT __declspec(dllimport)
#endif
#define YSOS_SDK_CALLBACK __stdcall
#else
#define YSOS_SDK_CALLBACK __attribute__((__stdcall__))
#define YSOS_SDK_EXPORT   /*__declspec(dllimport)*/  //need add for linux
#endif


/**
*@brief YSOS 服务名称
*/
#define YSOS_SDK_SERVICE_NAME_HEART_BEAT           "Heartbeat"
#define YSOS_SDK_SERVICE_NAME_SERVICE_LIST         "GetServiceList"
#define YSOS_SDK_SERVICE_NAME_WRITE_LOG            "WriteLog"
#define YSOS_SDK_SERVICE_NAME_REG_EVENT            "RegServiceEvent"
#define YSOS_SDK_SERVICE_NAME_UNREG_EVENT          "UnregServiceEvent"
#define YSOS_SDK_SERVICE_NAME_IOCTL                "IOCtl"
#define YSOS_SDK_SERVICE_NAME_LOAD_CONFIG          "LoadConfig"
#define YSOS_SDK_SERVICE_NAME_GET_SERVICE_STATE    "GetServiceState"
#define YSOS_SDK_SERVICE_NAME_SWITCH               "Switch"
#define YSOS_SDK_SERVICE_NAME_INIT                 "Init"
#define YSOS_SDK_SERVICE_NAME_SWITCH_NOTIFY        "SwitchNotify"
#define YSOS_SDK_SERVICE_NAME_READY                "Ready"
#define YSOS_SDK_SERVICE_NAME_EVENT_NOTIFY         "EventNotify"
#define YSOS_SDK_SERVICE_NAME_CUSTOM_EVENT         "CustomEvent"
#define YSOS_AGENT_SERVICE_NAME_GET_DATA           "GetData"
#define YSOS_AGENT_SERVICE_NAME_SET_DATA           "SetData"
#define YSOS_SDK_SERVICE_NAME_UNINIT               "Uninit"

/**
*@brief 平台返回的session id长度
*/
#define YSOS_SDK_SESSION_ID_LENGTH                  64

/**
*@brief YSOS SDK模式
*/
#define YSOS_SDK_CLIENT_MODE                        1
#define YSOS_SDK_SERVER_MODE                        2

/**
*@brief YSOS SDK Open 默认属性值
*/
#define YSOS_SDK_ADDRESS                            "127.0.0.1"
#define YSOS_SDK_PORT                               6002
#define YSOS_SDK_MAX_RECONNECT_AMOUNT               3
#define YSOS_SDK_RECONNECT_TIME                     10

/**
*@brief YSOS SDK Open json 字符串key值
*/
#define YSOS_SDK_OPEN_PARAM_TYPE                    "type"
#define YSOS_SDK_OPEN_PARAM_MODE                    "mode"
#define YSOS_SDK_OPEN_PARAM_ADDRESS                 "address"
#define YSOS_SDK_OPEN_PARAM_PORT                    "port"
#define YSOS_SDK_OPEN_PARAM_MAX_RECONNECT_AMOUNT    "max_reconnect_amount"
#define YSOS_SDK_OPEN_PARAM_RECONNECT_TIME          "reconnect_time"
#define YSOS_SDK_OPEN_PARAM_APPLICATION_NAME        "app_name"
#define YSOS_SDK_OPEN_PARAM_STRATEGY_NAME           "strategy_name"
#define YSOS_SDK_OPEN_PARAM_CONF_FILE_NAME          "conf_file_name"

/**
*@brief YSOS SDK Open json 字符串值长度
*/
#define YSOS_SDK_OPEN_PARAM_ADDRESS_LENGTH          16
#define YSOS_SDK_OPEN_PARAM_APP_NAME_LENGTH         260
#define YSOS_SDK_OPEN_PARAM_STRATEGY_NAME_LENGTH    260
#define YSOS_SDK_OPEN_PARAM_CONF_FILE_NAME_LENGTH   260

/**
*@brief YSOS SDK 错误处理代码
*/
#define YSOS_SDK_ERROR_CODE_OPERATION_ABORTED       3001
#define YSOS_SDK_ERROR_CODE_EOF                     3002
#define YSOS_SDK_ERROR_CODE_BAD_DESCRIPTOR          3003
#define YSOS_SDK_ERROR_CODE_CONNECTION_RESET        3004
#define YSOS_SDK_ERROR_CODE_OTHER                   3005


namespace ysos {

namespace ysos_sdk {

#ifdef __cplusplus
extern "C" {
#endif


//////////////////////////////////////////////////////////////////////////
/**
*@brief SDK Open函数参数结构体声明
*/
//////////////////////////////////////////////////////////////////////////
typedef struct OpenParams {
  /**
  *@brief 打开模式参数属性枚举
  */
  enum ModeProperties {
    JSON_RPC = 1,  ///< 设置jsonrpc属性， 用于本地或者远程通信
    PLATFORM_RPC,  ///< 设置平台rpc属性，用于ocx与平台通信
    PLATFORM,  ///< 设置平台属性，用于启动平台
  };

  /**
  *@brief 配置参数中字符串属性枚举
  */
  enum StringParamsProperties {
    SET_ADDRESS = 1,  ///< 设置地址
    SET_APP_NAME,  ///< 设置应用名称
    SET_STRATEGY_NAME,  ///< 设置策略名称
    SET_CONF_FILE_NAME,  ///< 设置配置文件名称
    SET_CONF_STR,  ///< 设置open配置使用json字符串
    SET_DEFAULT_VALUE,  ///< 设置默认属性值
    RESET_VALUE,  ///< 重置或者清空属性值
  };

  /**
  *@brief 回调函数指针声明
  */
  typedef std::tr1::function<void(const char* session_id, const char* error_param)> ErrorHandler;
  typedef std::tr1::function<void(const char* session_id, const char* service_name, const char* service_param)> DoServiceHandler;
  typedef std::tr1::function<void(const char* session_id, const char* service_name, const char* result)> OnDispatchMessageHandler;

  /**
  *@brief public成员变量声明
  */
  unsigned int type_;  ///< 设置打开sdk的类型，只可设置ModeProperties
  unsigned int mode_;  ///< 设置打开sdk的模式，只可设置服务器或者客户端模式
  char address_[YSOS_SDK_OPEN_PARAM_ADDRESS_LENGTH];  ///< 设置打开sdk的地址
  unsigned int port_;  ///< 设置打开sdk的端口号
  unsigned int max_reconnect_amount_;  ///< 设置打开sdk的最大重连次数
  unsigned int reconnect_time_;  ///< 设置打开sdk的重连事件
  char app_name_[YSOS_SDK_OPEN_PARAM_APP_NAME_LENGTH];  ///< 设置打开sdk的app名称，用于客户端模式下，与平台建立连接时，必须设置
  char strategy_name_[YSOS_SDK_OPEN_PARAM_STRATEGY_NAME_LENGTH];  ///< 设置打开sdk的策略名称，用于客户端模式下，与平台建立连接时，必须设置
  char conf_file_name_[YSOS_SDK_OPEN_PARAM_CONF_FILE_NAME_LENGTH];  ///< 设置打开sdk的属性文件名称，用于启动平台，必须设置

  ErrorHandler error_handler_;  ///< 错误处理回调函数
  DoServiceHandler do_service_handler_;  ///< 请求服务处理回调函数
  OnDispatchMessageHandler on_dispatch_message_handler_;  ///< 回复处理回调函数

  /**
  *@brief 构造函数
  */
  OpenParams(void)
    : type_(PLATFORM_RPC),
      mode_(YSOS_SDK_CLIENT_MODE),
      port_(YSOS_SDK_PORT),
      max_reconnect_amount_(YSOS_SDK_MAX_RECONNECT_AMOUNT),
      reconnect_time_(YSOS_SDK_RECONNECT_TIME),
      error_handler_(NULL),
      do_service_handler_(NULL),
      on_dispatch_message_handler_(NULL) {
    memset(app_name_, 0, YSOS_SDK_OPEN_PARAM_APP_NAME_LENGTH);
    memset(strategy_name_, 0, YSOS_SDK_OPEN_PARAM_STRATEGY_NAME_LENGTH);
    memset(conf_file_name_, 0, YSOS_SDK_OPEN_PARAM_CONF_FILE_NAME_LENGTH);
    memset(address_, 0, YSOS_SDK_OPEN_PARAM_ADDRESS_LENGTH);
    //strcpy_s(address_, YSOS_SDK_OPEN_PARAM_ADDRESS_LENGTH, YSOS_SDK_ADDRESS); //nedd update for linux
    strcpy(address_, YSOS_SDK_ADDRESS);     //nedd update for linux
  }

  /**
  *@brief 析构函数
  */
  ~OpenParams(void) {
  }

  /**
  *@brief 复制构造函数
  */
  OpenParams(const OpenParams& new_open_params) {
    this->type_ = new_open_params.type_;
    this->mode_ = new_open_params.mode_;
    this->port_ = new_open_params.port_;
    this->max_reconnect_amount_ = new_open_params.max_reconnect_amount_;
    this->reconnect_time_ = new_open_params.reconnect_time_;
    this->error_handler_ = new_open_params.error_handler_;
    this->do_service_handler_ = new_open_params.do_service_handler_;
    this->on_dispatch_message_handler_ = new_open_params.on_dispatch_message_handler_;

    #ifdef _WIN32
      if (NULL != new_open_params.app_name_ && 0 != strlen(new_open_params.app_name_)) {
        memset(this->app_name_, 0, YSOS_SDK_OPEN_PARAM_APP_NAME_LENGTH);
        strcpy_s(this->app_name_, YSOS_SDK_OPEN_PARAM_APP_NAME_LENGTH, new_open_params.app_name_);
      }

      if (NULL != new_open_params.strategy_name_ && 0 != strlen(new_open_params.strategy_name_)) {
        memset(this->strategy_name_, 0, YSOS_SDK_OPEN_PARAM_STRATEGY_NAME_LENGTH);
        strcpy_s(this->strategy_name_, YSOS_SDK_OPEN_PARAM_STRATEGY_NAME_LENGTH, new_open_params.strategy_name_);
      }

      if (NULL != new_open_params.conf_file_name_ && 0 != strlen(new_open_params.conf_file_name_)) {
        memset(this->conf_file_name_, 0, YSOS_SDK_OPEN_PARAM_CONF_FILE_NAME_LENGTH);
        strcpy_s(this->conf_file_name_, YSOS_SDK_OPEN_PARAM_CONF_FILE_NAME_LENGTH, new_open_params.conf_file_name_);
      }

      if (NULL != new_open_params.address_ && 0 != strlen(new_open_params.address_)) {
        memset(this->address_, 0, YSOS_SDK_OPEN_PARAM_ADDRESS_LENGTH);
        strcpy_s(this->address_, YSOS_SDK_OPEN_PARAM_ADDRESS_LENGTH, new_open_params.address_);
      }
    #else
      if (NULL != new_open_params.app_name_ && 0 != strlen(new_open_params.app_name_)) {
        memset(this->app_name_, 0, YSOS_SDK_OPEN_PARAM_APP_NAME_LENGTH);
        strcpy(this->app_name_, new_open_params.app_name_);
      }

      if (NULL != new_open_params.strategy_name_ && 0 != strlen(new_open_params.strategy_name_)) {
        memset(this->strategy_name_, 0, YSOS_SDK_OPEN_PARAM_STRATEGY_NAME_LENGTH);
        strcpy(this->strategy_name_, new_open_params.strategy_name_);
      }

      if (NULL != new_open_params.conf_file_name_ && 0 != strlen(new_open_params.conf_file_name_)) {
        memset(this->conf_file_name_, 0, YSOS_SDK_OPEN_PARAM_CONF_FILE_NAME_LENGTH);
        strcpy(this->conf_file_name_, new_open_params.conf_file_name_);
      }

      if (NULL != new_open_params.address_ && 0 != strlen(new_open_params.address_)) {
        memset(this->address_, 0, YSOS_SDK_OPEN_PARAM_ADDRESS_LENGTH);
        strcpy(this->address_, new_open_params.address_);
      }
    #endif  
  }

  /**
  *@brief 等号运算符重载
  */
  OpenParams& operator=(const OpenParams& new_open_params) {
    if (this == &new_open_params) {
      return (*this);
    }

    this->type_ = new_open_params.type_;
    this->mode_ = new_open_params.mode_;
    this->port_ = new_open_params.port_;
    this->max_reconnect_amount_ = new_open_params.max_reconnect_amount_;
    this->reconnect_time_ = new_open_params.reconnect_time_;
    this->error_handler_ = new_open_params.error_handler_;
    this->do_service_handler_ = new_open_params.do_service_handler_;
    this->on_dispatch_message_handler_ = new_open_params.on_dispatch_message_handler_;

    #ifdef _WIN32 
      if (NULL != new_open_params.app_name_ && 0 != strlen(new_open_params.app_name_)) {
        memset(this->app_name_, 0, YSOS_SDK_OPEN_PARAM_APP_NAME_LENGTH);
        strcpy_s(this->app_name_, YSOS_SDK_OPEN_PARAM_APP_NAME_LENGTH, new_open_params.app_name_);
      }

      if (NULL != new_open_params.strategy_name_ && 0 != strlen(new_open_params.strategy_name_)) {
        memset(this->strategy_name_, 0, YSOS_SDK_OPEN_PARAM_STRATEGY_NAME_LENGTH);
        strcpy_s(this->strategy_name_, YSOS_SDK_OPEN_PARAM_STRATEGY_NAME_LENGTH, new_open_params.strategy_name_);
      }

      if (NULL != new_open_params.conf_file_name_ && 0 != strlen(new_open_params.conf_file_name_)) {
        memset(this->conf_file_name_, 0, YSOS_SDK_OPEN_PARAM_CONF_FILE_NAME_LENGTH);
        strcpy_s(this->conf_file_name_, YSOS_SDK_OPEN_PARAM_CONF_FILE_NAME_LENGTH, new_open_params.conf_file_name_);
      }

      if (NULL != new_open_params.address_ && 0 != strlen(new_open_params.address_)) {
        memset(this->address_, 0, YSOS_SDK_OPEN_PARAM_ADDRESS_LENGTH);
        strcpy_s(this->address_, YSOS_SDK_OPEN_PARAM_ADDRESS_LENGTH, new_open_params.address_);
      }
    #else
      if (NULL != new_open_params.app_name_ && 0 != strlen(new_open_params.app_name_)) {
        memset(this->app_name_, 0, YSOS_SDK_OPEN_PARAM_APP_NAME_LENGTH);
        strcpy(this->app_name_, new_open_params.app_name_);
      }

      if (NULL != new_open_params.strategy_name_ && 0 != strlen(new_open_params.strategy_name_)) {
        memset(this->strategy_name_, 0, YSOS_SDK_OPEN_PARAM_STRATEGY_NAME_LENGTH);
        strcpy(this->strategy_name_, new_open_params.strategy_name_);
      }

      if (NULL != new_open_params.conf_file_name_ && 0 != strlen(new_open_params.conf_file_name_)) {
        memset(this->conf_file_name_, 0, YSOS_SDK_OPEN_PARAM_CONF_FILE_NAME_LENGTH);
        strcpy(this->conf_file_name_, new_open_params.conf_file_name_);
      }

      if (NULL != new_open_params.address_ && 0 != strlen(new_open_params.address_)) {
        memset(this->address_, 0, YSOS_SDK_OPEN_PARAM_ADDRESS_LENGTH);
        strcpy(this->address_, new_open_params.address_);
      }
    #endif
    return (*this);
  }
} OP, *OpenParamsPtr;
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
*@brief sdk外部调用接口函数声明
*/
//////////////////////////////////////////////////////////////////////////
/**
*@brief 打开并初始化传模块
*@param param[Input]： 输入的配置参数，例如 {"type": 1, "mode": 0, "address": "127.0.0.1", "port": 6002, "max_reconnect_amount": 3, "reconnect_time": 60, "app_name": "robotterminalapp", "strategy_name": "robotterminalapp", "conf_file_name": "ysos.conf" }
*@param session_id[Input]： 平台返回的session id
*@return： 成功返回0，失败返回相应错误值
*/
YSOS_SDK_EXPORT int OpenYSOSSDK(void* params, char* session_id = 0);
/**
*@brief 关闭并释放模块
*@param session_id[Input]： 平台返回的session id
*@return： 成功返回0，失败返回相应错误值
*/
YSOS_SDK_EXPORT int CloseYSOSSDK(const char* session_id = 0);
/**
*@brief 发送请求服务
*@param session_id[Input]： 平台返回的session id
*@param service_name[Input]： 服务名
*@param service_param[Input]： 服务参数
*@return： 成功返回0，失败返回相应错误值
*/
YSOS_SDK_EXPORT int YSOSSDKDoService(const char* session_id, const char* service_name, const char* service_param);
/**
*@brief 回复请求服务
*@param session_id[Input]： 平台返回的session id
*@param service_name[Input]： 服务名
*@param result[Input]： 服务处理结果
*@return： 成功返回0，失败返回相应错误值
*/
YSOS_SDK_EXPORT int YSOSSDKOnDispatchMessage(const char* session_id, const char* service_name, const char* result);
/**
*@brief 设置YSOSSDK log file path
*@param log_file_path_ptr[Input]： 日志文件路径
*@return： 成功返回true，失败返回false
*/
YSOS_SDK_EXPORT bool InitYSOSSDKLogger(const char* log_file_path_ptr);
/**
*@brief 设置打开参数
*@param open_params[Input]： 打开参数
*@param type[Input]： 参数类型
*@param value[Input]： 参数值
*@return： 成功返回0，失败返回相应错误值
*/
YSOS_SDK_EXPORT int SetYSOSSDKOpenParams(OpenParams& open_params, const unsigned type, const char* value);
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
*@brief sdk外部调用接口函数指针声明
*/
//////////////////////////////////////////////////////////////////////////
typedef int (*FP_OpenYSOSSDK)(void* params, char* session_id);
typedef int (*FP_CloseYSOSSDK)(const char* session_id);
typedef int (*FP_YSOSSDKDoService)(const char* session_id, const char* service_name, const char* service_param);
typedef int (*FP_YSOSSDKOnDispatchMessage)(const char* session_id, const char* service_name, const char* result);
typedef bool (*FP_InitYSOSSDKLogger)(const char* log_file_path_ptr);
typedef int (*FP_SetYSOSSDKOpenParams)(OpenParams& open_params, const unsigned type, const char* value);
//////////////////////////////////////////////////////////////////////////


#ifdef __cplusplus
}
#endif

}

}

#endif
