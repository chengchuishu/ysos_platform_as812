/**   //NOLINT
  *@file videocapturemodule.h
  *@brief Definition of 
  *@version 0.1
  *@author wanfenfen
  *@date Created on: 2016:12:1   18:49
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  *@howto Usage:
  *@todo
  */
#ifndef MODULE_VIDEOCAPTURE_H_  //NOLINT
#define MODULE_VIDEOCAPTURE_H_  //NOLINT

/// ysos headers
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemoduleimpl.h"         //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemodulecallbackimpl.h" //NOLINT
#include "../../../../../../core/trunk/main/public/include/os_hal_package/driverinterface.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basethreadmoduleimpl.h"

/// private headers
#include "../../../public/include/common/pluginbasethreadmoduleimpl.h"

namespace ysos {
  /**
   *@brief  ASR Module callback // NOLINT
  */
class YSOS_EXPORT VideoCaptureModule: public /*BaseModuleImpl*//*BaseThreadModuleImpl*/PluginBaseThreadModuleImpl
{
  DECLARE_PROTECT_CONSTRUCTOR(VideoCaptureModule);
  DISALLOW_COPY_AND_ASSIGN(VideoCaptureModule);
  DECLARE_CREATEINSTANCE(VideoCaptureModule);

  int Initialized(const std::string &key, const std::string &value) override;
  int RealOpen(LPVOID param) override;
  int GetProperty(int type_id, void *type) override;
  int Ioctl(INT32 control_id, LPVOID param = nullptr) override;

private:
  // 视频旋转角度
  int rotate_angle_;
  
};

} // namespace ysos

#endif    //MODULE_TTS_H_  //NOLINT