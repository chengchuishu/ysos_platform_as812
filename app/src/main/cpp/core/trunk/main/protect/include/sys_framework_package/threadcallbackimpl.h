/**
  *@file threadcallbackimpl.h
  *@brief thread callback impls
  *@version 1.0
  *@author Pan
  *@date Created on: 2016-06-15 17:25:20
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  */
#ifndef THREAD_CALLBACK_IMPL_H_  // NOLINT
#define THREAD_CALLBACK_IMPL_H_  // NOLINT

/// ysos private headers //  NOLINT
#include "../../../public/include/sys_interface_package/callbackqueue.h"  // NOLINT
#include "../../../protect/include/sys_framework_package/basemoduleimpl.h"         //NOLINT
#include "../../../protect/include/sys_framework_package/basemodulecallbackimpl.h" //NOLINT

namespace ysos {
/*
  @brief 多线程Callback，在Callback里实现了多线程调用
*/
class YSOS_EXPORT ThreadCallbackImpl : public CallbackInterface, public BaseInterfaceImpl {
  //DECLARE_CREATEINSTANCE(BaseModuleCallbackImpl);
  DISALLOW_COPY_AND_ASSIGN(ThreadCallbackImpl);
  DECLARE_PROTECT_CONSTRUCTOR(ThreadCallbackImpl);

 public:
  virtual ~ThreadCallbackImpl();
  /**
  *@brief 回调接口的实现函数  // NOLINT
  *@param input_buffer[Input]： 输入数据缓冲  // NOLINT
  *@param output_buffer[Output]：回调处理后输出的数据缓冲  // NOLINT
  *@param context[Input]： 回调处理的上下文数据,是ModuleInterface* 类型  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int Callback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, void *context = NULL); // NOLINT

protected:
  virtual int ThreadCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer,LPVOID context) = 0;
};

/*
  @brief 多线程Module的Callback，在Callback里实现了多线程调用
*/
class YSOS_EXPORT ThreadModuleCallbackImpl : public ThreadCallbackImpl{
 DECLARE_CREATEINSTANCE(ThreadModuleCallbackImpl);
 DISALLOW_COPY_AND_ASSIGN(ThreadModuleCallbackImpl);
 DECLARE_PROTECT_CONSTRUCTOR(ThreadModuleCallbackImpl);
 public:  
  /**
  *@brief 设置回调的输入输出数据类型  // NOLINT
  *@param input_type[Input]： 输入数据类型  // NOLINT
  *@param output_type[Intput]：输出的数据类型  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int SetType(const CallbackDataType input_type, const CallbackDataType output_type);
  /**
  *@brief 获得回调的输入输出数据类型  // NOLINT
  *@param input_type[Output]： 输入数据类型  // NOLINT
  *@param output_type[Output]：输出的数据类型  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int GetType(CallbackDataType *input_type, CallbackDataType *output_type);
  /**
  *@brief 回调处理是否准备好  // NOLINT
  *@param ： 无  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int IsReady();

protected:
  int ThreadCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer,LPVOID context);
};
}
#endif  // THREAD_CALLBACK_IMPL_H_  // NOLINT
