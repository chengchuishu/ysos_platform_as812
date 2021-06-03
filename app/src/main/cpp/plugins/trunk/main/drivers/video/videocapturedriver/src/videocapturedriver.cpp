/**   //NOLINT
  *@file videocapturedriver.h
  *@brief Definition of VideoCaptureDriver for windows.
  *@version 0.1
  *@author venucia
  *@date Created on: 2016/6/13   19:45
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  *
  */
/// self header
#include "../include/videocapturedriver.h"

#ifdef _WIN32
/// windows headers
#include <Windows.h>
#include <assert.h>
#else
#endif

/// boost headers
#include <boost/thread/thread_guard.hpp>
#include <json/json.h>

/// ysos headers
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/bufferpool.h"
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/callbackqueue.h"

/// opencv headers
#include "../../../../../thirdparty/opencv/include/opencv2/opencv.hpp"
#include "../../../../../thirdparty/opencv/include/opencv2/highgui/highgui.hpp"
#include "../../../../../thirdparty/opencv/include/opencv2/imgproc/imgproc.hpp"
#include "../../../../../thirdparty/opencv/include/opencv2/core/core.hpp"

/// private headers
#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"

namespace ysos {

DECLARE_PLUGIN_REGISTER_INTERFACE(VideoCaptureDriver, DriverInterface);
VideoCaptureDriver::VideoCaptureDriver(const std::string &strClassName) : BaseInterfaceImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.video");
  YSOS_LOG_DEBUG ("in VideoCaptureDriver construct");
  current_oper_camera_index_ =-1;
}

VideoCaptureDriver::~VideoCaptureDriver() {
  // 所有的资源释放放到UnInitialize函数中
  //UnInitialize();
}


int VideoCaptureDriver::RealUnInitialize(void *param) {

  YSOS_LOG_DEBUG("uninitialize logname:" <<logic_name_ );
  Close();
  return YSOS_ERROR_SUCCESS;
}

//int VideoCaptureDriver::Read(BufferInterfacePtr pBuffer) {
int VideoCaptureDriver::Read(BufferInterfacePtr pBuffer, DriverContexPtr context_ptr) {
  return YSOS_ERROR_SUCCESS;
}

//int VideoCaptureDriver::Write(BufferInterfacePtr pBuffer) {
int VideoCaptureDriver::Write(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, DriverContexPtr context_ptr) {
  return YSOS_ERROR_SUCCESS;
}


int VideoCaptureDriver::Ioctl(int iCtrlID, BufferInterfacePtr pInputBuffer, BufferInterfacePtr pOutputBuffer) {
  switch (iCtrlID) {
    case CMD_WEB_VIDEO_CAPTURE: {
      UINT8 *inbufferptr = NULL;
      UINT32 inbuffersize = 0;
      pInputBuffer->GetBufferAndLength(&inbufferptr, &inbuffersize);
      YSOS_LOG_INFO("WebVideoCapture: " << (char*)inbufferptr);

      Json::Reader reader;
      Json::Value root;
      // {"action":"0","height":"256","startX":"1232","startY":"285","width":"188"}
      if (!reader.parse((char*)inbufferptr, root, false)) {
        YSOS_LOG_ERROR("Parse response json failed.");
        return YSOS_ERROR_FAILED;
      }
      try {
        int action = atoi(root["action"].asString().c_str());
        switch (action) {
          case 0: {
            int x = atoi(root["startX"].asString().c_str());
            int y = atoi(root["startY"].asString().c_str());
            int width = atoi(root["width"].asString().c_str());
            int height = atoi(root["height"].asString().c_str());
            video_capture_manager_.StartWebCapture(x, y, width, height);
            break;
          }
          case 1:
            video_capture_manager_.PauseWebCapture();
            break;
          case 2:
            video_capture_manager_.StopWebCapture();
            break;
        }
      } catch (std::exception& ex) {
        YSOS_LOG_ERROR("VideoCaptureService exception: " << ex.what());
      }
      break;
    }
  }

  return YSOS_ERROR_SUCCESS;
}

