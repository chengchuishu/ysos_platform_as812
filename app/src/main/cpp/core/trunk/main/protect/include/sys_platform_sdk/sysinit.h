/**
*@file sysinit.h
*@brief Definition of SysInit
*@version 0.1
*@author jinchengzhe
*@date Created on: 2016-06-13 10:46:58
*@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
* 
*/

#ifndef SFP_SYS_INIT_H_
#define SFP_SYS_INIT_H_

/// 3rdparty Headers
//#include <log4cplus/configurator.h>
/// Self Headers //  NOLINT
#include <boost/shared_ptr.hpp>
#include "../../../public/include/core_help_package/utility.h"
#include "../../../protect/include/sys_interface_package/plugininterface.h"
#include "../../../protect/include/sys_framework_package/baseappmanagerimpl.h"
#include "../../../protect/include/core_help_package/singletontemplate.h"

namespace ysos {

class LightLock;
class PackageConfigImpl;
class ModuleInterface;
class ModuleLinkInterface;

class SysInit;
typedef boost::shared_ptr<SysInit> SysInitPtr;

class YSOS_EXPORT SysInit {
  DISALLOW_COPY_AND_ASSIGN(SysInit);
  DECLARE_PROTECT_CONSTRUCTOR(SysInit);

 public:
  ~SysInit();

  /* enum PluginType {
     MODULE,  ///<  module_interface  //  NOLINT
     CALLBACK, ///<  callback_interface  //  NOLINT
     STRATEGY, ///<  strategy_interface  //  NOLINT
     AGENT,  ///<  agent_interface  //  NOLINT
     PROTOCOL,  ///<  protocol_interface  //  NOLINT
     MODULE_LINK,  ///<  module_link_interface  //  NOLINT
     PluginTypeEnd
   };*/

  typedef char*(*GET_PLUGIN_NAME)(void);
  struct PluginInfoInterface {
    std::string       plugin_name;  ///<  plugin's class name //  NOLINT
    int               plugin_type;     ///<  plugin's type  //  NOLINT
    void             *plugin_create_function;  ///<  plugin's create function //  NOLINT
  };
  typedef boost::shared_ptr<PluginInfoInterface>  PluginInfoInterfacePtr;

  int Init(const std::string& file_path/*, const std::string &ip_addr, const unsigned short port*/);
  int Uninit(void);

 protected:
  int Load();
  int Unload();

  int Run(const std::string &ip_addr, const unsigned short port, const std::string &app_conf_dir);
  int Stop(void);

  int ManagerRecycle(void);
  int RecycleStrategyInterface(std::string &strategy_name);
  int RecycleModuleLinkInterface(std::string &module_link_name);
  int RecycleModuleInterface(std::string &module_name);
  int RecycleCallbackInterface(std::string &callback_name);
  int RecycleDriverInterface(std::string &driver_name);
  int RecycleProtocolInterface(std::string &protocol_name);
  int RecycleAgentInterface(std::string &agent_name);

  void InitGlobalFactoryAndManager();
  void UninitGlobalFactoryAndManager();

  void InitPlugin(void);
  void UnInitPlugin(void);

 private:
  PluginInfoInterfacePtr GetPluginInfo(void* plugin_handle);

  int RegisterClasses();
  int RegisterObjects();
  int CreateObjects();

//   int RegisterModuleLinkClasses();
//   int RegisterModuleClasses();
//   int RegisterCallbackClasses();

  int RegisterModuleLinkObjects();
  int RegisterModuleObjects();
  int RegisterCallbackObjects();
  int RegisterPlugins(void);

  int CreateModuleObjects();
  int CreateModuleLinkObjects();

  /// 扫描ModuleLink配置信息, 将其中内置的Repeater创建出来,//NOLINT
  /// 添加Callback, 并链到Link中.//NOLINT
  int CreateInherentRepeaterObjects();

  int AddCallback2Module(const std::string& module_name, const boost::shared_ptr<ModuleInterface> module_interface_ptr);
  std::string GetElementName(std::string& element_name);
  boost::shared_ptr<ModuleInterface> FindNextModule(std::string& next_module_id, std::string& module_link_name);

  boost::shared_ptr<PackageConfigImpl> package_config_impl_ptr_;
  int                                           config_wath_interval_;  ///<  检测Log配置文件的时间间隔 //  NOLINT
//  log4cplus::ConfigureAndWatchThread           *config_and_watch_thread_;  ///<  定期检测Log配置文件是否有更新 //  NOLINT
  BaseAppManagerImplPtr                         app_manager_ptr_;          ///< app manager指针  //  NOLINT
  log4cplus::Logger logger_;

  DECLARE_SINGLETON_VARIABLE(SysInit);
};
}  //  end of namespace
#define GetSysInit  ysos::SysInit::Instance
#endif
