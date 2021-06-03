/**   //NOLINT
  *@file hongrunfacedetectdriver.cpp
  *@brief Definition of
  *@version 0.1
  *@author wanfenfen
  *@date Created on: 2018:4:12   14:57
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  *@howto Usage:
  *@todo
  */

/// self headers
#include "../include/hongrunfacedetectdriver.h"

#ifdef _WIN32
  #include <Windows.h>
#else
  //TODO: add for linux
#endif

/// boost headers
#include <boost/thread/thread_guard.hpp>

/// third party opencv headers
#include "../../../../../thirdparty/opencv/include/opencv2/opencv.hpp"
#include "../../../../../thirdparty/opencv/include/opencv2/highgui/highgui.hpp"
#include "../../../../../thirdparty/opencv/include/opencv2/core/core.hpp"

/// ysos headers
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/bufferpool.h"
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/callbackqueue.h"

/// private headers
#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"

#include "../../../public/include/common/facedetectstruct.h"
#include "../include/face_module_config.h"
#include "../include/face_detect_factory.h"


/// 以下是为发兼容公版YSOS，参考模块face_detect_driver
#include "../include/common/common_struct.h"
#include "../include/common/mycommondefine.h"

namespace ysos {

    DECLARE_PLUGIN_REGISTER_INTERFACE ( HongRunFaceDetectDriver, DriverInterface );
    //HongRunFaceDetectDriver::HongRunFaceDetectDriver(const std::string &strClassName)
    //  : BaseInterfaceImpl(strClassName)
    HongRunFaceDetectDriver::HongRunFaceDetectDriver ( const std::string &strClassName )
        : BaseDriverImpl ( strClassName )
    {
        logger_ = MY_LOGGER;
        YSOS_LOG_INFO ( "HongRunFaceDetectDriver construct" );
        face_detect_ptr_ = NULL;
        face_recognize_ptr_ = NULL;
        //////////////////////////////////////////////////////////////////////////
        /// 以下是为发兼容公版YSOS，参考模块face_detect_driver
        base_track_id_ = -1;
        cmp_track_id_ = -1;
        //////////////////////////////////////////////////////////////////////////
        #ifdef _DEBUG
        ::MessageBox ( NULL, _T ( "TEST IN HONG RUN DRIVER" ), _T ( "TEST" ), MB_OK );
        #endif
    }

