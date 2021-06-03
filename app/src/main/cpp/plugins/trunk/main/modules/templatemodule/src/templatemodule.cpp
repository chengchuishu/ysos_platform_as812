/**
 *@file templatemodule.cpp
 *@brief template module interface
 *@version 1.0
 *@author z.gg
 *@date Created on: 2020-3-30 9:10:00
 *@copyright Copyright (c) 2020 YS Technology. All rights reserved.
 *
 */
/// Private Headers //  NOLINT
#include "../include/templatemodule.h"

#include <string>
#include <fstream>
#include <sstream>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/locale.hpp>
#include <boost/filesystem.hpp>
#include <json/json.h>
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/jsonutility.h"


namespace ysos {
DECLARE_PLUGIN_REGISTER_INTERFACE(TemplateModule, ModuleInterface);  ///<  插件的入口，一定要加上 //  NOLINT

TemplateModule::TemplateModule(const std::string &strClassName)
    : BaseThreadModuleImpl(strClassName) {
		change_result_ = -1;
	    cur_state_ = "";
  logger_ = GetUtility()->GetLogger("ysos.template");
}

TemplateModule::~TemplateModule(void) {
}

int TemplateModule::Ioctl(INT32 control_id, LPVOID param /* = nullptr */) {
  YSOS_LOG_DEBUG("module Ioctl execute");
  int n_return = YSOS_ERROR_SUCCESS;

  do {
    switch (control_id) {
      //TODO: transfer callback data to event when need to change current state
      case CMD_TRANSFER_EVENT: {
        YSOS_LOG_DEBUG("CMD_TRANSFER_EVENT command");
        std::string *str = reinterpret_cast<std::string*>(param);
        YSOS_LOG_DEBUG("get template CMD_TEMPLATE_CHANGE get data = " << *str);
        n_return = TransferType4Event(*str);
        break;
      }
      case CMD_GENERATE_EVENT: {
        YSOS_LOG_DEBUG("CMD_GENERATE_EVENT command");
        std::string *str = reinterpret_cast<std::string*>(param);
        YSOS_LOG_DEBUG("get template CMD_GENERATE_EVENT get data = " << *str);
        n_return = Generate4Event(*str);
        break;
      }
      case CMD_CHANGE_STATE: {
        YSOS_LOG_DEBUG("TEMPLATE_CHANGE command");

        std::string *str = reinterpret_cast<std::string*>(param);
        YSOS_LOG_DEBUG("get template CMD_TEMPLATE_CHANGE get data = " << *str);
        JsonValue json_value;
        n_return = GetJsonUtil()->JsonObjectFromString(*str, json_value);
        if (YSOS_ERROR_SUCCESS != n_return) {
          YSOS_LOG_ERROR("CMD_TEMPLATE_CHANGE parse to json failed: " << *str << " | " << logic_name_);
          break;
        }

        if (true == json_value.empty()) {
          YSOS_LOG_ERROR("json_value is empty");
          break;
        }

        std::string cond_data;
        if (true == json_value.isMember("condition") &&
          true == json_value["condition"].isString()) {
            YSOS_LOG_DEBUG("parse success");
            cond_data = json_value["condition"].asString();
        } else {
          YSOS_LOG_ERROR("parse failed");
          n_return = YSOS_ERROR_FAILED;
          break;
        }
        YSOS_LOG_ERROR("condition [" << cond_data << "]");
        n_return = ChangeState(cond_data);
        break;
      }
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

int TemplateModule::Flush(LPVOID param /* = nullptr */) {
  return YSOS_ERROR_SUCCESS;
}

int TemplateModule::Initialize(LPVOID param /* = nullptr */) {
  YSOS_LOG_DEBUG("module Initialize execute");
  int n_return = YSOS_ERROR_SUCCESS;

  do {
    if (IsInitSucceeded()) {
      n_return = YSOS_ERROR_FINISH_ALREADY;
      break;
    }

    n_return = BaseThreadModuleImpl::Initialize(param);
    if (n_return != YSOS_ERROR_SUCCESS) {
      YSOS_LOG_DEBUG("Initialize BaseThreadModuleImpl failed");
      break;
    }
  } while (0);

  YSOS_LOG_DEBUG("module Initialize done: "<< n_return);
  return n_return;
}

int TemplateModule::UnInitialize(LPVOID param /* = nullptr */) {
  return BaseThreadModuleImpl::UnInitialize(param);
}

int TemplateModule::GetProperty(int iTypeId, void *piType) {
  int ret = YSOS_ERROR_SUCCESS;

  switch (iTypeId) {
    case PROP_DATA: {
      YSOS_LOG_DEBUG("GetProperty-->PROP_DATA [Enter] ");
      /*
      if (data_.empty())
        data_ = GetCurrentStatus();
      YSOS_LOG_DEBUG("PROP_DATA: " << data_);*/
      if (data_.empty()) {
        ret = YSOS_ERROR_NOT_EXISTED;
        break;
      }
      YSOS_LOG_DEBUG("PROP_DATA: " << data_);
      BufferInterfacePtr *buffer_ptr_ptr = reinterpret_cast<BufferInterfacePtr*>(piType);
      if (NULL == buffer_ptr_ptr) {
        ret = YSOS_ERROR_INVALID_ARGUMENTS;
        break;
      }
      BufferInterfacePtr buffer_ptr = *buffer_ptr_ptr;
      if (NULL == buffer_ptr) {
        ret = YSOS_ERROR_INVALID_ARGUMENTS;
        break;
      }
      uint8_t *data = GetBufferUtility()->GetBufferData(buffer_ptr);
      size_t buffer_length = GetBufferUtility()->GetBufferLength(buffer_ptr);
      if (NULL == data || 0 == buffer_length || buffer_length <= data_.length()) {
        ret = YSOS_ERROR_INVALID_ARGUMENTS;
        break;
      }

      memcpy(data, data_.c_str(), data_.length());
      data[data_.length()] = '\0';
      data_.clear();
      break;
    }
    default:
      ret = BaseThreadModuleImpl::GetProperty(iTypeId, piType);
  }

  return ret;
}

int TemplateModule::InitalDataInfo(void) {
  YSOS_LOG_DEBUG("module InitalDataInfo execute");
  int n_return = YSOS_ERROR_SUCCESS;

  YSOS_LOG_DEBUG("module InitalDataInfo done");

  return n_return;
}

int TemplateModule::Initialized(const std::string &key, const std::string &value) {
  YSOS_LOG_DEBUG("module Initialized execute");
  int n_return = YSOS_ERROR_SUCCESS;

  do {
	  //< TODO: get config info
  } while (0);
  YSOS_LOG_DEBUG("module Initialized done");

  return n_return;
}

int TemplateModule::RealOpen(LPVOID param /* = nullptr */) {
  YSOS_LOG_DEBUG("module RealOpen execute");
  int n_return = YSOS_ERROR_SUCCESS;
  do {
    //< TODO: new object for single instance
    n_return = BaseThreadModuleImpl::RealOpen(param);
    if (n_return != YSOS_ERROR_SUCCESS)
      break;
  } while (0);

  YSOS_LOG_DEBUG("module RealOpen done: " << n_return);
  return n_return;
}

int TemplateModule::RealClose(void) {
  YSOS_LOG_DEBUG("module RealClose execute");
  int n_return = YSOS_ERROR_SUCCESS;
  do {
    n_return = BaseThreadModuleImpl::RealClose();
    if (n_return != YSOS_ERROR_SUCCESS)
      break;
  } while (0);

  YSOS_LOG_DEBUG("module RealClose done: " << n_return);
  return n_return;
}

int TemplateModule::RealPause(void) {
  return YSOS_ERROR_SUCCESS;
}

int TemplateModule::RealStop(void) {
  return YSOS_ERROR_SUCCESS;
}

int TemplateModule::RealRun(void) {
  return YSOS_ERROR_SUCCESS;
}

int TemplateModule::ChangeState(const std::string &condition) {

  Json::Value json_body;
  json_body["type"] = "template_result";

  do {
	
	YSOS_LOG_DEBUG("******ChangeState--[condition]"<<condition);
	if ("c1" == condition) {
		change_result_ = 0;
		new_state_ = "S1->main@PrepareService";
		if(new_state_.find(cur_state_)){
		   old_state_ = cur_state_;
		   cur_state_ = new_state_;
		} else{
		  return YSOS_ERROR_SUCCESS;
		}
	} else if("c2" == condition) {
		change_result_ = 0;
		new_state_ = "S2->main@WaitingService";
		if(new_state_.find(cur_state_) == std::string::npos){
		   old_state_ = cur_state_;
		   cur_state_ = new_state_;
		}  else{
		  return YSOS_ERROR_SUCCESS;
		}
	} else if("c3" == condition) {
		change_result_ = 0;
		new_state_ = "S3->main@OnService";
		if(new_state_.find(cur_state_) == std::string::npos){
		   old_state_ = cur_state_;
		   cur_state_ = new_state_;
		}  else{
		  return YSOS_ERROR_SUCCESS;
		}
	} else if("c4" == condition) {
		change_result_ = 0;
		new_state_ = "S4->main@SuspendService";
		if(new_state_.find(cur_state_) == std::string::npos){
		   old_state_ = cur_state_;
		   cur_state_ = new_state_;
		}  else{
		  return YSOS_ERROR_SUCCESS;
		}
    } else {
		change_result_ = 1;
		YSOS_LOG_ERROR("UnKnown condition !!!");
	}
	json_body["template_result"] = change_result_;
	json_body["old_state"] = old_state_;
	json_body["new_state"] = new_state_;
    data_ptr_->SetData("template_result", "0");
    data_ptr_->SetData("old_state", old_state_);
    data_ptr_->SetData("new_state", new_state_);

  } while (0);

  // 发事件给前端
  {
    Json::FastWriter writer;
    data_ = writer.write(json_body);
    data_ = GetUtility()->ReplaceAllDistinct ( data_, "\\r\\n", "" );
  }

  SetProperty(PROP_THREAD_NOTIFY, NULL);
  return YSOS_ERROR_SUCCESS;
}

std::string TemplateModule::GetCurrentStatus() {

	data_ptr_ = GetDataInterfaceManager()->GetData(GLOBAL_DATA_KEY);
	if (!data_ptr_) {
		YSOS_LOG_WARN("********==get data_ptr failed.==********"); 
	}
	std::string data_value;
	data_ptr_->GetData("machine_state", data_value);
	if (cur_state_.empty()) {
		cur_state_ = data_value;
	} /*else {
	   YSOS_LOG_WARN("********[no init machine_state params !!!]"); 
	}*/

	cur_state_ = data_value;

	std::string result = "{\"type\":\"template_state\",\"data\":{\"state\":\"";
	if (cur_state_.empty())
	{
	   result.append("\"}}");
	} else {
	   result.append(cur_state_).append("\"}}");
	}
	YSOS_LOG_DEBUG("********[GetCurrentStatus]="<< result);
	return result;
}

int TemplateModule::TransferType4Event(const std::string &typedata) {
  int n_return = YSOS_ERROR_FAILED;

  do {
        JsonValue json_value;
        n_return = GetJsonUtil()->JsonObjectFromString(typedata, json_value);
        if (YSOS_ERROR_SUCCESS != n_return) {
          YSOS_LOG_ERROR("TransferType4Event parse to json failed: " << typedata << " | " << logic_name_);
          break;
        }

        if (true == json_value.empty()) {
          YSOS_LOG_ERROR("json_value is empty");
          break;
        }

        if (true == json_value.isMember("type") && true == json_value["type"].isString()) {
            YSOS_LOG_DEBUG("parse type success ... ");
            YSOS_LOG_DEBUG("[Check][0]-->" << json_value["type"].asString() );
            n_return = YSOS_ERROR_SUCCESS;
        } else {
          YSOS_LOG_ERROR("[Error] parse type failed ... ");
          break;
        }
        
        data_ = typedata;
        SetProperty(PROP_THREAD_NOTIFY, NULL);
        break;

  } while (0);

  return n_return;
}

int TemplateModule::Generate4Event(const std::string &eventdata) {
  int n_return = YSOS_ERROR_FAILED;

  do {
        JsonValue json_value;
        n_return = GetJsonUtil()->JsonObjectFromString(eventdata, json_value);
        if (YSOS_ERROR_SUCCESS != n_return) {
          YSOS_LOG_ERROR("Generate4Event parse to json failed: " << eventdata << " | " << logic_name_);
          break;
        }

        if (true == json_value.empty()) {
          YSOS_LOG_ERROR("json_value is empty");
          break;
        }
        
        if (true == json_value.isMember("type") && true == json_value["type"].isString()) {
            YSOS_LOG_DEBUG("parse event_name success ... ");
            YSOS_LOG_DEBUG("[Check][0]-->" << json_value["type"].asString() );
            n_return = YSOS_ERROR_SUCCESS;
        } else {
          YSOS_LOG_ERROR("[Error] parse type failed ... ");
          break;
        }
        
        data_ = eventdata;
        SetProperty(PROP_THREAD_NOTIFY, NULL);
        break;

  } while (0);

  return n_return;
}

}