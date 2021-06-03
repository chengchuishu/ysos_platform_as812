/**
  *@file accountsstrategycallback.h
  *@brief Account Strategy Callback
  *@version 1.0
  *@author Xue Xiaojun
  *@date Created on: 2017-06-30 19:33:45
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * http://www.tangdi.com.cn
  */

#ifndef YSOS_PLUGIN_FACE_STRATEGY_CALLBACK_H_
#define YSOS_PLUGIN_FACE_STRATEGY_CALLBACK_H_

/// Ysos Sdk Headers
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemoduleimpl.h"                 //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemodulecallbackimpl.h"          //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basethreadmodulecallbackimpl.h"    //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/conditionentity.h"

namespace ysos {
class ConfigInformationCallback: public BaseThreadModuleCallbackImpl {
	DECLARE_CREATEINSTANCE(ConfigInformationCallback);
	DISALLOW_COPY_AND_ASSIGN(ConfigInformationCallback);
	DECLARE_PROTECT_CONSTRUCTOR(ConfigInformationCallback);

public:
	~ConfigInformationCallback(void);

protected:
    /**
  *@brief 回调接口的实现函数
  *@param input_buffer[Input]： 输入数据缓冲
  *@param output_buffer[Output]：回调处理后输出的数据缓冲
  *@param context[Input]： 回调处理的上下文数据
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值
  */
  int RealCallback(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr cur_output_buffer_ptr,BufferInterfacePtr output_buffer_ptr, void *context);
  

private:
	std::string event_name_;   ///< 上抛事件名称 // NOLINT
	std::string event_callback_name_;   ///< 上抛回调函数名称 // NOLINT
};

}
#endif


