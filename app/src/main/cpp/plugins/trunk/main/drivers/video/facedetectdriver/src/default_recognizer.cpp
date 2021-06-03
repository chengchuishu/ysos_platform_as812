/**
 *@file default_recognizer.cpp
 *@brief 公版人脸识别实现文件
 *@version 1.0
 *@author WangDaqian
 *@date Created on: 2018/1/19
 *@copyright Copyright(c) 2018 YunShen Technology. All rights reserved.
**/


#include "../include/default_recognizer.h"

#include "../include/common_struct.h"
#include "../include/face_module_config.h"
#include <boost/locale.hpp>
#include <exception>
#include "../../../public/include/httpclient/httpclient.h"
#include <json/json.h>


/// cloudwalk sdk headers
//#include <cloudwalk_sdk/Include/CWFaceAttribute.h>
//#include <cloudwalk_sdk/Include/CWFaceConfig.h>
//#include <cloudwalk_sdk/Include/CWFaceDetection.h>
//#include <cloudwalk_sdk/Include/CWFaceRecognition.h>
//#include <cloudwalk_sdk/Include/CWFaceVersion.h>

using boost::posix_time::from_iso_string;
using boost::posix_time::ptime;
using boost::posix_time::time_duration;

#define MAX_DEFAULT_TIMEOUT_MS 1


