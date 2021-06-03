/**
 *@file ModuleImpl.cpp
 *@brief Definition of ModuleImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// Self Header //  NOLINT
#include "../../../protect/include/sys_framework_package/basemoduleimpl.h"  // NOLINT
/// c headers //  NOLINT
#include <cassert>
/// ysos private headers //  NOLINT
#include "../../../protect/include/core_help_package/bufferimpl.h"
#include "../../../protect/include/core_help_package/bufferpoolimpl.h"  // NOLINT
#include "../../../protect/include/os_hal_package/teamparamsimpl.h"
#include "../../../protect/include/sys_framework_package/precallbackqueue.h"  // NOLINT
#include "../../../protect/include/sys_framework_package/nextcallbackqueue.h"  // NOLINT
#include "../../../protect/include/sys_framework_package/basemodulecallbackimpl.h"
#include "../../../public/include/core_help_package/utility.h"
// #include "../../../public/include/core_help_package/ysos_log.h"
#include "../../../protect/include/sys_framework_package/packageconfigimpl.h"
#include "../../../protect/include/sys_framework_package/manager_declare.h"
#include "../../../public/include/core_help_package/jsonutility.h"
#include "../../../public/include/os_hal_package/bufferutility.h"
#include "../../../public/include/sys_interface_package/ysos_struct.h"
#include "../../../protect/include/sys_framework_package/moduleutil.h"
#include "../../../protect/include/sys_framework_package/platforminfo.h"
#include "../../../protect/include/os_hal_package/msgimpl.h"

namespace ysos {
BaseModuleCallbackFunction::BaseModuleCallbackFunction(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, void *context_ptr) {
  input_buffer_ = input_buffer;
  output_buffer_ = output_buffer;
  context_ptr_ = context_ptr;
}

BaseModuleCallbackFunction::~BaseModuleCallbackFunction() {

}

int BaseModuleCallbackFunction::operator()(const CallbackInterfacePtr &callback_ptr, BaseModuleImpl *context_ptr) {
  BaseModuelCallbackContext *callback_context = static_cast<BaseModuelCallbackContext*>(context_ptr_);
  if (NULL == callback_context) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }
  BaseModuleCallbackImpl *base_module_callback = reinterpret_cast<BaseModuleCallbackImpl*>(callback_context->cur_callback_ptr);
  if (NULL == base_module_callback) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  return base_module_callback->InvokeNextCallback(input_buffer_, output_buffer_, callback_ptr, context_ptr_);//->Callback(input_buffer_, output_buffer_, context_ptr_);
}

BaseModuleImpl::BaseModuleImpl(const std::string& strClassName): BaseInterfaceImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.plugin");

  module_ref_count_ = 0;
  module_data_info_ = new ModuleDataInfo();
  module_data_info_->buffer_length = 0;
  module_data_info_->module_type = PROP_SOURCE;
  module_data_info_->in_datatypes = "";
  module_data_info_->out_datatypes = "";
  module_data_info_->is_self_allocator = false;
  module_data_info_->prefix_length = 0;
  module_data_info_->buffer_length = 0;
  module_data_info_->is_data_revise = false;
  module_data_info_->buffer_number = 0;
  module_data_info_->max_capacity = 1;
  module_stat_ = PROP_CLOSE;
//   if (module_data_info_) {
//     YSOS_LOG_DEBUG("s1: " << module_data_info_->in_datatypes);
//     YSOS_LOG_DEBUG("s2: " << module_data_info_->out_datatypes);
//   }
  // buffer_pool_ = NULL;//CreateBufferPool(BufferImpl);

}

BaseModuleImpl:: ~BaseModuleImpl(void) {
//   YSOS_LOG_DEBUG("BaseModuleImpl:: ~BaseModuleImpl(void)");
  YSOS_DELETE(module_lock_);
  YSOS_DELETE(module_data_info_);
  YSOS_DELETE(pre_callback_queue_);
  YSOS_DELETE(next_callback_queue_);
  YSOS_DELETE(team_param_for_data_flow_);
  YSOS_DELETE(team_param_for_control_flow_);
  YSOS_DELETE(team_param_for_data_flow_lock_);
  YSOS_DELETE(team_param_for_control_flow_lock_);
  YSOS_DELETE(next_callback_queue_lock_);
  YSOS_DELETE(pre_callback_queue_lock_);
  module_stat_info_map_.clear();
  if (NULL != buffer_pool_ptr_) {
    buffer_pool_ptr_->Decommit();
    buffer_pool_ptr_ = NULL;
  }

}

int BaseModuleImpl::SetModuleType(const std::string &value) {
  if ("Source" == value) {
    module_data_info_->module_type = PROP_SOURCE;
  } else if ("Transform" == value) {
    module_data_info_->module_type = PROP_TRANSFORM;
  } else if ("Destination" == value) {
    module_data_info_->module_type = PROP_DESTINATION;
  } else if ("Repeater" == value) {
    module_data_info_->module_type = PROP_REPEATER;
  } else {
    YSOS_LOG_DEBUG("Invalid module type: " << value);
  }

  return YSOS_ERROR_SUCCESS;
}

int BaseModuleImpl::Initialized(const std::string &key, const std::string &value) {
  uint32_t i = 0;

  return YSOS_ERROR_NOT_SUPPORTED;
}

int BaseModuleImpl::ConfigInitialize(void) {
  MapPtr conf_map = GetPackageConfigImpl()->FindModuleConf(logic_name_);
  if (NULL == conf_map) {
    YSOS_LOG_DEBUG("Module " << logic_name_ << " doesn't has a config file");
    return YSOS_ERROR_NOT_SUPPORTED;
  }

  int ret = YSOS_ERROR_SUCCESS;
  uint32_t i = 0;
  for (MapIterator it=conf_map->begin(); it!=conf_map->end(); ++it) {
    ret = YSOS_ERROR_SUCCESS;
    if ("in_datatypes" == it->first) {
      module_data_info_->in_datatypes = it->second;
    } else if ("out_datatypes" == it->first) {
      module_data_info_->out_datatypes = it->second;
    } else if ("buffer_length" == it->first) {
      module_data_info_->buffer_length = GetUtility()->ConvertFromString(it->second, i);
    } else if ("buffer_number" == it->first) {
      module_data_info_->buffer_number = GetUtility()->ConvertFromString(it->second, i);
    } else if ("allocator" == it->first) {
      module_data_info_->is_self_allocator = (it->second=="true"?true:false);
    } else if ("capacity" == it->first) {
      module_data_info_->max_capacity = GetUtility()->ConvertFromString(it->second, i);
    } else if ("prefix" == it->first) {
      module_data_info_->prefix_length = GetUtility()->ConvertFromString(it->second, i);
    } else if ("revise" == it->first) {
      module_data_info_->is_data_revise = (it->second=="true"?true:false);
    } else if ("type" == it->first) {
      SetModuleType(it->second);
    } else if ("version" == it->first) {
      version_ = it->second;
    } else if ("remark" == it->first) {
      remark_ = it->second;
    } else if ("callback" == it->first) {
      callback_ = it->second;
    } else if ("driver" == it->first) {
      driver_ = it->second;
    } else {
      ret = Initialized(it->first, it->second);
    }

    if (YSOS_ERROR_SUCCESS != ret) {
      YSOS_LOG_DEBUG("Initialized failed: [" << it->first << "|" << it->second << "] : " <<ret);
    }

    YSOS_LOG_DEBUG("Initialized " << it->first << " : " << it->second << " : " << logic_name_);
  }

  return ret;
}

int BaseModuleImpl::Initialize(void *param) {
  if (IsInitialized()) {
    return YSOS_ERROR_FINISH_ALREADY;
  }

  status_event_callback_ptr_ = GetCallbackInterfaceManager()->FindInterface("StatusEventModuleCallback");
  assert(NULL != status_event_callback_ptr_);
  module_lock_ = new LightLock();
  pre_callback_queue_ = PreCallbackQueue::CreateInstance();
  next_callback_queue_ = NextCallbackQueue::CreateInstance();
  ///module_stat_ = PROP_CLOSE;
  module_run_state_count_ = module_open_state_count_ = 0;
  team_param_for_control_flow_ = DefaultTeamParamsImpl::CreateInstance();
  team_param_for_data_flow_ = DefaultTeamParamsImpl::CreateInstance();
  team_param_for_control_flow_lock_ = new LightLock();
  team_param_for_data_flow_lock_ = new LightLock();
  team_param_for_control_flow_count_ = team_param_for_data_flow_count_ = 0;

  next_callback_queue_lock_ = new LightLock();
  pre_callback_queue_lock_ = new LightLock();

  ConfigInitialize();

  int ret = AllocateBufferPool();
  if (YSOS_ERROR_SUCCESS != ret) {
    YSOS_LOG_DEBUG("allocate buffer pool failed: " << ret);
  }

  return BaseInterfaceImpl::Initialize(param);
}

int BaseModuleImpl::UnInitialize(void *param) {
  if (!IsInitialized()) {
    return YSOS_ERROR_SUCCESS;
  }

//   YSOS_LOG_DEBUG("BaseModuleImpl:: UnInitialize");
//   YSOS_DELETE(module_lock_);
//   YSOS_DELETE(module_data_info_);
//   YSOS_DELETE(pre_callback_queue_);
//   YSOS_DELETE(next_callback_queue_);
//   YSOS_DELETE(team_param_for_data_flow_);
//   YSOS_DELETE(team_param_for_control_flow_);
//   YSOS_DELETE(team_param_for_data_flow_lock_);
//   YSOS_DELETE(team_param_for_control_flow_lock_);
//   YSOS_DELETE(next_callback_queue_lock_);
//   YSOS_DELETE(pre_callback_queue_lock_);
//   module_stat_info_map_.clear();
//   if(NULL != buffer_pool_ptr_) {
//     buffer_pool_ptr_->Decommit();
//     buffer_pool_ptr_ = NULL;
//   }

  return BaseInterfaceImpl::UnInitialize(param);
}

// ModuleInterface functions
int BaseModuleImpl::Open(LPVOID param, LPVOID context) {
  YSOS_LOG_DEBUG("begin to open: " << logic_name_);
  AutoLockOper lock(module_lock_);
#if 0 //  1, normal; 0, only for test.
  if (module_stat_info_map_.size() >= module_data_info_->max_capacity) {
    YSOS_LOG_DEBUG("open failed: cur module reach the max capacity: " << logic_name_ << " max capacity: " << module_data_info_->max_capacity << " cur size: " << module_stat_info_map_.size());

    return YSOS_ERROR_NOT_ENOUGH_RESOURCE;
  }
#endif
  if (PROP_CLOSE != module_stat_) {
    YSOS_LOG_ERROR("module open failed, cur stat is not close: " << logic_name_ << " cur state: " << module_stat_);
    return YSOS_ERROR_SUCCESS;
  }

  // Open时，不用考虑NextModule的状态
  int dst_stat = PROP_STOP;

  int ret = UpdateModuleStat(module_stat_, dst_stat);
  YSOS_LOG_DEBUG("end to open: " << ret << " | " << logic_name_);

  return ret;
}

int BaseModuleImpl::Stop(LPVOID param) {
  YSOS_LOG_DEBUG("begin to stop: " << logic_name_);
  AutoLockOper lock(module_lock_);
  int dst_stat = PROP_STOP;

  // PROP_CLOSE状态下不能PROP_STOP
  if (PROP_CLOSE == module_stat_) {
    YSOS_LOG_ERROR("module stop failed, cur stat is close: " << logic_name_ << " cur state: " << module_stat_);
    return YSOS_ERROR_LOGIC_ERROR;
  }

  if (ComputeModuleLinkCurStat() > dst_stat) {
    // next module is running, can't PROP_STOP or PROP_PAUSE
    YSOS_LOG_ERROR("mstop failed, module is busy: " << logic_name_);

    return YSOS_ERROR_IS_BUSY;
  }
  YSOS_LOG_DEBUG("end to stop: " << logic_name_);

  return UpdateModuleStat(module_stat_, dst_stat);
}

int BaseModuleImpl::AddCallback(CallbackInterfacePtr callback, INT64& owner_id, INT32 type) {  // NOLINT
  if (NULL == callback) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  //AutoLockOper lock(module_lock_);
  int ret = YSOS_ERROR_SUCCESS;
  if (0 == owner_id) {
    YSOS_LOG_DEBUG(this->GetName() << "Add PreCallback: " << callback->GetName());
    AutoLockOper lock(pre_callback_queue_lock_);
    ret = pre_callback_queue_->AddCallback(callback);
    if (YSOS_ERROR_SUCCESS == ret) {
      YSOS_LOG_DEBUG(logic_name_ << " add prev callback: " << callback->GetName() << " success");
    } else {
      YSOS_LOG_DEBUG(logic_name_ << " add prev callback: " << callback->GetName() << " failed");
    }
  } else {
    ret = AddNextCallback(callback, owner_id, type);
  }

  if (YSOS_ERROR_SUCCESS != ret) {
    // 记录日志  // NOLINT
    YSOS_LOG_DEBUG(this->GetName() << " Addcallback failed: " << callback->GetName());
  }

  return ret;
}

int BaseModuleImpl::AddNextCallback(CallbackInterfacePtr callback, INT64& owner_id, INT32 type) {
  assert(NULL != callback);
  assert(owner_id > 0);

  ///   Next Module 是不是Repeater      // NOLINT
  ///   对于Repeater作特殊处理：NextCalllback可以加入，不占用Capacity数量      // NOLINT
  bool is_next_module_repeater = false;
  /// 记录后链接Module的状态
  ModuleInterface *next_module_ptr = reinterpret_cast<ModuleInterface*>(owner_id);
  BaseModuleImpl *next_module = dynamic_cast<BaseModuleImpl*>(next_module_ptr);
  assert(NULL != next_module && NULL != next_module->module_data_info_);
  is_next_module_repeater = (PROP_REPEATER == next_module->module_data_info_->module_type);

  if (PROP_DESTINATION == module_data_info_->module_type && !is_next_module_repeater) {
    YSOS_LOG_ERROR("Destination module can't add normal module callback: " << logic_name_);
    return YSOS_ERROR_LOGIC_ERROR;
  }
#if 0  // only for test
  if (module_stat_info_map_.size() >= module_data_info_->max_capacity && !is_next_module_repeater) {
    YSOS_LOG_DEBUG(this->GetName() << " cur module reach the max capacity: " << module_stat_info_map_.size() << ":" << module_data_info_->max_capacity);
    // YSOS_LOG_DEBUG("cur module reach the max capacity");
    return YSOS_ERROR_NOT_ENOUGH_RESOURCE;
  }
#endif

  AutoLockOper  lock(next_callback_queue_lock_);
  int ret = next_callback_queue_->AddCallback(callback, owner_id, type);
  assert(YSOS_ERROR_SUCCESS == ret);

  std::pair<INT64, INT64> owner_id_pair = std::make_pair((INT64)this, owner_id);
  ret = callback->SetProperty(PROP_OWNER_ID, (void*)&owner_id_pair);

  int next_module_state = PROP_CLOSE;
  // 获取NextModule的当前状态
  ret = next_module_ptr->GetState(0, (INT32*)&next_module_state);
  assert(YSOS_ERROR_SUCCESS == ret);
  InsertModuleStatRecord(owner_id, next_module_state);
  YSOS_LOG_DEBUG(this->GetName() << "Add next callback success: " << module_stat_info_map_.size() << ":" << module_data_info_->max_capacity);

  YSOS_LOG_DEBUG(logic_name_ << " ********** add next callback: " << next_module_ptr->GetName());
  if (YSOS_ERROR_SUCCESS == ret) {
    ++module_ref_count_;
  }
  YSOS_LOG_DEBUG(this->GetName() << "Add NextCallback: " << ((BaseModuleImpl*)(owner_id))->GetName() << ":" << callback->GetName());

  return ret;
}

int BaseModuleImpl::RemoveCallback(INT64& owner_id) {
  int ret = YSOS_ERROR_SUCCESS;

  if (0 == owner_id) {
    AutoLockOper lock(pre_callback_queue_lock_);
    ret = pre_callback_queue_->RemoveByID(owner_id);
    ModuleInterface *next_module_ptr = reinterpret_cast<ModuleInterface*>(owner_id);
    if (YSOS_ERROR_SUCCESS == ret) {
      YSOS_LOG_DEBUG(logic_name_ << " remove prev module callback: " << next_module_ptr->GetName() << " success");
    } else {
      YSOS_LOG_DEBUG(logic_name_ << " remove prev module callback: " << next_module_ptr->GetName() << " failed");
    }
  } else {
    AutoLockOper lock(next_callback_queue_lock_);
    ret = next_callback_queue_->RemoveByID(owner_id);
    if (YSOS_ERROR_SUCCESS == ret) {
      /// 两个模块间只可能有一个连接 //  NOLINT
      --module_ref_count_;
    }

    if (owner_id != 0) {
      RemoveModuleStatRecord(owner_id);
    }

    ModuleInterface *next_module_ptr = reinterpret_cast<ModuleInterface*>(owner_id);
    YSOS_LOG_DEBUG(logic_name_ << " ********** remove next callback: " << next_module_ptr->GetName());
    YSOS_LOG_DEBUG(this->GetName() << " Remove next callback success: " << module_stat_info_map_.size() << ":" << module_data_info_->max_capacity);
  }

  if (YSOS_ERROR_SUCCESS != ret) {
    // 记录日志  // NOLINT
  }

  return ret;
}

int BaseModuleImpl::Ioctl(INT32 ctrol_id, LPVOID param) {
  int ret=YSOS_ERROR_SUCCESS;

  if (ctrol_id > CMD_CONTROL) {
    ret = UpdateControlCommand(ctrol_id, param);
  }

  return YSOS_ERROR_SUCCESS;
}

int BaseModuleImpl::RealIoctl(INT32 control_id, TeamParamsInterface<default_variant_t> *team_parm, LPVOID param) {
  int ret = YSOS_ERROR_SUCCESS;

  return YSOS_ERROR_SUCCESS;
}


/**
  *这个函数里，不对team_param_for_data_flow_加锁，
  *所以，data_flow无论什么时候变化，会被锁多久，都不会
  *对控制流有影响
  */
