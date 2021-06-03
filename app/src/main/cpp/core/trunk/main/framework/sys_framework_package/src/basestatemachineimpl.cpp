/**
 *@file BaseStateMachineImpl.cpp
 *@brief Definition of BaseStateMachineImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// Self Header //  NOLINT
#include "../../../protect/include/sys_framework_package/basestatemachineimpl.h"  // NOLINT
/// c headers //  NOLINT
#include <cassert>
/// boost headers //  NOLINT
#include <boost/algorithm/string.hpp>
/// ysos private headers //  NOLINT
#include "../../../public/include/sys_interface_package/common.h"
#include "../../../public/include/sys_interface_package/ysos_enum.h"
#include "../../../public/include/sys_framework_package/templatemanager.h"
#include "../../../protect/include/sys_framework_package/manager_declare.h"
#include "../../../protect/include/os_hal_package/streamimpl.h"
#include "../../../protect/include/core_help_package/bufferimpl.h"
#include "../../../protect/include/sys_framework_package/statemachineconfigparse.h"
#include "../../../protect/include/sys_framework_package/basestrategyimpl.h"
#include "../../../protect/include/os_hal_package/msgimpl.h"
#include "../../../public/include/core_help_package/xmlutil.h"
#include "../../../public/include/core_help_package/utility.h"
#include "../../../protect/include/core_help_package/propertytree.h"
#include "../../../protect/include/sys_framework_package/baserepeatermoduleimpl.h"
#include "../../../protect/include/sys_framework_package/platforminfo.h"
#include "../../../public/include/os_hal_package/bufferutility.h"

namespace ysos {
/*************************************************************************************************************
 *                      BaseStateMachineImpl                                                                     *
 *************************************************************************************************************/
BaseStateMachineImpl::BaseStateMachineImpl(const std::string& strClassName): BaseStrategyServiceCallbackImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.framework.strategy");
}

BaseStateMachineImpl:: ~BaseStateMachineImpl(void) {
}

int BaseStateMachineImpl::Initialize(void *param) {
  if (IsInitialized()) {
    return YSOS_ERROR_FINISH_ALREADY;
  }

  state_machine_lock_ = new StateMachineLock();
  cmd_table_lock_ = new StateMachineLock();
  event_table_lock_ = new StateMachineLock();
  cur_transition_list_lock_ = new StateMachineLock();
  cur_stat_ = 1;
  strategy_ptr_ = NULL; // 在创建的时候，就赋值
  state_manager_ptr_ = NULL;
  service_map_lock_ = new StateMachineLock();
  state_num_ = 1;  ///<  1是父状态的 //  NOLINT
  parent_state_name_id_ = 1;
  max_child_state_num_ = 1000;
  is_run_ = false;
  is_ready_ = false;
  is_changed_parent_ = false;

  if (NULL == state_machine_lock_ || NULL == cmd_table_lock_ || NULL == event_table_lock_ || NULL == service_map_lock_) {
    YSOS_LOG_DEBUG("Initialise failed");
    UnInitialize();
    return YSOS_ERROR_MEMORY_ALLOCATED_FAILED;
  }

  return BaseCallbackImpl::Initialize(param);
}

int BaseStateMachineImpl::UnInitialize(void *param) {
  if (!IsInitialized()) {
    return YSOS_ERROR_FINISH_ALREADY;
  }

  YSOS_DELETE(state_machine_lock_);
  YSOS_DELETE(cmd_table_lock_);
  YSOS_DELETE(event_table_lock_);
  YSOS_DELETE(cur_transition_list_lock_);
  strategy_ptr_ = NULL;  // 是从外面赋值的，不需要删除
  //state_list_.clear();
  //stat_transition_map_.clear();
  child_map_.clear();
  parent_ptr_ = NULL;
  strategy_ptr_ = NULL;
  state_manager_ptr_ = NULL;
  event_table_.clear();
  common_event_table_.clear();
  cmd_table_.clear();
  common_cmd_table_.clear();
  YSOS_DELETE(service_map_lock_);
  cur_transition_list_.clear();

  return BaseStrategyServiceCallbackImpl::UnInitialize(param);
}

void BaseStateMachineImpl::ClearStateEventAndCmd(void) {
  YSOS_LOG_DEBUG("Clear state event and cmd: " << cur_stat_);
  {
    AutoLockOper lock(cmd_table_lock_);
    cmd_table_.clear();
  }
  {
    AutoLockOper lock(event_table_lock_);
    event_table_.clear();
  }

  {
    AutoLockOper lock(service_map_lock_);
    reg_event_map_.clear();
    do_event_map_.clear();
    event_service_map_.clear();
  }

  if (NULL != parent_ptr_) {
    parent_ptr_->ClearStateEventAndCmd();
  }
}

int BaseStateMachineImpl::ExitState() {
  AutoLockOper lock(state_machine_lock_);
//   if (!is_run_) {
//     return YSOS_ERROR_SUCCESS;
//   }
  // 都调一次,有可能是由App直接切换的
  ClearStateEventAndCmd();
  RegisterTransitionInfoToRepeater(/*cur_stat_, cur_transition_list_, */false);

  if (NULL != parent_ptr_) {
    parent_ptr_->ExitState();
  }

  strategy_ptr_->Ioctl(CMD_CLEAR_MSG);
  strategy_ptr_->Ioctl(CMD_CLEAR_CMD);
  is_run_ = false;
  cur_stat_ = 0;
  is_changed_parent_ = false;

  std::string state_name = GetStateNameByID(cur_stat_);
  YSOS_LOG_DEBUG("Leave state: " << state_name << "success");

  return YSOS_ERROR_SUCCESS;
}

int BaseStateMachineImpl::Callback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, void *context) {
  bool cmd_ret=true, event_ret=true;
  while (cmd_ret || event_ret) {
    cmd_ret = HandleCommand();
    event_ret = HandleMessage();

    if (0 == cur_stat_ || is_changed_parent_) {
      break;
    }
  }

  return YSOS_ERROR_SUCCESS;
}

