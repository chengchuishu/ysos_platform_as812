/**
 *@file BaseStrategyServiceCallbackImpl.cpp
 *@brief ״̬��ͨ��Callback����,����StateMachine���¼���Service�ص�
 *@version 1.0
 *@author dhongqian
 *@date Created on: 2016-05-11 10:06:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// Self Header //  NOLINT
#include "../../../protect/include/sys_framework_package/basestrategyservicecallbackimpl.h"
/// Boost Headers
#include <boost/make_shared.hpp>
/// private headers
#include "../../../public/include/sys_interface_package/ysos_enum.h"
#include "../../../public/include/core_help_package/utility.h"
#include "../../../protect/include/sys_framework_package/manager_declare.h"
#include "../../../protect/include/sys_framework_package/modulelinkimpl.h"
#include "../../../protect/include/sys_framework_package/platformservice.h"

namespace ysos {

//DECLARE_PLUGIN_REGISTER_INTERFACE(StrategyCallback, CallbackInterface);
BaseStrategyServiceCallbackImpl::BaseStrategyServiceCallbackImpl(const std::string &strClassName):BaseCallbackImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.framework.strategy");
}

BaseStrategyServiceCallbackImpl::~BaseStrategyServiceCallbackImpl() {

}

int BaseStrategyServiceCallbackImpl::UnInitialize(void *param) {
  if (!IsInitialized()) {
    return YSOS_ERROR_SUCCESS;
  }

  repeater_map_.clear();

  return BaseCallbackImpl::UnInitialize(param);
}

BaseStrategyServiceCallbackImpl::RepeaterInfoPtr BaseStrategyServiceCallbackImpl::GetRepeaterByServiceName(const std::string &service_name) {
  RepeaterMap::iterator it = repeater_map_.find(service_name);
  if (repeater_map_.end() != it) {
    return it->second;
  }

  PlatformService::ServiceInfoPtr service_info = GetPlatformService()->GetService(service_name);
  if (NULL == service_info) {
    YSOS_LOG_DEBUG("Get service failed: [" << service_name << "] ");
    return NULL;
  }

  YSOS_LOG_DEBUG("begin to find module interface: " << service_info->name);
  ModuleInterfacePtr module_ptr = GetModuleInterfaceManager()->FindInterface(service_info->name);
  if (NULL == module_ptr) {
    YSOS_LOG_DEBUG("Find module failed: " << service_name);
    return NULL;
  }
  YSOS_LOG_DEBUG("end to find module interface: " << service_info->name);

  std::string module_link_name;
  int ret = module_ptr->GetProperty(PROP_MODULE_LINK, &module_link_name);

  YSOS_LOG_DEBUG("begin to find module link interface: " << module_link_name);
  ModuleLinkInterfacePtr module_link_ptr = GetModuleLinkInterfaceManager()->FindInterface(module_link_name);
  if (NULL == module_link_ptr) {
    YSOS_LOG_DEBUG("Get module link failed: " << module_link_name);
    return NULL;
  }
  YSOS_LOG_DEBUG("end to find module link interface: " << module_link_name);

  // get repeater
  RepeaterIncludeModule  repeater_include_module;
  /// not support composit and select type service yet
  std::string real_name = GetPlatformService()->GetServiceRealName(service_name);
  repeater_include_module.module_name_ = real_name;
  if (YSOS_ERROR_SUCCESS != module_link_ptr->GetProperty(PROP_MODULE_LINK_INFO_TYPE_EDGE_REPEATER, &repeater_include_module)) {
    YSOS_LOG_DEBUG("Get repeater failed:" << service_name);
    return NULL;
  }

  if (repeater_include_module.repeater_ptr_list_.size() == 0) {
    YSOS_LOG_DEBUG("Get repeater failed:" << service_name);
    return NULL;
  }

  RepeaterInfoPtr repeater_info_ptr = boost::make_shared<RepeaterInfo>();
  assert(NULL != repeater_info_ptr);
  repeater_info_ptr->real_name = real_name;
  repeater_info_ptr->repeater_ptr = *(repeater_include_module.repeater_ptr_list_.begin());
  repeater_map_[service_name] = repeater_info_ptr;


  return repeater_info_ptr;
}
}
