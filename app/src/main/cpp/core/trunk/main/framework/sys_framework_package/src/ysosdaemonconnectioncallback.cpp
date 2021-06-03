/**
*@file YsosDaemonConnnectionCalllback.cpp
*@brief Definition of YsosDaemonConnnectionCalllback
*@version 0.1
*@author dhongqian
*@date Created on: 2016-06-08 15:05:58
*@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
* 
*/

#include "../../../protect/include/sys_framework_package/ysosdaemonconnectioncallback.h"
#include "../../../public/include/core_help_package/utility.h"
#include "../../../public/include/sys_interface_package/messagetype.h"
#include "../../../protect/include/sys_framework_package/platformrpcentity.h"
#include "../../../public/include/sys_interface_package/ysos_enum.h"
#include "../../../protect/include/sys_framework_package/platforminfo.h"
#include "../../../public/include/os_hal_package/bufferutility.h"
#include "../../../protect/include/os_hal_package/msgimpl.h"
#include "../../../protect/include/sys_framework_package/basesessionimpl.h"

namespace ysos {

YsosDaemonConnectionCalllback::YsosDaemonConnectionCalllback(const std::string& strClassName)
  : BaseRPCCallbackImpl(strClassName),
    session_ptr_(NULL) {
  logger_ = GetUtility()->GetLogger("ysos.framework.strategy");
}

YsosDaemonConnectionCalllback::~YsosDaemonConnectionCalllback() {
    session_ptr_ = NULL;
}

int YsosDaemonConnectionCalllback::SetProperty(int type_id, void *type) {
  int ret = YSOS_ERROR_SUCCESS;

  switch (type_id) {
  case PROP_SESSIONPTR: {
    assert(NULL != type);
    session_ptr_ = reinterpret_cast<SessionInterface*>(type);
    break;
  }
  default:
    ret = BaseRPCCallbackImpl::SetProperty(type_id, type);
  }

  return ret;
}

void YsosDaemonConnectionCalllback::HandleDoService(const std::string& session_id, const std::string& service_name, const std::string& service_param) {
  if (NULL == session_ptr_) {
    YSOS_LOG_DEBUG_DEFAULT("session_ptr_ is NULL");
    return;
  }

  BaseSessionImpl::SessionAgentInfo session_agent_info(service_name, service_param);
  BaseSessionImpl* base_session_impl_ptr = dynamic_cast<BaseSessionImpl*>(session_ptr_);
  assert(NULL != base_session_impl_ptr);
  base_session_impl_ptr->SetProperty(BaseSessionImpl::DOSERVICE, &session_agent_info);

  return;
}

void YsosDaemonConnectionCalllback::HandleOnDispatchMessage(const std::string& session_id, const std::string& service_name, const std::string& result) {
  if (NULL == session_ptr_) {
    YSOS_LOG_DEBUG_DEFAULT("app_manager_ptr_ is NULL");
    return;
  }

  BaseSessionImpl::SessionAgentInfo session_agent_info(service_name, result);
  BaseSessionImpl* base_session_impl_ptr = dynamic_cast<BaseSessionImpl*>(session_ptr_);
  assert(NULL != base_session_impl_ptr);
  base_session_impl_ptr->SetProperty(BaseSessionImpl::DISPATCHSERVICE, &session_agent_info);

  return;
}

void YsosDaemonConnectionCalllback::HandleError(const std::string& session_id, const std::string& error_param) {
  if (NULL != error_handler_) {
    error_handler_(session_id, error_param);
  }

  return;
}

}  /// end of namespace
