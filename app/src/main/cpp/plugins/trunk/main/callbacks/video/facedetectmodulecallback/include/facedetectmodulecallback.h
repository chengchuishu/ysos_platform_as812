/**
 *@file facedetectmodulecallback.h
 *@brief Definition of face detect module
 *@version 2.0
 *@author libaoguo
 *@date Created on: 2017-08-25
 *@copyright Copyright(c) 2017 YunShen Technology. All rights reserved.
**/

#ifndef MODULE_FACE_DETECT_MODULE_CALLBACK_H_  //NOLINT
#define MODULE_FACE_DETECT_MODULE_CALLBACK_H_  //NOLINT

// ysos headers
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemoduleimpl.h"         //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemodulecallbackimpl.h" //NOLINT

// private headers
#include  "../../../public/include/common/pluginbasemodulecallbackimpl.h"


namespace ysos {
class YSOS_EXPORT FaceDetectModuleCallback : public PluginBaseModuleCallbackImpl {
  DECLARE_CREATEINSTANCE(FaceDetectModuleCallback);
  DISALLOW_COPY_AND_ASSIGN(FaceDetectModuleCallback);
  DECLARE_PROTECT_CONSTRUCTOR(FaceDetectModuleCallback);

 private:
  int frame_;
};

} // namespace ysos

#endif    // MODULE_FACE_DETECT_MODULE_CALLBACK_H_  //NOLINT