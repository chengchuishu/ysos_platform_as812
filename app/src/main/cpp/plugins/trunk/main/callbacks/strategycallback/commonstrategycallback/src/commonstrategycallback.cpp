/**
 *@file CommonStrategyCallback.cpp
 *@brief 状态机通用Callback,用于StateMachine层事件、Service回调
 *@version 1.0
 *@author dhongqian
 *@date Created on: 2016-05-11 10:06:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// Self Header //  NOLINT
#include "../include/commonstrategycallback.h"
/// private headers
#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"
/// Ysos Sdk Headers
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/messageinterface.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/packageconfigimpl.h"
#include <boost/make_shared.hpp>

namespace ysos {

DECLARE_PLUGIN_REGISTER_INTERFACE(CommonStrategyCallback, CallbackInterface);
CommonStrategyCallback::CommonStrategyCallback(const std::string &strClassName):BaseStrategyCallbackImpl(strClassName) {
  is_ignore_self_ = true;
  condition_before_list_ = boost::make_shared<ConditionEntityList>();
  condition_after_list_ = boost::make_shared<ConditionEntityList>();
  logger_ = GetUtility()->GetLogger("ysos.strategy");
}

CommonStrategyCallback::~CommonStrategyCallback() {
  condition_before_list_->Clear();
  condition_before_list_ = NULL;
  condition_after_list_->Clear();
  condition_after_list_ = NULL;

  ConditionListMap::iterator it=error_condition_map_.begin();
  while(error_condition_map_.end() != it) {
    ConditionEntityListPtr condition_list_ptr = it->second;
    condition_list_ptr->Clear();
  }
}

// StrategyConditionPtr CommonStrategyCallback::GetCondition(const std::string &event_name, ConditionMap  *map_ptr) {
//   assert(NULL != map_ptr && !event_name.empty());
//
//   ConditionMap::iterator it = map_ptr->find(event_name);
//   if (map_ptr->end() == it) {
//     return NULL;
//   }
//
//   return it->second;
// }

int CommonStrategyCallback::HandleCondition(BufferInterfacePtr input_buffer, ConditionEntityListPtr &condition_list, CallbackInterface *context) {
  assert(NULL != context);

  int ret = condition_list->DoAction(context, error_condition_map_);

  return ret;
}

int CommonStrategyCallback::HandleServiceMessage(std::string service_name, BufferInterfacePtr input_buffer, CallbackInterface *context) {
  assert(!service_name.empty() /*&& NULL != input_buffer*/ && NULL != context);

  uint8_t *data = GetBufferUtility()->GetBufferData(input_buffer);
  if (NULL == data && "ready" != service_name) {
    YSOS_LOG_ERROR("data is null: " << service_name);
    return YSOS_ERROR_FAILED;
  }

  int ret = YSOS_ERROR_SUCCESS;
  if ("switch" == service_name) {
    ret = context->SetProperty(PROP_SWITCH_REQ, (void*)data);
  } else if ("ioctl" == service_name) {
    ret = context->SetProperty(PROP_IOCTL_REQ, (void*)data);
  } else if ("event" == service_name) {
    ret = context->SetProperty(PROP_REG_SERVICE_EVENT_REQ, (void*)data);
  } else if ("ready" == service_name) {
    ReqReadyServiceParam ready_req;
    ret = DoReadyService(ready_req, context);
  } else {
    ret = YSOS_ERROR_NOT_SUPPORTED;
  }

  if (YSOS_ERROR_SUCCESS != ret) {
    YSOS_LOG_ERROR("handle service message failed: " << service_name << " : " << ret);
  }

  return YSOS_ERROR_SUCCESS;
}

int CommonStrategyCallback::HandleEventMessage(std::string event_name, BufferInterfacePtr input_buffer, CallbackInterface *context) {
  assert(!event_name.empty() /*&& NULL != input_buffer*/ && NULL != context);
  if(callback_name_.empty()) {
      /// 没有Callback，也认为忽略事件本身，等同于ignore_self==true
      return YSOS_ERROR_SUCCESS;
  }

  std::string content;
  const char *buffer_data = (const char*)GetBufferUtility()->GetBufferData(input_buffer);
  if(NULL != buffer_data) {
      content = buffer_data;
  }
  int ret = DoEventNotifyService(event_name, callback_name_, content, context);
  if(YSOS_ERROR_SUCCESS != ret) {
      YSOS_LOG_ERROR("handle event message failed: " << event_name << " | " << callback_name_ << " | " << content << " | " << logic_name_);
  }
  return ret;
}