int BaseStateMachineImpl::SetProperty(int type_id, void *type) {
  int ret = YSOS_ERROR_SUCCESS;

  switch (type_id) {
  case PROP_PARENT: {
    BaseStateMachineImplPtr *state_machine_ptr = static_cast<BaseStateMachineImplPtr*>(type);
    assert(NULL != state_machine_ptr);
    parent_ptr_ = *state_machine_ptr;
  }
  break;
  case PROP_CHILD: {
    BaseStateMachineImplPtr *state_machine_ptr = static_cast<BaseStateMachineImplPtr*>(type);
    assert(NULL != state_machine_ptr);
    BaseStateMachineImplPtr child_ptr = *state_machine_ptr;
    ret = AddChild(child_ptr);
  }
  break;
  case PROP_PARENT_NAME: {
    std::string *parent_name_ptr = static_cast<std::string*>(type);
    assert(NULL != parent_name_ptr);
    ret = YSOS_ERROR_NOT_SUPPORTED;
  }
  break;
  case PROP_PARENT_STATE_NAME: {
    std::string *parent_state_name_ptr = static_cast<std::string*>(type);
    assert(NULL != parent_state_name_ptr);
    parent_state_name_ = *parent_state_name_ptr;
    parent_state_name_id_ = 1;
    if (!parent_state_name_.empty()) {
      state_name_id_map_.insert(std::make_pair(parent_state_name_, parent_state_name_id_));
      state_id_name_map_.insert(std::make_pair(parent_state_name_id_, parent_state_name_));
    }
  }
  break;
  case PROP_STRATEGY: {
    StrategyInterface* strategy_ptr = static_cast<StrategyInterface*>(type);
    assert(NULL != strategy_ptr);
    strategy_ptr_ = strategy_ptr;
  }
  break;
  case PROP_MANAGER: {
    StateMachineManager* state_manager_ptr = static_cast<StateMachineManager*>(type);
    assert(NULL != state_manager_ptr);
    state_manager_ptr_ = state_manager_ptr;
  }
  break;
  case PROP_MECHANISM: {
    Mechanism *mechanism_ptr = static_cast<Mechanism*>(type);
    assert(NULL != mechanism_ptr);
    state_machine_name_ = mechanism_ptr->name;
    std::copy(mechanism_ptr->stat_list.begin(), mechanism_ptr->stat_list.end(), std::back_inserter(state_list_));
    GenerateStateIdNameRelationship(state_list_);

    TransitionList transition_list = mechanism_ptr->transition_list;
    for (TransitionList::iterator it=transition_list.begin(); it!=transition_list.end(); ++it) {
      TransitionPtr transition_ptr = *it;
      YSOS_LOG_DEBUG("from_str: " << transition_ptr->from_str);
      assert(state_name_id_map_.end() != state_name_id_map_.find(transition_ptr->from_str));
      transition_ptr->from = state_name_id_map_[transition_ptr->from_str];
      if (state_name_id_map_.end() != state_name_id_map_.find(transition_ptr->to_str)) {
        transition_ptr->to = state_name_id_map_[transition_ptr->to_str];
      }

      stat_id_transition_map_.insert(std::make_pair(transition_ptr->from, transition_ptr));
    }
    //AutoLockOper lock(common_event_table_lock_);
    /// 获取状态机的通用事件及处理函数 //  NOLINT
    GetStateMachineConfigParse()->ParseServiceEventList(mechanism_ptr->event_list, common_event_table_);
    ret = ConstructTransitionService(transition_list);
  }
  break;
  case PROP_SWITCH_REQ: {
    std::string service_str = NULL == type ? "" : (const char*)type; //GetBufferFromBufferInterfacePtrPtr(type);
    ret = HandleSwitchService(service_str);
    break;
  }
  case PROP_REG_SERVICE_EVENT_REQ: {
    std::string service_str = NULL == type ? "" : (const char*)type; //GetBufferFromBufferInterfacePtrPtr(type);
    ret = HandleRegServiceEventService(service_str);
    break;
  }
  case PROP_UN_REG_SERVICE_EVENT_REQ: {
    std::string service_str = NULL == type ? "" : (const char*)type; //GetBufferFromBufferInterfacePtrPtr(type);
    ret = HandleUnregServiceEventService(service_str);
    break;
  }
  case PROP_READY_REQ: {
    std::string service_str = NULL == type ? "" : (const char*)type; //GetBufferFromBufferInterfacePtrPtr(type);
    ret = HandleReadyService(service_str);
    break;
  }
  case PROP_IOCTL_REQ: {
    std::string service_str = NULL == type ? "" : (const char*)type; //GetBufferFromBufferInterfacePtrPtr(type);
    ret = HandleIoctlService(service_str);
    break;
  }
  case PROP_SWITCH_REQ_WITHOUT_DISPATCH: {
    std::string service_str = NULL == type ? "" : (const char*)type; //GetBufferFromBufferInterfacePtrPtr(type);
    ret = HandleSwitchService(service_str);
    break;
  }
  case PROP_REG_SERVICE_EVENT_REQ_WITHOUT_DISPATCH: {
    std::string service_str = NULL == type ? "" : (const char*)type; //GetBufferFromBufferInterfacePtrPtr(type);
    ret = HandleRegServiceEventService(service_str, false);
    break;
  }
  case PROP_UN_REG_SERVICE_EVENT_REQ_WITHOUT_DISPATCH: {
    std::string service_str = NULL == type ? "" : (const char*)type; //GetBufferFromBufferInterfacePtrPtr(type);
    ret = HandleUnregServiceEventService(service_str, false);
    break;
  }
  case PROP_READY_REQ_WITHOUT_DISPATCH: {
    std::string service_str = NULL == type ? "" : (const char*)type; //GetBufferFromBufferInterfacePtrPtr(type);
    ret = HandleReadyService(service_str, false);
    break;
  }
  case PROP_IOCTL_REQ_WITHOUT_DISPATCH: {
    std::string service_str = NULL == type ? "" : (const char*)type; //GetBufferFromBufferInterfacePtrPtr(type);
    ret = HandleIoctlService(service_str, false);
    break;
  }
  case PROP_STATE: {
    StatType *state_id = static_cast<StatType*>(type);
    ret = ChangeStat(*state_id);
    break;
  }
  case PROP_EVENT_NOTIFY_REQ: {
    ReqEventNotifyServiceParam *event_notify_req_ptr = static_cast<ReqEventNotifyServiceParam*>(type);
    assert(NULL != event_notify_req_ptr);
    BaseAgentImpl *agent_ptr = GetAgentByName("");
    if (NULL == agent_ptr) {
      YSOS_LOG_DEBUG("agent has been unregistered");
      return YSOS_ERROR_SKIP;
    }

    std::string callback_name = GetAgentCallbackName(event_notify_req_ptr->event_name);
    if (callback_name.empty() || event_notify_req_ptr->callback_name != callback_name) {
      /*YSOS_LOG_ERROR("not support event notify: " << event_notify_req_ptr->event_name << " | " << event_notify_req_ptr->callback_name << " | " << event_notify_req_ptr->data);
      return YSOS_ERROR_SKIP;*/
    }
    YSOS_LOG_ERROR("do event notify: " << event_notify_req_ptr->event_name << " | " << event_notify_req_ptr->callback_name << " | " << event_notify_req_ptr->data);
    ret = agent_ptr->DoEventNotifyService(*event_notify_req_ptr);
    break;
  }
  case PROP_SWITCH_NOTIFY_REQ: {
    ReqSwitchNotifyServiceParam *switch_notify_req = static_cast<ReqSwitchNotifyServiceParam*>(type);
    assert(NULL != switch_notify_req);
    BaseAgentImpl *agent_ptr = GetAgentByName("");
    if (NULL == agent_ptr) {
      YSOS_LOG_DEBUG("agent has been unregistered");
      return YSOS_ERROR_SKIP;
    }

    /// 满足转换条件后，与状态相关的事件与命令，都不再处理 //  NOLINT
    RegisterTransitionInfoToRepeater(/*cur_stat_, cur_transition_list_, */false);
    ClearStateEventAndCmd();

    ret = agent_ptr->DoSwitchNotifyService(*switch_notify_req);
    break;
  }
  case PROP_STOP: {
    ExitState();
    break;
  }
  case PROP_MSG: {
    if (NULL != strategy_ptr_) {
      strategy_ptr_->SetProperty(PROP_MSG, type);
    }
    break;
  }
  default: {
    YSOS_LOG_DEBUG("Not supported SetProperty: " << type_id);
    ret = YSOS_ERROR_NOT_SUPPORTED;
  }
  break;
  }

  return ret;
}

