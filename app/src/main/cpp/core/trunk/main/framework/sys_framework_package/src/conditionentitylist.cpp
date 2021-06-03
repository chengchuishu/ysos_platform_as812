/**
 *@file condition entity.h
 *@brief Definition of all the condition entity
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-06-21 08:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// Platform Headers
#include "../../../protect/include/sys_framework_package/conditionentitylist.h"
/// Ysos Sdk Headers
#include "../../../public/include/core_help_package/utility.h"
#include "../../../public/include/sys_interface_package/ysos_enum.h"
#include "../../../public/include/sys_interface_package/common.h"

namespace ysos {
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////          Base Condition                         ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ConditionEntityList::ConditionEntityList(const std::string &strClassName): BaseInterfaceImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.framework.strategy");
}

ConditionEntityList::~ConditionEntityList() {
  condition_list_.clear();
}

int ConditionEntityList::AddCondition(BaseConditionPtr condition_ptr) {
  assert(NULL != condition_ptr);

  ConditionList::iterator it = condition_list_.begin();
  for (; condition_list_.end()!=it; ++it) {
    BaseConditionPtr condition = *it;
    if (condition == condition_ptr) {
      return YSOS_ERROR_HAS_EXISTED;
    }
  }
  condition_list_.push_back(condition_ptr);

  return YSOS_ERROR_SUCCESS;
}

int ConditionEntityList::RemoveCondition(BaseConditionPtr condition_ptr) {
  assert(NULL != condition_ptr);

  ConditionList::iterator it = condition_list_.begin();
  for (; condition_list_.end()!=it; ++it) {
    BaseConditionPtr condition = *it;
    if (condition == condition_ptr) {
      condition_list_.erase(it);
    }
  }

  return YSOS_ERROR_SUCCESS;
}

int ConditionEntityList::DoAction(CallbackInterface *context, ConditionListMap &error_handler_map) {
  assert(NULL != context);

  int ret = YSOS_ERROR_SUCCESS;
  ConditionList::iterator it = condition_list_.begin();
  for (; condition_list_.end()!=it; ++it) {
    BaseConditionPtr condition_ptr = *it;
    ret = condition_ptr->DoAction(context);
    if (YSOS_ERROR_SUCCESS != ret && YSOS_ERROR_SKIP != ret) {
      std::string tag = condition_ptr->GetTag();
      ret = HandleError(context, tag, error_handler_map);
    }
    if (YSOS_ERROR_SUCCESS != ret && YSOS_ERROR_SKIP != ret) {
      return ret;
    }
  }

  return YSOS_ERROR_SUCCESS;
}

int ConditionEntityList::Clear(void) {
  condition_list_.clear();

  return YSOS_ERROR_SUCCESS;
}

int ConditionEntityList::HandleError(CallbackInterface *context, const std::string &tag, ConditionListMap &error_handler_map) {
  ConditionListMap::iterator it = error_handler_map.find(tag);
  if (error_handler_map.end() == it) {
    /// 没有异常处理时，认为不需要处理，返回0
    return YSOS_ERROR_SUCCESS;
  }

  ConditionListMap                   empty_handler_map;
  ConditionEntityListPtr condition_list_ptr = it->second;
  int ret = condition_list_ptr->DoAction(context, empty_handler_map);

  return ret;
}

}
