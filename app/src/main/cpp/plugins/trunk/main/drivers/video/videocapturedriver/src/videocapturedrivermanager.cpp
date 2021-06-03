/**   //NOLINT
  *@file VideoCaptureDriverImp.h
  *@brief Definition of VideoCaptureDriverImp for windows.
  *@version 0.1
  *@author venucia
  *@date Created on: 2016/6/13   19:45
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  *
  */
/// self header
#include "../include/videocapturedrivermanager.h"

#ifdef _WIN32
/// windows headers
  #include <Windows.h>
  #include <assert.h>
#else
#endif

/// boost headers
#include <boost/thread/thread_guard.hpp>
/// ysos headers        // NOLINT
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
/// privat header
/*#include "../include/tracelog.h"*/

#ifdef _WIN32
/// libs
  #pragma comment(lib,"Strmiids.lib")
#else
#endif

/*extern*/ /*TraceLog g_Log;*/

namespace ysos {
// static log4cplus::Logger logger_ = GetUtility()->GetLogger("ysos.video");

VideoCaptureDriverManage::VideoCaptureDriverManage()
  : video_rotate_angle_(0) { /*: video_capture_(0)*/
  //video_capture_0_ptr_ = boost::shared_ptr<cv::VideoCapture>(new VideoCapture()
  //width_ = 0;
  //height_ = 0;
  /* camera_width_ =-1;
  camera_height_ =-1;*/
  have_initialize_ =0;
}

VideoCaptureDriverManage::~VideoCaptureDriverManage() {
  UnInitialize();  ///< 释放所有摄像头资源
}


int VideoCaptureDriverManage::Initialize() {
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"),  "in WindowsAudioCaptureDriver::Initialize(): have initialize: " << have_initialize_);
  if (1 == have_initialize_) {
    return YSOS_ERROR_SUCCESS;
  }
  map_camera_info_.clear();   ///< 清空以前扫描过的摄像头信息
  // 扫描当前有多少个摄像头及名称
  int camera_count = CameraCount();
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"), "camera count: " << camera_count);
  char camera_name_buf[2048] = {'\0'};
  for (int i=0; i<camera_count; ++i) {
    int iret = CameraName(i, camera_name_buf, sizeof(camera_name_buf));
    if (YSOS_ERROR_SUCCESS != iret) {
      continue;
    }
    CameraInfoPtr ptr(new CameraInfo);
    assert(ptr);
    if (NULL == ptr) {
      continue;
    }
    ptr->camera_index = i;
    ptr->camera_name = /*camera_name_buf*/"/dev/video0";
    map_camera_info_.insert(std::make_pair(i, ptr));
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"),  "camera index: " << i << "camera name: " << camera_name_buf);
  }
  have_initialize_ =1;  ///< 标记已初始化
  return YSOS_ERROR_SUCCESS;
}

int VideoCaptureDriverManage::UnInitialize() {
  std::map<int, CameraInfoPtr>::iterator it = map_camera_info_.begin();
  for (it; it != map_camera_info_.end(); ++it) {
    CameraInfoPtr camera_ptr = it->second;
    if (NULL == camera_ptr || NULL == camera_ptr->video_capture_ptr) {
      continue;
    }
    camera_ptr->video_capture_ptr->release();
  }
  map_camera_info_.clear();
  return YSOS_ERROR_SUCCESS;
}


int VideoCaptureDriverManage::GetAllCameraList(std::list<std::string> &list_all_camera) {
  std::map<int, CameraInfoPtr>::iterator it = map_camera_info_.begin();
  for (it; it != map_camera_info_.end(); ++it) {
    CameraInfoPtr camera_ptr = it->second;
    if (camera_ptr->camera_name.length() <=0) {
      continue;
    }
    list_all_camera.push_back(camera_ptr->camera_name);
  }
  return YSOS_ERROR_SUCCESS;
}

int VideoCaptureDriverManage::GetCameraIndexAccordName(const std::string camera_name, int &camera_index) {
  camera_index =-1;
  assert(camera_name.length() >0);
  if (camera_name.length() <=0) {
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.video"), "error get camer index accord name, name is empty");
    assert(false);
    return YSOS_ERROR_FAILED;
  }
  std::map<int, CameraInfoPtr>::iterator it = map_camera_info_.begin();
  for (it; it != map_camera_info_.end(); ++it) {
    CameraInfoPtr camera_ptr = it->second;
    if (camera_ptr->camera_name.length() <=0) {
      continue;
    }
#ifdef _WIN32
    if (stricmp(camera_ptr->camera_name.c_str(), camera_name.c_str()) ==0) {
      camera_index = camera_ptr->camera_index;
      return YSOS_ERROR_SUCCESS;
    }
#else
    if (strcasecmp(camera_ptr->camera_name.c_str(), camera_name.c_str()) ==0) {
      camera_index = camera_ptr->camera_index;
      return YSOS_ERROR_SUCCESS;
    }
#endif
  }
  return YSOS_ERROR_FAILED;
}


int VideoCaptureDriverManage::Open() {
  return Open(0, -1, -1, -1);
}

int VideoCaptureDriverManage::Open(const int open_index, const int width, const int height, const int frame_rate_persec) {
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"),  "in WindowsVideoCaptureDriver::Open() openinde: " << open_index << "width: " << width
                        << "hegiht: " << height << "frame_rat: " << frame_rate_persec);
#ifdef _DEBUG
  ::MessageBox(NULL, "TEST IN VIDEO CAPTURE DRIVER OPEN FUNCTION", _T("TEST"), MB_OK);
#endif
  if (false == mutex_video_capture_oper_.try_lock_for(boost::chrono::milliseconds(4000))) {
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.video"), "调用 得到锁超时4秒，返回错误");
    return YSOS_ERROR_FAILED;
  }
  boost::lock_guard<boost::timed_mutex> lock_guard_times(mutex_video_capture_oper_,  boost::adopt_lock);
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"), "mutex_video_capture_oper_.try_lock_for(boost::chrono::milliseconds(4000))");
  CameraInfoPtr camera_ptr = GetTheCamera(open_index, true);
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"), "GetTheCamera(open_index, true)");
  if (NULL == camera_ptr  || NULL == camera_ptr->video_capture_ptr) {
    assert(false);
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.video"), "cant get the camera info");
    return YSOS_ERROR_FAILED;
  }
  //如果已经打开过，则不再打开
  if (camera_ptr->video_capture_ptr->isOpened()) {
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.video"),  "摄像头已打开，不用再打开,index: " << open_index);
    return YSOS_ERROR_SUCCESS;
  }
  //int  frate = video_capture_.get(CV_CAP_PROP_FPS);
  //{  /// 设置摄像头参数
  //  CvCapture *camera = cvCaptureFromCAM(nOpenIndex);
  //  assert(camera);
  //  if(camera) {
  //    cvSetCaptureProperty(camera , CV_CAP_PROP_FPS , /*camera_width_*/5);
  //    cvSetCaptureProperty(camera , CV_CAP_PROP_FRAME_WIDTH , camera_width_);
  //    cvSetCaptureProperty(camera , CV_CAP_PROP_FRAME_HEIGHT , camera_height_);
  //  }
  //}
  //  SetCaptureProperty(&video_capture_,  CV_CAP_PROP_FRAME_WIDTH, camera_width_);
  if (!camera_ptr->video_capture_ptr->open(open_index)) {
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.video"), "video_capture_ptr->open(" <<open_index << ") faild! ");
    return YSOS_ERROR_FAILED;
  }
  if (!camera_ptr->video_capture_ptr->isOpened()) {
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.video"), "video_capture_ptr->isOpened() faild! ");
    return YSOS_ERROR_FAILED;
  }
  //  frate = video_capture_.get(CV_CAP_PROP_FPS);
  /// 注意以下值只能是在 摄像头打开后才能设定，没打开前设置是没有用的
  //video_capture_.set(CV_CAP_PROP_FPS, /*25*/5);  ///< CV_CAP_PROP_FPS Frame rate.
  if (width >0) {
    camera_ptr->video_capture_ptr->set(CV_CAP_PROP_FRAME_WIDTH, /*width_*/ width);
  }
  if (height >0) {
    camera_ptr->video_capture_ptr->set(CV_CAP_PROP_FRAME_HEIGHT, /*height_*/ height);
  }
  if (frame_rate_persec >0) {
    camera_ptr->video_capture_ptr->set(CV_CAP_PROP_FPS, frame_rate_persec);
  }
  camera_ptr->width = (UINT32)camera_ptr->video_capture_ptr->get(CV_CAP_PROP_FRAME_WIDTH);
  camera_ptr->height = (UINT32)camera_ptr->video_capture_ptr->get(CV_CAP_PROP_FRAME_HEIGHT);
  camera_ptr->frame_rate = (UINT32)camera_ptr->video_capture_ptr->get(CV_CAP_PROP_FPS);
  camera_ptr->fram_size_byte = (UINT32)camera_ptr->width * camera_ptr->height * 3;  ///< 默认3个通道
  return YSOS_ERROR_SUCCESS;
}

