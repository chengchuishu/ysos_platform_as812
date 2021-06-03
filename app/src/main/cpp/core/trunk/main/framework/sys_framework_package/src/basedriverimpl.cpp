/*@version 1.0
  *@author dhongqian
  *@date Created on: 2016-06-23 10:06:20
  *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
  * 
*/
/// Private Headers //  NOLINT
#include "../../../protect/include/sys_framework_package/basedriverimpl.h"
#include "../../../public/include/core_help_package/utility.h"
#include "../../../protect/include/sys_framework_package/packageconfigimpl.h"
#include "../../../public/include/sys_interface_package/ysos_enum.h"

namespace ysos {

BaseDriverImpl::BaseDriverImpl(const std::string &strClassName /* =BaseDriverImpl */): BaseInterfaceImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.plugin");
}

BaseDriverImpl::~BaseDriverImpl(void) {
}

int BaseDriverImpl::Initialized(const std::string &key, const std::string &value) {
  return YSOS_ERROR_NOT_SUPPORTED;
}

int BaseDriverImpl::ConfigInitialize(void) {
  MapPtr conf_map = GetPackageConfigImpl()->FindDriverConf(logic_name_);
  if (NULL == conf_map) {
    YSOS_LOG_DEBUG("Driver " << logic_name_ << " doesn't has a config file");

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

int BaseDriverImpl::Initialize(void *param) {
  if (IsInitialized()) {
    return YSOS_ERROR_FINISH_ALREADY;
  }

  ConfigInitialize();

  return BaseInterfaceImpl::Initialize(param);
}

int BaseDriverImpl::Read(BufferInterfacePtr pBuffer, DriverContexPtr context_ptr) {
  return YSOS_ERROR_SUCCESS;
}

int BaseDriverImpl::Write(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, DriverContexPtr context_ptr) {
  return YSOS_ERROR_SUCCESS;
}

int BaseDriverImpl::Ioctl(int iCtrlID, BufferInterfacePtr pInputBuffer, BufferInterfacePtr pOutputBuffer) {
  return YSOS_ERROR_SUCCESS;
}

int BaseDriverImpl::Open(void *pParams) {
  return YSOS_ERROR_SUCCESS;
}

void BaseDriverImpl::Close(void *pParams) {
  return ;
}

int BaseDriverImpl::UnInitialize(void *param) {
  status_fun_.clear();
  status_notify_fun_.clear();

  return BaseInterfaceImpl::UnInitialize(param);
}

int BaseDriverImpl::SetProperty(int type_id, void *type) {
  int ret = YSOS_ERROR_SUCCESS;

  switch (type_id) {
  case PROP_STATUS_EVENT: {
    status_event_fun *fun = reinterpret_cast<status_event_fun*>(type);
    status_fun_ = *fun;
  }
  break;
  case PROP_NOTIFY_STATUS_EVENT: {
    status_notify_event_fun *fun = reinterpret_cast<status_notify_event_fun*>(type);
    status_notify_fun_ = *fun;
  }
  break;
  default:
    ret = BaseInterfaceImpl::SetProperty(type_id, type);
  }

  return ret;
}

int BaseDriverImpl::SendStatusEvent(const std::string &status_event_code, const std::string &detail) {
  if (status_fun_.empty()) {
    YSOS_LOG_ERROR("status event callback is null");
    return YSOS_ERROR_NOT_SUPPORTED;
  }

  return status_fun_(status_event_code, detail);
}

int BaseDriverImpl::NotifyStatusEvent(const std::string &status_event_code, const std::string &module_name, const std::string &detail) {
  if (status_notify_fun_.empty()) {
    YSOS_LOG_ERROR("notify status event callback is null");
    return YSOS_ERROR_NOT_SUPPORTED;
  }

  return status_notify_fun_(status_event_code, module_name, detail);
}

}
