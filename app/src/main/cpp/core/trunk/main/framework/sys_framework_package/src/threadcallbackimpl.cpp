/**
 *@file ThreadCallbackImpl.cpp
 *@brief Definition of ThreadCallbackImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// ysos private headers //  NOLINT
#include "../../../protect/include/sys_framework_package/threadcallbackimpl.h"
#include "../../../protect/include/sys_framework_package/basemodulecallbackimpl.h"
#include "../../../protect/include/sys_framework_package/basethreadmoduleimpl.h"
#include "../../../protect/include/sys_framework_package/basethreadmodulecallbackimpl.h"
#include "../../../public/include/core_help_package/utility.h"
#include "../../../protect/include/os_hal_package/datainterfacemanager.h"

namespace ysos {
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////                      ThreadCallbackImpl                    //////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ThreadCallbackImpl::ThreadCallbackImpl(const std::string &strClassName): BaseInterfaceImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.plugin");
}

ThreadCallbackImpl::~ThreadCallbackImpl() {
};

int ThreadCallbackImpl::Callback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, void *context) {
  BaseModuelCallbackContext *callback_context_ptr = new BaseModuelCallbackContext;
  assert(NULL != callback_context_ptr);
  {
    BaseModuelCallbackContext* module_callback_context_ptr = reinterpret_cast<BaseModuelCallbackContext*>(context);
    assert(NULL != module_callback_context_ptr);
    callback_context_ptr->prev_callback_ptr = module_callback_context_ptr->prev_callback_ptr;
    callback_context_ptr->prev_module_ptr = module_callback_context_ptr->prev_module_ptr;
    callback_context_ptr->cur_module_ptr = module_callback_context_ptr->cur_module_ptr;
    callback_context_ptr->cur_callback_ptr = module_callback_context_ptr->cur_callback_ptr;
  }

  BaseThreadModuleImpl* thread_module_ptr = dynamic_cast<BaseThreadModuleImpl*>(callback_context_ptr->cur_module_ptr);
  assert(NULL != thread_module_ptr);
  BaseThreadModuleImpl::ThreadData *tdata = NULL;
  int res = thread_module_ptr->GetProperty(PROP_THREADDATA,&tdata);
  if (res != YSOS_ERROR_SUCCESS) {
    YSOS_LOG_FATAL("create thread callback failed: " << res << " module name: " << thread_module_ptr->GetName());
    return res;
  }

  /// used for log
  std::string callback_name = callback_context_ptr->cur_callback_ptr->GetName();
  std::string module_name = callback_context_ptr->cur_module_ptr->GetName();
  int timeout = tdata->timeout == 0 ? 1000 : tdata->timeout;
  YSOS_LOG_DEBUG("ThreadCallbackImpl begin to run: " << module_name << " | " << callback_name);
  YSOS_LOG_DEBUG("time out is: " << timeout);
  int elapse = 0;
  while (true) {
    if (NULL != tdata->event_ptr) {
      res = tdata->event_ptr->Wait(timeout);
      if ((res == YSOS_ERROR_SUCCESS) && (1 == tdata->is_stoped)) {
        YSOS_LOG_DEBUG("thread module is stopping: " << callback_name);
        break;
      }
    } else {
      // boost::this_thread::sleep(boost::posix_time::milliseconds(5));
      boost::this_thread::sleep_for(boost::chrono::milliseconds(5));
      elapse += 5;
      if (tdata->is_stoped) {
        YSOS_LOG_DEBUG("thread module is stopping: " << callback_name);
        break;
      }

      if (elapse < timeout) {
        continue;
      }
      elapse = 0;
    }

    ThreadCallback(input_buffer,output_buffer,callback_context_ptr);
  }
  tdata->is_exited = true;
  YSOS_DELETE(callback_context_ptr);
  YSOS_LOG_DEBUG("ThreadCallbackImpl exit: " << logic_name_);

  return YSOS_ERROR_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////                ThreadModuleCallbackImpl                    //////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ThreadModuleCallbackImpl::ThreadModuleCallbackImpl(const std::string &strClassName):ThreadCallbackImpl(strClassName) {
}

int ThreadModuleCallbackImpl::SetType(const CallbackDataType input_type, const CallbackDataType output_type) {
  return YSOS_ERROR_SUCCESS;
}

int ThreadModuleCallbackImpl::GetType(CallbackDataType *input_type, CallbackDataType *output_type) {
  return YSOS_ERROR_SUCCESS;
}

int ThreadModuleCallbackImpl::IsReady() {
  return YSOS_ERROR_SUCCESS;
}

/*
  @brief 实现了在BaseModule中，多线程调用CallbackQueue
*/
class ThreadModuleCallbackFunction: public BaseModuleCallbackFunction {
 public:
  ThreadModuleCallbackFunction(BufferInterfacePtr input_buffer=NULL, BufferInterfacePtr output_buffer=NULL, void *context_ptr=NULL):
    BaseModuleCallbackFunction(input_buffer, output_buffer, context_ptr) {
  }
  virtual ~ThreadModuleCallbackFunction() {}

  virtual int operator()(const CallbackInterfacePtr &callback_ptr, BaseModuleImpl *context_ptr=NULL) {
    BaseThreadModuleCallbackImpl *thread_callback_ptr = dynamic_cast<BaseThreadModuleCallbackImpl*>(callback_ptr.get());
    if (NULL == thread_callback_ptr) {
      YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.plugin"), "Run thread callback failed: " << callback_ptr->GetName() << " is not a threadmodulecallback");
      return YSOS_ERROR_INVALID_ARGUMENTS;
    }

    return thread_callback_ptr->BaseModuleCallbackImpl::Callback(input_buffer_, output_buffer_, context_ptr_);
  }
};

int ThreadModuleCallbackImpl::ThreadCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer,LPVOID context) {
  BaseModuelCallbackContext* module_callback_context_ptr = reinterpret_cast<BaseModuelCallbackContext*>(context);
  assert(NULL != module_callback_context_ptr);

  BaseThreadModuleImpl* thread_module_ptr = dynamic_cast<BaseThreadModuleImpl*>(module_callback_context_ptr->cur_module_ptr);
  assert(thread_module_ptr);

  CallbackInterface *cur_callback_ptr = module_callback_context_ptr->cur_callback_ptr;
  assert(NULL != cur_callback_ptr);
  BaseThreadModuleCallbackImpl *thread_callback_ptr = dynamic_cast<BaseThreadModuleCallbackImpl*>(cur_callback_ptr);

  return thread_callback_ptr->BaseModuleCallbackImpl::Callback(input_buffer, output_buffer, context);
}
}
