/**
 *@file condition entity.h
 *@brief Definition of all the condition entity
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-06-21 08:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef SFP_CONDITION_ENTITY_LIST_H_       //NOLINT
#define SFP_CONDITION_ENTITY_LIST_H_       //NOLINT

/// Platform Headers
#include "../../../protect/include/sys_framework_package/conditionentity.h"

namespace ysos {

class ConditionEntityList;
typedef boost::shared_ptr<ConditionEntityList> ConditionEntityListPtr;
typedef std::map<std::string, ConditionEntityListPtr >           ConditionListMap;

/**
  *@brief condition实例的基类  //NOLINT
  */
class YSOS_EXPORT ConditionEntityList: public BaseInterfaceImpl {
//   DECLARE_CREATEINSTANCE(BaseAgentImpl);
//   DISALLOW_COPY_AND_ASSIGN(BaseAgentImpl);
//   DECLARE_PROTECT_CONSTRUCTOR(BaseAgentImpl);

 public:
  ConditionEntityList(const std::string &strClassName = "ConditionEntityList");
  virtual ~ConditionEntityList(void);

  typedef std::list<BaseConditionPtr >                             ConditionList;

  /**
    *@brief 添加一个Condition  // NOLINT
    *@param condition_ptr[Input]： 待添加的Condition  // NOLINT
    *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
    */
  virtual int AddCondition(BaseConditionPtr condition_ptr);
  /**
  *@brief 删除一个Condition  // NOLINT
  *@param condition_ptr[Input]： 待删除的Condition  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int RemoveCondition(BaseConditionPtr condition_ptr);
  /**
  *@brief 执行Condtion操作  // NOLINT
  *@param context[Input]： 上下文  // NOLINT
  *@param error_handler_map[Input]： 错误处理逻辑  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int DoAction(CallbackInterface *context, ConditionListMap &error_handler_map);
  /**
  *@brief 清空Condition List  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int Clear(void);

 protected:
  /**
  *@brief 执行Condtion操作  // NOLINT
  *@param context[Input]： 上下文  // NOLINT
  *@param tag[Input]： 要处理的Errror tag号  // NOLINT
  *@param error_handler_map[Input]： 错误处理逻辑  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int HandleError(CallbackInterface *context, const std::string &tag, ConditionListMap &error_handler_map);

 protected:
  ConditionList                      condition_list_;   ///< condition链表
  /// ConditionListMap                   error_handler_map_;    ///< error handler list
};

}
#endif  //  SFP_CONDITION_ENTITY_H_   //NOLINT
