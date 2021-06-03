/**
 *@file NextCallbackQueue.cpp
 *@brief Definition of NextCallbackQueue
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// ysos private headers //  NOLINT
#include "../../../protect/include/sys_framework_package/nextcallbackqueue.h"  // NOLINT
#include "../../../protect/include/sys_framework_package/basemodulecallbackimpl.h"
#include "../../../public/include/sys_interface_package/ysos_enum.h"

namespace ysos {
NextCallbackQueue::NextCallbackQueue(const std::string& name):BaseInterfaceImpl(name) {  // NOLINT
}

NextCallbackQueue::~NextCallbackQueue() {
  callback_queue_.clear();
}

int NextCallbackQueue::AddCallback(CallbackInterfacePtr callback, INT64 owner_id, UINT32 priority) {  // NOLINT
  if (NULL == callback) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  callback_queue_.Push(priority, callback);

  return YSOS_ERROR_SUCCESS;
}

int NextCallbackQueue::RemoveCallback(CallbackInterfacePtr callback) {
  CallbackQueueIterator it = callback_queue_.begin();
  for (; it != callback_queue_.end(); ++it) {
    const CallbackInterfacePtr next_callback = it->second;
    if (next_callback == callback) {
      break;
    }
  }

  if (it != callback_queue_.end()) {
    callback_queue_.erase(it);
  }

  return YSOS_ERROR_SUCCESS;
}

/**
 * 如下图所示，
 *                ----> MB
 *    MA（Ca）    ----> MB'     Cb
 *                ----> MB''
 * Cb的owner_id_map中存储的Key为：(MA,MB), (MA, MB'), (MA, MB'')
 * 当A删除MB的NextCallback时，传的owner_id为MB的owner_id,因此CB中只要找second为MB的，即可删除，
 * 因为MB只能有一个前链接。
 * 为满足扩展性，保留key MA
 */
bool NextCallbackQueue::IsMatchedCallback(CallbackInterfacePtr callback, INT64 owner_id, bool is_delete) {
  BaseModuleCallbackImpl::BaseModuleCallbackImplMap *owner_id_map = NULL;
  int ret = callback->GetProperty(PROP_OWNER_ID, &owner_id_map);
  assert(YSOS_ERROR_SUCCESS == ret);
  for(BaseModuleCallbackImpl::BaseModuleCallbackImplMap::iterator map_it=owner_id_map->begin();
    map_it != owner_id_map->end(); ++map_it){
      if(map_it->second == owner_id) {
        if(is_delete) {
          owner_id_map->erase(map_it);
        }

        return true;
      }
  }

  return false;
}

int NextCallbackQueue::RemoveByID(INT64 owner_id) {
  if(0 == owner_id) {
    callback_queue_.clear();
    return YSOS_ERROR_SUCCESS;
  }

  CallbackQueueIterator it = callback_queue_.begin();
  for (; it != callback_queue_.end();) {
    const CallbackInterfacePtr callback = it->second;

    if(IsMatchedCallback(callback, owner_id, true))
    {
      it = callback_queue_.erase(it);
      continue;
    }
    ++it;
  }

  return YSOS_ERROR_SUCCESS;
}


bool NextCallbackQueue::FindCallback(CallbackInterfacePtr *callback, CallbackIODataTypePair key) {  // NOLINT
  return false;
}

CallbackInterfacePtr NextCallbackQueue::FindCallback(bool first_not_next) {
  if (first_not_next) {
    cur_queue_it_ = callback_queue_.begin();
  } else {
//#if 1 // 0, normal; 1, only for test by XuLanyue
//    if (cur_queue_it_ == callback_queue_.end()) {
//      return NULL;
//    }
//#endif
    ++cur_queue_it_;
  }

  if (cur_queue_it_ == callback_queue_.end()) {
    return NULL;
  }

  return cur_queue_it_->second;
}
}  // NOLINT