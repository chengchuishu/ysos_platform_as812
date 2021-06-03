/**
 *@file face_detector.h
 *@brief 人脸检测接口
 *@version 1.0
 *@author WangDaqian
 *@date Created on: 2018/1/19
 *@copyright Copyright(c) 2018 YunShen Technology. All rights reserved.
**/


#ifndef YSOS_FACE_DETECTOR_H
#define YSOS_FACE_DETECTOR_H

#include <boost/shared_ptr.hpp>
#include <vector>


namespace ysos {

struct DetectSetting;
struct FaceInfo;
class Image;

class FaceDetector {
public:
  virtual int Detect(const Image &img, const DetectSetting &setting, std::vector<boost::shared_ptr<FaceInfo> > &face_infos) = 0;
};

} // namespace ysos

#endif  // YSOS_FACE_DETECTOR_H
