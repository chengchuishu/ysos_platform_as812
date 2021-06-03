/**
 *@file face_module_config.h
 *@brief 
 *@version 1.0
 *@author WangDaqian
 *@date Created on: 2018/1/19
 *@copyright Copyright(c) 2018 YunShen Technology. All rights reserved.
**/


#ifndef YSOS_FACE_MODULE_CONFIG_H
#define YSOS_FACE_MODULE_CONFIG_H

#include <string>


namespace ysos {

class FaceModuleConfig {
private:
  FaceModuleConfig();

public:
  static FaceModuleConfig *GetInstance();

  void SetDeviceID(const std::string &id) { device_id_ = id; }
  const char *GetDeviceID() const { return device_id_.c_str(); }

  void SetTermID(const std::string &id) { term_id_ = id; }
  const char *GetTermID() const { return term_id_.c_str(); }

  void SetOrgID(const std::string &id) { org_id_ = id; }
  const char *GetOrgID() const { return org_id_.c_str(); }

  void SetDetectInterval(int interval) { detect_interval_ = interval; }
  int GetDetectInterval() const { return detect_interval_; }

  void SetRecognizeInterval(int interval) { recognize_interval_ = interval; }
  int GetRecognizeInterval() const { return recognize_interval_; }

  void SetMinRecognzieScore(float score) { min_recognize_score_ = score; }
  float GetMinRecognizeScore() const { return min_recognize_score_; }

  void SetMinLocalRecognzieScore(float score) { min_local_recognize_score_ = score; }
  float GetMinLocalRecognizeScore() const { return min_local_recognize_score_; }

  void SetMinDetectScore(float score) { min_detect_score_ = score; }
  float GetMinDetectScore() const { return min_detect_score_; }

  void SetRecognizerServer(const std::string &url) { recognize_server_ = url; }
  const char *GetRecognizeServer() const { return recognize_server_.c_str(); }

  void SetMinFaceWidth(int width) { min_face_width_ = width; }
  int GetMinFaceWidth() const { return min_face_width_; }

  void SetMinFaceHeight(int height) { min_face_height_ = height; }
  int GetMinFaceHeight() const { return min_face_height_; }

  void EnableLocalRecognize(bool enable) { enable_local_recognize_ = enable; }
  bool IsLocalRecognizeEnabled() const { return enable_local_recognize_; }

private:
  // 单例
  static FaceModuleConfig *s_instance_;

  // 设备号
  std::string device_id_;
  // 终端号
  std::string term_id_;
  // 机构号
  std::string org_id_;
  // 检测时间间隔(毫秒)
  int detect_interval_;
  // 识别时间间隔(毫秒)
  int recognize_interval_;
  // 识别成功的最低分数
  float min_recognize_score_;
  // 本地识别成功的最低分数
  float min_local_recognize_score_;
  // 检测成功的最低分数
  float min_detect_score_;
  // 人脸识别服务器地址
  std::string recognize_server_;
  // 人脸的最小宽度
  int min_face_width_;
  // 人脸的最小高度
  int min_face_height_;
  // 是否启用本地人脸识别
  bool enable_local_recognize_;
};

} // namespace ysos

#endif  // YSOS_FACE_MODULE_CONFIG_H
