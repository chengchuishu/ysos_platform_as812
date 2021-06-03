/**
*@file sysinit.cpp
*@brief Definition of SysInit
*@version 0.1
*@author jinchengzhe
*@date Created on: 2016-06-13 10:46:58
*@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
* 
*/

/// Ysos Headers //  NOLINT
#include "../../../public/include/sys_interface_package/common.h"
#include "../../../public/include/sys_interface_package/ysos_enum.h"
#include "../../../public/include/core_help_package/utility.h"
// #include "../../../public/include/core_help_package/ysos_log.h"
#include "../../../public/include/sys_framework_package/templatemanager.h"
#include "../../../protect/include/sys_framework_package/manager_declare.h"
#include "../../../protect/include/sys_platform_sdk/sysinit.h"
#include "../../../protect/include/sys_framework_package/packageconfigimpl.h"
#include "../../../public/include/os_hal_package/lock.h"
#include "../../../public/include/sys_interface_package/moduleinterface.h"
#include "../../../protect/include/sys_framework_package/modulelinkimpl.h"
#include "../../../protect/include/sys_framework_package/basemoduleimpl.h"
#include "../../../protect/include/os_hal_package/pluginmanager.h"
#include "../../../public/include/os_hal_package/filepath.h"
#include "../../../protect/include/os_hal_package/datainterfacemanager.h"
#include "../../../protect/include/core_help_package/log.h"
//  agent
#include "../../../protect/include/sys_framework_package/baseagentimpl.h"
//  strategy
#include "../../../protect/include/sys_framework_package/basestrategyimpl.h"
#include "../../../protect/include/sys_framework_package/basecommonstrategyimpl.h"  // NOLINT
#include "../../../protect/include/sys_framework_package/ysosdaemonstrategyimpl.h"  // NOLINT
//  modulelink
#include "../../../protect/include/sys_framework_package/modulelinkimpl.h"
//  repeater
#include "../../../protect/include/sys_framework_package/baserepeatermoduleimpl.h"
#include "../../../protect/include/sys_framework_package/baserepeatercallbackimpl.h"
/// Callback
#include "../../../protect/include/sys_framework_package/statemachinecommoncallback.h"
#include "../../../protect/include/sys_framework_package/basetransitioncallbackimpl.h"
#include "../../../protect/include/sys_framework_package/statuseventmodulecallback.h"  // NOLINT
#include "../../../protect/include/sys_framework_package/basestrategyproxycallbackimpl.h"
/// Connection        // NOLINT
#include "../../../protect/include/sys_framework_package/baseconnectionimpl.h"

