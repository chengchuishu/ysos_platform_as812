/**
 *@file BaseModuleThreadCallbackImpl.cpp
 *@brief Definition of BaseModuleThreadCallbackImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// ysos private headers //  NOLINT
#include "../../../protect/include/sys_framework_package/basemodulethreadcallbackimpl.h"  // NOLINT
#include "../../../public/include/core_help_package/multimap.h"
#include "../../../public/include/sys_interface_package/bufferpool.h"
#include "../../../protect/include/sys_framework_package/basemoduleimpl.h"
// #include "../../../public/include/core_help_package/ysos_log.h"
#include "../../../public/include/core_help_package/utility.h"
#include "../../../protect/include/sys_framework_package/packageconfigimpl.h"
#include "../../../public/include/os_hal_package/bufferutility.h"
/// 3rdparty headers
#include <boost/make_shared.hpp>

namespace ysos {
BaseModuleThreadCallbackImpl::BaseModuleThreadCallbackImpl(const std::string& name): BaseModuleCallbackImpl(name) {  // NOLINT
  logger_ = GetUtility()->GetLogger("ysos.plugin");
  input_type_ = output_type_ = CallbackDataTypeAll;
  is_run_ = false;
  is_interrupt_ = false;
}

BaseModuleThreadCallbackImpl::~BaseModuleThreadCallbackImpl() {
  thread_callback_param_list_.clear();
}

int BaseModuleThreadCallbackImpl::Initialize(void *param) {
  if (IsInitialized()) {
    return YSOS_ERROR_FINISH_ALREADY;
  }

  is_new_msg_ = false;
  is_run_ = true;
  thread_group_.create_thread(boost::bind(&BaseModuleThreadCallbackImpl::ThreadCallback, this));

  return BaseModuleCallbackImpl::Initialize(param);
}

int BaseModuleThreadCallbackImpl::Initialized(const std::string &key, const std::string &value) {
  int ret = YSOS_ERROR_SUCCESS;

  if ("is_interrupt" == key) {
    is_interrupt_ = value=="true" ? true : false;
  } else {
    ret = BaseModuleCallbackImpl::Initialized(key, value);
  }

  return YSOS_ERROR_SUCCESS;
}

int BaseModuleThreadCallbackImpl::UnInitialize(void *param) {
  if (!IsInitialized()) {
    return YSOS_ERROR_FINISH_ALREADY;
  }

  is_run_ = false;
  thread_group_.join_all();

  return BaseModuleCallbackImpl::UnInitialize(param);
}

BaseModuelCallbackContext *BaseModuleThreadCallbackImpl::CloneCallbackContext(BaseModuelCallbackContext *callback_context) {
  BaseModuelCallbackContext *clone_callback_contex = new BaseModuelCallbackContext();
  if (NULL == clone_callback_contex) {
    return NULL;
  }

  clone_callback_contex->prev_callback_ptr = callback_context->prev_callback_ptr;
  clone_callback_contex->prev_module_ptr = callback_context->prev_module_ptr;
  clone_callback_contex->cur_callback_ptr = callback_context->cur_callback_ptr;
  clone_callback_contex->cur_module_ptr = callback_context->cur_module_ptr;

  return clone_callback_contex;;
}

/**
 *  当该Callback是NextCallbackQueue中的成员是，context参数不应为NULL。
 *  它应是Callback对应的module(next module)指针。
 *  这个Callback相当于该module的运行函数:
 *  先执行preCallback,再执行nextCallback
 */
