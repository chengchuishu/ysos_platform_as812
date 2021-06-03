/**
*@file powermanagermodulecallback.h
*@brief Power Manager Module
*@version 1.0
*@author wangxg
*@date Created on: 2016-11-28 13:47:00
*@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
* 
*/

#ifndef YSOS_PLUGIN_MODULE_CALLBACK_POWERMANAGER_H_  //NOLINT
#define YSOS_PLUGIN_MODULE_CALLBACK_POWERMANAGER_H_  //NOLINT

#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemoduleimpl.h"                  //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemodulecallbackimpl.h"          //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basethreadmodulecallbackimpl.h"    //NOLINT


namespace ysos {
/**
 *@brief  Power Manager Callback // NOLINT
*/
class YSOS_EXPORT PowerManagerModuleCallback : public BaseThreadModuleCallbackImpl {
  DECLARE_CREATEINSTANCE(PowerManagerModuleCallback);
  DISALLOW_COPY_AND_ASSIGN(PowerManagerModuleCallback);
  DECLARE_PROTECT_CONSTRUCTOR(PowerManagerModuleCallback);


public:
  ~PowerManagerModuleCallback();

  /**
  *@brief 回调处理是否准备好                                   // NOLINT
  *@param ： 无                                                // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值    // NOLINT
  */
  int IsReady();

 protected:
  /**
  *@brief 回调接口的实现函数                                // NOLINT
  *@param input_buffer[Input]： 输入数据缓冲                // NOLINT
  *@param output_buffer[Output]：回调处理后输出的数据缓冲   // NOLINT
  *@param context[Input]： 回调处理的上下文数据             // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  int RealCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr cur_output_buffer,BufferInterfacePtr output_buffer, void *context);


private:
  int GetJsonResult(int barrery_mode,int barrery_elect,std::string &strjson_result);

};


} // namespace ysos

#endif    //YSOS_PLUGIN_MODULE_CALLBACK_POWERMANAGER_H_  //NOLINT