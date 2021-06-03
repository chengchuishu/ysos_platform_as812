/**
  *@file BaseTransitionCallbackImpl.cpp
  *@brief Transition Callback
  *@version 1.0
  *@author Donghongqian
  *@date Created on: 2016-05-31 18:03:27
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  */

/// Private Header //  NOLINT
#include "../../../protect/include/sys_framework_package/basetransitioncallbackimpl.h"
/// Ysos Headers //  NOLINT
#include "../../../public/include/sys_interface_package/moduleinterface.h"
#include "../../../public/include/core_help_package/utility.h"
#include "../../../public/include/os_hal_package/bufferutility.h"
#include "../../../public/include/core_help_package/jsonutility.h"
#include "../../../protect/include/sys_framework_package/platforminfo.h"
/// Private Headers //  NOLINT
#include "../../../public/include/sys_interface_package/ysos_enum.h"

namespace ysos {

BaseTransitionCallbackImpl::BaseTransitionCallbackImpl(const std::string &strClassName /* =BaseTransitionCallbackImpl */): BaseStrategyServiceCallbackImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.framework.transition");
  status_event_msg_match_ = false;
  normal_data_msg_match_ = true;
}

BaseTransitionCallbackImpl::~BaseTransitionCallbackImpl() {

}

int BaseTransitionCallbackImpl::Initialized(const std::string &key, const std::string &value) {
  if ("is_status_event" == key) {
    status_event_msg_match_ = value == "true" ? true : false;
    YSOS_LOG_DEBUG("is_status_event set to " << value << " | " << logic_name_);
  } else if ("is_normal_data" == key) {
    normal_data_msg_match_ = value == "true" ? true : false;
  } else {
    return BaseStrategyServiceCallbackImpl::Initialized(key, value);
  }

  return YSOS_ERROR_SUCCESS;
}

bool BaseTransitionCallbackImpl::AddMessageToQueue(BufferInterfacePtr input_buffer, bool is_normal_data, TransitionContext *context_ptr) {
  assert(NULL != context_ptr);
//   MessageHeadPtr msg_header = GetMessageHeader(context_ptr);
//   if (NULL == msg_header) {
//     YSOS_LOG_ERROR("get message header failed: " << context_ptr->transition_ptr->Name << " | " << context_ptr->message_type);
//     return false;
//   }
//
//   // body
//   MessageDataPtr  message_data = MessageDataPtr(new MessageData());
//   if (NULL == message_data) {
//     YSOS_LOG_ERROR("message data is null: " << context_ptr->transition_ptr->Name << " | " << context_ptr->message_type);
//     return false;
//   }
//   message_data->message_data_ = GetBufferUtility()->CloneBuffer(input_buffer); // GetBufferUtility()->CreateBuffer((char*)(GetBufferUtility()->GetBufferData(input_buffer)));
  uint32_t msg_type = GetMessageID(context_ptr);
  if (0 == msg_type) {
    YSOS_LOG_ERROR("get msg type failed: " << context_ptr->module_ptr->GetName());
    return false;
  }

  // message
  /*  MsgInterfacePtr message_ptr = GetBufferUtility()->CreateMessage(msg_header, message_data);*/
  MsgInterfacePtr message_ptr = MsgInterfacePtr(new MsgImpl(msg_type, input_buffer));
  assert(NULL != message_ptr);
  // add to message queue
  assert(NULL != context_ptr->transition_ptr);
  int ret = context_ptr->transition_ptr->AddMessageToQueue(message_ptr);
  if (0 != ret) {
    YSOS_LOG_ERROR("add message to queue failed: " << context_ptr->transition_ptr->GetName() << " | " << context_ptr->message_type);
  }

  return YSOS_ERROR_SUCCESS == ret;
}

int BaseTransitionCallbackImpl::Callback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, void *context) {
  if (NULL == input_buffer || NULL == context) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  int ret = YSOS_ERROR_SUCCESS;
  TransitionContext *transition_context = reinterpret_cast<TransitionContext*>(context);
  if (status_event_msg_match_ && IsStatusEventMatched(input_buffer, transition_context)) {
    ret = AddMessageToQueue(input_buffer, false, transition_context);
    if (YSOS_ERROR_SUCCESS != ret) {
      YSOS_LOG_ERROR("Send Status Event Message failed: " << transition_context->transition_ptr->GetName() << " | " << transition_context->message_type);
    }
    return ret;
  }

  if (normal_data_msg_match_ && IsMatched(input_buffer, transition_context)) {
    bool is_success = AddMessageToQueue(input_buffer, false, transition_context);
    if (!is_success) {
      YSOS_LOG_ERROR("Send Normal Data Message failed: " << transition_context->transition_ptr->GetName() << " | " << transition_context->message_type);
    }
  }

  return ret;
}

bool BaseTransitionCallbackImpl::IsStatusEventMatched(BufferInterfacePtr input_buffer, TransitionContext *context_ptr) {
  if (NULL == input_buffer || NULL == context_ptr) {
    YSOS_LOG_ERROR("invalid status event" << context_ptr->transition_ptr->GetName() << " | " << context_ptr->message_type);
    return false;
  }

  uint8_t* data_ptr = GetBufferUtility()->GetBufferData(input_buffer);
  if (NULL == data_ptr) {
    YSOS_LOG_ERROR("status event message is null" << context_ptr->transition_ptr->GetName() << " | " << context_ptr->message_type);
    return false;
  }

  JsonValue json_value_object;
  if (YSOS_ERROR_SUCCESS != GetJsonUtil()->JsonObjectFromString((const char*)data_ptr, json_value_object)) {
    YSOS_LOG_DEBUG("status event message is invalid" << context_ptr->transition_ptr->GetName() << " | " << context_ptr->message_type);
    return false;
  }

  std::string type = GetJsonUtil()->GetTextElement("type", json_value_object);
  if (type.empty() || "status_event" != type) {
    YSOS_LOG_ERROR("status message is invalid: " << type << context_ptr->transition_ptr->GetName() << " | " << context_ptr->message_type);
    return false;
  }

  std::string status_code = GetJsonUtil()->GetTextElement("status_event_code", json_value_object);
  std::string service_name = GetJsonUtil()->GetTextElement("service_name", json_value_object);

  return IsStatusEventMatched(status_code, service_name, input_buffer, context_ptr);
}

bool BaseTransitionCallbackImpl::IsStatusEventMatched(std::string &status_code, std::string &service_name, BufferInterfacePtr input_buffer, TransitionContext *context_ptr) {
  return true;
}

bool BaseTransitionCallbackImpl::IsMatched(BufferInterfacePtr input_buffer, TransitionContext *context_ptr) {
  return true;
}

uint32_t BaseTransitionCallbackImpl::GetMessageID(TransitionContext *context_ptr) {
  assert(NULL!=context_ptr && NULL!=context_ptr->module_ptr);

  return GetPlatformInfo()->GetMessageIdByServiceName(context_ptr->module_ptr->GetName());
}

int BaseTransitionCallbackImpl::SetSwitchFlag(const std::string &module_name, TransitionContext *context_ptr) {
  assert(NULL!=context_ptr && NULL!=context_ptr->transition_ptr);

  return context_ptr->transition_ptr->SetProperty(PROP_SWITCH, context_ptr);
}
}
