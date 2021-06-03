/**
 *@file image_util.cpp
 *@brief 图片工具类实现文件
 *@version 1.0
 *@author WangDaqian
 *@date Created on: 2018/1/19
 *@copyright Copyright(c) 2018 YunShen Technology. All rights reserved.
**/


#include "../include/image_util.h"

#include "../include/image.h"
#include <boost/algorithm/string.hpp>
#include <exception>
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"

/// opencv headers
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


namespace ysos {

bool ImageUtil::ClipImage(const Image &img, std::string &base64,
                          int roi_x, int roi_y, int roi_w, int roi_h) {
  try {
    int type = CV_8UC(img.GetChannel());
    cv::Mat frame(img.GetHeight(), img.GetWidth(),
                  type, img.GetData());
    cv::Mat mat_align_face(
        frame, cv::Rect(roi_x, roi_y, roi_w, roi_h));
    if (mat_align_face.data) {
      std::vector<uchar> vecImg;
      std::vector<int> params;
      params.push_back(CV_IMWRITE_JPEG_QUALITY);
      params.push_back(100);
      vecImg.clear();
      cv::imencode(".jpg", mat_align_face, vecImg, params);
      base64 = GetUtility()->Base64Encode(vecImg.data(), vecImg.size());
      boost::replace_all(base64, "\r\n", "");
    }
  } catch (std::exception &ex) {
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.video"), "Clip image exception: " << ex.what());
    return false;
  }
  return true;
}

} // namespace ysos
