/**
 *@file ArtificialAuxiliaryModule.cpp
 *@brief Artificial Auxiliary module
 *@version 1.0
 *@author l.j..
 *@date Created on: 2016-10-20 14:50:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
/// Private Headers //  NOLINT
#include "../include/ArtificialAuxiliaryModule.h"

#ifdef _WIN32
/// OS Headers
#include <Windows.h>
#else

#endif

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/filesystem.hpp>

#include <json/json.h>

#ifndef SLEEP
#define SLEEP(a)      (boost::this_thread::sleep_for(boost::chrono::milliseconds(a)))
#endif

#define PI 3.14159265

namespace ysos {
DECLARE_PLUGIN_REGISTER_INTERFACE(ArtificialAuxiliaryModule, ModuleInterface);  ///<  插件的入口，一定要加上 //  NOLINT

ArtificialAuxiliaryModule::ArtificialAuxiliaryModule(const std::string &strClassName) : BaseThreadModuleImpl(strClassName) {
  thread_data_->timeout = 300;
  logger_ = GetUtility()->GetLogger("ysos.ArtificialAuxiliary");
}

ArtificialAuxiliaryModule::~ArtificialAuxiliaryModule(void) {

}

int ArtificialAuxiliaryModule::GetProperty(int type_id, void *type) {
  int n_return = YSOS_ERROR_SUCCESS;
  int n_result = -1;

  do {
    std::string str_result = "";
    Json::Reader js_reader;
    Json::FastWriter js_writer;
    Json::Value js_value;
    Json::Value js_out_value;

    if (type_id == CMD_ARTIFICIAL_AUXILIARY_RECV) {
      YSOS_LOG_DEBUG("GetProperty CMD_ARTIFICIAL_AUXILIARY_RECV execute");

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

      std::string temp_str;
      GetReceiveString(temp_str);
      if (0 == temp_str.length()) {
        n_return = YSOS_ERROR_FAILED;
        break;
      }
      YSOS_LOG_DEBUG("the saved string is:" << temp_str);
      std::string str_msg_type;
      js_reader.parse(temp_str, js_value, true);
      str_msg_type = js_value["messageType"].asString();
      YSOS_LOG_DEBUG("GetProperty str_msg_type = " << str_msg_type);
      if (str_msg_type.empty()) {
        n_return = YSOS_ERROR_FAILED;
        break;
      } else if (str_msg_type.compare("voice") == 0) { ///<  修改“voice”接口
        std::string str_msg_content = js_value["messageContent"].asString();
        std::string temp1 = str_msg_content.substr(0,8);
        if (temp1.compare("【语义】") == 0) { ///<  语义解析
          std::string temp2 = str_msg_content.substr(8);
          js_value["messageType"] = "voice_analysis";
          js_value["messageContent"] = temp2.c_str();
          js_out_value["type"] = "artificial_auxiliary_event";
          js_out_value["data"] = js_value;
          str_result = js_writer.write(js_out_value);
          str_result = GetUtility()->ReplaceAllDistinct ( str_result, "\\r\\n", "" );
        } else {  ///<  直接播报
          js_value["messageType"] = "voice_broadcast";
          js_out_value["type"] = "artificial_auxiliary_event";
          js_out_value["data"] = js_value;
          str_result = js_writer.write(js_out_value);
          str_result = GetUtility()->ReplaceAllDistinct ( str_result, "\\r\\n", "" );
        }
      } else if (str_msg_type.compare("testWalkToAddPos") == 0) {
        Json::Value temp_json_value = js_value["messageContent"];
        int in_x = temp_json_value["x"].asInt();
        int in_y = temp_json_value["y"].asInt();
        std::string temp_angle = temp_json_value["z"].asString();
        float in_angle = atof(temp_angle.c_str());
        int out_x = 0, out_y = 0;
        float out_angle = 0;
        MapCoordinatesConvert(in_x, in_y, in_angle, out_x, out_y, out_angle);
        Json::Value temp_content;
        temp_content["x"] = out_x;
        temp_content["y"] = out_y;
        temp_content["z"] = out_angle;
        js_value["messageContent"] = temp_content;
        js_out_value["type"] = "artificial_auxiliary_event";
        js_out_value["data"] = js_value;
        str_result = js_writer.write(js_out_value);
        str_result = GetUtility()->ReplaceAllDistinct ( str_result, "\\r\\n", "" );
      } else {   ///<  其他的直接传
        js_out_value["type"] = "artificial_auxiliary_event";
        js_out_value["data"] = js_value;
        str_result = js_writer.write(js_out_value);
        str_result = GetUtility()->ReplaceAllDistinct ( str_result, "\\r\\n", "" );
      }
      memcpy(buffer, str_result.c_str(),str_result.length());
      YSOS_LOG_DEBUG("the sendto string is:" << buffer);
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

int ArtificialAuxiliaryModule::Ioctl(INT32 control_id, LPVOID param /* = nullptr */) {
  YSOS_LOG_DEBUG("module Ioctl execute");
  int n_return = YSOS_ERROR_SUCCESS;

  if (NULL == driver_prt_) {
    YSOS_LOG_ERROR("driver is null: " << logic_name_);
    return YSOS_ERROR_FAILED;
  }

  do {
    BufferInterfacePtr in_buffer_ptr;
    std::string str_param = "";
    Json::Reader js_reader;
    Json::Value js_value;
    Json::Value js_send_value;
    Json::FastWriter js_writer;
    std::string str_send = "";

    n_return = buffer_pool_ptr_->GetBuffer(&in_buffer_ptr);
    if (YSOS_ERROR_SUCCESS!= n_return) {
      YSOS_LOG_DEBUG("GetBuffer error");
      break;
    }
    //< 需要修改
    in_buffer_ptr->SetLength(module_data_info_->buffer_length);

    UINT8* buffer;
    UINT32 buffer_size;
    n_return = in_buffer_ptr->GetBufferAndLength(&buffer, &buffer_size);
    if (YSOS_ERROR_SUCCESS != n_return) {
      YSOS_LOG_DEBUG("GetBufferAndLength error");
      break;
    }

    lock_.Lock();
    memset(buffer, 0, buffer_size);
    switch (control_id) {
    case CMD_ARTIFICIAL_AUXILIARY_SEND: {
      str_param = *(reinterpret_cast<std::string*>(param));
      js_reader.parse(str_param, js_value, true);
      js_send_value = js_value["data"];
      str_send = js_writer.write(js_send_value);
      str_send = GetUtility()->ReplaceAllDistinct ( str_send, "\\r\\n", "" );
      YSOS_LOG_DEBUG("the send string is:" << str_send);
      std::string str_send_urf8 = string_To_UTF8(str_send);
      memcpy(buffer, str_send_urf8.c_str(), str_send_urf8.length());
      YSOS_LOG_DEBUG("the real send string is:" << buffer);
      n_return = driver_prt_->Ioctl(CMD_ARTIFICIAL_AUXILIARY_SEND, in_buffer_ptr, nullptr);
      break;
    }

    case CMD_ARTIFICIAL_AUXILIARY_LOGIN: {
      driver_prt_->Ioctl(control_id, NULL, NULL);
      break;
    }

    case CMD_ARTIFICIAL_AUXILIARY_LOGOUT: {
      driver_prt_->Ioctl(control_id, NULL, NULL);
      break;
    }

    case CMD_ARTIFICIAL_AUXILIARY_CALL: {
      YSOS_LOG_DEBUG("module CMD_ARTIFICIAL_AUXILIARY_CALL in");
      str_param = *(reinterpret_cast<std::string*>(param));
      js_reader.parse(str_param, js_value, true);
      js_send_value = js_value["data"];
      str_send = js_writer.write(js_send_value);
      str_send = GetUtility()->ReplaceAllDistinct ( str_send, "\\r\\n", "" );
      YSOS_LOG_DEBUG("the send string is:" << str_send);
      std::string str_send_urf8 = string_To_UTF8(str_send);
      memcpy(buffer, str_send_urf8.c_str(), str_send_urf8.length());
      YSOS_LOG_DEBUG("the real send string is:" << buffer);
      n_return = driver_prt_->Ioctl(CMD_ARTIFICIAL_AUXILIARY_SEND, in_buffer_ptr, nullptr);
      ///< 发送坐席协助请求
      memset(buffer, 0, buffer_size);
      js_send_value.clear();
      js_send_value["messageType"] = "robotRequestSeatHelp";
      js_send_value["messageContent"] = "";
      js_send_value["platform_type"] = "1"; ///< 新平台
      str_send = js_writer.write(js_send_value);
      str_send = GetUtility()->ReplaceAllDistinct ( str_send, "\\r\\n", "" );
      YSOS_LOG_DEBUG("the send string is:" << str_send);
      str_send_urf8 = string_To_UTF8(str_send);
      memcpy(buffer, str_send_urf8.c_str(), str_send_urf8.length());
      YSOS_LOG_DEBUG("the real send string is:" << buffer);
      n_return = driver_prt_->Ioctl(CMD_ARTIFICIAL_AUXILIARY_SEND, in_buffer_ptr, nullptr);
      break;
    }

    case CMD_ARTIFICIAL_AUXILIARY_HANGUP: {
      YSOS_LOG_DEBUG("module CMD_ARTIFICIAL_AUXILIARY_HANGUP in");
      str_param = *(reinterpret_cast<std::string*>(param));
      js_reader.parse(str_param, js_value, true);
      js_send_value = js_value["data"];
      str_send = js_writer.write(js_send_value);
      memcpy(buffer, str_send.c_str(), str_send.length());
      YSOS_LOG_DEBUG("the real send string is:" << buffer);
      n_return = driver_prt_->Ioctl(CMD_ARTIFICIAL_AUXILIARY_SEND, in_buffer_ptr, nullptr);
      ///< 发送坐席取消协助请求
      memset(buffer, 0, buffer_size);
      js_send_value.clear();
      js_send_value["messageType"] = "robotRequestSeatOver";
      js_send_value["messageContent"] = "";
      str_send = js_writer.write(js_send_value);
      str_send = GetUtility()->ReplaceAllDistinct ( str_send, "\\r\\n", "" );
      YSOS_LOG_DEBUG("the send string is:" << str_send);
      std::string str_send_urf8 = string_To_UTF8(str_send);
      memcpy(buffer, str_send_urf8.c_str(), str_send_urf8.length());
      YSOS_LOG_DEBUG("the real send string is:" << buffer);
      n_return = driver_prt_->Ioctl(CMD_ARTIFICIAL_AUXILIARY_SEND, in_buffer_ptr, nullptr);
      break;
    }
    case CMD_ARTIFICIAL_AUXILIARY_PASS: {
      YSOS_LOG_INFO("module CMD_ARTIFICIAL_AUXILIARY_PASS in");
      str_param = *(reinterpret_cast<std::string*>(param));
      YSOS_LOG_DEBUG("the send string is:" << str_param);
      memcpy(buffer, str_param.c_str(), str_param.length());
      n_return = driver_prt_->Ioctl(CMD_ARTIFICIAL_AUXILIARY_SEND, in_buffer_ptr, nullptr);
      break;
    }
    default: {
      YSOS_LOG_DEBUG("Ioctl id error, control_id = " << control_id);
      n_return = YSOS_ERROR_FAILED;
      break;
    }
    } //switch

    n_return = buffer_pool_ptr_->ReleaseBuffer(in_buffer_ptr);
    if (n_return != YSOS_ERROR_SUCCESS) {
      YSOS_LOG_DEBUG("execute ReleaseBuffer failed");
      break;
    }
    lock_.Unlock();
  } while (0);

  int n_base = BaseModuleImpl::Ioctl(control_id, param);
  if (n_base != YSOS_ERROR_SUCCESS)
    n_return = n_base;

  YSOS_LOG_DEBUG("module Ioctl done");
  return n_return;
}

