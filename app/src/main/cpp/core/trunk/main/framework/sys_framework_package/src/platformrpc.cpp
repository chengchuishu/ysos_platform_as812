/*
# platformrpc.cpp
# Definition of PlatformRPC
# Created on: 2016-09-29 09:23:33
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20160929, created by JinChengZhe
*/

/// Private Headers
#include "../../../protect/include/sys_framework_package/platformrpc.h"
/// Platform Headers
#include "../../../public/include/sys_interface_package/error.h"
#include "../../../protect/include/protocol_help_package/jsonrpc.h"
#include "../../../protect/include/protocol_help_package/baserpccallbackimpl.h"
#include "../../../public/include/core_help_package/utility.h"
#include "../../../public/include/sys_interface_package/ysos_enum.h"

namespace ysos {
/**
*@brief Loger的静态声明  // NOLINT
*/
namespace ysos_platform_rpc_loger {
static log4cplus::Logger logger_;
}

using namespace ysos_platform_rpc_loger;

PlatformRPC::PlatformRPC(const std::string &strClassName /* =PlatformRPC */)
  : BaseInterfaceImpl(strClassName),
    json_rpc_ptr_(NULL),
    base_rpc_callback_impl_ptr_(NULL),
    running_status_(IS_INITIAL) {
  logger_ = GetUtility()->GetLogger("ysos.sdk");
}

PlatformRPC::~PlatformRPC() {
  if (NULL != json_rpc_ptr_) {
    delete json_rpc_ptr_;
    json_rpc_ptr_ = NULL;
  }

  if (NULL != base_rpc_callback_impl_ptr_) {
    base_rpc_callback_impl_ptr_ = NULL;
  }

  running_status_ = IS_INITIAL;
}

int PlatformRPC::Open(RPCCallbackInterface* params, std::string* session_id_ptr) {
  YSOS_LOG_DEBUG("PlatformRPC::Open[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {
    /// 判断模块是否已开启
    if (IS_RUNNING == GetRunningStatus()) {
      YSOS_LOG_DEBUG("JsonRPC::Open[It is already running]");
      result = YSOS_ERROR_SUCCESS;
      break;
    }

    YSOS_LOG_DEBUG("PlatformRPC::Open[Check Point][0]");

    /// 重置运行状态
    SetRunningStatus(IS_INITIAL);

    if (NULL == params) {
      YSOS_LOG_DEBUG("PlatformRPC::Open[Fail][params is NULL]");
      break;
    }

    YSOS_LOG_DEBUG("PlatformRPC::Open[Check Point][1]");

    base_rpc_callback_impl_ptr_ = dynamic_cast<BaseRPCCallbackImpl*>(params);

    YSOS_LOG_DEBUG("PlatformRPC::Open[Check Point][2]");

    /// 客户端模式下，判断是否传入状态机配置文件
#if (!_CERTUSNET)
    if (PROP_CLIENT == base_rpc_callback_impl_ptr_->GetMode() &&
        true == base_rpc_callback_impl_ptr_->GetStrategyName().empty()) {
      YSOS_LOG_DEBUG("PlatformRPC::Open[Fail][strategy file name is empty]");
      break;
    }
#endif

    YSOS_LOG_DEBUG("PlatformRPC::Open[Check Point][3]");

    if (NULL != json_rpc_ptr_) {
      YSOS_LOG_DEBUG("PlatformRPC::Open[Fail][json_rpc_ptr_ is NULL]");
      break;
    }

    YSOS_LOG_DEBUG("PlatformRPC::Open[Check Point][4]");

    json_rpc_ptr_ = JsonRPC::CreateInstance() ;
    if (NULL == json_rpc_ptr_) {
      YSOS_LOG_DEBUG("PlatformRPC::Open[Fail][json_rpc_ptr_ is NULL]");
      break;
    }

    YSOS_LOG_DEBUG("PlatformRPC::Open[Check Point][5]");

    if (YSOS_ERROR_SUCCESS != json_rpc_ptr_->Open(base_rpc_callback_impl_ptr_, session_id_ptr)) {
      YSOS_LOG_DEBUG("PlatformRPC::Open[Fail]The function[json_rpc_ptr_->Open()] is failed");
      break;
    }

    /// 设置运行状态
    SetRunningStatus(IS_RUNNING);

    YSOS_LOG_DEBUG("PlatformRPC::Open[Check Point][End]");

    result = YSOS_ERROR_SUCCESS;

  } while (0);

  YSOS_LOG_DEBUG("PlatformRPC::Open[Exit]");

  return result;
}

int PlatformRPC::Close(const std::string* session_id_ptr) {
  YSOS_LOG_DEBUG("PlatformRPC::Close[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {
    /// 判断模块是否已关闭
    if (IS_STOPPED == GetRunningStatus()) {
      YSOS_LOG_DEBUG("PlatformRPC::Close[It is already stopped]");
      result = YSOS_ERROR_SUCCESS;
      break;
    }

    YSOS_LOG_DEBUG("PlatformRPC::Close[Check Point][0]");

    /// 设置运行状态
    SetRunningStatus(IS_STOPPED);

    if (NULL == json_rpc_ptr_) {
      YSOS_LOG_DEBUG("PlatformRPC::Close[Fail][json_rpc_ptr_ is NULL]");
      break;
    }

    YSOS_LOG_DEBUG("PlatformRPC::Close[Check Point][1]");

    if (YSOS_ERROR_SUCCESS != json_rpc_ptr_->Close(session_id_ptr)) {
      YSOS_LOG_DEBUG("PlatformRPC::Close[Fail][1]");
      break;
    }

    YSOS_LOG_DEBUG("PlatformRPC::Close[Check Point][2]");

    if (NULL != json_rpc_ptr_) {
      YSOS_LOG_DEBUG("PlatformRPC::Close[Check Point][json_rpc_ptr_ is not NULL]");
      delete json_rpc_ptr_;
      json_rpc_ptr_ = NULL;
    }

    YSOS_LOG_DEBUG("PlatformRPC::Close[Check Point][3]");

    if (NULL != base_rpc_callback_impl_ptr_) {
      YSOS_LOG_DEBUG("PlatformRPC::Close[Check Point][base_rpc_callback_impl_ptr_ is not NULL]");
      base_rpc_callback_impl_ptr_ = NULL;
    }

    YSOS_LOG_DEBUG("PlatformRPC::Close[Check Point][End]");

    result = YSOS_ERROR_SUCCESS;

  } while (0);

  YSOS_LOG_DEBUG("PlatformRPC::Close[Exit]");

  return result;
}

int PlatformRPC::DoService(const std::string& session_id, const std::string& service_name, const std::string& service_param) {
  YSOS_LOG_DEBUG("PlatformRPC::DoService[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {
    /// 判断模块是否已开启
    if (IS_RUNNING != GetRunningStatus()) {
      YSOS_LOG_DEBUG("PlatformRPC::DoService[It is not running]");
      result = YSOS_ERROR_INVALID_OPERATION;
      break;
    }

    YSOS_LOG_DEBUG("PlatformRPC::DoService[Check Point][0]");

    if (NULL == json_rpc_ptr_) {
      YSOS_LOG_DEBUG("PlatformRPC::DoService[Fail][json_rpc_ptr_ is NULL]");
      break;
    }

    YSOS_LOG_DEBUG("PlatformRPC::DoService[Check Point][1]");

    if (YSOS_ERROR_SUCCESS != json_rpc_ptr_->DoService(session_id, service_name, service_param)) {
      YSOS_LOG_DEBUG("PlatformRPC::DoService[Fail]The function[json_rpc_ptr_->DoService()] is failed");
      break;
    }

    YSOS_LOG_DEBUG("PlatformRPC::DoService[Check Point][End]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  YSOS_LOG_DEBUG("PlatformRPC::DoService[Exit]");

  return YSOS_ERROR_SUCCESS;
}

int PlatformRPC::OnDispatchMessage(const std::string& session_id, const std::string& service_name, const std::string& result) {
  YSOS_LOG_DEBUG("PlatformRPC::OnDispatchMessage[Enter]");

  int return_value = YSOS_ERROR_FAILED;

  do {
    /// 判断模块是否已开启
    if (IS_RUNNING != GetRunningStatus()) {
      YSOS_LOG_DEBUG("PlatformRPC::OnDispatchMessage[It is not running]");
      return_value = YSOS_ERROR_INVALID_OPERATION;
      break;
    }

    YSOS_LOG_DEBUG("PlatformRPC::OnDispatchMessage[Check Point][0]");

    if (NULL == json_rpc_ptr_) {
      YSOS_LOG_DEBUG("PlatformRPC::OnDispatchMessage[Fail][json_rpc_ptr_ is NULL]");
      break;
    }

    YSOS_LOG_DEBUG("PlatformRPC::OnDispatchMessage[Check Point][1]");

    if (YSOS_ERROR_SUCCESS != json_rpc_ptr_->OnDispatchMessage(session_id, service_name, result)) {
      YSOS_LOG_DEBUG("PlatformRPC::OnDispatchMessage[Fail]The function[json_rpc_ptr_->OnDispatchMessage()] is failed");
      break;
    }

    YSOS_LOG_DEBUG("PlatformRPC::OnDispatchMessage[Check Point][End]");

    return_value = YSOS_ERROR_SUCCESS;
  } while (0);

  YSOS_LOG_DEBUG("PlatformRPC::OnDispatchMessage[Exit]");

  return YSOS_ERROR_SUCCESS;
}

}
