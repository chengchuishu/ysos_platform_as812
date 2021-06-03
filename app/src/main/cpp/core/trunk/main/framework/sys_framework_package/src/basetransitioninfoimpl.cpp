/**
 *@file BaseTransitionInfoImpl.cpp
 *@brief Definition of BaseTransitionInfoImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// Self Header
#include "../../../protect/include/sys_framework_package/basetransitioninfoimpl.h"
/// ysos private headers //  NOLINT
#include "../../../public/include/core_help_package/xmlutil.h"
#include "../../../protect/include/os_hal_package/msgimpl.h"
#include "../../../protect/include/sys_framework_package/basestrategyimpl.h"
#include "../../../protect/include/sys_framework_package/baserepeatermoduleimpl.h"
#include "../../../protect/include/sys_framework_package/platforminfo.h"
#include "../../../protect/include/sys_framework_package/ioinfofactory.h"
#include "../../../public/include/sys_framework_package/templatemanager.h"
#include "../../../protect/include/sys_framework_package/manager_declare.h"
#include "../../../protect/include/sys_framework_package/basemodulecallbackimpl.h"
/// c headers //  NOLINT
#include <cassert>

namespace ysos {
/*************************************************************************************************************
 *                      BaseTransitionInfoImpl                                                               *
 *************************************************************************************************************/
BaseTransitionInfoImpl::BaseTransitionInfoImpl(const std::string &strClassName /* =BaseTransitionInfoImpl */):
  BaseInterfaceImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.framework.strategy");
  is_matched_for_callback_ = false;
  matched_msg_id_ = 0;
}

BaseTransitionInfoImpl::~BaseTransitionInfoImpl() {
  UnInitialize();
}

int BaseTransitionInfoImpl::Initialize(void *param) {
  if (IsInitialized()) {
    return YSOS_ERROR_FINISH_ALREADY;
  }

  ioinfo_match_result_lock_ = new LightLock();
  if (NULL == ioinfo_match_result_lock_) {
    UnInitialize();
    return YSOS_ERROR_MEMORY_ALLOCATED_FAILED;
  }

  return BaseInterfaceImpl::Initialize(param);
}

int BaseTransitionInfoImpl::UnInitialize(void *param) {
  if (!IsInitialized()) {
    return YSOS_ERROR_FINISH_ALREADY;
  }

  logic_ioinfo_list_.clear();
  strategy_ptr_ = NULL;
  ioInfo_list_.clear();
  YSOS_DELETE(ioinfo_match_result_lock_);
  ioinfo_match_result_.clear();

  return BaseInterfaceImpl::UnInitialize(param);
}

TransitionContext BaseTransitionInfoImpl::GetTransitionContext(void *context_ptr) {
  TransitionContext context;
  if (NULL == context_ptr) {
    return context;
  }

  BaseModuelCallbackContext *module_callback_contex = reinterpret_cast<BaseModuelCallbackContext*>(context_ptr);
  context.module_ptr = module_callback_contex->prev_module_ptr;
  context.callback_ptr = module_callback_contex->prev_callback_ptr;
  context.transition_ptr = this;

  return context;
}

int BaseTransitionInfoImpl::SetSwitchFlag(const std::string &module_name, const std::string &data_type) {
  is_matched_for_callback_ = true;
  matched_msg_id_ = GetPlatformInfo()->GetMessageIdByName(data_type);
  YSOS_LOG_DEBUG("get message " << data_type << " id: " << matched_msg_id_);

  return YSOS_ERROR_SUCCESS;
}

int BaseTransitionInfoImpl::Filter(BufferInterfacePtr input_ptr, BufferInterfacePtr output_ptr, void *context_ptr) {
  /*if (YSOS_ERROR_SUCCESS != IsMatched(input_ptr, output_ptr, context_ptr)) {
    return YSOS_ERROR_NOT_SUPPORTED;
  }*/

  ModuleInterface *module_ptr = static_cast<ModuleInterface*>(context_ptr);
  if (NULL == module_ptr) {
    return YSOS_ERROR_NOT_SUPPORTED;
  }
  /// 如果有callback,直接调用Callback
  if (NULL != callback_ptr_) {
    TransitionContext context = GetTransitionContext(context_ptr);
    return callback_ptr_->Callback(input_ptr, output_ptr, &context);
  }

  for (IOInfoList::iterator it=ioInfo_list_.begin(); it!=ioInfo_list_.end(); ++it) {
    IoInfoInterfacePtr ioinfo_ptr = *it;
    /*if (module_ptr->GetName() != ioinfo_ptr->GetName()) {
      continue;
    }*/
    if (!GetIoinfoUtility()->IsNameMatch(module_ptr->GetName(), ioinfo_ptr->GetName())) {
      continue;
    }
    //YSOS_LOG<<" module_name: " << module_ptr->GetName() << " ioinfo_name: " << ioinfo_ptr->GetName() << std::endl;
    if (ioinfo_ptr->IsMatched(input_ptr, output_ptr, context_ptr)) {
      //ioinfo_ptr->Wrap(input_ptr, NULL, this);
      AutoLockOper lock(ioinfo_match_result_lock_);
      ioinfo_match_result_[ioinfo_ptr->GetName()] = "1";
    }
  }

  return YSOS_ERROR_SUCCESS;
}

