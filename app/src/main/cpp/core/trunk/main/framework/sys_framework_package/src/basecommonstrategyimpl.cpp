/**
 *@file BaseCommmonStrategyImpl.cpp
 *@brief Definition of BaseCommmonStrategyImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// Self Header //  NOLINT
#include "../../../protect/include/sys_framework_package/basecommonstrategyimpl.h"  // NOLINT
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
#include "../../../public/include/os_hal_package/bufferutility.h"
#include "../../../protect/include/sys_framework_package/platforminfo.h"
#include "../../../public/include/sys_interface_package/ysos_enum.h"
#include "../../../protect/include/sys_framework_package/manager_declare.h"

namespace ysos {
#define YSOS_DEBUG 1
/*************************************************************************************************************
 *                      BaseStrategyImpl                                                                     *
 *************************************************************************************************************/
// DECLARE_PLUGIN_REGISTER_INTERFACE(BaseCommonStrategyImpl, StrategyInterface);

#define LOCAL_SERVICE_EVENT_TAG "local_service_event"
#define CLIENT_SERVICE_EVENT_TAG "client_service_event"
#define INTERNAL_SERVICE_EVENT_TAG "internal_service_event"
BaseCommonStrategyImpl::BaseCommonStrategyImpl(const std::string& strClassName): BaseInterfaceImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.framework.strategy");
}

BaseCommonStrategyImpl:: ~BaseCommonStrategyImpl(void) {

}

int BaseCommonStrategyImpl::Initialize(void *param) {
  msg_queue_lock_ = new StrategyLock();
  strategy_lock_ = new StrategyLock();
  msg_queue_ = MsgQueueInterfacePtr(MsgQueueImpl::CreateInstance());
  is_run_ = false;
  is_loaded_ = false;
  local_service_strategy_ = client_service_strategy_ = internal_service_strategy_ = NULL;
  cur_thread_ = NULL;
  ip_ = "";
  port_ = 0;

  if (NULL == msg_queue_lock_ || NULL == strategy_lock_ || NULL == msg_queue_) {
    UnInitialize();
    return YSOS_ERROR_MEMORY_ALLOCATED_FAILED;
  }

  LoadStrategy(GetName());

  cur_thread_ = new boost::thread(boost::bind(&BaseCommonStrategyImpl::HandleStrategy, this));
  assert(NULL != cur_thread_);

  return BaseInterfaceImpl::Initialize(param);
}

int BaseCommonStrategyImpl::UnInitialize(void *param) {
  int ret = BaseInterfaceImpl::UnInitialize(param);

  is_run_ = false;
  assert(NULL != cur_thread_);
  cur_thread_->join();

  YSOS_DELETE(msg_queue_lock_);
  YSOS_DELETE(strategy_lock_);

  return ret;
}

int BaseCommonStrategyImpl::LoadStrategy(const std::string &strategy_name) {
  std::string conf_dir = GetPlatformInfo()->GetConfPath();
#ifdef _WIN32
  std::string strategy_full_path = conf_dir + "strategy\\" + strategy_name + ".xml";
#else
  std::string strategy_full_path = conf_dir + "strategy/" + strategy_name + ".xml";
#endif
  std::string strategy_xml = GetUtility()->ReadAllDataFromFile(strategy_full_path);
  ParseStrategyXml(strategy_xml);
  is_loaded_ = true;

  return YSOS_ERROR_SUCCESS;
}

int BaseCommonStrategyImpl::CreateStrategy(const std::string &strategy_name, void *params) {
  return YSOS_ERROR_NOT_SUPPORTED;
}

int BaseCommonStrategyImpl::DestroyStrategy() {
  return YSOS_ERROR_NOT_SUPPORTED;
}

int BaseCommonStrategyImpl::SaveStrategy() {
  return YSOS_ERROR_NOT_SUPPORTED;
}

int BaseCommonStrategyImpl::Ioctl(INT32 ctrl_id, LPVOID param) {
  int ret = YSOS_ERROR_SUCCESS;

  return ret;
}

