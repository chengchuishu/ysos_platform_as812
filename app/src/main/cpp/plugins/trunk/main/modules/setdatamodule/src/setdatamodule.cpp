/**
 *@file SetDataModule.cpp
 *@brief set datawukukk module
 *@version 1.0
 *@author dhongqian
 *@date Created on: 2016-05-26 14:39:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// self header
#include "../include/setdatamodule.h"
/// ysos headers
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/manager_declare.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/jsonutility.h"

namespace ysos {
DECLARE_PLUGIN_REGISTER_INTERFACE(SetDataModule, ModuleInterface);
SetDataModule::SetDataModule(const std::string &strClassName):BaseThreadModuleImpl(strClassName) {
  thread_data_->timeout = 100;
}

//int SetDataModule::Initialized(const std::string &key, const std::string &value)  {
//  if("left" == key) {
//    left_ = GetUtility()->ConvertFromString(value, width_);
//  } else if("top" == key) {
//    top_ = GetUtility()->ConvertFromString(value, width_);
//  } else if("width" == key) {
//    width_ = GetUtility()->ConvertFromString(value, width_);
//  } else if("height" == key) {
//    height_ = GetUtility()->ConvertFromString(value, width_);
//  } else if("title" == key) {
//    title_ = value;
//  }
//
//  return YSOS_ERROR_SUCCESS;
//}

int SetDataModule::InitalDataInfo() {
  return YSOS_ERROR_SUCCESS;
}

int SetDataModule::RealOpen(LPVOID param /* = NULL */) {
  return BaseThreadModuleImpl::RealOpen(param);
}

int SetDataModule::RealRun() {
  return YSOS_ERROR_SUCCESS;
}

int SetDataModule::RealPause() {
  return YSOS_ERROR_SUCCESS;
}

int SetDataModule::RealStop() {
  return YSOS_ERROR_SUCCESS;
}

int SetDataModule::RealClose() {

  return BaseThreadModuleImpl::RealClose();
}

int SetDataModule::Ioctl(INT32 control_id, LPVOID param) {
  int ret = YSOS_ERROR_SUCCESS;

  switch (control_id) {
  case CMD_SET_DATA: {
    if (NULL == param) {
      ret = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }
    ret = SetProperty(PROP_DATA, param);
    if (YSOS_ERROR_SUCCESS == ret) {
      ret = SetProperty(PROP_THREAD_NOTIFY, NULL);
    }
    break;
  }
  case CMD_SET_JSON_DATA: {
    if (NULL == param) {
      ret = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }
    ret = SetProperty(PROP_JSON_DATA, param);
    if (YSOS_ERROR_SUCCESS == ret) {
      ret = SetProperty(PROP_THREAD_NOTIFY, NULL);
    }
    break;
  }
  case CMD_SET_CUSTOM_DATA: {
    if (NULL == param) {
      ret = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }
    ret = SetProperty(PROP_CUSTOM_DATA, param);
    if (YSOS_ERROR_SUCCESS == ret) {
      ret = SetProperty(PROP_THREAD_NOTIFY, NULL);
    }
    break;
  }
  default:
    ret = BaseThreadModuleImpl::Ioctl(control_id, param);
  }

  return ret;
}

int SetDataModule::GetProperty(int iTypeId, void *piType) {
  int ret = YSOS_ERROR_SUCCESS;

  switch (iTypeId) {
  case PROP_DATA: {
    if (data_.empty()) {
      ret = YSOS_ERROR_NOT_EXISTED;
      break;
    }
    BufferInterfacePtr *buffer_ptr_ptr = reinterpret_cast<BufferInterfacePtr*>(piType);
    if (NULL == buffer_ptr_ptr) {
      ret = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }
    BufferInterfacePtr buffer_ptr = *buffer_ptr_ptr;
    if (NULL == buffer_ptr) {
      ret = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }
    uint8_t *data = GetBufferUtility()->GetBufferData(buffer_ptr);
    int buffer_length = GetBufferUtility()->GetBufferLength(buffer_ptr);
    if (NULL == data || 0 == buffer_length || buffer_length <= data_.length()) {
      ret = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }

    memcpy(data, data_.c_str(), data_.length());
    data[data_.length()] = '\0';
    data_ = "";
    break;
  }
  default:
    ret = BaseThreadModuleImpl::GetProperty(iTypeId, piType);
  }

  return ret;
}

