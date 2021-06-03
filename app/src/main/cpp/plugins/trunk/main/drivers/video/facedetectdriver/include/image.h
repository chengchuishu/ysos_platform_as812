/**
 *@file image.h
 *@brief 人脸检测用到的图像类头文件
 *@version 1.0
 *@author WangDaqian
 *@date Created on: 2018/1/19
 *@copyright Copyright(c) 2018 YunShen Technology. All rights reserved.
**/


#ifndef YSOS_FACE_IMAGE_H
#define YSOS_FACE_IMAGE_H


namespace ysos {

class Image {
public:
  Image();
  Image(void *data, int width, int height, int channel);
  virtual ~Image();

  // 返回图像数据
  void *GetData() const { return data_; }
  // 返回图像宽度
  int GetWidth() const { return width_; }
  // 返回图像高度
  int GetHeight() const { return height_; }
  // 返回颜色通道
  int GetChannel() const { return channel_; }

private:
  // 图像数据
  void *data_;
  // 图像宽度
  int width_;
  // 图像高度
  int height_;
  // 颜色通道
  int channel_;

};

} // namespace ysos

#endif  // YSOS_FACE_IMAGE_H