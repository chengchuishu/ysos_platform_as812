/**
*@file gaussian_chassis_callback.cpp
*@brief Implement of gaussian chassis callback
*@version 0.9.0.0
*@author Lu Min, Wang Xiaogui
*@date Created on: 2016-12-09 14:28:00
*@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
* 
*/

/// Myself Headers
#include "../include/chassis_callback.h"

/// YSOS Header     //NOLINT
#include "../../../../../../core/trunk/main/public/include/os_hal_package/lock.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
//#include "../../../../../../core/trunk/main/public/include/core_help_package/ysos_log.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"

/// BOOST Header     //NOLINT
#include <boost/date_time/posix_time/posix_time.hpp>  //NOLINT
#include <json/json.h>

#define CHASSIS_OUTPUT_TYPE                     "type"
#define CHASSIS_OUTPUT_DTAT                     "data"
#define CHASSIS_OUTPUT_TYPE_VALUE_OBSTACLE      "chassis_info_obstacle"
#define CHASSIS_OUTPUT_TYPE_VALUE_BATTERY       "chassis_info_battery"
#define CHASSIS_OUTPUT_TYPE_VALUE_MOVING        "chassis_info_moving"
#define CHASSIS_OUTPUT_TYPE_VALUE_GREET         "chassis_info_greet"
#define CHASSIS_OUTPUT_TYPE_VALUE_POSITION      "chassis_info_position"
#define CHASSIS_OUTPUT_TYPE_VALUE_CHARGE_MOVING "chassis_charge_moving"
#define CHASSIS_OUTPUT_TYPE_VALUE_MOVE_RELATIVE "chassis_move_relative"
#define CHASSIS_OUTPUT_TYPE_VALUE_MPA_INIT_OK   "chassis_map_init_OK_event"
#define CHASSIS_OUTPUT_TYPE_VALUE_MPA_INIT_NG   "chassis_map_init_NG_event"

#define CHASSIS_OUTPUT_TIMESTAMP                "time_stamp"
#define CHASSIS_OUTPUT_STATUS_TASK              "task_status"
#define CHASSIS_OUTPUT_STATUS_CHARGING          "charging"
#define CHASSIS_OUTPUT_STATUS_EMERGENCYSTOP     "emergencystop"
#define CHASSIS_OUTPUT_STATUS_BATTERY           "battery"

#define CHASSIS_OUTPUT_MODE_OBSTACLE            "get_obstacle_mode"
#define CHASSIS_OUTPUT_MODE_MOVE                "move_mode"

#define CHASSIS_GREET_STATUS_CODE               "greetStatusCode"

#define CHASSIS_OUTPUT_POSITION_X               "x"
#define CHASSIS_OUTPUT_POSITION_Y               "y"
#define CHASSIS_OUTPUT_POSITION_Z               "z"
#define CHASSIS_OUTPUT_POSITION_ANGLE           "angle"
#define CHASSIS_OUTPUT_POSITION_DISTANCE        "distance"

#define CHASSIS_OBSTANCLE_RANGES                "ranges"
#define CHASSIS_OBSTANCLE_RANGE_MINANGLE        "angle_min"
#define CHASSIS_OBSTANCLE_RANGE_MAXANGLE        "angle_max"
#define CHASSIS_OBSTANCLE_NEAREST_POINT         "nearest_laser_info_in_angle_range"

#define CHASSIS_OBSTACLE_NEAR                   "obstacle_near"
#define CHASSIS_OBSTACLE_FAR                    "obstacle_far"
#define CHASSIS_BATTERY_LOW                     "battery_low"
#define CHASSIS_BATTERY_HIGH                    "battery_high"

#define CHASSIS_POSITION                        "position"
#define CHASSIS_COORDINATE                      "coordinate"
#define CHASSIS_OBSTANCLE_DISTANCE_F            "chassis_obs_dis_front"
#define CHASSIS_OBSTANCLE_NEAR                  "chassis_obs_near"
#define CHASSIS_IS_CHARGING                     "is_charging"
#define CHASSIS_IS_STOP                         "is_emergencystop"

