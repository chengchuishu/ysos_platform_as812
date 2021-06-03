/**
 *@file PackageConfigImpl.h
 *@brief Definition of PackageConfigImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

#ifndef SFP_PACAGE_CONFIG_H_
#define SFP_PACAGE_CONFIG_H_

/// C Headers
#include <cstdlib>
#include <cstdio>
#include <cstring>
/// STL Headers
#include <list>
#include <string>
#include <iostream>
/// Boost Headers
#include <boost/shared_ptr.hpp>
/// Private Ysos Headers
#include "../../../public/include/sys_interface_package/common.h"
#include "../../../public/include/sys_interface_package/moduleinterface.h"
#include "../../../protect/include/sys_framework_package/modulelinkimpl.h"
#include "../../../protect/include/sys_framework_package/serviceinfotable.h"
#include <tinyxml2/tinyxml2.h>

namespace ysos {

struct SysConfInfoElement {
  std::string dir_name;
  std::list<std::string> exclude_file_list;
};

typedef boost::shared_ptr<SysConfInfoElement> SysConfInfoElementPtr;

struct SysConfInfo {
  SysConfInfoElementPtr   connection_info_ptr;
  SysConfInfoElementPtr   driver_info_ptr;
  SysConfInfoElementPtr   protocol_info_ptr;
  SysConfInfoElementPtr   callback_info_ptr;
  SysConfInfoElementPtr   module_info_ptr;
  SysConfInfoElementPtr   module_link_info_ptr;
  SysConfInfoElementPtr   service_info_list_ptr;
  SysConfInfoElementPtr   event_info_ptr;
  SysConfInfoElementPtr   plugin_info_ptr;
  SysConfInfoElementPtr   instance_info_ptr;
  SysConfInfoElementPtr   appmanager_info_ptr;

};

typedef std::list<std::string>::iterator SysConfInfoElementIterator;
typedef std::list<std::string> SysConfInfoElementList;

struct ModuleConfInfo {
  INT64  instance_id; ///<  used only for modulelink: moduleinstance id //NOLINT
  std::string instance_name;
  std::string module_name;
  std::string  name;
  std::string  version;
  std::string  type;
  bool  is_local;   ///< local
  std::string uri;
  std::string input_type;
  std::string output_type;
  bool  is_allocated;  ///< allocator
  INT64 prefix;
  INT64 buffer_length;
  bool  is_revised; ///< revise
  INT64 buffer_number;
  INT64 capacity;
  std::string remark;

  ModuleConfInfo() {
    instance_id = prefix = buffer_number = buffer_length = capacity = 0;
  }
};

typedef boost::shared_ptr<ModuleConfInfo> ModuleConfInfoPtr;

struct ModuleLinkModuleConfInfo {
  int64_t instance_id;  ///<  used only for modulelink: moduleinstance id
  uint16_t instance_level;
  std::string module_name;
  std::string instance_name;

  /// 为支持通过配置文件给ModuleLink的内置Repeater指定Callback
  std::string repeater_before_it;   ///< 以此模块为终点的某条边将加入对应的Repeater
  std::string repeater_after_it;    ///< 以此模块为始点的某条边将加入对应的Repeater

  std::list<std::string> next_modules;

  /// 若一个Module有> 1个PrevModule, 则默认对应的边将被加入内置的Repeater.//NOLINT
  /// 这里的名字为实例逻辑名.//NOLINT
  std::list<std::string> prev_modules;

  /// 由此导出将被包含在内置Repeater里的边.//NOLINT
  std::list<std::string> next_module_concerned_with_inherent_repeater;

  ModuleLinkModuleConfInfo():instance_id(0), instance_level(0) {}
};

struct InstanceInfo {
  std::string     logic_name;  ///<  模块的逻辑名 //  NOLINT
  std::string     type;  ///<  模块的类型，支持Moudle、Callback、Stategy、Agent、Driver等  //  NOLINT
  std::string     class_name;  ///<  模块所属的类的名称     //  NOLINT
};
typedef boost::shared_ptr<InstanceInfo> InstanceInfoPtr;
typedef std::list<InstanceInfoPtr >              InstanceInfoList;
typedef std::list<InstanceInfoPtr >::iterator              InstanceInfoListIterator;

typedef boost::shared_ptr<ModuleLinkModuleConfInfo>  ModuleLinkModuleConfInfoPtr;

typedef std::list<ModuleLinkModuleConfInfoPtr>        ModuleInfoListInLink;

typedef ModuleInfoListInLink::iterator                ModuleInfoListInLinkIterator;

/// 某个知名Repeater,及属于它的边的列表形成的pair
typedef std::pair<std::string, EdgeOfModuleNameList>  FamousRepeater_EdgeList_Pair;

/// 知名Repeater及属于它的边的列表形成的pair
typedef std::map<std::string, EdgeOfModuleNameList>   FamousRepeater_EdgeList_Map;
typedef FamousRepeater_EdgeList_Map::iterator         FamousRepeater_EdgeList_MapIterator;

template<typename EleType>
void push_back_if_not_in_list(std::list<EleType>& the_list, EleType& the_element) {
  typename std::list<EleType>::iterator it = std::find(the_list.begin(), the_list.end(), the_element);  //need add 'typename' for linux
  //auto it = std::find(the_list.begin(), the_list.end(), the_element);  //need add 'typename' for linux
  if (it == the_list.end()) {
    the_list.push_back(the_element);
  }
}

struct InherentRepeaterConfInfo {
  uint32_t level;
  std::string name;

  /// 为支持通过配置文件给ModuleLink的内置Repeater指定Callback
  std::string famous_name;


  std::list<std::string> prev_module_name_list;
  std::list<std::string> callback_name_list;
  EdgeOfModuleNameList edge_list;
  InherentRepeaterConfInfo() {}
  ~InherentRepeaterConfInfo() {
    level = 0;
    edge_list.clear();
    prev_module_name_list.clear();
    callback_name_list.clear();
  }
};

typedef std::map<uint32_t, InherentRepeaterConfInfo> InherentRepeaterConfInfoMap;
typedef InherentRepeaterConfInfoMap::iterator        InherentRepeaterConfInfoMapIterator;

struct ModuleLinkConfInfo {
  INT64 id;
  bool autorun;
  std::string name;
  std::string remark;
  ModuleInfoListInLink module_list;
  InherentRepeaterConfInfoMap repeater_info_list;

  FamousRepeater_EdgeList_Map famous_repeater_edge_list_map;

  /// max level of module in module_list
  uint32_t depth;
  ModuleLinkConfInfo() {
    id = 0;
    autorun = false;
    depth = -1;
  }
  ~ModuleLinkConfInfo() {
    id = 0;
    autorun = false;
    depth = 0xffffffffU;
    module_list.clear();
    famous_repeater_edge_list_map.clear();
    repeater_info_list.clear();
  }

  uint32_t GetDepth();
};

typedef boost::shared_ptr<ModuleLinkConfInfo>  \
ModuleLinkConfInfoPtr;
typedef std::list<ModuleLinkModuleConfInfoPtr>  \
ModuleLinkModuleConfInfoList;

struct CallbackConfInfo {
  std::string  name;
  std::string  instance_name;
  std::string  owner;
  std::string version;
  std::string remark;
};

typedef boost::shared_ptr<CallbackConfInfo> CallbackConfInfoPtr;

struct ProtocolConfInfo {
  std::string  name;
  std::string  owner;
  std::string version;
  std::string remark;
};

typedef boost::shared_ptr<ProtocolConfInfo> ProtocolConfInfoPtr;

struct DriverConfInfo {
  std::string  name;
  std::string version;
  std::string remark;
};
typedef boost::shared_ptr<DriverConfInfo> DriverConfInfoPtr;

typedef DriverConfInfo  ConnectionConfInfo;
typedef boost::shared_ptr<ConnectionConfInfo> ConnectionConfInfoPtr;

typedef boost::shared_ptr<std::multimap<std::string, std::string> > MapPtr;
typedef std::multimap<std::string, MapPtr >                         ConfMap;
typedef std::multimap<std::string, std::string>::iterator           MapIterator;

class LightLock;
class PackageConfigImpl;
typedef boost::shared_ptr<PackageConfigImpl> PackageConfigImplPtr;
class YSOS_EXPORT PackageConfigImpl {
 public:

  typedef  std::list<ModuleLinkConfInfoPtr>::iterator  ModuleLinkConfInfoIterator;
  typedef std::list<ModuleConfInfoPtr>::iterator       ModuleConfInfoIterator;
  typedef std::list<CallbackConfInfoPtr>::iterator     CallbackConfInfoIterator;
  typedef std::list<ModuleLinkModuleConfInfoPtr>::iterator  ModuleLinkModuleConfInfoIterator;
  typedef std::list<ProtocolConfInfoPtr>::iterator     ProtocolConfInfoIterator;
  typedef std::list<std::string>::iterator             ModuleLinkModuleConfInfoNextModulesIterator;
  typedef std::list<DriverConfInfoPtr>::iterator       DriverConfInfoPtrIterator;
  typedef std::list<ConnectionConfInfoPtr>::iterator   ConnectionConfInfoPtrIterator;

  ~PackageConfigImpl();

  MapPtr FindModuleConf(const std::string &module_name);
  MapPtr FindCallbackConf(const std::string &callback_name);
  MapPtr FindProtocolConf(const std::string &protocol_name);
  MapPtr FindDriverConf(const std::string &driver_name);
  MapPtr FindConnectionConf(const std::string &connection_name);

  ModuleLinkConfInfoIterator ModuleLinkConfInfoHead(void);
  ModuleLinkConfInfoIterator ModuleLinkConfInfoEnd(void);
  ModuleLinkConfInfoIterator FindModuleLinkConfInfo(const std::string &module_link_conf_info_name);

  ModuleConfInfoIterator ModuleConfInfoHead(void);
  ModuleConfInfoIterator ModuleConfInfoEnd(void);
  ModuleConfInfoIterator FindModuleConfInfo(const std::string &module_conf_info_name);

  CallbackConfInfoIterator CallbackConfInfoHead(void);
  CallbackConfInfoIterator CallbackConfInfoEnd(void);
  CallbackConfInfoIterator FindCallbackConfInfo(const std::string &callback_conf_info_name);

  ProtocolConfInfoIterator ProtocolConfInfoHead(void);
  ProtocolConfInfoIterator ProtocolConfInfoEnd(void);
  ProtocolConfInfoIterator FindProtocolConfInfo(const std::string &protocol_conf_info_name);

  DriverConfInfoPtrIterator DriverConfInfoHead(void);
  DriverConfInfoPtrIterator DriverConfInfoEnd(void);
  DriverConfInfoPtrIterator FindDriverConfInfo(const std::string &driver_conf_info_name);

  ConnectionConfInfoPtrIterator ConnectionConfInfoHead(void);
  ConnectionConfInfoPtrIterator ConnectionConfInfoEnd(void);
  ConnectionConfInfoPtrIterator FindConnectionConfInfo(const std::string &connection_conf_info_name);

  InstanceInfoListIterator InstanceInfoHead(void);
  InstanceInfoListIterator InstanceInfoEnd(void);
  InstanceInfoListIterator FindInstanceInfo(const std::string &instance_info_name);
  int GetCallbackNamList(const std::string& module_name, std::list<std::string> &callback_name_list);
  const std::string GetServerAddress(void) const;
  const uint16_t GetServerPort(void) const ;
  std::string GetAppManagerConf(void) const;

  static PackageConfigImplPtr instance();

  int ParseSysInfoFile(const std::string &configure_file);
  const std::string  GetConfPath(void) const;

  int CreateModuleLinkObject(ModuleLinkInterface* module_link_interface_ptr, std::string &module_link_name);
  boost::shared_ptr<ModuleInterface> FindNextModule(std::string& next_module_id, ModuleLinkConfInfoIterator& module_link_info_iterator);
  void Dump(void);

 protected:
  int ParseConfigureFile(void);

  int ParseServerInfo(tinyxml2::XMLElement *element);

  tinyxml2::XMLElement *GetRootElement(tinyxml2::XMLDocument &doc, const std::string &file_name, const std::string &next_element="");
  SysConfInfoElementPtr ParseSysConfInfoElement(tinyxml2::XMLElement *element);

  int ParseModuleConfInfo(SysConfInfoElementPtr module_info_ptr);
  ModuleConfInfoPtr ParseModuleConfInfoImpl(const std::string &file_name);

  int ParseModuleLinkConfInfo(SysConfInfoElementPtr module_link_info_ptr);
  ModuleLinkConfInfoPtr ParseModuleLinkConfInfoImpl(const std::string &file_name);
  ModuleLinkModuleConfInfoPtr ParseModuleLinkModuleConfInfoImpl(tinyxml2::XMLElement *element);

  /// 设置prev_modules实例逻辑名
  void UpdateModuleLinkModuleConfInfoImpl_FillPrevModuleList(ModuleInfoListInLink& module_list);

  /// 更新边表:生成将被包含在内置Repeater里的边表,并将这些边从原有边表中移除
  void UpdateModuleLinkModuleConfInfoImpl_UpdateNextModuleList(ModuleInfoListInLink& module_list);

  /// 获取知名Repeater的信息
  void GetFamousRepeaterInfo(ModuleLinkConfInfoPtr& module_link_conf_info_ptr);

  /// 由知名Repeater信息更新内置Repeater信息
  void UpdateInherentRepeaterListByFamousRepeaterInfo(ModuleLinkConfInfoPtr module_link_conf_info_ptr);

  void UpdateModuleLinkModuleConfInfoImpl(ModuleInfoListInLink& module_list);

  /// 生成内置Repeater信息//NOLINT
  void GenerateInherentRepeaterList(ModuleLinkConfInfoPtr& link_config_info);

  int ParseCallbackConfInfo(SysConfInfoElementPtr callback_info_ptr);
  CallbackConfInfoPtr ParseCallbackConfInfoImpl(const std::string &file_name);

  int ParseProtocolConfInfo(SysConfInfoElementPtr protocol_info_ptr);
  ProtocolConfInfoPtr ParseProtocolConfInfoImpl(const std::string &file_name);

  int ParseDriverConfInfo(SysConfInfoElementPtr driver_info_ptr);
  DriverConfInfoPtr ParseDriverConfInfoImpl(const std::string &file_name);

  int ParseConnectionConfInfo(SysConfInfoElementPtr connection_info_ptr);
  DriverConfInfoPtr ParseConnectionConfInfoImpl(const std::string &file_name);

  int ParseServiceConfInfoList(const SysConfInfoElementPtr service_info_list_ptr);
  int ParseServiceConfInfoListImpl(const std::string& file_name);
  ServiceInfoTable::ServiceConfInfoPtr ParseServiceConfInfoImpl(tinyxml2::XMLElement* element);
  std::string SeekModuleInstanceName(const std::string& sub_service_name);
  std::string SeekCallbackInstanceName(const std::string& service_name);

  int PaseEventConfInfo(SysConfInfoElementPtr event_info_ptr);

  int PaseInstanceConfInfo(SysConfInfoElementPtr plugin_info_ptr);
  int PaseInstanceConfInfoImpl(const std::string& file_name);
  InstanceInfoPtr PaseInstanceConfInfoImpl(tinyxml2::XMLElement* element);

  void DumpModuleConfInfoList(void);
  void DumpModuleLinkConfInfoList(void);
  void DumpCallbackConfInfoList(void);
  void DumpServiceConfInfoList(void);

  void PrintfModuleConfInfo(const ModuleConfInfoPtr &module_conf_info_ptr, bool is_module_link=false);
  void PrintfModuleLinkConfInfo(const ModuleLinkConfInfoPtr &module_link_conf_info_ptr);
  void PrintfCallbackConfInfo(const CallbackConfInfoPtr &callback_conf_info_ptr);
  void PrintfModuleLinkModuleConfInfo(const ModuleLinkModuleConfInfoPtr &module_link_module_conf_info_ptr);
  void PrintServiceConfInfo(const ServiceInfoTable::ServiceConfInfoPtr &service_conf_info_ptr);

 private:
  PackageConfigImpl();
  PackageConfigImpl(const PackageConfigImpl&);
  PackageConfigImpl& operator=(const PackageConfigImpl&);

  static PackageConfigImplPtr package_config_impl_ptr_;
  static boost::shared_ptr<LightLock> light_lock_ptr_;

  std::list<ModuleConfInfoPtr> module_conf_info_list_;
  std::list<ModuleLinkConfInfoPtr> module_link_conf_info_list_;
  std::list<CallbackConfInfoPtr> callback_conf_info_list_;
  std::list<ProtocolConfInfoPtr> protocol_conf_info_list_;
  std::list<DriverConfInfoPtr> driver_conf_info_list_;
  std::list<ConnectionConfInfoPtr> connection_conf_info_list_;
  InstanceInfoList               instance_info_list_;

  ConfMap                        module_conf_;    ///<  Module的配置文件信息 //  NOLINT
  ConfMap                        callback_conf_;  ///<  Callback的配置文件信息 //  NOLINT
  ConfMap                        protocol_conf_;  ///<  Protocol的配置文件信息 //  NOLINT
  ConfMap                        driver_conf_;    ///<  Driver的配置文件信息 //  NOLINT
  ConfMap                        connection_conf_;    ///<  Driver的配置文件信息 //  NOLINT

  std::string                    ip_address_;     ///< Server‘s address
  uint16_t                       ip_port_;        ///< Server's port

  SysConfInfo sys_conf_info_;
  std::string sys_conf_dir_;

  log4cplus::Logger logger_;
};

}
#define GetPackageConfigImpl  ysos::PackageConfigImpl::instance
#endif
