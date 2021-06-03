/**
 *@file basethreadmodulecallbackimpl.h
 *@brief base threading module callback impl
 *@version 1.0
 *@author Pan
 *@date Created on: 2016-06-15 20:06:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef SFP_BASE_THREAD_MODULE_CALLBACK_H_  //NOLINT
#define SFP_BASE_THREAD_MODULE_CALLBACK_H_  //NOLINT

/// boost headers //  NOLINT
#include <boost/thread.hpp>
/// ysos private headers //  NOLINT
#include "../../../protect/include/sys_framework_package/threadcallbackimpl.h"
#include "../../../protect/include/sys_framework_package/basemoduleimpl.h"         //NOLINT
#include "../../../protect/include/sys_framework_package/basemodulecallbackimpl.h" //NOLINT
#include "../../../public/include/os_hal_package/event.h"

namespace ysos {
/**
 *@brief  base thread module callback // NOLINT
*/
class YSOS_EXPORT BaseThreadModuleCallbackImpl : public  BaseModuleCallbackImpl {
  DECLARE_CREATEINSTANCE(BaseThreadModuleCallbackImpl);
  DECLARE_PROTECT_CONSTRUCTOR(BaseThreadModuleCallbackImpl);

 public:
  /**
  *@brief 回调接口的实现函数  // NOLINT
  *@param input_buffer[Input]： 输入数据缓冲  // NOLINT
  *@param output_buffer[Output]：回调处理后输出的数据缓冲  // NOLINT
  *@param context[Input]： 回调处理的上下文数据,是ModuleInterface* 类型  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  int Callback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, void *context = NULL); // NOLINT

 protected:
  /**
  *@brief 子类真正的实现函数  // NOLINT
  *@param input_buffer[Input]： 输入数据缓冲  // NOLINT
  *@param next_input_buffer[Output]： 当前Callback对input_buffer处理后的结果存在next_input_buffer,并作为NextCallback的输入  // NOLINT
  *@param pre_output_buffer[Output]：回调处理后输出的数据缓冲  // NOLINT
  *@param context[Input]： 回调处理的上下文数据,是BaseModuelCallbackContext* 类型  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int RealCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr next_input_buffer,
                           BufferInterfacePtr pre_output_buffer, void *context);

  std::pair<CallbackDataType, CallbackDataType> GetUniqueKey(void);

 private:
  ThreadModuleCallbackImpl* thread_module_callback_ptr_;
};
} // namespace ysos

#endif    //SFP_BASE_THREAD_MODULE_CALLBACK_H_  //NOLINT