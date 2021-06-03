/**
  *@file WindowsPrivilegeAdust.h
  *@brief Definition of WindowsPrivilegeAdust
  *@version 0.1
  *@author dhongqian
  *@date Created on: 2016-04-21 13:59:20
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  */
#ifndef OHP_ADJUST_WINDOWS_PRIVILEGE_H_
#define OHP_ADJUST_WINDOWS_PRIVILEGE_H_

/// ysos private headers //  NOLINT
#include "../../../public/include/os_hal_package/systemprivilegeinterface.h"
/// stl headers //  NOLINT
#include <string>
#include <list>

namespace ysos {
/**
 *@brief 系统权力提升或清空
 */
class WindowsPrivilegeAdjust: public SystemPrivilegeInterface
{
  DECLARE_CREATEINSTANCE(WindowsPrivilegeAdjust);
  DISALLOW_COPY_AND_ASSIGN(WindowsPrivilegeAdjust);
  DECLARE_PROTECT_CONSTRUCTOR(WindowsPrivilegeAdjust);
public:
  virtual ~WindowsPrivilegeAdjust(void);

  /**
    *@brief  调整指定的权限，如果权限名为空，则调整所有可以提升的权限//  NOLINT
    *@param privilege_name  待调整的权限名 //  NOLINT
    *@param is_enable true为提升，false为降低 //  NOLINT
    *@return 只要有一个权限调整成功，就返回true, 全部失败，返回false  //  NOLINT
    */
  virtual bool AjustPrivilege(const std::string &privilege_name="", bool is_enable=true);

protected:
  /**
    *@brief  获取待调整的权限列表 //  NOLINT
    *@param privilege_list  待调整的权限名都存放在list中 //  NOLINT
    *@param privilege_name 若为空，则取得所有可调整的权限名 //  NOLINT
    *@return 无  //  NOLINT
    */
  void GetPrivilegeList(std::list<std::string> &privilege_list, const std::string &privilege_name="");
  /**
    *@brief  实际调整指定的权限 //  NOLINT
    *@param privilege_name  待调整的权限名 //  NOLINT
    *@param is_enable true为提升，false为降低 //  NOLINT
    *@return true 成功，fale失败 //  NOLINT
    */
  bool SetPrivilege(const std::string privilege_name, bool is_enable);
};
}
#endif
