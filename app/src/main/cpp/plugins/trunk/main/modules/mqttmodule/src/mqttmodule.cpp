/**
 *@file mqttmodule.cpp
 *@brief mqtt module interface
 *@version 1.0
 *@author z.gg
 *@date Created on: 2020-3-30 9:10:00
 *@copyright Copyright (c) 2020 YS Technology. All rights reserved.
 *
 */
/// Private Headers //  NOLINT
#include "../include/mqttmodule.h"

#include <stdio.h>
#include <stdlib.h>
#include <mosqtt/mosquitto.h>

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

  struct mosquitto *mosq_;

  int floorId_;
  std::string mallId_;
  std::string mallName_;
  std::string mallTopic_;
  std::string mapName_;
  std::string robotCode_;
  int robotType_;
  std::string serialNumber_;
  std::string serverMallTopic_;

void mq_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
	
    if(message->payloadlen){
      printf("%s %s", message->topic, (char *)message->payload);
      YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.mqttex"), message->topic);
      YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.mqttex"), (char *)message->payload);
    }else{
      printf("%s (null)\n", message->topic);
      YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.mqttex"), message->topic << "is (null)");
    }
    fflush(stdout);
}

void mq_connect_callback(struct mosquitto *mosq, void *userdata, int result)
{
    int i;
    if(!result){
        /* Subscribe to broker information topics on successful connect. */
        mosquitto_subscribe(mosq, NULL, robotCode_.c_str(), 2);
        YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.mqttex"), "mosquitto_subscribe topic is " + robotCode_);
    }else{
        fprintf(stderr, "Connect failed\n");
        YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.mqttex"), "mosquitto_subscribe topic is " + robotCode_);
    }
}

void mq_subscribe_callback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos)
{
    int i;
    printf("Subscribed (mid: %d): %d", mid, granted_qos[0]);
    for(i=1; i<qos_count; i++){
        printf(", %d", granted_qos[i]);
    }
    printf("\n");
}

void mq_log_callback(struct mosquitto *mosq, void *userdata, int level, const char *str)
{
    /* Pring all log messages regardless of level. */
    printf("%s\n", str);
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.mqttex"), str);
}

DECLARE_PLUGIN_REGISTER_INTERFACE(MQttModule, ModuleInterface);  ///<  插件的入口，一定要加上 //  NOLINT

MQttModule::MQttModule(const std::string &strClassName)
    : BaseThreadModuleImpl(strClassName) {
		change_result_ = -1;
	  cur_state_ = "";

    mqs_host_ = "mq-robot.aiyunshen.com";
    mqs_port_ = 1883;
    mqs_username_ = "ops_jiagou";
    mqs_password_ = "Ops_jiagou1028";
    mqs_config_file_ = "../data/mqttmodule/";
  logger_ = GetUtility()->GetLogger("ysos.mqttex");
}

MQttModule::~MQttModule(void) {
}

