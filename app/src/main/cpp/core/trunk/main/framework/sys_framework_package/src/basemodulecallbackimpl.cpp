/**
 *@file BaseCallbackImpl.cpp
 *@brief Definition of BaseCallbackImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

#ifdef _WIN32
  #include <string>
#else
  #include <cstring>
#endif

/// ysos private headers //  NOLINT
#include "../../../protect/include/sys_framework_package/basemodulecallbackimpl.h"  // NOLINT
#include "../../../public/include/core_help_package/multimap.h"
#include "../../../public/include/sys_interface_package/bufferpool.h"
#include "../../../protect/include/sys_framework_package/basemoduleimpl.h"
// #include "../../../public/include/core_help_package/ysos_log.h"
#include "../../../public/include/core_help_package/utility.h"
#include "../../../protect/include/sys_framework_package/packageconfigimpl.h"

namespace ysos {
BaseModuleCallbackImpl::BaseModuleCallbackImpl(const std::string& name): BaseCallbackImpl(name) {  // NOLINT
  logger_ = GetUtility()->GetLogger("ysos.plugin");

  input_type_ = output_type_ = CallbackDataTypeAll;
}

BaseModuleCallbackImpl::~BaseModuleCallbackImpl() {
}

int BaseModuleCallbackImpl::Initialize(void *param) {
  if (IsInitialized()) {
    return YSOS_ERROR_FINISH_ALREADY;
  }

  return BaseCallbackImpl::Initialize(param);
}

int BaseModuleCallbackImpl::AllocateBuffer(BaseModuleImpl *module_ptr, BufferInterfacePtr *buffer_ptr) {
  assert(NULL != module_ptr);
  assert(NULL != buffer_ptr);

  bool is_allocated = false;
  int ret = module_ptr->GetProperty(PROP_SELF_ALLOCATOR, &is_allocated);
  assert(YSOS_ERROR_SUCCESS == ret);
  if (!is_allocated) {
    // YSOS_LOG_DEBUG("Cur module does not support allocat buffer");
    return YSOS_ERROR_SUCCESS;
  }

  BufferPoolInterfacePtr buffer_pool_ptr;
  ret = module_ptr->GetProperty(PROP_BUFFER_POOL, &buffer_pool_ptr);
  assert(YSOS_ERROR_SUCCESS == ret);
  assert(NULL != buffer_pool_ptr);

  ret = buffer_pool_ptr->GetBuffer(buffer_ptr);
  if (YSOS_ERROR_SUCCESS != ret) {
    return ret;
  }
  int32_t buffer_length = 0, prefix_length = 0;
  ret = module_ptr->GetProperty(PROP_BUFFER_LENGTH, &buffer_length);
  assert(YSOS_ERROR_SUCCESS == ret);
  ret = module_ptr->GetProperty(PROP_BUFFER_PREFIX_LENGTH, &prefix_length);
  assert(YSOS_ERROR_SUCCESS == ret);

  return (*buffer_ptr)->SetLength(buffer_length, prefix_length);
}

int BaseModuleCallbackImpl::ReleaseBuffer(BaseModuleImpl *module_ptr, BufferInterfacePtr buffer_ptr) {
  assert(NULL != module_ptr);
  // NULL,不需要释放
  if (NULL == buffer_ptr) {
    return YSOS_ERROR_SUCCESS;
  }

  BufferPoolInterfacePtr buffer_pool_ptr;
  int ret = module_ptr->GetProperty(PROP_BUFFER_POOL, &buffer_pool_ptr);
  assert(YSOS_ERROR_SUCCESS == ret);
  if (NULL == buffer_pool_ptr) {
    return YSOS_ERROR_SUCCESS;
  }

  return buffer_pool_ptr->ReleaseBuffer(buffer_ptr);
}

BaseModuleImpl *BaseModuleCallbackImpl::GetCurModule(void *context) {
  BaseModuelCallbackContext *callback_context = static_cast<BaseModuelCallbackContext*>(context);
  assert(NULL != callback_context);

  return dynamic_cast<BaseModuleImpl*>(callback_context->cur_module_ptr);
}

BaseModuleImpl *BaseModuleCallbackImpl::GetPrevModule(void *context) {
  BaseModuelCallbackContext *callback_context = static_cast<BaseModuelCallbackContext*>(context);
  assert(NULL != callback_context);

  return dynamic_cast<BaseModuleImpl*>(callback_context->prev_module_ptr);
}

CallbackInterface *BaseModuleCallbackImpl::GetPrevCallback(void *context) {
  BaseModuelCallbackContext *callback_context = static_cast<BaseModuelCallbackContext*>(context);
  assert(NULL != callback_context);

  return dynamic_cast<CallbackInterface*>(callback_context->prev_callback_ptr);
}

/**
 *  当该Callback是NextCallbackQueue中的成员是，context参数不应为NULL。
 *  它应是Callback对应的module(next module)指针。
 *  这个Callback相当于该module的运行函数:
 *  先执行preCallback,再执行nextCallback
 */
