/**
*@file event.h
*@brief Definition of event
*@version 0.1
*@author Dhongqian
*@date Created on: 2016-06-16 16:07:58
*@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
* 
*/

#ifndef OHP_EVENT_H_
#define OHP_EVENT_H_

/// Boost Headers //  NOLINT
#include <boost/thread.hpp>
#include <boost/thread/lock_factories.hpp>
#include <boost/thread/lockable_adapter.hpp>
#include <boost/thread/lockable_concepts.hpp>
#include <boost/thread/thread_guard.hpp>
#include <boost/thread/scoped_thread.hpp>
#include <boost/thread/shared_lock_guard.hpp>
/// Ysos Headers //  NOLINT
#include "../../../public/include/sys_interface_package/common.h"

namespace ysos {

#define WAIT_INFINITE           0xffffffff

/**
 *@brief 平台Event接口
 */
class YSOS_EXPORT EventInterface {
 public:
  virtual ~EventInterface() {}

  virtual int Wait(const int time_out = WAIT_INFINITE) = 0;
  virtual int Notify(void) = 0;
};

/**
 *@brief 平台Event类
 */
class YSOS_EXPORT Event: public EventInterface {
  DISALLOW_COPY_AND_ASSIGN(Event);

 public:
  Event(void);
  virtual ~Event();

  virtual int Wait(const int time_out = WAIT_INFINITE);
  virtual int Notify(void);

 private:
  boost::mutex       mutex_;
  boost::condition_variable_any  condition_;
};
}



#endif  //  _OHP_EVENT__H_//NOLINT
