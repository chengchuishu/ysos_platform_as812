/**
  *@file NlpRepeaterCallback.cpp
  *@brief Nlp Repeater Callback
  *@version 1.0
  *@author Donghongqian
  *@date Created on: 2016-05-31 18:03:27
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  */

/// Private Header //  NOLINT
#include "../include/nlprepeatercallback.h"
/// Ysos Headers //  NOLINT
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/jsonutility.h"

namespace ysos {

DECLARE_PLUGIN_REGISTER_INTERFACE(NlpRepeaterCallback, CallbackInterface);
NlpRepeaterCallback::NlpRepeaterCallback(const std::string &strClassName /* =NlpRepeaterCallback */): BaseRepeaterCallbackImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.audio");
}

NlpRepeaterCallback::~NlpRepeaterCallback() {

}

int NlpRepeaterCallback::Initialized(const std::string &key, const std::string &value) {
  if ("condition" == key) {
    condition_ = value;
  } else {
    BaseRepeaterCallbackImpl::Initialized(key, value);
  }

  return YSOS_ERROR_SUCCESS;
}

int NlpRepeaterCallback::FirstObserver(BufferInterfacePtr input_buffer, void *context) {
  return YSOS_ERROR_SUCCESS;
}

int NlpRepeaterCallback::LastObserver(BufferInterfacePtr input_buffer, void *context) {
  return Observer(input_buffer,context);
}

int NlpRepeaterCallback::Switch(SwitchContext *switch_context, CallbackInterfacePtr callback, void *context) {
  assert(NULL != switch_context && NULL != callback);
  YSOS_LOG_DEBUG("enter switch callback: " << logic_name_);

  int ret = YSOS_ERROR_SUCCESS;
  std::string callback_name = callback->GetName();
  if (condition_ == callback->GetName() && NULL != switch_context->output_buffer) {
    int len = GetBufferUtility()->GetBufferLength(switch_context->output_buffer);
    if (YSOS_ERROR_SUCCESS == switch_context->ret && len > 0) {
      switch_context->input_buffer = switch_context->output_buffer;
      switch_context->output_buffer = NULL;
    } else {
      LastObserver(switch_context->input_buffer, context);
    }
  }
  YSOS_LOG_DEBUG("leave switch callback: " << logic_name_);

  return BaseRepeaterCallbackImpl::Switch(switch_context, callback, context);

}

}