int VideoCaptureDriverManage::Close() {
  return Close(0);
}

int VideoCaptureDriverManage::Close(const int video_index) {
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"),  "in WindowsAudioCaptureDriver::Close(), index: " << video_index);
  if (false == mutex_video_capture_oper_.try_lock_for(boost::chrono::milliseconds(10000))) {  ///< 超时时间长一点
    //YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.video"), "调用 得到锁超时4秒，返回错误");
    return  YSOS_ERROR_FAILED;
  }
  boost::lock_guard<boost::timed_mutex> lock_guard_times(mutex_video_capture_oper_,  boost::adopt_lock);
  CameraInfoPtr camera_ptr = GetTheCamera(video_index, true);
  if (NULL == camera_ptr  || NULL == camera_ptr->video_capture_ptr) {
    assert(false);
    return YSOS_ERROR_FAILED;
  }
  if (camera_ptr->video_capture_ptr->isOpened()) {
    camera_ptr->video_capture_ptr->release();
  }
  return YSOS_ERROR_SUCCESS;
}


bool VideoCaptureDriverManage::IsOpen() {
  return IsOpen(0);
}
bool VideoCaptureDriverManage::IsOpen(const int video_index) {
  if (false == mutex_video_capture_oper_.try_lock_for(boost::chrono::milliseconds(4000))) {
    //YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.video"), "调用 得到锁超时4秒，返回错误");
    return false;
  }
  boost::lock_guard<boost::timed_mutex> lock_guard_times(mutex_video_capture_oper_,  boost::adopt_lock);
  CameraInfoPtr camera_ptr = GetTheCamera(video_index, true);
  if (NULL == camera_ptr  || NULL == camera_ptr->video_capture_ptr) {
    assert(false);
    return false;
  }
  return camera_ptr->video_capture_ptr->isOpened();
}

