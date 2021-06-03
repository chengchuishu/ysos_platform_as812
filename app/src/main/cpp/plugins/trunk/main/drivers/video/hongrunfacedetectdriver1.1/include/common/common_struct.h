/**
 *@file detect_setting.h
 *@brief 通用结构体定义
 *@version 1.0
 *@author WangDaqian
 *@date Created on: 2018/1/19
 *@copyright Copyright(c) 2018 YunShen Technology. All rights reserved.
**/


#ifndef YSOS_COMMON_STRUCT_H
#define YSOS_COMMON_STRUCT_H

#include "image.h"
#include <string>


namespace ysos {

// 人脸检测参数
struct DetectSetting {
  bool fetch_base64;

  DetectSetting() : fetch_base64(false) {}
};


// 人脸信息
struct FaceInfo {
  std::string image_base64;
  std::vector<char> filed_feature;
  std::vector<char> prob_feature;
  // 人脸质量，检测得分
  float face_quality;
  // 识别的相似度
  float similarity;
  // 年龄
  int age;
  // 性别, -1 表示女性，1表示男性
  int gender;
  // 用户类型
  int type;
  // 姓名
  std::string name;
  // 身份证号
  std::string id_no;
  //位置
  int x;
  int y;
  // 宽度
  int width;
  // 长度
  int height;
  // 第三方检测要传给识别的额外数据
  void *ex_data;
  // ex_data的长度, 可用做校验
  int ex_length;
  // 人脸追踪id
  int track_id;

  FaceInfo()
      : face_quality(0.f),
        similarity(0.f),
        age(0), gender(0),
        x(0), y(0),
        width(0), height(0),
        ex_data(0),
        track_id(-1) {}

  ~FaceInfo() { if (ex_data) delete ex_data; }
};

} // namespace ysos

#endif  // YSOS_COMMON_STRUCT_H