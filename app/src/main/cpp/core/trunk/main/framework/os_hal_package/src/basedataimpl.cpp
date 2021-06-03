/*
# basedataimpl.cpp
# Definition of BaseDataImpl
# Created on: 2016-11-23 17:20:13
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 201601123, created by JinChengZhe
*/

/// Private Headers
#include "../../../protect/include/os_hal_package/basedataimpl.h"
/// C++ Standard Headers
#include <algorithm>
/// Platform Headers
#include "../../../public/include/core_help_package/utility.h"

namespace ysos {

BaseDataImpl::BaseDataImpl(const std::string &strClassName /* =BaseDataImpl */) : BaseInterfaceImpl(strClassName) {

}

BaseDataImpl::~BaseDataImpl() {
  if (0 < data_map_.size()) {
    data_map_.clear();
  }
}

int BaseDataImpl::GetData(const std::string& key, std::string& value) {
  int result = YSOS_ERROR_FAILED;

  {
    AutoLockOper auto_lock_oper(&ll_lock_);
    YSOS_LOG_DEBUG("get data_map_ size: " << data_map_.size());
    DataMapIterator data_map_iterator = data_map_.find(key);
    if (data_map_.end() != data_map_iterator) {
      value = data_map_iterator->second;
      result = YSOS_ERROR_SUCCESS;
    }
  }

  if (YSOS_ERROR_SUCCESS == result) {
    YSOS_LOG_DEBUG("get data success: " << key << " | " << value);
  } else {
    YSOS_LOG_ERROR("get data failed: " << key << " error code: " << result);
  }

  return result;
}

int BaseDataImpl::SetData(const std::string& key, const std::string& value) {
  int result = YSOS_ERROR_FAILED;

  {
    AutoLockOper auto_lock_oper(&ll_lock_);
//    data_map_.insert(std::make_pair<std::string, std::string>(key, value));
    data_map_[key] = value;
    result = YSOS_ERROR_SUCCESS;
  }

  if (YSOS_ERROR_SUCCESS == result) {
    YSOS_LOG_DEBUG("set data success: " << key << " | " << value);
  } else {
    YSOS_LOG_ERROR("set data failed: " << key << " error code: " << result);
  }

  return result;
}

int BaseDataImpl::DeleteData(const std::string& key) {
  int result = YSOS_ERROR_FAILED;

  {
    AutoLockOper auto_lock_oper(&ll_lock_);
    DataMapIterator data_map_iterator = data_map_.find(key);
    if (data_map_.end() != data_map_iterator) {
      data_map_.erase(data_map_iterator);
      result = YSOS_ERROR_SUCCESS;
    }
  }

  if (YSOS_ERROR_SUCCESS == result) {
    YSOS_LOG_DEBUG("delete data success: " << key);
  } else {
    YSOS_LOG_ERROR("delete data failed: " << key << " error code: " << result);
  }

  return result;
}

int BaseDataImpl::ClearAllData(void) {
  int result = YSOS_ERROR_FAILED;

  do {
    AutoLockOper auto_lock_oper(&ll_lock_);
    data_map_.clear();
    result = YSOS_ERROR_SUCCESS;
  } while (0);

  if (YSOS_ERROR_SUCCESS == result) {
    YSOS_LOG_DEBUG("clear all data success: ");
  } else {
    YSOS_LOG_ERROR("clear all data failed: " << result);
  }

  return result;
}

}
