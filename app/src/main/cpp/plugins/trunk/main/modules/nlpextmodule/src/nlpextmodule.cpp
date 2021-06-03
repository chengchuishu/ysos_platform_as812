/**
 *@file nlpextmodule.cpp
 *@brief nlpext module
 *@version 1.0
 *@author l.j..
 *@date Created on: 2016-10-20 14:50:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
/// Private Headers //  NOLINT
#include "../include/nlpextmodule.h"

#include <json/json.h>
#include "../../../../../../core/trunk/main/public/include/core_help_package/jsonutility.h"

namespace ysos {
DECLARE_PLUGIN_REGISTER_INTERFACE(NlpExtModule, ModuleInterface);  ///<  插件的入口，一定要加上 //  NOLINT
NlpExtModule::NlpExtModule(const std::string &strClassName) : BaseModuleImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.audio.nlpext");
}

NlpExtModule::~NlpExtModule(void) {

}

int NlpExtModule::Ioctl(INT32 control_id, LPVOID param /* = nullptr */) {
  YSOS_LOG_DEBUG("module Ioctl execute");
  int n_return = YSOS_ERROR_SUCCESS;
  int n_result = -1;

  ioctl_lock_.Lock();
  do {
    std::string ioctl_value = *static_cast<std::string*>(param);
    YSOS_LOG_DEBUG("ioctl_value = " << ioctl_value);

    switch (control_id) {
    case CMD_AUDIO_JSON_NLP: {
      YSOS_LOG_DEBUG("CMD_AUDIO_JSON_NLP in");
      Json::Value json_ioctl;
      Json::Reader json_reader;
      Json::FastWriter json_writer;
      json_reader.parse(ioctl_value, json_ioctl, true);
      json_ioctl["rc"] = "0";
      json_ioctl["rg"] = "0";
      json_ioctl["answer_best"] = "";
      json_ioctl["answer_old"] = "";
      json_ioctl["service"] = "";
      json_ioctl["time_interval"] = 0;
      std::string question = json_ioctl["text"].asString();
      if (question.empty()) {
        json_ioctl["ioctl"] = true;
      }
      ioctl_value = json_writer.write(json_ioctl);
      ioctl_value = GetUtility()->ReplaceAllDistinct ( ioctl_value, "\\r\\n", "" );
      YSOS_LOG_DEBUG("CMD_AUDIO_JSON_NLP ioctl_value = " << ioctl_value);
      BufferInterfacePtr input_buffer = GetBufferUtility()->CreateBuffer(ioctl_value);
      if (NULL == input_buffer) {
        YSOS_LOG_ERROR("get CMD_AUDIO_JSON_NLP buffer failed");
        n_return = YSOS_ERROR_FAILED;
        break;
      }
      CallbackInterfacePtr cur_callback_ptr;
      {
        AutoLockOper lock(pre_callback_queue_lock_);
        cur_callback_ptr = pre_callback_queue_->FindCallback(true);
      }
      if (NULL == cur_callback_ptr) {
        YSOS_LOG_ERROR("does not have next callback: " << logic_name_);
        n_return = YSOS_ERROR_FAILED;
        break;
      }
      BaseModuelCallbackContext callback_contex;
      callback_contex.prev_callback_ptr = NULL;
      callback_contex.prev_module_ptr = NULL;
      callback_contex.cur_callback_ptr = cur_callback_ptr.get();
      callback_contex.cur_module_ptr = this;
      n_return = cur_callback_ptr->Callback(input_buffer, NULL, &callback_contex);
      if (YSOS_ERROR_SUCCESS != n_return) {
        YSOS_LOG_ERROR("CMD_AUDIO_JSON_NLP Callback failed");
        break;
      }

      break;
    }

    case CMD_AUDIO_CONTEXT_CLEAR: {
      YSOS_LOG_DEBUG("CMD_AUDIO_CONTEXT_CLEAR in");
      BufferInterfacePtr input_buffer = GetBufferUtility()->CreateBuffer(" ");
      if (NULL == input_buffer) {
        YSOS_LOG_DEBUG("get CMD_AUDIO_CONTEXT_CLEAR buffer failed");
        n_return = YSOS_ERROR_FAILED;
        break;
      }
      driver_prt_->Ioctl(CMD_AUDIO_CONTEXT_CLEAR, input_buffer, NULL);
      n_return = buffer_pool_ptr_->ReleaseBuffer(input_buffer);
      if (n_return != YSOS_ERROR_SUCCESS) {
        YSOS_LOG_DEBUG("execute ReleaseBuffer failed");
        break;
      }

      break;
    }

    case CMD_AUDIO_PARAMETER_SET: {
      YSOS_LOG_DEBUG("CMD_AUDIO_PARAMETER_SET in");
      Json::Value json_ioctl;
      Json::Value json_ioctl2;
      Json::Reader json_reader;
      Json::FastWriter json_writer;
      json_reader.parse(ioctl_value, json_ioctl2, true);
      json_ioctl["rc"] = "0";
      json_ioctl["rg"] = "0";
      json_ioctl["answer_best"] = "";
      json_ioctl["answer_old"] = "";
      json_ioctl["service"] = "";
      json_ioctl["time_interval"] = 0;

      //Json::Reader param_reader;
      //std::string param = json_ioctl2["data"].asString();
      //Json::Value paramget;
      //param_reader.parse(param, paramget, true);
      //Json::Value parameterSet = paramget["parameterSet"];
      //Json::Value robotstateSet = paramget["robotStateSet"];
      //json_ioctl["parameterSet"] = parameterSet;
      //json_ioctl["robotStateSet"] = robotstateSet;

      Json::Value parameterSet = json_ioctl2["data"]["parameterSet"];
      Json::Value robotstateSet = json_ioctl2["data"]["robotStateSet"];
      json_ioctl["parameterSet"] = parameterSet;
      json_ioctl["robotStateSet"] = robotstateSet;

      json_ioctl["ioctl"] = true;
      ioctl_value = json_writer.write(json_ioctl);
      ioctl_value = GetUtility()->ReplaceAllDistinct ( ioctl_value, "\\r\\n", "" );
      YSOS_LOG_DEBUG("CMD_AUDIO_PARAMETER_SET ioctl_value = " << ioctl_value);
      BufferInterfacePtr input_buffer = GetBufferUtility()->CreateBuffer(ioctl_value);
      if (NULL == input_buffer) {
        YSOS_LOG_ERROR("get CMD_AUDIO_PARAMETER_SET buffer failed");
        n_return = YSOS_ERROR_FAILED;
        break;
      }
      CallbackInterfacePtr cur_callback_ptr;
      {
        AutoLockOper lock(pre_callback_queue_lock_);
        cur_callback_ptr = pre_callback_queue_->FindCallback(true);
      }
      if (NULL == cur_callback_ptr) {
        YSOS_LOG_ERROR("does not have next callback: " << logic_name_);
        n_return = YSOS_ERROR_FAILED;
        break;
      }
      BaseModuelCallbackContext callback_contex;
      callback_contex.prev_callback_ptr = NULL;
      callback_contex.prev_module_ptr = NULL;
      callback_contex.cur_callback_ptr = cur_callback_ptr.get();
      callback_contex.cur_module_ptr = this;
      n_return = cur_callback_ptr->Callback(input_buffer, NULL, &callback_contex);
      if (YSOS_ERROR_SUCCESS != n_return) {
        YSOS_LOG_ERROR("CMD_AUDIO_PARAMETER_SET Callback failed");
        break;
      }

      break;
    }

    default: {
      YSOS_LOG_DEBUG("unsupport ioctl, control_id = " << control_id);
      n_return = YSOS_ERROR_FAILED;
      break;
    }
    } //switch
  } while (0);
  ioctl_lock_.Unlock();

  int n_base = BaseModuleImpl::Ioctl(control_id, param);
  if (n_base != YSOS_ERROR_SUCCESS)
    n_return = n_base;

  YSOS_LOG_DEBUG("module Ioctl done");

  return n_return;
}

