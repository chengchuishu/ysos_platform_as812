/*
# commonconnectioncallbackimpl.cpp
# Definition of commonconnectioncallbackimpl
# Created on: 2016-06-03 16:43:58
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20160912, created by JinChengZhe
*/

/// Private Headers
#include "../../../protect/include/protocol_help_package/commonconnectioncallbackimpl.h"
/// ThirdParty Headers
#include <boost/thread.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
/// Platform Headers
#include "../../../protect/include/protocol_help_package/basetransportcallbackimpl.h"
#include "../../../public/include/core_help_package/utility.h"
#include "../../../public/include/os_hal_package/bufferutility.h"
#include "../../../protect/include/protocol_help_package/platformprotocolimpl.h"


namespace ysos {

static const char* g_error_param_error_code = "error_code";
static const char* g_error_param_callback_type = "callback_type";
static const char* g_error_param_description = "description";

bool CommonConnectionCallbackImpl::ConnectionCallbackErrorParam::ToString(std::string& to_string) {
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

bool CommonConnectionCallbackImpl::ConnectionCallbackErrorParam::FromString(const std::string& from_string) {
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

CommonConnectionCallbackImpl::CommonConnectionCallbackImpl(const std::string &strClassName /* =CommonConnectionCallbackImpl */)
  : BaseInterfaceImpl(strClassName),
    mode_(CLIENT),
    address_("127.0.0.1"),
    port_(0),
    buffer_length_(0),
    buffer_number_(0),
    enable_wrap_(true),
    enable_read_(true),
    enable_write_(true),
    transport_callback_ptr_(NULL) {
  logger_ = GetUtility()->GetLogger("ysos.sdk");
  if (YSOS_ERROR_SUCCESS != Initialize()) {
    YSOS_LOG_DEBUG("CommonConnectionCallbackImpl::Initialize[Fail][The function[Initialize()] is failed]");
  }
}

CommonConnectionCallbackImpl::~CommonConnectionCallbackImpl() {
  if (NULL !=  transport_callback_ptr_) {
    transport_callback_ptr_ = NULL;
  }
}

int CommonConnectionCallbackImpl::Initialize(void *param) {
  YSOS_LOG_DEBUG("CommonConnectionCallbackImpl::Initialize[Enter]");

  if (NULL == transport_callback_ptr_) {
    YSOS_LOG_DEBUG("CommonConnectionCallbackImpl::Initialize[Check Point][0]");

    transport_callback_ptr_ = TransportCallbackInterfacePtr(BaseTransportCallbackImpl::CreateInstance());
    if (NULL == transport_callback_ptr_) {
      YSOS_LOG_DEBUG("CommonConnectionCallbackImpl::Initialize[Fail][0]");
      return YSOS_ERROR_FAILED;
    }
  }

  YSOS_LOG_DEBUG("CommonConnectionCallbackImpl::Initialize[Exit]");

  return YSOS_ERROR_SUCCESS;
}

UINT32 CommonConnectionCallbackImpl::GetMode(void) {
  return mode_;
}

void CommonConnectionCallbackImpl::SetMode(const UINT32 mode) {
  mode_ = mode;

  transport_callback_ptr_->SetMode(SERVER == mode_ ? PROP_SERVER : PROP_CLIENT);

  if (SERVERTERMINAL == mode_) {
    transport_callback_ptr_->SetMode(PROP_SERVER_TERMINAL);
  }

  return;
}

std::string CommonConnectionCallbackImpl::GetAddress(void) {
  return address_;
}

void CommonConnectionCallbackImpl::SetAddress(const std::string &address) {
  address_ = address;

  transport_callback_ptr_->SetAddress(address_);

  return;
}

short CommonConnectionCallbackImpl::GetPort(void) {
  return port_;
}

void CommonConnectionCallbackImpl::SetPort(const short port) {
  port_ = port;

  transport_callback_ptr_->SetPort(port_);

  return;
}

bool CommonConnectionCallbackImpl::GetEnableWrap(void) {
  return enable_wrap_;
}

void CommonConnectionCallbackImpl::SetEnableWrap(const bool enable_wrap) {
  enable_wrap_ = enable_wrap;
}

bool CommonConnectionCallbackImpl::GetEnableRead(void) {
  return enable_read_;
}

void CommonConnectionCallbackImpl::SetEnableRead(const bool enable_read) {
  enable_read_ = enable_read;

  transport_callback_ptr_->SetEnableRead(enable_read_);

  return;
}

bool CommonConnectionCallbackImpl::GetEnableWrite(void) {
  return enable_write_;
}

void CommonConnectionCallbackImpl::SetEnableWrite(const bool enable_write) {
  enable_write_ = enable_write;

  transport_callback_ptr_->SetEnableWrite(enable_write_);

  return;
}

TransportCallbackInterfacePtr CommonConnectionCallbackImpl::GetTransportCallbackPtr(void) {
  return transport_callback_ptr_;
}

void CommonConnectionCallbackImpl::SetTransportCallbackPtr(const TransportCallbackInterfacePtr& transport_callback_ptr) {
  transport_callback_ptr_ = transport_callback_ptr;

  return;
}

unsigned int CommonConnectionCallbackImpl::GetBufferLength(void) {
  return buffer_length_;
}

void CommonConnectionCallbackImpl::SetBufferLength(unsigned int buffer_length) {
  buffer_length_ = buffer_length;

  return;
}

unsigned int CommonConnectionCallbackImpl::GetBufferNumber(void) {
  return buffer_number_;
}

void CommonConnectionCallbackImpl::SetBufferNumber(unsigned int buffer_number) {
  buffer_number_ = buffer_number;

  return;
}

void CommonConnectionCallbackImpl::HandleAccept(const boost::system::error_code& error_code, const ConnectionInterfacePtr& connection_ptr) {
  YSOS_LOG_DEBUG("CommonConnectionCallbackImpl::HandleAccept()");
}

void CommonConnectionCallbackImpl::HandleConnect(const boost::system::error_code& error_code, const ConnectionInterfacePtr& connection_ptr) {
  YSOS_LOG_DEBUG("CommonConnectionCallbackImpl::HandleConnect()");
}

void CommonConnectionCallbackImpl::HandleDisconnect(const boost::system::error_code& error_code, const ConnectionInterfacePtr& connection_ptr) {
  YSOS_LOG_DEBUG("CommonConnectionCallbackImpl::HandleDisconnect()");
}

void CommonConnectionCallbackImpl::HandleRead(const BufferInterfacePtr& buffer_ptr, const ConnectionInterfacePtr& connection_ptr) {
  YSOS_LOG_DEBUG("CommonConnectionCallbackImpl::HandleRead()");
}

void CommonConnectionCallbackImpl::HandleWrite(const BufferInterfacePtr& buffer_ptr, const ConnectionInterfacePtr& connection_ptr) {
  YSOS_LOG_DEBUG("CommonConnectionCallbackImpl::HandleWrite()");
}

void CommonConnectionCallbackImpl::HandleError(const boost::system::error_code& error_code, const UINT32 callback_type, const ConnectionInterfacePtr& connection_ptr) {
  YSOS_LOG_DEBUG("CommonConnectionCallbackImpl::HandleError()");
}

}
