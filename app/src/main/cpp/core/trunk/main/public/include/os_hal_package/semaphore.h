/**
 *@file semaphore.h
 *@brief Definition of semaphore
 *@version 0.1
 *@author XuLanyue
 *@date Created on: 2016-04-13 08:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef OHP_SEMAPHORE_H     //NOLINT
#define OHP_SEMAPHORE_H     //NOLINT

// interprocess
#include <boost/interprocess/sync/interprocess_semaphore.hpp>

// named
#include <boost/interprocess/sync/named_semaphore.hpp>

namespace ysos {

// interprocess
typedef boost::interprocess::interprocess_semaphore InterprocessSemaphore;

// named
typedef boost::interprocess::named_semaphore NamedSemaphore;

}  // namespace ysos

#endif //  OHP_SEMAPHORE_H  //NOLINT