int VideoCaptureDriver::Open(void *pParams) {
  YSOS_LOG_DEBUG("in open" );
  video_capture_manager_.Initialize();

  CameraOpenParam camera_init_param;
  if(pParams)
  {
    std::string* popen_param = reinterpret_cast<std::string*>(pParams);
    assert(popen_param);
    if(NULL == popen_param)
      return YSOS_ERROR_FAILED;
    int iret = DecodeOpenParam(*popen_param, camera_init_param);
  }

  if(camera_init_param.camera_name.length() >0) {
    int temp_open_index =0;
    int iret = video_capture_manager_.GetCameraIndexAccordName(camera_init_param.camera_name, temp_open_index);
    YSOS_LOG_DEBUG("GetCameraIndexAccordName returcode:"<<iret <<", camer name:" <<camera_init_param.camera_name
      <<"  index:" <<temp_open_index);
    if(YSOS_ERROR_SUCCESS == iret) {
      camera_init_param.index = temp_open_index;
    }
  }
  if(camera_init_param.index <0)
    camera_init_param.index =0;

  //return video_capture_manager_.Open();
  current_oper_camera_index_ = camera_init_param.index;
  if (camera_init_param.camera_name =="Intel(R) RealSense(TM) Camera SR300 RGB")
  {
	  camera_init_param.width = 1280;
	  camera_init_param.height = 720;
  }
  else if (camera_init_param.camera_name =="USB Camera")
  {
	  camera_init_param.width = 1024;
	  camera_init_param.height = 768;
  }
  return video_capture_manager_.Open(camera_init_param.index, camera_init_param.width, camera_init_param.height, camera_init_param.frame_rate);
}

void VideoCaptureDriver::Close(void *pParams) {
  // video_capture_manager_.Close();
  if(current_oper_camera_index_ >=0) {  ///< 打开过摄像头
    video_capture_manager_.Close(current_oper_camera_index_);
    current_oper_camera_index_ = -1;
  }
  video_capture_manager_.UnInitialize();
}

//int VideoCaptureDriver::Initialize(int width, int height) {
//  //return video_capture_imp_.Initialize()
//  return YSOS_ERROR_SUCCESS;
//}

int VideoCaptureDriver::GetProperty(int iTypeId, void *piType) {
  switch (iTypeId) {
  case PROP_FUN_CALLABILITY: {
    //assert(piType);
    YSOS_LOG_DEBUG("VideoCaptureDriver::GetProperty  [PROP_FUN_CALLABILITY] = iTypeId: " << iTypeId);
    FunObjectCommon2* pobject = reinterpret_cast<FunObjectCommon2*>(piType);
    //assert(pobject);
    if (NULL == pobject) {
      YSOS_LOG_DEBUG("error call property PROP_Fun_CallAbility, the param is null" );
    }
    AbilityParam* pin = reinterpret_cast<AbilityParam*>(pobject->pparam1);
    AbilityParam* pout = reinterpret_cast<AbilityParam*>(pobject->pparam2);
    return CallAbility(pin, pout);
  }
  case PROP_GET_CAMERA_OPEN_STATUS_ACCORD_ID: {  ///< 获取摄像头 打开/关闭状态， 1 打开，0关闭
    assert(piType);
    FunObjectCommon2* pobject = reinterpret_cast<FunObjectCommon2*>(piType);
    assert(pobject);
    if (NULL == pobject) {
      YSOS_LOG_DEBUG("error call property PROP_Fun_CallAbility, the param is null" );
    }
    int* pin = reinterpret_cast<int*>(pobject->pparam1);
    int* pout = reinterpret_cast<int*>(pobject->pparam2);
    //todo
    bool is_open = video_capture_manager_.IsOpen(/*0*/current_oper_camera_index_);
    *pout = is_open ?1 :0;
    return YSOS_ERROR_SUCCESS;
  }
  case PROP_GET_CAMERA_OPEN_STATUS_ACCORD_NAME: {
    assert(piType);
    FunObjectCommon2* pobject = reinterpret_cast<FunObjectCommon2*>(piType);
    assert(pobject);
    if (NULL == pobject) {
      YSOS_LOG_DEBUG("error call property PROP_Fun_CallAbility, the param is null" );
    }
    const char* pin = reinterpret_cast<const char*>(pobject->pparam1);
    int* pout = reinterpret_cast<int*>(pobject->pparam2);
    //todo
    bool is_open = video_capture_manager_.IsOpen(/*0*/current_oper_camera_index_);
    *pout = is_open ?1 :0;
    return YSOS_ERROR_SUCCESS;
   }
  case PROP_CAMERA_TAKE_A_PICTURE: {
     assert(piType);
     FunObjectCommon2* pobject = reinterpret_cast<FunObjectCommon2*>(piType);
     assert(pobject);
     if (NULL == pobject) {
       YSOS_LOG_DEBUG("error call property PROP_Fun_CallAbility, the param is null" );
     }
     VideoTakeAPictureRequestV1* pin = reinterpret_cast<VideoTakeAPictureRequestV1*>(pobject->pparam1);
     VideoTakeAPictureResultV1* pout = reinterpret_cast<VideoTakeAPictureResultV1*>(pobject->pparam2);
     assert(pin &&pout);
     if(NULL == pin || NULL == pout) {
       YSOS_LOG_DEBUG("error process PROP_CAMERA_TAKE_A_PICTURE, the param is null" );
       return YSOS_ERROR_FAILED;
     }
     //todo
     int iret = video_capture_manager_.TakeAPicture(/*0*/current_oper_camera_index_, pin->width, pin->hegiht, pin->img_fromat_extension,
       pin->base64_convert, pin->image_quality, pout->vec_picture_data);
     if(YSOS_ERROR_SUCCESS != iret) {
       return iret;
     }
     pout->source_request = *pin;
     return YSOS_ERROR_SUCCESS;
  }
                                   
  case PROP_DATA: {
    string prop_data = video_capture_manager_.GetCaptureResult();
    if (prop_data.empty()) {
      return YSOS_ERROR_NOT_EXISTED;
    }

    YSOS_LOG_INFO("PROP_DATA: " << prop_data);
    BufferInterfacePtr *buffer_ptr_ptr = reinterpret_cast<BufferInterfacePtr*>(piType);
    if (NULL == buffer_ptr_ptr) {
      return YSOS_ERROR_INVALID_ARGUMENTS;
    }
    BufferInterfacePtr buffer_ptr = *buffer_ptr_ptr;
    if (NULL == buffer_ptr) {
      return YSOS_ERROR_INVALID_ARGUMENTS;
    }
    uint8_t *data = GetBufferUtility()->GetBufferData(buffer_ptr);
    size_t buffer_length = GetBufferUtility()->GetBufferLength(buffer_ptr);
    if (NULL == data || 0 == buffer_length || buffer_length <= prop_data.length()) {
      return  YSOS_ERROR_INVALID_ARGUMENTS;
    }

    memcpy(data, prop_data.c_str(), prop_data.length());
    data[prop_data.length()] = '\0';
    return YSOS_ERROR_SUCCESS;
  }
  }
  return YSOS_ERROR_INVALID_ARGUMENTS;
}

