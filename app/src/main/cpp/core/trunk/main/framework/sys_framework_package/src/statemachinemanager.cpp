/**
 *@file StateMachineManager.cpp
 *@brief Definition of StateMachineManager
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// ysos private headers //  NOLINT
#include "../../../protect/include/sys_framework_package/statemachinemanager.h"  // NOLINT
/// c headers //  NOLINT
#include <cassert>
/// stl headers
#include <algorithm>
/// boost headers //  NOLINT
#include <boost/algorithm/string.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread.hpp>
/// Ysos Headers //  NOLINT
#include "../../../protect/include/sys_framework_package/basestrategyimpl.h"  // NOLINT
#include "../../../public/include/core_help_package/xmlutil.h"
#include "../../../protect/include/os_hal_package/msgimpl.h"
#include "../../../public/include/os_hal_package/filepath.h"
#include "../../../protect/include/sys_framework_package/modulelinkimpl.h"
#include "../../../public/include/os_hal_package/bufferutility.h"
#include "../../../protect/include/sys_framework_package/statemachineconfigparse.h"
//#include "../../../protect/include/protocol_help_package/agent_protocol_utility.h"
#include "../../../protect/include/sys_framework_package/serviceinfotable.h"
#include "../../../protect/include/sys_framework_package/platforminfo.h"
#include "../../../public/include/sys_interface_package/ysos_enum.h"

namespace ysos {
#define YSOS_DEBUG 1
/*************************************************************************************************************
 *                      StateMachineManager                                                                     *
 *************************************************************************************************************/
StateMachineManager::StateMachineManager(StrategyInterface* strategy_ptr, const std::string &str): BaseInterfaceImpl(str) {
  logger_ = GetUtility()->GetLogger("ysos.framework.strategy");
  strategy_ptr_ = strategy_ptr;
}

StateMachineManager:: ~StateMachineManager(void) {
}

int StateMachineManager::Initialize(void *param) {
  is_run_ = false;
  //state_machine_manager_lock_ = new StateMachineManagerLock();
  //state_id_map_lock_ = new StateMachineManagerLock();
  //state_num_ = 0;
  default_state_id_ = 2;
  main_mechanism_ = "main";
  cur_thread_ = NULL;
  conf_dir_ = GetPlatformInfo()->GetConfPath();


  int ret = BaseInterfaceImpl::Initialize(param);

  cur_thread_ = new boost::thread(boost::bind(&StateMachineManager::HandleStrategy, this));
  assert(NULL != cur_thread_);

  return ret;
}

int StateMachineManager::UnInitialize(void *param) {
  free_mechanism_list_.clear();
  used_mechanism_list_.clear();
  //state_name_id_map_.clear();
  //state_id_name_map_.clear();
  cur_state_machine_ = NULL;
  for (BaseStateMachineImplPtrList::iterator it=state_machine_list_.begin(); it!=state_machine_list_.end(); ++it) {
    (*it)->UnInitialize();
  }
  //default_state_machine_ = NULL;

  //YSOS_DELETE(state_machine_manager_lock_);
  //YSOS_DELETE(state_id_map_lock_);

  int ret = BaseInterfaceImpl::UnInitialize(param);
  assert(NULL != cur_thread_);
  cur_thread_->join();

  YSOS_DELETE(cur_thread_);

  return ret;
}

int StateMachineManager::Ioctl(INT32 ctrl_id, LPVOID param) {
  int ret = YSOS_ERROR_SUCCESS;

  switch (ctrl_id) {
  case CMD_CHANGE_STATE_MACHINE: {
    BaseStateMachineImpl *state_machine_ptr = static_cast<BaseStateMachineImpl*>(param);
    assert(NULL != state_machine_ptr);
    BaseStateMachineImplPtrList::iterator it = std::find(state_machine_list_.begin(), state_machine_list_.end(), state_machine_ptr->shared_from_this());
    if (it != state_machine_list_.end()) {
      cur_state_machine_ = *it;
    } else {
      YSOS_LOG_DEBUG("Change state machine failed");
      ret = YSOS_ERROR_FAILED;
    }
    break;
  }
  case CMD_MECHANISM: {
    std::string *mechanism_xml_file_path = static_cast<std::string*>(param);
    if (NULL != mechanism_xml_file_path) {
      std::string mechanism_xml = GetUtility()->ReadAllDataFromFile(*mechanism_xml_file_path);
      ParseMechanismXml(mechanism_xml);
      ConstructStateMachineTree();
    }
  }
  break;
  case CMD_GET_SERVICE_LIST: {
    std::list<std::string> *service_list = static_cast<std::list<std::string>*>(param);
    assert(NULL != service_list);
    ret = GeServiceInfo(*service_list);
  }
  break;
  case CMD_GET_STATE_SWITCH_INFO: {
    std::multimap<std::string, std::string> *state_map = static_cast<std::multimap<std::string, std::string>*>(param);
    assert(NULL != state_map);
    ret = GeStateSwitchInfo(*state_map);
  }
  break;
  case CMD_GET_CURRENT_STATE_NAME: {
    assert(NULL != cur_state_machine_);
    ret = cur_state_machine_->GetProperty(PROP_CURRENT_STATE_NAME, param);
  }
  break;
  default:
    break;
  }

  return ret;
}

