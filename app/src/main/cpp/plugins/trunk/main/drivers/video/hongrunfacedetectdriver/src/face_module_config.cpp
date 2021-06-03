/**
 *@file face_module_config.cpp
 *@brief 参数配置
 *@version 1.0
 *@author WangDaqian
 *@date Created on: 2018/1/19
 *@copyright Copyright(c) 2018 YunShen Technology. All rights reserved.
**/


#include "../include/face_module_config.h"


namespace ysos {

FaceModuleConfig *FaceModuleConfig::s_instance_ = 0;

FaceModuleConfig::FaceModuleConfig()
    : detect_interval_(100),
      recognize_interval_(2000),
      min_face_width_(80),
      min_face_height_(80),
      min_detect_score_(0.6f),
      min_recognize_score_(0.5f) {
}

FaceModuleConfig *FaceModuleConfig::GetInstance() {
  if (s_instance_ == 0)
    s_instance_ = new FaceModuleConfig();
  return s_instance_;
}

} // namespace ysos
