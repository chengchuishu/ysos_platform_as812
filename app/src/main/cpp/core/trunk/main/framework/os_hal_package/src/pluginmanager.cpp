/**
 *@file PluginManager.cpp
 *@brief Definition of PluginManager
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// ysos private headers //  NOLINT
#include "../../../protect/include/os_hal_package/pluginmanager.h"
#include "../../../public/include/os_hal_package/filepath.h"
#include "../../../protect/include/os_hal_package/baseplugin.h"
#include "../../../public/include/core_help_package/utility.h"
#include <vector>

namespace ysos {
DEFINE_SINGLETON(PluginManager);
PluginManager::PluginManager(const std::string &name): BaseInterfaceImpl(name) {  // NOLINT
}

PluginManager::~PluginManager() {
}

int PluginManager::Initialize(void *param) {
  BasePlugin *plugin = BasePlugin::CreateInstance();
  if(NULL == plugin) {
    return YSOS_ERROR_MEMORY_ALLOCATED_FAILED;
  }
  int ret = plugin->Initialize();
  if(YSOS_ERROR_SUCCESS != ret) {
    plugin->UnInitialize();
    return ret;
  }
  plugin_ = PluginInterfacePtr(plugin);

  std::string *plugin_path = reinterpret_cast<std::string*>(param);
  if(NULL == plugin_path) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }
  std::vector<std::string> file_array = FilePath::GetAllFiles(*plugin_path);
  std::vector<std::string>::iterator it = file_array.begin();
  for(; it!=file_array.end(); ++it){
    LoadPlugin(*it);
  }

  return YSOS_ERROR_SUCCESS;
}

int PluginManager::UnInitialize(void *param) {
  UnloadPlugin();
  plugin_map_.clear();
  plugin_ = NULL;

  return YSOS_ERROR_SUCCESS;
}

int PluginManager::LoadPlugin(const std::string &plugin_name) {
  YSOS_PLUGIN_TYPE module_ptr = plugin_->LoadPlugin(plugin_name);
  if(NULL == module_ptr) {
    YSOS_LOG_ERROR("Load plugin failed: " << plugin_name);
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  if(plugin_map_.end() != plugin_map_.find(plugin_name)) {
    YSOS_LOG_ERROR("plugin already exist: " << plugin_name);
    return YSOS_ERROR_HAS_EXISTED;
  }
#ifdef _WIN32
  std::string key = plugin_name.substr(plugin_name.find_last_of("\\")+1);
#else
  std::string key = plugin_name.substr(plugin_name.find_last_of("/")+1);  //add for linux
#endif
  plugin_map_.insert(std::make_pair(key, module_ptr));
  return YSOS_ERROR_SUCCESS;
}

void* PluginManager::LookupPlugin(const std::string &plugin_name, const std::string &default_plugin_name) {
  YSOS_PLUGIN_TYPE module_ptr = NULL;
  std::string ysos_plugin_name = GetUtility()->GetPluginName(default_plugin_name.empty()?plugin_name:default_plugin_name);
  PluginMap::iterator it=plugin_map_.find(ysos_plugin_name);
  if(it == plugin_map_.end()) {
     return NULL;
    //it = plugin_map_.begin();
  }
  module_ptr = it->second;

  std::string function_name = std::string("Register_") + plugin_name + "_Class";
  return plugin_->LookupPlugin(module_ptr, function_name);
}

void* PluginManager::LookupPlugin(const YSOS_PLUGIN_TYPE plugin_handle, const std::string &function_name) {
  if(NULL == plugin_handle) {
    return NULL;
  }

  return plugin_->LookupPlugin(plugin_handle, function_name);
}

void PluginManager::UnloadPlugin(void) {
  for(PluginMap::iterator it=plugin_map_.begin(); it!=plugin_map_.end(); ++it){
    plugin_->UnloadPlugin(it->second);
  }
}

PluginManager::PluginMapIterator PluginManager::PluginHead(void) {
  return plugin_map_.begin();
}

PluginManager::PluginMapIterator PluginManager::PluginEnd(void) {
  return plugin_map_.end();
}

PluginManager::PluginMapIterator PluginManager::FindPlugin(const std::string &plugin_name) {
  return plugin_map_.find(plugin_name);
}
}