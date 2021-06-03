/**
 *@file facedetectmodulecallback.h
 *@brief Definition of face detect module
 *@version 2.0
 *@author libaoguo
 *@date Created on: 2017-08-25
 *@copyright Copyright(c) 2017 YunShen Technology. All rights reserved.
**/

#include "../include/facedetectmodulecallback.h"

// boost headers
#include <boost/thread/thread.hpp>

// ysos headers
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"

// private headers
#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"


namespace ysos {
DECLARE_PLUGIN_REGISTER_INTERFACE(FaceDetectModuleCallback, CallbackInterface);
FaceDetectModuleCallback::FaceDetectModuleCallback(const std::string &strClassName) : PluginBaseModuleCallbackImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.video");
}

}