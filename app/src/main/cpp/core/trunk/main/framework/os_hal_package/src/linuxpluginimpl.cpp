/**
 *@file LinuxPluginImpl.cpp
 *@brief Definition of LinuxPluginImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// ysos private headers //  NOLINT
#include "../../../protect/include/os_hal_package/linuxpluginimpl.h"

#ifdef _WIN32
  //#
#else
  #include <stdlib.h>
  #include <dlfcn.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <fcntl.h>
  #include <unistd.h> //work direc
#endif

namespace ysos {

LinuxPluginImpl::LinuxPluginImpl(const std::string &name): BaseInterfaceImpl(name) {  // NOLINT
}

LinuxPluginImpl::~LinuxPluginImpl() {
}

int LinuxPluginImpl::Initialize(void *param) {
  return YSOS_ERROR_SUCCESS;
}

int LinuxPluginImpl::UnInitialize(void *param) {
  return YSOS_ERROR_SUCCESS;
}

YSOS_PLUGIN_TYPE LinuxPluginImpl::LoadPlugin(const std::string &plugin_name) {
  //TODO:add for linux
   if (plugin_name.empty()) {
    return NULL;
  }

  YSOS_PLUGIN_TYPE plugin_handle = (YSOS_PLUGIN_TYPE)dlopen(plugin_name.c_str(), RTLD_LAZY);
  if (NULL == plugin_handle) {
    printf("LinuxPluginImpl::UnloadPlugin::dlopen[loadlibrary]-->dlerror(): [%s]\n", dlerror());
    YSOS_LOG_ERROR("LinuxPluginImpl::UnloadPlugin::dlopen[loadlibrary]-->dlerror() failed: " << plugin_name << ":" << dlerror());
  }
  return plugin_handle;
}

void* LinuxPluginImpl::LookupPlugin(const YSOS_PLUGIN_TYPE plugin_handle, const std::string &function_name) {
  //TODO: add for linux
  return (void *)dlsym(plugin_handle, function_name.c_str());
}

int LinuxPluginImpl::UnloadPlugin(YSOS_PLUGIN_TYPE plugin_handle) {
  //TODO: add for linux
  int ret = dlclose(plugin_handle);
  printf("LinuxPluginImpl::UnloadPlugin::dlclose-->dlerror(): [%s]\n", dlerror());
  YSOS_LOG_ERROR("LinuxPluginImpl::UnloadPlugin::dlclose-->dlerror(): " << plugin_handle << ":" << dlerror());
  return ret == 0 ? YSOS_ERROR_SUCCESS : YSOS_ERROR_FAILED;
}
}