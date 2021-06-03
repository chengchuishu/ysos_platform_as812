/**
 *@file StateMachineConfigParse.cpp
 *@brief Definition of StateMachineConfigParse
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// private header //  NOLINT
#include "../../../protect/include/sys_framework_package/statemachineconfigparse.h"
/// c headers //  NOLINT
#include <cassert>
/// boost headers //  NOLINT
#include <boost/algorithm/string.hpp>
/// ysos headers //  NOLINT
#include "../../../public/include/sys_interface_package/common.h"
#include "../../../public/include/sys_framework_package/templatemanager.h"
#include "../../../protect/include/sys_framework_package/manager_declare.h"
#include "../../../public/include/core_help_package/xmlutil.h"
#include "../../../public/include/core_help_package/utility.h"
#include "../../../public/include/sys_interface_package/ysos_enum.h"

namespace ysos {

////////////////////////////////////////////////////////////////////////////////////////////////
///////                 Parse Transition                                                  /////
//////////////////////////////////////////////////////////////////////////////////////////////
StateMachineConfigParsePtr  StateMachineConfigParse::s_utility_ = NULL;
boost::shared_ptr<LightLock> StateMachineConfigParse::s_utility_lock_ = boost::shared_ptr<LightLock>(new LightLock());
StateMachineConfigParse::StateMachineConfigParse(const std::string &strClassName /* =StateMachineConfigParse */): BaseInterfaceImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.framework.strategy");
}

StateMachineConfigParse::~StateMachineConfigParse() {

}

const StateMachineConfigParsePtr StateMachineConfigParse::Instance(void) {
  if (NULL == s_utility_) {
    AutoLockOper lock(s_utility_lock_.get());
    if (NULL == s_utility_) {
      s_utility_ = StateMachineConfigParsePtr(new StateMachineConfigParse());
    }
  }

  return s_utility_;
}

static void UpdateListItem(std::string &item) {
  item = item.substr(0, item.find_first_of('|'));
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.framework.strategy"), "             service name: " << item);
}

int StateMachineConfigParse::GetCmdAndCallback(const std::string &condition, std::string &service_event, std::string &callback_name) {
  std::list<std::string>   cmd_event_list;
  int ret = GetUtility()->SplitString(condition, "|", cmd_event_list);
  if (YSOS_ERROR_SUCCESS != ret) {
    return ret;
  }

  if (cmd_event_list.size() != 2) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }
  std::list<std::string>::iterator  it=cmd_event_list.begin();
  service_event = *it++;
  callback_name = *it;
  // callbac的Name可以为空
  if (service_event.empty()) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  return YSOS_ERROR_SUCCESS;
}

int StateMachineConfigParse::ParseServiceEventList(StringList &service_event_list, ServiceMap &service_event_map) {
  for (StringList::iterator it=service_event_list.begin(); it!=service_event_list.end(); ++it) {
    std::string service_event_name, callback_name;
    int ret = GetCmdAndCallback(*it, service_event_name, callback_name);
    if (YSOS_ERROR_SUCCESS != ret) {
      YSOS_LOG_DEBUG("ConstructCmdOrEvent: " << GetUtility()->ConvertDataFromUtf(*it, "gbk") << "failed");
      continue;
    }

    *it = service_event_name;
    CallbackInterfacePtr callback = GetCallbackInterfaceManager()->FindInterface(callback_name);
    if (callback_name.empty()) {
      YSOS_LOG_DEBUG("callback name is empty, event name: " << service_event_name);
      continue;
    }
    if (NULL == callback) {
      YSOS_LOG_DEBUG("Can't find callback: " << callback_name << " event name:" << service_event_name);
      continue;;
    }

    ServiceMap::iterator callback_it=service_event_map.find(service_event_name);
    if (callback_it == service_event_map.end()) {
      CallbackListPtr callback_list = boost::make_shared<CallbackList>();
      callback_list->AddObject(callback);
      service_event_map.insert(std::make_pair(service_event_name, callback_list));
    } else {
      callback_it->second->AddObject(callback);
    }
  }

  service_event_list.sort();
  service_event_list.unique();

  return YSOS_ERROR_SUCCESS;
}

TransitionPtr StateMachineConfigParse::ParseTransitionXml(tinyxml2::XMLElement *element) {
  TransitionPtr transition = TransitionPtr(new Transition());
  transition->name = GetXmlUtil()->GetElementTextValue(element, "name");
  transition->from_str = GetXmlUtil()->GetElementTextValue(element, "from");
  transition->to_str = GetXmlUtil()->GetElementTextValue(element, "to");

  ParseTransitionEventOrCommand(element->FirstChildElement("service"), transition->service_list, "service");
  ParseServiceEventList(transition->service_list, transition->service_map);
  ParseTransitionEventOrCommand(element->FirstChildElement("event"), transition->event_list, "event");
  ParseServiceEventList(transition->event_list, transition->event_map);
  transition->transition_info = ParseTransitionInfo(element->FirstChildElement("transition_info"));

  /// 根据transition->service_list，生成transition->service_ptr_list
  GetServiceInfoFromServiceName(transition->service_list, transition->service_info_list);

  //std::copy(transition->service_list.begin(), transition->service_list.end(), std::back_inserter(transition->service_name_list));
  //std::for_each(transition->service_name_list.begin(), transition->service_name_list.end(), UpdateListItem);

  return transition;
}