int ArtificialAuxiliaryModule::Flush(LPVOID param /* = nullptr */) {
  return YSOS_ERROR_SUCCESS;
}

int ArtificialAuxiliaryModule::Initialize(LPVOID param /* = nullptr */) {
  YSOS_LOG_DEBUG("module Initialize execute");
  int n_return = YSOS_ERROR_SUCCESS;

  n_return = BaseThreadModuleImpl::Initialize(param);
  if (n_return != YSOS_ERROR_SUCCESS) {
    YSOS_LOG_DEBUG("Initialize BaseThreadModuleImpl Initialize failed: " << logic_name_);
    return YSOS_ERROR_FAILED;
  }

  std::string data_path = GetPackageConfigImpl()->GetConfPath() + "..\\data";
  if (!boost::filesystem::exists(data_path))
    boost::filesystem::create_directory(data_path);
  data_path = GetPackageConfigImpl()->GetConfPath() + "..\\data\\ArtificialAuxiliaryModule";
  if (!boost::filesystem::exists(data_path))
    boost::filesystem::create_directory(data_path);

  if (driver_.empty()) {
    driver_ = "default@ArtificialAuxiliaryDriver";
  }
  YSOS_LOG_DEBUG("ArtificialAuxiliaryModule driver is " << driver_ << " | " << logic_name_);
  //ysos_sdk\Public\conf\instance\driver_instance.xml
  driver_prt_ = ysos::GetDriverInterfaceManager()->FindInterface(driver_);
  if (NULL == driver_prt_) {
    YSOS_LOG_ERROR("get driver failed: default@ArtificialAuxiliaryDriver | " << logic_name_);
    return YSOS_ERROR_FAILED;
  }

  YSOS_LOG_DEBUG("module Initialize done");
  return n_return;
}