bool BaseTransitionInfoImpl::IsMatched(BufferInterfacePtr input_ptr, BufferInterfacePtr output_ptr, void *context_ptr) {
  if (NULL != callback_ptr_) {
    //  uint32_t msg_id = msg->GetMessageID();
    if (is_matched_for_callback_) {
      MsgInterface *msg_ptr = dynamic_cast<MsgInterface*>(input_ptr.get());
      if(NULL == msg_ptr) {
        YSOS_LOG_ERROR("input ptr is not a valid msg ptr");
        return false;
      }
      uint32_t msg_id = msg_ptr->GetMessageID();
      if(0 == msg_id) {
        YSOS_LOG_ERROR("msg id is 0, it is a invalid msg");
        return false;
      }

      if(matched_msg_id_ != msg_id) {
        YSOS_LOG_ERROR("msg id is not the matched msg id: " << matched_msg_id_ << " | " << msg_id);
        return false;
      }

      Reset();
      YSOS_LOG_DEBUG("transition callback is matched: " << callback_ptr_->GetName());
      return true;
    }
    return is_matched_for_callback_;
  }

  std::string expression_str = expression_;
  {
    AutoLockOper lock(ioinfo_match_result_lock_);
    for (IOInfoList::iterator it=ioInfo_list_.begin(); it!=ioInfo_list_.end(); ++it) {
      IoInfoInterfacePtr ioinfo_ptr = *it;
      std::string is_matched = ioinfo_match_result_[ioinfo_ptr->GetName()];
      boost::algorithm::replace_first(expression_str, "${" + ioinfo_ptr->GetName() + "$}", is_matched);
    }
  }

  // 计算 expression_的结果
  if (GetIoinfoUtility()->ParseExpression(expression_str)) {
    Reset();
    return true;
  }

  return false;
}

void BaseTransitionInfoImpl::Reset(void) {
  AutoLockOper lock(ioinfo_match_result_lock_);
  is_matched_for_callback_ = false;
  matched_msg_id_ = 0;
  for (IOInfoList::iterator it=ioInfo_list_.begin(); it!=ioInfo_list_.end(); ++it) {
    IoInfoInterfacePtr ioinfo_ptr = *it;
    ioinfo_match_result_[ioinfo_ptr->GetName()] = "0";
  }
}

uint32_t BaseTransitionInfoImpl::GetMessageID(BufferInterfacePtr input_ptr, BufferInterfacePtr output_ptr, void *context_ptr) {
  IoInfoInterface *ioinfo_ptr = static_cast<IoInfoInterface*>(context_ptr);
  if (NULL == ioinfo_ptr) {
    YSOS_LOG_DEBUG("GetHeader failed: invalid ioinfo");
    return 0;
  }

  return GetPlatformInfo()->GetMessageIdByServiceName(ioinfo_ptr->GetName());
}

int BaseTransitionInfoImpl::GetProperty(int type_id, void *type) {
  int ret = YSOS_ERROR_SUCCESS;
  switch (type_id) {
  case PROP_TRANSITION_INFO_CALLBACK: {
    ret = YSOS_ERROR_NOT_SUPPORTED;
    break;
  }
  default:
    return BaseInterfaceImpl::GetProperty(type_id, type);
  }
  return YSOS_ERROR_SUCCESS;
}

int BaseTransitionInfoImpl::AddMessageToQueue(MsgInterfacePtr message_ptr, void *context_ptr) {
  assert(NULL != strategy_ptr_);
  strategy_ptr_->SetProperty(PROP_MSG, &message_ptr);
  return YSOS_ERROR_SUCCESS;
}

