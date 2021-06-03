/**
 *@file BaseIoInfoImpl.cpp
 *@brief Definition of BaseIoInfoImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// ysos private headers //  NOLINT
#include "../../../protect/include/sys_framework_package/baseioinfoimpl.h"
/// Ysos Headers //  NOLINT
#include "../../../public/include/sys_interface_package/ysos_enum.h"
#include "../../../protect/include/os_hal_package/msgimpl.h"

namespace ysos {
BaseIoInfoImpl::BaseIoInfoImpl(const std::string &strClassName): BaseInterfaceImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.framework.strategy");
}

BaseIoInfoImpl::~BaseIoInfoImpl() {
  transition_info_ptr_ = NULL;
  callback_ptr_ = NULL;
}

bool BaseIoInfoImpl::IsMatched(BufferInterfacePtr input_buffer, BufferInterfacePtr ouput_buffer, void *context_ptr) {
  is_mached_ = true;

  if (is_mached_) {
    Wrap(input_buffer, ouput_buffer, NULL);
  }

  return is_mached_;
}

int BaseIoInfoImpl::GetProperty(int type_id, void *type) {
  switch (type_id) {
  case IS_MATCHED: {
    bool *is_mached = reinterpret_cast<bool*>(type);
    if (NULL != is_mached) {
      *is_mached = is_mached_;
    }
    break;
  }
  default:
    return BaseInterfaceImpl::SetProperty(type_id, type);
  }
  return YSOS_ERROR_SUCCESS;
}

int BaseIoInfoImpl::SetProperty(int type_id, void *type) {
  switch (type_id) {
  case PROP_IOINFO_CALLBACK: {
    CallbackInterfacePtr *callback_ptr = reinterpret_cast<CallbackInterfacePtr*>(type);
    if (NULL != callback_ptr) {
      callback_ptr_ = *callback_ptr;
    }
    break;
  }
  case TRANSITION_INFO_ADD: {
    TransitionInfoInterface *transition_info_ptr = reinterpret_cast<TransitionInfoInterface*>(type);
    //if (NULL != transition_info_ptr) {
    transition_info_ptr_ = transition_info_ptr;
    //}
    break;
  }
  case TRANSITION_INFO_REMOVE: {
    transition_info_ptr_ = NULL;
    break;
  }
  case EXPRESSION: {
    std::string *expression = reinterpret_cast<std::string*>(type);
    if (NULL != expression) {
      condition_expression_ = *expression;
    }
    break;
  }
  default:
    return BaseInterfaceImpl::SetProperty(type_id, type);
  }
  return YSOS_ERROR_SUCCESS;
}

int BaseIoInfoImpl::Wrap(BufferInterfacePtr input_buffer, BufferInterfacePtr ouput_buffer, void *context_ptr) {
  if (NULL == input_buffer || NULL == transition_info_ptr_) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  YSOS_LOG_DEBUG("IOinfo Wrap: " << this->GetName());
  // head
  uint32_t message_id = transition_info_ptr_->GetMessageID(NULL, NULL, this);
  if (0 == message_id) {
    YSOS_LOG_DEBUG("Get MessageHeader Failed");
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  // message
  MsgInterfacePtr message_ptr = MsgInterfacePtr(new MsgImpl(message_id, input_buffer));
  assert(NULL != message_ptr);
  // add to message queue
  transition_info_ptr_->AddMessageToQueue(message_ptr);

  return YSOS_ERROR_SUCCESS;
}

int BaseIoInfoImpl::UnWrap(BufferInterfacePtr input_buffer, BufferInterfacePtr ouput_buffer, void *context_ptr) {
  return YSOS_ERROR_SUCCESS;
}
}
