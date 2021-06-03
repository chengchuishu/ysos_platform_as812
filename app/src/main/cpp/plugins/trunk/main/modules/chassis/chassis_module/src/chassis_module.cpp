/**
 *@file gaussian_chassis_module.cpp
 *@brief Implement of gaussian chassis module
 *@version 0.9.0.0
 *@author Lu Min, Wang Xiaogui
 *@date Created on: 2016-12-06 17:56:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// MySelf Headers //NOLINT
#include "../include/chassis_module.h"

/// Ysos Headers //NOLINT
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/manager_declare.h"
#include "../../../../../../core/trunk/main/protect/include/os_hal_package/datainterfacemanager.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/ysos_struct.h"

#include <json/json.h>

/// Private Headers  //NOLINT
#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"

#ifdef _WIN32
  #include <Windows.h>
#else
#endif

/** 依照视频像素点的移动参数 **/
const int CMA_COLUMN_NUM = 4;
const int CMA_ROW_NUM = 8;
// 纵向像素标定值
const int CMA_COLUMN_LABEL[CMA_COLUMN_NUM] =
{345, 368, 409, 480};
// 横向像素标定值
const int CMA_ROW_LABEL[CMA_COLUMN_NUM][CMA_ROW_NUM] =
{
    {141, 209, 263, 312, 361, 412, 459, 507},
    {114, 188, 252, 311, 372, 433, 493, 554},
    {52, 145, 229, 308, 388, 468, 547, 629},
    {0, 80, 190, 302, 414, 520, 610, 640}
};
// 对应标定栅格的距离值，使用整数，单位厘米
const int CMA_POLAR_R[CMA_COLUMN_NUM][CMA_ROW_NUM] =
{
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 439, 418, 405, 400, 405, 418, 439},
    {0, 350, 323, 306, 300, 306, 323, 250},
    {0, 269, 233, 208, 200, 208, 233, 269}
};
// 对应标定栅格的角度值，使用整数，单位度
const int CMA_POLAR_ANGLE[CMA_COLUMN_NUM][CMA_ROW_NUM] =
{
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 24, 17, 9, 0, -9, -17, -24},
    {0, 31, 22, 11, 0, -11, -22, -31},
    {0, 42, 31, 17, 0, -17, -31, -42}
};

#ifndef SLEEP
    #define SLEEP(a)      (boost::this_thread::sleep_for(boost::chrono::milliseconds(a)))
#endif

#ifndef M_PI
  #define M_PI                 3.1415926535898
#endif

#define MAX_LENGTH256        256
#define ZERO_DOUBLE          (1e-15)
#define CRUISE_POINT_SIZE    3

/// 检测导航状态最大次数
#define MAX_CHASSIS_NAVIGATION_PLAN_CNT         60
#define MAX_CHASSIS_GOHOME_PLAN_CNT             1200

/// 检测距离
#define MAX_CHASSIS_CHECK_DISTANCE              0.3

/// 角度未定义
#define CHASSIS_ANGLE_UNDEFINED                 0xFFEE

/// 底盘类型
#define CHASSIS_TYPE                 "chassistype"
#define CHASSIS_TYPE_GAUSSIAN        "gaussian"

/// 心跳状态
#define CHASSISS_HEART                "heart_beat"

/// 获取障碍物模式
#define CHASSIS_OBSTACLE_MODE         "get_obstacle_mode"

#define CHASSIS_OBSTACLE_NEAR         "obstacle_near"
#define CHASSIS_OBSTACLE_FAR          "obstacle_far"
#define CHASSIS_BATTERY_LOW           "battery_low"
#define CHASSIS_BATTERY_HIGH          "battery_high"

/// 移动模式
#define CHASSISS_MOVE_MODE            "move_mode"

/// 本地巡航
#define CHASSISS_MOVE_STILL_ROTATE_NAME    "still_rotate_name"
#define CHASSISS_MOVE_STILL_ROTATE_MODE    "still_rotate_mode"
#define CHASSISS_MOVE_STILL_ROTATE_RANGE   "still_rotate_range"

#define CHASSISS_MOVE_DIRECTION       "direction"
#define CHASSISS_MOVE_ROTATE          "rotate"
#define CHASSISS_MOVE_CONTINUOUS      "continuous"
#define CHASSISS_MAPNAME              "mapname"
#define CHASSISS_POSITIONNAME         "positionname"
#define CHASSISS_MAP_INITDIRECT       "initdirect"
#define CHASSISS_GREETARER            "greetarea"
#define CHASSISS_SPEED_LINE           "linearSpeed"
#define CHASSISS_SPEED_ANGULAR        "angularSpeed"

/// 位置信息
#define CHASSISS_POS_X                "x"
#define CHASSISS_POS_Y                "y"
#define CHASSISS_POS_ANGLE            "angle"
#define CHASSISS_POS_DISTANCE         "distance"

/// 设备导航状态
#define CHASSISS_STATUS             "status"
#define CHASSISS_STATUS_FINISH      "finish"
#define CHASSISS_STATUS_UNREACH     "unreach"
#define CHASSISS_STATUS_MOVING      "moving"
#define CHASSISS_STATUS_NULL        "null"
#define CHASSISS_STATUS_WARNING      "warning"
#define CHASSISS_STATUS_WARN_OBS     "warn_obs"
#define CHASSISS_STATUS_WARN_AIM     "warn_aim"

#define ALLOC_BUFFER_IO(fun_name) \
    BufferInterfacePtr input_buffer_ptr = NULL; \
    BufferInterfacePtr output_buffer_ptr = NULL; \
    if (buffer_pool_ptr_ != NULL) { \
      buffer_pool_ptr_->GetBuffer(&input_buffer_ptr); \
      buffer_pool_ptr_->GetBuffer(&output_buffer_ptr); \
    } \
    if (input_buffer_ptr == NULL || output_buffer_ptr==NULL) { \
      YSOS_LOG_ERROR(fun_name " get buff error"); \
      return YSOS_ERROR_FAILED; \
    } \

namespace ysos {

    DECLARE_PLUGIN_REGISTER_INTERFACE ( ChassisModule, ModuleInterface );
    ChassisModule::ChassisModule ( const std::string &strClassName /* =MChassisCapture */ )
        : BaseThreadModuleImpl ( strClassName )
        , chassis_driver_ptr_ ( NULL )
        , module_callback_ptr_ ( NULL )
        , move_input_buffer_ptr_ ( NULL )
        , move_output_buffer_ptr_ ( NULL )
        , heart_input_buffer_ptr_ ( NULL )
        , heart_output_buffer_ptr_ ( NULL )

        , time_interval_ ( 250 )
        , chassis_type_ ( CHASSIS_TYPE_GAUSSISS )
        , releate_driver_name_ ( "default@GaussianChassisDriver1" )
        , releate_callback_name_ ( "default@ChassisCallback1" )

        , strconnect_addr_ ( "192.168.11.1" )
        , strmap_name_ ( "office" )
        , strmap_position_ ( "" )
        , strmap_charging_point_ ( "c1" )
        , strmap_greet_area_ ( "001" )
        , binit_direct_ ( true )

        , chassis_heart_enable_ ( true )
        , chassis_connect_status_ ( CHASSIS_STATUS_NONE )
        , connect_heart_thread_()

        , move_mutex_()
        , move_mode_ ( CHASSIS_MODE_MOVE_STOP )
        , move_cmd_chg_ ( false )
        , move_status_chg_ ( false )
        , move_cmd_param_()
        , move_status_ ( CHASSISS_STATUS_NULL )
        , move_operation_thread_()

          /// 定点巡航
        , cruise_index_ ( 0 )
        , vecpoints_cruise_()

          /// 移动定点
        , move_points_()

          /// 本地巡航
        , still_rotate_name_()
        , still_rotate_x_ ( -1 )
        , still_rotate_y_ ( -1 )
        , still_rotate_angle_ ( 0 )
        , still_rotate_range_ ( 180 )
        , still_rotate_step_ ( 30 )
        , still_rotate_step_interval_ ( 2000 )
        , still_rotate_mode_ ( "" )

        , cmd_move_step_ ( 35 )
        , cmd_move_speed_ ( 0.2 )
        , cmd_rotate_step_ ( 90 )
        , cmd_rotate_speed_ ( 0.2 )
    {
        logger_ = GetUtility()->GetLogger ( "ysos.chassis" );
    }

    int ChassisModule::Initialize ( void *param )
    {
        YSOS_LOG_DEBUG ( "ChassisModule Start Initialize" );
        //MessageBox(NULL, "debug", "debug", MB_OK);
        if ( IsInitialized() )
        {
            YSOS_LOG_DEBUG ( "ChassisModule Initialize Have Finished" );
            return YSOS_ERROR_SUCCESS;
        }
        InitalDataInfo();
        int ireturn = BaseThreadModuleImpl::Initialize ( param );
        if ( YSOS_ERROR_SUCCESS != ireturn )
        {
            YSOS_LOG_WARN ( "ChassisModule BaseThreadModuleImpl Initialize Failed" );
        }
        if ( YSOS_ERROR_SUCCESS != SetupChassisBuffer() )
        {
            YSOS_LOG_WARN ( "ChassisModule Initialize Setup Buffer Fail" );
        }
        if ( thread_data_ )
        {
            thread_data_->timeout = time_interval_;
        }
        YSOS_LOG_DEBUG ( "ChassisModule Initialize Finished And End!" );
        return ireturn;
    }