int BaseModuleCallbackImpl::Callback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, void *context) {
  YSOS_LOG_DEBUG("enter callback: " << logic_name_);
  BaseModuelCallbackContext *callback_context = static_cast<BaseModuelCallbackContext*>(context);
  if (NULL == callback_context) {
    YSOS_LOG_ERROR("context is null: " << logic_name_);
    YSOS_LOG_DEBUG("leave callback: " << logic_name_);
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  callback_context->cur_callback_ptr = this;
  /// 获取module_ptr
  BaseModuleImpl* module_ptr = dynamic_cast<BaseModuleImpl*>(callback_context->cur_module_ptr);
  if (NULL == module_ptr) {
    YSOS_LOG_ERROR("module is null: " << logic_name_);
    YSOS_LOG_DEBUG("leave callback: " << logic_name_);
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  /// invoke before IsCurModuleRun
  int lock_ret = module_ptr->SetProperty(PROP_LOCK_MODULE, module_ptr);
  assert(0 == lock_ret);

  ModuleInterface* prev_module_ptr = dynamic_cast<BaseModuleImpl*>(callback_context->prev_module_ptr);
  bool is_run = IsCurModuleRun(module_ptr, prev_module_ptr);
  if (!is_run) {
    lock_ret = module_ptr->SetProperty(PROP_UNLOCK_MODULE, module_ptr);
    assert(0 == lock_ret);
    YSOS_LOG_DEBUG("module is not run: " << logic_name_);
    YSOS_LOG_DEBUG("leave callback: " << logic_name_);
    return YSOS_ERROR_SUCCESS;
  }

  // data_flow和ctrol_flow同步在这儿完成
  module_ptr->UpdateTeamParam();

  // 分配Buffer，作为RealCallback的输出，同时也作为NextCallback的输入
  BufferInterfacePtr buffer_ptr;
  int ret = AllocateBuffer(module_ptr, &buffer_ptr);
  if (YSOS_ERROR_SUCCESS != ret) {
    YSOS_LOG_ERROR("allocate buffer failed: " << ret << " | " << logic_name_);
    lock_ret = module_ptr->SetProperty(PROP_UNLOCK_MODULE, module_ptr);
    assert(0 == lock_ret);
    boost::this_thread::sleep_for(boost::chrono::milliseconds(30));
    YSOS_LOG_DEBUG("leave callback: " << logic_name_);
    return ret;
  }
  // YSOS_LOG_DEBUG("before real callback");
  // 真正的回调函数
  YSOS_LOG_DEBUG("********************* begin to realcallback: " << logic_name_);
  ret = RealCallback(input_buffer, buffer_ptr, output_buffer, context);
  YSOS_LOG_DEBUG("********************* end to realcallback: " << logic_name_);

  /// invoke immediately after realcallback
  lock_ret = module_ptr->SetProperty(PROP_UNLOCK_MODULE, module_ptr);
  assert(0 == lock_ret);
  YSOS_LOG_DEBUG("********************* end to realcallback: lock2 " << logic_name_);

  if (YSOS_ERROR_SUCCESS != ret) {
    ReleaseBuffer(module_ptr, buffer_ptr);
    YSOS_LOG_DEBUG("real callback ret is " << ret << " | " << logic_name_);
    YSOS_LOG_DEBUG("leave callback: " << logic_name_);
    return ret;
  }

  // 调用NextCallback,如果buffer_ptr是NULL，说明当前Module不需要分配Buffer，输入和输出用一个Buffer
  ret = InvokeNextCallback((NULL == buffer_ptr ? input_buffer : buffer_ptr), output_buffer, context);
  ReleaseBuffer(module_ptr, buffer_ptr);
  if (YSOS_ERROR_PASS == ret) {
    YSOS_LOG_DEBUG("Leave BaseModuleCallbackImpl::Callback return is [YSOS_ERROR_PASS]");
  } else {
    // YSOS_LOG_DEBUG("Leave BaseModuleCallbackImpl::Callback return is " << ret);
  }
  // YSOS_LOG_DEBUG("Leave BaseModuleCallbackImpl::Callback, context is : " << module_ptr->GetName());
  YSOS_LOG_DEBUG("leave callback: " << logic_name_);

  return ret;
}

bool BaseModuleCallbackImpl::IsCurModuleRun(ModuleInterface *cur_module, ModuleInterface *prev_module_ptr) {
  if (NULL == cur_module) {
    return false;
  }

  int cur_state = 0;
  int ret = cur_module->GetState(0, (INT32*)&cur_state, prev_module_ptr);
  if (YSOS_ERROR_SUCCESS != ret) {
    YSOS_LOG_INFO("BaseModuleCallbackImpl::IsCurModuleRun false: " << cur_state << " | " << logic_name_ << " | " << cur_module->GetName());
    return false;
  }
  //YSOS_LOG_INFO("BaseModuleCallbackImpl::IsCurModuleRun: " << cur_state << " | " << logic_name_ << " | " << cur_module->GetName());
  if (PROP_RUN != cur_state) {
    // YSOS_LOG_DEBUG("state is not in run : " << cur_state);
    return false;
  }

  return true;
}

int BaseModuleCallbackImpl::InvokeNextCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, void *context) {
  BaseModuelCallbackContext *callback_context = static_cast<BaseModuelCallbackContext*>(context);
  if (NULL == callback_context) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }
  /// 获取module_ptr
  BaseModuleImpl* module_ptr = reinterpret_cast<BaseModuleImpl*>(callback_context->cur_module_ptr);
  bool is_run = IsCurModuleRun(module_ptr, GetPrevModule(context));
  if (!is_run) {
    return YSOS_ERROR_SUCCESS;
  }

  BaseModuleCallbackFunction base_module_callback_function(input_buffer, output_buffer, context);
  return module_ptr->IteratorNextCallback(base_module_callback_function);
}

