/**
 *@file templatecallback.h
 *@brief the template call back
 *@version 1.0
 *@author z.gg
 *@date Created on: 2020-3-30 15:26:00
 *@copyright Copyright (c) 2020 YS Technology. All rights reserved.
 * 
 */

#ifndef TEMPLATE_CALLBACK_H_
#define TEMPLATE_CALLBACK_H_

/// Private Headers //  NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemoduleimpl.h"  //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basethreadmodulecallbackimpl.h"  //NOLINT
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"

namespace ysos {

/**
 *@brief  模块回调层 //NOLINT
*/
class YSOS_EXPORT TemplateCallback : public BaseThreadModuleCallbackImpl {
  DECLARE_CREATEINSTANCE(TemplateCallback);
  DISALLOW_COPY_AND_ASSIGN(TemplateCallback);
  DECLARE_PROTECT_CONSTRUCTOR(TemplateCallback);

 public:
  virtual ~TemplateCallback(void);
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
  virtual int RealCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr cur_output_buffer, BufferInterfacePtr output_buffer, void *context);

};

}///< namespace ysos    //NOLINT

#endif  ///< TEMPLATE_CALLBACK_H_    //NOLINT