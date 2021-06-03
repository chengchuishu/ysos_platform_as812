/**
 *@file sysinfo.h
 *@brief Definition of sysinfo
 *@version 0.1
 *@author jinchengzhe
 *@date Created on: 2016-06-21 15:22:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef SFP_SYS_INFO_H  //NOLINT
#define SFP_SYS_INFO_H  //NOLINT

#include "../../../protect/include/sys_framework_package/serviceinfotable.h"

namespace ysos {

class SysInfo {
 public:
  ServiceInfoTablePtr GetServiceInfoTable();
};


}

#endif