int NlpExtModule::Flush(LPVOID param /* = nullptr */) {
  return YSOS_ERROR_SUCCESS;
}

int NlpExtModule::Initialize(LPVOID param /* = nullptr */) {
  YSOS_LOG_DEBUG("module Initialize execute");
  int n_return = YSOS_ERROR_FAILED;

  do {
    if (IsInitSucceeded()) {
      n_return = YSOS_ERROR_FINISH_ALREADY;
      break;
    }

    n_return = BaseModuleImpl::Initialize(param);
    if (n_return != YSOS_ERROR_SUCCESS) {
      YSOS_LOG_DEBUG("Initialize BaseThreadModuleImpl failed");
      break;
    }
    n_return = YSOS_ERROR_SUCCESS;
  } while (0);

  YSOS_LOG_DEBUG("module Initialize done");
  return n_return;
}

int NlpExtModule::UnInitialize(LPVOID param /* = nullptr */) {
  return YSOS_ERROR_SUCCESS;
}

int NlpExtModule::GetProperty(int type_id, void *type) {
  int n_return = YSOS_ERROR_SUCCESS;
  int n_result = YSOS_ERROR_SUCCESS;

  do {
    if (type_id == PROP_FUN_CALLABILITY) {
      property_lock_.Lock();
      //< 如果当前状态 是 realstop 状态，则callback输出为空
      FunObjectCommon2* pobject = reinterpret_cast<FunObjectCommon2*>(type);
      if (NULL == pobject) {
        YSOS_LOG_DEBUG("error call property PROP_Fun_CallAbility, the param is null");
        n_return = YSOS_ERROR_FAILED;
        property_lock_.Unlock();
        break;
      }
      AbilityParam* pin = reinterpret_cast<AbilityParam*>(pobject->pparam1);
      AbilityParam* pout = reinterpret_cast<AbilityParam*>(pobject->pparam2);

      {
        //< 如果当前module没有在运行，则输出结果都是空，在能力调以前判断一次，在能力调之后判断一次
        INT32 module_state = 0;
        n_result = GetState(3000, &module_state, NULL);  //< 3秒超时，如果拿不到module的状态，则报错
        if (YSOS_ERROR_SUCCESS != n_result) {
          YSOS_LOG_DEBUG("can't get module status in 3000 miliseconds");
          property_lock_.Unlock();
          n_return = n_result;
          break;
        }
        if (PROP_RUN != module_state || YSOS_ERROR_SUCCESS != n_result) {
          //< 如果当前module没有在运行或获取状态失败，则输出结果都是空，callbck拿到结果为空后则不会向下一个callback处理
          YSOS_LOG_DEBUG("current module is not run, module_state = " << module_state <<", n_result = " << n_result);
          if (pout && pout->buf_interface_ptr) {
            pout->buf_interface_ptr->SetLength(0);  ///<  设置长度为空
          }
          n_return = YSOS_ERROR_SKIP; //< 通知callback ,module已经停了，callback也不要再往下传了
          property_lock_.Unlock();
          break;
        }
      }

      //< 数据类型转换
      if (list_module_datatype_convert_info_.size() > 0) {
        std::string input_datatype = pin ? pin->data_type_name : "";
        std::string output_dataype = pout ? pout->data_type_name :"";
        ModuleDataTypeConvertInfo convert_info;
        if (NeedConvertDataTypeInfo(input_datatype, output_dataype, convert_info)) {
#ifdef _WIN32
          if (convert_info.dest_input_datatype.length() >0 && pin) {
            strcpy_s(pin->data_type_name, sizeof(pin->data_type_name), convert_info.dest_input_datatype.c_str());
          }
          if (convert_info.dest_output_datatype.length() >0 && pout) {
            strcpy_s(pout->data_type_name, sizeof(pout->data_type_name), convert_info.dest_output_datatype.c_str());
          }
#else
          if (convert_info.dest_input_datatype.length() >0 && pin) {
            strcpy(pin->data_type_name, convert_info.dest_input_datatype.c_str());
          }
          if (convert_info.dest_output_datatype.length() >0 && pout) {
            strcpy(pout->data_type_name, convert_info.dest_output_datatype.c_str());
          }
#endif
        }
      }

      //< 这里全权传发
      n_return = driver_prt_->GetProperty(type_id, type);
      YSOS_LOG_DEBUG("driver_prt_->GetProperty n_return = " << n_return);

      {
        //< 如果当前module没有在运行，则输出结果都是空，在能力调以前判断一次，在能力调之后判断一次
        INT32 module_state = 0;
        n_result = GetState(3000, &module_state, NULL);  ///< 3秒超时，如果拿不到module的状态，则报错
        if (YSOS_ERROR_SUCCESS != n_result) {
          YSOS_LOG_DEBUG("can't get module status in 3000 miliseconds");
          property_lock_.Unlock();
          n_return = n_result;
          break;
        }
        YSOS_LOG_DEBUG("================ 1 n_return = " << n_return);
        if (PROP_RUN != module_state || YSOS_ERROR_SUCCESS != n_result) {
          //< 如果当前module没有在运行或获取状态失败，则输出结果都是空，callbck拿到结果为空后则不会向下一个callback处理
          YSOS_LOG_DEBUG("current module is not run, module_state = " << module_state <<", n_result = " << n_result);
          if (pout && pout->buf_interface_ptr) {
            pout->buf_interface_ptr->SetLength(0); //< 设置长度为空
          }
          n_return = YSOS_ERROR_SKIP; //< 通知callback ,module已经停了，callback也不要再往下传了
          property_lock_.Unlock();
          break;
        }
        YSOS_LOG_DEBUG("================ 2 n_return = " << n_return);
      }
      property_lock_.Unlock();
    } else {
      n_result = BaseModuleImpl::GetProperty(type_id, type);
      if (n_result != YSOS_ERROR_SUCCESS) {
        n_return = YSOS_ERROR_FAILED;
        YSOS_LOG_DEBUG("execute BaseThreadModuleImpl GetProperty failed");
        break;
      }
    }
  } while (0);

  YSOS_LOG_DEBUG("NlpExtModule::GetProperty return  = " << n_return);
  return n_return;
}

