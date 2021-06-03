/*
# MessageImpl.cpp
# Implement of MsgImpl and MessageQueueImpl
# Created on: 2016-04-27 13:59:20
# Original author: dhongqian
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
*/

/// Self Header
#include "../../../protect/include/os_hal_package/msgimpl.h"
/// Stl Headers
#include <map>
#include <list>
#include <queue>
/// Ysos Headers
#include "../../../public/include/sys_interface_package/callbackqueue.h"
#include "../../../protect/include/core_help_package/bufferimpl.h"

namespace ysos {
MsgImpl::MsgImpl(const UINT32 max_size, const uint32_t message_id, BufferPoolInterfacePtr allocate) {
  msg_id_ = message_id;
  if(0 == max_size) {
    return;
  } 

  msg_body_ptr_ = BufferInterfacePtr(new BufferImpl(allocate, max_size));
  source_callback_ptr_ = destination_callback_ptr_ = NULL;
}

MsgImpl::MsgImpl(const uint32_t message_id, BufferInterfacePtr message_body) {
  msg_id_ = message_id;
  msg_body_ptr_ = message_body;
  source_callback_ptr_ = destination_callback_ptr_ = NULL;
}

MsgImpl::MsgImpl(uint8_t *data_ptr, const UINT32 data_length, bool is_delegate) {
  msg_id_ = 0;
  msg_body_ptr_ = BufferInterfacePtr(new BufferImpl(data_ptr, data_length, is_delegate));
  source_callback_ptr_ = destination_callback_ptr_ = NULL;
}

MsgImpl::~MsgImpl() {
  source_callback_ptr_ = destination_callback_ptr_ = NULL;
  msg_body_ptr_ = NULL;
}

uint32_t MsgImpl::GetMessageID(void) {
  return msg_id_;
}

int MsgImpl::SetMessageID(const uint32_t &message_id) {
  msg_id_ = message_id;

  return YSOS_ERROR_SUCCESS;
}

BufferInterfacePtr MsgImpl::GetMessageBody(void) {
  return msg_body_ptr_;
}

int MsgImpl::SetMessageBody(BufferInterfacePtr message_body_ptr) {
  msg_body_ptr_ = message_body_ptr;

  return YSOS_ERROR_SUCCESS;
}

int MsgImpl::SetCalllback(CallbackInterfacePtr source_callback_ptr, CallbackInterfacePtr destination_callback_ptr) {
  source_callback_ptr_ = source_callback_ptr;
  destination_callback_ptr_ = destination_callback_ptr;

  return YSOS_ERROR_SUCCESS;
}

int MsgImpl::RunSourceCallback(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr, void *contex_ptr) {
  if(NULL == source_callback_ptr_) {
    return YSOS_ERROR_NOT_SUPPORTED;
  }

  return source_callback_ptr_->Callback(input_buffer_ptr, output_buffer_ptr, contex_ptr);
}

int MsgImpl::RunDestinationCallback(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr, void *contex_ptr) {
  if(NULL == destination_callback_ptr_) {
    return YSOS_ERROR_NOT_SUPPORTED;
  }

  return destination_callback_ptr_->Callback(input_buffer_ptr, output_buffer_ptr, contex_ptr);
}

int MsgImpl::GetBufferAndLength(UINT8** buffer, UINT32 *length) {
  if(NULL == msg_body_ptr_) {
    return YSOS_ERROR_NOT_SUPPORTED;
  }

  return msg_body_ptr_->GetBufferAndLength(buffer, length);
}

int MsgImpl::GetMaxLength(UINT32 *length) {
  if(NULL == msg_body_ptr_) {
    return YSOS_ERROR_NOT_SUPPORTED;
  }

  return msg_body_ptr_->GetMaxLength(length);
}

int MsgImpl::SetLength(UINT32 length, UINT32 prefix) {
  if(NULL == msg_body_ptr_) {
    return YSOS_ERROR_NOT_SUPPORTED;
  }

  return msg_body_ptr_->SetLength(length, prefix);
}

int MsgImpl::GetPrefixLength(UINT32 *prefix) {
  if(NULL == msg_body_ptr_) {
    return YSOS_ERROR_NOT_SUPPORTED;
  }

  return msg_body_ptr_->GetPrefixLength(prefix);
}

int MsgImpl::GetAllocator(BufferPoolInterfacePtr *allocator) {
  if(NULL == msg_body_ptr_) {
    return YSOS_ERROR_NOT_SUPPORTED;
  }

  return msg_body_ptr_->GetAllocator(allocator);
}

int MsgImpl::ResetAllocator(BufferPoolInterfacePtr allocator) {
  if(NULL == msg_body_ptr_) {
    return YSOS_ERROR_NOT_SUPPORTED;
  }

  return msg_body_ptr_->ResetAllocator(allocator);
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////         MsgPool                        ///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
MsgPool::MsgPool(const std::string &instance_name /* = "MsgPool" */) {

}

MsgPool::~MsgPool() {
  msg_list_.clear();
}

int MsgPool::Push(MsgInterfacePtr message_ptr) {
  AutoLockOper lock(&msg_list_lock_);
  msg_list_.push_back(message_ptr);

  return YSOS_ERROR_SUCCESS;
}

MsgInterfacePtr MsgPool::Get() {
  return Peek(PROP_PM_REMOVE);
}

MsgInterfacePtr MsgPool::Peek(int remove_flag /* = PM_NoRemove */) {
  AutoLockOper lock(&msg_list_lock_);
  if(msg_list_.empty()) {
    return NULL;
  }

  MsgInterfacePtr msg_ptr = *(msg_list_.begin());
  if(PROP_PM_REMOVE == remove_flag) {
    msg_list_.pop_front();
  }

  return msg_ptr;
}

int MsgPool::Clear() {
  AutoLockOper lock(&msg_list_lock_);
  msg_list_.clear();

  return YSOS_ERROR_SUCCESS;
}

MsgInterfacePtr MsgPool::Find(int find_type, const BufferInterfacePtr& message_data) {
  AutoLockOper lock(&msg_list_lock_);

  return NULL;
}

int MsgPool::GetMessageAmount() {
  AutoLockOper lock(&msg_list_lock_);

  return msg_list_.size();
}


///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////         MsgQueue                       ///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
MsgQueueImpl::MsgQueueImpl(const std::string &strClassName /* =MsgQueueImpl */): BaseInterfaceImpl(strClassName) {

}

MsgQueueImpl::~MsgQueueImpl() {

}

int MsgQueueImpl::Push(MsgInterfacePtr message_ptr, int message_id) {
  if(NULL == message_ptr) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  AutoLockOper lock(&msg_map_lock_);

  MsgPoolPtr msg_pool;
  MessageMap::iterator it = msg_map_.find(message_id);
  if(msg_map_.end() == it) {
    msg_pool = boost::make_shared<MsgPool>();
    assert(NULL != msg_pool);
    msg_map_[message_id] = msg_pool;
  } else {
    msg_pool = it->second;
  }

  return msg_pool->Push(message_ptr);
}

/**
  *@brief 从消息队列中根据消息ID获得消息  // NOLINT
  *@param message_ptr[Output]： 输出的消息  // NOLINT
  *@param message_id[Input]：可根据消息ID来获得消息，默认值为MessageAny  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
int MsgQueueImpl::Get(MsgInterfacePtr &message_ptr, int message_id) {
  AutoLockOper lock(&msg_map_lock_);

  MsgPoolPtr msg_pool;
  MessageMap::iterator it = msg_map_.find(message_id);
  if(msg_map_.end() == it) {
    return YSOS_ERROR_NOT_EXISTED;
  }
  msg_pool = it->second;

  message_ptr = msg_pool->Get();
  if(NULL == message_ptr) {
    return YSOS_ERROR_NOT_EXISTED;
  }

  return YSOS_ERROR_SUCCESS;
}

/**
  *@brief 从消息队列中根据消息ID获得消息  // NOLINT
  *@param message_ptr[Output]： 输出的消息  // NOLINT
  *@param message_id[Input]：可根据消息ID来获得消息，默认值为MessageAny  // NOLINT
  *@param remove_flag[Input]：取完消息后是否移除消息，默认值为不移除PM_NoRemove  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
int MsgQueueImpl::Peek(MsgInterfacePtr &message_ptr, int message_id, int remove_flag) {
    AutoLockOper lock(&msg_map_lock_);

    MsgPoolPtr msg_pool;
    MessageMap::iterator it = msg_map_.find(message_id);
    if(msg_map_.end() == it) {
      return YSOS_ERROR_NOT_EXISTED;
    }
    msg_pool = it->second;

    message_ptr = msg_pool->Peek(remove_flag);
    if(NULL == message_ptr) {
      return YSOS_ERROR_NOT_EXISTED;
    }

    return YSOS_ERROR_SUCCESS;
}

int MsgQueueImpl::GetMessageAmount() {
  AutoLockOper lock(&msg_map_lock_);

  int total_count = 0;
  MsgPoolPtr msg_pool;
  MessageMap::iterator it = msg_map_.begin();
  for(; msg_map_.end() != it; ++it) {
    msg_pool = it->second;
    total_count += msg_pool->GetMessageAmount();
  }

  return total_count;
}

/**
  *@brief 将消息发送出去  // NOLINT
  *@param message_ptr[Input]： 输入的消息  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
int MsgQueueImpl::Dispose(MsgInterfacePtr message_ptr) {
  return YSOS_ERROR_NOT_SUPPORTED;
}

/**
  *@brief 从队列中根据消息ID清除消息, 若为MessageAny则清除全部信息  // NOLINT
  *@param message_id[Input]： 输入的消息ID  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
int MsgQueueImpl::Clear(int message_id) {
  AutoLockOper lock(&msg_map_lock_);

  MsgPoolPtr msg_pool;
  MessageMap::iterator it = msg_map_.find(message_id);
  if(msg_map_.end() == it) {
    return YSOS_ERROR_NOT_EXISTED;
  }
  msg_pool = it->second;

  return msg_pool->Clear();
}

/**
  *@brief 从消息队列中根据消息ID获得消息  // NOLINT
  *@param message_ptr[Output]： 输出的消息  // NOLINT
  *@param find_type[Input]：消息的查找方式  // NOLINT
  *@param message_data[Input]：消息查找时需要的查找信息  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
int MsgQueueImpl::Find(MsgInterfacePtr &message_ptr, int find_type, const BufferInterfacePtr& message_data) {
  AutoLockOper lock(&msg_map_lock_);

//   MsgPoolPtr msg_pool;
//   MessageMap::iterator it = msg_map_.find(message_id);
//   if(msg_map_.end() == it) {
//     return YSOS_ERROR_NOT_EXISTED;
//   }
//   msg_pool = it->second;
// 
//   message_ptr = msg_pool->Find(find_type, message_data);
//   if(NULL == message_ptr) {
//     return YSOS_ERROR_NOT_EXISTED;
//   }

  return YSOS_ERROR_NOT_SUPPORTED;
}

}   // namespace ysos
