/*
# baserpccallbackimpl.cpp
# Definition of BaseRPCCallbackImpl
# Created on: 2016-09-29 09:28:02
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20160929, created by JinChengZhe
*/

/// Private Headers
#include "../../../protect/include/protocol_help_package/baserpccallbackimpl.h"
/// Platform Headers
#include "../../../public/include/core_help_package/utility.h"
#include "../../../public/include/os_hal_package/bufferutility.h"
#include "../../../protect/include/protocol_help_package/platformprotocolimpl.h"

namespace ysos {

static const char* g_error_param_error_code = "error_code";
static const char* g_error_param_callback_type = "callback_type";
static const char* g_error_param_description = "description";

bool BaseRPCCallbackImpl::RPCCallbackErrorParam::ToString(std::string& to_string) {
  bool result = false;

  do {
    JsonValue json_value_error_param;
    PlatformProtocolImpl::PlatformProtocolParamPair platform_protocol_param_pair;

    platform_protocol_param_pair.first = g_error_param_error_code;
    platform_protocol_param_pair.second = GetUtility()->ConvertToString(error_code);
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_error_param);

    platform_protocol_param_pair.first = g_error_param_callback_type;
    platform_protocol_param_pair.second = callback_type;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_error_param);

    platform_protocol_param_pair.first = g_error_param_description;
    platform_protocol_param_pair.second = description;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_error_param);

    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonValueToString(json_value_error_param, to_string)) {
      break;
    }

    result = true;
  } while (0);

  return result;
}

bool BaseRPCCallbackImpl::RPCCallbackErrorParam::FromString(const std::string& from_string) {
  bool result = false;

  do {
    JsonValue json_value_result;
    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonObjectFromString(from_string, json_value_result)) {
      break;
    }

    /// error_code
    if (
      true == json_value_result.isMember(g_error_param_error_code) &&
      true == json_value_result[g_error_param_error_code].isString()
    ) {
      uint32_t i = 0;
      error_code = GetUtility()->ConvertFromString(json_value_result[g_error_param_error_code].asString(), i);
    }

    /// callback_type
    if (
      true == json_value_result.isMember(g_error_param_callback_type) &&
      true == json_value_result[g_error_param_callback_type].isString()
    ) {
      callback_type = json_value_result[g_error_param_callback_type].asString();
    }

    /// description
    if (
      true == json_value_result.isMember(g_error_param_description) &&
      true == json_value_result[g_error_param_description].isString()
    ) {
      description = json_value_result[g_error_param_description].asString();
    }

    result = true;
  } while (0);

  return result;
}

BaseRPCCallbackImpl::BaseRPCCallbackImpl(const std::string &strClassName /* =BaseRPCCallbackImpl */)
  : BaseInterfaceImpl(strClassName),
    mode_(CLIENT),
    address_(BASE_RPC_CALLBACK_ADDRESS),
    port_(BASE_RPC_CALLBACK_PORT),
    max_reconnect_amount_(BASE_RPC_CALLBACK_MAX_RECONNECT_AMOUNT),
    reconnect_time_(BASE_RPC_CALLBACK_RECONNECT_TIME),
    buffer_length_(BASE_RPC_CALLBACK_BUFFER_LENGTH),
    application_name_(""),
    strategy_name_("") {
  logger_ = GetUtility()->GetLogger("ysos.sdk");
}

BaseRPCCallbackImpl::~BaseRPCCallbackImpl() {
}

UINT32 BaseRPCCallbackImpl::GetMode(void) {
  YSOS_LOG_DEBUG("BaseRPCCallbackImpl::GetMode[mode][::]"<<mode_);
  return mode_;
}

void BaseRPCCallbackImpl::SetMode(const UINT32 mode) {
  YSOS_LOG_DEBUG("BaseRPCCallbackImpl::SetMode[mode][::]"<< mode);
  mode_ = mode;
}

std::string BaseRPCCallbackImpl::GetAddress(void) {
  return address_;
}

void BaseRPCCallbackImpl::SetAddress(const std::string& address) {
  address_ = address;
}

UINT32 BaseRPCCallbackImpl::GetPort(void) {
  return port_;
}

void BaseRPCCallbackImpl::SetPort(const UINT32 port) {
  port_ = port;
}

UINT32 BaseRPCCallbackImpl::GetMaxReconnectAmount(void) {
  return max_reconnect_amount_;
}

void BaseRPCCallbackImpl::SetMaxReconnectAmount(const UINT32 max_reconnect_amount) {
  max_reconnect_amount_ = max_reconnect_amount;
}

UINT32 BaseRPCCallbackImpl::GetReconnectTime(void) {
  return reconnect_time_;
}

void BaseRPCCallbackImpl::SetReconnectTime(const UINT32 reconnect_time) {
  reconnect_time_ = reconnect_time;
}

UINT32 BaseRPCCallbackImpl::GetBufferLength(void) {
  return buffer_length_;
}

void BaseRPCCallbackImpl::SetBufferLength(const UINT32 buffer_length) {
  buffer_length_ = buffer_length;
}

std::string BaseRPCCallbackImpl::GetApplicationName(void) {
  return application_name_;
}

void BaseRPCCallbackImpl::SetAplicationName(const std::string &application_name) {
  application_name_ = application_name;
}

std::string BaseRPCCallbackImpl::GetStrategyName(void) {
  return strategy_name_;
}

void BaseRPCCallbackImpl::SetStrategyName(const std::string &strategy_name) {
  strategy_name_ = strategy_name;
}

void BaseRPCCallbackImpl::SetErrorHandler(const ErrorHandler error_handler) {
  error_handler_ = error_handler;
}

void BaseRPCCallbackImpl::HandleDoService(const std::string& session_id, const std::string& service_name, const std::string& service_param) {
  YSOS_LOG_DEBUG("BaseRPCCallbackImpl::HandleDoService()");
  return;
}

void BaseRPCCallbackImpl::HandleOnDispatchMessage(const std::string& session_id, const std::string& service_name, const std::string& result) {
  YSOS_LOG_DEBUG("BaseRPCCallbackImpl::HandleOnDispatchMessage()");
  return;
}

void BaseRPCCallbackImpl::HandleError(const std::string& session_id, const std::string& error_param) {
  YSOS_LOG_DEBUG("BaseRPCCallbackImpl::HandleError()");
  return;
}

}