int BaseModuleCallbackImpl::InvokeNextCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer,
    CallbackInterfacePtr callback, void *context) {
  if (NULL == callback || NULL == context) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  BaseModuelCallbackContext *callback_context = static_cast<BaseModuelCallbackContext*>(context);
  if (NULL == callback_context) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  CallbackDataType input_type, output_type;
  int ret = callback->GetType(&input_type, &output_type);
  assert(YSOS_ERROR_SUCCESS == ret);
  /// 当前Callback的输出数据类型要和NextCallback的输入数据类型匹配//NOLINT
  /// 2016-07-21, 忽略大小写比较//NOLINT
  //need update for linux  //'stricmp'=='strcasecmp' IGNORE CASE|LOWER
  #ifdef _WIN32
    if (0 != stricmp(output_type_.c_str(), CallbackDataTypeAll) &&0 != stricmp(input_type.c_str(), output_type_.c_str()) && 0 != stricmp(input_type.c_str(), CallbackDataTypeAll)) {
      return YSOS_ERROR_INVALID_ARGUMENTS;
    }
  #else
    if (0 != strcasecmp(output_type_.c_str(), CallbackDataTypeAll) &&0 != strcasecmp(input_type.c_str(), output_type_.c_str()) && 0 != strcasecmp(input_type.c_str(), CallbackDataTypeAll)) {
      return YSOS_ERROR_INVALID_ARGUMENTS;
    }
  #endif
  YSOS_LOG_DEBUG("searching for next callback: " << output_type_ << "|" << input_type << "|" << logic_name_ << "|" << callback->GetName());

  ModuleInterface *cur_module_ptr = callback_context->cur_module_ptr;
  INT64 cur_module_id = (INT64)(cur_module_ptr);

  BaseModuleCallbackImplMap *owner_id_map = NULL;
  ret = callback->GetProperty(PROP_OWNER_ID, &owner_id_map);
  assert(YSOS_ERROR_SUCCESS == ret);

  for (BaseModuleCallbackImplIterator it=owner_id_map->begin(); it!=owner_id_map->end(); ++it) {
    /// 这个主要目的，是取得NextModule的owner_id //  NOLINT
    if (it->first != cur_module_id) {
      continue;
    }
    if (YSOS_ERROR_SUCCESS != callback->IsReady()) {
      cur_module_ptr->SetProperty(PROP_NEXTCALLBACK_ERROR, (void*)(it->second));
      continue;
    }
    BaseModuelCallbackContext *next_callback_contex = new BaseModuelCallbackContext();
    if (NULL == next_callback_contex) {
      return YSOS_ERROR_OUTOFMEMORY;
    }

    next_callback_contex->prev_callback_ptr = this;
    next_callback_contex->prev_module_ptr = cur_module_ptr;
    next_callback_contex->cur_callback_ptr = callback.get();
    next_callback_contex->cur_module_ptr = reinterpret_cast<ModuleInterface*>(it->second);
    ret = callback->Callback(input_buffer, output_buffer, (void*)next_callback_contex);
    if (YSOS_ERROR_PASS == ret) {
      break;
    }
    if (YSOS_ERROR_SUCCESS != ret && YSOS_ERROR_SKIP != ret && YSOS_ERROR_PASS != ret) {
      cur_module_ptr->SetProperty(PROP_NEXTCALLBACK_ERROR, (void*)(it->second));
    }
    YSOS_DELETE(next_callback_contex);
    // Callback已经执行了，不需要循环了   //  NOLINT
    break;
  }

  return ret;
}
int BaseModuleCallbackImpl::SetType(const CallbackDataType input_type, const CallbackDataType output_type) {  // NOLINT
  input_type_ = input_type;
  output_type_ = output_type;

  return YSOS_ERROR_SUCCESS;
}

