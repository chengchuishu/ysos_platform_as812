/**
 *@file templatemanager.h
 *@brief Definition of template manager
 *@version 0.1
 *@author Steven.Shi
 *@date Created on: 2016-05-13 08:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef TEMPLATE_MANAGER_H  //NOLINT
#define TEMPLATE_MANAGER_H  //NOLINT

#define ADD_NAMESPACE(NS,FUNC) NS::FUNC   //need add 'typename' for linux


#include "templatefactory.h"
#include "../os_hal_package/lock.h"
#include "../ysoslog/ysoslog.h"
#include <list>
#include <string>
#include <iostream>

namespace ysos {

#define DECLARE_CREATEMANAGERINSTANCE   DECLARE_CREATESINGLEINSTANCE
#define IMPLEMENT_CREATEMANAGERINSTANCE IMPLEMENT_CREATESINGLEINSTANCE

  /**
   *@brief 定义管理器实现模板，不同的管理器将内部实现一个该模板实例，
     详细实例见ProtocolInterfaceManager的使用
  */
template <typename ObjectInterfacePtr>
class TemplateManager {
 public:
   /**
   *@brief 模板管理器构造函数  // NOLINT
   *@param factory_name[Input]： 类名称  // NOLINT
   *@return： 无  // NOLINT
   */
 explicit TemplateManager(const std::string &manager_name) 
  	: manager_name_(manager_name) {
  }
  
  virtual ~TemplateManager() {
    Clear();
  }
  void DumpObjectInterface();

  /**
   *@brief 利用名字对或者XML字符串将逻辑名称和类名称添加到管理器中  // NOLINT
     此处暂时没有实现  // NOLINT
   *@param names_table[Input]： 名字对  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，否则返回其他错误  // NOLINT
   */
  int  LoadNameTable(const std::string &names_table);

  /**
   *@brief 添加实例逻辑名称到管理器  // NOLINT
   *@param logic_name[Input]： 实例的逻辑名称  // NOLINT
   *@param class_name[Input]： 类名称  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，否则返回其他错误  // NOLINT
   */
  int  AddNameItem(const std::string &logic_name, const std::string &class_name) {
    std::pair<M_Logic2ClassIter, bool> ret;
    AutoLockOper lock(&object_interfaces_mutex_);
    ret = logic2class_names_.insert(std::pair<std::string,std::string>(logic_name, class_name));
    if (ret.second == false) {
        return YSOS_ERROR_FAILED;
    }
    return YSOS_ERROR_SUCCESS;
  }

  /**
   *@brief 获得逻辑名字的个数  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回名字个数，否则返回0  // NOLINT
   */
  int GetNameItemNumber() {
    AutoLockOper lock(&object_interfaces_mutex_);
    return logic2class_names_.size();
  }

  /**
   *@brief 通过实例的逻辑名字获得所属的类名称  // NOLINT
   *@param logic_name[Input]： 实例逻辑名称  // NOLINT
   *@return： 成功返回对象类名称，否则返回“”  // NOLINT
   */
  std::string GetThisClassName(const std::string &logic_name) {
    AutoLockOper lock(&object_interfaces_mutex_);
    M_Logic2ClassIter iter = logic2class_names_.find(logic_name);
    if( logic2class_names_.end() != iter )
        return iter->second;
    return std::string("");
  }

  /**
   *@brief 添加接口实例指针到管理器中  // NOLINT
   *@param object_interface_ptr[Input]： 接口实例指针  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，否则返回其他错误  // NOLINT
   */
  int  AddObjectInterface(ObjectInterfacePtr object_interface_ptr);

  /**
   *@brief 从管理器中删除接口实例  // NOLINT
   *@param object_name[Input]：实例对象的名称  // NOLINT
   *@return： 成功返回对象实例，否则返回NULL  // NOLINT
   */
  ObjectInterfacePtr RemoveObjectInterface(const std::string &object_name);

  /**
   *@brief 从管理器中查找接口实例  // NOLINT
   *@param object_name[Input]：实例对象的名称  // NOLINT
   *@return： 成功返回对象实例，否则返回NULL  // NOLINT
   */
  ObjectInterfacePtr FindObjectInterface(const std::string &object_name);

  /**
   *@brief 添加实例逻辑名称到管理器  // NOLINT
   *@param logic_name[Input]： 实例的逻辑名称  // NOLINT
   *@param class_name[Input]： 类名称  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，否则返回其他错误  // NOLINT
   */
  int GetObjectInterfaceNumber() {
    AutoLockOper autoLock(&object_interfaces_mutex_);
    return object_interfaces_.size();
  }

  /**
   *@brief 管理器启动  // NOLINT
   *@param ：无  // NOLINT
   *@return： 无  // NOLINT
   */
  void Start() {
    AutoLockOper autoLock(&shutting_down_mutex_);
    if (shutting_down_) {
      shutting_down_ = false;
    }
  };

