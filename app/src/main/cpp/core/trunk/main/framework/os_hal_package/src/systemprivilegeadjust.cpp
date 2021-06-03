/**
 *@file SystemPrivilegeAdust.cpp
 *@brief Definition of SystemPrivilegeAdust
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// ysos private headers //  NOLINT
#include "../../../public/include/os_hal_package/systemprivilegeadjust.h"
#include "../../../protect/include/os_hal_package/windowsprivilegeadjust.h"
/// c headers //  NOLINT
#ifdef WIN32
extern "C" {
#include <windows.h>
};
#endif
#include <cstdio>

namespace ysos {
SystemPrivilegeAdjust::SystemPrivilegeAdjust(SystemType system_type)
{
  if(WINDOWS == system_type) {
    #ifdef _WIN32
      privilege_adjust_impl_ptr_ = SystemPrivilegeInterfacePtr(WindowsPrivilegeAdjust::CreateInstance("WindowsPrivilegeAdjust"));
      assert(NULL != privilege_adjust_impl_ptr_);
    #else
      //TDOO: need add something action on here for futrue   // add fro linux
    #endif
  }
}


SystemPrivilegeAdjust::~SystemPrivilegeAdjust(void)
{
}

bool SystemPrivilegeAdjust::AjustPrivilege(const std::string &privilege_name, bool is_enable) {
  if(NULL == privilege_adjust_impl_ptr_) {
    return false;
  }
  
  return privilege_adjust_impl_ptr_->AjustPrivilege(privilege_name, is_enable);
}
}
