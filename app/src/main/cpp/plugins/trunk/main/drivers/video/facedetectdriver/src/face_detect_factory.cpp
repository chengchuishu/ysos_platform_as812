/**
 *@file face_detect_factory.cpp
 *@brief 创建人脸检测器和识别器
 *@version 1.0
 *@author WangDaqian
 *@date Created on: 2018/1/19
 *@copyright Copyright(c) 2018 YunShen Technology. All rights reserved.
**/


#include "../include/face_detect_factory.h"

//#include "../include/bankcomm_cw_recognizer.h"
//#include "../include/ccb_recognizer.h"
//#include "../include/cloud_walk_detector.h"
#include "../include/default_recognizer.h"
//#include "../include/face++_recognizer.h"
//#include "../include/tian_cheng_recognizer.h"

#include <string>


//---------------------------人脸检测器-----------------------------------------
// 云从人脸检测
const char *kCloudWalkDetector = "cloud_walk";

//---------------------------人脸识别器----------------------------------------
// 交行云从人脸识别
//const char *kBankCommCWRecognizer = "bankcomm_cw";
// 建行人脸识别
//const char *kCCBRecognizer = "ccb";
// 公版人脸识别
const char *kDefaultRecognizer = "default";
// Face++人脸识别
//const char *kFacePlusPlusRecognizer = "face++";
// 天诚盛业人脸识别
//const char *kTianChengRecognizer = "tian_cheng";


namespace ysos {

FaceDetector *FaceDetectFactory::CreateDetector(const char *type) {
#ifdef _WIN32  
  if (_stricmp(type, kCloudWalkDetector) == 0)
    return new CloudWalkDetector();
#else
//TODO: add for linux
  //YSOS_LOG_DEBUG("FaceDetectFactory::CreateDetector()  [type]:" << type);
#endif    
  return 0;
}

FaceRecognizer *FaceDetectFactory::CreateRecognizer(const char *type) {
#ifdef _WIN32
  if (_stricmp(type, kDefaultRecognizer) == 0)
    return new DefaultRecognizer();
#else
//TODO:add for linux
  //YSOS_LOG_DEBUG("FaceDetectFactory::CreateRecognizer()  [type]:" << type);
#endif
  return 0;
}

} // namespace ysos