int BaseModuleImpl::UpdateControlCommand(INT32 control_id, LPVOID param) {
  assert(NULL != team_param_for_data_flow_);
  assert(NULL != team_param_for_control_flow_);

  AutoLockOper lock(team_param_for_control_flow_lock_);

  /*
   * 不管当前是Data数据新，还是Control数据新，都取Data的最新值
  */
  if (team_param_for_control_flow_count_ != team_param_for_data_flow_count_) {
    // 获取最新的team_param_for_data_flow_数据
    team_param_for_control_flow_->CopyFrom(team_param_for_data_flow_);
    team_param_for_control_flow_count_ = team_param_for_data_flow_count_;
  }

  int ret = RealIoctl(control_id, team_param_for_control_flow_, param);

  if (YSOS_ERROR_SUCCESS == ret) {
    ++ team_param_for_control_flow_count_;
  }

  return ret;
}

int BaseModuleImpl::UpdateTeamParam(void) {
  if (team_param_for_data_flow_count_ == team_param_for_control_flow_count_) {
    return YSOS_ERROR_SUCCESS;
  }

  {
    AutoLockOper control_lock(team_param_for_control_flow_lock_);
    if (team_param_for_control_flow_count_ > team_param_for_data_flow_count_) {
      team_param_for_data_flow_->CopyFrom(team_param_for_control_flow_);
      team_param_for_data_flow_count_ = team_param_for_control_flow_count_;
    } else if (team_param_for_control_flow_count_ < team_param_for_data_flow_count_) {
      team_param_for_control_flow_->CopyFrom(team_param_for_data_flow_);
      team_param_for_control_flow_count_ = team_param_for_data_flow_count_;
    }
  }

  int ret = YSOS_ERROR_SUCCESS;
  {
    ret = RealUpdateTeamParam(team_param_for_data_flow_);
  }

  return ret;
}