    int HongRunFaceDetectDriver::GetProperty ( int iTypeId, void *piType )
    {
        switch ( iTypeId )
        {
            case PROP_FUN_CALLABILITY:
                {
                    FunObjectCommon2* pobject = reinterpret_cast<FunObjectCommon2*> ( piType );
                    if ( NULL == pobject )
                    {
                        YSOS_LOG_DEBUG ( "error call property PROP_Fun_CallAbility, the param is null" );
                        break;
                    }
                    AbilityParam* pin = reinterpret_cast<AbilityParam*> ( pobject->pparam1 );
                    AbilityParam* pout = reinterpret_cast<AbilityParam*> ( pobject->pparam2 );
                    return CallAbility ( pin, pout );
                }
        }
        /// 以下是为发兼容公版YSOS，参考模块face_detect_driver
        /*
        流程：
        1.视频流数据到facedetectmodule,
        2.facedetectmodule调driver 的PROP_FACE_RECOGNIZE（最大人脸的详细信息），如果有结果上送人脸信息事件（只包含最大人脸详细信息，!!!!模糊人脸的trackid放在person_id字段中）（已获取过则不获取），
          如果没有，则调 PROP_FACE_COMPARE，获取缓存的人脸比对结果，如果有结果上送人脸信息事件（只包含人脸比对分数结果）（已获取过则不获取）
          如果上面没有结果，则调 PROP_FACE_DETECT_ASYNC 获取人脸模糊信息，选1个最大模糊人脸并异步的去获取此人的详细信息，上送人脸信息事件（只包含所有的人脸模糊信息）
        3.策略层检测到人脸信息，
          如果有1个人在待服务区（未到服务区），则调 facedetectmodule ioctl（再调driver ioctl):CMD_FACE_RCGN_1_1_CMP_BY_ID, 保存当前待服务对象的trackid，driver中根据trackid得到这个人脸的模糊信息，同时与 “保存当前服务对象”进行比对，比对结果缓存起来
          如果有1个人在服务区（服务状态中），则调  facedetectmodule ioctl（再调driver ioctl):CMD_FACE_RCGN_1_1_SET_BASE，保存当前服务对象的trackid， driver中根据trackid注册这个人脸（保存当前服务对象），
          如果有多于1个人在服务区（服务状态中），则调  facedetectmodule ioctl（再调driver ioctl):CMD_FACE_RCGN_1_1_CLEAR_BASE，则清空当前服务对象，driver中清空 保存当前服务对象，
          检测到事件（人脸比对分数结果），则判断人脸是不是变了。
        */
        switch ( iTypeId )
        {
            case PROP_FACE_DETECT_ASYNC:
                {
                    #ifdef _WFF_DEBUG                      ///< 一次本地测试代码
                    YSOS_LOG_INFO ( "PROP_FACE_DETECT_ASYNC" );
                    std::string terminal_id;
                    std::string org_id ;
                    DataInterfacePtr data_ptr_ = GetDataInterfaceManager()->GetData ( GLOBAL_DATA_KEY );
                    data_ptr_->GetData ( "orgId", org_id );
                    data_ptr_->GetData ( "termId", terminal_id );
                    if ( terminal_id.length() <= 0 || org_id.length() <= 0 )
                    {
                        data_ptr_->SetData ( "orgId", "000000011" );
                        data_ptr_->SetData ( "termId", "00000224" );
                        // return YSOS_ERROR_SUCCESS;
                    }
                    #endif
                    return DetectFaceAsync ( piType ); ///< recognition =0;
                }
            case PROP_FACE_RECOGNIZE:
                {
                    #ifdef _WFF_DEBUG
                    YSOS_LOG_INFO ( "PROP_FACE_RECOGNIZE" );
                    #endif
                    return GetFaceRecognizeResult ( ( FaceComparisonResult1N* ) piType ); ///< recognition =1;
                }
            case PROP_FACE_COMPARE:
                {
                    #ifdef _WFF_DEBUG
                    YSOS_LOG_INFO ( "PROP_FACE_COMPARE" );
                    #endif
                    return GetFaceCompareResult ( ( int* ) piType );        ///< recognition=2
                }
        }
        return /*YSOS_ERROR_INVALID_ARGUMENTS*/BaseDriverImpl::GetProperty ( iTypeId, piType );
    }

