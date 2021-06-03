/**   //NOLINT
  *@file videocapturemodulecallback.h
  *@brief Definition of 
  *@version 0.1
  *@author wanfenfen
  *@date Created on: 2016:12:6   13:47
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  *@howto Usage:
  *@todo
  */
#ifndef MODULE_VIDEOCAPTURE_CALLBACK_H_  //NOLINT
#define MODULE_VIDEOCAPTURE_CALLBACK_H_  //NOLINT

/// ysos headers
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemoduleimpl.h"         //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemodulecallbackimpl.h" //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basethreadmodulecallbackimpl.h"

/// private headers
#include "../../../public/include/common/pluginbasethreademodulecallbackimpl.h"

namespace ysos {  
  /**
   *@brief  TTS callback // NOLINT
  */
class YSOS_EXPORT VideoCaptureModuleCallback : public  /*BaseModuleCallbackImpl*//*BaseThreadModuleCallbackImpl*/PluginBaseThreadModuleCallbackImpl{
DECLARE_CREATEINSTANCE(VideoCaptureModuleCallback);
DISALLOW_COPY_AND_ASSIGN(VideoCaptureModuleCallback);
DECLARE_PROTECT_CONSTRUCTOR(VideoCaptureModuleCallback);

public:

  /**
   *@brief 回调处理是否准备好  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  //int IsReady();

protected:

    /**
   *@brief 回调接口的实现函数  // NOLINT
   *@param input_buffer[Input]： 输入数据缓冲  // NOLINT
   *@param output_buffer[Output]：回调处理后输出的数据缓冲  // NOLINT
   *@param context[Input]： 回调处理的上下文数据  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  //int RealCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, BufferInterfacePtr external_buf, void *context);


private:
 
};
} // namespace ysos

#endif    //MODULE_TTS_CALLBACK_H_  //NOLINT