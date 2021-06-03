/*
# ysossdk.cpp
# Definition of YSOSSDK
# Created on: 2016-09-29 09:23:33
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20160929, created by JinChengZhe
*/

/// Private Headers
#include "../../../protect/include/sys_platform_sdk/ysossdk.h"
/// C++ Standard Headers
#include <iostream>
#include <utility>
/// ThirdParty Headers
#include <boost/function.hpp>  // NOLINT
#include <boost/shared_ptr.hpp>  // NOLINT
#include <boost/thread/recursive_mutex.hpp>
/// Platform Headers
#include "../../../public/include/sys_interface_package/error.h"
#include "../../../protect/include/protocol_help_package/jsonrpc.h"
#include "../../../protect/include/sys_framework_package/platformrpc.h"
#include "../../../protect/include/protocol_help_package/baserpccallbackimpl.h"
#include "../../../protect/include/protocol_help_package/platformprotocolimpl.h"
#include "../../../public/include/core_help_package/utility.h"
#include "../../../protect/include/sys_platform_sdk/sysinit.h"
#include "../../../protect/include/core_help_package/log.h"
#include "../../../public/include/sys_interface_package/ysos_enum.h"

namespace ysos {

static const char* g_error_param_error_code = "error_code";

/**
 *@brief YSOSSDKRPCCallbackImpl的具体实现  // NOLINT
 */
class YSOSSDKRPCCallbackImpl : public BaseRPCCallbackImpl {
  DECLARE_CREATEINSTANCE(YSOSSDKRPCCallbackImpl);
  DISALLOW_COPY_AND_ASSIGN(YSOSSDKRPCCallbackImpl);
  DECLARE_PROTECT_CONSTRUCTOR(YSOSSDKRPCCallbackImpl);

 public:
  typedef boost::function<void(const char* session_id, const char* error_param)> ErrorHandler;
  typedef boost::function<void(const char* session_id, const char* service_name, const char* service_param)> DoServiceHandler;
  typedef boost::function<void(const char* session_id, const char* service_name, const char* result)> OnDispatchMessageHandler;

  virtual ~YSOSSDKRPCCallbackImpl();
  void HandleDoService(const std::string& session_id, const std::string& service_name, const std::string& service_param);
  void HandleOnDispatchMessage(const std::string& session_id, const std::string& service_name, const std::string& result);
  void HandleError(const std::string& session_id, const std::string& error_param);

  void SetErrorHandler(const ErrorHandler error_handler);
  void SetDoServiceHandler(const DoServiceHandler do_service_handler);
  void SetOnDispatchMessageHandler(const OnDispatchMessageHandler on_dispatch_message_handler);