int NlpExtModule::InitalDataInfo(void) {
  YSOS_LOG_DEBUG("module InitalDataInfo execute");
  int n_return = YSOS_ERROR_SUCCESS;

  YSOS_LOG_DEBUG("module InitalDataInfo done");

  return n_return;
}

int NlpExtModule::RealOpen(LPVOID param /* = nullptr */) {
  YSOS_LOG_DEBUG("module RealOpen execute");
  int n_return = YSOS_ERROR_SUCCESS;

  do {
    driver_prt_ = GetDriverInterfaceManager()->FindInterface(relate_driver_);
    if (NULL == driver_prt_) {
      YSOS_LOG_DEBUG("get driver_prt_ failed");
      n_return = YSOS_ERROR_FAILED;
      break;
    }
    n_return = driver_prt_->Open(param);
    if (YSOS_ERROR_SUCCESS != n_return)
      break;

  } while (0);
  YSOS_LOG_DEBUG("module RealOpen done");

  return n_return;
}

int NlpExtModule::RealClose(void) {
  YSOS_LOG_DEBUG("module RealClose execute");
  int n_return = YSOS_ERROR_SUCCESS;

  do {
    driver_prt_->Close();

  } while (0);

  YSOS_LOG_DEBUG("module RealClose done");

  return n_return;
}

