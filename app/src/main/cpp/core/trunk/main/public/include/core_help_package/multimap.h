/**
 *@file multimap.h
 *@brief Definition of multimap
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef CHP_MULTIMAP_H  //NOLINT
#define CHP_MULTIMAP_H  //NOLINT

/// Boost Headers
#include <boost/assign.hpp>
#include <boost/unordered_map.hpp>
#include <boost/typeof/typeof.hpp>

namespace ysos {
template<typename T>
class MapCondition {
 public:
  MapCondition(T &map_pair) {
    map_pair_ = map_pair;
  }

  bool operator()(T map_pair) {
    return (map_pair.first==map_pair_.first && map_pair.second == map_pair_.second);
  }

 private:
  T   map_pair_;
};
}
#endif // CHP_MULTIMAP_H  //NOLINT