int VideoCaptureDriver::SetProperty(int iTypeId, void *piType) {
  switch (iTypeId) {
    case PROP_ROTATE_ANGLE:
      int angle = *((int*)piType);
      video_capture_manager_.SetVideoRotateAngle(angle);
      YSOS_LOG_DEBUG("Set video rotate angle: " << angle);
      break;
  }

  return YSOS_ERROR_SUCCESS;
}

int VideoCaptureDriver::CallAbility(AbilityParam* input_param, AbilityParam* output_param) {
  //assert(/*pInput && */output_param);
  YSOS_LOG_DEBUG("VideoCaptureDriver::CallAbility  [Enter]");
  if (/*NULL == pInput ||*/ NULL == output_param) {
    YSOS_LOG_DEBUG("error call ablicity , input/out alibityparam is null" );
    return YSOS_ERROR_FAILED;
  }
  YSOS_LOG_DEBUG("call ablity:" <<(output_param ? output_param->ability_name : "null") <<" datatype from:"<< (input_param ? input_param->data_type_name : "null") <<"  to-->" <<(output_param ? output_param->data_type_name : "null") );
#ifdef _WIN32
  if (stricmp(ABSTR_VIDEOCAPTURE, output_param->ability_name) ==0) {
    if (stricmp(DTSTR_STREAM_FRAME_RGB24, output_param->data_type_name) ==0) {
      return AbilityVideoCapture_StreamRGB24(input_param, output_param);
    } else if (stricmp(DTSTR_STREAM_FRAME_RGB24_P6, output_param->data_type_name) ==0) {
      return AbilityVideoCapture_StreamRGB24P6(input_param, output_param);
    }
  }
#else
  if (strcasecmp(ABSTR_VIDEOCAPTURE, output_param->ability_name) ==0) {
    if (strcasecmp(DTSTR_STREAM_FRAME_RGB24, output_param->data_type_name) ==0) {
      return AbilityVideoCapture_StreamRGB24(input_param, output_param);
    } else if (strcasecmp(DTSTR_STREAM_FRAME_RGB24_P6, output_param->data_type_name) ==0) {
      return AbilityVideoCapture_StreamRGB24P6(input_param, output_param);
    }
  }
#endif

  YSOS_LOG_DEBUG("error not support ability" );
  assert(false);
  return YSOS_ERROR_FAILED;
}

