/**
*@file basesessionimpl.cpp
*@brief Definition of basesessionimpl
*@version 0.1
*@author dhongqian
*@date Created on: 2016-06-08 15:05:58
*@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
* 
*/

#include "../../../protect/include/sys_framework_package/basesessionimpl.h"
#include "../../../public/include/sys_interface_package/appmanagerinterface.h"
#include "../../../public/include/sys_interface_package/agentinterface.h"
#include "../../../protect/include/sys_framework_package/baseagentimpl.h"
// #include "../../../public/include/core_help_package/ysos_log.h"

namespace ysos {
DEFINE_SINGLETON(BaseSessionImpl);
BaseSessionImpl::BaseSessionImpl(const std::string &strClassName /* =BaseSessionImpl */) : BaseInterfaceImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.framework.agent");
  agent_ptr_ = NULL;
  platform_rpc_ptr_ = NULL;
  is_connecting_ = (platform_rpc_ptr_ != NULL);
  manager_ptr_ = NULL;
  session_id_ = "0";
  is_connecting_ = false;
}

BaseSessionImpl::BaseSessionImpl(const std::string &session_id, const PlatformRPCPtr &platform_sdk_ptr, const std::string &str): BaseInterfaceImpl(str) {
  logger_ = GetUtility()->GetLogger("ysos.framework.agent");
  agent_ptr_ = NULL;
  platform_rpc_ptr_ = platform_sdk_ptr;
  is_connecting_ = (platform_sdk_ptr != NULL);
  manager_ptr_ = NULL;
  session_id_ = session_id;
}

BaseSessionImpl::~BaseSessionImpl() {
  agent_ptr_ = NULL;
}

SessionInterface *BaseSessionImpl::Clone(const std::string& session_id, const PlatformRPCPtr& platform_sdk_ptr, void* param) {
  BaseSessionImpl *session_ptr = new BaseSessionImpl(session_id, platform_sdk_ptr);
  if (NULL == session_ptr) {
    YSOS_LOG_DEBUG("allocate session failed");
    return NULL;
  }

  int ret = session_ptr->SetProperty(MANAGER, manager_ptr_);
  assert(YSOS_ERROR_SUCCESS == ret);
  ret = session_ptr->Initialize(param);
  if (YSOS_ERROR_SUCCESS != ret) {
    YSOS_LOG_DEBUG("Initialize failed");
    YSOS_DELETE(session_ptr);
  }

  return session_ptr;
}

/**
 *@brief 返回该Session的SessionId  // NOLINT
 *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
 */
std::string BaseSessionImpl::GetSessionId(void) {
  return session_id_;
}

/**
 *@brief 销毁一个指定的Session  // NOLINT
 *@param connection_ptr[Input]： 一个请求连接  // NOLINT
 *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
 */
int BaseSessionImpl::Destroy(SessionInterface *session_ptr) {
  YSOS_DELETE(session_ptr);
  return YSOS_ERROR_SUCCESS;
}

/**
 *@brief 接受某个应用的会话，并处理会话的初始化请求  // NOLINT
 *@param connection_ptr[Input]： 一个请求连接  // NOLINT
 *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
 */
int BaseSessionImpl::Accept(const std::string& session_id, const PlatformRPCPtr& platform_sdk_ptr, void* param) {
  SessionInterface *session_ptr = Clone(session_id, platform_sdk_ptr, param);
  if (NULL == session_ptr) {
    return YSOS_ERROR_FAILED;
  }

  assert(NULL != manager_ptr_);
  return manager_ptr_->AddSession(session_ptr);
}

int BaseSessionImpl::Initialize(void *param) {
  disconnect_count_ = 3;
  is_alive_ = true;
  alive_check_second_ = 30;
  max_disconnect_count_ = 3;
  max_reconnect_count_ = 3;
  wait_for_reconnect_count_ = 0;

  check_alive_thread_.create_thread(boost::bind(&BaseSessionImpl::CheckAlive, this));
  agent_ptr_ = GetBaseAgentImpl()->Clone(platform_rpc_ptr_, this, param);
  if (NULL == agent_ptr_) {
    YSOS_LOG_DEBUG("alocate agent failed");
    return YSOS_ERROR_OUTOFMEMORY;
  }

  return BaseInterfaceImpl::Initialize(param);
}

int BaseSessionImpl::UnInitialize(void *param) {
  YSOS_LOG_DEBUG("uninitialize");
  is_alive_ = false;
  check_alive_thread_.join_all();
  platform_rpc_ptr_ = NULL;

  GetBaseAgentImpl()->Destroy(agent_ptr_);

  return BaseInterfaceImpl::UnInitialize(param);
}

