/**
 *@file BaseStrategyImpl.cpp
 *@brief Definition of BaseStrategyImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// Self Header //  NOLINT
#include "../../../protect/include/sys_framework_package/basestrategyimpl.h"  // NOLINT
/// c headers //  NOLINT
#include <cassert>
/// stl headers //  NOLINT
#include <algorithm>
/// boost headers //  NOLINT
#include <boost/algorithm/string.hpp>
#include <boost/make_shared.hpp>
/// ysos headers //  NOLINT
#include "../../../public/include/core_help_package/xmlutil.h"
#include "../../../protect/include/os_hal_package/msgimpl.h"
#include "../../../protect/include/sys_framework_package/basemoduleimpl.h"
#include "../../../public/include/os_hal_package/filepath.h"
#include "../../../protect/include/sys_framework_package/modulelinkimpl.h"
#include "../../../public/include/os_hal_package/bufferutility.h"
#include "../../../protect/include/sys_framework_package/statemachineconfigparse.h"
//#include "../../../protect/include/protocol_help_package/agent_protocol_utility.h"
#include "../../../protect/include/sys_framework_package/statemachinemanager.h"
#include "../../../protect/include/sys_framework_package/platforminfo.h"
#include "../../../public/include/sys_interface_package/ysos_enum.h"

namespace ysos {
typedef std::map<std::string, AgentInterface*>  AgentServiceMap;
#define YSOS_DEBUG 1
/*************************************************************************************************************
 *                      BaseStrategyImpl                                                                     *
 *************************************************************************************************************/
DECLARE_PLUGIN_REGISTER_INTERFACE(BaseStrategyImpl, StrategyInterface);
BaseStrategyImpl::BaseStrategyImpl(const std::string& strClassName): BaseInterfaceImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.framework.strategy");
}

BaseStrategyImpl:: ~BaseStrategyImpl(void) {
  //UnInitialize();
}

int BaseStrategyImpl::Initialize(void *param) {
  msg_queue_lock_ = new StrategyLock();
  cmd_queue_lock_ = new StrategyLock();
  strategy_lock_ = new StrategyLock();
  agent_map_lock_ = new StrategyLock();
  msg_queue_ = MsgQueueInterfacePtr(MsgQueueImpl::CreateInstance());
  cmd_queue_ = MsgQueueInterfacePtr(MsgQueueImpl::CreateInstance());
  is_run_ = false;
  is_loaded_ = false;
  //agent_map_lock_ = new LightLock();

  if (NULL == msg_queue_lock_ || NULL == cmd_queue_lock_ || NULL == strategy_lock_
      || NULL == agent_map_lock_ || NULL == msg_queue_ || NULL == cmd_queue_) {
    UnInitialize();
    return YSOS_ERROR_MEMORY_ALLOCATED_FAILED;
  }

  LoadStrategy(GetName());

  return BaseInterfaceImpl::Initialize(param);
}

int BaseStrategyImpl::UnInitialize(void *param) {
  YSOS_DELETE(msg_queue_lock_);
  YSOS_DELETE(cmd_queue_lock_);
  YSOS_DELETE(strategy_lock_);
  YSOS_DELETE(agent_map_lock_);
  agent_map_.clear();
  BaseInterface *base_interface = dynamic_cast<BaseInterface*>(strategy_imp_.get());
  if (NULL != base_interface) {
    base_interface->UnInitialize();
  }
  state_name_ = "";

  return BaseInterfaceImpl::UnInitialize(param);
}

int BaseStrategyImpl::LoadStrategy(const std::string &strategy_name) {
  std::string conf_dir = GetPlatformInfo()->GetConfPath();
#ifdef _WIN32
  std::string strategy_full_path = conf_dir + "strategy\\" + strategy_name + ".xml";
#else
  std::string strategy_full_path = conf_dir + "strategy/" + strategy_name + ".xml";//add for linux
#endif
  std::string strategy_xml = GetUtility()->ReadAllDataFromFile(strategy_full_path);
  ParseStrategyXml(strategy_xml);
  is_loaded_ = true;

  return YSOS_ERROR_SUCCESS;
}

int BaseStrategyImpl::CreateStrategy(const std::string &strategy_name, void *params) {
  return YSOS_ERROR_NOT_SUPPORTED;
}

int BaseStrategyImpl::DestroyStrategy() {
  return YSOS_ERROR_NOT_SUPPORTED;
}

int BaseStrategyImpl::SaveStrategy() {
  return YSOS_ERROR_NOT_SUPPORTED;
}

int BaseStrategyImpl::Ioctl(INT32 ctrl_id, LPVOID param) {
  int ret = YSOS_ERROR_SUCCESS;

  switch (ctrl_id) {
  case CMD_CLEAR_MSG: {
    AutoLockOper lock(msg_queue_lock_);
    ret = msg_queue_->Clear();
    break;
  }
  case CMD_CLEAR_CMD: {
    AutoLockOper lock(cmd_queue_lock_);
    MsgInterfacePtr *msg_ptr = static_cast<MsgInterfacePtr*>(param);
    if (NULL != msg_ptr) {
      ret = cmd_queue_->Push(*msg_ptr);
    }
    break;
  }
  case CMD_MECHANISM: {
    if (NULL == strategy_imp_) {
      strategy_imp_ = ControlInterfacePtr(new StateMachineManager(this));
    }
    ret = strategy_imp_->Ioctl(StateMachineManager::CMD_MECHANISM, param);
  }
  break;
  default:
    break;
  }
  return ret;
}

