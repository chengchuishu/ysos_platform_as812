/**
 *@file baseagentimpl.cpp
 *@brief Implement of base agent implement
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-06-21 08:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// Private Headers
#include "../../../protect/include/sys_framework_package/baseagentimpl.h"
/// C++ Headers //  NOLINT
#include <cassert>
#include <algorithm>
/// Platform Headers
#include "../../../protect/include/sys_framework_package/basestrategyimpl.h"
#include "../../../protect/include/os_hal_package/basedataimpl.h"
#include "../../../public/include/sys_interface_package/messagetype.h"
#include "../../../public/include/os_hal_package/bufferutility.h"
#include "../../../protect/include/sys_framework_package/manager_declare.h"
#include "../../../protect/include/sys_framework_package/platformservice.h"
#include "../../../public/include/sys_interface_package/ysos_enum.h"
#include "../../../protect/include/os_hal_package/msgimpl.h"
#include "../../../protect/include/sys_framework_package/platforminfo.h"
#include "../../../protect/include/os_hal_package/datainterfacemanager.h"
#include "../../../public/include/core_help_package/jsonutility.h"

namespace ysos {
DEFINE_SINGLETON(BaseAgentImpl);
BaseAgentImpl::BaseAgentImpl(const std::string& strClassName): BaseInterfaceImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.framework.agent");
}

BaseAgentImpl::~BaseAgentImpl(void) {
  session_ptr_ = NULL;
}

BaseAgentImpl::BaseAgentImpl(const PlatformRPCPtr &platform_sdk_ptr, const SessionInterface *session_ptr, const std::string &strClassName): BaseInterfaceImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.framework.agent");
  platform_rpc_ptr_ = platform_sdk_ptr;
  session_ptr_ = session_ptr;
}

int BaseAgentImpl::Initialize(void *param) {
  YSOS_LOG_DEBUG("BaseAgentImpl::Initialize[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {
    YSOS_LOG_DEBUG("BaseAgentImpl::Initialize[Check Point][0]");

    if (NULL == param) {
      YSOS_LOG_DEBUG("BaseAgentImpl::Initialize[Fail][0]");
      break;
    }

    YSOS_LOG_DEBUG("BaseAgentImpl::Initialize[Check Point][1]");

    std::string* service_param_ptr = static_cast<std::string*>(param);

    YSOS_LOG_DEBUG("BaseAgentImpl::Initialize[Check Point][2]");

    ReqInitServiceParam service_param;
    if (true != service_param_ptr->empty()) {
      if (true != service_param.FromString(*service_param_ptr)) {
        YSOS_LOG_DEBUG("BaseAgentImpl::Initialize[Fail][1]");
        break;
      }
    }

    YSOS_LOG_DEBUG("app_name[" << service_param.app_name << "]");
    YSOS_LOG_DEBUG("strategy_name[" << service_param.strategy_name << "]");

    YSOS_LOG_DEBUG("BaseAgentImpl::Initialize[Check Point][3]");

    if (NULL != strategy_ptr_) {
      YSOS_LOG_DEBUG("BaseAgentImpl::Initialize[Fail][2]");
      break;
    }

    YSOS_LOG_DEBUG("BaseAgentImpl::Initialize[Check Point][4]");

    StrategyInterfaceManager* strategy_manager_ptr = GetStrategyInterfaceManager();

    YSOS_LOG_DEBUG("BaseAgentImpl::Initialize[Check Point][5]");

    if (NULL == strategy_manager_ptr) {
      YSOS_LOG_DEBUG("BaseAgentImpl::Initialize[Fail][3]");
      break;
    }

    YSOS_LOG_DEBUG("BaseAgentImpl::Initialize[Check Point][6]");

    strategy_ptr_ = ysos::GetStrategyInterfaceManager()->FindInterface(service_param.strategy_name);
    if (NULL == strategy_ptr_) {
      YSOS_LOG_DEBUG("BaseAgentImpl::Initialize[Fail][4]");
      break;
    }

    YSOS_LOG_DEBUG("BaseAgentImpl::Initialize[Check Point][7]");

    /// 获取状态机默认状态
    if (YSOS_ERROR_SUCCESS != strategy_ptr_->GetProperty(PROP_STATE, &default_state_)) {
      YSOS_LOG_DEBUG("BaseAgentImpl::Initialize[Fail][5]");
      break;
    }

    if (true == default_state_.empty()) {
      YSOS_LOG_DEBUG("BaseAgentImpl::Initialize[default_statemachine_state_ is empty]");
      break;
    }

    YSOS_LOG_DEBUG("BaseAgentImpl::Initialize[Check Point][8]");

    if (YSOS_ERROR_SUCCESS != RegisterAppToStrategy(service_param.strategy_name)) {
      YSOS_LOG_DEBUG("BaseAgentImpl::Initialize[Fail][5]");
      break;
    }

    YSOS_LOG_DEBUG("BaseAgentImpl::Initialize[Check Point][9]");

    if (YSOS_ERROR_SUCCESS != strategy_ptr_->Run()) {
      YSOS_LOG_DEBUG("BaseAgentImpl::Initialize[Fail][6]");
      break;
    }

    YSOS_LOG_DEBUG("BaseAgentImpl::Initialize[Check Point][10]");

    if (NULL != data_ptr_) {
      YSOS_LOG_DEBUG("BaseAgentImpl::Initialize[Fail][7]");
      break;
    }

    YSOS_LOG_DEBUG("BaseAgentImpl::Initialize[Check Point][11]");

//    data_ptr_ = BaseDataImplPtr(BaseDataImpl::CreateInstance());
    data_ptr_ = GetDataInterfaceManager()->GetData(GLOBAL_DATA_KEY);
    if (NULL == data_ptr_) {
      YSOS_LOG_DEBUG("BaseAgentImpl::Initialize[Fail][8]");
      break;
    }

    YSOS_LOG_DEBUG("BaseAgentImpl::Initialize[Check Point][12]");

    result = YSOS_ERROR_SUCCESS;

  } while (0);

  YSOS_LOG_DEBUG("BaseAgentImpl::Initialize[Exit]");

  return result;
}

int BaseAgentImpl::UnInitialize(void *param) {

  YSOS_LOG_DEBUG("BaseAgentImpl::UnInitialize[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {

    if (NULL == param) {
      YSOS_LOG_DEBUG("BaseAgentImpl::UnInitialize[Fail][0]");
      break;
    }

    YSOS_LOG_DEBUG("BaseAgentImpl::UnInitialize[Check Point][0]");

    AgentInterface* agent_ptr = static_cast<AgentInterface*>(param);

    /// 在对应的状态机里，反注册agent
    if (YSOS_ERROR_SUCCESS != strategy_ptr_->SetProperty(PROP_UNREG_AGENT, agent_ptr)) {
      YSOS_LOG_DEBUG("BaseAgentImpl::UnInitialize[Fail][1]");
      break;
    }

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  YSOS_LOG_DEBUG("BaseAgentImpl::UnInitialize[Exit]");

  return result;
}

AgentInterface *BaseAgentImpl::Clone(const PlatformRPCPtr &platform_sdk_ptr, const SessionInterface *session_ptr, void* param) {
  BaseAgentImpl* base_agent_impl_ptr = NULL;

  do {
    base_agent_impl_ptr = new BaseAgentImpl(platform_sdk_ptr, session_ptr);
    if (NULL == base_agent_impl_ptr) {
      break;
    }

    if (YSOS_ERROR_SUCCESS != base_agent_impl_ptr->Initialize(param)) {
      YSOS_DELETE(base_agent_impl_ptr);
    }
  } while (0);

  return base_agent_impl_ptr;
}

int BaseAgentImpl::Destroy(AgentInterface *agent_ptr) {

  YSOS_LOG_DEBUG("BaseAgentImpl::Destroy[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {
    if (NULL == agent_ptr) {
      YSOS_LOG_DEBUG("BaseAgentImpl::Destroy[Fail][0]");
      break;
    }

    YSOS_LOG_DEBUG("BaseAgentImpl::Destroy[Check Point][0]");

    if (YSOS_ERROR_SUCCESS != agent_ptr->UnInitialize(agent_ptr)) {
      YSOS_LOG_DEBUG("BaseAgentImpl::Destroy[Fail][1]");
      break;
    }

    YSOS_LOG_DEBUG("BaseAgentImpl::Destroy[Check Point][1]");

    YSOS_DELETE(agent_ptr);

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  YSOS_LOG_DEBUG("BaseAgentImpl::Destroy[Exit]");

  return result;
}

/**
*@brief 处理上层App请求或者响应函数，供basesessionimpl调用公有接口
*/
void BaseAgentImpl::HandleDoService(const std::string& service_name, const std::string& service_param) {

  int result = YSOS_ERROR_FAILED;

  do {
    if (true == service_name.empty()) {
      break;
    }

    MsgInterfacePtr message_ptr = NULL;

    if (YSOS_AGENT_SERVICE_NAME_REG_EVENT == service_name) {
      ReqRegServiceEventServiceParam req_service_param;
      if (true != req_service_param.FromString(service_param)) {
        break;
      }

      message_ptr = PackMessage(YSOS_AGENT_MESSAGE_REG_EVENT_REQ, service_param);
      if (NULL == message_ptr) {
        break;
      }

      if (YSOS_ERROR_SUCCESS != RegisterMessageToStrategy(message_ptr)) {
        break;
      }

    } else if (YSOS_AGENT_SERVICE_NAME_UNREG_EVENT == service_name) {
      ReqUnregServiceEventServiceParam req_service_param;
      if (true != req_service_param.FromString(service_param)) {
        break;
      }

      message_ptr = PackMessage(YSOS_AGENT_MESSAGE_UNREG_EVENT_REQ, service_param);
      if (NULL == message_ptr) {
        break;
      }

      if (YSOS_ERROR_SUCCESS != RegisterMessageToStrategy(message_ptr)) {
        break;
      }

    } else if (YSOS_AGENT_SERVICE_NAME_IOCTL == service_name) {
      ReqIOCtlServiceParam req_service_param;
      if (true != req_service_param.FromString(service_param)) {
        break;
      }

      message_ptr = PackMessage(YSOS_AGENT_MESSAGE_IOCTL_REQ, service_param);
      if (NULL == message_ptr) {
        break;
      }

      if (YSOS_ERROR_SUCCESS != RegisterMessageToStrategy(message_ptr)) {
        break;
      }

    } else if (YSOS_AGENT_SERVICE_NAME_GET_SERVICE_STATE == service_name) {
      ReqGetServiceStateServiceParam req_service_param;
      if (true != req_service_param.FromString(service_param)) {
        break;
      }

      message_ptr = PackMessage(YSOS_AGENT_MESSAGE_GET_SERVICE_STATE_REQ, service_param);
      if (NULL == message_ptr) {
        break;
      }

      if (YSOS_ERROR_SUCCESS != RegisterMessageToStrategy(message_ptr)) {
        break;
      }

    } else if (YSOS_AGENT_SERVICE_NAME_SWITCH == service_name) {
      message_ptr = PackMessage(YSOS_AGENT_MESSAGE_SWITCH_REQ, service_param);
      if (NULL == message_ptr) {
        break;
      }

      if (YSOS_ERROR_SUCCESS != RegisterMessageToStrategy(message_ptr)) {
        break;
      }

    } else if (YSOS_AGENT_SERVICE_NAME_READY == service_name) {
      message_ptr = PackMessage(YSOS_AGENT_MESSAGE_READY_RSQ, service_param);
      if (NULL == message_ptr) {
        break;
      }

      if (YSOS_ERROR_SUCCESS != RegisterMessageToStrategy(message_ptr)) {
        break;
      }

    } else if (YSOS_AGENT_SERVICE_NAME_LOAD_CONFIG == service_name) {
      ReqLoadConfigServiceParam req_service_param;

      if (true != req_service_param.FromString(service_param)) {
        break;
      }

      HandleLoadConfigService(req_service_param);
      break;

    } else if (YSOS_AGENT_SERVICE_NAME_HEART_BEAT == service_name) {
      ReqHeartbeatServiceParam req_service_param;
      if (true != req_service_param.FromString(service_param)) {
        break;
      }

      HandleHeartbeatService(req_service_param);
      break;

    } else if (YSOS_AGENT_SERVICE_NAME_WRITE_LOG == service_name) {
      ReqWriteLogServiceParam req_service_param;
      if (true != req_service_param.FromString(service_param)) {
        break;
      }

      HandleWriteLogService(req_service_param);
      break;

    } else if (YSOS_AGENT_SERVICE_NAME_SERVICE_LIST == service_name) {
      ReqGetServiceListServiceParam req_service_param;
      if (true != req_service_param.FromString(service_param)) {
        break;
      }

      HandleGetServiceListService(req_service_param);
      break;

    } else if (YSOS_AGENT_SERVICE_NAME_GET_DATA == service_name) {
      ReqGetDataServiceParam req_service_param;
      if (true != req_service_param.FromString(service_param)) {
        break;
      }

      HandleGetDataService(req_service_param);
      break;

    } else if (YSOS_AGENT_SERVICE_NAME_SET_DATA == service_name) {
      ReqSetDataServiceParam req_service_param;
      if (true != req_service_param.FromString(service_param)) {
        break;
      }

      HandleSetDataService(req_service_param);
      break;

    } else if (YSOS_AGENT_SERVICE_NAME_CUSTOM_EVENT == service_name) {
      YSOS_LOG_DEBUG("service_name: " << service_name);

      ReqCustomEventServiceParam req_service_param;
      if (true != req_service_param.FromString(service_param)) {
        YSOS_LOG_DEBUG("BaseAgentImpl::HandleDoService[Fail]The function[req_service_param.FromString()] is failed");
        break;
      }

      YSOS_LOG_DEBUG("req_service_param.service_name: " << req_service_param.service_name);

      UINT32 message_id = PlatformInfo::Instance()->GetMessageIdByName(req_service_param.service_name);
      YSOS_LOG_DEBUG("message_id: " << message_id);

      message_ptr = PackMessage(message_id, req_service_param.service_param);
      if (NULL == message_ptr) {
        YSOS_LOG_DEBUG("BaseAgentImpl::HandleDoService[Fail]The function[PackMessage()] is failed");
        break;
      }

      if (YSOS_ERROR_SUCCESS != RegisterMessageToStrategy(message_ptr)) {
        YSOS_LOG_DEBUG("BaseAgentImpl::HandleDoService[Fail]The function[RegisterMessageToStrategy()] is failed");
        break;
      }

      /// 直接返回成功
      boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
      ResCustomEventServiceParam res_service_param;
      res_service_param.status_code = "0";
      res_service_param.description = "succeeded";

      if (YSOS_ERROR_SUCCESS != DoCustomEventDispatch(res_service_param)) {
        YSOS_LOG_DEBUG("BaseAgentImpl::HandleDoService[Fail]The function[DoCustomEventDispatch()] is failed");
        break;
      }

    } else if (YSOS_AGENT_SERVICE_NAME_EVENT_NOTIFY == service_name) {
      /// 为了平台请求外围辅助进程获取服务支持用，例如更新进程生成的数据透传给前段应用
      YSOS_LOG_DEBUG("service_name: " << service_name);

      ReqEventNotifyServiceParam req_service_param;
      if (true != req_service_param.FromString(service_param)) {
        YSOS_LOG_DEBUG("BaseAgentImpl::HandleDoService[Fail]The function[req_service_param.FromString()] is failed");
        break;
      }

      YSOS_LOG_DEBUG("req_service_param.event_name: " << req_service_param.event_name);
      YSOS_LOG_DEBUG("req_service_param.callback_name: " << req_service_param.callback_name);
      YSOS_LOG_DEBUG("req_service_param.data: " << req_service_param.data);

      UINT32 message_id = PlatformInfo::Instance()->GetMessageIdByName(req_service_param.event_name);
      YSOS_LOG_DEBUG("message_id: " << message_id);

      message_ptr = PackMessage(message_id, req_service_param.data);
      if (NULL == message_ptr) {
          YSOS_LOG_DEBUG("BaseAgentImpl::HandleDoService[Fail]The function[PackMessage()] is failed");
          break;
      }

      if (YSOS_ERROR_SUCCESS != RegisterMessageToStrategy(message_ptr)) {
          YSOS_LOG_DEBUG("BaseAgentImpl::HandleDoService[Fail]The function[RegisterMessageToStrategy()] is failed");
          break;
      }
    } else {
      break;
    }

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  return;
}

void BaseAgentImpl::HandleOnDispatchMessage(const std::string& service_name, const std::string& result) {
  int return_value = YSOS_ERROR_FAILED;

  do {

    if (true == service_name.empty() || true == result.empty()) {
      break;
    }

    if (YSOS_AGENT_SERVICE_NAME_SWITCH_NOTIFY == service_name) {
      ResSwitchNotifyServiceParam res_service_param;
      if (true != res_service_param.FromString(result)) {
        break;
      }

      HandleSwitchNotifyDispatch(res_service_param);

    } else if (YSOS_AGENT_SERVICE_NAME_EVENT_NOTIFY == service_name) {
      ResEventNotifyServiceParam res_service_param;
      if (true != res_service_param.FromString(result)) {
        break;
      }

      HandleEventNotifyDispatch(res_service_param);

    } else {
      break;
    }

    return_value = YSOS_ERROR_SUCCESS;
  } while (0);

  return;
}

void BaseAgentImpl::HandleError(const std::string& error_param) {
  return;
}


/**
*@brief 向上层App发送请求或者响应函数，供basestatemachineimpl调用公有接口
*/

/// 平台向上层App发送服务请求
int BaseAgentImpl::DoSwitchNotifyService(ReqSwitchNotifyServiceParam& service_param) {
  int result = YSOS_ERROR_FAILED;

  do {
    std::string request_service_param;
    if (true != service_param.ToString(request_service_param)) {
      break;
    }

    if (YSOS_ERROR_SUCCESS != DoService(YSOS_AGENT_SERVICE_NAME_SWITCH_NOTIFY, request_service_param)) {
      break;
    }

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  return result;
}

int BaseAgentImpl::DoEventNotifyService(ReqEventNotifyServiceParam& service_param) {
  int result = YSOS_ERROR_FAILED;

  do {
    std::string request_service_param;
    if (true != service_param.ToString(request_service_param)) {
      break;
    }

    if (YSOS_ERROR_SUCCESS != DoService(YSOS_AGENT_SERVICE_NAME_EVENT_NOTIFY, request_service_param)) {
      break;
    }

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  return result;
}

int BaseAgentImpl::DoCustomEventService(ReqCustomEventServiceParam& service_param) {
  int result = YSOS_ERROR_FAILED;

  do {
    std::string request_service_param;
    if (true != service_param.ToString(request_service_param)) {
      break;
    }

    if (YSOS_ERROR_SUCCESS != DoService(YSOS_AGENT_SERVICE_NAME_CUSTOM_EVENT, request_service_param)) {
      break;
    }

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  return result;
}

/// 平台向上层App发送服务响应
int BaseAgentImpl::DoSwitchDispatch(ResSwitchServiceParam& result) {
  int return_value = YSOS_ERROR_FAILED;

  do {
    std::string response_result;
    if (true != result.ToString(response_result)) {
      break;
    }

    if (YSOS_ERROR_SUCCESS != OnDispatchMessage(YSOS_AGENT_SERVICE_NAME_SWITCH, response_result)) {
      break;
    }

    return_value = YSOS_ERROR_SUCCESS;
  } while (0);

  return return_value;
}

int BaseAgentImpl::DoRegServiceEventDispatch(ResRegServiceEventServiceParam& result) {
  int return_value = YSOS_ERROR_FAILED;

  do {
    std::string response_result;
    if (true != result.ToString(response_result)) {
      break;
    }

    if (YSOS_ERROR_SUCCESS != OnDispatchMessage(YSOS_AGENT_SERVICE_NAME_REG_EVENT, response_result)) {
      break;
    }

    return_value = YSOS_ERROR_SUCCESS;
  } while (0);

  return return_value;
}

int BaseAgentImpl::DoUnregServiceEventDispatch(ResUnregServiceEventServiceParam& result) {
  int return_value = YSOS_ERROR_FAILED;

  do {
    std::string response_result;
    if (true != result.ToString(response_result)) {
      break;
    }

    if (YSOS_ERROR_SUCCESS != OnDispatchMessage(YSOS_AGENT_SERVICE_NAME_UNREG_EVENT, response_result)) {
      break;
    }

    return_value = YSOS_ERROR_SUCCESS;
  } while (0);

  return return_value;
}

int BaseAgentImpl::DoReadyDispatch(ResReadyServiceParam& result) {
  int return_value = YSOS_ERROR_FAILED;

  do {
    std::string response_result;
    if (true != result.ToString(response_result)) {
      break;
    }

    if (YSOS_ERROR_SUCCESS != OnDispatchMessage(YSOS_AGENT_SERVICE_NAME_READY, response_result)) {
      break;
    }

    return_value = YSOS_ERROR_SUCCESS;
  } while (0);

  return return_value;
}

int BaseAgentImpl::DoIOCtlDispatch(ResIOCtlServiceParam& result) {
  int return_value = YSOS_ERROR_FAILED;

  do {
    std::string response_result;
    if (true != result.ToString(response_result)) {
      break;
    }

    if (YSOS_ERROR_SUCCESS != OnDispatchMessage(YSOS_AGENT_SERVICE_NAME_IOCTL, response_result)) {
      break;
    }

    return_value = YSOS_ERROR_SUCCESS;
  } while (0);

  return return_value;
}

int BaseAgentImpl::DoGetServiceStateDispatch(ResGetServiceStateServiceParam& result) {
  int return_value = YSOS_ERROR_FAILED;

  do {
    std::string response_result;
    if (true != result.ToString(response_result)) {
      break;
    }

    if (YSOS_ERROR_SUCCESS != OnDispatchMessage(YSOS_AGENT_SERVICE_NAME_GET_SERVICE_STATE, response_result)) {
      break;
    }

    return_value = YSOS_ERROR_SUCCESS;
  } while (0);

  return return_value;
}

int BaseAgentImpl::DoCustomEventDispatch(ResCustomEventServiceParam& result) {
  int return_value = YSOS_ERROR_FAILED;

  do {
    std::string response_result;
    if (true != result.ToString(response_result)) {
      break;
    }

    if (YSOS_ERROR_SUCCESS != OnDispatchMessage(YSOS_AGENT_SERVICE_NAME_CUSTOM_EVENT, response_result)) {
      break;
    }

    return_value = YSOS_ERROR_SUCCESS;
  } while (0);

  return return_value;
}


/**
*@brief 按具体服务类型细化的服务处理函数
*/

/// 服务处理函数，上层App发送请求，平台响应
int BaseAgentImpl::HandleHeartbeatService(const ReqHeartbeatServiceParam& service_param) {
  int result = YSOS_ERROR_FAILED;

  do {
    std::string response_result;
    ResHeartbeatServiceParam response_service_param;
    if (true != response_service_param.ToString(response_result)) {
      break;
    }

    if (YSOS_ERROR_SUCCESS != OnDispatchMessage(YSOS_AGENT_SERVICE_NAME_HEART_BEAT, response_result)) {
      break;
    }

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  return result;
}

int BaseAgentImpl::HandleWriteLogService(const ReqWriteLogServiceParam& service_param) {
  int result = YSOS_ERROR_FAILED;

  do {

    /// To do

    std::string response_result;
    ResHeartbeatServiceParam response_service_param;
    if (true != response_service_param.ToString(response_result)) {
      break;
    }

    if (YSOS_ERROR_SUCCESS != OnDispatchMessage(YSOS_AGENT_SERVICE_NAME_WRITE_LOG, response_result)) {
      break;
    }

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  return result;
}

int BaseAgentImpl::HandleGetServiceListService(const ReqGetServiceListServiceParam& service_param) {
  int result = YSOS_ERROR_FAILED;

  do {
    PlatformService::ServiceInfoMap service_info_map = GetPlatformService()->GetServiceMap();
    if (0 < service_info_map.size()) {
      break;
    }

    ResGetServiceListServiceParam response_service_param;
    response_service_param.status_code = "0";
    response_service_param.description = "succeeded";

    PlatformService::ServiceInfoPtr service_info_ptr = NULL;
    for (PlatformService::ServiceInfoMap::iterator iterator = service_info_map.begin();
         iterator != service_info_map.end();
         ++iterator
        ) {
      service_info_ptr = iterator->second;
      if (NULL != service_info_ptr) {
        if ("module" == service_info_ptr->type) {
          ServiceListUnit service_list_unit;
          service_list_unit.name = service_info_ptr->name;
          service_list_unit.alias = service_info_ptr->alias;
          service_list_unit.description = service_info_ptr->description;
          response_service_param.service_list_units.push_back(service_list_unit);
        }
      }
    }

    std::string response_result;
    if (true != response_service_param.ToString(response_result)) {
      break;
    }

    if (YSOS_ERROR_SUCCESS != OnDispatchMessage(YSOS_AGENT_SERVICE_NAME_SERVICE_LIST, response_result)) {
      break;
    }

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  return result;
}

int BaseAgentImpl::HandleLoadConfigService(const ReqLoadConfigServiceParam& service_param) {
  int result = YSOS_ERROR_FAILED;

  do {
    /// To do

    std::string response_result;
    ResLoadConfigServiceParam response_service_param;
    if (true != response_service_param.ToString(response_result)) {
      break;
    }

    if (YSOS_ERROR_SUCCESS != OnDispatchMessage(YSOS_AGENT_SERVICE_NAME_LOAD_CONFIG, response_result)) {
      break;
    }

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  return result;
}

int BaseAgentImpl::HandleGetDataService(const ReqGetDataServiceParam& service_param) {
  YSOS_LOG_DEBUG("BaseAgentImpl::HandleGetDataService[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {

    YSOS_LOG_DEBUG("BaseAgentImpl::HandleGetDataService[Check Point][0]");

    if (NULL == data_ptr_) {
      YSOS_LOG_DEBUG("BaseAgentImpl::HandleGetDataService[Fail][0]");
      break;
    }

    YSOS_LOG_DEBUG("BaseAgentImpl::HandleGetDataService[Check Point][1]");

    ResGetDataServiceParam response_service_param;

    bool success = true;
    KeyValueUnit key_value_unit;
    std::string value;
    for (
      KeyUnits::const_iterator iterator = service_param.key_units.begin();
      iterator != service_param.key_units.end();
      ++iterator
    ) {
      YSOS_LOG_DEBUG("BaseAgentImpl::HandleGetDataService[key][" << iterator->key << "]");

      if (YSOS_ERROR_SUCCESS != data_ptr_->GetData(iterator->key, value)) {
        YSOS_LOG_DEBUG("BaseAgentImpl::HandleGetDataService[Fail][1]");
//         DataInterfacePtr data_ptr = GetDataInterfaceManager()->GetData(GLOBAL_DATA_KEY);
//         if (NULL == data_ptr) {
//           YSOS_LOG_DEBUG("BaseAgentImpl::HandleGetDataService[Fail][2]");
//           success = false;
//           break;
//         }
//
//         if (YSOS_ERROR_SUCCESS != data_ptr->GetData(iterator->key, value)) {
//           YSOS_LOG_DEBUG("BaseAgentImpl::HandleGetDataService[Fail][3]");
//           success = false;
//         }
        success = false;
      }

      key_value_unit.Clear();
      key_value_unit.key = iterator->key;
      key_value_unit.value = value;

      response_service_param.key_value_units.push_back(key_value_unit);
    }

    YSOS_LOG_DEBUG("BaseAgentImpl::HandleGetDataService[Check Point][2]");

    if (true == success) {
      YSOS_LOG_DEBUG("BaseAgentImpl::HandleGetDataService[Check Point][3]");
      response_service_param.status_code = boost::lexical_cast<std::string>(YSOS_ERROR_SUCCESS);
      response_service_param.description = g_response_result_description_succeeded;
    } else {
      YSOS_LOG_DEBUG("BaseAgentImpl::HandleGetDataService[Check Point][4]");
      response_service_param.status_code = boost::lexical_cast<std::string>(YSOS_ERROR_FAILED);
      response_service_param.description = g_response_result_description_faild;
    }

    std::string response_result;
    if (true != response_service_param.ToString(response_result)) {
      YSOS_LOG_DEBUG("BaseAgentImpl::HandleGetDataService[Fail][4]");
      break;
    }

    YSOS_LOG_DEBUG("BaseAgentImpl::HandleGetDataService[Check Point][5]");

    if (YSOS_ERROR_SUCCESS != OnDispatchMessage(YSOS_AGENT_SERVICE_NAME_GET_DATA, response_result)) {
      YSOS_LOG_DEBUG("BaseAgentImpl::HandleGetDataService[Fail][5]");
      break;
    }

    YSOS_LOG_DEBUG("BaseAgentImpl::HandleGetDataService[Check Point][6]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  YSOS_LOG_DEBUG("BaseAgentImpl::HandleGetDataService[Exit]");

  return result;
}

int BaseAgentImpl::HandleSetDataService(const ReqSetDataServiceParam& service_param) {
  YSOS_LOG_DEBUG("BaseAgentImpl::HandleSetDataService[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {
    YSOS_LOG_DEBUG("BaseAgentImpl::HandleSetDataService[Check Point][0]");

    if (NULL == data_ptr_) {
      YSOS_LOG_DEBUG("BaseAgentImpl::HandleSetDataService[Fail][0]");
      break;
    }

    YSOS_LOG_DEBUG("BaseAgentImpl::HandleSetDataService[Check Point][1]");

    bool success = true;
    for (
      KeyValueUnits::const_iterator iterator = service_param.key_value_units.begin();
      iterator != service_param.key_value_units.end();
      ++iterator
    ) {
      YSOS_LOG_DEBUG("BaseAgentImpl::HandleSetDataService[key][" << iterator->key << "]");

      YSOS_LOG_DEBUG("BaseAgentImpl::HandleSetDataService[value][" << iterator->value << "]");

      if (YSOS_ERROR_SUCCESS != data_ptr_->SetData(iterator->key, iterator->value)) {
        YSOS_LOG_DEBUG("BaseAgentImpl::HandleSetDataService[Fail][1]");
        success = false;
        break;
      }
    }

    YSOS_LOG_DEBUG("BaseAgentImpl::HandleSetDataService[Check Point][2]");

    ResSetDataServiceParam response_service_param;
    if (true == success) {
      YSOS_LOG_DEBUG("BaseAgentImpl::HandleSetDataService[Check Point][3]");
      response_service_param.status_code = boost::lexical_cast<std::string>(YSOS_ERROR_SUCCESS);
      response_service_param.description = g_response_result_description_succeeded;
    } else {
      YSOS_LOG_DEBUG("BaseAgentImpl::HandleSetDataService[Check Point][4]");
      response_service_param.status_code = boost::lexical_cast<std::string>(YSOS_ERROR_FAILED);
      response_service_param.description = g_response_result_description_faild;
    }

    YSOS_LOG_DEBUG("BaseAgentImpl::HandleSetDataService[Check Point][5]");

    std::string response_result;
    if (true != response_service_param.ToString(response_result)) {
      YSOS_LOG_DEBUG("BaseAgentImpl::HandleSetDataService[Fail][2]");
      break;
    }

    YSOS_LOG_DEBUG("BaseAgentImpl::HandleSetDataService[Check Point][6]");

    if (YSOS_ERROR_SUCCESS != OnDispatchMessage(YSOS_AGENT_SERVICE_NAME_SET_DATA, response_result)) {
      YSOS_LOG_DEBUG("BaseAgentImpl::HandleSetDataService[Fail][3]");
      break;
    }

    YSOS_LOG_DEBUG("BaseAgentImpl::HandleSetDataService[Check Point][7]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  YSOS_LOG_DEBUG("BaseAgentImpl::HandleSetDataService[Exit]");

  return result;
}

/// 处理上层App响应平台请求服务函数，平台发送请求，上层App响应
int BaseAgentImpl::HandleSwitchNotifyDispatch(ResSwitchNotifyServiceParam& result) {
  YSOS_LOG_DEBUG("status_code: " << result.status_code);
  YSOS_LOG_DEBUG("description: " << result.description);

  int return_value = YSOS_ERROR_FAILED;

  do {
    return_value = YSOS_ERROR_SUCCESS;
  } while (0);

  return return_value;
}

int BaseAgentImpl::HandleEventNotifyDispatch(ResEventNotifyServiceParam& result) {
  YSOS_LOG_DEBUG("status_code: " << result.status_code);
  YSOS_LOG_DEBUG("description: " << result.description);

  int return_value = YSOS_ERROR_FAILED;

  do {
    return_value = YSOS_ERROR_SUCCESS;
  } while (0);

  return return_value;
}

/// 其他辅助函数
int BaseAgentImpl::RegisterAppToStrategy(const std::string& app_name) {
  int result = YSOS_ERROR_FAILED;

  do {
    if (true == app_name.empty()) {
      break;
    }

    if (NULL == strategy_ptr_) {
      break;
    }

    AgentServicePair app_name_agent_pair = std::make_pair(app_name, this);
    AgentInterface *agent_ptr = this;
    if (YSOS_ERROR_SUCCESS != strategy_ptr_->SetProperty(PROP_AGENT, agent_ptr)) {
      break;
    }

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  return result;
}

int BaseAgentImpl::RegisterMessageToStrategy(MsgInterfacePtr message_ptr) {
  int result = YSOS_ERROR_FAILED;

  do {

    if (NULL == message_ptr) {
      break;
    }

    if (NULL == strategy_ptr_) {
      break;
    }

    if (YSOS_ERROR_SUCCESS != strategy_ptr_->SetProperty(PROP_MSG, &message_ptr)) {
      break;
    }

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  return result;
}

StrategyInterfacePtr BaseAgentImpl::GetStrategy(const std::string &strategy_name) {
  return strategy_ptr_;
}

MsgInterfacePtr BaseAgentImpl::PackMessage(const UINT32 message_id, const std::string& service_param) {
  MsgInterfacePtr message_ptr = NULL;

  do {
    if (0 == message_id) {
      break;
    }

    BufferInterfacePtr buffer_ptr = NULL;
    if (true != service_param.empty()) {
      buffer_ptr = BufferUtility::Instance()->CreateBuffer(service_param);
      if (NULL == buffer_ptr) {
        break;
      }
    }

    message_ptr = MsgInterfacePtr(new MsgImpl(message_id, buffer_ptr));
    if (NULL == message_ptr) {
      break;
    }

  } while (0);

  return message_ptr;
}

int BaseAgentImpl::DoService(const std::string &service_name, const std::string &service_param) {
  assert(NULL != platform_rpc_ptr_ && NULL != session_ptr_);
  std::string session_id = ((SessionInterface*)session_ptr_)->GetSessionId();
  platform_rpc_ptr_->DoService(session_id.c_str(), service_name.c_str(), service_param.c_str());

  return YSOS_ERROR_SUCCESS;
}

int BaseAgentImpl::OnDispatchMessage(const std::string &service_name, const std::string &result) {
  assert(NULL != platform_rpc_ptr_ && NULL != session_ptr_);
  std::string session_id = ((SessionInterface*)session_ptr_)->GetSessionId();
  platform_rpc_ptr_->OnDispatchMessage(session_id.c_str(), service_name.c_str(), result.c_str());

  return YSOS_ERROR_SUCCESS;
}

int BaseAgentImpl::GetProperty(int type_id, void *type) {
  int result = YSOS_ERROR_NOT_SUPPORTED;
  switch (type_id) {
  case PROP_STATE: {
    if (true == default_state_.empty()) {
      result = YSOS_ERROR_FAILED;
    } else {
      std::string* state_ptr = static_cast<std::string*>(type);
      assert(NULL != state_ptr);
      *state_ptr = default_state_;
      result = YSOS_ERROR_SUCCESS;
    }
  }
  break;
  default:
    break;
  }

  return result;
}

int BaseAgentImpl::SetProperty(int type_id, void *type) {
  int result = YSOS_ERROR_NOT_SUPPORTED;
  switch (type_id) {
  case PROP_STATE: {
    if (true == default_state_.empty()) {
      result = YSOS_ERROR_FAILED;
      break;
    }

    /// 切换首个状态
    JsonValue json_value;
    json_value["state_name"] = default_state_;

    std::string json_string;
    if (YSOS_ERROR_SUCCESS != GetJsonUtil()->JsonValueToString(json_value, json_string)) {
      result = YSOS_ERROR_FAILED;
      break;
    }

    MsgInterfacePtr message_ptr = PackMessage(YSOS_AGENT_MESSAGE_SWITCH_REQ, json_string);
    if (NULL == message_ptr) {
      result = YSOS_ERROR_FAILED;
      break;
    }

    if (YSOS_ERROR_SUCCESS != RegisterMessageToStrategy(message_ptr)) {
      result = YSOS_ERROR_FAILED;
      break;
    }

    boost::this_thread::sleep_for(boost::chrono::milliseconds(50));

    /// 默认发送ready
    message_ptr = PackMessage(YSOS_AGENT_MESSAGE_READY_RSQ, "");
    if (NULL == message_ptr) {
      break;
    }

    if (YSOS_ERROR_SUCCESS != RegisterMessageToStrategy(message_ptr)) {
      break;
    }

    result = YSOS_ERROR_SUCCESS;
  }
  break;
  default:
    break;
  }

  return result;
}

}  /// end of namespace
