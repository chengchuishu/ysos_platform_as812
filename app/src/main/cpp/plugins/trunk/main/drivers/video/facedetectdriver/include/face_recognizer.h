/**
 *@file face_recognizer.h
 *@brief 
 *@version 1.0
 *@author WangDaqian
 *@date Created on: 2018/1/19
 *@copyright Copyright(c) 2018 YunShen Technology. All rights reserved.
**/


#ifndef YSOS_FACE_RECOGNIZER_H
#define YSOS_FACE_RECOGNIZER_H

#include <boost/shared_ptr.hpp>
#include <vector>


namespace ysos {

struct FaceInfo;

class FaceRecognizer {
public:
  // 人脸识别
  virtual void Recognize(
      std::vector<boost::shared_ptr<FaceInfo> > &recognize_params) = 0;
  // 获取异步执行的识别结果
  virtual bool GetRecognizeResult(
      std::vector<boost::shared_ptr<FaceInfo> > &recognize_params) = 0;

};

} // namespace ysos

#endif  // YSOS_FACE_RECOGNIZER_H
