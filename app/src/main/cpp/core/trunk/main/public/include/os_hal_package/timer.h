/**
 *@file timer.h
 *@brief Definition of timer
 *@version 0.1
 *@author XuLanyue
 *@date Created on: 2016-04-16 08:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef OHP_TIMER_H         //NOLINT
#define OHP_TIMER_H         //NOLINT

#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/deadline_timer_service.hpp>

namespace ysos {

typedef boost::asio::io_service IoService;
typedef boost::asio::deadline_timer Timer;

}  // namespace ysos

/*
Example:


      oTimer.expires_at(oTimer.expires_at() + boost::posix_time::seconds(1));
      oTimer.async_wait(boost::bind(&printer::print, this));
*/

#endif //  OHP_TIMER_H      //NOLINT
