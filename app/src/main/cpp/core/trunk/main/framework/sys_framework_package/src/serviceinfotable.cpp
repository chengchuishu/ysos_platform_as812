/**
*@file serviceinfotable.cpp
*@brief Definition of ServiceInfoTable
*@version 0.1
*@author jinchengzhe
*@date Created on: 2016-06-21 15:14:58
*@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
* 
*/

/// Self Header
#include "../../../protect/include/sys_framework_package/serviceinfotable.h"
/// Ysos Headers
#include "../../../public/include/os_hal_package/lock.h"
#include "../../../public/include/sys_interface_package/error.h"
// #include "../../../public/include/core_help_package/ysos_log.h"

namespace ysos {
ServiceInfoTablePtr ServiceInfoTable::service_info_table_ptr_ = NULL;
boost::shared_ptr<LightLock> ServiceInfoTable::light_lock_ptr_
  = boost::shared_ptr<LightLock>(new LightLock());

ServiceInfoTable::ServiceInfoTable() {
  logger_ = GetUtility()->GetLogger("ysos.framework");

  service_conf_info_list_.clear();
}

ServiceInfoTable::~ServiceInfoTable() {

  YSOS_LOG_DEBUG("ServiceInfoTable::~ServiceInfoTable()");

  Reset();

}

int ServiceInfoTable::AddServiceConfInfo(
  const ServiceConfInfoPtr service_conf_info_ptr) {
  if (NULL == service_conf_info_ptr) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  service_info_table_ptr_->service_conf_info_list_.push_back(
    service_conf_info_ptr);

  return YSOS_ERROR_SUCCESS;
}

int ServiceInfoTable::DeleteServiceConfInfo(
  const ServiceConfInfoPtr service_conf_info_ptr) {
  if (NULL == service_conf_info_ptr) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  ServiceConfInfoIterator iterator = std::find(
                                       this->service_conf_info_list_.begin(),
                                       this->service_conf_info_list_.end(),
                                       service_conf_info_ptr);
  if (this->service_conf_info_list_.end() != iterator) {
    this->service_conf_info_list_.erase(iterator);
  }

  return YSOS_ERROR_SUCCESS;
}

void ServiceInfoTable::Reset() {
  if (0 != service_conf_info_list_.size()) {
    for (
      ServiceConfInfoIterator iterator = service_conf_info_list_.begin();
      iterator != service_conf_info_list_.end();
      ++iterator
    ) {
      if (0 != (*iterator)->sub_services.size()) {
        (*iterator)->sub_services.clear();
      }
    }

    service_conf_info_list_.clear();
  }

  return;
}

const ServiceInfoTable::ServiceConfInfoList&
ServiceInfoTable::GetServiceConfInfoList() const {
  return service_conf_info_list_;
}

const std::string ServiceInfoTable::GetServiceNameByID(const UINT32 service_ID) {
  for (ServiceInfoTable::ServiceConfInfoIterator iterator
       = service_conf_info_list_.begin();
       iterator != service_conf_info_list_.end();
       ++iterator) {
    if (service_ID == (*iterator)->ID) {
      return (*iterator)->name;
    }
  }

  return "";
}

const UINT32 ServiceInfoTable::GetServiceIDByName(const std::string service_name) {
  for (ServiceInfoTable::ServiceConfInfoIterator iterator
       = service_conf_info_list_.begin();
       iterator != service_conf_info_list_.end();
       ++iterator) {
    if (service_name == (*iterator)->name) {
      return (*iterator)->ID;
    }
  }

  return 0;
}

const ServiceInfoTable::ServiceConfInfoPtr
ServiceInfoTable::GetServiceByName(const std::string service_name) {
  if (true == service_name.empty()) {
    return NULL;
  }

  for (ServiceInfoTable::ServiceConfInfoIterator iterator
       = service_conf_info_list_.begin();
       iterator != service_conf_info_list_.end();
       ++iterator) {
    if (service_name == (*iterator)->name) {
      return (*iterator);
    }
  }

  return NULL;
}

ServiceInfoTablePtr ServiceInfoTable::Instance() {

  if (NULL == service_info_table_ptr_) {
    AutoLockOper lock(light_lock_ptr_.get());
    service_info_table_ptr_ = ServiceInfoTablePtr(new ServiceInfoTable());
  }

  return service_info_table_ptr_;
}

}
