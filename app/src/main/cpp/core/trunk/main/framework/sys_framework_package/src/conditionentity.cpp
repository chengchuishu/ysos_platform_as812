/**
 *@file condition entity.h
 *@brief Definition of all the condition entity
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-06-21 08:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// Platform Headers
#include "../../../protect/include/sys_framework_package/conditionentity.h"
/// Ysos Sdk Headers
#include "../../../public/include/core_help_package/utility.h"
#include "../../../public/include/sys_interface_package/ysos_enum.h"
#include "../../../public/include/os_hal_package/bufferutility.h"
#include "../../../protect/include/sys_framework_package/platforminfo.h"
#include "../../../protect/include/os_hal_package/msgimpl.h"
#include "../../../protect/include/sys_framework_package/manager_declare.h"
#include "../../../protect/include/os_hal_package/datainterfacemanager.h"
#include "../../../public/include/core_help_package/jsonutility.h"

namespace ysos {
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////          Base Condition                         ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BaseCondition::BaseCondition(const std::string &strClassName): BaseInterfaceImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.framework.strategy.callback");
  if_ignore_error_ = true;
  tag_ = "";
}

PropertyTreePtr BaseCondition::ParseString(const std::string &config_str) {
  PropertyTreePtr xml_root = PropertyTreePtr(new PropertyTree());
  if (NULL == xml_root) {
    YSOS_LOG_INFO("allocate property tree failed");
    return NULL;
  }

  int ret = xml_root->ParseString(config_str);
  if (YSOS_ERROR_SUCCESS != ret) {
    YSOS_LOG_INFO("parse string failed: " << config_str);
    return NULL;
  }

  return xml_root;
}

std::string BaseCondition::GetTag(void) {
  return tag_;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////          Event Condition                         ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EventCondition::EventCondition(const std::string &strClassName /* = "EventCondition" */): BaseCondition(strClassName) {

}

EventCondition::~EventCondition() {

}

int EventCondition::FromString(const std::string &config_str) {
  PropertyTreePtr xml_node = ParseString(config_str);
  if (NULL == xml_node) {
    YSOS_LOG_DEBUG("parse string failed");
    return YSOS_ERROR_FAILED;
  }

  if_ignore_error_ = xml_node->GetAttribute("ignore_error") == "true" ? true : false;
  reg_service_event_req_.service_name = xml_node->GetAttribute("service_name");
  reg_service_event_req_.flag = xml_node->GetAttribute("flag");
  tag_ = xml_node->GetAttribute("tag");

  ServiceEventSpecUnit event_spec_uint;
  event_spec_uint.callback_name = xml_node->GetAttribute("callback_name");
  event_spec_uint.event_key = xml_node->GetAttribute("event_name");

  if (event_spec_uint.callback_name.empty() || event_spec_uint.event_key.empty() || reg_service_event_req_.flag.empty()) {
    YSOS_LOG_INFO("parse string failed: " << config_str);
    return YSOS_ERROR_FAILED;
  }
  reg_service_event_req_.service_event_spec_units.push_back(event_spec_uint);

  return YSOS_ERROR_SUCCESS;
}

