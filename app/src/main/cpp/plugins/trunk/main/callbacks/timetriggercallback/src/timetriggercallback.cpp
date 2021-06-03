/**
 *@file timetriggercallback.cpp
 *@brief time trigger callback
 *@version 1.0
 *@author z.gg
 *@date Created on: 2020-9-1 9:10:00
 *@copyright Copyright (c) 2020 YS Technology. All rights reserved.
 */
/// Private Headers //  NOLINT
#include "../include/timetriggercallback.h"

#include "../../../public/include/common/commonenumdefine.h"

namespace ysos {
DECLARE_PLUGIN_REGISTER_INTERFACE(TimeTriggerCallBack, CallbackInterface);  ///<  插件的入口，一定要加上 //  NOLINT
TimeTriggerCallBack::TimeTriggerCallBack(const std::string &strClassName):BaseThreadModuleCallbackImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.timetrigger");
}

TimeTriggerCallBack::~TimeTriggerCallBack(void) {

}

int TimeTriggerCallBack::IsReady(void) {
  return YSOS_ERROR_SUCCESS;
}

int TimeTriggerCallBack::RealCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr cur_output_buffer, BufferInterfacePtr output_buffer, void *context) {
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
      YSOS_LOG_DEBUG("get signin_module_ptr failed");
      n_return = YSOS_ERROR_FAILED;
      break;
    }

    base_module_ptr->GetProperty(25701, &cur_output_buffer);

  } while (0);

  YSOS_LOG_DEBUG("callback RealCallback done");

  return n_return;
}

}