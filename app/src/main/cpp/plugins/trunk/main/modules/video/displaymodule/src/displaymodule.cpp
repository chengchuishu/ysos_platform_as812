/**
 *@file DisplayModule.cpp
 *@brief display module
 *@version 1.0
 *@author dhongqian
 *@date Created on: 2016-05-26 14:39:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// self header
#include "../include/displaymodule.h"
/// ysos headers
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/manager_declare.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/jsonutility.h"

namespace ysos {
DECLARE_PLUGIN_REGISTER_INTERFACE(DisplayModule, ModuleInterface);
DisplayModule::DisplayModule(const std::string &strClassName):BaseModuleImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.video");
  left_ = top_ = 0;
  width_ = 120;
  height_ = 160;
  title_ = "tdocx_display";
  is_mirror_mode_ = true;
  is_capture_image_ = false;
  is_query_admin_ = false;
}

int DisplayModule::Initialized(const std::string &key, const std::string &value)  {
  if ("left" == key) {
    left_ = GetUtility()->ConvertFromString(value, width_);
  } else if ("top" == key) {
    top_ = GetUtility()->ConvertFromString(value, width_);
  } else if ("width" == key) {
    width_ = GetUtility()->ConvertFromString(value, width_);
  } else if ("height" == key) {
    height_ = GetUtility()->ConvertFromString(value, width_);
  } else if ("title" == key) {
    title_ = value;
  } else if ("mirror_mode" == key) {
    is_mirror_mode_ = ("true" == value);
  }

  return YSOS_ERROR_SUCCESS;
}

int DisplayModule::InitalDataInfo() {
  assert(module_data_info_);

  module_data_info_->module_type = PROP_DESTINATION;
  module_data_info_->in_datatypes = std::string("stream_frame_rgb@24@p6");
  module_data_info_->out_datatypes = "null";

  module_data_info_->is_self_allocator = true;
  module_data_info_->buffer_number = 1;
  module_data_info_->buffer_length = 10;

  return YSOS_ERROR_SUCCESS;
}

int DisplayModule::RealOpen(LPVOID param /* = NULL */) {
  if (driver_.empty()) {
    return YSOS_ERROR_SUCCESS;
  }
  driver_ptr_ = GetDriverInterfaceManager()->FindInterface(driver_);
  assert(NULL != driver_ptr_);
  driver_ptr_->SetProperty(PROP_LEFT, &left_);
  driver_ptr_->SetProperty(PROP_TOP, &top_);
  driver_ptr_->SetProperty(PROP_WIDTH, &width_);
  driver_ptr_->SetProperty(PROP_HEIGHT, &height_);
  driver_ptr_->SetProperty(PROP_MIRROR_MODE, &is_mirror_mode_);

  int ret = driver_ptr_->Open(&title_);
  if (YSOS_ERROR_SUCCESS != ret) {
    return ret;
  }

  ret = SetStatusEventCallback(driver_ptr_);

  return ret;
}

int DisplayModule::Ioctl(INT32 control_id, LPVOID param) {
  YSOS_LOG_ERROR("DisplayModule::Ioctl[Enter]");

  int ret = YSOS_ERROR_SUCCESS;

  switch (control_id) {
  case CMD_CAPTURE_IMAGE: {
    is_capture_image_ = true;
    YSOS_LOG_DEBUG("set capture image request");
    break;
  }
  case CMD_SET_IMAGE_WIDTH_HEIGHT: {
    YSOS_LOG_ERROR("DisplayModule::Ioctl()[CMD_SET_IMAGE_WIDTH_HEIGHT]");

    std::string *image_width_height = reinterpret_cast<std::string*>(param);
    if (NULL == image_width_height) {
      ret = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }

    YSOS_LOG_ERROR("DisplayModule::Ioctl()[image_width_height][" << *image_width_height <<"]");

    ret = SetImageWidthAndHeight(*image_width_height);
    break;
  }
  
  case CMD_PERSON_IS_ADMINISTRATER: {
    is_query_admin_ = true;
    break;
  }

  default:
    ret = BaseModuleImpl::Ioctl(control_id, param);
  }

  YSOS_LOG_ERROR("DisplayModule::Ioctl[Exit]");

  return ret;
}

