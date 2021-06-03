/**
 *@file templatefactory.h
 *@brief Definition of template factory
 *@version 0.1
 *@author Steven.Shi
 *@date Created on: 2016-05-13 08:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef TEMPLATE_FACTORY_H                      //NOLINT
#define TEMPLATE_FACTORY_H                      //NOLINT

#define ADD_NAMESPACE(NS,FUNC) NS::FUNC  //need add 'typename' for linux

#include "../../../public/include/sys_interface_package/common.h"
#include "../../../public/include/os_hal_package/lock.h"             //NOLINT
//#include "../../../public/include/core_help_package/utility.h"
#include <map>
#include <list>
#include <string>
#include <iostream>

namespace ysos {

/// 宏定义，用于定义单实例创建函数
#define DECLARE_CREATESINGLEINSTANCE(SINGLE_OBJECT)  \
  YSOS_EXPORT SINGLE_OBJECT* Create##SINGLE_OBJECT##Instance(const std::string &single_name = #SINGLE_OBJECT);  \
  YSOS_EXPORT void Destroy##SINGLE_OBJECT##Instance();  \
  YSOS_EXPORT SINGLE_OBJECT* Get##SINGLE_OBJECT();  \
  YSOS_EXPORT void Release##SINGLE_OBJECT();  

/// 宏定义，用于定义单实例创建的实现部分
#define IMPLEMENT_CREATESINGLEINSTANCE(SINGLE_OBJECT)  \
  static SINGLE_OBJECT *g_##SINGLE_OBJECT##_instance = NULL;  \
  static ysos::MutexLock g_##SINGLE_OBJECT##_mutex;  \
  SINGLE_OBJECT * Create##SINGLE_OBJECT##Instance(const std::string& single_name){ \
    if (NULL == g_##SINGLE_OBJECT##_instance) {  \
       g_##SINGLE_OBJECT##_mutex.Lock();  \
       if (NULL == g_##SINGLE_OBJECT##_instance) {  \
         g_##SINGLE_OBJECT##_instance = SINGLE_OBJECT::CreateInstance(single_name);  \
       }  \
       g_##SINGLE_OBJECT##_mutex.Unlock();  \
    } \
    return g_##SINGLE_OBJECT##_instance; \
  }  \
  void Destroy##SINGLE_OBJECT##Instance() {  \
    if (NULL != g_##SINGLE_OBJECT##_instance) {  \
      g_##SINGLE_OBJECT##_mutex.Lock();  \
      if (NULL != g_##SINGLE_OBJECT##_instance) {  \
          delete g_##SINGLE_OBJECT##_instance;  \
          g_##SINGLE_OBJECT##_instance = NULL;  \
      }  \
      g_##SINGLE_OBJECT##_mutex.Unlock();  \
    } \
  }  \
  SINGLE_OBJECT* Get##SINGLE_OBJECT(){ \
    return Create##SINGLE_OBJECT##Instance(#SINGLE_OBJECT);  \
  }  \
  void Release##SINGLE_OBJECT() {  \
    Destroy##SINGLE_OBJECT##Instance();  \
  } 


#define DECLARE_CREATEFACTORYINSTANCE  DECLARE_CREATESINGLEINSTANCE
#define IMPLEMENT_CREATEFACTORYINSTANCE  IMPLEMENT_CREATESINGLEINSTANCE

/// 定义类全局静态创建函数指针类型
#define DECLARE_CLASSCREATEFUNCTION(CLASS_OBJECT)  \
    typedef CLASS_OBJECT* (*CreateFunctionPtr)(const std::string &, INT32 *);


  /**
   *@brief 定义工厂模板，不同父基类将创建不同的工厂，
     详细实例见BaseInterfaceFactory的使用
  */
template<typename CLASS_OBJECT>
class TemplateFactory {
 public:
  DECLARE_CLASSCREATEFUNCTION(CLASS_OBJECT);

  /**
   *@brief 模板工厂构造函数  // NOLINT
   *@param factory_name[Input]： 类名称  // NOLINT
   *@return： 无  // NOLINT
   */
  explicit TemplateFactory(const std::string &factory_name) 
    : factory_name_(factory_name) {
  }

  virtual ~TemplateFactory() {
    class_map_.clear();
  }
  /**
   *@brief 注册类名称到工厂  // NOLINT
   *@param class_name[Input]： 类名称  // NOLINT
   *@param create_object_function[Input]：类创建函数  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，否则返回其他错误  // NOLINT
   */
  int RegisterClassNameToFactory(const std::string &class_name, CreateFunctionPtr create_object_function) {
    //std::pair<std::map<std::string,CreateFunctionPtr>::iterator, bool> ret;  // new update for linux
    if(NULL == create_object_function) {
      std::cout << "RegisterClassNameToFactory failed: " << class_name << std::endl;
      return YSOS_ERROR_FAILED;
    }

    if(class_map_.end() != class_map_.find(class_name)) {
      std::cout << "RegisterClassNameToFactory failed: " << class_name << " already exist." << std::endl;
      return YSOS_ERROR_HAS_EXISTED;
    }

    auto ret = class_map_.insert(std::pair<std::string,CreateFunctionPtr>(class_name,create_object_function));  // new update for linux

    return YSOS_ERROR_SUCCESS;
  }  

  /**
   *@brief 工厂创建类实例的方法  // NOLINT
   *@param class_name[Input]： 类名称  // NOLINT
   *@param object_name[Input]：实例对象的名称，用于创建时赋值  // NOLINT
   *@return： 成功返回对象实例，否则返回NULL  // NOLINT
   */
  CLASS_OBJECT* CreateObjectInstance(const std::string &class_name, const std::string &object_name);  

