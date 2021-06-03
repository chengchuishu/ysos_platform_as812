/**
 *@file ImagConvertCallback.cpp
 *@brief speech output
 *@version 1.0
 *@author dhongqian
 *@date Created on: 2016-05-26 14:39:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// self headers
#include "../include/imgconvertcallback.h"

/// Stl Headers //  NOLINT
#include <vector>

/// Boost Headers //  NOLINT
#include <boost/algorithm/string.hpp>

/// boost headers
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/filepath.h"
#include <json/json.h>

/// opencv headers
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>

/// private headers
#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"

#include "../../../../../../core/trunk/main/protect/include/os_hal_package/datainterfacemanager.h"


namespace ysos {
DECLARE_PLUGIN_REGISTER_INTERFACE(ImagConvertCallback, CallbackInterface);
ImagConvertCallback::ImagConvertCallback(const std::string &strClassName):BaseModuleCallbackImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.video");
  input_type_ = std::string("stream_frame_rgb@24@p6");
  output_type_ = "jpg";
  img_type_ = ".jpg";
  img_path_ = FilePath::GetCurPath() + "/../images/";
  boost::replace_all(img_path_, "\\", "/");
  output_pattern_ = "uri:";
  cur_interval_number_ = 0;
  interval_max_ = 1;
}

int ImagConvertCallback::Initialized(const std::string &key, const std::string &value)  {
  if ("interval_max" == key) {
    interval_max_ = GetUtility()->ConvertFromString(value, interval_max_);
    if (0 == interval_max_) {
      interval_max_ = 1;
    }
  } else if ("image_path" == key) {
    img_path_ = value;
    boost::replace_all(img_path_, "\\", "/");
    if (FilePath::IsDirectory(img_path_)) {
      FilePath::RemoveDirectory(img_path_);
    }
  } else {
    YSOS_LOG_ERROR(" unknown key,value: " << key << "," << value);
  }

  return YSOS_ERROR_SUCCESS;
}

int ImagConvertCallback::IsReady() {
  return YSOS_ERROR_SUCCESS;
}

std::string ImagConvertCallback::GetImageName(void) {
  if (!FilePath::IsDirectory(img_path_)) {
    bool ret = FilePath::CreateDirectory(img_path_);
    if (!ret) {
      YSOS_LOG_ERROR("Create dir: " << img_path_ << " failed");
      return "";
    }
  }

  std::string cur_time = GetUtility()->GetCurTime();
  cur_time = boost::replace_all_copy(cur_time, " ", "");
  cur_time = boost::replace_all_copy(cur_time, ":", "-");
  std::string::size_type pos = cur_time.find_first_of("|");
  cur_time = cur_time.substr(0, pos);
  return img_path_ + cur_time.append(img_type_);
}

int ImagConvertCallback::RealCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr next_output_buffer, BufferInterfacePtr pre_output_buf, void *context) {
  ModuleInterface *cur_module = GetCurModule(context);
  assert(NULL != cur_module);

  bool is_capture = false;
  cur_module->GetProperty(PROP_CAPTURE_IMAGE, &is_capture);
  if(!is_capture) {
    return YSOS_ERROR_SKIP;
  }

  std::string img_file_name = GetImageName();
  if (img_file_name.empty()) {
    return YSOS_ERROR_FAILED;
  }

  uint8_t *img_data = GetBufferUtility()->GetBufferData(input_buffer);
  int img_data_len = GetBufferUtility()->GetBufferLength(input_buffer);
  if (NULL == img_data || 0 == img_data_len) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  unsigned short *width = (unsigned short*)(img_data + 2);
  unsigned short *height = (unsigned short*)(img_data + 4);
  if (0 == *width || 0 == *height) {
    return YSOS_ERROR_SUCCESS;
  }

  uint8_t *bmp_data = img_data + 6;
  cv::Mat src_mat(*height, *width, CV_8UC3, bmp_data);
  bool ret = cv::imwrite(img_file_name, src_mat);
  if (!ret) {
    return YSOS_ERROR_FAILED;
  }

  std::string has_face;
  DataInterfacePtr data_ptr_ = GetDataInterfaceManager()->GetData(GLOBAL_DATA_KEY);
  if (data_ptr_) {
    data_ptr_->GetData("has_face", has_face);
  }

  if (NULL != next_output_buffer) {
    uint8_t *next_output_data = GetBufferUtility()->GetBufferData(next_output_buffer);
    int len = GetBufferUtility()->GetBufferLength(next_output_buffer);
    std::string output_string = img_file_name;
    int url_len = output_string.length();
    if (len > url_len) {
      Json::FastWriter jsWriter;
      Json::Value js_info;
      js_info["url"] = output_string;
      js_info["hasface"] = has_face.empty() ? "0" : has_face;

      // get person age gender information
      if(data_ptr_) {
        std::string person_age;
        data_ptr_->GetData("age", person_age);
        js_info["age"] = person_age;

        std::string person_gender;
        data_ptr_->GetData("gender", person_gender);
        js_info["gender"] = person_gender;
      }

      Json::Value js_snapshot;
      js_snapshot["type"] = "snapshot";
      js_snapshot["data"] = js_info;
      std::string strResult = jsWriter.write(js_snapshot);
      strResult = GetUtility()->ReplaceAllDistinct ( strResult, "\\r\\n", "" );

      memcpy(next_output_data, strResult.c_str(), strResult.length());
      next_output_data[strResult.length()] = '\0';
      GetBufferUtility()->SetBufferLength(next_output_buffer, strResult.length()+1);
      YSOS_LOG_DEBUG("face snapshot json: " << strResult);
    }
  }

  is_capture = false;
  cur_module->SetProperty(PROP_CAPTURE_IMAGE, &is_capture);
  return YSOS_ERROR_SUCCESS;
}

}