#define CHASSIS_CB_BUFFER_SIZE    1024
#define CHASSIS_CB_BUFFER_NUM     4

namespace ysos {

    DECLARE_PLUGIN_REGISTER_INTERFACE ( ChassisCallback, CallbackInterface );
    ChassisCallback::ChassisCallback ( const std::string &strClassName )
        : BaseThreadModuleCallbackImpl ( strClassName )
        , icurent_cycle_index_ ( 0 )
        , buffer_pool_ptr_ ( NULL )
        , data_ptr_ ( NULL )

        , obstacle_det_mode_ ( 1 )
        , obstacle_det_angle_ ( 18 )
        , obstacle_det_angle_near_ ( 58.0 )
        , obstacle_det_near_dis_ ( 1.0 )
        , obstacle_dis_revise_ ( 0.194 )
        , str_laser_obstacle_input_()
        , str_mobile_obstacle_input_()
    {
        buffer_pool_ptr_ = GetBufferUtility()->CreateBufferPool ( CHASSIS_CB_BUFFER_SIZE, CHASSIS_CB_BUFFER_NUM, 0 );
        data_ptr_ = GetDataInterfaceManager()->GetData ( GLOBAL_DATA_KEY );
        logger_ = GetUtility()->GetLogger ( "ysos.chassis" );
    }

    ChassisCallback::~ChassisCallback()
    {
        if ( buffer_pool_ptr_ != NULL )
        {
            buffer_pool_ptr_->Decommit();
            buffer_pool_ptr_ = NULL;
        }
    }

    int ChassisCallback::IsReady()
    {
        return YSOS_ERROR_SUCCESS;
    }

    int ChassisCallback::Initialized ( const std::string &key, const std::string &value )
    {
        YSOS_LOG_DEBUG ( key << " initialized: " << value );
        int retval = YSOS_ERROR_SUCCESS;
        if ( key == "obstacle_det_mode" )
        {
            obstacle_det_mode_ = atoi ( value.c_str() );
        }
        else if ( key == "obstacle_det_angle" )
        {
            obstacle_det_angle_ = atof ( value.c_str() );
        }
        else if ( key == "obstacle_det_angle_near" )
        {
            obstacle_det_angle_near_ = atof ( value.c_str() );
        }
        else if ( key == "obstacle_det_near_dis" )
        {
            obstacle_det_near_dis_ = atof ( value.c_str() );
        }
        else if ( key == "obstacle_dis_revise" )
        {
            obstacle_dis_revise_ = atof ( value.c_str() );
        }
        else
        {
            retval = BaseCallbackImpl::Initialized ( key, value );
        }
        return retval;
    }