int BaseModuleImpl::RealUpdateTeamParam(TeamParamsInterface<default_variant_t> *team_parm) {
  return YSOS_ERROR_SUCCESS;
}

int BaseModuleImpl::UpdateDataCommand(LPVOID param) {
  assert(NULL != team_param_for_data_flow_);

  AutoLockOper lock(team_param_for_data_flow_lock_);
  //int ret = UpdateCommand(team_param_for_data_flow_, param);

  /* if (YSOS_ERROR_SUCCESS == ret) {
     ++ team_param_for_control_flow_count_;
   }*/

  return YSOS_ERROR_SUCCESS;
}

int BaseModuleImpl::Close(LPVOID param) {
  YSOS_LOG_DEBUG("begin to close: " << logic_name_);
  AutoLockOper lock(module_lock_);
  int dst_stat = PROP_CLOSE;

  int ret = UpdateModuleStat(module_stat_, dst_stat);
  YSOS_LOG_DEBUG("end to close: " << ret << " | " << logic_name_);

  return ret;
}

int BaseModuleImpl::Pause(LPVOID param) {
  YSOS_LOG_DEBUG("begin to pause: " << logic_name_);
  AutoLockOper lock(module_lock_);
  int dst_stat = PROP_PAUSE;

  if (ComputeModuleLinkCurStat() > dst_stat) {
    // cur module is used by next modules
    YSOS_LOG_DEBUG("end to pause failed, module is busy: " << logic_name_);

    return YSOS_ERROR_IS_BUSY;
  }

  int ret = UpdateModuleStat(module_stat_, dst_stat);
  YSOS_LOG_DEBUG("end to pause: " << ret << " | " << logic_name_);

  return ret;
}