int StateMachineManager::GeStateSwitchInfo(std::multimap<std::string, std::string> &service_state_map) {
  for (MechanismPtrList::iterator it=used_mechanism_list_.begin(); it!=used_mechanism_list_.end(); ++it) {
    GeStateSwitchInfo((*it)->name, (*it)->transition_list, service_state_map);
  }
  return YSOS_ERROR_SUCCESS;
}

int StateMachineManager::GeStateSwitchInfo(std::string &state_machine_name, TransitionList &transition_list, std::multimap<std::string, std::string> &service_state_map) {
  for (TransitionList::iterator it=transition_list.begin(); it!=transition_list.end(); ++it) {
    service_state_map.insert(std::make_pair(state_machine_name, ((*it)->from_str + "-->" + (*it)->to_str)));
  }

  return YSOS_ERROR_SUCCESS;
}

int StateMachineManager::GeServiceInfo(std::list<std::string> &service_list) {
  for (MechanismPtrList::iterator it=used_mechanism_list_.begin(); it!=used_mechanism_list_.end(); ++it) {
    YSOS_LOG_DEBUG("Mechanism name: " << (*it)->name);
    GeServiceInfo((*it)->transition_list, service_list);
  }
  return YSOS_ERROR_SUCCESS;
}

int StateMachineManager::GeServiceInfo(TransitionList &transition_list, std::list<std::string> &service_list) {
  for (TransitionList::iterator it=transition_list.begin(); it!=transition_list.end(); ++it) {
    TransitionPtr transition_ptr = *it;
    std::copy(transition_ptr->service_list.begin(), transition_ptr->service_list.end(), std::back_inserter(service_list));
  }

  service_list.sort();
  service_list.unique();

  return YSOS_ERROR_SUCCESS;
}

int StateMachineManager::Stop(LPVOID param) {
  //AutoLockOper lock(state_machine_manager_lock_);
  if (!is_run_) {
    return YSOS_ERROR_SUCCESS;
  }

  is_run_ = false;

  if (NULL != cur_state_machine_) {
    cur_state_machine_->SetProperty(PROP_STOP, NULL);
  }
  /// YSOS_DELETE(cur_thread_);

  return YSOS_ERROR_SUCCESS;
}

int StateMachineManager::Pause(LPVOID param) {
  return YSOS_ERROR_SUCCESS;
}

int StateMachineManager::Run(LPVOID param) {
  if (is_run_) {
    YSOS_LOG_DEBUG("Strategy already run: " << strategy_ptr_->GetName());
    return YSOS_ERROR_SUCCESS;
  }

  // 使用第一个状态机
  //assert(state_num_ > 0);
  cur_state_machine_ = default_state_machine_;
  assert(NULL != cur_state_machine_);
  cur_state_machine_->SetProperty(PROP_STATE, &default_state_id_);

  is_run_ = true;

  return YSOS_ERROR_SUCCESS;
}

int StateMachineManager::Flush(LPVOID param) {
  return YSOS_ERROR_SUCCESS;
}

int StateMachineManager::GetState(UINT32 timeout, INT32 *state, LPVOID param) {
  return YSOS_ERROR_SUCCESS;
}

