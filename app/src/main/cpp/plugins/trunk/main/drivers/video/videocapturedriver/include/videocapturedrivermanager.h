/**   //NOLINT
  *@file VideoCaptureDriverImp.h
  *@brief Definition of VideoCaptureDriverImp for windows.
  *@version 0.1
  *@author venucia
  *@date Created on: 2016/6/13   19:45
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  *
  */
#ifndef YSOS_PLUGIN_WINDOWSVEDIOCAPTUREDRIVERIMP_H  //NOLINT
#define YSOS_PLUGIN_WINDOWSVEDIOCAPTUREDRIVERIMP_H  //NOLINT

#include "../include/web_video_capture.h"

#ifdef _WIN32
/// windows headers
#include <atlbase.h>
#include <windows.h>
#else
#endif

/// stl headers
#include <vector>  ///< TODO 放到前面会报错： Found C system header after C++ system header.  //NOLINT
#include <map>

/// boost headers
#include <boost/shared_ptr.hpp>  //NOLINT
#include <boost/thread/thread.hpp>  //NOLINT
#include <boost/shared_array.hpp>  //NOLINT
#include <boost/thread/mutex.hpp>   //NOLINT
#include <boost/atomic/atomic.hpp>

/// ysos headers
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/common.h"
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/bufferpool.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/driverinterface.h"

#ifdef _WIN32
/// directshow headers
#include <DirectShow/include/qedit.h>
#include <DirectShow/include/dshow.h>
#else
#endif

/// opencv headers
#include "../../../../../thirdparty/opencv/include/opencv2/opencv.hpp"
#include "../../../../../thirdparty/opencv/include/opencv2/highgui/highgui.hpp"
#include "../../../../../thirdparty/opencv/include/opencv2/imgproc/imgproc.hpp"
#include "../../../../../thirdparty/opencv/include/opencv2/core/core.hpp"
 
using namespace std;
using namespace cv;


namespace ysos {

typedef boost::shared_ptr<VideoCapture> VideoCapturePtr;
/**
 *@brief 摄像头信息
 */
 struct CameraInfo {
   VideoCapturePtr video_capture_ptr;
   UINT32 camera_index;
   std::string camera_name;
   UINT32 width;   ///<  打开摄像头中配置的宽度（像素）, 0 表示不设置，使用默认设置打开设置头，默认是640*320
   UINT32 height;  ///<  打开摄像头时配置的高度（像素），0 表示不设置，使用默认设置打开摄像头
   UINT32 frame_rate;   ///< 打开摄像头时配置的帧速度, 0 表示不设置，默认是0
   UINT32 fram_size_byte;  ///<  每一帧数据的大小

   CameraInfo() {
     video_capture_ptr = NULL;
     camera_index =0;
     //memset(camera_name_buf, 0, sizeof(camera_name_buf));
     width = 0;
     height =0;
     frame_rate =0;
   }
 };
 typedef boost::shared_ptr<CameraInfo> CameraInfoPtr;


//typedef boost::shared_ptr<WAVEHDR> WAVEHDRPtr;
/**
  *@brief VideoCaptureDriverImp for windows.
  */
class    VideoCaptureDriverManage {
public:
  VideoCaptureDriverManage();
  virtual ~VideoCaptureDriverManage();

    /**
   *@brief 初始化，扫描当前的摄像头
   *@param 
   *@return success return YSOS_ERROR_SUCCESS,
   *        fail return one of(YSOS_ERROR_LOGIC_ERROR)
   */
  virtual int Initialize();
  virtual int UnInitialize();  ///< 释放所有摄像头资源

  // 简单模式，只操作默认的摄像头（不指定)
  virtual int Open();
  virtual int Close();
  virtual bool IsOpen();
  virtual int GetFrame(UINT32& frame_width, UINT32& frame_height, UINT32& frame_channel, int buf_size, char* data_buf, int& recv_data_length);
  virtual int GetCameraInfo(UINT32& frame_width, UINT32& frame_height);
  virtual int TakeAPicture(const std::string img_fromat_extension, std::vector<uchar>& vecimage_data);
  virtual int GetAllCameraList(std::list<std::string>& list_all_camera);
  virtual int GetCameraIndexAccordName(const std::string camera_name, int& camera_index);

