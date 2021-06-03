/*@version 1.0
  *@author dhongqian
  *@date Created on: 2016-06-23 10:06:20
  *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
  * 
*/
/// Private Headers //  NOLINT
#include "../../../protect/include/sys_framework_package/baseconnectionimpl.h"
#include "../../../public/include/core_help_package/utility.h"
#include "../../../protect/include/sys_framework_package/packageconfigimpl.h"
#include "../../../public/include/sys_interface_package/ysos_enum.h"

namespace ysos {

BaseConnectionImpl::BaseConnectionImpl(const std::string &strClassName /* =BaseDriverImpl */): BaseInterfaceImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.plugin");
  is_enable_write_ = is_enable_read_ = true;
  is_enable_wrap_ = false;
}

BaseConnectionImpl::~BaseConnectionImpl(void) {
}

int BaseConnectionImpl::Initialized(const std::string &key, const std::string &value) {
  return YSOS_ERROR_NOT_SUPPORTED;
}

int BaseConnectionImpl::ConfigInitialize(void) {
  MapPtr conf_map = GetPackageConfigImpl()->FindConnectionConf(logic_name_);
  if (NULL == conf_map) {
    YSOS_LOG_DEBUG("Connection " << logic_name_ << " doesn't has a config file");

    return YSOS_ERROR_NOT_SUPPORTED;
  }

  int ret = YSOS_ERROR_SUCCESS;
  for (MapIterator it=conf_map->begin(); it!=conf_map->end(); ++it) {
    ret = YSOS_ERROR_SUCCESS;
    if ("version" == it->first) {
      version_ = it->second;
    } else if ("remark" == it->first) {
      remark_ = it->second;
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

int BaseConnectionImpl::Initialize(void *param) {
  if (IsInitialized()) {
    return YSOS_ERROR_FINISH_ALREADY;
  }

  ConfigInitialize();

  return BaseInterfaceImpl::Initialize(param);
}

int BaseConnectionImpl::Read(BufferInterfacePtr input_buffer_ptr, int input_length, void* context_ptr) {
  return YSOS_ERROR_SUCCESS;
}

int BaseConnectionImpl::Write(BufferInterfacePtr input_buffer_ptr, int input_length, void* context_ptr) {
  return YSOS_ERROR_SUCCESS;
}

void BaseConnectionImpl::EnableWrite(bool is_enable) {
  is_enable_write_ = is_enable;
}

void BaseConnectionImpl::EnableRead(bool is_enable) {
  is_enable_read_ = is_enable;
}

void BaseConnectionImpl::EnableWrap(bool is_enable) {
  is_enable_wrap_ = is_enable;
}

int BaseConnectionImpl::Open(void *params) {
  return YSOS_ERROR_SUCCESS;
}

void BaseConnectionImpl::Close(void *params) {
  return ;
}

}
