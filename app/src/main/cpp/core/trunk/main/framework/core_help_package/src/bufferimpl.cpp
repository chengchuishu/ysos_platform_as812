/**
 *@file BufferImpl.cpp
 *@brief Definition of BufferImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// ysos private headers //  NOLINT
#include "../../../protect/include/core_help_package/bufferimpl.h"  //  NOLINT
/// boost headers //  NOLINT
#include <boost/function.hpp>  //  NOLINT
/// for debug
//#include "../../../public/include/core_help_package/utility.h"  //  NOLINT

namespace ysos {
/*
    @brief 构造函数  //  NOLINT
     初始化时，只有max_size  //  NOLINT
*/
BufferImpl::BufferImpl(BufferPoolInterfacePtr allocate, const UINT32 max_size):  //  NOLINT
  cur_data_(NULL), cur_data_len_(max_size), max_data_len_(max_size), is_delegate_(true) {  //  NOLINT
  allocator_ = allocate;
  max_data_ = new uint8_t[max_size+1];
  if (NULL == max_data_) {
    //  return YSOS_ERROR_MEMORY_ALLOCATED_FAILED;
  }
  cur_data_ = max_data_;

  std::memset(max_data_, 0, max_size+1);
}

BufferImpl::BufferImpl(uint8_t *data_ptr, const UINT32 data_length, bool is_delegate):  //  NOLINT
  cur_data_(data_ptr), max_data_(data_ptr), cur_data_len_(data_length),
  max_data_len_(data_length), is_delegate_(is_delegate) {
}

BufferImpl::~BufferImpl() {
  if (!is_delegate_) {
    return;
  }
  delete []max_data_;
  max_data_ = NULL;
  allocator_ = NULL;
}

int BufferImpl::GetBufferAndLength(UINT8** buffer, UINT32 *length) {  //  NOLINT
  CHECK_ARGUMENT_IF_NULL(buffer);
  CHECK_ARGUMENT_IF_NULL(length);

  *buffer = cur_data_;
  *length = cur_data_len_;  //  cur_data_len_;

  return YSOS_ERROR_SUCCESS;
}

int BufferImpl::GetMaxLength(UINT32 *length) {
  CHECK_ARGUMENT_IF_NULL(length);

  *length = max_data_len_;

  return YSOS_ERROR_SUCCESS;
}

int BufferImpl::SetLength(UINT32 length, UINT32 prefix) {
  if ((length + prefix) > max_data_len_) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  cur_data_len_ = length;
  cur_data_ = max_data_ + prefix;

  return YSOS_ERROR_SUCCESS;
}

int BufferImpl::GetPrefixLength(UINT32 *prefix) {
  CHECK_ARGUMENT_IF_NULL(prefix);
  *prefix = cur_data_ - max_data_;

  return YSOS_ERROR_SUCCESS;
}

int BufferImpl::GetAllocator(BufferPoolInterfacePtr *allocator) {  //  NOLINT
  CHECK_ARGUMENT_IF_NULL(allocator);
  *allocator = allocator_;

  return 0;
}

int BufferImpl::ResetAllocator(BufferPoolInterfacePtr allocator) {
  allocator_ = allocator;

  return YSOS_ERROR_SUCCESS;
}

void DeleteBufferFunction::operator()(BufferInterface* buffer_ptr) {
  assert(NULL != buffer_ptr);
  BufferPoolInterfacePtr allocator_ptr;
  int ret = buffer_ptr->GetAllocator(&allocator_ptr);
  assert(0 == ret);
  if (NULL == allocator_ptr) {
    YSOS_DELETE(buffer_ptr);
    return;
  }

  /// for debug
//   UINT32 buffer_length = 0;
//   if (YSOS_ERROR_SUCCESS == buffer_ptr->GetMaxLength(&buffer_length)) {
//     if (9216 == buffer_length || 8192 == buffer_length) {
//       YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.sdk"), "!!!Release Buffer[buffer_ptr][" << reinterpret_cast<UINT64>(buffer_ptr) << "]");
//     }
//   }

  ret = allocator_ptr->ReleaseBuffer(buffer_ptr);
  if (YSOS_ERROR_SUCCESS != ret) {
    YSOS_DELETE(buffer_ptr);
  }
}
}  //  NOLINT
