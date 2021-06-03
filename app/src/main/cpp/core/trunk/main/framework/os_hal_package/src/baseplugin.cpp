/**
 *@file BasePlugin.cpp
 *@brief Definition of BasePlugin
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// ysos private headers //  NOLINT
#include "../../../protect/include/os_hal_package/baseplugin.h"
#ifdef _WIN32
#include "../../../protect/include/os_hal_package/windowspluginimpl.h"
#else
#include "../../../protect/include/os_hal_package/linuxpluginimpl.h"
#endif

namespace ysos {

BasePlugin::BasePlugin(const std::string &name): BaseInterfaceImpl(name) {  // NOLINT
}

BasePlugin::~BasePlugin() {
}

int BasePlugin::Initialize(void *param) {
  PluginInterface *plugin_ptr = NULL;
#ifdef _WIN32
  plugin_ptr = WindowsPluginImpl::CreateInstance();
  if(NULL == plugin_ptr) {
    return YSOS_ERROR_MEMORY_ALLOCATED_FAILED;
  }
#else
  //TODO: add for linux
  plugin_ptr = LinuxPluginImpl::CreateInstance();
  if(NULL == plugin_ptr) {
    return YSOS_ERROR_MEMORY_ALLOCATED_FAILED;
  }
#endif

  plugin_impl_ = PluginInterfacePtr(plugin_ptr);
  if(NULL == plugin_impl_) {
    return YSOS_ERROR_MEMORY_ALLOCATED_FAILED;
  }

  return YSOS_ERROR_SUCCESS;
}

int BasePlugin::UnInitialize(void *param) {
  plugin_impl_ = NULL;

  return YSOS_ERROR_SUCCESS;
}

YSOS_PLUGIN_TYPE BasePlugin::LoadPlugin(const std::string &plugin_name) {
  return plugin_impl_->LoadPlugin(plugin_name);
}

void* BasePlugin::LookupPlugin(const YSOS_PLUGIN_TYPE plugin_handle, const std::string &function_name) {
  return plugin_impl_->LookupPlugin(plugin_handle, function_name);
}

int BasePlugin::UnloadPlugin(YSOS_PLUGIN_TYPE plugin_handle) {
  return plugin_impl_->UnloadPlugin(plugin_handle);
}
}