    int ChassisModule::Initialized ( const std::string &key, const std::string &value )
    {
        YSOS_LOG_DEBUG ( "ChassisModule Initialized start:" << key );
        int iret = BaseModuleImpl::Initialized ( key, value );
        if ( key.compare ( "relate_driver" ) == 0 )        ///< 关联的驱动driver的逻辑名
        {
            releate_driver_name_ = value;
            YSOS_LOG_DEBUG ( "Chassis Driver Name: " << releate_driver_name_ );
        }
        else if ( key.compare ( "relate_callback" ) == 0 ) ///< 关联的回调callback的逻辑名
        {
            releate_callback_name_ = value;
            YSOS_LOG_DEBUG ( "Chassis Callback Name:" << releate_callback_name_ );
        }
        else if ( key.compare ( "ip" ) == 0 )              ///< 连接底盘的IP
        {
            strconnect_addr_ = value;
            YSOS_LOG_DEBUG ( "Chassis IP : " << strconnect_addr_ );
        }
        else if ( key.compare ( "map_name" ) == 0 )       ///< 连接底盘的地图名称
        {
            strmap_name_ = value;
            YSOS_LOG_DEBUG ( "Chassis Map Name: " << strmap_name_ );
        }
        else if ( key.compare ( "position" ) == 0 )       ///< 连接底盘的初始位置
        {
            strmap_position_ = value;
            YSOS_LOG_DEBUG ( "Chassis Position: " << strmap_position_ );
        }
        else if ( key.compare ( "charge_point" ) == 0 )       ///< 连接底盘的充电点
        {
            strmap_charging_point_ = value;
            YSOS_LOG_DEBUG ( "Chassis Charge Position: " << strmap_charging_point_ );
        }
        else if ( key.compare ( "greet_area" ) == 0 )       ///< 连接底盘的迎宾地图
        {
            strmap_greet_area_ = value;
            YSOS_LOG_DEBUG ( "Chassis Greeter area: " << strmap_greet_area_ );
        }
        else if ( key.compare ( "chassis_heart_enable" ) == 0 )       ///< 连接底盘的迎宾地图
        {
            chassis_heart_enable_ = ( value == "1" );
            YSOS_LOG_DEBUG ( "chassis_heart_enable: " << chassis_heart_enable_ );
        }
        else if ( key.compare ( "initdirect" ) == 0 )     ///< 连接底盘是否转圈
        {
            binit_direct_ = ( value == "1" );
            YSOS_LOG_DEBUG ( "Chassis Init Direct: " << binit_direct_ );
        }
        else if ( key.compare ( "chassis_type" ) == 0 )   ///< 底盘类型
        {
            int itype = atoi ( value.c_str() );
            chassis_type_ = itype;
            if ( CHASSIS_TYPE_GAUSSISS == chassis_type_ )
            {
                YSOS_LOG_DEBUG ( "Chassis Type:GAUSSISS !" );
            }
            else if ( CHASSIS_TYPE_SILAN == chassis_type_ )
            {
                YSOS_LOG_DEBUG ( "Chassis Type:SILAN !" );
            }
            else
            {
                YSOS_LOG_ERROR ( "Chassis Type Error: " << chassis_type_ );
            }
        }
        else if ( key.compare ( "cruise_point" ) == 0 )   ///< 定点巡航点
        {
            int i = 0;
            ChassisLocation pot;
            memset ( &pot, 0, sizeof ( pot ) );
            while ( 3 == sscanf ( value.c_str() + i, "%f,%f,%f", &pot.fposx, &pot.fposy, &pot.fposangle ) )
            {
                YSOS_LOG_DEBUG ( "Get Cruise Point [" << pot.fposx << "," << pot.fposy << "," << pot.fposangle << "]" );
                vecpoints_cruise_.push_back ( pot );
                i = value.find ( '|', i );
                if ( i == std::string::npos )
                {
                    break;
                }
                i++;
            }
        }
        else if ( key.compare ( "move_points" ) == 0 )    ///< 移动定点
        {
            int i = 0;
            char name[40];
            ChassisLocation pot;
            memset ( &pot, 0, sizeof ( pot ) );
            while ( 4 == sscanf ( value.c_str() + i, "%[^:]:%f,%f,%f", name, &pot.fposx, &pot.fposy, &pot.fposangle ) )
            {
                YSOS_LOG_DEBUG ( "Get Move Point [" << name << ":" << pot.fposx << "," << pot.fposy << "," << pot.fposangle << "]" );
                move_points_[name] = pot;
                i = value.find ( '|', i );
                if ( i == -1 )
                {
                    break;
                }
                i++;
            }
        }
        else if ( key.compare ( "still_rotate_name" ) == 0 )
        {
            still_rotate_name_ = value;
            YSOS_LOG_DEBUG ( "still_rotate_name:" << still_rotate_name_ );
        }
        else if ( key.compare ( "still_rotate_range" ) == 0 )
        {
            sscanf ( value.c_str(), "%f", &still_rotate_range_ );
            YSOS_LOG_DEBUG ( "still_rotate_range:" << still_rotate_range_ );
        }
        else if ( key.compare ( "still_rotate_step" ) == 0 )
        {
            still_rotate_step_ = atoi ( value.c_str() );
            YSOS_LOG_DEBUG ( "still_rotate_step_:" << still_rotate_step_ );
        }
        else if ( key.compare ( "still_rotate_mode" ) == 0 )
        {
            still_rotate_mode_ = value;
            YSOS_LOG_DEBUG ( "still_rotate_mode_:" << still_rotate_mode_ );
        }
        else if ( key.compare ( "still_rotate_step_interval" ) == 0 )
        {
            still_rotate_step_interval_ = atoi ( value.c_str() );
            YSOS_LOG_DEBUG ( "still_rotate_step_interval_:" << still_rotate_step_interval_ );
        }
        else if ( key.compare ( "cmd_move_step" ) == 0 )
        {
            sscanf ( value.c_str(), "%f", &cmd_move_step_ );
            YSOS_LOG_DEBUG ( "cmd_move_step:" << cmd_move_step_ );
        }
        else if ( key.compare ( "cmd_move_speed" ) == 0 )
        {
            sscanf ( value.c_str(), "%f", &cmd_move_speed_ );
            YSOS_LOG_DEBUG ( "cmd_move_speed:" << cmd_move_speed_ );
        }
        else if ( key.compare ( "cmd_rotate_step" ) == 0 )
        {
            sscanf ( value.c_str(), "%f", &cmd_rotate_step_ );
            YSOS_LOG_DEBUG ( "cmd_rotate_step:" << cmd_rotate_step_ );
        }
        else if ( key.compare ( "cmd_rotate_speed" ) == 0 )
        {
            sscanf ( value.c_str(), "%f", &cmd_rotate_speed_ );
            YSOS_LOG_DEBUG ( "cmd_rotate_speed:" << cmd_rotate_speed_ );
        }
        return iret;
    }

    int ChassisModule::Ioctl ( INT32 ctrl_id, LPVOID param /* = NULL */ )
    {
        int iret = YSOS_ERROR_FAILED;
        std::string emptyStr;
        std::string *stringBuff = ( NULL == param ) ? ( &emptyStr ) : reinterpret_cast<std::string *> ( param );
        YSOS_LOG_DEBUG ( "ChassisModule::Ioctl " << ctrl_id << " " << *stringBuff );
        if ( chassis_driver_ptr_ == NULL )
        {
            YSOS_LOG_ERROR ( "ChassisModule Driver Exception!" );
            return YSOS_ERROR_FAILED;
        }
        switch ( ctrl_id )
        {
            case CMD_CHASSIS_START_MOVE_STILL:
                Set_Move_Command ( CHASSIS_MODE_MOVE_STILL, *stringBuff );
                break;
            case CMD_CHASSIS_START_MOVE_TO_POINT:
                Set_Move_Command ( CHASSIS_MODE_MOVE_POINT, *stringBuff );
                break;
            case CMD_CHASSIS_START_MOVE_CRUISE:
                if ( vecpoints_cruise_.size() < 2 )
                {
                    YSOS_LOG_ERROR ( "cruise points is less than 2." );
                    return YSOS_ERROR_FAILED;
                }
                Set_Move_Command ( CHASSIS_MODE_MOVE_CRUISE, *stringBuff );
                break;
            case CMD_CHASSIS_START_MOVE_RANDOM:
                break;
            case CMD_CHASSIS_GO_HOME:
                Set_Move_Command ( CHASSIS_MODE_MOVE_GOHOME, *stringBuff );
                break;
            case CMD_CHASSIS_START_MOVE_BY_DIRECTION:
                Set_Move_Command ( CHASSIS_MODE_MOVE_DIRECTION, *stringBuff );
                break;
            case CMD_CHASSIS_MOVE_TO_CAMERA_POINT:
                Set_Move_Command ( CHASSIS_MODE_MOVE_CAMERA_POINT, *stringBuff );
                break;
            case CMD_CHASSIS_MOVE_RELATIVELY:
                Set_Move_Command ( CHASSIS_MODE_MOVE_RELATIVELY, *stringBuff );
                break;
            case CMD_CHASSIS_STOP_MOVE:
                Set_Move_Command ( CHASSIS_MODE_MOVE_STOP, *stringBuff );
                break;
            case CMD_CHASSIS_IS_POSITION_NEAR:
                {
                    float dis = Get_Distance_From ( *stringBuff );
                    if ( dis < 0 )
                    {
                        return YSOS_ERROR_INVALID_ARGUMENTS;    // unknown
                    }
                    if ( dis > MAX_CHASSIS_CHECK_DISTANCE )
                    {
                        return YSOS_ERROR_FAILED;    // far
                    }
                    return YSOS_ERROR_SUCCESS;               // near
                }
            default:
                {
                    ALLOC_BUFFER_IO ( "Ioctl" )
                    GetBufferUtility()->CopyStringToBuffer ( *stringBuff, input_buffer_ptr );
                    iret = chassis_driver_ptr_->Ioctl ( ctrl_id, input_buffer_ptr, output_buffer_ptr );
                    if ( YSOS_ERROR_SUCCESS != iret )
                    {
                        YSOS_LOG_WARN ( "ChassisModule Ioctl Return Fail!Result:" << iret );
                        return iret;
                    }
                    if ( NULL != param )
                    {
                        UINT8* outbuff = GetBufferUtility()->GetBufferData ( output_buffer_ptr );
                        if ( NULL != outbuff )
                        {
                            *stringBuff = reinterpret_cast<const char*> ( outbuff );
                        }
                        else
                        {
                            stringBuff->clear();
                        }
                    }
                }
                break;
        }
        return YSOS_ERROR_SUCCESS;
    }

