/**
 *@file BaseCallbackImpl.cpp
 *@brief Definition of BaseCallbackImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// ysos private headers //  NOLINT
#include "../../../protect/include/sys_framework_package/basecallbackimpl.h"
#include "../../../protect/include/sys_framework_package/packageconfigimpl.h"
// #include "../../../public/include/core_help_package/ysos_log.h"
#include "../../../public/include/core_help_package/utility.h"
#include "../../../public/include/sys_interface_package/ysos_enum.h"

namespace ysos {

BaseCallbackImpl::BaseCallbackImpl(const std::string &name): BaseInterfaceImpl(name) {  // NOLINT
  logger_ = GetUtility()->GetLogger("ysos.plugin");
  input_type_ = output_type_ = CallbackDataTypeAll;
}

BaseCallbackImpl::~BaseCallbackImpl() {
}


int BaseCallbackImpl::Initialized(const std::string &key, const std::string &value) {
  return YSOS_ERROR_NOT_SUPPORTED;
}

int BaseCallbackImpl::ConfigInitialize(void) {
  MapPtr conf_map = GetPackageConfigImpl()->FindCallbackConf(logic_name_);
  if (NULL == conf_map) {
    YSOS_LOG_DEBUG("Callback " << logic_name_ << " doesn't has a config file");

    return YSOS_ERROR_NOT_SUPPORTED;
  }

  int ret = YSOS_ERROR_SUCCESS;
  for (MapIterator it=conf_map->begin(); it!=conf_map->end(); ++it) {
    ret = YSOS_ERROR_SUCCESS;
    if ("input_type" == it->first) {
      input_type_ = it->second;
    } else if ("output_type" == it->first) {
      output_type_ = it->second;
    } else if ("version" == it->first) {
      version_ = it->second;
    } else if ("remark" == it->first) {
      remark_ = it->second;
    } else if ("class_name" == it->first) {
      /// ignore
    } else if ("name" == it->first) {
      /// ignore
    } else {
      ret = Initialized(it->first, it->second);
    }

    if (YSOS_ERROR_SUCCESS != ret) {
      YSOS_LOG_DEBUG("Initialized failed: [" << it->first << "|" << it->second << "] : " <<ret);
    }
  }

  return ret;
}

int BaseCallbackImpl::Initialize(void *param) {
  if (IsInitialized()) {
    return YSOS_ERROR_FINISH_ALREADY;
  }

  ConfigInitialize();

  return BaseInterfaceImpl::Initialize(param);
}

int BaseCallbackImpl::Callback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, void *context) {  // NOLINT

  return YSOS_ERROR_SUCCESS;
}

int BaseCallbackImpl::SetType(const CallbackDataType input_type, const CallbackDataType output_type) {  // NOLINT
  input_type_ = input_type;
  output_type_ = output_type;

  return YSOS_ERROR_SUCCESS;
}

int BaseCallbackImpl::GetType(CallbackDataType *input_type, CallbackDataType *output_type) {
  if (NULL != input_type) {
    *input_type = input_type_;
  }

  if (NULL != output_type) {
    *output_type = output_type_;
  }

  return YSOS_ERROR_SUCCESS;
}

int BaseCallbackImpl::GetProperty(int type_id, void *type) {
  int ret = YSOS_ERROR_SUCCESS;

  switch (type_id) {
  case PROP_VERSION: {
    std::string *version = static_cast<std::string*>(type);
    if (NULL == version) {
      ret = YSOS_ERROR_NOT_SUPPORTED;
      break;
    }
    *version = version_;
    ret = YSOS_ERROR_SUCCESS;
    break;
  }
  default:
    ret = YSOS_ERROR_NOT_SUPPORTED;
    break;
  }

  return ret;
}

int BaseCallbackImpl::IsReady() {
  return YSOS_ERROR_SUCCESS;
}
}  // NOLINT