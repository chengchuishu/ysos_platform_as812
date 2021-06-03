/**
 *@file audiostrategycallback.h
 *@brief the audio strategy call back
 *@version 1.0
 *@author l.j..
 *@date Created on: 2016-10-20 15:16:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

#ifndef IOCTL_CALLBACK_H_
#define IOCTL_CALLBACK_H_

#include <map>
#include <json/json.h>

/// Private Headers //  NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemoduleimpl.h"  //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemodulecallbackimpl.h"  //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basestrategycallbackimpl.h"  //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/packageconfigimpl.h"
#include "../../../../../../core/trunk/main/protect/include/os_hal_package/datainterfacemanager.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"

namespace ysos {

/**
 *@brief  audio策略层 //NOLINT
*/
class YSOS_EXPORT AudioStrategyCallback : public BaseStrategyCallbackImpl {
  DECLARE_CREATEINSTANCE(AudioStrategyCallback);
  DISALLOW_COPY_AND_ASSIGN(AudioStrategyCallback);
  DECLARE_PROTECT_CONSTRUCTOR(AudioStrategyCallback);

 public:
  ~AudioStrategyCallback(void);

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
  *@brief 请求平台服务  // NOLINT
  *@param service_name[Input]： 服务名  // NOLINT
  *@param id[Input]： 命令号  // NOLINT
  *@param type[Input]： 参数类型  // NOLINT
  *@param json_value[Input]： 参数数据  // NOLINT
  *@param context[Input]： 回调处理的上下文数据  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int RequestService(std::string service_name, std::string id, std::string type, std::string json_value, CallbackInterface *context);

  int EventNotify(const std::string &answer, const std::string &service, const std::string &event_name, CallbackInterface *context);

 private:
  std::map<std::string, std::string> menu_code_;        //< 菜单编码
  std::map<std::string, std::string> notify_event_;     //< 前端事件
  DataInterfacePtr data_ptr_;                           //< 内存指针
  LightLock lock_;                                      //< 互斥变量

};

}///< namespace ysos    //NOLINT

#endif///< AUDIO_CALLBACK_H_    //NOLINT