int ArtificialAuxiliaryModule::UnInitialize(LPVOID param /* = nullptr */) {
  return YSOS_ERROR_SUCCESS;
}

int ArtificialAuxiliaryModule::InitalDataInfo(void) {
  YSOS_LOG_DEBUG("module InitalDataInfo execute");
  int n_return = YSOS_ERROR_SUCCESS;

  YSOS_LOG_DEBUG("module InitalDataInfo done");
  return n_return;
}

int ArtificialAuxiliaryModule::RealOpen(LPVOID param /* = nullptr */) {
  YSOS_LOG_DEBUG("module RealOpen execute");
  int n_return = YSOS_ERROR_SUCCESS;

  if (NULL == driver_prt_) {
    YSOS_LOG_ERROR("driver is null: " << logic_name_);
    return YSOS_ERROR_FAILED;
  }

  do {
    n_return = driver_prt_->Open(param);
    if (n_return != YSOS_ERROR_SUCCESS)
      break;

    // start receive thread
    receive_thread_ = boost::thread(boost::bind(Receive_Thread,this));

    ///< 发送坐席链接请求
    Json::Value send_msg;
    send_msg["messageType"] = "robotRequestSeatHelp";
    send_msg["messageContent"] = "";
    send_msg["platform_type"] = "1"; ///< 新平台
    Json::Value real_json;
    real_json["type"] = "artificial_auxiliary_event";
    real_json["data"] = send_msg;
    Json::FastWriter writer;
    std::string strWrite = writer.write(real_json);
    strWrite = GetUtility()->ReplaceAllDistinct ( strWrite, "\\r\\n", "" );
    n_return = Ioctl(CMD_ARTIFICIAL_AUXILIARY_SEND,&strWrite);
    if (n_return != YSOS_ERROR_SUCCESS)
      break;

    n_return = BaseThreadModuleImpl::RealOpen(param);
    if (n_return != YSOS_ERROR_SUCCESS)
      break;
  } while (0);

  YSOS_LOG_DEBUG("module RealOpen done");
  return n_return;
}

