/**
  *@file ModuleLinkImpl.cpp
  *@brief Definition of ModuleLinkImpl
  *@version 0.1
  *@author XuLanyue
  *@date Created on: 2016-05-13 13:59:20
  *@copyright Copyright © 2016 YunShen Technology. All rights reserved.
  * 
  */

/// Self Header
#include "../../../protect/include/sys_framework_package/modulelinkimpl.h"

/// c headers
#include <string>
#include <list>

#include "boost/make_shared.hpp"

/// ysos private headers
// #include "../../../public/include/core_help_package/ysos_log.h"
#include "../../../public/include/sys_interface_package/baseinterface.h"
#include "../../../public/include/sys_framework_package/templatemanager.h"
#include "../../../public/include/sys_interface_package/moduleinterface.h"
#include "../../../public/include/sys_interface_package/ysos_enum.h"
#include "../../../protect/include/sys_framework_package/manager_declare.h"
#include "../../../protect/include/sys_framework_package/basemoduleimpl.h"
#include "../../../protect/include/sys_framework_package/moduleutil.h"
#include "../../../protect/include/sys_framework_package/baserepeatermoduleimpl.h"
#include "../../../protect/include/sys_framework_package/baserepeatercallbackimpl.h"
#include "../../../protect/include/sys_framework_package/packageconfigimpl.h"


#define CONTROL_TYPE_STANDARD                       0
#define CONTROL_TYPE_IOCTL                          1
#define CONTROL_TYPE_MODULE                         2

#define CONTROL_COMMAND_RUN                         200
#define CONTROL_COMMAND_PAUSE                       201
#define CONTROL_COMMAND_STOP                        202
#define CONTROL_COMMAND_FLUSH                       203

