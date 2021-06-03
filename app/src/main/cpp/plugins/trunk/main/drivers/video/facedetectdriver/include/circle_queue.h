/**
 *@file circle_queue.h
 *@brief 循环队列
 *@version 1.0
 *@author WangDaqian
 *@date Created on: 2018-01-13
 *@copyright Copyright(c) 2018 YunShen Technology. All rights reserved.
**/
#pragma once

#ifndef YSOS_CIRCLE_QUEUE_H //NOLINT
#define YSOS_CIRCLE_QUEUE_H //NOLINT

#include <vector>

namespace ysos {

template<class T> class CircleQueue {
public:
  CircleQueue();
  explicit CircleQueue(int size);
  ~CircleQueue();

  void Add(const T& item);
  T& Get(int i);

  int Size() const { return size_; }
  int Length() const { return length_; }

private:
  // 队列容量
  int size_;
  // 当前元素个数
  int length_;
  // 下一个存放位置
  int pos_;
  std::vector<T> data_;

};

template<class T>
CircleQueue<T>::CircleQueue()
    : length_(0),
      size_(5),
      pos_(0) {
  data_.resize(size_);
}

template<class T>
CircleQueue<T>::CircleQueue(int size)
  : length_(0),
  size_(size),
  pos_(0) {
  data_.resize(size_);
}

template<class T>
CircleQueue<T>::~CircleQueue() {
  data_.clear();
  length_ = 0;
  size_ = 0;
}

template<class T>
void CircleQueue<T>::Add(const T& item) {
  int i = pos_ % size_;
  pos_ = (pos_ + 1) % size_;
  data_[i] = item;
  ++length_;
  if (length_ > size_)
    length_ = size_;
}

template<class T>
T& CircleQueue<T>::Get(int i) {
  i = (pos_ - length_ + i + size_) % size_;
  return data_[i];
}

} // namespace ysos

#endif  // YSOS_CIRCLE_QUEUE_H 
