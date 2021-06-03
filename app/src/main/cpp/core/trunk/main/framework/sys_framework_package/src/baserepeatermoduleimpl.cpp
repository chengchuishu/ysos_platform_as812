/**
 *@file BaseRepeaterModuleImpl.cpp
 *@brief Definition of BaseRepeaterModuleImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// private header //  NOLINT
#include "../../../protect/include/sys_framework_package/baserepeatermoduleimpl.h"
/// c headers //  NOLINT
#include <set>
/// ysos headers //  NOLINT
#include "../../../public/include/sys_interface_package/ysos_enum.h"
#include "../../../protect/include/sys_framework_package/moduleutil.h"
#include "../../../public/include/core_help_package/utility.h"
// #include "../../../public/include/core_help_package/ysos_log.h"

namespace ysos {

typedef std::multimap<ModuleInterfacePtr, ModuleInterfacePtr>::const_iterator ModuleIterator;
typedef std::multimap<ModuleInterfacePtr, CallbackQueueInterfacePtr>::const_iterator CallbackQueueMapIterator;

BaseRepeaterModuleImpl::BaseRepeaterModuleImpl(const std::string &strClassName): BaseModuleImpl(strClassName) {
  repeat_next_callback_queue_ptr_.reset(NextCallbackQueue::CreateInstance());
  next_callback_queue_map_.clear();
  repeat_map_.clear();
  repeat_map_clone_.clear();
  prev_module_list_.clear();
  transition_list_.clear();
  transition_list_clone_.clear();
  module_stat_ = PROP_RUN;
  transition_list_lock_ = new LightLock();
  callback_queue_lock_ = new LightLock();
  repeat_map_lock_ = new LightLock();
  ioctrl_lock_ = new LightLock();
  InitalDataInfo();
  my_module_link_ptr_ = NULL;
}

BaseRepeaterModuleImpl::~BaseRepeaterModuleImpl() {
  repeat_map_.clear();
  prev_module_list_.clear();
  transition_list_.clear();
  YSOS_DELETE(transition_list_lock_);
  YSOS_DELETE(callback_queue_lock_);
  YSOS_DELETE(repeat_map_lock_);
  YSOS_DELETE(ioctrl_lock_);
}

int BaseRepeaterModuleImpl::SetProperty(int type_id, void *type) {
  int ret = YSOS_ERROR_SUCCESS;

  switch (type_id) {
  case PROP_MYLINK:
    my_module_link_ptr_ = *(ModuleLinkInterfacePtr*)type;
    break;
  default:
    ret = BaseModuleImpl::SetProperty(type_id,type);
  }

  return ret;
}

int BaseRepeaterModuleImpl::GetProperty(int type_id, void *type) {
  int ret = YSOS_ERROR_SUCCESS;

  switch (type_id) {
  case PROP_MYLINK:
    *(ModuleLinkInterfacePtr*)type = my_module_link_ptr_;
    break;
  default:
    ret = BaseModuleImpl::GetProperty(type_id,type);
  }

  return ret;
}

/**
  *@brief  repeat_map_、transition_list_里存放的是控制流数据，当有新的命令收到时，更新repeat_map_、transition_list_的内容，//  NOLINT
  *        repeat_map_clone_、transition_list_ 里是程序运行时真正使用的数据，//  NOLINT
  *        在每次Callback调用前，repeat_map_、transition_list_会根据repeat_map_clone_、transition_list_里的数据进行更新，//  NOLINT
  *        NOLINT保证程序运行时的数据，是最新的。                                   //  NOLINT
  */
