/**
 *@file templatecallback.cpp
 *@brief the template call back
 *@version 1.0
 *@author z.gg
 *@date Created on: 2020-3-30 15:26:00
 *@copyright Copyright (c) 2020 YS Technology. All rights reserved.
 * 
 */
/// Private Headers //  NOLINT
#include "../include/templatecallback.h"

#include "../../../public/include/common/commonenumdefine.h"

namespace ysos {
DECLARE_PLUGIN_REGISTER_INTERFACE(TemplateCallback, CallbackInterface);  ///<  插件的入口，一定要加上 //  NOLINT
TemplateCallback::TemplateCallback(const std::string &strClassName)
    : BaseThreadModuleCallbackImpl(strClassName) {
	logger_ = GetUtility()->GetLogger("ysos.template");
}

TemplateCallback::~TemplateCallback(void) {

}

int TemplateCallback::IsReady(void) {
  return YSOS_ERROR_SUCCESS;
}

int TemplateCallback::RealCallback(
    BufferInterfacePtr input_buffer,
    BufferInterfacePtr cur_output_buffer,
    BufferInterfacePtr output_buffer, void *context) {
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
      YSOS_LOG_DEBUG("get template_module_ptr failed");
      n_return = YSOS_ERROR_FAILED;
      break;
    }

    ModuleInterface *cur_module = GetCurModule(context);
    n_return = cur_module->GetProperty(PROP_DATA, &cur_output_buffer);
    if(YSOS_ERROR_SUCCESS != n_return) {
      break;
    }
  } while (0);

  YSOS_LOG_DEBUG("callback RealCallback done: " << n_return);
  return n_return;
}

}