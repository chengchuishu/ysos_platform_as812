/**
  *@file CommonTransitionCallback.h
  *@brief Common Transition Callback
  *@version 1.0
  *@author jinchengzhe
  *@date Created on: 2017-01-03 13:06:50
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * http://www.tangdi.com.cn
  */
#ifndef YSOS_PLUGIN_COMMON_TRANSITION_CALLBACK_H_
#define YSOS_PLUGIN_COMMON_TRANSITION_CALLBACK_H_

/// Ysos Headers //  NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basecallbackimpl.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basetransitioninfoimpl.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basetransitioncallbackimpl.h"

namespace ysos {

/**
  *@brief  CommonTransitionCallback的具体实现//  NOLINT
  */
class CommonTransitionCallback: public BaseTransitionCallbackImpl {
  DECLARE_CREATEINSTANCE(CommonTransitionCallback);
  DISALLOW_COPY_AND_ASSIGN(CommonTransitionCallback);
  DECLARE_PROTECT_CONSTRUCTOR(CommonTransitionCallback);

 public:
  /**
  *@brief  定义条件和状态码数据结构//  NOLINT
  */
  typedef std::tuple<std::string, std::string, std::string, bool> TransitionEntity;
  typedef std::pair<std::string, TransitionEntity> TransitionData;

  /**
   *@brief 析构函数
   */
  virtual ~CommonTransitionCallback();
 protected:
  /**
  *@brief  配置文件中设置的Key、Value对，会在Initialized函数中调用 //  NOLINT
  *@param key 配置参数中的Key  // NOLINT
  *@param value 与Key对应的Value  // NOLINT
  *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
  */
  virtual int Initialized(const std::string &key, const std::string &value);
  /**
  *@brief 判断input_buffer里的信息，是否满足条件   // NOLINT
  *@param  input_ptr 待匹配的数据   //  NOLINT
  *@param  context_ptr 上下文，可以为NULL //  NOLINT
  *@return 成功返回true，失败返回false  //  NOLINT
  */
  virtual bool IsMatched(BufferInterfacePtr input_buffer, TransitionContext *context_ptr);
  /**
  *@brief 判断input_buffer里的信息，是否满足条件   // NOLINT
  *@param  status_code 状态值   //  NOLINT
  *@param  service_name 服务名 //  NOLINT
  *@param  input_ptr 待匹配的数据   //  NOLINT
  *@param  context_ptr 上下文，可以为NULL //  NOLINT
  *@return 成功返回true，失败返回false  //  NOLINT
  */
  virtual bool IsStatusEventMatched(std::string &status_code, std::string &service_name, BufferInterfacePtr input_buffer, TransitionContext *context_ptr);
  /**
  *@brief 设置状态满足切换的标识  //  NOLINT
  *@param  module_name 数据所属的Module //  NOLINT
  *@param context_ptr 上下文，可以为NULL //  NOLINT
  *@return 成功返回0，失败返回其他值  //  NOLINT
  */
  virtual int SetSwitchFlag(const std::string &module_name, TransitionContext *context_ptr);
  /**
  *@brief 通过Context信息获取获取消息头的类型
  *@param context_ptr 上下文，可以为NULL //  NOLINT
  *@return 成功返回消息头的类型，失败返回0  //  NOLINT
  */
  virtual uint32_t GetMessageID(TransitionContext *context_ptr);
 private:
  /**
  *@brief 解析condition/status_event标签
  *@param value 标签内容，字符窜 //  NOLINT
  *@param tag_list 以|为分隔符，分解的字符段 //  NOLINT
  *@return 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  //  NOLINT
  */
  int SplitConditionTag(const std::string& value, std::list<std::string>& tag_list);

  std::list<std::string> exclution_module_list_;  ///< 保存排除模块名的列表
  std::map<std::string, TransitionEntity> condition_map_;  ///< 保存条件判断内容
  std::map<std::string, TransitionEntity> status_event_map_;  ///< 保存状态码判断内容
  std::set<std::string> specific_tag_set_;  ///< 保存condition标签中第一个字段，作为比较模块上抛json数据中的key值
};

}

#endif  /// YSOS_PLUGIN_COMMON_TRANSITION_CALLBACK_H_
