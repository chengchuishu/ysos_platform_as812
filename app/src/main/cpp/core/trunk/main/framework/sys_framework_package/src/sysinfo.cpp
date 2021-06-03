/**
 *@file sysinfo.cpp
 *@brief Definition of sysinfo
 *@version 0.1
 *@author jinchengzhe
 *@date Created on: 2016-06-21 15:22:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

#include "../../../protect/include/sys_framework_package/sysinfo.h"

namespace ysos {

ServiceInfoTablePtr SysInfo::GetServiceInfoTable() {
  return ServiceInfoTable::Instance();
}

}
