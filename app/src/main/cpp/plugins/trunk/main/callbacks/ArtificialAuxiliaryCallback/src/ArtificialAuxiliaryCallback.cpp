/**
 *@file ArtificialAuxiliaryCallback.cpp
 *@brief Artificial Auxiliary callback
 *@version 1.0
 *@author l.j..
 *@date Created on: 2016-10-20 15:16:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
/// self headers //  NOLINT
#include "../include/ArtificialAuxiliaryCallback.h"

/// windows headers
//#include <Windows.h>

/// third haders
#include <json/json.h>

/// private headers
#include "../../../public/include/common/commonenumdefine.h"


namespace ysos {
DECLARE_PLUGIN_REGISTER_INTERFACE(ArtificialAuxiliaryCallback, CallbackInterface);  ///<  插件的入口，一定要加上 //  NOLINT
ArtificialAuxiliaryCallback::ArtificialAuxiliaryCallback(const std::string &strClassName):BaseThreadModuleCallbackImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.ArtificialAuxiliary");
  data_ptr_ = GetDataInterfaceManager()->GetData(GLOBAL_DATA_KEY);
}

ArtificialAuxiliaryCallback::~ArtificialAuxiliaryCallback(void) {

}

int ArtificialAuxiliaryCallback::IsReady(void) {
  return YSOS_ERROR_SUCCESS;
}

int ArtificialAuxiliaryCallback::RealCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr cur_output_buffer, BufferInterfacePtr output_buffer, void *context) {
  int n_return = YSOS_ERROR_SUCCESS;
  YSOS_LOG_DEBUG("callback RealCallback execute");

  BaseModuelCallbackContext* module_callback_context_ptr = (BaseModuelCallbackContext*)context;
  if (NULL == module_callback_context_ptr) {
    YSOS_LOG_DEBUG("get module_callback_context_ptr failed");
    return YSOS_ERROR_FAILED;
  }

  BaseModuleImpl* base_module_ptr = reinterpret_cast<BaseModuleImpl*>(module_callback_context_ptr->cur_module_ptr);
  if (NULL == base_module_ptr) {
    YSOS_LOG_DEBUG("get base_module_ptr failed");
    return YSOS_ERROR_FAILED;
  }

  ///< 发送坐标等相关信息
  SendPositionInfo(base_module_ptr);

  n_return = base_module_ptr->GetProperty(CMD_ARTIFICIAL_AUXILIARY_RECV, &cur_output_buffer);   //< 获取数据
  if (n_return != YSOS_ERROR_SUCCESS) {
    YSOS_LOG_DEBUG("socket receive data failed");
    return n_return;;
  }

  YSOS_LOG_DEBUG("callback RealCallback done");
  return n_return;
}

int ArtificialAuxiliaryCallback::SendPositionInfo(ModuleInterface *module_ptr) {
  assert(NULL != module_ptr);
  if (NULL == data_ptr_) {
    YSOS_LOG_ERROR("send position failed, no data ptr: " << logic_name_);
    return YSOS_ERROR_FAILED;
  }

  Json::Reader reader;
  Json::FastWriter writer;

  int x,y,z,angle;
  std::string key = "position";
  std::string value;
  data_ptr_->GetData(key, value);
  Json::Value pos_info;
  if (!value.empty()) {
    reader.parse(value, pos_info, false);
  }
  YSOS_LOG_DEBUG("SendPositionInfo value = " << value);

  Json::Value real_json;
  real_json["type"] = "artificial_auxiliary_event";
  real_json["data"] = pos_info;
  std::string strWrite = writer.write(real_json);
  strWrite = GetUtility()->ReplaceAllDistinct ( strWrite, "\\r\\n", "" );
  YSOS_LOG_DEBUG("SendPositionInfo strWrite = " << strWrite);

  int ret = module_ptr->Ioctl(CMD_ARTIFICIAL_AUXILIARY_SEND, &strWrite);   //< 发送信息
  if (ret != YSOS_ERROR_SUCCESS) {
    YSOS_LOG_DEBUG("socket send info failed");
  }

  std::string token_id;
  data_ptr_->GetData("token_id", token_id);
  YSOS_LOG_DEBUG("token_id = " << token_id);
  static std::string pre_token_id = token_id;
  YSOS_LOG_DEBUG("pre_token_id = " << pre_token_id);
  std::string cur_state;
  data_ptr_->GetData("machine_state", cur_state);
  YSOS_LOG_DEBUG("cur_state = " << cur_state);
  static std::string pre_cur_state = cur_state;
  YSOS_LOG_ERROR("pre_cur_state = " << pre_cur_state);
  if (!(token_id == pre_token_id && cur_state == pre_cur_state)) {
    int pos = 0;
    Json::Value state_info;
    state_info["robotState"] = cur_state;
    state_info["stateLevel"] = 0;
    Json::Value state_set;
    state_set[pos] = state_info;
    Json::Value robot_state_set;
    robot_state_set["robotStateSet"] = state_set;
    robot_state_set["newToken"] = token_id;
    robot_state_set["lifeTime"] = 570000;
    robot_state_set["platForm"] = "1";
    Json::Value json_message;
    json_message["messageType"] = "TokenAndScene";
    json_message["messageContent"] = robot_state_set;
    Json::Value json_robot;
    json_robot["data"] = json_message;
    strWrite = writer.write(json_robot);
    strWrite = GetUtility()->ReplaceAllDistinct ( strWrite, "\\r\\n", "" );
    YSOS_LOG_DEBUG("strWrite = " << strWrite);
    int ret = module_ptr->Ioctl(CMD_ARTIFICIAL_AUXILIARY_SEND, &strWrite);   //< 发送信息
    if (ret != YSOS_ERROR_SUCCESS) {
      YSOS_LOG_DEBUG("socket send info failed");
    } else {
      pre_token_id = token_id;
      pre_cur_state = cur_state;
    }

  }

  return YSOS_ERROR_SUCCESS;
}

}