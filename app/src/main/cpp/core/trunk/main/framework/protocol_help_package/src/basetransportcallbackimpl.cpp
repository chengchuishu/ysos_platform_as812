/*
# basetransportcallbackimpl.cpp
# Definition of basetransportcallbackimpl
# Created on: 2016-06-03 16:43:58
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20160912, created by JinChengZhe
*/

/// Private Headers
#include "../../../protect/include/protocol_help_package/basetransportcallbackimpl.h"
/// Platform Headers
#include "../../../protect/include/protocol_help_package/commonconnectionimpl.h"
#include "../../../public/include/core_help_package/utility.h"
#include "../../../public/include/os_hal_package/bufferutility.h"

namespace ysos {

BaseTransportCallbackImpl::BaseTransportCallbackImpl(const std::string &strClassName /* =BaseTransportCallbackImpl */)
  : BaseInterfaceImpl(strClassName),
    mode_(CLIENT),
    address_("127.0.0.1"),
    port_(0),
    enable_read_(true),
    enable_write_(true),
    connection_ptr_(NULL) {
  logger_ = GetUtility()->GetLogger("ysos.sdk");
}

BaseTransportCallbackImpl::~BaseTransportCallbackImpl() {
  connection_ptr_ = NULL;
}

UINT32 BaseTransportCallbackImpl::GetMode(void) {
  return mode_;
}

void BaseTransportCallbackImpl::SetMode(const UINT32 mode) {
  mode_ = mode;
}

std::string BaseTransportCallbackImpl::GetAddress(void) {
  return address_;
}

void BaseTransportCallbackImpl::SetAddress(const std::string &address) {
  address_ = address;
}

short BaseTransportCallbackImpl::GetPort(void) {
  return port_;
}

void BaseTransportCallbackImpl::SetPort(const short port) {
  port_ = port;
}

bool BaseTransportCallbackImpl::GetEnableRead(void) {
  return enable_read_;
}

void BaseTransportCallbackImpl::SetEnableRead(const bool enable_read) {
  enable_read_ = enable_read;
}

bool BaseTransportCallbackImpl::GetEnableWrite(void) {
  return enable_write_;
}

void BaseTransportCallbackImpl::SetEnableWrite(const bool enable_write) {
  enable_write_ = enable_write;
}

ConnectionInterface *BaseTransportCallbackImpl::GetConnectionPtr(void) {
  return connection_ptr_;
}

void BaseTransportCallbackImpl::SetConnectionPtr(ConnectionInterface *connection_ptr) {
  connection_ptr_ = dynamic_cast<CommonConnectionImpl*>(connection_ptr);
}

void BaseTransportCallbackImpl::HandleAccept(const boost::system::error_code& error_code, const TransportInterfacePtr& transport_ptr) {
  YSOS_LOG_DEBUG("BaseTransportCallbackImpl::HandleAccept[Enter]");

  if (NULL == connection_ptr_) {
    YSOS_LOG_DEBUG("BaseTransportCallbackImpl::HandleAccept[Fail][0]");
    return;
  }

  connection_ptr_->HandleAccept(error_code, transport_ptr);

  YSOS_LOG_DEBUG("BaseTransportCallbackImpl::HandleAccept[Exit]");

  return;
}

void BaseTransportCallbackImpl::HandleConnect(const boost::system::error_code& error_code, const TransportInterfacePtr& transport_ptr) {
  YSOS_LOG_DEBUG("BaseTransportCallbackImpl::HandleConnect[Enter]");

  if (NULL == connection_ptr_) {
    YSOS_LOG_DEBUG("BaseTransportCallbackImpl::HandleConnect[Fail][0]");
    return;
  }

  connection_ptr_->HandleConnect(error_code, transport_ptr);

  YSOS_LOG_DEBUG("BaseTransportCallbackImpl::HandleConnect[Exit]");

  return;
}

void BaseTransportCallbackImpl::HandleDisconnect(const boost::system::error_code& error_code, const TransportInterfacePtr& transport_ptr) {
  YSOS_LOG_DEBUG("BaseTransportCallbackImpl::HandleDisconnect[Enter]");

  if (NULL == connection_ptr_) {
    YSOS_LOG_DEBUG("BaseTransportCallbackImpl::HandleDisconnect[Fail][0]");
    return;
  }

  connection_ptr_->HandleDisconnect(error_code, transport_ptr);

  YSOS_LOG_DEBUG("BaseTransportCallbackImpl::HandleDisconnect[Exit]");

  return;
}

void BaseTransportCallbackImpl::HandleRead(const boost::system::error_code& error_code, const std::size_t length, const BufferInterfacePtr& buffer_ptr, const TransportInterfacePtr& transport_ptr) {
  YSOS_LOG_DEBUG("BaseTransportCallbackImpl::HandleRead[Enter]");

  if (NULL == connection_ptr_) {
    YSOS_LOG_DEBUG("BaseTransportCallbackImpl::HandleRead[Fail][0]");
    return;
  }

  connection_ptr_->HandleRead(error_code, length, buffer_ptr, transport_ptr);

  YSOS_LOG_DEBUG("BaseTransportCallbackImpl::HandleRead[Exit]");

  return;
}

void BaseTransportCallbackImpl::HandleWrite(const boost::system::error_code& error_code, const std::size_t length, const BufferInterfacePtr& buffer_ptr, const TransportInterfacePtr& transport_ptr) {
  YSOS_LOG_DEBUG("BaseTransportCallbackImpl::HandleWrite[Enter]");

  if (NULL == connection_ptr_) {
    YSOS_LOG_DEBUG("BaseTransportCallbackImpl::HandleWrite[Fail][0]");
    return;
  }

  connection_ptr_->HandleWrite(error_code, length, buffer_ptr, transport_ptr);

  YSOS_LOG_DEBUG("BaseTransportCallbackImpl::HandleWrite[Exit]");

  return;
}

void BaseTransportCallbackImpl::HandleError(const boost::system::error_code& error_code, const UINT32 callback_type, const TransportInterfacePtr& transport_ptr) {
  YSOS_LOG_DEBUG("BaseTransportCallbackImpl::HandleError[Enter]");

  if (NULL == connection_ptr_) {
    YSOS_LOG_DEBUG("BaseTransportCallbackImpl::HandleError[Fail][0]");
    return;
  }

  connection_ptr_->HandleError(error_code, callback_type, transport_ptr);

  YSOS_LOG_DEBUG("BaseTransportCallbackImpl::HandleError[Exit]");

  return;
}

}