int SetDataModule::SetProperty(int type_id, void *type) {
  int ret = YSOS_ERROR_SUCCESS;

  switch (type_id) {
  case PROP_DATA: {
    std::string *data_ptr = reinterpret_cast<std::string*>(type);
    if (NULL == data_ptr) {
      ret = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }

    YSOS_LOG_ERROR("*data_ptr [" << *data_ptr << "]");

    data_ = *data_ptr;
    break;
  }
  case PROP_JSON_DATA: {
    std::string *data_ptr = reinterpret_cast<std::string*>(type);
    if (NULL == data_ptr) {
      ret = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }

    YSOS_LOG_ERROR("*data_ptr [" << *data_ptr << "]");

    JsonValue json_value;

    json_value["text"] = data_ptr->c_str();
    json_value["rc"] = "0";
    json_value["rg"] = "0";
    json_value["answer_best"] = "";
    json_value["service"] = "";
    json_value["other_answers"] = "";

    std::string json_str;
    int ret = GetJsonUtil()->JsonValueToString(json_value, json_str);
    if (YSOS_ERROR_SUCCESS != ret) {
      YSOS_LOG_ERROR("PROP_JSON_DATA wrap to json failed: " << *data_ptr << " | " << logic_name_);
      ret = YSOS_ERROR_FAILED;
      break;
    }

    YSOS_LOG_ERROR("PROP_JSON_DATA json_str: " << *data_ptr << " | " << logic_name_);

    data_ = json_str;
    break;
  }
  case PROP_CUSTOM_DATA: {
    std::string *data_ptr = reinterpret_cast<std::string*>(type);
    if (NULL == data_ptr) {
      ret = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }

    YSOS_LOG_ERROR("*data_ptr [" << *data_ptr << "]");

    JsonValue json_value;

    int ret = GetJsonUtil()->JsonObjectFromString(*data_ptr, json_value);
    if (YSOS_ERROR_SUCCESS != ret) {
      YSOS_LOG_ERROR("PROP_CUSTOM_DATA parse to json failed: " << *data_ptr << " | " << logic_name_);
      ret = YSOS_ERROR_FAILED;
      break;
    }

    if (true == json_value.empty()) {
      YSOS_LOG_ERROR("json_value is empty");
      break;
    }

    std::string text_data;
    if (true == json_value.isMember("text") &&
        true == json_value["text"].isString()) {
      YSOS_LOG_ERROR("parse success");
      text_data = json_value["text"].asString();
    }

    YSOS_LOG_ERROR("text_data [" << text_data << "]");

    JsonValue new_json_value;

    new_json_value["text"] = text_data;
    new_json_value["rc"] = "0";
    new_json_value["rg"] = "0";
    new_json_value["answer_best"] = "";
    new_json_value["service"] = "";
    new_json_value["other_answers"] = "";

    std::string json_str;
    ret = GetJsonUtil()->JsonValueToString(new_json_value, json_str);
    if (YSOS_ERROR_SUCCESS != ret) {
      YSOS_LOG_ERROR("PROP_CUSTOM_DATA wrap to json failed: " << *data_ptr << " | " << logic_name_);
      ret = YSOS_ERROR_FAILED;
      break;
    }

    YSOS_LOG_ERROR("PROP_CUSTOM_DATA json_str: " << *data_ptr << " | " << logic_name_);

    data_ = json_str;
    break;
  }
  default:
    ret = BaseThreadModuleImpl::SetProperty(type_id, type);
  }

  return ret;
}

}