int BaseModuleImpl::Run(LPVOID param) {
  YSOS_LOG_DEBUG("begin to run: " << logic_name_);
  AutoLockOper lock(module_lock_);
  int dst_stat = PROP_RUN;

  if (ComputeModuleLinkCurStat() > dst_stat) {
    // cur module is used by next modules
    YSOS_LOG_DEBUG("run failed, module is busy: " << logic_name_);
    return YSOS_ERROR_IS_BUSY;
  }

  int ret = UpdateModuleStat(module_stat_, dst_stat);
  YSOS_LOG_DEBUG("end to run: " << ret << " | " << logic_name_);

  return ret;
}

int BaseModuleImpl::Flush(LPVOID param) {
  return YSOS_ERROR_SUCCESS;
}

int BaseModuleImpl::GetState(UINT32 time_out, INT32 *state, LPVOID param) {
  /// 不用加锁 //  NOLINT
  *state = (INT32)module_stat_;
  // YSOS_LOG_INFO("BaseModuleImpl::GetState: " << *state << " | " << logic_name_);

  return YSOS_ERROR_SUCCESS;
}

int BaseModuleImpl::GetProperty(int type_id, void *type) {
  if (NULL == type) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  int ret = YSOS_ERROR_NOT_SUPPORTED;
  switch (type_id) {
  case PROP_VERSION: {
    AutoLockOper lock(module_lock_);
    std::string *version = static_cast<std::string*>(type);
    if (NULL == version) {
      break;
    }
    *version = version_;
    ret = YSOS_ERROR_SUCCESS;
    break;
  }
  case PROP_IN_DATA_TYPES: {
    AutoLockOper lock(module_lock_);
    CallbackDataType *data_type_ptr = static_cast<CallbackDataType*>(type);
    *data_type_ptr = module_data_info_->in_datatypes;
    ret = YSOS_ERROR_SUCCESS;
    break;
  }
  case PROP_OUT_DATA_TYPES: {
    AutoLockOper lock(module_lock_);
    CallbackDataType *data_type_ptr = static_cast<CallbackDataType*>(type);
    *data_type_ptr = module_data_info_->out_datatypes;
    ret = YSOS_ERROR_SUCCESS;
    break;
  }
  case PROP_SELF_ALLOCATOR: {
    AutoLockOper lock(module_lock_);
    bool *is_self_allocator = static_cast<bool*>(type);
    *is_self_allocator = module_data_info_->is_self_allocator;
    ret = YSOS_ERROR_SUCCESS;
    break;
  }
  case PROP_BUFFER_PREFIX_LENGTH: {
    AutoLockOper lock(module_lock_);
    uint32_t *prefix_length = static_cast<uint32_t*>(type);
    *prefix_length = module_data_info_->prefix_length;
    ret = YSOS_ERROR_SUCCESS;
    break;
  }
  case PROP_BUFFER_LENGTH: {
    AutoLockOper lock(module_lock_);
    uint32_t *buffer_length = static_cast<uint32_t*>(type);
    *buffer_length = module_data_info_->buffer_length;
    ret = YSOS_ERROR_SUCCESS;
    break;
  }
  case PROP_BUFFER_DATA_REVISE: {
    AutoLockOper lock(module_lock_);
    bool *is_data_revise = static_cast<bool*>(type);
    *is_data_revise = module_data_info_->is_data_revise;
    ret = YSOS_ERROR_SUCCESS;
    break;
  }
  case PROP_BUFFER_NUMBER: {
    AutoLockOper lock(module_lock_);
    uint32_t *buffer_number = static_cast<uint32_t*>(type);
    *buffer_number = module_data_info_->buffer_number;
    ret = YSOS_ERROR_SUCCESS;
    break;
  }
  case PROP_MAX_CAPBILITY: {
    AutoLockOper lock(module_lock_);
    uint32_t *max_capacity = static_cast<uint32_t*>(type);
    *max_capacity = module_data_info_->max_capacity;
    ret = YSOS_ERROR_SUCCESS;
    break;
  }
  case PROP_PREVCALLBACKQUEUE: {
    AutoLockOper lock(module_lock_);
    CallbackQueueInterface **callback_queue_ptr = static_cast<CallbackQueueInterface**>(type);
    *callback_queue_ptr = pre_callback_queue_;
    ret = YSOS_ERROR_SUCCESS;
    break;
  }
  case PROP_NEXTCALLBACKQUEUE: {
    AutoLockOper lock(module_lock_);
    CallbackQueueInterface **callback_queue_ptr = static_cast<CallbackQueueInterface**>(type);
    *callback_queue_ptr = next_callback_queue_;
    ret = YSOS_ERROR_SUCCESS;
    break;
  }
  case PROP_STATE_COUNT: {
    AutoLockOper lock(module_lock_);
    UINT32 *state_count = static_cast<UINT32*>(type);
    *state_count = module_run_state_count_;
    ret = YSOS_ERROR_SUCCESS;
    break;
  }
  case PROP_MODULE_TYPE: {
    AutoLockOper lock(module_lock_);
    int *module_type = static_cast<int*>(type);
    *module_type = module_data_info_->module_type;
    ret = YSOS_ERROR_SUCCESS;
    break;
  }
  case PROP_BUFFER_POOL: {
    AutoLockOper lock(module_lock_);
    BufferPoolInterfacePtr *buffer_pool = static_cast<BufferPoolInterfacePtr *>(type);
    *buffer_pool = buffer_pool_ptr_;
    ret = YSOS_ERROR_SUCCESS;
    break;
  }
  case PROP_NEXTCALLBACK_ERROR: {
    AutoLockOper lock(module_lock_);
    std::list<INT64> * owner_id_list = static_cast<std::list<INT64> *>(type);
    *owner_id_list = next_callback_error_list_;
    ret = YSOS_ERROR_SUCCESS;
    break;
  }
  case PROP_BUFFER_PREFIX: {
    //AutoLockOper lock(module_lock_);
    // not support
    break;
  }
  case PROP_MODULE_LINK: {
    AutoLockOper lock(module_lock_);
    std::string *module_link_name = static_cast<std::string*>(type);
    assert(NULL != module_link_name);
    *module_link_name = module_link_name_;
    ret = YSOS_ERROR_SUCCESS;
    break;
  }
  case PROP_MODULE_LINK_LEVEL: {
    AutoLockOper lock(module_lock_);
    uint32_t *module_link_level = static_cast<uint32_t*>(type);
    assert(NULL != module_link_level);
    *module_link_level = module_link_level_;
    ret = YSOS_ERROR_SUCCESS;
    break;
  }
  default:
    //YSOS_LOG_ERROR("not supported: " << type_id);
    ret = YSOS_ERROR_NOT_SUPPORTED;
    break;
  }

  return ret;
}

