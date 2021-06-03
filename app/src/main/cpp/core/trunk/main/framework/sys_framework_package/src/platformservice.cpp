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
#include "../../../protect/include/sys_framework_package/platformservice.h"
/// Ysos Headers
#include "../../../public/include/sys_interface_package/error.h"
// #include "../../../public/include/core_help_package/ysos_log.h"
#include "../../../protect/include/sys_framework_package/platforminfo.h"

namespace ysos {
DEFINE_SINGLETON(PlatformService);
PlatformService::PlatformService(const std::string &strClassName/* ="ServiceEvent" */): BaseInterfaceImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.framework");
}

PlatformService::~PlatformService() {
  service_map_.clear();
  alias_service_map_.clear();
}

void PlatformService::Dump(void) {
  ServiceInfoMap::iterator it=service_map_.begin();
  for (; it!=service_map_.end(); ++it) {
    ServiceInfoPtr service_info = it->second;

    YSOS_LOG_DEBUG("name: " << service_info->name << " ||  "
                   << "alias: " << service_info->alias << " ||  "
                   << "type: "  << service_info->type << " ||  "
                   << "default: " << service_info->default_type);
  }
}

int PlatformService::ParseService(const std::string &file_name) {
  std::string data = GetUtility()->ReadAllDataFromFile(file_name);
  if (data.empty()) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  TreeNodeIterator service_tree = TreeNodeIterator(new PropertyTree());
  assert(NULL != service_tree);
  int ret = service_tree->ParseString(data);
  if (YSOS_ERROR_SUCCESS != ret) {
    YSOS_LOG_DEBUG("Parse event service failed: invalid file: " << file_name);
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  TreeNodeIterator element = service_tree->FindFirstChild("service_list");
  if (NULL == element) {
    YSOS_LOG_DEBUG("parse service xml: " << file_name << " failed");
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }
  TreeNodeIterator service_ptr = element->FindFirstChild("service");
  if (NULL != service_ptr) {
    ParseService(service_ptr);
  }

  service_ptr = element->FindFirstChild("xi:include");
  if (NULL != service_ptr) {
    ParseIncludeService(service_ptr);
  }

  return YSOS_ERROR_SUCCESS;
}

int PlatformService::ParseService(TreeNodeIterator service_ptr) {
  int ret = YSOS_ERROR_SUCCESS;

  while (NULL != service_ptr) {
    ret = YSOS_ERROR_SUCCESS;
    ServiceInfoPtr service_info_ptr = ServiceInfoPtr(new ServiceInfo());
    if (NULL == service_info_ptr) {
      return YSOS_ERROR_MEMORY_ALLOCATED_FAILED;
    }

    service_info_ptr->name = service_ptr->GetAttribute("name");
    service_info_ptr->alias = service_ptr->GetAttribute("alias");
    service_info_ptr->type = service_ptr->GetAttribute("type");
    TreeNodeIterator descrition = service_ptr->FindFirstChild("description");
    service_info_ptr->description = descrition->GetNodeValue();

    if (service_info_ptr->name.empty()) {
      service_ptr = service_ptr->FindNextSibling("service");
      continue;
    }

    if (service_info_ptr->type.empty()) {
      service_info_ptr->type = "module";
    }

    if ("select" == service_info_ptr->type || "composit" == service_info_ptr->type) {
      ret = GetChildService(service_info_ptr, service_ptr);
    }

    if ("callback" != service_info_ptr->type && "module" != service_info_ptr->type) {
      YSOS_LOG_DEBUG("Invalid service type: " << service_info_ptr->type);
      service_ptr = service_ptr->FindNextSibling("service");
      continue;
    }
    if (service_map_.end() != service_map_.find(service_info_ptr->name)) {
      ret = YSOS_ERROR_HAS_EXISTED;
    }
    if (alias_service_map_.end() != alias_service_map_.find(service_info_ptr->alias)) {
      ret = YSOS_ERROR_HAS_EXISTED;
    }

    if (YSOS_ERROR_SUCCESS == ret) {
      service_map_.insert(std::make_pair(service_info_ptr->name, service_info_ptr));
      /// alias允许空 //  NOLINT
      if (!service_info_ptr->alias.empty()) {
        alias_service_map_.insert(std::make_pair(service_info_ptr->alias, service_info_ptr));
      }
    }
    service_ptr = service_ptr->FindNextSibling("service");
  }

  return YSOS_ERROR_SUCCESS;
}

int PlatformService::ParseIncludeService(TreeNodeIterator service_ptr) {
  while (NULL != service_ptr) {
    std::string filename = service_ptr->GetAttribute("href");
    ParseService(GetPlatformInfo()->GetConfPath() + filename);

    service_ptr = service_ptr->FindNextSibling("xi:include");
  }

  return YSOS_ERROR_SUCCESS;
}

int PlatformService::GetChildService(PlatformService::ServiceInfoPtr service_info_ptr, TreeNodeIterator service_ptr) {
  if (NULL == service_ptr || NULL == service_info_ptr) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  TreeNodeIterator child = service_ptr->FindFirstChild("service");
  while (NULL != child) {
    std::string name = child->GetAttribute("name");
    if (name.empty()) {
      continue;
    }

    std::string default_attribute = child->GetAttribute("default");
    if (GetUtility()->ToLower(default_attribute) == "true" && "select" == service_info_ptr->type) {
      service_info_ptr->default_type = name;
    }
    service_info_ptr->service_list.push_back(name);

    child = service_ptr->FindNextSibling("service");
  }

  return YSOS_ERROR_SUCCESS;
}

PlatformService::ServiceInfoPtr PlatformService::GetService(const std::string &service_name) {
  ServiceInfoMap::iterator it = service_map_.find(service_name);
  if (it != service_map_.end()) {
    return it->second;
  }

  it = alias_service_map_.find(service_name);
  if (it != alias_service_map_.end()) {
    return it->second;
  }

  return NULL;
}

std::string PlatformService::GetServiceRealName(const std::string &service_name) {
  if (std::string::npos != service_name.find_first_of('@')) {
    return service_name;
  }

  ServiceInfoPtr service_ptr = GetService(service_name);
  if (NULL == service_ptr) {
    return "";
  }

  if ("module" == service_ptr->type || "callback" == service_ptr->type) {
    YSOS_LOG_DEBUG("get real servie name: " << service_name << " | " << service_ptr->name);
    return service_ptr->name;
  }

  return "";
}

PlatformService::ServiceInfoMap& PlatformService::GetServiceMap(void) {
  return service_map_;
}

PlatformService::ServiceInfoMap& PlatformService::GetAliasServiceMap(void) {
  return alias_service_map_;
}

}