    int HongRunFaceDetectDriver::SetProperty ( int iTypeId, void *piType )
    {
        switch ( iTypeId )
        {
            //case PROP_FACE_MIN_SCORE:
            //  min_face_score_ = *((float*)piType);
            //  YSOS_LOG_DEBUG("Set min face score: " << min_face_score_);
            //  break;
            case PROP_FACE_RECOGNIZE_INTERVAL:
                {
                    int interval = * ( ( int* ) piType );
                    FaceModuleConfig::GetInstance()->SetRecognizeInterval ( interval );
                    break;
                }
            case PROP_MIN_RECOGNIZE_SCORE:
                {
                    float score = * ( ( float* ) piType );
                    FaceModuleConfig::GetInstance()->SetMinRecognzieScore ( score );
                    break;
                }
            case PROP_MIN_LOCAL_RECOGNIZE_SCORE:
                {
                    float score = * ( ( float* ) piType );
                    FaceModuleConfig::GetInstance()->SetMinLocalRecognzieScore ( score );
                    break;
                }
            case PROP_FACE_DETECTOR:
                {
                    const char *type = ( const char * ) piType;
                    /// 初始化不放在这里，直接放到构造函数中，以便加载虹软driver的配置
                    face_detect_ptr_ = FaceDetectFactory::CreateDetector ( type );
                    if ( NULL == face_detect_ptr_.get() )
                    {
                        //assert ( face_detect_ptr_ );
                        YSOS_LOG_ERROR ( "error, Wrong face detector type: " << type );
                    }
#ifdef _WIN32                    
                    /// 设置参数
                    if ( stricmp ( "hongrun", type ) == 0 && face_detect_ptr_ )
                    {
                        face_detect_ptr_->SetConfigKeyValue ( "hr_appid", map_driver_param_["hr_appid"] );
                        face_detect_ptr_->SetConfigKeyValue ( "hr_fd_sdkkey", map_driver_param_["hr_fd_sdkkey"] );
                        face_detect_ptr_->SetConfigKeyValue ( "hr_ft_sdkkey", map_driver_param_["hr_ft_sdkkey"] );
                        face_detect_ptr_->SetConfigKeyValue ( "hr_gender_sdkkey", map_driver_param_["hr_gender_sdkkey"] );
                        face_detect_ptr_->SetConfigKeyValue ( "hr_age_sdkkey", map_driver_param_["hr_age_sdkkey"] );
                        face_detect_ptr_->SetConfigKeyValue ( "hr_fr_sdkkey", map_driver_param_["hr_fr_sdkkey"] );
                        face_detect_ptr_->SetConfigKeyValue ( "min_face_avg_similar_score", map_driver_param_["min_face_avg_similar_score"] );
                        face_detect_ptr_->SetConfigKeyValue ( "face_trace_min_cache_face_info_milliseonds", map_driver_param_["face_trace_min_cache_face_info_milliseonds"] );
                        face_detect_ptr_->SetConfigKeyValue ( "estimate_people_distance_use_cameraid", map_driver_param_["estimate_people_distance_use_cameraid"] );
                        face_detect_ptr_->SetConfigKeyValue ( "max_horizon_angle_half", map_driver_param_["max_horizon_angle_half"] );
                        face_detect_ptr_->SetConfigKeyValue ( "max_vertical_angle_half", map_driver_param_["max_vertical_angle_half"] );
                    }
#else
                    /// 设置参数
                    if ( strcasecmp ( "hongrun", type ) == 0 && face_detect_ptr_ )
                    {
                        face_detect_ptr_->SetConfigKeyValue ( "hr_appid", map_driver_param_["hr_appid"] );
                        face_detect_ptr_->SetConfigKeyValue ( "hr_fd_sdkkey", map_driver_param_["hr_fd_sdkkey"] );
                        face_detect_ptr_->SetConfigKeyValue ( "hr_ft_sdkkey", map_driver_param_["hr_ft_sdkkey"] );
                        face_detect_ptr_->SetConfigKeyValue ( "hr_gender_sdkkey", map_driver_param_["hr_gender_sdkkey"] );
                        face_detect_ptr_->SetConfigKeyValue ( "hr_age_sdkkey", map_driver_param_["hr_age_sdkkey"] );
                        face_detect_ptr_->SetConfigKeyValue ( "hr_fr_sdkkey", map_driver_param_["hr_fr_sdkkey"] );
                        face_detect_ptr_->SetConfigKeyValue ( "min_face_avg_similar_score", map_driver_param_["min_face_avg_similar_score"] );
                        face_detect_ptr_->SetConfigKeyValue ( "face_trace_min_cache_face_info_milliseonds", map_driver_param_["face_trace_min_cache_face_info_milliseonds"] );
                        face_detect_ptr_->SetConfigKeyValue ( "estimate_people_distance_use_cameraid", map_driver_param_["estimate_people_distance_use_cameraid"] );
                        face_detect_ptr_->SetConfigKeyValue ( "max_horizon_angle_half", map_driver_param_["max_horizon_angle_half"] );
                        face_detect_ptr_->SetConfigKeyValue ( "max_vertical_angle_half", map_driver_param_["max_vertical_angle_half"] );
                    }
#endif
                    if ( face_detect_ptr_ )
                    {
                        face_detect_ptr_->Init ( NULL );
                    }
                    break;
                }
            case PROP_FACE_RECOGNIZER:
                {
                    const char *type = ( const char * ) piType;
                    face_recognize_ptr_ = FaceDetectFactory::CreateRecognizer ( type );
                    if ( NULL == face_recognize_ptr_.get() )
                    {
                        //assert ( face_recognize_ptr_ );
                        YSOS_LOG_ERROR ( "error, Wrong face recognizer type: " << type );
                    }
                    if ( face_recognize_ptr_ )
                    {
                        face_recognize_manager_.Init ( face_recognize_ptr_ );
                        face_recognize_manager_.SetConfigKeyValue ( "call1n_max_times", map_driver_param_["call1n_max_times"] );
                    }
                    break;
                }
            case PROP_RECOGNIZE_SERVER:
                {
                    const char *url = ( const char * ) piType;
                    FaceModuleConfig::GetInstance()->SetRecognizerServer ( url );
                    break;
                }
            case PROP_ORG_ID:
                {
                    const char *id = ( const char * ) piType;
                    //id = "000000001";
                    FaceModuleConfig::GetInstance()->SetOrgID ( id );
                    break;
                }
            case PROP_TERM_ID:
                {
                    const char *id = ( const char * ) piType;
                    //id = "T0000001";
                    FaceModuleConfig::GetInstance()->SetTermID ( id );
                    break;
                }
            case PROP_MIN_FACE_WIDTH:
                {
                    int width = * ( ( int* ) piType );
                    FaceModuleConfig::GetInstance()->SetMinFaceWidth ( width );
                    break;
                }
            case PROP_MIN_FACE_HEIGHT:
                {
                    int height = * ( ( int* ) piType );
                    FaceModuleConfig::GetInstance()->SetMinFaceHeight ( height );
                    break;
                }
            case PROP_ENABLE_LOCAL_RECOGNIZE:
                {
                    bool enable = * ( ( bool* ) piType );
                    FaceModuleConfig::GetInstance()->EnableLocalRecognize ( enable );
                    break;
                }
        }
        return YSOS_ERROR_SUCCESS;
    }

