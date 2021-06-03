/**
 *@file TemplatePriQueue.h
 *@brief Definition of TemplatePriQueue
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

#ifndef _CHP_TEMPLATE_PRI_QUEUE_
#define _CHP_TEMPLATE_PRI_QUEUE_

#include <map>
#include <list>

namespace ysos {
template<typename KEY, typename VALUE, typename _COMPARE=std::greater<KEY>>
class TemplatePriQueue : public std::multimap<KEY, VALUE, _COMPARE> {
 public:
  TemplatePriQueue() {};
  ~TemplatePriQueue() {};

  typedef typename std::multimap<KEY, VALUE, _COMPARE>::iterator ITER;
  bool IsEmpty() const {
    return std::multimap<KEY, VALUE>::empty();
  }
  int GetCount() const {
    return (int)std::multimap<KEY, VALUE>::size();
  }
  int GetKeyCount(const KEY &k) {
    return std::multimap<KEY, VALUE>::count(k);
  }
  void Push(const KEY &k, const VALUE &v) {
    std::pair<KEY, VALUE> kvPair(k, v);
    this->insert(kvPair);
  }
  void Pop() {
    std::multimap<KEY, VALUE>::erase(std::multimap<KEY, VALUE>::begin());
  }
  VALUE Top() const {
    return std::multimap<KEY, VALUE>::begin()->second;
  }
  void PopTail() {
    std::multimap<KEY, VALUE>::erase(std::multimap<KEY, VALUE>::rbegin());
  }
  VALUE Tail() const {
    return std::multimap<KEY, VALUE>::rbegin()->second;
  }
  void Erase(const KEY &k, const VALUE &v) {
    ITER iter;
    if (Find(k, v, iter)) {
      std::multimap<KEY, VALUE>::erase(iter);
    }
  }
  bool Find(const KEY &k, ITER &i) {
    i = std::multimap<KEY, VALUE>::find(k);
    if (std::multimap<KEY, VALUE>::end() == i) return false;
    return true;
  }
  bool Find(const KEY &k, const VALUE &v, ITER &i) {
    bool bFound = false;
    ITER iter = std::multimap<KEY, VALUE>::find(k);
    if (iter == std::multimap<KEY, VALUE>::end()) return false;
    while (iter->first == k) {
      if (iter->second == v) {
        i = iter;
        bFound = true;
        break;
      }
      iter ++;
    }
    return bFound;
  }
  KEY Key(ITER iter) {
    return iter->first;
  }
  VALUE Value(ITER iter) {
    return iter->second;
  }
  int Find(const KEY &k, std::list<VALUE> &lst) {
    int count = 0;
    ITER iter = std::multimap<KEY, VALUE>::find(k);
    while (iter->first == k) {
      lst.push_back(iter->second);
      iter ++;
      count ++;
    }
    return count;
  }
};
}

#endif /*_CHP_TEMPLATE_PRI_QUEUE_*/