int BaseModuleImpl::SetProperty(int type_id, void *type) {
  if (NULL == type) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  int ret = YSOS_ERROR_NOT_SUPPORTED;
  switch (type_id) {
  case PROP_IN_DATA_TYPES: {
    AutoLockOper lock(module_lock_);
    CallbackDataType *data_type_ptr = static_cast<CallbackDataType*>(type);
    module_data_info_->in_datatypes = *data_type_ptr;
    ret = YSOS_ERROR_SUCCESS;
    break;
  }
  case PROP_OUT_DATA_TYPES: {
    AutoLockOper lock(module_lock_);
    CallbackDataType *data_type_ptr = static_cast<CallbackDataType*>(type);
    module_data_info_->out_datatypes = *data_type_ptr;
    ret = YSOS_ERROR_SUCCESS;
    break;
  }
  case PROP_SELF_ALLOCATOR: {
    AutoLockOper lock(module_lock_);
    bool *is_self_allocator = static_cast<bool*>(type);
    module_data_info_->is_self_allocator = *is_self_allocator;
    ret = YSOS_ERROR_SUCCESS;
    break;
  }
  case PROP_BUFFER_PREFIX_LENGTH: {
    AutoLockOper lock(module_lock_);
    uint32_t *prefix_length = static_cast<uint32_t*>(type);
    module_data_info_->prefix_length = *prefix_length;
    ret = YSOS_ERROR_SUCCESS;
    break;
  }
  case PROP_BUFFER_LENGTH: {
    AutoLockOper lock(module_lock_);
    uint32_t *buffer_length = static_cast<uint32_t*>(type);
    module_data_info_->buffer_length = *buffer_length;
    break;
  }
  case PROP_BUFFER_DATA_REVISE: {
    AutoLockOper lock(module_lock_);
    bool *is_data_revise = static_cast<bool*>(type);
    module_data_info_->is_data_revise = *is_data_revise;
    ret = YSOS_ERROR_SUCCESS;
    break;
  }
  case PROP_BUFFER_NUMBER: {
    AutoLockOper lock(module_lock_);
    uint32_t *buffer_number = static_cast<uint32_t*>(type);
    module_data_info_->buffer_number = *buffer_number;
    ret = YSOS_ERROR_SUCCESS;
    break;
  }
  case PROP_MAX_CAPBILITY: {
    AutoLockOper lock(module_lock_);
    uint32_t *max_capacity = static_cast<uint32_t*>(type);
    module_data_info_->max_capacity = *max_capacity;
    ret = YSOS_ERROR_SUCCESS;
    break;
  }
  case PROP_NEXTCALLBACKQUEUE: {
    AutoLockOper lock(module_lock_);
    std::cout<<" Not supported callbackqueue set\n";
    /*CallbackQueueInterface *callback_queue_ptr = static_cast<CallbackQueueInterface*>(type);
    pre_callback_queue_ = callback_queue_ptr;*/
    break;
  }
  case PROP_PREVCALLBACKQUEUE: {
    AutoLockOper lock(module_lock_);
    std::cout<<" Not supported callbackqueue set\n";
    /*CallbackQueueInterface *callback_queue_ptr = static_cast<CallbackQueueInterface*>(type);
    pre_callback_queue_ = callback_queue_ptr;*/
    break;
  }
  case PROP_STATE_COUNT: {
    AutoLockOper lock(module_lock_);
    UINT32 *state_count = static_cast<UINT32*>(type);
    module_run_state_count_ = *state_count;
    ret = YSOS_ERROR_SUCCESS;
    break;
  }
  case PROP_MODULE_TYPE: {
    AutoLockOper lock(module_lock_);
    int *module_type = static_cast<int*>(type);
    module_data_info_->module_type = *module_type;
    ret = YSOS_ERROR_SUCCESS;
    break;
  }
  case PROP_BUFFER_POOL: {
    /*AutoLockOper lock(module_lock_);
    BufferPoolInterfacePtr *buffer_pool = static_cast<BufferPoolInterfacePtr *>(type);
    *buffer_pool = buffer_pool_ptr_;*/
    std::cout<<" Not support BUFFE_POOL set\n";
    break;
  }
  case PROP_NEXTCALLBACK_ERROR: {
    AutoLockOper lock(module_lock_);
    INT64 owner_id = reinterpret_cast<INT64>(type);
    if (std::find(next_callback_error_list_.begin(),
                  next_callback_error_list_.end(), owner_id)
        == next_callback_error_list_.end()) {
      next_callback_error_list_.push_back(owner_id);
    }
    ret = YSOS_ERROR_SUCCESS;
    break;
  }
  //case PROP_BUFFER_PREFIX: {
  //  //AutoLockOper lock(module_lock_);
  //  BufferInterfacePtr *buffer_ptr = static_cast<BufferInterfacePtr *>(type);
  //  BufferInterfacePtr buffer = *buffer_ptr;
  //  uint32_t length = 0, prefix = 0;
  //  if (NULL == buffer) {
  //    ret = YSOS_ERROR_INVALID_ARGUMENTS;
  //    break;
  //  }
  //  assert(YSOS_ERROR_SUCCESS == buffer->GetBufferAndLength(NULL, &length));
  //  assert(YSOS_ERROR_SUCCESS == buffer->GetPrefixLength(&prefix));
  //  if (0 != prefix) {
  //    // length should not be less than orignal value
  //    // This should be guaranteed by module(link)
  //    length += prefix - module_data_info_->prefix_length;
  //  }
  //  buffer->SetLength(length, module_data_info_->prefix_length);
  //  ret = YSOS_ERROR_SUCCESS;
  //  break;
  //}
  case PROP_MODULE_LINK: {
    AutoLockOper lock(module_lock_);
    std::string *module_link_name = static_cast<std::string*>(type);
    assert(NULL !=module_link_name);
    module_link_name_ = *module_link_name;
    ret = YSOS_ERROR_SUCCESS;
    break;
  }
  case PROP_MODULE_LINK_LEVEL: {
    AutoLockOper lock(module_lock_);
    uint32_t *module_link_level = static_cast<uint32_t*>(type);
    assert(NULL != module_link_level);
    module_link_level_ = *module_link_level;
    ret = YSOS_ERROR_SUCCESS;
    break;
  }
  case PROP_STATUS_EVENT: {
    StatusEventInfo *event_info = reinterpret_cast<StatusEventInfo*>(type);
    if (NULL == event_info) {
      YSOS_LOG_ERROR("event info param is null");
      ret = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }
    ret = SendStatusEventMessage(event_info->code, event_info->detail);
    break;
  }
  case PROP_LOCK_MODULE: {
    module_lock_->Lock();
    ret = YSOS_ERROR_SUCCESS;
    break;
  }
  case PROP_UNLOCK_MODULE: {
    module_lock_->Unlock();
    ret = YSOS_ERROR_SUCCESS;
    break;
  }

  default:
    YSOS_LOG_ERROR("not supported: " << type_id);
    ret = YSOS_ERROR_NOT_SUPPORTED;
    break;
  }

  return ret;
}