    int ChassisModule::GetProperty ( int iTypeId, void *piType )
    {
        YSOS_LOG_DEBUG ( "ChassisModule::GetProperty " << iTypeId );
        int ret = YSOS_ERROR_SUCCESS;
        switch ( iTypeId )
        {
            case PROP_CHASSIS_SYSTEM_INFO:
                {
                    assert ( NULL != chassis_driver_ptr_ );
                    ret = chassis_driver_ptr_->GetProperty ( iTypeId, piType );
                    break;
                }
            case PROP_CHASSIS_TYPE_INFO:
                {
                    assert ( NULL != chassis_driver_ptr_ );
                    ret = chassis_driver_ptr_->GetProperty ( iTypeId, piType );
                    break;
                }
            case PROP_CHASSIS_GET_MOVE_MODE:
                {
                    /// 构造输出Json
                    if ( NULL != piType )
                    {
                        BufferInterfacePtr *buffer_ptr_ptr = reinterpret_cast<BufferInterfacePtr*> ( piType );
                        ret = Get_Move_Status ( *buffer_ptr_ptr ) ? YSOS_ERROR_SUCCESS : YSOS_ERROR_FAILED;
                    }
                    break;
                }
            default:
                ret = BaseThreadModuleImpl::GetProperty ( iTypeId, piType );
                break;
        }
        return ret;
    }

    int ChassisModule::SetProperty ( int iTypeId, void *piType )
    {
        return BaseThreadModuleImpl::SetProperty ( iTypeId, piType );
    }

    int ChassisModule::SetupChassisBuffer()
    {
        move_input_buffer_ptr_ = GetBufferUtility()->CreateBuffer ( 1024 );
        move_output_buffer_ptr_ = GetBufferUtility()->CreateBuffer ( 1024 );
        heart_input_buffer_ptr_ = GetBufferUtility()->CreateBuffer ( 1024 );
        heart_output_buffer_ptr_ = GetBufferUtility()->CreateBuffer ( 1024 );
        return YSOS_ERROR_SUCCESS;
    }

    int ChassisModule::InitalDataInfo()
    {
        int return_value = YSOS_ERROR_INVALID_ARGUMENTS;
        if ( module_data_info_ )
        {
            module_data_info_->module_type = PROP_SOURCE;
            module_data_info_->in_datatypes = DTSTR_NULL;
            module_data_info_->out_datatypes = DTSTR_TEXT_JSON;
            module_data_info_->is_self_allocator = true;
            module_data_info_->prefix_length = 0;
            return_value = YSOS_ERROR_SUCCESS;
        }
        return return_value;
    }

    int ChassisModule::RealOpen ( LPVOID param /* = NULL */ )
    {
        YSOS_LOG_DEBUG ( "ChassisModule RealOpen Function Start!" );
        chassis_driver_ptr_ = GetDriverInterfaceManager()->FindInterface ( releate_driver_name_ );
        if ( NULL == chassis_driver_ptr_ )
        {
            YSOS_LOG_ERROR ( "ChassisModule Find Driver Fail" );
            return YSOS_ERROR_FAILED;
        }
        YSOS_LOG_DEBUG ( " ChassisModule Find Driver Success:" << releate_driver_name_ );
        data_ptr_ = GetDataInterfaceManager()->GetData ( GLOBAL_DATA_KEY );
        if ( !data_ptr_ )
        {
            YSOS_LOG_DEBUG ( "get data_ptr_ failed" );
            return YSOS_ERROR_FAILED;
        }
        /// 获取底盘类型
        if ( YSOS_ERROR_SUCCESS != Get_Chassis_Type() )
        {
            YSOS_LOG_WARN ( "ChassisModule RealOpen Function Get Chassis Type Error!" );
        }
        /// 连接底盘
        if ( YSOS_ERROR_SUCCESS != chassis_driver_ptr_->Open ( NULL ) )
        {
            YSOS_LOG_WARN ( "ChassisModule RealOpen Function Open Fail!" );
            return YSOS_ERROR_FAILED;
        }
        YSOS_LOG_DEBUG ( " ChassisModule RealOpen Function Open Success!" );
        /// ready to post event
        SetStatusEventCallback ( chassis_driver_ptr_ );
        // start connect heart thread
        connect_heart_thread_ = boost::thread ( boost::bind ( Connect_Heart_Thread, this ) );
        // start move thread
        move_operation_thread_ = boost::thread ( boost::bind ( Move_Thread, this ) );
        int iret = BaseThreadModuleImpl::RealOpen ( param );
        if ( YSOS_ERROR_SUCCESS != iret )
        {
            YSOS_LOG_WARN ( "Base ChassisModule::RealOpen Fail:" << iret );
            return iret;
        }
        YSOS_LOG_DEBUG ( "ChassisModule RealOpen Function Success And End!" );
        return YSOS_ERROR_SUCCESS;
    }

    int ChassisModule::RealPause()
    {
        YSOS_LOG_DEBUG ( "ChassisModule::RealPause" );
        return YSOS_ERROR_SUCCESS;
    }

    int ChassisModule::RealRun()
    {
        YSOS_LOG_DEBUG ( "ChassisModule::RealRun" );
        return YSOS_ERROR_SUCCESS;
    }

    int ChassisModule::RealStop()
    {
        YSOS_LOG_DEBUG ( "ChassisModule::RealStop" );
        return YSOS_ERROR_SUCCESS;
    }

    int ChassisModule::RealClose()
    {
        YSOS_LOG_DEBUG ( "ChassisModule::RealClose" );
        // stop move thread
        move_operation_thread_.interrupt();
        move_operation_thread_.join();
        Stop_Move();
        // stop connect heart thread
        connect_heart_thread_.interrupt();
        connect_heart_thread_.join();
        if ( chassis_driver_ptr_ )
        {
            chassis_driver_ptr_->Close();
        }
        return BaseThreadModuleImpl::RealClose();
    }