    int HongRunFaceDetectDriver::Open ( void *pParams )
    {
        //face_detect_ptr_->Init(NULL);
        //face_recognize_manager_.Init(face_recognize_ptr_);
        return YSOS_ERROR_SUCCESS;
    }

    void HongRunFaceDetectDriver::Close ( void *pParams )
    {
        face_detect_ptr_->UnInit();
        face_recognize_manager_.UnInit();
    }


    int HongRunFaceDetectDriver::DetectFaceRecognize ( FaceDetectRequest* pin, std::vector<TDFaceDetectUnit>& vec_face_unit )
    {
        YSOS_LOG_DEBUG("HongRunFaceDetectDriver::DetectFaceRecognize [Enter]");
        assert ( pin );
        if ( NULL == pin )
        {
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == pin->frame_data )
        {
            YSOS_LOG_ERROR ( "the frame data is null" );
            return YSOS_ERROR_FAILED;
        }
        YSOS_LOG_DEBUG("HongRunFaceDetectDriver::DetectFaceRecognize [Check][0]");
        std::vector<TDFaceDetectInfoPtr> vec_face_info;
        int det_ret = face_detect_ptr_->FaceDetect ( ( const char* ) pin->frame_data,
                      pin->frame_widht, pin->frame_height, pin->frame_channels,
                      true, true, false,
                      true, false, true,
                      false, false,
                      vec_face_info );
        YSOS_LOG_DEBUG("HongRunFaceDetectDriver::DetectFaceRecognize [Check][1]");
        YSOS_LOG_DEBUG("HongRunFaceDetectDriver::DetectFaceRecognize [Check][1][det_ret]=" << det_ret);
        if ( YSOS_ERROR_SUCCESS != det_ret )
        {
            return det_ret;
        }
        vec_face_unit.clear();
        int recog_ret = face_recognize_manager_.FaceRecognition ( vec_face_info, vec_face_unit ); ///< 这里需要人脸base64来查人脸信息
        assert ( YSOS_ERROR_SUCCESS == recog_ret && "人脸识别失败" );
        ///// 去掉imagebase64
        //if(false == pin->get_face_image_base64 && vec_face_unit.size() >0) {
        //  std::vector<TDFaceDetectUnit> tmp_vec = vec_face_unit;
        //  for (int i=0; i< vec_face_unit.size(); i)
        //  {
        //    TDFaceDetectInfoPtr tmp_ptr(new TDFaceDetectInfo);
        //    if(NULL == tmp_ptr) {
        //      return YSOS_ERROR_FAILED;
        //    }
        //
        //  }
        //}
        YSOS_LOG_DEBUG("HongRunFaceDetectDriver::DetectFaceRecognize [Check][2]");
        YSOS_LOG_DEBUG("HongRunFaceDetectDriver::DetectFaceRecognize [Check][2][recog_ret]=" << recog_ret);
        YSOS_LOG_DEBUG("HongRunFaceDetectDriver::DetectFaceRecognize [End]");
        return det_ret;
    }

    int HongRunFaceDetectDriver::Initialized ( const std::string &key, const std::string &value )
    {
        YSOS_LOG_INFO ( "initilized, key:" << key << "value:" << value );
        map_driver_param_[key] = value;
        //int base_ret = face_detect_ptr_->SetConfigKeyValue(key, value);
        //if (YSOS_ERROR_SUCCESS == base_ret) { ///< 此参数被 hongrun_face_detect_ 处理过
        //  return YSOS_ERROR_SUCCESS;
        //}
        int base_ret = face_recognize_manager_.SetConfigKeyValue ( key, value );
        if ( YSOS_ERROR_SUCCESS == base_ret )
        {
            return YSOS_ERROR_SUCCESS;
        }
        return YSOS_ERROR_FAILED;
    }

