/**
 *@file timetriggercallback.h
 *@brief the time trigger call back
 *@version 1.0
 *@author z.gg
 *@date Created on: 2020-9-1 9:10:00
 *@copyright Copyright (c) 2020 YS Technology. All rights reserved.
 */

#ifndef TIME_TRIGGER_CALLBACK_H_
#define TIME_TRIGGER_CALLBACK_H_

/// Private Headers //  NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemoduleimpl.h"  //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemodulecallbackimpl.h"  //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basethreadmodulecallbackimpl.h"  //NOLINT
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"

namespace ysos {

/**
 *@brief  定时器模块回调层 //NOLINT
*/
class YSOS_EXPORT TimeTriggerCallBack : public BaseThreadModuleCallbackImpl {
  DECLARE_CREATEINSTANCE(TimeTriggerCallBack);
  DISALLOW_COPY_AND_ASSIGN(TimeTriggerCallBack);
  DECLARE_PROTECT_CONSTRUCTOR(TimeTriggerCallBack);

 public:
  virtual ~TimeTriggerCallBack(void);
  /**
   *@brief 回调处理是否准备好  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回TDOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  int IsReady();

 protected:
  /**
  *@brief 回调接口的实现函数  // NOLINT
  *@param input_buffer[Input]： 输入数据缓冲  // NOLINT
  *@param output_buffer[Output]：回调处理后输出的数据缓冲  // NOLINT
  *@param context[Input]： 回调处理的上下文数据  // NOLINT
  *@return： 成功返回TDOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int RealCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr cur_output_buffer, BufferInterfacePtr output_buffer, void *context);

};

}///< namespace tdos    //NOLINT

#endif///< TIME_TRIGGER_CALLBACK_H_    //NOLINT

