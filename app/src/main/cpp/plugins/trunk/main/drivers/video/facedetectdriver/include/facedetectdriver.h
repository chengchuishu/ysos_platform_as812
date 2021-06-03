/**
 *@file facedetectdriver.h
 *@brief 
 *@version 1.0
 *@author WangDaqian
 *@date Created on: 2018/1/19
 *@copyright Copyright(c) 2018 YunShen Technology. All rights reserved.
**/


#ifndef YSOS_FACE_DETECT_H
#define YSOS_FACE_DETECT_H

// ysos headers
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/common.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/driverinterface.h"

#include <boost/scoped_ptr.hpp>

namespace ysos {

class FaceCompare1vs1;
class FaceDetector;
class FaceRecognizer;
struct FaceComparisonResult1N;

class FaceDetectDriver : public DriverInterface,
                         public BaseInterfaceImpl {
  DISALLOW_COPY_AND_ASSIGN(FaceDetectDriver)
  DECLARE_PROTECT_CONSTRUCTOR(FaceDetectDriver)
  DECLARE_CREATEINSTANCE(FaceDetectDriver)
public:
  /**
   *@brief inherit from DriverInterface;
   *@param pBuffer[Input/Output]
   *@return success return YSOS_ERROR_SUCCESS，
   *        fail return one of the error types
   **/
  virtual int Read(BufferInterfacePtr pBuffer,
                   DriverContexPtr context_ptr = NULL) override {
    return YSOS_ERROR_SUCCESS;
  }

  /**
   *@brief inherit from DriverInterface;
   *@param pBuffer[Input/Output]
   *@return success return YSOS_ERROR_SUCCESS
   *        fail return one of the error types
  **/
  //virtual int Write(BufferInterfacePtr pBuffer);
  virtual int Write(BufferInterfacePtr input_buffer,
                    BufferInterfacePtr output_buffer = NULL,
                    DriverContexPtr context_ptr = NULL) override {
    return YSOS_ERROR_SUCCESS;
  }

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
  **/
  int Ioctl(int iCtrlID, BufferInterfacePtr pInputBuffer,
            BufferInterfacePtr pOutputBuffer) override;

  /**
   *@brief inherit from DriverInterface
   *@      only open device(not include start recorder)
   *@param pParams[Input]
   *@return success return YSOS_ERROR_SUCCESS,
   *        fail return (YSOS_ERROR_MEMORY_ALLOCATED_FAILED,YSOS_ERROR_LOGIC_ERROR)
  **/
  virtual int Open(void *pParams = NULL) override {
    return YSOS_ERROR_SUCCESS;
  }

  /**
   *@brief inherit from DriverInterface
  **/
  virtual void Close(void *pParams = NULL) {
  }

  /**
   *@brief inherit from BaseInterface
   *@return success return YSOS_ERROR_SUCCESS,
   *        fail return one of(YSOS_ERROR_LOGIC_ERROR)
  **/
  virtual int Initialize(int width, int height) {
    return YSOS_ERROR_SUCCESS;
  }

  /**
  *@brief 在系统退出时，会依次调Module、Callback和Driver的uninitialize的,在Close后
   *@param 
   *@return   
  **/
  virtual int RealUnInitialize(void *param = NULL) {
    return YSOS_ERROR_SUCCESS;
  }
    
  /**
   *@brief inherit from BaseInterface
   *@param iTypeId[Input] property id
   *@param piType[Output] property value
   *@return success return YSOS_ERROR_SUCCESS,
   *        fail return one of (YSOS_ERROR_INVALID_ARGUMENTS)
  **/
  virtual int GetProperty(int iTypeId, void *piType);

  /**
   *@brief inherit from BaseInterface
   *@param iTypeId[Input] property id
   *       if iTypeId = VoiceDataCallbackFun, Then piType is a function pointer like void (*AudioCaptureDriverOnReadCallback)(BufferInterfacePtr pOutputBuffer);
   *@param piType[Input] property value
   *@return success return YSOS_ERROR_SUCCESS,
   *        fail return (YSOS_ERROR_INVALID_ARGUMENTS)
  **/
  virtual int SetProperty(int iTypeId, void *piType);

private:
  // 检测到人脸后通知识别线程，然后返回
  int DetectFaceAsync(void* param);
  // 获取人脸识别结果
  int GetFaceRecognizeResult(FaceComparisonResult1N *result);
  // 获取人脸1vs1结果
  int GetFaceCompareResult(int* result);

  // 人脸检测接口
  boost::scoped_ptr<FaceDetector> face_detector_;
  // 人脸识别接口
  boost::scoped_ptr<FaceRecognizer> face_recognizer_;
  // 人脸1比1接口
  boost::scoped_ptr<FaceCompare1vs1> face_compare_;
  float min_face_score_;
  // 是否只识别最大人脸
  bool only_biggest_face_;
  
};

} // namespace ysos

#endif  // YSOS_FACE_DETECT_H