    int HongRunFaceDetectDriver::CallAbility ( AbilityParam* pInput, AbilityParam* pOut )
    {
        YSOS_LOG_DEBUG("HongRunFaceDetectDriver::CallAbility [Enter]");
        if ( NULL == pInput || NULL == pOut )
        {
            YSOS_LOG_DEBUG ( "error call ablicity , input/out alibityparam is null" );
            return YSOS_ERROR_FAILED;
        }
        YSOS_LOG_DEBUG("HongRunFaceDetectDriver::CallAbility [Check][0]");
#ifdef _WIN32
        if ( stricmp ( ABSTR_FACEDETECT_RECOGNITION, pOut->ability_name ) == 0 )
        {
            if ( stricmp ( DTSTR_STREAM_FRAME_RGB24_P6, pInput->data_type_name ) == 0
                    && stricmp ( DTSTR_TEXT_JSON, pOut->data_type_name ) == 0 )
            {
                return AbilityFaceDetectRecognition_RG24BP62TextJson ( pInput, pOut );
            }
        }
        else
        {
            YSOS_LOG_DEBUG ( "not support ability_name = " << pOut->ability_name );
            return YSOS_ERROR_FAILED;
        }
#else
        if ( strcasecmp ( ABSTR_FACEDETECT_RECOGNITION, pOut->ability_name ) == 0 )
        {
            YSOS_LOG_DEBUG("HongRunFaceDetectDriver::CallAbility [Check][1]");
            YSOS_LOG_DEBUG("HongRunFaceDetectDriver::CallAbility [Check][stream_frame_rgb@24@p6]=**=[pInput->data_type_name]=" << pInput->data_type_name);
			YSOS_LOG_DEBUG("HongRunFaceDetectDriver::CallAbility [Check][text_json]=**=[pOut->data_type_name]=" << pOut->data_type_name);
            //YSOS_LOG_DEBUG("HongRunFaceDetectDriver::CallAbility [Check][text_json]=**=[pOut->data_type_name]==" << strcasecmp ( DTSTR_TEXT_JSON, trim(pOut->data_type_name, ' ').c_str()));
            if ( strcasecmp ( DTSTR_STREAM_FRAME_RGB24_P6, pInput->data_type_name ) == 0 && strcasecmp ( DTSTR_TEXT_JSON, trim(pOut->data_type_name, ' ').c_str()) == 0 )
            {
                YSOS_LOG_DEBUG("HongRunFaceDetectDriver::CallAbility [Check][2]");
                return AbilityFaceDetectRecognition_RG24BP62TextJson ( pInput, pOut );
            }
        }
        else
        {
            YSOS_LOG_DEBUG("HongRunFaceDetectDriver::CallAbility [Check][3]");
            YSOS_LOG_DEBUG ( "not support ability_name = " << pOut->ability_name );
            return YSOS_ERROR_FAILED;
        }
#endif
        YSOS_LOG_DEBUG("HongRunFaceDetectDriver::CallAbility [Check][4]");
		YSOS_LOG_DEBUG("HongRunFaceDetectDriver::CallAbility [End]");
        return YSOS_ERROR_SUCCESS;
    }

