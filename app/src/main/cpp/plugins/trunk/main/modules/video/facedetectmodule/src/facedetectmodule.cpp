/**
 *@file facedetectmodule.h
 *@brief Definition of face detect module
 *@version 2.0
 *@author libaoguo
 *@date Created on: 2017-08-25
 *@copyright Copyright(c) 2017 YunShen Technology. All rights reserved.
**/

#include "../include/facedetectmodule.h"

#ifdef _WIN32
  #include <Windows.h>
#else
  //TODO://add for linux
  #include <strings.h>
  #include <sstream>
#endif

// boost headers
#include <boost/thread/thread.hpp>

// ysos headers
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/manager_declare.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"
#include "../../../../../../core/trunk/main/protect/include/os_hal_package/datainterfacemanager.h"

using boost::posix_time::from_iso_string;
using boost::posix_time::ptime;

#define MAX_TOKEN_ID 10000
long token_id = 0;

namespace ysos {

std::string IntToString(long value) {
  std::stringstream stream;
  stream << value;
  return stream.str();
}

DECLARE_PLUGIN_REGISTER_INTERFACE(FaceDetectModule, ModuleInterface);
FaceDetectModule::FaceDetectModule(const std::string &strClassName)
  : PluginBaseModuleImpl(strClassName),
    detect_interval_(200),
    recognize_interval_(2000),
    min_recognize_score_(0.8f),
    min_local_recognize_score_(0.9f),
    min_face_width_(50),
    min_face_height_(50),
    face_detector_("cloud_walk"),
    face_recognizer_("default"),
    enable_local_recognize_(false),
    //is_detect_enable_(true),
    last_detect_time_(from_iso_string("20100101T000000")) {
  logger_ = GetUtility()->GetLogger("ysos.video");
  mp_select_one_min_score_ = 0.8f;
  min_face_score_ = 0.6f;
  last_recognize_time_ = last_detect_time_;
  buffer_ptr_ = GetBufferUtility()->CreateBuffer(64);
}

int FaceDetectModule::Initialized(const std::string &key, const std::string &value) {
#ifdef _WIN32
  if (stricmp(key.c_str(), "mp_select_one_min_score") ==0) {
    mp_select_one_min_score_ = (float)atof(value.c_str());
  } else if (stricmp(key.c_str(), "min_face_score") ==0) {
    min_face_score_ = (float)atof(value.c_str());
  } else if (stricmp(key.c_str(), "need_compare1n") ==0) {
    need_compare1n_ = (atoi(value.c_str()) == 1) ? true : false;
  } else if (stricmp(key.c_str(), "need_imgbase64") ==0) {
    need_imgbase64_ = (atoi(value.c_str()) == 1) ? true : false;
  } else if (stricmp(key.c_str(), "device_id") == 0) {
    device_id_ = value;
    YSOS_LOG_DEBUG("Initalize device id: " << device_id_);
  } else if (stricmp(key.c_str(), "detect_interval") == 0) {
    detect_interval_ = atoi(value.c_str());
    YSOS_LOG_DEBUG("Initalize recognize interval: " << detect_interval_);
  } else if (stricmp(key.c_str(), "recognize_interval") == 0) {
    recognize_interval_ = atoi(value.c_str());
    YSOS_LOG_DEBUG("Initalize recognize interval: " << recognize_interval_);
  } else if (stricmp(key.c_str(), "min_recognize_score") == 0) {
    min_recognize_score_ = (float)atof(value.c_str());
    YSOS_LOG_DEBUG("Initalize min recognize score: " << min_recognize_score_);
  } else if (stricmp(key.c_str(), "min_local_recognize_score") == 0) {
    min_local_recognize_score_ = (float)atof(value.c_str());
    YSOS_LOG_DEBUG("Initalize min local recognize score: " << min_local_recognize_score_);
  } else if (stricmp(key.c_str(), "recognize_server") == 0) {
    recognize_server_= value;
    YSOS_LOG_DEBUG("Initialize recognize server url: " << recognize_server_);
  } else if (stricmp(key.c_str(), "face_detector") == 0) {
    face_detector_ = value;
    YSOS_LOG_DEBUG("Initialize face detector: " << value);
  } else if (stricmp(key.c_str(), "face_recognizer") == 0) {
    face_recognizer_ = value;
    YSOS_LOG_DEBUG("Initialize face recognizer: " << value);
  } else if (stricmp(key.c_str(), "min_face_width") == 0) {
    min_face_width_ = atoi(value.c_str());
    YSOS_LOG_DEBUG("Initalize min face width: " << min_face_width_);
  } else if (stricmp(key.c_str(), "min_face_height") == 0) {
    min_face_height_ = atoi(value.c_str());
    YSOS_LOG_DEBUG("Initalize min face width: " << min_face_height_);
  } else if (stricmp(key.c_str(), "enable_local_recognize") ==0) {
    enable_local_recognize_ = (atoi(value.c_str()) == 1) ? true : false;
  } else {
    return PluginBaseModuleImpl::Initialized(key, value);
  }
#else
  if (strcasecmp(key.c_str(), "mp_select_one_min_score") ==0) {
    mp_select_one_min_score_ = (float)atof(value.c_str());
  } else if (strcasecmp(key.c_str(), "min_face_score") ==0) {
    min_face_score_ = (float)atof(value.c_str());
  } else if (strcasecmp(key.c_str(), "need_compare1n") ==0) {
    need_compare1n_ = (atoi(value.c_str()) == 1) ? true : false;
  } else if (strcasecmp(key.c_str(), "need_imgbase64") ==0) {
    need_imgbase64_ = (atoi(value.c_str()) == 1) ? true : false;
  } else if (strcasecmp(key.c_str(), "device_id") == 0) {
    device_id_ = value;
    YSOS_LOG_DEBUG("Initalize device id: " << device_id_);
  } else if (strcasecmp(key.c_str(), "detect_interval") == 0) {
    detect_interval_ = atoi(value.c_str());
    YSOS_LOG_DEBUG("Initalize recognize interval: " << detect_interval_);
  } else if (strcasecmp(key.c_str(), "recognize_interval") == 0) {
    recognize_interval_ = atoi(value.c_str());
    YSOS_LOG_DEBUG("Initalize recognize interval: " << recognize_interval_);
  } else if (strcasecmp(key.c_str(), "min_recognize_score") == 0) {
    min_recognize_score_ = (float)atof(value.c_str());
    YSOS_LOG_DEBUG("Initalize min recognize score: " << min_recognize_score_);
  } else if (strcasecmp(key.c_str(), "min_local_recognize_score") == 0) {
    min_local_recognize_score_ = (float)atof(value.c_str());
    YSOS_LOG_DEBUG("Initalize min local recognize score: " << min_local_recognize_score_);
  } else if (strcasecmp(key.c_str(), "recognize_server") == 0) {
    recognize_server_= value;
    YSOS_LOG_DEBUG("Initialize recognize server url: " << recognize_server_);
  } else if (strcasecmp(key.c_str(), "face_detector") == 0) {
    face_detector_ = value;
    YSOS_LOG_DEBUG("Initialize face detector: " << value);
  } else if (strcasecmp(key.c_str(), "face_recognizer") == 0) {
    face_recognizer_ = value;
    YSOS_LOG_DEBUG("Initialize face recognizer: " << value);
  } else if (strcasecmp(key.c_str(), "min_face_width") == 0) {
    min_face_width_ = atoi(value.c_str());
    YSOS_LOG_DEBUG("Initalize min face width: " << min_face_width_);
  } else if (strcasecmp(key.c_str(), "min_face_height") == 0) {
    min_face_height_ = atoi(value.c_str());
    YSOS_LOG_DEBUG("Initalize min face width: " << min_face_height_);
  } else if (strcasecmp(key.c_str(), "enable_local_recognize") ==0) {
    enable_local_recognize_ = (atoi(value.c_str()) == 1) ? true : false;
  } else {
    return PluginBaseModuleImpl::Initialized(key, value);
  }
#endif
  return YSOS_ERROR_SUCCESS;
}

int FaceDetectModule::GetProperty(int iTypeId, void *piType) {
  //if (is_detect_enable_) {
  switch (iTypeId) {
  case PROP_FUN_CALLABILITY:
    return OnPropCallAbility2(piType);
  case PROP_FACE_SELECT_ONE:
    return OnPropFaceSelectOne(piType);
  }
  /*} else {
  YSOS_LOG_DEBUG("Not sign in yet.");
  }*/
  return PluginBaseModuleImpl::GetProperty(iTypeId, piType);
}

int FaceDetectModule::Ioctl(INT32 control_id, LPVOID param) {
  //if (!is_detect_enable_) {
  //  YSOS_LOG_DEBUG("Not sign in yet.");
  //  return YSOS_ERROR_SUCCESS;
  //}
  GetBufferUtility()->CopyStringToBuffer(*(std::string *)(param), buffer_ptr_);
  return driver_ptr_->Ioctl(control_id, buffer_ptr_, nullptr);
}

int FaceDetectModule::PickOnePerson(const int &face_number, const TDFaceDetectInfoArray &face_info_arr) {
  int iface_number_pick_one =0;
  for (int i=1; i< face_number; ++i) {
    if (iface_number_pick_one <0) {
      if (IsInExcludeFace(&face_info_arr[i].face_feature)) {
        continue;
      }
      iface_number_pick_one = i;
      continue;
    } else {
      if (face_info_arr[iface_number_pick_one].width*face_info_arr[iface_number_pick_one].hight  <
          face_info_arr[i].width * face_info_arr[i].hight) {
        if (IsInExcludeFace(&face_info_arr[i].face_feature)) {
          continue;
        }
        iface_number_pick_one = i;
      }
    }
  }
  return iface_number_pick_one;
}

bool FaceDetectModule::IsInExcludeFace(const std::string *face_feature) {
  //assert(pcamera_module_);
  //assert(face_feature);
  //if (NULL == face_feature)
  //  return false;
  //if (face_feature->length() <=0) { ///< 没有人脸特征
  //  return false;
  //}
  //{
  //  boost::lock_guard<boost::mutex> lock_guard_mutex_temp(mutex_map_exclude_service_face_zombies_);
  //  float fscore_compare =0;
  //  std::map<int, std::string>::iterator it = map_exclude_service_face_zombies_.begin();
  //  for (it; it != map_exclude_service_face_zombies_.end(); ++it) {
  //    fscore_compare = /*camera_ptr_*/pcamera_module_->TDFaceCompare2((CHAR*)config_terminal_id_.c_str(), (char*)config_org_id_.c_str(),
  //      (char*)it->second.c_str(), it->second.length(), (char*)face_feature->c_str(), face_feature->length());
  //    if (fscore_compare >= config_match_people_11_min_score_) {
  //      ///< 在排除的人里面
  //      return true;
  //    }
  //  }
  //}
  return false;
}

int FaceDetectModule::AbilityFaceRecognition_StreamRGB24P6ToTextJson(
  AbilityParam *input_param, AbilityParam *output_param) {
  if (NULL == input_param->buf_interface_ptr ||
      NULL == output_param->buf_interface_ptr) {
    return YSOS_ERROR_FAILED;
  }
  UINT8 *pdata = NULL;
  UINT32 buffer_size =0;
  assert(input_param->buf_interface_ptr);
  input_param->buf_interface_ptr->GetBufferAndLength(&pdata,&buffer_size);
  assert(buffer_size >=6);
  if (buffer_size <6) {
    return YSOS_ERROR_FAILED;
  }
  TDFaceDetectUnit  face_detect_unit;
  FunObjectCommon2 object_select_one;
  object_select_one.pparam1 = input_param;
  object_select_one.pparam2 = &face_detect_unit;
  int get_face_detect_unig = GetProperty(PROP_FACE_SELECT_ONE, &object_select_one);
  std::string json_return;
  int ret = GenerateFaceInfo(&face_detect_unit, json_return);
  if (YSOS_ERROR_SUCCESS != ret) {
    YSOS_LOG_DEBUG("generate face info failed");
    return YSOS_ERROR_FAILED;
  }
  // out put the data
  UINT8 *pout = NULL;
  UINT32 pout_size =0;
  assert(output_param->buf_interface_ptr);
  output_param->buf_interface_ptr->GetBufferAndLength(&pout, &pout_size);
  UINT32 max_length = 0, prefix_length =0;
  output_param->buf_interface_ptr->GetMaxLength(&max_length);
  output_param->buf_interface_ptr->GetPrefixLength(&prefix_length);
  pout_size = max_length - prefix_length;
  assert(NULL != pout);
  assert(pout_size >= (json_return.length() + 1));
  if (pout_size < (json_return.length() + 1)) {
    YSOS_LOG_DEBUG("error call ability rgb -->face recognition, output buffer is to small");
    return YSOS_ERROR_FAILED;
  }
  if (json_return.length() > 0) {
#ifdef _WIN32
    strcpy_s(reinterpret_cast<char *>(pout),(json_return.length()+1), json_return.c_str());
#else
    strcpy(reinterpret_cast<char *>(pout), json_return.c_str()); //add for linux
#endif
    output_param->buf_interface_ptr->SetLength(json_return.length()+1);
  } else {
    output_param->buf_interface_ptr->SetLength(0);
  }
  YSOS_LOG_DEBUG("face recog json:" << json_return);
  return YSOS_ERROR_SUCCESS;
}

int FaceDetectModule::GenerateFaceInfo(TDFaceDetectUnit *puint, std::string &json_string) {
  Json::Value json_data;
  Json::Value json_face;
  // get the perfect face info if exist
  if (puint && puint->have_perfect_face_ == 1) {
    TDFaceDetectInfoPtr pFaceDetectInfo = puint->face_detect_info_ptr_;
    FaceComparisonResult1NPtr pComarisonResult1N = puint->perfect_face_1n_ptr_;
    int face_num = pComarisonResult1N->vec_faces.size();
    const std::vector<FaceCompareResultPtr> &vec_faces = pComarisonResult1N->vec_faces;
    for (int i = 0; i < face_num; ++i) {
      long tmpLint = (token_id < MAX_TOKEN_ID) ? token_id++ : token_id = 0;
      json_face["token_id"]= IntToString(tmpLint);  //add for linux long int to int
      json_face["person_id"]= vec_faces[i]->person_id.c_str();
      json_face["gender"]= atoi(vec_faces[i]->sex.c_str());
      json_face["age"]= atoi(vec_faces[i]->age.c_str());
      json_face["person_name"]= vec_faces[i]->person_name;
      json_face["img_base64"]= puint->face_detect_info_ptr_->img_base64_data;
      // only get the highest score face, default the first
      json_data.append(json_face);
      break;
    }
  } else {
    // get the face property info if exist
    if (puint && puint->face_detect_info_ptr_) {
      long tmpLint = (token_id < MAX_TOKEN_ID) ? token_id++ : token_id = 0;
      json_face["token_id"]= IntToString(tmpLint);  //add for linux long int to int
      json_face["person_id"]= "";
      json_face["gender"]= atoi(puint->face_detect_info_ptr_->gender.c_str());
      json_face["age"]= puint->face_detect_info_ptr_->age;
      json_face["person_name"]= "";
      json_face["img_base64"]= puint->face_detect_info_ptr_->img_base64_data;
      json_data.append(json_face);
    }
  }
  Json::Value json_body;
  json_body["face_num"] = json_data.size();
  json_body["face"] = json_data;
  Json::Value json_root;
  json_root["type"] = "face_info";
  json_root["data"] = json_body;
  Json::FastWriter writer;
  json_string = writer.write(json_root);
  json_string = GetUtility()->ReplaceAllDistinct ( json_string, "\\r\\n", "" );
  return YSOS_ERROR_SUCCESS;
}

int FaceDetectModule::RealOpen(LPVOID param) {
  int ret = PluginBaseModuleImpl::RealOpen(param);
  if (ret != YSOS_ERROR_SUCCESS) {
    return ret;
  }
  driver_ptr_->SetProperty(PROP_FACE_MIN_SCORE, &min_face_score_);
  driver_ptr_->SetProperty(PROP_FACE_RECOGNIZE_INTERVAL, &recognize_interval_);
  driver_ptr_->SetProperty(PROP_MIN_RECOGNIZE_SCORE, &min_recognize_score_);
  driver_ptr_->SetProperty(PROP_MIN_LOCAL_RECOGNIZE_SCORE,
                           &min_local_recognize_score_);
  driver_ptr_->SetProperty(PROP_FACE_DETECTOR, (void *)face_detector_.c_str());
  driver_ptr_->SetProperty(PROP_FACE_RECOGNIZER, (void *)face_recognizer_.c_str());
  driver_ptr_->SetProperty(PROP_RECOGNIZE_SERVER, (void *)recognize_server_.c_str());
  driver_ptr_->SetProperty(PROP_MIN_FACE_WIDTH, &min_face_width_);
  driver_ptr_->SetProperty(PROP_MIN_FACE_HEIGHT, &min_face_height_);
  driver_ptr_->SetProperty(PROP_ENABLE_LOCAL_RECOGNIZE, &enable_local_recognize_);
  return YSOS_ERROR_SUCCESS;
}

int FaceDetectModule::RealRun() {
  int ret = PluginBaseModuleImpl::RealRun();
  if (ret != YSOS_ERROR_SUCCESS) {
    return ret;
  }
  DataInterfacePtr data_ptr = GetDataInterfaceManager()->GetData(GLOBAL_DATA_KEY);
  std::string org_id;
  std::string term_id;
  //TODO:add for linux  4 debug
  //* 
  data_ptr->SetData("signin_orgid", "000000006");
  data_ptr->SetData("signin_termid", "00000001");
  //*/
  data_ptr->GetData("orgId", org_id);          ///< 直接使用终端号机构号，没必要还得签到成功才能用。 wff modify 20180508
  data_ptr->GetData("termId", term_id);
  YSOS_LOG_INFO("Initialize orgid: " << org_id);
  YSOS_LOG_INFO("Initialize termid: " << term_id);
  device_id_ = term_id;
  driver_ptr_->SetProperty(PROP_ORG_ID, (void *)org_id.c_str());
  driver_ptr_->SetProperty(PROP_TERM_ID, (void *)term_id.c_str());
  //is_detect_enable_ = !org_id.empty();
  return YSOS_ERROR_SUCCESS;
}

void FaceDetectModule::GenerateRecognizeResult(
  const FaceComparisonResult1N &result, AbilityParam *pout) {
  std::string json_string;
  Json::Value json_data;
  Json::Value json_face;
  int face_num = result.vec_faces.size();
  const std::vector<FaceCompareResultPtr> &vec_faces = result.vec_faces;
  for (int i = 0; i < face_num; ++i) {
    long tmpLint = (token_id < MAX_TOKEN_ID) ? token_id++ : token_id = 0;
    json_face["token_id"] = IntToString(tmpLint);  //add for linux long int to int
    json_face["gender"] = atoi(vec_faces[i]->sex.c_str());
    json_face["age"] = atoi(vec_faces[i]->age.c_str());
    json_face["type"] = vec_faces[i]->type;
    json_face["person_name"] = vec_faces[i]->person_name;
    json_face["id_card"] = vec_faces[i]->id_card;
    json_face["track_id"] = atoi(vec_faces[i]->person_id.c_str());
    json_data.append(json_face);
  }
  Json::Value json_body;
  json_body["face_num"] = json_data.size();
  json_body["face"] = json_data;
  json_body["recognition"] = 1;
  Json::Value json_root;
  json_root["type"] = "face_info";
  json_root["data"] = json_body;
  Json::FastWriter writer;
  json_string = writer.write(json_root);
  json_string = GetUtility()->ReplaceAllDistinct ( json_string, "\\r\\n", "" );
  YSOS_LOG_DEBUG("face recognize json:" << json_string);
  if (json_string.length() > 0) {
    UINT8 *buf = NULL;
    UINT32 pout_size = 0;
    pout->buf_interface_ptr->GetBufferAndLength(&buf, &pout_size);
#ifdef _WIN32
    strcpy_s(reinterpret_cast<char *>(buf), (json_string.length() + 1), json_string.c_str());
#else
    strcpy(reinterpret_cast<char *>(buf), json_string.c_str());// add for linux
#endif
    pout->buf_interface_ptr->SetLength(json_string.length() + 1);
  } else {
    pout->buf_interface_ptr->SetLength(0);
  }
}

int FaceDetectModule::DetectFace( AbilityParam *input_param, AbilityParam *output_param) {
  // 检查人脸检测的时间间隔
  ptime time_now = boost::posix_time::microsec_clock::local_time();
  int64_t time_elapse = (time_now - last_detect_time_).total_milliseconds();
  YSOS_LOG_DEBUG("Last face detect time: " << last_detect_time_ << ". Time elapse: " << time_elapse << ". Detect interval: " << detect_interval_);
  if (time_elapse < detect_interval_) {
    YSOS_LOG_DEBUG("Face detection cancelled because of too often.");
    return YSOS_ERROR_FAILED;
  }
  last_detect_time_ = time_now;
  if (NULL == input_param || NULL == output_param) {
    assert(false);
    YSOS_LOG_ERROR("FaceDetectModule::OnPropFaceSelectOne error: in/out param is null");
    return YSOS_ERROR_FAILED;
  }
#ifdef _WIN32
  if (stricmp(input_param->data_type_name, DTSTR_STREAM_FRAME_RGB24_P6) != 0) {
    //assert(false);
    YSOS_LOG_ERROR("Wrong ability name:" << input_param->ability_name);
    return YSOS_ERROR_FAILED;
  }
#else
  if (strcasecmp(input_param->data_type_name, DTSTR_STREAM_FRAME_RGB24_P6) != 0) {
    assert(false);
    YSOS_LOG_ERROR("Wrong ability name:" << input_param->ability_name);
    return YSOS_ERROR_FAILED;
  }
#endif
  //get current frame all people;
  uint8_t *data = GetBufferUtility()->GetBufferData(input_param->buf_interface_ptr);
  if (NULL == data) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }
  unsigned short width = *(unsigned short *)(data + 2);
  unsigned short height = *(unsigned short *)(data + 4);
  uint8_t *frame_data = data + 6;
  FaceDetectRequest face_detect_rquest;
  face_detect_rquest.frame_data = frame_data;
  face_detect_rquest.frame_widht = width;
  face_detect_rquest.frame_height = height;
  face_detect_rquest.frame_channels = 3;
  face_detect_rquest.get_face_property = true;
  face_detect_rquest.get_face_property_ex = false;
  face_detect_rquest.get_face_image_base64 = true;
  face_detect_rquest.get_face_feature = false;
  face_detect_rquest.terminal_id = device_id_;
  FaceDetectInfoGroup face_detect_result;
  FunObjectCommon2 call_object;
  call_object.pparam1 = &face_detect_rquest;
  call_object.pparam2 = &face_detect_result;
  // 人脸检测
  YSOS_LOG_ERROR("PROP_FACE_DETECT_ASYNC begin");
  int iret = driver_ptr_->GetProperty(PROP_FACE_DETECT_ASYNC, &call_object);
  YSOS_LOG_ERROR("PROP_FACE_DETECT_ASYNC end. Return: " << iret);
  if (YSOS_ERROR_SUCCESS != iret) {
    return iret;
  }
  std::string json_string;
  Json::Value json_data;
  Json::Value json_face;
  int face_count = face_detect_result.face_number;
  TDFaceDetectInfoArray arr_face_info = face_detect_result.arr_face_info;
  for (int i = 0; i < face_count; ++i) {
    //add for linux  
    //TODO: long transfer string
    long tmpLint = (token_id < MAX_TOKEN_ID) ? token_id++ : token_id = 0;
    json_face["token_id"] = IntToString(tmpLint); //add for linux long int to int 
    json_face["gender"] = atoi(arr_face_info[i].gender.c_str());
    json_face["age"] = arr_face_info[i].age;
    //json_face["x_center"] = arr_face_info[i].x + arr_face_info[i].width / 2;
    float x_rate =
      (arr_face_info[i].x + arr_face_info[i].width / 2.f - width / 2) / width * 2;
    json_face["x_rate"] = -x_rate;
    json_face["track_id"] = arr_face_info[i].trackid;
    json_face["rate"] =
      arr_face_info[i].width * arr_face_info[i].hight / (float)width / height;
    json_data.append(json_face);
  }
  Json::Value json_body;
  //json_body["face_num"] = json_data.size();
  // 公版检测结果不上抛
  json_body["face_num"] = face_count;
  json_body["face"] = json_data;
  json_body["recognition"] = 0;
  Json::Value json_root;
  json_root["type"] = "face_info";
  json_root["data"] = json_body;
  Json::FastWriter writer;
  json_string = writer.write(json_root);
  json_string = GetUtility()->ReplaceAllDistinct ( json_string, "\\r\\n", "" );
  YSOS_LOG_DEBUG("face detect json:" << json_string);
  if (json_string.length() > 0) {
    UINT8 *buf = NULL;
    UINT32 pout_size = 0;
    assert(output_param->buf_interface_ptr);
    output_param->buf_interface_ptr->GetBufferAndLength(&buf, &pout_size);
#ifdef _WIN32    
    strcpy_s(reinterpret_cast<char *>(buf), (json_string.length() + 1), json_string.c_str());
#else
    strcpy(reinterpret_cast<char *>(buf), json_string.c_str());// add for linux
#endif
    output_param->buf_interface_ptr->SetLength(json_string.length() + 1);
  } else {
    output_param->buf_interface_ptr->SetLength(0);
  }
  return YSOS_ERROR_SUCCESS;
}