    /////////////////////////////////////////////////////////////////////////////
    ///////////////////////////Module Logic /////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////
    int ChassisModule::Move_Thread ( LPVOID lpParam )
    {
        ChassisModule *chassismodule_ptr = ( ChassisModule * ) lpParam;
        if ( NULL == chassismodule_ptr )
        {
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        YSOS_LOG_DEBUG_CUSTOM ( chassismodule_ptr->logger_, "Move_Thread start running" );
        int lastmovemode = CHASSIS_MODE_MOVE_STOP;
        int movemode = CHASSIS_MODE_MOVE_STOP;
        std::string moveparam;
        while ( true )
        {
            if ( !chassismodule_ptr->Get_Move_Command ( movemode, moveparam ) )
            {
                SLEEP ( 100 );
                continue;
            }
            YSOS_LOG_DEBUG_CUSTOM ( chassismodule_ptr->logger_, "Move_Mode Is Changed " << lastmovemode << " -> " << movemode );
            boost::this_thread::interruption_point();
            lastmovemode = movemode;
            // start the new command
            switch ( movemode )
            {
                case CHASSIS_MODE_MOVE_STILL:
                    chassismodule_ptr->Move_Still ( moveparam );
                    break;
                case CHASSIS_MODE_MOVE_POINT:
                    {
                        int isReach = YSOS_ERROR_FAILED;
                        ChassisLocation aim;
                        if ( chassismodule_ptr->Get_Move_Point ( moveparam, aim ) )
                        {
                            isReach = chassismodule_ptr->Move_To_Point ( aim );
                        }
                        // save the result
                        if ( chassismodule_ptr->Is_Move_Mode ( CHASSIS_MODE_MOVE_POINT ) )
                        {
                            if ( YSOS_ERROR_SUCCESS == isReach )
                            {
                                chassismodule_ptr->Set_Move_Status ( CHASSISS_STATUS_FINISH );
                                YSOS_LOG_DEBUG_CUSTOM ( chassismodule_ptr->logger_, "Move_To_Point " CHASSISS_STATUS_FINISH );
                            }
                            else
                            {
                                // SetProperty
                                StatusEventInfo event_info;
                                event_info.code = "000100000802000004";
                                event_info.detail = "failed to move to position";
                                chassismodule_ptr->SetProperty ( PROP_STATUS_EVENT, &event_info );
                                chassismodule_ptr->Set_Move_Status ( CHASSISS_STATUS_UNREACH );
                                YSOS_LOG_DEBUG_CUSTOM ( chassismodule_ptr->logger_, "Move_To_Point " CHASSISS_STATUS_UNREACH );
                            }
                        }
                        break;
                    }
                case CHASSIS_MODE_MOVE_CRUISE:
                    chassismodule_ptr->Move_Cruise();
                    break;
                case CHASSIS_MODE_MOVE_RANDOM:
                    break;
                case CHASSIS_MODE_MOVE_GOHOME:
                    {
                        int isReach = chassismodule_ptr->Go_Home();
                        // save the result
                        if ( chassismodule_ptr->Is_Move_Mode ( CHASSIS_MODE_MOVE_GOHOME ) )
                        {
                            if ( YSOS_ERROR_SUCCESS == isReach )
                            {
                                chassismodule_ptr->Set_Move_Status ( CHASSISS_STATUS_FINISH );
                                YSOS_LOG_DEBUG_CUSTOM ( chassismodule_ptr->logger_, "Go_Home " CHASSISS_STATUS_FINISH );
                            }
                            else
                            {
                                // SetProperty
                                StatusEventInfo event_info;
                                event_info.code = "000100000802000003";
                                event_info.detail = "failed to go home";
                                chassismodule_ptr->SetProperty ( PROP_STATUS_EVENT, &event_info );
                                chassismodule_ptr->Set_Move_Status ( CHASSISS_STATUS_UNREACH );
                                YSOS_LOG_DEBUG_CUSTOM ( chassismodule_ptr->logger_, "Go_Home " CHASSISS_STATUS_UNREACH );
                            }
                        }
                        break;
                    }
                case CHASSIS_MODE_MOVE_DIRECTION:
                    chassismodule_ptr->Move_Direction ( moveparam );
                    break;
                case CHASSIS_MODE_MOVE_CAMERA_POINT:
                    {
                        ChassisLocation aim;
                        if ( chassismodule_ptr->Get_Move_Camera_Point ( moveparam, aim ) )
                        {
                            chassismodule_ptr->Move_To_Point ( aim );
                        }
                        break;
                    }
                case CHASSIS_MODE_MOVE_RELATIVELY:
                    {
                        int isReach = YSOS_ERROR_FAILED;
                        ChassisLocation aim;
                        if ( chassismodule_ptr->Get_Move_Relative_Point ( moveparam, aim ) )
                        {
                            isReach = chassismodule_ptr->Move_To_Point ( aim, false );
                        }
                        // save the result
                        if ( chassismodule_ptr->Is_Move_Mode ( CHASSIS_MODE_MOVE_RELATIVELY ) )
                        {
                            if ( YSOS_ERROR_SUCCESS == isReach )
                            {
                                chassismodule_ptr->Set_Move_Status ( CHASSISS_STATUS_FINISH );
                                YSOS_LOG_DEBUG_CUSTOM ( chassismodule_ptr->logger_, "move relatively " CHASSISS_STATUS_FINISH );
                            }
                            else
                            {
                                chassismodule_ptr->Set_Move_Status ( CHASSISS_STATUS_UNREACH );
                                YSOS_LOG_DEBUG_CUSTOM ( chassismodule_ptr->logger_, "move relatively " CHASSISS_STATUS_UNREACH );
                            }
                        }
                        break;
                    }
                default: // CHASSIS_MODE_MOVE_STOP
                    chassismodule_ptr->Stop_Move();
                    break;
            } // end switch
        } // end while
        YSOS_LOG_DEBUG_CUSTOM ( chassismodule_ptr->logger_, "Move_Thread end running" );
        return YSOS_ERROR_SUCCESS;
    }

    std::string ChassisModule::Get_Current_Navigation_Status ( const UINT8 *input_buffer_ptr )
    {
        Json::Reader js_reader;
        Json::Value js_value;
        if ( js_reader.parse ( reinterpret_cast<const char*> ( input_buffer_ptr ), js_value, false ) )
        {
            return js_value[CHASSISS_STATUS].asString();
        }
        return CHASSISS_STATUS_NULL;
    }

    std::string ChassisModule::Make_MoveTo_Point ( float fposx, float fposy, float fangle )
    {
        Json::FastWriter js_writer;
        Json::Value js_value;
        js_value[CHASSISS_POS_X] = fposx;
        js_value[CHASSISS_POS_Y] = fposy;
        if ( fangle != CHASSIS_ANGLE_UNDEFINED )
        {
            js_value[CHASSISS_POS_ANGLE] = fangle;
        }
        std::string strposjson = js_writer.write ( js_value );
        strposjson = GetUtility()->ReplaceAllDistinct ( strposjson, "\\r\\n", "" );
        #ifdef _DEBUG
        YSOS_LOG_DEBUG ( "Make_MoveTo_Point:" << strposjson );
        #endif
        return strposjson;
    }

    std::string ChassisModule::Make_Rotate_Position ( float fangle )
    {
        Json::FastWriter js_writer;
        Json::Value js_value;
        js_value[CHASSISS_MOVE_ROTATE] = fangle;
        std::string strposjson = js_writer.write ( js_value );
        strposjson = GetUtility()->ReplaceAllDistinct ( strposjson, "\\r\\n", "" );
        #ifdef _DEBUG
        YSOS_LOG_DEBUG ( "Make_Rotate_Position:" << strposjson );
        #endif
        return strposjson;
    }

    std::string ChassisModule::Make_Moveing_Cmd ( float linearSpeed, float angularSpeed )
    {
        Json::FastWriter js_writer;
        Json::Value js_value;
        js_value[CHASSISS_SPEED_LINE] = linearSpeed;
        js_value[CHASSISS_SPEED_ANGULAR] = angularSpeed;
        std::string strposjson = js_writer.write ( js_value );
        strposjson = GetUtility()->ReplaceAllDistinct ( strposjson, "\\r\\n", "" );
        #ifdef _DEBUG
        YSOS_LOG_DEBUG ( "Make_Moveing_Cmd:" << strposjson );
        #endif
        return strposjson;
    }

    bool ChassisModule::Get_Current_Point_Pos ( const UINT8 *input_buffer_ptr, float &fposx, float &fposy, float &fangle )
    {
        Json::Reader js_reader;
        Json::Value js_value;
        if ( !js_reader.parse ( reinterpret_cast<const char*> ( input_buffer_ptr ), js_value, false ) )
        {
            YSOS_LOG_ERROR ( "point info json error" );
            return false;
        }
        fposx = float ( js_value[CHASSISS_POS_X].asDouble() );
        fposy = float ( js_value[CHASSISS_POS_Y].asDouble() );
        fangle = float ( js_value[CHASSISS_POS_ANGLE].asDouble() );
        #ifdef _DEBUG
        YSOS_LOG_DEBUG ( "Current Position:[" << fposx << "," << fposy << "," << fangle << "]" );
        #endif
        return true;
    }

    float ChassisModule::Calculate_Distance_To_ObjPoint ( float fcurposx, float fcurposy, float fobjposx, float fobjposy )
    {
        double fdistance_x = fcurposx - fobjposx;
        double fdistance_y = fcurposy - fobjposy;
        double fdistance = sqrt ( fdistance_x * fdistance_x + fdistance_y * fdistance_y );
        /// 高仙是一个单位表示5厘米,所以这里需要转一下
        if ( CHASSIS_TYPE_GAUSSISS == chassis_type_ )
        {
            fdistance = fdistance * 5.0 / 100.0;
        }
        else if ( CHASSIS_TYPE_SILAN == chassis_type_ )
        {
        }
        YSOS_LOG_DEBUG ( "Distance :" << fdistance );
        return static_cast<float> ( fdistance );
    }

    float ChassisModule::Get_Distance_From ( const std::string &aim )
    {
        struct ChassisLocation point;
        if ( !Get_Move_Point ( aim, point ) )
        {
            return -1;
        }
        // get the current point
        ALLOC_BUFFER_IO ( "Get_Distance_From" )
        float posx, posy, angle;
        if ( YSOS_ERROR_SUCCESS != chassis_driver_ptr_->Ioctl ( CMD_CHASSIS_GET_LOCALIZATION_STATUS, input_buffer_ptr, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "CMD_CHASSIS_GET_LOCALIZATION_STATUS failed!" );
            return -1;
        }
        if ( !Get_Current_Point_Pos ( GetBufferUtility()->GetBufferData ( output_buffer_ptr ), posx, posy, angle ) )
        {
            return -1;
        }
        return Calculate_Distance_To_ObjPoint ( point.fposx, point.fposy, posx, posy );
    }

    std::string ChassisModule::Make_Gohome_Json_String()
    {
        Json::FastWriter js_writer;
        Json::Value js_value;
        js_value[CHASSISS_MAPNAME] = strmap_name_;
        js_value[CHASSISS_POSITIONNAME] = strmap_charging_point_;
        std::string strjsonresult = js_writer.write ( js_value );
        strjsonresult = GetUtility()->ReplaceAllDistinct ( strjsonresult, "\\r\\n", "" );
        #ifdef _DEBUG
        YSOS_LOG_DEBUG ( "Chassis Go Home Info:" << strjsonresult );
        #endif
        return strjsonresult;
    }

    std::string ChassisModule::Make_Greet_Json_String()
    {
        Json::FastWriter js_writer;
        Json::Value js_value;
        js_value[CHASSISS_MAPNAME] = strmap_name_;
        js_value[CHASSISS_GREETARER] = strmap_greet_area_;
        std::string strjsonresult = js_writer.write ( js_value );
        strjsonresult = GetUtility()->ReplaceAllDistinct ( strjsonresult, "\\r\\n", "" );
        #ifdef _DEBUG
        YSOS_LOG_DEBUG ( "Chassis Greet Info:" << strjsonresult );
        #endif
        return strjsonresult;
    }

    int ChassisModule::Get_Need_Rotate ( int& direction, int& angle )
    {
        //+,-
        if ( still_rotate_mode_ == "+" || still_rotate_mode_ == "-" )
        {
            return direction * still_rotate_step_;
        }
        //+-+,-+-
        int range_down = int ( still_rotate_angle_ - still_rotate_range_ / 2 );
        if ( range_down <= -180 )
        {
            range_down += 360;
        }
        int range_up = int ( still_rotate_angle_ + still_rotate_range_ / 2 );
        if ( range_up > 180 )
        {
            range_down -= 360;
        }
        bool inRange;
        if ( range_down <= range_up )
        {
            // case: not contain 180 degrees point
            inRange = ( angle >= range_down && angle <= range_up );
        }
        else
        {
            // case: contain 180 degrees point
            inRange = ( angle >= range_down || angle <= range_up );
        }
        int need = 0;
        if ( inRange )
        {
            // case: angle is in the rotate range
            if ( direction == 1 )
            {
                // to anti-clockwise
                int toup;
                if ( ( toup = range_up - angle ) < 0 )
                {
                    toup += 360;
                }
                if ( toup > still_rotate_step_ )
                {
                    need = still_rotate_step_;
                    if ( ( angle += still_rotate_step_ ) > 180 )
                    {
                        angle -= 360;
                    }
                }
                else
                {
                    need = toup;
                    angle = range_up;
                    direction = -1;
                    still_rotate_mode_ = "-+-";
                }
            }
            else
            {
                // to clockwise
                int todown;
                if ( ( todown = angle - range_down ) < 0 )
                {
                    todown += 360;
                }
                if ( todown > still_rotate_step_ )
                {
                    need = -still_rotate_step_;
                    if ( ( angle -= still_rotate_step_ ) <= -180 )
                    {
                        angle += 360;
                    }
                }
                else
                {
                    need = -todown;
                    angle = range_down;
                    direction = 1;
                    still_rotate_mode_ = "+-+";
                }
            }
        }
        else
        {
            // case: angle is out of the rotate range
            int toup, todown;
            if ( ( todown = range_down - angle ) < 0 )
            {
                todown += 360;
            }
            if ( ( toup = angle - range_up ) < 0 )
            {
                toup += 360;
            }
            if ( todown <= toup )
            {
                need = todown;
                angle = range_down;
                direction = 1;
                still_rotate_mode_ = "+-+";
            }
            else
            {
                need = -toup;
                angle = range_up;
                direction = -1;
                still_rotate_mode_ = "-+-";
            }
        }
        return need;
    }

    /************************************************************************/
    /* strsource:
    {
      "still_rotate_name":"p1",   <!-- c1 is also OK -->
      "still_rotate_mode":"+-+",
      "still_rotate_range":"180"
    }
      All keys are optional.
    */
    /************************************************************************/
    int ChassisModule::Get_Move_Still ( const std::string &strsource )
    {
        do
        {
            if ( strsource.length() == 0 )
            {
                break;
            }
            Json::Reader js_reader;
            Json::Value js_value;
            if ( !js_reader.parse ( strsource, js_value, false ) )
            {
                YSOS_LOG_ERROR ( "json error" );
                break;
            }
            if ( js_value.isMember ( CHASSISS_MOVE_STILL_ROTATE_NAME ) )
            {
                still_rotate_name_ = js_value[CHASSISS_MOVE_STILL_ROTATE_NAME].asString();
                YSOS_LOG_DEBUG ( "get point from name:" << still_rotate_name_ );
            }
            if ( js_value.isMember ( CHASSISS_MOVE_STILL_ROTATE_MODE ) )
            {
                still_rotate_mode_ = js_value[CHASSISS_MOVE_STILL_ROTATE_MODE].asString();
                YSOS_LOG_DEBUG ( "update rotate mode " << still_rotate_mode_ );
            }
            if ( js_value.isMember ( CHASSISS_MOVE_STILL_ROTATE_RANGE ) )
            {
                std::string stillRotateRange = js_value[CHASSISS_MOVE_STILL_ROTATE_RANGE].asString();
                sscanf ( stillRotateRange.c_str(), "%f", &still_rotate_range_ );
                YSOS_LOG_DEBUG ( "update rotate range " << still_rotate_range_ );
            }
        } while ( 0 );
        if ( still_rotate_name_ == "c1" )
        {
            return YSOS_ERROR_PASS;
        }
        std::map<std::string, struct ChassisLocation>::iterator it = move_points_.find ( still_rotate_name_ );
        if ( it == move_points_.end() )
        {
            YSOS_LOG_ERROR ( "still_rotate_name is not found" );
        }
        else
        {
            still_rotate_x_ = it->second.fposx;
            still_rotate_y_ = it->second.fposy;
            still_rotate_angle_ = it->second.fposangle;
        }
        return YSOS_ERROR_SUCCESS;
    }

    int ChassisModule::Move_Still ( const std::string &strsource )
    {
        if ( chassis_driver_ptr_ == NULL )
        {
            YSOS_LOG_ERROR ( "Driver Pointer Exception" );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        if ( move_input_buffer_ptr_ == NULL || move_output_buffer_ptr_ == NULL )
        {
            YSOS_LOG_ERROR ( "Buffer Exception" );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        std::string electcharge = "0";
        GetDataInterfaceManager()->GetData ( GLOBAL_DATA_KEY )->GetData ( "electcharge", electcharge );
        if ( electcharge == "1" )
        {
            YSOS_LOG_ERROR ( "The Battery is Charging!!!" );
            return YSOS_ERROR_FAILED;
        }
        int rst = Get_Move_Still ( strsource );
        if ( rst == YSOS_ERROR_PASS )
        {
            std::string charge = "0";
            GetDataInterfaceManager()->GetData ( GLOBAL_DATA_KEY )->GetData ( "is_charging", charge );
            if ( charge == "0" )
            {
                Go_Home();
            }
            return YSOS_ERROR_SUCCESS;
        }
        // get the current point
        float posx, posy, angle;
        if ( YSOS_ERROR_SUCCESS != chassis_driver_ptr_->Ioctl ( CMD_CHASSIS_GET_LOCALIZATION_STATUS, move_input_buffer_ptr_, move_output_buffer_ptr_ ) )
        {
            YSOS_LOG_WARN ( "CMD_CHASSIS_GET_LOCALIZATION_STATUS (1) failed!" );
            return YSOS_ERROR_FAILED;
        }
        if ( !Get_Current_Point_Pos ( GetBufferUtility()->GetBufferData ( move_output_buffer_ptr_ ), posx, posy, angle ) )
        {
            return YSOS_ERROR_FAILED;
        }
        if ( still_rotate_x_ < 0 || still_rotate_y_ < 0 )
        {
            still_rotate_x_ = posx;
            still_rotate_y_ = posy;
            still_rotate_angle_ = angle;
        }
        else
        {
            float fdistance = Calculate_Distance_To_ObjPoint ( posx, posy, still_rotate_x_, still_rotate_y_ );
            if ( fdistance > MAX_CHASSIS_CHECK_DISTANCE )
            {
                ChassisLocation point;
                point.fposx = still_rotate_x_;
                point.fposy = still_rotate_y_;
                point.fposangle = still_rotate_angle_;
                if ( YSOS_ERROR_SUCCESS != Move_To_Point ( point ) )
                {
                    YSOS_LOG_WARN ( "Move_To_Point Failed!" );
                }
                if ( Is_Move_Command_Updated() )
                {
                    return YSOS_ERROR_SUCCESS;
                }
                // update the current angle
                if ( YSOS_ERROR_SUCCESS != chassis_driver_ptr_->Ioctl ( CMD_CHASSIS_GET_LOCALIZATION_STATUS, move_input_buffer_ptr_, move_output_buffer_ptr_ ) )
                {
                    YSOS_LOG_WARN ( "CMD_CHASSIS_GET_LOCALIZATION_STATUS (2) failed!" );
                    return YSOS_ERROR_FAILED;
                }
                if ( !Get_Current_Point_Pos ( GetBufferUtility()->GetBufferData ( move_output_buffer_ptr_ ), posx, posy, angle ) )
                {
                    return YSOS_ERROR_FAILED;
                }
            }
        }
        if ( still_rotate_mode_.size() == 0 || still_rotate_mode_[0] != '+' && still_rotate_mode_[0] != '-' )
        {
            YSOS_LOG_WARN ( "still_rotate_mode_ is unknown:" << still_rotate_mode_ );
            if ( still_rotate_angle_ != angle )
            {
                float rotate = still_rotate_angle_ - angle;
                if ( rotate >= 180 )
                {
                    rotate -= 360;
                }
                else if ( rotate <= -180 )
                {
                    rotate += 360;
                }
                GetBufferUtility()->CopyStringToBuffer ( Make_Rotate_Position ( rotate ), move_input_buffer_ptr_ );
                chassis_driver_ptr_->Ioctl ( CMD_CHASSIS_START_ROTATE, move_input_buffer_ptr_, move_output_buffer_ptr_ );
            }
            return YSOS_ERROR_SUCCESS;
        }
        int current_direction = ( still_rotate_mode_[0] == '+' ? 1 : -1 );
        int current_angle = static_cast<int> ( ( angle >= 0 ) ? ( angle + 0.5 ) : ( angle - 0.5 ) );
        int need_rotate = 0;
        while ( !Is_Move_Command_Updated() )
        {
            // rotate a step
            need_rotate = Get_Need_Rotate ( current_direction, current_angle );
            YSOS_LOG_DEBUG ( "Rotate a step: " << need_rotate << " " << current_angle );
            GetBufferUtility()->CopyStringToBuffer ( Make_Rotate_Position ( ( float ) need_rotate ), move_input_buffer_ptr_ );
            if ( YSOS_ERROR_SUCCESS != chassis_driver_ptr_->Ioctl ( CMD_CHASSIS_START_ROTATE, move_input_buffer_ptr_, move_output_buffer_ptr_ ) )
            {
                YSOS_LOG_WARN ( "Rotate a step fail!" );
                continue;
            }
            // waiting
            Wait_For_Rotating();
            //sleep
            int count_time = still_rotate_step_interval_ / 100;
            for ( int i = 0; i < count_time; i++ )
            {
                if ( Is_Move_Command_Updated() )
                {
                    return YSOS_ERROR_SUCCESS;
                }
                SLEEP ( 100 );
            }
        }
        return YSOS_ERROR_SUCCESS;
    }

    /************************************************************************/
    /* strsource:
    {
      "positionname":"p1"
    }
    or
    {
      "x":1.4,"y":1.1,"angle":2.0
    }
      The angle is optional.
    */
    /************************************************************************/
    bool ChassisModule::Get_Move_Point ( const std::string &strsource, struct ChassisLocation &point )
    {
        Json::Reader js_reader;
        Json::Value js_value;
        if ( !js_reader.parse ( strsource, js_value, false ) )
        {
            YSOS_LOG_ERROR ( "point info json error" );
            return false;
        }
        if ( js_value.isMember ( CHASSISS_POSITIONNAME ) )
        {
            std::string posname = js_value[CHASSISS_POSITIONNAME].asString();
            #ifdef _DEBUG
            YSOS_LOG_DEBUG ( "get point from name:" << posname );
            #endif
            std::map<std::string, struct ChassisLocation>::iterator it;
            it = move_points_.find ( posname );
            if ( it == move_points_.end() )
            {
                YSOS_LOG_ERROR ( "position name not found" );
                return false;
            }
            point = it->second;
        }
        else
        {
            point.fposx = float ( js_value[CHASSISS_POS_X].asDouble() );
            point.fposy = float ( js_value[CHASSISS_POS_Y].asDouble() );
            if ( js_value.isMember ( CHASSISS_POS_ANGLE ) )
            {
                point.fposangle = float ( js_value[CHASSISS_POS_ANGLE].asDouble() );
            }
            else
            {
                point.fposangle = CHASSIS_ANGLE_UNDEFINED;
            }
        }
        return true;
    }

    int ChassisModule::Move_To_Point ( const ChassisLocation &struobjpos, bool persist )
    {
        if ( chassis_driver_ptr_ == NULL )
        {
            YSOS_LOG_ERROR ( "Driver Pointer Exception" );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        if ( move_input_buffer_ptr_ == NULL || move_output_buffer_ptr_ == NULL )
        {
            YSOS_LOG_ERROR ( "Buffer Exception" );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        std::string electcharge = "0";
        GetDataInterfaceManager()->GetData ( GLOBAL_DATA_KEY )->GetData ( "electcharge", electcharge );
        if ( electcharge == "1" )
        {
            YSOS_LOG_ERROR ( "The Battery is Charging!!!" );
            return YSOS_ERROR_FAILED;
        }
        GetBufferUtility()->CopyStringToBuffer (
            Make_MoveTo_Point ( struobjpos.fposx, struobjpos.fposy, struobjpos.fposangle ), move_input_buffer_ptr_ );
        int isReach = YSOS_ERROR_FAILED;
        do
        {
            // move to a point
            if ( YSOS_ERROR_SUCCESS != chassis_driver_ptr_->Ioctl ( CMD_CHASSIS_START_MOVE_TO, move_input_buffer_ptr_, move_output_buffer_ptr_ ) )
            {
                YSOS_LOG_WARN ( "Move To Object Point Fail" );
                break;
            }
            Set_Move_Status ( CHASSISS_STATUS_MOVING );
            // waiting
            isReach = Wait_For_Moving ( MAX_CHASSIS_NAVIGATION_PLAN_CNT, persist );
            if ( Is_Move_Command_Updated() )
            {
                break;
            }
            // if SILAN, need to rotate
            if ( CHASSIS_TYPE_SILAN == chassis_type_ && CHASSIS_ANGLE_UNDEFINED != struobjpos.fposangle )
            {
                // rotate to
                GetBufferUtility()->CopyStringToBuffer ( Make_Rotate_Position ( struobjpos.fposangle ), move_input_buffer_ptr_ );
                if ( YSOS_ERROR_SUCCESS != chassis_driver_ptr_->Ioctl ( CMD_CHASSIS_START_ROTATE_TO, move_input_buffer_ptr_, move_output_buffer_ptr_ ) )
                {
                    YSOS_LOG_WARN ( "Rotate To Object Position Fail!Now Start Next Point!" );
                    break;
                }
                // waiting
                Wait_For_Rotating();
                if ( Is_Move_Command_Updated() )
                {
                    break;
                }
            }
            // if UNREACH, check distance
            if ( isReach != YSOS_ERROR_SUCCESS )
            {
                if ( YSOS_ERROR_SUCCESS != chassis_driver_ptr_->Ioctl ( CMD_CHASSIS_GET_LOCALIZATION_STATUS, move_input_buffer_ptr_, move_output_buffer_ptr_ ) )
                {
                    YSOS_LOG_DEBUG ( "Get Position Fail! Now Start Next Point!" );
                    break;
                }
                float posx, posy, angle;
                if ( Get_Current_Point_Pos ( GetBufferUtility()->GetBufferData ( move_output_buffer_ptr_ ), posx, posy, angle ) )
                {
                    float fdistance = Calculate_Distance_To_ObjPoint ( posx, posy, struobjpos.fposx, struobjpos.fposy );
                    if ( fdistance <= MAX_CHASSIS_CHECK_DISTANCE )
                    {
                        isReach = YSOS_ERROR_SUCCESS;
                    }
                }
            }
        } while ( 0 );
        return isReach;
    }

    int ChassisModule::Move_Cruise()
    {
        std::string electcharge = "0";
        GetDataInterfaceManager()->GetData ( GLOBAL_DATA_KEY )->GetData ( "electcharge", electcharge );
        if ( electcharge == "1" )
        {
            YSOS_LOG_ERROR ( "The Battery is Charging!!!" );
            return YSOS_ERROR_FAILED;
        }
        while ( true )
        {
            YSOS_LOG_DEBUG ( "Move_Cruise " << cruise_index_ );
            Move_To_Point ( vecpoints_cruise_[cruise_index_] );
            if ( Is_Move_Command_Updated() )
            {
                break;
            }
            cruise_index_ = ( cruise_index_ + 1 == vecpoints_cruise_.size() ) ? 0 : ( cruise_index_ + 1 );
            SLEEP ( 100 );
        }
        return YSOS_ERROR_SUCCESS;
    }

    int ChassisModule::Go_Home()
    {
        if ( chassis_driver_ptr_ == NULL )
        {
            YSOS_LOG_ERROR ( "Driver Pointer Exception" );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        if ( move_input_buffer_ptr_ == NULL || move_output_buffer_ptr_ == NULL )
        {
            YSOS_LOG_ERROR ( "Buffer Exception" );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        int isReach = YSOS_ERROR_FAILED;
        do
        {
            // if GAUSSISS, set param
            if ( CHASSIS_TYPE_GAUSSISS == chassis_type_ )
            {
                GetBufferUtility()->CopyStringToBuffer ( Make_Gohome_Json_String(), move_input_buffer_ptr_ );
            }
            // go home
            if ( YSOS_ERROR_SUCCESS != chassis_driver_ptr_->Ioctl ( CMD_CHASSIS_GO_HOME, move_input_buffer_ptr_, move_output_buffer_ptr_ ) )
            {
                YSOS_LOG_WARN ( "Go Home Failed!" );
                break;
            }
            Set_Move_Status ( CHASSISS_STATUS_MOVING );
            // waiting
            isReach = Wait_For_Moving ( MAX_CHASSIS_GOHOME_PLAN_CNT );
        } while ( 0 );
        return isReach;
    }

    /************************************************************************/
    /* strsource:
    { "direction": "+/-", "continuous": true/false }
      or
    { "direction": "35/-35" }
      or
    { "rotate": "+/-", "continuous": true/false }
      or
    { "rotate": "35/-35" }
    */
    /************************************************************************/
    int ChassisModule::Get_Move_Direction ( const std::string &strsource, float &value )
    {
        Json::Reader js_reader;
        Json::Value js_value;
        if ( !js_reader.parse ( strsource, js_value, false ) )
        {
            YSOS_LOG_ERROR ( "move direction json error" );
            return -1;
        }
        bool continuous = false;
        if ( js_value.isMember ( CHASSISS_MOVE_CONTINUOUS ) )
        {
            continuous = js_value[CHASSISS_MOVE_CONTINUOUS].asBool();
        }
        if ( js_value.isMember ( CHASSISS_MOVE_DIRECTION ) )
        {
            std::string strval = js_value[CHASSISS_MOVE_DIRECTION].asString();
            if ( strval == "+" )
            {
                if ( !continuous )
                {
                    value = cmd_move_step_;
                    return 0; // move one step
                }
                else
                {
                    value = cmd_move_speed_;
                    return 1; // move continuously
                }
            }
            if ( strval == "-" )
            {
                if ( !continuous )
                {
                    value = -cmd_move_step_;
                    return 0; // move one step
                }
                else
                {
                    value = -cmd_move_speed_;
                    return 1; // move continuously
                }
            }
            sscanf ( strval.c_str(), "%f", &value );
            return 0;
        }
        if ( js_value.isMember ( CHASSISS_MOVE_ROTATE ) )
        {
            std::string strval = js_value[CHASSISS_MOVE_ROTATE].asString();
            if ( strval == "+" )
            {
                if ( !continuous )
                {
                    value = cmd_rotate_step_;
                    return 2; // move one step
                }
                else
                {
                    value = cmd_rotate_speed_;
                    return 3; // move continuously
                }
            }
            if ( strval == "-" )
            {
                if ( !continuous )
                {
                    value = -cmd_rotate_step_;
                    return 2; // move one step
                }
                else
                {
                    value = -cmd_rotate_speed_;
                    return 3; // move continuously
                }
            }
            sscanf ( strval.c_str(), "%f", &value );
            return 2;
        }
        return -1;
    }

    int ChassisModule::Move_Direction ( const std::string &strsource )
    {
        if ( chassis_driver_ptr_ == NULL )
        {
            YSOS_LOG_ERROR ( "Driver Pointer Exception" );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        if ( move_input_buffer_ptr_ == NULL || move_output_buffer_ptr_ == NULL )
        {
            YSOS_LOG_ERROR ( "Buffer Exception" );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        std::string electcharge = "0";
        GetDataInterfaceManager()->GetData ( GLOBAL_DATA_KEY )->GetData ( "electcharge", electcharge );
        if ( electcharge == "1" )
        {
            YSOS_LOG_ERROR ( "The Battery is Charging!!!" );
            return YSOS_ERROR_FAILED;
        }
        int isReach = YSOS_ERROR_FAILED;
        float value = 0;
        int mode = Get_Move_Direction ( strsource, value );
        // move one step
        if ( mode == 0 )
        {
            do
            {
                // get the current point and caculate the aim point
                if ( YSOS_ERROR_SUCCESS != chassis_driver_ptr_->Ioctl ( CMD_CHASSIS_GET_LOCALIZATION_STATUS, move_input_buffer_ptr_, move_output_buffer_ptr_ ) )
                {
                    YSOS_LOG_DEBUG ( "Get Position Fail!" );
                    break;
                }
                float fposx, fposy, fangle;
                if ( !Get_Current_Point_Pos ( GetBufferUtility()->GetBufferData ( move_output_buffer_ptr_ ), fposx, fposy, fangle ) )
                {
                    break;
                }
                if ( CHASSIS_TYPE_GAUSSISS == chassis_type_ )
                {
                    value *= 20;
                }
                else if ( CHASSIS_TYPE_SILAN == chassis_type_ )
                {
                }
                fposx += float ( value * cos ( fangle / 180 * M_PI ) / 100.0 );
                fposy += float ( value * sin ( fangle / 180 * M_PI ) / 100.0 );
                if ( value < 0 )
                {
                    fangle += ( fangle > 0 ) ? -180 : 180;
                }
                // move to
                GetBufferUtility()->CopyStringToBuffer ( Make_MoveTo_Point ( fposx, fposy, fangle ), move_input_buffer_ptr_ );
                if ( YSOS_ERROR_SUCCESS != chassis_driver_ptr_->Ioctl ( CMD_CHASSIS_START_MOVE_TO, move_input_buffer_ptr_, move_output_buffer_ptr_ ) )
                {
                    YSOS_LOG_WARN ( "Move To Object Point Fail!" );
                    break;
                }
                Set_Move_Status ( CHASSISS_STATUS_MOVING );
                // waiting
                isReach = Wait_For_Moving ( MAX_CHASSIS_NAVIGATION_PLAN_CNT );
            } while ( 0 );
            const char* result = ( YSOS_ERROR_SUCCESS == isReach ) ? CHASSISS_STATUS_FINISH : CHASSISS_STATUS_UNREACH;
            YSOS_LOG_DEBUG ( "Move_Direction " << result );
            Set_Move_Status ( result );
        }
        // move continuously
        else if ( mode == 1 )
        {
            GetBufferUtility()->CopyStringToBuffer ( Make_Moveing_Cmd ( value, 0 ), move_input_buffer_ptr_ );
            while ( !Is_Move_Command_Updated() )
            {
                chassis_driver_ptr_->Ioctl ( CMD_CHASSIS_START_MOVE_BY_DIRECTION, move_input_buffer_ptr_, move_output_buffer_ptr_ );
                SLEEP ( 100 );
            }
        }
        // rotate one step
        else if ( mode == 2 )
        {
            // move to
            GetBufferUtility()->CopyStringToBuffer ( Make_Rotate_Position ( ( float ) value ), move_input_buffer_ptr_ );
            if ( YSOS_ERROR_SUCCESS != chassis_driver_ptr_->Ioctl ( CMD_CHASSIS_START_ROTATE, move_input_buffer_ptr_, move_output_buffer_ptr_ ) )
            {
                YSOS_LOG_WARN ( "Move To Object Point Fail!" );
            }
            else
            {
                Set_Move_Status ( CHASSISS_STATUS_MOVING );
                // waiting
                isReach = Wait_For_Rotating();
            }
            const char* result = ( YSOS_ERROR_SUCCESS == isReach ) ? CHASSISS_STATUS_FINISH : CHASSISS_STATUS_UNREACH;
            YSOS_LOG_DEBUG ( "Move_Direction " << result );
            Set_Move_Status ( result );
        }
        // rotate continuously
        else if ( mode == 3 )
        {
            GetBufferUtility()->CopyStringToBuffer ( Make_Moveing_Cmd ( 0, value ), move_input_buffer_ptr_ );
            while ( !Is_Move_Command_Updated() )
            {
                chassis_driver_ptr_->Ioctl ( CMD_CHASSIS_START_MOVE_BY_DIRECTION, move_input_buffer_ptr_, move_output_buffer_ptr_ );
                SLEEP ( 100 );
            }
        }
        else
        {
        }
        return isReach;
    }

    /************************************************************************/
    /* strsource:
      { "mouseX": "320", "mouseY": "420" }
    */
    /************************************************************************/
    bool ChassisModule::Get_Move_Camera_Point ( const std::string &strsource, struct ChassisLocation &point )
    {
        Json::Reader js_reader;
        Json::Value js_value;
        if ( !js_reader.parse ( strsource, js_value, false ) )
        {
            YSOS_LOG_ERROR ( "move camera point json error" );
            return false;
        }
        int px = atoi ( js_value["mouseX"].asString().c_str() );
        int py = atoi ( js_value["mouseY"].asString().c_str() );
        YSOS_LOG_ERROR ( "move camera point: " << px << " " << py );
        // step 1: (pixelx,pixely) -> (distance, angle)
        int i, j;
        for ( i = 0; i < CMA_COLUMN_NUM - 1 && py > CMA_COLUMN_LABEL[i]; i++ );
        for ( j = 0; j < CMA_ROW_NUM - 1 && px > CMA_ROW_LABEL[i][j]; j++ );
        ChassisRelative relative;
        relative.distance = CMA_POLAR_R[i][j];
        relative.angle = CMA_POLAR_ANGLE[i][j];
        YSOS_LOG_ERROR ( "dis and angle: " << relative.distance << " " << relative.angle );
        if ( relative.distance == 0 )
        {
            return false;
        }
        // step 2: (distance, angle) -> (x,y,angle)
        return Make_Point_From_Relative ( relative, point );
    }

    /************************************************************************/
    /* strsource:
      { "distance": 2.0, "angle": 3.4 }
    */
    /************************************************************************/
    bool ChassisModule::Get_Move_Relative_Point ( const std::string &strsource, struct ChassisLocation &point )
    {
        Json::Reader js_reader;
        Json::Value js_value;
        if ( !js_reader.parse ( strsource, js_value, false ) )
        {
            YSOS_LOG_ERROR ( "move camera point json error" );
            return false;
        }
        ChassisRelative relative;
        relative.distance = float ( js_value[CHASSISS_POS_DISTANCE].asDouble() );
        relative.angle = float ( js_value[CHASSISS_POS_ANGLE].asDouble() );
        YSOS_LOG_ERROR ( "move relatvie point: " << relative.distance << " " << relative.angle );
        return Make_Point_From_Relative ( relative, point );
    }

    bool ChassisModule::Make_Point_From_Relative ( ChassisRelative &relative, struct ChassisLocation &point )
    {
        if ( chassis_driver_ptr_ == NULL )
        {
            YSOS_LOG_ERROR ( "Driver Pointer Exception" );
            return false;
        }
        if ( move_input_buffer_ptr_ == NULL || move_output_buffer_ptr_ == NULL )
        {
            YSOS_LOG_ERROR ( "Buffer Exception" );
            return false;
        }
        if ( YSOS_ERROR_SUCCESS != chassis_driver_ptr_->Ioctl ( CMD_CHASSIS_GET_LOCALIZATION_STATUS, move_input_buffer_ptr_, move_output_buffer_ptr_ ) )
        {
            YSOS_LOG_DEBUG ( "Get Position Fail!" );
            return false;
        }
        float fposx, fposy, fangle;
        if ( !Get_Current_Point_Pos ( GetBufferUtility()->GetBufferData ( move_output_buffer_ptr_ ), fposx, fposy, fangle ) )
        {
            return false;
        }
        if ( CHASSIS_TYPE_GAUSSISS == chassis_type_ )
        {
            relative.distance /= 5;
        }
        else if ( CHASSIS_TYPE_SILAN == chassis_type_ )
        {
        }
        point.fposangle = fangle + relative.angle;
        point.fposx = fposx + relative.distance * cos ( point.fposangle / 180 * M_PI );
        point.fposy = fposy + relative.distance * sin ( point.fposangle / 180 * M_PI );
        if ( point.fposangle > 180 )
        {
            point.fposangle -= 180;
        }
        else if ( point.fposangle < -180 )
        {
            point.fposangle += 180;
        }
        return true;
    }

    int ChassisModule::Stop_Move()
    {
        if ( chassis_driver_ptr_ == NULL )
        {
            YSOS_LOG_ERROR ( "Driver Pointer Exception" );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        return chassis_driver_ptr_->Ioctl ( CMD_CHASSIS_STOP_MOVE, NULL, move_output_buffer_ptr_ );
    }

    int ChassisModule::Start_Greet()
    {
        if ( chassis_driver_ptr_ == NULL )
        {
            YSOS_LOG_ERROR ( "Driver Pointer Exception" );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        GetBufferUtility()->CopyStringToBuffer ( Make_Greet_Json_String(), move_input_buffer_ptr_ );
        return chassis_driver_ptr_->Ioctl ( CMD_CHASSIS_START_GREET, move_input_buffer_ptr_, move_output_buffer_ptr_ );
    }

    int ChassisModule::Stop_Greet()
    {
        if ( chassis_driver_ptr_ == NULL )
        {
            YSOS_LOG_ERROR ( "Driver Pointer Exception" );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        return chassis_driver_ptr_->Ioctl ( CMD_CHASSIS_STOP_GREET, NULL, move_output_buffer_ptr_ );
    }

    int ChassisModule::Wait_For_Rotating()
    {
        UINT8 *outbufferptr = NULL;
        UINT32 ioutbuffersize = 0;
        move_output_buffer_ptr_->GetBufferAndLength ( &outbufferptr, &ioutbuffersize );
        while ( true )
        {
            if ( Is_Move_Command_Updated() )
            {
                GetBufferUtility()->CopyStringToBuffer ( Make_Rotate_Position ( ( float ) 0.0 ), move_input_buffer_ptr_ );
                chassis_driver_ptr_->Ioctl ( CMD_CHASSIS_START_ROTATE, move_input_buffer_ptr_, move_output_buffer_ptr_ );
                break;
            }
            // get rotate status
            if ( YSOS_ERROR_SUCCESS != chassis_driver_ptr_->Ioctl ( CMD_CHASSIS_GET_ROTATE_STATUS, NULL, move_output_buffer_ptr_ ) )
            {
                YSOS_LOG_WARN ( "Get Rotate Status Fail!" );
                break;
            }
            // check rotate status
            std::string rotate_status = Get_Current_Navigation_Status ( outbufferptr );
            if ( rotate_status == CHASSISS_STATUS_FINISH )
            {
                return YSOS_ERROR_SUCCESS;
            }
            if ( rotate_status == CHASSISS_STATUS_UNREACH )
            {
                break;
            }
            SLEEP ( 100 );
        }
        return YSOS_ERROR_FAILED;
    }

    int ChassisModule::Wait_For_Moving ( const int maxPlanCnt, bool persist )
    {
        UINT8 *outbufferptr = NULL;
        UINT32 ioutbuffersize = 0;
        move_output_buffer_ptr_->GetBufferAndLength ( &outbufferptr, &ioutbuffersize );
        int iplan_time_out_cnt = 0;
        while ( !Is_Move_Command_Updated() )
        {
            // get navigation status
            if ( YSOS_ERROR_SUCCESS != chassis_driver_ptr_->Ioctl ( CMD_CHASSIS_GET_NAVIGATION_STATUS, NULL, move_output_buffer_ptr_ ) )
            {
                YSOS_LOG_WARN ( "Get Navigation Status Fail!" );
                break;
            }
            std::string navi_status = Get_Current_Navigation_Status ( outbufferptr );
            // check navigation status
            if ( navi_status == CHASSISS_STATUS_NULL )
            {
                if ( ++iplan_time_out_cnt == maxPlanCnt )
                {
                    break;
                }
            }
            else if ( navi_status == CHASSISS_STATUS_MOVING )
            {
                iplan_time_out_cnt = 0;
                Set_Move_Status ( CHASSISS_STATUS_MOVING );
            }
            else if ( navi_status == CHASSISS_STATUS_WARN_OBS )
            {
                if ( !persist )
                {
                    break;
                }
                Set_Move_Status ( CHASSISS_STATUS_WARNING );
            }
            else if ( navi_status == CHASSISS_STATUS_WARN_AIM )
            {
                if ( !persist )
                {
                    break;
                }
                Set_Move_Status ( CHASSISS_STATUS_WARNING );
            }
            else if ( navi_status == CHASSISS_STATUS_FINISH )
            {
                return YSOS_ERROR_SUCCESS;
            }
            else if ( navi_status == CHASSISS_STATUS_UNREACH )
            {
                break;
            }
            else
            {
            }
            SLEEP ( 100 );
        }
        Stop_Move();
        return YSOS_ERROR_FAILED;
    }

    int ChassisModule::Get_Chassis_Type()
    {
        char szInput[MAX_LENGTH256] = {'\0'};
        if ( YSOS_ERROR_SUCCESS == GetProperty ( PROP_CHASSIS_TYPE_INFO, szInput ) )
        {
            Json::Reader js_reader;
            Json::Value js_value;
            if ( !js_reader.parse ( szInput, js_value, false ) )
            {
                YSOS_LOG_ERROR ( "Output is Exception!" );
                return YSOS_ERROR_FAILED;
            }
            std::string strchassisname = js_value[CHASSIS_TYPE].asString();
            if ( 0 == strchassisname.compare ( CHASSIS_TYPE_GAUSSIAN ) )
            {
                chassis_type_ = CHASSIS_TYPE_GAUSSISS;
            }
        }
        else
        {
            chassis_type_ = CHASSIS_TYPE_SILAN;
        }
        return YSOS_ERROR_SUCCESS;
    }

    int ChassisModule::Get_Chassis_Connect_Status()
    {
        return chassis_connect_status_;
    }

    int ChassisModule::Set_Chassis_Connect_Status ( int chassis_status )
    {
        chassis_connect_status_ = chassis_status;
        return YSOS_ERROR_SUCCESS;
    }

    int ChassisModule::Connect_Heart_Thread ( LPVOID lpParam )
    {
        ChassisModule *chass_module_ptr = ( ChassisModule* ) lpParam;
        if ( chass_module_ptr == NULL )
        {
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        YSOS_LOG_DEBUG_CUSTOM ( chass_module_ptr->logger_, "Connect_Heart_Thread start running" );
        DriverInterfacePtr chassis_driver_ptr = chass_module_ptr->chassis_driver_ptr_;
        BufferInterfacePtr input_buffer_ptr = chass_module_ptr->heart_input_buffer_ptr_;
        BufferInterfacePtr output_buffer_ptr = chass_module_ptr->heart_output_buffer_ptr_;
        if ( chassis_driver_ptr == NULL || input_buffer_ptr == NULL || output_buffer_ptr == NULL )
        {
            YSOS_LOG_ERROR_CUSTOM ( chass_module_ptr->logger_, "ChassisModule Input Or Output Exception!" );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        /// map init
        std::string strconnect_string = chass_module_ptr->Make_Connect_Json_String();
        GetBufferUtility()->CopyStringToBuffer ( strconnect_string, input_buffer_ptr );
        chassis_driver_ptr->Ioctl ( CMD_CHASSIS_MAP_INITIALIZE, input_buffer_ptr, output_buffer_ptr );
        bool benable_heart_flag = false;
        int exception_cnt = 0;
        int chassis_connect_status = CHASSIS_STATUS_NONE;
        while ( true )
        {
            switch ( chassis_connect_status )
            {
                case CHASSIS_STATUS_NONE:
                case CHASSIS_STATUS_DISCONNECT:
                    {
                        /// 连接
                        if ( YSOS_ERROR_SUCCESS == chassis_driver_ptr->Ioctl ( CMD_CHASSIS_CONNECT, input_buffer_ptr, output_buffer_ptr ) )
                        {
                            chassis_connect_status = CHASSIS_STATUS_CONNECTED;
                            chass_module_ptr->Set_Chassis_Connect_Status ( CHASSIS_STATUS_CONNECTED );
                            YSOS_LOG_DEBUG_CUSTOM ( chass_module_ptr->logger_, "ChassisModule Connect Chassis Success!!!!" );
                            chass_module_ptr->data_ptr_->SetData ( "chassis_connect", "1" );
                            if ( !chass_module_ptr->chassis_heart_enable_ )
                            {
                                YSOS_LOG_DEBUG_CUSTOM ( chass_module_ptr->logger_, "Chassis Heart Disable." );
                                return YSOS_ERROR_SUCCESS;
                            }
                        }
                        else
                        {
                            YSOS_LOG_WARN_CUSTOM ( chass_module_ptr->logger_, "ChassisModule Connect Fail!!!!" );
                            chass_module_ptr->data_ptr_->SetData ( "chassis_connect", "0" );
                            chassis_connect_status = CHASSIS_STATUS_DISCONNECT;
                            SLEEP ( 5 * 1000 );
                        }
                    }
                    break;
                case CHASSIS_STATUS_CONNECTED:
                    {
                        /// 使能心跳
                        if ( !benable_heart_flag )
                        {
                            if ( YSOS_ERROR_SUCCESS == chassis_driver_ptr->Ioctl ( CMD_CHASSIS_ENABLE_HEART_BEAT, input_buffer_ptr, output_buffer_ptr ) )
                            {
                                benable_heart_flag = true;
                            }
                        }
                        /// 发送心跳
                        if ( benable_heart_flag )
                        {
                            if ( YSOS_ERROR_SUCCESS != chassis_driver_ptr->Ioctl ( CMD_CHASSIS_SEND_HEART_BEAT, input_buffer_ptr, output_buffer_ptr ) )
                            {
                                if ( ++exception_cnt >= 4 )
                                {
                                    chassis_connect_status = CHASSIS_STATUS_EXCEPTION;
                                    chass_module_ptr->Set_Chassis_Connect_Status ( CHASSIS_STATUS_EXCEPTION );
                                    YSOS_LOG_WARN_CUSTOM ( chass_module_ptr->logger_, "ChassisModule Connect Chassis Exception!!!!" );
                                }
                            }
                            else
                            {
                                exception_cnt = 0;
                            }
                        }
                    }
                    break;
                case CHASSIS_STATUS_EXCEPTION:
                    {
                        /// 断开连接
                        if ( YSOS_ERROR_SUCCESS == chassis_driver_ptr->Ioctl ( CMD_CHASSIS_DISCONNECT, input_buffer_ptr, output_buffer_ptr ) )
                        {
                            chassis_connect_status = CHASSIS_STATUS_DISCONNECT;
                            chass_module_ptr->Set_Chassis_Connect_Status ( CHASSIS_STATUS_DISCONNECT );
                            YSOS_LOG_DEBUG_CUSTOM ( chass_module_ptr->logger_, "ChassisModule Disconnect Chassis!!!!" );
                            benable_heart_flag = false;
                            exception_cnt = 0;
                        }
                    }
                    break;
                default:
                    break;
            }
            SLEEP ( 250 );
        }
        return YSOS_ERROR_SUCCESS;
    }

    std::string ChassisModule::Make_Connect_Json_String()
    {
        Json::FastWriter js_writer;
        Json::Value js_value;
        js_value["ip"] = strconnect_addr_;
        js_value[CHASSISS_MAPNAME] = strmap_name_;
        js_value[CHASSISS_POSITIONNAME] = strmap_position_;
        js_value[CHASSISS_MAP_INITDIRECT] = binit_direct_;
        std::string strjsonresult = js_writer.write ( js_value );
        strjsonresult = GetUtility()->ReplaceAllDistinct ( strjsonresult, "\\r\\n", "" );
        YSOS_LOG_DEBUG ( "Connect Chassis Info:" << strjsonresult );
        return strjsonresult;
    }

    bool ChassisModule::Set_Move_Command ( const int movemode, const std::string& param )
    {
        boost::lock_guard<boost::mutex> autolock ( move_mutex_ );
        move_mode_ = movemode;
        move_cmd_param_ = param;
        move_status_ = "null";
        move_cmd_chg_ = true;
        move_status_chg_ = true;
        return true;
    }

    bool ChassisModule::Get_Move_Command ( int &movemode, std::string& param )
    {
        boost::lock_guard<boost::mutex> autolock ( move_mutex_ );
        if ( move_cmd_chg_ )
        {
            movemode = move_mode_;
            param = move_cmd_param_;
            move_cmd_chg_ = false;
            return true;
        }
        return false;
    }

    bool ChassisModule::Is_Move_Command_Updated()
    {
        // just a bool var, not to need lock
        return move_cmd_chg_;
    }

    bool ChassisModule::Is_Move_Mode ( int move_mode )
    {
        boost::lock_guard<boost::mutex> autolock ( move_mutex_ );
        return ( !move_cmd_chg_ ) && ( move_mode_ == move_mode );
    }

    bool ChassisModule::Set_Move_Status ( const char* status )
    {
        boost::lock_guard<boost::mutex> autolock ( move_mutex_ );
        if ( move_cmd_chg_ )
        {
            return false;
        }
        move_status_chg_ = true;
        move_status_ = status;
        return true;
    }

    bool ChassisModule::Get_Move_Status ( BufferInterfacePtr &out_buffer_ptr )
    {
        boost::lock_guard<boost::mutex> autolock ( move_mutex_ );
        if ( !move_status_chg_ )
        {
            return false;
        }
        UINT8 *outbufferptr = NULL;
        UINT32 ioutbuffersize = 0;
        out_buffer_ptr->GetBufferAndLength ( &outbufferptr, &ioutbuffersize );
        // {"move_mode":"0/1/...","task_status":"moving/finish/unreach/null"}
        sprintf ( reinterpret_cast<char*> ( outbufferptr ), "{\"move_mode\":\"%d\",\"task_status\":\"%s\"}", move_mode_, move_status_.c_str() );
        move_status_chg_ = false;
        return true;
    }

}
