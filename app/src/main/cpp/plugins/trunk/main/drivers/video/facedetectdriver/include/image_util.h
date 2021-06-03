/**
 *@file image_util.h
 *@brief 
 *@version 1.0
 *@author WangDaqian
 *@date Created on: 2018/1/19
 *@copyright Copyright(c) 2018 YunShen Technology. All rights reserved.
**/


#ifndef YSOS_IMAGE_UTIL_H
#define YSOS_IMAGE_UTIL_H

#include <string>


namespace ysos {

class Image;

class ImageUtil {
public:
  // 图片裁剪后转成base64
  // @img 图片
  // @base64 输出的base64编码
  // @roi_x 裁剪区域左上角x轴坐标
  // @roi_y 裁剪区域左上角y轴坐标
  // @roi_w 裁剪区域宽度
  // @roi_h 裁剪区域高度
  static bool ClipImage(const Image &img, std::string &base64,
                        int roi_x, int roi_y , int roi_w, int roi_h);
};

} // namespace ysos

#endif  // YSOS_IMAGE_UTIL_H