int NlpExtModule::RealPause(void) {
  return YSOS_ERROR_SUCCESS;
}

int NlpExtModule::RealStop(void) {
  return YSOS_ERROR_SUCCESS;
}

int NlpExtModule::RealRun(void) {
  return YSOS_ERROR_SUCCESS;
}

int NlpExtModule::Initialized(const std::string &key, const std::string &value) {
  YSOS_LOG_DEBUG("module Initialized execute");
  int n_return = YSOS_ERROR_FAILED;

  do {
#ifdef _WIN32
    if (stricmp(key.c_str(), "datatype_convert_info") == 0) { //< 数据类型转换
      std::list<std::string> list_params;
      int iret= GetUtility()->SplitString(value, "|", list_params);
      if (YSOS_ERROR_SUCCESS != iret || list_params.size() != 4) {
        YSOS_LOG_DEBUG("error datatype_convert config is error");
        break;
      }

      ModuleDataTypeConvertInfo convert_info;
      std::list<std::string>::iterator it_list_params = list_params.begin();
      convert_info.source_input_datatype = *it_list_params;
      ++it_list_params;
      convert_info.source_output_datatype = *it_list_params;
      ++it_list_params;
      convert_info.dest_input_datatype = *it_list_params;
      ++it_list_params;
      convert_info.dest_output_datatype = *it_list_params;
      list_module_datatype_convert_info_.push_back(convert_info);
    } else if (stricmp(key.c_str(), "output_ability") == 0) { //< 输出能力,如果有多个能力，则使用 ｜ 分开
      string_output_abilities_ = value;
      GetUtility()->SplitString(value, "|", list_output_abilities_);
    } else if (stricmp(key.c_str(), "relate_driver") ==0) {
      relate_driver_ = value;
    }
#else
    if (strcasecmp(key.c_str(), "datatype_convert_info") == 0) { //< 数据类型转换
      std::list<std::string> list_params;
      int iret= GetUtility()->SplitString(value, "|", list_params);
      if (YSOS_ERROR_SUCCESS != iret || list_params.size() != 4) {
        YSOS_LOG_DEBUG("error datatype_convert config is error");
        break;
      }

      ModuleDataTypeConvertInfo convert_info;
      std::list<std::string>::iterator it_list_params = list_params.begin();
      convert_info.source_input_datatype = *it_list_params;
      ++it_list_params;
      convert_info.source_output_datatype = *it_list_params;
      ++it_list_params;
      convert_info.dest_input_datatype = *it_list_params;
      ++it_list_params;
      convert_info.dest_output_datatype = *it_list_params;
      list_module_datatype_convert_info_.push_back(convert_info);
    } else if (strcasecmp(key.c_str(), "output_ability") == 0) { //< 输出能力,如果有多个能力，则使用 ｜ 分开
      string_output_abilities_ = value;
      GetUtility()->SplitString(value, "|", list_output_abilities_);
    } else if (strcasecmp(key.c_str(), "relate_driver") ==0) {
      relate_driver_ = value;
    }
#endif
    YSOS_LOG_DEBUG("output_ability = " << string_output_abilities_ << "; relate_driver = " << relate_driver_);

    n_return = YSOS_ERROR_SUCCESS;
  } while (0);

  YSOS_LOG_DEBUG("module Initialized done");

  return n_return;
}

