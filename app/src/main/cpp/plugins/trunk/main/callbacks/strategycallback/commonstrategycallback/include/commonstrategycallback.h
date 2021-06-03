/**
 *@file CommonStrategyCallback.h
 *@brief 状态机通用Callback,用于StateMachine层事件、Service回调
 *@version 1.0
 *@author dhongqian
 *@date Created on: 2016-05-11 10:06:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

#ifndef YSOS_PLUGIN_COMMON_STRATEGY_CALLBACK_H_  //NOLINT
#define YSOS_PLUGIN_COMMON_STRATEGY_CALLBACK_H_  //NOLINT

/// Stl Headers
#include <list>
#include <map>
/// Ysos Sdk Headers
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basestrategycallbackimpl.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/conditionentity.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/conditionentitylist.h"
#include "../../../../../../core/trunk/main/protect/include/core_help_package/propertytree.h"

namespace ysos {

/**
 *@brief 通用StrategyCallback
 */
class /*YSOS_EXPORT*/ CommonStrategyCallback : public  BaseStrategyCallbackImpl {
  DECLARE_CREATEINSTANCE(CommonStrategyCallback);
  DISALLOW_COPY_AND_ASSIGN(CommonStrategyCallback);
  DECLARE_PROTECT_CONSTRUCTOR(CommonStrategyCallback);

 public:
  virtual ~CommonStrategyCallback();

// typedef   std::map<std::string, StrategyConditionPtr >               ConditionMap;
/*  typedef std::list<BaseConditionPtr >                                         ConditionList;*/
  typedef std::map<std::string, ConditionEntityListPtr >                                ConditionListMap;

 protected:
  /**
   *@brief  配置文件中设置的Key、Value对，会在Initialized函数中调用 //  NOLINT
   *@param key 配置参数中的Key  // NOLINT
   *@param value 与Key对应的Value  // NOLINT
   *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
   */
  virtual int Initialized(const std::string &key, const std::string &value);
  /**
  *@brief 处理事件的消息  // NOLINT
  *@param event_name[Input]： 事件的类型  // NOLINT
  *@param input_buffer[input]：待处理的数据  // NOLINT
  *@param context[Input]： 回调处理的上下文数据  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int HandleMessage(const std::string &event_name, BufferInterfacePtr input_buffer, CallbackInterface *context);
  /**
  *@brief 解析Condition配置文件  // NOLINT
  *@param config_file[Input]： Condition的配置文件  // NOLINT
  *@param is_before_condition[input]：true 前置条件，false 后置条件  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int ParseConfigureFile(const std::string &config_file, bool is_before_condition=true);
  /**
  *@brief 根据输入的文件名获得XML结构  // NOLINT
  *@param config_str[Input]： 配置文件名  // NOLINT
  *@return： 成功返回PropertyTreePtr，失败返回NULL  // NOLINT
  */
  PropertyTreePtr ParseFile(const std::string &config_file);
  /**
  *@brief 根据名称，获取对应的StrategyCondition，如果StrategyCondition不存在，就先创建一个  // NOLINT
  *@param condition_parent[Input]： StrategyCondition的指针  // NOLINT
  *@param condition_list[input]：条件Condition链表  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  int GetStrategyCondition(TreeNodeIterator condition_parent, ConditionEntityListPtr &condition_list);
  /**
  *@brief 解析具体的Condition  // NOLINT
  *@param condition_ptr[Input]： 待解析的Condition指针  // NOLINT
  *@param condition_list[input]：条件Condition链表  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  int ParseCondition(TreeNodeIterator condition_ptr, ConditionEntityListPtr &condition_list);
  /**
  *@brief 解析并构建Errror Handler Map  // NOLINT
  *@param condition_ptr[Input]： 待解析的Condition指针  // NOLINT
  *@param error_handler_map[input]：解析的结果  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  int ParseErrorHandler(TreeNodeIterator condition_ptr, ConditionListMap &error_handler_map);
  /**
  *@brief 将Condtion加入到Condition Map中  // NOLINT
  *@param condition_ptr[Input]： 待加入的Condition指针  // NOLINT
  *@param error_handler_map[input]：待加入的Condition Map  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  int AddToConditionMap(BaseConditionPtr condition_ptr, ConditionListMap &error_handler_map);
  /**
  *@brief 根据EventName获取Condition  // NOLINT
  *@param event_name[Input]： 待匹配的事件名  // NOLINT
  *@param map_ptr[input]：Condition的链表  // NOLINT
  *@return： 成功返回strategy conditin指针，失败返回NULL  // NOLINT
  */
  //StrategyConditionPtr GetCondition(const std::string &event_name, ConditionMap  *map_ptr);
  /**
  *@brief 处理event_name对应的Condition  // NOLINT
  *@param input_buffer[input]：待处理的数据  // NOLINT
  *@param condition_list[input]：要处理的Condition  // NOLINT
  *@param context[Input]： 回调处理的上下文数据  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  int HandleCondition(BufferInterfacePtr input_buffer, ConditionEntityListPtr &condition_list, CallbackInterface *context);
  /**
  *@brief 继续处理本身的消息  // NOLINT
  *@param input_buffer[input]：待处理的数据  // NOLINT
  *@param context[Input]： 回调处理的上下文数据  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  int HandleSelfMessage(BufferInterfacePtr input_buffer, CallbackInterface *context);
  /**
  *@brief 处理Service本身的消息  // NOLINT
  *@param service_name[input]：要处理的service名称  // NOLINT
  *@param input_buffer[input]：待处理的数据  // NOLINT
  *@param context[Input]： 回调处理的上下文数据  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  int HandleServiceMessage(std::string service_name, BufferInterfacePtr input_buffer, CallbackInterface *context);
  /**
  *@brief 处理Event本身的消息  // NOLINT
  *@param event_name[input]：要处理的event名称  // NOLINT
  *@param input_buffer[input]：待处理的数据  // NOLINT
  *@param context[Input]： 回调处理的上下文数据  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  int HandleEventMessage(std::string event_name, BufferInterfacePtr input_buffer, CallbackInterface *context);

 protected:
  std::string                                                         condition_name_;   ///< condition的Name
  std::string                                                         condition_type_;   ///< condition的Ttype类型:service、event
  bool                                                                is_ignore_self_;   ///< 是否忽略自身的事件
  ConditionEntityListPtr                                              condition_before_list_;   ///< 前置Condition
  ConditionEntityListPtr                                              condition_after_list_;   ///<  后置Condition
  ConditionListMap                                                    error_condition_map_;   ///< 用以处理错误的Condition
  std::string                                                         callback_name_;  ///< 与condition_name_对应的callback_name, 可为空
  std::string                                                         event_name_;     ///< event notify时的事件名
};
}  /// namespace ysos //  NOLINT

#endif  ///<  YSOS_PLUGIN_COMMON_STRATEGY_CALLBACK_H_  //  NOLINT