// int VideoCaptureDriverManage::test() {
//   /* VideoCapture cap(0);
//   if(!cap.isOpened())
//   {
//   return -1;
//   }  */
//   video_capture_.open(0);
//   if (!video_capture_.isOpened()) {
//     return -1;
//   }
//   cv::Mat frame;
//   cv::Mat edges;
//
//   bool stop = false;
//   while (!stop) {
//     video_capture_>>frame;
//     /* cvtColor(frame, edges, CV_BGR2GRAY);
//     GaussianBlur(edges, edges, Size(7,7), 1.5, 1.5);
//     Canny(edges, edges, 0, 30, 3);
//     imshow("当前视频",edges);  */
//     imshow("当前视频",/*edges*/frame);
//     if (waitKey(30) >=0)
//       stop = true;
//   }
//   return 0;
// }


// int VideoCaptureDriverManage::GetWidth(const int nOpenIndex) {
//   double fwidht = video_capture_.get(CV_CAP_PROP_FRAME_WIDTH);
//   return static_cast<int>(fwidht);
// }
// int VideoCaptureDriverManage::GetHeight(const int nOpenIndex) {
//   double fheight = video_capture_.get(CV_CAP_PROP_FRAME_HEIGHT);
//   return static_cast<int>(fheight);
// }