int FaceDetectModule::OnPropCallAbility(void *piType) {
  FunObjectCommon2 *object = reinterpret_cast<FunObjectCommon2 *>(piType);
  if (NULL == object) {
    YSOS_LOG_DEBUG("FaceDetectModule::OnPropCallAbility error: null param");
    return YSOS_ERROR_FAILED;
  }
  AbilityParam *pin = reinterpret_cast<AbilityParam *>(object->pparam1);
  AbilityParam *pout = reinterpret_cast<AbilityParam *>(object->pparam2);
  if (NULL == pin || NULL == pout) {
    YSOS_LOG_DEBUG("FaceDetectModule::OnPropCallAbility error: null in/out param");
    return YSOS_ERROR_FAILED;
  }
#ifdef _WIN32
  if (stricmp(ABSTR_FACERECOGNITION, pout->ability_name) != 0) {
    YSOS_LOG_DEBUG("FaceDetectModule::OnPropCallAbility error: wrong ability name");
    return YSOS_ERROR_FAILED;
  }
#else
  if (strcasecmp(ABSTR_FACERECOGNITION, pout->ability_name) != 0) {
    YSOS_LOG_DEBUG("FaceDetectModule::OnPropCallAbility error: wrong ability name");
    return YSOS_ERROR_FAILED;
  }
#endif
  // 如果 当前module 没有在运行，则输出结果都是空
  // 在能力调以前 判断一次，在能力调之后判断一次
  INT32 module_state = 0;
  ///<  3秒超时，如果拿不到module的状态，则报错
  int igetstate_ret = GetState(3000, &module_state, NULL);
  if (YSOS_ERROR_SUCCESS != igetstate_ret) {
    YSOS_LOG_DEBUG("error my cant get module status in 3000 miliseconds");
    //assert(false);
  }
  // 如果 当前module 没有在运行 或获取状态失败，则输出结果都是空
  // callbck拿到结果为空后则不会向下一个callback处理
  if (PROP_RUN != module_state || YSOS_ERROR_SUCCESS != igetstate_ret) {
    YSOS_LOG_DEBUG("current is not run (current state:%" << module_state << " ) getsatatus fun return:" << igetstate_ret);
    if (pout && pout->buf_interface_ptr) {
      pout->buf_interface_ptr->SetLength(0);
    }
    return YSOS_ERROR_SKIP;
  }
  return AbilityFaceRecognition_StreamRGB24P6ToTextJson(pin, pout);
}

