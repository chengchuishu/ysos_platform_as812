/**
  *@file SystemPrivilegeAdust.h
  *@brief Definition of SystemPrivilegeAdust
  *@version 0.1
  *@author dhongqian
  *@date Created on: 2016-04-21 13:59:20
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  */
#ifndef OHP_ADJUST_SYSTEM_PRIVILEGE_H_
#define OHP_ADJUST_SYSTEM_PRIVILEGE_H_

/// ysos private headers //  NOLINT
#include "../../../public/include/os_hal_package/systemprivilegeinterface.h"
/// stl headers //  NOLINT
#include <string>
#include <list>

namespace ysos {
/**
 *@brief 系统权力提升或清空
 */
class YSOS_EXPORT SystemPrivilegeAdjust
{
public:
  enum SystemType {WINDOWS, LINUX, ANDROID, IOS, SytemType_End};

  SystemPrivilegeAdjust(SystemType system_type=WINDOWS);
  virtual ~SystemPrivilegeAdjust(void);

  
  /**
    *@brief  调整指定的权限，如果权限名为空，则调整所有可以提升的权限//  NOLINT
    *@param privilege_name  待调整的权限名 //  NOLINT
    *@param is_enable true为提升，false为降低 //  NOLINT
    *@return 只要有一个权限调整成功，就返回true, 全部失败，返回false  //  NOLINT
    */
  bool AjustPrivilege(const std::string &privilege_name="", bool is_enable=true);

private:
  SystemPrivilegeInterfacePtr  privilege_adjust_impl_ptr_;
};
}
#endif
