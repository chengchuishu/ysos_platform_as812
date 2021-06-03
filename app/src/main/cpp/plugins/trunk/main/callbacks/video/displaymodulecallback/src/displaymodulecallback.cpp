/**
 *@file DisplayModuleCallback.cpp
 *@brief speech output  
 *@version 1.0
 *@author dhongqian
 *@date Created on: 2016-05-26 14:39:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// self headers
#include "../include/displaymodulecallback.h"

/// boost headers
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"

/// private headers
#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"

#include "../../../../../../core/trunk/main/protect/include/os_hal_package/datainterfacemanager.h"

#include <json/json.h>

namespace ysos {
DECLARE_PLUGIN_REGISTER_INTERFACE(DisplayModuleCallback, CallbackInterface);
DisplayModuleCallback::DisplayModuleCallback(const std::string &strClassName):BaseModuleCallbackImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.video");
  input_type_ = std::string("stream_frame_rgb@24@p6");
  output_type_ = "null";
}

int DisplayModuleCallback::IsReady() {
  return YSOS_ERROR_SUCCESS;
}

int DisplayModuleCallback::RealCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr next_output_buffer, BufferInterfacePtr pre_output_buf, void *context) {
  ModuleInterface *cur_module = GetCurModule(context);
  assert(NULL != cur_module);

  int ret = cur_module->SetProperty(PROP_BUFFER, &input_buffer);
  if(YSOS_ERROR_SUCCESS != ret) {
    return ret;
  }

  bool is_query_admin = false;
  cur_module->GetProperty(CMD_PERSON_IS_ADMINISTRATER, &is_query_admin);
  if(is_query_admin) {
    std::string is_admin;
    DataInterfacePtr data_ptr_ = GetDataInterfaceManager()->GetData(GLOBAL_DATA_KEY);
    if (data_ptr_) {
      data_ptr_->GetData("is_admin", is_admin);
    }

    if (NULL != next_output_buffer) {
      uint8_t *next_output_data = GetBufferUtility()->GetBufferData(next_output_buffer);
      int len = GetBufferUtility()->GetBufferLength(next_output_buffer);

      Json::FastWriter jsWriter;
      Json::Value js_info;
      js_info["is_admin"] = is_admin.empty() ? "0" : is_admin;

      Json::Value js_snapshot;
      js_snapshot["type"] = "administrator";
      js_snapshot["data"] = js_info;
      std::string strResult = jsWriter.write(js_snapshot);
      strResult = GetUtility()->ReplaceAllDistinct ( strResult, "\\r\\n", "" );
      memcpy(next_output_data, strResult.c_str(), strResult.length());
      next_output_data[strResult.length()] = '\0';
      GetBufferUtility()->SetBufferLength(next_output_buffer, strResult.length()+1);
      YSOS_LOG_DEBUG("is administrator json: " << strResult);
    }

    is_query_admin = false;
    cur_module->SetProperty(CMD_PERSON_IS_ADMINISTRATER, &is_query_admin);
    return YSOS_ERROR_SUCCESS;
  }
  
  return YSOS_ERROR_SKIP;
}
}