int BaseRepeaterModuleImpl::RealUpdateTeamParam(TeamParamsInterface<default_variant_t> *team_parm) {
  // copy to clone
  repeat_map_clone_.clear();
  ModuleIterator module_iter = repeat_map_.begin();
  for (; module_iter != repeat_map_.end(); module_iter++) {
    repeat_map_clone_.insert(*module_iter);
  }

  transition_list_clone_.clear();
  std::set<TransitionPtr>::const_iterator const_iter = transition_list_.begin();
  for (; const_iter != transition_list_.end(); const_iter++) {
    transition_list_clone_.insert(*const_iter);
  }

  int res = UpdateNextCallbackQueueMap();
  if (res != YSOS_ERROR_SUCCESS) {
    return res;
  }

  return YSOS_ERROR_SUCCESS;
}

int BaseRepeaterModuleImpl::RealIoctl(INT32 control_id, TeamParamsInterface<default_variant_t> *team_parm, LPVOID param /* = NULL */) {
  int ret = YSOS_ERROR_SUCCESS;

  YSOS_LOG_DEBUG("control id: " << control_id);

  switch (control_id) {
  case CMD_TRANSITION_ADD: {
    TransitionPtr *transition_ptr = static_cast<TransitionPtr*>(param);
    if (NULL != transition_ptr) {
      transition_list_.insert(*transition_ptr);
    }
    break;
  }
  case CMD_TRANSITION_REMOVE: {
    TransitionPtr *transition_ptr = static_cast<TransitionPtr*>(param);
    if (NULL != transition_ptr) {
      transition_list_.erase(*transition_ptr);
    }
    break;
  }
  case CMD_MAPPINGPAIR_ADD:
    repeat_map_.insert((*(std::pair<ModuleInterfacePtr,ModuleInterfacePtr>*)param));
    UpdatePrevModuleList();
    break;
  case CMD_MAPPINGPAIR_REMOVE:
    EraseSingleMappingPair((*(std::pair<ModuleInterfacePtr,ModuleInterfacePtr>*)param));
    break;
  case CMD_MAPPINGPAIR_REMOVE_ALL:
    repeat_map_.clear();
    break;
  default:
    break;
  }

  return ret;
}

int BaseRepeaterModuleImpl::Ioctl(INT32 control_id, LPVOID param) {
  AutoLockOper lock_ioctrl(ioctrl_lock_);
  int ret = YSOS_ERROR_SUCCESS;

  if (CMD_CONTROL_MODULE == control_id) {
    assert(my_module_link_ptr_);
    ret = my_module_link_ptr_->Ioctl(CMD_MODULE_LINK_CTRL_MODULE,param);

  } else {
    ret = BaseModuleImpl::Ioctl(control_id,param);
  }

  return ret;
}

int BaseRepeaterModuleImpl::AddCallback(CallbackInterfacePtr callback, INT64& owner_id, INT32 type) {
  AutoLockOper lock(callback_queue_lock_);
  return BaseModuleImpl::AddCallback(callback,owner_id,type);
}

int BaseRepeaterModuleImpl::RemoveCallback(INT64& owner_id) {
  AutoLockOper lock(callback_queue_lock_);
  return BaseModuleImpl::RemoveCallback(owner_id);
}

int BaseRepeaterModuleImpl::InitalDataInfo() {
  assert(module_data_info_);
  module_data_info_->module_type = PROP_REPEATER;
  module_data_info_->in_datatypes = "ALL";
  module_data_info_->out_datatypes = "ALL";
  module_data_info_->max_capacity = 1000;  ///< repeater no limited

  return YSOS_ERROR_SUCCESS;
}

void BaseRepeaterModuleImpl::UpdatePrevModuleList() {
  prev_module_list_.clear();
  for (ModuleIterator module_iter = repeat_map_.begin(); module_iter != repeat_map_.end(); ++module_iter) {
    if (!IsModuleInList(prev_module_list_, module_iter->first)) {
      prev_module_list_.push_back(module_iter->first);
    }
  }
}