namespace ysos {

bool ModuleLinkImpl::IsModuleOnSubLinkByLinkInfo(
  ModuleInterface *module_ptr,
  ModuleInterfacePtr module_sptr, ///< range_to
  ModuleLinkInfoPtr link_info_ptr) { ///< range_to
  bool return_value = false;

  if (NULL == module_ptr) {
    return return_value;
  }
  std::string next_non_repeater = "";
  while (NULL != module_sptr && NULL != link_info_ptr) {
    if (module_sptr.get() == module_ptr) {
      return_value = true;
      break;
    }

    if (link_info_ptr->prev_module_list_.size() > 0) {
      if (IsRepeaterModule(link_info_ptr->module_ptr_)) {
        std::set<std::string> prev_module_name_set;
        GetPrevModuleSetByRepeaterName(
          link_info_ptr->module_ptr_->GetName(),
          next_non_repeater, prev_module_name_set);

        /// 无适当的prev
        if (prev_module_name_set.size() == 0) {
          break;
        }

        /// 在prev中查找
        for (std::set<std::string>::iterator it = prev_module_name_set.begin();
             it != prev_module_name_set.end(); ++it) {
          if ((*it) == module_ptr->GetName()) {
            return true;
          }
        }
        for (std::set<std::string>::iterator it = prev_module_name_set.begin();
             it != prev_module_name_set.end(); ++it) {

          /// 递归调用
          if (YSOS_ERROR_SUCCESS != GetModuleLinkInfoPtrByName(link_info_ptr, (*it))) {
            continue;
          }
          module_sptr = link_info_ptr->module_ptr_;
          if (IsModuleOnSubLinkByLinkInfo(module_ptr, module_sptr, link_info_ptr)) {
            return true;
          }
        }
      } else {
        next_non_repeater = link_info_ptr->module_ptr_->GetName();
        if (link_info_ptr->prev_module_list_.size() == 1) {
          link_info_ptr = link_info_ptr->prev_module_list_.front();
          module_sptr = link_info_ptr->module_ptr_;
        } else {
          for (std::list<ModuleLinkInfoPtr>::iterator it = link_info_ptr->prev_module_list_.begin();
               it != link_info_ptr->prev_module_list_.end(); ++it) {
            if (IsModuleOnSubLinkByLinkInfo(module_ptr, (*it)->module_ptr_, *it)) {
              return true;
            }
          }
        }
      }
    } else {
      link_info_ptr = NULL;
      module_sptr = NULL;
    }
  }

  return return_value;
}

/// 模块在子链上
bool ModuleLinkImpl::IsModuleOnSubLink(ModuleInterface *module_ptr, SubLinkInfoInModuleLink *sub_link_ptr) {
  bool return_value = false;
  do {
    if (NULL == module_ptr || NULL == sub_link_ptr) {
      break;
    }

    ModuleLinkInfoPtr link_info_ptr = *sub_link_ptr->link_info_position_;
    ModuleInterfacePtr module_sptr = sub_link_ptr->module_ptr_;
    return_value = IsModuleOnSubLinkByLinkInfo(module_ptr, module_sptr, link_info_ptr);
    // YSOS_LOG_DEBUG("Is Module " << module_ptr->GetName()
    //   << " on SubLink " << sub_link_ptr->module_ptr_->GetName() << ": " << return_value << ".");
  } while (false);
  return return_value;
}

bool ModuleLinkImpl::GetSubLinkByModule(ModuleInterface *module_ptr, std::list<SubLinkInfoInModuleLink> &sub_link_info_list,
                                        SubLinkInfoInModuleLink &sub_link) {

  bool return_value = false;
  do {
    if (NULL == module_ptr || sub_link_info_list.size() == 0) {
      break;
    }

    for (std::list<SubLinkInfoInModuleLink>::iterator it = sub_link_info_list.begin();
         it != sub_link_info_list.end(); ++it) {
      ModuleLinkInfoPtr link_info_ptr = *(it->link_info_position_);
      ModuleInterfacePtr module_sptr = it->module_ptr_;
      while (NULL != module_sptr && NULL == link_info_ptr) {
        if (module_sptr.get() == module_ptr) {
          return_value = true;
          sub_link = (*it);
          break;
        }

        if (link_info_ptr->prev_module_list_.size() > 0) {
          /// 除了Repeater, 每个Module至多只有一个PrevModule
          link_info_ptr = link_info_ptr->prev_module_list_.front();
          module_sptr = link_info_ptr->module_ptr_;
        } else {
          link_info_ptr = NULL;
          module_sptr = NULL;
        }
      }
    }
  } while (false);
  return return_value;
}

/// 模块在子链上
bool ModuleLinkImpl::IsModuleOnSubLink(ModuleInterface *module_ptr, ModuleInterface *range_to_module_ptr,
                                       std::list<SubLinkInfoInModuleLink> &sub_link_info_list) {
  SubLinkInfoInModuleLink sub_link;
  bool return_value = false;
  do {
    if (NULL == module_ptr || sub_link_info_list.size() == 0) {
      break;
    }

    if (!GetSubLinkByModule(range_to_module_ptr, sub_link_info_list, sub_link)) {
      break;
    }

    ModuleLinkInfoPtr link_info_ptr = *sub_link.link_info_position_;
    ModuleInterfacePtr module_sptr = sub_link.module_ptr_;

    return_value = IsModuleOnSubLinkByLinkInfo(module_ptr, module_sptr, link_info_ptr);
  } while (false);
  return return_value;
}

int ModuleLinkImpl::GetSetOfModuleWithoutNextInRepeater(
  const std::string &repeater_name,
  std::list<std::string> &name_set_of_module_without_next) {
  int return_value = YSOS_ERROR_SUCCESS;
  RepeaterModuleInfoPtrList* temp_list_ptrs[] = {
    &inherent_repeater_info_list_, &temporary_repeater_info_list_
  };
  bool found = false;
  do {
    for (int i = 0; i < 2; ++i) {
      if (found) {
        break;
      }
      for (RepeaterModuleInfoPtrListIterator it = temp_list_ptrs[i]->begin(); it != temp_list_ptrs[i]->end(); ++it) {
        if ((*it)->name == repeater_name || (*it)->famous_name == repeater_name) {
          found = true;

          ModuleInterfacePtrList prev_module_list;
          for (ModulePtrEdgeListIterator it_edge = (*it)->current_edge_set.begin();
               it_edge != (*it)->current_edge_set.end(); ++it_edge) {
            if (!IsModuleInList(prev_module_list, it_edge->first)) {
              prev_module_list.push_back(it_edge->first);
            }
          }
          for (ModuleInterfacePtrList::iterator it2 = prev_module_list.begin();
               it2 != prev_module_list.end(); ++it2) {

            ModuleInterfacePtrList next_module_list;
            for (ModulePtrEdgeListIterator it_edge = (*it)->current_edge_set.begin();
                 it_edge != (*it)->current_edge_set.end(); ++it_edge) {

              if ((*it2) == it_edge->first) {
                if (NULL != it_edge->second) {
                  next_module_list.push_back(it_edge->second);
                }
              }
            }

            if (next_module_list.size() == 0) {
              name_set_of_module_without_next.push_back((*it2)->GetName());
            }
          }
          break;
        }
      }
    }
  } while (false);
  return return_value;
}

int ModuleLinkImpl::GetPrevModuleSetByRepeaterName(
  const std::string &repeater_name, const std::string &next_non_repeater, std::set<std::string> &prev_module_name_set) {
  int return_value = YSOS_ERROR_SUCCESS;
  RepeaterModuleInfoPtrList* temp_list_ptrs[] = {
    &inherent_repeater_info_list_, &temporary_repeater_info_list_
  };
  bool found = false;
  do {
    for (int i = 0; i < 2; ++i) {
      if (found) {
        break;
      }
      for (RepeaterModuleInfoPtrListIterator it = temp_list_ptrs[i]->begin(); it != temp_list_ptrs[i]->end(); ++it) {
        if ((*it)->name == repeater_name || (*it)->famous_name == repeater_name) {
          found = true;
          for (ModulePtrEdgeListIterator it_edge = (*it)->current_edge_set.begin();
               it_edge != (*it)->current_edge_set.end(); ++it_edge) {
            bool will_insert = true;
            std::string name_1st = (*it_edge).first->GetName();
            if (next_non_repeater.length() == 0) {
            } else {
              if (NULL != (*it_edge).second) {
                std::string name_2nd = (*it_edge).second->GetName();
                if (name_2nd != next_non_repeater) {
                  will_insert = false;
                }
              } else {
                will_insert = false;
              }
            }
            if (will_insert) {
              prev_module_name_set.insert(name_1st);
            }
          }
          break;
        }
      }
    }
  } while (false);
  return return_value;
}

/// 返回值
/// -1, 出错;
/// 0, module_name不在range_from到range_to对应的链片段上;
/// 1, module_name在range_from到range_to对应的链片段上;
///
/// TODO:from与to的相对位置关系.
///
/// step1.令current = range_to;
/// step2.若range_from==current,
/// 则,
///   若range_from==module_name,current==module_name或,则返回1,否则返回0
/// 否则:
///   找到current对应的信息项,
///   查看其Prev列表中是否有module_name,
///   若有,则返回1; 否则:

int ModuleLinkImpl::IsModuleOnSubLinkPiece(
  const ModuleLinkInfoPtr &mlip_range_from,
  const ModuleLinkInfoPtr &mlip_range_to,
  const ModuleLinkInfoPtr &mlip_module_name,
  const std::string &next_non_repeater) {

  int return_value = -1;
  ModuleLinkInfoPtr mlip_current = mlip_range_to;
  ModuleLinkInfoPtr mlip_next_non_repeater = NULL;
  do {
    if (mlip_module_name == mlip_range_to) {
      return_value = 1;
      break;
    }

    /// 对每一个Prev,
    /// 若其为Repeater,则找出其边表中以current作为second的,找出其first,形成一个vector,对其中的每一个递归执行step1;
    if (IsRepeaterModule(mlip_current->module_ptr_)) {
      /// 找到Repeater信息.
      std::string repeater_name = mlip_current->module_ptr_->GetName();
      std::set<std::string> prev_module_name_set;
      GetPrevModuleSetByRepeaterName(
        repeater_name, next_non_repeater, prev_module_name_set);

      /// 无适当的prev
      if (prev_module_name_set.size() == 0) {
        return_value = 0;
        break;
      }

      /// 在prev中查找
      for (std::set<std::string>::iterator it = prev_module_name_set.begin();
           it != prev_module_name_set.end(); ++it) {
        if ((*it) == mlip_module_name->module_ptr_->GetName()) {
          return_value = 1;
          break;
        }
      }
      if (return_value == 1) {
        break;
      }

      /// 对prev中的每个元素, 递归调用
      int temp_return_value = -1;
      for (std::set<std::string>::iterator it = prev_module_name_set.begin();
           it != prev_module_name_set.end(); ++it) {

        /// 递归调用
        ModuleLinkInfoPtr mlip_range_to_;
        if (YSOS_ERROR_SUCCESS != GetModuleLinkInfoPtrByName(mlip_range_to_, (*it))) {
          continue;
        }
        temp_return_value = IsModuleOnSubLinkPiece(
                              mlip_range_from, mlip_range_to_, mlip_module_name, next_non_repeater);
        if (temp_return_value == 1) {
          return_value = 1;
          break;
        }
      }
      if (return_value == 1) {
        break;
      }
    } else {
      /// 否则:找出其Prev
      ///
      if (mlip_current->prev_module_list_.size() == 0) {
        return_value = 0;
        break;
      }
      for (std::list<ModuleLinkInfoPtr>::iterator it = mlip_current->prev_module_list_.begin();
           it != mlip_current->prev_module_list_.end(); ++it) {
        if ((*it) == mlip_module_name) {
          return_value = 1;
          break;
        }
      }
      if (return_value == 1) {
        break;
      }
      int temp_return_value = -1;
      for (std::list<ModuleLinkInfoPtr>::iterator it = mlip_current->prev_module_list_.begin();
           it != mlip_current->prev_module_list_.end(); ++it) {

        /// 递归调用
        std::string next_non_repeater_ = mlip_current->module_ptr_->GetName();
        temp_return_value = IsModuleOnSubLinkPiece(
                              mlip_range_from, (*it), mlip_module_name, next_non_repeater_);
        if (temp_return_value == 1) {
          return_value = 1;
          break;
        }
      }
      if (return_value == 1) {
        break;
      }
    }

    return_value = 0;
  } while (false);
  return return_value;
}

int ModuleLinkImpl::IsModuleOnSubLinkPiece(
  const std::string &range_from, const std::string &range_to, const std::string &module_name) {
  int return_value = -1;
  std::string range_from_ = range_from;
  std::string current = range_to;
  std::string next_non_repeater = "";
  ModuleLinkInfoPtr mlip_range_from = NULL;
  ModuleLinkInfoPtr mlip_current = NULL;
  ModuleLinkInfoPtr mlip_module_name = NULL;
  do {
    /// 参数校验
    if (current.length() == 0 || module_name.length() == 0) {
      break;
    }

    if (range_from.length() == 0) {
      range_from_ = (*source_position_of_the_link)->module_ptr_->GetName();
    }

    /// from与to的相对位置关系.

    /// 特殊情形
    if (0 == strcmp(module_name.c_str(), range_from_.c_str()) ||
        0 == strcmp(module_name.c_str(), current.c_str())) {
      return_value = 1;
      break;
    }

    /// 获取对应的ModuleLinkInfoPtr
    if (YSOS_ERROR_SUCCESS != GetModuleLinkInfoPtrByName(mlip_range_from, range_from_)) {
      break;
    }
    if (YSOS_ERROR_SUCCESS != GetModuleLinkInfoPtrByName(mlip_current, current)) {
      break;
    }
    if (YSOS_ERROR_SUCCESS != GetModuleLinkInfoPtrByName(mlip_module_name, module_name)) {
      break;
    }
    if (!IsRepeaterModule(mlip_current->module_ptr_)) {
      next_non_repeater = current;
    }
    return_value = IsModuleOnSubLinkPiece(
                     mlip_range_from, mlip_current, mlip_module_name, next_non_repeater);
  } while (false);
  return return_value;
}

/// 从指定的根开始访问树
int ModuleLinkImpl::VisitLinkInfoTree(ModuleLinkInfoIterator root, void* param_ptr, void* context_ptr, bool &failed,
                                      VISIT_FUNC_INT visit_func, SubLinkInfoInModuleLink *sub_link_ptr = NULL) {
  int return_value = 0;
  do {
    if (!visit_func) {
      break;
    }

    /// 访问根
    visit_func(root, param_ptr, context_ptr, failed, return_value);
    if (failed) {
      break;
    }

    YSOS_LOG_DEBUG("visit returns " << return_value);

    /// 若已到达控制终点
    if (NULL != param_ptr && param_ptr == (void*)((*root)->module_ptr_.get())) {
      break;
    }

    for (ModuleLinkInfoIterator it = (*root)->next_module_list_.begin();
         it != (*root)->next_module_list_.end(); ++it) {
      if (!(*it)) {
        continue;
      }

      /// 不在子链上的, Skip
      if (NULL != sub_link_ptr && !IsModuleOnSubLink((*it)->module_ptr_.get(), (ModuleInterface*)param_ptr,
          sub_link_info_list_)) {
        continue;
      }

      /// 递归访问根下的每棵子树
      return_value = VisitLinkInfoTree(it, param_ptr, context_ptr, failed, visit_func, sub_link_ptr);
      if (failed) {
        break;
      }
    }
  } while (false);
  return return_value;
}

/// 由Open调用
int OpenNode(ModuleLinkInfoIterator it, void* param_ptr, void* context_ptr, bool &failed, int& return_value) {
  return_value = 0;
  if ((*it) && (*it)->module_ptr_) {
    return_value = (*it)->module_ptr_->Open(param_ptr, context_ptr);
  }
  if (return_value < 0) {
    failed = true;
  }
  return return_value;
}

/// 由Close调用
int CloseNode(ModuleLinkInfoIterator it, void* param_ptr, void* context_ptr, bool &failed, int& return_value) {
  return_value = 0;
  if ((*it) && (*it)->module_ptr_) {
    return_value = (*it)->module_ptr_->Close(param_ptr);
  }
  if (return_value < 0) {
    failed = true;
  }
  return return_value;
}

ModuleLinkImpl::ModuleLinkImpl(const std::string &object_name) : BaseInterfaceImpl(object_name) {

  logger_ = GetUtility()->GetLogger("ysos.framework.modulelink");

  autorun_ = false;
  inherent_ = true;
  link_depth_ = 0;
  link_opened_ = false;
  // link_stat_ = PROP_STOP;
  tree_built_ = true;
}

ModuleLinkImpl::~ModuleLinkImpl() {
  UnInitialize();
}

/**
 *@brief 类似于模块的初始化功能, 同步, 从前往后, 调用Open
 *@return 成功返回0，否则失败
 */
int ModuleLinkImpl::Initialize(void* param) {
  if (IsInitialized()) {
    //YSOS_LOG_ERROR("ModuleLinkImpl::Initialize IsInitialized()-[0]: " << IsInitialized()); //add for linux  only-4-debug
    return YSOS_ERROR_SUCCESS;
  }
  //YSOS_LOG_DEBUG("ModuleLinkImpl::Initialize IsInitialized()-[1]: " << IsInitialized()); //add for linux  only-4-debug
  int return_value = YSOS_ERROR_FAILED;
  do {
    if (NULL == param) {
      std::string tmoudlelinkname = this->GetName(); //need add for linux
      //YSOS_LOG_DEBUG("ModuleLinkImpl::Initialize -[tmoudlelinkname]: " << tmoudlelinkname); //add for linux  only-4-debug
      return_value = GetPackageConfigImpl()->CreateModuleLinkObject(this, tmoudlelinkname/*this->GetName()*/);
      if (YSOS_ERROR_SUCCESS != return_value) {
        YSOS_LOG_ERROR("Try to create a module link with name " << this->GetName()
                       << " from config failed with error code " << return_value << ".");
        break;
      }
    }

    /// 若全部Module都在同一棵树中了,且((Link长度>1且有头有尾)||(Link长度==1且为头))
    if (!IsEachModuleLinked() || !ExistSourceModule()) {
      return_value = YSOS_ERROR_FAILED;
      YSOS_LOG_ERROR("Try to open a module link with name " << this->GetName()
                     << " but not each module linked or no source module.");
      DumpAll();
      break;
    }

    if (!RebuildModuleLinkInfoTree()) {
      YSOS_LOG_ERROR("Try to open a module link with name " << this->GetName()
                     << " but failed to rebuild the module link info tree.");
      break;
    }

    return_value = Open(param);
    if (YSOS_ERROR_SUCCESS != return_value) {
      YSOS_LOG_ERROR("Try to open a module link with name " << this->GetName()
                     << " failed with error code " << return_value << ".");
      break;
    }
  } while (false);

  if (YSOS_ERROR_SUCCESS == return_value) {
    return BaseInterfaceImpl::Initialize(param);
  }
  return return_value;
}

int ModuleLinkImpl::UnInitialize(void *param) {
  int return_value = YSOS_ERROR_SUCCESS;
  if (module_link_info_list_.size() == 0 || !IsInitialized()) {
    return return_value;
  }
  RepeaterModuleInfoPtrList *repeater_list_array[] = {
    &inherent_repeater_info_list_,
    &temporary_repeater_info_list_
  };
  do {
#if 1
    if (true/* == opened*/) {
      Stop();
    }
#endif
    for (int repeater_list_index = 0; repeater_list_index < 2; ++repeater_list_index) {
      for (RepeaterModuleInfoPtrListIterator it = repeater_list_array[repeater_list_index]->begin();
           it != repeater_list_array[repeater_list_index]->end(); ++it) {

        ModuleInterfacePtr repeater_module = NULL;
        repeater_module = ysos::GetModuleInterfaceManager()->FindInterface(
                            (((*it)->famous_name.size() != 0) ? (*it)->famous_name : (*it)->name));
        if (NULL == repeater_module) {
          continue;
        }
        for (ModulePtrEdgeListIterator it_edge = (*it)->current_edge_set.begin();
             it_edge != (*it)->current_edge_set.end(); ++it_edge) {
          repeater_module->Ioctl(CMD_MAPPINGPAIR_REMOVE, (void*)&(*it_edge));
        }
      }
    }
    if (module_link_info_list_.size() > 0) {
      Close(NULL);
    }

    /// RemoveModule
    for (ModuleLinkInfoIterator it = module_link_info_list_.begin(); it != module_link_info_list_.end(); ++it) {
      std::string module_name = (*it)->module_ptr_->GetName();
      RemoveModule(module_name);
    }
    module_link_info_list_.clear();
    sub_link_info_list_.clear();
    inherent_repeater_info_list_.clear();
    temporary_repeater_info_list_.clear();

    return_value = BaseInterfaceImpl::UnInitialize(param);
  } while (false);

  return return_value;
}

/**
 *@brief 获取接口的属性值
 *@param type_id[Input]： 属性的ID
 *@param type[Input/Output]：属性值的值
 *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值
 */
int ModuleLinkImpl::GetProperty(int type_id, void *type) {
  int return_value = YSOS_ERROR_SUCCESS;
  if (NULL == type) {
    return_value = YSOS_ERROR_INVALID_ARGUMENTS;
  } else {
    AutoLockOper lock(&module_link_lock_);
    switch (type_id) {
    /// 该ModuleLink是否固有的
    case PROP_MODULE_LINK_IS_INHERENT: {
      *reinterpret_cast<bool*>(type) = inherent_;
    }
    break;

    /// 该ModuleLink是否自动运行的的
    case PROP_MODULE_LINK_INFO_AUTORUN: {
      *reinterpret_cast<bool*>(type) = autorun_;
    }
    break;

    /// 该ModuleLink的深度
    case PROP_MODULE_LINK_INFO_DEPTH: {
      *reinterpret_cast<uint32_t*>(type) = link_depth_;
    }
    break;

    /// 整个Link的信息,但忽略Repeator的信息
    case PROP_MODULE_LINK_INFO_TYPE_ALL:
      //  for (ModuleLinkInfoIterator it = module_link_info_list_.begin();
      //    it != module_link_info_list_.end();
      //    ++it) {
      //    if ((*it)->next_module_list_.size() == 0) {
      //      //
      //    }
      //  }
      break;

    /// Link中所有的Destination(不含Repeator)的信息
    case PROP_MODULE_LINK_INFO_TYPE_LEAF:
      //  for (ModuleLinkInfoIterator it = module_link_info_list_.begin();
      //    it != module_link_info_list_.end();
      //    ++it) {
      //    if ((*it)->next_module_list_.size() == 0) {
      //      //
      //    }
      //  }
      break;

    /// Link的某一层的所有节点信息,忽略Repeator的信息
    case PROP_MODULE_LINK_INFO_TYPE_LAYER:
      //  for (ModuleLinkInfoIterator it = module_link_info_list_.begin();
      //    it != module_link_info_list_.end();
      //    ++it) {
      //    if ((*it)->next_module_list_.size() == 0) {
      //      //
      //    }
      //  }
      break;

    /// Link中某条边的节点信息,忽略Repeator的信息
    case PROP_MODULE_LINK_INFO_TYPE_EDGE:
      break;

    /// Link中某条边上的Repeater的信息
    case PROP_MODULE_LINK_INFO_TYPE_EDGE_REPEATER: {
      return_value = YSOS_ERROR_FAILED;
      RepeaterIncludeModule* repeater_info_ptr = reinterpret_cast<RepeaterIncludeModule*>(type);
      ModuleInterfacePtr curr_module = NULL;
      size_t pos_of_edge_separator = 0;
      bool inherent = false;
      bool exist_edge_not_in_repeater = false;
      RepeaterModuleInfoPtrListIterator repeater_info_ptr_list_iterator;

      do {
        YSOS_LOG_DEBUG("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
        if (NULL == repeater_info_ptr) {
          return_value = YSOS_ERROR_INVALID_ARGUMENTS;
          YSOS_LOG_ERROR("Get repeater and NULL == repeater_info_ptr.");
          break;
        }
        std::string module_or_edge_name = repeater_info_ptr->module_name_;
        YSOS_LOG_DEBUG("Monitor " << module_or_edge_name);
        if (repeater_info_ptr->module_name_.length() > 0) {
          pos_of_edge_separator = repeater_info_ptr->module_name_.find('|');
          if (pos_of_edge_separator != std::string::npos) {
            repeater_info_ptr->module_name_
              = repeater_info_ptr->module_name_.substr(0, pos_of_edge_separator);
          }
          curr_module = ysos::GetModuleInterfaceManager()->FindInterface(repeater_info_ptr->module_name_);
        }
        if (NULL == curr_module) {
          YSOS_LOG_ERROR("Get repeater and NULL == curr_module.");
          break;
        }
        if (ExistRepeaterWithBeginModule(curr_module, inherent, repeater_info_ptr_list_iterator)) {
          ModuleInterfacePtr repeater_module = NULL;
          repeater_module = ysos::GetModuleInterfaceManager()->FindInterface(
                              (((*repeater_info_ptr_list_iterator)->famous_name.size() != 0) ?
                               (*repeater_info_ptr_list_iterator)->famous_name : (*repeater_info_ptr_list_iterator)->name));

          /// the edge list in repeater?
          if (pos_of_edge_separator != std::string::npos) {
            ModuleInterfacePtr curr_module = NULL;
            ModuleInterfacePtr next_module = NULL;
            ModulePtrEdgeList edge_list;
            if (YSOS_ERROR_SUCCESS == GetEdgesFromString(
                  module_or_edge_name, edge_list, curr_module, next_module)
                && edge_list.size() != 0) {
              for (ModulePtrEdgeListIterator it = edge_list.begin();
                   it != edge_list.end(); ++it) {
                ModulePtrEdgeListIterator it2 = std::find(
                                                  (*repeater_info_ptr_list_iterator)->current_edge_set.begin(),
                                                  (*repeater_info_ptr_list_iterator)->current_edge_set.end(), (*it));
                if (it2 == (*repeater_info_ptr_list_iterator)->current_edge_set.end()) {
                  exist_edge_not_in_repeater = true;
                  break;
                }
              }
            }
          }
          return_value = YSOS_ERROR_SUCCESS;
          if (!exist_edge_not_in_repeater) {
            repeater_info_ptr->repeater_ptr_list_.push_back(repeater_module);
            YSOS_LOG_DEBUG("Get repeater and each_edge_in_repeater, need not new a repeater.");
            break;
          }
        } else {
          exist_edge_not_in_repeater = true;
        }
        YSOS_LOG_DEBUG("Get repeater by Ioctl CMD_MODULE_LINK_ADD_EDGE.");
        return_value = Ioctl((inherent ? CMD_MODULE_LINK_ADD_INHERENT_REPEATER : CMD_MODULE_LINK_ADD_EDGE),
                             &module_or_edge_name);
        if (return_value != YSOS_ERROR_SUCCESS) {
          YSOS_LOG_ERROR("Get repeater and exist_edge_not_in_repeater, failed to add edge.");
          break;
        }
        YSOS_LOG_DEBUG("Get repeater and exist_edge_not_in_repeater, succeeded to add edge.");
        if (!ExistRepeaterWithBeginModule(curr_module, inherent, repeater_info_ptr_list_iterator)) {
          YSOS_LOG_DEBUG("Get repeater and each_edge_in_repeater, need not new a repeater.");
          return_value = YSOS_ERROR_SUCCESS;
          break;
        }
        ModuleInterfacePtr repeater_module = NULL;
        repeater_module = ysos::GetModuleInterfaceManager()->FindInterface(
                            (((*repeater_info_ptr_list_iterator)->famous_name.size() != 0) ?
                             (*repeater_info_ptr_list_iterator)->famous_name : (*repeater_info_ptr_list_iterator)->name));
        if (NULL == repeater_module) {
          YSOS_LOG_ERROR("Get repeater and failed to FindInterface "
                         << (*repeater_info_ptr_list_iterator)->name << ".");
          return_value = YSOS_ERROR_FAILED;
          break;
        }
        repeater_info_ptr->repeater_ptr_list_.push_back(repeater_module);
        return_value = YSOS_ERROR_SUCCESS;
      } while (false);
    }
    break;

    /// Link中所有Repeater的信息
    case PROP_MODULE_LINK_INFO_TYPE_REPEATER:
      break;

    default:
      break;
    }
  }

  return return_value;
}

/**
 *@brief 设置接口的属性值
 *@param type_id[Input]： 属性的ID
 *@param type[Input/Output]： 属性值的值
 *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值
 */
int ModuleLinkImpl::SetProperty(int type_id, void *type) {
  int return_value = YSOS_ERROR_SUCCESS;
  if (NULL == type) {
    return_value = YSOS_ERROR_INVALID_ARGUMENTS;
  } else {
    AutoLockOper lock(&module_link_lock_);
    switch (type_id) {
    /// 该ModuleLink是否固有的
    case PROP_MODULE_LINK_IS_INHERENT: {
      inherent_ = *reinterpret_cast<bool*>(type);
    }
    break;

    /// 该ModuleLink是否自动运行的
    case PROP_MODULE_LINK_INFO_AUTORUN: {
      autorun_ = *reinterpret_cast<bool*>(type);
    }
    break;

    /// 该ModuleLink的深度
    case PROP_MODULE_LINK_INFO_DEPTH: {
      link_depth_ = *reinterpret_cast<uint32_t*>(type);
    }
    break;

    /// 整个Link的信息,但忽略Repeator的信息
    case PROP_MODULE_LINK_INFO_TYPE_ALL: {
      return_value = YSOS_ERROR_NOT_SUPPORTED;
    }
    break;

    /// Link中所有的Destination(不含Repeator)的信息
    case PROP_MODULE_LINK_INFO_TYPE_LEAF: {
      return_value = YSOS_ERROR_NOT_SUPPORTED;
    }
    break;

    /// Link的某一层的所有节点信息,忽略Repeator的信息
    case PROP_MODULE_LINK_INFO_TYPE_LAYER: {
      return_value = YSOS_ERROR_NOT_SUPPORTED;
    }
    break;

    /// Link中某条边的节点信息,忽略Repeator的信息
    case PROP_MODULE_LINK_INFO_TYPE_EDGE: {
      return_value = YSOS_ERROR_NOT_SUPPORTED;
    }
    break;

    /// Link中某条边上的Repeater的信息
    case PROP_MODULE_LINK_INFO_TYPE_EDGE_REPEATER: {
      return_value = YSOS_ERROR_NOT_SUPPORTED;
    }
    break;

    /// Link中所有Repeater的信息
    case PROP_MODULE_LINK_INFO_TYPE_REPEATER: {
      return_value = YSOS_ERROR_NOT_SUPPORTED;
    }
    break;

    default:
      break;
    }
  }

  return return_value;
}

/**
  *@brief 类似于模块的初始化功能, 同步, 从前往后, 打开相关的Module
  *@param param: NULL, 整个Link; else, ControlRangeString, std::string*, 指定tail, 子链(或子链的列表)
  *@param context: 上下文
  *@return 成功返回0，否则失败
  */
int ModuleLinkImpl::Open(LPVOID param, LPVOID context) {
  int return_value = YSOS_ERROR_FAILED;
  bool failed = false;
  AutoLockOper lock_oper(&module_link_lock_);
  SubLinkInfoInModuleLink sub_link;
  ModuleInterface* from_ptr = NULL;
  ModuleInterface* to_ptr = NULL;

  if (NULL == param || (NULL != param && YSOS_ERROR_SUCCESS == GetCaseOfControlRange(param, &from_ptr, &to_ptr)
                        && GetSubLinkByModule(to_ptr, sub_link_info_list_, sub_link))) {

    return_value = VisitLinkInfoTree(source_position_of_the_link, param, context, failed,
                                     OpenNode, ((NULL == param) ? NULL : &sub_link));
  }
  link_opened_ = true;
  return return_value;
}

/**
  *@brief 减少计数, 同步, 从前往后, 关闭相关的Module
  *@param param: NULL, 整个Link; else, ControlRangeString, std::string*, 指定tail, 子链(或子链的列表)
  *@return 成功返回0，否则失败
  */
int ModuleLinkImpl::Close(LPVOID param) {
  int return_value = YSOS_ERROR_FAILED;
  bool failed = false;
  AutoLockOper lock_oper(&module_link_lock_);
  SubLinkInfoInModuleLink sub_link;
  ModuleInterface* from_ptr = NULL;
  ModuleInterface* to_ptr = NULL;

  if (IsInitSucceeded()) {
    if (NULL == param || (NULL != param && YSOS_ERROR_SUCCESS == GetCaseOfControlRange(param, &from_ptr, &to_ptr)
                          && GetSubLinkByModule(to_ptr, sub_link_info_list_, sub_link))) {

      return_value = VisitLinkInfoTree(source_position_of_the_link, param, NULL, failed,
                                       CloseNode, ((NULL == param) ? NULL : &sub_link));
    }
  }

  link_opened_ = false;
  return return_value;
}

int ModuleLinkImpl::GetModuleLinkInfoPtrByName(ModuleLinkInfoPtr& link_info_ptr, const std::string &module_name) {

  int return_value = YSOS_ERROR_SUCCESS;
  ModuleInterfacePtr module_ptr;
  return_value = FindModuleByName(&module_ptr, module_name);
  if (return_value == YSOS_ERROR_SUCCESS) {
    IsModuleAlreadyInLink(module_ptr, link_info_ptr);
  }

  return return_value;
}

/*
  tail = (module is Destination or Repeater after Destination;
  ControlRange:
  "module"              /// only for one module;ioctl supported, other(Open,Close,Stop,Pause,Run,Flush) unsupported
  ""or"|"               /// whole
  "|tail"               /// from head to tail
  "|module"             /// UNSUPPORTED: from head to non-tail module; 非open/close, 先控制其后的, 再自此向头控制
  "module|tail"         /// from non-head module to tail
  "module|"             /// from non-head module to each tail
  "moduleA|moduleB"     /// UNSUPPORTED: moduleA not head, moduleB not tail, for sub link segment
*/
#define LINK_CTRL_RANGE_CASE_INVALID              0   /// 不支持
#define LINK_CTRL_RANGE_CASE_SINGLE_MODULE        1   /// 支持
#define LINK_CTRL_RANGE_CASE_WHOLE                2   /// 支持
#define LINK_CTRL_RANGE_CASE_HEAD_TO_MODULE       3   /// 不支持
#define LINK_CTRL_RANGE_CASE_HEAD_TO_TAIL         4   /// 支持
#define LINK_CTRL_RANGE_CASE_MODULE_TO_TAIL       5   /// 支持
#define LINK_CTRL_RANGE_CASE_MODULE_TO_EACH_TAIL  6   /// 支持
#define LINK_CTRL_RANGE_CASE_MODULE_TO_MODULE     7   /// 不支持

int ModuleLinkImpl::GetControlRangePart(void* param, std::string &range_from, std::string &range_to) {
  int return_value = LINK_CTRL_RANGE_CASE_INVALID;
  bool is_control_from_head = false;
  bool is_control_to_tail = false;
  bool is_control_to_each_tail = false;
  bool is_control_single_module = false;
  do {
    //////////////////////////////////////////////////////////////////////////
    /// Check parameter(s)
    if (NULL == param) {
      return_value = LINK_CTRL_RANGE_CASE_WHOLE;
      break;
    }

    std::string* control_range_ptr = reinterpret_cast<std::string*>(param);
    if (NULL == control_range_ptr) {
      break;
    }

    /// control range string
    std::string ctrl_range_str = *control_range_ptr;
    unsigned long string_length = ctrl_range_str.length();
    if (string_length == 0) {
      return_value = LINK_CTRL_RANGE_CASE_WHOLE;
      break;
    }

    unsigned long pos = ctrl_range_str.find_first_of('|');

    if (string_length == 1 && pos == 0) {
      return_value = LINK_CTRL_RANGE_CASE_WHOLE;
      break;
    }

    /// more than one '|'
    unsigned long pos_last = ctrl_range_str.find_last_of('|');
    if (pos_last != pos) {
      YSOS_LOG_ERROR(ctrl_range_str << " with more than one.");
      break;
    }

    //////////////////////////////////////////////////////////////////////////
    /// get range from and range to
    if (pos == 0) {
      range_to = ctrl_range_str.substr(pos + 1, string_length - pos - 1);
    } else if (pos == string_length - 1) {
      range_from = ctrl_range_str.substr(0, pos);
    } else if (0 < pos && pos < string_length - 1) {
      range_from = ctrl_range_str.substr(0, pos);
      range_to = ctrl_range_str.substr(pos + 1, string_length - pos - 1);
    } else {
      range_from = ctrl_range_str;
      range_to = ctrl_range_str;
    }

  } while (false);
  return return_value;
}

/// param: 控制范围, std::string*  
int ModuleLinkImpl::GetCaseOfControlRange(void* param,
    ModuleInterface** from_ptr_ptr, ModuleInterface** to_ptr_ptr) {
  static char* case_of_ctrl_range_prompt_strs[] = {
    (char*)"LINK_CTRL_RANGE_CASE_INVALID",
    (char*)"LINK_CTRL_RANGE_CASE_SINGLE_MODULE",
    (char*)"LINK_CTRL_RANGE_CASE_WHOLE",
    (char*)"LINK_CTRL_RANGE_CASE_HEAD_TO_MODULE",
    (char*)"LINK_CTRL_RANGE_CASE_HEAD_TO_TAIL",
    (char*)"LINK_CTRL_RANGE_CASE_MODULE_TO_TAIL",
    (char*)"LINK_CTRL_RANGE_CASE_MODULE_TO_EACH_TAIL",
    (char*)"LINK_CTRL_RANGE_CASE_MODULE_TO_MODULE"
  };  //need update fro linux  --//(char*)

  int return_value = LINK_CTRL_RANGE_CASE_INVALID;
  ModuleInterface* module_range_from_ptr = NULL;
  ModuleInterface* module_range_to_ptr = NULL;
  ModuleLinkInfoPtr link_info_ptr;
  std::string head_name_of_the_module_link = "";
  std::string range_from = "";
  std::string range_to = "";
  bool is_control_from_head = false;
  bool is_control_to_tail = false;
  bool is_control_to_each_tail = false;
  bool is_control_single_module = false;
  do {
    //////////////////////////////////////////////////////////////////////////
    /// Check parameter(s)
    if (NULL == param) {
      return_value = LINK_CTRL_RANGE_CASE_WHOLE;
      break;
    }

    std::string* control_range_ptr = reinterpret_cast<std::string*>(param);
    if (NULL == control_range_ptr) {
      break;
    }

    /// control range string
    std::string ctrl_range_str = *control_range_ptr;
    unsigned long string_length = ctrl_range_str.length();
    if (string_length == 0) {
      return_value = LINK_CTRL_RANGE_CASE_WHOLE;
      break;
    }

    unsigned long pos = ctrl_range_str.find_first_of('|');

    if (string_length == 1 && pos == 0) {
      return_value = LINK_CTRL_RANGE_CASE_WHOLE;
      break;
    }

    /// more than one '|'
    unsigned long pos_last = ctrl_range_str.find_last_of('|');
    if (pos_last != pos) {
      YSOS_LOG_ERROR(ctrl_range_str << " with more than one.");
      break;
    }

    head_name_of_the_module_link = (*source_position_of_the_link)->module_ptr_->GetName();
    if (head_name_of_the_module_link.length() == 0) {
      YSOS_LOG_ERROR("head_name_of_the_module_link is empty.");
      break;
    }

    //////////////////////////////////////////////////////////////////////////
    /// get range from and range to
    if (pos == 0) {
      is_control_from_head = true;
      range_to = ctrl_range_str.substr(pos + 1, string_length - pos - 1);
    } else if (pos == string_length - 1) {
      is_control_to_each_tail = true;
      range_from = ctrl_range_str.substr(0, pos);
    } else if (0 < pos && pos < string_length - 1) {
      range_from = ctrl_range_str.substr(0, pos);
      range_to = ctrl_range_str.substr(pos + 1, string_length - pos - 1);
    } else {
      range_from = ctrl_range_str;
      is_control_single_module = true;
    }
    if (head_name_of_the_module_link == range_from) {
      range_to = "";
      is_control_from_head = true;
    }
    if (range_to.size() > 0 && YSOS_ERROR_SUCCESS == GetModuleLinkInfoPtrByName(link_info_ptr, range_to)
        && NULL != link_info_ptr) {
      if (link_info_ptr->next_module_list_.size() == 0) {
        if (sub_link_info_list_.size() == 1) {
          is_control_to_each_tail = true;
        } else {
          is_control_to_tail = true;
        }
      } else {
        /// range_to是sub_link的情形.
        for (SubLinkInfoListIterator it = sub_link_info_list_.begin();
             it != sub_link_info_list_.end(); ++it) {
          if (it->module_ptr_ == link_info_ptr->module_ptr_) {
            is_control_to_tail = true;
            break;
          }
        }
      } /// 2017-03-02, deleted by XuLanyue
    }
    if (range_from.length() > 0 && range_from == range_to) {
      range_to = "";
      is_control_single_module = true;
    }

    if (range_from.length() > 0) {
      ModuleInterfacePtr module_ptr;
      if (YSOS_ERROR_SUCCESS != FindModuleByName(&module_ptr, range_from)) {
        return_value = LINK_CTRL_RANGE_CASE_INVALID;
        YSOS_LOG_DEBUG("Invalid module name for range from " << range_from);
        break;
      }
      module_range_from_ptr = module_ptr.get();
    }

    /// 若range_from后的模块为结尾的Repeater.
    if (range_from.size() > 0 && YSOS_ERROR_SUCCESS == GetModuleLinkInfoPtrByName(link_info_ptr, range_from)
        && NULL != link_info_ptr && link_info_ptr->next_module_list_.size() == 1) {
      ModuleLinkInfoPtr link_item_info_ptr;
      if (IsRepeaterModule(link_info_ptr->next_module_list_.front()->module_ptr_) &&
          IsModuleAlreadyInLink(link_info_ptr->next_module_list_.front()->module_ptr_, link_item_info_ptr)
          && NULL != link_item_info_ptr && link_item_info_ptr->next_module_list_.size() == 0) {
        range_to = "";
        is_control_to_each_tail = false;
        is_control_single_module = true;
      }
    }

    if (range_to.size() > 0) {
      ModuleInterfacePtr module_ptr;
      if (YSOS_ERROR_SUCCESS != FindModuleByName(&module_ptr, range_to)) {
        return_value = LINK_CTRL_RANGE_CASE_INVALID;
        YSOS_LOG_DEBUG("Invalid module name for range to " << range_to);
        break;
      }
      module_range_to_ptr = module_ptr.get();
    }

    //////////////////////////////////////////////////////////////////////////
    /// get case
    if (is_control_single_module && is_control_to_each_tail) {
      return_value = LINK_CTRL_RANGE_CASE_WHOLE;
      break;
    }

    if (range_from.size() > 0 && range_to.size() > 0) {
      if (is_control_single_module) {
        return_value = LINK_CTRL_RANGE_CASE_SINGLE_MODULE;
      } else {
        return_value = LINK_CTRL_RANGE_CASE_MODULE_TO_MODULE;
      }
    } else if (range_from.size() > 0 && range_to.size() == 0) {
      if (is_control_to_each_tail) {
        return_value = LINK_CTRL_RANGE_CASE_MODULE_TO_EACH_TAIL;
      } else {
        if (is_control_single_module) {
          return_value = LINK_CTRL_RANGE_CASE_SINGLE_MODULE;
        } else {
          return_value = LINK_CTRL_RANGE_CASE_MODULE_TO_TAIL;
        }
      }
    } else if (range_from.size() == 0 && range_to.size() > 0) {
      if (is_control_to_tail) {
        return_value = LINK_CTRL_RANGE_CASE_HEAD_TO_TAIL;
      } else {
        return_value = LINK_CTRL_RANGE_CASE_HEAD_TO_MODULE;
      }
    } else { /*if (range_from.size() == 0 && range_to.size() == 0) */
      /* */
    }
  } while (false);
  if (NULL != module_range_from_ptr && NULL != from_ptr_ptr) {
    *from_ptr_ptr = module_range_from_ptr;
  }
  if (NULL != module_range_to_ptr && NULL != to_ptr_ptr) {
    *to_ptr_ptr = module_range_to_ptr;
  }
  YSOS_LOG_DEBUG(" GetCaseOfControlRange " << case_of_ctrl_range_prompt_strs[return_value]);
  return return_value;
}

/**
  *@brief 控制函数，可对该接口进行详细控制.
    异步, 从后往前.
    若param中逻辑子链的名字为空则委托给各Module
    逻辑的,从某个DestinationModule往Source.
    传入的是一个Module的名字,发给Repeater该Module发出的所有的边.
  *@param ctrl_id[Input]： 详细控制ID号
  *@param param[Input/Output]：详细配置的参数
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值
  */
int ModuleLinkImpl::Ioctl(INT32 ctrol_id, LPVOID param) {
  AutoLockOper lock_oper(&module_link_lock_);
  int return_value = YSOS_ERROR_SUCCESS;
  ModuleInterface* module_range_from_ptr = NULL;
  ModuleInterface* module_range_to_ptr = NULL;
  std::string prompt_string = "ModuleLinkImpl.Ioctl.";
  std::string control_range_string = "";
  void* out_param = NULL;
  int case_of_control_range = LINK_CTRL_RANGE_CASE_INVALID;

  /// 对某些ctrol_id, 重新解释参数param及ctrol_id
  if (ctrol_id == CMD_MODULE_LINK_CTRL_MODULE) {

    /// Check parameter(s)
    if (NULL == param) {
      return_value = YSOS_ERROR_INVALID_ARGUMENTS;
      return return_value;
    }

    PropertyInfo* control_info = reinterpret_cast<PropertyInfo*>(param);
    if (NULL == control_info) {
      return_value = YSOS_ERROR_INVALID_ARGUMENTS;
      return return_value;
    }

    ctrol_id = control_info->id;
    out_param = control_info->param;
    control_range_string = control_info->str_id;

    case_of_control_range = GetCaseOfControlRange(&control_range_string,
                            &module_range_from_ptr, &module_range_to_ptr);
    if (LINK_CTRL_RANGE_CASE_INVALID == case_of_control_range
        || LINK_CTRL_RANGE_CASE_HEAD_TO_MODULE == case_of_control_range
        || LINK_CTRL_RANGE_CASE_MODULE_TO_MODULE == case_of_control_range) {
      return_value = YSOS_ERROR_INVALID_ARGUMENTS;
      return return_value;
    }
  }

  /// 对单个模块的控制
  if (case_of_control_range == LINK_CTRL_RANGE_CASE_SINGLE_MODULE) {
    return_value = ControlModule(ctrol_id, module_range_from_ptr, out_param);
    YSOS_LOG_DEBUG("Control single module and returns " << return_value);
    return return_value;
  }

  switch (ctrol_id) {

  /// 添加边到临时Repeater
  case CMD_MODULE_LINK_ADD_EDGE: {
    prompt_string += "OnAddEdgeToRepeater ";
    return_value = OnAddEdgeToRepeater((std::string*)param);
  }
  break;

  /// 添加边到固有Repeater
  case CMD_MODULE_LINK_ADD_INHERENT_REPEATER: {
    prompt_string += "OnAddInherentRepeater ";
    return_value = OnAddInherentRepeater(param);
  }
  break;

  /// 从Repeater移除边
  case CMD_MODULE_LINK_REMOVE_EDGE: {
    prompt_string += "OnRemoveEdgeFromRepeater ";
    return_value = OnRemoveEdgeFromRepeater((std::string*)param);
  }
  break;

  /// Open
  case CMD_OPEN: {
    prompt_string += "Open ";
    return_value = Open(param, NULL);   // Open(module_range_to_ptr, out_param);
  }
  break;

  /// Close
  case CMD_CLOSE: {
    prompt_string += "Close ";
    return_value = Close(param);
  }
  break;

  /// Stop
  case CMD_STOP: {
    prompt_string += "Stop ";
    return_value = Stop(&control_range_string);
  }
  break;

  /// Pause
  case CMD_PAUSE: {
    prompt_string += "Pause ";
    return_value = Pause(&control_range_string);
  }
  break;

  /// Run
  case CMD_RUN: {
    prompt_string += "Run ";
    return_value = Run(&control_range_string);
  }
  break;

  /// Flush
  case CMD_FLUSH: {
    prompt_string += "Flush ";
    return_value = Flush(&control_range_string);
  }
  break;

  default: {
    prompt_string += "CONTROL_TYPE_IOCTL ";
    return_value = ControlSubLink(NULL, NULL, param, ctrol_id, CONTROL_TYPE_IOCTL);
  }
  break;
  }
  YSOS_LOG_DEBUG(prompt_string << "and returns " << return_value);
  return return_value;
}

int ModuleLinkImpl::Stop(LPVOID param) {
  if (!IsInitSucceeded()) {
    YSOS_LOG_DEBUG("Try to stop the link which not init succeeded.");
    return YSOS_ERROR_SUCCESS;
  }

  AutoLockOper lock_oper(&module_link_lock_);
  YSOS_LOG_DEBUG("Stop the link");
  int return_value = IterateControlSubLink(param, CONTROL_COMMAND_STOP, CONTROL_TYPE_STANDARD);
  return return_value;
}

int ModuleLinkImpl::Pause(LPVOID param) {
  if (!IsInitSucceeded()) {
    YSOS_LOG_DEBUG("Try to pause the link which not init succeeded.");
    return YSOS_ERROR_SUCCESS;
  }

  AutoLockOper lock_oper(&module_link_lock_);
  YSOS_LOG_DEBUG("Pause the link");
  int return_value = IterateControlSubLink(param, CONTROL_COMMAND_PAUSE, CONTROL_TYPE_STANDARD);
  return return_value;
}

int ModuleLinkImpl::Run(LPVOID param) {
  if (!IsInitSucceeded()) {
    YSOS_LOG_DEBUG("Try to run the link which not init succeeded.");
    return YSOS_ERROR_SUCCESS;
  }

  AutoLockOper lock_oper(&module_link_lock_);
  YSOS_LOG_DEBUG("Run the link");
  int return_value = IterateControlSubLink(param, CONTROL_COMMAND_RUN, CONTROL_TYPE_STANDARD);
  return return_value;
}

int ModuleLinkImpl::Flush(LPVOID param) {
  if (!IsInitSucceeded()) {
    YSOS_LOG_DEBUG("Try to flush the link which not init succeeded.");
    return YSOS_ERROR_SUCCESS;
  }

  AutoLockOper lock_oper(&module_link_lock_);
  YSOS_LOG_DEBUG("Flush the link");
  int return_value = IterateControlSubLink(param, CONTROL_COMMAND_FLUSH, CONTROL_TYPE_STANDARD);
  return return_value;
}

int ModuleLinkImpl::GetState(UINT32 time_out, INT32 *state, LPVOID param) {
  AutoLockOper lock_oper(&module_link_lock_);
  int return_value = YSOS_ERROR_INVALID_ARGUMENTS;
  do {
    if (!state) {
      break;
    }
    if (!link_opened_) {
      *state = PROP_CLOSE;
      return_value = YSOS_ERROR_SUCCESS;
      break;
    }
    if (!(*source_position_of_the_link)->module_ptr_) {
      break;
    }
    return_value = (*source_position_of_the_link)->module_ptr_->GetState(time_out, state);
  } while (false);

  return return_value;
}

/// Control single module by ModuleLink.
int ModuleLinkImpl::ControlModule(INT32 control_id, ModuleInterface *module_ptr, void* param) {
  int return_value = YSOS_ERROR_SUCCESS;
  PropertyInfo* control_info = NULL;
  do {
    if (NULL == module_ptr) {
      return_value = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }
    switch (control_id) {
    case CMD_OPEN:
      return_value = module_ptr->Open(param);
      YSOS_LOG_DEBUG("Stop module " << module_ptr->GetName() << " returns " << return_value);
      break;
    case CMD_CLOSE:
      return_value = module_ptr->Close(param);
      YSOS_LOG_DEBUG("Stop module " << module_ptr->GetName() << " returns " << return_value);
      break;
    case CMD_STOP:
      return_value = module_ptr->Stop(param);
      YSOS_LOG_DEBUG("Stop module " << module_ptr->GetName() << " returns " << return_value);
      break;
    case CMD_PAUSE:
      return_value = module_ptr->Pause(param);
      YSOS_LOG_DEBUG("Stop module " << module_ptr->GetName() << " returns " << return_value);
      break;
    case CMD_RUN:
      return_value = module_ptr->Run(param);
      YSOS_LOG_DEBUG("Stop module " << module_ptr->GetName() << " returns " << return_value);
      break;
    case CMD_FLUSH:
      return_value = module_ptr->Flush(param);
      YSOS_LOG_DEBUG("Stop module " << module_ptr->GetName() << " returns " << return_value);
      break;
    // return_value = YSOS_ERROR_INVALID_ARGUMENTS;
    // break;
    default:
      return_value = module_ptr->Ioctl(control_id, param);
      YSOS_LOG_DEBUG("Ioctl module " << module_ptr->GetName() << " with CtrlId " << control_id << " returns " << return_value);
      break;
    }
  } while (false);

  return return_value;
}

/// 执行控制命令
int ModuleLinkImpl::ExecuteCommand(
  int command,
  int control_type,
  ModuleInterfacePtr module_ptr,
  void* param_ptr) {
  int return_value = YSOS_ERROR_SUCCESS;
  do {
    /// 若为无效控制命令则返回
    if (command == MODULE_INVALID_IOCTL_COMMAND) {
      break;
    }

    int temp_return_value = YSOS_ERROR_SUCCESS;
    /// CONTROL_TYPE_STANDARD
    if (control_type == CONTROL_TYPE_STANDARD) {
      if (!IsRepeaterModule(module_ptr)) {
        param_ptr = NULL;
      }
      if (command == CONTROL_COMMAND_STOP) {
        temp_return_value = module_ptr->Stop(param_ptr);
        YSOS_LOG_DEBUG("Execute Stop Command for module " << module_ptr->GetName() << " returns " << temp_return_value);
      } else if (command == CONTROL_COMMAND_PAUSE) {
        temp_return_value = module_ptr->Pause(param_ptr);
        YSOS_LOG_DEBUG("Execute Pause Command for module " << module_ptr->GetName() << " returns " << temp_return_value);
      } else if (command == CONTROL_COMMAND_RUN) {
        temp_return_value = module_ptr->Run(param_ptr);
        YSOS_LOG_DEBUG("Execute Run Command for module " << module_ptr->GetName() << " returns " << temp_return_value);
      } else if (command == CONTROL_COMMAND_FLUSH) {
        temp_return_value = module_ptr->Flush(param_ptr);
        YSOS_LOG_DEBUG("Execute Flush Command for module " << module_ptr->GetName() << " returns " << temp_return_value);
      }
      break;
    }

    /// CONTROL_TYPE_MODULE, CONTROL_TYPE_IOCTL
    temp_return_value = module_ptr->Ioctl(command, param_ptr);
    YSOS_LOG_DEBUG("Ioctl module " << module_ptr->GetName() << " with CtrlId " << command << " returns " << temp_return_value);
  } while (false);

  return return_value;
}

/**
  *@brief 从链的某个模块开始, 向链首方向, 控制单条逻辑子链; 若遇Repeater则控制权被转给Repeater并返回.
  *@param link_info_ptr[Input]： ModuleLinkInfoPtr
  *@param param_ptr[Input]：控制参数
  *@param command[Input]： 控制命令
  *@param control_type[Input]：控制类别
  *@return： 返回0表示控制权转交给了Repeater,返回1表示需要继续处理
  */
int ModuleLinkImpl::ControlSubLink(
  ModuleLinkInfoPtr from_ptr,
  ModuleLinkInfoPtr to_ptr,
  void* param_ptr,
  int command,
  int control_type) {
  bool is_tail = true;
  ModuleLinkInfoPtr current_ptr;
  if (NULL != from_ptr) {
    YSOS_LOG_DEBUG("ControlSubLink range from " << from_ptr->module_ptr_->GetName());
  } else {
    YSOS_LOG_DEBUG("ControlSubLink range from head");
  }

  if (NULL != to_ptr) {
    YSOS_LOG_DEBUG("ControlSubLink range to " << to_ptr->module_ptr_->GetName());
  } else {
    YSOS_LOG_DEBUG("ControlSubLink range to tail");
  }

  for (current_ptr = to_ptr; current_ptr != NULL; /**/) {
    ModuleInterfacePtr module_ptr = current_ptr->module_ptr_;
    if (module_ptr) {
      YSOS_LOG_DEBUG("Control Module " << (int64_t)param_ptr << " " << module_ptr->GetName());

      /// 若遇到了Repeater, 则控制转交给该Repeater.
      if (IsRepeaterModule(module_ptr)) {
        YSOS_LOG_DEBUG("Reaches a RepeaterModule " << (int64_t)param_ptr << " "
                       << module_ptr->GetName());
        return ExecuteCommand(command, control_type, module_ptr, param_ptr);
      }
      ExecuteCommand(command, control_type, module_ptr, param_ptr);
    }

    if (from_ptr == current_ptr) {
      break;
    }

    if (current_ptr->prev_module_list_.size() > 0) {
      /// 除了Repeater, 每个Module至多只有一个PrevModule
      current_ptr = current_ptr->prev_module_list_.front();
    } else {
      current_ptr = NULL;
    }
    if (is_tail == true) {
      is_tail = false;
    }
  }
  return 1;
}

/**
  *@brief 迭代地, 从链的某个模块开始, 向链首方向, 控制单条逻辑子链
  *@param link_info_ptr[Input]： ModuleLinkInfoPtr
  *@param param_ptr[Input]：控制参数, 若为空则表示对整个Link进行控制
  *@param command[Input]： 控制命令
  *@param control_type[Input]：控制类别
  *@return： 返回0表示控制权转交给了Repeater,返回1表示需要继续处理
  */
int ModuleLinkImpl::IterateControlSubLink(void* param_ptr, int command, int control_type) {
  int return_value = YSOS_ERROR_SUCCESS;

  ModuleInterface *module_from_ptr = NULL;
  ModuleInterface *module_to_ptr = NULL;

  ModuleLinkInfoPtr from_ptr;
  ModuleLinkInfoPtr to_ptr;
  int case_of_control_range = LINK_CTRL_RANGE_CASE_INVALID;

  do {

    std::string* range_string_ptr = reinterpret_cast<std::string*>(param_ptr);
    if (NULL == range_string_ptr) {
      case_of_control_range = LINK_CTRL_RANGE_CASE_WHOLE;
    } else {
      case_of_control_range = GetCaseOfControlRange(range_string_ptr, &module_from_ptr, &module_to_ptr);
      if (LINK_CTRL_RANGE_CASE_INVALID == case_of_control_range) {
        return_value = YSOS_ERROR_INVALID_ARGUMENTS;
        return return_value;
      }
    }

    if (NULL != module_from_ptr) {
      IsModuleAlreadyInLink(module_from_ptr, from_ptr);
    }

    if (NULL != module_to_ptr) {
      IsModuleAlreadyInLink(module_to_ptr, to_ptr);
    }

    if (LINK_CTRL_RANGE_CASE_SINGLE_MODULE == case_of_control_range) {
      return_value = ExecuteCommand(command, control_type, from_ptr->module_ptr_, param_ptr);;
      return return_value;
    }

    if (NULL == from_ptr) {
      from_ptr = *source_position_of_the_link;
    }

    if (LINK_CTRL_RANGE_CASE_WHOLE == case_of_control_range
        || LINK_CTRL_RANGE_CASE_MODULE_TO_EACH_TAIL == case_of_control_range) {
      if (LINK_CTRL_RANGE_CASE_WHOLE == case_of_control_range) {
        YSOS_LOG_DEBUG("IterateControlSubLink for whole link");
      } else { /*if (LINK_CTRL_RANGE_CASE_MODULE_TO_EACH_TAIL == case_of_control_range) */
        YSOS_LOG_DEBUG("IterateControlSubLink for Module2EachTail");
      }

      /// 遍历每条逻辑子链
      for (SubLinkInfoListIterator it = sub_link_info_list_.begin();
           it != sub_link_info_list_.end(); ++it) {
        ModuleLinkInfoPtr link_info_ptr = (*(*it).link_info_position_);

        /// 当前子链是期望的那条子链
        if (/* LINK_CTRL_RANGE_CASE_MODULE_TO_EACH_TAIL == case_of_control_range && */
          NULL != from_ptr && !IsModuleOnSubLink(from_ptr->module_ptr_.get(), &(*it))) {
          continue;
        }

        /// 执行控制
        return_value = ControlSubLink(from_ptr, link_info_ptr, param_ptr, command, control_type);
        if (return_value == YSOS_ERROR_SUCCESS) {
          continue; //  return return_value;
        }
      }
      break;
    }

    if (LINK_CTRL_RANGE_CASE_HEAD_TO_TAIL == case_of_control_range
        || LINK_CTRL_RANGE_CASE_MODULE_TO_TAIL == case_of_control_range) {
      if (LINK_CTRL_RANGE_CASE_HEAD_TO_TAIL == case_of_control_range) {
        YSOS_LOG_DEBUG("IterateControlSubLink for sub link");
      } else { /* if (LINK_CTRL_RANGE_CASE_MODULE_TO_TAIL == case_of_control_range)*/
        YSOS_LOG_DEBUG("IterateControlSubLink for module2tail");
      }

      /// 遍历每条逻辑子链
      for (SubLinkInfoListIterator it = sub_link_info_list_.begin();
           it != sub_link_info_list_.end(); ++it) {
        ModuleLinkInfoPtr link_info_ptr = (*(*it).link_info_position_);

        /// 当前子链是期望的那条子链
        if (NULL != to_ptr && !IsModuleOnSubLink(to_ptr->module_ptr_.get(), &(*it))) {
          continue;
        }
        if (NULL != from_ptr && !IsModuleOnSubLink(from_ptr->module_ptr_.get(), &(*it))) {
          continue;
        }

        /// 执行控制
        return_value = ControlSubLink(from_ptr, link_info_ptr, param_ptr, command, control_type);
      }
      break;
    }
    if (LINK_CTRL_RANGE_CASE_HEAD_TO_MODULE == case_of_control_range
        || LINK_CTRL_RANGE_CASE_MODULE_TO_MODULE == case_of_control_range) {
      if (LINK_CTRL_RANGE_CASE_HEAD_TO_MODULE == case_of_control_range) {
        YSOS_LOG_DEBUG("IterateControlSubLink for head2module");
      } else { /* if (LINK_CTRL_RANGE_CASE_MODULE_TO_MODULE == case_of_control_range)*/
        YSOS_LOG_DEBUG("IterateControlSubLink for module2module");
      }

      /// 遍历每条逻辑子链
      for (SubLinkInfoListIterator it = sub_link_info_list_.begin();
           it != sub_link_info_list_.end(); ++it) {
        ModuleLinkInfoPtr link_info_ptr = (*(*it).link_info_position_);

        /// 当前子链是期望的那条子链
        if (!IsModuleOnSubLink(to_ptr->module_ptr_.get(), &(*it))) {
          continue;
        }
        if (NULL != from_ptr && !IsModuleOnSubLink(from_ptr->module_ptr_.get(), &(*it))) {
          continue;
        }

        /// 找到控制点
        for (ModuleLinkInfoIterator it2 = module_link_info_list_.begin();
             it2 != module_link_info_list_.end(); ++it2) {
          if ((*it2)->module_ptr_.get() == module_to_ptr) {
            link_info_ptr = *it2;
            break;
          }
        }

        /// 执行控制
        return_value = ControlSubLink(from_ptr, link_info_ptr, param_ptr, command, control_type);
      }
      break;
    }

  } while (false);

  if (return_value > YSOS_ERROR_SUCCESS) {
    return_value = YSOS_ERROR_SUCCESS;
  }

  return return_value;
}

int ModuleLinkImpl::GetEdgesFromString(
  const std::string& vertex_or_edge_name,
  ModulePtrEdgeList& edge_list,
  ModuleInterfacePtr& curr_module,
  ModuleInterfacePtr& next_module) {
  int return_value = YSOS_ERROR_FAILED;
  bool will_get_edges = true;
  do {
    if (vertex_or_edge_name.length() == 0) {
      break;
    }

    int32_t pos = vertex_or_edge_name.find('|');
    if (pos >= 0) {
      will_get_edges = false;
    }

    if (will_get_edges) {
      curr_module = ysos::GetModuleInterfaceManager()->FindInterface(vertex_or_edge_name);
      if (!curr_module) {
        break;
      }
      ModuleLinkInfoPtr curr_module_info = NULL;
      IsModuleAlreadyInLink(curr_module, curr_module_info);
      if (!curr_module_info) {
        break;
      }
      if (curr_module_info->next_module_list_.size() != 0) {
        for (ModuleLinkInfoIterator it = curr_module_info->next_module_list_.begin();
             it != curr_module_info->next_module_list_.end(); ++it) {
          ModulePtrEdge edge;
          edge.first = curr_module;
          edge.second = (*it)->module_ptr_;
          edge_list.push_back(edge);
        }
      } else {
        ModulePtrEdge edge;
        edge.first = curr_module;
        edge.second = NULL;
        edge_list.push_back(edge);
      }
    } else {
      std::string module_name = vertex_or_edge_name.substr(0, pos);
      std::string next_module_name = vertex_or_edge_name.substr(pos + 1, vertex_or_edge_name.length() - pos - 1);
      curr_module = ysos::GetModuleInterfaceManager()->FindInterface(module_name);
      if (next_module_name.length() == 0) {
        next_module = NULL;
      } else {
        next_module = ysos::GetModuleInterfaceManager()->FindInterface(next_module_name);
      }
      if (NULL == curr_module || (next_module_name.length() != 0 && NULL == next_module)) {
        break;
      }
      {
        ModulePtrEdge edge;
        edge.first = curr_module;
        edge.second = next_module;
        edge_list.push_back(edge);
      }
    }
    return_value = YSOS_ERROR_SUCCESS;
  } while (false);
  return return_value;
}

bool ModuleLinkImpl::ExistRepeaterWithBeginLevel(
  uint32_t level,
  bool &inherent,
  bool act_as_destination,
  RepeaterModuleInfoPtrListIterator& repeater_info_ptr_list_iterator) {
  bool ret_value = false;
  uint32_t link_depth = 0;
  this->GetLinkDepth(&link_depth);
  if (!ret_value) {
    std::string repeater_name = MakeRepeaterName(
                                  "", link_depth, level, true, act_as_destination);

    uint32_t name_length = repeater_name.length();
    for (RepeaterModuleInfoPtrListIterator it = inherent_repeater_info_list_.begin();
         it != inherent_repeater_info_list_.end(); ++it) {
      #ifdef _WIN32
        if ((*it)->name.length() > name_length
            && 0 == stricmp((*it)->name.c_str() + ((*it)->name.length() - name_length),
                            repeater_name.c_str())) {
          ret_value = true;
          inherent = true;
          repeater_info_ptr_list_iterator = it;
          break;
        }
      #else
        if ((*it)->name.length() > name_length
            && 0 == strcasecmp((*it)->name.c_str() + ((*it)->name.length() - name_length),
                            repeater_name.c_str())) {
          ret_value = true;
          inherent = true;
          repeater_info_ptr_list_iterator = it;
          break;
        }
      #endif
    }
  }
  if (!ret_value) {
    std::string repeater_name = MakeRepeaterName(
                                  "", link_depth, level, false, act_as_destination);
    uint32_t name_length = repeater_name.length();
    for (RepeaterModuleInfoPtrListIterator it = temporary_repeater_info_list_.begin();
         it != temporary_repeater_info_list_.end(); ++it) {
      #ifdef _WIN32
        if ((*it)->name.length() > name_length
            && 0 == stricmp((*it)->name.c_str() + ((*it)->name.length() - name_length), repeater_name.c_str())) {
          ret_value = true;
          inherent = false;
          repeater_info_ptr_list_iterator = it;
          break;
        }
      #else
        if ((*it)->name.length() > name_length
            && 0 == strcasecmp((*it)->name.c_str() + ((*it)->name.length() - name_length), repeater_name.c_str())) {
          ret_value = true;
          inherent = false;
          repeater_info_ptr_list_iterator = it;
          break;
        }
      #endif  
    }
  }
  return ret_value;
}

bool ModuleLinkImpl::ExistRepeaterWithBeginModule(
  ModuleInterfacePtr prev,
  bool &inherent,
  RepeaterModuleInfoPtrListIterator& repeater_info_ptr_list_iterator) {
  bool return_value = false;
  if (!return_value && inherent_repeater_info_list_.size() > 0) {
    for (RepeaterModuleInfoPtrListIterator it = inherent_repeater_info_list_.begin();
         it != inherent_repeater_info_list_.end(); ++it) {
      for (ModulePtrEdgeListIterator it_edge = (*it)->current_edge_set.begin();
           it_edge != (*it)->current_edge_set.end(); ++it_edge) {
        if ((*it_edge).first.get() == prev.get()) {
          return_value = true;
          repeater_info_ptr_list_iterator = it;
          inherent = true;
          break;
        }
      }
      if (return_value) {
        break;
      }
    }
  }

  if (!return_value && temporary_repeater_info_list_.size() > 0) {
    for (RepeaterModuleInfoPtrListIterator it = temporary_repeater_info_list_.begin();
         it != temporary_repeater_info_list_.end(); ++it) {
      for (ModulePtrEdgeListIterator it_edge = (*it)->current_edge_set.begin();
           it_edge != (*it)->current_edge_set.end(); ++it_edge) {
        if ((*it_edge).first.get() == prev.get()) {
          return_value = true;
          repeater_info_ptr_list_iterator = it;
          inherent = false;
          break;
        }
      }
      if (return_value) {
        break;
      }
    }
  }
  return return_value;
}

bool IsModuleInfoInList(
  ModuleLinkInfoPtr& link_info_ptr,
  ModuleLinkInfoList& link_info_list) {
  bool return_value = false;
  if (link_info_ptr && link_info_list.size() > 0) {
    for (ModuleLinkInfoList::iterator it = link_info_list.begin();
         it != link_info_list.end(); ++it) {
      if ((*it) == link_info_ptr) {
        return_value = true;
        break;
      }
    }
  }
  return return_value;
}

/// 边在ModuleLink中作为普通边已存在
/// 若存在则返回true, 否则返回false
bool ModuleLinkImpl::IsEdgeAsNormalAlreadyInModuleLink(const ModulePtrEdge& edge) {
  bool return_value = false;
  ModuleLinkInfoPtr link_info_ptr = NULL;
  ModuleLinkInfoPtr next_link_info_ptr = NULL;
  do {
    if (false == IsModuleAlreadyInLink(edge.first, link_info_ptr) || !link_info_ptr) {
      break;
    }
    if (false == IsModuleAlreadyInLink(edge.second, next_link_info_ptr) || !next_link_info_ptr) {
      break;
    }
    return_value = IsModuleInfoInList(
                     next_link_info_ptr, link_info_ptr->next_module_list_);
  } while (false);
  return return_value;
}

/// 存在边在ModuleLink中作为普通边已存在
/// 若存在则返回true, 否则返回false
bool ModuleLinkImpl::ExistEdgeAsNormalAlreadyInModuleLink(const ModulePtrEdgeList& edge_list) {
  bool return_value = false;
  for (ModulePtrEdgeList::const_iterator it = edge_list.begin();
       it != edge_list.end(); ++it) {
    if (IsEdgeAsNormalAlreadyInModuleLink((*it))) {
      return_value = true;
      break;
    }
  }
  return return_value;
}

bool ModuleLinkImpl::OnCaseExistRepeaterWithBeginLevel(
  ModulePtrEdgeList& edge_list,
  bool in_inherent_repeater_list,
  RepeaterModuleInfoPtrListIterator& repeater_info_ptr_list_iterator,
  RepeaterModuleInfoPtr& repeater_info_ptr,
  bool& restart_required) {
  bool return_value = false;
  /// 是否有边已在链中. 若有则停止, 否则不停.
  bool will_stop = false;
  bool broken = false;
  int32_t state = PROP_CLOSE;
  do {
    ModuleInterfacePtr repeater_ptr = GetModuleInterfaceManager()->FindInterface(
                                        (((*repeater_info_ptr_list_iterator)->famous_name.size() != 0) ?
                                         (*repeater_info_ptr_list_iterator)->famous_name : (*repeater_info_ptr_list_iterator)->name));
    if (!repeater_ptr) {
      YSOS_LOG_ERROR("OnCaseExistRepeaterWithBeginLevel failed to FindInterface for repeater "
                     << (*repeater_info_ptr_list_iterator)->name << ".");
      break;
    }
    if (ExistEdgeAsNormalAlreadyInModuleLink(edge_list)) {
      will_stop = true;
    }
#if ALWAYS_STOP_LINK_IF_ADD_OR_DEL_EDGE
    if (YSOS_ERROR_SUCCESS != GetState(0xffffffff, &state)) {
      break;
    }
    if (state != PROP_CLOSE && state != PROP_STOP) {
      will_stop = true;
    }
#endif

    if (state == PROP_RUN) {
      restart_required = true;
    }
    if (will_stop) {
      YSOS_LOG_DEBUG("Need to stop.");
      Stop();
    } else {
      YSOS_LOG_DEBUG("Need not to stop.");
    }
    for (ModulePtrEdgeListIterator it = edge_list.begin(); it != edge_list.end(); ++it) {
      (*repeater_info_ptr_list_iterator)->current_edge_set.push_back((*it));
      /// MakeCallbackName
      ModuleInterfacePtr curr_module = (*it).first;
      ModuleInterfacePtr next_module = (*it).second;
      CallbackInterfacePtr curr_callback = NULL;
      CallbackInterfacePtr next_callback = NULL;
      std::string callback_instance_name = MakeRepeaterCallbackInstanceName(
                                             curr_module, curr_callback, edge_list);
      YSOS_LOG_DEBUG("callback_instance_name: " << callback_instance_name);
      REGISTER_LOGIC_CLASS_NAME(
        ysos::GetCallbackInterfaceManager(),
        callback_instance_name,
        BASE_REPEATER_CALLBACK_IMPL_CLASS_NAME);
      ysos::CallbackInterfacePtr repeater_callback_ptr_
        = ysos::GetCallbackInterfaceManager()->FindInterface(callback_instance_name);
      if (NULL == repeater_callback_ptr_) {
        YSOS_LOG_ERROR("OnCaseExistRepeaterWithBeginLevel failed to FindInterface for callback "
                       << callback_instance_name << ".");
        broken = true;
        break;
      }

      ModuleInterfacePtr prev_module_of_callback = NULL;
      if (YSOS_ERROR_SUCCESS != repeater_callback_ptr_->GetProperty(PROP_PREV_MODULE, &prev_module_of_callback)) {
        YSOS_LOG_ERROR("OnCaseExistRepeaterWithBeginLevel failed to GetProperty PROP_PREV_MODULE.");
        broken = true;
        break;
      }
      if (prev_module_of_callback != curr_module) {
        int64_t owner_id = 0;
        if (YSOS_ERROR_SUCCESS != repeater_callback_ptr_->SetProperty(PROP_PREV_MODULE, &curr_module)) {
          YSOS_LOG_ERROR("OnCaseExistRepeaterWithBeginLevel failed to SetProperty PROP_PREV_MODULE.");
          broken = true;
          break;
        }
        if (YSOS_ERROR_SUCCESS != repeater_ptr->AddCallback(repeater_callback_ptr_, owner_id, 0)) {
          YSOS_LOG_ERROR("OnCaseExistRepeaterWithBeginLevel failed to AddCallback.");
          broken = true;
          break;
        }
      }
    }
    if (broken) {
      break;
    }

    /// 断链(备份LinkInfo, 止能原有普通边).
    for (ModulePtrEdgeListIterator it = edge_list.begin(); it != edge_list.end(); ++it) {
      if (IsEdgeAsNormalAlreadyInModuleLink((*it))) {
        if (YSOS_ERROR_SUCCESS != UnlinkModules((*it).first, (*it).second, &repeater_info_ptr)) {
          YSOS_LOG_ERROR("OnCaseExistRepeaterWithBeginLevel failed to UnlinkModules.");
          broken = true;
          break;
        }
      }
    }
    if (broken) {
      break;
    }

    int return_value_ = AddModule(repeater_ptr, edge_list);
    YSOS_LOG_DEBUG(" Add repeater1 Module returns " << return_value_);
    return_value = (YSOS_ERROR_SUCCESS == return_value_);
  } while (false);
  return return_value;
}

bool ModuleLinkImpl::OnCaseNotExistRepeaterWithBeginLevel(
  ModulePtrEdgeList& edge_list,
  bool in_inherent_repeater_list,
  RepeaterModuleInfoPtrListIterator& repeater_info_ptr_list_iterator,
  RepeaterModuleInfoPtr& repeater_info_ptr,
  ModuleInterfacePtr& curr_module,
  ModuleInterfacePtr& next_module,
  uint32_t& level,
  uint32_t level_repeater,
  bool& act_as_destination,
  bool& restart_required,
  bool inherent_repeater) {
  bool return_value = false;
  ModuleInterfacePtr repeater_ptr;
  int32_t state = PROP_CLOSE;
  do {

    if (YSOS_ERROR_SUCCESS != GetState(0xffffffff, &state)) {
      break;
    }
    if (state == PROP_RUN) {
      restart_required = true;
    }
    if (state != PROP_CLOSE && state != PROP_STOP) {
      Stop();
    }
    /// 否则, 创建一个Repeater, 添加边到该Repeater, 再将该Repeater添加到临时列表.
    uint32_t link_depth = 0;
    this->GetLinkDepth(&link_depth);
    std::string repeater_name = MakeRepeaterName(
                                  this->GetName(), link_depth, level, in_inherent_repeater_list, act_as_destination);
    //  YSOS_LOG_DEBUG("RepeaterName:" << repeater_name);

    REGISTER_LOGIC_CLASS_NAME(
      GetModuleInterfaceManager(),
      repeater_name,
      BASE_REPEATER_MODULE_IMPL_CLASS_NAME);
    repeater_ptr = GetModuleInterfaceManager()->FindInterface(repeater_name);
    if (!repeater_ptr) {
      break;
    }

    repeater_info_ptr = boost::make_shared<RepeaterModuleInfo>();
    if (in_inherent_repeater_list) {
      for (ModulePtrEdgeListIterator it = edge_list.begin(); it != edge_list.end(); ++it) {
        repeater_info_ptr->current_edge_set.push_back((*it));
      }
    } else {
      for (ModulePtrEdgeListIterator it = edge_list.begin(); it != edge_list.end(); ++it) {
        repeater_info_ptr->current_edge_set.push_back((*it));
      }
    }
    repeater_info_ptr->inherent = false;
    repeater_info_ptr->name = repeater_name;
    repeater_info_ptr->level_after = level_repeater >> 16;
    temporary_repeater_info_list_.push_back(repeater_info_ptr);

    /// MakeCallbackName
    CallbackInterfacePtr curr_callback = NULL;
    CallbackInterfacePtr next_callback = NULL;
#if 0
    std::string callback_instance_name = MakeRepeaterCallbackInstanceName(curr_module, curr_callback, next_module);
    YSOS_LOG_DEBUG("callback_instance_name: " << callback_instance_name);

    /// 对于一个Repeater, 它有多少个Prev, 就要有多少份匹配的Callback的实例
    REGISTER_LOGIC_CLASS_NAME(
      ysos::GetCallbackInterfaceManager(),
      callback_instance_name,
      BASE_REPEATER_CALLBACK_IMPL_CLASS_NAME);
    ysos::CallbackInterfacePtr repeater_callback_ptr_
      = ysos::GetCallbackInterfaceManager()->FindInterface(callback_instance_name);
    int64_t owner_id = 0;
    repeater_callback_ptr_->SetProperty(PROP_PREV_MODULE, &curr_module);
    repeater_ptr->AddCallback(repeater_callback_ptr_, owner_id, 0);

#else
    /// 对于一个Repeater, 它有多少个Prev, 就要有多少份匹配的Callback的实例
    ModuleInterfacePtrList prev_module_list;
    for (ModulePtrEdgeListIterator it2 = edge_list.begin();
         it2 != edge_list.end(); ++it2) {
      if (!IsModuleInList(prev_module_list, it2->first)) {
        prev_module_list.push_back(it2->first);
      }
    }

    for (ModuleInterfacePtrListIterator it2 = prev_module_list.begin();
         it2 != prev_module_list.end(); ++it2) {
      /// MakeCallbackName
      ModuleInterfacePtr curr_module = (*it2);
      CallbackInterfacePtr curr_callback = NULL;
      std::string callback_instance_name = "";
#if 1
      callback_instance_name = MakeRepeaterCallbackInstanceName(curr_module, curr_callback, edge_list);

      YSOS_LOG_DEBUG("callback_instance_name: " << callback_instance_name);
#else
      callback_instance_name = curr_repeater_info.name;
#endif
      REGISTER_LOGIC_CLASS_NAME(
        ysos::GetCallbackInterfaceManager(),
        callback_instance_name,
        BASE_REPEATER_CALLBACK_IMPL_CLASS_NAME);
      ysos::CallbackInterfacePtr repeater_callback_ptr_
        = ysos::GetCallbackInterfaceManager()->FindInterface(callback_instance_name);
      if (NULL == repeater_callback_ptr_) {
        YSOS_LOG_ERROR("Failed to FindInterface for " << callback_instance_name);
        continue;
      }
      int64_t owner_id = 0;
      repeater_callback_ptr_->SetProperty(
        PROP_PREV_MODULE, &curr_module);
      repeater_ptr->AddCallback(
        repeater_callback_ptr_, owner_id, 0);
    }
#endif
    repeater_ptr->SetProperty(
      PROP_MODULE_LINK_LEVEL, &level_repeater);

    /// 断链(备份LinkInfo, 止能原有普通边).
    for (ModulePtrEdgeListIterator it = edge_list.begin(); it != edge_list.end(); ++it) {
      UnlinkModules((*it).first, (*it).second, &repeater_info_ptr);
    }

    int return_value_ = AddModule(repeater_ptr, edge_list);
    YSOS_LOG_DEBUG("Add repeater Module returns " << return_value_);
    return_value = true;
  } while (false);
  return return_value;
}

/// 解析是顶点还是边. 若是顶点, 导出该顶点发出的全部边.
/// 判断边中是否有在已存在的Repeater内的.
/// 若有, 则添加边到该Repeater.
/// 否则, 判断是否有该顶点所在的层发出的Repeater, 若有, 则添加边到该Repeater.
/// 否则, 创建一个Repeater, 添加边到该Repeater, 再将该Repeater添加到临时列表.
/// 断链(备份LinkInfo, 止能原有普通边).
int ModuleLinkImpl::OnAddEdgeToRepeater(std::string* param) {
  int return_value = YSOS_ERROR_FAILED;
  bool will_get_edges = true;
  ModuleInterfacePtr curr_module = NULL;
  ModuleInterfacePtr next_module = NULL;
  ModulePtrEdgeList edge_list;
  bool restart_required = false;
  LightLock light_lock;
  AutoLockOper auto_lock(&light_lock);
  do {
    if (!param) {
      YSOS_LOG_ERROR("Try to OnAddEdgeToRepeater with null parameter.");
      break;
    }

    /// 解析是顶点还是边. 若是顶点, 导出该顶点发出的全部边.
    const std::string& vertex_or_edge_name = *param;
    if (YSOS_ERROR_SUCCESS != GetEdgesFromString(
          vertex_or_edge_name, edge_list, curr_module, next_module)
        || edge_list.size() == 0) {
      YSOS_LOG_ERROR("Try to OnAddEdgeToRepeater but failed to GetEdgesFromString.");
      break;
    }
    bool exist_in_link = false;
    bool in_inherent_repeater_list = false;
    RepeaterModuleInfoPtr repeater_info_ptr = NULL;
    RepeaterModuleInfoPtrListIterator repeater_info_ptr_list_iterator;
    uint32_t level = 0;
    bool act_as_destination = true;
    for (ModulePtrEdgeList::iterator it = edge_list.begin(); it != edge_list.end(); ++it) {
      if (NULL != it->second) {
        act_as_destination = false;
        break;
      }
    }
    edge_list.front().first->GetProperty(
      PROP_MODULE_LINK_LEVEL, &level);
    uint32_t level_repeater = (level & 0xffff0000) + (act_as_destination ? 0 : ((level & 0x0000ffff) + 1));
    level >>= 16;
    if (ExistRepeaterWithBeginLevel(
          level,
          in_inherent_repeater_list,
          act_as_destination,
          repeater_info_ptr_list_iterator)) {
      YSOS_LOG_DEBUG("Try to OnAddEdgeToRepeater while ExistRepeaterWithBeginLevel.");
      exist_in_link = true;
    } else {
      YSOS_LOG_DEBUG("Try to OnAddEdgeToRepeater while NotExistRepeaterWithBeginLevel.");
    }

    if (exist_in_link) {
      /// 若有, 则添加边到该Repeater.
      if (false == OnCaseExistRepeaterWithBeginLevel(
            edge_list, in_inherent_repeater_list, repeater_info_ptr_list_iterator, repeater_info_ptr, restart_required)) {
        YSOS_LOG_ERROR("Try to OnAddEdgeToRepeater but failed OnCaseExistRepeaterWithBeginLevel.");
        break;
      }
    } else {
      if (false == OnCaseNotExistRepeaterWithBeginLevel(
            edge_list,
            in_inherent_repeater_list,
            repeater_info_ptr_list_iterator,
            repeater_info_ptr,
            curr_module,
            next_module,
            level,
            level_repeater,
            act_as_destination,
            restart_required)) {
        YSOS_LOG_ERROR("Try to OnAddEdgeToRepeater but failed OnCaseNotExistRepeaterWithBeginLevel.");
        break;
      }
    }
    return_value = YSOS_ERROR_SUCCESS;
  } while (false);
#if ALWAYS_STOP_LINK_IF_ADD_OR_DEL_EDGE
  if (YSOS_ERROR_SUCCESS == return_value && (!IsEachModuleLinked() || !ExistSourceModule())) {
    YSOS_LOG_ERROR("Try to run a module link with name " << this->GetName()
                   << " but not each module linked or no source module.");
    DumpAll();
    return_value = YSOS_ERROR_FAILED;
  }

  if (YSOS_ERROR_SUCCESS == return_value && !RebuildModuleLinkInfoTree()) {
    YSOS_LOG_ERROR("Try to run a module link with name " << this->GetName()
                   << " but failed to rebuild the module link info tree.");
    return_value = YSOS_ERROR_FAILED;
  }
  if (YSOS_ERROR_SUCCESS == return_value && restart_required) {
    Run();
  }
#else
  if (restart_required) {
    if (IsEachModuleLinked() && ExistSourceModule()) {
      /// NextToDo: 应该提供一个对应的部分更新Link信息的函数
      RebuildModuleLinkInfoTree();
    }
    Run();
  }
#endif
  return return_value;
}

/// NextToDo:
/// 从Repeater中移除边, 使能其中的原有普通边.
/// 解析是顶点还是边. 若是顶点, 导出该顶点发出的全部边.
/// 过滤掉边中不存在于Repeater内的.
/// 判断要移除的边里是否有固有边, 若有则置将停止链标志.
/// 若Repeater从边表移除边后边表就空了,则置将停止链标志.
/// 若某边是临时边则直接移除, 否则
/// 若有, 则停止链, 并: 添加边到该Repeater.
/// 断链(备份LinkInfo, 使能原有普通边).
/// 若Repeater从边表移出的边中有原普通边, 则必须Stop, 并加入到ModuleLink.
/// 若Repeater从边表移除边后边表就空了,则必须Stop, 从链中移出该Repeater.
int ModuleLinkImpl::OnRemoveEdgeFromRepeater(std::string* param) {
  int return_value = YSOS_ERROR_FAILED;
  bool will_get_edges = true;
  ModuleInterfacePtr curr_module = NULL;
  ModuleInterfacePtr next_module = NULL;
  ModulePtrEdgeList edge_list;
  bool restart_required = false;
  LightLock light_lock;
  AutoLockOper auto_lock(&light_lock);
  do {
    if (!param) {
      break;
    }

    /// 解析是顶点还是边. 若是顶点, 导出该顶点发出的全部边.
    const std::string& vertex_or_edge_name = *param;
    if (YSOS_ERROR_SUCCESS != GetEdgesFromString(
          vertex_or_edge_name, edge_list, curr_module, next_module)
        || edge_list.size() == 0) {
      break;
    }
  } while (false);
  return return_value;
}

bool ModuleLinkImpl::MakeEdgeList(
  ModulePtrEdgeList& edge_list,
  EdgeOfModuleNameList& edge_of_name_list) {
  bool return_value = false;
  bool broken = false;
  do {
    for (EdgeOfModuleNameListIterator it_edge = edge_of_name_list.begin();
         it_edge != edge_of_name_list.end(); ++it_edge) {
      ModuleInterfacePtr prev = NULL;
      ModuleInterfacePtr next = NULL;
      prev = GetModuleInterfaceManager()->FindInterface(it_edge->first);
      if (!prev) {
        YSOS_LOG_DEBUG("Module " << it_edge->first
                       << " as prev instance not found.");
        broken = true;
        break;
      }
      if (it_edge->second.length() > 0) {
        next = GetModuleInterfaceManager()->FindInterface(it_edge->second);
        if (!next) {
          YSOS_LOG_DEBUG("Module " << it_edge->second
                         << " as next instance not found.");
          broken = true;
          break;
        }
      }
      edge_list.push_back(std::make_pair(prev, next));
    }
    if (broken) {
      break;
    }
    return_value = true;
  } while (false);
  return return_value;
}

/// 添加固有Repeater.
/// 调用时机: Link在Open前,而其他固有Module已链入.
int ModuleLinkImpl::OnAddInherentRepeater(void* param) {
  int return_value = YSOS_ERROR_FAILED;
  LightLock light_lock;
  AutoLockOper auto_lock(&light_lock);
  bool broken = false;
  do {
    if (!param) {
      break;
    }
    InherentRepeaterConfInfoMap* repeater_info_list_ptr
      = reinterpret_cast<InherentRepeaterConfInfoMap*>(param);

    RepeaterModuleInfoPtrListIterator repeater_info_ptr_list_iterator;
    for (InherentRepeaterConfInfoMapIterator it = repeater_info_list_ptr->begin();
         it != repeater_info_list_ptr->end(); ++it) {
      RepeaterModuleInfoPtr repeater_info_ptr = NULL;
      InherentRepeaterConfInfo& curr_repeater_info = (*it).second;
      uint32_t level_repeater = curr_repeater_info.level;

      std::string repeater_name = curr_repeater_info.name;
      std::string repeater_famous_name = curr_repeater_info.famous_name;
      YSOS_LOG_DEBUG("RepeaterName:" << repeater_name);

      /// 生成Repeater对象
      REGISTER_LOGIC_CLASS_NAME(
        GetModuleInterfaceManager(),
        ((repeater_famous_name.size() != 0) ? repeater_famous_name : repeater_name),
        BASE_REPEATER_MODULE_IMPL_CLASS_NAME);
      ModuleInterfacePtr repeater_ptr
        = GetModuleInterfaceManager()->FindInterface(((repeater_famous_name.size() != 0) ? repeater_famous_name : repeater_name));
      if (!repeater_ptr) {
        YSOS_LOG_DEBUG(" creation failed.");
        broken = true;
        break;
      }

      ModulePtrEdgeList edge_list;
      if (!MakeEdgeList(edge_list, curr_repeater_info.edge_list)) {
        break;
      }
      repeater_info_ptr = boost::make_shared<RepeaterModuleInfo>();
      for (ModulePtrEdgeListIterator it2 = edge_list.begin();
           it2 != edge_list.end(); ++it2) {
        repeater_info_ptr->current_edge_set.push_back((*it2));
      }
      repeater_info_ptr->inherent = true;
      repeater_info_ptr->famous_name = repeater_famous_name;
      repeater_info_ptr->name = repeater_name;
      repeater_info_ptr->level_after = (*it).first;
      inherent_repeater_info_list_.push_back(repeater_info_ptr);

      /// 对于一个Repeater, 它有多少个Prev, 就要有多少份匹配的Callback的实例
      ModuleInterfacePtrList prev_module_list;
      for (ModulePtrEdgeListIterator it2 = edge_list.begin(); it2 != edge_list.end(); ++it2) {
        if (!IsModuleInList(prev_module_list, it2->first)) {
          prev_module_list.push_back(it2->first);
        }
      }

      if (curr_repeater_info.callback_name_list.size() == 0) {
        curr_repeater_info.callback_name_list.push_back(BASE_REPEATER_CALLBACK_IMPL_CLASS_NAME);
      }
      for (ModuleInterfacePtrListIterator it3 = prev_module_list.begin(); it3 != prev_module_list.end(); ++it3) {
        ModuleInterfacePtr curr_module = (*it3);
        for (std::list<std::string>::iterator it2 = curr_repeater_info.callback_name_list.begin();
             it2 != curr_repeater_info.callback_name_list.end(); ++it2) {
          //        /// MakeCallbackName
          //        CallbackInterfacePtr curr_callback = NULL;
          //        std::string callback_instance_name = "";
          //        /// callback_name事实上应该是class name
          //#if 1
          //        callback_instance_name = curr_repeater_info.famous_name;
          //        if (callback_instance_name.length() == 0) {
          //          callback_instance_name = MakeRepeaterCallbackInstanceName(curr_module, curr_callback, edge_list);
          //        }
          //
          //        YSOS_LOG_DEBUG("callback_instance_name: " << callback_instance_name);
          //#else
          //        callback_instance_name = curr_repeater_info.name;
          //#endif
          std::string callback_instance_name = (*it2);
          //REGISTER_LOGIC_CLASS_NAME(
          //  ysos::GetCallbackInterfaceManager(),
          //  callback_instance_name,
          //  curr_repeater_info.callback_name);
          ysos::CallbackInterfacePtr repeater_callback_ptr_
            = ysos::GetCallbackInterfaceManager()->FindInterface(callback_instance_name);
          if (NULL == repeater_callback_ptr_) {
            YSOS_LOG_ERROR("Failed to FindInterface for " << curr_repeater_info.name
                           << " with class name " << callback_instance_name);
            continue;
          }
          int64_t owner_id = 0;
          repeater_callback_ptr_->SetProperty(PROP_PREV_MODULE, &curr_module); // &curr_module // &repeater_ptr
          repeater_ptr->AddCallback(repeater_callback_ptr_, owner_id, 0);
        }
      }

      repeater_ptr->SetProperty(
        PROP_MODULE_LINK_LEVEL, &level_repeater);

      /// 断链(备份LinkInfo, 止能原有普通边).
      for (ModulePtrEdgeListIterator it = edge_list.begin(); it != edge_list.end(); ++it) {
        UnlinkModules((*it).first, (*it).second, &repeater_info_ptr);
      }

      int return_value_ = AddModule(repeater_ptr, edge_list);
      YSOS_LOG_DEBUG("Add repeater Module returns " << return_value_);
    }
    if (broken) {
      break;
    }
#if 1   //  0, normal; 1, only for test
    if (IsEachModuleLinked() && ExistSourceModule()) {
      RebuildModuleLinkInfoTree();
    }
#endif
    return_value = YSOS_ERROR_SUCCESS;
  } while (false);
  return return_value;
}

bool WillAddToNextModuleListOfPrevModule(
  ModuleLinkInfoPtr prev,
  ModuleLinkInfoPtr next) {
  /// 若prev不在list之中, 则添加
  bool will_add = true;
  if (next->prev_module_list_.size() > 0) {
    for (ModuleLinkInfoIterator it = next->prev_module_list_.begin();
         it != next->prev_module_list_.end(); ++it) {
      if ((*it).get() == prev.get()) {
        will_add = false;
        break;
      }
    }
  }
  return will_add;
}

bool ModuleLinkImpl::AddModuleLinkInfo(
  ModuleLinkInfoPtr prev,
  ModuleLinkInfoPtr next) {
  /// Prev和Next只有都不可添加时才真的不可添加
  if (!CanAddToPrevField(prev, next)) {
    return false;
  }

  std::string prefix = prev->module_path_ + "/";
  //  next->module_path_ = prefix + next->module_path_;
  UpdatePathForSelfAndDescendent(next, prefix, true);

  /// 若prev不在list之中, 则添加
  bool will_add = WillAddToNextModuleListOfPrevModule(prev, next);
  if (will_add) {
    next->prev_module_list_.push_back(prev);
  }

#if 0 //  1, for temporary
  YSOS_LOG_DEBUG("prev_module_list of " << next->module_ptr_->GetName());
  for (ModuleLinkInfoIterator it = next->prev_module_list_.begin();
       it != next->prev_module_list_.end(); ++it) {
    YSOS_LOG_DEBUG("  " << (*it).get()->module_ptr_->GetName());
  }
#endif

  /// 按优先级插入
  //  prev->next_module_list_.push_back(next);
  InsertModuleToNextModueListOfPrevModule(prev, next);

  return true;
}

int GetLinkCase(ModuleInterfacePtr curr_module_ptr, ModuleInterfacePtr prev_module_ptr, ModuleInterfacePtr next_module_ptr) {
  /// 1, curr
  /// 2, prev
  /// 4, next
  int link_case = LINK_CASE_ILLEGAL;
  if (curr_module_ptr) {
    link_case |= 1;
  }
  if (prev_module_ptr) {
    link_case |= 2;
  }
  if (next_module_ptr) {
    link_case |= 4;
  }
  return link_case;
}

bool ModuleLinkImpl::InsertModuleIfPossible(ModuleInterfacePtr module_ptr, ModuleLinkInfoPtr &module_link_info_ptr, bool &already_in_list) {
  bool return_value = false;

  if (!IsModuleAlreadyInLink(module_ptr, module_link_info_ptr)) {
    std::string module_link_name = this->GetName();
    module_ptr->SetProperty(PROP_MODULE_LINK, &module_link_name);
    already_in_list = false;
    module_link_info_ptr = boost::make_shared<ModuleLinkInfo>(module_ptr);
    module_link_info_ptr->is_strategy_ = IsRepeaterModule(module_ptr);
    module_link_info_list_.push_back(module_link_info_ptr);
    SetLinkCompleted(false);
    return_value = true;
  }

  return return_value;
}

void ModuleLinkImpl::InsertModuleIfPossible(
  ModuleInterfacePtr curr_module_ptr,
  ModuleInterfacePtr prev_module_ptr,
  ModuleInterfacePtr next_module_ptr,
  bool &curr_already_in_list,
  bool &prev_already_in_list,
  bool &next_already_in_list,
  ModuleLinkInfoPtr &curr_module_link_info_ptr,
  ModuleLinkInfoPtr &prev_module_link_info_ptr,
  ModuleLinkInfoPtr &next_module_link_info_ptr) {
  if (prev_module_ptr) {
    InsertModuleIfPossible(
      prev_module_ptr, prev_module_link_info_ptr, prev_already_in_list);
  }
  if (curr_module_ptr) {
    InsertModuleIfPossible(
      curr_module_ptr, curr_module_link_info_ptr, curr_already_in_list);
  }
  if (next_module_ptr) {
    InsertModuleIfPossible(
      next_module_ptr, next_module_link_info_ptr, next_already_in_list);
  }
}

/**
这里假定prev,curr,next所在链均无环
prev,curr,next若所在链长度==1则其Module可来自外部或内部.                      //NOLINT
prev,curr,next若所在链长度>1则其Module必来自本ModuleLink已有Module集合.       //NOLINT
prev,curr若非空则可能指向其所在链的任何一个节点                                  //NOLINT
prev与curr均非空时连接的条件:curr为其所在链的首节点,且prev!=curr,且无环              //NOLINT
curr与next均非空时连接的条件:next为其所在链的首节点,且curr!=next,且无环              //NOLINT
不能出现超过一个非策略的源//NOLINT
参数检测
不在列表中则尝试加入
*/
int ModuleLinkImpl::AddModule(ModuleInterfacePtr curr_module_ptr, ModuleInterfacePtr prev_module_ptr, ModuleInterfacePtr next_module_ptr) {
  int ret_value = YSOS_ERROR_SUCCESS;
  int link_case = LINK_CASE_ILLEGAL;
  int link_task = LINK_CASE_ILLEGAL;

  bool curr_already_in_list = true;
  bool prev_already_in_list = true;
  bool next_already_in_list = true;

  ModuleLinkInfoPtr curr_module_link_info_ptr = NULL;
  ModuleLinkInfoPtr prev_module_link_info_ptr = NULL;
  ModuleLinkInfoPtr next_module_link_info_ptr = NULL;

  bool prev_curr_match = false;
  bool curr_next_match = false;

  bool prev_curr_linked_already = false;
  bool curr_next_linked_already = false;
  bool prev_next_linked_already = false;

  ModuleLinkInfoPtr prev_link_head_ptr = NULL;
  ModuleLinkInfoPtr curr_link_head_ptr = NULL;
  ModuleLinkInfoPtr next_link_head_ptr = NULL;
  do {
    /// Parameter check
    if (!curr_module_ptr                      /// curr_module_ptr不能为空
        || curr_module_ptr == prev_module_ptr   /// curr与prev不能相同
        || next_module_ptr == curr_module_ptr   /// next与curr不能相同
        || (prev_module_ptr == next_module_ptr  /// next与prev不能相同且非空
            && prev_module_ptr)) {
      YSOS_LOG_ERROR("invalid param for module link: " << GetName());
      ret_value = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }

    std::string cur_module_name = curr_module_ptr->GetName();

    link_case = GetLinkCase(curr_module_ptr, prev_module_ptr, next_module_ptr);

    InsertModuleIfPossible(curr_module_ptr, prev_module_ptr, next_module_ptr,
                           curr_already_in_list, prev_already_in_list, next_already_in_list,
                           curr_module_link_info_ptr, prev_module_link_info_ptr, next_module_link_info_ptr);

    /// Check module linked already
    if (!CanAddToPrevField(curr_module_link_info_ptr)) {
      YSOS_LOG_DEBUG("curr module linked already.: " << cur_module_name << " : " << this->GetName());
      ret_value = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }

    if (next_module_ptr && !CanAddToPrevField(next_module_link_info_ptr, curr_module_link_info_ptr)) {
      YSOS_LOG_DEBUG("next module linked already : " << this->GetName());
      ret_value = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }

    if (link_case == LINK_CASE_PREV_CURR || link_case == LINK_CASE_PREV_CURR_NEXT) {
      prev_curr_match = IsPrevOutAndNextInMatched(prev_module_ptr, curr_module_ptr);
    }
    if (link_case == LINK_CASE_CURR_NEXT || link_case == LINK_CASE_PREV_CURR_NEXT) {
      curr_next_match = IsPrevOutAndNextInMatched(curr_module_ptr, next_module_ptr);
    }
    if ((link_case == LINK_CASE_PREV_CURR
         && prev_curr_match == false)                 /// Not matched
        || (link_case == LINK_CASE_CURR_NEXT
            && curr_next_match == false)
        || (link_case == LINK_CASE_PREV_CURR_NEXT
            && prev_curr_match == false
            && curr_next_match == false)) {
      if (true == curr_already_in_list) {
        ret_value = YSOS_ERROR_INVALID_ARGUMENTS;
        break;
      } else {
        /* */
        ret_value = YSOS_ERROR_MODULE_ERROR_BASE + ADD_MODULE_CURR_ADDED;
        break;
      }
    }

    /// Update link task
    link_task = link_case;
    if (link_case == LINK_CASE_PREV_CURR) {
      if (prev_curr_match == false) {
        link_task = LINK_CASE_ILLEGAL;
      }
    } else if (link_case == LINK_CASE_CURR_NEXT) {
      if (curr_next_match == false) {
        link_task = LINK_CASE_ILLEGAL;
      }
    } else if (link_case == LINK_CASE_PREV_CURR_NEXT) {
      if (prev_curr_match == false && curr_next_match == false) {
        link_task = LINK_CASE_ILLEGAL;
      } else if (prev_curr_match == false) {
        link_task = LINK_CASE_CURR_NEXT;
      } else if (curr_next_match == false) {
        link_task = LINK_CASE_PREV_CURR;
      }
    }

    //  IsLinkLenMoreThan1()

    /// 获取模块所在链的链信息列表的头
    if (prev_module_ptr) {
      GetTheHeadOfLinkInfoListByModule(prev_module_ptr, prev_link_head_ptr);
    }
    GetTheHeadOfLinkInfoListByModule(curr_module_ptr, curr_link_head_ptr);
    
    if (next_module_ptr) {
      GetTheHeadOfLinkInfoListByModule(next_module_ptr, next_link_head_ptr);
    }

#if 1   //  Check prev is not null and next is source
    if ((link_task & LINK_CASE_PREV_CURR) == LINK_CASE_PREV_CURR) {
      if (prev_link_head_ptr
          && ((!IsRepeaterModule(curr_module_ptr) && !IsRepeaterModule(prev_module_ptr))
              && (ExistSourceModule(curr_link_head_ptr) || IsSourceModule(curr_module_ptr)))) {
#if 1
        ret_value = YSOS_ERROR_INVALID_ARGUMENTS;
        break;
#else
        if (link_task == LINK_CASE_PREV_CURR) {
          link_task = LINK_CASE_ILLEGAL;
        } else if (link_task == LINK_CASE_PREV_CURR_NEXT) {
          link_task = LINK_CASE_CURR_NEXT;
        }
#endif
      }
    }
    if ((link_task & LINK_CASE_CURR_NEXT) == LINK_CASE_CURR_NEXT) {
      if (curr_link_head_ptr
          && ((!IsRepeaterModule(curr_module_ptr) && !IsRepeaterModule(prev_module_ptr))
              && (ExistSourceModule(next_link_head_ptr) || IsSourceModule(next_module_ptr)))) {
#if 1
        ret_value = YSOS_ERROR_INVALID_ARGUMENTS;
        break;
#else
        if (link_task == LINK_CASE_CURR_NEXT) {
          link_task = LINK_CASE_ILLEGAL;
        } else if (link_task == LINK_CASE_PREV_CURR_NEXT) {
          link_task = LINK_CASE_PREV_CURR;
        }
#endif
      }
    }
#endif

#if 1   //  Check double source
    if ((link_task & LINK_CASE_PREV_CURR) == LINK_CASE_PREV_CURR) {
      if (ExistSourceModule(prev_link_head_ptr)
          && ExistSourceModule(curr_link_head_ptr) &&
          (!IsRepeaterModule(prev_module_ptr) && !IsRepeaterModule(curr_module_ptr))) {
        if (link_task == LINK_CASE_PREV_CURR) {
          link_task = LINK_CASE_ILLEGAL;
        } else if (link_task == LINK_CASE_PREV_CURR_NEXT) {
          link_task = LINK_CASE_CURR_NEXT;
        }
      }
    }
    if ((link_task & LINK_CASE_CURR_NEXT) == LINK_CASE_CURR_NEXT) {
      if (ExistSourceModule(curr_link_head_ptr)
          && ExistSourceModule(next_link_head_ptr) &&
          (!IsRepeaterModule(curr_module_ptr) && !IsRepeaterModule(next_module_ptr))) {
        if (link_task == LINK_CASE_CURR_NEXT) {
          link_task = LINK_CASE_ILLEGAL;
        } else if (link_task == LINK_CASE_PREV_CURR_NEXT) {
          link_task = LINK_CASE_PREV_CURR;
        }
      }
    }
#endif

#if 1   //  link
    if ((link_task & LINK_CASE_PREV_CURR) == LINK_CASE_PREV_CURR) {
      if (!ExistCycleIfModulesLinked(prev_link_head_ptr, curr_link_head_ptr)) {
        AddModuleLinkInfo(prev_module_link_info_ptr, curr_module_link_info_ptr);
      }
    }
    if ((link_task & LINK_CASE_CURR_NEXT) == LINK_CASE_CURR_NEXT) {
      if (!ExistCycleIfModulesLinked(curr_link_head_ptr, next_link_head_ptr)) {
        AddModuleLinkInfo(curr_module_link_info_ptr, next_module_link_info_ptr);
      }
    }
#endif
#if 0
    /// 是否所有Module都在同一棵树中了,且有头有尾
    if (IsEachModuleLinked() && ExistSourceModule(curr_link_head_ptr)) {
      RebuildModuleLinkInfoTree();
      ret_value = link_task;
    }
#endif
  } while (false);
  return ret_value;
}

/**
  *@brief 添加中继器模块到模块链, 并设置其路由集//NOLINT
  *@param repeater_ptr[Input]： 要添加的中继器模块指针//NOLINT
  *@param edges_list[Input]： 该中继器的路由集//NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值//NOLINT
  */
int ModuleLinkImpl::AddModule(ModuleInterfacePtr repeater_ptr, const ModulePtrEdgeList &edges_list) {
  int return_value = YSOS_ERROR_FAILED;
  do {
    //  参数检测//NOLINT
    if (!repeater_ptr ||
        !IsRepeaterModule(repeater_ptr) ||
        edges_list.size() == 0) {
      return_value = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }

    //  扫描得到PrevModuleList和NextModuleList

    //  扫描得到Link中原有的对应路由集

    //  添加匹配的路由集

    //  对PrevModuleList中的各个Module,
    //    保存其NextCallbackQueue

    ///   存放固有的Repeater的相关信息.
    //  inherent_repeater_info_list_;

    ///   存放临时的Repeater的相关信息.
    //  temporary_repeater_info_list_;
    //    清空其NextCallbackQueue,
    //    添加RepeaterModule的PrevCallback;

    //  对RepeaterModule,依次
    //    添加NextModuleList中各个Module的PrevCallback
    for (ModulePtrEdgeList::const_iterator it = edges_list.begin(); it != edges_list.end(); ++it) {
      int return_value_ = AddModule(repeater_ptr, (*it).first, (*it).second);
      YSOS_LOG_DEBUG("Add repeater with prev and next returns " << return_value_);
      repeater_ptr->Ioctl(CMD_MAPPINGPAIR_ADD, (void*)&(*it));
    }

    //  设置ModuleLink
    ModuleLinkInterfacePtr link_ptr;
    repeater_ptr->GetProperty(PROP_MYLINK, &link_ptr);
    if (!link_ptr) {
      ModuleLinkInterface* this_ptr = reinterpret_cast<ModuleLinkInterface*>(this);
      ModuleLinkInterfacePtr module_link_ptr(this_ptr);
      repeater_ptr->SetProperty(PROP_MYLINK, &module_link_ptr);
    }

    return_value = YSOS_ERROR_SUCCESS;
  } while (false);
  return return_value;
}

/// NextToDo: 考虑Repeater扮演Destination角色的情形.
/// 边的链接信息是否在Repeater中已有记录
bool ExistLinkInfoBetweenModulesOfEdge(
  ModuleInterfacePtr prev,
  ModuleInterfacePtr next,
  RepeaterModuleInfoPtrList& repeater_info_ptr_list,
  RepeaterModuleInfoPtrListIterator& repeater_info_ptr_list_iterator) {
  bool return_value = false;
  do {
    if (!prev || !next || repeater_info_ptr_list.size() == 0) {
      break;
    }

    //  对于RepeaterModuleInfoPtrList中的每一个.
    for (RepeaterModuleInfoPtrListIterator it = repeater_info_ptr_list.begin();
         it != repeater_info_ptr_list.end(); ++it) {
      if ((*it)->backup_link_info.size() == 0) {
        continue;
      }
      //  对于其LinkInfoBetweenModulesList,
      for (LinkInfoBetweenModulesListIterator it_inner = (*it)->backup_link_info.begin();
           it_inner != (*it)->backup_link_info.end(); ++it_inner) {
        //  若该边的记录已存在,则保存相关信息,并返回.
        if ((*it_inner).edge.first.get() == prev.get()
            && (*it_inner).edge.second.get() == next.get()) {
          repeater_info_ptr_list_iterator = it;
          return_value = true;
          break;
        }
      }
      //  退出外层循环.
      if (return_value) {
        break;
      }
    }
  } while (false);
  return return_value;
}

/// NextToDo: 考虑Repeater扮演Destination角色的情形.
/// 边的链接信息是否在某Repeater中已有记录.
/// 若有则返回true;否则返回false.
bool ExistLinkInfoBetweenModulesOfEdge(
  ModuleInterfacePtr prev,
  ModuleInterfacePtr next,
  RepeaterModuleInfoPtr repeater_info_ptr) {
  bool return_value = false;
  do {
    if (!prev || !next || !repeater_info_ptr) {
      break;
    }

    //  对于其LinkInfoBetweenModulesList,
    for (LinkInfoBetweenModulesListIterator it_inner = repeater_info_ptr->backup_link_info.begin();
         it_inner != repeater_info_ptr->backup_link_info.end(); ++it_inner) {
      //  若该边的记录已存在,则保存相关信息,并返回.
      if ((*it_inner).edge.first.get() == prev.get()
          && (*it_inner).edge.second.get() == next.get()) {
        return_value = true;
        break;
      }
    }
  } while (false);
  return return_value;
}

bool RemoveModuleFromNextModuleList(
  ModuleLinkInfoPtr prev_info,
  ModuleLinkInfoPtr next_info) {
  bool return_value = false;
  do {
    if (!prev_info || !next_info) {
      break;
    }
#if 0
    for (ModuleLinkInfoIterator it = prev_info->next_module_list_.begin();
         it != prev_info->next_module_list_.end(); ++it) {
      if ((*it)->module_ptr_ == next_info->module_ptr_) {
        prev_info->next_module_list_.erase(it);
      }
    }
#else
    prev_info->next_module_list_.remove(next_info);
#endif
    return_value = true;
  } while (false);
  return return_value;
}

bool RemoveModuleFromPrevModuleList(
  ModuleLinkInfoPtr prev_info,
  ModuleLinkInfoPtr next_info) {
  bool return_value = false;
  do {
    if (!prev_info || !next_info) {
      break;
    }
#if 0
    for (ModuleLinkInfoIterator it = next_info->prev_module_list_.begin();
         it != next_info->prev_module_list_.end(); ++it) {
      if ((*it)->module_ptr_ == prev_info->module_ptr_) {
        next_info->prev_module_list_.erase(it);
      }
    }
#else
    next_info->prev_module_list_.remove(prev_info);
#endif
    return_value = true;
  } while (false);
  return return_value;
}

int ModuleLinkImpl::RemoveNonRepeaterModule(const ModuleInterfacePtr &module_ptr) {
  int return_value = YSOS_ERROR_INVALID_ARGUMENTS;
  do {
    ModuleLinkInfoPtr module_info_ptr;
    if (IsModuleAlreadyInLink(module_ptr, module_info_ptr)) {
      RemoveEachNextCallbackFromModule(module_info_ptr);
      return_value = YSOS_ERROR_SUCCESS;
    }
  } while (false);
  return return_value;
}

int ModuleLinkImpl::RemoveRepeaterModule(const ModuleInterfacePtr &module_ptr, bool fixed_repeater) {
  int return_value = YSOS_ERROR_SUCCESS;
  do {
  } while (false);
  return return_value;
}

bool ModuleLinkImpl::ExistLinkInfoBetweenModulesOfEdgeInLink(
  ModuleInterfacePtr prev,
  ModuleInterfacePtr next,
  bool& in_inherent_repeater_list,
  RepeaterModuleInfoPtrListIterator& repeater_info_ptr_list_iterator) {
  bool return_value = false;
  do {
    if (!prev || !next
        || (inherent_repeater_info_list_.size() == 0
            && temporary_repeater_info_list_.size() == 0)) {
      break;
    }
    if (ExistLinkInfoBetweenModulesOfEdge(
          prev, next, inherent_repeater_info_list_, repeater_info_ptr_list_iterator)) {
      in_inherent_repeater_list = true;
      return_value = true;
      break;
    }
    if (ExistLinkInfoBetweenModulesOfEdge(
          prev, next, temporary_repeater_info_list_, repeater_info_ptr_list_iterator)) {
      in_inherent_repeater_list = false;
      return_value = true;
      break;
    }
  } while (false);
  return return_value;
}

/**
  *@brief 断开前后两个模块的链接//NOLINT
   若不做恢复,则先关闭这两个模块//NOLINT
   若必要则备份PrevModule的NextCallbackQueue、//NOLINT
   PrevModule的NextModuleList、NextModule的PrevModuleList.//NOLINT
   若必要, 则将prev的NextCallbackQueue中的所有Callback,//NOLINT
   备份到repeater_info_ptr_ptr所指;//NOLINT
   将其中来自next的移除.//NOLINT
   从prev的NextModuleList中移除next.//NOLINT
   从next的PrevModuleList中移除prev.//NOLINT
  */
int ModuleLinkImpl::UnlinkModules(
  ModuleInterfacePtr prev,
  ModuleInterfacePtr next,
  RepeaterModuleInfoPtr* repeater_info_ptr_ptr) {
  int ret_value = YSOS_ERROR_INVALID_ARGUMENTS;
  ModuleLinkInfoPtr prev_info = NULL;
  ModuleLinkInfoPtr next_info = NULL;
  do {
    //  参数无效//NOLINT
    if (!prev || !next) {
      break;
    }
#if 0 // removed
    //  关闭这两个模块//NOLINT
    if (!repeater_info_ptr_ptr) {
      //  prev->Close();
      //  next->Close();
      Close();
    } else {
      Stop();
    }
#endif
    //  获取prev和next的LinkInfo指针.//NOLINT
    IsModuleAlreadyInLink(prev, prev_info);
    IsModuleAlreadyInLink(next, next_info);

    //  若必要则备份PrevModule的NextCallbackQueue、//NOLINT
    //  PrevModule的NextModuleList、NextModule的PrevModuleList.//NOLINT
    if (repeater_info_ptr_ptr && *repeater_info_ptr_ptr && (*repeater_info_ptr_ptr).get()
        && !ExistLinkInfoBetweenModulesOfEdge(prev, next, *repeater_info_ptr_ptr)) {
      RepeaterModuleInfoPtr repeater_ptr = *repeater_info_ptr_ptr;
      LinkInfoBetweenModules link_info;
      link_info.edge.first = prev;
      link_info.edge.second = next;
      link_info.inherent_edge = true;

      //  备份NextCallbackQueue//NOLINT
      if (!BackupNextCallbackQueueToList(prev, link_info.callback_list)) {
        break;
      }

      //  备份PrevModule的NextModuleList//NOLINT
      for (ModuleLinkInfoIterator it = prev_info->next_module_list_.begin();
           it != prev_info->next_module_list_.end(); ++it) {
        link_info.next_module_list_ptr.push_back((*it));
      }

      //  备份NextModule的PrevModuleList//NOLINT
      for (ModuleLinkInfoIterator it = next_info->prev_module_list_.begin();
           it != next_info->prev_module_list_.end(); ++it) {
        link_info.prev_module_list_ptr.push_back((*it));
      }

      repeater_ptr->backup_link_info.push_back(link_info);
    }

    //  将PrevModule的NextCallbackQueue中来自NextModule的移除//NOLINT
    RemoveNextCallbackFromPrevModule(prev, next);

    //  从PrevModule的NextModuleList中移除NextModule//NOLINT
    RemoveModuleFromNextModuleList(prev_info, next_info);

    //  从NextModule的PrevModuleList中移除PrevModule//NOLINT
    RemoveModuleFromPrevModuleList(prev_info, next_info);
    ret_value = YSOS_ERROR_SUCCESS;
  } while (false);
  return ret_value;
}

int UnlinkModule(ModuleLinkInfoPtr curr) {
  int ret_value = YSOS_ERROR_INVALID_ARGUMENTS;
  do {
    //  参数无效                                                  //NOLINT
    if (!curr) {
      break;
    }

    //  断开所有的Prev
    //  断开所有的Next

    //  关闭这两个模块                                             //NOLINT
    curr->module_ptr_->Close();

    ret_value = YSOS_ERROR_SUCCESS;
  } while (false);
  return ret_value;
}

//  断开与curr链接的所有模块                                        //NOLINT
//  关闭这两个模块                                                 //NOLINT
//  将PrevModule的NextCallbackQueue中来自NextModule的移除          //NOLINT
//  从PrevModule的NextModuleList中移除NextModule                  //NOLINT
//  从NextModule的PrevModuleList中移除PrevModule                  //NOLINT
int ModuleLinkImpl::UnlinkModule(ModuleInterfacePtr curr) {
  int ret_value = YSOS_ERROR_INVALID_ARGUMENTS;
  do {
    //  参数无效                                                  //NOLINT
    if (!curr) {
      break;
    }

    //  断开所有的Prev
    //  断开所有的Next

    //  关闭这两个模块                                             //NOLINT
    curr->Close();

    ret_value = YSOS_ERROR_SUCCESS;
  } while (false);
  return ret_value;
}

//  StopCurrentAndNext
//  Unlinked
//  DoRemove
//  StateOfEachDst:Unlinked
int ModuleLinkImpl::RemoveModule(const std::string &module_name) {
  int return_value = YSOS_ERROR_INVALID_ARGUMENTS;
  AutoLockOper lock_oper(&module_link_lock_);
  do {
    //  Stop the link if necessary
    {
      int32_t state = PROP_CLOSE;
      if (YSOS_ERROR_SUCCESS != GetState(0xffffffff, &state)) {
        break;
      }
      if (state != PROP_CLOSE && state != PROP_STOP) {
        Stop();
      }
    }

    //  Find the ModuleLinkInfo
    ModuleInterfacePtr module_ptr = NULL;
    ModuleLinkInfoPtr module_info_ptr = NULL;
    if (YSOS_ERROR_SUCCESS != FindModuleByName(&module_ptr, module_name)) {
      break;
    }
    if (!IsModuleAlreadyInLink(module_ptr, module_info_ptr)) {
      return_value = YSOS_ERROR_FAILED;
      break;
    }

    //
  } while (false);
  return return_value;
}

int ModuleLinkImpl::FindModuleByName(
  ModuleInterfacePtr *module_ptr,
  const std::string &module_name) {
  int return_value = YSOS_ERROR_INVALID_ARGUMENTS;
  AutoLockOper lock_oper(&module_link_lock_);
  do {
    if (!module_ptr) {
      break;
    }

    return_value = YSOS_ERROR_FAILED;
    for (ModuleLinkInfoIterator it = module_link_info_list_.begin();
         it != module_link_info_list_.end(); ++it) {
      #ifdef _WIN32
        if (0 == stricmp((*it)->module_ptr_->GetName().c_str(), module_name.c_str())) {
          *module_ptr = (*it)->module_ptr_;
          return_value = YSOS_ERROR_SUCCESS;
          break;
        }
      #else
        if (0 == strcasecmp((*it)->module_ptr_->GetName().c_str(), module_name.c_str())) {
          *module_ptr = (*it)->module_ptr_;
          return_value = YSOS_ERROR_SUCCESS;
          break;
        }
      #endif  
    }
  } while (false);

  return return_value;
}

int ModuleLinkImpl::FindModuleByType(ModuleInterfacePtr *module_ptr, int module_type) {
  int return_value = YSOS_ERROR_INVALID_ARGUMENTS;
  AutoLockOper lock_oper(&module_link_lock_);
  do {
    if (!module_ptr) {
      break;
    }

    return_value = YSOS_ERROR_FAILED;
    for (ModuleLinkInfoIterator it = module_link_info_list_.begin();
         it != module_link_info_list_.end(); ++it) {
      int current_module_type = static_cast<int>(PROP_SOURCE);
      if (0 == GetModuleType((*it)->module_ptr_, current_module_type) &&
          current_module_type == module_type) {
        *module_ptr = (*it)->module_ptr_;
        return_value = YSOS_ERROR_SUCCESS;
        break;
      }
    }
    return_value = YSOS_ERROR_SUCCESS;
  } while (false);
  return return_value;
}

void ModuleLinkImpl::UpdatePathForSelfAndDescendent(
  ModuleLinkInfoPtr root,
  std::string& prefix,
  bool add_prefix = true) { ///< true to add the prefix, false to remove the prefix
#if 0
  if (!root) {
    return;
  }
  if (add_prefix == true) {
    root->module_path_ = prefix + root->module_path_;
  } else {
    root->module_path_ = root->module_path_.substr(
                           prefix.length(), root->module_path_.length() - prefix.length());
  }
  for (ModuleLinkInfoIterator it = root->next_module_list_.begin();
       it != root->next_module_list_.end();
       ++it) {
    if (!(*it)) {
      continue;
    }
    std::string prefix_next_level = "";
    if (add_prefix == true) {
      prefix_next_level = prefix + (*it)->module_path_;
    } else {
      prefix_next_level = prefix;
    }
    (*it)->module_path_ = prefix_next_level;
    UpdatePathForSelfAndDescendent((*it), prefix_next_level, add_prefix);
  }
#endif
}

//  PrevModule与NextModule的类型是否匹配
bool ModuleLinkImpl::IsPrevOutAndNextInMatched(ModuleInterfacePtr prev, ModuleInterfacePtr next) {
  bool ret_value = false;
  CallbackDataType prev_out_type;
  CallbackDataType next_in_type;
  std::list<std::string> out_type_list;
  std::list<std::string> in_type_list;

  do {
    if (!prev || !next) {
      break;
    }

    //  RepeaterModule可与任何类别的Module链接
    if (IsRepeaterModule(prev) || IsRepeaterModule(next)) {
      ret_value = true;
      break;
    }

    if (!GetAtomicDataTypeList(prev, out_type_list, false) ||
        !GetAtomicDataTypeList(next, in_type_list, true)) {
      break;
    }

    if ((out_type_list.size() == 1 && out_type_list.front() == "all") ||
        (in_type_list.size() == 1 && in_type_list.front() == "all")) {
      ret_value = true;
    }

    //  判断两个list中是否存在相同项.
    bool existing_matched_type = false;
    for (std::list<std::string>::iterator it_out = out_type_list.begin(); it_out != out_type_list.end(); ++it_out) {
      for (std::list<std::string>::iterator it_in = in_type_list.begin(); it_in != in_type_list.end(); ++it_in) {
        if (*it_out == *it_in) {
          existing_matched_type = true;
          break;
        }
      }
      if (existing_matched_type) {
        break;
      }
    }
    if (existing_matched_type) {
      ret_value = true;
      break;
    }

    // if (((YSOS_ERROR_SUCCESS == prev->GetProperty(
    //       PROP_OUT_DATA_TYPES, &prev_out_type)
    //     && YSOS_ERROR_SUCCESS == next->GetProperty(
    //       PROP_IN_DATA_TYPES, &next_in_type)
    //     && next_in_type == prev_out_type) ||
    //     prev_out_type == "all" ||
    //     next_in_type == "all")) {
    //   ret_value = true;
    // }
  } while (false);
  //  TODO: Buffer的尺寸和数量是否匹配
  return ret_value;
}

bool ModuleLinkImpl::ExistSourceModule(ModuleLinkInfoPtr module_link_head_ptr) {
  bool ret_value = false;
  int module_type;
  void* module_type_ptr = reinterpret_cast<void*>(&module_type);
  if (module_link_head_ptr && module_link_head_ptr->module_ptr_
      && YSOS_ERROR_SUCCESS == module_link_head_ptr->module_ptr_->GetProperty(PROP_MODULE_TYPE, module_type_ptr)
      && module_type == PROP_SOURCE) {
    ret_value = true;
  }
  return ret_value;
}

bool ModuleLinkImpl::ExistSourceModule() {
  bool ret_value = false;
  ModuleLinkInfoPtr module_link_head_ptr = NULL;
  if (module_link_info_list_.size() > 0) {
    module_link_head_ptr = module_link_info_list_.front();
    while (module_link_head_ptr->prev_module_list_.size() > 0) {
      module_link_head_ptr = module_link_head_ptr->prev_module_list_.front();
    }
    ret_value = ExistSourceModule(module_link_head_ptr);
  }
  return ret_value;
}

bool ModuleLinkImpl::CanAddToPrevField(ModuleLinkInfoPtr prev, ModuleLinkInfoPtr next) {
  bool ret_value = false;
  if (prev && next && (CanAddToPrevField(prev) || CanAddToPrevField(next))) {
    ret_value = true;
  }
  return ret_value;
}

bool ModuleLinkImpl::CanAddToPrevField(ModuleLinkInfoPtr module_link_info_ptr) {
  bool ret_value = false;
  if (module_link_info_ptr
      && module_link_info_ptr->module_ptr_
      && (IsRepeaterModule(module_link_info_ptr->module_ptr_)
          || module_link_info_ptr->prev_module_list_.empty())) {
    ret_value = true;
  }
  return ret_value;
}

bool ModuleLinkImpl::IsEachModuleLinked() {
  bool ret_value = false;
  bool exist_more_than_1_null_prev = false;
  do {
    /// empty module_link_info_list_
    if (module_link_info_list_.empty()) {
      break;
    }

    /// only 1 element in module_link_info_list_
    if (module_link_info_list_.size() == 1) {
#if ENABLE_SOURCE_AS_WHOLE_LINK
      ModuleInterfacePtr module_ptr = module_link_info_list_.front()->module_ptr_;
      if (IsSourceModule(module_ptr) || IsRepeaterModule(module_ptr)) {
        ret_value = true;

        GetTheHeadPositionOfLinkInfoListByModule(
          module_link_info_list_.front()->module_ptr_,
          source_position_of_the_link);
        break;
      }
#else
      break;
#endif
    }
    //  NextToDo:
    uint32_t null_prev_count = 0;
    for (ModuleLinkInfoIterator it = module_link_info_list_.begin();
         it != module_link_info_list_.end(); ++it) {
      ModuleInterfacePtr module_ptr = (*it)->module_ptr_;
      if ((*it)->prev_module_list_.empty()) {
        if (!IsRepeaterModule((*it)->module_ptr_)) {
          ++null_prev_count;
          if (null_prev_count > 1) {
            exist_more_than_1_null_prev = true;
            break;
          }
        }
      }
    }
    if (exist_more_than_1_null_prev) {
      break;
    }
    ret_value = true;

    //  SetTheLinkHead
    GetTheHeadPositionOfLinkInfoListByModule(
      module_link_info_list_.front()->module_ptr_,
      source_position_of_the_link);
  } while (false);
  if (!ret_value) {
    SetLinkCompleted(false);
  }
  return ret_value;
}

bool ModuleLinkImpl::IsLinkCompleted() {
  return tree_built_;
}

void ModuleLinkImpl::SetLinkCompleted(bool link_completed) {
  tree_built_ = link_completed;
}

void DoClearForCompleteForNode(std::list<ModuleLinkInfoPtr> root) {
  for (ModuleLinkInfoIterator it = root.begin();
       it != root.end(); ++it) {
    RemoveEachNextCallbackFromModule(*it);
  }
}

void ModuleLinkImpl::DoCompleteForNode(ModuleLinkInfoIterator it) {
  //  (*it)->module_path_ = "";
  //  BufferLength
  YSOS_LOG_DEBUG((*it)->module_path_ << (*it)->module_priority_ << (*it)->module_ptr_->GetName());
  ModifySelfAllocatorAndPriority(*it);
  GetMaxPrefixBufferLengthForModuleSharedBuffer(*it);
  //  SourceOrSelfAllocate: AllocatorProperty

#if 1
  //  next.PrevCallbackQueue set to prev.NextCallbackQueue
  for (ModuleLinkInfoIterator next_module_it = (*it)->next_module_list_.begin();
       next_module_it != (*it)->next_module_list_.end();
       ++next_module_it) {
    //  Add PrevCb Of Next Module to NextCbQ Of Prev Module

    //  RemoveEachNextCallbackFromModule(*it);
    {
      std::list<std::string> prompt_strings;
      AddNextCallbackToPrevModule((*it)->module_ptr_, (*next_module_it)->module_ptr_, &prompt_strings);
      for (std::list<std::string>::iterator string_list_it = prompt_strings.begin();
           string_list_it != prompt_strings.end(); ++string_list_it) {
        YSOS_LOG_DEBUG(*string_list_it);
      }
    }
  }
#endif
}

/// 将各无NextModule的Module视为一个逻辑SubLink//NOLINT
/// DestinationModule or RepeaterModule without next module
/// 若Module后接Repeater且其中Module所在的边集的second均为空, 则Module视为一个逻辑SubLink.
bool ModuleLinkImpl::RebuildSubLinkInfoInModuleLink() {
  bool ret_value = false;

  sub_link_info_list_.clear();

  /// 调用时机有问题, 2017-02-21, XuLanyue
  // link_depth_ = 0;

  ///   如果这是一个无NextModule的Module,那么应该添加一个子链信息节点//NOLINT
  for (ModuleLinkInfoIterator it = module_link_info_list_.begin();
       it != module_link_info_list_.end(); ++it) {
    if ((*it)->next_module_list_.size() == 0 && !IsRepeaterModule((*it)->module_ptr_)) {
      SubLinkInfoInModuleLink sub_link_info;
      sub_link_info.link_info_position_ = it;
      sub_link_info.module_ptr_ = (*it)->module_ptr_;
      sub_link_info.logic_name_ = (*it)->module_ptr_->GetName();
      sub_link_info_list_.push_back(sub_link_info);

      /// 调用时机有问题, 2017-02-21, XuLanyue
      // if (link_depth_ < (*it)->level_) {
      //   link_depth_ = (*it)->level_;
      // }
      ret_value = true;
    } else if (IsRepeaterModule((*it)->module_ptr_)) {
      std::string repeater_name = (*it)->module_ptr_->GetName();
      std::list<std::string> name_set_of_module_without_next;
      GetSetOfModuleWithoutNextInRepeater(
        repeater_name, name_set_of_module_without_next);
      if (name_set_of_module_without_next.size() == 0) {
        continue;
      }
      for (std::list<std::string>::iterator it_name = name_set_of_module_without_next.begin();
           it_name != name_set_of_module_without_next.end(); ++it_name) {
        SubLinkInfoInModuleLink sub_link_info;

        for (ModuleLinkInfoIterator it_inner = module_link_info_list_.begin();
             it_inner != module_link_info_list_.end(); ++it_inner) {
          if ((*it_inner)->module_ptr_->GetName() != (*it_name)) {
            continue;
          }
          sub_link_info.link_info_position_ = it_inner;
          sub_link_info.module_ptr_ = (*it_inner)->module_ptr_;
          sub_link_info.logic_name_ = (*it_inner)->module_ptr_->GetName();
          sub_link_info_list_.push_back(sub_link_info);

          /// 调用时机有问题, 2017-02-21, XuLanyue
          // if (link_depth_ < (*it_inner)->level_) {
          //   link_depth_ = (*it_inner)->level_;
          // }
        }
      }
      ret_value = true;
    }
  }
  return ret_value;
}


/// Dump info of callback queue
bool ModuleLinkImpl::DumpCallbackQueue() {
  bool ret_value = false;
  YSOS_LOG_DEBUG("Callback(s) in the link:");
  for (ModuleLinkInfoIterator it = module_link_info_list_.begin(); it != module_link_info_list_.end(); ++it) {
    BaseModuleImpl* curr_module = reinterpret_cast<BaseModuleImpl*>((*it)->module_ptr_.get());
    curr_module->DumpPreCallback();
    curr_module->DumpNextCallback();
  }
  return ret_value;
}

/// Dump info of the whole link
void ModuleLinkImpl::DumpAll() {
  YSOS_LOG_DEBUG("Dump ModuleLink info:");
  for (ModuleLinkInfoIterator it = module_link_info_list_.begin();
       it != module_link_info_list_.end(); ++it) {
    BaseModuleImpl* curr_module = reinterpret_cast<BaseModuleImpl*>((*it)->module_ptr_.get());
    YSOS_LOG_DEBUG("------------------------------------------------------------");
    YSOS_LOG_DEBUG("Callback(s) of Module " << (*it)->module_ptr_->GetName() << ":");
    curr_module->DumpPreCallback();
    curr_module->DumpNextCallback();
    YSOS_LOG_DEBUG("Prev_module_list of Module " << (*it)->module_ptr_->GetName() << ":");
    if ((*it)->prev_module_list_.size() > 0) {
      for (ModuleLinkInfoIterator prev_it = (*it)->prev_module_list_.begin();
           prev_it != (*it)->prev_module_list_.end(); ++prev_it) {
        YSOS_LOG_DEBUG("Module " << (*prev_it)->module_ptr_->GetName() << ".");
      }
    } else {
      YSOS_LOG_DEBUG("(None)");
    }
    YSOS_LOG_DEBUG("Next_module_list of Module " << (*it)->module_ptr_->GetName() << ":");
    if ((*it)->next_module_list_.size() > 0) {
      for (ModuleLinkInfoIterator next_it = (*it)->next_module_list_.begin();
           next_it != (*it)->next_module_list_.end(); ++next_it) {
        YSOS_LOG_DEBUG("Module " << (*next_it)->module_ptr_->GetName() << ".");
      }
    } else {
      YSOS_LOG_DEBUG("(None)");
    }
  }
  YSOS_LOG_DEBUG("------------------------------------------------------------");
  YSOS_LOG_DEBUG("Dump ModuleLink info end.");
}

/// Rebuild the tree of the link:
///   Clear each next callback queue.
///   Save the root and each destination(RepeaterModule as leaf, or DestinationModule)
///   Save info of each RepeaterModule
bool ModuleLinkImpl::RebuildModuleLinkInfoTree() {
  bool ret_value = false;
  //  ClearTree
  DoClearForCompleteForNode(module_link_info_list_);
  RebuildModuleLinkInfoTree_(source_position_of_the_link);
  ret_value = RebuildSubLinkInfoInModuleLink();
  if (ret_value) {
    SetLinkCompleted(true);
  }
  DumpAll();
  return ret_value;
}

int ModuleLinkImpl::GetLinkDepth(uint32_t* depth_ptr) {
  int return_value = YSOS_ERROR_INVALID_ARGUMENTS;
  do {
    if (!depth_ptr) {
      break;
    }
    *depth_ptr = link_depth_;
    return_value = YSOS_ERROR_SUCCESS;
  } while (false);
  return return_value;
}

void ModuleLinkImpl::RebuildModuleLinkInfoTree_(ModuleLinkInfoIterator root) {
  DoCompleteForNode(root);
  for (ModuleLinkInfoIterator it = (*root)->next_module_list_.begin();
       it != (*root)->next_module_list_.end(); ++it) {
    if (!(*it)) {
      continue;
    }
    RebuildModuleLinkInfoTree_(it);
  }
}

bool ModuleLinkImpl::GetTheHeadOfLinkInfoListByModule(
  ModuleInterfacePtr module_ptr,
  ModuleLinkInfoPtr& link_info_ptr) {
  bool ret_value = false;
  for (ModuleLinkInfoIterator it = module_link_info_list_.begin();
       it != module_link_info_list_.end(); ++it) {
    if ((*it)->module_ptr_.get() == module_ptr.get()) {
      ret_value = true;
      link_info_ptr = (*it);
      break;
    }
  }
  if (ret_value) {
    do {
      ModuleLinkInfoPtr parent_link_info_ptr = NULL;
      if (!link_info_ptr->prev_module_list_.empty()) {
        parent_link_info_ptr = link_info_ptr->prev_module_list_.front();
      }
      if (!parent_link_info_ptr) {
        break;
      }
      link_info_ptr = parent_link_info_ptr;
    } while (true);
  }

  if (ret_value) {
    for (ModuleLinkInfoIterator it = module_link_info_list_.begin();
         it != module_link_info_list_.end(); ++it) {
      if ((*it)->module_ptr_.get() == link_info_ptr->module_ptr_.get()) {
        ret_value = true;
        link_info_ptr = (*it);
        break;
      }
    }
  }
  return ret_value;
}

bool ModuleLinkImpl::GetTheHeadPositionOfLinkInfoListByModule(
  ModuleInterfacePtr module_ptr,
  ModuleLinkInfoIterator &link_info_head_position) {
  bool ret_value = false;
  ModuleLinkInfoPtr link_info_ptr;
  for (ModuleLinkInfoIterator it = module_link_info_list_.begin();
       it != module_link_info_list_.end();
       ++it) {
    if ((*it)->module_ptr_.get() == module_ptr.get()) {
      ret_value = true;
      link_info_ptr = (*it);
      break;
    }
  }
  if (ret_value) {
    do {
      ModuleLinkInfoPtr parent_link_info_ptr = NULL;
      if (!link_info_ptr->prev_module_list_.empty()) {
        parent_link_info_ptr = link_info_ptr->prev_module_list_.front();
      }
      if (!parent_link_info_ptr) {
        break;
      }
      link_info_ptr = parent_link_info_ptr;
    } while (true);

    if (ret_value) {
      for (ModuleLinkInfoIterator it = module_link_info_list_.begin();
           it != module_link_info_list_.end();
           ++it) {
        if ((*it)->module_ptr_.get() == link_info_ptr->module_ptr_.get()) {
          ret_value = true;
          link_info_head_position = it;
          break;
        }
      }
    }
  }
  return ret_value;
}

void ModuleLinkImpl::IsModuleInModuleLinkInfoList(
  ModuleInterfacePtr module_ptr,
  ModuleLinkInfoPtr head_link_info_ptr,
  bool& ret_value) {
  ModuleLinkInfoPtr curr_link_info_ptr = NULL;
  if (head_link_info_ptr->module_ptr_.get() == module_ptr.get()) {
    ret_value = true;
    return;
  }
  for (ModuleLinkInfoIterator it = curr_link_info_ptr->next_module_list_.begin();
       it != curr_link_info_ptr->next_module_list_.end(); ++it) {
    if (!(*it)) {
      continue;
    }
    IsModuleInModuleLinkInfoList(module_ptr, (*it), ret_value);
    if (ret_value == true) {
      break;
    }
  }
}

bool ModuleLinkImpl::ExistCycleIfModulesLinked(
  ModuleLinkInfoPtr prev_link_head_ptr,
  ModuleLinkInfoPtr next_link_head_ptr) {
  bool ret_value = false;
  bool existing = false;
  do {
    //  获取模块所在链的链信息列表的头
    if (prev_link_head_ptr.get() || next_link_head_ptr.get()) {
      break;
    }

    IsModuleInModuleLinkInfoList(
      prev_link_head_ptr->module_ptr_,
      next_link_head_ptr,
      existing);
    if (existing) {
      ret_value = true;
      break;
    }

    for (ModuleLinkInfoIterator it
         = prev_link_head_ptr->next_module_list_.begin();
         it != prev_link_head_ptr->next_module_list_.end();
         ++it) {
      if (!(*it)) {
        continue;
      }
      IsModuleInModuleLinkInfoList(
        (*it)->module_ptr_,
        next_link_head_ptr, ret_value);
      if (ret_value == true) {
        break;
      }
    }
  } while (false);
  return ret_value;
}

bool ModuleLinkImpl::ExistCycleIfModulesLinked(
  ModuleInterfacePtr prev_module_ptr,
  ModuleInterfacePtr next_module_ptr) {
  bool ret_value = false;
  bool existing = false;
  ModuleLinkInfoPtr prev_link_info_ptr = NULL;
  ModuleLinkInfoPtr next_link_info_ptr = NULL;
  do {
    //  获取模块所在链的链信息列表的头
    if (!GetTheHeadOfLinkInfoListByModule(
          prev_module_ptr, prev_link_info_ptr)) {
      break;
    }
    if (!GetTheHeadOfLinkInfoListByModule(
          next_module_ptr, next_link_info_ptr)) {
      break;
    }

    IsModuleInModuleLinkInfoList(
      prev_link_info_ptr->module_ptr_,
      next_link_info_ptr,
      existing);
    if (existing) {
      ret_value = true;
      break;
    }

    for (ModuleLinkInfoIterator it = prev_link_info_ptr->next_module_list_.begin();
         it != prev_link_info_ptr->next_module_list_.end();
         ++it) {
      if (!(*it)) {
        continue;
      }
      IsModuleInModuleLinkInfoList(
        (*it)->module_ptr_,
        next_link_info_ptr, ret_value);
      if (ret_value == true) {
        break;
      }
    }
  } while (false);
  return ret_value;
}

bool ModuleLinkImpl::IsModuleAlreadyInLink(const ModuleInterfacePtr &module_ptr, ModuleLinkInfoPtr& link_info_ptr) {
  bool ret_value = false;
  for (ModuleLinkInfoIterator it = module_link_info_list_.begin(); it != module_link_info_list_.end(); ++it) {
    if ((*it)->module_ptr_.get() == module_ptr.get()) {
      YSOS_LOG_ERROR(module_ptr->GetName() << " is already exist in " << GetName());
      ret_value = true;
      link_info_ptr = (*it);
      break;
    }
  }

  return ret_value;
}

bool ModuleLinkImpl::IsModuleAlreadyInLink(ModuleInterface *module_ptr, ModuleLinkInfoPtr& link_info_ptr) {
  bool ret_value = false;
  for (ModuleLinkInfoIterator it = module_link_info_list_.begin();
       it != module_link_info_list_.end(); ++it) {
    if ((*it)->module_ptr_.get() == module_ptr) {
      ret_value = true;
      link_info_ptr = (*it);
      break;
    }
  }
  return ret_value;
}

void GetStateOfEachNextModule(
  const ModuleLinkInfoPtr &module_link_info_ptr,
  const ModuleInterfacePtr &specified_next_module_ptr,
  int &exist_run,
  int &exist_pause,
  int &exist_stop,
  int &state_of_specified_next_module) {
  bool specified_next_module_found = false;
  int state_of_module = PROP_STOP;
  for (ModuleLinkInfoIterator it = module_link_info_ptr->next_module_list_.begin();
       it != module_link_info_ptr->next_module_list_.end();
       ++it) {
    if (!specified_next_module_found
        && (*it)->module_ptr_.get() == specified_next_module_ptr.get()) {
      specified_next_module_ptr->GetState(0, &state_of_specified_next_module);
      specified_next_module_found = true;
    }
    (*it)->module_ptr_->GetState(0, &state_of_module);
    if (state_of_module == PROP_RUN) {
      ++exist_run;
    } else if (state_of_module == PROP_PAUSE) {
      ++exist_pause;
    } else if (state_of_module == PROP_STOP) {
      ++exist_stop;
    }
  }
}

void GetStateOfEachNextModule(
  const ModuleLinkInfoPtr &module_link_info_ptr,
  int &exist_run,
  int &exist_pause,
  int &exist_stop) {
  int state_of_module = PROP_STOP;
  for (ModuleLinkInfoIterator it = module_link_info_ptr->next_module_list_.begin();
       it != module_link_info_ptr->next_module_list_.end();
       ++it) {
    (*it)->module_ptr_->GetState(0, &state_of_module);
    if (state_of_module == PROP_RUN) {
      ++exist_run;
    } else if (state_of_module == PROP_PAUSE) {
      ++exist_pause;
    } else if (state_of_module == PROP_STOP) {
      ++exist_stop;
    }
  }
}

ModuleLinkInfo::ModuleLinkInfo(const ModuleInterfacePtr &module_ptr) {
  module_ptr_ = module_ptr;
  module_path_ = "";
  module_priority_ = GetModulePriority(module_ptr);
  is_strategy_ = IsRepeaterModule(module_ptr);
  GetModuleProperty(module_ptr, module_properties);
  new_thread_ = false;
  level_ = 0;
  inherent_ = false;
}

ModuleLinkInfo::~ModuleLinkInfo() {
  prev_module_list_.clear();
  next_module_list_.clear();
}

/// helper functions
}
