/**
 *@file ImagConvertCallback.h
 *@brief speech output  
 *@version 1.0
 *@author dhongqian
 *@date Created on: 2016-05-26 14:39:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

#ifndef YSOS_PLUGIN_IMG_CONVERT_CALLBACK_H_  //NOLINT
#define YSOS_PLUGIN_IMG_CONVERT_CALLBACK_H_  //NOLINT

/// ysos headers
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemoduleimpl.h"         //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemodulecallbackimpl.h" //NOLINT

namespace ysos {  
  /**
   *@brief  TTS callback // NOLINT
  */
class YSOS_EXPORT ImagConvertCallback : public  BaseModuleCallbackImpl{
DECLARE_CREATEINSTANCE(ImagConvertCallback);
DISALLOW_COPY_AND_ASSIGN(ImagConvertCallback);
DECLARE_PROTECT_CONSTRUCTOR(ImagConvertCallback);

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
  /**
   *@brief  配置文件中设置的Key、Value对，会在Initialized函数中调用 //  NOLINT
   *@param key 配置参数中的Key  // NOLINT
   *@param value 与Key对应的Value  // NOLINT
   *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
   */
  virtual int Initialized(const std::string &key, const std::string &value);
  /**
   *@brief  获取图像的名称 //  NOLINT
   *@return 成功返回图像名称，失败返回空  //  NOLINT
   */
  std::string GetImageName(void);

private:
  std::string     img_path_;  ///< 当前图像保存的目录
  std::string     img_type_;  ///< 图像要保存的数据类型
  std::string     output_pattern_;  ///< 输出数据类型格式
  int             cur_interval_number_;   ///<  当前已间隔的次数，抓拍图像 //  NOLINT
  int             interval_max_;  ///<  间隔多少次，抓捕图像 //  NOLINT


};
} // namespace ysos

#endif    //  YSOS_PLUGIN_IMG_CONVERT_CALLBACK_H_  //NOLINT