int VideoCaptureDriver::AbilityVideoCapture_StreamRGB24(AbilityParam* input_param, AbilityParam* output_param) {
  //assert(input_param->buf_interface_ptr);
  YSOS_LOG_DEBUG("VideoCaptureDriver::AbilityVideoCapture_StreamRGB24  [Enter]");
  assert(output_param->buf_interface_ptr);
  if (/*NULL == input_param->buf_interface_ptr ||*/NULL == output_param->buf_interface_ptr)
    return YSOS_ERROR_FAILED;

  //UINT8* ptext = NULL;
  //UINT32 buffer_size =0;
  //assert(input_param->buf_interface_ptr);
  //input_param->buf_interface_ptr->GetBufferAndLength(&ptext,&buffer_size);
  //std::string str_text_need_nlp = reinterpret_cast<char*>(ptext);
  //std::string strnlp_result;
  ////int iret =cxfasr_.TDNLP(0, str_text_need_nlp, 1, &strnlp_result);
  //cv::Mat frame;
  //int iret = video_capture_manager_.GetFrame(frame);
  //assert(YSOS_ERROR_SUCCESS == iret);
  //if (frame.empty()) {
  //  output_param->buf_interface_ptr->SetLength(0);
  //  return YSOS_ERROR_SUCCESS;
  //}
  //int frame_data_length = frame.dataend - frame.data;
  /*if(YSOS_ERROR_SUCCESS != iret) {
    return YSOS_ERROR_FAILED;
  }*/
  int frame_data_length = 0; //frame.dataend - frame.data;
  UINT16 frame_width = 0;  //static_cast<UINT16>(frame.cols);
  UINT16 frame_height = 0;  //static_cast<UINT16>(frame.rows);
  UINT8 frame_chanel = 0;  //static_cast<UINT16>(frame.channels());

   // out put the data
  UINT8* pout = NULL;
  UINT32 pout_size =0;
  assert(output_param->buf_interface_ptr);
  output_param->buf_interface_ptr->GetBufferAndLength(&pout, &pout_size);
  {
    UINT32 max_length = 0, prefix_length =0;
    output_param->buf_interface_ptr->GetMaxLength(&max_length);
    output_param->buf_interface_ptr->GetPrefixLength(&prefix_length);
    pout_size = max_length - prefix_length;
  }
  assert(NULL != pout);
  UINT32 frame_width_u32=0;
  UINT32 frame_height_u32 =0;
  UINT32 frame_channel_u32 =0;
  video_capture_manager_.GetFrame(current_oper_camera_index_,frame_width_u32, frame_height_u32, frame_channel_u32, pout_size, reinterpret_cast<char*>(pout), frame_data_length);
  frame_width = static_cast<UINT16>(frame_width_u32);
  frame_height = static_cast<UINT16>(frame_height_u32);
  frame_chanel = static_cast<UINT8>(frame_channel_u32);


  if ((int)pout_size < frame_data_length) {
    YSOS_LOG_DEBUG("error call ability pcmx1x->asr, output buffer is to small" );
    return YSOS_ERROR_FAILED;
  }

 /* if (frame_data_length >0) {
    memcpy_s(reinterpret_cast<char*>(pout), frame_data_length, frame.data, frame_data_length);
    output_param->buf_interface_ptr->SetLength(frame_data_length);
  } else {
    output_param->buf_interface_ptr->SetLength(0);
  }*/
  output_param->buf_interface_ptr->SetLength(frame_data_length);
  return YSOS_ERROR_SUCCESS;
}