int ArtificialAuxiliaryModule::RealClose(void) {
  YSOS_LOG_DEBUG("module RealClose execute");
  int n_return = YSOS_ERROR_SUCCESS;

  do {
    ///< 停止坐席链接请求
    Json::Value send_msg;
    send_msg["messageType"] = "robotRequestSeatOver";
    send_msg["messageContent"] = "";
    Json::Value real_json;
    real_json["type"] = "artificial_auxiliary_event";
    real_json["data"] = send_msg;
    Json::FastWriter writer;
    std::string strWrite = writer.write(real_json);
    strWrite = GetUtility()->ReplaceAllDistinct ( strWrite, "\\r\\n", "" );
    n_return = Ioctl(CMD_ARTIFICIAL_AUXILIARY_SEND,&strWrite);
    if (n_return != YSOS_ERROR_SUCCESS) {
      YSOS_LOG_DEBUG("robotRequestSeatOver fail");
    }

    // stop receive thread
    receive_thread_.interrupt();
    receive_thread_.join();

    driver_prt_->Close();

    n_return = BaseThreadModuleImpl::RealClose();
    if (n_return != YSOS_ERROR_SUCCESS)
      break;
  } while (0);

  YSOS_LOG_DEBUG("module RealClose done");
  return n_return;
}

