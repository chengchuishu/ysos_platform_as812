/**
 *@file ModuleUtil.h
 *@brief Helpers for module and module link
 *@version 0.1
 *@author XuLanyue
 *@date Created on: 2016-06-13 21:30:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef CHP_MODULE_UTIL_H   //NOLINT
#define CHP_MODULE_UTIL_H   //NOLINT

#include "../../../public/include/sys_interface_package/moduleinterface.h"
#include "../../../protect/include/sys_framework_package/modulelinkimpl.h"
#include "../../../public/include/os_hal_package/lock.h"

namespace ysos {
bool GetModuleProperty(
  const ModuleInterfacePtr &module_ptr,
  ModuleInterface::ModuleDataInfo &module_properties);
bool GetModuleDataType(
  const ModuleInterfacePtr &module_ptr,
  CallbackDataType &module_data_type,
  bool for_input_data_type = true);
bool GetAtomicDataTypeList(
  const ModuleInterfacePtr &module_ptr,
  std::list<std::string> &data_type_list,
  bool for_input_data_type = true);
bool GetModuleType(const ModuleInterfacePtr &module_ptr, int &module_type);
bool IsSourceModule(const ModuleInterfacePtr &module_ptr);
bool IsDestinationModule(const ModuleInterfacePtr &module_ptr);
bool IsRepeaterModule(const ModuleInterfacePtr &module_ptr);
int GetModulePriority(const ModuleInterfacePtr &module_ptr);

bool IsCallbackInCallbackQueueOfModule(
  ModuleInterfacePtr module_ptr, CallbackInterfacePtr callback_ptr, bool for_prev_callback_queue = true);

bool IsModuleInList(
  ModuleInterfacePtrList& module_list,
  ModuleInterfacePtr module_ptr);

//  试图获取module_ptr所指Module的PrevCallbackQueue的指针到callback_queue_ptr所指//NOLINT
bool GetPrevCallbackQueue(
  const ModuleInterfacePtr &module_ptr,
  CallbackQueueInterface** callback_queue_ptr);

//  试图获取module_ptr所指Module的NextCallbackQueue的指针到callback_queue_ptr所指//NOLINT
bool GetNextCallbackQueue(
  const ModuleInterfacePtr &module_ptr,
  CallbackQueueInterface** callback_queue_ptr);

//  试图从module_ptr所指Module的NextCallbackQueue中, 移除//NOLINT
//  next_module_ptr所指Module的PrevCallbackQueue中匹配的Callback//NOLINT
//  若成功则返回true,否则返回false.//NOLINT
bool RemoveNextCallbackFromPrevModule(
  ModuleInterfacePtr &module_ptr,
  ModuleInterfacePtr &next_module_ptr);

//  试图备份module_ptr所指Module的NextCallbackQueue中的各个callback到callback_list//NOLINT
bool BackupNextCallbackQueueToList(
  const ModuleInterfacePtr &module_ptr,
  std::list<CallbackInterfacePtr>& callback_list);

void RemoveEachNextCallbackFromModule(ModuleLinkInfoPtr& module_info_ptr);

/// 将模块添加到其Prev模块的NextModuleList中//NOLINT
bool InsertModuleToNextModueListOfPrevModule(
  ModuleLinkInfoPtr &prev_module_link_ptr,
  const ModuleLinkInfoPtr &module_link_ptr);

/// 通过链接信息, 修改模块的自分配和优先级属性//NOLINT
void ModifySelfAllocatorAndPriority(const ModuleLinkInfoPtr &module_link_ptr);

/// NextToDo: 考虑Repeater扮演Destination角色的情形.//NOLINT
/// 边的链接信息是否在Repeater中已有记录.//NOLINT
/// 若有则置repeater_info_ptr_list_iterator并返回true;否则返回false.//NOLINT
bool ExistLinkInfoBetweenModulesOfEdge(
  ModuleInterfacePtr prev,
  ModuleInterfacePtr next,
  RepeaterModuleInfoPtrList& repeater_info_ptr_list,
  RepeaterModuleInfoPtrListIterator& repeater_info_ptr_list_iterator);

/// NextToDo: 考虑Repeater扮演Destination角色的情形.//NOLINT
/// 边的链接信息是否在某Repeater中已有记录.//NOLINT
/// 若有则返回true;否则返回false.//NOLINT
bool ExistLinkInfoBetweenModulesOfEdge(
  ModuleInterfacePtr prev,
  ModuleInterfacePtr next,
  RepeaterModuleInfoPtr repeater_info_ptr,
  RepeaterModuleInfoPtrListIterator& repeater_info_ptr_list_iterator);

bool RemoveModuleFromNextModuleList(
  ModuleLinkInfoPtr prev_info,
  ModuleLinkInfoPtr next_info);

bool RemoveModuleFromPrevModuleList(
  ModuleLinkInfoPtr prev_info,
  ModuleLinkInfoPtr next_info);

bool IsModuleInfoInList(
  ModuleLinkInfoPtr& link_info_ptr,
  ModuleLinkInfoList& link_info_list);

/// 试图设置callback_queue_ptr中匹配的Callback,
/// 到module_ptr所指Module的NextCallbackQueue.
/// 若有匹配的Callback被设置则返回true,否则返回false
bool AddNextCallbackToPrevModule(
  ModuleInterfacePtr &module_ptr,
  ModuleInterfacePtr &next_module_ptr,
  std::list<std::string> *prompt_strings_ptr);

/// 获取共享缓冲区的模块的缓冲区前缀长度的最大值//NOLINT
uint32_t GetMaxPrefixBufferLengthForModuleSharedBuffer(
  const ModuleLinkInfoPtr &module_link_ptr);

/// Repeate命名规则://NOLINT
/// 固有的Repeate命名形如//NOLINT
/// ModuleLinkName_i_hhhhhhhh_Repeater
/// 临时的Repeate命名形如//NOLINT
/// ModuleLinkName_t_hhhhhhhh_Repeater
std::string MakeRepeaterName(
  const std::string& module_link_name,
  uint32_t link_depth,
  uint32_t level,
  bool inherent,
  bool act_as_destination);

/// Repeater的Callback实例名形如: 有NextModule情形//NOLINT
/// BaseRepeaterCallbackImpl___PrevModuleName__In__Out___NextModuleName__In__Out
/// 或, 无NextModule情形//NOLINT
/// BaseRepeaterCallbackImpl___PrevModuleName__In__Out___
std::string MakeRepeaterCallbackInstanceName(
  ModuleInterfacePtr prev_module,
  CallbackInterfacePtr prev_callback,
  ModulePtrEdgeList& edge_list);

/// -1, 出错
/// 0, 无需控制, 因为module_ptr对应的module不在to module的所在子链.
/// 1, 需要控制
int RebuildControlRangeString(void* param, ModuleLinkInterfacePtr link_ptr, ModuleInterface* module_ptr, std::string &new_ctrl_range);
}

#endif // CHP_MODULE_UTIL_H //NOLINT
