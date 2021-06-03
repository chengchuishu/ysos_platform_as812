/**
 *@file BaseCommmonStrategyImpl.cpp
 *@brief Definition of BaseCommmonStrategyImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// Self Header //  NOLINT
#include "../../../protect/include/sys_framework_package/ysosdaemonstrategyimpl.h"  // NOLINT
/// c headers //  NOLINT
#include <cassert>
/// stl headers //  NOLINT
#include <algorithm>
/// boost headers //  NOLINT
#include <boost/algorithm/string.hpp>
#include <boost/make_shared.hpp>
/// ysos headers //  NOLINT
#include "../../../public/include/core_help_package/xmlutil.h"
#include "../../../protect/include/os_hal_package/msgimpl.h"
#include "../../../protect/include/sys_framework_package/basemoduleimpl.h"
#include "../../../public/include/os_hal_package/filepath.h"
#include "../../../public/include/os_hal_package/bufferutility.h"
#include "../../../protect/include/sys_framework_package/platforminfo.h"
#include "../../../public/include/sys_interface_package/ysos_enum.h"
#include "../../../protect/include/sys_framework_package/manager_declare.h"
#include "../../../public/include/sys_interface_package/messagetype.h"

namespace ysos {
/*************************************************************************************************************
 *                      BaseStrategyImpl                                                                     *
 *************************************************************************************************************/
YsosDaemonStrategyImpl::YsosDaemonStrategyImpl(const std::string& strClassName)
  : BaseCommonStrategyImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.framework.strategy");
}

YsosDaemonStrategyImpl:: ~YsosDaemonStrategyImpl(void) {

}

int YsosDaemonStrategyImpl::Initialize(void *param) {
  return BaseCommonStrategyImpl::Initialize(param);
}

int YsosDaemonStrategyImpl::UnInitialize(void *param) {
  return BaseCommonStrategyImpl::UnInitialize(param);
}

}