  /**
   *@brief 管理器停止  // NOLINT
   *@param ：无  // NOLINT
   *@return： 无  // NOLINT
   */
  void Shutdown() {
    AutoLockOper autoLock(&shutting_down_mutex_);
    if (shutting_down_) {
      shutting_down_ = true;
    }
  }

  /**
   *@brief 将管理器中所有实例指针清除  // NOLINT
   *@param ：无  // NOLINT
   *@return： 无  // NOLINT
   */
  void Clear() {
    Shutdown();
    {
      AutoLockOper autoLock(&object_interfaces_mutex_);
      //  for (M_ObjectInterfaces::iterator t = object_interfaces_.begin();
      //    t != object_interfaces_.end(); ++t) {
      //    ObjectInterfacePtr pRetObject = (*t).second;
      //    delete pRetObject;
      //  }
      object_interfaces_.clear();
    }
  }

 protected:
  typedef std::map<std::string, std::string>  M_Logic2Class; 
  typedef std::map<std::string, std::string>::iterator  M_Logic2ClassIter;
  typedef std::map<std::string, ObjectInterfacePtr> M_ObjectInterfaces;
  //  typedef std::map<std::string, ObjectInterfacePtr>::iterator  M_ObjectInterfacesIter;
  typedef std::list<ObjectInterfacePtr> L_ObjectInterfaces;
  std::string manager_name_;    ///< 管理器名称
  M_Logic2Class logic2class_names_;    ///< 逻辑名称和类名称映射表，用于创建和查找
  M_ObjectInterfaces object_interfaces_;    ///< 对象实例表
  ysos::MutexLock object_interfaces_mutex_;   ///< 用于保护对象实例表
  volatile bool shutting_down_;             ///< 停止标志
  ysos::MutexLock shutting_down_mutex_;     ///< 用于保护停止标志
 private:
  //DISALLOW_COPY_AND_ASSIGN(TemplateManager<ObjectInterfacePtr>);
 public:
  void DumpLogic2ClassName() {
    AutoLockOper autoLock(&object_interfaces_mutex_);
     for (M_Logic2Class::iterator t = logic2class_names_.begin();
      t != logic2class_names_.end(); ++t) {
      std::cout << "\r\n" << (*t).first << ":" << (*t).second;
    }
    std::cout << "\r\n";
  }  
};

template <typename ObjectInterfacePtr>
void TemplateManager<ObjectInterfacePtr>::DumpObjectInterface() {
  AutoLockOper autoLock(&object_interfaces_mutex_);
  std::cout << "Count = " << object_interfaces_.size() << std::endl;
  //for (M_ObjectInterfaces::iterator t = object_interfaces_.begin();  // new update for linux
  for (auto t = object_interfaces_.begin();
    t != object_interfaces_.end(); ++t) {
  //  std::cout << (*t).first << " | " << ((BaseInterface*)(*t).second)->GetName() << std::endl;
  }
}

template <typename ObjectInterfacePtr>
int TemplateManager<ObjectInterfacePtr>::LoadNameTable(const std::string &names_table) {
  return 0;
}

template <typename ObjectInterfacePtr>
int TemplateManager<ObjectInterfacePtr>::AddObjectInterface(
  ObjectInterfacePtr object_interface_ptr) {
  AutoLockOper autoLock(&object_interfaces_mutex_);
  object_interfaces_.insert(
    std::pair<std::string,ObjectInterfacePtr>(
    ((BaseInterface*)(object_interface_ptr.get()))->GetName(),object_interface_ptr));
  return (object_interfaces_.size() - 1);
}

template <typename ObjectInterfacePtr>
ObjectInterfacePtr TemplateManager<ObjectInterfacePtr>::RemoveObjectInterface(
  const std::string &object_name) {
  AutoLockOper autoLock(&object_interfaces_mutex_);
  // M_ObjectInterfaces::iterator t = object_interfaces_.find(object_name);  // new update for linux
  auto t = object_interfaces_.find(object_name);
  if(t != object_interfaces_.end()) {
    ObjectInterfacePtr pRetObject = (*t).second;
    object_interfaces_.erase(t);
    return pRetObject;
  }
  return ObjectInterfacePtr();
}

template <typename ObjectInterfacePtr>
ObjectInterfacePtr TemplateManager<ObjectInterfacePtr>::FindObjectInterface(
  const std::string &object_name) {
  AutoLockOper autoLock(&object_interfaces_mutex_);
  // M_ObjectInterfaces::iterator t = object_interfaces_.find(object_name);  // new update for linux
  auto t = object_interfaces_.find(object_name);
  if(t != object_interfaces_.end()) {
    return (*t).second;
  }
  return ObjectInterfacePtr();
}

