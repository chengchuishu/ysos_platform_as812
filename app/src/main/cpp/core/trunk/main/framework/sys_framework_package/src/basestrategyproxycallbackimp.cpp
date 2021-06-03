/**
 *@file BaseStrategyCallbackImpl.cpp
 *@brief ״̬��ͨ��Callback����,����StateMachine���¼���Service�ص�
 *@version 1.0
 *@author dhongqian
 *@date Created on: 2016-05-11 10:06:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// Self Header //  NOLINT
#include "../../../protect/include/sys_framework_package/basestrategyproxycallbackimpl.h"

/// private headers
#include "../../../public/include/sys_interface_package/ysos_enum.h"
#include "../../../public/include/core_help_package/utility.h"
#include "../../../public/include/sys_interface_package/messageinterface.h"
#include "../../../public/include/sys_interface_package/messagehead.h"
#include "../../../public/include/sys_interface_package/messagetype.h"
#include "../../../public/include/os_hal_package/bufferutility.h"
#include "../../../protect/include/sys_framework_package/platforminfo.h"
#include "../../../protect/include/sys_framework_package/baseagentimpl.h"
#include "../../../protect/include/os_hal_package/msgimpl.h"
#include "../../../protect/include/sys_framework_package/manager_declare.h"

namespace ysos {

//DECLARE_PLUGIN_REGISTER_INTERFACE(StrategyCallback, CallbackInterface);
BaseStrategyProxyCallbackImpl::BaseStrategyProxyCallbackImpl(const std::string &strClassName):BaseStrategyServiceCallbackImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.framework.strategy.callback");
  agent_ptr_ = NULL;
  is_init_strategy_ = false;
}

BaseStrategyProxyCallbackImpl::~BaseStrategyProxyCallbackImpl() {

}

int BaseStrategyProxyCallbackImpl::Initialized(const std::string &key, const std::string &value) {
  int ret = YSOS_ERROR_SUCCESS;

  if ("mode" == key) {
    if ("client" == value) {
      service_type_ = CLIENT_SERVICE;
    } else if ("internal" == value) {
      service_type_ = INTERNAL_SERVICE;
    } else if ("local" == value) {
      service_type_ = LOCAL_SERVICE;
    } else {
      service_type_ = STRATEGY_SERVICE_END;
    }
  } else if ("agent" == key) {
    // agent_ptr_ =
  } else if ("strategy" == key) {
    strategy_name_ = value;
    /// strategy_ptr_ = GetStrategyInterfaceManager()->FindInterface(value);
  } else if ("ioctl" == key) {
    ret = ParseIoctlParam(value);
    /// not support yet
  } else {
    ret = BaseStrategyServiceCallbackImpl::Initialized(key, value);
  }

  return YSOS_ERROR_SUCCESS;
}

int BaseStrategyProxyCallbackImpl::ParseIoctlParam(const std::string &ioctl_param) {
  std::list<std::string> split_list;
  const int ioctl_param_num = 3;

  int ret = GetUtility()->SplitString(ioctl_param, "|", split_list);
  assert(YSOS_ERROR_SUCCESS == ret);

  if (ioctl_param_num != split_list.size()) {
    YSOS_LOG_ERROR("invalid ioctl param: " << ioctl_param << " | " << logic_name_);
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  ReqIOCtlServiceParam req_param;
  std::list<std::string>::iterator it = split_list.begin();
  std::string event_name = *it++;
  req_param.service_name = *it++;
  req_param.id = *it++;
  req_param.value = *it++;
  req_param.type = "text";

  ioctl_map_[event_name] = req_param;
  YSOS_LOG_DEBUG("add event succeed: " << event_name << " | " << ioctl_param << " | " << logic_name_);

  return ret;
}

int BaseStrategyProxyCallbackImpl::SetProperty(int type_id, void *type) {
  int ret = YSOS_ERROR_SUCCESS;

  switch (type_id) {
  case PROP_AGENT: {
    assert(NULL != type);
    agent_ptr_ = reinterpret_cast<AgentInterface*>(type);
    break;
  }
  default:
    ret = BaseStrategyServiceCallbackImpl::SetProperty(type_id, type);
  }

  return ret;
}

int BaseStrategyProxyCallbackImpl::Callback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, void *context) {
  YSOS_LOG_DEBUG("BaseStrategyCallbackImpl : Dealing with input buffer: " << logic_name_);
  MsgInterface  *msg_ptr = dynamic_cast<MsgInterface*>(input_buffer.get());
//  CallbackInterface *callback_ptr = static_cast<CallbackInterface*>(context);
  if (NULL == msg_ptr /*|| NULL == callback_ptr*/) {
    return BaseCallbackImpl::Callback(input_buffer, output_buffer, context);
  }

  uint32_t msg_id  = msg_ptr->GetMessageID();
  BufferInterfacePtr msg_buffer = msg_ptr->GetMessageBody();

  std::string event_name = GetPlatformInfo()->GetMessageNameById(msg_id);
  if (event_name.empty()) {
    YSOS_LOG_ERROR("event name is empty");
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  std::string lower_event_name = boost::to_lower_copy(event_name);
  return HandleMessage(lower_event_name, msg_buffer, NULL);
}

