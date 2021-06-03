/**
  *@file NlpRepeaterCallback.h
  *@brief Nlp Repeater Callback
  *@version 1.0
  *@author Donghongqian
  *@date Created on: 2016-05-31 18:03:27
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  */
#ifndef YSOS_PLUGIN_NLP_REPEATER_CALLBACK_H_
#define YSOS_PLUGIN_NLP_REPEATER_CALLBACK_H_

/// Ysos Headers //  NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/baserepeatercallbackimpl.h"

namespace ysos {

/**
  *@brief  Repeater的基类Callback  //  NOLINT
  *        //  NOLINT
  */
class YSOS_EXPORT NlpRepeaterCallback : public BaseRepeaterCallbackImpl {
  DECLARE_CREATEINSTANCE(NlpRepeaterCallback);
  DISALLOW_COPY_AND_ASSIGN(NlpRepeaterCallback);
  DECLARE_PROTECT_CONSTRUCTOR(NlpRepeaterCallback);

public:
  /**
   *@brief 析构函数
   */
  virtual ~NlpRepeaterCallback();

protected:
  /**
   *@brief 在Callback开始时调用Observer  // NOLINT
   *@param input_buffer[Input]： 输入数据缓冲  // NOLINT
   *@param context[Input]： 回调处理的上下文数据  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int FirstObserver(BufferInterfacePtr input_buffer, void *context);
  /**
   *@brief 在Callback结束前调用Observer  // NOLINT
   *@param input_buffer[Input]： 输入数据缓冲  // NOLINT
   *@param context[Input]： 回调处理的上下文数据  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int LastObserver(BufferInterfacePtr input_buffer, void *context);
  /**
   *@brief 根据输入的信息，决定下一步的动作  // NOLINT
   *@param input_buffer[Input]： 输入数据缓冲  // NOLINT
   *@param output_buffer[Output]：回调处理后输出的数据缓冲  // NOLINT
   *@param callback[Input]： 当前NextCallbackQueue中的NextCallback，NextCallbackQueue中有几个Callback，该函数就会被调用几次  // NOLINT
   *@param context[Input]： 回调处理的上下文数据,是BaseModuelCallbackContext* 类型，与Callback中的Contex同  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Switch(SwitchContext *switch_context, CallbackInterfacePtr callback, void *context);
  /**
   *@brief  配置文件中设置的Key、Value对，会在Initialized函数中调用 //  NOLINT
   *@param key 配置参数中的Key  // NOLINT
   *@param value 与Key对应的Value  // NOLINT
   *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
   */
  virtual int Initialized(const std::string &key, const std::string &value);

protected:
  std::string             condition_;   ///<  要判断的条件 //  NOLINT

};
}
#endif  ///<  YSOS_PLUGIN_NLP_REPEATER_CALLBACK_H_ //  NOLINT