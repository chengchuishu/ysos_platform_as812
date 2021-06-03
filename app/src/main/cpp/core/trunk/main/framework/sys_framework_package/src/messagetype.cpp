/**
 *@file MessageType.cpp
 *@brief message id map
 *@version 1.0
 *@author dhongqian
 *@date Created on: 2016-05-11 10:06:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// Self Header
#include "../../../public/include/sys_framework_package/messagetype.h"
/// Ysos Headers
#include "../../../public/include/core_help_package/utility.h"
#include "../../../public/include/os_hal_package/lock.h"
#include "../../../public/include/sys_interface_package/error.h"
// #include "../../../public/include/core_help_package/ysos_log.h"

namespace ysos {
MessageTypePtr MessageType::message_type_ptr_ = NULL;
boost::shared_ptr<LightLock> MessageType::light_lock_ptr_
  = boost::shared_ptr<LightLock>(new LightLock());

MessageType::MessageType(const std::string& message_type/* ="MessageType" */): BaseInterfaceImpl(message_type) {
}

MessageType::~MessageType() {
  service_event_map_.clear();
  name_id_map_.clear();
  id_name_map_.clear();
  event_service_map_.clear();
}

MessageTypePtr MessageType::Instance() {
  if (NULL == message_type_ptr_) {
    AutoLockOper lock(light_lock_ptr_.get());
    if (NULL == message_type_ptr_) {
      message_type_ptr_ = MessageTypePtr(new MessageType());
    }
  }

  return message_type_ptr_;
}

int MessageType::ParseMessageType(const std::string &file_name) {
  std::string data = GetUtility()->ReadAllDataFromFile(file_name);
  if (data.empty()) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  TreeNodeIterator message_tree = TreeNodeIterator(new PropertyTree());
  assert(NULL != message_tree);
  int ret = message_tree->ParseString(data);
  if (YSOS_ERROR_SUCCESS != ret) {
    YSOS_LOG_DEBUG("Parse MessageType failed: invalid file: " << file_name);
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  TreeNodeIterator element = message_tree->FindFirstChild("event");
  while (NULL != element) {
    TreeNodeIterator id = element->FindFirstChild("id");
    TreeNodeIterator name = element->FindFirstChild("name");
    TreeNodeIterator service = element->FindFirstChild("service");
    
    UpdateNameIdMap(name, id);
    UpdateEventServiceMap(name, service);

    element = element->FindNextSibling("event");
  }

  return YSOS_ERROR_SUCCESS;
}

void MessageType::UpdateNameIdMap(TreeNodeIterator name, TreeNodeIterator id) {
  if(NULL == name || NULL == id) {
    return;
  }

  uint32_t default_id = -1;
  uint32_t message_id = GetUtility()->ConvertFromString(id->GetNodeValue(), default_id);
  if (name_id_map_.end() != name_id_map_.find(name->GetNodeValue()) ||
    id_name_map_.end() != id_name_map_.find(message_id)) {
      YSOS_LOG_DEBUG ("Parse MessageType failed: " << "[" << name->GetNodeValue() << ":" << id->GetNodeValue() << "] already exist.");
      return;
  }

  name_id_map_.insert(std::make_pair(name->GetNodeValue(), message_id));
  id_name_map_.insert(std::make_pair(message_id, name->GetNodeValue()));
}

void MessageType::UpdateEventServiceMap(TreeNodeIterator event, TreeNodeIterator service) {
  if(NULL == event || NULL == service) {
    return;
  }

  if (service_event_map_.end() == service_event_map_.find(service->GetNodeValue()) ||
        event_service_map_.end() == event_service_map_.find(event->GetNodeValue())) {
      service_event_map_.insert(std::make_pair(service->GetNodeValue(), event->GetNodeValue()));
      event_service_map_.insert(std::make_pair(event->GetNodeValue(), service->GetNodeValue()));
    }
}

uint32_t MessageType::GetMessageTypeId(const std::string &message_name) {
  if (name_id_map_.end() != name_id_map_.find(message_name)) {
    return name_id_map_[message_name];
  }

  return 0;
}

std::string MessageType::GetMessageTypeName(const uint32_t &message_id) {
  if (id_name_map_.end() != id_name_map_.find(message_id)) {
    return id_name_map_[message_id];
  }

  return "";
}

std::string MessageType::GetServiceByEvent(const std::string &event_name) {
  if (event_service_map_.end() != event_service_map_.find(event_name)) {
    return event_service_map_[event_name];
  }

  return "";
}

std::string MessageType::GetEventByService(const std::string &service_name) {
  if (service_event_map_.end() != service_event_map_.find(service_name)) {
    return service_event_map_[service_name];
  }

  return "";
}
}