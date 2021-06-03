/**
 *@file customerdetstrategycallback.cpp
 *@brief the customer detect strategy call back
 *@version 1.0
 *@author LiShengJun
 *@date Created on: 2017-08-21 10:00:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
/// Private Headers //  NOLINT
#include "../include/customerdetstrategycallback.h"
#include "../../../public/include/common/commonenumdefine.h"

#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"

const double INFINATE_DIS = 99.0;
const char *INFINATE_CSTM_ID = "&##&";

const char *TTS_MODULE          = "default@TtsExtModule";
const char *TTS_IOCTL_ID        = "8139";

const char *AUDIO_CAPTURE       = "default@AudioCaptureExtModule";
const char *AUDIO_OPEN          = "26501";
const char *AUDIO_CLOSE         = "26502";

const char *CHASSIS_MODULE      = "default@ChassisModule1";

const char *ARTIFICIAL_MODULE   = "default@ArtificialAuxiliaryModule";

const char *FACE_DETECT_MODULE  = "default@FaceDetectModule1";

const char *WORK_STATE          = "work_state";
const char *IS_CHARGING         = "is_charging";
const char *IS_BATTERY_LOW      = "is_battery_low";

const char *ROBOT_STATE          = "robot_state";
const char *ROBOT_STATE_NULL     = "null";
const char *ROBOT_PREPARE_CRUISE = "prepare_cruise";
const char *ROBOT_CRUISE_SERVE   = "cruise_serve";
const char *ROBOT_RUN_MOVING     = "run_moving";
const char *ROBOT_MOVEPOINT_END  = "point_end";
const char *ROBOT_MOVEDIR_END    = "dir_end";
const char *ROBOT_GO_HOME        = "go_home";
const char *ROBOT_CHARGING       = "charging";
const char *ROBOT_FAIL_CHARGE    = "failcharge";

const char *CUSTOMER_STATUS     = "customer_status";
const char *FRONT_CONTROL       = "front_control";

const char *OBS_DIS_FRONT       = "chassis_obs_dis_front";
const char *OBS_NEAR_ANGLE      =  "chassis_obs_near";
const char *CHASSIS_COORDINATE  = "coordinate";

const char *LAST_CRUISE_STATUS  = "last_cruise_status";
const char *CURISE_ANS          = "curise_ans";

// customer server status
const char *SERVER_STATUS_IDLE = "0";          ///< 未服务状态        // NOLINT
const char *SERVER_STATUS_AT_SERVICE = "1";    ///< 服务状态          // NOLINT

enum FACE_DET_FLAG
{
    FACE_DET_INVAILD = -1,                  ///< 非法值

    FACE_DET_NONE = 0,                      ///< 不需要识别
    FACE_DET_UPDATE_ONLY,                   ///< 换人重新识别（不需要问候，不要切主页）

    FACE_DET_HOME_GREET,                    ///< 需要问候，需要切主页，默认换人
    FACE_DET_CHG_GREET,                     ///< 换人才问候，不要切主页，默认没换人
    FACE_DET_HOME_CHG_GREET,                ///< 换人才问候，需要切主页，默认没换人

    FACE_DET_HOME_GREET_SAME,               ///< 同一个人，切主页
};

namespace ysos {
    DECLARE_PLUGIN_REGISTER_INTERFACE (
                CustomerDetStrategyCallback,
                CallbackInterface );  ///<  插件的入口，一定要加上 //  NOLINT
    CustomerDetStrategyCallback::CustomerDetStrategyCallback (
                const std::string &strClassName ) :
        BaseStrategyCallbackImpl ( strClassName )
        , low_power_refuse_()
        , charging_serve_ ( true )
        , static_immediate_ ( false )
        , audio_open_ ( false )
        , prepare_cruise_tm_ ( 5.0 )
        , prepare_start_()
        , greet_dis_ ( 0.85 )
        , rcg_max_wait_ ( 1.5 )
        , rcg_start_()
        , HandleMessage_start_()
        , facedet_flag_ ( FACE_DET_NONE )
        , facedet_collect_ ( 0 )
        , may_change_ ( false )
        , chg_face_tm_()
        , face_chg_ctu_ ( 5.0 )
        , face_chg_rate_ ( 0.088 )
        , customer_num_ ( 0 )
        , customer_info_()
        , last_serve_tm_()
        , range_num ( 3 )
        , near_remind_falg_ ( false )
        , near_remind_tm_ ( 70 )
        , near_remind_()
        , mid_remind_falg_ ( false )
        , mid_remind_()
        , enfc_wait_tag_ ( 0 )
        , enfc_wait_start_()
        , enforce_wait_ ( 1.3 )
        , farewell_()
        , genderM ( "先生" )
        , genderF ( "女士" )
        , genderFM ( "大家" )
        , send_name_flag ( "0" )
        , send_name()
        , data_ptr_ ( GetDataInterfaceManager()->GetData (
                                GLOBAL_DATA_KEY ) )
    {
        logger_ = GetUtility()->GetLogger ( "ysos.obsdetstrategy" );
        dis_range_[0] = 0.9;
        dis_range_[1] = 1.1;
        dis_range_[2] = 1.3;
        wait_time_[0] = 90;
        wait_time_[1] = 30;
        wait_time_[2] = 15;
        //ResetStatus();
    }

    CustomerDetStrategyCallback::~CustomerDetStrategyCallback ( void )
    {
    }

    int CustomerDetStrategyCallback::Initialized (
                const std::string &key,
                const std::string &value )
    {
        YSOS_LOG_DEBUG ( key << ": " << value );
        int n_return = YSOS_ERROR_SUCCESS;
        if ( key == "charging_serve" )
        {
            charging_serve_ = ( value == "1" );
        }
        else if ( key == "static_immediate" )
        {
            static_immediate_ = ( value == "1" );
        }
        else if ( key == "low_power_refuse" )
        {
            low_power_refuse_ = value;
        }
        else if ( key == "prepare_cruise_tm" )
        {
            prepare_cruise_tm_ = atof ( value.c_str() );
        }
        else if ( key == "greet_distance" )
        {
            greet_dis_ = atof ( value.c_str() );
        }
        else if ( key == "recognise_max_wait" )
        {
            rcg_max_wait_ = atof ( value.c_str() );
        }
        else if ( key == "face_chg_rate" )
        {
            face_chg_rate_ = atof ( value.c_str() );
        }
        else if ( key == "face_chg_continue" )
        {
            face_chg_ctu_ = atof ( value.c_str() );
        }
        else if ( key == "distance_wait" )
        {
            unsigned int i = 0;
            range_num = 0;
            while ( range_num < DISTANCE_RANGE_MAXNUM
                        && 2 == sscanf ( value.c_str() + i, "%lf,%lf",
                                    &dis_range_[range_num],
                                    &wait_time_[range_num] ) )
            {
                range_num++;
                i = value.find ( '|', i );
                if ( i == std::string::npos )
                {
                    break;
                }
                i++;
            }
        }
        else if ( key == "near_remind" )
        {
            sscanf ( value.c_str(), "%d", &near_remind_tm_ );
            int i = value.find ( '|' );
            if ( i >= 0 )
            {
                near_remind_ = value.substr ( i + 1 );
            }
        }
        else if ( key == "mid_remind" )
        {
            mid_remind_ = value;
        }
        else if ( key == "enforce_wait" )
        {
            enforce_wait_ = atof ( value.c_str() );
        }
        else if ( key == "farewell" )
        {
            farewell_ = value;
        }
        else if ( key == "genderM" )
        {
            genderM = value;
        }
        else if ( key == "genderF" )
        {
            genderF = value;
        }
        else if ( key == "genderFM" )
        {
            genderFM = value;
        }
        else if ( key == "send_name_flag" )
        {
            send_name_flag = value;
        }
        return n_return;
    }

    int CustomerDetStrategyCallback::HandleMessage (const std::string &event_name, BufferInterfacePtr input_buffer, CallbackInterface *context )
    {
        YSOS_LOG_DEBUG ( "HandleMessage " << event_name );
        if ( data_ptr_ == NULL )
        {
            YSOS_LOG_ERROR ( "data_ptr_ is NULL" );
            return YSOS_ERROR_FAILED;
        }
        
        if ( event_name == "face_info" )
        {
            int face_num = 0;
            CstmFaceInfo face_info;
            int rst = ParserFaceInfo ( input_buffer, face_num, face_info );
            YSOS_LOG_DEBUG ( "ParserFaceInfo rst " << rst << " num " << face_num );
            if ( rst == -1 )
            {
                return YSOS_ERROR_FAILED;
            }
            
            // Strategy (1): to detect arriving
            YSOS_LOG_DEBUG ( "rcg_max_wait_: " << rcg_max_wait_ );
            YSOS_LOG_DEBUG ( "GetTimeElapse(rcg_start_): " << GetTimeElapse ( rcg_start_ ) );

            if ( rst == 0 && face_num > 0 && HandleMessage_start_ == boost::date_time::not_a_date_time )
            {
                StartTimer ( HandleMessage_start_ );
                YSOS_LOG_DEBUG ( "GetTimeElapse(HandleMessage_start_): " << GetTimeElapse ( HandleMessage_start_ ) );
            }
            if ( rst == 1 && face_num > 0 )
            {
                customer_num_ = face_num;
                customer_info_ = face_info;
                //customer_info_.id_card = INFINATE_CSTM_ID; // to mark never said hello
                CustomerArrive ( context );
                facedet_flag_ = FACE_DET_HOME_GREET;
            }
            else if ( rst == 0 && face_num > 0 && ( GetTimeElapse ( HandleMessage_start_ ) >= rcg_max_wait_ ) )
            {
                customer_num_ = face_num;
                customer_info_ = face_info;
                //customer_info_.id_card = INFINATE_CSTM_ID; // to mark never said hello
                CustomerArrive ( context );
                facedet_flag_ = FACE_DET_HOME_GREET;
            } else if ( rst == 0 && face_num == 0 ) {
                customer_num_ = face_num;
                CustomerLeave ( "", context );
            } 
        }
        return YSOS_ERROR_SUCCESS;
    }

    int CustomerDetStrategyCallback::CustomerArrive ( CallbackInterface *context )
    {
        YSOS_LOG_DEBUG ( "Customer Arrive [Enter]" );
        //NotifyCustomerServe ( context );
        RequestService("default@TemplateModule", boost::lexical_cast<std::string>(CMD_TRANSFER_EVENT), "text", "{\"type\":\"customer_arrive\",\"data\":{\"key\":\"value\"}}", context);
        YSOS_LOG_DEBUG ( "Customer Arrive [End]" );
        return YSOS_ERROR_SUCCESS;
    }

    int CustomerDetStrategyCallback::CustomerLeave (const std::string &robot_state, CallbackInterface *context )
    {
        YSOS_LOG_DEBUG ( "CustomerLeave " << robot_state );
        // notify
        //NotifyCustomerLeave ( context );
        RequestService("default@TemplateModule", boost::lexical_cast<std::string>(CMD_TRANSFER_EVENT), "text", "{\"type\":\"customer_leave\",\"data\":{\"key\":\"value\"}}", context);
        YSOS_LOG_DEBUG (" CustomerLeave [End] ");
        return YSOS_ERROR_SUCCESS;
    }

    void CustomerDetStrategyCallback::NotifyCustomerServe ( CallbackInterface *context )
    {
        char cstminfo[400];
        if ( customer_info_.age >= 0 && customer_info_.gender != 0 )
        {
            sprintf ( cstminfo, "{\"age\":%d,\"gender\":\"%s\",\"name\":\"%s\",\"id_card\":\"%s\"}",
                        customer_info_.age,
                        customer_info_.gender == 1 ? "male" : "female",
                        customer_info_.name.empty() ? "--" :
                        customer_info_.name.c_str(),
                        customer_info_.id_card.empty() ? "--" :
                        customer_info_.id_card.c_str() );
            DoEventNotifyService ( "customer_info_event", "customer_info_callback", cstminfo, context );
        }
    }

    void CustomerDetStrategyCallback::NotifyCustomerLeave (CallbackInterface *context )
    {
        DoEventNotifyService ( "customer_leave_event", "customer_leave_callback", "", context );
    }

    int CustomerDetStrategyCallback::ParserFaceInfo (BufferInterfacePtr input_buffer, int &num, CstmFaceInfo &face_info )
    {
        const char *buffer = reinterpret_cast<const char *>( GetBufferUtility()->GetBufferData ( input_buffer ) );
        if ( buffer == NULL )
        {
            YSOS_LOG_ERROR ( "buffer is NULL!" );
            return YSOS_ERROR_FAILED;
        }
        YSOS_LOG_DEBUG ( "input_buffer: " << buffer );
        Json::Reader js_reader;
        Json::Value js_value;
        if ( !js_reader.parse ( buffer, js_value, false ) )
        {
            YSOS_LOG_ERROR ( "ParserFaceInfoJsonStr failed!" );
            return YSOS_ERROR_FAILED;
        }
        js_value = js_value["data"];
        int in_num = atoi(js_value["facenum"].asString().c_str());
        //num = 0;
        num = in_num;
        return 0;
        /*
        if ( js_value["recognition"].asInt() == 0 )
        {
            for ( int i = 0; i < in_num; i++ )
            {
                Json::Value face = js_value["face"][ ( Json::UInt ) i];
                double rate = face["rate"].asDouble();
                if ( ++num == 1 || rate > face_info.x_rate )
                {
                    face_info.track_id = face["track_id"].asInt();
                    face_info.rate = rate;
                    face_info.x_rate = face["x_rate"].asDouble();
                    face_info.age = face["age"].asInt();
                    face_info.gender = face["gender"].asInt();
                    face_info.id_card = "";
                    face_info.name = "";
                }
            }
            if ( in_num != 0 )
            {
                printf ( "track %d\n", face_info.track_id );
            }
            return 0;
        }
        else if ( js_value["recognition"].asInt() == 1 )
        {
            if ( in_num == 0 )
            {
                return -1;
            }
            num = 1;
            Json::Value face = js_value["face"][ ( Json::UInt ) 0];
            face_info.track_id = face["track_id"].asInt();
            face_info.rate = 0;
            face_info.x_rate = 0;
            face_info.age = face["age"].asInt();
            face_info.gender = face["gender"].asInt();
            face_info.id_card = face["id_card"].asString();
            face_info.name = face["person_name"].asString();
            return 1;
        }
        else
        {
            num = 1;
            face_info.track_id = js_value["track_id"].asInt();
            face_info.rate = js_value["result"].asInt();
            return 2;
        }*/
    }

    int CustomerDetStrategyCallback::TrackIdUpdated (int face_num, CstmFaceInfo &face_info )
    {
        int rst = YSOS_ERROR_FAILED; // to confirm
        if ( customer_num_ == 1 )
        {
            if ( face_num == 1 )
            {
                if ( customer_info_.track_id != face_info.track_id )
                {
                    if ( face_info.rate > face_chg_rate_ )
                    {
                        may_change_ = false;
                        rst = YSOS_ERROR_SUCCESS; // changed (1->1)
                    }
                    else if ( !may_change_ )
                    {
                        may_change_ = true;
                        StartTimer ( chg_face_tm_ );
                    }
                    else if ( GetTimeElapse ( chg_face_tm_ ) > face_chg_ctu_ )
                    {
                        may_change_ = false;
                        rst = YSOS_ERROR_SUCCESS; // changed (1->1)
                    }
                }
                else
                {
                    may_change_ = false;
                    rst = YSOS_ERROR_SKIP;  // the same
                }
            }
            else
            {
                may_change_ = false;
                rst = YSOS_ERROR_SUCCESS; // changed (1->n)
            }
        }
        else
        {
            if ( face_num == 1 )
            {
                if ( face_info.rate > face_chg_rate_ )
                {
                    may_change_ = false;
                    rst = YSOS_ERROR_SUCCESS; // changed (n->1)
                }
                else if ( !may_change_ )
                {
                    may_change_ = true;
                    StartTimer ( chg_face_tm_ );
                }
                else if ( GetTimeElapse ( chg_face_tm_ ) > face_chg_ctu_ )
                {
                    may_change_ = false;
                    rst = YSOS_ERROR_SUCCESS; // changed (n->1)
                }
            }
        }
        YSOS_LOG_DEBUG ( "TrackIdUpdated " << rst );
        return rst;
    }

    int CustomerDetStrategyCallback::RequestService (
                std::string service_name, std::string id,
                std::string type, std::string json_value,
                CallbackInterface *context )
    {
        YSOS_LOG_DEBUG ( "RequestService " << service_name << " " << id << " " << type << " " << json_value );
        int n_return = YSOS_ERROR_SUCCESS;
        ReqIOCtlServiceParam req_service;
        req_service.id = id;
        req_service.service_name = service_name;
        if ( !type.empty() )
        {
            req_service.type = type;
        }
        if ( !json_value.empty() )
        {
            req_service.value = json_value;
        }
        n_return = DoIoctlService ( req_service, context );
        if ( YSOS_ERROR_SUCCESS != n_return )
        {
            YSOS_LOG_DEBUG ( "execute DoIoctlService failed, n_return = " << n_return );
        }
        return n_return;
    }

    void CustomerDetStrategyCallback::StartTimer ( boost::posix_time::ptime &cur_tm )
    {
        cur_tm = boost::posix_time::microsec_clock::local_time();
    }

    double CustomerDetStrategyCallback::GetTimeElapse ( boost::posix_time::ptime &last_tm_ )
    {
        boost::posix_time::ptime curtime = boost::posix_time::microsec_clock::local_time();
        boost::posix_time::millisec_posix_time_system_config::time_duration_type time_elapse = curtime - last_tm_;
        return double ( time_elapse.total_milliseconds() ) / 1000.0;
    }

}