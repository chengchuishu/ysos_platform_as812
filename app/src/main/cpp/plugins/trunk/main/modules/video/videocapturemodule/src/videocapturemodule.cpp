/**   //NOLINT
  *@file videocapturemodule.cpp
  *@brief Definition of
  *@version 0.1
  *@author wanfenfen
  *@date Created on: 2016:12:1   18:49
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  *@howto Usage:

  *@todo

  */
/// self header
#include "../include/videocapturemodule.h"

/// boost headers
#include <boost/thread/thread.hpp>

/// ysos headers
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/manager_declare.h"

/// private headers
#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"

namespace ysos {
DECLARE_PLUGIN_REGISTER_INTERFACE(VideoCaptureModule, ModuleInterface);
VideoCaptureModule::VideoCaptureModule(const std::string &strClassName)
    : PluginBaseThreadModuleImpl(strClassName),
      rotate_angle_(0) {
  logger_ = GetUtility()->GetLogger("ysos.video");
}

int VideoCaptureModule::Initialized(
    const std::string &key, const std::string &value) {
#ifdef _WIN32
  if (stricmp(key.c_str(), "rotation") == 0) {
    rotate_angle_ = atoi(value.c_str());
  } else {
    return PluginBaseThreadModuleImpl::Initialized(key, value);
  }
#else
  YSOS_LOG_DEBUG("VideoCaptureModule Initialized :::: key: " << key);
  if (strcasecmp(key.c_str(), "rotation") == 0) {
    rotate_angle_ = atoi(value.c_str());
    YSOS_LOG_DEBUG("VideoCaptureModule Initialized :::: rotate_angle_: " << rotate_angle_);
    return YSOS_ERROR_SUCCESS;
  } else {
    return PluginBaseThreadModuleImpl::Initialized(key, value);
  }
#endif
}

int VideoCaptureModule::RealOpen(LPVOID param) {
  int ret = PluginBaseThreadModuleImpl::RealOpen(param);
  if (ret != YSOS_ERROR_SUCCESS || !driver_ptr_.get())
    return ret;

  driver_ptr_->SetProperty(PROP_ROTATE_ANGLE, &rotate_angle_);
  return YSOS_ERROR_SUCCESS;
}

int VideoCaptureModule::GetProperty(int type_id, void *type) {
  switch (type_id) {
    case PROP_DATA:
      return driver_ptr_->GetProperty(PROP_DATA, type);
  }
  return PluginBaseThreadModuleImpl::GetProperty(type_id, type);
}

int VideoCaptureModule::Ioctl(INT32 control_id, LPVOID param) {
  YSOS_LOG_DEBUG("VideoCaptureModule Ioctl: " << control_id);

  int ret = YSOS_ERROR_SUCCESS;
  switch (control_id) {
    case CMD_WEB_VIDEO_CAPTURE: {
      BufferInterfacePtr buf =
          GetBufferUtility()->CreateBuffer(*(std::string*)(param));
      return driver_ptr_->Ioctl(control_id, buf, nullptr);
    }
  }

  return ret;
}

}