///   这里的param为空或ModuleInterface*, 下同//NOLINT
int BaseRepeaterModuleImpl::Run(LPVOID param /* = NULL */) {
  assert(my_module_link_ptr_);
  for (ModuleInterfacePtrListIterator module_iter = prev_module_list_.begin(); module_iter != prev_module_list_.end(); ++module_iter) {
    // int res = my_module_link_ptr_->Run((*module_iter).get());
    std::string new_ctrl_range = "";
    if (1 != RebuildControlRangeString(param, my_module_link_ptr_, (*module_iter).get(), new_ctrl_range)) {
      continue;
    }
    int res = my_module_link_ptr_->Run(&new_ctrl_range);
    RETURNONFAIL(res)
  }

  return YSOS_ERROR_SUCCESS;
}

int BaseRepeaterModuleImpl::Pause(LPVOID param /* = NULL */) {
  assert(my_module_link_ptr_);
  std::set<ModuleInterfacePtr> cannotpausemodules;
  /// get all pre_module that can not be paused //  NOLINT
  for (ModuleIterator module_iter = repeat_map_.begin(); module_iter != repeat_map_.end(); module_iter++) {
    int res;
    INT32 module_state;

    if (NULL != module_iter->second) {
      res = module_iter->second->GetState(0,&module_state);
      RETURNONFAIL(res)
    } else {
      module_state = PROP_CLOSE;
    }
    if (module_state > PROP_PAUSE) {
      cannotpausemodules.insert(module_iter->first);
    }
  }

  /// 对所有可以pause的pre_module执行Pause操作 //  NOLINT
  for (ModuleInterfacePtrListIterator module_iter = prev_module_list_.begin();
       module_iter != prev_module_list_.end(); ++module_iter) {
    if (cannotpausemodules.end() == cannotpausemodules.find(*module_iter)) {
      // int res = my_module_link_ptr_->Pause((*module_iter).get());
      std::string new_ctrl_range = "";
      if (1 != RebuildControlRangeString(param, my_module_link_ptr_, (*module_iter).get(), new_ctrl_range)) {
        continue;
      }
      int res = my_module_link_ptr_->Pause(&new_ctrl_range);
      RETURNONFAIL(res)
    }
  }

  return YSOS_ERROR_SUCCESS;
}

int BaseRepeaterModuleImpl::Stop(LPVOID param /* = NULL */) {
  assert(my_module_link_ptr_);
  std::set<ModuleInterfacePtr> cannotstopmodules;
  /// get all pre_module that can not be stopped //  NOLINT
  for (ModuleIterator module_iter = repeat_map_.begin(); module_iter != repeat_map_.end(); module_iter++) {
    INT32 module_state;
    if (NULL != module_iter->second) {
      int res = module_iter->second->GetState(0,&module_state);
      RETURNONFAIL(res)
    } else {
      module_state = PROP_CLOSE;
    }
    if (module_state > PROP_STOP) {
      cannotstopmodules.insert(module_iter->first);
    }
  }
  // for (std::set<ModuleInterfacePtr>::iterator it = cannotstopmodules.begin();
  //   it != cannotstopmodules.end(); ++it) {
  //   YSOS_LOG_DEBUG((*it)->GetName() << " could not stop.");
  // }
  /// 对所有可以stop的pre_module执行stop操作 //  NOLINT
  for (ModuleInterfacePtrListIterator module_iter = prev_module_list_.begin();
       module_iter != prev_module_list_.end(); ++module_iter) {
    if (cannotstopmodules.end() == cannotstopmodules.find(*module_iter)) {
      // int res = my_module_link_ptr_->Stop((*module_iter).get());
      std::string new_ctrl_range = "";
      if (1 != RebuildControlRangeString(param, my_module_link_ptr_, (*module_iter).get(), new_ctrl_range)) {
        // YSOS_LOG_DEBUG((*module_iter)->GetName() << " should not be stopped.");
        continue;
      }
      int res = my_module_link_ptr_->Stop(&new_ctrl_range);
      RETURNONFAIL(res)
    }
  }

  return YSOS_ERROR_SUCCESS;
}