  /**
   *@brief 获得工厂名字方法  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回工厂名称，否则返回“”  // NOLINT
   */
  std::string GetFactoryName() const { return factory_name_; };

  /**
   *@brief 获得工厂中能够创建的类个数  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回工厂中拥有的类个数，否则返回0  // NOLINT
   */
  int GetClassNumber() const { return class_map_.size(); }
 protected:
  std::string factory_name_;
  //typedef std::map<std::string,CreateFunctionPtr>::iterator  M_ClassMapIter;
  typedef std::map<std::string,CreateFunctionPtr> M_ClassMap;
  M_ClassMap class_map_;
 private:
  //DISALLOW_COPY_AND_ASSIGN(TemplateFactory<ObjectInterfacePtr>);

};

template<typename CLASS_OBJECT>
CLASS_OBJECT* TemplateFactory<CLASS_OBJECT>::CreateObjectInstance(const std::string &class_name, const std::string &object_name)  
{  
  CreateFunctionPtr createFunc = NULL;  
  //std::map<std::string,CreateFunctionPtr>::iterator it = class_map_.find(class_name);  // new update for linux
  auto it = class_map_.find(class_name);  // new add for linux
  if (it != class_map_.end()) {  
      createFunc=(*it).second;  
  }  
  if (NULL != createFunc) {
    CLASS_OBJECT *pObject = reinterpret_cast<CLASS_OBJECT*>(createFunc(object_name, NULL));
    return pObject;
  }  
  return NULL;  
}  

///  定义类的全局静态创建函数，类声明时使用
#define DECLARE_CREATEINSTANCE(CLASS_OBJECT) \
 public: \
  static CLASS_OBJECT* CreateInstance(  \
    const std::string &object_name = #CLASS_OBJECT, INT32 *phr = 0){ \
      CLASS_OBJECT *pObj = new CLASS_OBJECT(object_name); \
      if (NULL == pObj && NULL != phr) {  \
         *phr = -1;  \
      }else{  \
           \
      }  \
      return pObj;  \
    };

// 定义类注册，用于初始化时，将类注册到工厂中
#define REGISTER_CLASS(FACTORY_OBJECT, CLASS_OBJECT, FACTORY_CLASS)  \
    FACTORY_OBJECT->RegisterClassNameToFactory(#CLASS_OBJECT,  \
      reinterpret_cast<FACTORY_CLASS::CreateFunctionPtr>(CLASS_OBJECT::CreateInstance));

#define DYNAMIC_REGISTER_CLASS(FACTORY_OBJECT, CLASS_OBJECT, FACTORY_CLASS, CREATE_FUNCTION)  \
  FACTORY_OBJECT->RegisterClassNameToFactory(CLASS_OBJECT,  \
  reinterpret_cast<FACTORY_CLASS::CreateFunctionPtr>(CREATE_FUNCTION));

}  // namespace ysos

/// 注册所有由BaseInterface接口继承的子类到工厂类，方便工厂统一创建//NOLINT
#define REGISTER_INTERFACE_CLASS(FACTORY_OBJECT, CLASS_OBJECT, FACTORY_CLASS)  \
    CLASS_OBJECT::SetClassName(#CLASS_OBJECT);  \
    FACTORY_OBJECT.RegisterClassNameToFactory(#CLASS_OBJECT,  \
    reinterpret_cast<FACTORY_CLASS::CreateFunctionPtr>(CLASS_OBJECT::CreateInstance));

/// 注册所有由BaseInterface接口继承的子类到工厂类，方便工厂统一创建
#define REGISTER_BASEINTERFACE_CLASS    REGISTER_INTERFACE_CLASS

/// 统一定义接口工厂的类定义，各个接口及其子类将统一用该工厂创建
#define DECLARE_INTERFACEFACTORY(InterfaceName)  \
class  YSOS_EXPORT InterfaceName##Factory : public ysos::TemplateFactory<ADD_NAMESPACE(ysos,InterfaceName)/*ysos::##InterfaceName*/> { \
 private: \
    InterfaceName##Factory(const std::string &FactoryName, INT32 *phr = NULL) :  \
       ysos::TemplateFactory<ADD_NAMESPACE(ysos,InterfaceName)/*ysos::##InterfaceName*/>(FactoryName) {}  \
 public: \
    DECLARE_CREATEINSTANCE(InterfaceName##Factory);  \
};  \
DECLARE_CREATEFACTORYINSTANCE(InterfaceName##Factory);

/// 定义插件的注册接口，在每个插件模块的类声明后，添加 //  NOLINT     
//need update for linux   change 'char *'-> 'char const *'  before in 'Get_YSOS_Plugin_Name' and 'Get_YSOS_Plugin_Interface_Name'
#define DECLARE_PLUGIN_REGISTER_INTERFACE(class_name, interface_name) \
  YSOS_EXTERN_C YSOS_EXPORT char const *Get_YSOS_Plugin_Name(void) {\
  return #class_name ;\
}\
YSOS_EXTERN_C YSOS_EXPORT char const *Get_YSOS_Plugin_Interface_Name(void) { \
  return #interface_name ; \
} \
YSOS_EXTERN_C YSOS_EXPORT  interface_name *GET_YSOS_Plugin_Interface (  \
  const std::string &object_name = #class_name, INT32 *phr = 0) {\
    return class_name::CreateInstance(object_name, phr);\
}

#endif // TEMPLATE_FACTORY_H  //NOLINT
