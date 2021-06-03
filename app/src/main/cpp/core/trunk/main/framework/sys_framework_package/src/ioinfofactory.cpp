/**
 *@file IoInfoFactory.cpp
 *@brief Definition of IoInfoFactory
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// ysos private headers //  NOLINT
#include "../../../protect/include/sys_framework_package/ioinfofactory.h"
#include "../../../protect/include/sys_framework_package/baseioinfoimpl.h"

namespace ysos {
DEFINE_SINGLETON(IoInfoFactory);
IoInfoFactory::IoInfoFactory(const std::string &strClassName): BaseInterfaceImpl(strClassName) {
}

IoInfoFactory::~IoInfoFactory() {
}

IoInfoInterfacePtr IoInfoFactory::CreateObject(const std::string &object_type, const std::string &object_name) {
  IoInfoInterface *ioinfo_ptr = NULL;
  if ("int" == object_type) {
    ioinfo_ptr = new BaseIoInfoImplExtend<int>(object_name);
  } else if ("float" == object_type) {
    ioinfo_ptr = new BaseIoInfoImplExtend<float>(object_name);
  } else if ("char" == object_type) {
    ioinfo_ptr = new BaseIoInfoImplExtend<char>(object_name);
  } else if ("long" == object_type) {
    ioinfo_ptr = new BaseIoInfoImplExtend<long>(object_name);
  } else if ("double" == object_type) {
    ioinfo_ptr = new BaseIoInfoImplExtend<double>(object_name);
  } else if ("string" == object_type || "text" == object_type) {
    ioinfo_ptr = new BaseIoInfoImplExtend<std::string>(object_name);
  } else {
    ioinfo_ptr = new BaseIoInfoImpl(object_name);
  }
  if (NULL == ioinfo_ptr) {
    return NULL;
  }

  return IoInfoInterfacePtr(ioinfo_ptr);
}
}
