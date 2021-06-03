/**
 *@file BufferUtility.cpp
 *@brief Definition of BufferUtility
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// ysos private headers //  NOLINT
#include "../../../public/include/os_hal_package/bufferutility.h"
#include "../../../public/include/sys_interface_package/common.h"
#include "../../../protect/include/core_help_package/bufferimpl.h"
/*#include "../../../protect/include/core_help_package/bufferwrapimpl.h"*/
// #include "../../../protect/include/os_hal_package/messageimpl.h"
#include "../../../public/include/core_help_package/utility.h"
#include "../../../protect/include/core_help_package/bufferpoolimpl.h"
/// c headers //  NOLINT
#include <cstring>
#include <cstdio>
#include <cstdlib>

namespace ysos {

DEFINE_SINGLETON(BufferUtility);
BufferUtility::~BufferUtility() {
}

BufferUtility::BufferUtility(const std::string &strClassName): BaseInterfaceImpl(strClassName) {
}

BufferPoolInterfacePtr BufferUtility::CreateBufferPool(const uint32_t max_buffer_size, const int buffer_number, const uint32_t prefix_size) {
  AllocatorProperties allocate_properties;
  allocate_properties.cbAlign = 0;
  allocate_properties.cbBuffer = max_buffer_size;
  allocate_properties.cbPrefix = prefix_size;
  allocate_properties.cBuffers = buffer_number;

  AllocatorProperties actual_allocate_properties;
  BufferPoolInterfacePtr buffer_pool_ptr_ = BufferPoolInterfacePtr(new BufferPoolImpl<BufferImpl>());
  int ret = buffer_pool_ptr_->SetProperties(&allocate_properties, &actual_allocate_properties);
  if (YSOS_ERROR_SUCCESS != ret) {
    return NULL;
  }

  ret = buffer_pool_ptr_->Commit();
  if (YSOS_ERROR_SUCCESS != ret) {
    buffer_pool_ptr_->Decommit();
    return NULL;
  }

  return buffer_pool_ptr_;
}

BufferInterfacePtr BufferUtility::GetBufferFromBufferPool(const BufferPoolInterfacePtr &bufffer_pool_ptr) {
  if (NULL == bufffer_pool_ptr) {
    return NULL;
  }

  BufferInterfacePtr buffer_ptr;
  int ret = bufffer_pool_ptr->GetBuffer(&buffer_ptr);
  if (YSOS_ERROR_SUCCESS != ret) {
    return NULL;
  }

  return buffer_ptr;
}

int BufferUtility::InitialBuffer(const BufferInterfacePtr &buffer_ptr, const char initial_value) {
  uint32_t buffer_data_length = GetBufferLength(buffer_ptr);
  uint8_t* buffer_data_ptr = GetBufferData(buffer_ptr);
  if (NULL == buffer_data_ptr) {
    return YSOS_ERROR_FAILED;
  }

  std::memset(buffer_data_ptr, initial_value, buffer_data_length);
//  assert(YSOS_ERROR_SUCCESS== SetBufferLength(buffer_ptr, 0));

  return YSOS_ERROR_SUCCESS;
}

BufferInterfacePtr BufferUtility::CreateBuffer(const int buffer_length, const char initial_value) {
  if (0 == buffer_length) {
    return NULL;
  }

  uint8_t *data_ptr = new uint8_t[buffer_length];
  if (NULL == data_ptr) {
    YSOS_LOG_DEBUG("no sufficient memory");
    return NULL;
  }
  std::memset(data_ptr, initial_value, buffer_length);

  return WrapBuffer(data_ptr, buffer_length, true);
}

BufferInterfacePtr BufferUtility::CreateBuffer(const std::string &str) {
  if (str.empty()) {
    return NULL;
  }

  int buffer_length = str.length() + 1;
  uint8_t *data_ptr = new uint8_t[buffer_length];
  if (NULL == data_ptr) {
    YSOS_LOG_DEBUG("no sufficient memory");
    return NULL;
  }
  std::memcpy(data_ptr, str.c_str(), buffer_length);

  return WrapBuffer(data_ptr, buffer_length, true);
}

BufferInterfacePtr BufferUtility::CloneBuffer(const BufferInterfacePtr &src_buffer_ptr) {
  int buffer_length = GetBufferLength(src_buffer_ptr);
  int buffer_prefix = GetBufferPrefixLength(src_buffer_ptr);
  int buffer_max_length = GetBufferMaxLength(src_buffer_ptr);
  assert(buffer_max_length >= (buffer_prefix + buffer_length));

  int ret = src_buffer_ptr->SetLength(buffer_prefix+buffer_length);
  assert(YSOS_ERROR_SUCCESS == ret);
  uint8_t *buffer_data = GetBufferData(src_buffer_ptr);
  if (NULL == buffer_data) {
    return NULL;
  }
  int clone_max_length = buffer_prefix + buffer_length + 1;
  uint8_t *data_ptr = new uint8_t[clone_max_length];
  if (NULL == data_ptr) {
    YSOS_LOG_DEBUG("no sufficient memory");
    return NULL;
  }
  memcpy(data_ptr, buffer_data, buffer_length + buffer_prefix);
  data_ptr[clone_max_length-1] = '\0';

  ret = src_buffer_ptr->SetLength(buffer_length, buffer_prefix);
  assert(YSOS_ERROR_SUCCESS == ret);

  BufferInterfacePtr dst_buffer_ptr = WrapBuffer(data_ptr, clone_max_length, true);
  ret = dst_buffer_ptr->SetLength(buffer_length, buffer_prefix);
  assert(YSOS_ERROR_SUCCESS == ret);

  return dst_buffer_ptr;
}

BufferInterfacePtr BufferUtility::WrapBuffer(uint8_t *&buffer, const int buffer_length, bool is_delegate) {
  if (NULL == buffer || 0 == buffer_length) {
    return NULL;
  }

  BufferInterfacePtr buffer_ptr = BufferInterfacePtr(new BufferImpl(buffer, buffer_length, is_delegate));
  /*buffer_ptr->SetBuffer(buffer, buffer_length);*/

  return buffer_ptr;
}

uint8_t *BufferUtility::GetBufferData(const BufferInterfacePtr &buffer_ptr) {
  if (NULL == buffer_ptr) {
    return NULL;
  }

  uint8_t *data_ptr = NULL;
  uint32_t data_length = 0;
  int ret = buffer_ptr->GetBufferAndLength(&data_ptr, &data_length);
  assert(YSOS_ERROR_SUCCESS == ret);
  if (0 == data_length) {
    return NULL;
  }

  return data_ptr;
}

int BufferUtility::GetBufferPrefixLength(const BufferInterfacePtr &buffer_ptr) {
  if (NULL == buffer_ptr) {
    //return NULL;
    return YSOS_ERROR_INVALID_ARGUMENTS;  //need update for linux
  }

  uint32_t prefix_length = 0;
  int ret = buffer_ptr->GetPrefixLength(&prefix_length);
  assert(YSOS_ERROR_SUCCESS == ret);

  return prefix_length;
}

int BufferUtility::SetBufferPrefixLength(const BufferInterfacePtr &buffer_ptr, const uint32_t prefix_length) {
  if (NULL == buffer_ptr) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  uint32_t buffer_length = 0;
  uint8_t *buffer = NULL;
  int ret = buffer_ptr->GetBufferAndLength(&buffer, &buffer_length);
  assert(YSOS_ERROR_SUCCESS == ret);

  return buffer_ptr->SetLength(buffer_length, prefix_length);
}

int BufferUtility::GetBufferLength(const BufferInterfacePtr &buffer_ptr) {
  if (NULL == buffer_ptr) {
    //return NULL;
    return YSOS_ERROR_INVALID_ARGUMENTS;  //need update for linux
  }

  uint8_t *data_ptr = NULL;
  uint32_t data_length = 0;
  int ret = buffer_ptr->GetBufferAndLength(&data_ptr, &data_length);
  assert(YSOS_ERROR_SUCCESS == ret);

  return data_length;
}

int BufferUtility::GetBufferUsableLength(const BufferInterfacePtr &buffer_ptr) {
  if (NULL == buffer_ptr) {
    //return NULL;
    return YSOS_ERROR_INVALID_ARGUMENTS;  //need update for linux
  }

  uint32_t prefix_length = GetBufferPrefixLength(buffer_ptr);
  uint32_t max_length = GetBufferMaxLength(buffer_ptr);


  return (max_length - prefix_length);
}

int BufferUtility::GetBufferMaxLength(const BufferInterfacePtr &buffer_ptr) {
  if (NULL == buffer_ptr) {
    //return NULL;
    return YSOS_ERROR_INVALID_ARGUMENTS;  //need update for linux
  }

  uint32_t max_buffer_length = 0;
  int ret = buffer_ptr->GetMaxLength(&max_buffer_length);
  assert(YSOS_ERROR_SUCCESS == ret);

  return max_buffer_length;
}

int BufferUtility::SetBufferLength(const BufferInterfacePtr &buffer_ptr, const uint32_t buffer_length) {
  if (NULL == buffer_ptr) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  uint32_t buffer_prefix = 0;
  int ret = buffer_ptr->GetPrefixLength(&buffer_prefix);
  assert(YSOS_ERROR_SUCCESS == ret);
  return buffer_ptr->SetLength(buffer_length, buffer_prefix);
}

int BufferUtility::CopyStringToBuffer(const std::string &src, const BufferInterfacePtr &out_buffer_ptr) {
  if (NULL == out_buffer_ptr) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  UINT32 out_buffer_length = BufferUtility::Instance()->GetBufferUsableLength(out_buffer_ptr);
  if (0 == out_buffer_length) {
    return YSOS_ERROR_FAILED;
  }

  if (out_buffer_length < src.length()+1) {
    return YSOS_ERROR_OUTOFMEMORY;
  }

  UINT8 *out_buffer_data_ptr = NULL;
  UINT32 data_buff_length = 0;
  out_buffer_ptr->GetBufferAndLength(&out_buffer_data_ptr, &data_buff_length);
  //UINT8* out_buffer_data_ptr = BufferUtility::Instance()->GetBufferData(out_buffer_ptr);
  if (NULL == out_buffer_data_ptr) {
    return YSOS_ERROR_MEMORY_ALLOCATED_FAILED;
  }

  std::memcpy(out_buffer_data_ptr, src.c_str(), src.length()+1);
  CHECK_FINISH(SetBufferLength(out_buffer_ptr, src.length()+1));

  return YSOS_ERROR_SUCCESS;
}

// MessageInterfacePtr BufferUtility::CreateMessage(MessageHeadPtr &message_head_ptr, MessageDataPtr &message_data_ptr) {
//   if (NULL == message_head_ptr || NULL == message_data_ptr) {
//     return NULL;
//   }
//
//   MessageInterfacePtr message_ptr = MessageInterfacePtr(new MessageImpl());
//   assert(YSOS_ERROR_SUCCESS == message_ptr->WriteItem(0, 0, &message_head_ptr));
//   assert(YSOS_ERROR_SUCCESS == message_ptr->WriteItem(1, 0, &message_data_ptr));
//
//   return message_ptr;
// }
//
// MessageHeadPtr BufferUtility::GetMessageHead(const MessageInterfacePtr &message_ptr) {
//   return GetMessageHead(message_ptr.get());
// }
//
// MessageHeadPtr BufferUtility::GetMessageHead(MessageInterface *message_ptr) {
//   if (NULL == message_ptr) {
//     return NULL;
//   }
//
//   MessageHeadPtr message_head_ptr;
//   assert(YSOS_ERROR_SUCCESS == message_ptr->ReadItem(0, 0, &message_head_ptr));
//
//   return message_head_ptr;
// }
//
// MessageDataPtr BufferUtility::GetMessageBody(const MessageInterfacePtr &message_ptr) {
//   return GetMessageBody(message_ptr.get());
// }
//
// MessageDataPtr BufferUtility::GetMessageBody(MessageInterface *message_ptr) {
//   if (NULL == message_ptr) {
//     return NULL;
//   }
//
//   MessageDataPtr message_data_ptr;
//   assert(YSOS_ERROR_SUCCESS == message_ptr->ReadItem(1, 0, &message_data_ptr));
//
//   return message_data_ptr;
// }
//
// BufferInterfacePtr BufferUtility::GetDataFromMessageBody(const MessageInterfacePtr &message_ptr) {
//   return GetDataFromMessageBody(message_ptr.get());
// }
//
// BufferInterfacePtr BufferUtility::GetDataFromMessageBody(MessageInterface *message_ptr) {
//   if (NULL == message_ptr) {
//     return NULL;
//   }
//
//   MessageDataPtr msg_data = GetBufferUtility()->GetMessageBody(message_ptr);
//   if (NULL == msg_data) {
//     return NULL;
//   }
//
//   //BufferInterfacePtr buffer_ptr = msg_data->message_data_;
//   /*if (NULL == buffer_ptr) {
//   return NULL;
//   }
//
//   uint8_t *data = GetBufferUtility()->GetBufferData(buffer_ptr);
//   uint32_t data_len = GetBufferUtility()->GetBufferLength(buffer_ptr);
//
//   if (0 == data_len) {
//   return NULL;
//   }*/
//
//   return msg_data->message_data_;
// }

MsgInterfacePtr BufferUtility::GetMessageFromQueue(const MsgQueueInterfacePtr &msg_queue_ptr) {
  MsgInterfacePtr msg_ptr;

  int ret = msg_queue_ptr->Get(msg_ptr);
  if (YSOS_ERROR_SUCCESS != ret) {
    return NULL;
  }

  return msg_ptr;
}
}