int BaseModuleThreadCallbackImpl::Callback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, void *context) {
  YSOS_LOG_DEBUG("enter callback: " << logic_name_);
  BaseModuelCallbackContext *callback_context = static_cast<BaseModuelCallbackContext*>(context);
  if (NULL == callback_context) {
    YSOS_LOG_ERROR("context is null: " << logic_name_);
    YSOS_LOG_DEBUG("leave callback: " << logic_name_);
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  BaseModuelCallbackContext *clone_callback_context = CloneCallbackContext(callback_context);
  if (NULL == clone_callback_context) {
    YSOS_LOG_ERROR("clone context is null: " << logic_name_);
    YSOS_LOG_DEBUG("leave callback: " << logic_name_);
    return YSOS_ERROR_OUTOFMEMORY;
  }

  clone_callback_context->cur_callback_ptr = this;
  /// 获取module_ptr
  BaseModuleImpl* module_ptr = dynamic_cast<BaseModuleImpl*>(callback_context->cur_module_ptr);
  if (NULL == module_ptr) {
    YSOS_LOG_ERROR("module is null: " << logic_name_);
    YSOS_LOG_DEBUG("leave callback: " << logic_name_);
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  /// invoke before IsCurModuleRun
//   int ret = module_ptr->SetProperty(PROP_LOCK_MODULE, module_ptr);
//   assert(0 == ret);

  ModuleInterface* prev_module_ptr = dynamic_cast<BaseModuleImpl*>(callback_context->prev_module_ptr);
  bool is_run = IsCurModuleRun(module_ptr, prev_module_ptr);
  if (!is_run) {
//     ret = module_ptr->SetProperty(PROP_UNLOCK_MODULE, module_ptr);
//     assert(0 == ret);
    YSOS_LOG_DEBUG("module is not run: " << logic_name_);
    YSOS_LOG_DEBUG("leave callback: " << logic_name_);
    return YSOS_ERROR_SUCCESS;
  }

  //// data_flow和ctrol_flow同步在这儿完成
  //module_ptr->UpdateTeamParam();

  ThreadCallbackParamPtr callback_param_ptr = boost::make_shared<ThreadCallbackParam>();
  if (NULL == callback_param_ptr) {
    YSOS_LOG_DEBUG("callback param is not run: " << logic_name_);
    YSOS_LOG_DEBUG("leave callback: " << logic_name_);
    return YSOS_ERROR_OUTOFMEMORY;
  }
  callback_param_ptr->buffer_ptr = input_buffer;
  callback_param_ptr->callback_context = clone_callback_context;

  {
    uint8_t *data = GetBufferUtility()->GetBufferData(input_buffer);
    if (NULL != data) {
      YSOS_LOG_DEBUG("new message: " << ((const char*)data) << " | " << logic_name_);
    } else {
      YSOS_LOG_ERROR("new message is null: " << logic_name_);
    }
  }

  {
    AutoLockOper lock(&thread_callback_param_list_lock_);
    if (is_interrupt_) {
      thread_callback_param_list_.clear();
    }
    thread_callback_param_list_.push_back(callback_param_ptr);
    is_new_msg_ = true;
  }
  YSOS_LOG_DEBUG("leave callback: " << logic_name_);

  return YSOS_ERROR_SUCCESS;
}

BaseModuleThreadCallbackImpl::ThreadCallbackParamPtr BaseModuleThreadCallbackImpl::GetCallbackParam(void) {
  ThreadCallbackParamPtr callback_param_ptr;

  AutoLockOper lock(&thread_callback_param_list_lock_);
  ThreadCallbackParamList::iterator it = thread_callback_param_list_.begin();
  if (thread_callback_param_list_.end() == it) {
    return NULL;
  }

  callback_param_ptr = *it;
  thread_callback_param_list_.erase(it);
  is_new_msg_ = false;

  return callback_param_ptr;
}

int BaseModuleThreadCallbackImpl::ThreadCallback(void) {
  YSOS_LOG_DEBUG("begin to run thread callback: " << is_run_ << " | " << logic_name_);

  while (is_run_) {
    ThreadCallbackParamPtr callback_param_ptr = GetCallbackParam();
    if (NULL == callback_param_ptr) {
      /// YSOS_LOG_ERROR("callback param is null: " << logic_name_);
      boost::this_thread::sleep_for(boost::chrono::microseconds(50));
      continue;
    }

    assert(NULL != callback_param_ptr);
    BaseModuelCallbackContext *callback_context = callback_param_ptr->callback_context;
    assert(NULL != callback_context);
    /// 获取module_ptr
    BaseModuleImpl* module_ptr = dynamic_cast<BaseModuleImpl*>(callback_context->cur_module_ptr);
    if (NULL == module_ptr) {
      return YSOS_ERROR_INVALID_ARGUMENTS;
    }

    BufferInterfacePtr buffer_ptr;
    int ret = AllocateBuffer(module_ptr, &buffer_ptr);
    if (YSOS_ERROR_SUCCESS != ret || NULL == buffer_ptr) {
      YSOS_LOG_ERROR("allocate buffer failed: " << logic_name_);
      boost::this_thread::sleep_for(boost::chrono::microseconds(50));
      continue;
    }

    /// lock
    int lock_ret = module_ptr->SetProperty(PROP_LOCK_MODULE, module_ptr);
    assert(0 == lock_ret);

    ModuleInterface* prev_module_ptr = dynamic_cast<BaseModuleImpl*>(callback_context->prev_module_ptr);
    bool is_run = IsCurModuleRun(module_ptr, prev_module_ptr);
    if (!is_run) {
      /// unlock
      lock_ret = module_ptr->SetProperty(PROP_UNLOCK_MODULE, module_ptr);
      assert(0 == lock_ret);
      boost::this_thread::sleep_for(boost::chrono::microseconds(100));
      continue;
    }

    //// data_flow和ctrol_flow同步在这儿完成
    module_ptr->UpdateTeamParam();

    BufferInterfacePtr input_buffer = callback_param_ptr->buffer_ptr;
    ret = RealCallback(input_buffer, buffer_ptr, NULL, callback_param_ptr->callback_context);
    /// unlock
    lock_ret = module_ptr->SetProperty(PROP_UNLOCK_MODULE, module_ptr);
    assert(0 == lock_ret);

    if (YSOS_ERROR_SUCCESS != ret) {
      YSOS_LOG_ERROR("realcallback execute ret is not 0: " << ret << " | " << logic_name_);
      ReleaseBuffer(module_ptr, buffer_ptr);
      boost::this_thread::sleep_for(boost::chrono::microseconds(50));
      continue;
    }

    ret = InvokeNextCallback((NULL == buffer_ptr ? input_buffer : buffer_ptr), NULL, callback_param_ptr->callback_context);
    ReleaseBuffer(module_ptr, buffer_ptr);
    if (YSOS_ERROR_PASS == ret) {
      YSOS_LOG_DEBUG("Leave BaseModuleCallbackImpl::Callback return is [YSOS_ERROR_PASS]");
    } else {
      // YSOS_LOG_DEBUG("Leave BaseModuleCallbackImpl::Callback return is " << ret);
    }
  }

  YSOS_LOG_DEBUG("end to run thread callback: " << is_run_ << " | " << logic_name_);

  return YSOS_ERROR_SUCCESS;
}

}  // NOLINT