int BaseCommonStrategyImpl::Stop(LPVOID param) {
  AutoLockOper  lock(strategy_lock_);
  if (!is_run_) {
    YSOS_LOG_DEBUG("strategy already stopped: " << this->GetName());
    return YSOS_ERROR_SUCCESS;
  }

  is_run_ = false;

  return YSOS_ERROR_SUCCESS;
}

int BaseCommonStrategyImpl::Pause(LPVOID param) {
  return YSOS_ERROR_NOT_SUPPORTED;
}

int BaseCommonStrategyImpl::Run(LPVOID param) {
  AutoLockOper lock(strategy_lock_);
  if (is_run_) {
    YSOS_LOG_DEBUG("Strategy already run: " << this->GetName());
    return YSOS_ERROR_SUCCESS;
  }

  is_run_ = true;

  return YSOS_ERROR_SUCCESS;
}

int BaseCommonStrategyImpl::Flush(LPVOID param) {
  return YSOS_ERROR_SUCCESS;
}

int BaseCommonStrategyImpl::GetProperty(int type_id, void *type) {
  int ret = YSOS_ERROR_SUCCESS;

  switch (type_id) {
  case PROP_MSG: {
    break;
  }
  case PROP_STATE: {
    if (NULL == type) {
      ret = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }
    std::string* state_ptr = static_cast<std::string*>(type);
    *state_ptr = state_;
    break;
  }
  default:
    ret = BaseInterfaceImpl::GetProperty(type_id, type);
  }

  return ret;
}

int BaseCommonStrategyImpl::SetProperty(int type_id, void *type) {
  int ret = YSOS_ERROR_SUCCESS;

  switch (type_id) {
  case PROP_MSG: {
    AutoLockOper lock(msg_queue_lock_);
    MsgInterfacePtr *msg_ptr = static_cast<MsgInterfacePtr*>(type);
    MsgInterfacePtr msg = *msg_ptr;
    if (NULL != msg) {
      msg_queue_->Push(msg);
    }
    break;
  }
  case PROP_AGENT: {
    if (NULL == client_service_strategy_) {
      ret = YSOS_ERROR_NOT_SUPPORTED;
      break;
    }
    ret = client_service_strategy_->SetProperty(type_id, type);
    break;
  }
  default:
    ret = BaseInterfaceImpl::SetProperty(type_id, type);
  }

  return ret;
}

int BaseCommonStrategyImpl::GetState(UINT32 timeout, INT32 *state, LPVOID param) {
  return YSOS_ERROR_NOT_SUPPORTED;
}

MsgInterfacePtr BaseCommonStrategyImpl::GetMessage(void) {
  AutoLockOper lock(msg_queue_lock_);
  return GetBufferUtility()->GetMessageFromQueue(msg_queue_);
}

