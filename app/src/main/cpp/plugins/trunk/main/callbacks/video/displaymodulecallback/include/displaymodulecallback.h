/**
 *@file DisplayModuleCallback.h
 *@brief speech output  
 *@version 1.0
 *@author dhongqian
 *@date Created on: 2016-05-26 14:39:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

#ifndef YSOS_PLUGIN_DISPLAY_MODULE_CALLBACK_H_  //NOLINT
#define YSOS_PLUGIN_DISPLAY_MODULE_CALLBACK_H_  //NOLINT

/// ysos headers
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemoduleimpl.h"         //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemodulecallbackimpl.h" //NOLINT

namespace ysos {  
  /**
   *@brief  TTS callback // NOLINT
  */
class YSOS_EXPORT DisplayModuleCallback : public  BaseModuleCallbackImpl{
DECLARE_CREATEINSTANCE(DisplayModuleCallback);
DISALLOW_COPY_AND_ASSIGN(DisplayModuleCallback);
DECLARE_PROTECT_CONSTRUCTOR(DisplayModuleCallback);

public:

  /**
   *@brief 回调处理是否准备好  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  int IsReady();

protected:

    /**
   *@brief 回调接口的实现函数  // NOLINT
   *@param input_buffer[Input]： 输入数据缓冲  // NOLINT
   *@param output_buffer[Output]：回调处理后输出的数据缓冲  // NOLINT
   *@param context[Input]： 回调处理的上下文数据  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  int RealCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, BufferInterfacePtr external_buf, void *context);

};
} // namespace ysos

#endif    //  YSOS_PLUGIN_DISPLAY_MODULE_CALLBACK_H_  //NOLINT