    int HongRunFaceDetectDriver::AbilityFaceDetectRecognition_RG24BP62TextJson ( AbilityParam* input_param, AbilityParam* pOut )
    {
        // FaceDetectRequest* pdetect_request = reinterpret_cast<FaceDetectRequest*>(pInput);
        YSOS_LOG_DEBUG("HongRunFaceDetectDriver::AbilityFaceDetectRecognition_RG24BP62TextJson [Enter]");
        FaceDetectRequest detect_request;
        {
#ifdef _WIN32
            if ( stricmp ( DTSTR_STREAM_FRAME_RGB24_P6, input_param->data_type_name ) != 0 )
            {
                assert ( false );
                YSOS_LOG_ERROR ( "Wrong ability name:" << input_param->ability_name );
                return YSOS_ERROR_FAILED;
            }
#else
            if ( strcasecmp ( DTSTR_STREAM_FRAME_RGB24_P6, input_param->data_type_name ) != 0 )
            {
                //assert ( false );
                YSOS_LOG_ERROR ( "Wrong ability name:" << input_param->ability_name );
                return YSOS_ERROR_FAILED;
            }
#endif
            UINT8* data = NULL;
            UINT32 data_size = 0;
            input_param->buf_interface_ptr->GetBufferAndLength ( &data, &data_size );
            if ( NULL == data )
            {
                return YSOS_ERROR_INVALID_ARGUMENTS;
            }
            char channes[2] = {'\0'};
            channes[0] = * ( char* ) ( data + 1 );
            unsigned short width = * ( unsigned short* ) ( data + 2 );
            unsigned short height = * ( unsigned short* ) ( data + 4 );
            uint8_t *frame_data = data + 6;
            detect_request.frame_widht = width;
            detect_request.frame_height = height;
            detect_request.frame_data = frame_data;
            detect_request.frame_channels = /*atoi(channes)*/3;
            detect_request.get_face_feature = false;
            detect_request.get_face_image_base64 = false;
            detect_request.get_face_property = true;
            detect_request.get_face_property_ex = true;
            //detect_request.terminal_id
        }
        YSOS_LOG_DEBUG("HongRunFaceDetectDriver::AbilityFaceDetectRecognition_RG24BP62TextJson [Check][0]");
        std::vector<TDFaceDetectUnit> vec_units;
        int det = DetectFaceRecognize ( &detect_request, vec_units );        
		YSOS_LOG_DEBUG("HongRunFaceDetectDriver::AbilityFaceDetectRecognition_RG24BP62TextJson [Check][1]");
        YSOS_LOG_DEBUG("HongRunFaceDetectDriver::AbilityFaceDetectRecognition_RG24BP62TextJson [Check][1][det] = " << det);
        if ( YSOS_ERROR_SUCCESS != det )
        {
            return det;
        }
        YSOS_LOG_DEBUG("HongRunFaceDetectDriver::AbilityFaceDetectRecognition_RG24BP62TextJson [Check][2]");
        Json::Value json_vec_unit;
        int encoderet = EncodeJson2 ( vec_units, false, NULL, &json_vec_unit ); ///< 不要把image base64加进去
        Json::Value root_json;
        root_json["type"] = "face_info";
        root_json["data"] = json_vec_unit;
        Json::FastWriter json_write;
        std::string json_string = json_write.write ( root_json );
        json_string = GetUtility()->ReplaceAllDistinct ( json_string, "\\r\\n", "" );
        if ( json_string.length() > 0 && pOut )
        {
            UINT8* pout = NULL;
            UINT32 pout_size = 0;
            pOut->buf_interface_ptr->GetBufferAndLength ( &pout, &pout_size );
            {
                UINT32 max_length = 0, prefix_length = 0;
                pOut->buf_interface_ptr->GetMaxLength ( &max_length );
                pOut->buf_interface_ptr->GetPrefixLength ( &prefix_length );
                pout_size = max_length - prefix_length;
            }
            if ( pout_size < ( json_string.length() + 1 ) )
            {
                YSOS_LOG_DEBUG ( "error call ability pcmx1x->asr, output buffer is to small" );
                pOut->buf_interface_ptr->SetLength ( 0 );
                return YSOS_ERROR_FAILED;
            }
#ifdef _WIN32
            strcpy_s ( reinterpret_cast<char*> ( pout ), ( json_string.length() + 1 ), json_string.c_str() );
#else
            strcpy ( reinterpret_cast<char*> ( pout ), json_string.c_str() );
#endif
            pOut->buf_interface_ptr->SetLength ( json_string.length() + 1 );
            YSOS_LOG_DEBUG("HongRunFaceDetectDriver::AbilityFaceDetectRecognition_RG24BP62TextJson [Check][json_string] = " <<json_string.c_str());
            YSOS_LOG_DEBUG("HongRunFaceDetectDriver::AbilityFaceDetectRecognition_RG24BP62TextJson [Check][3]");
            return YSOS_ERROR_SUCCESS;
        }
        else
        {
            //no result;
            YSOS_LOG_DEBUG("HongRunFaceDetectDriver::AbilityFaceDetectRecognition_RG24BP62TextJson [Check][4]");
            pOut->buf_interface_ptr->SetLength ( 0 );
            return YSOS_ERROR_SUCCESS;
        }
        YSOS_LOG_DEBUG("HongRunFaceDetectDriver::AbilityFaceDetectRecognition_RG24BP62TextJson [Check][5]");
		YSOS_LOG_DEBUG("HongRunFaceDetectDriver::AbilityFaceDetectRecognition_RG24BP62TextJson [End]");
        return YSOS_ERROR_SUCCESS;
    }

