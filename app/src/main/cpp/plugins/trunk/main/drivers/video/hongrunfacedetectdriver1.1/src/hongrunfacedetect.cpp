/**   //NOLINT
  *@file hongrunfacedetect.cpp
  *@brief Definition of
  *@version 0.1
  *@author wanfenfen
  *@date Created on: 2018:4:12   15:10
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  *@howto Usage:
  *@todo
  */

/// self headers
#include "../include/hongrunfacedetect.h"

/// stl headers
#include <string>
#include <map>

/// boost headers
#include <boost/locale.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

/// opencv headers
#include "../../../../../thirdparty/opencv/include/opencv2/highgui/highgui.hpp"
#include "../../../../../thirdparty/opencv/include/opencv2/imgproc/imgproc.hpp"

/// hongruan sdk headers
#include "../../../../../thirdparty/hongruan/include/merror.h"
#include "../../../../../thirdparty/hongruan/include/arcsoft_fsdk_face_detection.h"
#include "../../../../../thirdparty/hongruan/include/arcsoft_fsdk_age_estimation.h"
#include "../../../../../thirdparty/hongruan/include/arcsoft_fsdk_gender_estimation.h"
#include "../../../../../thirdparty/hongruan/include/arcsoft_fsdk_face_recognition.h"
#include "../../../../../thirdparty/hongruan/include/arcsoft_fsdk_face_tracking.h"

/// private headers
//#include "../../../public/include/common/base64.h"
#include "../include/common/mycommondefine.h"

/// hongrun lib
/*
#pragma comment(lib,"libarcsoft_fsdk_face_detection.lib")
#pragma comment(lib,"libarcsoft_fsdk_gender_estimation.lib")
#pragma comment(lib,"libarcsoft_fsdk_age_estimation.lib")
#pragma comment(lib,"libarcsoft_fsdk_face_recognition.lib")
#pragma comment(lib,"libarcsoft_fsdk_face_tracking.lib")
*/
namespace ysos {

    // 分配给引擎使用的内存大小
#define WORKBUF_SIZE_FD        (40*1024*1024)
#define WORKBUF_SIZE_GENDER    (30*1024*1024)
#define WORKBUF_SIZE_AGE       (30*1024*1024)
#define WORKBUF_SIZE_FT        (40*1024*1024)
#define WORKBUF_SIZE_FR        (40*1024*1024)


    FaceDetectComparisonHR::FaceDetectComparisonHR()
    {
        hr_max_face_count_ = 10;
        hr_min_scale_ = 16;
        hr_facedetect_engine_handle_  = NULL;
        hr_fd_work_buf_ptr_  = NULL;
        hr_face_input_rect_ptr_ = NULL;
        hr_face_input_orient_ptr_ = NULL;
        hr_gender_engine_handle_ = NULL;
        hr_gender_work_buf_ptr_ = NULL;
        hr_age_engine_handle_ = NULL;
        hr_age_work_buf_ptr_ = NULL;
        hr_facetrack_engine_handle_ = NULL;
        hr_facetrace_work_buf_ptr_ = NULL;
        hr_facerecognition_engine_handle_ = NULL;
        hr_facerecognition_work_buf_ptr_ = NULL;
        max_track_id_ = 0;
        config_score_min_face_match_ = ( float ) 0.6;
        config_face_track_min_blur_score_ = ( float ) 0.7;
        config_face_track_min_yaw_ = ( float ) 35;
        config_face_track_min_pitch_ = ( float ) 35;
        face_trace_min_cache_face_info_milliseonds_ = ( float ) 30000;
        tmp_face_image_track_hr_folder_ = "../log/tmp_face_image_track_hr";
        tmp_face_image_track_useself_folder_ = "../log/tmp_face_image_track_useself";
        tmp_face_image_track_recheck_foler_ = "../log/tmp_face_image_track_recheck";
        config_estimate_people_distance_use_cameraid_ = 1;
        config_max_horizon_angle_half_ = 30;
        config_max_vertical_angle_half_ = 30;
    }

    FaceDetectComparisonHR::~FaceDetectComparisonHR()
    {
        UnInit();
    }