int CommonStrategyCallback::HandleSelfMessage(BufferInterfacePtr input_buffer, CallbackInterface *context) {
  if ("service" == condition_type_) {
    int ret = HandleServiceMessage(condition_name_, input_buffer, context);
    if (YSOS_ERROR_SUCCESS != ret) {
      YSOS_LOG_ERROR("handle service message failed: " << condition_name_);
    }
  } else if ("event" == condition_type_) {
    int ret = HandleEventMessage(condition_name_, input_buffer, context);
    if (YSOS_ERROR_SUCCESS != ret) {
      YSOS_LOG_ERROR("handle service message failed: " << condition_name_);
    }
  } else {
    YSOS_LOG_ERROR("unknown type: " << condition_type_ << " : " << condition_name_);
  }

  return YSOS_ERROR_SUCCESS;
}

int CommonStrategyCallback::HandleMessage(const std::string &event_name, BufferInterfacePtr input_buffer, CallbackInterface *context) {
  YSOS_LOG_DEBUG("CommonStrategyCallback::HandleMessage event_name: " << event_name << " condition_name: " << condition_name_);
  assert(event_name == condition_name_);
  /// 比如，Ready的input_buffer，可以是NULLL
  assert(/*NULL != input_buffer &&*/ NULL != context);

  YSOS_LOG_DEBUG("Enter CommonStrategyCallback::HandleMessage: " << logic_name_);
  int ret = HandleCondition(input_buffer, condition_before_list_, context);
  if (YSOS_ERROR_SUCCESS != ret) {
    YSOS_LOG_ERROR("handle message： " << event_name << " failed");
  }

  if (!is_ignore_self_) {
    ret = HandleSelfMessage(input_buffer, context);
    if (YSOS_ERROR_SUCCESS != ret) {
      YSOS_LOG_ERROR("handle self message failed： " << event_name << " failed");
    }
  }

  ret = HandleCondition(input_buffer, condition_after_list_, context);
  if (YSOS_ERROR_SUCCESS != ret) {
    YSOS_LOG_ERROR("handle message： " << event_name << " failed");
  }
  YSOS_LOG_DEBUG("Leave CommonStrategyCallback::HandleMessage: " << logic_name_);

  return YSOS_ERROR_SUCCESS;
}

int CommonStrategyCallback::Initialized(const std::string &key, const std::string &value) {
  int ret = YSOS_ERROR_SUCCESS;

  if ("condition" == key) {
    std::string condition_file = GetPackageConfigImpl()->GetConfPath() + value;
    ret = ParseConfigureFile(condition_file, true);
  } else {
    ret = YSOS_ERROR_NOT_SUPPORTED;
  }

  if (YSOS_ERROR_NOT_SUPPORTED == ret) {
    YSOS_LOG_DEBUG("not recognise config param: " << key << " value: " << value);
  }

  return ret;
}

PropertyTreePtr CommonStrategyCallback::ParseFile(const std::string &config_file) {
  if (config_file.empty()) {
    YSOS_LOG_DEBUG("invalid config file name: " << config_file);
    return NULL;
  }

  PropertyTreePtr xml_root = PropertyTreePtr(new PropertyTree());
  if (NULL == xml_root) {
    YSOS_LOG_INFO("allocate property tree failed");
    return NULL;
  }

  std::string xml_content = GetUtility()->ReadAllDataFromFile(config_file);
  if (xml_content.empty()) {
    YSOS_LOG_DEBUG("read config file failed: " << config_file);
    return NULL;
  }
  int ret = xml_root->ParseString(xml_content);
  if (YSOS_ERROR_SUCCESS != ret) {
    YSOS_LOG_DEBUG("parse config xml failed: " << xml_content);
    return NULL;
  }

  return xml_root;
}

int CommonStrategyCallback::ParseCondition(TreeNodeIterator condition_ptr, ConditionEntityListPtr &condition_list) {
  assert(NULL != condition_ptr);

  TreeNodeIterator child_node = condition_ptr->FindFirstChild();
  while (NULL != child_node) {
    std::string condition_name = child_node->GetNodeName();
    std::string condition_xml = child_node->ConvertToString();
    BaseConditionPtr condition_ptr = GetConditionManager()->GetCondition(condition_name, condition_xml);
    if (NULL != condition_ptr) {
      YSOS_LOG_INFO("add new condition: " << condition_name << " | " << condition_xml);
      condition_list->AddCondition(condition_ptr);
    } else {
      YSOS_LOG_ERROR("parse condition failed: " << condition_name << " | " << condition_xml);
    }

    child_node = child_node->FindNextSibling();
  }

  return YSOS_ERROR_SUCCESS;
}

