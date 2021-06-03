/**
 *@file callbackqueue.h
 *@brief Definition of callback queue interface
 *@version 0.1
 *@author Steven.Shi
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
*/
#ifndef CALLBACK_QUEUE_INTERFACE_H  //NOLINT
#define CALLBACK_QUEUE_INTERFACE_H  //NOLINT

#include "../../../public/include/sys_interface_package/baseinterface.h"
#include "../../../public/include/sys_interface_package/bufferpool.h"
#include <string>
#include <boost/shared_ptr.hpp>

namespace ysos {

typedef std::string CallbackDataType;
typedef std::pair<CallbackDataType, CallbackDataType> CallbackIODataTypePair;
#define CallbackDataTypeAll  "ALL"
#define StatusEventDataType  "StatusEvent"
  /**
   *@brief CallbackInterface是系统内回调函数的统一接口，
       主要用于ModuleInterface，所有涉及到回调的位置都采用该接口
   */
class YSOS_EXPORT CallbackInterface : virtual public BaseInterface {
  IMPLEMENT_PUBLIC_EMPTY_DESTRUCTOR(CallbackInterface)
 public:

/**
   *@brief 回调接口的实现函数  // NOLINT
   *@param input_buffer[Input]： 输入数据缓冲  // NOLINT
   *@param output_buffer[Output]：回调处理后输出的数据缓冲  // NOLINT
   *@param context[Input]： 回调处理的上下文数据  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Callback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, void *context = NULL) = 0;

  /**
   *@brief 设置回调的输入输出数据类型  // NOLINT
   *@param input_type[Input]： 输入数据类型  // NOLINT
   *@param output_type[Intput]：输出的数据类型  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int SetType(const CallbackDataType input_type, const CallbackDataType output_type) = 0;

  /**
   *@brief 获得回调的输入输出数据类型  // NOLINT
   *@param input_type[Output]： 输入数据类型  // NOLINT
   *@param output_type[Output]：输出的数据类型  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int GetType(CallbackDataType *input_type, CallbackDataType *output_type) = 0;

  /**
   *@brief 回调处理是否准备好  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int IsReady() = 0;
};

typedef boost::shared_ptr<CallbackInterface> CallbackInterfacePtr;

  /**
   *@brief CallbackQueueInterface 是回调函数指针队列的统一接口，
       主要用于ModuleInterface
   */
class YSOS_EXPORT CallbackQueueInterface : virtual public BaseInterface {
  IMPLEMENT_PUBLIC_EMPTY_DESTRUCTOR(CallbackQueueInterface)
 public:

   /**
   *@brief 添加Callback到队列中  // NOLINT
   *@param callback[Input]： CallbackInterface指针  // NOLINT
   *@param owner_id[Input]： CallbackInterface隶属模块ID，如Module  // NOLINT
   *@param priority[Input]： CallbackInterface优先级  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int AddCallback(CallbackInterfacePtr callback, INT64 owner_id = 0, UINT32 priority = 0) = 0;

   /**
   *@brief 从队列中移除CallbackInterface的指针  // NOLINT
   *@param callback[Input]： CallbackInterface指针  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int RemoveCallback(CallbackInterfacePtr callback) = 0;

   /**
   *@brief 从队列中移除CallbackInterface的指针，按照隶属模块ID  // NOLINT
   *@param owner_id[Input]： CallbackInterface指针  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int RemoveByID(INT64 owner_id) = 0;

  /**
   *@brief  获取与Key匹配的Callback
   *@param pCallback[Output]：  成功返回Callback，失败返回NULL
   *@param key[Input]  条件Key，是数据输入类型和输出类型组合
   *@return  成功返回 true，失败返回 false
   */
  virtual bool FindCallback(CallbackInterfacePtr *callback, CallbackIODataTypePair key) = 0;

  /**
   *@brief  获取第一个或下一个Callback
   *@param first_not_next[Input]:  true 获取第一个Callback false 获取下一个Callback
   *@return 返回获得的CallbackInterface指针，失败返回NULL
   */
  virtual CallbackInterfacePtr FindCallback(bool first_not_next = false) = 0;
};

typedef boost::shared_ptr<CallbackQueueInterface> CallbackQueueInterfacePtr;

} // namespace ysos

#endif  //CALLBACK_QUEUE_INTERFACE_H  //NOLINT
