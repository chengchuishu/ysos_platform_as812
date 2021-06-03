/**
 *@file ServiceEvent.cpp
 *@brief ServiceEven
 *@version 1.0
 *@author dhongqian
 *@date Created on: 2016-05-11 10:06:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// Self Header
#include "../../../protect/include/sys_framework_package/serviceevent.h"
/// Ysos Headers
#include "../../../public/include/core_help_package/utility.h"
#include "../../../protect/include/core_help_package/propertytree.h"
#include "../../../public/include/sys_interface_package/error.h"
// #include "../../../public/include/core_help_package/ysos_log.h"

namespace ysos {
DEFINE_SINGLETON(ServiceEvent);
ServiceEvent::ServiceEvent(const std::string& service_event/* ="ServiceEvent" */): BaseInterfaceImpl(service_event) {

}

ServiceEvent::~ServiceEvent() {

}

int ServiceEvent::ParseServiceEvent(const std::string &file_name) {
  std::string data = GetUtility()->ReadAllDataFromFile(file_name);
  if (data.empty()) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  TreeNodeIterator service_event_tree = TreeNodeIterator(new PropertyTree());
  assert(NULL != service_event_tree);
  int ret = service_event_tree->ParseString(data);
  if (YSOS_ERROR_SUCCESS != ret) {
    YSOS_LOG_DEBUG("Parse event service failed: invalid file: " << file_name);
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  TreeNodeIterator element = service_event_tree->FindFirstChild("service");
  while (NULL != element) {
    TreeNodeIterator service = element->FindFirstChild("name");
    TreeNodeIterator event = element->FindFirstChild("event_name");
    if (NULL == service || NULL == event) {
      YSOS_LOG_DEBUG("Parse MessageType failed: invalid file: " << file_name);
      continue;
    }

    std::string service_name = service->GetNodeValue();
    std::string event_name = event->GetNodeValue();
    if(service_name.empty() || event_name.empty()) {
      continue;
    }

    if(service_event_map_.end() != service_event_map_.find(service_name)) {
      continue;
    }

    service_event_map_.insert(std::make_pair(service_name, event_name));
    element = element->FindNextSibling("service");
  }

  return YSOS_ERROR_SUCCESS;
}

std::string ServiceEvent::GetEventName(const std::string &service_name) {
  if (service_event_map_.end() != service_event_map_.find(service_name)) {
    return service_event_map_[service_name];
  }

  return "";
}
}