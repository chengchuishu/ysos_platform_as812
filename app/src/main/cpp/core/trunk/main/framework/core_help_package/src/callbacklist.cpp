/**
 *@file CallbackList.cpp
 *@brief Definition of CallbackList
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-26 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
/*
#include "../../../Public/include/core_help_package/callbacklist.h"

namespace ysos {

CallbackList::CallbackList() {

}

CallbackList::~CallbackList() {
  Clear();
}

int CallbackList::AddCallback(CallbackInterfacePtr callback_ptr) {
  if(callback_list_.end() != std::find(callback_list_.begin(), callback_list_.end(), callback_ptr)) {
    return YSOS_ERROR_HAS_EXISTED;
  }

  callback_list_.push_back(callback_ptr);

  return YSOS_ERROR_SUCCESS;
}

int CallbackList::RemoveCallback(CallbackInterfacePtr callback_ptr) {
  std::list<CallbackInterfacePtr>::iterator it = std::find(callback_list_.begin(), callback_list_.end(), callback_ptr);
  if(it != callback_list_.end()) {
    callback_list_.erase(it);
  }

  return YSOS_ERROR_SUCCESS;
}

void CallbackList::Clear(void) {
  callback_list_.clear();
}


int CallbackList::Notify(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, void *context) {
  std::list<CallbackInterfacePtr>::iterator it = callback_list_.begin();
  for(; it !=callback_list_.end(); ++it) {
    (*it)->Callback(input_buffer, output_buffer, context);
  }

  return YSOS_ERROR_SUCCESS;
}

} */  // namespace ysos
