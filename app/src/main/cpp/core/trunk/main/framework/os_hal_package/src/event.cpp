/**
*@file event.cpp
*@brief Definition of event
*@version 0.1
*@author Dhongqian
*@date Created on: 2016-06-16 16:07:58
*@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
* 
*/

/// Self Header //  NOLINT
#include "../../../public/include/os_hal_package/event.h"

namespace ysos {

Event::Event(void) {
}

Event::~Event() {

}

int Event::Wait(const int time_out) {
  boost::unique_lock<boost::mutex> lock = boost::make_unique_lock(mutex_);
  //condition_.wait(lock);
  condition_.timed_wait(lock,  boost::get_system_time() + boost::posix_time::millisec(time_out));

  return YSOS_ERROR_SUCCESS;
}
int Event::Notify(void) {
  condition_.notify_all();

  return YSOS_ERROR_SUCCESS;
}
}