int BaseStrategyImpl::Stop(LPVOID param) {
  AutoLockOper  lock(strategy_lock_);
  if (!is_run_) {
    YSOS_LOG_DEBUG("strategy already stopped: " << this->GetName());
    return YSOS_ERROR_SUCCESS;
  }

  is_run_ = false;
  strategy_imp_->Stop(this);

  return YSOS_ERROR_SUCCESS;
}

int BaseStrategyImpl::Pause(LPVOID param) {
  return YSOS_ERROR_NOT_SUPPORTED;
}

int BaseStrategyImpl::Run(LPVOID param) {
  AutoLockOper  lock(strategy_lock_);
  if (is_run_) {
    YSOS_LOG_DEBUG("Strategy already run: " << this->GetName());
    return YSOS_ERROR_SUCCESS;
  }
  is_run_ = true;
  strategy_imp_->Run(this);

  return YSOS_ERROR_SUCCESS;
}

int BaseStrategyImpl::Flush(LPVOID param) {
  return YSOS_ERROR_SUCCESS;
}

int BaseStrategyImpl::GetProperty(int type_id, void *type) {
  int ret = YSOS_ERROR_SUCCESS;

  switch (type_id) {
  case PROP_MSG: {
    MsgInterfacePtr *msg_ptr = static_cast<MsgInterfacePtr*>(type);
    assert(NULL != msg_ptr);
    MsgInterfacePtr msg = GetMessage();
    *msg_ptr = msg;
  }
  break;
  case PROP_CMD: {
    MsgInterfacePtr *cmd_ptr = static_cast<MsgInterfacePtr*>(type);
    assert(NULL != cmd_ptr);
    MsgInterfacePtr cmd = GetCmd();
    *cmd_ptr = cmd;
  }
  break;
  case PROP_AGENT: {
    assert(NULL != type);
    AgentInterface **agent_ptr_ptr = static_cast<AgentInterface**>(type);
    if (NULL == type) {
      ret = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }

    AgentInterface* agent_ptr = FindAgentByName("");
    *agent_ptr_ptr = agent_ptr;
  }
  break;
  case PROP_STATE_SWITCH_INFO: {
    ret = strategy_imp_->Ioctl(StateMachineManager::CMD_GET_STATE_SWITCH_INFO, type);
  }
  break;
  case CMD_GET_SERVICE_LIST: {
    ret = strategy_imp_->Ioctl(StateMachineManager::CMD_GET_SERVICE_LIST, type);
  }
  break;
  case PROP_STATE: {
    std::string *state = reinterpret_cast<std::string*>(type);
    if (NULL == state) {
      YSOS_LOG_ERROR("get state name failed, type is null");
      break;
    }
    *state = state_name_;
  }
  break;
  default:
    break;
  }
  return YSOS_ERROR_SUCCESS;
}

int BaseStrategyImpl::SetProperty(int type_id, void *type) {
  int ret = YSOS_ERROR_NOT_SUPPORTED;

  switch (type_id) {
  case PROP_MSG: {
    AutoLockOper lock(msg_queue_lock_);
    MsgInterfacePtr *msg_ptr = static_cast<MsgInterfacePtr*>(type);
    MsgInterfacePtr msg = *msg_ptr;
    if (NULL != msg) {
      msg_queue_->Push(msg);
    }
    ret = YSOS_ERROR_SUCCESS;
    break;
  }
  case PROP_CMD: {
    AutoLockOper lock(cmd_queue_lock_);
    MsgInterfacePtr *cmd_ptr = static_cast<MsgInterfacePtr*>(type);
    MsgInterfacePtr cmd = *cmd_ptr;
    if (NULL != cmd) {
      cmd_queue_->Push(cmd);
    }
    ret = YSOS_ERROR_SUCCESS;
    break;
  }
  case PROP_AGENT: {
    assert(NULL != type);
    AgentInterface *agent_ptr = static_cast<AgentInterface*>(type);
    ret = RegAgent(agent_ptr);
  }
  break;
  case PROP_UNREG_AGENT: {
    assert(NULL != type);
    AgentInterface *agent_ptr = static_cast<AgentInterface*>(type);
    ret = UnRegAgent(agent_ptr);
  }
  break;
  /*case CMD_AGENT_DISPATCH: {
    ret = OnDispatch(type);
  }
  break;*/
  default:
    break;
  }

  return ret;
}