 private:
  ErrorHandler error_handler_;
  DoServiceHandler do_service_handler_;
  OnDispatchMessageHandler on_dispatch_message_handler_;
};

YSOSSDKRPCCallbackImpl::YSOSSDKRPCCallbackImpl(const std::string &strClassName /* =YSOSSDKRPCCallbackImpl */)
  : BaseRPCCallbackImpl(strClassName),
    error_handler_(NULL),
    do_service_handler_(NULL),
    on_dispatch_message_handler_(NULL) {
}

YSOSSDKRPCCallbackImpl::~YSOSSDKRPCCallbackImpl() {
}

void YSOSSDKRPCCallbackImpl::HandleDoService(const std::string& session_id, const std::string& service_name, const std::string& service_param) {
  YSOS_LOG_DEBUG("YSOSSDKRPCCallbackImpl::HandleDoService[Enter]");

  if (NULL != do_service_handler_) {
    YSOS_LOG_DEBUG("YSOSSDKRPCCallbackImpl::HandleDoService[Check Point][0]");

    do_service_handler_(session_id.c_str(), service_name.c_str(), service_param.c_str());
  }

  YSOS_LOG_DEBUG("YSOSSDKRPCCallbackImpl::HandleDoService[Exit]");

  return;
}

void YSOSSDKRPCCallbackImpl::HandleOnDispatchMessage(const std::string& session_id, const std::string& service_name, const std::string& result) {
  YSOS_LOG_DEBUG("YSOSSDKRPCCallbackImpl::HandleOnDispatchMessage[Enter]");

  if (NULL != on_dispatch_message_handler_) {
    YSOS_LOG_DEBUG("YSOSSDKRPCCallbackImpl::HandleOnDispatchMessage[Check Point][0]");
    on_dispatch_message_handler_(session_id.c_str(), service_name.c_str(), result.c_str());
  }

  YSOS_LOG_DEBUG("YSOSSDKRPCCallbackImpl::HandleOnDispatchMessage[Exit]");

  return;
}

void YSOSSDKRPCCallbackImpl::HandleError(const std::string& session_id, const std::string& error_param) {
  YSOS_LOG_DEBUG("YSOSSDKRPCCallbackImpl::HandleError[Enter]");

  if (NULL != error_handler_) {
    YSOS_LOG_DEBUG("YSOSSDKRPCCallbackImpl::HandleError[Check Point][0]");

    error_handler_(session_id.c_str(), error_param.c_str());
  }

  YSOS_LOG_DEBUG("YSOSSDKRPCCallbackImpl::HandleError[Exit]");

  return;
}

void YSOSSDKRPCCallbackImpl::SetErrorHandler(const ErrorHandler error_handler) {
  YSOS_LOG_DEBUG("YSOSSDKRPCCallbackImpl::SetErrorHandler[Enter]");

  error_handler_ = error_handler;

  YSOS_LOG_DEBUG("YSOSSDKRPCCallbackImpl::SetErrorHandler[Exit]");

  return;
}

void YSOSSDKRPCCallbackImpl::SetDoServiceHandler(const DoServiceHandler do_service_handler) {
  YSOS_LOG_DEBUG("YSOSSDKRPCCallbackImpl::SetDoServiceHandler[Enter]");

  do_service_handler_ = do_service_handler;

  YSOS_LOG_DEBUG("YSOSSDKRPCCallbackImpl::SetDoServiceHandler[Exit]");

  return;
}

void YSOSSDKRPCCallbackImpl::SetOnDispatchMessageHandler(const OnDispatchMessageHandler on_dispatch_message_handler) {
  YSOS_LOG_DEBUG("YSOSSDKRPCCallbackImpl::SetOnDispatchMessageHandler[Enter]");

  on_dispatch_message_handler_ = on_dispatch_message_handler;

  YSOS_LOG_DEBUG("YSOSSDKRPCCallbackImpl::SetOnDispatchMessageHandler[Exit]");

  return;
}

/**
*@brief Loger的静态声明  // NOLINT
*/
namespace ysos_sdk_loger {
static log4cplus::Logger logger_;
}
using namespace ysos_sdk_loger;

/**
 *@brief YSOSSDK的具体实现  // NOLINT
 */
DEFINE_SINGLETON(YSOSSDK)
YSOSSDK::YSOSSDK(const std::string &strClassName /* =YSOSSDK */)
  : open_params_ptr_(NULL),
    rpc_ptr_(NULL),
    ysos_sdk_rpc_callback_impl_ptr_(NULL),
    retry_connect_params_ptr_(NULL),
    reconnect_amount_counter_(0),
    running_status_(IS_INITIAL) {
  if (false == logger_.valid()) {
    logger_ = GetUtility()->GetLogger("ysos.sdk");
  }
}

YSOSSDK::~YSOSSDK() {
  if (NULL != open_params_ptr_) {
    delete open_params_ptr_;
    open_params_ptr_ = NULL;
  }

  if (NULL != rpc_ptr_) {
    delete rpc_ptr_;
    rpc_ptr_ = NULL;
  }

  if (NULL != ysos_sdk_rpc_callback_impl_ptr_) {
    delete ysos_sdk_rpc_callback_impl_ptr_;
    ysos_sdk_rpc_callback_impl_ptr_ = NULL;
  }

  if (NULL != retry_connect_params_ptr_) {
    delete retry_connect_params_ptr_;
    retry_connect_params_ptr_ = NULL;
  }

  reconnect_amount_counter_ = 0;

  running_status_ = IS_INITIAL;
}

int YSOSSDK::Open(void* params, char* session_id) {
  YSOS_LOG_DEBUG("YSOSSDK::Open[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {
    /// 判断模块是否已开启
    if (IS_RUNNING == GetRunningStatus()) {
      YSOS_LOG_DEBUG("YSOSSDK::Open[It is already running]");
      result = YSOS_ERROR_SUCCESS;
      break;
    }

    YSOS_LOG_DEBUG("YSOSSDK::Open[Check Point][0]");

    /// 重置运行状态
    SetRunningStatus(IS_INITIAL);

    if (NULL == params) {
      YSOS_LOG_DEBUG("YSOSSDK::Open[Fail][params is NULL]");
      break;
    }

    YSOS_LOG_DEBUG("YSOSSDK::Open[Check Point][1]");

    /// 强转成YSOSSDK的Open配置参数指针
    OpenParams* duplicated_open_params_ptr = static_cast<OpenParams*>(params);

    /// 创建YSOSSDK的Open配置参数，并拷贝一份传入的配置参数给刚创建的YSOSSDK的Open配置参数
    if (NULL == open_params_ptr_) {
      YSOS_LOG_DEBUG("YSOSSDK::Open[Check Point][ysos_sdk_config_params_ptr_ is NULL]");

      open_params_ptr_ = new OpenParams(*duplicated_open_params_ptr);
      if (NULL == open_params_ptr_) {
        YSOS_LOG_DEBUG("YSOSSDK::Open[Fail][ysos_sdk_config_params_ptr_ is NULL]");
        break;
      }

      YSOS_LOG_DEBUG("YSOSSDK::Open[Check Point][2]");
    }

    YSOS_LOG_DEBUG("YSOSSDK::Open[Check Point][3]");

    if (OpenParams::PLATFORM == open_params_ptr_->type_) {
      YSOS_LOG_DEBUG("YSOSSDK::Open[Check Point][OpenParams::PLATFORM][Open][Enter]");

      if (YSOS_ERROR_SUCCESS != GetSysInit()->Init(open_params_ptr_->conf_file_name_)) {
        YSOS_LOG_DEBUG("YSOSSDK::Open[Fail][OpenParams::PLATFORM]The function[GetSysInit()->Init()] is failed");
        break;
      }

      YSOS_LOG_DEBUG("YSOSSDK::Open[Check Point][OpenParams::PLATFORM][Open][End]");

      result = YSOS_ERROR_SUCCESS;
      break;
    }

    YSOS_LOG_DEBUG("YSOSSDK::Open[Check Point][4]");

    /// PLATFORMRPC/客户端模式下，判断是否传入了状态机配置文件名称
    if (OpenParams::PLATFORM_RPC == open_params_ptr_->type_ && YSOS_SDK_CLIENT_MODE == open_params_ptr_->mode_) {
      YSOS_LOG_DEBUG("YSOSSDK::Open[Check strategy name]");
#if (!_CERTUSNET)
      if (NULL == open_params_ptr_->strategy_name_ || 0 == strlen(open_params_ptr_->strategy_name_)) {
        YSOS_LOG_DEBUG("YSOSSDK::Open[Fail][Check strategy name fail]");
        break;
      }
#endif
      YSOS_LOG_DEBUG("YSOSSDK::Open[Check Point][5]");
    }

    /// for debug
    //////////////////////////////////////////////////////////////////////////
    YSOS_LOG_DEBUG("type: " << open_params_ptr_->type_);
    YSOS_LOG_DEBUG("mode: " << open_params_ptr_->mode_);
    YSOS_LOG_DEBUG("address: " << open_params_ptr_->address_);
    YSOS_LOG_DEBUG("port: " << open_params_ptr_->port_);
    YSOS_LOG_DEBUG("reconnect_time: " << open_params_ptr_->reconnect_time_);
    YSOS_LOG_DEBUG("max_reconnect_amount: " << open_params_ptr_->max_reconnect_amount_);
    YSOS_LOG_DEBUG("application_name: " << open_params_ptr_->app_name_);
    YSOS_LOG_DEBUG("strategy_name: " << open_params_ptr_->strategy_name_);

    if (NULL != open_params_ptr_->do_service_handler_) {
      YSOS_LOG_DEBUG("DoServiceCallback: availed");
    }

    if (NULL != open_params_ptr_->on_dispatch_message_handler_) {
      YSOS_LOG_DEBUG("OnDispatchMessageCallback: availed");
    }

    if (NULL != open_params_ptr_->error_handler_) {
      YSOS_LOG_DEBUG("HandleErrorCallback: availed");
    }
    //////////////////////////////////////////////////////////////////////////

    YSOS_LOG_DEBUG("YSOSSDK::Open[Check Point][6]");

    /// 初始化重连参数
    retry_connect_params_ptr_ = new std::pair<std::string, std::string>;
    if (NULL == retry_connect_params_ptr_) {
      YSOS_LOG_DEBUG("YSOSSDK::Open[Fail][retry_connect_params_ptr_ is NULL]");
      break;
    }

    YSOS_LOG_DEBUG("YSOSSDK::Open[Check Point][7]");

    /// 初始化最大重连次数计数器
    reconnect_amount_counter_ = 0;

    /// 创建rpc callback并初始化，用于rpc Open参数
    if (NULL != ysos_sdk_rpc_callback_impl_ptr_) {
      YSOS_LOG_DEBUG("YSOSSDK::Open[Fail][ysos_sdk_rpc_callback_impl_ptr_ is not NULL]");
      break;
    }

    YSOS_LOG_DEBUG("YSOSSDK::Open[Check Point][8]");

    ysos_sdk_rpc_callback_impl_ptr_ = YSOSSDKRPCCallbackImpl::CreateInstance();
    if (NULL == ysos_sdk_rpc_callback_impl_ptr_) {
      YSOS_LOG_DEBUG("YSOSSDK::Open[Fail][ysos_sdk_rpc_callback_impl_ptr_ is NULL]");
      break;
    }

    YSOS_LOG_DEBUG("YSOSSDK::Open[Check Point][9]");

    if (YSOS_SDK_CLIENT_MODE == open_params_ptr_->mode_) {
      ysos_sdk_rpc_callback_impl_ptr_->SetMode(PROP_CLIENT);
    } else {
      ysos_sdk_rpc_callback_impl_ptr_->SetMode(PROP_SERVER);
    }

    YSOS_LOG_DEBUG("YSOSSDK::Open[Check Point][10]");

    ysos_sdk_rpc_callback_impl_ptr_->SetAddress(open_params_ptr_->address_);
    ysos_sdk_rpc_callback_impl_ptr_->SetPort(open_params_ptr_->port_);
    ysos_sdk_rpc_callback_impl_ptr_->SetReconnectTime(open_params_ptr_->reconnect_time_);
    ysos_sdk_rpc_callback_impl_ptr_->SetMaxReconnectAmount(open_params_ptr_->max_reconnect_amount_);
    ysos_sdk_rpc_callback_impl_ptr_->SetAplicationName(open_params_ptr_->app_name_);
    ysos_sdk_rpc_callback_impl_ptr_->SetStrategyName(open_params_ptr_->strategy_name_);
    ysos_sdk_rpc_callback_impl_ptr_->SetErrorHandler(boost::bind(&YSOSSDK::HandleError, this, _1, _2));
    ysos_sdk_rpc_callback_impl_ptr_->SetDoServiceHandler(boost::bind(&YSOSSDK::HandleDoService, this, _1, _2, _3));
    ysos_sdk_rpc_callback_impl_ptr_->SetOnDispatchMessageHandler(boost::bind(&YSOSSDK::HandleOnDispatchMessage, this, _1, _2, _3));

    YSOS_LOG_DEBUG("YSOSSDK::Open[Check Point][11]");

    /// 创建rpc实例对象
    if (NULL != rpc_ptr_) {
      YSOS_LOG_DEBUG("YSOSSDK::Open[Fail][rpc_ptr_ is not NULL]");
      break;
    }

    YSOS_LOG_DEBUG("YSOSSDK::Open[Check Point][12]");

    if (OpenParams::JSON_RPC == open_params_ptr_->type_) {
      rpc_ptr_ = JsonRPC::CreateInstance();
    } else if (OpenParams::PLATFORM_RPC == open_params_ptr_->type_) {
      rpc_ptr_ = PlatformRPC::CreateInstance();
    }

    YSOS_LOG_DEBUG("YSOSSDK::Open[Check Point][13]");

    if (NULL == rpc_ptr_) {
      YSOS_LOG_DEBUG("YSOSSDK::Open[Fail][rpc_ptr_ is NULL]");
      break;
    }

    YSOS_LOG_DEBUG("YSOSSDK::Open[Check Point][14]");

    /// 打开rpc
    std::string duplicated_sesssion_id;
    if (YSOS_ERROR_SUCCESS != rpc_ptr_->Open(ysos_sdk_rpc_callback_impl_ptr_, &duplicated_sesssion_id)) {
      YSOS_LOG_DEBUG("YSOSSDK::Open[Fail]The function[rpc_ptr_->Open()] is failed");
      break;
    }

    YSOS_LOG_DEBUG("YSOSSDK::Open[Check Point][15]");

    /// 客户端模式下，把服务器返回的session id设置给传入的session_id
    if (PROP_CLIENT == ysos_sdk_rpc_callback_impl_ptr_->GetMode()) {
      if (true == duplicated_sesssion_id.empty() ||
          "0" == duplicated_sesssion_id) {
        YSOS_LOG_DEBUG("YSOSSDK::Open[Fail] duplicated_sesssion_id is empty or 0");
        break;
      } else {
        if (NULL != session_id) {
          YSOS_LOG_DEBUG("YSOSSDK::Open[Check Point][Copy session id]");
          #ifdef _WIN32
            strcpy_s(session_id, YSOS_SDK_SESSION_ID_LENGTH, duplicated_sesssion_id.c_str());   //need update for linux
          #else  
            strcpy(session_id, duplicated_sesssion_id.c_str());  //need update for linux
          #endif
          YSOS_LOG_DEBUG("session_id: " << session_id);
        } else {
          YSOS_LOG_DEBUG("YSOSSDK::Open[Fail] session_id is NULL");
          break;
        }
      }
    }

    /// 设置运行状态
    SetRunningStatus(IS_RUNNING);

    YSOS_LOG_DEBUG("YSOSSDK::Open[Check Point][End]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  YSOS_LOG_DEBUG("YSOSSDK::Open[Exit]");

  return result;
}

int YSOSSDK::Close(const char* session_id) {
  YSOS_LOG_DEBUG("YSOSSDK::Close[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {
    /// 判断模块是否已关闭
    if (IS_STOPPED == GetRunningStatus()) {
      YSOS_LOG_DEBUG("YSOSSDK::Close[It is already stopped]");
      result = YSOS_ERROR_SUCCESS;
      break;
    }

    YSOS_LOG_DEBUG("YSOSSDK::Close[Check Point][0]");

    /// 设置运行状态
    SetRunningStatus(IS_STOPPED);

    /// OpenParams::PLATFORM模式下，关闭机制
    if (OpenParams::PLATFORM == open_params_ptr_->type_) {
      YSOS_LOG_DEBUG("YSOSSDK::Close[Check Point][OpenParams::PLATFORM][Close][Enter]");
      if (YSOS_ERROR_SUCCESS != GetSysInit()->Uninit()) {
        YSOS_LOG_DEBUG("YSOSSDK::Close[Fail]The function[GetSysInit()->Uninit()] is failed");
        break;
      }
      YSOS_LOG_DEBUG("YSOSSDK::Close[Check Point][OpenParams::PLATFORM][Close][End]");
      result = YSOS_ERROR_SUCCESS;
      break;
    }

    YSOS_LOG_DEBUG("YSOSSDK::Close[Check Point][1]");

    /// 根据传入的session id，关闭rpc
    if (NULL == rpc_ptr_) {
      YSOS_LOG_DEBUG("YSOSSDK::Close[Fail][rpc_ptr_ is NULL]");
      break;
    }

    YSOS_LOG_DEBUG("YSOSSDK::Close[Check Point][2]");

    if (NULL != session_id) {
      YSOS_LOG_DEBUG("YSOSSDK::Close[Check Point][session_id][" << session_id << "]");

      std::string duplicated_session_id = session_id;
      if (YSOS_ERROR_SUCCESS != rpc_ptr_->Close(&duplicated_session_id)) {
        YSOS_LOG_DEBUG("YSOSSDK::Close[Fail]The function[rpc_ptr_->Close()] is failed");
        break;
      }
    } else {
      YSOS_LOG_DEBUG("YSOSSDK::Close[Check Point][session_id is NULL]");
      if (YSOS_ERROR_SUCCESS != rpc_ptr_->Close()) {
        YSOS_LOG_DEBUG("YSOSSDK::Close[Fail]The function[rpc_ptr_->Close()] is failed");
        break;
      }
    }

    YSOS_LOG_DEBUG("YSOSSDK::Close[Check Point][3]");

    if (NULL != rpc_ptr_) {
      YSOS_LOG_DEBUG("YSOSSDK::Close[Check Point][rpc_ptr_ is NULL]");
      delete rpc_ptr_;
      rpc_ptr_ = NULL;
    }

    YSOS_LOG_DEBUG("YSOSSDK::Close[Check Point][4]");

    /// 释放YSOSSDK配置参数
    if (NULL != open_params_ptr_) {
      YSOS_LOG_DEBUG("YSOSSDK::Close[Check Point][ysos_sdk_config_params_ptr_ is NULL]");
      delete open_params_ptr_;
      open_params_ptr_ = NULL;
    }

    YSOS_LOG_DEBUG("YSOSSDK::Close[Check Point][5]");

    /// 释放rpccallback配置参数
    if (NULL != ysos_sdk_rpc_callback_impl_ptr_) {
      YSOS_LOG_DEBUG("YSOSSDK::Close[Check Point][ysos_sdk_rpc_callback_impl_ptr_ is NULL]");
      delete ysos_sdk_rpc_callback_impl_ptr_;
      ysos_sdk_rpc_callback_impl_ptr_ = NULL;
    }

    YSOS_LOG_DEBUG("YSOSSDK::Close[Check Point][6]");

    /// 释放重连接配置参数
    if (NULL != retry_connect_params_ptr_) {
      YSOS_LOG_DEBUG("YSOSSDK::Close[Check Point][retry_connect_params_ptr_ is NULL]");
      delete retry_connect_params_ptr_;
      retry_connect_params_ptr_ = NULL;
    }

    /// 重置最大重连次数计数器
    reconnect_amount_counter_ = 0;

    YSOS_LOG_DEBUG("YSOSSDK::Close[Check Point][End]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  YSOS_LOG_DEBUG("YSOSSDK::Close[Exit]");

  return result;
}

int YSOSSDK::DoService(const char* session_id, const char* service_name, const char* service_param) {
  YSOS_LOG_DEBUG("YSOSSDK::DoService[Enter]");

  if (NULL != session_id) {
    YSOS_LOG_DEBUG("session_id: " << session_id);
  }

  if (NULL != service_name) {
    YSOS_LOG_DEBUG("service_name: " << service_name);
  }

  if (NULL != service_param) {
    YSOS_LOG_DEBUG("service_param: " << service_param);
  }

  int result = YSOS_ERROR_FAILED;

  do {
    /// 日志服务，暂时只为前端留本地日志
    if (0 == strcmp(service_name, YSOS_SDK_SERVICE_NAME_WRITE_LOG)) {
      YSOS_LOG_DEBUG("YSOSSDK::DoService[WriteLog][Enter]");
      YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.ysosapp"), service_param);
      YSOS_LOG_DEBUG("YSOSSDK::DoService[WriteLog][Exit]");
      result = YSOS_ERROR_SUCCESS;
      break;
    }

    YSOS_LOG_DEBUG("YSOSSDK::DoService[Check Point][0]");

    /// 判断模块是否已开启
    if (IS_RUNNING != GetRunningStatus()) {
      YSOS_LOG_DEBUG("YSOSSDK::DoService[It is not running]");
      result = YSOS_ERROR_INVALID_OPERATION;
      break;
    }

    YSOS_LOG_DEBUG("YSOSSDK::DoService[Check Point][1]");

    /// 其他服务
    if (NULL == rpc_ptr_) {
      YSOS_LOG_DEBUG("YSOSSDK::DoService[Fail][rpc_ptr_ is NULL]");
      break;
    }

    YSOS_LOG_DEBUG("YSOSSDK::DoService[Check Point][2]");

    std::string duplicated_session_id, duplicated_service_name, duplicated_service_param;

    if (NULL != session_id) {
      duplicated_session_id = session_id;
    }

    YSOS_LOG_DEBUG("YSOSSDK::DoService[Check Point][3]");

    if (NULL != service_name) {
      duplicated_service_name = service_name;
    }

    YSOS_LOG_DEBUG("YSOSSDK::DoService[Check Point][4]");

    if (NULL != service_param) {
      duplicated_service_param = service_param;
    }

    YSOS_LOG_DEBUG("YSOSSDK::DoService[Check Point][5]");

//     YSOS_LOG_DEBUG("duplicated_session_id: " << duplicated_session_id << " duplicated_service_name: " << duplicated_service_name << " duplicated_service_param: " << duplicated_service_param );

    if (YSOS_ERROR_SUCCESS != rpc_ptr_->DoService(duplicated_session_id, duplicated_service_name, duplicated_service_param)) {
      YSOS_LOG_DEBUG("YSOSSDK::DoService[Fail]The function[rpc_ptr_->DoService()] is failed");
      break;
    }

    YSOS_LOG_DEBUG("YSOSSDK::DoService[Check Point][End]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  YSOS_LOG_DEBUG("YSOSSDK::DoService[Exit]");

  return result;
}

int YSOSSDK::OnDispatchMessage(const char* session_id, const char* service_name, const char* result) {
  YSOS_LOG_DEBUG("YSOSSDK::OnDispatchMessage[Enter]");

  if (NULL != session_id) {
    YSOS_LOG_DEBUG("session_id: " << session_id);
  }

  if (NULL != service_name) {
    YSOS_LOG_DEBUG("service_name: " << service_name);
  }

  if (NULL != result) {
    YSOS_LOG_DEBUG("result: " << result);
  }

  int return_value = YSOS_ERROR_FAILED;

  do {
    /// 判断模块是否已开启
    if (IS_RUNNING != GetRunningStatus()) {
      YSOS_LOG_DEBUG("YSOSSDK::OnDispatchMessage[It is not running]");
      return_value = YSOS_ERROR_INVALID_OPERATION;
      break;
    }

    YSOS_LOG_DEBUG("YSOSSDK::OnDispatchMessage[Check Point][0]");

    if (NULL == rpc_ptr_) {
      YSOS_LOG_DEBUG("YSOSSDK::OnDispatchMessage[Fail][rpc_ptr_ is NULL]");
      break;
    }

    YSOS_LOG_DEBUG("YSOSSDK::OnDispatchMessage[Check Point][1]");

    std::string duplicated_session_id, duplicated_service_name, duplicated_result;

    if (NULL != session_id) {
      duplicated_session_id = session_id;
    }

    YSOS_LOG_DEBUG("YSOSSDK::OnDispatchMessage[Check Point][2]");

    if (NULL != service_name) {
      duplicated_service_name = service_name;
    }

    YSOS_LOG_DEBUG("YSOSSDK::OnDispatchMessage[Check Point][3]");

    if (NULL != result) {
      duplicated_result = result;
    }

    YSOS_LOG_DEBUG("YSOSSDK::OnDispatchMessage[Check Point][4]");

//     YSOS_LOG_DEBUG("duplicated_session_id: " << duplicated_session_id << " duplicated_service_name: " << duplicated_service_name << " duplicated_result: " << duplicated_result );

    if (YSOS_ERROR_SUCCESS != rpc_ptr_->OnDispatchMessage(duplicated_session_id, duplicated_service_name, duplicated_result)) {
      YSOS_LOG_DEBUG("YSOSSDK::OnDispatchMessage[Fail]The function[rpc_ptr_->OnDispatchMessage(] is failed");
      break;
    }

    YSOS_LOG_DEBUG("YSOSSDK::OnDispatchMessage[Check Point][End]");

    return_value = YSOS_ERROR_SUCCESS;
  } while (0);

  YSOS_LOG_DEBUG("YSOSSDK::OnDispatchMessage[Exit]");

  return return_value;
}

void YSOSSDK::HandleDoService(const char* session_id, const char* service_name, const char* service_param) {
  YSOS_LOG_DEBUG("YSOSSDK::HandleDoService[Enter]");

  if (NULL != session_id) {
    YSOS_LOG_DEBUG("session_id: " << session_id);
  }

  if (NULL != service_name) {
    YSOS_LOG_DEBUG("service_name: " << service_name);
  }

  if (NULL != service_param) {
    YSOS_LOG_DEBUG("service_param: " << service_param);
  }

  int result = YSOS_ERROR_FAILED;

  do {
    /// 判断模块是否已关闭
    if (IS_STOPPED == GetRunningStatus()) {
      YSOS_LOG_DEBUG("YSOSSDK::HandleDoService[It is already stopped]");
      result = YSOS_ERROR_SUCCESS;
      break;
    }

    YSOS_LOG_DEBUG("YSOSSDK::HandleDoService[Check Point][0]");

    if (NULL == open_params_ptr_) {
      YSOS_LOG_DEBUG("YSOSSDK::HandleDoService[Fail][ysos_sdk_config_params_ptr_ is failed]");
      break;
    }

    YSOS_LOG_DEBUG("YSOSSDK::HandleDoService[Check Point][1]");

    if (NULL != open_params_ptr_->do_service_handler_) {
      YSOS_LOG_DEBUG("YSOSSDK::HandleDoService[Check Point][Call DoServiceCallback()]");
      open_params_ptr_->do_service_handler_(session_id, service_name, service_param);
    }

    YSOS_LOG_DEBUG("YSOSSDK::HandleDoService[Check Point][End]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  YSOS_LOG_DEBUG("YSOSSDK::HandleDoService[Exit]");

  return;
}

void YSOSSDK::HandleOnDispatchMessage(const char* session_id, const char* service_name, const char* result) {
  YSOS_LOG_DEBUG("YSOSSDK::HandleOnDispatchMessage[Enter]");

  if (NULL != session_id) {
    YSOS_LOG_DEBUG("session_id: " << session_id);
  }

  if (NULL != service_name) {
    YSOS_LOG_DEBUG("service_name: " << service_name);
  }

  if (NULL != result) {
    YSOS_LOG_DEBUG("result: " << result);
  }

  int return_value = YSOS_ERROR_FAILED;
  do {
    /// 判断模块是否已关闭
    if (IS_STOPPED == GetRunningStatus()) {
      YSOS_LOG_DEBUG("YSOSSDK::HandleOnDispatchMessage[It is already stopped]");
      result = YSOS_ERROR_SUCCESS;
      break;
    }

    YSOS_LOG_DEBUG("YSOSSDK::HandleOnDispatchMessage[Check Point][0]");

    if (NULL == open_params_ptr_) {
      YSOS_LOG_DEBUG("YSOSSDK::HandleOnDispatchMessage[Fail][ysos_sdk_config_params_ptr_ is NULL]");
      break;
    }

    YSOS_LOG_DEBUG("YSOSSDK::HandleOnDispatchMessage[Check Point][1]");

    if (NULL != open_params_ptr_->on_dispatch_message_handler_) {
      YSOS_LOG_DEBUG("YSOSSDK::HandleOnDispatchMessage[Check Point][Call OnDispatchMessageCallback()]");
      open_params_ptr_->on_dispatch_message_handler_(session_id, service_name, result);
    }

    YSOS_LOG_DEBUG("YSOSSDK::HandleOnDispatchMessage[Check Point][End]");

    return_value = YSOS_ERROR_SUCCESS;
  } while (0);

  YSOS_LOG_DEBUG("YSOSSDK::HandleOnDispatchMessage[Exit]");

  return;
}

void YSOSSDK::HandleError(const char* session_id, const char* error_param) {
  YSOS_LOG_DEBUG("YSOSSDK::HandleError[Enter]");

  if (NULL != session_id) {
    YSOS_LOG_DEBUG("session_id: " << session_id);
  }

  if (NULL != error_param) {
    YSOS_LOG_DEBUG("error_param: " << error_param);
  }

  int result = YSOS_ERROR_FAILED;
  do {
    /// 客户端模式下，检测到远程服务器关闭，尝试重新连接
    if (PROP_CLIENT == ysos_sdk_rpc_callback_impl_ptr_->GetMode()) {

      YSOS_LOG_DEBUG("YSOSSDK::HandleError[Check Point][PROP_CLIENT][Enter]");

      *retry_connect_params_ptr_ = std::make_pair<std::string, std::string>(session_id, error_param);

      boost::thread(boost::bind(&YSOSSDK::RetryConnect, this, retry_connect_params_ptr_));

      YSOS_LOG_DEBUG("YSOSSDK::HandleError[Check Point][PROP_CLIENT][End]");

      result = YSOS_ERROR_SUCCESS;
      break;
    }

    YSOS_LOG_DEBUG("YSOSSDK::HandleError[Check Point][0]");

    if (NULL == open_params_ptr_) {
      YSOS_LOG_DEBUG("YSOSSDK::HandleError[Fail][ysos_sdk_config_params_ptr_ is NULL]");
      break;
    }

    YSOS_LOG_DEBUG("YSOSSDK::HandleError[Check Point][1]");

    if (NULL != open_params_ptr_->error_handler_) {
      YSOS_LOG_DEBUG("YSOSSDK::HandleError[Check Point][Call HandleErrorCallback()]");
      open_params_ptr_->error_handler_(session_id, error_param);
    }

    YSOS_LOG_DEBUG("YSOSSDK::HandleError[Check Point][End]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  YSOS_LOG_DEBUG("YSOSSDK::HandleError[Exit]");

  return;
}

void YSOSSDK::RetryConnect(void* params) {
  YSOS_LOG_DEBUG("YSOSSDK::RetryConnect[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {
    /// 判断模块是否已关闭
    if (IS_STOPPED == GetRunningStatus()) {
      YSOS_LOG_DEBUG("YSOSSDK::RetryConnect[It is already stopped]");
      result = YSOS_ERROR_SUCCESS;
      break;
    }

    YSOS_LOG_DEBUG("YSOSSDK::RetryConnect[Check Point][0]");

    if (NULL == params) {
      YSOS_LOG_DEBUG("YSOSSDK::RetryConnect[Fail][params is NULL]");
      break;
    }

    YSOS_LOG_DEBUG("YSOSSDK::RetryConnect[Check Point][1]");

    std::pair<std::string, std::string>* retry_connect_params_ptr = static_cast<std::pair<std::string, std::string>*>(params);

    YSOS_LOG_DEBUG("YSOSSDK::RetryConnect[Check Point][2]");

    /// 服务器断开时，error_param返回json字符串，即retry_connect_params_ptr->second
    /*
    {
    "callback_type": "OnDisconnect",
    "description": "远程主机强迫关闭了一个现有的连接。",
    "error_code": "3004"
    }
    */

    /// 客户端模式下，检测到远程服务器关闭，尝试重新连接
    JsonValue json_value_error_param;
    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonObjectFromString(std::string(retry_connect_params_ptr->second), json_value_error_param)) {
      YSOS_LOG_DEBUG("YSOSSDK::RetryConnect[Fail]The function[PlatformProtocolImpl::JsonObjectFromString()] is failed");
      break;
    }

    YSOS_LOG_DEBUG("YSOSSDK::RetryConnect[Check Point][3]");

    std::string error_code;
    if (true == json_value_error_param.isMember(g_error_param_error_code) &&
        true == json_value_error_param[g_error_param_error_code].isString()) {
      YSOS_LOG_DEBUG("YSOSSDK::RetryConnect[Check Point][Catch Error Code]");
      error_code = json_value_error_param[g_error_param_error_code].asString();
    }

    YSOS_LOG_DEBUG("YSOSSDK::RetryConnect[Check Point][error_code][" << error_code << "]");

    /// 判断是否是3004
    if (YSOS_ERROR_TRANSPORT_ERROR_CONNECTION_RESET == boost::lexical_cast<int>(error_code.c_str())) {
      YSOS_LOG_DEBUG("YSOSSDK::RetryConnect[Check Point][Retry connect][Enter]");

      bool is_reconnect = false;
      while (reconnect_amount_counter_ < ysos_sdk_rpc_callback_impl_ptr_->GetMaxReconnectAmount()) {

        YSOS_LOG_DEBUG("YSOSSDK::RetryConnect[Check Point][Start][reconnect_amount_counter_][" << reconnect_amount_counter_ << "]");

        /// 判断模块是否已关闭
        if (IS_STOPPED == GetRunningStatus()) {
          YSOS_LOG_DEBUG("YSOSSDK::RetryConnect[It is already stopped]");
          return;
        }

        YSOS_LOG_DEBUG("YSOSSDK::RetryConnect[Check Point][4]");

        /// 关闭rpc连接
        if (YSOS_ERROR_SUCCESS != rpc_ptr_->Close()) {
          YSOS_LOG_DEBUG("YSOSSDK::RetryConnect[Fail]The function[rpc_ptr_->Close()] is failed");
          break;
        }

        YSOS_LOG_DEBUG("YSOSSDK::RetryConnect[Check Point][5]");

        /// 休眠指定时间
        boost::this_thread::sleep_for(boost::chrono::seconds(ysos_sdk_rpc_callback_impl_ptr_->GetReconnectTime()));

        YSOS_LOG_DEBUG("YSOSSDK::RetryConnect[Check Point][6]");

        /// 将原有的session id给到服务器，让服务器重定向
        std::string duplicated_sesssion_id = retry_connect_params_ptr->first;

        /// 重新打开rpc连接
        if (YSOS_ERROR_SUCCESS != rpc_ptr_->Open(ysos_sdk_rpc_callback_impl_ptr_, &duplicated_sesssion_id)) {
          YSOS_LOG_DEBUG("YSOSSDK::RetryConnect[Fail]The function[rpc_ptr_->Open()] is failed");
        } else {
          YSOS_LOG_DEBUG("YSOSSDK::RetryConnect[Check Point][Retry connect success]");

          /// 重置重连次数
          reconnect_amount_counter_ = 0;

          /// 成功重连跳出
          is_reconnect = true;

          break;
        }

        YSOS_LOG_DEBUG("YSOSSDK::RetryConnect[Check Point][7]");

        /// 自增重连次数
        ++reconnect_amount_counter_;

        YSOS_LOG_DEBUG("YSOSSDK::RetryConnect[Check Point][End][reconnect_amount_counter_][" << reconnect_amount_counter_ << "]");
      }

      /// 重连成功，直接返回
      if (true == is_reconnect) {
        YSOS_LOG_DEBUG("YSOSSDK::RetryConnect[Check Point][is_reconnect is true][Retry connect success]");
        result = YSOS_ERROR_SUCCESS;
        break;
      }

      YSOS_LOG_DEBUG("YSOSSDK::RetryConnect[Check Point][Retry connect][End]");
    }

    YSOS_LOG_DEBUG("YSOSSDK::RetryConnect[Check Point][8]");

    /// 超出最大重连次数，回调上层错误处理函数
    if (NULL == open_params_ptr_) {
      YSOS_LOG_DEBUG("YSOSSDK::RetryConnect[Fail][ysos_sdk_config_params_ptr_ is NULL]");
      break;
    }

    YSOS_LOG_DEBUG("YSOSSDK::RetryConnect[Check Point][9]");

//     if (NULL != ysos_sdk_config_params_ptr_->error_handler_) {
//       YSOS_LOG_DEBUG("YSOSSDK::RetryConnect[Check Point][Call HandleErrorCallback()]");
//       ysos_sdk_config_params_ptr_->error_handler_(retry_connect_params_ptr->first.c_str(), retry_connect_params_ptr->second.c_str());
//     }

    YSOS_LOG_DEBUG("YSOSSDK::RetryConnect[Check Point][End]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  YSOS_LOG_DEBUG("YSOSSDK::RetryConnect[Exit]");

  return;
}

bool YSOSSDK::InitYSOSSDKLogger(const char* log_file_path_ptr) {
  assert(NULL != log_file_path_ptr);
  if (true != (log::InitLogger(log_file_path_ptr))) {
    return false;
  }

  if (false == logger_.valid()) {
    logger_ = GetUtility()->GetLogger("ysos.sdk");
  }

  return true;
}

}
