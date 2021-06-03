/**
 *@file ModuleImpl.cpp
 *@brief Definition of ModuleImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
//#include "../../../protect/include/sys_framework_package/bufferimpl.h"  //  NOLINTbufferpoolimpl.h"
//#include "../../../protect/include/sys_framework_package/bufferimpl.h"
//#include <string>
//#include <exception>
//#include <iostream>
//#include <boost/thread/locks.hpp>
//#include <boost/function.hpp>

//
//namespace ysos {
//typedef boost::lock_guard<boost::mutex> MutexLock;
//#define MUTEX_LOCK MutexLock lock(mutex_);
///*********************************************************************************
//*                  BufferPoolImpl 实现
//**********************************************************************************/
///*
//   @brief: BufferImpl使用shared_ptr指针时的定制删除函数
//*/
//class DeleteBuffer {
// public:
//  DeleteBuffer(BufferPoolImpl *pPool): pool_ptr_(pPool) {
//    assert(NULL!=pPool);
//  }
//  void operator()(BufferInterface* pBuffer) {
//    BufferImpl *pBuf = static_cast<BufferImpl*>(pBuffer);
//
//    // 不用自动释放，用户会主动释放
//   // pool_ptr_->ReleaseBuffer(pBuffer);
//  }
//
// private:
//  BufferPoolImpl  *pool_ptr_;
//};
//
///*
//     BufferPoolImpl使用shared_ptr指针时的定制删除函数
//     BufferPoolImpl指针不需要BufferImpl释放
//*/
//class DeleteBufferPool {
// public:
//  void operator()(BufferPoolInterface* pBufferPool) {
//  }
//};
//
//BufferPoolImpl::BufferPoolImpl(void)  {
//  std::memset(&allocate_properties_, 0, sizeof(allocate_properties_));
//  used_count_ = request_size_ = max_buffer_number_ = 0;
//  object_pool_ = NULL;
//
//  DeleteBufferPool delete_buffer_pool = delete_buffer_pool;
//  pool_ptr_ = new BufferPoolInterfacePtr(this, delete_buffer_pool);
//}
//
//BufferPoolImpl::~BufferPoolImpl() {
//  Decommit();
//
//  if (NULL != object_pool_) {
//    delete object_pool_;
//    object_pool_ = NULL;
//  }
//
//  if (NULL != pool_ptr_) {
//    delete pool_ptr_;
//    pool_ptr_ = NULL;
//  }
//}
//
//int BufferPoolImpl::GetAlign(const int align) {
//  //首先对齐字节数，必须是2的幂
//  int nTmp = 1;
//  while (align > nTmp) {
//    nTmp <<= 1;
//  }
//
//  return nTmp - 1;
//}
//
//int BufferPoolImpl::GetRequestSize(void) {
//  return allocate_properties_.cbBuffer + GetAlign(allocate_properties_.cbPrefix) + sizeof(BufferImpl);
//}
//
//int BufferPoolImpl::SetProperties(AllocatorProperties *pRequest, AllocatorProperties *pActual) {
//  assert(NULL!=pRequest);
//  // 现在pActual与pRequest都一致
//  std::memcpy(&allocate_properties_, pRequest, sizeof(allocate_properties_));
//
//  if(NULL != pActual) {
//    std::memcpy(pActual, &allocate_properties_, sizeof(allocate_properties_));
//  }
//
//  return 0;
//}
//
//int BufferPoolImpl::GetProperties(AllocatorProperties *pProps) {
//  assert(NULL!=pProps);
//  std::memcpy(pProps, &allocate_properties_, sizeof(allocate_properties_));
//  DumpMemoryUsage();
//
//  return 0;
//}
//
///*
//   @brief 正式分配内存，分配成功后，才可以使用
//   @return:           成功返回实际的长度，否则返回0
//*/
//int BufferPoolImpl::Commit() {
//  // 已经Commit完，无须再commit
//  if (NULL != object_pool_) {
//    return 0;
//  }
//
// MUTEX_LOCK
//  request_size_ = GetRequestSize();
//  max_buffer_number_ = allocate_properties_.cBuffers;
//  /*
//    参数设置完成后，内存会在申请第一块Buffer的时候，一次性分配完成
//  */
//  object_pool_ = new BufferPoolBase<BufferImpl, boost::default_user_allocator_new_delete>();
//  object_pool_->SetRequestParam(request_size_, max_buffer_number_);
//  assert(NULL!=object_pool_);
//
//  return 0;
//}
//
///*
//   @brief 释放管理的所有内存，在Commit前，不能提供内存申请
//   @return:           成功返回实际的长度，否则返回0
//*/
//int BufferPoolImpl::Decommit() {
//  MUTEX_LOCK
//
//  delete object_pool_;
//  object_pool_ = NULL;
//  used_count_ = max_buffer_number_ = 0;
//
//  return 0;
//}
//
///*
//   @brief 申请一块内存池。
//          只有在成功Commit后，才能申请成功
//   @param ppBuffer: 申请到的内存
//   @return:           成功返回实际的长度，否则返回0
//*/
//int BufferPoolImpl::GetBuffer(BufferInterfacePtr *ppBuffer) {
//
//  try {
//    // 请先commit
//    if (NULL==object_pool_) {
//      return 1;
//    }
//    /*
//       通过shared_ptr管理的内存块，是由bufferpoolbase分配的，当shared_ptr
//       使用完时，应交由bufferpoolbase来释放内存，而不应通过析构函数释放内存，
//       所以在定义shared_ptr对象时，定制了shared_ptr的删除函数
//    */
//    DeleteBuffer delete_buffer(this);
//    MUTEX_LOCK
//    if (used_count_ >= max_buffer_number_) {
//      return 0;
//    }
//    ++used_count_;
//    BufferImpl *ptr = object_pool_->construct(pool_ptr_, allocate_properties_.cbBuffer + allocate_properties_.cbPrefix);
//    *ppBuffer = BufferInterfacePtr(ptr, delete_buffer);
//  } catch (std::bad_alloc e) {
//    // TDLOG_CONSOLE_TRACE << "Exception: " << std::endl;
//  } catch (...) {
//    //  TDLOG_CONSOLE_TRACE << "Allocate memory failed\n" ;
//
//    return 1;
//  }
//
//  return 0;
//}
//
//BufferImpl *BufferPoolImpl::GetBufferFromInterfacePtr(BufferInterfacePtr pBuffer) {
//  assert(pBuffer);
//  BufferImpl *ptr = static_cast<BufferImpl*>(pBuffer.get());
//
//  return ptr;
//}
//
///*
//   @brief 将内存释放回内存池
//   @param pBuffer: 申请到的内存
//   @return:         成功返回实际的长度，否则返回0
//*/
//int BufferPoolImpl::ReleaseBuffer(BufferInterfacePtr pBuffer) {
//  BufferImpl *ptr = GetBufferFromInterfacePtr(pBuffer);
//  return ReleaseBuffer(ptr);
//
//  return 0;
//}
//
///*
//   @brief 将内存释放回内存池
//   @param pBuffer: 申请到的内存
//   @return:        成功返回实际的长度，否则返回0
//*/
//int BufferPoolImpl::ReleaseBuffer(BufferInterface* pBuffer) {
//  MUTEX_LOCK
//  BufferImpl *ptr = static_cast<BufferImpl*>(pBuffer);
//  if (object_pool_->is_from(ptr)) {
//    object_pool_->destroy(ptr);
//    --used_count_;
//  }
//
//  return 0;
//}
//
//void BufferPoolImpl::DumpMemoryUsage() {
//  if (NULL == object_pool_) {
//    return;
//  }
//
//  unsigned int reqest_size=0, start_size=0;
//  //object_pool_->GetRequesetParam(&reqest_size, &start_size);
//
//  TDLOG_CONSOLE_TRACE<<"single buffer size: "<<reqest_size << "allocated number: " << start_size << std::endl;
//}
//}T