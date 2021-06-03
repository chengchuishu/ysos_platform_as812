/**
 *@file condition.h
 *@brief Definition of Condition
 *@version 0.1
 *@author XuLanyue
 *@date Created on: 2016-04-13 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef OHP_CONDITION_H     //NOLINT
#define OHP_CONDITION_H     //NOLINT

// interthread
#include <boost/thread/condition.hpp>
#include <boost/thread/condition_variable.hpp>

// interprocess
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/interprocess_condition_any.hpp>

// named
#include <boost/interprocess/sync/named_condition.hpp>
#include <boost/interprocess/sync/named_condition_any.hpp>

namespace ysos {

// interthread
typedef boost::condition Condition;

// interprocess

// It can be placed in shared memory or memory mapped files.
// Unlike std::condition_variable in C++11, it is NOT safe to invoke the
// destructor if all threads have been only notified. It is required that they
// have exited their respective wait functions.
typedef boost::interprocess::interprocess_condition InterprocessCondition;

// It can be placed in shared memory or memory mapped files.
// It is a generalization of interprocess_condition.
// Whereas interprocess_condition works only on Locks with mutex_type ==
// interprocess_mutex, interprocess_condition_any can operate on any
// user-defined lock that meets the BasicLockable requirements (lock()/unlock()
// member functions).
// Unlike std::condition_variable in C++11, it is NOT safe to invoke the
// destructor if all threads have been only notified. It is required that they
// have exited their respective wait functions.
typedef boost::interprocess::interprocess_condition_any InterprocessConditionAny;	//NOLINT

// named
// A global condition variable that can be created by name.
// It is designed to work with named_mutex and can't be placed in shared memory
// or memory mapped files.

typedef boost::interprocess::named_condition NamedCondition;
typedef boost::interprocess::named_condition_any NamedConditionAny;

}  // namespace ysos

#endif //  OHP_CONDITION_H  //NOLINT
