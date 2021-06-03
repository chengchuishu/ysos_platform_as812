/**
 *@file Platform.cpp
 *@brief All Platform Info
 *@version 1.0
 *@author dhongqian
 *@date Created on: 2016-05-11 10:06:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// ysos private headers //  NOLINT
#include "../../../protect/include/sys_framework_package/platforminfo.h"
/// Ysos Headers
#include "../../../public/include/core_help_package/utility.h"
#include "../../../protect/include/core_help_package/propertytree.h"
#include "../../../public/include/os_hal_package/lock.h"
#include "../../../public/include/sys_interface_package/error.h"
// #include "../../../public/include/core_help_package/ysos_log.h"
#include "../../../public/include/sys_framework_package/messagetype.h"
#include "../../../protect/include/sys_framework_package/packageconfigimpl.h"
#include "../../../protect/include/sys_framework_package/serviceevent.h"

namespace ysos {
PlatformInfoPtr PlatformInfo::platform_info_ptr_ = NULL;
boost::shared_ptr<LightLock> PlatformInfo::light_lock_ptr_
  = boost::shared_ptr<LightLock>(new LightLock());

PlatformInfo::PlatformInfo(const std::string& platform_info): BaseInterfaceImpl(platform_info) {

}

PlatformInfo::~PlatformInfo() {

}

PlatformInfoPtr PlatformInfo::Instance() {
  if (NULL == platform_info_ptr_) {
    AutoLockOper lock(light_lock_ptr_.get());
    if (NULL == platform_info_ptr_) {
      platform_info_ptr_ = PlatformInfoPtr(new PlatformInfo());
    }
  }

  return platform_info_ptr_;
}

uint32_t  PlatformInfo::GetMessageIdByName(const std::string &message_name) {
  return GetMessageType()->GetMessageTypeId(message_name);
}

std::string  PlatformInfo::GetMessageNameById(const uint32_t &message_id) {
  return MessageType::Instance()->GetMessageTypeName(message_id);
}

uint32_t PlatformInfo::GetMessageIdByServiceName(const std::string &service_name) {
  std::string event_name = GetMessageType()->GetEventByService(service_name);
  if (event_name.empty()) {
    /// service name 有可能是 module_logic_name + "_123" //  NOLINT
    std::string service = service_name.substr(0, service_name.find_last_of("_"));
    event_name = GetMessageType()->GetEventByService(service);
    if (event_name.empty()) {
      return 0;
    }
  }

  return GetMessageIdByName(event_name);
}

uint32_t PlatformInfo::GetServiceIdByName(const std::string &service_name) {
  return ServiceInfoTable::Instance()->GetServiceIDByName(service_name);
}

ServiceInfoTable::ServiceConfInfoPtr  PlatformInfo::GetServiceByName(const std::string &service_name) {
  return ServiceInfoTable::Instance()->GetServiceByName(service_name);
}

std::string PlatformInfo::GetServiceNameById(const uint32_t &service_id) {
  return ServiceInfoTable::Instance()->GetServiceNameByID(service_id);
}

const std::string PlatformInfo::GetConfPath(void) const {
  return PackageConfigImpl::instance()->GetConfPath();
}
}