void StateMachineManager::HandleStrategy(void) {
  CallbackInterfacePtr active_machine = NULL;
  while (is_init_succeeded_) {
    if (is_run_) {
      {
        //AutoLockOper lock(strategy_lock_);
        active_machine = cur_state_machine_;
      }
      assert(NULL != active_machine);
      active_machine->Callback(NULL, NULL, this);
    }

    //boost::this_thread::sleep(boost::posix_time::millisec(1));
    boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
  }

  return ;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////      parse mechanism                                                       /////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int StateMachineManager::ParseMechanismXml(const std::string &mechanis_xml, bool is_first) {
  if (is_first) {
    free_mechanism_list_.clear();
  }

  tinyxml2::XMLDocument doc;
  tinyxml2::XMLElement *element = GetXmlUtil()->GetRootElement(&doc, mechanis_xml, "mechanism");
  while (NULL != element) {
    MechanismPtr mechanism = boost::make_shared<Mechanism>();
    mechanism->name = GetXmlUtil()->GetElementTextValue(element, "name");
    if (NULL != element->Attribute("type")) {
      mechanism->type = element->Attribute("type");
    }
    if (NULL != element->Attribute("file")) {
      mechanism->file = element->Attribute("file");
      ParseMechanismXml(GetXmlUtil()->GetXmlStringFromFile(mechanism->file), false);
    }

    GetStateMachineConfigParse()->ParseMechanismTransition(element, mechanism->transition_list, conf_dir_);

    for (TransitionList::iterator it=mechanism->transition_list.begin(); it!=mechanism->transition_list.end(); ++it) {
      TransitionInfoInterfacePtr transition_info_ptr = (*it)->transition_info;
      transition_info_ptr->SetProperty(PROP_STRATEGY, strategy_ptr_);
    }

    tinyxml2::XMLElement *parent_element = element->FirstChildElement("parent");
    if (NULL != parent_element) {
      mechanism->parent_state_machine = GetXmlUtil()->GetElementTextValue(parent_element, "name");
      mechanism->parent_state = GetXmlUtil()->GetElementTextValue(parent_element, "state");
    }

    GetStateMachineConfigParse()->ParseMechanismState(element, mechanism->stat_list);
    GetStateMachineConfigParse()->ParseMechanismEvent(element, mechanism->event_list);
    ///GetStateMachineConfigParse()->ParseMechanismCmd(element, mechanism->event_list);
    free_mechanism_list_.push_back(mechanism);

    element = element->NextSiblingElement("mechanism");
  }

  return YSOS_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////      Construct State Machine                                               /////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
MechanismPtr StateMachineManager::GetMechanismByName(const std::string &name, bool is_child) {
  MechanismPtr mechanism_ptr;
  for (std::list<MechanismPtr>::iterator it=free_mechanism_list_.begin(); it!=free_mechanism_list_.end(); ++it) {
    // find self
    if (!is_child  && (*it)->name == name) {
      mechanism_ptr = *it;
      free_mechanism_list_.erase(it);
      break;
    }
    // find child
    if (is_child && (*it)->parent_state_machine == name) {
      mechanism_ptr = *it;
      free_mechanism_list_.erase(it);
      break;
    }
  }

  return mechanism_ptr;
}

int StateMachineManager::ConstructStateMachineTree(void) {
  MechanismPtr mechanism_ptr = GetMechanismByName(main_mechanism_, false);
  if (NULL == mechanism_ptr) {
    YSOS_LOG_DEBUG("lack main stat");
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }
  default_state_machine_ = ConstructStateMachineTree(mechanism_ptr);

  if (free_mechanism_list_.size() > 0) {
    YSOS_LOG_DEBUG("error mechanism configure file");
  }

  return YSOS_ERROR_SUCCESS;
}

BaseStateMachineImplPtr StateMachineManager::ConstructStateMachineTree(MechanismPtr mechanism_ptr) {
  if (NULL == mechanism_ptr) {
    return NULL;
  }

  BaseStateMachineImplPtr cur_state_machine = ConstructStateMachine(mechanism_ptr);
  if (NULL == cur_state_machine) {
    YSOS_LOG_DEBUG("Construct state machine failed: " << GetUtility()->ConvertDataFromUtf(mechanism_ptr->name, "gbk"));
    return NULL;
  }
  used_mechanism_list_.push_back(mechanism_ptr);
  state_machine_list_.push_back(cur_state_machine);

  MechanismPtr child = GetMechanismByName(mechanism_ptr->name);
  while (NULL != child) {
    BaseStateMachineImplPtr child_state_machine = ConstructStateMachineTree(child);
    if (NULL == child_state_machine) {
      YSOS_LOG_DEBUG("Construct state machine failed: " << GetUtility()->ConvertDataFromUtf(mechanism_ptr->name, "gbk"));
    } else {
      child_state_machine->SetProperty(PROP_PARENT, &cur_state_machine);
      cur_state_machine->SetProperty(PROP_CHILD, &child_state_machine);
    }

    child = GetMechanismByName(mechanism_ptr->name);
  }

  return cur_state_machine;
}

BaseStateMachineImplPtr StateMachineManager::ConstructStateMachine(MechanismPtr mechanism_ptr) {
  assert(NULL != mechanism_ptr);
  BaseStateMachineImplPtr state_machine = BaseStateMachineImplPtr(BaseStateMachineImpl::CreateInstance());
  if (YSOS_ERROR_SUCCESS != state_machine->Initialize()) {
    return NULL;
  }
  state_machine->SetProperty(PROP_MANAGER, this);
  state_machine->SetProperty(PROP_STRATEGY, strategy_ptr_);
  state_machine->SetProperty(PROP_PARENT_NAME, &(mechanism_ptr->name));
  state_machine->SetProperty(PROP_PARENT_STATE_NAME, &(mechanism_ptr->parent_state));
  state_machine->SetProperty(PROP_MECHANISM, mechanism_ptr.get());
  YSOS_LOG_DEBUG("Construct statemachine[" << mechanism_ptr->name<<"]'s tranition");

  return state_machine;
}
}