int BaseCommonStrategyImpl::ParseStrategyXml(const std::string &strategy_xml) {
  tinyxml2::XMLDocument doc;
  tinyxml2::XMLElement *element = GetXmlUtil()->GetRootElement(&doc, strategy_xml, std::string("strategy"));
  if (NULL == element) {
    YSOS_LOG_DEBUG("Parse strategy xml failed: " << strategy_xml.c_str());

    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  local_service_name_ = GetXmlUtil()->GetElementTextValue(element, "local_service");
  client_service_name_ = GetXmlUtil()->GetElementTextValue(element, "client_service");
  internal_service_name_ = GetXmlUtil()->GetElementTextValue(element, "internal_service");
  state_ = GetXmlUtil()->GetElementTextValue(element, "state");
  ip_ = GetXmlUtil()->GetElementTextValue(element, "ip");
  port_ = GetXmlUtil()->GetElementInt32Value(element, "port");

  if (!local_service_name_.empty()) {
    tinyxml2::XMLElement *child = element->FirstChildElement(LOCAL_SERVICE_EVENT_TAG);
    GetServiceEvent(LOCAL_SERVICE_EVENT_TAG, child, local_event_list_);
    local_service_strategy_ = GetCallbackInterfaceManager()->FindInterface(local_service_name_);
  }

  if (!client_service_name_.empty()) {
    tinyxml2::XMLElement *child = element->FirstChildElement(CLIENT_SERVICE_EVENT_TAG);
    GetServiceEvent(CLIENT_SERVICE_EVENT_TAG, child, client_event_list_);
    client_service_strategy_ = GetCallbackInterfaceManager()->FindInterface(client_service_name_);
  }

  if (!internal_service_name_.empty()) {
    tinyxml2::XMLElement *child = element->FirstChildElement(INTERNAL_SERVICE_EVENT_TAG);
    GetServiceEvent(INTERNAL_SERVICE_EVENT_TAG, child, internal_event_list_);
    internal_service_strategy_ = GetCallbackInterfaceManager()->FindInterface(internal_service_name_);
  }

  return YSOS_ERROR_SUCCESS;
}

void BaseCommonStrategyImpl::GetServiceEvent(const std::string &service_name_tag, tinyxml2::XMLElement *event_child, BaseCommonStrategyImpl::EventList &event_list) {
  if (NULL == event_child) {
    YSOS_LOG_ERROR("there is no event child for " << service_name_tag << " | " << logic_name_);
    return;
  }

  while (NULL != event_child) {
    std::string value = GetXmlUtil()->GetElementTextValue(event_child);
    if (!value.empty()) {
      event_list.push_back(value);
    }

    event_child = event_child->NextSiblingElement(service_name_tag.c_str());
  }
}

void BaseCommonStrategyImpl::HandleStrategy(void) {
  while (is_init_succeeded_) {
    boost::this_thread::sleep_for(boost::chrono::milliseconds(300));
    if (!is_run_) {
      continue;
    }

    MsgInterfacePtr msg_ptr = GetMessage();
    if (NULL == msg_ptr) {
      continue;
    }

    int ret = HandleMesssage(msg_ptr);
    if (YSOS_ERROR_SUCCESS != ret) {
      YSOS_LOG_ERROR("handle message failed: " << logic_name_);
    }
  } ///< end while
}

int BaseCommonStrategyImpl::HandleMesssage(MsgInterfacePtr msg_ptr) {
  assert(NULL != msg_ptr);
  uint32_t msg_id = msg_ptr->GetMessageID();
  BufferInterfacePtr buffer_ptr = msg_ptr->GetMessageBody();
  std::string msg_name = PlatformInfo::Instance()->GetMessageNameById(msg_id);
  if (msg_name.empty()) {
    YSOS_LOG_ERROR("not exist message id: " << msg_id << " | " << logic_name_);

    return YSOS_ERROR_NOT_EXISTED;
  }

  return HandleMesssage(msg_name.c_str(), msg_ptr, NULL);
}

int BaseCommonStrategyImpl::HandleMesssage(const char *event_name, const BufferInterfacePtr input_buffer, void *context_ptr) {
  if (NULL != local_service_strategy_) {
    bool is_matched = IfMatchedMesssage(event_name, local_event_list_);
    if (is_matched) {
      local_service_strategy_->Callback(input_buffer, NULL, NULL);
    }
  }

  if (NULL != client_service_strategy_) {
    bool is_matched = IfMatchedMesssage(event_name, client_event_list_);
    if (is_matched) {
      client_service_strategy_->Callback(input_buffer, NULL, NULL);
    }
  }

  if (NULL != internal_service_strategy_) {
    bool is_matched = IfMatchedMesssage(event_name, internal_event_list_);
    if (is_matched) {
      internal_service_strategy_->Callback(input_buffer, NULL, NULL);
    }
  }

  return YSOS_ERROR_SUCCESS;
}

bool BaseCommonStrategyImpl::IfMatchedMesssage(const char *event_name, BaseCommonStrategyImpl::EventList &event_list) {
  assert(NULL != event_name);
  EventList::iterator it = std::find(event_list.begin(), event_list.end(), event_name);

  return (event_list.end() != it);
}

}
