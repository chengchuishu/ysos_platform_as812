/**
 *@file movepointstrategycallback.cpp
 *@brief move to point strategy call back
 *@version 1.0
 *@author LiShengJun
 *@date Created on: 2016-10-20 15:16:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
/// Private Headers //  NOLINT
#include "../include/movepointstrategycallback.h"

#include "../../../public/include/common/commonenumdefine.h"

#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"

const char* TTS_MODULE          = "default@TtsExtModule";
const char* TTS_IOCTL_ID        = "8139";

const char* CHASSIS_MODULE      = "default@ChassisModule1";

const char* IS_BATTERY_LOW      = "is_battery_low";
const char* CUSTOMER_STATUS     = "customer_status";
const char* LAST_CRUISE_STATUS  = "last_cruise_status";

const char* ROBOT_STATE          = "robot_state";
const char* ROBOT_STATE_NULL     = "null";
const char* ROBOT_CRUISE_SERVE   = "cruise_serve";
const char* ROBOT_RUN_MOVING     = "run_moving";
const char* ROBOT_MOVEPOINT_END  = "point_end";
const char* ROBOT_MOVEDIR_END    = "dir_end";
const char* ROBOT_GO_HOME        = "go_home";
const char* ROBOT_CHARGING       = "charging";
const char* ROBOT_FAIL_CHARGE    = "failcharge";

const char* TIME_STAMP          = "time_stamp";

const char* ROBOT_MOVING_ANS    = "moving_ans";
const char* ROBOT_MOVING_AIM    = "moving_aim";

const char* MOVE_MODE           = "move_mode";
const char* MOVE_MODE_POINT     = "2";
const char* MOVE_MODE_DIRECTION = "6";
const char* MOVE_NAME           = "name";
const char* MOVE_STATE          = "task_status";
const char* MOVE_STATE_REACH    = "finish";
const char* MOVE_STATE_UNREACH  = "unreach";
const char* MOVE_STATE_WARNING  = "warning";

namespace ysos {
    DECLARE_PLUGIN_REGISTER_INTERFACE (
                MovePointStrategyCallback,
                CallbackInterface ); ///<  插件的入口，一定要加上 //  NOLINT
    MovePointStrategyCallback::MovePointStrategyCallback (
                const std::string &strClassName ) :
        BaseStrategyCallbackImpl ( strClassName )
        , data_ptr_ ( GetDataInterfaceManager()->GetData (
                                GLOBAL_DATA_KEY ) )
        , timer_start_()
        , speaked_ ( false )

        , move_dir_wait_spk_ ( 10 )
        , move_dir_wait_go_ ( 20 )
        , md_wait_words_ ( "宝宝看不到您了，宝宝需要面对面的交流哦" )
        , low_power_refuse_ ( "对不起，宝宝能量不足，无法执行命令了" )

        , walk_names_()
        , walk_name_()
        , move_point_wait_spk_ ( 10 )
        , move_point_wait_go_ ( 20 )
        , mp_wait_words_ ( "请问您还有其它需要帮助的吗" )
        , move_success_words_ ( "我们走到%s了" )
        , move_failed_words_ ( "对不起，%s现在过不去" )
        , move_skip_words_ ( "我们已经在%s附近了" )
        , move_unknown_words_ ( "对不起，宝宝不认识%s" )
        , low_power_warn_ ( "对不起，宝宝能量不够，无法给您带路了" )
        , warning_gap_ ( 10 )
        , last_warning_spk_()
        , warning_words_ ( "宝宝正在执行任务，请大家避让一下" )
    {
        logger_ = GetUtility()->GetLogger ( "ysos.obsdetstrategy" );
    }

    MovePointStrategyCallback::~MovePointStrategyCallback (
                void )
    {
    }

    int MovePointStrategyCallback::HandleMessage (
                const std::string &event_name,
                BufferInterfacePtr input_buffer,
                CallbackInterface *context )
    {
        YSOS_LOG_DEBUG ( "[MP] HandleMessage " <<
                    event_name );
        if ( data_ptr_ == NULL )
        {
            YSOS_LOG_ERROR ( "data_ptr_ is NULL!" );
            return YSOS_ERROR_FAILED;
        }
        // chassis info
        std::string robot_state;
        data_ptr_->GetData ( ROBOT_STATE, robot_state );
        if ( robot_state == ROBOT_CRUISE_SERVE
                    || robot_state == ROBOT_CHARGING
                    || robot_state == ROBOT_FAIL_CHARGE )
        {
            // move dir command
            std::string cmd_param;
            if ( event_name == "move_front_chassis_event" )
            {
                cmd_param = "{\"direction\":\"+\"}";
            }
            else if ( event_name ==
                        "move_back_chassis_event" )
            {
                cmd_param = "{\"direction\":\"-\"}";
            }
            else if ( event_name ==
                        "move_left_chassis_event" )
            {
                cmd_param = "{\"rotate\":\"+\"}";
            }
            else if ( event_name ==
                        "move_right_chassis_event" )
            {
                cmd_param = "{\"rotate\":\"-\"}";
            }
            if ( cmd_param.size() > 0 )
            {
                std::string battery_low = "0";
                data_ptr_->GetData ( IS_BATTERY_LOW,
                            battery_low );
                if ( battery_low == "1" )
                {
                    RequestService ( TTS_MODULE, TTS_IOCTL_ID, "text",
                                low_power_refuse_, context );
                }
                else
                {
                    RequestService ( TTS_MODULE, TTS_IOCTL_ID, "text",
                                "好的", context );
                    data_ptr_->SetData ( ROBOT_STATE,
                                ROBOT_RUN_MOVING );
                    data_ptr_->SetData ( CUSTOMER_STATUS, "1" );
                    RequestService ( CHASSIS_MODULE,
                                GetUtility()->ConvertToString (
                                            CMD_CHASSIS_START_MOVE_BY_DIRECTION ), "text",
                                cmd_param, context );
                }
                return YSOS_ERROR_SUCCESS;
            }
            // move to point command
            if ( event_name == "move_walk_service" )
            {
                std::string battery_low = "0";
                data_ptr_->GetData ( IS_BATTERY_LOW,
                            battery_low );
                if ( battery_low == "1" )
                {
                    RequestService ( TTS_MODULE, TTS_IOCTL_ID, "text",
                                low_power_warn_, context );
                    return YSOS_ERROR_SUCCESS;
                }
                MoveCmdParam cmdParam;
                if ( YSOS_ERROR_SUCCESS != ParserMoveWalk (
                                        input_buffer, cmdParam ) )
                {
                    return YSOS_ERROR_FAILED;
                }
                int check_dis = RequestService ( CHASSIS_MODULE,
                                        GetUtility()->ConvertToString (
                                                    CMD_CHASSIS_IS_POSITION_NEAR ), "text",
                                        cmdParam.aim, context );
                if ( YSOS_ERROR_SUCCESS == check_dis )
                {
                    std::string words = MakeAnswerWords (
                                            move_skip_words_, cmdParam.name );
                    RequestService ( TTS_MODULE, TTS_IOCTL_ID, "text",
                                words, context );
                }
                else if ( YSOS_ERROR_INVALID_ARGUMENTS ==
                            check_dis )
                {
                    std::string words = MakeAnswerWords (
                                            move_unknown_words_, cmdParam.name );
                    RequestService ( TTS_MODULE, TTS_IOCTL_ID, "text",
                                words, context );
                }
                else
                {
                    walk_name_ = cmdParam.name;
                    data_ptr_->SetData ( ROBOT_MOVING_ANS,
                                cmdParam.answer );
                    data_ptr_->SetData ( ROBOT_MOVING_AIM,
                                cmdParam.aim );
                    DoSwitchNotifyService ( "WaitingService@TargetSearching@MoveToPoint",
                                context );
                    last_warning_spk_ =
                                boost::posix_time::microsec_clock::local_time();
                }
                return YSOS_ERROR_SUCCESS;
            }
        }
        else if ( robot_state == ROBOT_RUN_MOVING )
        {
            if ( event_name != "chassis_info_moving" )
            {
                return YSOS_ERROR_SUCCESS;
            }
            MoveParam mode_param;
            if ( YSOS_ERROR_SUCCESS != ParserMoveInfo (
                                    input_buffer, mode_param ) )
            {
                return YSOS_ERROR_SUCCESS;
            }
            if ( mode_param.move_mode == MOVE_MODE_POINT )
            {
                // move to point
                std::string words;
                if ( mode_param.move_state == MOVE_STATE_REACH )
                {
                    DoEventNotifyService ( "move_to_point_finish_event",
                                "move_to_point_finish_callback", "", context );
                    words = MakeAnswerWords ( move_success_words_,
                                            walk_name_ );
                    RequestService ( TTS_MODULE, TTS_IOCTL_ID, "text",
                                words, context );
                }
                else if ( mode_param.move_state ==
                            MOVE_STATE_UNREACH )
                {
                    DoEventNotifyService ( "move_to_point_failed_event",
                                "move_to_point_failed_callback", "", context );
                    words = MakeAnswerWords ( move_failed_words_,
                                            walk_name_ );
                    RequestService ( TTS_MODULE, TTS_IOCTL_ID, "text",
                                words, context );
                }
                else if ( mode_param.move_state ==
                            MOVE_STATE_WARNING )
                {
                    boost::posix_time::ptime curtime =
                                boost::posix_time::microsec_clock::local_time();
                    boost::posix_time::millisec_posix_time_system_config::time_duration_type
                    time_elapse = curtime - last_warning_spk_;
                    int elaspse = time_elapse.total_seconds();
                    if ( elaspse >= warning_gap_ )
                    {
                        RequestService ( TTS_MODULE, TTS_IOCTL_ID, "text",
                                    warning_words_, context );
                        last_warning_spk_ = curtime;
                    }
                    return YSOS_ERROR_SUCCESS;
                }
                else
                {
                    return YSOS_ERROR_SUCCESS;
                }
                data_ptr_->SetData ( ROBOT_STATE,
                            ROBOT_MOVEPOINT_END );
                StartTimer();
                speaked_ = false;
            }
            else if ( mode_param.move_mode ==
                        MOVE_MODE_DIRECTION )
            {
                // move to direction
                if ( mode_param.move_state == MOVE_STATE_REACH
                            || mode_param.move_state == MOVE_STATE_UNREACH )
                {
                    data_ptr_->SetData ( ROBOT_STATE,
                                ROBOT_MOVEDIR_END );
                    StartTimer();
                    speaked_ = false;
                }
            }
        }
        else if ( robot_state == ROBOT_MOVEPOINT_END )
        {
            // move point end
            int elaspse = GetElaspse();
            if ( !speaked_ )
            {
                if ( elaspse >= move_point_wait_spk_ )
                {
                    RequestService ( TTS_MODULE, TTS_IOCTL_ID, "text",
                                mp_wait_words_, context );
                    speaked_ = true;
                }
            }
            else
            {
                if ( elaspse >= move_point_wait_go_ )
                {
                    std::string last_cruise;
                    data_ptr_->GetData ( LAST_CRUISE_STATUS,
                                last_cruise );
                    YSOS_LOG_DEBUG ( "LAST_CRUISE_STATUS " <<
                                last_cruise );
                    DoSwitchNotifyService ( last_cruise, context );
                }
            }
        }
        else if ( robot_state == ROBOT_MOVEDIR_END )
        {
            // move direction end
            int elaspse = GetElaspse();
            if ( !speaked_ )
            {
                if ( elaspse >= move_dir_wait_spk_ )
                {
                    RequestService ( TTS_MODULE, TTS_IOCTL_ID, "text",
                                md_wait_words_, context );
                    speaked_ = true;
                }
            }
            else
            {
                if ( elaspse >= move_dir_wait_go_ )
                {
                    data_ptr_->SetData ( ROBOT_STATE,
                                ROBOT_CRUISE_SERVE );
                }
            }
        }
        return YSOS_ERROR_SUCCESS;
    }

    int MovePointStrategyCallback::Initialized (
                const std::string &key,
                const std::string &value )
    {
        YSOS_LOG_DEBUG ( key << ": " << value );
        if ( key == "move_dir_wait_speak" )
        {
            move_dir_wait_spk_ = atoi ( value.c_str() );
        }
        else if ( key == "move_dir_wait_go" )
        {
            move_dir_wait_go_ = atoi ( value.c_str() );
        }
        else if ( key == "move_dir_wait_words" )
        {
            md_wait_words_ = value;
        }
        else if ( key == "low_power_refuse" )
        {
            low_power_refuse_ = value;
        }
        else if ( key == "move_point_wait_speak" )
        {
            move_point_wait_spk_ = atoi ( value.c_str() );
        }
        else if ( key == "move_point_wait_go" )
        {
            move_point_wait_go_ = atoi ( value.c_str() );
        }
        else if ( key == "move_point_wait_words" )
        {
            mp_wait_words_ = value;
        }
        else if ( key == "move_walk_names" )
        {
            char *next = const_cast<char*> ( value.c_str() );
            char *position, *name;
            for ( bool end = false; !end; )
            {
                name = position = next;
                for ( ; *name != '\0' && *name != ':'; ++name );
                if ( *name == '\0' )
                    break;
                *name = '\0';
                next = ++name;
                for ( ; *next != '\0' && *next != '|'; ++next );
                if ( *next == '\0' )
                    end = true;
                else
                    *next++ = '\0';
                walk_names_[position] = name;
            }
        }
        else if ( key == "move_success_words" )
        {
            move_success_words_ = value;
        }
        else if ( key == "move_failed_words" )
        {
            move_failed_words_ = value;
        }
        else if ( key == "move_skip_words" )
        {
            move_skip_words_ = value;
        }
        else if ( key == "move_unknown_words" )
        {
            move_unknown_words_ = value;
        }
        else if ( key == "warning_gap" )
        {
            warning_gap_ = atoi ( value.c_str() );
        }
        else if ( key == "warning_spk_gap" )
        {
            warning_words_ = value;
        }
        else if ( key == "low_power_warn" )
        {
            low_power_warn_ = value;
        }
        return YSOS_ERROR_SUCCESS;
    }

    int MovePointStrategyCallback::RequestService (
                std::string service_name, std::string id,
                std::string type, std::string json_value,
                CallbackInterface *context )
    {
        YSOS_LOG_DEBUG ( "RequestService " << service_name
                    << " " << id << " " << type << " " <<
                    json_value );
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
        n_return = DoIoctlService ( req_service,
                                context );
        if ( YSOS_ERROR_SUCCESS != n_return )
        {
            YSOS_LOG_DEBUG ( "execute DoIoctlService failed, n_return = "
                        << n_return );
        }
        return n_return;
    }

    int MovePointStrategyCallback::ParserMoveWalk (
                BufferInterfacePtr input_buffer,
                MoveCmdParam &cmd_param )
    {
        const char* buffer =
                    reinterpret_cast<const char*>
                    ( GetBufferUtility()->GetBufferData (
                                            input_buffer ) );
        if ( buffer == NULL )
        {
            YSOS_LOG_ERROR ( "buffer is NULL!" );
            return YSOS_ERROR_FAILED;
        }
        YSOS_LOG_DEBUG ( "input_buffer: " << buffer );
        Json::Reader js_reader;
        Json::Value js_value;
        if ( !js_reader.parse ( buffer, js_value,
                                false ) )
        {
            YSOS_LOG_ERROR ( "ParserMoveWalk failed!" );
            return YSOS_ERROR_FAILED;
        }
        if ( js_value.isMember ( "x" ) )
        {
            cmd_param.answer =
                        js_value.removeMember ( "moving_ans" ).asString();
            cmd_param.name =
                        js_value.removeMember ( "aim_name" ).asString();
            Json::FastWriter writer;
            cmd_param.aim = writer.write ( js_value );
            cmd_param.aim = GetUtility()->ReplaceAllDistinct ( cmd_param.aim, "\\r\\n", "" );
        }
        else
        {
            std::string moving_ans;
            moving_ans = js_value["answer_old"].asString();
            int p = moving_ans.rfind ( "*" );
            if ( p == -1 )
            {
                return YSOS_ERROR_FAILED;
            }
            cmd_param.answer = moving_ans.substr ( 0, p );
            moving_ans = moving_ans.substr ( p + 1 );
            std::map<std::string, std::string>::iterator it =
                        walk_names_.find ( moving_ans );
            cmd_param.name = ( it != walk_names_.end() ) ?
                        it->second : "";
            cmd_param.aim = "{\"positionname\":\"";
            cmd_param.aim.append ( moving_ans );
            cmd_param.aim.append ( "\"}" );
        }
        return YSOS_ERROR_SUCCESS;
    }

    int MovePointStrategyCallback::ParserMoveInfo (
                BufferInterfacePtr input_buffer,
                MoveParam &mode_param )
    {
        const char* buffer =
                    reinterpret_cast<const char*>
                    ( GetBufferUtility()->GetBufferData (
                                            input_buffer ) );
        if ( buffer == NULL )
        {
            YSOS_LOG_ERROR ( "buffer is NULL!" );
            return YSOS_ERROR_FAILED;
        }
        YSOS_LOG_DEBUG ( "input_buffer: " << buffer );
        Json::Reader js_reader;
        Json::Value js_value;
        if ( !js_reader.parse ( buffer, js_value,
                                false ) )
        {
            YSOS_LOG_ERROR ( "ParserMoveInfo failed!" );
            return YSOS_ERROR_FAILED;
        }
        js_value = js_value["data"];
        if ( js_value.isMember ( MOVE_MODE ) )
        {
            mode_param.move_mode =
                        js_value[MOVE_MODE].asString();
            mode_param.move_state =
                        js_value[MOVE_STATE].asString();
            return YSOS_ERROR_SUCCESS;
        }
        return YSOS_ERROR_FAILED;
    }

    std::string
    MovePointStrategyCallback::MakeAnswerWords (
                const std::string& format,
                const std::string& name )
    {
        YSOS_LOG_DEBUG ( format << " " << name );
        if ( format.find ( "%s" ) == -1 )
        {
            return format;
        }
        char buff[128];
        sprintf ( buff, format.c_str(), name.c_str() );
        return buff;
    }

    int MovePointStrategyCallback::StartTimer()
    {
        timer_start_ = boost::posix_time::microsec_clock::local_time();
        return 0;
    }

    int MovePointStrategyCallback::GetElaspse()
    {
        boost::posix_time::ptime curtime = boost::posix_time::microsec_clock::local_time();
        boost::posix_time::millisec_posix_time_system_config::time_duration_type time_elapse = curtime - timer_start_;
        int elaspse = time_elapse.total_seconds();
        YSOS_LOG_DEBUG ( "elaspse " << elaspse );
        return elaspse;
    }

}
