/**
*@file BufferWrapImpl.h
*@brief Definition of BufferWrapImpl
*@date Created on: 2016-05-06 13:59:20
*@author Original author: XuLanyue
*@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
* 
*/

#ifndef CHP_BUFFER_WRAP_IMPL_H                                                  //NOLINT
#define CHP_BUFFER_WRAP_IMPL_H                                                  //NOLINT

/// ysos private headers //  NOLINT
#include "../../../public/include/os_hal_package/lock.h"
#include "../../../public/include/sys_interface_package/common.h"
#include "../../../public/include/sys_interface_package/bufferpool.h"
/// stl headers //  NOLINT
#include <map>
#include <string>
/// boost headers //  NOLINT
#include <boost/variant.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

namespace ysos {
   /**
   *@brief BufferWrap仅仅是对一个已有Buffer的包装,它不考虑这个被包装的Buffer资源的释放    //NOLINT
   */

#define IMPLEMENT_BUFFER_WRAP_INTERFACE  \
public:  \
virtual int GetBufferAndLength(UINT8** buffer, UINT32 *length) { \
  if (NULL != buffer && length > 0) { \
    *buffer = data_ + prefix_len_; \
    *length = cur_data_len_; \
    return YSOS_ERROR_SUCCESS; \
  } else { \
    return YSOS_ERROR_FAILED; \
  } \
}  \
virtual int GetMaxLength(UINT32 *length) { \
  if (NULL != length) { \
    *length = max_data_len_; \
    return YSOS_ERROR_SUCCESS; \
  } else { \
    return YSOS_ERROR_FAILED; \
  } \
}  \
virtual int SetLength(UINT32 length, UINT32 prefix) { \
  if (length > 0 && (length + prefix <= max_data_len_)) { \
    cur_data_len_ = length; \
    prefix_len_ = prefix; \
    return YSOS_ERROR_SUCCESS; \
  } else { \
    return YSOS_ERROR_FAILED; \
  } \
}  \
virtual int GetPrefixLength(UINT32 *prefix) { \
  if (NULL != prefix) { \
    *prefix = prefix_len_; \
    return YSOS_ERROR_SUCCESS; \
  } else { \
    return YSOS_ERROR_FAILED; \
  } \
}  \
virtual int GetAllocator(BufferPoolInterfacePtr *allocator) { \
  return YSOS_ERROR_NOT_SUPPORTED; \
}  \
virtual int ResetAllocator(BufferPoolInterfacePtr allocator=NULL) { \
  return YSOS_ERROR_NOT_SUPPORTED; \
}\
private:  \
UINT8* data_; \
UINT32 cur_data_len_; \
UINT32 max_data_len_; \
UINT32 prefix_len_; \
bool is_delegate_;

class YSOS_EXPORT BufferWrapImpl : virtual public BufferWrapInterface {
 public:
  BufferWrapImpl(bool is_delegate=false): is_delegate_(is_delegate) {
    // YSOS_LOG_DEBUG("BufferWrapImpl()");
  }
  virtual ~BufferWrapImpl() {
    if(is_delegate_) {
      YSOS_DELETE(data_);
    }
    // YSOS_LOG_DEBUG("~BufferWrapImpl()");
  }

  virtual int SetBuffer(UINT8 *pBuffer, UINT32 iLength) {
    if (NULL != pBuffer && iLength > 0) {
      data_ = pBuffer;
      max_data_len_ = iLength;
      cur_data_len_ = iLength;
      prefix_len_ = 0;
      return YSOS_ERROR_SUCCESS;
    } else {
      return YSOS_ERROR_FAILED;
    }
  }
IMPLEMENT_BUFFER_WRAP_INTERFACE
};

}   // namespace ysos

#endif  // BUFFER_WRAP_IMPL_H                                               //NOLINT