int VideoCaptureDriverManage::GetFrame(UINT32 &frame_width, UINT32 &frame_height, UINT32 &frame_channel, int buf_size, char *data_buf, int &recv_data_length) {
  return GetFrame(0, frame_width, frame_height, frame_channel, buf_size, data_buf, recv_data_length);
}
int VideoCaptureDriverManage::GetFrame(const int video_index, UINT32 &frame_width, UINT32 &frame_height, UINT32 &frame_channel, int buf_size, char *data_buf, int &recv_data_length) {
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"),  "VideoCaptureDriverManage::GetFrame [Enter]");
  recv_data_length =0;
  cv::Mat video_frame;
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"),  "VideoCaptureDriverManage::GetFrame [Check][0]");
  int iret = GetFrame(video_index, video_frame);
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"),  "VideoCaptureDriverManage::GetFrame [Check][0][iret]=" << iret);
  if (YSOS_ERROR_SUCCESS != iret) {
    return iret;
  }
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"),  "VideoCaptureDriverManage::GetFrame [Check][1]");
  frame_width = video_frame.cols;
  frame_height = video_frame.rows;
  frame_channel = video_frame.channels();
  int frame_size = video_frame.dataend - video_frame.data;
  assert(buf_size >= frame_size);
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"),  "VideoCaptureDriverManage::GetFrame [Check][1][buf_size]=" << buf_size);
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"),  "VideoCaptureDriverManage::GetFrame [Check][1][frame_size]=" << frame_size);
  if (buf_size < frame_size) {
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.video"),  "error the recv buff size is too small, buffer size: " << buf_size << "frame size: " << frame_size);
    return YSOS_ERROR_FAILED;
  }
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"),  "VideoCaptureDriverManage::GetFrame [Check][2]" );
#ifdef _WIN32
  memcpy_s(data_buf, buf_size, video_frame.data, frame_size);
#else
  memcpy(data_buf, video_frame.data, frame_size);
#endif
  recv_data_length = frame_size;
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"),  "VideoCaptureDriverManage::GetFrame [Check][2][recv_data_length]=" << recv_data_length);
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"),  "VideoCaptureDriverManage::GetFrame [Check][3]");
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"),  "VideoCaptureDriverManage::GetFrame [End]");
  return YSOS_ERROR_SUCCESS;
}

