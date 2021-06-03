/**   //NOLINT
  *@file videocapturedriver.h
  *@brief Definition of VideoCaptureDriver for windows.
  *      注意事项：此驱动一个实体，只能操作一个摄像头，如果要操作多个摄像头，需要创建多个实体
  *@version 0.1
  *@author venucia
  *@date Created on: 2016/6/13   19:45
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  *
  */
#ifndef YSOS_PLUGIN_WINDOWSVEDIOCAPTUREDRIVER_H  //NOLINT
#define YSOS_PLUGIN_WINDOWSVEDIOCAPTUREDRIVER_H  //NOLINT

#ifdef _WIN32
/// windows headers
#include <atlbase.h>
#include <windows.h>
#else

#endif

/// stl headers
#include <vector>  ///< TODO 放到前面会报错： Found C system header after C++ system header.  //NOLINT

/// boost headers
#include <boost/shared_ptr.hpp>  //NOLINT
#include <boost/thread/thread.hpp>  //NOLINT
#include <boost/shared_array.hpp>  //NOLINT
#include <boost/thread/mutex.hpp>   //NOLINT
#include <boost/atomic.hpp>

/// ysos headers
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/common.h"
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/bufferpool.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/driverinterface.h"

#ifdef _WIN32
/// third party directshow headers
#include <DirectShow/Include/qedit.h>
#include <DirectShow/Include/dshow.h>
#else
#endif

/// private headers
#include "../include/videocapturedrivermanager.h"
#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"

namespace ysos {

//typedef boost::shared_ptr<WAVEHDR> WAVEHDRPtr;
/**
  *@brief VideoCaptureDriver for windows.
  */
class YSOS_EXPORT  VideoCaptureDriver: public DriverInterface, public BaseInterfaceImpl {
  DISALLOW_COPY_AND_ASSIGN(VideoCaptureDriver)
  DECLARE_PROTECT_CONSTRUCTOR(VideoCaptureDriver)
  DECLARE_CREATEINSTANCE(VideoCaptureDriver)
 public:
  //enum PropertiesEx {
  //  BufferLength = 0,       ///< buffer length ,datatype:UINT32
  //  BufferNumber,           ///< datatype:UINT32
  //  MemoryBlockAlign  ,      ///< block align (unit:Byte), datatype:UINT32
  //  VoiceDataCallbackFun      ///< voice data callback function
  //};
  //enum IoCtlCommand {
  //  ICC_SetOutputBuf =0,
  //  ICC_Start,
  //  ICC_Stop
  //};
  virtual ~VideoCaptureDriver();


  /**
   *@brief inherit from DriverInterface;
   *@param pBuffer[Input/Output]
   *@return success return YSOS_ERROR_SUCCESS，
   *        fail return one of the error types
   */
  //virtual int Read(BufferInterfacePtr pBuffer);
  virtual int Read(BufferInterfacePtr pBuffer, DriverContexPtr context_ptr=NULL);

  /**
   *@brief inherit from DriverInterface;
   *@param pBuffer[Input/Output]
   *@return success return YSOS_ERROR_SUCCESS
   *        fail return one of the error types
   */
  //virtual int Write(BufferInterfacePtr pBuffer);
  virtual int Write(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer=NULL, DriverContexPtr context_ptr=NULL);

  /**
   *@brief inherit from DriverInterface ,setting input or output buffer
   *@param iCtrlID[Input]
   *       0 set output buffer, relate param pOutputBuffer
   *       1 start record
   *       2 stop record
   *@param pInputBuffer[Input/Output]
   *@param pOutputBuffer[Input/Output]
   *@return success return YSOS_ERROR_SUCCESS，
   *        fail return one of the error types
   */
  virtual int Ioctl(int iCtrlID, BufferInterfacePtr pInputBuffer, BufferInterfacePtr pOutputBuffer);