    int ChassisCallback::RealCallback ( BufferInterfacePtr input_buffer, BufferInterfacePtr next_input_buffer,
                                        BufferInterfacePtr pre_output_buffer, void *context )
    {
        YSOS_LOG_DEBUG ( "*************** Entry ChassisRealCallbackCallback *************" );
        if ( NULL == context )
        {
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        BaseModuelCallbackContext *module_callback_context_ptr = reinterpret_cast<BaseModuelCallbackContext*> ( context );
        if ( NULL == module_callback_context_ptr )
        {
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        BaseModuleImpl *current_module_Interface_ptr = dynamic_cast<BaseModuleImpl*> ( module_callback_context_ptr->cur_module_ptr );
        if ( NULL == current_module_Interface_ptr )
        {
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        if ( next_input_buffer == NULL )
        {
            YSOS_LOG_WARN ( "GetBufferFromBufferPool failed" );
            return YSOS_ERROR_FAILED;
        }
        int iret = YSOS_ERROR_FAILED;
        std::string strbuffer;
        // GET_LOCALIZATION_STATUS
        strbuffer.clear();
        iret = current_module_Interface_ptr->Ioctl ( CMD_CHASSIS_GET_LOCALIZATION_STATUS, &strbuffer );
        if ( YSOS_ERROR_SUCCESS == iret )
        {
            LoactionInfo ( strbuffer, context );
        }
        else
        {
            YSOS_LOG_WARN ( "CMD_CHASSIS_GET_LOCALIZATION_STATUS Fail:" << iret );
        }
        // GET_OBSTACLE
        if ( str_mobile_obstacle_input_.empty() && str_laser_obstacle_input_.empty() )
        {
            ObstacleMode();
        }
        if ( obstacle_det_mode_ <= 1 )
        {
            strbuffer = str_laser_obstacle_input_;
            iret = current_module_Interface_ptr->Ioctl ( CMD_CHASSIS_GET_NEAREST_LASER_INFO_IN_ANGLE_RANGE, &strbuffer );
            if ( YSOS_ERROR_SUCCESS == iret )
            {
                NearestLaserInfo ( strbuffer, context );
            }
            else
            {
                YSOS_LOG_WARN ( "CMD_CHASSIS_GET_NEAREST_LASER_INFO_IN_ANGLE_RANGE Fail:" << iret );
            }
        }
        if ( obstacle_det_mode_ >= 1 )
        {
            strbuffer = str_mobile_obstacle_input_;
            iret = current_module_Interface_ptr->Ioctl ( CMD_CHASSIS_GET_OBSTACLE_DATA, &strbuffer );
            if ( YSOS_ERROR_SUCCESS == iret )
            {
                MobileObstacleInfo ( strbuffer, context );
            }
            else
            {
                YSOS_LOG_WARN ( "CMD_CHASSIS_GET_OBSTACLE_DATA Fail:" << iret );
            }
        }
        // GET_MOVE_MODE and MOVE_STATUS
        if ( YSOS_ERROR_SUCCESS == current_module_Interface_ptr->GetProperty ( PROP_CHASSIS_GET_MOVE_MODE, &next_input_buffer ) )
        {
            MoveMode ( next_input_buffer, context );
        }
        if ( ( icurent_cycle_index_ % 3 ) == 0 )
        {
            // GET_DEVICE_STATUS
            strbuffer.clear();
            iret = current_module_Interface_ptr->Ioctl ( CMD_CHASSIS_GET_DEVICE_STATUS, &strbuffer );
            if ( YSOS_ERROR_SUCCESS == iret )
            {
                DeviceStatusInfo ( strbuffer, context );
                YSOS_LOG_WARN ( "CMD_CHASSIS_GET_DEVICE_STATUS strbuffer:" << strbuffer );
            }
            else
            {
                YSOS_LOG_WARN ( "CMD_CHASSIS_GET_DEVICE_STATUS Fail:" << iret );
            }
        }
        if ( ( icurent_cycle_index_ % 30 ) == 29 )
        {
            // CHECK_ERROR_STATUS
            current_module_Interface_ptr->Ioctl ( CMD_CHASSIS_CHECK_ERROR_STATUS, NULL );
        }
        if ( ++icurent_cycle_index_ == 90 )
        {
            icurent_cycle_index_ = 0;
        }
        return YSOS_ERROR_SKIP;
    }

    void ChassisCallback::ObstacleMode()
    {
        Json::FastWriter js_writer;
        if ( obstacle_det_mode_ <= 1 )
        {
            Json::Value js_value;
            Json::Value js_unit;
            js_unit[CHASSIS_OBSTANCLE_RANGE_MINANGLE] = -obstacle_det_angle_;
            js_unit[CHASSIS_OBSTANCLE_RANGE_MAXANGLE] = obstacle_det_angle_;
            js_value[CHASSIS_OBSTANCLE_RANGES].append ( js_unit );
            str_laser_obstacle_input_ = js_writer.write ( js_value );
            str_laser_obstacle_input_ = GetUtility()->ReplaceAllDistinct ( str_laser_obstacle_input_, "\\r\\n", "" );
        }
        if ( obstacle_det_mode_ >= 1 )
        {
            Json::Value js_value;
            js_value[CHASSIS_OBSTANCLE_RANGE_MINANGLE] = -obstacle_det_angle_near_;
            js_value[CHASSIS_OBSTANCLE_RANGE_MAXANGLE] = obstacle_det_angle_near_;
            str_mobile_obstacle_input_ = js_writer.write ( js_value );
            str_mobile_obstacle_input_ = GetUtility()->ReplaceAllDistinct ( str_mobile_obstacle_input_, "\\r\\n", "" );
        }
        // init
        data_ptr_->SetData ( CHASSIS_OBSTANCLE_DISTANCE_F, "99" );
        data_ptr_->SetData ( CHASSIS_OBSTANCLE_NEAR, "180" );
    }

    /// input_buffer: {"x":3.1,"y":1.2,"angle":30.5,"quality":8}
    void ChassisCallback::LoactionInfo ( const std::string &strbuffer, void *context )
    {
        Json::Reader js_reader;
        Json::Value js_value;
        if ( !js_reader.parse ( strbuffer, js_value, false ) )
        {
            return;
        }
        int x = int ( js_value[CHASSIS_OUTPUT_POSITION_X].asDouble() + 0.5 );
        int y = int ( js_value[CHASSIS_OUTPUT_POSITION_Y].asDouble() + 0.5 );
        int angle = int ( js_value[CHASSIS_OUTPUT_POSITION_ANGLE].asDouble() );
        char pstdata[160];
        sprintf ( pstdata, "{\"messageType\":\"TEST_MAP_POS_LINK\",\"messageSource\":null,\"messageContent\":"
                  "{\"x\":%d,\"y\":%d,\"angle\":%d},\"messageTarget\":\"\"}", x, y, angle );
        data_ptr_->SetData ( CHASSIS_POSITION, pstdata );
        sprintf ( pstdata, "%d|%d|0|%d", x, y, angle );
        data_ptr_->SetData ( CHASSIS_COORDINATE, pstdata );
        if ( ( icurent_cycle_index_ % 3 ) != 1 )
        {
            // needn’t notify too frequently
            return;
        }
        js_value.clear();
        js_value[CHASSIS_OUTPUT_TIMESTAMP] = to_simple_string ( boost::posix_time::microsec_clock::local_time() );
        js_value[CHASSIS_OUTPUT_DTAT] = pstdata;
        Json::Value js_root;
        js_root[CHASSIS_OUTPUT_TYPE] = CHASSIS_OUTPUT_TYPE_VALUE_POSITION;
        js_root[CHASSIS_OUTPUT_DTAT] = js_value;
        Json::FastWriter js_writer;
        std::string ss = js_writer.write ( js_root );
        ss = GetUtility()->ReplaceAllDistinct ( ss, "\\r\\n", "" );
        #ifdef _DEBUG
        YSOS_LOG_DEBUG ( "strresult =\n\t" << ss );
        #endif
        BufferInterfacePtr next_input_buffer = GetBufferUtility()->GetBufferFromBufferPool ( buffer_pool_ptr_ );
        if ( next_input_buffer != NULL )
        {
            GetBufferUtility()->CopyStringToBuffer ( ss, next_input_buffer );
            InvokeNextCallback ( next_input_buffer, NULL, context );
        }
        else
        {
            YSOS_LOG_WARN ( "GetBufferFromBufferPool failed (loaction)" );
        }
    }

    ///< Laser mode: strbuffer:{"data":[{"distance":0.55,"angle":2.0},{"distance":0.45, "angle":20.0}]}
    void ChassisCallback::MobileObstacleInfo ( const std::string &strbuffer, void *context )
    {
        Json::Reader js_reader;
        Json::Value js_value;
        if ( !js_reader.parse ( strbuffer, js_value, false ) )
        {
            return;
        }
        double dis_front = 99.0;
        double angle_near = 180;
        js_value = js_value[CHASSIS_OUTPUT_DTAT];
        int size = js_value.size();
        for ( int i = 0; i < size; i++ )
        {
            double angle = js_value[i][CHASSIS_OUTPUT_POSITION_ANGLE].asDouble();
            double distance = js_value[i][CHASSIS_OUTPUT_POSITION_DISTANCE].asDouble();
            YSOS_LOG_DEBUG ( "mobile: " << angle << " " << distance );
            if ( distance < obstacle_dis_revise_ )
            {
                continue;
            }
            double abs_angle = abs ( angle );
            distance -= obstacle_dis_revise_;
            if ( abs_angle <= obstacle_det_angle_ )
            {
                if ( distance < dis_front )
                {
                    dis_front = distance;
                }
            }
            if ( distance < obstacle_det_near_dis_ )
            {
                if ( abs_angle < abs ( angle_near ) )
                {
                    angle_near = angle;
                }
            }
        }
        // write DataManager
        if ( data_ptr_ != NULL )
        {
            char strbuff[20];
            if ( obstacle_det_mode_ == 2 )
            {
                sprintf ( strbuff, "%.3lf", dis_front );
                YSOS_LOG_DEBUG ( "distance: " << strbuff );
                data_ptr_->SetData ( CHASSIS_OBSTANCLE_DISTANCE_F, strbuff );
            }
            sprintf ( strbuff, "%.3lf", angle_near );
            YSOS_LOG_DEBUG ( "near: " << strbuff );
            data_ptr_->SetData ( CHASSIS_OBSTANCLE_NEAR, strbuff );
        }
    }

    ///< Laser mode: strbuffer:{"distance":[1.1, 2.2, 3.3]}
    void ChassisCallback::NearestLaserInfo ( const std::string &strbuffer, void *context )
    {
        Json::Reader js_reader;
        Json::Value js_value;
        if ( !js_reader.parse ( strbuffer, js_value, false ) )
        {
            return;
        }
        js_value = js_value[CHASSIS_OUTPUT_POSITION_DISTANCE];
        double distance = js_value[ ( Json::Value::UInt ) 0].asDouble();
        YSOS_LOG_DEBUG ( "distance: " << distance );
        // write DataManager
        if ( data_ptr_ != NULL )
        {
            char strdistance[20];
            sprintf ( strdistance, "%.3lf", distance );
            data_ptr_->SetData ( CHASSIS_OBSTANCLE_DISTANCE_F, strdistance );
        }
    }

    /// input_buffer: {"move_mode":"0/1/...","task_status":"moving/finish/unreach/null"}
    void ChassisCallback::MoveMode ( BufferInterfacePtr input_buffer, void *context )
    {
        uint8_t * bufferdata = GetBufferUtility()->GetBufferData ( input_buffer );
        if ( bufferdata == NULL || bufferdata[0] == '\0' )
        {
            return;
        }
        Json::Reader js_reader;
        Json::Value js_value;
        if ( !js_reader.parse ( reinterpret_cast<char*> ( bufferdata ), js_value, false ) )
        {
            return;
        }
        std::string mode = js_value[CHASSIS_OUTPUT_MODE_MOVE].asString();
        std::string status = js_value[CHASSIS_OUTPUT_STATUS_TASK].asString();
        Json::Value js_root;
        js_value[CHASSIS_OUTPUT_TIMESTAMP] = to_simple_string ( boost::posix_time::microsec_clock::local_time() );
        if ( mode == "5" )
        {
            js_root[CHASSIS_OUTPUT_TYPE] = CHASSIS_OUTPUT_TYPE_VALUE_CHARGE_MOVING;
            js_root[CHASSIS_OUTPUT_DTAT] = js_value;
        }
        else if ( mode == "8" )
        {
            if ( status == "unreach" || status == "finish" )
            {
                js_root[CHASSIS_OUTPUT_TYPE] = CHASSIS_OUTPUT_TYPE_VALUE_MOVE_RELATIVE;
                js_root[CHASSIS_OUTPUT_DTAT] = js_value;
            }
            else
            {
                return;
            }
        }
        else if ( mode == "9" )
        {
            if ( status == "finish" )
            {
                js_root[CHASSIS_OUTPUT_TYPE] = CHASSIS_OUTPUT_TYPE_VALUE_MPA_INIT_OK;
            }
            else if ( status == "unreach" )
            {
                js_root[CHASSIS_OUTPUT_TYPE] = CHASSIS_OUTPUT_TYPE_VALUE_MPA_INIT_NG;
            }
            else
            {
                return;
            }
            js_root[CHASSIS_OUTPUT_DTAT] = js_value;
        }
        else if ( mode == "2" || mode == "6" )
        {
            js_root[CHASSIS_OUTPUT_TYPE] = CHASSIS_OUTPUT_TYPE_VALUE_MOVING;
            js_root[CHASSIS_OUTPUT_DTAT] = js_value;
        }
        else
        {
            return;
        }
        Json::FastWriter js_writer;
        std::string ss = js_writer.write ( js_root );
        ss = GetUtility()->ReplaceAllDistinct ( ss, "\\r\\n", "" );
        YSOS_LOG_DEBUG ( "strresult =\n\t" << ss );
        BufferInterfacePtr next_input_buffer = GetBufferUtility()->GetBufferFromBufferPool ( buffer_pool_ptr_ );
        if ( next_input_buffer != NULL )
        {
            GetBufferUtility()->CopyStringToBuffer ( ss, next_input_buffer );
            InvokeNextCallback ( next_input_buffer, NULL, context );
        }
        else
        {
            YSOS_LOG_WARN ( "GetBufferFromBufferPool failed (moving)" );
        }
    }

    /// strbuffer: {"battery":60,"charging":true}
    void ChassisCallback::DeviceStatusInfo ( const std::string &strbuffer, void *context )
    {
        YSOS_LOG_DEBUG ( "DeviceStatusInfo: BEGIN"  );
        Json::Reader js_reader;
        Json::Value js_value;
        if ( !js_reader.parse ( strbuffer, js_value, false ) )
        {
            return;
        }
        bool charging = js_value[CHASSIS_OUTPUT_STATUS_CHARGING].asBool();
        bool emergencystop = js_value[CHASSIS_OUTPUT_STATUS_EMERGENCYSTOP].asBool();
        YSOS_LOG_DEBUG ( "CHASSIS_IS_STOP: " << emergencystop );
        YSOS_LOG_DEBUG ( " emergencystop ? '1' : '0' ==" << ( emergencystop ? "1" : "0" ) );
        data_ptr_->SetData ( CHASSIS_IS_CHARGING, charging ? "1" : "0" );
        data_ptr_->SetData ( CHASSIS_IS_STOP, emergencystop ? "1" : "0" );
        BufferInterfacePtr next_input_buffer = GetBufferUtility()->GetBufferFromBufferPool ( buffer_pool_ptr_ );
        if ( next_input_buffer != NULL )
        {
            UINT8 *curbufferptr = NULL;
            UINT32 icurbuffersize = 0;
            next_input_buffer->GetBufferAndLength ( &curbufferptr, &icurbuffersize );
            sprintf ( ( char* ) curbufferptr, "{\"type\":\"" CHASSIS_OUTPUT_TYPE_VALUE_BATTERY "\",\"data\":\"\"}" );
            InvokeNextCallback ( next_input_buffer, NULL, context );
        }
        else
        {
            YSOS_LOG_WARN ( "GetBufferFromBufferPool failed (device)" );
        }
        YSOS_LOG_DEBUG ( "DeviceStatusInfo: END"  );
    }

}