int BaseTransitionInfoImpl::SetProperty(int type_id, void *type) {
  int ret = YSOS_ERROR_SUCCESS;
  switch (type_id) {
  case PROP_LOGIC: {
    std::string *name = static_cast<std::string*>(type);
    if (NULL != name) {
      LogicIoInfoPtr logic_ptr = boost::make_shared<LogicIoInfo>();
      logic_ptr->value = *name;
      logic_ptr->type = LOGIC_TYPE;
      logic_ioinfo_list_.push_back(logic_ptr);
      expression_ += GetUtility()->GetOperator(*name);
    }
  }
  break;
  case PROP_IOINFO: {
    IoInfoPtr *ioinfo_ptr = static_cast<IoInfoPtr*>(type);
    if (NULL != ioinfo_ptr) {
      LogicIoInfoPtr logic_ptr = boost::make_shared<LogicIoInfo>();
      logic_ptr->ioinfo_ptr = *ioinfo_ptr;
      logic_ptr->type = INFO_TYPE;
      logic_ioinfo_list_.push_back(logic_ptr);
      expression_ += "${" + logic_ptr->ioinfo_ptr->name + "$}";
      ConstructIoInfo(logic_ptr->ioinfo_ptr);
      /*if (ioinfo_event_map_.end() == ioinfo_event_map_.find((*ioinfo_ptr)->name)) {
        ioinfo_event_map_.insert(std::make_pair(logic_ptr->ioinfo_ptr->name, logic_ptr->ioinfo_ptr->event_name));
      } else {
        ret = YSOS_ERROR_HAS_EXISTED;
      }*/
    }
  }
  break;
  case PROP_TRANSITION_INFO_CALLBACK: {
    CallbackInterfacePtr *callback_ptr = reinterpret_cast<CallbackInterfacePtr*>(type);
    if (NULL != callback_ptr) {
      callback_ptr_ = *callback_ptr;
    }
  }
  break;
  case PROP_STRATEGY: {
    StrategyInterface *strategy_ptr = static_cast<StrategyInterface*>(type);
    assert(NULL != strategy_ptr);
    strategy_ptr_ = strategy_ptr;
  }
  break;
  case PROP_IOINFO_CALLBACK: {
    for (IOInfoList::iterator it=ioInfo_list_.begin(); it!=ioInfo_list_.end(); ++it) {
      (*it)->SetProperty(PROP_IOINFO_CALLBACK, type);
    }
    break;
  }
  case PROP_CLEAR_RESULT: {
    for (IOInfoList::iterator it=ioInfo_list_.begin(); it!=ioInfo_list_.end(); ++it) {
      IoInfoInterfacePtr ioinfo_ptr = *it;
      ioinfo_match_result_[ioinfo_ptr->GetName()] = "0";
    }
    break;
  }
  case PROP_SWITCH: {
    TransitionContext *transition_context_ptr = reinterpret_cast<TransitionContext*>(type);
    assert(NULL != transition_context_ptr && NULL != transition_context_ptr->module_ptr);
    ret = SetSwitchFlag(transition_context_ptr->module_ptr->GetName(), transition_context_ptr->message_type);
    break;
  }
  default:
    return BaseInterfaceImpl::SetProperty(type_id, type);
    break;
  }
  return ret;
}

void BaseTransitionInfoImpl::ConstructIoInfo(IoInfoPtr ioinfo) {
  if (NULL == ioinfo) {
    return ;
  }

  //IoinfoUtility ioinfo_utility;
  IoInfoInterfacePtr ioinfo_ptr = GetIoInfoFactory()->CreateObject(ioinfo->data_type, ioinfo->name);
  if (NULL == ioinfo_ptr) {
    return;
  }
  if (!ioinfo->callback_name.empty()) {
    CallbackInterfacePtr callback = GetCallbackInterfaceManager()->FindInterface(ioinfo->callback_name);
    if (NULL == callback) {
      YSOS_LOG_DEBUG("ioinfo get callback: " << ioinfo->callback_name << " failed");
      return ;
    }
    ioinfo_ptr->SetProperty(PROP_IOINFO_CALLBACK, &callback);
    return;
  }

  std::string expression = GetIoinfoUtility()->ParseIoInfo(ioinfo);
  if ("string" == ioinfo->data_type || "text" == ioinfo->data_type) {
    expression = /*GetUtility()->Utf8ToAscii(*/ioinfo->data/*)*/;
  }

  ioinfo_ptr->SetProperty(BaseIoInfoImpl::TRANSITION_INFO_ADD, this);
  ioinfo_ptr->SetProperty(BaseIoInfoImpl::EXPRESSION, &expression);
  //ioinfo_ptr->SetProperty(BaseIoInfoImpl::TRANSITION_INFO, shared_from_this());
  ioInfo_list_.push_back(ioinfo_ptr);
  ioinfo_match_result_.insert(std::make_pair(ioinfo_ptr->GetName(), "0"));
}
}