int BaseRepeaterModuleImpl::GetState(UINT32 time_out, INT32 *state, LPVOID param) {
  /// 为NULL，是Repeater本身的状态，非NULL，取指定Module的NextModule状态 //  NOLINT
  if (NULL == param) {
    /// 不用加锁 //  NOLINT
    *state = (INT32)module_stat_;
    return YSOS_ERROR_SUCCESS;
  }

  ModuleInterface *module_ptr = static_cast<ModuleInterface*>(param);
  if (NULL == module_ptr) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  bool is_found = false;
  int module_state = PROP_CLOSE;
  for (RepeaterMap::iterator it=repeat_map_.begin(); it!=repeat_map_.end(); ++it) {
    ModuleInterfacePtr module_key_ptr = it->first;
    if (module_key_ptr.get() != module_ptr) {
      continue;
    }

    ModuleInterfacePtr next_module_ptr = it->second;
    if (NULL == next_module_ptr) {
      break;
    }

    int ret = next_module_ptr->GetState(time_out, (INT32*)&module_state);
    assert(YSOS_ERROR_SUCCESS == ret);
    if (module_state > (int)(*state)) {
      *state = module_state;
    }
    is_found = true;
  }

  if (!is_found) {
    *state = (INT32)module_stat_;
  }

  return YSOS_ERROR_SUCCESS;
}

int BaseRepeaterModuleImpl::RealOpen(LPVOID param /* = NULL */) {
  return YSOS_ERROR_SUCCESS;
}

int BaseRepeaterModuleImpl::RealClose() {
  return YSOS_ERROR_SUCCESS;
}

int BaseRepeaterModuleImpl::RealRun() {
  return YSOS_ERROR_SUCCESS;
}

int BaseRepeaterModuleImpl::RealPause() {
  return YSOS_ERROR_SUCCESS;
}

int BaseRepeaterModuleImpl::RealStop() {
  return YSOS_ERROR_SUCCESS;
}

BaseModuleImpl *BaseRepeaterModuleImpl::GetPrevModule(void *context) {
  BaseModuelCallbackContext *callback_context = static_cast<BaseModuelCallbackContext*>(context);
  assert(NULL != callback_context);

  return dynamic_cast<BaseModuleImpl*>(callback_context->prev_module_ptr);
}

int BaseRepeaterModuleImpl::Observe(BufferInterfacePtr data_ptr,void* context_ptr) {
  //AutoLockOper lock(transition_list_lock_);
  TransitionPtrList::const_iterator const_iter = transition_list_clone_.begin();
  for (; const_iter != transition_list_clone_.end(); const_iter++) {
    BaseModuleImpl *pre_module_ptr = GetPrevModule(context_ptr);
    if (NULL == pre_module_ptr) {
      YSOS_LOG_DEBUG("Invalid pre_module: " << logic_name_);
      continue;
    }

    TransitionPtr transition_ptr = (*const_iter);
//     std::list<std::string> &service_list = transition_ptr->service_list;
//     std::list<std::string>::iterator it = std::find(service_list.begin(), service_list.end(), pre_module_ptr->GetName());
//     if (service_list.end() == it) {
//       continue;
//     }
    bool is_matched = IfMatchedByTransition(transition_ptr, pre_module_ptr->GetName());
    if (!is_matched) {
      continue;
    }

    assert(NULL != transition_ptr->transition_info);
    transition_ptr->transition_info->Filter(data_ptr,0,context_ptr);
  }

  return YSOS_ERROR_SUCCESS;
}

/// 根据服务名匹配服务的实例名或别名，如果匹配上，就进入Transition的Filter
bool BaseRepeaterModuleImpl::IfMatchedByTransition(TransitionPtr transition_ptr, const std::string &service_name) {
  ServiceList &service_list = transition_ptr->service_info_list;
  ServiceList::iterator it = service_list.begin();
  bool is_matched = false;

  for (; service_list.end() != it; ++it) {
    PlatformService::ServiceInfoPtr service_ptr = *it;
    /// 一个服务至多有一个别名
    if (service_name == service_ptr->name || service_name == service_ptr->alias) {
      is_matched = true;
      break;
    }
  }

  return is_matched;
}

