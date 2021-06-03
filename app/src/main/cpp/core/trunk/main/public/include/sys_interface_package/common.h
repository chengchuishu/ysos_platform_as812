/**
  *@file common.h
  *@brief Definition of common functions
  *@version 0.1
  *@author dhongqian
  *@date Created on: 2016-04-21 13:59:20
  *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
  * 
  */

#ifndef CHP_COMMON_H                            //NOLINT
#define CHP_COMMON_H                            //NOLINT

//  使用boost标准整数类型                       //NOLINT
#include <boost/cstdint.hpp>
typedef boost::int8_t       int8_t, INT8, *PINT8;
typedef boost::uint8_t      uint8_t, UINT8, *PUINT8;
typedef boost::int16_t      int16_t, INT16, *PINT16;
typedef boost::uint16_t     uint16_t, UINT16, *PUINT16;
typedef boost::int32_t      int32_t, INT32, *PINT32;
typedef boost::uint32_t     uint32_t, UINT32, *PUINT32;
typedef boost::int64_t      int64_t, INT64, *PINT64;
typedef boost::uint64_t     uint64_t, UINT64, *PUINT64;
typedef void*               LPVOID;
typedef void *HANDLE;

//  返回值或错误状态定义                         //NOLINT
#include "./error.h"                             //NOLINT

//  消息头定义                                   //NOLINT
//#include "./messagehead.h"

// 通用宏定义                                    //NOLINT
#define CHECK_ARGUMENT_IF_NULL(ARG) \
{if(NULL==ARG) return YSOS_ERROR_INVALID_ARGUMENTS;}

#define YSOS_NEW(POINTER, EXPRESSION) \
{POINTER=new EXPRESSION; if(NULL==POINTER) return YSOS_ERROR_MEMORY_ALLOCATED_FAILED;}

#define YSOS_DELETE(POINTER)  \
{delete POINTER; POINTER=NULL;}

#define CHECK_FINISH(EXPRESSION) \
{\
  int EXPRESSION_RET = EXPRESSION; \
  if(EXPRESSION_RET != YSOS_ERROR_SUCCESS) {\
       /*BOOST_LOG(lg) << "return code: " << EXPRESSION_RET << "\n";*/\
       return EXPRESSION_RET; \
  }\
}

#define  OBJECT_NAME (this->GetName()/*.substr(79)*/)
#define  GET_OBJECT_NAME(OBJECT_PTR) (this->GetName()/*.substr(79)*/)

#ifdef  WIN32
#ifdef _YSOS_DLL_EXPORTING
#define YSOS_EXPORT __declspec(dllexport)
#else
#define YSOS_EXPORT __declspec(dllimport)
#endif
#define PROGAMA_ONCE #pragma once
#else
#ifdef _YSOS_DLL_EXPORTING
#define YSOS_EXPORT  /*__declspec(dllexport)*/
#else
#define YSOS_EXPORT  /*__declspec(dllimport)*/
#endif
#define PROGAMA_ONCE
#endif

#define YSOS_EXTERN_C  extern "C"

// 定义默认构造函数，用于工厂创建                  //NOLINT
#define DECLARE_DEFCONSTRUCTOR(CLASS_OBJECT, BASE_CLASS)  \
  explicit CLASS_OBJECT(const std::string &name = #CLASS_OBJECT) : BASE_CLASS(name) { }

// 使类成为noncopyable的, 通过声明copy构造与赋值操作符为私有化的, 无需实现copy构造与赋值操作符
#ifndef DISALLOW_COPY_AND_ASSIGN
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
 private: \
  TypeName(const TypeName&); \
  TypeName& operator=(const TypeName&);
#endif  //  DISALLOW_COPY_AND_ASSIGN

//  将类的构造函数声明为保护的                     //NOLINT
#define DECLARE_PROTECT_CONSTRUCTOR(classname) \
 protected: \
  classname(const std::string &strClassName = #classname);

//  将类的构造函数实现为虚的空函数                 //NOLINT
#define IMPLEMENT_PUBLIC_EMPTY_DESTRUCTOR(classname) \
 public: \
  virtual ~classname(void){};

#endif //  CHP_COMMON_H                            //NOLINT

#ifdef WIN32
#pragma warning(disable: 4250)      //  dominance
#pragma warning(disable: 4251)      //  dominance
#pragma warning(disable: 4996)      //  std::_Fill_n
#pragma warning(disable: 4275)      //  std::_Fill_n
#endif

/// 北京赛特斯项目预定义                          //NOLINT
#define _CERTUSNET                  0