int VideoCaptureDriverManage::GetFrame(const int video_index, cv::Mat &recvFrame) {
  //video_capture_>>recvFrame;
  //读入下一帔，视频自动前过一步
  // bool read_ret = video_capture_.read(recvFrame);
  //#ifdef _DEBUG
  //  ::MessageBox(NULL, "TEST IN VIDEO CAPTURE DRIVER", _T("TEST"), MB_OK);
  //#endif
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"),  "VideoCaptureDriverManage::GetFrame2 [Enter]");
  CameraInfoPtr camera_ptr = GetTheCamera(video_index, true);
  if (NULL == camera_ptr  || NULL == camera_ptr->video_capture_ptr) {
    assert(false);
    return false;
  }
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"),  "VideoCaptureDriverManage::GetFrame2 [Check][0]");
  if (!camera_ptr->video_capture_ptr->isOpened()) {
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.video"), "error the camer is not open, index: " << video_index);
    assert(false);
    return YSOS_ERROR_FAILED;
  }
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"),  "VideoCaptureDriverManage::GetFrame2 [Check][1]");
  //bool read_ret  = camera_ptr->video_capture_ptr->retrieve(recvFrame);
  //TODO: unknown why read can work but retrieve dont using
  bool read_ret  = camera_ptr->video_capture_ptr->read(recvFrame);// add for linux
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"),  "VideoCaptureDriverManage::GetFrame2 [Check][1][read_ret]=" << read_ret);
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"),  "VideoCaptureDriverManage::GetFrame2 [Check][2]");
  // 旋转
  if (rotate_mat_.empty()) {
    cv::Point2f center = cv::Point2f((float)recvFrame.cols / 2, (float)recvFrame.rows / 2);
    rotate_mat_ = cv::getRotationMatrix2D(center, video_rotate_angle_, 1.0);
  }
  cv::warpAffine(recvFrame, recvFrame, rotate_mat_, recvFrame.size());
  web_capture_.OnVideoFrame(recvFrame);
  //add for debug
  if (false) {
    Mat ttframe; 
    {
    namedWindow("window_name",WINDOW_AUTOSIZE);
	  while (camera_ptr->video_capture_ptr->isOpened())
	  {
        //retrieve||read
        //camera_ptr->video_capture_ptr->read(ttframe);
        ttframe = recvFrame.clone();    
        imshow("window_name", ttframe);
        if (waitKey(10000))
        {
          destroyAllWindows();
          break;
        }
    }
    //camera_ptr->video_capture_ptr->release();
    //destroyAllWindows();
    }
    {
      vector<int> vecCompression_params;
      vecCompression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
      vecCompression_params.push_back(90);
      {
        vector<uchar> vecImg;
        imencode(".jpg", ttframe, vecImg, vecCompression_params);
        std::string mybase64_string = GetUtility()->Base64Encode(vecImg.data(), vecImg.size());
        YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"), "mybase64string: " << mybase64_string.c_str());
        //vecimage_data.insert(vecimage_data.begin(), mybase64_string.c_str(), (mybase64_string.c_str() + mybase64_string.length()));
      }
    } 
  }
  //end add for linux
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"),  "VideoCaptureDriverManage::GetFrame2 [Check][3]");
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"),  "VideoCaptureDriverManage::GetFrame2 [End]");
  // assert(read_ret && false == recvFrame.empty());
  //g_Log.Trace_Print(TraceLogLevel_DEBUG, __FILE__, __LINE__, "fram width :%d, height:%d", recvFrame.cols, recvFrame.rows);
  return  read_ret ? YSOS_ERROR_SUCCESS : YSOS_ERROR_FAILED;
}


int VideoCaptureDriverManage::GetCameraInfo(UINT32 &frame_width, UINT32 &frame_height) {
  return GetCameraInfo(0, frame_width, frame_height);
}
int VideoCaptureDriverManage::GetCameraInfo(const int video_index, UINT32 &frame_width, UINT32 &frame_height) {
  CameraInfoPtr camera_ptr = GetTheCamera(video_index, true);
  if (NULL == camera_ptr  || NULL == camera_ptr->video_capture_ptr) {
    assert(false);
    return false;
  }
  frame_width = camera_ptr->width;
  frame_height = camera_ptr->height;
  return YSOS_ERROR_SUCCESS;
}


int VideoCaptureDriverManage::CameraCount() {
  int count = 0;
#ifdef _WIN32
  CoInitialize(NULL);
  // enumerate all video capture devices
  CComPtr<ICreateDevEnum> pCreateDevEnum;
  HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
                                IID_ICreateDevEnum, (void **)&pCreateDevEnum);
  CComPtr<IEnumMoniker> pEm;
  hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,
       &pEm, 0);
  if (hr != NOERROR) {
    //_LogTTrace<<"Camera Count："<<count<<" "<<_LogTTraceEnd;
    return count;
  }
  pEm->Reset();
  ULONG cFetched;
  IMoniker *pM;
  while (hr = pEm->Next(1, &pM, &cFetched), hr==S_OK) {
    count++;
  }
  pCreateDevEnum = NULL;
  pEm = NULL;
  //_LogTTrace<<"Camera Count："<<count<<" "<<_LogTTraceEnd;
#else
//TODO:add for linux
  count = 1;
#endif
  return count;
}

