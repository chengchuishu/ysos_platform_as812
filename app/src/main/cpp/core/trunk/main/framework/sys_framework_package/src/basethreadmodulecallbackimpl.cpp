/**
 *@file BaseThreadModuleCallbackImpl.cpp
 *@brief Definition of BaseThreadModuleCallbackImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// ysos private headers //  NOLINT
#include "../../../protect/include/sys_framework_package/basethreadmodulecallbackimpl.h"
#include "../../../public/include/core_help_package/utility.h"

namespace ysos {
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////                BaseThreadModuleCallbackImpl                //////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BaseThreadModuleCallbackImpl::BaseThreadModuleCallbackImpl(const std::string &strClassName): BaseModuleCallbackImpl(strClassName) {
  thread_module_callback_ptr_ = ThreadModuleCallbackImpl::CreateInstance();
}

std::pair<CallbackDataType, CallbackDataType> BaseThreadModuleCallbackImpl::GetUniqueKey(void) {
  std::pair<CallbackDataType,CallbackDataType> callback_key;
  callback_key.first = input_type_;
  callback_key.second = output_type_;
  return callback_key;
}

int BaseThreadModuleCallbackImpl::Callback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, void *context) {
  BaseModuelCallbackContext* module_callback_context_ptr = reinterpret_cast<BaseModuelCallbackContext*>(context);
  assert(NULL != module_callback_context_ptr);
  module_callback_context_ptr->cur_callback_ptr = this;

  if (NULL != thread_module_callback_ptr_) {
    YSOS_LOG_DEBUG( " Callback run: " << this->GetName() );
    return thread_module_callback_ptr_->Callback(input_buffer,output_buffer,context);
  }

  return BaseModuleCallbackImpl::Callback(input_buffer,output_buffer,context);
}

int BaseThreadModuleCallbackImpl::RealCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr next_input_buffer,
    BufferInterfacePtr pre_output_buffer, void *context) {

  return YSOS_ERROR_SUCCESS;
}
}