namespace ysos {

DefaultRecognizer::DefaultRecognizer()
    : is_working_(false),
      is_thread_quit_(false),
      is_result_returned_(true),
      local_face_infos_(10),
      last_recognize_time_(from_iso_string("20080101T000000")),
      logger_(GetUtility()->GetLogger("ysos.video")) {
  thread_.reset(new boost::thread(boost::bind(&DefaultRecognizer::ThreadFunc, this)));
#ifdef _WIN32
  work_event_ = CreateEvent(NULL, FALSE, FALSE, NULL);
  recognize_handle_file_ = "..\\data\\facedetectdriver\\CWModels\\CWR_Config_1_1.xml";
#else
  work_event_ = event_create(false, false);//replace old process
  //recognize_handle_file_ = "..\\data\\facedetectdriver\\CWModels\\CWR_Config_1_1.xml";
#endif
  Init();
}

DefaultRecognizer::~DefaultRecognizer() {
  Uninit();
}

void DefaultRecognizer::Recognize(
    std::vector<boost::shared_ptr<FaceInfo> > &params) {
      YSOS_LOG_DEBUG("DefaultRecognizer::Recognize()");
  if (is_working_) {
    YSOS_LOG_DEBUG("FaceRecognizeHelper is working when called.");
    return;
  }
  
  // 有结果没返回, 不进行识别
  if (!is_result_returned_) {
    YSOS_LOG_DEBUG("Recognize result has not returned.");
    return;
  }

  if (params.size() == 0) {
    YSOS_LOG_DEBUG("Recognize param size is 0.");
    return;
  }

  // 检查人脸识别测的时间间隔
  int interval = FaceModuleConfig::GetInstance()->GetRecognizeInterval();
  ptime time_now = boost::posix_time::microsec_clock::local_time();
  int64_t time_elapse =
      (time_now - last_recognize_time_).total_milliseconds();
  YSOS_LOG_DEBUG("Last face recognize time: " << last_recognize_time_ <<
                 ". Time elapse: " << time_elapse <<
                 ". Recognize interval: " << interval);
  if (time_elapse < interval) {
    YSOS_LOG_DEBUG("Face recognize cancelled because of too often.");
    return;
  }

  // 选最大人脸. 前面判断过params的长度
  int max = 0;
  int max_size = params[0]->width * params[0]->height;
  for (size_t i = 1; i < params.size(); ++i) {
    int size = params[i]->width * params[i]->height;
    if (size > max_size) {
      max = i;
      max_size = size;
    }
  }
  recog_face_ = params[max];

  if (last_recog_face_.get() &&
      last_recog_face_->track_id == recog_face_->track_id) {
    recog_face_->name = last_recog_face_->name;
    recog_face_->id_no = last_recog_face_->id_no;
    recog_face_->age = last_recog_face_->age;
    recog_face_->gender = last_recog_face_->gender;
    is_result_returned_ = false;
    YSOS_LOG_DEBUG("Recognize by track id: " << recog_face_->track_id
                       << ", name: " << recog_face_->name);
  } else {
    last_recognize_time_ = time_now;
    is_working_ = true;

#ifdef _WIN32
    SetEvent(work_event_);
#else
      //TODO:add for linux
      //set event handleid
      event_set(work_event_);
#endif
  }
}

bool DefaultRecognizer::GetRecognizeResult(
    std::vector<boost::shared_ptr<FaceInfo> > &params) {
       YSOS_LOG_DEBUG("DefaultRecognizer::GetRecognizeResult()");
  // 结果已经返回
  if (is_result_returned_)
    return false;

  boost::lock_guard<boost::mutex> guard(result_lock_);
  params.clear();
  if (recog_face_.get())
    params.push_back(recog_face_);
  is_result_returned_ = true;
  return true;
}

void DefaultRecognizer::Init() {
  YSOS_LOG_DEBUG("DefaultRecognizer::Init()");
#ifdef _WIn32
  cw_errcode_t errCode = CW_SDKLIT_OK;
  recog_handle_ = cwCreateRecogHandle(&errCode, recognize_handle_file_.c_str(), 0, CW_FEATURE_EXTRACT);
  if (recog_handle_ == 0)
    YSOS_LOG_ERROR("Create recognize handle failed. Error: " << errCode);
#else
// TODO:add for linux
#endif
}

void DefaultRecognizer::Uninit() {
  YSOS_LOG_DEBUG("DefaultRecognizer::Uninit()");
#ifdef _WIN32
  if (recog_handle_) {
    cwReleaseRecogHandle(recog_handle_);
    recog_handle_ = 0;
  }
#else
// TODO:add for linux
#endif
}

void DefaultRecognizer::ThreadFunc() {
  YSOS_LOG_DEBUG("Recognize thread start.");

#ifdef _WIN32
  while (!is_thread_quit_) {
    if (WaitForSingleObject(work_event_, INFINITE) != WAIT_OBJECT_0)
      break;

    YSOS_LOG_DEBUG("Recognize thread work.");
    if (FaceModuleConfig::GetInstance()->IsLocalRecognizeEnabled() && LocalRecognize())
      is_result_returned_ = false;
    RequestRecognize();

    is_working_ = false;
    is_result_returned_ = false;
  }
#else
  
#endif
}

bool DefaultRecognizer::LocalRecognize() {
   YSOS_LOG_DEBUG("DefaultRecognizer::LocalRecognize()");
#ifdef _WIN32
  // 获取人脸的特征
  if (recog_face_->ex_data == 0 ||
      recog_face_->ex_length != sizeof(cw_aligned_face_t)) {
    YSOS_LOG_ERROR("Not cloud walk aligned face(cw_aligned_face_t).");
    return false;
  }
  cw_errcode_t errCode = CW_SDKLIT_OK;
  int feature_len = cwGetFeatureLength(recog_handle_);

  if (recog_face_->prob_feature.size() != feature_len) {
    recog_face_->prob_feature.resize(feature_len);
    errCode = cwGetProbeFeature(
        recog_handle_, (cw_aligned_face_t*)recog_face_->ex_data,
        &(recog_face_->prob_feature[0]));
    if (CW_SDKLIT_OK != errCode) {
      YSOS_LOG_DEBUG("Get face prob feature failed.");
      recog_face_->prob_feature.clear();
    }
  }

  float score = 0.f;
  for (int j = 0; j < local_face_infos_.Length(); ++j) {
    boost::shared_ptr<FaceInfo> &face = local_face_infos_.Get(j);
    errCode = cwComputeMatchScore(
        recog_handle_, recog_face_->prob_feature.data(),
        recog_face_->prob_feature.size(), 1,
        face->filed_feature.data(), face->filed_feature.size(), 1, &score);
    if (errCode != CW_SDKLIT_OK) {
      YSOS_LOG_DEBUG("cwComputeMatchScore error: " << errCode);
      return false;
    }

    YSOS_LOG_INFO("Local compared with " << face->name << ", score: " << score);
    if (score > FaceModuleConfig::GetInstance()->GetMinLocalRecognizeScore()) {
      boost::lock_guard<boost::mutex> guard(result_lock_);
      recog_face_->similarity = score;
      recog_face_->id_no = face->id_no;
      recog_face_->name = face->name;
      recog_face_->age = face->age;
      recog_face_->gender = face->gender;
      YSOS_LOG_INFO("Local recognize " << face->name << ", score: " << score);
      return true;
    }
  }
  return false;
#else
// TODO:add for linux
  return true;
#endif
}

void DefaultRecognizer::RequestRecognize() {
  YSOS_LOG_DEBUG("DefaultRecognizer::RequestRecognize()");
#ifdef _WIN32
  std::string recognize_url =
      FaceModuleConfig::GetInstance()->GetRecognizeServer();
  if (recognize_url.empty()) {
    YSOS_LOG_ERROR("Recognize server url is empty.");
    return;
  }

  /// 构造发送Json数据
  Json::Value json_request;
  Json::Value req_head(Json::objectValue);
  Json::Value req_body(Json::objectValue);
  Json::Value req_base64(Json::arrayValue);
  req_body["termId"] = FaceModuleConfig::GetInstance()->GetTermID();
  req_body["orgId"] = FaceModuleConfig::GetInstance()->GetOrgID();
  req_body["topN"] = "1";
  req_base64.append(recog_face_->image_base64);
  req_body["datas"] = req_base64;
  json_request["REQ_HEAD"] = req_head;
  json_request["REQ_BODY"] = req_body;
  Json::FastWriter writer;
  std::string request_data = writer.write(json_request);
  request_data = GetUtility()->ReplaceAllDistinct ( request_data, "\\r\\n", "" );
  request_data = "REQ_MESSAGE=" + request_data;
  YSOS_LOG_INFO("Request recognation data: " << request_data);
  boost::replace_all(request_data, "+", "%2B");
  
  boost::scoped_ptr<HttpClientInterface> http_client(new HttpClient());
  http_client->SetHttpUrl(recognize_url);
  http_client->SetHttpHeader("Content-Type", "application/x-www-form-urlencoded");
  http_client->SetHttpContent(request_data.c_str(), request_data.length());
  http_client->SetTimeOut(MAX_DEFAULT_TIMEOUT_MS, MAX_DEFAULT_TIMEOUT_MS);
  std::string response_data;
  int ret = http_client->Request(response_data, HTTP_POST);
  response_data = boost::locale::conv::from_utf<char>(response_data.c_str(), "gbk");
  YSOS_LOG_INFO("Recognize response: " << response_data);
  if (ret != 0) {
    YSOS_LOG_ERROR("request failed, code: " << ret);
    return;
  }

  Json::Reader reader;
  Json::Value root;
  if (!reader.parse(response_data.c_str(), root, false)) {
    YSOS_LOG_ERROR("Parse response json failed.");
    return;
  }

  try {
    Json::Value rep_head = root.get("REP_HEAD", Json::objectValue);
    std::string resp_code = rep_head.get("TRAN_CODE", Json::Value("")).asString();
    std::string resp_msg = rep_head.get("TRAN_RSPMSG", Json::Value("")).asString();
    
    // the server will return "000000" if request successed
    if ("000000" != resp_code) {
      YSOS_LOG_ERROR("Comparison failed, resp_code: " << resp_code);
      YSOS_LOG_ERROR("Comparison failed, resp_msg: " << resp_msg);
      return;
    }

    Json::Value rep_body = root.get("REP_BODY",
                                    Json::Value(Json::objectValue));
    Json::Value faces = rep_body.get("faces", Json::Value(Json::arrayValue));
    boost::lock_guard<boost::mutex> guard(result_lock_);
    if (faces.size() > 0) {
      Json::Value item = faces.get(Json::UInt(0), Json::Value());

      // 相似度太低就丢弃
      float score = (float)item.get("score", Json::Value(0)).asDouble();
      if (score < FaceModuleConfig::GetInstance()->GetMinRecognizeScore())
        return;

      std::string name =
          item.get("personName", Json::Value("")).asString();
      std::string idno = item.get("idCard", Json::Value("")).asString();
      std::string sex = item.get("sex", Json::Value("")).asString();
      int age = 0;
      if (item.get("age", Json::Value("")).isString())
        age = atoi(item.get("age", Json::Value("")).asString().c_str());
      else
        age = item.get("age", Json::Value(0)).asInt();
 
      recog_face_->similarity = score;
      recog_face_->id_no = idno;
      recog_face_->name = name;
      recog_face_->age = age;
      if (sex == "M")
        recog_face_->gender = 1;
      else if (sex == "F")
        recog_face_->gender = -1;

      if (FaceModuleConfig::GetInstance()->IsLocalRecognizeEnabled())
        UpdateLocalFaceInfo(recog_face_);

      // 更新上一次识别的人脸
      last_recog_face_.reset(new FaceInfo);
      last_recog_face_->track_id = recog_face_->track_id;
      last_recog_face_->name = recog_face_->name;
      last_recog_face_->id_no = recog_face_->id_no;
      last_recog_face_->age = recog_face_->age;
      last_recog_face_->gender = recog_face_->gender;
      YSOS_LOG_DEBUG("Update last recog, track id: " << recog_face_->track_id
                     << ", name: " << recog_face_->name);
    }
  } catch(std::exception &ex) {
    YSOS_LOG_DEBUG("Request recognize exception: " << ex.what());
  }
#else
// TODO:add for linux
#endif
}

void DefaultRecognizer::UpdateLocalFaceInfo(boost::shared_ptr<FaceInfo> face) {
  YSOS_LOG_DEBUG("DefaultRecognizer::UpdateLocalFaceInfo()");
#ifdef _WIN32
  cw_errcode_t errCode = CW_SDKLIT_OK;
  int feature_len = cwGetFeatureLength(recog_handle_);
  if (face->filed_feature.size() != feature_len) {
    face->filed_feature.resize(feature_len);
    errCode = cwGetFiledFeature(
        recog_handle_, (cw_aligned_face_t*)face->ex_data,
        &(face->filed_feature[0]));
    if (CW_SDKLIT_OK != errCode) {
      YSOS_LOG_DEBUG("Get face filed feature failed: " << errCode);
      face->filed_feature.clear();
    }
  }

  // 更新已存的人脸信息
  for (int i = 0; i < local_face_infos_.Length(); ++i) {
    boost::shared_ptr<FaceInfo> &local = local_face_infos_.Get(i);
    if (local->id_no == face->id_no) {
      if (face->similarity > local->similarity) {
        YSOS_LOG_DEBUG ("Update local face info of " << face->name << ", " << local->similarity << " >> " << face->similarity);
        local->prob_feature = face->prob_feature;
        local->filed_feature = face->filed_feature;
        local->similarity = face->similarity;
      }
      return;
    }
  }

  // 添加新的人脸信息
  YSOS_LOG_DEBUG ("Add local face info of " << face->name << ", " << face->similarity << " >> " << face->similarity);
  local_face_infos_.Add(face);
#else
// TODO:add for linux
#endif
}


} // namespace ysos