int FaceDetectModule::OnPropCallAbility2(void *piType) {
  YSOS_LOG_DEBUG("FaceDetectModule::OnPropCallAbility2 [Enter]");
  FunObjectCommon2 *object = reinterpret_cast<FunObjectCommon2 *>(piType);
  if (NULL == object) {
    YSOS_LOG_DEBUG("FaceDetectModule::OnPropCallAbility error: null param");
    return YSOS_ERROR_FAILED;
  }
  AbilityParam *pin = reinterpret_cast<AbilityParam *>(object->pparam1);
  AbilityParam *pout = reinterpret_cast<AbilityParam *>(object->pparam2);
  if (NULL == pin || NULL == pout) {
    YSOS_LOG_DEBUG("FaceDetectModule::OnPropCallAbility error: null in/out param");
    return YSOS_ERROR_FAILED;
  }
#ifdef _WIN32
  {
    /// wff add ，为发兼容低配置，添加一个判断，不影响公版处理
    if (stricmp(ABSTR_FACEDETECT_RECOGNITION, pout->ability_name) ==0) {
      // 这里全权传发给driver。
      if (driver_ptr_) {
        int driver_ret =  driver_ptr_->GetProperty(PROP_FUN_CALLABILITY, piType);
        return driver_ret;
      }
    }
  }
  if (stricmp(ABSTR_FACERECOGNITION, pout->ability_name) != 0) {
    YSOS_LOG_DEBUG("FaceDetectModule::OnPropCallAbility error: wrong ability name");
    return YSOS_ERROR_FAILED;
  }
#else
  {
    YSOS_LOG_DEBUG("FaceDetectModule::OnPropCallAbility2 [Check][0]");
    YSOS_LOG_DEBUG("FaceDetectModule::OnPropCallAbility2 [pout->ability_name]: " << pout->ability_name);
    /// wff add ，为发兼容低配置，添加一个判断，不影响公版处理
    if (strcasecmp(ABSTR_FACEDETECT_RECOGNITION, pout->ability_name) ==0) {
      // 这里全权传发给driver。
      YSOS_LOG_DEBUG("FaceDetectModule::OnPropCallAbility2 [Check][1]");
      if (driver_ptr_) {
        int driver_ret =  driver_ptr_->GetProperty(PROP_FUN_CALLABILITY, piType);
        YSOS_LOG_DEBUG("FaceDetectModule::OnPropCallAbility2 [driver_ret]: " << driver_ret);
        return driver_ret;
      }
    }
  }
   YSOS_LOG_DEBUG("FaceDetectModule::OnPropCallAbility2 [Check][2]");
  if (strcasecmp(ABSTR_FACERECOGNITION, pout->ability_name) != 0) {
    YSOS_LOG_DEBUG("FaceDetectModule::OnPropCallAbility error: wrong ability name");
    return YSOS_ERROR_FAILED;
  }
   YSOS_LOG_DEBUG("FaceDetectModule::OnPropCallAbility2 [Check][3]");
#endif
  // 如果 当前module 没有在运行，则输出结果都是空
  // 在能力调以前 判断一次，在能力调之后判断一次
  INT32 module_state = 0;
  ///<  3秒超时，如果拿不到module的状态，则报错
  int igetstate_ret = GetState(3000, &module_state, NULL);
  if (YSOS_ERROR_SUCCESS != igetstate_ret) {
    YSOS_LOG_DEBUG("error my cant get module status in 3000 miliseconds");
    //assert(false);
  }
  YSOS_LOG_DEBUG("FaceDetectModule::OnPropCallAbility2 [igetstate_ret]: " << igetstate_ret);
  // 如果 当前module 没有在运行 或获取状态失败，则输出结果都是空
  // callbck拿到结果为空后则不会向下一个callback处理
  if (PROP_RUN != module_state || YSOS_ERROR_SUCCESS != igetstate_ret) {
    YSOS_LOG_DEBUG("current is not run (current state:%" << module_state << " ) getsatatus fun return:" << igetstate_ret);
    if (pout && pout->buf_interface_ptr) {
      pout->buf_interface_ptr->SetLength(0);
    }
    return YSOS_ERROR_SKIP;
  }
  YSOS_LOG_DEBUG("FaceDetectModule::OnPropCallAbility2 [Check][3]");
  // 获取人脸识别数据, 有的话直接返回
  FaceComparisonResult1N result;
  int ret = driver_ptr_->GetProperty(PROP_FACE_RECOGNIZE, &result);
  if (ret == YSOS_ERROR_SUCCESS) {
    YSOS_LOG_DEBUG("Get face recognize result.");
    GenerateRecognizeResult(result, pout);
    return ret;
  }
  YSOS_LOG_DEBUG("FaceDetectModule::OnPropCallAbility2 [Check][3][ret]="<< ret);
  YSOS_LOG_DEBUG("FaceDetectModule::OnPropCallAbility2 [Check][3][ret]="<< result.vec_faces.size());
  YSOS_LOG_DEBUG("FaceDetectModule::OnPropCallAbility2 [Check][4]");
  // 获取人脸1比1结果
  // 0: track id; 1: 是不是同一个人
  int compare_result[2];
  ret = driver_ptr_->GetProperty(PROP_FACE_COMPARE, &compare_result);
  if (ret == YSOS_ERROR_SUCCESS) {
    YSOS_LOG_DEBUG("Get face compare result.");
    std::string json_string;
    Json::Value json_body;
    json_body["track_id"] = compare_result[0];
    json_body["recognition"] = 2;
    json_body["result"] = compare_result[1];
    Json::Value json_root;
    json_root["type"] = "face_info";
    json_root["data"] = json_body;
    Json::FastWriter writer;
    json_string = writer.write(json_root);
    json_string = GetUtility()->ReplaceAllDistinct ( json_string, "\\r\\n", "" );
    YSOS_LOG_DEBUG("face compare json:" << json_string);
    if (json_string.length() > 0) {
      UINT8 *buf = NULL;
      UINT32 pout_size = 0;
      pout->buf_interface_ptr->GetBufferAndLength(&buf, &pout_size);
#ifdef _WIN32      
      strcpy_s(reinterpret_cast<char *>(buf), (json_string.length() + 1), json_string.c_str());
#else
      strcpy(reinterpret_cast<char *>(buf), json_string.c_str());//add for linux
#endif
      pout->buf_interface_ptr->SetLength(json_string.length() + 1);
    } else {
      pout->buf_interface_ptr->SetLength(0);
    }
    return ret;
  }
  YSOS_LOG_DEBUG("FaceDetectModule::OnPropCallAbility2 [Check][4][ret]="<< ret);
  YSOS_LOG_DEBUG("FaceDetectModule::OnPropCallAbility2 [Check][5]");
  YSOS_LOG_DEBUG("FaceDetectModule::OnPropCallAbility2 [End]");
  // 没有人脸识别数据，进行人脸检测
  return DetectFace(pin, pout);
}