int CommonStrategyCallback::AddToConditionMap(BaseConditionPtr condition_ptr, ConditionListMap &error_handler_map) {
  assert(NULL != condition_ptr);

  std::string tag = condition_ptr->GetTag();
  if(tag.empty()) {
    YSOS_LOG_ERROR("tag is empty: " << condition_ptr->GetName());
    return YSOS_ERROR_NOT_SUPPORTED;
  }

  ConditionEntityListPtr condition_list_ptr;
  ConditionListMap::iterator it = error_handler_map.find(tag);
  if(error_handler_map.end() == it) {
    condition_list_ptr = boost::make_shared<ConditionEntityList >();
    if(NULL == condition_list_ptr) {
      YSOS_LOG_ERROR("allocate memory failed: " << tag);
      return YSOS_ERROR_MEMORY_ALLOCATED_FAILED;
    }
    error_handler_map[tag] = condition_list_ptr;
  } else {
    condition_list_ptr = it->second;
  }

  condition_list_ptr->AddCondition(condition_ptr);

  return YSOS_ERROR_SUCCESS;
}

int CommonStrategyCallback::ParseErrorHandler(TreeNodeIterator condition_ptr, ConditionListMap &error_handler_map) {
  assert(NULL != condition_ptr);

  TreeNodeIterator child_node = condition_ptr->FindFirstChild();
  while (NULL != child_node) {
    std::string condition_name = child_node->GetNodeName();
    std::string condition_xml = child_node->ConvertToString();
    BaseConditionPtr condition_ptr = GetConditionManager()->GetCondition(condition_name, condition_xml);
    if (NULL != condition_ptr) {
      YSOS_LOG_INFO("add new condition: " << condition_name << " | " << condition_xml);
      int ret = AddToConditionMap(condition_ptr, error_handler_map);
      if(YSOS_ERROR_SUCCESS != ret) {
        YSOS_LOG_ERROR("add to condition map failed: " << ret << " | " << condition_ptr->GetName());
      }
    } else {
      YSOS_LOG_ERROR("parse condition failed: " << condition_name << " | " << condition_xml);
    }

    child_node = child_node->FindNextSibling();
  }

  return YSOS_ERROR_SUCCESS;
}

int CommonStrategyCallback::ParseConfigureFile(const std::string &config_file, bool is_before_condition) {
  PropertyTreePtr xml_root = ParseFile(config_file);
  if (NULL == xml_root) {
    YSOS_LOG_DEBUG("parse string failed");
    return YSOS_ERROR_FAILED;
  }

  TreeNodeIterator condition_ptr = xml_root->FindFirstChild("condition");
  if (NULL == condition_ptr) {
    YSOS_LOG_DEBUG("config xml has no scondition node: " << config_file);
    return YSOS_ERROR_FAILED;
  }
  condition_name_ = condition_ptr->GetAttribute("name");
  condition_type_ = condition_ptr->GetAttribute("type");
  is_ignore_self_ = condition_ptr->GetAttribute("ignore_self")=="true" ? true : false;
  callback_name_ = condition_ptr->GetAttribute("callback");
  event_name_ = condition_ptr->GetAttribute("event_name");
  if(event_name_.empty()) {
      event_name_ = condition_name_;
  }

  boost::to_lower(condition_name_);

  if (condition_name_.empty() || condition_type_.empty()) {
    YSOS_LOG_ERROR("parse config file failed: " << condition_name_ << " : " << condition_type_);
    return YSOS_ERROR_FAILED;
  }

  TreeNodeIterator condition_before_ptr = condition_ptr->FindFirstChild("condition_before");
  if (NULL != condition_before_ptr) {
    int ret = ParseCondition(condition_before_ptr, condition_before_list_);
    if (YSOS_ERROR_SUCCESS != ret) {
      YSOS_LOG_ERROR("parse condition before failed: " << config_file);
    }
  }

  TreeNodeIterator condition_after_ptr = condition_ptr->FindFirstChild("condition_after");
  if (NULL != condition_after_ptr) {
    int ret = ParseCondition(condition_after_ptr, condition_after_list_);
    if (YSOS_ERROR_SUCCESS != ret) {
      YSOS_LOG_ERROR("parse condition after failed: " << config_file);
    }
  }

  TreeNodeIterator error_handler_ptr = condition_ptr->FindFirstChild("error_handler");
  if(NULL != error_handler_ptr) {
    int ret = ParseErrorHandler(error_handler_ptr, error_condition_map_);
    if(YSOS_ERROR_SUCCESS != ret) {
      YSOS_LOG_ERROR("parse condition after failed: " << config_file);
    }
  }

  return YSOS_ERROR_SUCCESS;
}
}
