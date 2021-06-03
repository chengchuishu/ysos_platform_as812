/**
 *@file BaseInterfaceImpl.cpp
 *@brief Definition of BaseInterfaceImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"
//#include <log4cplus/logger.h>

namespace ysos {
std::string BaseInterfaceImpl::class_name_ = std::string("BaseInterfaceImpl");
uuid BaseInterfaceImpl::class_uuid_ = boost::uuids::string_generator()("{82B28479-C9D8-40AD-80CE-6D2CD653558B}"); // NOLINT

BaseInterfaceImpl::BaseInterfaceImpl(const std::string &name) {
  // YSOS_LOG_DEBUG("BaseInterfaceImpl()");
  //* need add for linux
  if (!logger_.valid()) {
    logger_ = log4cplus::Logger::getInstance("ysos");
  }//*/
  logic_name_ = name;
  boost::uuids::string_generator genStr;
  boost::uuids::uuid uuidBase = genStr("{82B28479-C9D8-40AD-80CE-6D2CD653558B}");
  boost::uuids::name_generator genName(uuidBase);
  class_uuid_ = genName("BaseInterfaceImpl");
  std::stringstream ss;
  std::string strTemp;
  ss << class_uuid_;
  strTemp = ss.str();
  ss << (unsigned long)(static_cast<const void *>(this));
  strTemp += ss.str();
  instance_uuid_ = genName(strTemp);
  ss << instance_uuid_;
  strTemp = ss.str();
  boost::replace_all(strTemp, "-", "");
  instance_name_ = strTemp + logic_name_;
  is_initialize_ = is_init_succeeded_ = false;
}

BaseInterfaceImpl::~BaseInterfaceImpl(void) {
  // YSOS_LOG_DEBUG("~BaseInterfaceImpl()"); 
}

int BaseInterfaceImpl::Initialize(void *param) {
  if(is_initialize_ && is_init_succeeded_) {
    YSOS_LOG_DEBUG("already initialzed and succeeded " << logic_name_); //need add for linux 
    return YSOS_ERROR_SUCCESS;
  }

  int ret = RealInitialize(param);
  if(YSOS_ERROR_SUCCESS == ret) {
    is_initialize_ = is_init_succeeded_ = true;
  } else {
    is_initialize_ = true;
    is_init_succeeded_ = false;
  }

  return ret;
}

bool BaseInterfaceImpl::IsInitialized(void) {
  return is_initialize_;
}

bool BaseInterfaceImpl::IsInitSucceeded(void) {
  return is_init_succeeded_;
}

int BaseInterfaceImpl::UnInitialize(void *param) {
  if(!is_initialize_) {
    YSOS_LOG_DEBUG("uninitialize yet"); //need add for linux 
    return YSOS_ERROR_SUCCESS;
  }

  int ret = RealUnInitialize(param);
  if(YSOS_ERROR_SUCCESS != ret) {
    YSOS_LOG_ERROR("real uninitialize failed: " << ret << " | " << logic_name_); //need add for linux 
  }

  is_initialize_ = is_init_succeeded_ = false;

  return ret;
}

int BaseInterfaceImpl::RealInitialize(void *param) {
  return YSOS_ERROR_SUCCESS;
}

int BaseInterfaceImpl::RealUnInitialize(void *param) {
  return YSOS_ERROR_SUCCESS;
}

std::string BaseInterfaceImpl::GetName(bool is_full_name) const {
  if (is_full_name) {
    return instance_name_;
  } else {
    return logic_name_;
  }
}
uuid BaseInterfaceImpl::GetUUID() const {
  return instance_uuid_;
}
int BaseInterfaceImpl::GetProperty(int type_id, void *type) {
  return YSOS_ERROR_SUCCESS;
}
int BaseInterfaceImpl::SetProperty(int type_id, void *type) {
  return YSOS_ERROR_SUCCESS;
}

std::string BaseInterfaceImpl::GetThisClassName() {
  return class_name_;
};
void BaseInterfaceImpl::SetClassName(const std::string &class_name) {
  class_name_ = class_name;
  class_uuid_ = boost::uuids::string_generator()(class_name_);
}
}
