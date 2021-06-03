/**
 *@file ModuleUtil.cpp
 *@brief Helpers for module and module link
 *@version 0.1
 *@author XuLanyue
 *@date Created on: 2016-06-13 21:30:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

#include "../../../protect/include/sys_framework_package/moduleutil.h"

#include "../../../public/include/sys_interface_package/baseinterface.h"
#include "../../../public/include/sys_interface_package/moduleinterface.h"
#include "../../../public/include/sys_interface_package/callbackqueue.h"
// #include "../../../public/include/core_help_package/ysos_log.h"
#include "../../../protect/include/sys_framework_package/basemoduleimpl.h"
#include "../../../protect/include/sys_framework_package/modulelinkimpl.h"
#include "../../../protect/include/sys_framework_package/basemodulecallbackimpl.h"
#include "../../../protect/include/sys_framework_package/baserepeatercallbackimpl.h"
#include "../../../protect/include/sys_framework_package/precallbackqueue.h"
#include "../../../protect/include/sys_framework_package/nextcallbackqueue.h"
#include "../../../public/include/core_help_package/utility.h"
#include <boost/make_shared.hpp>

namespace ysos {

bool GetModuleProperty(
  const ModuleInterfacePtr &module_ptr,
  ModuleInterface::ModuleDataInfo &module_properties) {
    bool ret_value = true;
    ModuleInterface::ModuleDataInfo module_properties_temp;

    //  Type
    if (ret_value && module_ptr
      && YSOS_ERROR_SUCCESS == module_ptr->GetProperty(
      PROP_MODULE_TYPE, (void*)&module_properties_temp.module_type)) {
        module_properties.module_type = module_properties_temp.module_type;
        ret_value = true;
    }

    //  In_DataType
    if (ret_value && module_ptr
      && YSOS_ERROR_SUCCESS == module_ptr->GetProperty(
      PROP_IN_DATA_TYPES, (void*)&module_properties_temp.in_datatypes)) {
        module_properties.in_datatypes = module_properties_temp.in_datatypes;
        ret_value = true;
    }

    //  Out_DataType
    if (ret_value && module_ptr
      && YSOS_ERROR_SUCCESS == module_ptr->GetProperty(
      PROP_OUT_DATA_TYPES, (void*)&module_properties_temp.out_datatypes)) {
        module_properties.out_datatypes = module_properties_temp.out_datatypes;
        ret_value = true;
    }

    //  Buffer_Prefix_Length
    if (ret_value && module_ptr
      && YSOS_ERROR_SUCCESS == module_ptr->GetProperty(
      PROP_BUFFER_PREFIX_LENGTH, (void*)&module_properties_temp.prefix_length)) {
        module_properties.prefix_length = module_properties_temp.prefix_length;
        ret_value = true;
    }

    //  Buffer_Length
    if (ret_value && module_ptr
      && YSOS_ERROR_SUCCESS == module_ptr->GetProperty(
      PROP_BUFFER_LENGTH, (void*)&module_properties_temp.buffer_length)) {
        module_properties.buffer_length = module_properties_temp.buffer_length;
        ret_value = true;
    }

    //  Buffer_Number
    if (ret_value && module_ptr
      && YSOS_ERROR_SUCCESS == module_ptr->GetProperty(
      PROP_BUFFER_NUMBER, (void*)&module_properties_temp.buffer_number)) {
        module_properties.buffer_number = module_properties_temp.buffer_number;
        ret_value = true;
    }

    //  Buffer_Data_Revise
    if (ret_value && module_ptr
      && YSOS_ERROR_SUCCESS == module_ptr->GetProperty(
      PROP_BUFFER_DATA_REVISE, (void*)&module_properties_temp.is_data_revise)) {
        module_properties.is_data_revise = module_properties_temp.is_data_revise;
        ret_value = true;
    }

    //  Self_Allocator
    if (ret_value && module_ptr
      && YSOS_ERROR_SUCCESS == module_ptr->GetProperty(
      PROP_SELF_ALLOCATOR, (void*)&module_properties_temp.is_self_allocator)) {
        module_properties.is_self_allocator = module_properties_temp.is_self_allocator;
        ret_value = true;
    }

    //  Max_Capbility
    if (ret_value && module_ptr
      && YSOS_ERROR_SUCCESS == module_ptr->GetProperty(
      PROP_MAX_CAPBILITY, (void*)&module_properties_temp.max_capacity)) {
        module_properties.max_capacity = module_properties_temp.max_capacity;
        ret_value = true;
    }

    return ret_value;
}

bool GetModuleDataType(
  const ModuleInterfacePtr &module_ptr,
  CallbackDataType &module_data_type,
  bool for_input_data_type) {
  bool ret_value = false;
  CallbackDataType data_type;

  if (module_ptr && YSOS_ERROR_SUCCESS == module_ptr->GetProperty(
    (for_input_data_type ? PROP_IN_DATA_TYPES : PROP_OUT_DATA_TYPES),
    (void*)&data_type)) {
      module_data_type = data_type;
      ret_value = true;
  }

  return ret_value;
}

bool GetModuleType(
  const ModuleInterfacePtr &module_ptr,
  int &module_type) {
    bool ret_value = false;
    ModuleInterface::ModuleDataInfo module_properties;
    if (module_ptr
      && YSOS_ERROR_SUCCESS == module_ptr->GetProperty(
      PROP_MODULE_TYPE, (void*)&module_properties.module_type)) {
        module_type = (int)module_properties.module_type;
        ret_value = true;
    }
    return ret_value;
}

bool IsSourceModule(const ModuleInterfacePtr &module_ptr) {
  bool ret_value = false;
  int module_type = -1;
  if (GetModuleType(module_ptr, module_type)
    && module_type == PROP_SOURCE) {
      ret_value = true;
  }
  return ret_value;
}

bool IsDestinationModule(const ModuleInterfacePtr &module_ptr) {
  bool ret_value = false;
  int module_type = -1;
  if (GetModuleType(module_ptr, module_type)
    && module_type == PROP_DESTINATION) {
      ret_value = true;
  }
  return ret_value;
}

bool IsRepeaterModule(const ModuleInterfacePtr &module_ptr) {
  bool ret_value = false;
  int module_type = -1;
  if (GetModuleType(module_ptr, module_type)
    && module_type == PROP_REPEATER) {
      ret_value = true;
  }
  return ret_value;
}

int GetModulePriority(const ModuleInterfacePtr &module_ptr) {
  int return_value = YSOS_ERROR_FAILED;
  ModuleInterface::ModuleDataInfo module_properties;
  do {
    if (!GetModuleProperty(module_ptr, module_properties)) {
      break;
    }
    return_value = MODULE_LOWEST_PRIORITY + 4;
    if (module_properties.is_data_revise) {
      return_value = MODULE_LOWEST_PRIORITY + 1;
    } else if (module_properties.is_self_allocator) {
      return_value = MODULE_LOWEST_PRIORITY + 2;
    } else if (module_properties.prefix_length > 0) {
      return_value = MODULE_LOWEST_PRIORITY + 3;
    } else if (IsRepeaterModule(module_ptr)) {
      return_value = MODULE_LOWEST_PRIORITY + 5;
    }
  } while (false);
  return return_value;
}

class ModuleCallbackFunc_ForIsCallbackOf:
  public ysos::BaseModuleCallbackFunction
{
public:
  ModuleCallbackFunc_ForIsCallbackOf(
    const CallbackInterfacePtr callback_ptr, bool *is_callback_of) {
    assert(NULL != callback_ptr);
    assert(NULL != is_callback_of);
    is_callback_of_ = is_callback_of;
    callback_ptr_ = callback_ptr;
  }

  int operator()(
    const CallbackInterfacePtr &callback_ptr,
    BaseModuleImpl *context_ptr = NULL) {
    if (*is_callback_of_) {
      return YSOS_ERROR_SUCCESS;
    }

    if (callback_ptr == callback_ptr_) {
      *is_callback_of_ = true;
    }

    return YSOS_ERROR_SUCCESS;
  }

private:
  bool *is_callback_of_;
  CallbackInterfacePtr callback_ptr_;
};

bool IsCallbackInCallbackQueueOfModule(
  ModuleInterfacePtr module_ptr, CallbackInterfacePtr callback_ptr, bool for_prev_callback_queue) {
  bool return_value = false;
  do {
    BaseModuleImpl *base_module = static_cast<BaseModuleImpl*>(module_ptr.get());
    if (!base_module) {
      break;
    }
    ModuleCallbackFunc_ForIsCallbackOf callback_func(callback_ptr, &return_value);
    if (for_prev_callback_queue) {
      base_module->IteratorPrevCallback(callback_func);
    } else {
      base_module->IteratorNextCallback(callback_func);
    }
  } while (false);

  return return_value;
}

bool IsModuleInList(ModuleInterfacePtrList& module_list, ModuleInterfacePtr module_ptr) {
  bool return_value = false;
  do {
    if (!module_ptr || module_list.size() == 0) {
      break;
    }
    for (ModuleInterfacePtrListIterator module_iter = module_list.begin();
      module_iter != module_list.end(); ++module_iter) {
        if((*module_iter).get() == module_ptr.get()) {
          return_value = true;
          break;
        }
    }
  } while (false);
  return return_value;
}

#if 1
//  NextToDo:
//  在Callback/Module/ModuleLink的xml定义正式确定后, 这些实现将被修改.//NOLINT
//  试图获取module_ptr所指Module的PrevCallbackQueue的指针到callback_queue_ptr所指//NOLINT
bool GetPrevCallbackQueue(
  const ModuleInterfacePtr &module_ptr,
  CallbackQueueInterface** callback_queue_ptr) {
  bool return_value = false;
  if (module_ptr
    && callback_queue_ptr
    && YSOS_ERROR_SUCCESS == module_ptr->GetProperty(
    PROP_PREVCALLBACKQUEUE, (void*)callback_queue_ptr)) {
    return_value = true;
  }
  return return_value;
}

//  试图获取module_ptr所指Module的NextCallbackQueue的指针到callback_queue_ptr所指//NOLINT
bool GetNextCallbackQueue(
  const ModuleInterfacePtr &module_ptr,
  CallbackQueueInterface** callback_queue_ptr) {
  bool return_value = false;
  if (module_ptr
    && callback_queue_ptr
    && YSOS_ERROR_SUCCESS == module_ptr->GetProperty(
    PROP_NEXTCALLBACKQUEUE, (void*)callback_queue_ptr)) {
      return_value = true;
  }
  return return_value;
}

class ModuleCallbackFunc_BackupNextCbQ:
  public ysos::BaseModuleCallbackFunction {
public:
  ModuleCallbackFunc_BackupNextCbQ(std::list<CallbackInterfacePtr> *callback_list_ptr) {
      assert(NULL != callback_list_ptr);
      callback_list_ptr_ = callback_list_ptr;
  }

  int operator()(
    const CallbackInterfacePtr &callback_ptr,
    BaseModuleImpl *context_ptr = NULL) {
      if (NULL != callback_ptr) {
        callback_list_ptr_->push_back(callback_ptr);
      }

      return YSOS_ERROR_SUCCESS;
  }

private:
  std::list<CallbackInterfacePtr> *callback_list_ptr_;
};

//  试图备份module_ptr所指Module的NextCallbackQueue中的各个callback到callback_list//NOLINT
bool BackupNextCallbackQueueToList(
  const ModuleInterfacePtr &module_ptr,
  std::list<CallbackInterfacePtr>& callback_list) {
  bool return_value = false;
  do {
    if (!module_ptr) {
      break;
    }

    BaseModuleImpl *base_module = static_cast<BaseModuleImpl*>(module_ptr.get());
    if (!base_module) {
      break;
    }
    ModuleCallbackFunc_BackupNextCbQ callback_func(&callback_list);
    base_module->IteratorNextCallback(callback_func);
    return_value = true;
  } while (false);
  return return_value;
}

bool GetAtomicDataTypeList(
  const ModuleInterfacePtr &module_ptr,
  std::list<std::string> &data_type_list,
  bool for_input_data_type) {
  bool return_value = false;
  CallbackDataType module_data_type = "";
  do {
    if (!GetModuleDataType(
      module_ptr,
      module_data_type,
      for_input_data_type)) {
        break;
    }
    if (module_data_type.length() == 0) {
      break;
    }
    int data_type_string_begin = 0;
    int data_type_string_end = module_data_type.find('|');
    if (data_type_string_end == -1) {
      data_type_list.push_back(module_data_type);
      return_value = true;
      break;
    }
    return_value = (YSOS_ERROR_SUCCESS == GetUtility()->SplitString(module_data_type, "|", data_type_list));
  } while (false);
  return return_value;
}

void RemoveEachNextCallbackFromModule(ModuleLinkInfoPtr& module_info_ptr) {
  do {
    if (!module_info_ptr) {
      break;
    }
    ModuleInterfacePtr &module_ptr = module_info_ptr->module_ptr_;
    if (!module_ptr) {
      break;
    }
    LightLock module_link_lock;
    AutoLockOper lock_oper(&module_link_lock);
    for (std::list<ModuleLinkInfoPtr>::iterator it = module_info_ptr->next_module_list_.begin();
      it != module_info_ptr->next_module_list_.end(); ++it) {
      if ((*it)) {
        int64_t owner_id = (int64_t)(void*)((*it)->module_ptr_.get());
        module_ptr->RemoveCallback(owner_id);
      }
    }
  } while (false);
}

class ModuleCallbackFunc_ForAddNextCallbackToPrevModule:
  public ysos::BaseModuleCallbackFunction
{
public:
  ModuleCallbackFunc_ForAddNextCallbackToPrevModule(
    const ModuleInterfacePtr &module_ptr, const ModuleInterfacePtr &next_module_ptr,
    std::string* data_type_ptr, int *callback_match_case_ptr, std::string *prompt_str_ptr) {
    assert(NULL != data_type_ptr);
    assert(NULL != callback_match_case_ptr);
    module_ptr_ = module_ptr;
    next_module_ptr_ = next_module_ptr;
    data_type_ptr_ = data_type_ptr;
    callback_match_case_ptr_ = callback_match_case_ptr;
    prompt_str_ptr_ = prompt_str_ptr;
  }

  int operator()(
    const CallbackInterfacePtr &callback_ptr,
    BaseModuleImpl *context_ptr = NULL) {

    CallbackDataType input_type;
    CallbackDataType output_type;
    if (YSOS_ERROR_SUCCESS == callback_ptr->GetType(&input_type, &output_type)) {
      std::string temp_prompt_str = "For type " + *data_type_ptr_ + ": " + "callback " + callback_ptr->GetName() + " type is (" + input_type + ", " + output_type + ");";

      /// PrevModule的输出数据类型与NextModule的Callback的输入数据类型匹配   //need update from linux  stricmp<->strcasecmp
      if (0 == strcasecmp(input_type.c_str(), data_type_ptr_->c_str()) ||
        0 == strcasecmp(input_type.c_str(), CallbackDataTypeAll) ||
        0 == strcasecmp(data_type_ptr_->c_str(), CallbackDataTypeAll) ||
        IsRepeaterModule(next_module_ptr_)) {

        int64_t owner_id = (int64_t)(void*)next_module_ptr_.get();
        int priority = GetModulePriority(next_module_ptr_);
        bool will_add = true;
        /// for PREV_MODULE
        /// 对于Repeater, Callback的PrevModule匹配吗?//NOLINT
        if (IsRepeaterModule(next_module_ptr_)) {
          ModuleInterfacePtr prev_ptr = NULL;
          callback_ptr->GetProperty(PROP_PREV_MODULE, &prev_ptr);
          if (!prev_ptr || prev_ptr.get() != module_ptr_.get()) {
            will_add = false;
          }
        }
        if (will_add && IsCallbackInCallbackQueueOfModule(module_ptr_, callback_ptr, false)) {
          will_add = false;
        }
        if (will_add) {
          module_ptr_->AddCallback(callback_ptr, owner_id, priority);
          *callback_match_case_ptr_ = 2;
          temp_prompt_str += " match and add.";
        } else {
          *callback_match_case_ptr_ = 1;
          temp_prompt_str += " match but need not add.";
        }
      } else {
        temp_prompt_str += " no matched.";
      }
      if (NULL != prompt_str_ptr_) {
        *prompt_str_ptr_ = temp_prompt_str;
      }
    }
    return YSOS_ERROR_SUCCESS;
  }

private:
  ModuleInterfacePtr module_ptr_;
  ModuleInterfacePtr next_module_ptr_;
  std::string* data_type_ptr_;

  // 0, no matched
  // 1, match but need not add
  // 2, match and add
  int *callback_match_case_ptr_;
  std::string *prompt_str_ptr_;
};

//  试图设置next_module_ptr所指Module的PrevCallbackQueue中匹配的Callback
//  到module_ptr所指Module的NextCallbackQueue.//NOLINT
//  若有匹配的Callback被设置则返回true,否则返回false//NOLINT
bool AddNextCallbackToPrevModule(
  ModuleInterfacePtr &module_ptr,
  ModuleInterfacePtr &next_module_ptr,
  std::list<std::string> *prompt_strings_ptr) {
  bool return_value = false;
  /// 2, match and add
  /// 1, match but need not add
  /// 0, no matched
  int exist_matched_callback = 0;
  std::list<std::string> data_type_list;
  do {
    if (!module_ptr || !next_module_ptr) {
      break;
    }
    if (!GetAtomicDataTypeList(module_ptr, data_type_list, false)) {
        break;
    }
    for (std::list<std::string>::iterator it = data_type_list.begin();
      it != data_type_list.end();
      ++it) {
      BaseModuleImpl *base_module = static_cast<BaseModuleImpl*>(next_module_ptr.get());
      if (!base_module) {
        break;
      }
      std::string prompt_str;
      ModuleCallbackFunc_ForAddNextCallbackToPrevModule callback_func(
        module_ptr, next_module_ptr, &(*it),
        &exist_matched_callback,
        ((NULL != prompt_strings_ptr) ? (&prompt_str) : NULL));
      base_module->IteratorPrevCallback(callback_func);
      if (NULL != prompt_strings_ptr) {
        prompt_strings_ptr->push_back(prompt_str);
      }
    }

    if (exist_matched_callback == 2) {
      return_value = true;
    }
  } while (false);
  return return_value;
}

//  试图从module_ptr所指Module的NextCallbackQueue中, 移除//NOLINT
//  next_module_ptr所指Module的PrevCallbackQueue中匹配的Callback//NOLINT
//  若成功则返回true,否则返回false.//NOLINT
bool RemoveNextCallbackFromPrevModule(
  ModuleInterfacePtr &module_ptr,
  ModuleInterfacePtr &next_module_ptr) {
  bool return_value = false;
  bool exist_matched_callback = false;
  std::list<std::string> data_type_list;
  do {
    if (!module_ptr || !next_module_ptr) {
      break;
    }
    int64_t owner_id = (int64_t)(void*)next_module_ptr.get();
    module_ptr->RemoveCallback(owner_id);
    return_value = true;
  } while (false);
  return return_value;
}
#endif

/// 将模块添加到其Prev模块的NextModuleList中//NOLINT
bool InsertModuleToNextModueListOfPrevModule(
  ModuleLinkInfoPtr &prev_module_link_ptr,
  const ModuleLinkInfoPtr &module_link_ptr) {
  bool return_value = false;
  if (prev_module_link_ptr && module_link_ptr) {
    int priority = GetModulePriority(module_link_ptr->module_ptr_);
    size_t element_amount = prev_module_link_ptr->next_module_list_.size();

    if (element_amount == 0) {
      prev_module_link_ptr->next_module_list_.push_back(module_link_ptr);
    } else {
      //  是否需要添加//NOLINT
      bool will_add = true;
      for (ModuleLinkInfoIterator it = prev_module_link_ptr->next_module_list_.begin();
        it != prev_module_link_ptr->next_module_list_.end();
        ++it) {
          //  若已存在则无需添加//NOLINT
          if ((*it)->module_ptr_.get() == module_link_ptr->module_ptr_.get()) {
            will_add = false;
            break;
          }
      }
      if (will_add) {
        int front_priority = GetModulePriority(
          prev_module_link_ptr->next_module_list_.front()->module_ptr_);
        int back_priority = GetModulePriority(
          prev_module_link_ptr->next_module_list_.back()->module_ptr_);
        if (front_priority < priority) {
          prev_module_link_ptr->next_module_list_.push_front(module_link_ptr);
        } else if (back_priority >= priority) {
          prev_module_link_ptr->next_module_list_.push_back(module_link_ptr);
        } else {
          for (ModuleLinkInfoIterator it = prev_module_link_ptr->next_module_list_.begin();
            it != prev_module_link_ptr->next_module_list_.end();
            ++it) {
            int curr_priority = GetModulePriority((*it)->module_ptr_);
            if (curr_priority < priority) {
              --it;
              prev_module_link_ptr->next_module_list_.insert(it, module_link_ptr);
              break;
            }
          }
        }
      }
    }
    return_value = true;
  }
  return return_value;
}

/// 通过链接信息, 修改模块的自分配和优先级属性//NOLINT
void ModifySelfAllocatorAndPriority(const ModuleLinkInfoPtr &module_link_ptr) {
  int amount_of_revise_data_module = 0;
  if (module_link_ptr->module_ptr_) {
    for (ModuleLinkInfoIterator it = module_link_ptr->next_module_list_.begin();
      it != module_link_ptr->next_module_list_.end();
      ++it) {
      if ((*it)->module_ptr_) {
        ModuleInterface::ModuleDataInfo module_properties;
        GetModuleProperty((*it)->module_ptr_, module_properties);
        if (module_properties.is_data_revise) {
          ++amount_of_revise_data_module;

          //  若NextModuleList中有超过一个Module修改了数据, 则自第二个这样的Module起,
          //  修改其SelfAllocator和优先级
          if (amount_of_revise_data_module >= 2) {
            //  SelfAllocator
            bool is_self_allocator = true;
            (*it)->module_ptr_->SetProperty(
              PROP_SELF_ALLOCATOR, &is_self_allocator);

            //  优先级
            (*it)->module_priority_ = MODULE_LOWEST_PRIORITY + 0;
          }
        }
      }
    }
  }
}

/// 获取共享缓冲区的模块的缓冲区前缀长度的最大值//NOLINT
uint32_t GetMaxPrefixBufferLengthForModuleSharedBuffer(
  const ModuleLinkInfoPtr &module_link_ptr) {
    uint32_t return_value = 0;
    // if (!SelfAllocator) {}
    if (module_link_ptr->module_ptr_) {
      for (ModuleLinkInfoIterator it = module_link_ptr->next_module_list_.begin();
        it != module_link_ptr->next_module_list_.end();
        ++it) {
          if ((*it)->module_ptr_) {
            ModuleInterface::ModuleDataInfo module_properties;
            GetModuleProperty((*it)->module_ptr_, module_properties);
            if (!module_properties.is_self_allocator) {
              if (return_value < module_properties.prefix_length) {
                return_value = module_properties.prefix_length;
              }
            }
          }
      }
    }
    return return_value;
}

std::string MakeRepeaterName(
  const std::string& module_link_name,
  uint32_t link_depth,
  uint32_t level,
  bool inherent,
  bool act_as_destination) {
    std::string repeater_name = module_link_name;
    repeater_name += (inherent ? "_i_" : "_t_");
    char buf_level[21];
    memset(buf_level, 0, sizeof(buf_level));
    // sprintf(buf_level, "%04x%04x", level, (act_as_destination ? 0 : (level + 1)));
    sprintf(buf_level, "%04x%04x", level, ((link_depth <= level) ? 0 : (level + 1)));
    repeater_name += buf_level;
    repeater_name += "_Repeater";
    return repeater_name;
}

/// Repeater的Callback实例名形如: 有NextModule情形//NOLINT
/// BaseRepeaterCallbackImpl___PrevModuleName__In__Out___NextModuleName__In__Out
/// 或, 无NextModule情形//NOLINT
/// BaseRepeaterCallbackImpl___PrevModuleName__In__Out___
std::string MakeRepeaterCallbackInstanceName(
  ModuleInterfacePtr prev_module,
  CallbackInterfacePtr prev_callback,
  ModulePtrEdgeList& edge_list) {
  std::string callback_instance_name = BASE_REPEATER_CALLBACK_IMPL_CLASS_NAME;
  callback_instance_name += "___";
  if (prev_module) {
    callback_instance_name += prev_module->GetName();
    callback_instance_name += "__";
    if (prev_callback) {
      CallbackDataType in_type;
      CallbackDataType out_type;
      if (YSOS_ERROR_SUCCESS == prev_callback->GetType(&in_type, &out_type)) {
        callback_instance_name += in_type;
        callback_instance_name += "__";
        callback_instance_name += out_type;
      }
    }
  }
  callback_instance_name += "___";
  if (prev_module) {
    for (ModulePtrEdgeListIterator it3 = edge_list.begin();
      it3 != edge_list.end(); ++it3) {
        if (it3->first == prev_module) {
          if (it3->second) {
            callback_instance_name += it3->second->GetName();
          }
        }
    }
  }

  return callback_instance_name;
}

/// -1, 出错
/// 0, 无需控制, 因为module_ptr对应的module不在to module的所在子链.
/// 1, 需要控制
int RebuildControlRangeString(void* param, ModuleLinkInterfacePtr link_ptr, ModuleInterface* module_ptr, std::string &new_ctrl_range) {
  int return_value = 1;
  do {
    if (NULL == module_ptr) {
      return_value = -1;
      break;
    }

    if (NULL == param) {
      new_ctrl_range = "|";
    } else {
      std::string *ctrl_range_string_ptr = (std::string*)param;
      unsigned long length = ctrl_range_string_ptr->size();
      if (length == 0) {
        new_ctrl_range = "|";
      } else {
        unsigned long pos = ctrl_range_string_ptr->find_first_of('|');
        if ((pos == 0) || (pos > 0 && pos < length)) {
          std::string name_of_from_module = "";
          if (pos == 0) {
            new_ctrl_range = "|";
          } else if (pos > 0 && pos < length){
            new_ctrl_range = ctrl_range_string_ptr->substr(0, pos + 1);

            /// to module
            name_of_from_module = ctrl_range_string_ptr->substr(0, pos);
          }
          std::string name_of_to_module = ctrl_range_string_ptr->substr(pos + 1, length - pos - 1);
          std::string module_name = module_ptr->GetName();
          ModuleLinkImpl* mlink_impl_ptr = (ModuleLinkImpl*)link_ptr.get();
          return_value = mlink_impl_ptr->IsModuleOnSubLinkPiece(
            name_of_from_module, name_of_to_module, module_name);
        } else {
          new_ctrl_range = *ctrl_range_string_ptr;
          new_ctrl_range += "|";
        }
      }
    }

    new_ctrl_range += module_ptr->GetName();
  } while (false);
  return return_value;
}

}
