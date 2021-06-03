/**
 *@file BaseRepeaterCallbackImpl.cpp
 *@brief Definition of BaseRepeaterCallbackImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// ysos private headers //  NOLINT
#include "../../../protect/include/sys_framework_package/baserepeatercallbackimpl.h"
#include "../../../public/include/core_help_package/utility.h"
// #include "../../../public/include/core_help_package/ysos_log.h"

namespace ysos {

BaseRepeaterCallbackImpl::BaseRepeaterCallbackImpl(const std::string &strClassName): BaseModuleCallbackImpl(strClassName) {
}

BaseRepeaterCallbackImpl::~BaseRepeaterCallbackImpl() {
}

int BaseRepeaterCallbackImpl::SetProperty(int type_id, void *type) {
  switch (type_id) {
  case PROP_PREV_MODULE:
    prev_module_ptr_ = *(ModuleInterfacePtr*)type;
    return YSOS_ERROR_SUCCESS;
  default:
    return BaseModuleCallbackImpl::SetProperty(type_id,type);
  }
}

int BaseRepeaterCallbackImpl::GetProperty(int type_id, void *type) {
  if (!type) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }
  switch (type_id) {
  case PROP_PREV_MODULE:
    *(ModuleInterfacePtr*)type = prev_module_ptr_;
    return YSOS_ERROR_SUCCESS;
  default:
    return BaseModuleCallbackImpl::GetProperty(type_id,type);
  }
}

BaseRepeaterModuleImpl *BaseRepeaterCallbackImpl::GetCurRepeater(void *context) {
  BaseModuleImpl *module_ptr = GetCurModule(context);
  assert(NULL != module_ptr);

  return dynamic_cast<BaseRepeaterModuleImpl*>(module_ptr);
}

int BaseRepeaterCallbackImpl::Observer(BufferInterfacePtr input_buffer, void *context) {
  BaseRepeaterModuleImpl* repeater_ptr = GetCurRepeater(context);
  assert(NULL != repeater_ptr);

  return repeater_ptr->Observe(input_buffer, context);
}

int BaseRepeaterCallbackImpl::FirstObserver(BufferInterfacePtr input_buffer, void *context) {
  return Observer(input_buffer, context);
}

int BaseRepeaterCallbackImpl::LastObserver(BufferInterfacePtr input_buffer, void *context) {
  return YSOS_ERROR_SUCCESS;
}

int BaseRepeaterCallbackImpl::Callback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, void *context) {
  YSOS_LOG_DEBUG("enter callback: " << logic_name_);
  BaseModuleImpl* module_ptr = GetCurModule(context);
  assert(NULL != module_ptr);

  // data_flow和ctrol_flow同步在这儿完成
  module_ptr->UpdateTeamParam();

  int ret = FirstObserver(input_buffer, context);
  if (YSOS_ERROR_SUCCESS != ret) {
    YSOS_LOG_DEBUG( "FirstObserver execute failed: " << ret );
  }
  if (YSOS_ERROR_SKIP == ret) {
    YSOS_LOG_DEBUG("now require  skip: " << logic_name_);
    YSOS_LOG_DEBUG("leave callback: " << logic_name_);
    return ret;
  }

  ret = BaseModuleCallbackImpl::Callback(input_buffer, output_buffer, context);
  if (YSOS_ERROR_SUCCESS != ret) {
    YSOS_LOG_DEBUG( "Callback execute failed: " << ret );
  }

  /// 当Callback执行成功或返回Pass的时候，不执行LastObserver //  NOLINT
  //if (YSOS_ERROR_SUCCESS != ret && YSOS_ERROR_PASS != ret) {
  //  ret = LastObserver(input_buffer, context);
  //}
  YSOS_LOG_DEBUG("leave callback: " << logic_name_);

  return ret;
}

int BaseRepeaterCallbackImpl::RealCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer,
  BufferInterfacePtr external_buf, void *context) {
  YSOS_LOG_DEBUG("BaseRepeaterCallbackImpl::RealCallback of " << this->GetName());
  return YSOS_ERROR_SUCCESS;
}

CallbackIODataTypePair BaseRepeaterCallbackImpl::GetUniqueKey(void) {
  CallbackIODataTypePair callback_key;
  if (prev_module_ptr_) {
    callback_key.first = prev_module_ptr_->GetName();
    callback_key.second = prev_module_ptr_->GetName();
  } else {
    this->GetType(&callback_key.first, &callback_key.second);
  }

  return callback_key;
}

int BaseRepeaterCallbackImpl::Switch(SwitchContext *switch_context, CallbackInterfacePtr callback, void *context) {
  assert(NULL != switch_context);
  switch_context->ret = BaseModuleCallbackImpl::InvokeNextCallback(switch_context->input_buffer, switch_context->output_buffer, callback, context);

  return switch_context->ret;
}

int BaseRepeaterCallbackImpl::InvokeNextCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, void *context) {
  int ret = YSOS_ERROR_SUCCESS;

  YSOS_LOG_DEBUG("enter to invoke next callback: " << logic_name_);
  BaseRepeaterModuleImpl* repeater_ptr = GetCurRepeater(context);
  assert(NULL != repeater_ptr);

  CallbackQueueInterfacePtr next_call_queue_ptr;
  next_call_queue_ptr.reset();

  //  获取与prev_module_ptr_对应的NextCallbackQueue//NOLINT
  ret = repeater_ptr->GetRepeatedNextCallbackQueue(prev_module_ptr_,next_call_queue_ptr);
  if (NULL == next_call_queue_ptr) {
    YSOS_LOG_DEBUG( "No nex callback to iterator :" << GetName() );
    return YSOS_ERROR_SUCCESS;
  }

  CallbackInterface *prev_callback_ptr = GetPrevCallback(context);
  assert(NULL != prev_callback_ptr);
  CallbackDataType prev_input_type, prev_output_type;
  ret = prev_callback_ptr->GetType(&prev_input_type, &prev_output_type);
  assert(YSOS_ERROR_SUCCESS == ret);

  // 对于该NextCallbackQueue中的每一个callback//NOLINT
  SwitchContext  switch_context(input_buffer, output_buffer);
  CallbackInterfacePtr repeat_next_callback_ptr = next_call_queue_ptr->FindCallback(true);
  while (NULL != repeat_next_callback_ptr) {
    YSOS_LOG_DEBUG("begin to invoke next callback: " << repeat_next_callback_ptr->GetName() << " | " << logic_name_);
    CallbackDataType next_input_type, next_output_type;
    ret = repeat_next_callback_ptr->GetType(&next_input_type, &next_output_type);
    assert(YSOS_ERROR_SUCCESS == ret);
    #ifdef _WIN32
      if(0 != stricmp(next_input_type.c_str(), prev_output_type.c_str())) {
        YSOS_LOG_DEBUG("next input type " << next_input_type << " unmatched prev output type " << prev_output_type << "|" << prev_callback_ptr->GetName() << "|" << repeat_next_callback_ptr->GetName());
        repeat_next_callback_ptr = next_call_queue_ptr->FindCallback();

        continue;
      }
    #else
      if(0 != strcasecmp(next_input_type.c_str(), prev_output_type.c_str())) {
        YSOS_LOG_DEBUG("next input type " << next_input_type << " unmatched prev output type " << prev_output_type << "|" << prev_callback_ptr->GetName() << "|" << repeat_next_callback_ptr->GetName());
        repeat_next_callback_ptr = next_call_queue_ptr->FindCallback();

        continue;
      }
    #endif
    ret = Switch(&switch_context, repeat_next_callback_ptr, context);
    if (YSOS_ERROR_SUCCESS != ret) {
      YSOS_LOG_DEBUG( "InvokeNextCallback failed: " << ret );
    }
    if(YSOS_ERROR_PASS == ret) {
      YSOS_LOG_DEBUG( "InvokeNextCallback is pass: " << ret );
      break;
    }

    repeat_next_callback_ptr = next_call_queue_ptr->FindCallback();
  }
  YSOS_LOG_DEBUG("leave to invoke next callback: " << logic_name_);

  return ret;
}
}
