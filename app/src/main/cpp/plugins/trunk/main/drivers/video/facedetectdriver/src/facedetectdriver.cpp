/**
 *@file facedetectdriver.cpp
 *@brief 
 *@version 1.0
 *@author WangDaqian
 *@date Created on: 2018/1/19
 *@copyright Copyright(c) 2018 YunShen Technology. All rights reserved.
**/


#include "../include/facedetectdriver.h"

#include "../include/common_struct.h"
#include "../include/face_compare_1vs1.h"
#include "../include/face_detector.h"
#include "../include/face_detect_factory.h"
#include "../include/face_module_config.h"
#include "../include/face_recognizer.h"
#include "../include/image.h"

#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"

namespace ysos {

DECLARE_PLUGIN_REGISTER_INTERFACE(FaceDetectDriver, DriverInterface);
FaceDetectDriver::FaceDetectDriver(const std::string &strClassName)
    : BaseInterfaceImpl(strClassName),
      min_face_score_(0.7f),
      face_compare_(new FaceCompare1vs1),
      only_biggest_face_(true) {
  YSOS_LOG_DEBUG ("FaceDetectDriver construct");
  logger_ = GetUtility()->GetLogger("ysos.video");
}

int FaceDetectDriver::Ioctl(int iCtrlID, BufferInterfacePtr pInputBuffer,
    BufferInterfacePtr pOutputBuffer) {
  UINT8 *inbufferptr = NULL;
  UINT32 inbuffersize = 0;
  pInputBuffer->GetBufferAndLength(&inbufferptr, &inbuffersize);

  YSOS_LOG_DEBUG("FaceDetectDirver::Ioctl " << iCtrlID);
  switch (iCtrlID) {
    case CMD_FACE_RCGN_1_1_SET_BASE: {
      int track_id = atoi((char*)inbufferptr);
      face_compare_->Register(track_id);
      break;
    }
    case CMD_FACE_RCGN_1_1_CMP_BY_ID: {
      int track_id = atoi((char*)inbufferptr);
      face_compare_->Compare(track_id);
      break;
    }
    case CMD_FACE_RCGN_1_1_CLEAR_BASE: {
      int track_id = atoi((char*)inbufferptr);
      face_compare_->Unregister(track_id);
      break;
    }
  }
  return YSOS_ERROR_SUCCESS;
}

int FaceDetectDriver::GetProperty(int iTypeId, void *piType) {
  switch (iTypeId) {
    case PROP_FACE_DETECT_ASYNC:
      return DetectFaceAsync(piType);
    case PROP_FACE_RECOGNIZE:
      return GetFaceRecognizeResult((FaceComparisonResult1N*)piType);
    case PROP_FACE_COMPARE:
      return GetFaceCompareResult((int*)piType);
  }

  return YSOS_ERROR_INVALID_ARGUMENTS;
}

int FaceDetectDriver::SetProperty(int iTypeId, void *piType) {
  switch (iTypeId) {
    case PROP_FACE_MIN_SCORE:
      min_face_score_ = *((float*)piType);
      YSOS_LOG_DEBUG("Set min face score: " << min_face_score_);
      break;
    case PROP_FACE_RECOGNIZE_INTERVAL: {
      int interval = *((int*)piType);
      FaceModuleConfig::GetInstance()->SetRecognizeInterval(interval);
      break;
    }
    case PROP_MIN_RECOGNIZE_SCORE: {
      float score = *((float*)piType);
      FaceModuleConfig::GetInstance()->SetMinRecognzieScore(score);
      break;
    }
    case PROP_MIN_LOCAL_RECOGNIZE_SCORE: {
      float score = *((float*)piType);
      FaceModuleConfig::GetInstance()->SetMinLocalRecognzieScore(score);
      break;
    }
    case PROP_FACE_DETECTOR: {
      const char *type = (const char *)piType;
      face_detector_.reset(FaceDetectFactory::CreateDetector(type));
      if (face_detector_.get() == NULL)
        YSOS_LOG_ERROR("Wrong face detector type: " << type);
      break;
    }
    case PROP_FACE_RECOGNIZER: {
      const char *type = (const char *)piType;
      face_recognizer_.reset(FaceDetectFactory::CreateRecognizer(type));
      if (face_recognizer_.get() == NULL)
        YSOS_LOG_ERROR("Wrong face recognizer type: " << type);
      break;
    }
    case PROP_RECOGNIZE_SERVER: {
      const char *url = (const char *)piType;
      FaceModuleConfig::GetInstance()->SetRecognizerServer(url);
      break;
    }
    case PROP_ORG_ID: {
      const char *id = (const char *)piType;
      FaceModuleConfig::GetInstance()->SetOrgID(id);
      break;
    }
    case PROP_TERM_ID: {
      const char *id = (const char *)piType;
      FaceModuleConfig::GetInstance()->SetTermID(id);
      break;
    }
    case PROP_MIN_FACE_WIDTH: {
      int width = *((int*)piType);
      FaceModuleConfig::GetInstance()->SetMinFaceWidth(width);
      break;
    }
    case PROP_MIN_FACE_HEIGHT: {
      int height = *((int*)piType);
      FaceModuleConfig::GetInstance()->SetMinFaceHeight(height);
      break;
    }
    case PROP_ENABLE_LOCAL_RECOGNIZE: {
      bool enable = *((bool*)piType);
      FaceModuleConfig::GetInstance()->EnableLocalRecognize(enable);
      break;
    }
  }
  return YSOS_ERROR_SUCCESS;
}

int FaceDetectDriver::DetectFaceAsync(void* param) {
  FunObjectCommon2* pobject = reinterpret_cast<FunObjectCommon2*>(param);
  if (NULL == pobject) {
    YSOS_LOG_DEBUG("Null param when call property PROP_FACE_DETECT_ASYNC");
    return YSOS_ERROR_FAILED;
  }

  FaceDetectRequest* pin = reinterpret_cast<FaceDetectRequest*>(pobject->pparam1);
  FaceDetectInfoGroup* pout =
      reinterpret_cast<FaceDetectInfoGroup*>(pobject->pparam2);
  if (NULL == pin || NULL == pout) {
    YSOS_LOG_DEBUG("in/out param is null" );
    return YSOS_ERROR_FAILED;
  }

  if(NULL == pin->frame_data) {
    YSOS_LOG_ERROR("the frame data is null");
    return YSOS_ERROR_FAILED;
  }

  if (face_detector_.get()) {
    DetectSetting setting;
    setting.fetch_base64 = true;
    std::vector<boost::shared_ptr<FaceInfo> > detect_infos;
    Image image(pin->frame_data, pin->frame_widht,
                pin->frame_height,  pin->frame_channels);
    face_detector_->Detect(image, setting, detect_infos);

    // 写入返回结果
    pout->arr_face_info =
        TDFaceDetectInfoArray(new TDFaceDetectInfo[detect_infos.size()]);
#ifdef _WIN32
    for (size_t i = 0; i < detect_infos.size(); ++i) {
      pout->arr_face_info[i].age = detect_infos[i]->age;
      pout->arr_face_info[i].width = detect_infos[i]->width;
      pout->arr_face_info[i].hight = detect_infos[i]->height;
      pout->arr_face_info[i].x = detect_infos[i]->x;
      pout->arr_face_info[i].y = detect_infos[i]->y;
      pout->arr_face_info[i].face_quality_score = detect_infos[i]->face_quality;
      pout->arr_face_info[i].trackid = detect_infos[i]->track_id;
      char buf[16];
      sprintf_s(buf, "%d", detect_infos[i]->gender);
      pout->arr_face_info[i].gender = buf;

      // 添加新的人脸追踪信息
      face_compare_->AddFaceInfo(detect_infos[i]);
    }
#else
    for (size_t i = 0; i < detect_infos.size(); ++i) {
      pout->arr_face_info[i].age = detect_infos[i]->age;
      pout->arr_face_info[i].width = detect_infos[i]->width;
      pout->arr_face_info[i].hight = detect_infos[i]->height;
      pout->arr_face_info[i].x = detect_infos[i]->x;
      pout->arr_face_info[i].y = detect_infos[i]->y;
      pout->arr_face_info[i].face_quality_score = detect_infos[i]->face_quality;
      pout->arr_face_info[i].trackid = detect_infos[i]->track_id;
      char buf[16];
      sprintf(buf, "%d", detect_infos[i]->gender);
      pout->arr_face_info[i].gender = buf;

      // 添加新的人脸追踪信息
      face_compare_->AddFaceInfo(detect_infos[i]);
    }
#endif
    pout->face_number = detect_infos.size();

    if (face_recognizer_.get() != 0) {
      // 人脸过滤, 进行识别. 达到一定分数才识别
      std::vector<boost::shared_ptr<FaceInfo> > recognize_params;
      for (int i = 0; i < pout->face_number; ++i) {
        YSOS_LOG_INFO("Face detect score: " << detect_infos[i]->face_quality);
        if (detect_infos[i]->face_quality > min_face_score_)
          recognize_params.push_back(detect_infos[i]);
      }
      YSOS_LOG_INFO("Good face count: " << recognize_params.size());
      if (recognize_params.size() > 0)
        face_recognizer_->Recognize(recognize_params);
    }
  } else {
    YSOS_LOG_ERROR("Face detector is null.");
    return YSOS_ERROR_FAILED;
  }

  return YSOS_ERROR_SUCCESS;
}

int FaceDetectDriver::GetFaceRecognizeResult(FaceComparisonResult1N *result) {
  if (face_recognizer_.get() == 0) {
    YSOS_LOG_ERROR("Face recognizer is null.");
    return YSOS_ERROR_FAILED;
  }

  if (result == 0) {
    YSOS_LOG_ERROR("Ouput param is null when get recognize result.");
    return YSOS_ERROR_FAILED;
  }

  std::vector<boost::shared_ptr<FaceInfo> > params;
  if (face_recognizer_->GetRecognizeResult(params)) {
    for (size_t i = 0; i < params.size(); ++i) {
      if (params[i].get() == 0)
        continue;

      FaceCompareResultPtr ptr = FaceCompareResultPtr(new FaceCompareResult);
      ptr->id_card = params[i]->id_no;
      ptr->person_name = params[i]->name;
      char buf[16];
#ifdef _WIN32
      sprintf_s(buf, "%d", params[i]->age);
      ptr->age = buf;
      sprintf_s(buf, "%d", params[i]->gender);
      ptr->sex = buf;
      sprintf_s(buf, "%f", params[i]->similarity);
      ptr->score = buf;
      sprintf_s(buf, "%d", params[i]->type);
      ptr->type = buf;
      sprintf_s(buf, "%d", params[i]->track_id);
#else
      sprintf(buf, "%d", params[i]->age);
      ptr->age = buf;
      sprintf(buf, "%d", params[i]->gender);
      ptr->sex = buf;
      sprintf(buf, "%f", params[i]->similarity);
      ptr->score = buf;
      sprintf(buf, "%d", params[i]->type);
      ptr->type = buf;
      sprintf(buf, "%d", params[i]->track_id);
#endif      
      ptr->person_id = buf;
      result->vec_faces.push_back(ptr);
    }
    return YSOS_ERROR_SUCCESS;
  }
  return YSOS_ERROR_FAILED;
}

int FaceDetectDriver::GetFaceCompareResult(int* result) {
  if (face_compare_->GetCompareResult(result)) {
    return YSOS_ERROR_SUCCESS;
  }
  return YSOS_ERROR_FAILED;
}

} // namespace ysos