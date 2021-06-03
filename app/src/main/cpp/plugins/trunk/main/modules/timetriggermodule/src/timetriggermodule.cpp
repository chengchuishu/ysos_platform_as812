/**
 *@file timetriggermodule.cpp
 *@brief time trigger module
 *@version 1.0
 *@author z.gg
 *@date Created on: 2020-9-1 9:10:00
 *@copyright Copyright (c) 2020 YS Technology. All rights reserved.
 *
 */
/// Private Headers //  NOLINT
#include "../include/timetriggermodule.h"
#include <json/json.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

namespace ysos {
DECLARE_PLUGIN_REGISTER_INTERFACE(TimeTriggerModule, ModuleInterface);  ///<  插件的入口，一定要加上 //  NOLINT

TimeTriggerModule::TimeTriggerModule(const std::string &strClassName) : BaseThreadModuleImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.timetrigger");
}

TimeTriggerModule::~TimeTriggerModule(void) {

}

int TimeTriggerModule::GetProperty(int type_id, void *type) {
  int n_return = YSOS_ERROR_SUCCESS;
  int n_result = -1;

  do {
    if (type_id == 25701) {
      std::string str_result = "";
      Json::Reader js_reader;
      Json::FastWriter js_writer;
      Json::Value js_value;

      BufferInterfacePtr *buffer_ptr = static_cast<BufferInterfacePtr*>(type);
      BufferInterfacePtr in_buffer_ptr = *buffer_ptr;
      if (in_buffer_ptr == NULL) {
        YSOS_LOG_DEBUG("in_buffer_ptr is null");
        n_return = YSOS_ERROR_FAILED;
        break;
      }

      UINT8* buffer;
      UINT32 buffer_size;
      n_return = in_buffer_ptr->GetBufferAndLength(&buffer, &buffer_size);
      if (YSOS_ERROR_SUCCESS != n_return) {
        YSOS_LOG_DEBUG("GetBufferAndLength error");
        n_return = YSOS_ERROR_FAILED;
        break;
      }
      memset(buffer, 0, buffer_size);
      lock_.Lock();
      switch (type_id) {
      case 25701: {
        YSOS_LOG_DEBUG("module GetProperty CMD_CALLBACK_AUTO");
        //YYYYMMDDTHHMMSS，日期和时间用大写字母T隔开了
        break;
      }
      default: {
      }
      }
      lock_.Unlock();
    } else {
      n_result = BaseThreadModuleImpl::GetProperty(type_id, type);
      if (n_result != YSOS_ERROR_SUCCESS) {
        n_return = YSOS_ERROR_FAILED;
        YSOS_LOG_DEBUG("execute BaseThreadModuleImpl GetProperty failed");
        break;
      }
    }
  } while (0);

  return n_return;
}

int TimeTriggerModule::Ioctl(INT32 control_id, LPVOID param /* = nullptr */) {
  YSOS_LOG_DEBUG("module Ioctl execute");
  int n_return = YSOS_ERROR_SUCCESS;

  do {
    switch (control_id) {

    default: {
      YSOS_LOG_DEBUG("Ioctl id error, control_id = " << control_id);
      n_return = YSOS_ERROR_FAILED;
      break;
    }
    }
  } while (0);

  YSOS_LOG_DEBUG("module Ioctl done");

  return n_return;
}

int TimeTriggerModule::Flush(LPVOID param /* = nullptr */) {
  return YSOS_ERROR_SUCCESS;
}

int TimeTriggerModule::Initialize(LPVOID param /* = nullptr */) {
  YSOS_LOG_DEBUG("module Initialize execute");
  int n_return = YSOS_ERROR_SUCCESS;

  do {
    if (IsInitSucceeded()) {
      n_return = YSOS_ERROR_FINISH_ALREADY;
      break;
    }

    if (YSOS_ERROR_SUCCESS != BaseModuleImpl::Initialize(param)) {
      YSOS_LOG_DEBUG("Initialize BaseThreadModuleImpl failed");
      n_return = YSOS_ERROR_FAILED;
      break;
    }
  } while (0);

  YSOS_LOG_DEBUG("module Initialize done");

  return n_return;
}

int TimeTriggerModule::UnInitialize(LPVOID param /* = nullptr */) {
  return YSOS_ERROR_SUCCESS;
}

int TimeTriggerModule::InitalDataInfo(void) {
  YSOS_LOG_DEBUG("module InitalDataInfo execute");
  int n_return = YSOS_ERROR_SUCCESS;

  YSOS_LOG_DEBUG("module InitalDataInfo done");

  return n_return;
}

int TimeTriggerModule::Initialized(const std::string &key, const std::string &value) {
  YSOS_LOG_DEBUG("module Initialized execute");
  int n_return = YSOS_ERROR_SUCCESS;

  do {
    if ("period" == key)
      period_ = value;
    else if ("eventtime" == key)
      event_time_ = value;
    else if ("timeinterval" == key)
      time_interval_ = boost::lexical_cast<UINT32>(value);
    else if ("eventinterval" == key)
      event_interval_ = boost::lexical_cast<UINT32>(value);

  } while (0);

  YSOS_LOG_DEBUG("module Initialized done");

  return n_return;
}

int TimeTriggerModule::RealOpen(LPVOID param /* = nullptr */) {
  YSOS_LOG_DEBUG("module RealOpen execute");
  int n_return = YSOS_ERROR_SUCCESS;
  do {
    YSOS_LOG_DEBUG("period_ = " << period_);
    YSOS_LOG_DEBUG("event_time_ = " << event_time_);
    YSOS_LOG_DEBUG("time_interval_ = " << time_interval_);
    YSOS_LOG_DEBUG("event_interval_ = " << event_interval_);

    thread_data_->timeout = time_interval_*1000;

    n_return = BaseThreadModuleImpl::RealOpen(param);
    if (n_return != YSOS_ERROR_SUCCESS)
      break;
  } while (0);
  YSOS_LOG_DEBUG("module RealOpen done");

  return n_return;
}

int TimeTriggerModule::RealClose(void) {
  YSOS_LOG_DEBUG("module RealClose execute");
  int n_return = YSOS_ERROR_SUCCESS;
  do {
    n_return = BaseThreadModuleImpl::RealClose();
    if (n_return != YSOS_ERROR_SUCCESS)
      break;
  } while (0);
  YSOS_LOG_DEBUG("module RealClose done");

  return n_return;
}

int TimeTriggerModule::RealPause(void) {
  return YSOS_ERROR_SUCCESS;
}

int TimeTriggerModule::RealStop(void) {
  return YSOS_ERROR_SUCCESS;
}

int TimeTriggerModule::RealRun(void) {
  return YSOS_ERROR_SUCCESS;
}


}