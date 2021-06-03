/**
  *@file StatusEventModuleCallback.h
  *@brief Definition of StatusEventModuleCallback
  *@version 0.1
  *@author dhongqian
  *@date Created on: 2016-04-21 13:59:20
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  */
#ifndef SFP_STATUS_EVENT_MODULE_CALLBACK_H_  // NOLINT
#define SFP_STATUS_EVENT_MODULE_CALLBACK_H_  // NOLINT

/// stl headers //  NOLINT
#include <map>
#include <string>
/// ysos private headers //  NOLINT
#include "../../../public/include/os_hal_package/config.h"
#include "../../../public/include/sys_interface_package/common.h"  // NOLINT
#include "../../../public/include/sys_interface_package/callbackqueue.h"  // NOLINT
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../protect/include/sys_framework_package/basecallbackimpl.h"
#include "../../../protect/include/sys_framework_package/basemodulecallbackimpl.h"

namespace ysos {
/**
  *@brief  BaseModule的Status Event Callback  //  NOLINT
  */
class YSOS_EXPORT StatusEventModuleCallback : public BaseModuleCallbackImpl {
  DECLARE_CREATEINSTANCE(StatusEventModuleCallback);
  DISALLOW_COPY_AND_ASSIGN(StatusEventModuleCallback);
  DECLARE_PROTECT_CONSTRUCTOR(StatusEventModuleCallback);

 public:

  virtual ~StatusEventModuleCallback();

  /**
   *@brief 基本初始化  // NOLINT
   *@param param： 初始化的参数  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  //virtual int Initialize(void *param=NULL);
  /**
  *@brief 回调接口的实现函数  // NOLINTㅐ
  *@param input_buffer[Input]： 输入数据缓冲  // NOLINT
  *@param output_buffer[Output]：回调处理后输出的数据缓冲  // NOLINT
  *@param context[Input]： 回调处理的上下文数据,是BaseModuelCallbackContext* 类型  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int Callback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, void *context = NULL); // NOLINT

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
};
}
#endif  // SFP_STATUS_EVENT_MODULE_CALLBACK_H_  // NOLINT