int BaseModuleImpl::AllocateBufferPool(void) {
  if (!module_data_info_->is_self_allocator) {
    YSOS_LOG_DEBUG(GetName() << " doesn't need to allocater buffer pool.");

    return YSOS_ERROR_SUCCESS;
  }

  AutoLockOper lock(module_lock_);
  AllocatorProperties allocate_properties;
  allocate_properties.cbAlign = 0;
  allocate_properties.cbBuffer = module_data_info_->buffer_length;
  allocate_properties.cbPrefix = module_data_info_->prefix_length;
  allocate_properties.cBuffers = module_data_info_->buffer_number;

  AllocatorProperties actual_allocate_properties;
  buffer_pool_ptr_ = BufferPoolInterfacePtr(new BufferPoolImpl<BufferImpl>());
  int ret = buffer_pool_ptr_->SetProperties(&allocate_properties, &actual_allocate_properties);
  assert(YSOS_ERROR_SUCCESS == ret);
  ret = buffer_pool_ptr_->Commit();
  assert(YSOS_ERROR_SUCCESS == ret);

  return YSOS_ERROR_SUCCESS;
}

BaseModuleImpl::ModuleStatInfoPtr BaseModuleImpl::FindNextModuleStatInfoByOwnerId(const INT64 &owner_id) {
  for (ModuleStatInfoIterator it=module_stat_info_map_.begin(); it!=module_stat_info_map_.end(); ++it) {
    if (it->first == owner_id) {
      return it->second;
    }
  }

  return ModuleStatInfoPtr();
}

