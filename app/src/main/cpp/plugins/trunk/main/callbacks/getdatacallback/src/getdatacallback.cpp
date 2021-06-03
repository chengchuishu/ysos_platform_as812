/**
 *@file GetDataCallback.cpp
 *@brief get data  
 *@version 1.0
 *@author dhongqian
 *@date Created on: 2016-05-26 14:39:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// self headers
#include "../include/getdatacallback.h"
/// boost headers
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"
/// private headers
#include "../../../../../../core/trunk/main/public/include/plugins/commonenumdefine.h"
#include "../../../../../../core/trunk/main/public/include/plugins/commonstructdefine.h"


namespace ysos {
DECLARE_PLUGIN_REGISTER_INTERFACE(GetDataCallback, CallbackInterface);
GetDataCallback::GetDataCallback(const std::string &strClassName):BaseThreadModuleCallbackImpl(strClassName) {
  input_type_ = "null";
  output_type_ = "text";
}

int GetDataCallback::IsReady() {
  return YSOS_ERROR_SUCCESS;
}

int GetDataCallback::RealCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr next_output_buffer, BufferInterfacePtr pre_output_buf, void *context) {
  ModuleInterface *cur_module = GetCurModule(context);
  assert(NULL != cur_module);

  int ret = cur_module->GetProperty(PROP_DATA, &next_output_buffer);
  if(YSOS_ERROR_SUCCESS != ret) {
    return ret;
  }
  
  return YSOS_ERROR_SUCCESS;
}
}