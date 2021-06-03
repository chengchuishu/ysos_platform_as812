/**
 *@file face_detect_factory.cpp
 *@brief 创建人脸检测器和识别器
 *@version 1.0
 *@author WangDaqian
 *@date Created on: 2018/1/19
 *@copyright Copyright(c) 2018 YunShen Technology. All rights reserved.
**/


/// stl headers
#include <string>

/// private headers
#include "../include/face_detect_factory.h"
#include "../include/hongrunfacedetect.h"
#include "../include/facecomparison.h"


namespace ysos {

/// 虹软人脸识别
const char* face_detect_hongrun = "hongrun";

// 公版人脸识别
const char *kDefaultRecognizer = "default";


FaceDetectorInterfacePtr FaceDetectFactory::CreateDetector(const char *type) {
#ifdef _WIN32  
  if (_stricmp(type, face_detect_hongrun) == 0) {
    return FaceDetectorInterfacePtr(new FaceDetectComparisonHR);
  }
  assert(false);
#else
  if (strcasecmp(type, face_detect_hongrun) == 0) {
    return FaceDetectorInterfacePtr(new FaceDetectComparisonHR);
  }
#endif
  return 0;
}

FaceComparisonInterfacePtr FaceDetectFactory::CreateRecognizer(const char *type) {
#ifdef _WIN32
  if (_stricmp(type, kDefaultRecognizer) == 0) {
    return FaceComparisonInterfacePtr(new FaceDetectComparison);
  }
  assert(false);
#else
  if (strcasecmp(type, kDefaultRecognizer) == 0) {
    return FaceComparisonInterfacePtr(new FaceDetectComparison);
  }
#endif
  return 0;
}

} // namespace ysos