int BaseModuleImpl::UpdateSpecifyNextModuleStatByOwnerId(const INT64 &owner_id, int stat) {
  ModuleStatInfoPtr mode_stat_ptr = FindNextModuleStatInfoByOwnerId(owner_id);
  if (NULL == mode_stat_ptr) {
    return YSOS_ERROR_NOT_EXISTED;
  }

  mode_stat_ptr->stat = stat;

  return YSOS_ERROR_SUCCESS;
}

int  BaseModuleImpl::ComputeModuleLinkCurStat(void) {
  int cur_stat = PROP_CLOSE;
  for (ModuleStatInfoIterator it=module_stat_info_map_.begin(); it!=module_stat_info_map_.end(); ++it) {
    BaseModuleImpl *next_module = reinterpret_cast<BaseModuleImpl*>(it->second->owner_id);
    assert(NULL != next_module);
    int next_module_state = PROP_CLOSE;
    int ret = next_module->GetState(0, (INT32*)&next_module_state, this);
    assert(YSOS_ERROR_SUCCESS == ret);
    it->second->stat = static_cast<int>(next_module_state);

    /// 去除Repeater的影响
    {
      if (next_module->module_data_info_->module_type == PROP_REPEATER) {
        continue;
      }
    }

    // PROP_CLOSE < PROP_STOP < PROP_PAUSE < PROP_RUN
    if (it->second->stat > cur_stat) {
      cur_stat = it->second->stat;
    }
  }

  return cur_stat;
}

int BaseModuleImpl::UpdateModuleStat(int &cur_stat, int &dst_stat, LPVOID param) {
  if (cur_stat == dst_stat) {
    return YSOS_ERROR_SUCCESS;
  }

  int ret = YSOS_ERROR_SUCCESS;
  if (cur_stat == PROP_CLOSE && dst_stat == PROP_STOP) {
    ret = RealOpen(param);
    YSOS_LOG_DEBUG("module has Opened " << ret);
  } else if (cur_stat == PROP_STOP && dst_stat == PROP_PAUSE) {
    ret = RealPause();
    YSOS_LOG_DEBUG("module has Paused " << ret);
  } else if (cur_stat == PROP_STOP && dst_stat == PROP_RUN) {
    ret = RealRun();
    YSOS_LOG_DEBUG("module has Runned " << ret);
  } else if (cur_stat == PROP_STOP && dst_stat == PROP_CLOSE) {
    ret = RealClose();
    YSOS_LOG_DEBUG("module has Closed " << ret);
  } else if (cur_stat == PROP_PAUSE && dst_stat == PROP_RUN) {
    ret = RealRun();
    YSOS_LOG_DEBUG("module has Runned " << ret);
  } else if (cur_stat == PROP_PAUSE && dst_stat == PROP_STOP) {
    ret = RealStop();
    YSOS_LOG_DEBUG("module has Stopped " << ret);
  } else if (cur_stat == PROP_RUN && dst_stat == PROP_PAUSE) {
    ret = RealPause();
    YSOS_LOG_DEBUG("module has Paused " << ret);
  } else if (cur_stat == PROP_RUN && dst_stat == PROP_STOP) {
    ret = RealStop();
    YSOS_LOG_DEBUG("module has Stopped " << ret);
  } else {
    YSOS_LOG_DEBUG("unsupported stat changed: " << cur_stat << ":" << dst_stat);
    ret = YSOS_ERROR_LOGIC_ERROR;
  }

  if (ret != YSOS_ERROR_SUCCESS) {
    /*YSOS_LOG_DEBUG("Change stat failed: " << cur_stat << ":" << dst_stat);*/
    YSOS_LOG_ERROR("UpdateModuleStat failed: " << cur_stat << " to " << dst_stat);
  } else {
    cur_stat = dst_stat;
    YSOS_LOG_DEBUG("UpdateModuleStat successed: " << cur_stat << " to " << dst_stat);
    //UpdateSpecifyNextModuleStatByOwnerId(0, cur_stat);
  }

  return ret;
}

int BaseModuleImpl::InsertModuleStatRecord(INT64 owner_id, const int mode_stat) {
  ModuleStatInfoPtr module_stat = boost::make_shared<ModuleStatInfo>();
  module_stat->owner_id = owner_id;
  module_stat->stat = mode_stat;
  ModuleInterface *next_module_ptr = reinterpret_cast<ModuleInterface*>(owner_id);
  if (module_stat_info_map_.end() != module_stat_info_map_.find(owner_id)) {
    YSOS_LOG_DEBUG("record already exist: " << owner_id);
    YSOS_LOG_DEBUG(logic_name_ << " ********** record already exist: " << next_module_ptr->GetName());
    return YSOS_ERROR_HAS_EXISTED;
  }

  module_stat_info_map_.insert(std::make_pair(owner_id, module_stat));
  YSOS_LOG_DEBUG(logic_name_ << " ********** add new record : " << next_module_ptr->GetName());
  return YSOS_ERROR_SUCCESS;
}

int BaseModuleImpl::RemoveModuleStatRecord(INT64 owner_id) {
  ModuleInterface *next_module_ptr = reinterpret_cast<ModuleInterface*>(owner_id);
  ModuleStatInfoIterator it = module_stat_info_map_.find(owner_id);
  if (it != module_stat_info_map_.end()) {
    module_stat_info_map_.erase(it);
    YSOS_LOG_DEBUG(logic_name_ << " ********** remove record: " << next_module_ptr->GetName());

    return YSOS_ERROR_SUCCESS;
  }

  YSOS_LOG_DEBUG(logic_name_ << " ********** record not exist: " << next_module_ptr->GetName());
  return YSOS_ERROR_NOT_EXISTED;
}

int BaseModuleImpl::IteratorNextCallback(BaseModuleCallbackFunction &function_ptr) {
  AutoLockOper lock(next_callback_queue_lock_);
  CallbackInterfacePtr callback = next_callback_queue_->FindCallback(true);
  int ret = YSOS_ERROR_SUCCESS;
  //YSOS_LOG << "************8 Begin to IteratorNextCallback: " << GetName() << std::endl;
  while (NULL != callback) {
    //YSOS_LOG << "************* callback Name is: " << callback->GetName() << std::endl;
    ret = function_ptr(callback, this);
    if (YSOS_ERROR_PASS == ret) {
      // YSOS_LOG_DEBUG("BaseModuleImpl::IteratorNextCallback ret is [YSOS_ERROR_PASS]");
      break;
    }

    callback = next_callback_queue_->FindCallback();
  }

  //YSOS_LOG << "************* End IteratorNextCallback: " << GetName() << std::endl;
  return ret;
}

