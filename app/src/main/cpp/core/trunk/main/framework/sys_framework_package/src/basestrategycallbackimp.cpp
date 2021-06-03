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
#include "../../../protect/include/sys_framework_package/basestrategycallbackimpl.h"

/// private headers
#include "../../../public/include/sys_interface_package/ysos_enum.h"
#include "../../../public/include/core_help_package/utility.h"
#include "../../../public/include/sys_interface_package/messageinterface.h"
#include "../../../public/include/sys_interface_package/messagehead.h"
#include "../../../public/include/sys_interface_package/messagetype.h"
#include "../../../public/include/os_hal_package/bufferutility.h"
#include "../../../protect/include/sys_framework_package/platforminfo.h"

namespace ysos {

//DECLARE_PLUGIN_REGISTER_INTERFACE(StrategyCallback, CallbackInterface);
BaseStrategyCallbackImpl::BaseStrategyCallbackImpl(const std::string &strClassName):BaseStrategyServiceCallbackImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.framework.strategy.callback");
}

BaseStrategyCallbackImpl::~BaseStrategyCallbackImpl() {

}

int BaseStrategyCallbackImpl::Callback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, void *context) {
  YSOS_LOG_DEBUG("BaseStrategyCallbackImpl : Dealing with input buffer: " << logic_name_);
  MsgInterface  *msg_ptr = dynamic_cast<MsgInterface*>(input_buffer.get());
  CallbackInterface *callback_ptr = static_cast<CallbackInterface*>(context);
  if (NULL == msg_ptr || NULL == callback_ptr) {
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
  return HandleMessage(lower_event_name, msg_buffer, callback_ptr);
}

int BaseStrategyCallbackImpl::HandleMessage(const std::string &event_name, BufferInterfacePtr input_buffer, CallbackInterface *context) {
  return YSOS_ERROR_SUCCESS;
}

std::string BaseStrategyCallbackImpl::GetCallbackNameByEventName(const std::string &event_name, CallbackInterface *context) {
  assert(NULL != context && !event_name.empty());

  std::string callback_name = event_name;
  context->GetProperty(PROP_GET_EVENT_CALLBACK, &callback_name);

  return callback_name;
}

int BaseStrategyCallbackImpl::DoEventNotifyService(const std::string &event_name, const std::string &callback_name, const std::string &content, CallbackInterface *context) {
  assert(NULL != context);

  ReqEventNotifyServiceParam event_notify_req;

  event_notify_req.callback_name = callback_name;
  event_notify_req.event_name = event_name;
  event_notify_req.data = content;

  return context->SetProperty(PROP_EVENT_NOTIFY_REQ, &event_notify_req);
}

int BaseStrategyCallbackImpl::DoSwitchNotifyService(const std::string &next_state_name, CallbackInterface *context) {
  assert(NULL != context);

  ReqSwitchNotifyServiceParam switch_notify_req;
  switch_notify_req.state_name = next_state_name;

  return context->SetProperty(PROP_SWITCH_NOTIFY_REQ, &switch_notify_req);
}

int BaseStrategyCallbackImpl::DoIoctlService(ReqIOCtlServiceParam &ioctl_req, CallbackInterface *context) {
  assert(NULL != context);
  std::string req_str;
  if (!ioctl_req.ToString(req_str)) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  return context->SetProperty(PROP_IOCTL_REQ, (void*)req_str.c_str());
}

int BaseStrategyCallbackImpl::DoRegEventService(ReqRegServiceEventServiceParam &reg_service_event_req, CallbackInterface *context) {
  assert(NULL != context);
  std::string req_str;
  if (!reg_service_event_req.ToString(req_str)) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  return context->SetProperty(PROP_REG_SERVICE_EVENT_REQ, (void*)req_str.c_str());
}

int BaseStrategyCallbackImpl::DoUnRegEventService(ReqUnregServiceEventServiceParam &un_reg_service_event_req, CallbackInterface *context) {
  assert(NULL != context);
  std::string req_str;
  if (!un_reg_service_event_req.ToString(req_str)) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  return context->SetProperty(PROP_UN_REG_SERVICE_EVENT_REQ, (void*)req_str.c_str());
}

int BaseStrategyCallbackImpl::DoReadyService(ReqReadyServiceParam &ready_req, CallbackInterface *context) {
  assert(NULL != context);
  std::string req_str;
  if (!ready_req.ToString(req_str)) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  return context->SetProperty(PROP_READY_REQ, (void*)req_str.c_str());
}

int BaseStrategyCallbackImpl::DoSwitchService(ReqSwitchServiceParam &switch_req, CallbackInterface *context) {
  assert(NULL != context);
  std::string req_str;
  if (!switch_req.ToString(req_str)) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  return context->SetProperty(PROP_SWITCH_REQ, (void*)req_str.c_str());
}
}