    /// 以下是为发兼容公版YSOS，参考模块face_detect_driver
    //////////////////////////////////////////////////////////////////////////
    int HongRunFaceDetectDriver::DetectFaceAsync ( void* param )
    {
        FunObjectCommon2* pobject = reinterpret_cast<FunObjectCommon2*> ( param );
        if ( NULL == pobject )
        {
            YSOS_LOG_DEBUG ( "Null param when call property PROP_FACE_DETECT_ASYNC" );
            return YSOS_ERROR_FAILED;
        }
        FaceDetectRequest* pin = reinterpret_cast<FaceDetectRequest*> ( pobject->pparam1 );
        FaceDetectInfoGroup* pout =
            reinterpret_cast<FaceDetectInfoGroup*> ( pobject->pparam2 );
        if ( NULL == pin || NULL == pout )
        {
            YSOS_LOG_DEBUG ( "in/out param is null" );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == pin->frame_data )
        {
            YSOS_LOG_ERROR ( "the frame data is null" );
            return YSOS_ERROR_FAILED;
        }
        std::vector<TDFaceDetectUnit> vec_units;
        int det = DetectFaceRecognize ( pin, vec_units );
        #ifdef _WFF_DEBUG
        YSOS_LOG_INFO ( "DetectFaceRecognize face num:" << vec_units.size() );
        #endif
        if ( YSOS_ERROR_SUCCESS != det )
        {
            return det;
        }
        // 写入返回结果
        pout->arr_face_info =
            TDFaceDetectInfoArray ( new TDFaceDetectInfo[vec_units.size()] );
        for ( size_t i = 0; i < vec_units.size(); ++i )
        {
            TDFaceDetectInfoPtr& detect_infos_ret = vec_units[i].face_detect_info_ptr_;
            pout->arr_face_info[i].age = detect_infos_ret->age;
            pout->arr_face_info[i].width = detect_infos_ret->width;
            pout->arr_face_info[i].hight = detect_infos_ret->hight;
            pout->arr_face_info[i].x = detect_infos_ret->x;
            pout->arr_face_info[i].y = detect_infos_ret->y;
            pout->arr_face_info[i].face_quality_score = detect_infos_ret->face_quality_score;
            pout->arr_face_info[i].trackid = detect_infos_ret->trackid;
            pout->arr_face_info[i].gender = detect_infos_ret->gender;
            // 添加新的人脸追踪信息
            /*face_compare_->AddFaceInfo(detect_infos[i]);*/
        }
        pout->face_number = vec_units.size();
        last_time_faces_info_ = vec_units;
        return YSOS_ERROR_SUCCESS;
    }

    /// 如果有人脸，则返回success,否则返回fail
    int HongRunFaceDetectDriver::GetFaceRecognizeResult ( FaceComparisonResult1N *result )
    {
        if ( NULL == result )
        {
            YSOS_LOG_ERROR ( "Ouput param is null when get recognize result." );
            return YSOS_ERROR_FAILED;
        }
        // 选最大人脸. 如果有详细人脸信息，则返回此人信息
        int max_indx = -1;
        int max_size = 0;
        for ( size_t i = 0; i < last_time_faces_info_.size(); ++i )
        {
            TDFaceDetectInfoPtr& face_detect_ptr_ref = last_time_faces_info_[i].face_detect_info_ptr_;
            if ( max_size < face_detect_ptr_ref->width * face_detect_ptr_ref->hight )
            {
                max_size = face_detect_ptr_ref->width * face_detect_ptr_ref->hight;
                max_indx = i;
            }
        }
        if ( max_indx < 0 )
        {
            return YSOS_ERROR_FAILED;
        }
        if ( -1 == last_time_faces_info_[max_indx].have_perfect_face_ )
        {
            last_time_faces_info_.clear();
            return YSOS_ERROR_SUCCESS;
        }
        if ( 1 != last_time_faces_info_[max_indx].have_perfect_face_ )
        {
            return YSOS_ERROR_FAILED;
        }
        if ( ! ( last_time_faces_info_[max_indx].perfect_face_1n_ptr_ && last_time_faces_info_[max_indx].perfect_face_1n_ptr_->vec_faces.size() > 0 ) )
        {
            return YSOS_ERROR_FAILED;
        }
        /// !!!! 为了兼容module，trackid 要放到personid字段中
        ///  result->vec_faces.push_back(last_time_faces_info_[max_indx].perfect_face_1n_ptr_->vec_faces[0]);
        FaceCompareResultPtr new_result_ptr ( new FaceCompareResult );
        if ( NULL == new_result_ptr )
        {
            return YSOS_ERROR_FAILED;
        }
        *new_result_ptr = *last_time_faces_info_[max_indx].perfect_face_1n_ptr_->vec_faces[0];
        new_result_ptr->person_id = boost::lexical_cast<std::string> ( last_time_faces_info_[max_indx].face_detect_info_ptr_->trackid );
        result->vec_faces.push_back ( new_result_ptr );
        last_time_faces_info_.clear(); ///< 清空数据.
        #ifdef _WFF_DEBUG
        YSOS_LOG_INFO ( "GetFaceRecognizeResult, result size:" << result->vec_faces.size() );
        #endif
        return YSOS_ERROR_SUCCESS;
    }