void StateMachineConfigParse::GetServiceInfoFromServiceName(StringList &service_list, ServiceList &service_info_list) {
  StringList::iterator service_it = service_list.begin();
  ServiceList::iterator service_info_it = service_info_list.begin();
  for (; service_list.end() != service_it; ++service_it) {
    PlatformService::ServiceInfoPtr service_info_ptr = GetPlatformService()->GetService(*service_it);
    if (NULL == service_info_ptr) {
      YSOS_LOG_ERROR("Get service: " << *service_it << " failed");
      continue;
    }

    service_info_it = std::find(service_info_list.begin(), service_info_list.end(), service_info_ptr);
    if (service_info_list.end() != service_info_it) {
      YSOS_LOG_ERROR(*service_it << " already exist");
      continue;
    }
    service_info_list.push_back(service_info_ptr);
  }
}

int StateMachineConfigParse::ParseTransitionXml(const std::string &transition_xml, TransitionList &transition_list) {
  tinyxml2::XMLDocument doc;
  tinyxml2::XMLElement *element = GetXmlUtil()->GetRootElement(&doc, transition_xml, "transition");
  while (NULL != element) {
    TransitionPtr transition = ParseTransitionXml(element);
    if (NULL != transition) {
      transition_list.push_back(transition);
    }
    element = element->NextSiblingElement("transition");
  }

  return YSOS_ERROR_SUCCESS;
}

TransitionInfoInterfacePtr StateMachineConfigParse::ParseTransitionInfo(tinyxml2::XMLElement *element) {
  if (NULL == element) {
    return NULL;
  }

  BaseTransitionInfoImpl *transition_info_ptr = BaseTransitionInfoImpl::CreateInstance();
  if (NULL == transition_info_ptr) {
    return NULL;
  }
  if (YSOS_ERROR_SUCCESS != transition_info_ptr->Initialize()) {
    return NULL;
  }
  TransitionInfoInterfacePtr transition_info = TransitionInfoInterfacePtr(transition_info_ptr);
  ///  //  NOLINT
  tinyxml2::XMLElement *child = element->FirstChildElement("callback");
  if (NULL != child) {
    std::string callback_name = GetXmlUtil()->GetElementTextValue(child);
    if (!callback_name.empty()) {
      CallbackInterfacePtr callback = GetCallbackInterfaceManager()->FindInterface(callback_name);
      YSOS_LOG_DEBUG(" find transition callback: " << callback_name);
      assert(NULL != callback);
      if (NULL != callback) {
        transition_info_ptr->SetProperty(PROP_TRANSITION_INFO_CALLBACK, &callback);
        return transition_info;
      } else {
        YSOS_LOG_DEBUG("Transition Callback not exist: " << callback_name);
      }
      return NULL;
    }
  }

  child = element->FirstChildElement();
  while (NULL != child) {
    if (0 == std::strcmp(child->Name(), "logic")) {
      std::string logic_name = GetXmlUtil()->GetElementTextValue(child);
      transition_info->SetProperty(PROP_LOGIC, &logic_name);
      YSOS_LOG_DEBUG("----    ParseTransitionInfo logic: " << GetUtility()->ConvertDataFromUtf(logic_name, "gbk"));
    } else if (0 == std::strcmp(child->Name(), "ioinfo")) {
      IoInfoPtr ioinfo_ptr = ParseIoInfoPtr(child);
      transition_info->SetProperty(PROP_IOINFO, &ioinfo_ptr);
      //YSOS_LOG_DEBUG( "----    ParseTransitionInfo ioinfo: " << GetUtility()->ConvertDataFromUtf(ioinfo_ptr->name, "gbk") );
    }

    child = child->NextSiblingElement();
  }

  return transition_info;
}