int ArtificialAuxiliaryModule::RealPause(void) {
  return YSOS_ERROR_SUCCESS;
}

int ArtificialAuxiliaryModule::RealStop(void) {
  return YSOS_ERROR_SUCCESS;
}

int ArtificialAuxiliaryModule::RealRun(void) {
  return YSOS_ERROR_SUCCESS;
}

int ArtificialAuxiliaryModule::Initialized(const std::string &key, const std::string &value) {
  YSOS_LOG_DEBUG("module Initialized execute");
  int n_return = YSOS_ERROR_SUCCESS;

  do {
    if (key.compare("map_convert_type") == 0) {
      map_convert_type_ = atoi(value.c_str());
      YSOS_LOG_DEBUG("the map_convert_type_ is:" << map_convert_type_);
    } else if (key.compare("show_map_width") == 0) {
      show_map_width_ = atoi(value.c_str());
      YSOS_LOG_DEBUG("the show_map_width_ is:" << show_map_width_);
    } else if (key.compare("chassis_map_height") == 0) {
      chassis_map_height_ = atoi(value.c_str());
      YSOS_LOG_DEBUG("the chassis_map_height_ is:" << chassis_map_height_);
    } else if (key.compare("map_convert_point_angle") == 0) {
      std::string temp_str = value;
      sscanf(value.c_str(),"%d,%d,%f",&center_point_x_,&center_point_y_,&center_point_angle_);
      YSOS_LOG_DEBUG("the center point is:" << center_point_x_ << center_point_y_ << center_point_angle_);
    }
  } while (0);

  YSOS_LOG_DEBUG("module Initialized done");
  return n_return;
}

void ArtificialAuxiliaryModule::SetReceiveString(std::string in_str) {
  boost::lock_guard<boost::mutex> autolock(receive_mutex_);
  receive_list_.push_back(in_str);
}

void ArtificialAuxiliaryModule::GetReceiveString(std::string& out_str) {
  boost::lock_guard<boost::mutex> autolock(receive_mutex_);
  if (receive_list_.size() > 0) {
    out_str = *receive_list_.begin();
    receive_list_.pop_front();
  }
}

int ArtificialAuxiliaryModule::Receive_Thread(LPVOID lpParam) {
  ArtificialAuxiliaryModule *artificialauxiliarmodule_ptr = (ArtificialAuxiliaryModule *)lpParam;
  if (NULL==artificialauxiliarmodule_ptr) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  int n_return = YSOS_ERROR_SUCCESS;
  int n_result;

  BufferInterfacePtr out_buffer_ptr;

  n_return = artificialauxiliarmodule_ptr->buffer_pool_ptr_->GetBuffer(&out_buffer_ptr);
  UINT8* buffer;
  UINT32 buffer_size;
  n_return = out_buffer_ptr->GetBufferAndLength(&buffer, &buffer_size);

  YSOS_LOG_DEBUG_CUSTOM(artificialauxiliarmodule_ptr->logger_, "Receive_Thread start running");

  while (true) {
    boost::this_thread::interruption_point();
    SLEEP(300);
    n_result = artificialauxiliarmodule_ptr->driver_prt_->Ioctl(CMD_ARTIFICIAL_AUXILIARY_RECV, nullptr, out_buffer_ptr);
    if (YSOS_ERROR_SUCCESS != n_result) {
      continue;
    }

    YSOS_LOG_DEBUG_CUSTOM(artificialauxiliarmodule_ptr->logger_, "the receive string is:" << buffer);
    std::string str_recv_utf8 = reinterpret_cast<char *>(buffer);
    std::string str_recv = artificialauxiliarmodule_ptr->UTF8_To_string(str_recv_utf8);
    YSOS_LOG_DEBUG_CUSTOM(artificialauxiliarmodule_ptr->logger_, "the real receive string is:" << str_recv);
#ifdef _DEBUGVIEW
    ::OutputDebugString((std::string("ysos.ArtificialAuxiliary receive string:") + str_recv).c_str());
#endif
    artificialauxiliarmodule_ptr->SetReceiveString(str_recv);
  }

  YSOS_LOG_DEBUG_CUSTOM(artificialauxiliarmodule_ptr->logger_, "Receive_Thread end running");
  return YSOS_ERROR_SUCCESS;
}