int MQttModule::Ioctl(INT32 control_id, LPVOID param /* = nullptr */) {
  YSOS_LOG_DEBUG("module Ioctl execute");
  int n_return = YSOS_ERROR_SUCCESS;

  do {
    switch (control_id) {
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

int MQttModule::Flush(LPVOID param /* = nullptr */) {
  return YSOS_ERROR_SUCCESS;
}

int MQttModule::Initialize(LPVOID param /* = nullptr */) {
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

int MQttModule::UnInitialize(LPVOID param /* = nullptr */) {

  return BaseThreadModuleImpl::UnInitialize(param);
}

int MQttModule::GetProperty(int iTypeId, void *piType) {
  int ret = YSOS_ERROR_SUCCESS;

  switch (iTypeId) {
    case PROP_DATA: {
      if (data_.empty())
        data_ = GetCurrentStatus();
      YSOS_LOG_DEBUG("PROP_DATA: " << data_);
      if (data_.empty()) {
        ret = YSOS_ERROR_NOT_EXISTED;
        break;
      }
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

int MQttModule::InitalDataInfo(void) {
  YSOS_LOG_DEBUG("module InitalDataInfo execute");
  int n_return = YSOS_ERROR_SUCCESS;

  YSOS_LOG_DEBUG("module InitalDataInfo done");

  return n_return;
}

int MQttModule::Initialized(const std::string &key, const std::string &value) {
  YSOS_LOG_DEBUG("module Initialized execute");
  int n_return = YSOS_ERROR_SUCCESS;

  do {
	  //< TODO: get config info
    if ("host" == key) {
      mqs_host_ = value;
    } else if ("port" == key) {
      mqs_port_ = atoi(value.c_str());
    } else if ("username" == key) {
      mqs_username_ = value;
    } else if ("passwd" == key) {
      mqs_password_ = value;
    } else if ("filepath" == key) {
      mqs_config_file_ = value;
    } else if ("floorId" == key) {
      floorId_ = atoi(value.c_str());
    } else if ("mallId" == key) {
      mallId_ = value;
    } else if ("mallName" == key) {
      mallName_ = boost::locale::conv::from_utf ( value, "GBK" );
    } else if ("mallTopic" == key) {
      mallTopic_ = value;
    } else if ("mapName" == key) {
      mapName_ = value;
    } else if ("robotCode" == key) {
      robotCode_ = value;
    } else if ("robotType" == key) {
      robotType_ = atoi(value.c_str());
    } else if ("serialNumber" == key) {
      serialNumber_ = value;
    } else if ("serverMallTopic" == key) {
      serverMallTopic_ = value;
    }
  } while (0);
  YSOS_LOG_DEBUG("module Initialized done");

  return n_return;
}

int MQttModule::RealOpen(LPVOID param /* = nullptr */) {
  YSOS_LOG_DEBUG("module RealOpen execute");
  int n_return = YSOS_ERROR_SUCCESS;
  do {
    //< TODO: new object for single instance
    if (NULL == mosq_) {
      bool ret = mqtt_create();
      if (!ret) {
        YSOS_LOG_DEBUG("get mosq_ instance failed");
        n_return = YSOS_ERROR_FAILED;
        break;
      } else {
        YSOS_LOG_DEBUG("get mosq_ instance success");
        ret = mqtt_connect();
        if(ret) {
          YSOS_LOG_DEBUG("get mosq_ mqtt_connect success");
        } else {
          YSOS_LOG_DEBUG("get mosq_ mqtt_connect failed");
          n_return = YSOS_ERROR_FAILED;
          break;
        }
      }
    }
    n_return = BaseThreadModuleImpl::RealOpen(param);
    if (n_return != YSOS_ERROR_SUCCESS)
      break;
  } while (0);

  YSOS_LOG_DEBUG("module RealOpen done: " << n_return);
  return n_return;
}

int MQttModule::RealClose(void) {
  YSOS_LOG_DEBUG("module RealClose execute");
  int n_return = YSOS_ERROR_SUCCESS;
  do {
    mqtt_destory();
    n_return = BaseThreadModuleImpl::RealClose();
    if (n_return != YSOS_ERROR_SUCCESS)
      break;
  } while (0);

  YSOS_LOG_DEBUG("module RealClose done: " << n_return);
  return n_return;
}

int MQttModule::RealPause(void) {
  return YSOS_ERROR_SUCCESS;
}

int MQttModule::RealStop(void) {
  return YSOS_ERROR_SUCCESS;
}

int MQttModule::RealRun(void) {
  return YSOS_ERROR_SUCCESS;
}

int MQttModule::ChangeState(const std::string &condition) {

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

std::string MQttModule::GetCurrentStatus() {

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

bool MQttModule::mqtt_create()
{
  //struct mosquitto *mosq = NULL;
	char buff[512];
  //libmosquitto 库初始化
  mosquitto_lib_init();
  //创建mosquitto客户端
  mosq_ = mosquitto_new(NULL, true, NULL);
  if(!mosq_){
      printf("create client failed..\n");
      mosquitto_lib_cleanup();
      return false;
  }
  /* set the username and password */
  mosquitto_username_pw_set(mosq_, mqs_username_.c_str(), mqs_password_.c_str());

  //设置回调函数，需要时可使用
  mosquitto_log_callback_set(mosq_, mq_log_callback);
  mosquitto_connect_callback_set(mosq_, mq_connect_callback);
  mosquitto_message_callback_set(mosq_, mq_message_callback);
  mosquitto_subscribe_callback_set(mosq_, mq_subscribe_callback);
  return true;
}
bool MQttModule::mqtt_connect()
{
    //连接服务器
    int reconnect_time = 0;
    int result = 1;
    while (reconnect_time < 5 && result) {
        result = mosquitto_connect(mosq_, mqs_host_.c_str(), mqs_port_, 60);
        if(result){
            fprintf(stderr, "Unable to connect, try to reconnect...\n");
            reconnect_time += 1;
        }
    }
    if (result) {
      fprintf(stderr, "Connect failed");
      return false;
    } else {
      fprintf(stderr, "Connect success");
    }
    //开启一个线程，在线程里不停的调用 mosquitto_loop() 来处理网络信息
    int loop = mosquitto_loop_start(mosq_); 
    if(loop != MOSQ_ERR_SUCCESS)
    {
        printf("mosquitto loop error\n");
        return false;
    }
    return true;
}

void MQttModule::mqtt_destory()
{
    mosquitto_destroy(mosq_);
    mosquitto_lib_cleanup();
}

}