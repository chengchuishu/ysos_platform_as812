/**
 *@file face_detect_factory.h
 *@brief 
 *@version 1.0
 *@author WangDaqian
 *@date Created on: 2018/1/19
 *@copyright Copyright(c) 2018 YunShen Technology. All rights reserved.
**/


#ifndef YSOS_FACE_DETECT_FACTORY_H
#define YSOS_FACE_DETECT_FACTORY_H

/// private headers
#include "facedetector.h"
#include "facecomparisoninterface.h"

namespace ysos {
 
  

class FaceDetectFactory {
public:
  // 创建人脸检测接口
  // @type 接口类型
  static FaceDetectorInterfacePtr CreateDetector(const char *type);
  // 创建人脸识别接口
  // @type 接口类型
  static FaceComparisonInterfacePtr CreateRecognizer(const char *type);
};

} // namespace ysos

#endif  // YSOS_FACE_DETECT_FACTORY_H