int BaseStateMachineImpl::AddChild(BaseStateMachineImplPtr &child_state_machine) {
  if (NULL == child_state_machine) {
    YSOS_LOG_DEBUG("child state_machine is null ");
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  std::string child_state_name = child_state_machine->parent_state_name_ + "@" + child_state_machine->state_machine_name_;
  assert(state_name_id_map_.end() != state_name_id_map_.find(child_state_machine->parent_state_name_ + "@"));
  StatType cur_state_child_num = state_name_id_map_[child_state_machine->parent_state_name_ + "@"];
  if (state_name_id_map_.end() == state_name_id_map_.find(child_state_name)) {
    state_name_id_map_.insert(std::make_pair(child_state_name, ++cur_state_child_num));
    state_id_name_map_.insert(std::make_pair(cur_state_child_num, child_state_name));
    state_name_id_map_[child_state_machine->parent_state_name_ + "@"] = cur_state_child_num;
  }

  StatType child_state_id = state_name_id_map_[child_state_name];
  child_map_.insert(std::make_pair(child_state_id, child_state_machine));

  return YSOS_ERROR_SUCCESS;
}

void BaseStateMachineImpl::GenerateStateIdNameRelationship(StateList &state_list) {
  if (state_list.end() == std::find(state_list.begin(), state_list.end(), state_machine_name_)) {
    state_list.push_front(state_machine_name_);
  }

  for (StateList::iterator it=state_list.begin(); it!=state_list.end(); ++it) {
    assert(state_name_id_map_.end() == state_name_id_map_.find(*it));
    state_name_id_map_.insert(std::make_pair(*it, ++state_num_));
    state_id_name_map_.insert(std::make_pair(state_num_, *it));

    StatType cur_child_state_num = state_num_ * max_child_state_num_;
    state_name_id_map_.insert(std::make_pair(*it + "@", cur_child_state_num));
    state_id_name_map_.insert(std::make_pair(cur_child_state_num, *it + "@"));
  }
}

std::string BaseStateMachineImpl::GetBufferFromBufferInterfacePtrPtr(void *type) {
  BufferInterfacePtr *buffer_ptr_ptr = static_cast<BufferInterfacePtr*>(type);
  if (NULL == buffer_ptr_ptr) {
    return NULL;
  }

  BufferInterfacePtr buffer_ptr = *buffer_ptr_ptr;
  uint8_t *buffer = GetBufferUtility()->GetBufferData(buffer_ptr);

  return (NULL!=buffer ? (const char*)buffer : "");
}

int BaseStateMachineImpl::DoSwitchDispatch(const int &ret_code) {
  BaseAgentImpl *agent_ptr = GetAgentByName("");
  if (NULL == agent_ptr) {
    YSOS_LOG_DEBUG("agent has been unregistered");
    return YSOS_ERROR_SKIP;
  }
  ResSwitchServiceParam switch_rsp;
  switch_rsp.status_code = GetUtility()->ConvertToString(ret_code);
  switch_rsp.switch_service_state_unit.cur_state = GetCurActiveStateName();
  switch_rsp.description = ret_code  == YSOS_ERROR_SUCCESS ? "succeeded" : "failed";

  return agent_ptr->DoSwitchDispatch(switch_rsp);
}

int BaseStateMachineImpl::HandleSwitchService(const std::string &service_xml, bool if_need_dispatch) {
  if (service_xml.empty()) {
    YSOS_LOG_ERROR("request xml is empty");
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }
  YSOS_LOG_DEBUG("request xml: " << service_xml);

  ReqSwitchServiceParam switch_req;
  bool ret = switch_req.FromString(service_xml);

  if (!ret && if_need_dispatch) {
    YSOS_LOG_ERROR("parse service xml failed: " << ret);
    return DoSwitchDispatch(YSOS_ERROR_INVALID_ARGUMENTS);
  }
  int ret_code = CheckStatePath(switch_req.state_name); // ChangeStat(switch_req.state_name);
  YSOS_LOG_DEBUG("CheckStatePath is: " << ret_code);

  if (if_need_dispatch) {
    DoSwitchDispatch(ret_code);
  }

  return ret_code;
}

int BaseStateMachineImpl::DoRegServiceEventDispatch(const int &ret_code, const std::string &service_name) {
  BaseAgentImpl *agent_ptr = GetAgentByName("");
  if (NULL == agent_ptr) {
    YSOS_LOG_DEBUG("agent has been unregistered");
    return YSOS_ERROR_SKIP;
  }
  ResRegServiceEventServiceParam reg_service_event_rsp;
  reg_service_event_rsp.status_code = GetUtility()->ConvertToString(ret_code);
  reg_service_event_rsp.description = ret_code == YSOS_ERROR_SUCCESS ? "succeeded" : "failed";
  reg_service_event_rsp.details.push_back(std::make_pair("service_name", service_name));

  return agent_ptr->DoRegServiceEventDispatch(reg_service_event_rsp);
}

int BaseStateMachineImpl::HandleRegServiceEventService(const std::string &service_xml, bool if_need_dispatch) {
  ReqRegServiceEventServiceParam reg_service_event_req;
  bool ret = reg_service_event_req.FromString(service_xml);
  if (!ret && if_need_dispatch) {
    YSOS_LOG_ERROR("parse request xml failed: " << service_xml);
    return DoRegServiceEventDispatch(YSOS_ERROR_INVALID_ARGUMENTS, "");
  }

  RegEventMap *event_map = NULL;
  std::string flag = reg_service_event_req.flag;
  bool is_run_stop_service = false;
  if ("0" == flag || "2" == flag) {
    event_map = &reg_event_map_;
  } else {
    event_map = &do_event_map_;
  }

  if ("2" == flag || "3" == flag) {
    if (!reg_service_event_req.service_name.empty()) {
      is_run_stop_service = true;
    }
  }

  ServiceEventSpecUnits reg_event_vec = reg_service_event_req.service_event_spec_units;
  ServiceEventSpecUnits::iterator it = reg_event_vec.begin();
  AutoLockOper  lock(service_map_lock_);
  for (; it!=reg_event_vec.end(); ++it) {
    event_map->insert(std::make_pair(it->event_key, it->callback_name));
    YSOS_LOG_DEBUG("reg_event_map: " << it->event_key << ":" << it->callback_name);

    if (!is_run_stop_service) {
      continue;
    }

    if (event_service_map_.end() != event_service_map_.find(it->event_key)) {
      continue;
    }
    event_service_map_.insert(std::make_pair(it->event_key, reg_service_event_req.service_name));
  }

  if (is_run_stop_service) {
    RunService(reg_service_event_req.service_name);
  }

  if (if_need_dispatch) {
    DoRegServiceEventDispatch(YSOS_ERROR_SUCCESS, reg_service_event_req.service_name);
  }

  return YSOS_ERROR_SUCCESS;
}

int BaseStateMachineImpl::DoUnregServiceEventDispatch(const int &ret_code) {
  ResUnregServiceEventServiceParam un_reg_service_event_rsp;
  un_reg_service_event_rsp.status_code = GetUtility()->ConvertToString(ret_code);
  un_reg_service_event_rsp.description = ret_code  == YSOS_ERROR_SUCCESS ? "succeeded" : "failed";
  BaseAgentImpl *agent_ptr = GetAgentByName("");
  if (NULL == agent_ptr) {
    YSOS_LOG_DEBUG("agent has been unregistered");
    return YSOS_ERROR_SKIP;
  }

  return agent_ptr->DoUnregServiceEventDispatch(un_reg_service_event_rsp);
}

int BaseStateMachineImpl::HandleUnregServiceEventService(const std::string &service_xml, bool if_need_dispatch) {
  if (service_xml.empty()) {
    YSOS_LOG_DEBUG("request message is empty");
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }
  ReqUnregServiceEventServiceParam un_reg_service_event_req;
  bool ret = un_reg_service_event_req.FromString(service_xml);

  if (!ret && if_need_dispatch) {
    YSOS_LOG_ERROR("parse service xml failed: " << ret);
    return DoUnregServiceEventDispatch(YSOS_ERROR_INVALID_ARGUMENTS);
  }

  /*RegEventMap *event_map = NULL;*/
  ServiceEventSpecUnits reg_event_vec = un_reg_service_event_req.service_event_spec_units;
  ServiceEventSpecUnits::iterator it = reg_event_vec.begin();
  AutoLockOper  lock(service_map_lock_);
  for (; it!=reg_event_vec.end(); ++it) {
    RegEventMap::iterator event_it=do_event_map_.find(it->event_key);
    if (event_it != do_event_map_.end()) {
      do_event_map_.erase(event_it);
      YSOS_LOG_DEBUG("do event map erase: " << event_it->first << "|" << event_it->second);
    } else {
      event_it = reg_event_map_.find(it->event_key);
      if (event_it != reg_event_map_.end()) {
        reg_event_map_.erase(event_it);
        YSOS_LOG_DEBUG("reg event map erase: " << event_it->first << "|" << event_it->second);
      }
    }

    RegEventMap::iterator event_service_it = event_service_map_.find(it->event_key);
    if (event_service_map_.end() != event_service_it) {
      event_service_map_.erase(event_service_it);
      StopService(event_service_it->second);
      YSOS_LOG_DEBUG("event service map erase and stop: " << event_it->first << "|" << event_it->second);
    }
    /*   event_map->insert(std::make_pair(it->event_key, it->callback_name));*/
    YSOS_LOG_DEBUG("reg_event_map: " << it->event_key << ":" << it->event_key);
  }

  if (if_need_dispatch) {
    DoUnregServiceEventDispatch(YSOS_ERROR_SUCCESS);
  }

  return YSOS_ERROR_SUCCESS;
}

int BaseStateMachineImpl::DoReadyDispatch(void) {
  BaseAgentImpl *agent_ptr = GetAgentByName("");
  if (NULL == agent_ptr) {
    YSOS_LOG_DEBUG("agent has been unregistered");
    return YSOS_ERROR_SKIP;
  }

  ResReadyServiceParam ready_rsp;
  ready_rsp.status_code = "0";
  ready_rsp.description = "succeeded";

  return agent_ptr->DoReadyDispatch(ready_rsp);
}

int BaseStateMachineImpl::HandleReadyService(const std::string &service_xml, bool if_need_dispatch) {
  YSOS_LOG_DEBUG("handle ready service");

  if (is_changed_parent_) {
    YSOS_LOG_ERROR("error!!!, is parent state machine: " << state_machine_name_);
    return YSOS_ERROR_FAILED;
  }

  RegisterTransitionInfoToRepeater(/*cur_stat_, cur_transition_list_, */true);

  if (if_need_dispatch) {
    DoReadyDispatch();
  }

  return YSOS_ERROR_SUCCESS;
}

int BaseStateMachineImpl::HandleIoctlService(const std::string &service_xml, bool if_need_dispatch) {
  ReqIOCtlServiceParam ioctl_req;
  bool ret = ioctl_req.FromString(service_xml);
  if (!ret && if_need_dispatch) {
    YSOS_LOG_ERROR("parse service xml failed: " << ret << ":" << service_xml);
    return DoIOCtlDispatch(ret, ioctl_req);
  }

  return HandleIoctlService(ioctl_req, if_need_dispatch);
}

int BaseStateMachineImpl::DoIOCtlDispatch(const int &ret_code, const ReqIOCtlServiceParam &service_info) {
  ResIOCtlServiceParam ioctl_resp;
  ioctl_resp.status_code = GetUtility()->ConvertToString(ret_code);
  ioctl_resp.description = ret_code == YSOS_ERROR_SUCCESS ? "succeeded" : "failed";
  ioctl_resp.details.push_back(std::make_pair("service_name", service_info.service_name));
  ioctl_resp.details.push_back(std::make_pair("id", service_info.id));
  BaseAgentImpl *agent_ptr = GetAgentByName("");
  if (NULL == agent_ptr) {
    YSOS_LOG_DEBUG("agent has been unregistered");
    return YSOS_ERROR_SKIP;
  }

  return agent_ptr->DoIOCtlDispatch(ioctl_resp);
}

int BaseStateMachineImpl::HandleIoctlService(const ReqIOCtlServiceParam &service_info, bool if_need_dispatch) {
  int ret = YSOS_ERROR_SUCCESS;

  RepeaterInfoPtr repeater_info_ptr = GetRepeaterByServiceName(service_info.service_name);
  if (NULL == repeater_info_ptr) {
    ret = YSOS_ERROR_NOT_EXISTED;
    YSOS_LOG_DEBUG("get repeater failed: " << service_info.service_name);
    if (if_need_dispatch) {
      DoIOCtlDispatch(ret, service_info);
    }
    return ret;
  }

  PropertyInfo property_info;
  property_info.id = GetUtility()->ConvertFromString(service_info.id, ret);
  property_info.str_id = repeater_info_ptr->real_name;// GetPlatformService()->GetServiceRealName(service_info.service_name);
  /// only support text type
  property_info.param = (void*)&(service_info.value);

  RepeaterInterfacePtr repeater_ptr = repeater_info_ptr->repeater_ptr;
  YSOS_LOG_DEBUG("repeater invoke ioctl before: " << property_info.str_id << ":" << service_info.value << ":" << ret);
  ret = repeater_ptr->Ioctl(CMD_CONTROL_MODULE, &property_info);
  YSOS_LOG_DEBUG("repeater invoke ioctl after: " << property_info.str_id << ":" << service_info.value << ":" << ret);

  if (if_need_dispatch) {
    DoIOCtlDispatch(ret, service_info);
  }

  return ret;
}

int BaseStateMachineImpl::GetProperty(int type_id, void *type) {
  int ret = YSOS_ERROR_SUCCESS;

  switch (type_id) {
  /*case AGENT_DISPATCH: {
    ret = YSOS_ERROR_NOT_SUPPORTED;
  }
  break;*/
  case PROP_CURRENT_STATE_NAME: {
    std::string *state_name = static_cast<std::string*>(type);
    assert(NULL != state_name);
    *state_name = GetCurStateName(true);
  }
  break;
  case PROP_GET_EVENT_CALLBACK: {
    std::string *event_name = static_cast<std::string*>(type);
    assert(NULL != event_name && !event_name->empty());
    *event_name = GetAgentCallbackName(*event_name);
    break;
  }
  default:
    ret = YSOS_ERROR_NOT_SUPPORTED;
    break;
  }

  return ret;
}

int BaseStateMachineImpl::HandleMessageAndCmd(ServiceMap &cmd_msg_map, const MsgInterfacePtr &cmd_msg, std::string &event_cmd_name,
    StateMachineLock *lock) {
  YSOS_LOG_DEBUG("cmd/event name : " << event_cmd_name);
  int is_handled = 0;
  ServiceMap::iterator it = cmd_msg_map.end();

  {
    AutoLockOper auto_lock(lock);
    it = cmd_msg_map.find(event_cmd_name);
  }
  if (it != cmd_msg_map.end()) {
    YSOS_LOG_DEBUG("Handle " << event_cmd_name << " callback");
    CallbackInterface *callback_ptr = this;
    it->second->Notify(cmd_msg, NULL, callback_ptr);
    is_handled |= 1;
    is_handled <<= 1;
  }

  return is_handled;
}

int BaseStateMachineImpl::HandleCommand(const MsgInterfacePtr cmd) {
  if (NULL == cmd) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

//   MessageHeadPtr msg_head = GetBufferUtility()->GetMessageHead(cmd);
//   assert(NULL != msg_head);
  uint32_t msg_id = cmd->GetMessageID();
  std::string cmd_name = PlatformInfo::Instance()->GetMessageNameById(msg_id);

  int is_handled = HandleMessageAndCmd(cmd_table_, cmd, cmd_name, cmd_table_lock_);
  if (0 == is_handled) {
    is_handled = HandleMessageAndCmd(common_cmd_table_, cmd, cmd_name, cmd_table_lock_);
  }

  if (NULL != parent_ptr_ && 0 == is_handled) {
    is_handled = parent_ptr_->HandleCommand(cmd);
  }

  if (0 == is_handled) {
    YSOS_LOG_DEBUG(this->GetName() << ": no support event ");
  }

  return is_handled;
}

bool BaseStateMachineImpl::HandleCommand() {
  MsgInterfacePtr cmd = GetCmd();
  if (NULL == cmd) {
    return false;
  }
  HandleCommand(cmd);

  return true;
}

int BaseStateMachineImpl::DoSwitchNotifyService(const std::string &next_stat) {
  BaseAgentImpl *agent_ptr = GetAgentByName("");
  if (NULL == agent_ptr) {
    YSOS_LOG_DEBUG("agent has been unregistered");
    return YSOS_ERROR_SKIP;
  }

  ReqSwitchNotifyServiceParam switch_notify_req;
  switch_notify_req.state_name = next_stat;

  YSOS_LOG_DEBUG("DoSwitchNotifyService: " << switch_notify_req.state_name);
  return agent_ptr->DoSwitchNotifyService(switch_notify_req);
}

int BaseStateMachineImpl::CheckStateChangeCondition(const MsgInterfacePtr msg) {
  int ret = YSOS_ERROR_SUCCESS;

  bool is_match_switch_ = false;
  TransitionPtr transition_ptr;
  {
    AutoLockOper lock(cur_transition_list_lock_);
    for (TransitionList::iterator it=cur_transition_list_.begin(); it!=cur_transition_list_.end(); ++it) {
      transition_ptr = *it;

      assert(NULL != transition_ptr);
      if (transition_ptr->to_str.empty()) {
        continue;
      }
      assert(NULL != transition_ptr->transition_info);
      TransitionInfoInterfacePtr transition_info_ptr = transition_ptr->transition_info;
      // test
      is_match_switch_ = transition_info_ptr->IsMatched(msg);
      if (!is_match_switch_) {
        continue;;
      }

      break;
    }
  }

  if (is_match_switch_) {
    YSOS_LOG_DEBUG("************* begin to switch from: " << transition_ptr->from_str << " to: " << transition_ptr->to_str);

    /// 满足转换条件后，与状态相关的事件与命令，都不再处理 //  NOLINT
    RegisterTransitionInfoToRepeater(/*cur_stat_, cur_transition_list_, */false);
    ClearStateEventAndCmd();
    ret = DoSwitchNotifyService(transition_ptr->to_str);
  }

  return ret;
}

BaseAgentImpl *BaseStateMachineImpl::GetAgentByName(const std::string &name) {
  assert(NULL != strategy_ptr_);
  AgentInterface *agent_ptr = NULL;
  if (YSOS_ERROR_SUCCESS != strategy_ptr_->GetProperty(PROP_AGENT, &agent_ptr)) {
    YSOS_LOG_DEBUG("get agent failed");
    return NULL;
  }

  return dynamic_cast<BaseAgentImpl*>(agent_ptr);
}

int BaseStateMachineImpl::DoEventNotifyService(const std::string &event_name, const std::string &callback_name, MsgInterfacePtr &msg_ptr) {
  ReqEventNotifyServiceParam event_notify_req;
  BaseAgentImpl *agent_ptr = GetAgentByName("");
  if (NULL == agent_ptr) {
    YSOS_LOG_DEBUG("agent has been unregistered");
    return YSOS_ERROR_SKIP;
  }

  event_notify_req.callback_name = callback_name;
  event_notify_req.event_name = event_name;

  BufferInterfacePtr msg_data = msg_ptr->GetMessageBody();
  if (NULL != msg_data) {
    char *buffer_data = (char*)GetBufferUtility()->GetBufferData(msg_data);
    if (NULL != buffer_data) {
      event_notify_req.data = buffer_data;
    }
  }

  YSOS_LOG_DEBUG("************** " << event_notify_req.event_name << " | " << event_notify_req.callback_name << " | " << event_notify_req.data);

  return agent_ptr->DoEventNotifyService(event_notify_req);
}

std::string BaseStateMachineImpl::GetAgentCallbackName(const std::string &event_name) {
  std::string callback_name;
  AutoLockOper lock(service_map_lock_);
  RegEventMap::iterator it = do_event_map_.find(event_name);
  if (do_event_map_.end() != it) {
    callback_name = do_event_map_[event_name];
    do_event_map_.erase(it);
    YSOS_LOG_DEBUG("do event map erase event: " << event_name);
    it = event_service_map_.find(event_name);
    if (event_service_map_.end() != it) {
      StopService(it->second);
      YSOS_LOG_DEBUG("event service map erase: " << event_name << " and stop service: " << it->second);
      event_service_map_.erase(it);
    }
  } else if (reg_event_map_.end() != reg_event_map_.find(event_name)) {
    callback_name = reg_event_map_[event_name];
    YSOS_LOG_DEBUG("get callback name from reg event map: " << event_name << " : " << callback_name);
  }

  return callback_name;
}

int BaseStateMachineImpl::HandleMessageByRegisterEvent(MsgInterfacePtr message_ptr) {
  assert(NULL != message_ptr);
//   MessageHeadPtr msg_head = GetBufferUtility()->GetMessageHead(message_ptr);
//   assert(NULL != msg_head);
  uint32_t msg_id = message_ptr->GetMessageID();
  std::string msg_name = PlatformInfo::Instance()->GetMessageNameById(msg_id);
  std::string callback_name = GetAgentCallbackName(msg_name);
  if (callback_name.empty()) {
    YSOS_LOG_DEBUG("*********************msg_name has not event callback name:" << msg_name);
    return YSOS_ERROR_NOT_SUPPORTED;
  }

  YSOS_LOG_DEBUG("*********************msg_name get event callback_name:" << msg_name << "callback_name: " << callback_name);

  return DoEventNotifyService(msg_name, callback_name, message_ptr);
}

int BaseStateMachineImpl::HandleMessage(const MsgInterfacePtr msg) {
  if (NULL == msg) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

//   MessageHeadPtr msg_head = GetBufferUtility()->GetMessageHead(msg);
//   assert(NULL != msg_head);
  uint32_t msg_id = msg->GetMessageID();
  std::string msg_name = PlatformInfo::Instance()->GetMessageNameById(msg_id);

  int is_handled = YSOS_ERROR_SUCCESS;
  if (YSOS_ERROR_SUCCESS == HandleMessageByRegisterEvent(msg)) {
    is_handled = 1;
  }

  if (0 == is_handled) {
    is_handled = HandleMessageAndCmd(event_table_, msg, msg_name, event_table_lock_);
  }

  if (0 == is_handled) {
    is_handled = HandleMessageAndCmd(common_event_table_, msg, msg_name, event_table_lock_);
  }

  if (NULL != parent_ptr_ && 0 == is_handled) {
    parent_ptr_->HandleMessage(msg);
    is_handled |= 1;
  }

  BaseAgentImpl *agent_ptr = GetAgentByName("");
  //assert(NULL != agent_ptr);
  if (NULL == agent_ptr) {
    YSOS_LOG_DEBUG("agent has been unregistered");
    return YSOS_ERROR_SKIP;
  }

  // 检查状态是否满足切换条件，如果满足，执行Dispatch，通知app切换
  YSOS_LOG_DEBUG("Begin to check change condition, message name is: " << msg_name);

  // boost::thread::sleep(boost::get_system_time() + boost::posix_time::millisec(50));
  boost::this_thread::sleep_for(boost::chrono::milliseconds(50));
  CheckStateChangeCondition(msg);

  return YSOS_ERROR_SUCCESS;
}

bool BaseStateMachineImpl::HandleMessage() {
  MsgInterfacePtr msg = GetMessage();
  if (NULL == msg) {
    return false;
  } else {
    HandleMessage(msg);
  }

  return true;
}

bool BaseStateMachineImpl::IsValidStat(const uint32_t &next_state) {
  bool is_valid = false;
  AutoLockOper lock(cur_transition_list_lock_);
  for (TransitionList::iterator it=cur_transition_list_.begin(); it!=cur_transition_list_.end(); ++it) {
    TransitionPtr transition_ptr = *it;

    if (NULL != transition_ptr) {
      if (transition_ptr->to == next_state) {
        is_valid = true;
        YSOS_LOG_DEBUG("state name: " << transition_ptr->to_str);
        break;
      }
    }
  }

  YSOS_LOG_DEBUG("check state valid: " << next_state << " : " << is_valid);

  return is_valid;
}

std::string BaseStateMachineImpl::GetCurStateName(bool is_full_name) {
  std::string parent_state_name;

//   if(NULL != parent_ptr_ && is_full_name) {
//     parent_state_name = parent_ptr_->GetCurStateName(true);
//   }

  if (!parent_state_name_.empty() && is_full_name) {
    parent_state_name  = parent_state_name_ + "@" + state_machine_name_ + "@";
  }

//   AutoLockOper lock(cur_transition_list_lock_);
//   assert(cur_transition_list_.size() > 0);
//   TransitionPtr transition_ptr = *(cur_transition_list_.begin());
//   std::string cur_state_name = transition_ptr->from_str;
  std::string cur_state_name = GetStateNameByID(cur_stat_);

  return (parent_state_name + cur_state_name);
}

std::string BaseStateMachineImpl::GetCurActiveStateName(void) {
  std::string state_name;
  state_manager_ptr_->Ioctl(StateMachineManager::CMD_GET_CURRENT_STATE_NAME, &state_name);

  return state_name;
}

int BaseStateMachineImpl::RealChangeStat(const uint32_t next_stat, bool is_chagne_parent) {
  if (2 == next_stat) { ///<  如果是状态机名，不需要切换transition，直接返回就可以 //  NOLINT
    cur_stat_ = next_stat;
    AutoLockOper lock(cur_transition_list_lock_);
    cur_transition_list_.clear();
    is_run_ = true;

    YSOS_LOG_DEBUG("Change to state " << next_stat << " success ");
    return YSOS_ERROR_SUCCESS;
  } else { // 如果不是状态机名，先切换进状态机名
    RealChangeStat(2);
  }

  if (stat_id_transition_map_.end() == stat_id_transition_map_.find(next_stat)) {
    /// 父状态可以没有Transition
    if (is_chagne_parent) {
      cur_stat_ = next_stat;

      return YSOS_ERROR_SUCCESS;
    }

    YSOS_LOG_DEBUG(" no transition for cur state: " << next_stat);
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }
  // 先进入父状态
  if (NULL != parent_ptr_) {
    parent_ptr_->ChangeStat(parent_state_name_, true);
  }

  cur_stat_ = next_stat;
  /// 父状态中的Transition和Transition中的事件，子状态不继承
  if (is_chagne_parent) {
    is_run_ = true;
    YSOS_LOG_DEBUG("Change to state " << next_stat << " success ");
    return YSOS_ERROR_SUCCESS;
  }

  AutoLockOper lock(cur_transition_list_lock_);
  cur_transition_list_.clear();
  GetTransitionByStateID(next_stat, cur_transition_list_);
  for (TransitionList::iterator it=cur_transition_list_.begin(); it!=cur_transition_list_.end(); ++it) {
    TransitionPtr transition_ptr = *it;
    {
      AutoLockOper lock(event_table_lock_);
      UpdateEventTable(transition_ptr->event_map, event_table_);
    }
    {
      AutoLockOper lock(cmd_table_lock_);
      UpdateCmdTable(transition_ptr->service_map, cmd_table_);
    }
  }

  is_run_ = true;
  YSOS_LOG_DEBUG("Change to state " << next_stat << " success ");
  return YSOS_ERROR_SUCCESS;
}

StatType BaseStateMachineImpl::GetStateIdByStateName(const std::string &state_name) {
  StatType state_id = 0;
  if (state_name_id_map_.end() != state_name_id_map_.find(state_name)) {
    state_id = state_name_id_map_[state_name];
  }

  return state_id;
}

int BaseStateMachineImpl::SplitStateName(const std::string &state_name, std::list<std::string> &state_list) {
  return GetUtility()->SplitString(state_name, "@", state_list);
}

int BaseStateMachineImpl::CheckStatePath(const std::string &state_name) {
  std::list<std::string> next_state_list;
  YSOS_LOG_DEBUG("state name is " << state_name);
  int ret = SplitStateName(state_name, next_state_list);
  assert(YSOS_ERROR_SUCCESS == ret);
  if (next_state_list.size() == 0) {
    YSOS_LOG_ERROR("state name is invalid: " << state_name);
    return YSOS_ERROR_NOT_SUPPORTED;
  }

  ret = CheckStatePath(next_state_list);
  if (YSOS_ERROR_SUCCESS != ret) {
    YSOS_LOG_ERROR("state name is invalid: " << state_name << " : " << ret);
    return ret;
  }

  return CheckStatePath(next_state_list, false);
}

int BaseStateMachineImpl::CheckStatePath(std::list<std::string> &state_list, bool checked) {
  int ret=YSOS_ERROR_NOT_SUPPORTED;
  bool is_state_name_already = false; ///<  是否已经与状态机名匹配过了 //  NOLINT
  std::string prev_state_name; // 只记录非状态机名的状态名
  assert(state_list.size()>0);

  for (std::list<std::string>::iterator it=state_list.begin(); it!=state_list.end(); ++it) {
    std::string state_name = (*it);
    // 不匹配当前状态机的状态
    if (state_list_.end() == std::find(state_list_.begin(), state_list_.end(), state_name)) {
      /// 是否是子状态机的状态要名
      if (!is_state_name_already && prev_state_name.empty()) { ///<  检查是否是当前活动状态的子状态机名 //  NOLINT
        prev_state_name = GetCurStateName();
      }
      if (prev_state_name.empty()) { ///<  说明不是当前状态机（含子状态机）中的状态 //  NOLINT
        ret = YSOS_ERROR_NOT_SUPPORTED;
        break;
      }

      state_name = prev_state_name + "@" + state_name; // 说明当前it指向的状态名是子状态机名
      std::list<std::string> sub_state_list(it, state_list.end());
      ret = CheckSubStatePath(state_name, sub_state_list, checked);
      if (YSOS_ERROR_SUCCESS == ret) {
        return ret;
      }
      break;
    } ///< end 不匹配当前状态机的状态

    if (state_name == state_machine_name_) { ///<  是状态机名 //  NOLINT
      // 两个状态机名，非法
      if (is_state_name_already) {
        ret = YSOS_ERROR_INVALID_ARGUMENTS; ///<  非法的状态名 //  NOLINT
        break;
      }
      ret = YSOS_ERROR_SUCCESS;
      is_state_name_already = true;
      continue;
    }
    // 只记录非状态机名的状态名
    prev_state_name = state_name;
    ret = YSOS_ERROR_SUCCESS;
  } // end for state_list

  if (YSOS_ERROR_SUCCESS == ret || YSOS_ERROR_INVALID_ARGUMENTS == ret) {
    if (YSOS_ERROR_SUCCESS == ret && !checked) {
      ChangeStat(prev_state_name.empty()?state_machine_name_:prev_state_name);
    }
    return ret;
  }

  return CheckParentStatePath(state_list, checked);
}

int BaseStateMachineImpl::CheckParentStatePath(std::list<std::string> &state_list, bool checked) {
  if (NULL == parent_ptr_) {
    return YSOS_ERROR_NOT_SUPPORTED;
  }

  if (!checked) ExitState();
  return parent_ptr_->CheckStatePath(state_list, checked);
}

BaseStateMachineImplPtr BaseStateMachineImpl::GetChildStateMachineByStateName(const std::string &state_name) {
  StatType child_state_id = GetStateIdByStateName(state_name);
  if (0 == child_state_id) {
    return NULL;
  }
  if (child_map_.end() == child_map_.find(child_state_id)) {
    return NULL;
  }
  // 获取子状态机的指针，开始检查子状态机
  BaseStateMachineImplPtr state_machine_ptr = child_map_[child_state_id];
  assert(NULL != state_machine_ptr);

  return state_machine_ptr;
}

int BaseStateMachineImpl::CheckCurStatePath(std::list<std::string> &state_list, bool checked) {
  return YSOS_ERROR_SUCCESS;
}

int BaseStateMachineImpl::CheckSubStatePath(std::string &state_name, std::list<std::string> &state_list, bool checked) {
  int ret = YSOS_ERROR_NOT_SUPPORTED;

  if (state_list_.end() == std::find(state_list_.begin(), state_list_.end(), state_name)) { ///< 说明是子状态机的状态
    BaseStateMachineImplPtr child_state_machine_ptr = GetChildStateMachineByStateName(state_name);
    if (NULL != child_state_machine_ptr) {
      ret = child_state_machine_ptr->CheckStatePath(state_list, checked); ///<  进入子状态机检查 //  NOLINT
    } else {
      ret = YSOS_ERROR_NOT_SUPPORTED;
    }
  }

  return ret;
}

int BaseStateMachineImpl::ChangeStat(const std::string &next_stat, bool is_chagne_parent) {
  //if (next_stat == parent_state_name_) {
  //  YSOS_LOG_DEBUG( "change to parent state: " << next_stat );
  //  // ExitState();
  //  return parent_ptr_->ChangeStat(next_stat, is_chagne_parent);
  //}

  StatType state_id = GetStateIdByStateName(next_stat);
  if (0 == state_id) {
    YSOS_LOG_DEBUG("invalid state name " << next_stat);
    return YSOS_ERROR_NOT_EXISTED;
  }

  /*if (cur_stat_ == state_id) {
  YSOS_LOG_DEBUG("cur state is already changed" << next_stat << ":" << state_id);
  return YSOS_ERROR_SUCCESS;
  }*/

  YSOS_LOG_DEBUG("change to state: " << next_stat << ":" << state_id);
  return ChangeStat(state_id, is_chagne_parent);
}

int BaseStateMachineImpl::ChangeStat(const uint32_t next_stat, bool is_chagne_parent) {
  // 切换状态前，当前状态要退出
  ExitState();

  AutoLockOper lock(state_machine_lock_);
  cur_stat_ = next_stat;
  if (2 == cur_stat_) { ///<  如果是状态机名的话，默认切换到第一个状态 //  NOLINT
    cur_stat_ = 3;  ///<  一个状态机里必须有一个状态 //  NOLINT
    /*return ChangeStat(cur_stat_);*/
  }

  BaseStateMachineImpl *state_machine_ptr = this;
  int ret = YSOS_ERROR_SUCCESS;
  if (!is_chagne_parent) {
    /*if( cur_stat_ == next_stat) {
      YSOS_LOG_DEBUG("cur state equal next stat: " << cur_stat_);
    } else {
      state_manager_ptr_->Ioctl(StateMachineManager::CMD_CHANGE_STATE_MACHINE, state_machine_ptr);
      assert(NULL != state_machine_ptr);
    }*/
    state_manager_ptr_->Ioctl(StateMachineManager::CMD_CHANGE_STATE_MACHINE, state_machine_ptr);
    assert(NULL != state_machine_ptr);
  } else {
    is_changed_parent_ = true;
  }

  return RealChangeStat(cur_stat_, is_changed_parent_);
}

int BaseStateMachineImpl::StopService(const std::string &service_name) {
  RepeaterInfoPtr repeater_info_ptr = GetRepeaterByServiceName(service_name);
  if (NULL == repeater_info_ptr) {
    YSOS_LOG_DEBUG("Get Repeater failed: " << service_name);
    return YSOS_ERROR_NOT_EXISTED;
  }

//   std::string real_service_name = GetPlatformService()->GetServiceRealName(service_name);
//   if(real_service_name.empty()) {
//     // composit、select暂时不支持
//     YSOS_LOG_DEBUG("Get real service name failed: " << service_name);
//     return YSOS_ERROR_INVALID_ARGUMENTS;
//     }

  PropertyInfo property_info;
  property_info.id = CMD_STOP;
  property_info.str_id = repeater_info_ptr->real_name;

  YSOS_LOG_DEBUG("***************************** begin to stop service: " << repeater_info_ptr->real_name);
  RepeaterInterfacePtr repeater_ptr = repeater_info_ptr->repeater_ptr;
  int ret = repeater_ptr->Ioctl(CMD_CONTROL_MODULE, &property_info);
  YSOS_LOG_DEBUG("***************************** end to stop service: " << repeater_info_ptr->real_name);
  YSOS_LOG_DEBUG("Stop service " << service_name << " : " << ret);

  return YSOS_ERROR_SUCCESS;
}

int BaseStateMachineImpl::RunService(const std::string &service_name) {
  RepeaterInfoPtr repeater_info_ptr = GetRepeaterByServiceName(service_name);
  if (NULL == repeater_info_ptr) {
    YSOS_LOG_DEBUG("Get Repeater failed: " << service_name);
    return YSOS_ERROR_NOT_EXISTED;
  }
//   std::string real_service_name = GetPlatformService()->GetServiceRealName(service_name);
//   if(real_service_name.empty()) {
//     // composit、select暂时不支持
//     YSOS_LOG_DEBUG("Get real service name failed: " << service_name);
//     return YSOS_ERROR_INVALID_ARGUMENTS;
//     }

  PropertyInfo property_info;
  property_info.id = CMD_RUN;
  property_info.str_id = service_name.empty() ? "" : std::string("|").append(repeater_info_ptr->real_name);

  RepeaterInterfacePtr repeater_ptr = repeater_info_ptr->repeater_ptr;
  YSOS_LOG_DEBUG("***************************** begin to run service: " << repeater_info_ptr->real_name);
  int ret = repeater_ptr->Ioctl(CMD_CONTROL_MODULE, &property_info);
  YSOS_LOG_DEBUG("***************************** end to run service: " << repeater_info_ptr->real_name);

  YSOS_LOG_DEBUG("Run service " << service_name << " : " << ret);

  return YSOS_ERROR_SUCCESS;
}

int BaseStateMachineImpl::RunService(const StringList &service_list) {
  StringList::const_iterator it=service_list.begin();
  for (; it!=service_list.end(); ++it) {
    YSOS_LOG_DEBUG("Run service: " << *it);
    RunService(*it);
  }

  return YSOS_ERROR_SUCCESS;
}

int BaseStateMachineImpl::StopService(const StringList &service_list) {
  StringList::const_iterator it=service_list.begin();
  for (; it!=service_list.end(); ++it) {
    YSOS_LOG_DEBUG("Stop service: " << *it);
    StopService(*it);
  }

  return YSOS_ERROR_SUCCESS;
}

int BaseStateMachineImpl::RegisterTransitionInfoToRepeater(/*const StatType &cur_state, TransitionList &transition_list, */bool is_register) {
  if (is_ready_ == is_register) {
    return YSOS_ERROR_SUCCESS;
  }

  std::string state_name = GetStateNameByID(cur_stat_);
  if (stat_id_transition_map_.end() == stat_id_transition_map_.find(cur_stat_)) {
    YSOS_LOG_DEBUG(" no repeater for cur state: " << state_name);
    return YSOS_ERROR_SUCCESS;
  }

  {
    AutoLockOper lock(cur_transition_list_lock_);
    for (TransitionList::iterator it=cur_transition_list_.begin(); it!=cur_transition_list_.end(); ++it) {
      TransitionPtr transition_ptr = *it;
      if (NULL == transition_ptr) {
        continue;;
      }

      RegisterTransitionInfoToRepeater(transition_ptr->repeater_list, transition_ptr, is_register);

      if (is_register) {
        RunService(transition_ptr->service_list);
      } else {
        StopService(transition_ptr->service_list);
      }

      if (transition_ptr->to_str.empty()) {
        continue;
      }
    }
  }

  is_ready_ = is_register;

  /// 父类的Transition子类不继承
//   if (NULL != parent_ptr_) {
//     parent_ptr_->RegisterTransitionInfoToRepeater(is_register);
//   }

  return YSOS_ERROR_SUCCESS;
}

int BaseStateMachineImpl::RegisterTransitionInfoToRepeater(RepeaterList &repleater_list, TransitionPtr transition_ptr, bool is_register) {
  //transition_info_ptr->SetProperty(is_register?BaseTransitionInfoImpl::TRANSITION_INFO_ADD:BaseTransitionInfoImpl::TRANSITION_INFO_REMOVE, &transition_info_ptr);
  for (std::list<ModuleInterfacePtr>::iterator it=repleater_list.begin(); it!=repleater_list.end(); ++it) {
    RepeaterInterfacePtr repeater_ptr = *it;
    repeater_ptr->Ioctl(is_register?CMD_TRANSITION_ADD:CMD_TRANSITION_REMOVE, &transition_ptr);
  }

  return YSOS_ERROR_SUCCESS;
}

std::string BaseStateMachineImpl::GetStateNameByID(const StatType state_id) {
  StateIDNameMap::iterator it = state_id_name_map_.find(state_id);
  if (state_id_name_map_.end() == it) {
    YSOS_LOG_ERROR("not exist state id: " << state_id << " statemachine name: " << state_machine_name_);
    return "";
  }

  return it->second;
}

int BaseStateMachineImpl::GetTransitionByStateID(const StatType &state_id, TransitionList &transition_list) {
  ///  transition是和状态机相联系的，当state_id不属于当前的状态机时，会为NULL //  NOLINT
  std::string state_name = GetStateNameByID(state_id);
  if (stat_id_transition_map_.end() == stat_id_transition_map_.find(state_id)) {
    if (is_changed_parent_) {
      YSOS_LOG_DEBUG(" no transition for parent state: " << state_name);
    } else {
      YSOS_LOG_DEBUG(" no transition for cur state: " << state_name);
    }

    return YSOS_ERROR_FAILED;
  }

  for (StateIdTransitionMap::iterator it=stat_id_transition_map_.begin(); it!=stat_id_transition_map_.end(); ++it) {
    if (it->first == state_id) {
      transition_list.push_back(it->second);
    }
  }

  return YSOS_ERROR_SUCCESS;
}

int BaseStateMachineImpl::UpdateEventTable(ServiceMap &event_map, ServiceMap &dst_event_map) {
  return UpdateCmdTable(event_map, dst_event_map);
}

int BaseStateMachineImpl::UpdateCmdTable(ServiceMap &cmd_map, ServiceMap &dst_cmd_map) {
  ServiceMap::iterator it=cmd_map.begin();
  for (; it!=cmd_map.end(); ++it) {
    if (dst_cmd_map.end() != dst_cmd_map.find(it->first)) {
      dst_cmd_map[it->first] = it->second;
      YSOS_LOG_DEBUG(" insert event: " << it->first);
    } else {
      dst_cmd_map.insert(std::make_pair(it->first, it->second));
      YSOS_LOG_DEBUG(" insert event: " << it->first);
    }
  }

  return YSOS_ERROR_SUCCESS;
}

MsgInterfacePtr BaseStateMachineImpl::GetMessage(void) {
  MsgInterfacePtr msg_ptr;
  int ret = strategy_ptr_->GetProperty(PROP_MSG, &msg_ptr);
  assert(YSOS_ERROR_SUCCESS == ret);

  return msg_ptr;
}

MsgInterfacePtr BaseStateMachineImpl::GetCmd(void) {
  MsgInterfacePtr cmd_ptr;
  int ret = strategy_ptr_->GetProperty(PROP_CMD, &cmd_ptr);
  assert(YSOS_ERROR_SUCCESS == ret);

  return cmd_ptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////      Construct Service                                                     /////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int BaseStateMachineImpl::ConstructTransitionService(TransitionList &transition_list) {
  for (TransitionList::iterator it=transition_list.begin(); it!=transition_list.end(); ++it) {
    // ServiceConfInfoPtr service_info = ServiceInfoTable::Instance()->GetServiceByName(*it);
    ConstructService((*it)->service_list, *it);
  }
  return YSOS_ERROR_SUCCESS;
}

int BaseStateMachineImpl::ConstructService(std::list<std::string> &service_name_list, TransitionPtr &transation_ptr) {
  for (std::list<std::string>::iterator it=service_name_list.begin(); it!=service_name_list.end(); ++it) {
    PlatformService::ServiceInfoPtr service_info = GetPlatformService()->GetService(*it);
    if (NULL == service_info) {
      YSOS_LOG_DEBUG("Get service failed: [" << *it << "] ");
      continue;
    }
    YSOS_LOG_DEBUG("construct service: " << *it);
    ConstructService(service_info, transation_ptr);
  }
  return YSOS_ERROR_SUCCESS;
}

int BaseStateMachineImpl::ConstructService(PlatformService::ServiceInfoPtr service_info, TransitionPtr &transation_ptr) {
  YSOS_LOG_DEBUG("BaseStateMachineImpl::ConstructService: " << service_info->name);
  int ret = YSOS_ERROR_SUCCESS;
  /// 状态机里不支持Callback类型的服务 //  NOLINT
  if (NULL == service_info || service_info->name.empty() || service_info->type == "callback") {
    YSOS_LOG_DEBUG("Invalid service: " << service_info->name << ": " << service_info->alias);
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  return RegisterService(service_info, transation_ptr);
}

int BaseStateMachineImpl::RegisterService(PlatformService::ServiceInfoPtr service_info, TransitionPtr &transation_ptr) {
  if (NULL == service_info) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  if ("module" == service_info->type) {
    return RegisterRepeaterByServiceName(service_info->name, transation_ptr);
  }
  if ("select" == service_info->type) {
    /// defaut有可能是别名 //  NOLINT
    PlatformService::ServiceInfoPtr service_info_ptr = GetPlatformService()->GetService(service_info->default_type);
    if (NULL == service_info_ptr) {
      return YSOS_ERROR_INVALID_ARGUMENTS;
    }
    return RegisterRepeaterByServiceName(service_info_ptr->name, transation_ptr);
  }
  if ("composit" != service_info->type) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  /// composit服务较复杂 //  NOLINT
  PlatformService::ServiceInfo::ServiceInfoList::iterator it=service_info->service_list.begin();
  for (; it!=service_info->service_list.end(); ++it) {
    PlatformService::ServiceInfoPtr service_info_ptr = GetPlatformService()->GetService(*it);
    if (NULL != service_info_ptr) {
      RegisterService(service_info_ptr, transation_ptr);
    }
  }

  return YSOS_ERROR_SUCCESS;
}

int BaseStateMachineImpl::RegisterRepeaterByServiceName(const std::string &service_name, TransitionPtr &transation_ptr) {
  RepeaterInfoPtr repeater_info_ptr = GetRepeaterByServiceName(service_name);
  if (NULL != repeater_info_ptr) {
    RepeaterInterfacePtr repeater_ptr = repeater_info_ptr->repeater_ptr;
    // add to transation_ptr
    // transation_ptr->service_repeater_map_
    RepeaterInterfaceListPtr repeater_list;
    ServiceRepeaterMap::iterator it = transation_ptr->service_repeater_map_.find(service_name);
    if (it == transation_ptr->service_repeater_map_.end()) {
      RepeaterInterfaceListPtr repeater_list = RepeaterInterfaceListPtr(new std::list<RepeaterInterfacePtr>());
      repeater_list->push_back(repeater_ptr);
      transation_ptr->service_repeater_map_.insert(std::make_pair(service_name, repeater_list));
    } else {
      repeater_list = transation_ptr->service_repeater_map_[service_name];
      repeater_list->push_back(repeater_ptr);
    }

    /// 添加到Repeater_List //  NOLINT
    RepeaterList::iterator list_it = std::find(transation_ptr->repeater_list.begin(), transation_ptr->repeater_list.end(), repeater_ptr);
    if (transation_ptr->repeater_list.end() == list_it) {
      transation_ptr->repeater_list.push_back(repeater_ptr);
    }
  }

  return YSOS_ERROR_SUCCESS;
}

//RepeaterInterfacePtr BaseStateMachineImpl::GetRepeaterByServiceName(const std::string &service_name) {
//  ModuleInterfacePtr module_ptr = GetModuleInterfaceManager()->FindInterface(service_name);
//  if (NULL == module_ptr) {
//    YSOS_LOG_DEBUG( "Find module failed: " << service_name );
//    return NULL;
//  }
//  std::string module_link_name;
//  int ret = module_ptr->GetProperty(PROP_MODULE_LINK, &module_link_name);
//  ModuleLinkInterfacePtr module_link_ptr = GetModuleLinkInterfaceManager()->FindInterface(module_link_name);
//  if (NULL == module_link_ptr) {
//    YSOS_LOG_DEBUG( "Get module link failed: " << module_link_name );
//    return NULL;
//  }
//
//  // get repeater
//  RepeaterIncludeModule  repeater_include_module;
//  repeater_include_module.module_name_ = service_name;
//  if (YSOS_ERROR_SUCCESS != module_link_ptr->GetProperty(PROP_MODULE_LINK_INFO_TYPE_EDGE_REPEATER, &repeater_include_module)) {
//    YSOS_LOG_DEBUG( "Get repeater failed:" << service_name );
//    return NULL;
//  }
//
//  if (repeater_include_module.repeater_ptr_list_.size()>0) {
//    return *(repeater_include_module.repeater_ptr_list_.begin());
//  } else {
//    YSOS_LOG_DEBUG( "Get repeater failed:" << service_name );
//  }
//  return NULL;
//}
}