    int FaceDetectComparisonHR::Init ( const char* pModuleFilePath )
    {
        boost::lock_guard<boost::recursive_mutex> guard ( mutex_public_fun_ );
        assert ( hr_appid_.length() > 0 && hr_fd_sdkkey_.length() > 0 );
        if ( hr_appid_.length() <= 0 || hr_fd_sdkkey_.length() <= 0 )
        {
            YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "虹软人脸初始化失败，没有设置 appid, sdkkey" );
            return YSOS_ERROR_FAILED;
        }
        {
            // log appinfo
            std::string temp_hr_appid = hr_appid_;
            std::string temp_hr_sdkkey = hr_fd_sdkkey_;
            // hide some message
            if ( temp_hr_appid.length() > 6 )
            {
                temp_hr_appid.replace ( 0, 6, "******" );
            }
            if ( temp_hr_sdkkey.length() > 6 )
            {
                temp_hr_sdkkey.replace ( 0, 6, "******" );
            }
            YSOS_LOG_INFO_CUSTOM ( MY_LOGGER, "AFD_FSDK_InitialFaceEngine, appid:" << temp_hr_appid << ", sdkkey:" << temp_hr_sdkkey );
        }
        int init_ret = InitFDEngine();
        if ( YSOS_ERROR_SUCCESS != init_ret )
        {
            return YSOS_ERROR_FAILED;
        }
        if ( YSOS_ERROR_SUCCESS != InitFTEngine() )
        {
            return YSOS_ERROR_FAILED;
        }
        InitGenderEngine();
        InitAgeEngine();
        InitFREngine();
        ///// 创建一些临时目录
        {
            boost::filesystem::path path_save_face_info ( tmp_face_image_track_hr_folder_ );
            boost::system::error_code myecc;
            boost::filesystem::remove_all ( path_save_face_info, myecc );
            if ( false == boost::filesystem::exists ( path_save_face_info, myecc ) )
            {
                boost::filesystem::create_directories ( path_save_face_info, myecc );
            }
        }
        {
            boost::filesystem::path path_save_face_info ( tmp_face_image_track_useself_folder_ );
            boost::system::error_code myecc;
            boost::filesystem::remove_all ( path_save_face_info, myecc );
            if ( false == boost::filesystem::exists ( path_save_face_info, myecc ) )
            {
                boost::filesystem::create_directories ( path_save_face_info, myecc );
            }
        }
        {
            boost::filesystem::path path_save_face_info ( tmp_face_image_track_recheck_foler_ );
            boost::system::error_code myecc;
            boost::filesystem::remove_all ( path_save_face_info, myecc );
            if ( false == boost::filesystem::exists ( path_save_face_info, myecc ) )
            {
                boost::filesystem::create_directories ( path_save_face_info, myecc );
            }
        }
        return YSOS_ERROR_SUCCESS;
    }

    int FaceDetectComparisonHR::InitFDEngine()
    {
        assert ( hr_appid_.length() > 0 && hr_fd_sdkkey_.length() > 0 );
        if ( hr_appid_.length() <= 0 || hr_fd_sdkkey_.length() <= 0 )
        {
            YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "虹软人脸初始化失败，没有设置 appid, sdkkey" );
            return YSOS_ERROR_FAILED;
        }
        if ( hr_facedetect_engine_handle_ )
        {
            YSOS_LOG_INFO_CUSTOM ( MY_LOGGER, "reinit fd engine again" );
            return YSOS_ERROR_SUCCESS;
        }
        /// 分配输入数据内存
        if ( NULL == hr_face_input_rect_ptr_ )
        {
            hr_face_input_rect_ptr_.reset ( new MRECT[hr_max_face_count_] );
            if ( !hr_face_input_rect_ptr_ )
            {
                YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "hr 人脸识别，分配 gender 输入数据内存失败" );
                return YSOS_ERROR_FAILED;
            }
        }
        if ( NULL == hr_face_input_orient_ptr_ )
        {
            hr_face_input_orient_ptr_.reset ( new MInt32 ( hr_max_face_count_ ) );
            if ( !hr_face_input_orient_ptr_ )
            {
                YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "hr 人脸识别，分配 gender 输入数据内存失败" );
                return YSOS_ERROR_FAILED;
            }
        }
        /// 分配工作内存
        if ( NULL == hr_fd_work_buf_ptr_ )
        {
            hr_fd_work_buf_ptr_.reset ( new MByte[WORKBUF_SIZE_FD] );
            if ( ! hr_fd_work_buf_ptr_ )
            {
                YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "hr人脸识别 分配内部失败，分配大小:" << WORKBUF_SIZE_FD );
                return YSOS_ERROR_FAILED;
            }
        }
        MRESULT nRet = MERR_UNKNOWN;
        nRet = AFD_FSDK_InitialFaceEngine ( ( MPChar ) hr_appid_.c_str(), ( MPChar ) hr_fd_sdkkey_.c_str(), hr_fd_work_buf_ptr_.get(),
                                            WORKBUF_SIZE_FD, &hr_facedetect_engine_handle_, AFD_FSDK_OPF_0_HIGHER_EXT, hr_min_scale_, hr_max_face_count_ );
        if ( nRet != MOK )
        {
            YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "AFD_FSDK_InitialFaceEngine fail, ret:" <<  nRet );
            //assert ( false && "AFD_FSDK_InitialFaceEngine fail" );
            return YSOS_ERROR_FAILED;
        }
        //assert ( hr_facedetect_engine_handle_ );
        /// 打印版本信息
        const AFD_FSDK_Version * pVersionInfo = nullptr;
        pVersionInfo = AFD_FSDK_GetVersion ( hr_facedetect_engine_handle_ );
        if ( pVersionInfo )
        {
            YSOS_LOG_INFO_CUSTOM ( MY_LOGGER, "facedetect sdk engine version:" << pVersionInfo->lCodebase << " " <<  pVersionInfo->lMajor << " " << pVersionInfo->lMinor << " " << pVersionInfo->lBuild );
            YSOS_LOG_INFO_CUSTOM ( MY_LOGGER, "facedetect sdk engine version2:" <<  pVersionInfo->Version );
            YSOS_LOG_INFO_CUSTOM ( MY_LOGGER, "facedetect sdk engine builddate:" << pVersionInfo->BuildDate );
            YSOS_LOG_INFO_CUSTOM ( MY_LOGGER, "facedetect sdk engine copyright:" <<  pVersionInfo->CopyRight );
        } else {
            YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "error cant get AFD_FSDK_GetVersion" );
        }
        return YSOS_ERROR_SUCCESS;
    }

    int FaceDetectComparisonHR::UnInitFDEngine()
    {
        if ( hr_facedetect_engine_handle_ )
        {
            MRESULT nRet = MERR_UNKNOWN;
            nRet = AFD_FSDK_UninitialFaceEngine ( hr_facedetect_engine_handle_ );
            if ( nRet != MOK )
            {
                YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "UninitialFaceEngine failed , errorcode is  " << nRet );
                assert ( MOK == nRet && "UninitialFaceEngine failed" );
            }
            hr_facedetect_engine_handle_ = NULL;
        }
        /// 释放工作内存
        hr_fd_work_buf_ptr_ = NULL;
        hr_face_input_orient_ptr_ = NULL;
        hr_face_input_rect_ptr_ = NULL;
        return YSOS_ERROR_SUCCESS;
    }


    int FaceDetectComparisonHR::InitGenderEngine()
    {
        assert ( hr_appid_.length() > 0 && hr_fd_sdkkey_.length() > 0 );
        if ( hr_appid_.length() <= 0 || hr_fd_sdkkey_.length() <= 0 )
        {
            YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "虹软人脸初始化失败，没有设置 appid, sdkkey" );
            return YSOS_ERROR_FAILED;
        }
        if ( hr_gender_engine_handle_ )
        {
            YSOS_LOG_INFO_CUSTOM ( MY_LOGGER, "reinit gender engine handle again" );
            return YSOS_ERROR_SUCCESS;
        }
        /// 分配工作内存
        if ( NULL == hr_gender_work_buf_ptr_ )
        {
            hr_gender_work_buf_ptr_.reset ( new MByte[WORKBUF_SIZE_GENDER] );
            if ( ! hr_gender_work_buf_ptr_ )
            {
                YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "hr人脸识别 分配内部失败，分配大小:" << WORKBUF_SIZE_GENDER );
                return YSOS_ERROR_FAILED;
            }
        }
        MInt32 res = MOK;
        res = ASGE_FSDK_InitGenderEngine ( ( MPChar ) hr_appid_.c_str(), ( MPChar ) hr_gender_sdkkey_.c_str(), hr_gender_work_buf_ptr_.get(), WORKBUF_SIZE_GENDER, &hr_gender_engine_handle_ );
        if ( MOK != res )
        {
            YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "error, init genderengine error, retcode:" << res );
            assert ( false && "error, init genderengine error" );
            return YSOS_ERROR_FAILED;
        }
        assert ( hr_gender_engine_handle_ );
        /// log version info
        const ASGE_FSDK_Version * pVersionInfo = nullptr;
        pVersionInfo = ASGE_FSDK_GetVersion ( hr_gender_engine_handle_ );
        if ( pVersionInfo )
        {
            YSOS_LOG_INFO_CUSTOM ( MY_LOGGER, "gender sdk version:" <<  pVersionInfo->lCodebase << " " << pVersionInfo->lMajor << " " << pVersionInfo->lMinor << " " << pVersionInfo->lBuild );
            YSOS_LOG_INFO_CUSTOM ( MY_LOGGER, "gender sdk version2:" << pVersionInfo->Version );
            YSOS_LOG_INFO_CUSTOM ( MY_LOGGER, "gender sdk builddate:" <<  pVersionInfo->BuildDate );
            YSOS_LOG_INFO_CUSTOM ( MY_LOGGER, "gender sdk copyright:" << pVersionInfo->CopyRight );
        }
        return YSOS_ERROR_SUCCESS;
    }

    int FaceDetectComparisonHR::UnInitGenderEngine()
    {
        if ( hr_gender_engine_handle_ )
        {
            MInt32 res = MOK;
            res = ASGE_FSDK_UninitGenderEngine ( hr_gender_engine_handle_ );
            if ( MOK != res )
            {
                YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "uninit gender engine error, retcode:" <<  res );
                assert ( MOK == res && "uninit gender engine error" );
            }
            hr_gender_engine_handle_ = NULL;
        }
        /// release memory
        hr_gender_work_buf_ptr_ = NULL;
        return YSOS_ERROR_SUCCESS;
    }

    int FaceDetectComparisonHR::InitAgeEngine()
    {
        assert ( hr_appid_.length() > 0 && hr_fd_sdkkey_.length() > 0 );
        if ( hr_appid_.length() <= 0 || hr_fd_sdkkey_.length() <= 0 )
        {
            YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "虹软人脸初始化失败，没有设置 appid, sdkkey" );
            return YSOS_ERROR_FAILED;
        }
        if ( hr_age_engine_handle_ )
        {
            YSOS_LOG_INFO_CUSTOM ( MY_LOGGER, "reinit age engine again" );
            return YSOS_ERROR_SUCCESS;
        }
        /// 分配工作内存
        if ( NULL == hr_age_work_buf_ptr_ )
        {
            hr_age_work_buf_ptr_.reset ( new MByte[WORKBUF_SIZE_AGE] );
            if ( ! hr_age_work_buf_ptr_ )
            {
                YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "hr人脸识别 分配内部失败，分配大小:" <<  WORKBUF_SIZE_AGE );
                return YSOS_ERROR_FAILED;
            }
        }
        MInt32 res = MOK;
        res = ASAE_FSDK_InitAgeEngine ( ( MPChar ) hr_appid_.c_str(), ( MPChar ) hr_age_sdkkey_.c_str(), hr_age_work_buf_ptr_.get(), WORKBUF_SIZE_AGE, &hr_age_engine_handle_ );
        if ( MOK != res )
        {
            YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "error, ASAE_FSDK_InitAgeEngine fail retcode:" <<  res );
            assert ( false && "error, ASAE_FSDK_InitAgeEngine fail" );
            return YSOS_ERROR_FAILED;
        }
        assert ( hr_age_engine_handle_ );
        /// log version info
        const ASAE_FSDK_Version * pVersionInfo = nullptr;
        pVersionInfo = ASAE_FSDK_GetVersion ( hr_age_engine_handle_ );
        if ( pVersionInfo )
        {
            YSOS_LOG_INFO_CUSTOM ( MY_LOGGER, "age sdk engine version:" <<  pVersionInfo->lCodebase << pVersionInfo->lMajor << pVersionInfo->lMinor << pVersionInfo->lBuild );
            YSOS_LOG_INFO_CUSTOM ( MY_LOGGER, "age sdk engine versin2:" << pVersionInfo->Version );
            YSOS_LOG_INFO_CUSTOM ( MY_LOGGER, "age sdk engine build date:" << pVersionInfo->BuildDate );
            YSOS_LOG_INFO_CUSTOM ( MY_LOGGER, "age sdk engine copyright:" <<  pVersionInfo->CopyRight );
        }
        return YSOS_ERROR_SUCCESS;
    }

    int FaceDetectComparisonHR::UnInitAgeEngine()
    {
        if ( hr_age_engine_handle_ )
        {
            MInt32 res = MOK;
            res = ASAE_FSDK_UninitAgeEngine ( hr_age_engine_handle_ );
            if ( MOK != res )
            {
                YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "uninit age engine error, retcode:" <<  res );
                assert ( MOK == res && "uninit age engine error" );
            }
            hr_age_engine_handle_ = NULL;
        }
        /// release memory
        hr_age_work_buf_ptr_ = NULL;
        return YSOS_ERROR_SUCCESS;
    }

    int FaceDetectComparisonHR::InitFTEngine()
    {
        assert ( hr_appid_.length() > 0 && hr_fd_sdkkey_.length() > 0 );
        if ( hr_appid_.length() <= 0 || hr_fd_sdkkey_.length() <= 0 )
        {
            YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "虹软人脸初始化失败，没有设置 appid, sdkkey" );
            return YSOS_ERROR_FAILED;
        }
        if ( hr_facetrack_engine_handle_ )
        {
            YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "reinit face track engine again" );
            return YSOS_ERROR_SUCCESS;
        }
        /// 分配输入数据内存
        if ( NULL == hr_face_input_rect_ptr_ )
        {
            hr_face_input_rect_ptr_.reset ( new MRECT[hr_max_face_count_] );
            if ( !hr_face_input_rect_ptr_ )
            {
                YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "hr 人脸识别，分配 gender 输入数据内存失败" );
                return YSOS_ERROR_FAILED;
            }
        }
        if ( NULL == hr_face_input_orient_ptr_ )
        {
            hr_face_input_orient_ptr_.reset ( new MInt32 ( hr_max_face_count_ ) );
            if ( hr_face_input_orient_ptr_ )
            {
                YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "hr 人脸识别，分配 gender 输入数据内存失败" );
                return YSOS_ERROR_FAILED;
            }
        }
        /// 分配工作内存
        if ( NULL == hr_facetrace_work_buf_ptr_ )
        {
            hr_facetrace_work_buf_ptr_.reset ( new MByte[WORKBUF_SIZE_FT] );
            if ( ! hr_facetrace_work_buf_ptr_ )
            {
                YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "hr人脸识别 分配内部失败，分配大小:" <<  WORKBUF_SIZE_FT );
                return YSOS_ERROR_FAILED;
            }
        }
        MInt32 res = MOK;
        res = AFT_FSDK_InitialFaceEngine ( ( MPChar ) hr_appid_.c_str(), ( MPChar ) hr_ft_sdkkey_.c_str(), hr_facetrace_work_buf_ptr_.get()
                                           , WORKBUF_SIZE_FT, &hr_facetrack_engine_handle_, AFT_FSDK_OPF_0_HIGHER_EXT, hr_min_scale_, hr_max_face_count_ );
        if ( MOK != res )
        {
            YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "error, AFT_FSDK_InitialFaceEngine fail,retcode:" <<  res );
            assert ( false && "error, AFT_FSDK_InitialFaceEngine fail" );
            return YSOS_ERROR_FAILED;
        }
        assert ( hr_facetrack_engine_handle_ );
        /// log version
        const AFT_FSDK_Version * pVersionInfo = nullptr;
        pVersionInfo = AFT_FSDK_GetVersion ( hr_facetrack_engine_handle_ );
        if ( pVersionInfo )
        {
            YSOS_LOG_INFO_CUSTOM ( MY_LOGGER, "facetrack sdk engine version:" << pVersionInfo->lCodebase << " " << pVersionInfo->lMajor << " " << pVersionInfo->lMinor << " " << pVersionInfo->lBuild );
            YSOS_LOG_INFO_CUSTOM ( MY_LOGGER, "facetrack sdk engine version2:" <<  pVersionInfo->Version );
            YSOS_LOG_INFO_CUSTOM ( MY_LOGGER, "facetrack sdk engine  builddate:" <<  pVersionInfo->BuildDate );
            YSOS_LOG_INFO_CUSTOM ( MY_LOGGER, "facetrack sdk engine copyright:" <<  pVersionInfo->CopyRight );
        }
        return YSOS_ERROR_SUCCESS;
    }

    int FaceDetectComparisonHR::UnInitFTEngine()
    {
        if ( hr_facetrack_engine_handle_ )
        {
            MInt32 res = MOK;
            res = AFT_FSDK_UninitialFaceEngine ( hr_facetrack_engine_handle_ );
            if ( MOK != res )
            {
                YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "uninit face track engine error, retcode:" <<  res );
                assert ( MOK == res && "uninit face track engine error" );
            }
            hr_facetrack_engine_handle_ = NULL;
        }
        /// release memory
        hr_facetrace_work_buf_ptr_ = NULL;
        return YSOS_ERROR_SUCCESS;
    }

    int FaceDetectComparisonHR::InitFREngine()
    {
        assert ( hr_appid_.length() > 0 && hr_fd_sdkkey_.length() > 0 );
        if ( hr_appid_.length() <= 0 || hr_fd_sdkkey_.length() <= 0 )
        {
            YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "虹软人脸初始化失败，没有设置 appid, sdkkey" );
            return YSOS_ERROR_FAILED;
        }
        if ( hr_facerecognition_engine_handle_ )
        {
            YSOS_LOG_INFO_CUSTOM ( MY_LOGGER, "reinit face recognition engine again" );
            return YSOS_ERROR_SUCCESS;
        }
        /// 分配工作内存
        if ( NULL == hr_facerecognition_work_buf_ptr_ )
        {
            hr_facerecognition_work_buf_ptr_.reset ( new MByte[WORKBUF_SIZE_FR] );
            if ( ! hr_facerecognition_work_buf_ptr_ )
            {
                YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "hr人脸识别 分配内部失败，分配大小:" <<  WORKBUF_SIZE_FR );
                return YSOS_ERROR_FAILED;
            }
        }
        MInt32 res = MOK;
        res = AFR_FSDK_InitialEngine ( ( MPChar ) hr_appid_.c_str(), ( MPChar ) hr_fr_sdkkey_.c_str(), hr_facerecognition_work_buf_ptr_.get()
                                       , WORKBUF_SIZE_FR, &hr_facerecognition_engine_handle_ );
        if ( MOK != res )
        {
            YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "error, AFR_FSDK_InitialEngine fail, retcode:" <<  res );
            assert ( false && "error, AFR_FSDK_InitialEngine fail" );
            return YSOS_ERROR_FAILED;
        }
        assert ( hr_facerecognition_engine_handle_ );
        /// 打印版本信息
        const AFR_FSDK_Version * pVersionInfo = nullptr;
        pVersionInfo = AFR_FSDK_GetVersion ( hr_facerecognition_engine_handle_ );
        if ( pVersionInfo )
        {
            YSOS_LOG_INFO_CUSTOM ( MY_LOGGER, "facerecognition sdk version:" <<  pVersionInfo->lCodebase << " " << pVersionInfo->lMajor << " " << pVersionInfo->lMinor << " "  << pVersionInfo->lBuild << " " << pVersionInfo->lFeatureLevel );
            YSOS_LOG_INFO_CUSTOM ( MY_LOGGER, "facerecognition sdk version2" << pVersionInfo->Version );
            YSOS_LOG_INFO_CUSTOM ( MY_LOGGER, "facerecognition sdk build date:" << pVersionInfo->BuildDate );
            YSOS_LOG_INFO_CUSTOM ( MY_LOGGER, "facerecognition sdk copyright:" <<  pVersionInfo->CopyRight );
        }
        return YSOS_ERROR_SUCCESS;
    }

    int FaceDetectComparisonHR::UnInitFREngine()
    {
        if ( hr_facerecognition_engine_handle_ )
        {
            MInt32 res = MOK;
            res = AFR_FSDK_UninitialEngine ( hr_facerecognition_engine_handle_ );
            if ( MOK != res )
            {
                YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "uninit face recognition engine error, retcode:" << res );
                assert ( MOK == res && "uninit face recognition engine error" );
            }
            hr_facerecognition_engine_handle_ = NULL;
        }
        /// release memory
        hr_facerecognition_work_buf_ptr_ = NULL;
        return YSOS_ERROR_SUCCESS;
    }
    int FaceDetectComparisonHR::UnInit()
    {
        boost::lock_guard<boost::recursive_mutex> guard ( mutex_public_fun_ );
        ///// 释放引擎和内存
        UnInitGenderEngine();
        UnInitAgeEngine();
        UnInitFREngine();
        UnInitFTEngine();
        UnInitFDEngine();  ///< 放到最后释放
        return YSOS_ERROR_SUCCESS;
    }


    int FaceDetectComparisonHR::Comparison11Local ( const char* face_feature_a, const int face_feature_a_length,
            const char* face_feature_b, const int face_feature_b_length, FaceComparisonResult11* pCompareResult11 )
    {
        boost::lock_guard<boost::recursive_mutex> guard ( mutex_public_fun_ );
        assert ( face_feature_a && face_feature_a_length > 0 && face_feature_b && face_feature_b_length > 0 && "compare 11 fail ,param is null" );
        if ( NULL == face_feature_a || face_feature_a_length <= 0 || NULL == face_feature_b || face_feature_b_length <= 0 )
        {
            YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "compare 11 fail ,param is null" );
            return YSOS_ERROR_FAILED;
        }
        if ( !hr_facerecognition_engine_handle_ )
        {
            YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "人脸识别引擎未初始化，不能使用1比1" );
            assert ( false && "人脸识别引擎未初始化，不能使用1比1" );
            return YSOS_ERROR_FAILED;
        }
        MFloat scores = 0.0;
        MRESULT nRet = MERR_UNKNOWN;
        AFR_FSDK_FACEMODEL facemode1 = { 0 }, facemode2 = {0};
        facemode1.pbFeature = ( MByte * ) face_feature_a;
        facemode1.lFeatureSize = face_feature_a_length;
        facemode2.pbFeature = ( MByte * ) face_feature_b;
        facemode2.lFeatureSize = face_feature_b_length;
        nRet = AFR_FSDK_FacePairMatching ( hr_facerecognition_engine_handle_, &facemode1, &facemode2, &scores );
        if ( MOK != nRet )
        {
            YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "error in Comparison11Local, compute macth score return error code:" << nRet );
            return YSOS_ERROR_FAILED;
        }
        YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, " in Comparison11Local compute match score:" << scores );
        if ( pCompareResult11 )
        {
            pCompareResult11->score = scores;
        }
        return YSOS_ERROR_SUCCESS;
    }

    int FaceDetectComparisonHR::FaceDetect ( const char* pdata, const int& width, const int& height, const int& channels,
            const bool& need_track, const bool& need_face_qulity, const bool& need_key_point,
            const bool& need_face_attribute, const bool& need_face_nationality,
            const bool& get_face_image_base64,
            const bool& get_field_feature,
            const bool& get_prob_feature,
            std::vector<TDFaceDetectInfoPtr>& vec_face_detect_info )
    {
        // !!! 注意，由于虹软 动态跟踪功能下，人脸侧着角度大的时候（或部分人脸遮护的时候），则人脸1比1比起来分数是很少的（比如0.28，远不到0.8的分数），
        // 这样一比下来就会被认不是同一个人，但虹软在视频流检测模式下人脸角度没有值
        // 测试在图片检测模式下，这个问题不是那么严重，所以下面的代码，如果开启跟踪功能的也使用图片检索方式
        // 测试过，单个人，如果是视频流方式检测是2毫秒，如果是图片检测方式则是11毫秒。
        YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, "FaceDetectComparisonHR::FaceDetect [Enter]" );
        YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, "FaceDetectComparisonHR::FaceDetect [width]:"<< width << "|| [height]:"<< height <<"|| [channels]:" << channels );
        boost::lock_guard<boost::recursive_mutex> guard ( mutex_public_fun_ );
        std::vector<cv::Mat> vec_face_mat;
        bool need_face_algin_data = need_face_attribute || need_face_nationality || get_face_image_base64 || get_field_feature || get_prob_feature;
        vec_face_detect_info.clear();
        assert ( pdata && width > 0 && height > 0 && "face detect frame data is error" );
        YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, "FaceDetectComparisonHR::FaceDetect [Check][0]" );
        if ( NULL == pdata || width <= 0 || height <= 0 )
        {
            return YSOS_ERROR_FAILED;
        }
        assert ( hr_facedetect_engine_handle_ );
        if ( !hr_facedetect_engine_handle_ )
        {
            YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "人脸引擎未初始化，不进行人脸检测" );
            return YSOS_ERROR_FAILED;
        }
        ASVLOFFSCREEN offInput = { 0 };
        if ( 3 == channels )
        {
            offInput.u32PixelArrayFormat = ASVL_PAF_RGB24_B8G8R8;
        }
        else
        {
            assert ( false && "不支持的通道数" );
            YSOS_LOG_INFO_CUSTOM ( MY_LOGGER, "不支持的通道数:" << channels );
            return YSOS_ERROR_FAILED;
        }
        offInput.i32Width = width;
        offInput.i32Height = height;
        offInput.ppu8Plane[0] = ( uint8_t * ) pdata;
        offInput.pi32Pitch[0] = offInput.i32Width * 3;
        YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, "FaceDetectComparisonHR::FaceDetect [Check][1]" );
        /// 人脸检测
        MRESULT nRet = MERR_UNKNOWN;
        LPAFD_FSDK_FACERES fd_faceres = nullptr;
        LPAFT_FSDK_FACERES ft_faceres = NULL;
        if ( /*need_track*/false ) ///<  按视频方式 检测
        {
            /* 人脸跟踪 */
            nRet = AFT_FSDK_FaceFeatureDetect ( hr_facetrack_engine_handle_, &offInput, &ft_faceres );
            if ( MOK != nRet )
            {
                YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "Face Detection failed, error code: " << nRet );
                return YSOS_ERROR_FAILED;
            }
            /// alloc meory ret
            for ( int i = 0; i < ft_faceres->nFace; ++i )
            {
                TDFaceDetectInfoPtr face_info_ptr ( new TDFaceDetectInfo );
                if ( NULL == face_info_ptr )
                {
                    /// alloc meory error
                    return YSOS_ERROR_FAILED;
                }
                {
                    /// 兼容以前版本
                    TDFaceDetectInfoDetailPtr face_detail_ptr ( new TDFaceDetectInfoDetail );
                    if ( !face_detail_ptr )
                    {
                        return YSOS_ERROR_FAILED;
                    }
                    face_info_ptr->face_detect_detail_ptr = face_detail_ptr;
                }
                vec_face_detect_info.push_back ( face_info_ptr );
            }
            // copy data
            for ( int i = 0; i < ft_faceres->nFace; ++i )
            {
                vec_face_detect_info[i]->trackid = i;
                vec_face_detect_info[i]->x = ft_faceres->rcFace[i].left;
                vec_face_detect_info[i]->y = ft_faceres->rcFace[i].top;
                vec_face_detect_info[i]->width = abs ( ft_faceres->rcFace[i].right - ft_faceres->rcFace[i].left );
                vec_face_detect_info[i]->hight = abs ( ft_faceres->rcFace[i].bottom - ft_faceres->rcFace[i].top );
                switch ( ft_faceres->lfaceOrient )
                {
                    case AFT_FSDK_FOC_0:
                        vec_face_detect_info[i]->face_detect_detail_ptr->headPose.yaw = 0;
                        break;
                    case AFT_FSDK_FOC_90:
                        vec_face_detect_info[i]->face_detect_detail_ptr->headPose.yaw = 90;
                        break;
                    case AFT_FSDK_FOC_270:
                        vec_face_detect_info[i]->face_detect_detail_ptr->headPose.yaw = 270;
                        break;
                    case AFT_FSDK_FOC_180:
                        vec_face_detect_info[i]->face_detect_detail_ptr->headPose.yaw = 180;
                        break;
                    default:
                        assert ( false && "not known face orient in hongrun face detect" );
                }
            }
        }
        else     ///< 按静态图片的方式检测
        {
            nRet = AFD_FSDK_StillImageFaceDetection ( hr_facedetect_engine_handle_, &offInput, &fd_faceres );
            if ( MOK != nRet )
            {
                YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "Face Detection failed, error code: " <<  nRet );
                return YSOS_ERROR_FAILED;
            }
            YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, "FaceDetectComparisonHR::FaceDetect [Check][1][nRet]=" << nRet );
            YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, "FaceDetectComparisonHR::FaceDetect [Check][2]");
            /// alloc meory ret
            for ( int i = 0; i < fd_faceres->nFace; ++i )
            {
                TDFaceDetectInfoPtr face_info_ptr ( new TDFaceDetectInfo );
                if ( NULL == face_info_ptr )
                {
                    // alloc memory fail
                    return YSOS_ERROR_FAILED;
                }
                {
                    /// 兼容以前版本
                    TDFaceDetectInfoDetailPtr face_detail_ptr ( new TDFaceDetectInfoDetail );
                    if ( !face_detail_ptr )
                    {
                        return YSOS_ERROR_FAILED;
                    }
                    face_info_ptr->face_detect_detail_ptr = face_detail_ptr;
                }
                vec_face_detect_info.push_back ( face_info_ptr );
            }
            YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, "FaceDetectComparisonHR::FaceDetect [Check][3]" );
            // copy data
            for ( int i = 0; i < fd_faceres->nFace; ++i )
            {
                vec_face_detect_info[i]->trackid = -1;
                vec_face_detect_info[i]->x = fd_faceres->rcFace[i].left;
                vec_face_detect_info[i]->y = fd_faceres->rcFace[i].top;
                vec_face_detect_info[i]->width = abs ( fd_faceres->rcFace[i].right - fd_faceres->rcFace[i].left );
                vec_face_detect_info[i]->hight = abs ( fd_faceres->rcFace[i].bottom - fd_faceres->rcFace[i].top );
                switch ( fd_faceres->lfaceOrient[i] )
                {
                    case AFD_FSDK_FOC_0:
                        vec_face_detect_info[i]->face_detect_detail_ptr->headPose.yaw = 0;
                        break;   // 0 degree
                    case AFD_FSDK_FOC_90:
                        vec_face_detect_info[i]->face_detect_detail_ptr->headPose.yaw = 90;
                        break;  // 90 degree
                    case AFD_FSDK_FOC_270:
                        vec_face_detect_info[i]->face_detect_detail_ptr->headPose.yaw = 270;
                        break;// 270 degree
                    case AFD_FSDK_FOC_180:
                        vec_face_detect_info[i]->face_detect_detail_ptr->headPose.yaw = 180;
                        break; // 180 degree
                    case AFD_FSDK_FOC_30:
                        vec_face_detect_info[i]->face_detect_detail_ptr->headPose.yaw = 30;
                        break; // 30 degree
                    case AFD_FSDK_FOC_60:
                        vec_face_detect_info[i]->face_detect_detail_ptr->headPose.yaw = 60;
                        break; // 60 degree
                    case AFD_FSDK_FOC_120:
                        vec_face_detect_info[i]->face_detect_detail_ptr->headPose.yaw = 120;
                        break; // 120 degree
                    case AFD_FSDK_FOC_150:
                        vec_face_detect_info[i]->face_detect_detail_ptr->headPose.yaw = 150;
                        break; // 150 degree
                    case AFD_FSDK_FOC_210:
                        vec_face_detect_info[i]->face_detect_detail_ptr->headPose.yaw = 210;
                        break; // 210 degree
                    case AFD_FSDK_FOC_240:
                        vec_face_detect_info[i]->face_detect_detail_ptr->headPose.yaw = 240;
                        break; // 240 degree
                    case AFD_FSDK_FOC_300:
                        vec_face_detect_info[i]->face_detect_detail_ptr->headPose.yaw = 300;
                        break; // 300 degree
                    case AFD_FSDK_FOC_330:
                        vec_face_detect_info[i]->face_detect_detail_ptr->headPose.yaw = 330;
                        break; // 330 degree
                    default:
                        assert ( false && "not known face orient,in fd detect" );
                }
            }
            YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, "FaceDetectComparisonHR::FaceDetect [Check][4]" );
        }

        //TODO:for debug
		if ( false ) {
			int data_format = channels;
			switch ( data_format )
			{
				case 1:
					data_format = CV_8UC1;
					break;
				case 2:
					data_format = CV_8UC2;
					break;
				case 3:
					data_format = CV_8UC3;
					break;
				default:
					return YSOS_ERROR_FAILED;
			}
			cv::Mat tempframe ( int ( height ), int ( width ), data_format, ( void* ) pdata );
            std::vector<uchar> vecImg;                               //Mat 图片数据转换为vector<uchar>
            std::vector<int> vecCompression_params;
            vecCompression_params.push_back ( CV_IMWRITE_JPEG_QUALITY );
            vecCompression_params.push_back ( 100 ); ///<  这里设置为最高质量
            vecImg.clear();
            cv::imencode ( ".jpg", tempframe, vecImg, vecCompression_params );
            std::string img_base64_data = GetUtility()->Base64Encode ( vecImg.data(), vecImg.size() );
            //去掉/r/n,算了，这里不计算，减少计算量
            //pinfo->img_base64_data = replace_all(pinfo->img_base64_data, "\r\n", "");  ///< 按理\r\n是应该得有的，但传给上层js后js还得再进行一次\r\n的转换才能使用，所以这里直接去掉。
            YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, "mat_align_face basea64:" <<  img_base64_data.c_str() );
        }
        //end for debug
        YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, "FaceDetectComparisonHR::FaceDetect [Check][5]" );
        if ( vec_face_detect_info.size() <= 0 )
        {
            YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, "FaceDetectComparisonHR::FaceDetect [Check][5][vec_face_detect_info.size()]=" << vec_face_detect_info.size() );
            return YSOS_ERROR_SUCCESS;
        }
        YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, "FaceDetectComparisonHR::FaceDetect [Check][6]" );
        if ( need_face_attribute && hr_gender_engine_handle_ )
        {
            ASGE_FSDK_GENDERFACEINPUT tmp_gender_input_face;
            tmp_gender_input_face.lFaceNumber = vec_face_detect_info.size();
            tmp_gender_input_face.pFaceRectArray = hr_face_input_rect_ptr_.get();
            tmp_gender_input_face.pFaceOrientArray = hr_face_input_orient_ptr_.get();
            ASGE_FSDK_GENDERRESULT gender_out_res;
            if ( /*need_track*/false ) ///< 按视频方式 检测  -->改成使用图片检测方式
            {
                for ( int i = 0; i < ft_faceres->nFace; ++i )
                {
                    tmp_gender_input_face.pFaceRectArray[i].left = ft_faceres->rcFace[i].left;
                    tmp_gender_input_face.pFaceRectArray[i].top = ft_faceres->rcFace[i].top;
                    tmp_gender_input_face.pFaceRectArray[i].right = ft_faceres->rcFace[i].right;
                    tmp_gender_input_face.pFaceRectArray[i].bottom = ft_faceres->rcFace[i].bottom;
                    tmp_gender_input_face.pFaceOrientArray[i] = ft_faceres->lfaceOrient;          ///< 这里只有一个人脸角度值
                }
                nRet = ASGE_FSDK_GenderEstimation_Preview ( hr_gender_engine_handle_, &offInput, &tmp_gender_input_face, &gender_out_res );
            }
            else      ///< 按静态图片的方式检测
            {
                YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, "FaceDetectComparisonHR::FaceDetect [Check][7]" );
                for ( int i = 0; i < fd_faceres->nFace; ++i )
                {
                    tmp_gender_input_face.pFaceRectArray[i].left = fd_faceres->rcFace[i].left;
                    tmp_gender_input_face.pFaceRectArray[i].top = fd_faceres->rcFace[i].top;
                    tmp_gender_input_face.pFaceRectArray[i].right = fd_faceres->rcFace[i].right;
                    tmp_gender_input_face.pFaceRectArray[i].bottom = fd_faceres->rcFace[i].bottom;
                    tmp_gender_input_face.pFaceOrientArray[i] = fd_faceres->lfaceOrient[i];
                }
                nRet = ASGE_FSDK_GenderEstimation_StaticImage ( hr_gender_engine_handle_, &offInput, &tmp_gender_input_face, &gender_out_res );
            }
            YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, "FaceDetectComparisonHR::FaceDetect [Check][8]" );
            YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, "FaceDetectComparisonHR::FaceDetect [Check][8][nRet]=" << nRet );
            if ( MOK == nRet )
            {
                for ( unsigned int i = 0; i < vec_face_detect_info.size(); ++i )
                {
                    /// "1" represents female, "0" represents male, and "-1" represents unknown.
                    switch ( gender_out_res.pGenderResultArray[i] )
                    {
                        case 1:
                            vec_face_detect_info[i]->gender = "-1";
                            break;
                        case 0:
                            vec_face_detect_info[i]->gender = "1";
                            break;
                        default:
                            vec_face_detect_info[i]->gender = "";
                    }
                }
                YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, "FaceDetectComparisonHR::FaceDetect [Check][9]" );
            }
            else
            {
                YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "error, gender estimation error, retcode:" <<  nRet );
                assert ( false && "error, gender estimation error" );
            }
            YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, "FaceDetectComparisonHR::FaceDetect [Check][10]" );
        }
        YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, "FaceDetectComparisonHR::FaceDetect [Check][11]" );
        if ( need_face_attribute && hr_age_engine_handle_ )
        {
            ASAE_FSDK_AGEFACEINPUT tmp_age_input_face;
            tmp_age_input_face.lFaceNumber = vec_face_detect_info.size();
            tmp_age_input_face.pFaceRectArray = hr_face_input_rect_ptr_.get();
            tmp_age_input_face.pFaceOrientArray = hr_face_input_orient_ptr_.get();
            ASAE_FSDK_AGERESULT age_out_res;
            if ( /*need_track*/false ) ///< 按视频方式 检测   -->改成使用图片检测方式
            {
                for ( int i = 0; i < tmp_age_input_face.lFaceNumber; i++ )
                {
                    tmp_age_input_face.pFaceRectArray[i].left = ft_faceres->rcFace[i].left;
                    tmp_age_input_face.pFaceRectArray[i].top = ft_faceres->rcFace[i].top;
                    tmp_age_input_face.pFaceRectArray[i].right = ft_faceres->rcFace[i].right;
                    tmp_age_input_face.pFaceRectArray[i].bottom = ft_faceres->rcFace[i].bottom;
                    tmp_age_input_face.pFaceOrientArray[i] = ft_faceres->lfaceOrient;
                }
                nRet = ASAE_FSDK_AgeEstimation_Preview ( hr_age_engine_handle_, &offInput, &tmp_age_input_face, &age_out_res );
            }
            else      ///< 按静态图片检测
            {
                YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, "FaceDetectComparisonHR::FaceDetect [Check][12]" );
                for ( int i = 0; i < tmp_age_input_face.lFaceNumber; i++ )
                {
                    tmp_age_input_face.pFaceRectArray[i].left = fd_faceres->rcFace[i].left;
                    tmp_age_input_face.pFaceRectArray[i].top = fd_faceres->rcFace[i].top;
                    tmp_age_input_face.pFaceRectArray[i].right = fd_faceres->rcFace[i].right;
                    tmp_age_input_face.pFaceRectArray[i].bottom = fd_faceres->rcFace[i].bottom;
                    tmp_age_input_face.pFaceOrientArray[i] = fd_faceres->lfaceOrient[i];
                }
                nRet = ASAE_FSDK_AgeEstimation_StaticImage ( hr_age_engine_handle_, &offInput, &tmp_age_input_face, &age_out_res );
                YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, "FaceDetectComparisonHR::FaceDetect [Check][13]" );
                YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, "FaceDetectComparisonHR::FaceDetect [Check][13][nRet]=" << nRet );
            }
            if ( MOK == nRet )
            {
                for ( unsigned int i = 0; i < vec_face_detect_info.size(); ++i )
                {
                    vec_face_detect_info[i]->age = age_out_res.pAgeResultArray[i];  ///< "0" represents unknown,
                }
                YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, "FaceDetectComparisonHR::FaceDetect [Check][14]" );
            }
            else
            {
                YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "error, age estimation error, retcode:" <<  nRet );
                assert ( false && "error, age estimation error" );
            }
            YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, "FaceDetectComparisonHR::FaceDetect [Check][15]" );
        }
        YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, "FaceDetectComparisonHR::FaceDetect [Check][16]" );
        //增加人脸距离估算
        if ( need_face_attribute && need_track ) ///< 视频方式的才估算距离
        {
            // 获取摄像头最大的水平和垂直角度(计算时会把角度分半，一半负一半正）
            //float max_horizon_angle_half =0, max_vertical_angle_half =0, oriented_with_horizontal_angle=0, middle_offset_distance=0;
            double x_max_width = width / 2;
            double y_max_hegiht = height / 2;
            for ( unsigned int i = 0; i < vec_face_detect_info.size(); ++i )
            {
                ///< 水平角度，以摄像头为视角，水平中间为原点，左边为正数角度，右边为负数角度
                vec_face_detect_info[i]->horizontal_angle = ( float ) - ( ( ( ( double ) vec_face_detect_info[i]->x + vec_face_detect_info[i]->width / 2 ) - x_max_width ) / x_max_width * config_max_horizon_angle_half_ );
                ///< 垂直角度，以摄像头为视频，垂直中间为原点，上面为正数角度，下边负正数角度
                vec_face_detect_info[i]->vertical_angle = ( float ) - ( ( ( ( double ) vec_face_detect_info[i]->y + vec_face_detect_info[i]->hight / 2 ) - y_max_hegiht ) / y_max_hegiht * config_max_vertical_angle_half_ );
                //vec_face_detect_info[i]->vertical_ratio = (double)vec_face_detect_info[i]->hight/height;
                // 计算人脸所在的距离（粗估）
                vec_face_detect_info[i]->estimate_distance = ( float ) EstimatePeopleDistanceAccordFaceSize ( /*vec_face_detect_info[i]->vertical_ratio*/ ( float ) ( ( double ) vec_face_detect_info[i]->hight / height ) ); ///<修改一下大小与 在线版对应。
                //// 修正为水平方向的距离,根据摄像头向上的角度。
                //if(oriented_with_horizontal_angle >ZERO_DOUBLE) {
                //  vec_face_detect_info[i]->estimate_distance = vec_face_detect_info[i]->estimate_distance * cos((abs(vec_face_detect_info[i]->horizontal_angle) + abs(oriented_with_horizontal_angle))/180.0*g_match_pi);
                //}
                //// 修正为水平方向的距离,根据摄像头相对 中间的位置。
                //if(abs(middle_offset_distance) > ZERO_DOUBLE) {
                //  // 修正水平方向角，摄像头不是放在机器人中间。
                //  vec_face_detect_info[i]->horizontal_angle += asin(middle_offset_distance/vec_face_detect_info[i]->estimate_distance)/g_match_pi*180.0;
                //  vec_face_detect_info[i]->estimate_distance = sqrt( pow(vec_face_detect_info[i]->estimate_distance, 2) - pow(middle_offset_distance, 2));
                //}
            }
        }
        YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, "FaceDetectComparisonHR::FaceDetect [Check][17]" );
        /// 填充默认值, 由于虹软没有这些功能，所以得填充一些默认值，以兼容现有框架。
        int data_format = channels;
        switch ( data_format )
        {
            case 1:
                data_format = CV_8UC1;
                break;
            case 2:
                data_format = CV_8UC2;
                break;
            case 3:
                data_format = CV_8UC3;
                break;
            default:
                return YSOS_ERROR_FAILED;
        }
        cv::Mat frame ( int ( height ), int ( width ), data_format, ( void* ) pdata );
        for ( unsigned int i = 0; i < vec_face_detect_info.size(); ++i )
        {
            vec_face_detect_info[i]->face_quality_score = 1.0;
            cv::Mat roi_frame ( frame, cv::Rect ( vec_face_detect_info[i]->x, vec_face_detect_info[i]->y, vec_face_detect_info[i]->width, vec_face_detect_info[i]->hight ) );
            //vec_face_detect_info[i]->face_mat = roi_frame.clone();
            vec_face_mat.push_back ( roi_frame.clone() );
        }
        YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, "FaceDetectComparisonHR::FaceDetect [Check][18]" );
        if ( get_face_image_base64 ) ///< 需自己提取
        {
            for ( unsigned int i = 0; i < vec_face_detect_info.size(); ++i )
            {
                //TDFaceDetectInfoPtr pinfo = vec_face_detect_info[i];
                //cv::Mat mat_single_face(data_frame, cv::Rect(vec_face_detect_info[i]->x, vec_face_detect_info[i]->y, vec_face_detect_info[i]->width, vec_face_detect_info[i]->hight));
                //if (mat_single_face.data)
                {
                    std::vector<uchar> vecImg;                               //Mat 图片数据转换为vector<uchar>
                    std::vector<int> vecCompression_params;
                    vecCompression_params.push_back ( CV_IMWRITE_JPEG_QUALITY );
                    vecCompression_params.push_back ( 100 ); ///<  这里设置为最高质量
                    vecImg.clear();
                    cv::imencode ( ".jpg", vec_face_mat[i], vecImg, vecCompression_params );
                    vec_face_detect_info[i]->img_base64_data = GetUtility()->Base64Encode ( vecImg.data(), vecImg.size() );
                    //去掉/r/n,算了，这里不计算，减少计算量
                    //pinfo->img_base64_data = replace_all(pinfo->img_base64_data, "\r\n", "");  ///< 按理\r\n是应该得有的，但传给上层js后js还得再进行一次\r\n的转换才能使用，所以这里直接去掉。
                    YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, "mat_align_face basea64:" <<  vec_face_detect_info[i]->img_base64_data.c_str() );
                }
            }
            YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, "FaceDetectComparisonHR::FaceDetect [Check][19]" );
        }
        if ( /*need_track*/false && ( get_field_feature || get_prob_feature ) && hr_facerecognition_engine_handle_ ) ///< 人脸特征提取
        {
            for ( int i = 0; i < ft_faceres->nFace; ++i )
            {
                AFR_FSDK_FACEINPUT faceInput;
                //第一张人脸信息通过face detection\face tracking获得
                faceInput.lOrient = ft_faceres->lfaceOrient;//人脸方向
                //人脸框位置
                faceInput.rcFace.left = ft_faceres->rcFace[i].left;
                faceInput.rcFace.top = ft_faceres->rcFace[i].top;
                faceInput.rcFace.right = ft_faceres->rcFace[i].right;
                faceInput.rcFace.bottom = ft_faceres->rcFace[i].bottom;
                /// 提取人脸特征
                AFR_FSDK_FACEMODEL LocalFaceModels = { 0 };
                nRet = AFR_FSDK_ExtractFRFeature ( hr_facerecognition_engine_handle_, &offInput, &faceInput, &LocalFaceModels );
                if ( MOK != nRet )
                {
                    YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "fail to Extract FR Feature, error code:" <<  nRet );
                    continue; ///< 提取下一个人脸的特征
                }
                vec_face_detect_info[i]->vec_face_feature.insert ( vec_face_detect_info[i]->vec_face_feature.begin(), LocalFaceModels.pbFeature, LocalFaceModels.pbFeature + LocalFaceModels.lFeatureSize );
            }
        }
        else if ( ( get_field_feature || get_prob_feature || need_track ) &&  hr_facerecognition_engine_handle_ )
        {
            YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, "FaceDetectComparisonHR::FaceDetect [Check][20]" );
            /// 按图片方式检索
            for ( int i = 0; i < fd_faceres->nFace; ++i )
            {
                AFR_FSDK_FACEINPUT faceInput;
                //第一张人脸信息通过face detection\face tracking获得
                faceInput.lOrient = fd_faceres->lfaceOrient[i]; //人脸方向
                //人脸框位置
                faceInput.rcFace.left = fd_faceres->rcFace[i].left;
                faceInput.rcFace.top = fd_faceres->rcFace[i].top;
                faceInput.rcFace.right = fd_faceres->rcFace[i].right;
                faceInput.rcFace.bottom = fd_faceres->rcFace[i].bottom;
                /// 提取人脸特征
                AFR_FSDK_FACEMODEL LocalFaceModels = { 0 };
                nRet = AFR_FSDK_ExtractFRFeature ( hr_facerecognition_engine_handle_, &offInput, &faceInput, &LocalFaceModels );
                if ( MOK != nRet )
                {
                    YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "fail to Extract FR Feature, error code: " <<  nRet );
                    continue; ///< 提取下一个人脸的特征
                }
                YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, "FaceDetectComparisonHR::FaceDetect [Check][21]" );
                YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, "FaceDetectComparisonHR::FaceDetect [Check][21][nRet]=" << nRet );
                vec_face_detect_info[i]->vec_face_feature.insert ( vec_face_detect_info[i]->vec_face_feature.begin(), LocalFaceModels.pbFeature, LocalFaceModels.pbFeature + LocalFaceModels.lFeatureSize );
            }
        }
        // 更新人脸跟踪号
        if ( need_track )
        {
            std::vector<TDFaceDetectInfoPtr> vec2;
            std::vector<cv::Mat> vec2_mat;
            /// 过滤掉那些人脸质量不高的人脸
            for ( unsigned int i = 0; i < vec_face_detect_info.size(); ++i )
            {
                //if (vec_face_detect_info[i]->face_quality_score < config_score_min_face_match_) { ///< 人脸质量不够
                //  continue;
                //}
                //if (vec_face_detect_info[i]->face_detect_detail_ptr->quality.clearness < config_face_track_min_blur_score_) { ///< 人脸清晰度不够
                //  continue;
                //}
                if ( vec_face_detect_info[i]->vec_face_feature.size() <= 0 ) ///< 人脸特征获取不了。
                {
                    continue;
                }
                if ( need_face_attribute && vec_face_detect_info[i]->age <= 0 )           ///< 人脸年龄获取不了
                {
                    continue;
                }
                if ( need_face_attribute && vec_face_detect_info[i]->gender.length() <= 0 ) ///< 人脸性别获取不了，说明人脸也比较模糊吧
                {
                    continue;
                }
                //if(abs(vec_face_detect_info[i]->face_detect_detail_ptr->headPose.yaw) >config_face_track_min_yaw_) {  ///< 人脸偏航角（左右）超过默认值
                //  continue;
                //}
                //if(abs(vec_face_detect_info[i]->face_detect_detail_ptr->headPose.roll) > config_face_track_min_pitch_) {  ///< 人脸俯仰角（上下）超过默认值
                //  continue;
                //}
                vec2.push_back ( vec_face_detect_info[i] );
                vec2_mat.push_back ( vec_face_mat[i] );
            }
            YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, "FaceDetectComparisonHR::FaceDetect [Check][22]" );
            vec_face_detect_info = vec2;
            vec_face_mat = vec2_mat;
            ClearTrackFaceInfoList();
            int update_track_ret = UpdateFaceTrackId ( vec_face_detect_info, vec_face_mat );
            YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, "FaceDetectComparisonHR::FaceDetect [Check][22][nRet]=" << nRet );
            if ( YSOS_ERROR_SUCCESS != update_track_ret )
            {
                YSOS_LOG_ERROR_CUSTOM ( MY_LOGGER, "人脸设置跟踪号失败" );
                assert ( false && "人脸设置跟踪号失败" );
                return YSOS_ERROR_FAILED;
            }
            YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, "FaceDetectComparisonHR::FaceDetect [Check][23]" );
        }
        YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, "FaceDetectComparisonHR::FaceDetect [Check][24]" );
        YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, "FaceDetectComparisonHR::FaceDetect [End]" );
        return YSOS_ERROR_SUCCESS;
    }


    void FaceDetectComparisonHR::ClearTrackFaceInfoList()
    {
        boost::lock_guard<boost::mutex> guard ( mutex_face_traing_oper_ );
        if ( list_track_face_info_.size() <= 0 )
        {
            return;
        }
        clock_t cur_clock = clock();
        // const int max_save_face_milliseconds = 30000;  ///< 丢失人脸后保留的最多时间 是30秒
        std::list<TDTrackFaceInfoPtr>::iterator it = list_track_face_info_.begin();
        for ( it; it != list_track_face_info_.end(); )
        {
            if ( ( cur_clock - ( *it )->track_latest_clock ) > face_trace_min_cache_face_info_milliseonds_ )
            {
                YSOS_LOG_INFO_CUSTOM ( MY_LOGGER, "clear old face info, trackid:" << ( *it )->face_detect_ptr->trackid << ", have_not_see_face_clock:" << ( cur_clock - ( *it )->track_latest_clock ) << ", face_on_camera_clock:" << ( *it )->track_latest_clock - ( *it )->track_begin_clock );
                it = list_track_face_info_.erase ( it );
                continue;
            }
            ++it;
        }
        // 最多保存最近10个人
    }


    int FaceDetectComparisonHR::UpdateFaceTrackId ( std::vector<TDFaceDetectInfoPtr>& vec_face_info, std::vector<cv::Mat>& vec_face_mat )
    {
        if ( vec_face_info.size() <= 0 )
        {
            return YSOS_ERROR_SUCCESS;
        }
        assert ( vec_face_info.size() == vec_face_mat.size() && "in UpdateFaceTrackId error" );
        boost::lock_guard<boost::mutex> guard ( mutex_face_traing_oper_ );
        std::vector<TDFaceDetectInfoPtr> vec_tmp_valid_face_info;
        std::list<TDTrackFaceInfoPtr> tmp_trace_face_info = list_track_face_info_;
        FaceComparisonResult11 face_com_result;
        int comp_ret = YSOS_ERROR_SUCCESS;
        TDTrackFaceInfoPtr find_person_ptr = NULL;
        for ( unsigned int i = 0; i < vec_face_info.size(); ++i )
        {
            // 设置默认值
            vec_face_info[i]->trackid = -1;
            find_person_ptr = NULL;
            assert ( vec_face_info[i]->vec_face_feature.size() > 0 );
            if ( vec_face_info[i]->vec_face_feature.size() <= 0 )
            {
                vec_face_info[i]->trackid = -1;
                continue;
            }
            std::list<TDTrackFaceInfoPtr>::iterator it = tmp_trace_face_info.begin();  ///< 改成从最开始的检索，防止人脸模糊导致人脸跟丢情况。
            comp_ret = YSOS_ERROR_SUCCESS;
            for ( it; it != tmp_trace_face_info.end(); ++it )
            {
                assert ( ( *it )->face_detect_ptr->vec_face_feature.size() > 0 );
                comp_ret = Comparison11Local ( & ( *it )->face_detect_ptr->vec_face_feature[0], ( *it )->face_detect_ptr->vec_face_feature.size(),
                                               &vec_face_info[i]->vec_face_feature[0], vec_face_info[i]->vec_face_feature.size(), &face_com_result );
                if ( YSOS_ERROR_SUCCESS != comp_ret )
                {
                    break;
                }
                if ( face_com_result.score >= config_score_min_face_match_ )
                {
                    // find the person
                    find_person_ptr = *it;
                    // remove from the next time check person list
                    //tmp_trace_face_info.erase((++rit).base());
                    tmp_trace_face_info.erase ( it );
                    break;
                }
                else
                {
                    #ifdef _DEBUG
                    YSOS_LOG_DEBUG_CUSTOM ( MY_LOGGER, "compte socre:" <<  face_com_result.score );
                    #endif
                }
            }
            if ( YSOS_ERROR_SUCCESS != comp_ret )
            {
                vec_face_info[i]->trackid = -1;
            }
            TDFaceDetectInfoPtr find_person_old_fae_detect_ptr = find_person_ptr ? find_person_ptr->face_detect_ptr : NULL;  ///< 保存人脸图片用的
            if ( find_person_ptr )
            {
                // find the person
                vec_face_info[i]->trackid = find_person_ptr->track_id;
                //save to list
                find_person_ptr->track_latest_clock = clock();
                //更新本地人脸质量
                if ( vec_face_info[i]->face_quality_score > ( find_person_ptr->face_detect_ptr->face_quality_score + 0.05 ) )
                {
                    find_person_ptr->face_detect_ptr = vec_face_info[i];
                    find_person_ptr->face_mat = vec_face_mat[i];
                }
            }
            else
            {
                ++max_track_id_;
                vec_face_info[i]->trackid = max_track_id_;
                // save to list
                TDTrackFaceInfoPtr save_track_info_ptr ( new TDTrackFaceInfo );
                if ( NULL == save_track_info_ptr )
                {
                    return YSOS_ERROR_FAILED;
                }
                save_track_info_ptr->track_id = max_track_id_;
                save_track_info_ptr->track_begin_clock = clock();
                save_track_info_ptr->track_latest_clock = clock();
                save_track_info_ptr->face_detect_ptr = vec_face_info[i];
                save_track_info_ptr->face_mat = vec_face_mat[i];
                list_track_face_info_.push_back ( save_track_info_ptr );
            }
            ///  保存图片信息，以方便日志查看人脸信息
            {
                if ( ( find_person_ptr && ( vec_face_info[i]->face_quality_score > find_person_old_fae_detect_ptr->face_quality_score + 0.05 ) )
                        || NULL == find_person_ptr )
                {
                    std::string current_time_string = boost::posix_time::to_iso_string ( boost::posix_time::second_clock::local_time() );
                    boost::filesystem::path path_save_face_info_ ( tmp_face_image_track_useself_folder_ );
                    char tempbuf_filename[2048] = {'\0'};
#ifdef _WIN32                    
                    sprintf_s ( tempbuf_filename, sizeof ( tempbuf_filename ), "%s_t%d_q%.2f_c%.2f_f%d_g%s_a%d_y%.2f_p%.2f_r%.2f.jpg",
                                current_time_string.c_str(),
                                vec_face_info[i]->trackid,
                                vec_face_info[i]->face_quality_score, vec_face_info[i]->face_detect_detail_ptr->quality.clearness,
                                vec_face_info[i]->vec_face_feature.size() > 0 ? 1 : 0,  vec_face_info[i]->gender.c_str(), vec_face_info[i]->age,
                                vec_face_info[i]->face_detect_detail_ptr ? vec_face_info[i]->face_detect_detail_ptr->headPose.yaw : 0,
                                vec_face_info[i]->face_detect_detail_ptr ? vec_face_info[i]->face_detect_detail_ptr->headPose.pitch : 0,
                                vec_face_info[i]->face_detect_detail_ptr ? vec_face_info[i]->face_detect_detail_ptr->headPose.roll : 0 );
#else
                    sprintf ( tempbuf_filename, "%s_t%d_q%.2f_c%.2f_f%d_g%s_a%d_y%.2f_p%.2f_r%.2f.jpg",
                                current_time_string.c_str(),
                                vec_face_info[i]->trackid,
                                vec_face_info[i]->face_quality_score, vec_face_info[i]->face_detect_detail_ptr->quality.clearness,
                                vec_face_info[i]->vec_face_feature.size() > 0 ? 1 : 0,  vec_face_info[i]->gender.c_str(), vec_face_info[i]->age,
                                vec_face_info[i]->face_detect_detail_ptr ? vec_face_info[i]->face_detect_detail_ptr->headPose.yaw : 0,
                                vec_face_info[i]->face_detect_detail_ptr ? vec_face_info[i]->face_detect_detail_ptr->headPose.pitch : 0,
                                vec_face_info[i]->face_detect_detail_ptr ? vec_face_info[i]->face_detect_detail_ptr->headPose.roll : 0 );
#endif
                    //if (vec_face_info[i]->face_detect_detail_ptr->face_align.data) {
                    //  cv::Mat mat_save_image(vec_face_info[i]->face_detect_detail_ptr->face_align.height, vec_face_info[i]->face_detect_detail_ptr->face_align.width, /*CV_8UC3*/CV_8UC1,
                    //    vec_face_info[i]->face_detect_detail_ptr->face_align.data);
                    //  cv::imwrite(boost::filesystem::path(path_save_face_info_).append(tempbuf_filename).string(), mat_save_image);
                    //}
                    if ( false == vec_face_mat[i].empty() )
                    {
                        cv::imwrite ( boost::filesystem::path ( path_save_face_info_ ).append ( tempbuf_filename ).string(), /*vec_face_info[i]->face_mat*/vec_face_mat[i] );
                    }
                }
            }
            vec_tmp_valid_face_info.push_back ( vec_face_info[i] );
        }
        vec_face_info = vec_tmp_valid_face_info;
        return YSOS_ERROR_SUCCESS;
    }


    int FaceDetectComparisonHR::SetConfigKeyValue ( const std::string &key, const std::string &value )
    {
        if ( value.length() <= 0 )
        {
            return YSOS_ERROR_FAILED;
        }
        if ( "hr_appid" == key )
        {
            hr_appid_ = value;
            return YSOS_ERROR_SUCCESS;
        }
        else if ( "hr_fd_sdkkey" == key )
        {
            hr_fd_sdkkey_ = value;
            return YSOS_ERROR_SUCCESS;
        }
        else if ( "hr_ft_sdkkey" == key )
        {
            hr_ft_sdkkey_ = value;
            return YSOS_ERROR_SUCCESS;
        }
        else if ( "hr_gender_sdkkey" == key )
        {
            hr_gender_sdkkey_ = value;
            return YSOS_ERROR_SUCCESS;
        }
        else if ( "hr_age_sdkkey" == key )
        {
            hr_age_sdkkey_ = value;
            return YSOS_ERROR_SUCCESS;
        }
        else if ( "hr_fr_sdkkey" == key )
        {
            hr_fr_sdkkey_ = value;
            return YSOS_ERROR_SUCCESS;
        }
        else if ( "min_face_avg_similar_score" == key )
        {
            config_score_min_face_match_ = ( float ) atof ( value.c_str() );
            return YSOS_ERROR_SUCCESS;
        }
        else if ( "face_trace_min_cache_face_info_milliseonds" == key )
        {
            face_trace_min_cache_face_info_milliseonds_ = ( float ) atof ( value.c_str() );
            return YSOS_ERROR_SUCCESS;
        }
        else if ( "estimate_people_distance_use_cameraid" == key )
        {
            config_estimate_people_distance_use_cameraid_ = atoi ( value.c_str() );
        }
        return YSOS_ERROR_FAILED;
    }

    float FaceDetectComparisonHR::EstimatePeopleDistanceAccordFaceSize ( const float face_ratio )
    {
        if ( face_ratio < ZERO_DOUBLE ) ///< 如果人脸比例小于0，则估计不出
        {
            return 0;
        }
        double people_distance = 0;
        switch ( config_estimate_people_distance_use_cameraid_ )
        {
            case FPDUC_LOGIC720P:
            case FPDUC_REALSENSE:   ///< 还同有建过模块，默认使用logi的模型吧
                {
                    //people_distance  = -3.573*pow(face_ratio, 3) + 10.035*pow(face_ratio, 2) - 9.6456*face_ratio + 3.5581;
                    /*
                    以上方程 通过以下测试参数得到

                    人脸比例（垂直方向）	人脸距离（单位m)
                    1	0.365
                    0.95	0.39
                    0.644	0.6
                    0.35	1.166
                    0.272	1.57
                    0.136	2.57
                    0.1166667	3.18  (未加入拟合，加入这个数据后会导致拟合的曲线不是单调函数）
                    0.000001	3.5
                    */
                    float new_face_ratio = face_ratio;
                    if ( FPDUC_REALSENSE == config_estimate_people_distance_use_cameraid_ )
                    {
                        new_face_ratio = ( float ) ( new_face_ratio * 42 / 35.0 );
                    }
                    if ( new_face_ratio > 0.95 )
                    {
                        people_distance = -new_face_ratio + 1.2;
                    }
                    else if ( new_face_ratio > 0.644 )
                    {
                        people_distance = -0.1634 * new_face_ratio + 0.4052;
                    }
                    else if ( new_face_ratio > 0.35 )
                    {
                        people_distance = -0.6803 * new_face_ratio + 0.7381;
                    }
                    else if ( new_face_ratio > 0.2 )
                    {
                        people_distance =  -3.3333 * new_face_ratio + 1.6667;
                    }
                    else if ( new_face_ratio > 0.126 )
                    {
                        people_distance = -17.568 * new_face_ratio + 4.5135;
                    }
                    /*else if(new_face_ratio >0.1166667) {
                      people_distance = -31.552 * new_face_ratio + 6.861;
                    }*/
                    else   /*if (face_ratio > 0.000001)*/
                    {
                        people_distance = -9.5239 * new_face_ratio + 3.5;
                    }
                    break;
                }
            /*case FPDUC_REALSENSE:
            {

            break;
            }*/
            case FPDUC_USE_WIDE_ANGLE:   ///< 广角摄像头的
                {
                    /*
                    0.9	0.1
                    0.56	0.2
                    0.35	0.3
                    0.22	0.5
                    0.125	1
                    0.08	2
                    0.066	2.6
                    */
                    //算了还是使用分段function
                    if ( face_ratio > 0.56 )
                    {
                        /*0.9	0.1
                        0.56	0.2
                        */
                        people_distance = -0.2941 * face_ratio + 0.3647;
                    }
                    else if ( face_ratio > 0.35 )
                    {
                        /*
                        0.56	0.2
                        0.35	0.3
                        */
                        people_distance = -0.4762 * face_ratio + 0.4667;
                    }
                    else if ( face_ratio > 0.22 )
                    {
                        /*
                        0.35	0.3
                        0.22	0.5
                        */
                        people_distance = -1.5385 * face_ratio + 0.8385;
                    }
                    else if ( face_ratio > 0.125 )
                    {
                        /*
                        0.22	0.5
                        0.125	1
                        */
                        people_distance = -5.2632 * face_ratio + 1.6579;
                    }
                    else if ( face_ratio > 0.08 )
                    {
                        /*
                        0.125	1
                        0.08	2
                        */
                        people_distance = -22.222 * face_ratio + 3.7778;
                    }
                    else
                    {
                        /*
                        0.08	2
                        0.066	2.6
                        */
                        people_distance = -42.857 * face_ratio + 5.4286;
                    }
                    break;
                }
            default:
                {
                    assert ( false && "not support estimate distance camera id" );
                    break;
                }
        }
        return static_cast<float> ( people_distance );
    }


}