int BaseStrategyProxyCallbackImpl::HandleMessage(const std::string &event_name, BufferInterfacePtr input_buffer, CallbackInterface *context) {
  if (event_name.empty()) {
    YSOS_LOG_ERROR("event name is empty: " << logic_name_);
    return YSOS_ERROR_FAILED;
  }

  int ret = YSOS_ERROR_SUCCESS;
  if (CLIENT_SERVICE == service_type_) {
    ret = DoCustomEventService(event_name, input_buffer, context);
  } else if (INTERNAL_SERVICE == service_type_) {
    ret = DoInternalService(event_name, input_buffer, context);
  } else if (LOCAL_SERVICE == service_type_) {
    ret = DoIoctlService(event_name, input_buffer, context);
  } else {
    YSOS_LOG_ERROR("not support service type: " << service_type_ << " | " << logic_name_);
    ret = YSOS_ERROR_NOT_SUPPORTED;
  }

  return ret;
}

int BaseStrategyProxyCallbackImpl::DoCustomEventService(const std::string &event_name, BufferInterfacePtr input_buffer, CallbackInterface *context) {
  if (NULL == agent_ptr_) {
    YSOS_LOG_ERROR("agent ptr not exist: " << logic_name_);
    return YSOS_ERROR_FAILED;
  }

  BaseAgentImpl *agent_ptr = dynamic_cast<BaseAgentImpl*>(agent_ptr_);
  if (NULL == agent_ptr) {
    YSOS_LOG_ERROR("agent ptr is invalid: " << logic_name_);
    return YSOS_ERROR_FAILED;
  }

  ReqCustomEventServiceParam req_param;
  req_param.service_name = event_name;
  uint8_t *data = GetBufferUtility()->GetBufferData(input_buffer);
  if (NULL != data) {
    req_param.service_param = (char*)data;
  }

  int ret = agent_ptr->DoCustomEventService(req_param);

  return ret;
}

int BaseStrategyProxyCallbackImpl::DoInternalService(const std::string &event_name, BufferInterfacePtr input_buffer, CallbackInterface *context) {
  if (!is_init_strategy_) {
    strategy_ptr_ = GetStrategyInterfaceManager()->FindInterface(strategy_name_);
    is_init_strategy_ = true;
  }

  if (NULL == strategy_ptr_) {
    if (NULL == strategy_ptr_) {
      YSOS_LOG_ERROR("strategy ptr is invalid: " << event_name << " | " << logic_name_);
      return YSOS_ERROR_FAILED;
    }
  }

  uint32_t event_id = GetPlatformInfo()->GetMessageIdByName(event_name);
  if (0 == event_id) {
    YSOS_LOG_ERROR("not invalid event name: " << event_name << " | " << logic_name_);
    return YSOS_ERROR_FAILED;
  }

  MsgInterfacePtr message_ptr = MsgInterfacePtr(new MsgImpl(event_id, input_buffer));
  int ret = strategy_ptr_->SetProperty(PROP_MSG, &message_ptr);

  return ret;
}

int BaseStrategyProxyCallbackImpl::DoIoctlService(const std::string &event_name, BufferInterfacePtr input_buffer, CallbackInterface *context) {
  IoctlMap::iterator it = ioctl_map_.find(event_name);
  if (ioctl_map_.end() == it) {
    YSOS_LOG_ERROR("event name not exist: " << event_name << " | " << logic_name_);
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  ReqIOCtlServiceParam req = it->second;
  ModuleInterfacePtr service_ptr = GetModuleInterfaceManager()->FindInterface(req.service_name);
  if (NULL == service_ptr) {
    YSOS_LOG_ERROR("service not exist: " << req.service_name << " | " << logic_name_);
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  uint8_t *data = GetBufferUtility()->GetBufferData(input_buffer);
  if (NULL != data) {
    req.value = (const char*)data;
  }
  int type_id =0;
  type_id = GetUtility()->ConvertFromString(req.id, type_id);
  int ret = service_ptr->Ioctl(type_id, &req.value);

  return ret;
}

}
