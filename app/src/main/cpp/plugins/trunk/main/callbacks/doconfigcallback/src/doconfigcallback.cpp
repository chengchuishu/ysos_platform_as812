/**
 *@file doconfigcallback.cpp
 *@brief doconfig callback
 *@version 1.0
 *@author l.j..
 *@date Created on: 2016-10-20 15:16:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
/// Private Headers //  NOLINT
#include "../include/doconfigcallback.h"

#include "../../../public/include/common/commonenumdefine.h"

namespace ysos {
DECLARE_PLUGIN_REGISTER_INTERFACE(DoconfigCallback, CallbackInterface);  ///<  插件的入口，一定要加上 //  NOLINT
DoconfigCallback::DoconfigCallback(const std::string &strClassName):BaseThreadModuleCallbackImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.doconfig");
}

DoconfigCallback::~DoconfigCallback(void) {

}

int DoconfigCallback::IsReady(void) {
  return YSOS_ERROR_SUCCESS;
}

int DoconfigCallback::RealCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr cur_output_buffer, BufferInterfacePtr output_buffer, void *context) {
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

    n_return = base_module_ptr->GetProperty(CMD_CONFIG_AUTO, &cur_output_buffer);
  } while (0);

  YSOS_LOG_DEBUG("callback RealCallback done");

  return n_return;
}

}