/**
 *@file time.h
 *@brief Definition of time
 *@version 0.1
 *@author XuLanyue
 *@date Created on: 2016-04-16 08:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef OHP_TIME_H          //NOLINT
#define OHP_TIME_H          //NOLINT

// time meter: timer
// functions:
//  restart
//  elapsed
//  elapsed_max
//  elapsed_min
#include <boost/timer.hpp>

// use boost::chrono directly
// it has
//    duration,
//    clock: system_clock, stready_clock, high_resolution_clock,
//    time_point,
//    time meter: cpu_timer(based on high_resolution_clock)
#include <boost/chrono.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>

// local time
#include <boost/date_time/local_time/local_time.hpp>

namespace ysos {

typedef boost::timer TimeMeter;

}  // namespace ysos

#endif //  OHP_TIME_H       //NOLINT
