/**
 *@file BaseThreadModuleImpl.cpp
 *@brief Definition of BaseThreadModuleImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// ysos private headers //  NOLINT
#include "../../../protect/include/sys_framework_package/basethreadmoduleimpl.h"
#include "../../../protect/include/sys_framework_package/basethreadmodulecallbackimpl.h"
#include "../../../public/include/core_help_package/utility.h"

namespace ysos {
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////                BaseThreadModuleImpl                        //////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
  @brief 实现了在BaseModule中，多线程调用CallbackQueue
*/
class BaseThreadModuleCallbackFunction: public BaseModuleCallbackFunction {
 public:
  BaseThreadModuleCallbackFunction(BaseThreadModuleImpl *thread_module_ptr=NULL): BaseModuleCallbackFunction() {
    thread_module_ptr_ = thread_module_ptr;
  }
  virtual ~BaseThreadModuleCallbackFunction() {}

  virtual int operator()(const CallbackInterfacePtr &callback_ptr, BaseModuleImpl *context_ptr=NULL) {
    assert(NULL != thread_module_ptr_);

    return thread_module_ptr_->CreateThreadFromCallback(callback_ptr, context_ptr);
  }

 private:
  BaseThreadModuleImpl            *thread_module_ptr_;
};

static const int s_thread_timeout = 1000;   ///<  默认值 //  NOLINT
static const int s_thread_wait_time = 5;   ///<  默认值 //  NOLINT

BaseThreadModuleImpl::BaseThreadModuleImpl(const std::string &strClassName): BaseModuleImpl(strClassName) {
  //InitalDataInfo();
  //thread_callback_ptr_ = BaseThreadModuleCallbackImpl::CreateInstance();
  thread_data_ = new ThreadData();
  assert(NULL != thread_data_);
  thread_data_->event_ptr = new Event();
  assert(NULL != thread_data_->event_ptr);
  thread_data_->is_stoped = false;
  thread_data_->is_exited = false;
  thread_data_->timeout = s_thread_timeout;  ///<  默认值 //  NOLINT
// thread_data_->event_ptr = NULL; ///< new BaseEvent();
}

BaseThreadModuleImpl::~BaseThreadModuleImpl() {
  thread_data_->is_stoped = true;
  if (NULL != thread_data_->event_ptr) {
    thread_data_->event_ptr->Notify();
  }
  thread_group_.join_all();
  //YSOS_DELETE(callback_context_ptr_);
  YSOS_DELETE(thread_data_);
}

int BaseThreadModuleImpl::InitalDataInfo() {
  return YSOS_ERROR_SUCCESS;
}

int BaseThreadModuleImpl::Initialized(const std::string &key, const std::string &value) {
  int ret = YSOS_ERROR_SUCCESS;

  if (key == "thread_interval") {
    thread_data_->timeout = GetUtility()->ConvertFromString(value, ret);
  } else {
    ret = BaseModuleImpl::Initialized(key, value);
  }

  return YSOS_ERROR_NOT_SUPPORTED;
}

int BaseThreadModuleImpl::CreateThreadFromCallback(CallbackInterfacePtr callback, void *context) {
  BufferInterfacePtr input_buffer,output_buffer;
  BaseThreadModuleCallbackImpl *thread_callback_ptr = dynamic_cast<BaseThreadModuleCallbackImpl*>(callback.get());
  if (NULL == thread_callback_ptr) {
    YSOS_LOG_DEBUG("Open thread module callback failed: " << callback->GetName() << " is not a threadmodulecallback");
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  BaseModuelCallbackContext *callback_context_ptr = new BaseModuelCallbackContext();
  callback_context_ptr->prev_callback_ptr = NULL;
  callback_context_ptr->prev_module_ptr = NULL;
  callback_context_ptr->cur_module_ptr = this;
  callback_context_ptr->cur_callback_ptr = thread_callback_ptr;

  thread_group_.create_thread(boost::bind(&BaseThreadModuleCallbackImpl::Callback, thread_callback_ptr, input_buffer,output_buffer, callback_context_ptr));

  return YSOS_ERROR_SUCCESS;
}

int BaseThreadModuleImpl::RealOpen(LPVOID param /* = NULL */) {
  BufferInterfacePtr input_buffer,output_buffer;

  YSOS_LOG_DEBUG("thread timeout is: " << thread_data_->timeout << " | " << logic_name_);

  BaseThreadModuleCallbackFunction thread_module_callback(this);
  int ret = this->IteratorPrevCallback(thread_module_callback);

  // YSOS_DELETE(callback_context_ptr_);·····················

  return ret;
}

int BaseThreadModuleImpl::RealClose() {
  thread_data_->is_stoped = 1;
  assert(NULL != thread_data_->event_ptr);
  thread_data_->event_ptr->Notify();
  //  wait for thread exit
//   for (int i = 0 ; i <s_thread_wait_time; i++) {
//     if (thread_data_->is_exited) {
//       thread_group_.join_all();
//       YSOS_DELETE(callback_context_ptr_);
//       return YSOS_ERROR_SUCCESS;
//     }
//     //boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
//     boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
//   }
  // thread_group_.join_all();
  // YSOS_DELETE(callback_context_ptr_);

  boost::this_thread::sleep(boost::posix_time::milliseconds(1000));

  return YSOS_ERROR_IS_BUSY;
}

int BaseThreadModuleImpl::SetProperty(int type_id, void *type) {
  int ret = YSOS_ERROR_SUCCESS;

  switch (type_id) {
  case PROP_THREAD_NOTIFY: {
    if (NULL == thread_data_ || NULL == thread_data_->event_ptr) {
      ret = YSOS_ERROR_NOT_SUPPORTED;
      break;
    }
    thread_data_->event_ptr->Notify();
    break;
  }
  default:
    ret = BaseModuleImpl::SetProperty(type_id,type);
  }

  return ret;
}

int BaseThreadModuleImpl::GetProperty(int type_id, void *type) {
  int ret = YSOS_ERROR_SUCCESS;

  switch (type_id) {
  case PROP_THREADDATA: {
    ThreadData **data_type = static_cast<ThreadData**>(type);
    assert(NULL != data_type);
    *data_type = thread_data_;
    break;
  }
  //case PROP_THREADCALLBACK:
  //  *(BaseThreadModuleCallbackImpl**)(type) = thread_callback_ptr_;
  //  break;
  default:
    ret = BaseModuleImpl::GetProperty(type_id,type);
  }

  return ret;
}

int BaseThreadModuleImpl::RealRun() {
  return YSOS_ERROR_SUCCESS;
}

int BaseThreadModuleImpl::RealPause() {
  return YSOS_ERROR_SUCCESS;
}

int BaseThreadModuleImpl::RealStop() {
  return YSOS_ERROR_SUCCESS;
}
}