int BaseModuleCallbackImpl::GetType(CallbackDataType *input_type, CallbackDataType *output_type) {
  if (NULL != input_type) {
    *input_type = input_type_;
  }

  if (NULL != output_type) {
    *output_type = output_type_;
  }

  return YSOS_ERROR_SUCCESS;
}

int BaseModuleCallbackImpl::IsReady() {
  return YSOS_ERROR_SUCCESS;
}

CallbackIODataTypePair BaseModuleCallbackImpl::GetUniqueKey(void) {
  return std::make_pair(input_type_, output_type_);
}

int BaseModuleCallbackImpl::GetProperty(int type_id, void *type) {
  switch (type_id) {
  case PROP_OWNER_ID: {
    BaseModuleCallbackImplMap **owner_id_map
      = static_cast<BaseModuleCallbackImplMap**>(type);
    *owner_id_map = &owner_id_map_;
    break;
  }
  case PROP_UNIQUE_KEY: {    //  UNIQUE_KEY
    CallbackIODataTypePair *key
      = static_cast<CallbackIODataTypePair*>(type);
    CallbackIODataTypePair new_key = GetUniqueKey();
    key->first = new_key.first;
    key->second = new_key.second;
    break;
  }
  default:
    return BaseCallbackImpl::GetProperty(type_id, type);
  }

  return YSOS_ERROR_SUCCESS;
}

int BaseModuleCallbackImpl::SetProperty(int type_id, void *type) {
  switch (type_id) {
  /**  MB、MB'和MB''是Module MB的三个对象，CallackB是回调函数CallbackB的对象，
   *   CallbackB同时属于MB、MB'和MB''
   *   MA是MB、MB'和MB''的前链接，如下图示：
   *          ----> MB
   *    MA    ----> MB'
   *          ----> MB''
   *    则CallbackB是MA的NextCallback，当MA调用到CallbackB的时候，
   *    MA希望依次调用MB、MB'和MB''的CallbackB
   *    在owner_id_map_中，MA为Key，MB、MB'和MB''分别是Value
   */
  case PROP_OWNER_ID: {
    std::pair<INT64, INT64> *owner_id_pair
      = reinterpret_cast<std::pair<INT64, INT64> *>(type);
    if (std::find_if(
          owner_id_map_.begin(),
          owner_id_map_.end(),
          MapCondition<std::pair<INT64, INT64>>(*owner_id_pair))
        == owner_id_map_.end()) {
      owner_id_map_.insert(*owner_id_pair);
    }

    break;
  }
  default:
    break;
  }

  return YSOS_ERROR_SUCCESS;
}
}  // NOLINT