int BaseModuleImpl::IteratorPrevCallback(BaseModuleCallbackFunction &function_ptr) {
  AutoLockOper lock(pre_callback_queue_lock_);
  CallbackInterfacePtr callback = pre_callback_queue_->FindCallback(true);
  int ret = YSOS_ERROR_SUCCESS;
  while (NULL != callback) {
    ret = function_ptr(callback, this);
    if (YSOS_ERROR_PASS == ret) {
      YSOS_LOG_DEBUG("BaseModuleImpl::IteratorPrevCallback ret is [YSOS_ERROR_PASS]");
      break;
    }

    callback = pre_callback_queue_->FindCallback();
  }
  return ret;
}

BufferInterfacePtr BaseModuleImpl::WrapStatusEventMessage(const std::string &status_event_code, const std::string &module_name, const std::string &detail) {
  if (status_event_code.empty()) {
    YSOS_LOG_ERROR("status code is empty " << logic_name_);
    return NULL;
  }

  JsonValue json_value_result;
  json_value_result["type"] = "status_event";
  json_value_result["status_event_code"] = status_event_code.c_str();
  json_value_result["service_name"] = logic_name_.c_str();
  json_value_result["detail"] = detail.c_str();

  std::string json_str;
  int ret = GetJsonUtil()->JsonValueToString(json_value_result, json_str);
  if (YSOS_ERROR_SUCCESS != ret) {
    YSOS_LOG_ERROR("status event message wrap to json failed: " << status_event_code << " | " << logic_name_ << " | " << detail);
    return NULL;
  }

  BufferInterfacePtr buffer_ptr = GetBufferUtility()->CreateBuffer(json_str);
  if (NULL == buffer_ptr) {
    YSOS_LOG_ERROR("status event message wrap to buffer failed: " << status_event_code << " | " << logic_name_ << " | " << detail);
    return NULL;
  }

  return buffer_ptr;
}

int BaseModuleImpl::SendStatusEventMessage(const std::string &status_event_code, const std::string &detail) {
  BufferInterfacePtr buffer_ptr = WrapStatusEventMessage(status_event_code, "", detail);
  if (NULL == buffer_ptr) {
    YSOS_LOG_ERROR("status event message wrap to buffer failed: " << status_event_code << " | " << logic_name_ << " | " << detail);
    return YSOS_ERROR_FAILED;
  }

  BaseModuelCallbackContext callback_contex;

  callback_contex.prev_callback_ptr = NULL;
  callback_contex.prev_module_ptr = NULL;
  callback_contex.cur_callback_ptr = status_event_callback_ptr_.get();
  callback_contex.cur_module_ptr = this;

  YSOS_LOG_DEBUG("begin to send status event message: " << status_event_code << " | " << detail << " | " << logic_name_);
  return status_event_callback_ptr_->Callback(buffer_ptr, NULL, &callback_contex);
}

int BaseModuleImpl::NotifyStatusEventMessage(const std::string &status_event_code, const std::string &module_name, const std::string &detail) {
  if (NULL == status_notify_callback_ptr_) {
    YSOS_LOG_ERROR("status notify callback is null: " << status_event_code<< " | " << module_name << " | " << detail << " | " << logic_name_);
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  BufferInterfacePtr buffer_ptr = WrapStatusEventMessage(status_event_code, module_name, detail);
  if (NULL == buffer_ptr) {
    YSOS_LOG_ERROR("status event message wrap to buffer failed: " << status_event_code << " | " << logic_name_ << " | " << detail);
    return YSOS_ERROR_FAILED;
  }

  uint32_t message_id = GetPlatformInfo()->GetMessageIdByName("status_event");
  YSOS_LOG_DEBUG("message_id: " << message_id << " | " << logic_name_);
  if (0 == message_id) {
    YSOS_LOG_ERROR("message id is not exist: " << "status_event" << " | " << logic_name_);
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  MsgInterfacePtr msg_ptr = MsgInterfacePtr(new MsgImpl(message_id, buffer_ptr));
  if (NULL == msg_ptr) {
    YSOS_LOG_ERROR("allocate msg failed: " << status_event_code << " | " << module_name << " | " << detail << " | " << logic_name_);
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  int ret = status_notify_callback_ptr_->Callback(msg_ptr, NULL, NULL);

  return ret;
}

int BaseModuleImpl::SetStatusEventCallback(DriverInterfacePtr driver_ptr_) {
  if (NULL == driver_ptr_) {
    YSOS_LOG_ERROR("driver is null");
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  typedef boost::function<int(const std::string&, const std::string &)> status_event_fun;
  status_event_fun status_callback = boost::bind(&BaseModuleImpl::SendStatusEventMessage, this, _1, _2);
  driver_ptr_->SetProperty(PROP_STATUS_EVENT, &status_callback);

  typedef boost::function<int(const std::string&, const std::string &, const std::string &)> notify_event_fun;
  notify_event_fun status_notify_callback = boost::bind(&BaseModuleImpl::NotifyStatusEventMessage, this, _1, _2, _3);

  return driver_ptr_->SetProperty(PROP_NOTIFY_STATUS_EVENT, &status_notify_callback);
}

void BaseModuleImpl::DumpPreCallback() {
  AutoLockOper lock(pre_callback_queue_lock_);
  CallbackInterfacePtr callback = pre_callback_queue_->FindCallback(true);
  if (NULL == callback) {
    YSOS_LOG_DEBUG("Prev Callback Name: (None)");
  }
  while (NULL != callback) {
    YSOS_LOG_DEBUG("Prev Callback Name: " << callback->GetName());
    callback = pre_callback_queue_->FindCallback();
  }
}

void BaseModuleImpl::DumpNextCallback() {
  AutoLockOper lock(next_callback_queue_lock_);
  CallbackInterfacePtr callback = next_callback_queue_->FindCallback(true);
  if (NULL == callback) {
    YSOS_LOG_DEBUG("Next Callback Name: (None)");
  }
  while (NULL != callback) {
    YSOS_LOG_DEBUG("Next Callback Name: " << callback->GetName());
    callback = next_callback_queue_->FindCallback();
  }
}
}