IoInfoPtr  StateMachineConfigParse::ParseIoInfoPtr(tinyxml2::XMLElement *element) {
  assert(NULL != element);
  IoInfoPtr ioinfo_ptr = boost::make_shared<IoInfo>();

  ioinfo_ptr->callback_name = GetXmlUtil()->GetElementTextValue(element, "callback");
  ioinfo_ptr->data_type = GetXmlUtil()->GetElementTextValue(element, "data_type");
  ioinfo_ptr->operator_type = GetXmlUtil()->GetElementTextValue(element, "operator");
  ioinfo_ptr->data = GetXmlUtil()->GetElementTextValue(element, "data");
//  ioinfo_ptr->data = GetUtility()->Utf8ToAscii(ioinfo_ptr->data);
  ioinfo_ptr->event_name = GetXmlUtil()->GetElementTextValue(element, "event_name");
  ioinfo_ptr->name = GetXmlUtil()->GetElementTextValue(element, "name");

  YSOS_LOG_DEBUG("----    ParseTransitionInfo ioinfo callback name: " << GetUtility()->ConvertDataFromUtf(ioinfo_ptr->callback_name, "gbk"));
  YSOS_LOG_DEBUG("----    ParseTransitionInfo ioinfo name: " << GetUtility()->ConvertDataFromUtf(ioinfo_ptr->name, "gbk"));
  YSOS_LOG_DEBUG("----    ParseTransitionInfo ioinfo data_type: " << GetUtility()->ConvertDataFromUtf(ioinfo_ptr->data_type, "gbk"));
  YSOS_LOG_DEBUG("----    ParseTransitionInfo ioinfo: operator_type: " << GetUtility()->ConvertDataFromUtf(ioinfo_ptr->operator_type, "gbk"));
  YSOS_LOG_DEBUG("----    ParseTransitionInfo ioinfo: data: " << GetUtility()->ConvertDataFromUtf(ioinfo_ptr->data, "gbk"));
  YSOS_LOG_DEBUG("----    ParseTransitionInfo ioinfo: event_name: " << GetUtility()->ConvertDataFromUtf(ioinfo_ptr->event_name, "gbk"));
  YSOS_LOG_DEBUG("----    ParseTransitionInfo ioinfo: name: " << GetUtility()->ConvertDataFromUtf(ioinfo_ptr->name, "gbk"));

  return ioinfo_ptr;
}

void StateMachineConfigParse::ParseTransitionEventOrCommand(tinyxml2::XMLElement *element, std::list<std::string> &cmd_event_list, const std::string &dest_name) {
  if (NULL == element) {
    YSOS_LOG_DEBUG("ParseTransitionEventOrCommand failed: "  << dest_name);
    return;
  }

  while (NULL != element) {
    std::string event = GetXmlUtil()->GetElementTextValue(element);
    if (!event.empty()) {
      cmd_event_list.push_back(event);
      YSOS_LOG_DEBUG("----    ParseTransitionEventOrCommand : " << GetUtility()->ConvertDataFromUtf(event, "gbk"));
    }
    element = element->NextSiblingElement(dest_name.c_str());
  }
}

void StateMachineConfigParse::ParseMechanismTransition(const tinyxml2::XMLElement *element, TransitionList &transition_list, const std::string &conf_dir) {
  if (NULL == element) {
    YSOS_LOG_DEBUG("Failed to parse mechanism transition ");
    return;
  }

  const tinyxml2::XMLElement *transition = element->FirstChildElement("transition");
  while (NULL != transition) {
#ifdef _WIN32
    std::string file_name = transition->Attribute("file");
    /*int ret = */ParseTransitionXml(GetXmlUtil()->GetXmlStringFromFile(conf_dir + "strategy\\" + file_name), transition_list);
    /*if (NULL != transition_ptr) {
      transition_list.push_back(transition_ptr);
    }*/
#else
   std::string file_name = transition->Attribute("file");
    /*int ret = */ParseTransitionXml(GetXmlUtil()->GetXmlStringFromFile(conf_dir + "strategy/" + file_name), transition_list);
    /*if (NULL != transition_ptr) {
      transition_list.push_back(transition_ptr);
    }*/
#endif
    transition = transition->NextSiblingElement("transition");
  }
}

void StateMachineConfigParse::ParseMechanismState(const tinyxml2::XMLElement *element, Mechanism::StateList &state_list) {
  std::string tnodename = "state"; //need update for linux
  ParseMechanismByNodeName(element, tnodename, state_list);
}

void StateMachineConfigParse::ParseMechanismEvent(const tinyxml2::XMLElement *element, Mechanism::TStringList &event_list) {
  std::string tnodename = "event"; //need update for linux
  ParseMechanismByNodeName(element, tnodename, event_list);
}

void StateMachineConfigParse::ParseMechanismCmd(const tinyxml2::XMLElement *element, Mechanism::TStringList &cmd_list) {
  std::string tnodename = "command"; //need update for linux
  ParseMechanismByNodeName(element, tnodename, cmd_list);
}
void StateMachineConfigParse::ParseMechanismByNodeName(const tinyxml2::XMLElement *element, std::string &node_name, Mechanism::TStringList &string_list) {
  if (NULL == element) {
    YSOS_LOG_DEBUG("Failed to parse mechanism stat ");
    return;
  }

  const tinyxml2::XMLElement *child = element->FirstChildElement(node_name.c_str());
  while (NULL != child) {
    std::string state = GetXmlUtil()->GetElementTextValue((tinyxml2::XMLElement*)child);
    if (!state.empty()) {
      string_list.push_back(state);
    }
    child = child->NextSiblingElement(node_name.c_str());
  }
}
}