int VideoCaptureDriver::AbilityVideoCapture_StreamRGB24P6(AbilityParam* input_param, AbilityParam* output_param) {
 // assert(input_param->buf_interface_ptr);
 YSOS_LOG_DEBUG("VideoCaptureDriver::AbilityVideoCapture_StreamRGB24P6  [Enter]");
  assert(output_param->buf_interface_ptr);
  if (/*NULL == input_param->buf_interface_ptr ||*/NULL == output_param->buf_interface_ptr)
    return YSOS_ERROR_FAILED;

//   UINT8* ptext = NULL;
//   UINT32 buffer_size =0;
//   assert(input_param->buf_interface_ptr);
//   input_param->buf_interface_ptr->GetBufferAndLength(&ptext,&buffer_size);
//   std::string str_text_need_nlp = reinterpret_cast<char*>(ptext);
//   std::string strnlp_result;
//   //int iret =cxfasr_.TDNLP(0, str_text_need_nlp, 1, &strnlp_result);

  ////cv::Mat frame;
  //int iret = video_capture_manager_.GetFrame(frame );
  //assert(YSOS_ERROR_SUCCESS == iret);
  //if (frame.empty()) {
  //  output_param->buf_interface_ptr->SetLength(0);
  //  return YSOS_ERROR_SUCCESS;
  //}
  //int frame_data_length = frame.dataend - frame.data;
  //UINT16 frame_width = static_cast<UINT16>(frame.cols);
  //UINT16 frame_height = static_cast<UINT16>(frame.rows);
  //UINT8 frame_chanel = static_cast<UINT16>(frame.channels());
  //
  /*if(YSOS_ERROR_SUCCESS != iret) {
    return YSOS_ERROR_FAILED;
  }*/
  YSOS_LOG_DEBUG("VideoCaptureDriver::AbilityVideoCapture_StreamRGB24P6  [Check][0]");
  int frame_data_length = 0; //frame.dataend - frame.data;
  UINT16 frame_width = 0;  //static_cast<UINT16>(frame.cols);
  UINT16 frame_height = 0;  //static_cast<UINT16>(frame.rows);
  UINT8 frame_chanel = 0;  //static_cast<UINT16>(frame.channels());

  // out put the data
  UINT8* pout = NULL;
  UINT32 pout_size =0;
  assert(output_param->buf_interface_ptr);
  output_param->buf_interface_ptr->GetBufferAndLength(&pout, &pout_size);
  {
    UINT32 max_length = 0, prefix_length =0;
    output_param->buf_interface_ptr->GetMaxLength(&max_length);
    output_param->buf_interface_ptr->GetPrefixLength(&prefix_length);
    pout_size = max_length - prefix_length;
  }
  assert(NULL != pout);
  UINT32 frame_channel_u32 =0;
  UINT32 frame_width_u32=0;
  UINT32 frame_height_u32 =0;
  int ret = video_capture_manager_.GetFrame(current_oper_camera_index_,frame_width_u32, frame_height_u32, frame_channel_u32, pout_size, reinterpret_cast<char*>(pout), frame_data_length);
  YSOS_LOG_DEBUG("VideoCaptureDriver::AbilityVideoCapture_StreamRGB24P6  [Check][0][ret]=" << ret);
  frame_width = static_cast<UINT16>(frame_width_u32);
  frame_height = static_cast<UINT16>(frame_height_u32);
  frame_chanel = static_cast<UINT8>(frame_channel_u32);
  YSOS_LOG_DEBUG("VideoCaptureDriver::AbilityVideoCapture_StreamRGB24P6  [Check][1]");
  if ((int)pout_size < frame_data_length +6) {
    YSOS_LOG_DEBUG("error call ability pcmx1x->asr, output buffer is to small" );
    return YSOS_ERROR_FAILED;
  }
  YSOS_LOG_DEBUG("VideoCaptureDriver::AbilityVideoCapture_StreamRGB24P6  [Check][2]");
  if (frame_data_length >0) {
    memset(pout, 0, 6);
    UINT8* pchannel = reinterpret_cast<UINT8*>(&pout[1]);
    UINT16* pwidth = reinterpret_cast<UINT16*>(&pout[2]);
    UINT16* pheight = reinterpret_cast<UINT16*>(&pout[4]);
    *pchannel= frame_chanel;
    *pwidth = frame_width;
    *pheight = frame_height;

    //memcpy_s(reinterpret_cast<char*>(&pout[6]), frame_data_length, frame.data, frame_data_length);
    output_param->buf_interface_ptr->SetLength(frame_data_length +6);
  } else {
    output_param->buf_interface_ptr->SetLength(0);
  }
  YSOS_LOG_DEBUG("VideoCaptureDriver::AbilityVideoCapture_StreamRGB24P6  [Check][2][frame_data_length]=" << frame_data_length);
  YSOS_LOG_DEBUG("VideoCaptureDriver::AbilityVideoCapture_StreamRGB24P6  [Check][3]");
  YSOS_LOG_DEBUG("VideoCaptureDriver::AbilityVideoCapture_StreamRGB24P6  [End]");
  return YSOS_ERROR_SUCCESS;
}

