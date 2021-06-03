/**
 *@file resolutioncallback.cpp
 *@brief resolution callback
 *@version 1.0
 *@author l.j..
 *@date Created on: 2016-10-20 15:16:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
/// Private Headers //  NOLINT
#include "../include/resolutioncallback.h"

#include "../../../../../../core/trunk/main/public/include/plugins/commonenumdefine.h"

namespace ysos {
DECLARE_PLUGIN_REGISTER_INTERFACE(ResolutionCallback, CallbackInterface);  ///<  插件的入口，一定要加上 //  NOLINT
ResolutionCallback::ResolutionCallback(const std::string &strClassName):BaseThreadModuleCallbackImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.resolution");
  YSOS_LOG_DEBUG("ResolutionCallback");
}

ResolutionCallback::~ResolutionCallback(void) {
  YSOS_LOG_DEBUG("~ResolutionCallback");
}

int ResolutionCallback::IsReady(void) {
  return YSOS_ERROR_SUCCESS;
}

int ResolutionCallback::RealCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr cur_output_buffer, BufferInterfacePtr output_buffer, void *context) {
  int n_return = YSOS_ERROR_SUCCESS;

  YSOS_LOG_DEBUG("callback RealCallback execute");

  do {
    BaseModuelCallbackContext* module_callback_context_ptr = NULL;
    module_callback_context_ptr = (BaseModuelCallbackContext*)context;
    if (!module_callback_context_ptr) {
      YSOS_LOG_DEBUG("get module_callback_context_ptr failed");
      n_return = YSOS_ERROR_FAILED;
      break;
    }

    BaseModuleImpl* base_module_ptr = reinterpret_cast<BaseModuleImpl*>(module_callback_context_ptr->cur_module_ptr);
    if (!base_module_ptr) {
      YSOS_LOG_DEBUG("get port_module_ptr failed");
      n_return = YSOS_ERROR_FAILED;
      break;
    }

    n_return = base_module_ptr->GetProperty(CMD_SCREEN_CALLBACKAUTO, &cur_output_buffer);
  } while (0);

  YSOS_LOG_DEBUG("callback RealCallback done");

  return n_return;
}

}