bool NlpExtModule::NeedConvertDataTypeInfo(const std::string source_input_datatype, const std::string source_output_datatypa, ModuleDataTypeConvertInfo &convert_info) {
  bool result = false;

  do {
    if (source_input_datatype.length() <=0 && source_output_datatypa.length() <=0) {
      YSOS_LOG_DEBUG("param error");
      break;
    }

    std::list<ModuleDataTypeConvertInfo>::iterator it = list_module_datatype_convert_info_.begin();
    for (it; it != list_module_datatype_convert_info_.end(); ++it) {
      ModuleDataTypeConvertInfo* pinfo = &(*it);
#ifdef _WIN32
      if (source_input_datatype.length() > 0 && pinfo->source_input_datatype.length() > 0) {
        if (stricmp(source_input_datatype.c_str(), pinfo->source_input_datatype.c_str()) != 0)
          continue;
      }
      if (source_output_datatypa.length() > 0 && pinfo->source_output_datatype.length() > 0) {
        if (stricmp(source_output_datatypa.c_str(), pinfo->source_output_datatype.c_str()) != 0)
          continue;
      }
#else
      if (source_input_datatype.length() > 0 && pinfo->source_input_datatype.length() > 0) {
        if (strcasecmp(source_input_datatype.c_str(), pinfo->source_input_datatype.c_str()) != 0)
          continue;
      }
      if (source_output_datatypa.length() > 0 && pinfo->source_output_datatype.length() > 0) {
        if (strcasecmp(source_output_datatypa.c_str(), pinfo->source_output_datatype.c_str()) != 0)
          continue;
      }
#endif
      //< 匹配上了
      YSOS_LOG_DEBUG("convert success, source_input = "<< source_input_datatype <<",source_output = " << source_output_datatypa << ",dest_input = " << pinfo->dest_input_datatype << ",dest_output = " << pinfo->dest_output_datatype);
      convert_info = *pinfo;
      result = true;
    }
  } while (0);

  return result;
}

}