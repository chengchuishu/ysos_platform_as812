/**   //NOLINT
  *@file hongrunfacedetectdriver.h
  *@brief Definition of
  *@version 0.1
  *@author wanfenfen
  *@date Created on: 2018:4:12   14:57
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  *@howto Usage:
  *@todo
  */

#ifndef YSOS_PLUGIN_HONGRUN_FACE_DETECT_DRIVER_H
#define YSOS_PLUGIN_HONGRUN_FACE_DETECT_DRIVER_H


// boost headers
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_array.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/atomic.hpp>

// ysos headers
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/common.h"
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/bufferpool.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/driverinterface.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basedriverimpl.h"

// private headers
#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"
#include "facedetector.h"
#include "facerecognizemanager.h"

namespace ysos {


    //class HongRunFaceDetectDriver : public DriverInterface,
    //  public BaseInterfaceImpl {
    //  DISALLOW_COPY_AND_ASSIGN(HongRunFaceDetectDriver)
    //  DECLARE_PROTECT_CONSTRUCTOR(HongRunFaceDetectDriver)
    //  DECLARE_CREATEINSTANCE(HongRunFaceDetectDriver)
    class HongRunFaceDetectDriver : public BaseDriverImpl {
            DISALLOW_COPY_AND_ASSIGN ( HongRunFaceDetectDriver )
            DECLARE_PROTECT_CONSTRUCTOR ( HongRunFaceDetectDriver )
            DECLARE_CREATEINSTANCE ( HongRunFaceDetectDriver )
        public:
            /**
             *@brief inherit from DriverInterface;
             *@param pBuffer[Input/Output]
             *@return success return YSOS_ERROR_SUCCESS，
             *        fail return one of the error types
             **/
            virtual int Read ( BufferInterfacePtr pBuffer,
                               DriverContexPtr context_ptr = NULL ) override {
                return YSOS_ERROR_SUCCESS;
            }

            /**
             *@brief inherit from DriverInterface;
             *@param pBuffer[Input/Output]
             *@return success return YSOS_ERROR_SUCCESS
             *        fail return one of the error types
            **/
            //virtual int Write(BufferInterfacePtr pBuffer);
            virtual int Write ( BufferInterfacePtr input_buffer,
                                BufferInterfacePtr output_buffer = NULL,
                                DriverContexPtr context_ptr = NULL ) override {
                return YSOS_ERROR_SUCCESS;
            }


            /**
             *@brief inherit from DriverInterface
             *@      only open device(not include start recorder)
             *@param pParams[Input]
             *@return success return YSOS_ERROR_SUCCESS,
             *        fail return (YSOS_ERROR_MEMORY_ALLOCATED_FAILED,YSOS_ERROR_LOGIC_ERROR)
            **/
            virtual int Open ( void *pParams = NULL ) override;

            /**
             *@brief inherit from DriverInterface
            **/
            virtual void Close ( void *pParams = NULL );

            /**
             *@brief inherit from BaseInterface
             *@return success return YSOS_ERROR_SUCCESS,
             *        fail return one of(YSOS_ERROR_LOGIC_ERROR)
            **/
            virtual int Initialize ( int width, int height ) {
                return YSOS_ERROR_SUCCESS;
            }

            /**
            *@brief 在系统退出时，会依次调Module、Callback和Driver的uninitialize的,在Close后
             *@param
             *@return
            **/
            virtual int RealUnInitialize ( void *param = NULL ) {
                return YSOS_ERROR_SUCCESS;
            }

            /**
             *@brief inherit from BaseInterface
             *@param iTypeId[Input] property id
             *@param piType[Output] property value
             *@return success return YSOS_ERROR_SUCCESS,
             *        fail return one of (YSOS_ERROR_INVALID_ARGUMENTS)
            **/
            virtual int GetProperty ( int iTypeId, void *piType );

            /**
             *@brief inherit from BaseInterface
             *@param iTypeId[Input] property id
             *       if iTypeId = VoiceDataCallbackFun, Then piType is a function pointer like void (*AudioCaptureDriverOnReadCallback)(BufferInterfacePtr pOutputBuffer);
             *@param piType[Input] property value
             *@return success return YSOS_ERROR_SUCCESS,
             *        fail return (YSOS_ERROR_INVALID_ARGUMENTS)
            **/
            virtual int SetProperty ( int iTypeId, void *piType );


            virtual int CallAbility ( AbilityParam* pInput, AbilityParam* pOut );
            virtual int AbilityFaceDetectRecognition_RG24BP62TextJson ( AbilityParam* pInput, AbilityParam* pOut );

            //////////////////////////////////////////////////////////////////////////
            /// 以下是为发兼容公版YSOS，参考模块face_detect_driver
            // 检测到人脸后通知识别线程，然后返回
            int DetectFaceAsync ( void* param );
            // 获取人脸识别结果
            int GetFaceRecognizeResult ( FaceComparisonResult1N *result );
            // 获取人脸1vs1结果
            int GetFaceCompareResult ( int* result );

            int Ioctl ( int iCtrlID, BufferInterfacePtr pInputBuffer,
                        BufferInterfacePtr pOutputBuffer ) override;

            std::vector<TDFaceDetectUnit> last_time_faces_info_;
            boost::atomic<int> base_track_id_;
            boost::atomic<int> cmp_track_id_;

            //////////////////////////////////////////////////////////////////////////

        protected:
            /**
            *@brief  配置文件中设置的Key、Value对，会在Initialized函数中调用 //  NOLINT
            *@param key 配置参数中的Key  // NOLINT
            *@param value 与Key对应的Value  // NOLINT
            *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
            */
            virtual int Initialized ( const std::string &key, const std::string &value );
            //virtual int CallAbility(AbilityParam* pInput, AbilityParam* pOut);
        private:
            // 检测到人脸后通知识别线程，然后返回
            int DetectFaceRecognize ( FaceDetectRequest* pin, std::vector<TDFaceDetectUnit>& vec_face_unit );
            // 获取人脸识别结果
            //int GetFaceRecognizeResult(FaceComparisonResult1N *result);

            // 检测去掉字符串前后指定字符
            std::string trim(const char * src, char ch); 

            // 人脸检测接口
            FaceDetectorInterfacePtr face_detect_ptr_;
            FaceRecognizeManager face_recognize_manager_;
            FaceComparisonInterfacePtr face_recognize_ptr_;

            std::map<std::string, std::string> map_driver_param_;  ///< driver文件的配置参数
            // 人脸识别接口
            //boost::scoped_ptr<FaceRecognizer> face_recognizer_;
            //   float min_face_score_;
            //   // 是否只识别最大人脸
            //   bool only_biggest_face_;

    };

}

#endif
