/**
 *@file BufferPoolImpl.h
 *@brief Definition of BufferPoolImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef CHP_BUFFER_POOL_H_  // NOLINT
#define CHP_BUFFER_POOL_H_  // NOLINT

/// stl headers //  NOLINT
#include <list>
/// boost headers //  NOLINT
#include <boost/shared_ptr.hpp>   // NOLINT
#include <boost/thread/mutex.hpp>  // NOLINT
#include <boost/thread/lock_guard.hpp>  // NOLINT
#include <boost/foreach.hpp>
/// ysos private headers //  NOLINT
#include "../../../public/include/os_hal_package/lock.h"  // NOLINT
#include "../../../public/include/sys_interface_package/common.h"  // NOLINT
#include "../../../public/include/sys_interface_package/bufferpool.h"  // NOLINT
#include "../../../protect/include/core_help_package/bufferpoolimpl.h"
#include "../../../protect/include/core_help_package/bufferimpl.h"

namespace ysos {
class BufferImpl;

// typedef boost::lock_guard<boost::mutex> MutexLock;
// #define MUTEX_LOCK MutexLock lock(mutex_)

/**
 *@brief BufferPoolInterface的具体实现,支持BufferInterface的各种派生类
 */
template<typename T>
class /*YSOS_EXPORT*/ BufferPoolImpl : public BufferPoolInterface {
 public:
//  DECLARE_CREATEINSTANCE(BufferPoolImpl);

  BufferPoolImpl(void);
  virtual ~BufferPoolImpl();

  /**
   *@param pActual: 返回实际的值，可以为NULL  // NOLINT
   */
  virtual int SetProperties(AllocatorProperties *request, AllocatorProperties *actual);  // NOLINT
  virtual int GetProperties(AllocatorProperties *props);
  /**
   *@brief 正式分配内存，分配成功后，才可以使用  // NOLINT
   *@return:           成功返回实际的长度，否则返回0  // NOLINT
  */
  virtual int Commit() {
    AutoLockOper lock(&mutex_);
    // 已经Commit完，无须再commit  // NOLINT
    UINT32 i = 0;
    while (i++ < allocate_properties_.cBuffers) {
      DeleteBufferFunction delete_function;
      BufferInterfacePtr ptr(new T(this->shared_from_this(), allocate_properties_.cbBuffer + allocate_properties_.cbPrefix), delete_function);  // NOLINT
      free_list_.push_back(ptr);
    }

    return YSOS_ERROR_SUCCESS;
  }
  /*
   *@brief 释放管理的所有内存，在Commit前，不能提供内存申请  // NOLINT
   *@return:           成功返回实际的长度，否则返回0  // NOLINT
  */
  virtual int Decommit();
  /**
   *@brief 申请一块内存池。  // NOLINT
   *       只有在成功Commit后，才能申请成功  // NOLINT
   *@param ppBuffer: 申请到的内存  // NOLINT
   *@return:           成功返回实际的长度，否则返回0  // NOLINT
  */
  inline virtual int GetBuffer(BufferInterfacePtr *buffer);
  /**
   *@brief 将内存释放回内存池  // NOLINT
   *@param pBuffer: 申请到的内存  // NOLINT
   *@return:         成功返回实际的长度，否则返回0  // NOLINT
  */
  inline virtual int ReleaseBuffer(BufferInterfacePtr buffer);
  /**
   *@brief 将内存释放回内存池  // NOLINT
   *@param pBuffer: 申请到的内存  // NOLINT
   *@return:         成功返回实际的长度，否则返回0  // NOLINT
  */
  inline virtual int ReleaseBuffer(BufferInterface *buffer);

  void  DumpMemoryUsage(void) {}

 private:
  BufferPoolImpl(const BufferPoolImpl &buffer_pool_impl);
  BufferPoolImpl& operator=(const BufferPoolImpl &buffer_pool_impl);

 private:
  AllocatorProperties allocate_properties_;
  // BufferPoolInterfacePtr        pool_ptr_;
  MutexLock                     mutex_;   ///< lock when required or release a buffer  // NOLINT
  std::list<BufferInterfacePtr > free_list_;
  ///< 仅用来标示，是否是该BuffferPoool的Bufffer
  std::list<BufferInterface*>  used_list_;
  typedef std::list<BufferInterfacePtr>::iterator ListIterator;
};

/**
 *@brief shared_ptr默认析构类函数
 */
class DeleteBufferPool {
 public:
  void operator()(BufferPoolInterface* buffer_pool) {
  }
};

template<typename T> BufferPoolImpl<T>::BufferPoolImpl(void)  {
  std::memset(&allocate_properties_, 0, sizeof(allocate_properties_));

//   DeleteBufferPool delete_buffer_pool = delete_buffer_pool;
//   pool_ptr_ = BufferPoolInterfacePtr(this, delete_buffer_pool);
}

template<typename T> BufferPoolImpl<T>::~BufferPoolImpl() {
  Decommit();
}