int VideoCaptureDriverManage::CameraName(int nCamID, const char *sName, int nBufferSize) {
  int count = 0;
#ifdef _WIN32
  CoInitialize(NULL);
  // enumerate all video capture devices
  CComPtr<ICreateDevEnum> pCreateDevEnum;
  HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
                                IID_ICreateDevEnum, (void **)&pCreateDevEnum);
  CComPtr<IEnumMoniker> pEm;
  hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,
       &pEm, 0);
  if (hr != NOERROR) {
    return 0;
  }
  pEm->Reset();
  ULONG cFetched;
  IMoniker *pM;
  while (hr = pEm->Next(1, &pM, &cFetched), hr==S_OK) {
    if (count == nCamID) {
      IPropertyBag *pBag=0;
      hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
      if (SUCCEEDED(hr)) {
        VARIANT var;
        var.vt = VT_BSTR;
        hr = pBag->Read(L"FriendlyName", &var, NULL); //还有其他属性,像描述信息等等...
        if (hr == NOERROR) {
          //获取设备名称
          WideCharToMultiByte(CP_ACP,0,var.bstrVal,-1,sName, nBufferSize,"",NULL);
          SysFreeString(var.bstrVal);
        }
        pBag->Release();
      }
      pM->Release();
      break;
    }
    count++;
  }
  pCreateDevEnum = NULL;
  pEm = NULL;
#else
//TODO:add for linux
nCamID = 0;
sName = "/dev/video0";
#endif
  return /*1*/YSOS_ERROR_SUCCESS;
}


int VideoCaptureDriverManage::TakeAPicture(const std::string img_fromat_extension, std::vector<uchar> &vecimage_data) {
  return TakeAPicture(0, -1, -1, img_fromat_extension, false, -1, vecimage_data);
}

/*
 支持的图片格式，官网上的说明
 Windows bitmaps - *.bmp, *.dib (always supported)
 JPEG files - *.jpeg, *.jpg, *.jpe (see the Notes section)
 JPEG 2000 files - *.jp2 (see the Notes section)
 Portable Network Graphics - *.png (see the Notes section)
 WebP - *.webp (see the Notes section)
 Portable image format - *.pbm, *.pgm, *.ppm (always supported)
 Sun rasters - *.sr, *.ras (always supported)
 TIFF files - *.tiff, *.tif (see the Notes section)

 图片压缩率，官网上的说明
 For JPEG, it can be a quality ( CV_IMWRITE_JPEG_QUALITY ) from 0 to 100 (the higher is the better). Default value is 95.
 For WEBP, it can be a quality ( CV_IMWRITE_WEBP_QUALITY ) from 1 to 100 (the higher is the better). By default (without any parameter) and for quality above 100 the lossless compression is used.
 For PNG, it can be the compression level ( CV_IMWRITE_PNG_COMPRESSION ) from 0 to 9. A higher value means a smaller size and longer compression time. Default value is 3.
 For PPM, PGM, or PBM, it can be a binary format flag ( CV_IMWRITE_PXM_BINARY ), 0 or 1. Default value is 1.
 */