int DisplayModule::SetImageWidthAndHeight(const std::string &image_width_height) {
  YSOS_LOG_ERROR("DisplayModule::SetImageWidthAndHeight[Enter]");
  
  if (NULL == driver_ptr_) {
    YSOS_LOG_ERROR("display driver is null");

    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  JsonValue root_value;
  int ret = GetJsonUtil()->JsonObjectFromString(image_width_height, root_value);
  if (YSOS_ERROR_SUCCESS != ret) {
    YSOS_LOG_ERROR("parse image width and height failed: " << image_width_height);
    return ret;
  }

  int width = width_;
  int height = height_;

  YSOS_LOG_ERROR("DisplayModule::SetImageWidthAndHeight[width][" << width << "]");
  YSOS_LOG_ERROR("DisplayModule::SetImageWidthAndHeight[height][" << height << "]");

  if (root_value.isMember("width") && root_value["width"].isInt()) {
  width = root_value["width"].asInt();
  YSOS_LOG_ERROR("DisplayModule::SetImageWidthAndHeight[reset][width][" << width << "]");
  }

  if (root_value.isMember("height") && root_value["height"].isInt()) {
  height = root_value["height"].asInt();
  YSOS_LOG_ERROR("DisplayModule::SetImageWidthAndHeight[reset][height][" << height << "]");
  }

//   width_ = width;
//   height_ = height;

  YSOS_LOG_ERROR("DisplayModule::SetImageWidthAndHeight[width_][" << width_ << "]");
  YSOS_LOG_ERROR("DisplayModule::SetImageWidthAndHeight[height_][" << height_ << "]");

  driver_ptr_->SetProperty(PROP_WIDTH, &width);
  driver_ptr_->SetProperty(PROP_HEIGHT, &height);

  YSOS_LOG_ERROR("DisplayModule::SetImageWidthAndHeight[Exit]");

  return YSOS_ERROR_SUCCESS;
}

int DisplayModule::RealRun() {
  return YSOS_ERROR_SUCCESS;
}

int DisplayModule::RealPause() {
  return YSOS_ERROR_SUCCESS;
}

int DisplayModule::RealStop() {
  return YSOS_ERROR_SUCCESS;
}

int DisplayModule::RealClose() {

  return YSOS_ERROR_SUCCESS;
}

int DisplayModule::GetProperty(int iTypeId, void *piType) {
  int ret = YSOS_ERROR_SUCCESS;

  switch (iTypeId) {
  case PROP_CAPTURE_IMAGE: {
    bool *is_capture = reinterpret_cast<bool*>(piType);
    assert(NULL!=is_capture);
    *is_capture = is_capture_image_;
    break;
  }

  case CMD_PERSON_IS_ADMINISTRATER: {
    bool *is_query_admin = reinterpret_cast<bool*>(piType);
    assert(NULL != is_query_admin);
    *is_query_admin = is_query_admin_;
    break;
  }

  default:
    ret = BaseModuleImpl::GetProperty(iTypeId, piType);
  }

  return ret;
}

int DisplayModule::SetProperty(int type_id, void *type) {
  int ret = YSOS_ERROR_SUCCESS;

  switch (type_id) {
  case PROP_BUFFER: {
    BufferInterfacePtr *buffer_ptr_ptr = static_cast<BufferInterfacePtr*>(type);
    if (NULL == buffer_ptr_ptr) {
      ret = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }
    BufferInterfacePtr buffer_ptr = *buffer_ptr_ptr;
    driver_ptr_->Write(buffer_ptr);
    break;
  }

  case PROP_CAPTURE_IMAGE: {
    bool *is_capture = reinterpret_cast<bool*>(type);
    assert(NULL!=is_capture);
    is_capture_image_ = *is_capture;
    break;
  }

  case CMD_PERSON_IS_ADMINISTRATER: {
    bool *is_query_admin = reinterpret_cast<bool*>(type);
    assert(NULL != is_query_admin);
    is_query_admin_ = *is_query_admin;
    break;
  }

  default:
    ret = BaseModuleImpl::SetProperty(type_id, type);
  }

  return ret;
}

}