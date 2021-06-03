/**   //NOLINT
  *@file PluginBaseThreadModuleCallbackImpl.h
  *@brief Definition of 
  *@version 0.1
  *@author wanfenfen
  *@date Created on: 2016:12:15   13:54
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  *@howto Usage:
  *@todo
  */
#ifndef CALLBACK_PLUGINBASEMODULECALLBACKIMPL_H  //NOLINT
#define CALLBACK_PLUGINBASEMODULECALLBACKIMPL_H  //NOLINT

/// ysos headers
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemoduleimpl.h"         //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemodulecallbackimpl.h" //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basethreadmodulecallbackimpl.h"
 

namespace ysos {
/**
 *@brief  ASR callback // NOLINT
*/
class YSOS_EXPORT PluginBaseThreadModuleCallbackImpl : public /*BaseModuleCallbackImpl*/BaseThreadModuleCallbackImpl {
 // DECLARE_CREATEINSTANCE(PluginBaseThreadModuleCallbackImpl);
  DECLARE_PROTECT_CONSTRUCTOR(PluginBaseThreadModuleCallbackImpl);

 public:
  /**
   *@brief 回调处理是否准备好  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  int IsReady();

 protected:

   virtual int Initialized(const std::string &key, const std::string &value);

     /**
  *@brief 回调接口的实现函数  // NOLINT
  *@param input_buffer[Input]： 输入数据缓冲  // NOLINT
  *@param output_buffer[Output]：回调处理后输出的数据缓冲  // NOLINT
  *@param context[Input]： 回调处理的上下文数据  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
   int RealCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, BufferInterfacePtr external_buf, void *context);

 protected:
  std::string input_ability_name_;
  std::string output_ability_name_;
};
 
} // namespace ysos

#endif    //MODULE_ASR_H  //NOLINT