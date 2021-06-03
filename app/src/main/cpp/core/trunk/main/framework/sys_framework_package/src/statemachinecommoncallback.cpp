/**
 *@file StateMachineCommonCallback.cpp
 *@brief 状态机通用Callback
 *@version 1.0
 *@author dhongqian
 *@date Created on: 2016-05-11 10:06:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// Self Header //  NOLINT
#include "../../../protect/include/sys_framework_package/statemachinecommoncallback.h"
/// STL Headers //  NOLINT
#include <sstream>
/// Ysos Headers //  NOLINT
#include "../../../public/include/core_help_package/utility.h"
#include "../../../protect/include/sys_framework_package/basestatemachineimpl.h"
#include "../../../protect/include/os_hal_package/msgimpl.h"
#include "../../../public/include/sys_interface_package/teamparamsinterface.h"
#include "../../../public/include/sys_interface_package/messagetype.h"

namespace ysos {
//DECLARE_PLUGIN_REGISTER_INTERFACE(StateMachineCommonCallback, CallbackInterface);
StateMachineCommonCallback::StateMachineCommonCallback(const std::string &strClassName):BaseCallbackImpl(strClassName) {
  // YSOS_LOG<< "StateMachineCommonCallback created:" << GetName() << std::endl;
}

StateMachineCommonCallback::~StateMachineCommonCallback() {

}

int StateMachineCommonCallback::Callback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, void *context) {
  YSOS_LOG_DEBUG ("TestCallback : Dealing with input buffer: " << GetName());
  MsgInterface  *msg_ptr = dynamic_cast<MsgInterface*>(input_buffer.get());
  CallbackInterface *callback_ptr = static_cast<CallbackInterface*>(context);
  if (NULL == msg_ptr || NULL == callback_ptr) {
    return BaseCallbackImpl::Callback(input_buffer, output_buffer, context);
  }

  uint32_t msg_id = msg_ptr->GetMessageID();
  BufferInterfacePtr msg_body = msg_ptr->GetMessageBody();

  uint8_t *data = GetBufferUtility()->GetBufferData(msg_body);
  int ret = YSOS_ERROR_SUCCESS;

  switch(msg_id) {
  case YSOS_AGENT_MESSAGE_REG_EVENT_REQ:
    {
      ret = callback_ptr->SetProperty(PROP_REG_SERVICE_EVENT_REQ, data);
      break;
    }
  case YSOS_AGENT_MESSAGE_UNREG_EVENT_REQ:
    {
      ret = callback_ptr->SetProperty(PROP_UN_REG_SERVICE_EVENT_REQ, data);
      break;
    }
  case YSOS_AGENT_MESSAGE_IOCTL_REQ:
    {
      ret = callback_ptr->SetProperty(PROP_IOCTL_REQ, data);
      break;
    }
  case YSOS_AGENT_MESSAGE_SWITCH_REQ:
    {
      ret = callback_ptr->SetProperty(PROP_SWITCH_REQ, data);
      return ret;
    }
  case YSOS_AGENT_MESSAGE_READY_RSQ:
    {
      ret = callback_ptr->SetProperty(PROP_READY_REQ, data);
      return ret;
    }
  default:
    break;
  }

  return ret;
}
}
