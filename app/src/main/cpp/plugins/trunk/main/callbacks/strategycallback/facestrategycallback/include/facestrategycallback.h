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
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basestrategycallbackimpl.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/conditionentity.h"

namespace ysos {
class FaceStrategyCallback: public BaseStrategyCallbackImpl {
	DECLARE_CREATEINSTANCE(FaceStrategyCallback);
	DISALLOW_COPY_AND_ASSIGN(FaceStrategyCallback);
	DECLARE_PROTECT_CONSTRUCTOR(FaceStrategyCallback);

public:
	~FaceStrategyCallback(void);

protected:
  /**
  *@brief 处理事件的消息  // NOLINT
  *@param event_name[Input]： 事件的类型  // NOLINT
  *@param input_buffer[input]：待处理的数据  // NOLINT
  *@param context[Input]： 回调处理的上下文数据  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int HandleMessage(const std::string &event_name, BufferInterfacePtr input_buffer, CallbackInterface *context);

    /**
  *@brief  配置文件中设置的Key、Value对，会在Initialized函数中调用 //  NOLINT
  *@param key 配置参数中的Key  // NOLINT
  *@param value 与Key对应的Value  // NOLINT
  *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
  */
  virtual int Initialized(const std::string &key, const std::string &value);

private:
  /**
  *@brief 分析人脸信息数据  // NOLINT
  *@param fece_info[Input]： 人脸的信息  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  int ChangePeopleState(const std::string& fece_info,std::string &output_str);

private:
	unsigned int base_nobody_number_; ///< 无人的连续的次数 // NOLINT
	unsigned int base_somebody_number_; ///< 有人的连续的次数 // NOLINT
	std::string event_name_;   ///< 上抛事件名称 // NOLINT
	std::string event_callback_name_;   ///< 上抛回调函数名称 // NOLINT
};

}
#endif