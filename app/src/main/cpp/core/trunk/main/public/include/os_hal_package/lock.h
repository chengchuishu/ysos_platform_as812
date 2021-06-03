/**
 *@file lock.h
 *@brief Definition of Lock
 *@version 0.1
 *@author XuLanyue
 *@date Created on: 2016-04-21 08:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

#ifndef YSOS_LOCK_H                                                             //NOLINT
#define YSOS_LOCK_H                                                             //NOLINT
// 将锁拆分成描述临界区的类(其可以带锁操作)XXXMutex和                                   //NOLINT
// 描述上锁/解锁动作的类XXXLock                                                     //NOLINT
// 有以下3种锁:                                                                   //NOLINT
// 假锁:FakeLock                                                                 //NOLINT
// 线程间的锁:LightLock                                                           //NOLINT
// 进程间的锁:WeightLock                                                          //NOLINT
// AutoLockOper将ScopedLock操作自动化, 也就是说, 不需要显式的调用Lock和Unlock.          //NOLINT
#include "../../../public/include/sys_interface_package/common.h"
#include <boost/shared_ptr.hpp>
#include <boost/interprocess/permissions.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/mutex.hpp>

namespace ysos {

typedef boost::interprocess::permissions permissions;

enum LOCK_TYPE {
  LOCK_TYPE_BASE = 0,
  LOCK_TYPE_FAKE = 1,
  LOCK_TYPE_LIGHT = 2,
  LOCK_TYPE_WEIGHT = 3,
};

// 锁基类:BaseLock                                                                 //NOLINT
class YSOS_EXPORT BaseLock {
 private:
  BaseLock(const BaseLock &refFakeLock);
  BaseLock &operator=(const BaseLock &refFakeLock);
 public:
  BaseLock(int lock_type = LOCK_TYPE_BASE) : lock_type_(lock_type) {};
  virtual ~BaseLock() {};
  virtual void Lock() {};
  virtual void Unlock() {};
 protected:
  int lock_type_;
};


// 假锁:FakeLock                                                                 //NOLINT
class YSOS_EXPORT FackLock : public BaseLock {
 public:
  FackLock() : BaseLock(LOCK_TYPE_FAKE) {};
  virtual ~FackLock() {};
 private:
  FackLock(const FackLock &refLightLock);
  FackLock &operator=(const FackLock &refLightLock);
};

// 线程间的锁:LightLock                                                           //NOLINT
class YSOS_EXPORT LightLock : public BaseLock {
 public:
  LightLock();
  virtual ~LightLock();
  void Lock();
  void Unlock();
 private:
  LightLock(const LightLock &refLightLock);
  LightLock &operator=(const LightLock &refLightLock);
  boost::recursive_mutex mutex_;
};

// 互斥锁:MutexLock                                                           //NOLINT
class YSOS_EXPORT MutexLock : public BaseLock {
 public:
  MutexLock();
  virtual ~MutexLock();
  void Lock();
  void Unlock();
 private:
  MutexLock(const MutexLock &refLightLock);
  MutexLock &operator=(const MutexLock &refLightLock);
  boost::mutex mutex_;
};

// 进程间的锁:WeightLock                                                          //NOLINT
class WeightLock : public BaseLock {
 public:
  enum OpenOrCreate_t {
    CREATE_ONLY,
    OPEN_OR_CREATE,
    OPEN_ONLY,
  };

  // if pName == NULL, then the WeightLock is as anonymous,
  //    and the 2nd and 3rd params are ignored.
  // else, if the 2nd param == OPEN_ONLY, then the 3rd param is ignored.
  explicit WeightLock(
    const char* pName = NULL,
    OpenOrCreate_t open_or_create = CREATE_ONLY,
    const permissions &perm = boost::interprocess::permissions());
  virtual ~WeightLock();
  void Lock();
  bool TryLock();
  bool TimedLock(const boost::posix_time::ptime& timeAbs);
  void Unlock();

  //  Erases a named recursive mutex from the system
  static bool Remove(const char* szName);

 private:
  WeightLock(const WeightLock &refWeightLock);
  WeightLock &operator=(const WeightLock &refWeightLock);
  void *data_ptr_;
  bool is_named_;
};

// AutoLockOper使得ScopedLock不需要显式的调用Lock和Unlock.                          //NOLINT
class YSOS_EXPORT AutoLockOper {
 public:
  explicit AutoLockOper(BaseLock*  pLock) {
    if (NULL != pLock) {
      lock_ptr_ = pLock;
      lock_ptr_->Lock();
    }
  }
  virtual ~AutoLockOper() {
    if (NULL != lock_ptr_) {
      lock_ptr_->Unlock();
    }
  }
 private:
  AutoLockOper(const AutoLockOper &refAutoLockOper);
  AutoLockOper &operator=(const AutoLockOper &refAutoLockOper);
  BaseLock* lock_ptr_;
};

}  // namespace ysos

/*
Example:

#include <boost/function.hpp>
#include <boost/thread/thread.hpp>
#include <iostream>
#include "lock.h"
ysos::LightLock mutexIoForThreadUnit;
void count2(int id) {
  for (int i = 0; i < 20; ++i) {
    // using ScopedLock of Mutex, Lock and Unlock calls needn't any more.
    ysos::AutoLockOper oLock(&mutexIoForThreadUnit);
    YSOS_LOG_DEBUG(id << ": " << i);
  }
}
void test() {
// either with & or without & can work.
  boost::thread thrd3(boost::bind(&count2, 3));
  boost::thread thrd4(boost::bind(&count2, 4));
}
*/
#endif  //  YSOS_LOCK_H                                                         //NOLINT
