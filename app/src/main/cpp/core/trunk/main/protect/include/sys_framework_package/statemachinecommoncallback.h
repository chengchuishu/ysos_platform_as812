/**
 *@file StateMachineCommonCallback.h
 *@brief 状态机通用Callback
 *@version 1.0
 *@author dhongqian
 *@date Created on: 2016-05-11 10:06:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

#ifndef SFP_STATE_MACHINE_COMMON_CALLBACK_H_  //NOLINT
#define SFP_STATE_MACHINE_COMMON_CALLBACK_H_  //NOLINT

#include "../../../protect/include/sys_framework_package/basemoduleimpl.h"  //NOLINT
#include "../../../protect/include/sys_framework_package/basecallbackimpl.h"  //NOLINT
//#include "../../protect/include/sys_framework_package/basecallbackimpl.h"  //NOLINT

namespace ysos {
class YSOS_EXPORT StateMachineCommonCallback : public  BaseCallbackImpl {
  DECLARE_CREATEINSTANCE(StateMachineCommonCallback);
  DISALLOW_COPY_AND_ASSIGN(StateMachineCommonCallback);
  DECLARE_PROTECT_CONSTRUCTOR(StateMachineCommonCallback);

 public:
  ~StateMachineCommonCallback();

  /**
  *@brief 回调接口的实现函数  // NOLINT
  *@param input_buffer[Input]： 输入数据缓冲  // NOLINT
  *@param output_buffer[Output]：回调处理后输出的数据缓冲  // NOLINT
  *@param context[Input]： 回调处理的上下文数据  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
   virtual int Callback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, void *context = NULL);
};
} // namespace ysos

#endif    //SFP_STATE_MACHINE_COMMON_CALLBACK_H_  //NOLINT