AgentInterface* BaseStrategyImpl::FindAgentByName(const std::string &service_name) {
  AgentInterface* agent_ptr = NULL;
  //YSOS_LOG_DEBUG( "service name request is : " << service_name );
  AutoLockOper lock(agent_map_lock_);
  /// 暂时不考虑 service_name //  NOLINT
  if (agent_map_.end() != agent_map_.begin()) {
    agent_ptr = agent_map_.begin()->second;
  }

  return agent_ptr;
}

int BaseStrategyImpl::RegAgent(AgentInterface *agent_ptr) {
  assert(NULL != agent_ptr);
  std::string agent_name = agent_ptr->GetName();
  assert(!agent_name.empty());

  strategy_imp_->Run();
  YSOS_LOG_DEBUG("****** [BaseStrategyImpl::RegAgent] [before] ******" << agent_name);
  AutoLockOper lock(agent_map_lock_);
  YSOS_LOG_DEBUG("****** [BaseStrategyImpl::RegAgent] [after] ******" << agent_name);
  if (agent_map_.end() == agent_map_.find(agent_name)) {
    agent_map_.insert(std::make_pair(agent_name, agent_ptr));
  }
  YSOS_LOG_DEBUG("****** [BaseStrategyImpl::RegAgent] [End] ******");
  return YSOS_ERROR_SUCCESS;
}

int BaseStrategyImpl::UnRegAgent(AgentInterface *agent_ptr) {
  assert(NULL != agent_ptr);
  std::string agent_name = agent_ptr->GetName();
  assert(!agent_name.empty());

  YSOS_LOG_DEBUG("###### [BaseStrategyImpl::UnRegAgent] [before] ######" << agent_name);
  AutoLockOper lock(agent_map_lock_);
  YSOS_LOG_DEBUG("###### [BaseStrategyImpl::UnRegAgent] [after] ######" << agent_name);
  AgentServiceMap::iterator it = agent_map_.find(agent_name);
  if (agent_map_.end() != it) {
    YSOS_LOG_DEBUG("###### [BaseStrategyImpl::UnRegAgent][Stop] [before] ######");
    strategy_imp_->Stop();
    YSOS_LOG_DEBUG("###### [BaseStrategyImpl::UnRegAgent][Stop] [after] ######");
    agent_map_.erase(it);
  } else {
    YSOS_LOG_DEBUG("###### [BaseStrategyImpl::UnRegAgent] [End] ######");
    return YSOS_ERROR_NOT_EXISTED;
  }

  YSOS_LOG_DEBUG("###### [BaseStrategyImpl::UnRegAgent] [End] ######");
  return YSOS_ERROR_SUCCESS;
}

int BaseStrategyImpl::GetState(UINT32 timeout, INT32 *state, LPVOID param) {
  //*state = (INT32)&cur_stat_;

  return YSOS_ERROR_SUCCESS;
}

MsgInterfacePtr BaseStrategyImpl::GetMessage(void) {
  AutoLockOper lock(msg_queue_lock_);
  return GetBufferUtility()->GetMessageFromQueue(msg_queue_);
}

MsgInterfacePtr BaseStrategyImpl::GetCmd(void) {
  AutoLockOper lock(cmd_queue_lock_);
  return GetBufferUtility()->GetMessageFromQueue(cmd_queue_);
}

int BaseStrategyImpl::ParseStrategyXml(const std::string &strategy_xml) {
  tinyxml2::XMLDocument doc;
  tinyxml2::XMLElement *element = GetXmlUtil()->GetRootElement(&doc, strategy_xml, std::string("strategy"));
  if (NULL == element) {
    YSOS_LOG_DEBUG("Parse strategy xml failed: strategy_xml");
    state_name_ = "";
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  //StrategyInfoPtr strategy_info = StrategyInfoPtr(new StrategyInfo());
  state_name_ = GetXmlUtil()->GetElementTextValue(element, "state");
  std::string mechanism_name = GetXmlUtil()->GetElementTextValue(element, "mechanism");
  if (mechanism_name.empty()) {
    YSOS_LOG_DEBUG("invalid mechanism");
    state_name_ = "";

    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  // check type
  StateMachineManager *state_manager_ptr = new StateMachineManager(this);
  if (NULL == state_manager_ptr) {
    return YSOS_ERROR_MEMORY_ALLOCATED_FAILED;
  }
  CHECK_FINISH(state_manager_ptr->Initialize());
  //TODO:add for linux
#ifdef _WIN32
  std::string conf_dir = GetPlatformInfo()->GetConfPath();
  std::string mechanism_full_path = conf_dir + "strategy\\" + mechanism_name;
  strategy_imp_ = ControlInterfacePtr(state_manager_ptr);
  strategy_imp_->Ioctl(StateMachineManager::CMD_MECHANISM, (void*)&mechanism_full_path);
#else
  std::string conf_dir = GetPlatformInfo()->GetConfPath();
  std::string mechanism_full_path = conf_dir + "strategy/" + mechanism_name;
  strategy_imp_ = ControlInterfacePtr(state_manager_ptr);
  strategy_imp_->Ioctl(StateMachineManager::CMD_MECHANISM, (void*)&mechanism_full_path);
#endif
  return YSOS_ERROR_SUCCESS;
}
}