  // 复杂模式，操作不同的摄像头
  /**
   *@brief 打开摄像头
   *@param open_index[Input] 打开的索引号
   *@parm width[Input] 打开时的宽度，如果小于或等于0，则表示使用默认设置
   *@param hegiht[Input] 打开时的高度，如果小于或等于0，则表示使用默认设置
   *@param frame_rate_persec[Input] 打开时的帧率，如果小于或等于0，则使用默认设置
   *@param 
   *@param pParams[Input]
   *@return success return YSOS_ERROR_SUCCESS,
   *        fail return (YSOS_ERROR_MEMORY_ALLOCATED_FAILED,YSOS_ERROR_LOGIC_ERROR)
   */
  virtual int Open(const int open_index , const int width, const int height, const int frame_rate_persec);
  virtual int Close(const int nOpenIndex);

  
  /**
   *@brief 是否打开了摄像头, 如果索引号不存在，则返回false
   *@param 
   *@return   
   */
  virtual bool IsOpen(const int nOpenIndex);

  /**
   *@brief 如果摄像头没有打开，则返回失败
   *@param 
   *@return   
   */
  virtual int GetFrame(const int video_index, UINT32& frame_width, UINT32& frame_height, UINT32& frame_channel, int buf_size, char* data_buf, int& recv_data_length);
 
   
  virtual int GetCameraInfo(const int video_index, UINT32& frame_width, UINT32& frame_height);
 

  virtual int TakeAPicture(const int video_index, const UINT32 width, const UINT32 height, const std::string img_fromat_extension, 
    const UINT8 base64_convert,  UINT8 image_quality, std::vector<uchar>& vecimage_data);
  
  // 设置视频旋转角度
  void SetVideoRotateAngle(int angle) { video_rotate_angle_ = angle; }

  // 获取视频采集结果
  std::string GetCaptureResult();
  // 开始视频采集
  void StartWebCapture(int x, int y, int width, int height);
  // 暂停采集
  void PauseWebCapture();
  // 结束采集
  void StopWebCapture();

private:
   /**
    *@brief 扫描注册表，得到摄像头的个数
    *@param 
    *@return   
    */
   int CameraCount();
   /**
    *@brief 根据摄像头索引，所到摄像头的名称
    *@param 
    *@return   
    */
   int CameraName(int nCamID, const char* sName, int nBufferSize);

    /**
     *@brief 得到opencv操作摄像头
     *@param index[Input] 操作摄像头的索引号, 从map_camera_info_ 中检测，
     *@param create_always[Input] 是否总是创建，如果map_camera_info_中没有，得打开创建一个
     *@return   
     */
   CameraInfoPtr GetTheCamera(const int index, bool create_always);

      /**
   *@brief 获取当前视频帔数据
   *@param 
   *@return success return YSOS_ERROR_SUCCESS,
   *        fail return one of(YSOS_ERROR_LOGIC_ERROR)
   */
  virtual int GetFrame(const int nOpenIndex, cv::Mat& recvFrame);  ///< 获取当前摄像头数据

  

  boost::timed_mutex mutex_video_capture_oper_;   ///< video capture的互斥量
 // VideoCapture video_capture_;
 // int camera_width_;   ///< camera视频的宽度, 默认值是-1，表示未设置，如果未设置，则使用默认值打开摄像头
 // int camera_height_;
  //boost::shared_ptr<cv::VideoCapture> video_capture_0_ptr_;  ///< 打开默认的第一个摄像头指针

  boost::atomic<int> have_initialize_;   ///<  是否初始化，只初化一次
 // std::multimap<std::string, int> mulmap_camera_name_id_;   ///< 当前可用的所有摄像头的名字及索引号的对应值
  std::map<int, CameraInfoPtr> map_camera_info_;
  boost::mutex mutex_map_camera_info_;                ///< 互斥操作成员变量 mulmap_camera_name_id_;  

  // 视频旋转角度
  int video_rotate_angle_;
  cv::Mat rotate_mat_;
  // 视频采集
  WebVideoCapture web_capture_;
   
};

typedef boost::shared_ptr<VideoCaptureDriverManage> VideoCaptureDriverImpPtr;

}  ///< namespace ysos
#endif  /// <OHP_WINDOWSVIDEOCAPTUREDRIVER_H   //NOLINT
