/**
 *@file face_compare_1vs1.cpp
 *@brief 人脸1比1
 *@version 1.0
 *@author WangDaqian
 *@date Created on: 2018/4/2
 *@copyright Copyright(c) 2018 YunShen Technology. All rights reserved.
**/


#include "../include/face_compare_1vs1.h"

#include "../include/common_struct.h"
#include <boost/locale.hpp>
#include <exception>

#ifdef _WIN32
/// cloudwalk sdk headers
#include <cloudwalk_sdk/Include/CWFaceAttribute.h>
#include <cloudwalk_sdk/Include/CWFaceConfig.h>
#include <cloudwalk_sdk/Include/CWFaceDetection.h>
#include <cloudwalk_sdk/Include/CWFaceRecognition.h>
#include <cloudwalk_sdk/Include/CWFaceVersion.h>
#else
//TODO:add for linux
#endif

namespace ysos {

FaceCompare1vs1::FaceCompare1vs1()
    : is_thread_quit_(false),
      faces_info_(10),
      compare_score_(0.f),
      compare_track_id_(-1),
      is_result_returned_(true),
      logger_(GetUtility()->GetLogger("ysos.video")) {
#ifdef _WIN32
  recognize_handle_file_ = "..\\data\\facedetectdriver\\CWModels\\CWR_Config_1_1.xml";
#else
//TODO:add for linux
#endif
  thread_.reset(new boost::thread(&FaceCompare1vs1::ThreadFunc, this));
  Init();
}

FaceCompare1vs1::~FaceCompare1vs1() {
  Uninit();
}

void FaceCompare1vs1::Init() {
#ifdef _WIN32    
  cw_errcode_t errCode = CW_SDKLIT_OK;
  recog_handle_ = cwCreateRecogHandle(&errCode, recognize_handle_file_.c_str(), 0, CW_FEATURE_EXTRACT);
  if (recog_handle_ == 0)
    YSOS_LOG_ERROR("Create recognize handle failed. Error: " << errCode);
#else
//TODO:add for linux
#endif
}

void FaceCompare1vs1::Uninit() {
  is_thread_quit_ = true;
#ifdef _WIN32
  if (recog_handle_) {
    cwReleaseRecogHandle(recog_handle_);
    recog_handle_ = 0;
  }
#else
//TODO:add for linux
#endif
}

void FaceCompare1vs1::Compare(int track_id) {
  YSOS_LOG_INFO("Compare track id: " << track_id);
  if (!register_face_.get()) {
    YSOS_LOG_INFO("No face registered.");
    is_result_returned_ = false;
    compare_track_id_ = track_id;
    return;
  }
  
  {
    boost::mutex::scoped_lock lock(mutex_);
    thread_operation_ = boost::bind(&FaceCompare1vs1::DoCompare, this, track_id);
  }

  thread_waiter_.notify_one();
}

bool FaceCompare1vs1::GetCompareResult(int* result) {
  if (is_result_returned_ || !result)
    return false;

  YSOS_LOG_DEBUG("FaceCompare1vs1::GetCompareResult");
  boost::mutex::scoped_lock lock(mutex_);
  result[0] = compare_track_id_;
  result[1] = compare_score_ > 0.85f ? 1 : 0;
  is_result_returned_ = true;
  compare_score_ = 0.f;
  return true;
}

void FaceCompare1vs1::AddFaceInfo(boost::shared_ptr<FaceInfo>& face) {
  YSOS_LOG_DEBUG("FaceCompare1vs1::AddFaceInfo");
  boost::mutex::scoped_lock lock(mutex_);
  for (int i = 0; i < faces_info_.Length(); ++i) {
    boost::shared_ptr<FaceInfo> &local = faces_info_.Get(i);
    // Update face info if track id exist.
    if (local->track_id == face->track_id &&
        local->face_quality < face->face_quality) {
      local = face;
      return;
    }
  }

  // Or, add face info.
  faces_info_.Add(face);
}

void FaceCompare1vs1::Register(int track_id) {
  YSOS_LOG_INFO("Register track id: " << track_id);
  {
    boost::mutex::scoped_lock lock(mutex_);
    thread_operation_ = boost::bind(&FaceCompare1vs1::DoRegister, this, track_id);
  }

  thread_waiter_.notify_one();
}

void FaceCompare1vs1::Unregister(int track_id) {
  YSOS_LOG_INFO("Unregister track id: " << track_id);
  register_face_ = nullptr;
}

void FaceCompare1vs1::ThreadFunc() {
  YSOS_LOG_INFO("Compare thread start.");

  while (!is_thread_quit_) {
    boost::mutex::scoped_lock lock(mutex_);
    thread_waiter_.wait(lock);
    thread_operation_();
  }
}

void FaceCompare1vs1::DoCompare(int track_id) {
  // Do not need lock here, the lock is in ThreadFunc.
  YSOS_LOG_INFO("Do compare track id: " << track_id);
#ifdef _WIN32
  cw_errcode_t errCode = CW_SDKLIT_OK;
  if (!register_face_.get()) {
    YSOS_LOG_INFO("No face registered.");
  } else {
    for (int i = 0; i < faces_info_.Length(); ++i) {
      boost::shared_ptr<FaceInfo> &local = faces_info_.Get(i);
      if (local->track_id == track_id) {
        static int feature_len = cwGetFeatureLength(recog_handle_);
        YSOS_LOG_DEBUG("Feature len: " << feature_len);
        if (local->prob_feature.size() != feature_len) {
          local->prob_feature.resize(feature_len);
          errCode = cwGetProbeFeature(
              recog_handle_, (cw_aligned_face_t*)(local->ex_data),
              &(local->prob_feature[0]));
          if (CW_SDKLIT_OK != errCode) {
            YSOS_LOG_DEBUG("Get face prob feature failed: " << errCode);
            local->prob_feature.clear();
          }
        }
        errCode = cwComputeMatchScore(
            recog_handle_, local->prob_feature.data(),
            local->prob_feature.size(), 1,
            register_face_->filed_feature.data(),
            register_face_->filed_feature.size(), 1, &compare_score_);
        break;
      }
    }
  }
  YSOS_LOG_INFO("Compare score: " << compare_score_ << ", code: " << errCode);
  compare_track_id_ = track_id;
  is_result_returned_ = false;
#else
//TODO:add for linux
#endif
}

void FaceCompare1vs1::DoRegister(int track_id) {
  // Do not need lock here, the lock is in ThreadFunc.
  YSOS_LOG_INFO("Do register track id: " << track_id);
#ifdef _WIN32
  for (int i = 0; i < faces_info_.Length(); ++i) {
    boost::shared_ptr<FaceInfo> &local = faces_info_.Get(i);
    if (local->track_id == track_id) {
      cw_errcode_t errCode = CW_SDKLIT_OK;
      static int feature_len = cwGetFeatureLength(recog_handle_);
      YSOS_LOG_DEBUG("Feature len: " << feature_len);
      if (local->filed_feature.size() != feature_len) {
        local->filed_feature.resize(feature_len);
        errCode = cwGetFiledFeature(
            recog_handle_, (cw_aligned_face_t*)(local->ex_data),
            &(local->filed_feature[0]));
        if (CW_SDKLIT_OK != errCode) {
          YSOS_LOG_DEBUG("Get face filed feature failed: " << errCode);
          local->filed_feature.clear();
        }
      }
      register_face_ = local;
      return;
    }
  }
#else
//TODO:add for linux
#endif
  YSOS_LOG_INFO("Register track id doesn't exist " << track_id);
}

} // namespace ysos
