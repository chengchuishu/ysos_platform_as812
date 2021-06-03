/*
# DataInterfaceManager.cpp
# Definition of DataInterfaceManager
# Created on: 2016-11-23 17:20:13
# Original author: dhongqian
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 201601123, created by dhongqian
*/

/// Private Headers
#include "../../../protect/include/os_hal_package/datainterfacemanager.h"
/// C++ Standard Headers
#include <algorithm>
/// Platform Headers
#include "../../../public/include/core_help_package/utility.h"
#include "../../../protect/include/os_hal_package/basedataimpl.h"


namespace ysos {

DEFINE_SINGLETON(DataInterfaceManager);
DataInterfaceManager::DataInterfaceManager(const std::string &strClassName /* =BaseDataImpl */) : BaseInterfaceImpl(strClassName) {

}

DataInterfaceManager::~DataInterfaceManager() {
  if (0 < data_map_.size()) {
    data_map_.clear();
  }
}

// const DataInterfaceManagerPtr DataInterfaceManager::Instance(void) {
//   if (NULL == s_instance_) {
//     AutoLockOper lock(s_lock_.get());
//     if (NULL == s_instance_) {
//       s_instance_ = DataInterfaceManagerPtr(new DataInterfaceManager("DataInterfaceManager"));
//     }
//   }
// 
//   return s_instance_;
// }

DataInterfacePtr DataInterfaceManager::GetData(const std::string &data_key) {
  if (data_key.empty()) {
    return NULL;
  }

  AutoLockOper lock(&data_map_lock_);
  DataInterfaceMap::iterator it = data_map_.find(data_key);
  if (data_map_.end() != it) {
    return it->second;
  }

  DataInterface *data_ptr = BaseDataImpl::CreateInstance();
  if (NULL == data_ptr) {
    return NULL;
  }

  DataInterfacePtr data_interface_ptr(data_ptr);
  data_map_.insert(std::make_pair(data_key, data_interface_ptr));
  YSOS_LOG_DEBUG("create a new data interface: " << data_key);

  return data_interface_ptr;
}

std::string DataInterfaceManager::GetData(const std::string &data_key, const std::string &key) {
  if (data_key.empty() || key.empty()) {
    return "";
  }

  DataInterfacePtr data_ptr = GetData(data_key);
  if (NULL == data_ptr) {
    return "";
  }

  std::string value;
  int ret = data_ptr->GetData(key, value);
  if (YSOS_ERROR_SUCCESS != ret) {
    return "";
  }

  return value;
}

int DataInterfaceManager::DeleteData(const std::string &data_key) {
  if (data_key.empty()) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  AutoLockOper lock(&data_map_lock_);
  DataInterfaceMap::iterator it = data_map_.find(data_key);
  if (data_map_.end() == it) {
    return YSOS_ERROR_NOT_EXISTED;
  }
  data_map_.erase(it);
  YSOS_LOG_DEBUG("delete a data interface: " << data_key);

  return YSOS_ERROR_SUCCESS;
}

int DataInterfaceManager::DeleteData(const std::string &data_key, const std::string &key) {
  if (data_key.empty() || key.empty()) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  DataInterfacePtr data_ptr = GetData(data_key);
  if (NULL == data_ptr) {
    return YSOS_ERROR_NOT_EXISTED;
  }

  return data_ptr->DeleteData(key);
}

}