namespace ysos {

/// 普通类注册 //  NOLINT
///   Protocol   //  NOLINT
#define RegisterProtocolClass(protocol)  \
    REGISTER_CLASS(ysos::GetProtocolInterfaceFactory(), protocol, ysos::ProtocolInterfaceFactory);
///  Module  //  NOLINT
#define RegisterModuleClass(module)  \
    REGISTER_CLASS(ysos::GetModuleInterfaceFactory(), module, ysos::ModuleInterfaceFactory);
///  Callback  //  NOLINT
#define RegisterCallbackClass(callback)  \
    REGISTER_CLASS(ysos::GetCallbackInterfaceFactory(), callback, ysos::CallbackInterfaceFactory);
///  Driver  //  NOLINT
#define RegisterDriverClass(driver)  \
  REGISTER_CLASS(ysos::GetDriverInterfaceFactory(), driver, ysos::DriverInterfaceFactory);
///  ModuleLink  //  NOLINT
#define RegisterModuleLinkClass(module_link) \
    REGISTER_CLASS(ysos::GetModuleLinkInterfaceFactory(), module_link, ysos::ModuleLinkInterfaceFactory);
///  Strategy  //  NOLINT
#define RegisterStrategyClass(strategy) \
    REGISTER_CLASS(ysos::GetStrategyInterfaceFactory(), strategy, ysos::StrategyInterfaceFactory);
/// Agent  //  NOLINT
#define RegisteAgentClass(agent) \
    REGISTER_CLASS(ysos::GetAgentInterfaceFactory(), agent, ysos::AgentInterfaceFactory);
/// Connection  //  NOLINT
#define RegisteConnectionClass(connection) \
  REGISTER_CLASS(ysos::GetConnectionInterfaceFactory(), connection, ysos::ConnectionInterfaceFactory);

/// 插件注册 //  NOLINT
///  Protocol  //  NOLINT
#define DynamicRegisterProtocolClass(protocol, create_function)  \
    DYNAMIC_REGISTER_CLASS(ysos::GetProtocolInterfaceFactory(), protocol, ysos::ProtocolInterfaceFactory, create_function);
///  Module  //  NOLINT
#define DynamicRegisterModuleClass(module, create_function)  \
    DYNAMIC_REGISTER_CLASS(ysos::GetModuleInterfaceFactory(), module, ysos::ModuleInterfaceFactory, create_function);
///  Driver  //  NOLINT
#define DynamicRegisterDriverClass(driver, create_function)  \
  DYNAMIC_REGISTER_CLASS(ysos::GetDriverInterfaceFactory(), driver, ysos::DriverInterfaceFactory, create_function);
///  Callback  //  NOLINT
#define DynamicRegisterCallbackClass(callback, create_function)  \
    DYNAMIC_REGISTER_CLASS(ysos::GetCallbackInterfaceFactory(), callback, ysos::CallbackInterfaceFactory, create_function);
///  ModuleLink  //  NOLINT
#define DynamicRegisterModuleLinkClass(module_link, create_function) \
    DYNAMIC_REGISTER_CLASS(ysos::GetModuleLinkInterfaceFactory(), module_link, ysos::ModuleLinkInterfaceFactory, create_function);
///  Strategy  //  NOLINT
#define DynamicRegisterStrategyClass(strategy, create_function) \
    DYNAMIC_REGISTER_CLASS(ysos::GetStrategyInterfaceFactory(), strategy, ysos::StrategyInterfaceFactory, create_function);
///  Agent  //  NOLINT
#define DynamicRegisteAgentClass(agent, create_function) \
    DYNAMIC_REGISTER_CLASS(ysos::GetAgentInterfaceFactory(), agent, ysos::AgentInterfaceFactory, create_function);
///  Connection  //  NOLINT
#define DynamicRegisteConnectionClass(connection, create_function) \
    DYNAMIC_REGISTER_CLASS(ysos::GetConnectionInterfaceFactory(), connection, ysos::ConnectionInterfaceFactory, create_function);

/// 逻辑名注册 //  NOLINT
///  Protocol  //  NOLINT
#define Register_Protocol_Object(logic_name, class_name) \
    REGISTER_LOGIC_CLASS_NAME(ysos::GetProtocolInterfaceManager(), logic_name, class_name);
///  Module  //  NOLINT
#define Register_Module_Object(logic_name, class_name) \
  REGISTER_LOGIC_CLASS_NAME(ysos::GetModuleInterfaceManager(), logic_name, class_name);
///  Repeater  //  NOLINT
#define Register_Repeater_Object(logic_name, class_name) \
  REGISTER_LOGIC_CLASS_NAME(ysos::GetModuleInterfaceManager(), logic_name, class_name);
///  Callback  //  NOLINT
#define Register_Callback_Object(logic_name, class_name) \
  REGISTER_LOGIC_CLASS_NAME(ysos::GetCallbackInterfaceManager(), logic_name, class_name);
///  Driver  //  NOLINT
#define Register_Driver_Object(logic_name, class_name) \
  REGISTER_LOGIC_CLASS_NAME(ysos::GetDriverInterfaceManager(), logic_name, class_name);
///  ModuleLink  //  NOLINT
#define Register_Module_Link_Object(logic_name, class_name) \
  REGISTER_LOGIC_CLASS_NAME(ysos::GetModuleLinkInterfaceManager(), logic_name, class_name);
///  Strategy  //  NOLINT
#define Register_Strategy_Object(logic_name, class_name) \
  REGISTER_LOGIC_CLASS_NAME(ysos::GetStrategyInterfaceManager(), logic_name, class_name);
///  Agent  //  NOLINT
#define Register_Agent_Object(logic_name, class_name) \
  REGISTER_LOGIC_CLASS_NAME(ysos::GetAgentInterfaceManager(), logic_name, class_name);
///  Connection  //  NOLINT
#define Register_Connection_Object(logic_name, class_name) \
  REGISTER_LOGIC_CLASS_NAME(ysos::GetConnectionInterfaceManager(), logic_name, class_name);
///  ConnectionCallback  //  NOLINT
#define Register_Connection_Callback_Object(logic_name, class_name) \
    REGISTER_LOGIC_CLASS_NAME(ysos::GetConnectionCallbackInterfaceManager(), logic_name, class_name);
///  Transport  //  NOLINT
#define Register_Transport_Object(logic_name, class_name) \
    REGISTER_LOGIC_CLASS_NAME(ysos::GetTransportInterfaceManager(), logic_name, class_name);
///  TransportCallback  //  NOLINT
#define Register_Transport_Callback_Object(logic_name, class_name) \
    REGISTER_LOGIC_CLASS_NAME(ysos::GetTransportCallbackInterfaceManager(), logic_name, class_name);
///  RPC  //  NOLINT
#define Register_RPC_Object(logic_name, class_name) \
    REGISTER_LOGIC_CLASS_NAME(ysos::GetRPCInterfaceManager(), logic_name, class_name);
///  RPCCallback  //  NOLINT
#define Register_RPC_Callback_Object(logic_name, class_name) \
    REGISTER_LOGIC_CLASS_NAME(ysos::GetRPCCallbackInterfaceManager(), logic_name, class_name);

////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////            SysInit                /////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
DEFINE_SINGLETON(SysInit);
SysInit::SysInit(const std::string &strClassName) {
  config_wath_interval_ = 30 * 1000;   ///<  30秒 //  NOLINT
}

SysInit::~SysInit() {
  YSOS_LOG_INFO("SysInit::~SysInit Start");
}

int SysInit::Init(const std::string& file_path/*, const std::string &ip_addr, const unsigned short port*/) {
  if (NULL == package_config_impl_ptr_) {
    package_config_impl_ptr_ = GetPackageConfigImpl();
  }

  std::string log_path = "../log/";
  if (!FilePath::IsDirectory(log_path)) {
    bool ret = FilePath::CreateDirectory(log_path);
  }

  std::string log_file_path = GetPackageConfigImpl()->GetConfPath() + "log/log.properties";
  if (true != log::InitLogger(log_file_path)) {
    return YSOS_ERROR_FAILED;
  }

  /*
  //log4cplus::PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT(log_file_path.c_str()));
  config_and_watch_thread_ = new log4cplus::ConfigureAndWatchThread(log_file_path.c_str(), config_wath_interval_);
  assert(NULL != config_and_watch_thread_);
  // log4cplus::ConfigureAndWatchThread configureThread("log4cplus.properties", 5 * 1000);
  */

  logger_ = GetUtility()->GetLogger("ysos");
  YSOS_LOG_INFO("SysInit::Init Start");

  int ret = GetPackageConfigImpl()->ParseSysInfoFile(file_path);
  if (YSOS_ERROR_SUCCESS != ret) {
    YSOS_LOG_ERROR("SysInit::Init failed on ParseSysInfoFile");
    return ret;
  }
  YSOS_LOG_INFO("SysInit::GetPackageConfigImpl()->ParseSysInfoFile()");
//   package_config_impl_ptr_->Dump();
  InitGlobalFactoryAndManager();
  YSOS_LOG_INFO("SysInit::Init InitGlobalFactoryAndManager()");
  // init plugin
  InitPlugin();
  YSOS_LOG_INFO("SysInit::Init InitPlugin()");
  //  Register and Create
  ret = RegisterClasses();
  if (YSOS_ERROR_SUCCESS != ret) {
    YSOS_LOG_ERROR("\r\nSysInit::Init failed on RegisterClasses");
    return ret;
  }
  YSOS_LOG_INFO("SysInit::Init RegisterClasses()");
  ret = RegisterObjects();
  if (YSOS_ERROR_SUCCESS != ret) {
    YSOS_LOG_ERROR("\r\nSysInit::Init failed on RegisterObjects");
    return ret;
  }
  YSOS_LOG_INFO("SysInit::Init RegisterObjects()");
  ret = CreateObjects();
  if (YSOS_ERROR_SUCCESS != ret) {
    YSOS_LOG_ERROR("\r\nSysInit::Init failed on CreateObjects");
    return ret;
  }
  YSOS_LOG_INFO("SysInit::Init CreateObjects()");
  ret = Load();
  if (YSOS_ERROR_SUCCESS != ret) {
    YSOS_LOG_ERROR("Load failed: " << ret);
    return ret;
  }
  YSOS_LOG_INFO("SysInit::Init Load()");
  std::string ip_address = package_config_impl_ptr_->GetServerAddress();
  const uint16_t ip_port = package_config_impl_ptr_->GetServerPort();
  std::string app_conf_dir = package_config_impl_ptr_->GetAppManagerConf();

  ret = Run(ip_address, ip_port, app_conf_dir);
  if (YSOS_ERROR_SUCCESS != ret) {
    YSOS_LOG_ERROR("Run failed: " << ret);
  }
  YSOS_LOG_INFO("SysInit::Init Load()");
  YSOS_LOG_INFO("SysInit::Init End");
  return ret;
}

int SysInit::Uninit(void) {
  Stop();
//  Unload();
  ManagerRecycle();
  UninitGlobalFactoryAndManager();
  UnInitPlugin();
//  YSOS_DELETE(config_and_watch_thread_);

  return YSOS_ERROR_SUCCESS;
}

int SysInit::Load() {
  if (NULL == package_config_impl_ptr_) {
    return YSOS_ERROR_FAILED;
  }

  std::string temp_string;
  PackageConfigImpl::ModuleLinkConfInfoIterator it = package_config_impl_ptr_->ModuleLinkConfInfoHead();
  for (; it != package_config_impl_ptr_->ModuleLinkConfInfoEnd(); ++it) {
    if (true != (*it)->name.empty()) {
      YSOS_LOG_DEBUG("module link name: " << (*it)->name);
      temp_string = (*it)->name;
      if (temp_string.empty()) {
        continue;
      }

      ysos::ModuleLinkInterfacePtr module_link_ptr = ysos::GetModuleLinkInterfaceManager()->FindInterface((*it)->name);
      if (NULL == module_link_ptr) {
        continue;
      }
      if (YSOS_ERROR_SUCCESS != module_link_ptr->Initialize()) {
        return YSOS_ERROR_FAILED;
      }

      bool autorun = false;
      if (YSOS_ERROR_SUCCESS != module_link_ptr->GetProperty(PROP_MODULE_LINK_INFO_AUTORUN, &autorun)) {
        return YSOS_ERROR_FAILED;
      }

      if (autorun && YSOS_ERROR_SUCCESS != module_link_ptr->Run(NULL)) {
        return YSOS_ERROR_FAILED;
      }
    }
  }  // end for

  return YSOS_ERROR_SUCCESS;
}

int SysInit::Run(const std::string &ip_addr, const unsigned short port, const std::string &app_conf_dir) {
  AppManagerInterface::AppManagerParamPtr app_param_ptr = AppManagerInterface::AppManagerParamPtr(new AppManagerInterface::AppManagerParam());
  assert(NULL != app_param_ptr);
  app_param_ptr->address = ip_addr;
  //app_param_ptr->address = "192.168.1.134";
  app_param_ptr->port = port;
  //app_param_ptr->session_type = "SERVER";

  YSOS_LOG_DEBUG_DEFAULT("Begin to start server " << ip_addr << " : " << port);
  app_manager_ptr_ = ysos::BaseAppManagerImplPtr(ysos::BaseAppManagerImpl::CreateInstance());
  int ret = app_manager_ptr_->Start(app_param_ptr, app_conf_dir);
  if (YSOS_ERROR_SUCCESS != ret) {
    YSOS_LOG_ERROR("app manager start failed: " << ret);
  }

  return ret;
}

int SysInit::Stop(void) {
  assert(NULL != app_manager_ptr_);

  return app_manager_ptr_->Stop();
}

int SysInit::Unload() {
  if (NULL == package_config_impl_ptr_) {
    return YSOS_ERROR_FAILED;
  }

  std::string temp_string;
  PackageConfigImpl::ModuleLinkConfInfoIterator it = package_config_impl_ptr_->ModuleLinkConfInfoHead();
  for (; it != package_config_impl_ptr_->ModuleLinkConfInfoEnd(); ++it) {
    if (true != (*it)->name.empty()) {

      YSOS_LOG_DEBUG("module link name: " << (*it)->name);
      temp_string = (*it)->name;
      if (temp_string.empty()) {
        continue;
      }

      ysos::ModuleLinkInterfacePtr module_link_ptr = ysos::GetModuleLinkInterfaceManager()->FindInterface((*it)->name);
      if (NULL == module_link_ptr) {
        continue;
      }

      if (YSOS_ERROR_SUCCESS != module_link_ptr->Stop(NULL)) {
        return YSOS_ERROR_FAILED;
      }
    }
  }  // end for

  return YSOS_ERROR_SUCCESS;
}

int SysInit::ManagerRecycle(void) {
  InstanceInfoListIterator it = GetPackageConfigImpl()->InstanceInfoHead();
  InstanceInfoListIterator end_it = GetPackageConfigImpl()->InstanceInfoEnd();
  for (; end_it != it; ++it) {
    InstanceInfoPtr info_ptr = *it;
    if ("strategy" == info_ptr->type) {
      RecycleStrategyInterface(info_ptr->logic_name);
    }
  }

  it = GetPackageConfigImpl()->InstanceInfoHead();
  end_it = GetPackageConfigImpl()->InstanceInfoEnd();
  for (; end_it != it; ++it) {
    InstanceInfoPtr info_ptr = *it;
    if ("module_link" == info_ptr->type) {
      RecycleModuleLinkInterface(info_ptr->logic_name);
    }
  }

  it = GetPackageConfigImpl()->InstanceInfoHead();
  end_it = GetPackageConfigImpl()->InstanceInfoEnd();
  for (; end_it != it; ++it) {
    InstanceInfoPtr info_ptr = *it;
    if ("module" == info_ptr->type) {
      RecycleModuleInterface(info_ptr->logic_name);
    }
  }

  it = GetPackageConfigImpl()->InstanceInfoHead();
  end_it = GetPackageConfigImpl()->InstanceInfoEnd();
  for (; end_it != it; ++it) {
    InstanceInfoPtr info_ptr = *it;
    if ("callback" == info_ptr->type) {
      RecycleCallbackInterface(info_ptr->logic_name);
    }
  }

  it = GetPackageConfigImpl()->InstanceInfoHead();
  end_it = GetPackageConfigImpl()->InstanceInfoEnd();
  for (; end_it != it; ++it) {
    InstanceInfoPtr info_ptr = *it;
    if ("driver" == info_ptr->type) {
      RecycleDriverInterface(info_ptr->logic_name);
    }
  }

  it = GetPackageConfigImpl()->InstanceInfoHead();
  end_it = GetPackageConfigImpl()->InstanceInfoEnd();
  for (; end_it != it; ++it) {
    InstanceInfoPtr info_ptr = *it;
    if ("protocol" == info_ptr->type) {
      RecycleProtocolInterface(info_ptr->logic_name);
    }
  }

  return YSOS_ERROR_SUCCESS;
}

int SysInit::RecycleStrategyInterface(std::string &strategy_name) {
  StrategyInterfacePtr strategy_ptr = GetStrategyInterfaceManager()->FindInterface(strategy_name);
  if (NULL == strategy_ptr) {
    YSOS_LOG_ERROR("not exist strategy: " << strategy_name);
    return YSOS_ERROR_FAILED;
  }

  strategy_ptr->Stop();
  strategy_ptr->UnInitialize();

  return YSOS_ERROR_SUCCESS;
}

int SysInit::RecycleModuleLinkInterface(std::string &module_link_name) {
  ModuleLinkInterfacePtr module_link_ptr = GetModuleLinkInterfaceManager()->FindInterface(module_link_name);
  if (NULL == module_link_ptr) {
    YSOS_LOG_ERROR("not exist module link: " << module_link_name);
    return YSOS_ERROR_FAILED;
  }

  module_link_ptr->Stop();
  module_link_ptr->UnInitialize();

  return YSOS_ERROR_SUCCESS;
}

int SysInit::RecycleModuleInterface(std::string &module_name) {
  ModuleInterfacePtr module_ptr = GetModuleInterfaceManager()->FindInterface(module_name);
  if (NULL == module_ptr) {
    YSOS_LOG_ERROR("not exist module: " << module_name);
    return YSOS_ERROR_FAILED;
  }

  //module_ptr->Close();
  module_ptr->UnInitialize();

  return YSOS_ERROR_SUCCESS;
}

int SysInit::RecycleCallbackInterface(std::string &callback_name) {
  CallbackInterfacePtr callback_ptr = GetCallbackInterfaceManager()->FindInterface(callback_name);
  if (NULL == callback_ptr) {
    YSOS_LOG_ERROR("not exist callback: " << callback_name);
    return YSOS_ERROR_FAILED;
  }

  //callback_ptr->Stop();
  callback_ptr->UnInitialize();

  return YSOS_ERROR_SUCCESS;
}

int SysInit::RecycleDriverInterface(std::string &driver_name) {
  DriverInterfacePtr driver_ptr = GetDriverInterfaceManager()->FindInterface(driver_name);
  if (NULL == driver_ptr) {
    YSOS_LOG_ERROR("not exist driver: " << driver_name);
    return YSOS_ERROR_FAILED;
  }

  /// driver_ptr->Stop();
  driver_ptr->UnInitialize();

  return YSOS_ERROR_SUCCESS;
}

int SysInit::RecycleProtocolInterface(std::string &protocol_name) {
  ProtocolInterfacePtr protocol_ptr = GetProtocolInterfaceManager()->FindInterface(protocol_name);
  if (NULL == protocol_ptr) {
    YSOS_LOG_ERROR("not exist protocol: " << protocol_name);
    return YSOS_ERROR_FAILED;
  }

  /// protocol_ptr->Stop();
  protocol_ptr->UnInitialize();

  return YSOS_ERROR_SUCCESS;
}

int SysInit::RecycleAgentInterface(std::string &agent_name) {
  AgentInterfacePtr agent_ptr = GetAgentInterfaceManager()->FindInterface(agent_name);
  if (NULL == agent_ptr) {
    YSOS_LOG_ERROR("not exist protocol: " << agent_name);
    return YSOS_ERROR_FAILED;
  }

  /// protocol_ptr->Stop();
  agent_ptr->UnInitialize();

  return YSOS_ERROR_SUCCESS;
}

void SysInit::InitGlobalFactoryAndManager() {
  ysos::GetBaseInterfaceFactory();

  ysos::GetAgentInterfaceFactory();
  ysos::GetAgentInterfaceManager();

  ysos::GetStrategyInterfaceFactory();
  ysos::GetStrategyInterfaceManager();

  ysos::GetProtocolInterfaceFactory();
  ysos::GetProtocolInterfaceManager();

  ysos::GetCallbackInterfaceFactory();
  ysos::GetCallbackInterfaceManager();

  ysos::GetModuleInterfaceFactory();
  ysos::GetModuleInterfaceManager();

  ysos::GetModuleLinkInterfaceFactory();
  ysos::GetModuleLinkInterfaceManager();
}

void SysInit::InitPlugin(void) {
#ifdef _WIN32  
  std::string plugin_lib = GetPackageConfigImpl()->GetConfPath() + "..\\plugins\\";
#else
  std::string plugin_lib = GetPackageConfigImpl()->GetConfPath() + "../plugins/";
#endif
  GetPluginManager()->Initialize(&plugin_lib);
}

void SysInit::UnInitPlugin(void) {
  GetPluginManager()->UnInitialize();
}

void SysInit::UninitGlobalFactoryAndManager() {
  ysos::ReleaseModuleLinkInterfaceManager();
  //ysos::ReleaseModuleLinkInterfaceFactory();

  ysos::ReleaseModuleInterfaceManager();
  //ysos::ReleaseModuleInterfaceFactory();

  ysos::ReleaseCallbackInterfaceManager();
  //ysos::ReleaseCallbackInterfaceFactory();

  ysos::ReleaseDriverInterfaceManager();
  //ysos::ReleaseDriverInterfaceFactory();

  ysos::ReleaseProtocolInterfaceManager();
  //ysos::ReleaseProtocolInterfaceFactory();

  ysos::ReleaseStrategyInterfaceManager();
  //ysos::ReleaseStrategyInterfaceFactory();

  ysos::ReleaseAgentInterfaceManager();
  //ysos::ReleaseAgentInterfaceFactory();

  //ysos::ReleaseBaseInterfaceFactory();
}

static const std::string s_plugin_get_name = "Get_YSOS_Plugin_Name";
static const std::string s_plugin_get_interface_name = "Get_YSOS_Plugin_Interface_Name";
static const std::string s_plugin_get_plugin_interface = "GET_YSOS_Plugin_Interface";

SysInit::PluginInfoInterfacePtr SysInit::GetPluginInfo(YSOS_PLUGIN_TYPE plugin_handle) {
  PluginInfoInterfacePtr plugin_ptr = PluginInfoInterfacePtr(new PluginInfoInterface());
  if (NULL == plugin_ptr || NULL == plugin_handle) {
    return NULL;
  }

  GET_PLUGIN_NAME plugin_fun = (GET_PLUGIN_NAME)(GetPluginManager()->LookupPlugin(plugin_handle, s_plugin_get_name));
  if (NULL == plugin_fun) {
    return NULL;
  }
  char *p = plugin_fun();
  if (NULL == p) {
    return NULL;
  }
  plugin_ptr->plugin_name = p;

  plugin_fun = (GET_PLUGIN_NAME)(GetPluginManager()->LookupPlugin(plugin_handle, s_plugin_get_interface_name));
  if (NULL == plugin_fun) {
    return NULL;
  }
  p = plugin_fun();
  if (NULL == p) {
    return NULL;
  }
  std::string interface_name = p;
  if (interface_name == "ModuleInterface") {
    plugin_ptr->plugin_type = PROP_MODULE;
  } else if ("CallbackInterface" == interface_name) {
    plugin_ptr->plugin_type = PROP_CALLBACK;
  } else if ("DriverInterface" == interface_name) {
    plugin_ptr->plugin_type = PROP_DRIVER;
  } else if ("ProtocolInterface" == interface_name) {
    plugin_ptr->plugin_type = PROP_PROTOCOL;
  } else if ("ModuleLinkInterface" == interface_name) {
    plugin_ptr->plugin_type = PROP_MODULE_LINK;
  } else if ("StrategyInterface" == interface_name) {
    plugin_ptr->plugin_type = PROP_STRATEGY;
  } else if ("AgentInterface" == interface_name) {
    plugin_ptr->plugin_type = PROP_AGENT;
  } else if ("ConnectionInterface" == interface_name) {
    plugin_ptr->plugin_type = PROP_CONNECTION;
  } else {
    return NULL;
  }
  plugin_ptr->plugin_create_function = GetPluginManager()->LookupPlugin(plugin_handle, s_plugin_get_plugin_interface);

  return NULL==plugin_ptr->plugin_create_function ? NULL : plugin_ptr;
}

int SysInit::RegisterPlugins(void) {
  PluginManager::PluginMapIterator it = GetPluginManager()->PluginHead();
  for (; it!=GetPluginManager()->PluginEnd(); ++it) {
    PluginInfoInterfacePtr plugin_ptr = GetPluginInfo(it->second);
    if (NULL == plugin_ptr) {
      YSOS_LOG_ERROR("Register plugin failed: " << it->first);
      continue;
    }

    if (plugin_ptr->plugin_type == PROP_MODULE) {
      DynamicRegisterModuleClass(plugin_ptr->plugin_name, plugin_ptr->plugin_create_function);
    } else if (plugin_ptr->plugin_type == PROP_CALLBACK) {
      DynamicRegisterCallbackClass(plugin_ptr->plugin_name, plugin_ptr->plugin_create_function);
    } else if (plugin_ptr->plugin_type == PROP_DRIVER) {
      DynamicRegisterDriverClass(plugin_ptr->plugin_name, plugin_ptr->plugin_create_function);
    } else if (plugin_ptr->plugin_type == PROP_PROTOCOL) {
      DynamicRegisterProtocolClass(plugin_ptr->plugin_name, plugin_ptr->plugin_create_function);
    } else if (plugin_ptr->plugin_type == PROP_MODULE_LINK) {
      DynamicRegisterModuleLinkClass(plugin_ptr->plugin_name, plugin_ptr->plugin_create_function);
    } else if (plugin_ptr->plugin_type == PROP_STRATEGY) {
      DynamicRegisterStrategyClass(plugin_ptr->plugin_name, plugin_ptr->plugin_create_function);
    } else if (plugin_ptr->plugin_type == PROP_AGENT) {
      DynamicRegisteAgentClass(plugin_ptr->plugin_name, plugin_ptr->plugin_create_function);
    } else if (plugin_ptr->plugin_type == PROP_CONNECTION) {
      DynamicRegisteConnectionClass(plugin_ptr->plugin_name, plugin_ptr->plugin_create_function);
    } else {
      YSOS_LOG_ERROR("No support type: [" << plugin_ptr->plugin_name << ":" << plugin_ptr->plugin_type << "]");
    }

    YSOS_LOG_DEBUG("Register Plugin [" << plugin_ptr->plugin_name << ":" << plugin_ptr->plugin_type << "] success");
  }

  return YSOS_ERROR_SUCCESS;
}

int SysInit::RegisterClasses() {
  /// Agent
  RegisteAgentClass(BaseAgentImpl);

  /// Strategy
  RegisterStrategyClass(BaseStrategyImpl);
  RegisterStrategyClass(BaseCommonStrategyImpl)
  RegisterStrategyClass(YsosDaemonStrategyImpl)

  /// ModuleLink
  RegisterModuleLinkClass(ModuleLinkImpl);
  /// Repeater //  NOLINT
  RegisterModuleClass(BaseRepeaterModuleImpl);
  /// Callback //  NOLINT
  RegisterCallbackClass(BaseRepeaterCallbackImpl);
  RegisterCallbackClass(StateMachineCommonCallback);
  RegisterCallbackClass(BaseTransitionCallbackImpl);
  RegisterCallbackClass(StatusEventModuleCallback);
  RegisterCallbackClass(BaseStrategyProxyCallbackImpl);
  /// Connection        // NOLINT
  RegisterConnectionClass(BaseConnectionImpl);
  /// RPCCallback
  RegisterRPCCallbackClass(YsosDaemonConnectionCalllback);

  RegisterPlugins();

  return YSOS_ERROR_SUCCESS;
}

static void RegisterInstance(InstanceInfoPtr instance_info_ptr) {
  if (NULL == instance_info_ptr) {
    return;
  }

  if (instance_info_ptr->type.empty() || instance_info_ptr->class_name.empty() || instance_info_ptr->logic_name.empty()) {
    return;
  }

  if ("module" == instance_info_ptr->type) {
    Register_Module_Object(instance_info_ptr->logic_name, instance_info_ptr->class_name);
  } else if ("module_link" == instance_info_ptr->type) {
    Register_Module_Link_Object(instance_info_ptr->logic_name, instance_info_ptr->class_name);
  } else if ("driver" == instance_info_ptr->type) {
    Register_Driver_Object(instance_info_ptr->logic_name, instance_info_ptr->class_name);
  } else if ("callback" == instance_info_ptr->type) {
    Register_Callback_Object(instance_info_ptr->logic_name, instance_info_ptr->class_name);
  } else if ("repeater" == instance_info_ptr->type) {
    Register_Repeater_Object(instance_info_ptr->logic_name, instance_info_ptr->class_name);
  } else if ("agent" == instance_info_ptr->type) {
    Register_Agent_Object(instance_info_ptr->logic_name, instance_info_ptr->class_name);
  } else if ("protocol" == instance_info_ptr->type) {
    Register_Protocol_Object(instance_info_ptr->logic_name, instance_info_ptr->class_name);
  } else if ("strategy" == instance_info_ptr->type) {
    Register_Strategy_Object(instance_info_ptr->logic_name, instance_info_ptr->class_name);
  } else if ("connection" == instance_info_ptr->type) {
    Register_Connection_Object(instance_info_ptr->logic_name, instance_info_ptr->class_name);
  } else if ("connectioncallback" == instance_info_ptr->type) {
    Register_Connection_Callback_Object(instance_info_ptr->logic_name, instance_info_ptr->class_name);
  } else if ("transport" == instance_info_ptr->type) {
    Register_Transport_Object(instance_info_ptr->logic_name, instance_info_ptr->class_name);
  } else if ("transportcallback" == instance_info_ptr->type) {
    Register_Transport_Callback_Object(instance_info_ptr->logic_name, instance_info_ptr->class_name);
  } else if ("rpc" == instance_info_ptr->type) {
    Register_RPC_Object(instance_info_ptr->logic_name, instance_info_ptr->class_name);
  } else if ("rpccallback" == instance_info_ptr->type) {
    Register_RPC_Callback_Object(instance_info_ptr->logic_name, instance_info_ptr->class_name);
  } else {
    YSOS_LOG_ERROR_DEFAULT("Register Instance Failed: " << instance_info_ptr->type << ": " << instance_info_ptr->logic_name << ": " << instance_info_ptr->class_name);
  }
  YSOS_LOG_ERROR_DEFAULT("Register Instance Success: " << instance_info_ptr->type << ": " << instance_info_ptr->logic_name << ": " << instance_info_ptr->class_name);
}

int SysInit::RegisterObjects() {
  /// builtin objects //  NOLINT
  Register_Callback_Object("StateMachineCommonCallback", "StateMachineCommonCallback");
  Register_Callback_Object("StatusEventModuleCallback", "StatusEventModuleCallback");

  InstanceInfoListIterator it = GetPackageConfigImpl()->InstanceInfoHead();
  for (; it != GetPackageConfigImpl()->InstanceInfoEnd(); ++it) {
    RegisterInstance(*it);
  }

  return YSOS_ERROR_SUCCESS;
}

static void CreateInstance(InstanceInfoPtr instance_info_ptr) {
  if (NULL == instance_info_ptr) {
    return;
  }

  if (instance_info_ptr->type.empty() || instance_info_ptr->class_name.empty() || instance_info_ptr->logic_name.empty()) {
    return;
  }

  if ("module" == instance_info_ptr->type) {
    ModuleInterfacePtr module_ptr = GetModuleInterfaceManager()->FindInterface(instance_info_ptr->logic_name, true);
  } else if ("module_link" == instance_info_ptr->type) {
    ModuleLinkInterfacePtr module_link_ptr =GetModuleLinkInterfaceManager()->FindInterface(instance_info_ptr->logic_name, true);
  } else if ("driver" == instance_info_ptr->type) {
    DriverInterfacePtr driver_ptr = GetDriverInterfaceManager()->FindInterface(instance_info_ptr->logic_name, true);
  } else if ("callback" == instance_info_ptr->type) {
    CallbackInterfacePtr callback_ptr = GetCallbackInterfaceManager()->FindInterface(instance_info_ptr->logic_name, true);
  } else if ("repeater" == instance_info_ptr->type) {
    ModuleInterfacePtr module_ptr = GetModuleInterfaceManager()->FindInterface(instance_info_ptr->logic_name, true);
  } else if ("agent" == instance_info_ptr->type) {
    AgentInterfacePtr agent_ptr = GetAgentInterfaceManager()->FindInterface(instance_info_ptr->logic_name, true);
  } else if ("protocol" == instance_info_ptr->type) {
    ProtocolInterfacePtr protocol_ptr = GetProtocolInterfaceManager()->FindInterface(instance_info_ptr->logic_name, true);
  } else if ("strategy" == instance_info_ptr->type) {
    StrategyInterfacePtr strategy_ptr = GetStrategyInterfaceManager()->FindInterface(instance_info_ptr->logic_name, true);
  } else if ("connection" == instance_info_ptr->type) {
    ConnectionInterfacePtr connection_ptr = GetConnectionInterfaceManager()->FindInterface(instance_info_ptr->logic_name, true);
  } else {
    YSOS_LOG_ERROR_DEFAULT("Create Instance Failed: " << instance_info_ptr->type << ": " << instance_info_ptr->logic_name << ": " << instance_info_ptr->class_name);
  }
  YSOS_LOG_ERROR_DEFAULT("Create Instance Success: " << instance_info_ptr->type << ": " << instance_info_ptr->logic_name << ": " << instance_info_ptr->class_name);
}

int SysInit::CreateObjects() {
  if (YSOS_ERROR_SUCCESS != CreateModuleObjects()) {
    return YSOS_ERROR_FAILED;
  }

  if (YSOS_ERROR_SUCCESS != CreateModuleLinkObjects()) {
    return YSOS_ERROR_FAILED;
  }

  /// builtin objects //  NOLINT
  CallbackInterfacePtr callback_ptr = GetCallbackInterfaceManager()->FindInterface("StateMachineCommonCallback", true);
  callback_ptr = GetCallbackInterfaceManager()->FindInterface("StatusEventModuleCallback", true);

  InstanceInfoListIterator it = GetPackageConfigImpl()->InstanceInfoHead();
  for (; it != GetPackageConfigImpl()->InstanceInfoEnd(); ++it) {
    CreateInstance(*it);
  }

  return YSOS_ERROR_SUCCESS;
}

std::string SysInit::GetElementName(std::string& element_name) {
  std::list<std::string> temp_list;
  GetUtility()->SplitString(element_name, "_", temp_list);

  return *(temp_list.begin());
}

int SysInit::AddCallback2Module(const std::string& module_name, const ModuleInterfacePtr module_interface_ptr) {
  if (NULL == module_interface_ptr || true == module_name.empty()) {
    return YSOS_ERROR_FAILED;
  }

  std::string callback_owner;
  INT64 owner_id = 0;
  //  Seek Callback and Add Callback to Module
  PackageConfigImpl::CallbackConfInfoIterator callback_info_iterator = package_config_impl_ptr_->CallbackConfInfoHead();
  for (; callback_info_iterator != package_config_impl_ptr_->CallbackConfInfoEnd(); ++callback_info_iterator) {
    /// 合法性检查
    if ((*callback_info_iterator)->owner.empty()) {
      continue;
    }
    callback_owner = (*callback_info_iterator)->owner;
    if (callback_owner.empty()) {
      continue;
    }
    if (module_name != callback_owner) {
      continue;
    }
    /// 找到匹配的Callback
    std::string callback_instance_name = (*callback_info_iterator)->name;
    CallbackInterfacePtr the_callback  = GetCallbackInterfaceManager()->FindInterface(callback_instance_name);
    if (NULL == the_callback) {
      YSOS_LOG_ERROR("Find callback: " << callback_instance_name << " failed");
      continue;
    }
    module_interface_ptr->AddCallback(the_callback, owner_id, 0);
  }  //  end for

  return YSOS_ERROR_SUCCESS;
}

//  扫描ModuleLink, 将其中引用了的Module创建出来, 并添加Callback.//NOLINT
int SysInit::CreateModuleObjects() {
  if (NULL == package_config_impl_ptr_) {
    return YSOS_ERROR_FAILED;
  }

  std::string module_link_name;
  //  Seek Module Link, Module and Register Module Link, Module Object
  PackageConfigImpl::ModuleLinkConfInfoIterator module_link_info_iterator = package_config_impl_ptr_->ModuleLinkConfInfoHead();
  for (; module_link_info_iterator != package_config_impl_ptr_->ModuleLinkConfInfoEnd(); ++module_link_info_iterator) {
    if ((*module_link_info_iterator)->name.empty()) {
      continue;
    }

    YSOS_LOG_DEBUG("module link name: " << (*module_link_info_iterator)->name);
    //  Get Module Link Name
    module_link_name = (*module_link_info_iterator)->name;
    if (module_link_name.empty()) {
      continue;
    }
    if (0 == (*module_link_info_iterator)->module_list.size()) {
      continue;
    }

    ModuleInterfacePtr module_interface_ptr = NULL;
    std::string module_instance_name;
    //  Seek Module Info
    PackageConfigImpl::ModuleLinkModuleConfInfoIterator module_info_iterator = (*module_link_info_iterator)->module_list.begin();
    for (; module_info_iterator != (*module_link_info_iterator)->module_list.end(); ++module_info_iterator) {
      if ((*module_info_iterator)->instance_name.empty()) {
        continue;
      }
      module_instance_name = (*module_info_iterator)->instance_name;
      //+ "@" + module_link_name;
      module_interface_ptr = ysos::GetModuleInterfaceManager()->FindInterface(module_instance_name);
      if (NULL == module_interface_ptr) {
        YSOS_LOG_ERROR("Find Module: " << module_instance_name << " failed");
        continue;
      } else {
        module_interface_ptr->SetProperty(PROP_MODULE_LINK, &module_link_name);
        YSOS_LOG_ERROR("ModuleLink: " << module_link_name << "  Module: " << module_instance_name << "  ModuleName: " << module_interface_ptr->GetName());
      }

      // Add Callback
      if (YSOS_ERROR_SUCCESS != AddCallback2Module(module_instance_name, module_interface_ptr)) {
        continue;
      }
    }  // end for
  }  // end for

  return YSOS_ERROR_SUCCESS;
}

int SysInit::CreateModuleLinkObjects() {
  if (NULL == package_config_impl_ptr_) {
    return YSOS_ERROR_FAILED;
  }

  std::string module_link_name;
  //////////////////////////////////////////////////////////////////////////////
  PackageConfigImpl::ModuleLinkConfInfoIterator module_link_info_iterator = package_config_impl_ptr_->ModuleLinkConfInfoHead();
  for (; module_link_info_iterator!=package_config_impl_ptr_->ModuleLinkConfInfoEnd(); ++module_link_info_iterator) {
    if ((*module_link_info_iterator)->name.empty()) {
      continue;
    }
    YSOS_LOG_DEBUG("module link name: " << (*module_link_info_iterator)->name);

    //  Get Module Link Name
    module_link_name = (*module_link_info_iterator)->name;
    //YSOS_LOG_DEBUG("CreateModuleLinkObjects::module_link_name-: " << module_link_name);//add for linux
    if (module_link_name.empty()) {
      continue;
    }
    //YSOS_LOG_DEBUG("CreateModuleLinkObjects::GetModuleLinkInterfaceManager()->FindInterface:[Enter]"); //add for linux
    ysos::ModuleLinkInterfacePtr module_link_interface_ptr = GetModuleLinkInterfaceManager()->FindInterface(module_link_name);
    GetModuleLinkInterfaceManager()->Dump();
    if (NULL == module_link_interface_ptr) {
      continue;
    }

    if (YSOS_ERROR_SUCCESS != module_link_interface_ptr->SetProperty(
          PROP_MODULE_LINK_INFO_AUTORUN, &(*module_link_info_iterator)->autorun)) {
      continue;
    }

    if (0 == (*module_link_info_iterator)->module_list.size()) {
      continue;
    }

    ysos::ModuleInterfacePtr module_interface_ptr = NULL;
    std::string module_instance_name;

    //  Seek Module Info
    PackageConfigImpl::ModuleLinkModuleConfInfoIterator module_info_iterator = (*module_link_info_iterator)->module_list.begin();
    for (; module_info_iterator != (*module_link_info_iterator)->module_list.end(); ++module_info_iterator) {
      //  Get Module Instance Name
      if ((*module_info_iterator)->instance_name .empty()) {
        continue;
      }

      module_instance_name = (*module_info_iterator)->instance_name;
      if (module_instance_name.empty()) {
        continue;
      }

      YSOS_LOG_DEBUG("module instance name: " << module_instance_name);
      module_interface_ptr = ysos::GetModuleInterfaceManager()->FindInterface(module_instance_name);
      //  Add Module
      if (NULL == module_interface_ptr) {
        continue;
      }
      uint32_t level_in_link = (*module_info_iterator)->instance_level * 0x10001;
      module_interface_ptr->SetProperty(PROP_MODULE_LINK_LEVEL, &level_in_link);
      if (NULL != module_interface_ptr) {
        module_link_interface_ptr->AddModule(module_interface_ptr);
      }
    }  // end for

    /// 设置Link的深度
    // uint32_t depth = (*module_link_info_iterator)->GetDepth();
    // module_link_interface_ptr->SetProperty(PROP_MODULE_LINK_INFO_DEPTH, &depth);

  }  // end for

  //////////////////////////////////////////////////////////////////////////////
  /*PackageConfigImpl::ModuleLinkConfInfoIterator */module_link_info_iterator = package_config_impl_ptr_->ModuleLinkConfInfoHead();
  for (; module_link_info_iterator != package_config_impl_ptr_->ModuleLinkConfInfoEnd(); ++module_link_info_iterator) {
    if ((*module_link_info_iterator)->name.empty()) {
      continue;
    }
    YSOS_LOG_DEBUG("module link name: " << (*module_link_info_iterator)->name);

    //  Get Module Link Name
    module_link_name = (*module_link_info_iterator)->name;
    if (module_link_name.empty()) {
      continue;
    }

    ModuleLinkInterfacePtr module_link_interface_ptr  = GetModuleLinkInterfaceManager()->FindInterface(module_link_name);
    if (NULL == module_link_interface_ptr) {
      continue;
    }

    if (0 == (*module_link_info_iterator)->module_list.size()) {
      continue;
    }
    ysos::ModuleInterfacePtr current_module_interface_ptr = NULL;
    ysos::ModuleInterfacePtr next_module_interface_ptr = NULL;
    std::string current_module_instance_name;
    std::string next_module_instance_name;

    //  Seek Module Info
    PackageConfigImpl::ModuleLinkModuleConfInfoIterator module_info_iterator = (*module_link_info_iterator)->module_list.begin();
    for (; module_info_iterator != (*module_link_info_iterator)->module_list.end(); ++module_info_iterator) {
      //  Get Module Instance Name
      if ((*module_info_iterator)->instance_name .empty()) {
        continue;
      }
      current_module_instance_name = (*module_info_iterator)->instance_name;
      YSOS_LOG_DEBUG("current module instance name: " << current_module_instance_name);
      current_module_interface_ptr = GetModuleInterfaceManager()->FindInterface(current_module_instance_name);
      if (NULL == current_module_interface_ptr) {
        continue;
      }

      //  Source or Destination directly add
      if (0 == (*module_info_iterator)->next_modules.size()) {
        continue;
      }
      PackageConfigImpl::ModuleLinkModuleConfInfoNextModulesIterator iterator = (*module_info_iterator)->next_modules.begin();
      for (; iterator != (*module_info_iterator)->next_modules.end(); ++iterator) {
        next_module_interface_ptr = FindNextModule(*iterator, module_link_name);

        //  Add Edge
        if (NULL != next_module_interface_ptr) {
          module_link_interface_ptr->AddModule(next_module_interface_ptr, current_module_interface_ptr);
        }
      }
    }  // end for
  }  // end for

  //  deleted by XuLanyue for replicate
  // CreateInherentRepeaterObjects();

  return YSOS_ERROR_SUCCESS;
}

/// 扫描ModuleLink配置信息, 将其中内置的Repeater创建出来,//NOLINT
/// 添加Callback, 并链到Link中.//NOLINT
int SysInit::CreateInherentRepeaterObjects() {
  int return_value = YSOS_ERROR_INVALID_ARGUMENTS;
  do {
    PackageConfigImpl::ModuleLinkConfInfoIterator module_link_info_iterator = package_config_impl_ptr_->ModuleLinkConfInfoHead();
    for (; module_link_info_iterator != package_config_impl_ptr_->ModuleLinkConfInfoEnd(); ++module_link_info_iterator) {
      if ((*module_link_info_iterator)->name.empty()) {
        continue;
      }

      // GetLinkName&LinkPtr
      std::string module_link_name = (*module_link_info_iterator)->name;
      ysos::ModuleLinkInterfacePtr module_link_interface_ptr = ysos::GetModuleLinkInterfaceManager()->FindInterface(module_link_name);
      if (NULL == module_link_interface_ptr) {
        YSOS_LOG_DEBUG("The link " << module_link_name << " is invalid.");
        continue;
      }

      YSOS_LOG_DEBUG("Create repeater(s) in link " << module_link_name);
      if ((*module_link_info_iterator)->repeater_info_list.size() == 0) {
        YSOS_LOG_DEBUG("no repeater in this link");
        continue;
      }

      // 创建Repeater并链入.//NOLINT
      module_link_interface_ptr->Ioctl(CMD_MODULE_LINK_ADD_INHERENT_REPEATER, &(*module_link_info_iterator)->repeater_info_list);
    }
    return_value = YSOS_ERROR_SUCCESS;
  } while (false);
  return return_value;
}

boost::shared_ptr<ModuleInterface> SysInit::FindNextModule(std::string& next_module_id, std::string& module_link_name) {
  if (true == next_module_id.empty() || true == module_link_name.empty()) {
    return NULL;
  }

  std::string found_module_link_name;
  ysos::ModuleInterfacePtr found_module_interface_ptr = NULL;
  std::string found_module_instance_name;

  PackageConfigImpl::ModuleLinkConfInfoIterator module_link_info_iterator = package_config_impl_ptr_->ModuleLinkConfInfoHead();
  for (; module_link_info_iterator != package_config_impl_ptr_->ModuleLinkConfInfoEnd(); ++module_link_info_iterator) {
    if ((*module_link_info_iterator)->name.empty()) {
      continue;
    }

    YSOS_LOG_DEBUG("module link name: " << (*module_link_info_iterator)->name);

    //  Get Module Link Name
    //  found_module_link_name
    //    = GetElementName((*module_link_info_iterator)->name);
    found_module_link_name = (*module_link_info_iterator)->name;
    if (module_link_name.empty()) {
      continue;
    }
    if (module_link_name != found_module_link_name) {
      continue;
    }
    ModuleLinkInterfacePtr module_link_interface_ptr = GetModuleLinkInterfaceManager()->FindInterface(module_link_name);
    if (NULL == module_link_interface_ptr) {
      continue;
    }
    if (0 == (*module_link_info_iterator)->module_list.size()) {
      continue;
    }

    //  Seek Module Info
    PackageConfigImpl::ModuleLinkModuleConfInfoIterator module_info_iterator = (*module_link_info_iterator)->module_list.begin();
    for (; module_info_iterator != (*module_link_info_iterator)->module_list.end(); ++module_info_iterator) {
      //  Get Module Instance Name
      if ((*module_info_iterator)->instance_name .empty()) {
        continue;
      }
      if (std::stoi(next_module_id) != (*module_info_iterator)->instance_id) {
        continue;
      }

      //  found_module_instance_name
      //    = GetElementName((*module_info_iterator)->instance_name);
      found_module_instance_name = (*module_info_iterator)->instance_name;
      if (found_module_instance_name.empty()) {
        continue;
      }
      found_module_instance_name = found_module_instance_name;
      YSOS_LOG_DEBUG("found module instance name: " << found_module_instance_name);
      found_module_interface_ptr = GetModuleInterfaceManager()->FindInterface(found_module_instance_name);
      return found_module_interface_ptr;
    }  // end for
    //   }
    //  }
  }  // end for

  return NULL;
}
}   //  end of namespace