std::string ArtificialAuxiliaryModule::string_To_UTF8(const std::string & str) {
#ifdef _WIN32
  int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);

  wchar_t * pwBuf = new wchar_t[nwLen + 1];//一定要加1，不然会出现尾巴
  ZeroMemory(pwBuf, nwLen * 2 + 2);

  ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);

  int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

  char * pBuf = new char[nLen + 1];
  ZeroMemory(pBuf, nLen + 1);

  ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

  std::string retStr(pBuf);

  delete []pwBuf;
  delete []pBuf;

  pwBuf = NULL;
  pBuf = NULL;

  return retStr;
#else
  std::wstring wstr = StringToWstring(str.c_str());
  return GetUtility()->UnicodeToUtf8(wstr);
#endif
}

std::string ArtificialAuxiliaryModule::UTF8_To_string(const std::string & str) {
#ifdef _WIN32
  int nwLen = ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);

  wchar_t * pwBuf = new wchar_t[nwLen + 1];//一定要加1，不然会出现尾巴
  memset(pwBuf, 0, nwLen * 2 + 2);

  ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), pwBuf, nwLen);

  int nLen = ::WideCharToMultiByte(CP_ACP, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

  char * pBuf = new char[nLen + 1];
  memset(pBuf, 0, nLen + 1);

  ::WideCharToMultiByte(CP_ACP, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

  std::string retStr = pBuf;

  delete []pBuf;
  delete []pwBuf;

  pBuf = NULL;
  pwBuf = NULL;

  return retStr;
#else
  std::wstring wstr = GetUtility()->Utf8ToUnicode(str);
  return WstringToString(wstr.c_str());
#endif
}

void ArtificialAuxiliaryModule::MapCoordinatesConvert(int in_x, int in_y, float in_angle, int& out_x, int& out_y, float& out_angle) {
  YSOS_LOG_DEBUG("the input point is:" << in_x << in_y << in_angle);
  int temp_x = 0, temp_y = 0;
  if (map_convert_type_ == 1) {
    temp_x = in_x;
    temp_y = -in_y;
  } else if (map_convert_type_ == 2) {
    temp_x = show_map_width_ - in_y;
    temp_y = -in_x;
  }

  float angle_radian = center_point_angle_ * PI / 180;
  out_x = temp_x * cos(angle_radian) + temp_y * sin(angle_radian) + center_point_x_;
  out_y = -1 * (-temp_x * sin(angle_radian) + temp_y * cos(angle_radian)) + center_point_y_;
  out_y = chassis_map_height_ - out_y;
  out_angle = in_angle;
  YSOS_LOG_DEBUG("the output point is:" << out_x << out_y << out_angle);
}

std::wstring ArtificialAuxiliaryModule::StringToWstring(const char *pc)
{
  std::wstring val = L"";

	if(NULL == pc)
	{
		return val;
	}
	//size_t size_of_ch = strlen(pc)*sizeof(char);
	//size_t size_of_wc = get_wchar_size(pc);
	size_t size_of_wc;
	size_t destlen = mbstowcs(0, pc, 0);
	if (destlen ==(size_t)(-1))
	{
		return val;
	}
	size_of_wc = destlen+1;
	wchar_t * pw  = new wchar_t[size_of_wc];
	mbstowcs(pw, pc, size_of_wc);
	val = pw;
	delete pw;
	return val;
}

std::string ArtificialAuxiliaryModule::WstringToString(const wchar_t * pw)
{
  std::string val = "";
	if(!pw)
	{
   		return val;
	}
	size_t size= wcslen(pw)*sizeof(wchar_t);
	char *pc = NULL;
	if(!(pc = (char*)malloc(size)))
	{
   		return val;
	}
	size_t destlen = wcstombs(pc,pw,size);
	/*转换不为空时，返回值为-1。如果为空，返回值0*/
	if (destlen ==(size_t)(0))
	{
		return val;
	}
	val = pc;
	delete pc;
	return val;
}

}