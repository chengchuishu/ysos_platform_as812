/**
 *@file audioplayextcallback.h
 *@brief the audio play ext call back
 *@version 1.0
 *@author l.j..
 *@date Created on: 2016-10-20 15:16:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

#ifndef AUDIOPLAYEXT_CALLBACK_H_
#define AUDIOPLAYEXT_CALLBACK_H_

/// boost headers
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/atomic.hpp>
#include <boost/thread/mutex.hpp>

/// Private Headers //  NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemoduleimpl.h"  //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basethreadmodulecallbackimpl.h"  //NOLINT
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"

namespace ysos {

/**
 *@brief  语音捕获扩展模块回调层 //NOLINT
*/
class YSOS_EXPORT AudioPlayExtCallback : public BaseModuleCallbackImpl {
  DECLARE_CREATEINSTANCE(AudioPlayExtCallback);
  DISALLOW_COPY_AND_ASSIGN(AudioPlayExtCallback);
  DECLARE_PROTECT_CONSTRUCTOR(AudioPlayExtCallback);

 public:
  virtual ~AudioPlayExtCallback(void);

 protected:
  /**
  *@brief 回调接口的实现函数  // NOLINT
  *@param input_buffer[Input]： 输入数据缓冲  // NOLINT
  *@param output_buffer[Output]：回调处理后输出的数据缓冲  // NOLINT
  *@param context[Input]： 回调处理的上下文数据  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int RealCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr cur_output_buffer, BufferInterfacePtr output_buffer, void *context);

  /**
  *@brief  配置文件中设置的Key、Value对，会在Initialized函数中调用 //  NOLINT
  *@param key 配置参数中的Key  // NOLINT
  *@param value 与Key对应的Value  // NOLINT
  *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
  */
  virtual int Initialized(const std::string &key, const std::string &value);

 private:
  std::string input_ability_name_;
  std::string output_ability_name_;

};

}///< namespace ysos    //NOLINT

#endif///< AUDIOPLAYEXT_CALLBACK_H_    //NOLINT