    int HongRunFaceDetectDriver::GetFaceCompareResult ( int* result )
    {
        if ( -1 == base_track_id_ || -1 == cmp_track_id_ )
        {
            return YSOS_ERROR_FAILED;
        }
        #ifdef _WFF_DEBUG
        YSOS_LOG_INFO ( "GetFaceCompareResult, base trackid:" << base_track_id_ << "  cmp track id:" << cmp_track_id_ );
        #endif
        result[0] = cmp_track_id_;
        result[1] = base_track_id_ == cmp_track_id_ ? 1 : 0;
        if ( base_track_id_ != cmp_track_id_ ) ///< 再通过1比n数据进行比对
        {
            int is_match = 0;
            face_recognize_manager_.FaceCompareAccordDetailInfo ( base_track_id_, cmp_track_id_, is_match );
            if ( 1 == is_match )
            {
                result[1] = 1;   ///< 是同一个人
            }
        }
        /// 获取一次后清空数据
        //base_track_id_ = -1;
        cmp_track_id_ = -1;
        return YSOS_ERROR_SUCCESS;
    }

    /// 以下是为发兼容公版YSOS，参考模块face_detect_driver
    int HongRunFaceDetectDriver::Ioctl ( int iCtrlID, BufferInterfacePtr pInputBuffer,
                                         BufferInterfacePtr pOutputBuffer )
    {
        UINT8 *inbufferptr = NULL;
        UINT32 inbuffersize = 0;
        pInputBuffer->GetBufferAndLength ( &inbufferptr, &inbuffersize );
        YSOS_LOG_DEBUG ( "FaceDetectDirver::Ioctl " << iCtrlID );
        switch ( iCtrlID )
        {
            case CMD_FACE_RCGN_1_1_SET_BASE:            ///< 以下是为发兼容公版YSOS，参考模块face_detect_driver
                {
                    /// 查看策略层代码，此指令只有服务对象为1个时 开始服务 时调 :保存当前服务对象的id
                    int track_id = atoi ( ( char* ) inbufferptr );
                    //face_compare_->Register(track_id);
                    base_track_id_ = track_id;
                    break;
                }
            case CMD_FACE_RCGN_1_1_CMP_BY_ID:          ///< 以下是为发兼容公版YSOS，参考模块face_detect_driver
                {
                    /// 查看策略层代码，此指令只有服务对象为1个时 客户靠近才会调:保存待服务对象的id
                    int track_id = atoi ( ( char* ) inbufferptr );
                    //face_compare_->Compare(track_id);
                    cmp_track_id_ = track_id;
                    break;
                }
            case CMD_FACE_RCGN_1_1_CLEAR_BASE:        ///< 以下是为发兼容公版YSOS，参考模块face_detect_driver
                {
                    /// 查看策略层代码，此指令只有在多人的情况下才会调 ，开始服务时调，如果是多少服务，则清除当前服务对象的id
                    int track_id = atoi ( ( char* ) inbufferptr );
                    //face_compare_->Unregister(track_id);
                    base_track_id_ = -1;
                    cmp_track_id_ = -1;
                    break;
                }
        }
        return YSOS_ERROR_SUCCESS;
    }

    //////////////////////////////////////////////////////////////////////////
    std::string HongRunFaceDetectDriver::trim(const char * src, char ch /*= ' '*/) {
        int startpos = 0;
        //int endpos = _tcslen(src) -1;
        int endpos = strlen(src) -1;

        while (startpos<=endpos && ((src[startpos] == ch) || (src[startpos] == '\n') || (src[startpos] == '\r'))) ++startpos;
        if (startpos>endpos) return ("");

        while (endpos>=startpos && ((src[startpos] == ch) || (src[startpos] == '\n') || (src[startpos] == '\r'))) --endpos;

        return std::string(src+startpos, endpos-startpos+1);
    }


}