int VideoCaptureDriverManage::TakeAPicture(const int video_index,const UINT32 width, const UINT32 height, const std::string img_fromat_extension,
    const UINT8 base64_convert,  UINT8 image_quality, std::vector<uchar> &vecimage_data) {
  CameraInfoPtr camera_ptr = GetTheCamera(video_index, true);
  if (NULL == camera_ptr  || NULL == camera_ptr->video_capture_ptr) {
    assert(false);
    return YSOS_ERROR_FAILED;
  }
  if (!camera_ptr->video_capture_ptr->isOpened()) {
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.video"), "error , in TDTakeAPicture video is not open, index: " << video_index);
    return YSOS_ERROR_FAILED;
  }
  cv::Mat frame;
  //video_capture_ptr_->video_capture_>>frame;
  if (YSOS_ERROR_SUCCESS != GetFrame(video_index, frame)) {
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.video"), "获取摄像头数据失败");
    return YSOS_ERROR_FAILED;
  }
  if (NULL == frame.data) {
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.video"), "error in TDTakeAPicture 当前摄像头没有数据");
    return YSOS_ERROR_FAILED;
  }
#ifdef _WIN32  
  // 根据输出图片扩展名来
  if (stricmp(img_fromat_extension.c_str(), ".jpg") ==0) {
    //vector<uchar> vecImg;                               //Mat 图片数据转换为vector<uchar>
    vector<int> vecCompression_params;
    vecCompression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
    vecCompression_params.push_back(90);
    if (base64_convert) {
      vector<uchar> vecImg;
      imencode(".jpg", frame, /*vecImg*/vecimage_data, vecCompression_params);
      std::string mybase64_string = ZBase64_Encode(vecImg.data(), vecImg.size());
      YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"), "mybase64string: " << mybase64_string.c_str());
      vecimage_data.insert(vecimage_data.begin(), mybase64_string.c_str(), (mybase64_string.c_str() + mybase64_string.length()));
    } else {
      imencode(".jpg", frame, /*vecImg*/vecimage_data, vecCompression_params);
    }
    return YSOS_ERROR_SUCCESS;
  } else {
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.video"), "error not supoort image type");
    assert(false);
  }
#else
// 根据输出图片扩展名来
  if (strcasecmp(img_fromat_extension.c_str(), ".jpg") ==0) {
    //vector<uchar> vecImg;                               //Mat 图片数据转换为vector<uchar>
    vector<int> vecCompression_params;
    vecCompression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
    vecCompression_params.push_back(90);
    if (base64_convert) {
      vector<uchar> vecImg;
      imencode(".jpg", frame, /*vecImg*/vecimage_data, vecCompression_params);
      std::string mybase64_string = GetUtility()->Base64Encode(vecImg.data(), vecImg.size());
      YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"), "mybase64string: " << mybase64_string.c_str());
      vecimage_data.insert(vecimage_data.begin(), mybase64_string.c_str(), (mybase64_string.c_str() + mybase64_string.length()));
    } else {
      imencode(".jpg", frame, /*vecImg*/vecimage_data, vecCompression_params);
    }
    return YSOS_ERROR_SUCCESS;
  } else {
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.video"), "error not supoort image type");
    assert(false);
  }
#endif
  //*strPictureBase64 = _com_util::ConvertStringToBSTR(mybase64_string.c_str());
  return YSOS_ERROR_FAILED;
}

std::string VideoCaptureDriverManage::GetCaptureResult() {
  return web_capture_.GetCaptureResult();
}

void VideoCaptureDriverManage::StartWebCapture(
  int x, int y, int width, int height) {
  web_capture_.Start(x, y, width, height);
}

void VideoCaptureDriverManage::PauseWebCapture() {
  web_capture_.Pause();
}

void VideoCaptureDriverManage::StopWebCapture() {
  web_capture_.Stop();
}

CameraInfoPtr VideoCaptureDriverManage::GetTheCamera(const int index, bool create_always) {
  std::map<int, CameraInfoPtr>::iterator find_it = map_camera_info_.find(index);
  if (find_it == map_camera_info_.end()) {
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.video"), "error ,getvideocapture , camera can't find, index: " << index);
    //assert(false);
    return NULL;
  }
  CameraInfoPtr ptr = find_it->second;
  //assert(ptr);
  //YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"), "DEBUG ,getvideocapture , GetTheCamera, index: " << index << "  create_always: "<< create_always);
  if (NULL == ptr->video_capture_ptr && create_always) { ///< 如果没有创建摄像头对象 ，同时指定了要创建此对象，则创建此对象
    ptr->video_capture_ptr = VideoCapturePtr(new VideoCapture);
    //assert(ptr->video_capture_ptr);
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"), "DEBUG ,getvideocapture , GetTheCamera, VideoCapturePtr CREATE NEW OBJECT ... " );
  }
  //YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"), "DEBUG ,getvideocapture , GetTheCamera [over]" );
  return ptr;
}
}  /// end of namespace ysos