int EventCondition::DoAction(CallbackInterface *context) {
  assert(NULL != context);
  std::string req_str;
  if (!reg_service_event_req_.ToString(req_str)) {
    YSOS_LOG_ERROR("do event action failed");
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  int ret = context->SetProperty(PROP_REG_SERVICE_EVENT_REQ_WITHOUT_DISPATCH, (void*)req_str.c_str());
  if (YSOS_ERROR_SUCCESS != ret && if_ignore_error_) {
    YSOS_LOG_ERROR("do event action failed: " << req_str);
    return YSOS_ERROR_SKIP;
  }
  YSOS_LOG_DEBUG("do event action success: " << req_str);

  return ret;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////          Ioctl Condition                         ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
IoctlCondition::IoctlCondition(const std::string &strClassName /* = "EventCondition" */): BaseCondition(strClassName) {

}

IoctlCondition::~IoctlCondition() {

}

int IoctlCondition::FromString(const std::string &config_str) {
  PropertyTreePtr xml_node = ParseString(config_str);
  if (NULL == xml_node) {
    YSOS_LOG_DEBUG("parse string failed");
    return YSOS_ERROR_FAILED;
  }

  if_ignore_error_ = xml_node->GetAttribute("ignore_error") == "true" ? true : false;
  ioctl_req_.id = xml_node->GetAttribute("id");
  ioctl_req_.service_name = xml_node->GetAttribute("service_name");
  ioctl_req_.type = xml_node->GetAttribute("type");
  ioctl_value_ = ioctl_req_.value = xml_node->GetAttribute("value");
  tag_ = xml_node->GetAttribute("tag");
  protcol_ = xml_node->GetAttribute("protocol");
  key_ = xml_node->GetAttribute("key");

  if (ioctl_req_.id.empty() || ioctl_req_.service_name.empty()) {
    YSOS_LOG_INFO("parse ioctl failed: " << config_str << logic_name_);
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  return YSOS_ERROR_SUCCESS;
}

std::string IoctlCondition::FromatValue(void) {
  std::string result;
  if (protcol_.empty()) {
    YSOS_LOG_DEBUG("protocol is empty");
    return result;
  }

  std::string protocol = protcol_;
  // protcol_ = "";

  ProtocolInterfacePtr protocol_ptr = GetProtocolInterfaceManager()->FindInterface(protocol);
  if (NULL == protocol_ptr || key_.empty()) {
    YSOS_LOG_ERROR("IoctlCondition::FromatValue protcol not exist: " << protocol << " | " << key_ << logic_name_);
    return result;
  }

  BufferInterfacePtr input_buffer = GetBufferUtility()->CreateBuffer(key_);
  BufferInterfacePtr output_buffer = GetBufferUtility()->CreateBuffer(1024);
  if (NULL == input_buffer || NULL == output_buffer) {
    YSOS_LOG_ERROR("IoctlCondition::FromatValue create buffer failed: " << key_ << logic_name_);
    return result;
  }

  int ret = protocol_ptr->FormMessage(input_buffer, output_buffer, 0, NULL);
  if (YSOS_ERROR_SUCCESS != ret) {
    YSOS_LOG_ERROR("IoctlCondition::FromatValue form message failed: " << logic_name_);
    return result;
  }

  const char *value = (const char*)GetBufferUtility()->GetBufferData(output_buffer);
  if (NULL != value) {
    YSOS_LOG_DEBUG("IoctlCondition::FromatValue protocol get value is: " << value);
    result = std::string(value);
  } else {
    YSOS_LOG_DEBUG("IoctlCondition::FromatValue protocol's value is null");
  }

  return result;
}

int IoctlCondition::DoAction(CallbackInterface *context) {
  assert(NULL != context);

  std::string result = FromatValue();
  if (!result.empty()) {
    ioctl_req_.value = result + ioctl_value_;
  } else {
    ioctl_req_.value = ioctl_value_;
  }

  std::string req_str;
  if (!ioctl_req_.ToString(req_str)) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  int ret = context->SetProperty(PROP_IOCTL_REQ_WITHOUT_DISPATCH, (void*)req_str.c_str());
  if (YSOS_ERROR_SUCCESS != ret && if_ignore_error_) {
    YSOS_LOG_ERROR("do ioctl action failed: " << req_str);
    return YSOS_ERROR_SKIP;
  }
  YSOS_LOG_DEBUG("do ioctl action success: " << req_str);

  return ret;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////          Ready Condition                         ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ReadyCondition::ReadyCondition(const std::string &strClassName /* = "EventCondition" */): BaseCondition(strClassName) {

}

ReadyCondition::~ReadyCondition() {

}

int ReadyCondition::FromString(const std::string &config_str) {
  PropertyTreePtr xml_node = ParseString(config_str);
  if (NULL == xml_node) {
    YSOS_LOG_DEBUG("parse string failed");
    return YSOS_ERROR_FAILED;
  }

  if_ignore_error_ = xml_node->GetAttribute("ignore_error") == "true" ? true : false;
  tag_ = xml_node->GetAttribute("tag");

  ready_req_.FromString(config_str);

  return YSOS_ERROR_SUCCESS;
}

int ReadyCondition::DoAction(CallbackInterface *context) {
  assert(NULL != context);
  std::string req_str;
  if (!ready_req_.ToString(req_str)) {
    YSOS_LOG_ERROR("do ready action failed: " << req_str);
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  int ret = context->SetProperty(PROP_READY_REQ_WITHOUT_DISPATCH, (void*)req_str.c_str());
  if (YSOS_ERROR_SUCCESS != ret && if_ignore_error_) {
    return YSOS_ERROR_SKIP;
  }
  YSOS_LOG_DEBUG("do ready action success: " << req_str);

  return ret;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////          Switch Condition                         ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SwitchCondition::SwitchCondition(const std::string &strClassName /* = "EventCondition" */): BaseCondition(strClassName) {

}

SwitchCondition::~SwitchCondition() {

}

int SwitchCondition::FromString(const std::string &config_str) {
  PropertyTreePtr xml_node = ParseString(config_str);
  if (NULL == xml_node) {
    YSOS_LOG_DEBUG("parse string failed");
    return YSOS_ERROR_FAILED;
  }

  if_ignore_error_ = xml_node->GetAttribute("ignore_error") == "true" ? true : false;
  tag_ = xml_node->GetAttribute("tag");

  switch_req_.state_name = xml_node->GetAttribute("state_name");
  if (switch_req_.state_name.empty()) {
    YSOS_LOG_INFO("parse switch failed: " << config_str);
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  return YSOS_ERROR_SUCCESS;
}

int SwitchCondition::DoAction(CallbackInterface *context) {
  assert(NULL != context);
  std::string req_str;
  if (!switch_req_.ToString(req_str)) {
    YSOS_LOG_DEBUG("invalid switch config param");

    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  int ret = context->SetProperty(PROP_SWITCH_REQ_WITHOUT_DISPATCH, (void*)req_str.c_str());
  if (YSOS_ERROR_SUCCESS != ret && if_ignore_error_) {
    YSOS_LOG_ERROR("do switch action failed: " << req_str);
    return YSOS_ERROR_SKIP;
  }
  YSOS_LOG_DEBUG("do switch action success: " << req_str);

  return ret;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////          Switch Notify Condition                 ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SwitchNotifyCondition::SwitchNotifyCondition(const std::string &strClassName /* = "EventCondition" */): BaseCondition(strClassName) {

}

SwitchNotifyCondition::~SwitchNotifyCondition() {

}

int SwitchNotifyCondition::FromString(const std::string &config_str) {
  PropertyTreePtr xml_node = ParseString(config_str);
  if (NULL == xml_node) {
    YSOS_LOG_DEBUG("parse string failed");
    return YSOS_ERROR_FAILED;
  }

  if_ignore_error_ = xml_node->GetAttribute("ignore_error") == "true" ? true : false;
  tag_ = xml_node->GetAttribute("tag");

  switch_notify_req_.state_name = xml_node->GetAttribute("state_name");
  if (switch_notify_req_.state_name.empty()) {
    YSOS_LOG_INFO("parse switch failed: " << config_str);
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  return YSOS_ERROR_SUCCESS;
}

int SwitchNotifyCondition::DoAction(CallbackInterface *context) {
  assert(NULL != context);

  int ret = context->SetProperty(PROP_SWITCH_NOTIFY_REQ, &switch_notify_req_);
  if (YSOS_ERROR_SUCCESS != ret && if_ignore_error_) {
    YSOS_LOG_ERROR("do switch notify action failed: " << switch_notify_req_.state_name);
    return YSOS_ERROR_SKIP;
  }
  YSOS_LOG_DEBUG("do switch notify action success: " << switch_notify_req_.state_name);

  return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////          Event Notify Condition                  ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EventNotifyCondition::EventNotifyCondition(const std::string &strClassName /* = "EventCondition" */): BaseCondition(strClassName) {

}

EventNotifyCondition::~EventNotifyCondition() {

}

int EventNotifyCondition::FromString(const std::string &config_str) {
  PropertyTreePtr xml_node = ParseString(config_str);
  if (NULL == xml_node) {
    YSOS_LOG_DEBUG("parse string failed");
    return YSOS_ERROR_FAILED;
  }

  if_ignore_error_ = xml_node->GetAttribute("ignore_error") == "true" ? true : false;
  tag_ = xml_node->GetAttribute("tag");

  event_notify_req_.event_name = xml_node->GetAttribute("event_name");
  event_notify_req_.callback_name = xml_node->GetAttribute("callback_name");
  event_notify_req_.data = xml_node->GetAttribute("data");
  if (event_notify_req_.event_name.empty() || event_notify_req_.callback_name.empty()) {
    YSOS_LOG_INFO("parse event notify failed: " << config_str);
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  return YSOS_ERROR_SUCCESS;
}

int EventNotifyCondition::DoAction(CallbackInterface *context) {
  assert(NULL != context);

  int ret = context->SetProperty(PROP_EVENT_NOTIFY_REQ, &event_notify_req_);
  if (YSOS_ERROR_SUCCESS != ret && if_ignore_error_) {
    YSOS_LOG_ERROR("do event notify action failed: " << event_notify_req_.event_name << " | " << event_notify_req_.callback_name);
    return YSOS_ERROR_SKIP;
  }
  YSOS_LOG_DEBUG("do event notify action success: " << event_notify_req_.event_name << " | " << event_notify_req_.callback_name);

  return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////          Send Event Condition                    ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SendEventCondition::SendEventCondition(const std::string &strClassName /* = "SendEventCondition" */): BaseCondition(strClassName) {

}

SendEventCondition::~SendEventCondition() {

}

int SendEventCondition::FromString(const std::string &config_str) {
  PropertyTreePtr xml_node = ParseString(config_str);
  if (NULL == xml_node) {
    YSOS_LOG_DEBUG("parse string failed");
    return YSOS_ERROR_FAILED;
  }

  if_ignore_error_ = xml_node->GetAttribute("ignore_error") == "true" ? true : false;
  tag_ = xml_node->GetAttribute("tag");

  event_name_ = xml_node->GetAttribute("event_name");
  content_ = xml_node->GetAttribute("content");

  return YSOS_ERROR_SUCCESS;
}

int SendEventCondition::DoAction(CallbackInterface *context) {
  assert(NULL != context);

  uint32_t msg_type = GetPlatformInfo()->GetMessageIdByName(event_name_);
  if (0 == msg_type) {
    YSOS_LOG_ERROR("get msg type failed: " << event_name_);
    return false;
  }

  /// message body
  BufferInterfacePtr msg_body_ptr = GetBufferUtility()->CreateBuffer(content_);
  // message
  MsgInterfacePtr message_ptr = MsgInterfacePtr(new MsgImpl(msg_type, msg_body_ptr));
  assert(NULL != message_ptr);

  int ret = context->SetProperty(PROP_EVENT_NOTIFY_REQ, &message_ptr);
  if (YSOS_ERROR_SUCCESS != ret && if_ignore_error_) {
    YSOS_LOG_ERROR("send event action failed: " << event_name_ << " | " << content_);
    return YSOS_ERROR_SKIP;
  }
  YSOS_LOG_DEBUG("do send event action success: " << event_name_ << " | " << content_);

  return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////          Set Data Service Condition              ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SetDataCondition::SetDataCondition(const std::string &strClassName /* = "SendEventCondition" */): BaseCondition(strClassName) {

}

SetDataCondition::~SetDataCondition() {

}

int SetDataCondition::FromString(const std::string &config_str) {
  PropertyTreePtr xml_node = ParseString(config_str);
  if (NULL == xml_node) {
    YSOS_LOG_DEBUG("parse string failed");
    return YSOS_ERROR_FAILED;
  }

  if_ignore_error_ = xml_node->GetAttribute("ignore_error") == "true" ? true : false;
  tag_ = xml_node->GetAttribute("tag");

  data_interface_name_ = xml_node->GetAttribute("data_interface");
  data_key = xml_node->GetAttribute("data_key");
  data_content = xml_node->GetAttribute("data_content");

  return YSOS_ERROR_SUCCESS;
}

int SetDataCondition::DoAction(CallbackInterface *context) {
  assert(NULL != context);

  DataInterfacePtr data_interface_ptr = GetDataInterfaceManager()->GetData(data_interface_name_);
  if (NULL == data_interface_ptr) {
    YSOS_LOG_ERROR("invalid data interface name: " << data_interface_name_);
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  return data_interface_ptr->SetData(data_key, data_content);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////          Callback Condition                      ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CallbackCondition::CallbackCondition(const std::string &strClassName /* = "CallbackCondition" */): BaseCondition(strClassName) {

}

CallbackCondition::~CallbackCondition() {

}

int CallbackCondition::FormatParam(const std::string &key, const std::string &value) {
  /// 设置默认缓存大小为1024
  buffer_ptr_ = GetBufferUtility()->CreateBuffer(1024);
  assert(NULL != buffer_ptr_);

  std::string value_str;
  JsonValue root;
  root[data_key.c_str()] = data_content.c_str();
  int ret = GetJsonUtil()->JsonValueToString(root, value_str);
  if (YSOS_ERROR_SUCCESS != ret) {
    YSOS_LOG_ERROR("json value to string failed: " << logic_name_);
    return YSOS_ERROR_FAILED;
  }
  int value_lenth = value_str.length()+1;
  if (value_lenth > 1024) {
    YSOS_LOG_ERROR("key value buffer is too larger: " << value_lenth << " | " << 1024 << " | " << logic_name_);
    return YSOS_ERROR_FAILED;
  }

  GetBufferUtility()->SetBufferLength(buffer_ptr_, value_lenth);
  uint8_t *data = GetBufferUtility()->GetBufferData(buffer_ptr_);
  assert(NULL != data);
  memcpy(data, value_str.c_str(), value_str.length()+1);

  return YSOS_ERROR_SUCCESS;
}

int CallbackCondition::FromString(const std::string &config_str) {
  PropertyTreePtr xml_node = ParseString(config_str);
  if (NULL == xml_node) {
    YSOS_LOG_DEBUG("parse string failed");
    return YSOS_ERROR_FAILED;
  }

  if_ignore_error_ = xml_node->GetAttribute("ignore_error") == "true" ? true : false;
  tag_ = xml_node->GetAttribute("tag");

  callback_name_ = xml_node->GetAttribute("callback_name");
  data_key = xml_node->GetAttribute("data_key");
  data_content = xml_node->GetAttribute("data_content");

  callback_ptr_ = GetCallbackInterfaceManager()->FindInterface(callback_name_);
  if (NULL == callback_ptr_) {
    YSOS_LOG_ERROR("find callback " << callback_name_ << " failed");
  } else {
    YSOS_LOG_DEBUG("find callback " << callback_name_ << " success");
  }

  int ret = YSOS_ERROR_SUCCESS;
  ret = FormatParam(data_key, data_content);
  if (YSOS_ERROR_SUCCESS != ret) {
    YSOS_LOG_ERROR("format param failed: " << logic_name_);
  }

  return ret;
}

int CallbackCondition::DoAction(CallbackInterface *context) {
  assert(NULL != context);

  if (NULL == callback_ptr_) {
    return YSOS_ERROR_NOT_EXISTED;
  }

  return callback_ptr_->Callback(buffer_ptr_, NULL, NULL);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////          Condition manager                       ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DEFINE_SINGLETON(ConditionManager);
ConditionManager::ConditionManager(const std::string &strClassName /* = "ConditionManager" */): BaseInterfaceImpl(strClassName) {

}

ConditionManager::~ConditionManager() {

}

BaseConditionPtr ConditionManager::GetCondition(const std::string &type, const std::string &config_str) {
  if (type.empty() || config_str.empty()) {
    YSOS_LOG_DEBUG("invalid condition param");
    return NULL;
  }

  BaseCondition *condition_ptr = NULL;
  if ("event" == type) {
    condition_ptr = new EventCondition();
  } else if ("ioctl" == type) {
    condition_ptr = new IoctlCondition();
  } else if ("ready" == type) {
    condition_ptr = new ReadyCondition();
  } else if ("switch" == type) {
    condition_ptr = new SwitchCondition();
  } else if ("switch_notify" == type) {
    condition_ptr = new SwitchNotifyCondition();
  } else if ("event_notify" == type) {
    condition_ptr = new EventNotifyCondition();
  } else if ("send_event" == type) {
    condition_ptr = new SendEventCondition();
  } else if ("set_data" == type) {
    condition_ptr = new SetDataCondition();
  } else if ("callback" == type) {
    condition_ptr = new CallbackCondition();
  } else {
    YSOS_LOG_DEBUG("unknown condition type: " << type);
  }

  if (NULL == condition_ptr) {
    return NULL;
  }

  int ret = condition_ptr->FromString(config_str);
  if (YSOS_ERROR_SUCCESS != ret) {
    YSOS_LOG_DEBUG("invalid condition param: " << config_str);
    return NULL;
  }

  return BaseConditionPtr(condition_ptr);
}
}