int BaseRepeaterModuleImpl::UpdateNextCallbackQueueMap() {
  next_callback_queue_map_.clear();
  for (ModuleIterator module_iter = repeat_map_clone_.begin(); module_iter != repeat_map_clone_.end(); module_iter++) {
    CallbackQueueInterfacePtr module_next_call_back_queue;
    CallbackQueueMapIterator found_iter = next_callback_queue_map_.find(module_iter->first);

    // if not exist, create a new one, else set second to it
    if (next_callback_queue_map_.end() == found_iter) {
      module_next_call_back_queue.reset(NextCallbackQueue::CreateInstance());

      std::pair<ModuleInterfacePtr,CallbackQueueInterfacePtr> new_map_pair;
      new_map_pair.first = module_iter->first;
      new_map_pair.second = module_next_call_back_queue;
      next_callback_queue_map_.insert(new_map_pair);

    } else {
      module_next_call_back_queue = found_iter->second;
    }

    CallbackInterfacePtr next_callback_ptr = next_callback_queue_->FindCallback(true);
    //  对于next_callback_queue_中的每一个callback//NOLINT
    while (NULL != next_callback_ptr) {
      //  在mapped_modules的元素中
      //  next_callback_ptr是否该元素的second的callback? 若是则//NOLINT
      if (IsCallbackOf(module_iter->second,next_callback_ptr)) {
        //  添加到repeat_next_callback_queue_ptr_并对该next_callback_ptr结束查找//NOLINT
        module_next_call_back_queue->AddCallback(next_callback_ptr, (int64_t)module_iter->second.get(), GetModulePriority(module_iter->second));
      }
      next_callback_ptr = next_callback_queue_->FindCallback();
    }
  }
  return YSOS_ERROR_SUCCESS;
}

int  BaseRepeaterModuleImpl::GetRepeatedNextCallbackQueue(ModuleInterfacePtr prev_module,CallbackQueueInterfacePtr& next_call_queue) {
  CallbackQueueMapIterator found_iter = next_callback_queue_map_.find(prev_module);
  if (next_callback_queue_map_.end() == found_iter) {
    next_call_queue.reset();
    return YSOS_ERROR_FAILED;
  } else {
    next_call_queue = found_iter->second;
    return YSOS_ERROR_SUCCESS;
  }
}

// 删除一条链接  prev_module--->next_module
void BaseRepeaterModuleImpl::EraseSingleMappingPair(
  std::pair<ModuleInterfacePtr,ModuleInterfacePtr> pair_to_erase) {
  for (ModuleIterator module_iter = repeat_map_.begin(); module_iter != repeat_map_.end(); module_iter++) {
    if (module_iter->first.get() == pair_to_erase.first.get() && module_iter->second.get() == pair_to_erase.second.get()) {
      repeat_map_.erase(module_iter);
      UpdatePrevModuleList();
      return;
    }
  }
}

bool BaseRepeaterModuleImpl::IsCallbackOf(ModuleInterfacePtr module_ptr,CallbackInterfacePtr test_callback_ptr) {
  if (NULL == module_ptr) {
    return false;
  }

  CallbackQueueInterface* pre_callback_queue = NULL;
  module_ptr->GetProperty(PROP_PREVCALLBACKQUEUE,&pre_callback_queue);

  if (NULL == pre_callback_queue) {
    return false;
  }

  CallbackInterfacePtr pre_callback_ptr = pre_callback_queue->FindCallback(true);
  while (NULL != pre_callback_ptr.get()) {
    if (test_callback_ptr == pre_callback_ptr) {
      return true;
    }
    pre_callback_ptr = pre_callback_queue->FindCallback();
  }

  return false;
}
}