template<typename T> int BufferPoolImpl<T>::SetProperties(AllocatorProperties *request, AllocatorProperties *actual) {
  assert(NULL!=request);
  // 现在pActual与pRequest都一致  // NOLINT
  std::memcpy(&allocate_properties_, request, sizeof(allocate_properties_));

  if (NULL != actual) {
    std::memcpy(actual, &allocate_properties_, sizeof(allocate_properties_));
  }

  return YSOS_ERROR_SUCCESS;
}

template<typename T>
int BufferPoolImpl<T>::GetProperties(AllocatorProperties *props) {
  assert(NULL!=props);
  std::memcpy(props, &allocate_properties_, sizeof(allocate_properties_));

  return YSOS_ERROR_SUCCESS;
}

/*
   @brief 释放管理的所有内存，在Commit前，不能提供内存申请  // NOLINT
   @return:           成功返回实际的长度，否则返回0  // NOLINT
*/
template<typename T>
int BufferPoolImpl<T>::Decommit() {
  AutoLockOper lock(&mutex_);

  std::list<BufferInterfacePtr >::iterator free_it = free_list_.begin();
  for (; free_list_.end() != free_it; ++free_it) {
    BufferInterfacePtr buffer_ptr = *free_it;
    buffer_ptr->ResetAllocator();
  }

  std::list<BufferInterface*>::iterator used_it = used_list_.begin();
  for (; used_list_.end() != used_it; ++used_it) {
    BufferInterface *buffer_ptr = *used_it;
    buffer_ptr->ResetAllocator();
  }

  free_list_.clear();
  used_list_.clear();
//   if (used_list_.size() > 0) {
//     return YSOS_ERROR_LOGIC_ERROR;
//   }

  return YSOS_ERROR_SUCCESS;
}

/*
   @brief 申请一块内存池。  // NOLINT
          只有在成功Commit后，才能申请成功  // NOLINT
   @param ppBuffer: 申请到的内存  // NOLINT
   @return:           成功返回实际的长度，否则返回0  // NOLINT
*/
template<typename T>
inline int BufferPoolImpl<T>::GetBuffer(BufferInterfacePtr *buffer) {
  AutoLockOper lock(&mutex_);
  if (free_list_.size() > 0) {
    BufferInterfacePtr ptr = free_list_.front();
    free_list_.erase(free_list_.begin());
    used_list_.push_back(ptr.get());
    *buffer = ptr;
  } else {
    return YSOS_ERROR_NOT_ENOUGH_RESOURCE;
  }

  return YSOS_ERROR_SUCCESS;
}

//  BufferImpl *BufferPoolImpl::GetBufferFromInterfacePtr(BufferInterfacePtr pBuffer) {  // NOLINT
//  assert(pBuffer);
//  BufferImpl *ptr = static_cast<BufferImpl*>(pBuffer.get());
//
//  return ptr;
//  }

/*
   @brief 将内存释放回内存池  // NOLINT
   @param pBuffer: 申请到的内存  // NOLINT
   @return:         成功返回实际的长度，否则返回0  // NOLINT
*/
template<typename T>
inline int BufferPoolImpl<T>::ReleaseBuffer(BufferInterfacePtr buffer) {
  /// Bufffer支持自动释放
  /// 主动释放，不建议使用
//   AutoLockOper lock(&mutex_);
//   bool is_found = false;
//   std::list<BufferInterface*>::iterator it = used_list_.begin();
//   while (it != used_list_.end()) {
//     BufferInterface *ptr = *it;
// 
//     // 还要判断下ptr当前的引用数  // NOLINT
//     if (ptr == buffer.get()) {
//       used_list_.erase(it);
//       free_list_.push_back(buffer);
//       is_found = true;
//       break;
//     }
// 
//     ++it;
//   }
// 
//   return is_found ? YSOS_ERROR_SUCCESS : YSOS_ERROR_NOT_EXISTED;
  return YSOS_ERROR_SUCCESS;
}

/*
   @brief 将内存释放回内存池  // NOLINT
   @param pBuffer: 申请到的内存  // NOLINT
   @return:         成功返回实际的长度，否则返回0  // NOLINT
*/
template<typename T>
inline int BufferPoolImpl<T>::ReleaseBuffer(BufferInterface *buffer) {
  AutoLockOper lock(&mutex_);
  bool is_found = false;
  std::list<BufferInterface*>::iterator it = used_list_.begin();
  while (it != used_list_.end()) {
    BufferInterface *ptr = *it;

    // 还要判断下ptr当前的引用数  // NOLINT
    if (ptr == buffer) {
      used_list_.erase(it);
      DeleteBufferFunction delete_function;
      BufferInterfacePtr buffer_ptr(buffer, delete_function);
      free_list_.push_back(buffer_ptr);
      is_found = true;
      break;
    }

    ++it;
  }

  return is_found ? YSOS_ERROR_SUCCESS : YSOS_ERROR_NOT_EXISTED;
}
}
#endif  // SFP_BUFFER_POOL_H_  // NOLINT