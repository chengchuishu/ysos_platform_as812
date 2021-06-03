/**
 *@file BaseProtocolImpl.cpp
 *@brief Definition of Base64ProtocolImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-05-09 13:12:58
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// Self Header
#include "../../../protect/include/sys_framework_package/baseprotocolimpl.h"
#include "../../../protect/include/sys_framework_package/packageconfigimpl.h"
#include "../../../public/include/core_help_package/utility.h"

namespace ysos {

BaseProtocolImpl::BaseProtocolImpl(const std::string &strClassName /* =BaseProtocolImpl */): BaseInterfaceImpl(strClassName) {

}

BaseProtocolImpl::~BaseProtocolImpl() {

}

int BaseProtocolImpl::Initialized(const std::string &key, const std::string &value) {
  return YSOS_ERROR_NOT_SUPPORTED;
}

int BaseProtocolImpl::ConfigInitialize(void) {
  MapPtr conf_map = GetPackageConfigImpl()->FindProtocolConf(logic_name_);
  if (NULL == conf_map) {
    YSOS_LOG_DEBUG("Protocol " << logic_name_ << " doesn't has a config file");

    return YSOS_ERROR_NOT_SUPPORTED;
  }

  int ret = YSOS_ERROR_SUCCESS;
  for (MapIterator it=conf_map->begin(); it!=conf_map->end(); ++it) {
    ret = YSOS_ERROR_SUCCESS;
    if ("version" == it->first) {
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

int BaseProtocolImpl::Initialize(void *param) {
  if (IsInitialized()) {
    return YSOS_ERROR_FINISH_ALREADY;
  }

  ConfigInitialize();

  return BaseInterfaceImpl::Initialize(param);
}

int BaseProtocolImpl::ParseMessage(BufferInterfacePtr in_buffer, BufferInterfacePtr out_buffer, ProtocolFormatId format_id, void* context_ptr) {
  return YSOS_ERROR_NOT_SUPPORTED;
}

int BaseProtocolImpl::FormMessage(BufferInterfacePtr in_buffer, BufferInterfacePtr out_buffer, ProtocolFormatId format_id, void* context_ptr) {
  return YSOS_ERROR_NOT_SUPPORTED;
}

int BaseProtocolImpl::GetLength(BufferInterfacePtr in_buffer, ProtocolFormatId format_id) {
  return 0;
}
}