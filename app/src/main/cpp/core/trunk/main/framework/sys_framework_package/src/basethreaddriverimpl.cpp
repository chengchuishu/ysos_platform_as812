/*@version 1.0
  *@author dhongqian
  *@date Created on: 2016-06-23 10:06:20
  *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
  * 
*/
/// Private Headers //  NOLINT
#include "../../../protect/include/sys_framework_package/basethreaddriverimpl.h"
#include "../../../public/include/core_help_package/utility.h"
#include "../../../protect/include/sys_framework_package/packageconfigimpl.h"
#include "../../../public/include/sys_interface_package/ysos_enum.h"

namespace ysos {

BaseThreadDriverImpl::BaseThreadDriverImpl(const std::string &strClassName /* =BaseDriverImpl */): BaseDriverImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.plugin");
  is_run_ = false;
  // is_interrupt_ = false;
}

BaseThreadDriverImpl::~BaseThreadDriverImpl(void) {
  thread_driver_param_list_.clear();
}

int BaseThreadDriverImpl::Initialize(void *param) {
  int ret = BaseDriverImpl::Initialize(param);
  if (YSOS_ERROR_SUCCESS != ret) {
    return ret;
  }

  is_new_msg_ = false;
  is_run_ = true;
  thread_group_.create_thread(boost::bind(&BaseThreadDriverImpl::ThreadCallback, this));

  return ret;
}

BaseThreadDriverImpl::DriverParamPtr BaseThreadDriverImpl::GetDriverParam(void) {
  AutoLockOper lock(&thread_driver_param_list_lock_);
  ThreadDriverParamList::iterator it = thread_driver_param_list_.begin();
  if (thread_driver_param_list_.end() == it) {
    return NULL;
  }

  DriverParamPtr driver_param_ptr = *it;
  thread_driver_param_list_.erase(it);
  is_new_msg_ = false;

  return driver_param_ptr;
}

int BaseThreadDriverImpl::SetDriverParam(BufferInterfacePtr buffer_ptr) {
  if (NULL == buffer_ptr) {
    YSOS_LOG_ERROR("buffer ptr is null: " << logic_name_);
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  {
    AutoLockOper lock(&thread_driver_param_list_lock_);
    thread_driver_param_list_.push_back(buffer_ptr);
    is_new_msg_ = true;
  }

  return YSOS_ERROR_SUCCESS;
}

int BaseThreadDriverImpl::ThreadCallback(void) {
  YSOS_LOG_DEBUG("begin to run thread callback: " << is_run_ << " | " << logic_name_);

  while (is_run_) {
    DriverParamPtr driver_param_ptr = GetDriverParam();
    if (NULL == driver_param_ptr) {
      /// YSOS_LOG_ERROR("callback param is null: " << logic_name_);
      boost::this_thread::sleep_for(boost::chrono::microseconds(50));
      continue;
    }

    assert(NULL != driver_param_ptr);
    int ret = RealCallback(driver_param_ptr, NULL);
    YSOS_LOG_DEBUG("execute real callback result: " << ret << " | " << logic_name_);
  }

  YSOS_LOG_DEBUG("end to run thread callback: " << is_run_ << " | " << logic_name_);

  return YSOS_ERROR_SUCCESS;
}

int BaseThreadDriverImpl::RealCallback(BufferInterfacePtr input_buffer, void *context) {
  return YSOS_ERROR_SUCCESS;
}

int BaseThreadDriverImpl::Open(void *pParams) {
  return BaseDriverImpl::Open(pParams);
}

void BaseThreadDriverImpl::Close(void *pParams) {
  return BaseDriverImpl::Close(pParams);
}

int BaseThreadDriverImpl::UnInitialize(void *param) {
  int ret = BaseDriverImpl::UnInitialize(param);
  if (YSOS_ERROR_SUCCESS != ret) {
    YSOS_LOG_ERROR("uninitialize failed: " << ret << " | " << logic_name_);
    return ret;
  }

  is_run_ = false;
  thread_group_.join_all();

  return YSOS_ERROR_SUCCESS;
}

}
