/*
# lock.cpp
# Implemention of lock
# Created on: 2016-04-21 13:59:20
# Original author: XuLanyue
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20160421, created by XuLanyue
*/

#include "../../../public/include/os_hal_package/lock.h"    //NOLINT

// interthread
// #include <boost/thread/null_mutex.hpp>
// #include <boost/thread/shared_mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>

// interprocess
// #include <boost/interprocess/sync/interprocess_mutex.hpp>
// #include <boost/interprocess/sync/null_mutex.hpp>
// #include <boost/interprocess/sync/interprocess_sharable_mutex.hpp>
#include <boost/interprocess/sync/interprocess_recursive_mutex.hpp>
// #include <boost/interprocess/sync/interprocess_upgradable_mutex.hpp>

// named
// #include <boost/interprocess/sync/named_mutex.hpp>
// #include <boost/interprocess/sync/named_sharable_mutex.hpp>
#include <boost/interprocess/sync/named_recursive_mutex.hpp>
// #include <boost/interprocess/sync/named_upgradable_mutex.hpp>

#include <string>

namespace ysos {

const int YSOS_LOCK_TYPE_FAKE = 0;
const int YSOS_LOCK_TYPE_LIGHT = 1;
const int YSOS_LOCK_TYPE_WEIGHT = 2;

///////////////////////////////////////////////////////////////
///////////   Light Lock   ///////////////////////////////////
/////////////////////////////////////////////////////////////
struct LightLock_s {
  LightLock_s() {}
  boost::recursive_mutex oMutex;
};

LightLock::LightLock() : BaseLock(LOCK_TYPE_LIGHT) {
}

LightLock::~LightLock() {
}

void LightLock::Lock() {
  try {
    mutex_.lock();
  } catch (const boost::interprocess::interprocess_exception& e) {
    e.what();
  }
}

void LightLock::Unlock() {
  try {
    mutex_.unlock();
  } catch (const boost::interprocess::interprocess_exception& e) {
    e.what();
  }
}

///////////////////////////////////////////////////////////////
///////////   Mutex Lock   ///////////////////////////////////
/////////////////////////////////////////////////////////////
MutexLock::MutexLock() : BaseLock(LOCK_TYPE_LIGHT) {
}

MutexLock::~MutexLock() {
}

void MutexLock::Lock() {
  try {
    mutex_.lock();
  } catch (const boost::interprocess::interprocess_exception& e) {
    e.what();
  }
}

void MutexLock::Unlock() {
  try {
    mutex_.unlock();
  } catch (const boost::interprocess::interprocess_exception& e) {
    e.what();
  }
}

///////////////////////////////////////////////////////////////
///////////   Weight Lock   ///////////////////////////////////
/////////////////////////////////////////////////////////////
struct NamedWeightLock_s {
  NamedWeightLock_s(
    const char* pName,
    boost::interprocess::create_only_t create_only,
    const permissions &perm):
    strName(pName), oNamedMutex(create_only, pName, perm) {}
  NamedWeightLock_s(
    const char* pName,
    boost::interprocess::open_or_create_t open_or_create,
    const permissions &perm):
    strName(pName), oNamedMutex(open_or_create, pName, perm) {}
  NamedWeightLock_s(
    const char* pName,
    boost::interprocess::open_only_t open_only):
    strName(pName), oNamedMutex(open_only, pName) {}
  ~NamedWeightLock_s() {}
  boost::interprocess::named_recursive_mutex oNamedMutex;
  std::string strName;
};

struct AnonymousWeightLock_s {
  AnonymousWeightLock_s() {}
  ~AnonymousWeightLock_s() {}
  boost::interprocess::interprocess_recursive_mutex oAnonymousMutex;
};

WeightLock::WeightLock(
  const char* pName,
  OpenOrCreate_t open_or_create,
  const permissions &perm) : BaseLock(LOCK_TYPE_WEIGHT) {
  if ((pName != NULL) && (*pName != 0)) {
    if (open_or_create == WeightLock::CREATE_ONLY) {
      data_ptr_ = static_cast<void*>(
                    new NamedWeightLock_s(pName,
                                          boost::interprocess::create_only,
                                          perm));
    } else if (open_or_create == WeightLock::OPEN_OR_CREATE) {
      data_ptr_ = static_cast<void*>(
                    new NamedWeightLock_s(pName,
                                          boost::interprocess::open_or_create,
                                          perm));
    } else if (open_or_create == WeightLock::OPEN_ONLY) {
      data_ptr_ = static_cast<void*>(
                    new NamedWeightLock_s(pName, boost::interprocess::open_only));
    }
    is_named_ = true;
  } else {
    data_ptr_ = static_cast<void*>(new AnonymousWeightLock_s());
    is_named_ = false;
  }
}

WeightLock::~WeightLock() {
  if (is_named_) {
    NamedWeightLock_s* pWeightLock_s =
      static_cast<NamedWeightLock_s*>(data_ptr_);
    delete pWeightLock_s;
  } else {
    AnonymousWeightLock_s* pWeightLock_s =
      static_cast<AnonymousWeightLock_s*>(data_ptr_);
    delete pWeightLock_s;
  }
}

void WeightLock::Lock() {
  if (is_named_) {
    NamedWeightLock_s* pWeightLock_s =
      static_cast<NamedWeightLock_s*>(data_ptr_);
    if (pWeightLock_s != NULL) {
      try {
        pWeightLock_s->oNamedMutex.lock();
      } catch (const boost::interprocess::interprocess_exception& e) {
        e.what();
      }
    }
  } else {
    AnonymousWeightLock_s* pWeightLock_s =
      static_cast<AnonymousWeightLock_s*>(data_ptr_);
    if (pWeightLock_s != NULL) {
      try {
        pWeightLock_s->oAnonymousMutex.lock();
      } catch (const boost::interprocess::interprocess_exception& e) {
        e.what();
      }
    }
  }
}

bool WeightLock::TryLock() {
  bool bRet = false;
  if (is_named_) {
    NamedWeightLock_s* pWeightLock_s =
      static_cast<NamedWeightLock_s*>(data_ptr_);
    if (pWeightLock_s != NULL) {
      try {
        bRet = pWeightLock_s->oNamedMutex.try_lock();
      } catch (const boost::interprocess::interprocess_exception& e) {
        e.what();
      }
    }
  } else {
    AnonymousWeightLock_s* pWeightLock_s =
      static_cast<AnonymousWeightLock_s*>(data_ptr_);
    if (pWeightLock_s != NULL) {
      try {
        bRet = pWeightLock_s->oAnonymousMutex.try_lock();
      } catch (const boost::interprocess::interprocess_exception& e) {
        e.what();
      }
    }
  }
  return bRet;
}

bool WeightLock::TimedLock(const boost::posix_time::ptime& timeAbs) {
  bool bRet = false;
  if (is_named_) {
    NamedWeightLock_s* pWeightLock_s =
      static_cast<NamedWeightLock_s*>(data_ptr_);
    if (pWeightLock_s != NULL) {
      try {
        bRet = pWeightLock_s->oNamedMutex.timed_lock(timeAbs);
      } catch (const boost::interprocess::interprocess_exception& e) {
        e.what();
      }
    }
  } else {
    AnonymousWeightLock_s* pWeightLock_s =
      static_cast<AnonymousWeightLock_s*>(data_ptr_);
    if (pWeightLock_s != NULL) {
      try {
        bRet = pWeightLock_s->oAnonymousMutex.timed_lock(timeAbs);
      } catch (const boost::interprocess::interprocess_exception& e) {
        e.what();
      }
    }
  }
  return bRet;
}
void WeightLock::Unlock() {
  if (is_named_) {
    NamedWeightLock_s* pWeightLock_s =
      static_cast<NamedWeightLock_s*>(data_ptr_);
    if (pWeightLock_s != NULL) {
      try {
        pWeightLock_s->oNamedMutex.unlock();
      } catch (const boost::interprocess::interprocess_exception& e) {
        e.what();
      }
    }
  } else {
    AnonymousWeightLock_s* pWeightLock_s =
      static_cast<AnonymousWeightLock_s*>(data_ptr_);
    if (pWeightLock_s != NULL) {
      try {
        pWeightLock_s->oAnonymousMutex.unlock();
      } catch (const boost::interprocess::interprocess_exception& e) {
        e.what();
      }
    }
  }
}
bool WeightLock::Remove(const char* szName) {
  bool bRet = false;
  if ((szName != NULL) && (*szName != 0)) {
    try {
      bRet = boost::interprocess::named_recursive_mutex::remove(szName);
    } catch (const boost::interprocess::interprocess_exception& e) {
      e.what();
    }
  }
  return bRet;
}
}  // namespace ysos
