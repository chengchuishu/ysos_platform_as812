/**   //NOLINT
  *@file videocapturemodulecallback.cpp
  *@brief Definition of 
  *@version 0.1
  *@author wanfenfen
  *@date Created on: 2016:12:6   13:47
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  *@howto Usage:
  *@todo
  */

/// self header
#include "../include/videocapturemodulecallback.h"

/// windows headers
//#include <assert.h>

/// Boost Headers //  NOLINT
#include <boost/thread/thread.hpp>

/// ysos headers
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"

/// private headers
#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"


namespace ysos {
DECLARE_PLUGIN_REGISTER_INTERFACE(VideoCaptureModuleCallback, CallbackInterface);
VideoCaptureModuleCallback::VideoCaptureModuleCallback(const std::string &strClassName /* =ASRCallback */):/*BaseModuleCallbackImpl*//*BaseThreadModuleCallbackImpl*/PluginBaseThreadModuleCallbackImpl(strClassName) {
//   input_type_ = /*"NULL"*/DTSTR_NULL;
//   output_type_ = /*"PCM"*/DTSTR_STREAM_FRAME_RGB24_P6;
  logger_ = GetUtility()->GetLogger("ysos.video");
}
}