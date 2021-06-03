/**
 *@file PreCallbackQueue.cpp
 *@brief Definition of PreCallbackQueue
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// Private Header //  NOLINT
#include "../../../protect/include/sys_framework_package/basemodulecallbackimpl.h"  // NOLINT
/// ysos private headers //  NOLINT
// #include "../../../public/include/core_help_package/ysos_log.h"
#include "../../../protect/include/sys_framework_package/precallbackqueue.h"  // NOLINT
#include "../../../public/include/sys_interface_package/ysos_enum.h"

namespace ysos {

PreCallbackQueue::PreCallbackQueue(const std::string& name):BaseInterfaceImpl(name) {  // NOLINT
  logger_ = GetUtility()->GetLogger("ysos.framework");
}

PreCallbackQueue::~PreCallbackQueue() {
  callback_map_.clear();
}

int PreCallbackQueue::AddCallback(CallbackInterfacePtr callback, INT64 owner_id, UINT32 priority) {  // NOLINT
  if (NULL == callback) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  CallbackIODataTypePair key;
  callback->GetProperty(PROP_UNIQUE_KEY, &key);
  if (callback_map_.end() != callback_map_.find(key)) {
    YSOS_LOG_DEBUG("add callback failed: " << callback->GetName());

    return YSOS_ERROR_INVALID_ARGUMENTS;
  }
  callback_map_.insert(std::make_pair(key, callback));

  return YSOS_ERROR_SUCCESS;
}

int PreCallbackQueue::RemoveCallback(CallbackInterfacePtr callback) {
  MapIterator it = callback_map_.begin();
  for (; it != callback_map_.end(); ++it) {
    if (it->second == callback) {
      break;
    }
  }

  if (it != callback_map_.end()) {
    callback_map_.erase(it);
  }

  return YSOS_ERROR_SUCCESS;
}

int PreCallbackQueue::RemoveByID(INT64 owner_id) {
  /// PreCallbackQueue里的Callback都是当前Module的，不需要owner_id指定 //  NOLINT
  callback_map_.clear();

  return YSOS_ERROR_SUCCESS;
}


bool PreCallbackQueue::FindCallback(CallbackInterfacePtr *callback, CallbackIODataTypePair key) {  // NOLINT
  if (callback_map_.end() == callback_map_.find(key)) {
    return false;
  }
  *callback = callback_map_[key];

  return true;
}

CallbackInterfacePtr PreCallbackQueue::FindCallback(bool first_not_next) {
  if (first_not_next) {
    cur_iterator_ = callback_map_.begin();
  } else {
    if (cur_iterator_ != callback_map_.end()) {
      cur_iterator_++;
    }
  }

  if (cur_iterator_ != callback_map_.end()) {
    return cur_iterator_->second;
  }

  return NULL;
}
}  // NOLINT