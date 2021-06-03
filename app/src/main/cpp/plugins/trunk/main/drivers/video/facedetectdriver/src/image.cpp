/**
 *@file image.cpp
 *@brief 人脸检测用到的图像类实现
 *@version 1.0
 *@author WangDaqian
 *@date Created on: 2018/1/19
 *@copyright Copyright(c) 2018 YunShen Technology. All rights reserved.
**/


#include "../include/image.h"

#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include <string>


namespace ysos {

Image::Image()
    : data_(0),
      width_(0),
      height_(0),
      channel_(0) {
}

Image::Image(void *data, int width, int height, int channel)
    : data_(0),
      width_(0),
      height_(0),
      channel_(0) {
  int len = width * height * channel;
  if (len > 0 && data != 0) {
    data_ = new char[len];
    if (data_ != 0)
      memcpy(data_, data, len);
    else
      YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.video"), "Alloc memery for image failed.");
    width_ = width;
    height_ = height;
    channel_ = channel;
  }
}

Image::~Image() {
  if (data_)
    delete[] data_;
}

} // namespace ysos