int BaseSessionImpl::GetProperty(int type_id, void *type) {
  int ret = YSOS_ERROR_NOT_SUPPORTED;

  switch (type_id) {
  case PROP_STATE: {
    if (NULL == agent_ptr_) {
      ret = YSOS_ERROR_FAILED;
    } else {
      ret = agent_ptr_->GetProperty(type_id, type);
    }
  }
  break;
  default:
    break;
  }

  return ret;
}

int BaseSessionImpl::SetProperty(int type_id, void *type) {
  int ret = YSOS_ERROR_NOT_SUPPORTED;

  switch (type_id) {
  case MANAGER: {
    manager_ptr_ = static_cast<AppManagerInterface*>(type);
    if (NULL == manager_ptr_) {
      ret = YSOS_ERROR_INVALID_ARGUMENTS;
    } else {
      ret = YSOS_ERROR_SUCCESS;
    }
    break;
  }
  case DOSERVICE: {
    SessionAgentInfo *session_agent_info = static_cast<SessionAgentInfo*>(type);
    if (NULL == session_agent_info) {
      ret = YSOS_ERROR_INVALID_ARGUMENTS;
    } else {
      agent_ptr_->HandleDoService(session_agent_info->service_name, session_agent_info->service_param);
      ret = YSOS_ERROR_SUCCESS;
    }
    break;
  }
  case DISPATCHSERVICE: {
    SessionAgentInfo *session_agent_info = static_cast<SessionAgentInfo*>(type);
    if (NULL == session_agent_info) {
      ret = YSOS_ERROR_INVALID_ARGUMENTS;
    } else {
      agent_ptr_->HandleOnDispatchMessage(session_agent_info->service_name, session_agent_info->service_param);
      ret = YSOS_ERROR_SUCCESS;
    }
    break;
  }
  case ERROR: {
    SessionAgentInfo *session_agent_info = static_cast<SessionAgentInfo*>(type);
    if (NULL == session_agent_info) {
      ret = YSOS_ERROR_INVALID_ARGUMENTS;
    } else {
      agent_ptr_->HandleError(session_agent_info->service_param);
      ret = YSOS_ERROR_SUCCESS;
    }
    break;
  }
  case PROP_STATE: {
    if (NULL == agent_ptr_) {
      ret = YSOS_ERROR_FAILED;
    } else {
      ret = agent_ptr_->SetProperty(type_id, type);
      ret = YSOS_ERROR_SUCCESS;
    }
    break;
  }
  default:
    break;
  }

  return ret;
}

/**
 *@brief 检查保活信息  // NOLINT
 *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
 */
int BaseSessionImpl::CheckAlive(void) {
  return 0;
  while (is_alive_) {
    if (!is_connecting_) {
      CheckLifeCycle();
      continue;
    } else if (++disconnect_count_ == max_disconnect_count_) {
      is_connecting_ = false;
    }

    //boost::this_thread::sleep(boost::posix_time::seconds(alive_check_second_));
    boost::this_thread::sleep_for(boost::chrono::seconds(alive_check_second_));
  }

  if (!is_alive_) {
    YSOS_LOG_DEBUG("checkalive finish, begin to remove session: " << session_id_);
    YSOS_DELETE(agent_ptr_);
    manager_ptr_->RemoveSession(session_id_);
  }

  return YSOS_ERROR_SUCCESS;
}

/**
 *@brief 处理心跳返回信息  // NOLINT
 *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
 */
int BaseSessionImpl::HandleAliveResponse(const std::string &response) {
  if (is_connecting_) {
    disconnect_count_ = 0;
  } else {
    YSOS_LOG_DEBUG("checkalive timeout: " << disconnect_count_);
    return YSOS_ERROR_TIMEOUT;
  }

  return YSOS_ERROR_SUCCESS;
}

/**
 *@brief 检查Agent的生命周期，如果生命周期结束，销毁Agent  // NOLINT
 *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
 */
int BaseSessionImpl::CheckLifeCycle(void) {
  if (++wait_for_reconnect_count_ == max_reconnect_count_) {
    is_alive_ = false;
  }

  return YSOS_ERROR_SUCCESS;
}

/**
 *@brief 权限检查，App发送请求时，要先通过Checkjurisdiction，才能向Strategy发送请求  // NOLINT
 *@param service[Input]： 请求的服务  // NOLINT
 *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
 */
int BaseSessionImpl::CheckJurisdiction(const std::string &service) {
  return YSOS_ERROR_SUCCESS;
}

}// end of namespace