int FaceDetectModule::OnPropFaceSelectOne(void *piType) {
  FunObjectCommon2 *object = reinterpret_cast<FunObjectCommon2 *>(piType);
  if (NULL == object) {
    YSOS_LOG_DEBUG("FaceDetectModule::OnPropFaceSelectOne error: null param");
    return YSOS_ERROR_FAILED;
  }
  // 检查人脸检测的时间间隔
  ptime time_now = boost::posix_time::microsec_clock::local_time();
  int64_t time_elapse = (time_now - last_detect_time_).total_milliseconds();
  YSOS_LOG_DEBUG("Last face detect time: " << last_detect_time_ <<
                 ". Time elapse: " << time_elapse <<
                 ". Detect interval: " << detect_interval_);
  if (time_elapse < detect_interval_) {
    YSOS_LOG_DEBUG("Face detection cancelled because of too often.");
    return YSOS_ERROR_FAILED;
  }
  last_detect_time_ = time_now;
  AbilityParam *pin = reinterpret_cast<AbilityParam *>(object->pparam1);
  TDFaceDetectUnit *pout = reinterpret_cast<TDFaceDetectUnit *>(object->pparam2);
  if (NULL == pin || NULL == pout) {
    assert(false);
    YSOS_LOG_ERROR("FaceDetectModule::OnPropFaceSelectOne error: in/out param is null");
    return YSOS_ERROR_FAILED;
  }
#ifdef _WIN32
  if (stricmp(pin->data_type_name, DTSTR_STREAM_FRAME_RGB24_P6) != 0) {
    //assert(false);
    YSOS_LOG_ERROR("Wrong ability name:" << pin->ability_name);
    return YSOS_ERROR_FAILED;
  }
#else
  if (strcasecmp(pin->data_type_name, DTSTR_STREAM_FRAME_RGB24_P6) != 0) {
    //assert(false);
    YSOS_LOG_ERROR("Wrong ability name:" << pin->ability_name);
    return YSOS_ERROR_FAILED;
  }
#endif
  //get current frame all people;
  uint8_t *data = GetBufferUtility()->GetBufferData(pin->buf_interface_ptr);
  if (NULL == data) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }
  unsigned short *width = (unsigned short *)(data + 2);
  unsigned short *height = (unsigned short *)(data + 4);
  uint8_t *frame_data = data + 6;
  FaceDetectRequest face_detect_rquest;
  face_detect_rquest.frame_data = frame_data;
  face_detect_rquest.frame_widht = *width;
  face_detect_rquest.frame_height = *height;
  //face_detect_rquest.frame_type = 0; //CV_8UC3;
  face_detect_rquest.frame_channels = 3;
  face_detect_rquest.get_face_property = true;
  face_detect_rquest.get_face_property_ex = false;
  face_detect_rquest.get_face_image_base64 = true;
  face_detect_rquest.get_face_feature = true;
  FaceDetectInfoGroup face_detect_result;
  FunObjectCommon2 call_object;
  call_object.pparam1 = &face_detect_rquest;
  call_object.pparam2 = &face_detect_result;
  YSOS_LOG_ERROR("PROP_FACE_DETECT begin");
  // 人脸检测
  int iret = driver_ptr_->GetProperty(PROP_FACE_DETECT, &call_object);
  YSOS_LOG_ERROR("PROP_FACE_DETECT end. Return:" << iret);
  if (YSOS_ERROR_SUCCESS != iret) {
    return iret;
  }
  pout->ClearContent();
  std::string has_face;
  DataInterfacePtr data_ptr_ =
    GetDataInterfaceManager()->GetData(GLOBAL_DATA_KEY);
  int face_number = face_detect_result.face_number;
  TDFaceDetectInfoArray arr_face_info = face_detect_result.arr_face_info;
  // 检测到人脸进行识别
  if (face_number > 0 && arr_face_info) {
    int iface_number_pick_one = PickOnePerson(face_number, arr_face_info);
    if (iface_number_pick_one >= 0) {
      pout->face_detect_info_ptr_.reset(new TDFaceDetectInfo);
      *(pout->face_detect_info_ptr_.get()) =
        arr_face_info[iface_number_pick_one];
      // set person age gender information
      if (data_ptr_ && pout->face_detect_info_ptr_) {
        data_ptr_->SetData("age", IntToString(pout->face_detect_info_ptr_->age));
        data_ptr_->SetData("gender", pout->face_detect_info_ptr_->gender);
      }
      // 数据太多，去掉facefeature数据
      if (pout->face_detect_info_ptr_) {
        pout->face_detect_info_ptr_->face_feature = "";
      }
      // 数据太多，去掉facebaser64数据
      if (pout->face_detect_info_ptr_ && !need_imgbase64_) {
        pout->face_detect_info_ptr_->img_base64_data = "";
      }
      if (need_compare1n_ &&
          arr_face_info[iface_number_pick_one].face_feature.length() > 0) {
        FaceComparision1nRequest face_compare1n_request;
        face_compare1n_request.terminalid = "";
        face_compare1n_request.orgid = "";
        face_compare1n_request.topn = 2;
        face_compare1n_request.facefeature =
          arr_face_info[iface_number_pick_one].img_base64_data;
        // get terminal id , orgid
        if (data_ptr_) {
          data_ptr_->GetData("signin_orgid", face_compare1n_request.orgid);
          data_ptr_->GetData("signin_termid", face_compare1n_request.terminalid);
          YSOS_LOG_ERROR("PROP_FACE_COMPARISON_1N_V1 signin_orgid: " <<
                         face_compare1n_request.orgid);
          YSOS_LOG_ERROR("PROP_FACE_COMPARISON_1N_V1 signin_termid: " <<
                         face_compare1n_request.terminalid);
        }
        // 检查人脸识别的时间间隔
        time_elapse =
          (time_now - last_recognize_time_).total_milliseconds();
        YSOS_LOG_DEBUG("Last face recognize time: " << last_detect_time_ <<
                       ". Time elapse: " << time_elapse <<
                       ". Recognize interval: " << recognize_interval_);
        if (time_elapse > recognize_interval_) {
          last_recognize_time_ = time_now;
          pout->perfect_face_1n_ptr_.reset(new FaceComparisonResult1N);
          FunObjectCommon2 object_1n;
          object_1n.pparam1 = &face_compare1n_request;
          object_1n.pparam2 = pout->perfect_face_1n_ptr_.get();
          YSOS_LOG_ERROR("PROP_FACE_COMPARISON_1N_V1 begin");
          int get_ret = driver_ptr_->GetProperty(
                          PROP_FACE_COMPARISON_1N_V1_CCB, &object_1n);
          YSOS_LOG_ERROR("PROP_FACE_COMPARISON_1N_V1 end");
          if (YSOS_ERROR_SUCCESS == get_ret) {
            if (pout->perfect_face_1n_ptr_->vec_faces.size() > 0) {
              pout->have_perfect_face_ = true;
            }
          } else {
            YSOS_LOG_ERROR("PROP_FACE_COMPARISON_1N_V1 failed");
          }
        } else {
          YSOS_LOG_DEBUG("Face recognize cancelled because of too often.");
        }
        // 如果低于0.8分，则表示没有这个人, 后期这个0.x过滤
        if (pout->have_perfect_face_ && pout->perfect_face_1n_ptr_ &&
            pout->perfect_face_1n_ptr_->vec_faces.size() > 0 &&
            (float)atof(pout->perfect_face_1n_ptr_->vec_faces[0]->score.c_str()) <
            mp_select_one_min_score_) {
          pout->have_perfect_face_ = false;
          pout->perfect_face_1n_ptr_ = NULL;
        }
        if (data_ptr_ && pout->have_perfect_face_) {
          data_ptr_->SetData("person_name",
                             pout->perfect_face_1n_ptr_->vec_faces[0]->person_name);
          data_ptr_->SetData("person_mobile",
                             pout->perfect_face_1n_ptr_->vec_faces[0]->mobile);
          data_ptr_->SetData("is_admin",
                             pout->perfect_face_1n_ptr_->vec_faces[0]->is_admin);
        } else if (data_ptr_) {
          data_ptr_->SetData("person_name", "");
        }
      }
    }
    has_face = "1";
  } else {
    has_face = "0";
  }
  // set if has face in current frame
  if (data_ptr_) {
    data_ptr_->SetData("has_face", has_face);
  }
  return YSOS_ERROR_SUCCESS;
}

}