  /**
   *@brief inherit from DriverInterface
   *@      only open device(not include start recorder)
   *@param pParams[Input]
   *@return success return YSOS_ERROR_SUCCESS,
   *        fail return (YSOS_ERROR_MEMORY_ALLOCATED_FAILED,YSOS_ERROR_LOGIC_ERROR)
   */
  virtual int Open(void *pParams = NULL);

  /**
   *@brief inherit from DriverInterface
   */
  virtual void Close(void *pParams = NULL);

    /**
  *@brief 在系统退出时，会依次调Module、Callback和Driver的uninitialize的,在Close后
   *@param 
   *@return   
   */
  virtual int RealUnInitialize(void *param=NULL);

  /**
   *@brief inherit from BaseInterface
   *@return success return YSOS_ERROR_SUCCESS,
   *        fail return one of(YSOS_ERROR_LOGIC_ERROR)
   */
  //virtual int Initialize(int width, int height);

    /**
   *@brief inherit from BaseInterface
   *@param iTypeId[Input] property id
   *@param piType[Output] property value
   *@return success return YSOS_ERROR_SUCCESS,
   *        fail return one of (YSOS_ERROR_INVALID_ARGUMENTS)
   */
  virtual int GetProperty(int iTypeId, void *piType);

  /**
   *@brief inherit from BaseInterface
   *@param iTypeId[Input] property id
   *       if iTypeId = VoiceDataCallbackFun, Then piType is a function pointer like void (*AudioCaptureDriverOnReadCallback)(BufferInterfacePtr pOutputBuffer);
   *@param piType[Input] property value
   *@return success return YSOS_ERROR_SUCCESS,
   *        fail return (YSOS_ERROR_INVALID_ARGUMENTS)
   */
  virtual int SetProperty(int iTypeId, void *piType);
 
  /**
   *@brief 提供的能力
   *  videocapture
   *      in:null
   *      out:stream_frame_rgb24
   *@param 
   *@return   
   */
  virtual int CallAbility(AbilityParam* input_param, AbilityParam* output_param);
  virtual int AbilityVideoCapture_StreamRGB24(AbilityParam* input_param, AbilityParam* output_param);
  virtual int AbilityVideoCapture_StreamRGB24P6(AbilityParam* input_param, AbilityParam* output_param);
 private:
   //摄像头配置的参数
   struct CameraOpenParam {
     std::string camera_name;
     int index;  ///<-1 表示默认
     int width;  ///<-1 表示默认
     int height; ///<-1 表示默认
     int frame_rate; ///<-1 表示默认
     int horizontal_max_angle;  ///< 水平方向最大可拍摄角度
     int vertical_max_angle;    ///< 垂直方向最大可拍摄角度

     CameraOpenParam() {
       index = -1;
       width =-1;
       height =-1;
       frame_rate =-1;
       horizontal_max_angle =-1;
       vertical_max_angle =-1;
     }
   };
   typedef boost::shared_ptr<CameraOpenParam> CameraConfigInitParamPtr;


  /**
   *@brief 解析打开参数
   *@param open_param[Input] 如果字符串为空，则使用默认设置（摄像头索引0），必须指定摄像头名或索引号，其他的可以为默认
   *                         格式如下：name=Logitech HD Webcam C270;width=640;height=360;frame_rate=25;horizontal_max_angle=60;vertical_max_angel=60
   *@return 成功，所有的参数都解决成功。
   */
  int DecodeOpenParam(const std::string& open_param, CameraOpenParam& camera_param);
  VideoCaptureDriverManage video_capture_manager_;
  boost::atomic<int> current_oper_camera_index_;   ///< 当前的待操作的摄像头索引号，默认值是-1，表示没有操作的摄像头
};

typedef boost::shared_ptr<VideoCaptureDriver> VideoCaptureDriverPtr;

}  ///< namespace ysos
#endif  /// <OHP_WINDOWSVIDEOCAPTUREDRIVER_H   //NOLINT