template<class T>
class DeletePtrFunction {
public:
    void operator()(T* object) {
        if(NULL != object) {
            std::string instance_name = object->GetName();
            std::cout << instance_name << " destruct\n";
            delete object;
            object = NULL;
        }
    }
};

/// 统一定义接口管理器的类定义，用于管理从一个基接口继承的各个接口实例,
/// 这些实例将被管理器创建并管理，可进行查找、添加、删除操作   
/// need delete InterfaceName##Manager::InterfaceName##Manager
#define DECLARE_INTERFACEMANAGER(InterfaceName)  \
  class YSOS_EXPORT InterfaceName##Manager  \
  {  \
      DISALLOW_COPY_AND_ASSIGN(InterfaceName##Manager);  \
  protected:  \
      InterfaceName##Manager(const std::string &strClassName = "InterfaceName##Manager" ) {  \
        manager_ptr_ = new ysos::TemplateManager<InterfaceName##Ptr>(strClassName);  \
        factory_ptr_ = ysos::Create##InterfaceName##FactoryInstance(); \
      }  \
  public:  \
      DECLARE_CREATEINSTANCE(InterfaceName##Manager);  \
      virtual ~InterfaceName##Manager() {  \
        if (NULL != manager_ptr_) {  \
            delete manager_ptr_;  \
            manager_ptr_ = NULL;  \
        }  \
        if (NULL != factory_ptr_) {  \
        delete factory_ptr_;  \
        factory_ptr_ = NULL;  \
        }  \
      }  \
      int  LoadNameTable(const std::string &names_table) {  \
        if (NULL != manager_ptr_) return manager_ptr_->LoadNameTable(names_table);  return 0;\
      }  \
      int  AddNameItem(const std::string &logic_name, const std::string &class_name) {  \
        if (NULL != manager_ptr_) return manager_ptr_->AddNameItem(logic_name, class_name);  \
        return YSOS_ERROR_INVALID_ARGUMENTS; \
      }  \
      void Dump() {manager_ptr_->DumpLogic2ClassName(); manager_ptr_->DumpObjectInterface();}  \
      \
      InterfaceName##Ptr FindInterface(const std::string &object_name, bool is_create = true) {  \
        ysos::InterfaceName##Ptr InterfaceName##_ptr = manager_ptr_->FindObjectInterface(object_name);  \
        if( NULL == InterfaceName##_ptr && is_create ) {  \
          if( NULL == factory_ptr_ ){  \
            return 0;  \
          }else{  \
            AutoLockOper autoLock(&create_object_interface_mutex_);  \
            InterfaceName##_ptr = manager_ptr_->FindObjectInterface(object_name);  \
            if( NULL != InterfaceName##_ptr ) {  \
              return InterfaceName##_ptr;  \
            }  \
            std::string class_name = manager_ptr_->GetThisClassName(object_name);  \
            std::cout << "FindInterface [class_name]" << class_name << std::endl; \
            YSOS_LOG_DEBUG_DEFAULT("FindInterface [class_name] :" << class_name);  \
            ysos::DeletePtrFunction</*ysos::##InterfaceName*/ADD_NAMESPACE(ysos,InterfaceName)> delete_function; \
            InterfaceName##_ptr = boost::shared_ptr</*ysos::##InterfaceName*/ADD_NAMESPACE(ysos,InterfaceName)>(factory_ptr_->CreateObjectInstance(class_name, object_name), delete_function);  \
            if (NULL != InterfaceName##_ptr) { \
                InterfaceName *ptr = InterfaceName##_ptr.get();\
                std::cout << "FindInterface " << ptr->GetName() << std::endl; \
                YSOS_LOG_DEBUG_DEFAULT("FindInterface " << ptr->GetName());  \
                int ret = ptr->Initialize(0); \
                 if(0 != ret) { YSOS_LOG_DEBUG_DEFAULT (ptr->GetName() << "Initialize failed:" << ret); std::cout<< "Initialize failed: " << ret << std::endl; return 0; } \
                  manager_ptr_->AddObjectInterface(InterfaceName##_ptr);  \
            }\
          }  \
        }  \
        return InterfaceName##_ptr;  \
      }  \
  private:  \
      ysos::TemplateManager<InterfaceName##Ptr>  *manager_ptr_;  \
      ysos::InterfaceName##Factory *factory_ptr_;  \
      ysos::MutexLock create_object_interface_mutex_;  \
  };  \
  DECLARE_CREATEMANAGERINSTANCE(InterfaceName##Manager);

// 定义逻辑实例注册，用于初始化时，将类实例的逻辑名字注册到管理器中
#define REGISTER_LOGIC_CLASS_NAME(MANAGER_OBJECT, OBJECT_NAME, CLASS_NAME)  \
    MANAGER_OBJECT->AddNameItem(OBJECT_NAME, CLASS_NAME);  

}  // namespace ysos

#endif // TEMPLATE_MANAGER_H  //NOLINT