//注意：当字符串为空时，也会返回一个空字符串
static void StrSplit(const std::string& s, const std::string& delim,std::vector< std::string >* ret) {
  size_t last = 0;
  size_t index=s.find_first_of(delim,last);
  while (index!=std::string::npos) {
    ret->push_back(s.substr(last,index-last));
    last=index+1;
    index=s.find_first_of(delim,last);
  }
  if (index-last>0) {
    ret->push_back(s.substr(last,index-last));
  }
}

int VideoCaptureDriver::DecodeOpenParam(const std::string& open_param, CameraOpenParam& camera_param) {
  if(open_param.length() <=0)
    return YSOS_ERROR_FAILED;

  YSOS_LOG_DEBUG("decode open param" <<open_param );
  // 拆分字符串
   std::vector<std::string> vec_params;
  vec_params.clear();
  

  StrSplit(std::string(/*config_param_buf*/open_param), std::string(";="), &vec_params);
  assert(vec_params.size() %2 ==0);
  if (vec_params.size() %2 !=0) {
    //g_Log.Trace_Print(TraceLogLevel_ERROR, __FILE__, __LINE__, "decode camera init param error:%s,%s",config_name_buf, config_param_buf);
    YSOS_LOG_DEBUG("decode camera init param error:"<< open_param );
    return YSOS_ERROR_FAILED;
    //continue;
  }

  CameraOpenParam camer_config_param;
  int param_count = vec_params.size()/2;
  std::string param_name, param_value;
#ifdef _WIN32  
  for (int ii=0; ii < param_count; ++ii) {
    param_name = vec_params[2*ii];
    param_value = vec_params[2*ii +1];
    if (stricmp(param_name.c_str(), "name") ==0) {
      camer_config_param.camera_name = param_value;
    } 
    else if(stricmp(param_name.c_str(), "index") ==0) {
      camer_config_param.index = atoi(param_value.c_str());
    }
    else if (stricmp(param_name.c_str(), "width") ==0) {
      camer_config_param.width = atoi(param_value.c_str());
    } else if (stricmp(param_name.c_str(), "height") ==0) {
      camer_config_param.height = atoi(param_value.c_str());
    } else if (stricmp(param_name.c_str(), "frame_rate") ==0) {
      camer_config_param.frame_rate = atoi(param_value.c_str());
    } else if (stricmp(param_name.c_str(), "horizontal_max_angle") ==0) {
      camer_config_param.horizontal_max_angle = atoi(param_value.c_str());
    } else if (stricmp(param_name.c_str(), "vertical_max_angel") ==0) {
      camer_config_param.vertical_max_angle = atoi(param_value.c_str());
    } else {
      assert(false);
      YSOS_LOG_DEBUG("decode camea init param error, param name not valid:" <<param_name );
      return YSOS_ERROR_FAILED;
    }
  }
#else
  for (int ii=0; ii < param_count; ++ii) {
    param_name = vec_params[2*ii];
    param_value = vec_params[2*ii +1];
    if (strcasecmp(param_name.c_str(), "name") ==0) {
      camer_config_param.camera_name = param_value;
    } 
    else if(strcasecmp(param_name.c_str(), "index") ==0) {
      camer_config_param.index = atoi(param_value.c_str());
    }
    else if (strcasecmp(param_name.c_str(), "width") ==0) {
      camer_config_param.width = atoi(param_value.c_str());
    } else if (strcasecmp(param_name.c_str(), "height") ==0) {
      camer_config_param.height = atoi(param_value.c_str());
    } else if (strcasecmp(param_name.c_str(), "frame_rate") ==0) {
      camer_config_param.frame_rate = atoi(param_value.c_str());
    } else if (strcasecmp(param_name.c_str(), "horizontal_max_angle") ==0) {
      camer_config_param.horizontal_max_angle = atoi(param_value.c_str());
    } else if (strcasecmp(param_name.c_str(), "vertical_max_angel") ==0) {
      camer_config_param.vertical_max_angle = atoi(param_value.c_str());
    } else {
      assert(false);
      YSOS_LOG_DEBUG("decode camea init param error, param name not valid:" <<param_name );
      return YSOS_ERROR_FAILED;
    }
  }
#endif

  camera_param = camer_config_param;
  YSOS_LOG_DEBUG("deoce camra open param succ: camera name:" <<camera_param.camera_name
    <<" index:" <<camera_param.index
    <<" width:" <<camera_param.width
    <<" height:" <<camera_param.height
    <<" farme rate:" <<camera_param.frame_rate
    );
  return YSOS_ERROR_SUCCESS;
}

}  /// end of namespace ysos
