/**   //NOLINT
  *@file counterreleasedeposit.h
  *@brief Definition of 引用计数自动释放
  *@version 0.1
  *@author wanfenfen
  *@date Created on: 2016:11:4   18:11
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  */

#ifndef _TD_COUNTER_RELEASE_DEPOSIT_H_
#define _TD_COUNTER_RELEASE_DEPOSIT_H_
#pragma once

#include <boost/atomic/atomic.hpp>

///@brief 独享所有权，不允许赋值拷贝
template<class _Ty>
class counter_add_deposit {
 public:
  explicit counter_add_deposit(_Ty *_Ptr = 0) throw(): _Myptr(_Ptr) {}
  ~counter_add_deposit() {
    if (_Myptr!=NULL)	*_Myptr = *_Myptr +1;/*delete _Myptr;*/
  }
  // 	_Ty *get() const throw(){assert(_Myptr!=NULL);return (_Myptr);}
  // 	void reset(_Ty *_Ptr = 0){ if (_Ptr != _Myptr && _Myptr!=NULL)	delete _Myptr;_Myptr = _Ptr;}
 private:
   counter_add_deposit(counter_add_deposit const &);
   counter_add_deposit & operator=(counter_add_deposit const &);
 private:
  _Ty *_Myptr;// the wrapped object pointer
};

template<class _Ty>
class counter_sub_deposit {
public:
  explicit counter_sub_deposit(_Ty *_Ptr = 0) throw(): _Myptr(_Ptr) {}
  ~counter_sub_deposit() {
    if (_Myptr!=NULL)	*_Myptr = *_Myptr -1;/*delete _Myptr;*/
  }
  // 	_Ty *get() const throw(){assert(_Myptr!=NULL);return (_Myptr);}
  // 	void reset(_Ty *_Ptr = 0){ if (_Ptr != _Myptr && _Myptr!=NULL)	delete _Myptr;_Myptr = _Ptr;}
private:
  counter_sub_deposit(counter_sub_deposit const &);
  counter_sub_deposit & operator=(counter_sub_deposit const &);
private:
  _Ty *_Myptr;// the wrapped object pointer
};

#endif