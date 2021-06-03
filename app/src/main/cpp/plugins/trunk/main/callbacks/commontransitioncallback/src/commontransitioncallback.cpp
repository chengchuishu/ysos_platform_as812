/**
  *@file CommonTransitionCallback.h
  *@brief Common Transition Callback
  *@version 1.0
  *@author jinchengzhe
  *@date Created on: 2017-01-03 13:06:50
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * http://www.tangdi.com.cn
  */

/// Private Header //  NOLINT
#include "../include/commontransitioncallback.h"
/// 
#include "../../../../../../core/trunk/main/public/include/plugins/commonenumdefine.h"
/// Standard C++ Header
#include <algorithm>
/// Ysos Headers //  NOLINT
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/moduleinterface.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/jsonutility.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/platforminfo.h"


namespace ysos {

DECLARE_PLUGIN_REGISTER_INTERFACE(CommonTransitionCallback, CallbackInterface);
CommonTransitionCallback::CommonTransitionCallback(const std::string &strClassName /* =CommonTransitionCallback */): BaseTransitionCallbackImpl(strClassName) {

}

CommonTransitionCallback::~CommonTransitionCallback() {

}

int CommonTransitionCallback::SplitConditionTag(const std::string& value, std::list<std::string>& tag_list) {
  int result = YSOS_ERROR_FAILED;

  do {
    if (true == value.empty()) {
      break;
    }

    if (YSOS_ERROR_SUCCESS != GetUtility()->SplitString(value, "|", tag_list)) {
      break;
    }

    if (0 == tag_list.size()) {
      break;
    }

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  return result;
}

int CommonTransitionCallback::Initialized(const std::string &key, const std::string &value)  {
  YSOS_LOG_DEBUG("CommonTransitionCallback::Initialized[Enter] [" << GetName() << "]");
  int result = YSOS_ERROR_FAILED;

  do {
    if (true == key.empty() ||
        true == value.empty()) {
      YSOS_LOG_DEBUG("CommonTransitionCallback::Initialized[Fail][key or value is empty.]");
      break;
    }

    YSOS_LOG_DEBUG("CommonTransitionCallback::Initialized[Check Point][0]");

    /// 解析条件标签
    if ("condition" == key) {
      YSOS_LOG_DEBUG("CommonTransitionCallback::Initialized[Check Point][Enter to parse condition]");

      /// 逐行解析
      std::list<std::string> tag_list;
      if (YSOS_ERROR_SUCCESS != SplitConditionTag(value, tag_list)) {
        YSOS_LOG_DEBUG("CommonTransitionCallback::Initialized[Fail][The function[SplitConditionTag()] is failed.]");
        break;
      }

      /// 以每行4个元素为标准
      if (0 < tag_list.size() && 4 >= tag_list.size()) {
        YSOS_LOG_DEBUG("CommonTransitionCallback::Initialized[Check Point][Initialize condition params]");
        std::list<std::string>::iterator itr = tag_list.begin();

        std::string tag1 = *itr;
        std::string tag2 = *(++itr);
        std::string tag3 = *(++itr);
        std::string tag4 = *(++itr);

        /// 第一个标签将于模块上抛的json字符窜的key值作比较
        specific_tag_set_.insert(tag1);

        /// 插入到条件判断map
        condition_map_.insert(TransitionData(tag2, TransitionEntity(tag1, tag2, tag3, (tag4 == "true" ? true : false))));
      }

      YSOS_LOG_DEBUG("CommonTransitionCallback::Initialized[Check Point][Exit to parse condition]");
    } else if ("exclution_module" == key) {
      YSOS_LOG_DEBUG("CommonTransitionCallback::Initialized[Check Point][Enter to parse exclution module]");

      if (YSOS_ERROR_SUCCESS == GetUtility()->SplitString(value, "|", exclution_module_list_)) {
        YSOS_LOG_DEBUG("CommonTransitionCallback::Initialized[Fail][The function[GetUtility()->SplitString()] is failed.]");
        break;
      }

      YSOS_LOG_DEBUG("CommonTransitionCallback::Initialized[Check Point][Exit to parse exclution module]");
    } else if ("status_event" == key) {
      YSOS_LOG_DEBUG("CommonTransitionCallback::Initialized[Check Point][Enter to parse status event]");

      /// 逐行解析
      std::list<std::string> tag_list;
      if (YSOS_ERROR_SUCCESS != SplitConditionTag(value, tag_list)) {
        YSOS_LOG_DEBUG("CommonTransitionCallback::Initialized[Fail][The function[SplitConditionTag()] is failed.]");
        break;
      }

      /// 以每行4个元素为标准
      if (0 < tag_list.size() && 4 >= tag_list.size()) {
        YSOS_LOG_DEBUG("CommonTransitionCallback::Initialized[Check Point][Initialize status event params]");
        std::list<std::string>::iterator itr = tag_list.begin();

        std::string tag1 = *itr;
        std::string tag2 = *(++itr);
        std::string tag3 = *(++itr);
        std::string tag4 = *(++itr);

        /// 插入到状态码判断map
        status_event_map_.insert(TransitionData(tag1, TransitionEntity(tag1, tag2, tag3, (tag4 == "true" ? true : false))));
      }

      YSOS_LOG_DEBUG("CommonTransitionCallback::Initialized[Check Point][Exit to parse status event]");
    } else {
      YSOS_LOG_DEBUG("CommonTransitionCallback::Initialized[Fail][unknown key]");
      result = BaseTransitionCallbackImpl::Initialized(key, value);
      break;
    }

    YSOS_LOG_DEBUG("CommonTransitionCallback::Initialized[Check Point][condition_map_ size] [" << condition_map_.size() << "]");
    YSOS_LOG_DEBUG("CommonTransitionCallback::Initialized[Check Point][exclution_module_list_ size] [" << exclution_module_list_.size() << "]");
    YSOS_LOG_DEBUG("CommonTransitionCallback::Initialized[Check Point][status_event_map_ size] [" << status_event_map_.size() << "]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  YSOS_LOG_DEBUG("CommonTransitionCallback::Initialized[Exit] [" << GetName() << "]");

  return result;
}

bool CommonTransitionCallback::IsMatched(BufferInterfacePtr input_buffer, TransitionContext *context_ptr) {
  YSOS_LOG_DEBUG("CommonTransitionCallback::IsMatched[Enter] [" << GetName() << "]");

  bool result = false;

  do {
    if (NULL == input_buffer ||
        NULL == context_ptr ||
        NULL == context_ptr->module_ptr) {
      YSOS_LOG_DEBUG("CommonTransitionCallback::IsMatched[Fail][The parameters is wrong.]");
      break;
    }

    YSOS_LOG_DEBUG("CommonTransitionCallback::IsMatched[Check Point][0]");

    std::string module_name = context_ptr->module_ptr->GetName();
    if (true == module_name.empty()) {
      YSOS_LOG_DEBUG("CommonTransitionCallback::IsMatched[Fail][module_name is empty.]");
      break;
    }

    YSOS_LOG_DEBUG("CommonTransitionCallback::IsMatched[Check Point][1]");

    /// 排除需要过滤的模块
    std::list<std::string>::iterator itr1 = std::find(exclution_module_list_.begin(), exclution_module_list_.end(), module_name);
    if (itr1 != exclution_module_list_.end()) {
      YSOS_LOG_DEBUG("CommonTransitionCallback::IsMatched[exclution module][" << *itr1 << "]");
      return true;
    }

    YSOS_LOG_DEBUG("CommonTransitionCallback::IsMatched[Check Point][2]");

    /// 获取模块上抛的数据
    uint8_t* data_ptr = GetBufferUtility()->GetBufferData(input_buffer);
    if (NULL == data_ptr) {
      YSOS_LOG_DEBUG("CommonTransitionCallback::IsMatched[Fail][The function[GetBufferUtility()->GetBufferData()] is failed.]");
      break;
    }
    
    YSOS_LOG_DEBUG("***********CommonTransitionCallback::IsMatched[Check Point][module_name] [" << module_name.c_str() << "]");
    YSOS_LOG_DEBUG("***********CommonTransitionCallback::IsMatched[Check Point][data_ptr] [" << (char*)data_ptr << "]");

    /// 解析模块上抛的数据
    JsonValue json_value_object;
    if (YSOS_ERROR_SUCCESS != GetJsonUtil()->JsonObjectFromString((const char*)data_ptr, json_value_object)) {
      YSOS_LOG_DEBUG("CommonTransitionCallback::IsMatched[Fail][The function[GetJsonUtil()->JsonObjectFromString()] is failed.]");
      break;
    }

    YSOS_LOG_DEBUG("CommonTransitionCallback::IsMatched[Check Point][3]");

    /// 查找json结构体中是否存在要查询的指定关键字和对应的值
    if (true == specific_tag_set_.empty()) {
      YSOS_LOG_DEBUG("CommonTransitionCallback::IsMatched[Check Point][transition_keyword_ is empty.]");
      break;
    }

    YSOS_LOG_DEBUG("CommonTransitionCallback::IsMatched[Check Point][4]");

    std::string specific_tag, specific_tag_value;
    for (std::set<std::string>::iterator itr2 = specific_tag_set_.begin(); itr2 != specific_tag_set_.end(); ++itr2) {
      if (true == json_value_object.isMember(*itr2)) {
        specific_tag = *itr2;
        if (true == specific_tag.empty()) {
          YSOS_LOG_DEBUG("CommonTransitionCallback::IsMatched[Fail][specific_tag is empty.]");
          break;
        }

        YSOS_LOG_DEBUG("CommonTransitionCallback::IsMatched[success][specific_tag][" << specific_tag << "]");

        /// 获取指定关键字对应的值
        JsonValue json_value_specific_tag_value = json_value_object[specific_tag];
        if (true == json_value_specific_tag_value.empty() ||
            true != json_value_specific_tag_value.isString()) {
          YSOS_LOG_DEBUG("CommonTransitionCallback::IsMatched[Fail][json_value_specific_tag_value is empty or is not string.]");
          break;
        }

        specific_tag_value = json_value_specific_tag_value.asString();

        YSOS_LOG_DEBUG("specific_tag_value: [" << specific_tag_value << "]");

        /// 找到了跳出循环
        break;
      }
    }

    YSOS_LOG_DEBUG("CommonTransitionCallback::IsMatched[Check Point][5]");

    /// 没找到对应的关键字和关键字对应的值，则匹配失败跳出
    if (true == specific_tag.empty() ||
        true == specific_tag_value.empty()) {
      YSOS_LOG_DEBUG("CommonTransitionCallback::IsMatched[Fail][specific_tag or specific_tag_value is empty.]");
      break;
    }

    YSOS_LOG_DEBUG("CommonTransitionCallback::IsMatched[Check Point][6]");

    /// 查找关键字对应的值，是否在条件map里
    std::map<std::string, TransitionEntity>::iterator itr3 = condition_map_.find(specific_tag_value);
    if (itr3 == condition_map_.end()) {
      YSOS_LOG_DEBUG("CommonTransitionCallback::IsMatched[Fail][Can't find specific_tag_value][" << specific_tag_value << "]");
      break;
    }

    YSOS_LOG_DEBUG("CommonTransitionCallback::IsMatched[Check Point][7]");

    /// 查找关键字，是否在条件map里
    if (specific_tag != std::get<0>(itr3->second)) {
      YSOS_LOG_DEBUG("CommonTransitionCallback::IsMatched[Fail][Can't match specific_tag][" << specific_tag << "]");
      break;
    }

    YSOS_LOG_DEBUG("###########CommonTransitionCallback::IsMatched[Success]");
    YSOS_LOG_DEBUG("CommonTransitionCallback::IsMatched[specific_tag][" << std::get<0>(itr3->second) << "]");
    YSOS_LOG_DEBUG("CommonTransitionCallback::IsMatched[specific_value][" << std::get<1>(itr3->second) << "]");
    YSOS_LOG_DEBUG("CommonTransitionCallback::IsMatched[relative_event_name][" << std::get<2>(itr3->second) << "]");
    YSOS_LOG_DEBUG("CommonTransitionCallback::IsMatched[switch_flag][" << ((std::get<3>(itr3->second) == true) ? "true" : "false") << "]");

    /// 配置消息类型
    context_ptr->message_type = std::get<2>(itr3->second);

    YSOS_LOG_DEBUG("context_ptr->message_type: [" << context_ptr->message_type << "]");

    /// 如果需要切换页面，则设置以下项
    if (true == std::get<3>(itr3->second)) {
      YSOS_LOG_DEBUG("CommonTransitionCallback::IsMatched[switch_falg]");
      SetSwitchFlag(context_ptr->module_ptr->GetName(), context_ptr);
    }

    YSOS_LOG_DEBUG("CommonTransitionCallback::IsMatched[Check Point][End]");

    result = true;
  } while (0);

  YSOS_LOG_DEBUG("CommonTransitionCallback::IsMatched[Exit] [" << GetName() << "]");

  return result;
}

bool CommonTransitionCallback::IsStatusEventMatched(std::string &status_code, std::string &service_name, BufferInterfacePtr input_buffer, TransitionContext *context_ptr) {
  YSOS_LOG_DEBUG("CommonTransitionCallback::IsStatusEventMatched[Enter] [" << GetName() << "]");

  bool result = false;

  do {
    if (true == status_code.empty() ||
        true == service_name.empty() ||
        NULL == input_buffer ||
        NULL == context_ptr ||
        NULL == context_ptr->module_ptr) {
      YSOS_LOG_DEBUG("CommonTransitionCallback::IsStatusEventMatched[Fail][The parameters is wrong.]");
      break;
    }

    YSOS_LOG_DEBUG("CommonTransitionCallback::IsStatusEventMatched[service_name][" << context_ptr->module_ptr->GetName() << "]");
    YSOS_LOG_DEBUG("CommonTransitionCallback::IsStatusEventMatched[status_code][" << status_code << "]");
    YSOS_LOG_DEBUG("CommonTransitionCallback::IsStatusEventMatched[service_name][" << service_name << "]");

    /// 查找指定的状态码是否在状态map里存在
    std::map<std::string, TransitionEntity>::iterator itr = status_event_map_.find(status_code);
    if (itr == status_event_map_.end()) {
      YSOS_LOG_DEBUG("CommonTransitionCallback::IsStatusEventMatched[Fail][Can't find status_code][" << status_code << "]");
      break;
    }

    YSOS_LOG_DEBUG("CommonTransitionCallback::IsStatusEventMatched[Check Point][0]");

    /// 查找指定的服务名是否匹配
    if (service_name != std::get<1>(itr->second)) {
      YSOS_LOG_DEBUG("CommonTransitionCallback::IsStatusEventMatched[Fail][Can't match service_name][" << std::get<1>(itr->second) << "]");
      break;
    }

    YSOS_LOG_DEBUG("###########CommonTransitionCallback::IsStatusEventMatched[Success]");
    YSOS_LOG_DEBUG("CommonTransitionCallback::IsStatusEventMatched[status_code][" << std::get<0>(itr->second) << "]");
    YSOS_LOG_DEBUG("CommonTransitionCallback::IsStatusEventMatched[service_name][" << std::get<1>(itr->second) << "]");
    YSOS_LOG_DEBUG("CommonTransitionCallback::IsStatusEventMatched[relative_event_name][" << std::get<2>(itr->second) << "]");
    YSOS_LOG_DEBUG("CommonTransitionCallback::IsStatusEventMatched[switch_flag][" << ((std::get<3>(itr->second) == true) ? "true" : "false") << "]");

    /// 设置消息类型
    context_ptr->message_type = std::get<2>(itr->second);

    YSOS_LOG_DEBUG("CommonTransitionCallback::IsStatusEventMatched[Check Point][1]");

    /// 如果需要切换页面，则设置以下项
    if (true == std::get<3>(itr->second)) {
      YSOS_LOG_DEBUG("CommonTransitionCallback::IsStatusEventMatched[set switch falg]");
      SetSwitchFlag(context_ptr->module_ptr->GetName(), context_ptr);
    }

    result = true;
  } while (0);

  YSOS_LOG_DEBUG("CommonTransitionCallback::IsStatusEventMatched[Exit] [" << GetName() << "]");

  return result;
}

int CommonTransitionCallback::SetSwitchFlag(const std::string &module_name, TransitionContext *context_ptr) {
  return BaseTransitionCallbackImpl::SetSwitchFlag(module_name, context_ptr);
}

uint32_t CommonTransitionCallback::GetMessageID(TransitionContext *context_ptr) {
  YSOS_LOG_DEBUG("CommonTransitionCallback::GetMessageType[Enter] [" << GetName() << "]");

  uint32_t result = 0;

  do {
    if (NULL == context_ptr || NULL == context_ptr->module_ptr) {
      YSOS_LOG_DEBUG("CommonTransitionCallback::GetMessageType[Fail][context_ptr or context_ptr->module_ptr is NULL.]");
      break;
    }

    YSOS_LOG_DEBUG("CommonTransitionCallback::GetMessageType[Check Point][0]");

    std::string module_name = context_ptr->module_ptr->GetName();
    if (true == module_name.empty()) {
      YSOS_LOG_DEBUG("MainTransitionCallback::IsMatched[Fail][module_name is empaty]");
      break;
    }

    YSOS_LOG_DEBUG("CommonTransitionCallback::GetMessageType[Check Point][1]");

    /// 排除需要过滤的模块
    for (std::list<std::string>::iterator itr = exclution_module_list_.begin();
         itr != exclution_module_list_.end();
         ++itr) {
      if (*itr == module_name) {
        YSOS_LOG_DEBUG("CommonTransitionCallback::IsMatched[exclution module][" << *itr << "]");
        return BaseTransitionCallbackImpl::GetMessageID(context_ptr);
      }
    }

    YSOS_LOG_DEBUG("CommonTransitionCallback::GetMessageType[Check Point][2]");

    if (true != context_ptr->message_type.empty()) {
      YSOS_LOG_DEBUG("find out message_type: [" << context_ptr->message_type << "]");
      result = GetPlatformInfo()->GetMessageIdByName(context_ptr->message_type);
    }

    YSOS_LOG_DEBUG("CommonTransitionCallback::GetMessageType[Check Point][End]");
  } while (0);

  YSOS_LOG_DEBUG("result: [" << result << "]");

  YSOS_LOG_DEBUG("CommonTransitionCallback::GetMessageType[Exit] [" << GetName() << "]");

  return result;
}

}
