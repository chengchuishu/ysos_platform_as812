/**
*@file gaussian_chassis_driverinterface.cpp
*@brief Implement of gaussian chassis driver
*@version 0.9.0.0
*@author Lu Min, Wang Xiaogui
*@date Created on: 2016-12-01 12:40:00
*@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
* 
*/

/// Myself Headers
#include "../include/gaussian_chassis_driver_impl.h"

/// Public Header
#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"

/// YSOS Headers
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"
#include <json/json.h>
#include "../../../../../../thirdparty/include/curl/curl.h"

#ifdef  WIN32
    #define SLEEP(a)      Sleep(a)
#else
    #define SLEEP(a)      (boost::thread::sleep(boost::get_system_time()+boost::posix_time::milliseconds(a)))
#endif


#define HTTP_URL_LENGTH               256
#define MAX_LENGTH6                   6
#define MAX_LENGTH16                  16
#define MAX_LENGTH32                  32
#define MAX_LENGTH64                  64
#define MAX_LENGTH256                 256
#define MAX_LENGTH1024                1024

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#define EXCHANGE_DIRECT_RATIO         7
#define CHECK_OBSTANCLE_MIN_DISTANCE  0.06

//#define TEST 1
#ifdef  TEST
    #define HTTP_SERVER_ADDR              "api.gs-robot.me"
    #define HTTP_SERVER_PORT              6789
    #define WEBSOCKRT_PORT                8089
#else
    #define HTTP_SERVER_ADDR              "10.7.5.88"
    #define HTTP_SERVER_PORT              8080
    #define WEBSOCKRT_PORT                8089
#endif

#define HTTP_OBJECT_NAME_GETSDKVERSION      "/gs-robot/info/version"
#define HTTP_OBJECT_NAME_LOADMAP            "/gs-robot/cmd/use_map?map_name=%s"
#define HTTP_OBJECT_NAME_INIT               "/gs-robot/cmd/initialize?map_name=%s&init_point_name=%s"
#define HTTP_OBJECT_NAME_DIRECT_INIT        "/gs-robot/cmd/initialize_directly?map_name=%s&init_point_name=%s"
#define HTTP_OBJECT_NAME_INIT_FINISH        "/gs-robot/cmd/is_initialize_finished"

#define HTTP_OBJECT_NAME_GETCURPOS          "/gs-robot/real_time_data/position"
#define HTTP_OBJECT_NAME_GETLASERRAW        "/gs-robot/real_time_data/laser_raw"
#define HTTP_OBJECT_NAME_GETULTRASONICRAW   "/gs-robot/real_time_data/ultrasonic_raw?frame_id=%d"
#define HTTP_OBJECT_NAME_GETPROTECTORRAW    "/gs-robot/real_time_data/protector"
#define HTTP_OBJECT_NAME_GETNAVIGATEPATH    "/gs-robot/real_time_data/navigation_path"
#define HTTP_OBJECT_NAME_MOBILEDATA         "/gs-robot/real_time_data/mobile_data"
#define HTTP_OBJECT_NAME_NONMAPDATA         "/gs-robot/real_time_data/non_map_data"

#define HTTP_OBJECT_NAME_HEARTBEATMACH      "/gs-robot/cmd/ping"
#define HTTP_OBJECT_NAME_PINGCHASSIS        "/gs-robot/cmd/ping?device_id=1"
#define HTTP_OBJECT_NAME_STARTPINGCHECK     "/gs-robot/cmd/set_heart?flag=1"
#define HTTP_OBJECT_NAME_STOPPINGCHECK      "/gs-robot/cmd/set_heart?flag=0"

#define HTTP_OBJECT_NAME_MOVETO             "/gs-robot/cmd/navigate"
#define HTTP_OBJECT_NAME_PAUSENAVIGATE      "/gs-robot/cmd/pause_navigate"
#define HTTP_OBJECT_NAME_RESUMENAVIGATE     "/gs-robot/cmd/resume_navigate"
#define HTTP_OBJECT_NAME_STOPMOVE           "/gs-robot/cmd/cancel_navigate"
#define HTTP_OBJECT_NAME_MOVETOBYNAME       "/gs-robot/cmd/position/navigate?map_name=%s&position_name=%s"
#define HTTP_OBJECT_NAME_MOVEBYDIR          "/gs-robot/cmd/move_manual_mode"

#define HTTP_OBJECT_NAME_SPEED              "/gs-robot/cmd/move"
#define HTTP_OBJECT_NAME_ROTATETO           "/gs-robot/cmd/rotate"
#define HTTP_OBJECT_NAME_ROTATE_STATUS      "/gs-robot/cmd/is_rotate_finished"
#define HTTP_OBJECT_NAME_POWEROFF           "/gs-robot/cmd/power_off"

#define HTTP_OBJECT_NAME_CONFIG_STATUS      "/gs-robot/data/get_config_status"
#define HTTP_OBJECT_NAME_SET_PROTECTOR      "/gs-robot/cmd/set_protector?protector="
#define HTTP_OBJECT_NAME_SET_UTRASONIC      "/gs-robot/cmd/set_utrasonic?utrasonic="

#define HTTP_OBJECT_NAME_START_RECORDPATH   "/gs-robot/cmd/start_record_path?map_name=%s&path_name=%s"
#define HTTP_OBJECT_NAME_STOP_RECORDPATH    "/gs-robot/cmd/stop_record_path"
#define HTTP_OBJECT_NAME_CANCEL_RECORDPATH  "/gs-robot/cmd/cancel_record_path"

#define HTTP_OBJECT_NAME_GET_OBSTACLE       "/gs-robot/data/virtual_obstacles?map_name="
#define HTTP_OBJECT_NAME_UPDATE_OBSTACLE    "/gs-robot/cmd/update_virtual_obstacles"

#define HTTP_OBJECT_NAME_GET_POS            "/gs-robot/data/positions?map_name=%s"
#define HTTP_OBJECT_NAME_ADD_POS            "/gs-robot/cmd/add_position?position_name=%s&type=2"
#define HTTP_OBJECT_NAME_DEL_POS            "/gs-robot/cmd/delete_position?map_name=%s&position_name=%s"
#define HTTP_OBJECT_NAME_GET_PATH           "/gs-robot/data/paths?map_name=%s"
#define HTTP_OBJECT_NAME_DEL_PATH           "/gs-robot/cmd/delete_path?map_name=%s&path_name=%s"
#define HTTP_OBJECT_NAME_RENAME_PATH        "/gs-robot/cmd/rename_path?map_name=%s&origin_path_name=%s&new_path_name=%s"

#define HTTP_OBJECT_NAME_START_CHARGE       "/gs-robot/cmd/start_charge"
#define HTTP_OBJECT_NAME_STOP_CHARGE        "/gs-robot/cmd/stop_charge"

#define HTTP_OBJECT_NAME_MAP_GET_MAPLIST     "/gs-robot/data/maps"
#define HTTP_OBJECT_NAME_MAP_GET_PNG         "/gs-robot/data/map_png?map_name=%s"
#define HTTP_OBJECT_NAME_MAP_DEL             "/gs-robot/cmd/delete_map?map_name=%s"
#define HTTP_OBJECT_NAME_MAP_RENAME          "/gs-robot/cmd/rename_map?origin_map_name=%s&new_map_name=%s"
#define HTTP_OBJECT_NAME_MAP_SCAN_START      "/gs-robot/cmd/start_scan_map?map_name=%s"
#define HTTP_OBJECT_NAME_MAP_SCAN_STOP       "/gs-robot/cmd/stop_scan_map"
#define HTTP_OBJECT_NAME_MAP_SCAN_CANCEL     "/gs-robot/cmd/cancel_scan_map"

// robot param
#define HTTP_OBJECT_NAME_ROBOT_PARAM_LIST    "/gs-robot/data/robot_param_list"
#define HTTP_OBJECT_NAME_MODIFY_ROBOT_PARAM  "/gs-robot/cmd/modify_robot_param"

// greet
#define HTTP_OBJECT_NAME_STARTGREET         "/gs-robot/cmd/start_greeter?map_name=%s&greeter_name=%s"
#define HTTP_OBJECT_NAME_STOPGREET          "/gs-robot/cmd/stop_greeter"
#define HTTP_OBJECT_NAME_PAUSEGREET         "/gs-robot/cmd/pause_greeter"
#define HTTP_OBJECT_NAME_RESUMEGREET        "/gs-robot/cmd/resume_greeter"

/// Web Socket Status
#define WEBSOCKET_STATUS_DEVICE             "/gs-robot/notice/device_status"
#define WEBSOCKET_STATUS_NAVIGATION         "/gs-robot/notice/navigation_status"
#define WEBSOCKET_STATUS_HEALTH             "/gs-robot/notice/system_health_status"
#define WEBSOCKET_STATUS_GREET              "/gs-robot/notice/status"

/// 返回结果
#define RESULT_SUCCESS                "successed"
#define RESULT_ERRCODE                "errorCode"
#define RESULT_TRUE                   "true"
#define RESULT_FALSE                  "false"
#define RESULT_DATA                   "data"
#define RESULT_STATUS                 "status"
#define RESULT_ERROR                  "error"
#define RESULT_MESSAGE                "msg"

/// 底盘类型
#define CHASSIS_TYPE                 "chassistype"

/// 版本_SDK获取
#define CHASSIS_SDK_VERSION           "version"
#define CHASSIS_SDK_MODELTYPE         "modelType"
#define CHASSIS_SDK_PRODUCTID         "productId"
/// 版本_输出
#define CHASSIS_SDK_NAME_GAUSSIAN    "gaussian"
#define CHASSIS_SDK_DEVICEID         "deviceID"
#define CHASSIS_SDK_MANUFACTUREID    "manufacturerID"
#define CHASSIS_SDK_MANUFACTURENAME  "manufacturerName"
#define CHASSIS_SDK_MODELID          "modelID"
#define CHASSIS_SDK_MODELNAME        "modelName"
#define CHASSIS_SDK_HARDWAREVERSION  "hardwareVersion"
#define CHASSIS_SDK_SOFTWAREVERSION  "softwareVersion"

/// 地图
#define CHASSISS_MAP_NAME             "mapname"
#define CHASSISS_MAP_POSITION_NAME    "positionname"
#define CHASSISS_MAP_GREETARER        "greetarea"
#define CHASSISS_MAP_INITDIRECT       "initdirect"
#define CHASSISS_MAP_ORIGIN_NAME      "originname"
#define CHASSISS_MAP_NEW_NAME         "newname"
#define CHASSISS_MAP_PATH_NAME        "pathname"

/// 位置信息
#define CHASSISS_POS_X                "x"
#define CHASSISS_POS_Y                "y"
#define CHASSISS_POS_ANGLE            "angle"
#define CHASSISS_POS_ROTATE           "rotate"
#define CHASSISS_POS_ROTATETO         "rotateto"
#define CHASSISS_POS_MOVEDIRECTION    "movedirection"
#define CHASSISS_POS_QUALITY          "quality"

#define CHASSISS_POS_ROTATAANGLE      "rotateAngle"
#define CHASSISS_POS_GRID             "gridPosition"
#define CHASSISS_POS_DESTINATION      "destination"
#define CHASSISS_POS_DISTANCE         "distance"

/// 速度
#define CHASSISS_SPEED                "speed"
#define CHASSISS_SPEED_LEVEL          "speedlevel"
#define CHASSISS_SPEED_LINE           "linearSpeed"
#define CHASSISS_SPEED_ANGULAR        "angularSpeed"
#define CHASSISS_ROTATE_ISFINISH      "is_rotate_finish"

/// 激光和防碰撞
#define CHASSISS_CONFIG_PROTECTOR     "protector"
#define CHASSISS_CONFIG_ULTRASONIC    "ultrasonic"

/// 激光检测障碍物
#define CHASSISS_OBSTANCLE_RANGES     "ranges"
#define CHASSISS_HAVE_OBSTANCLE       "haveobstacle"
#define CHASSISS_OBSTANCLE_AVGDIS     "obstacledistance"

/// 障碍物
#define CHASSISS_OBSTANCLE                  "obstacle"
#define CHASSISS_OBSTANCLE_NONMAP           "non_map_value"
#define CHASSISS_OBSTANCLE_MOBILE           "mobile_value"
#define CHASSISS_OBSTANCLE_AVG_DIST         "avg_obstacle_distance"
#define CHASSISS_OBSTANCLE_ANGLE_FRONT      "angle_front"
#define CHASSISS_OBSTANCLE_ANGLE_PARANOMIC  "angle_paranomic"
#define CHASSISS_OBSTANCLE_OBJECT_ANGLE      "objectAngle"
#define CHASSISS_OBSTANCLE_OBJECT_SPEED      "objectSpeed"
#define CHASSISS_OBSTANCLE_POSSIBILITY       "possibility"
#define CHASSISS_OBSTANCLE_RANGE_MINANGLE    "angle_min"
#define CHASSISS_OBSTANCLE_RANGE_MAXANGLE    "angle_max"
#define CHASSISS_OBSTANCLE_RANGE_INCREANGLE  "angle_increment"

/// 设备状态
#define CHASSISS_DEVICE_STATUS_BATTERY         "battery"
#define CHASSISS_DEVICE_STATUS_CHARGE          "charging"
#define CHASSISS_DEVICE_STATUS_SPEED           "speed"
#define CHASSISS_DEVICE_STATUS_MILLAGE         "millage"
#define CHASSISS_DEVICE_STATUS_EMERGENCYSTOP   "emergencyStop"
#define CHASSISS_DEVICE_STATUS_DC              "dc"

/// 设备健康状态
#define CHASSISS_HEALTH_STATUS_CONNECT_MCU      "MCUConnection"
#define CHASSISS_HEALTH_STATUS_CONNECT_LASER    "laserConnection"
#define CHASSISS_HEALTH_STATUS_CONNECT_ROUTER   "routerConnection"

/// 设备导航状态
#define CHASSISS_NAVIGATION_STATUS_FINISH      "finish"
#define CHASSISS_NAVIGATION_STATUS_UNREACH     "unreach"
#define CHASSISS_NAVIGATION_STATUS_MOVING      "moving"
#define CHASSISS_NAVIGATION_STATUS_NULL        "null"
#define CHASSISS_NAVIGATION_STATUS_WARN_OBS    "warn_obs"
#define CHASSISS_NAVIGATION_STATUS_WARN_AIM    "warn_aim"

/// gaussian navi state
#define NOTICE_TYPE_INFO_PLANING       "PLANNING"
#define NOTICE_TYPE_INFO_HEADING       "HEADING"
#define NOTICE_TYPE_INFO_UNREACHABLE   "UNREACHABLE"
#define NOTICE_TYPE_INFO_UNREACHED     "UNREACHED"
#define NOTICE_TYPE_INFO_REACHED       "REACHED"
#define NOTICE_TYPE_WARN_LOCAL_FAIL    "LOCALIZATION_FAILED"
#define NOTICE_TYPE_WARN_GOAL_NOSAFE   "GOAL_NOT_SAFE"
#define NOTICE_TYPE_WARN_CLOSEOBSTACLE "TOO_CLOSE_TO_OBSTACLES"

#define CHASSISS_GREET_STATUS_CODE             "greetStatusCode"

/// 设备信息
#define CHASSISS_DEVICE_SENSORS               "sensors"
#define CHASSISS_DEVICE_ID                    "id"
#define CHASSISS_DEVICE_POSE                  "pose"
#define CHASSISS_DEVICE_TYPE                  "type"
#define CHASSISS_DEVICE_FREQ                  "freq"
#define CHASSISS_DEVICE_KIND                  "kind"
#define CHASSISS_DEVICE_METHOD                "method"

static log4cplus::Logger s_logtag = GetUtility()->GetLogger ( "ysos.chassis" );

/// HTTP接受数据长度

#ifdef _DEBUG
int use_debug_info ( CURL *curl_ptr, curl_infotype itype, char *data_ptr, size_t size, void *param_ptr )
{
    //   switch(itype){
    //   case CURLINFO_TEXT:
    //     YSOS_LOG << "CURLINFO_TEXT:" <<data_ptr<< std::endl;
    //     break;
    //   case CURLINFO_HEADER_IN:
    //     YSOS_LOG << "CURLINFO_HEADER_IN:" <<data_ptr<< std::endl;
    //     break;
    //   case CURLINFO_HEADER_OUT:
    //     YSOS_LOG << "CURLINFO_HEADER_OUT:" <<data_ptr<< std::endl;
    //     break;
    //   case CURLINFO_DATA_IN:
    //     YSOS_LOG << "CURLINFO_DATA_IN:" <<data_ptr<< std::endl;
    //     break;
    //   case CURLINFO_DATA_OUT:
    //     YSOS_LOG << "CURLINFO_DATA_OUT:" <<data_ptr<< std::endl;
    //     break;
    //   default:
    //     break;
    //   }
    return 0;
}
#endif

namespace ysos {

    DECLARE_PLUGIN_REGISTER_INTERFACE ( GaussianChassisDriver, DriverInterface );
    GaussianChassisDriver::GaussianChassisDriver ( const std::string &strClassName ) : BaseDriverImpl ( strClassName )
    {
        //server_addr_ptr_.reset();
        //out_buffer_ptr_.reset();
        server_port_ = HTTP_SERVER_PORT;
        http_time_out_ = 2;
        version_sdp_ = "0.0.0";
        version_sdk_ = "";
        connect_map_ = "office";
        is_open_ = false;
        is_sdp_connected_ = false;
        is_websocket_connect_device_ = false;
        is_websocket_connect_navigation_ = false;
        is_websocket_connect_health_ = false;
        is_websocket_connect_greet_ = false;
        is_enable_heart_ = true;
        connect_sleep_time_ = 5000;
        speed_line_ = 0.0;
        speed_angular_ = 0.0;
        obstacle_detection_range_ = 1.0;
        obstacle_detection_angle_ = 15.0;
        logger_ = GetUtility()->GetLogger ( "ysos.chassis" );
    }

    GaussianChassisDriver::~GaussianChassisDriver()
    {
        if ( is_sdp_connected_ )
        {
            websocket_close_device_status();
            websocket_close_health_status();
            websocket_close_navigation_status();
            websocket_close_greet_status();
            is_sdp_connected_ = false;
        }
        if ( is_open_ )
        {
            is_open_ = false;
            curl_global_cleanup();
        }
        YSOS_LOG_ERROR ( "Gaussian Chassis Driver End" );
    }


    int GaussianChassisDriver::GetProperty ( int itypeid, void *itype_ptr )
    {
        if ( NULL == itype_ptr )
        {
            YSOS_LOG_ERROR ( "Get chassis system info Error." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        switch ( itypeid )
        {
            case PROP_CHASSIS_SYSTEM_INFO:
                {
                    if ( YSOS_ERROR_SUCCESS != GetChassisVersion() )
                    {
                        YSOS_LOG_WARN ( "Get Chassis Version Fail." );
                        return YSOS_ERROR_FAILED;
                    }
                    /// 构造输出Json
                    Json::Value js_data;
                    js_data[CHASSIS_SDK_DEVICEID] = chassis_productId_;
                    js_data[CHASSIS_SDK_MANUFACTUREID] = "";
                    js_data[CHASSIS_SDK_MANUFACTURENAME] = CHASSIS_SDK_NAME_GAUSSIAN;
                    js_data[CHASSIS_SDK_MODELID] = chassis_module_type_;
                    js_data[CHASSIS_SDK_MODELNAME] = "";
                    js_data[CHASSIS_SDK_HARDWAREVERSION] = "";
                    js_data[CHASSIS_SDK_SOFTWAREVERSION] = version_sdk_;
                    Json::FastWriter js_writer;
                    std::string strjson_result = js_writer.write ( js_data );
                    strjson_result = GetUtility()->ReplaceAllDistinct ( strjson_result, "\\r\\n", "" );
                    memcpy ( itype_ptr, strjson_result.c_str(), strjson_result.length() );
                }
                break;
            case PROP_CHASSIS_TYPE_INFO:
                {
                    /// 构造输出Json
                    Json::Value js_data;
                    js_data[CHASSIS_TYPE] = CHASSIS_SDK_NAME_GAUSSIAN;
                    Json::FastWriter js_writer;
                    std::string strjson_result = js_writer.write ( js_data );
                    strjson_result = GetUtility()->ReplaceAllDistinct ( strjson_result, "\\r\\n", "" );
                    memcpy ( itype_ptr, strjson_result.c_str(), strjson_result.length() );
                }
                break;
            default:
                YSOS_LOG_WARN ( "This property is not support." );
                return YSOS_ERROR_NOT_SUPPORTED;
        }
        return YSOS_ERROR_SUCCESS;
    }

    int GaussianChassisDriver::SetProperty ( int itypeid, void *itype_ptr )
    {
        return YSOS_ERROR_SUCCESS;
    }

    int GaussianChassisDriver::Open ( void *params_ptr )
    {
        CURLcode curlcode = curl_global_init ( CURL_GLOBAL_ALL );
        if ( CURLE_OK != curlcode )
        {
            YSOS_LOG_WARN ( "curl_global_init Fail:" << curl_easy_strerror ( curlcode ) );
        }
        is_open_ = true;
        return YSOS_ERROR_SUCCESS;
    }

    void GaussianChassisDriver::Close ( void *params_ptr )
    {
        if ( is_open_ )
        {
            is_open_ = false;
            curl_global_cleanup();
        }
    }

    int GaussianChassisDriver::Read ( BufferInterfacePtr outbuffer_ptr, DriverContexPtr context_ptr )
    {
        return YSOS_ERROR_SUCCESS;
    }


    int GaussianChassisDriver::Write ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr, DriverContexPtr context_ptr )
    {
        return YSOS_ERROR_SUCCESS;
    }

    int GaussianChassisDriver::Ioctl ( int ictrl_id, BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        switch ( ictrl_id )
        {
            case CMD_CHASSIS_CONNECT:
                return IoctlConnect ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_DISCONNECT:
                return IoctlDisconnect ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_RECONNECT:
                return IoctlReconnect ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_RESTART:
                return IoctlRestart ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_SET_DEVICE_INFO:
                return IoctlSetDeviceInfo ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_GET_DEVICE_INFO:
                return IoctlGetDeviceInfo ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_ENABLE_HEART_BEAT:
                return IoctlEnableHeartBeat ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_DISABLE_HEART_BEAT:
                return IoctlDisableHeartBeat ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_POWER_OFF:
                return IoctlPowerOff ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_MAP_INITIALIZE:
                return IoctlMapInitialize ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_SET_MAP_INFO:
                return IoctlSetMapInfo ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_GET_MAP_INFO:
                return IoctlGetMapInfo ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_SET_VIRTUAL_OBSTACLE:
                return IoctlSetVirtualObstacle ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_GET_VIRTUAL_OBSTACLE:
                return IoctlGetVirtualObstacle ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_SET_PREDEFINED_POSITION:
                return IoctlSetPredefinedPosition ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_GET_PREDEFINED_POSITION:
                return IoctlGetPredefinedPosition ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_SET_OBSTACLE_DETECTION_RANGE:
                return IoctlSetObstacleDetectionRange ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_GET_OBSTACLE_DETECTION_RANGE:
                return IoctlGetObstacleDetectionRange ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_SET_MOVE_SPEED:
                return IoctlSetMoveSpeed ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_GET_MOVE_SPEED:
                return IoctlGetMoveSpeed ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_SEND_HEART_BEAT:
                return IoctlSendHeartBeat ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_GET_DEVICE_STATUS:
                return IoctlGetDeviceStatus ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_GET_HEALTH_STATUS:
                return IoctlGetHealthStatus ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_GET_LOCALIZATION_STATUS:
                return IoctlGetLocalizationStatus ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_GET_NAVIGATION_STATUS:
                return IoctlGetNavigationStatus ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_GET_SENSOR_DATA:
                return IoctlGetSensorData ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_GET_OBSTACLE_DATA:
                return IoctlGetObstacleData ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_GO_HOME:
                return IoctlGoHome ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_START_MOVE_BY_DIRECTION:
                return IoctlStartMoveByDirection ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_START_MOVE_TO:
                return IoctlStartMoveTo ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_START_MOVE_BY_PATH:
                return IoctlStartMoveByPath ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_PAUSE_MOVE:
                return IoctlPauseMove ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_RESUME_MOVE:
                return IoctlResumeMove ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_STOP_MOVE:
                return IoctlStopMove ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_START_RECORD_PATH:
                return IoctlStartRecordPath ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_STOP_RECORD_PATH:
                return IoctlStopRecordPath ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_START_ROTATE_TO:
                return IoctlStartRotateTo ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_START_ROTATE:
                return IoctlStartRotate ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_GET_ROTATE_STATUS:
                return IoctlGetRotateStatus ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_CHECK_ERROR_STATUS:
                return IoctlCheckErrorStatus ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_GET_NEAREST_LASER_INFO_IN_ANGLE_RANGE:
                return IoctlGetNearestLaserInfoInAngleRange ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_GET_GREET_STATUS_CODE:
                return IoctlGetGreetStatusCode ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_START_GREET:
                return IoctlStartGreet ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_PAUSE_GREET:
                return IoctlPauseGreet ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_RESUME_GREET:
                return IoctlResumeGreet ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_STOP_GREET:
                return IoctlStopGreet ( input_buffer_ptr, output_buffer_ptr );
            case CMD_CHASSIS_SET_GREET_PARAM:
                return IoctlSetGreetParam ( input_buffer_ptr, output_buffer_ptr );
            default:
                {
                    YSOS_LOG_WARN ( "this cmd is not supported." );
                    return YSOS_ERROR_NOT_SUPPORTED;
                }
        }
        return YSOS_ERROR_SUCCESS;
    }

    int GaussianChassisDriver::IoctlConnect ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlConnect" );
        if ( YSOS_ERROR_SUCCESS == websocket_connect_device_status ( ( UINT8* ) HTTP_SERVER_ADDR, WEBSOCKRT_PORT, ( UINT8* ) WEBSOCKET_STATUS_DEVICE ) )
        {
            is_websocket_connect_device_ = true;
            YSOS_LOG_DEBUG ( "Web socket connect device status success." );
        }
        else
        {
            is_websocket_connect_device_ = false;
            YSOS_LOG_WARN ( "Web socket connect device status fail." );
        }
        if ( YSOS_ERROR_SUCCESS == websocket_connect_navigation_status ( ( UINT8* ) HTTP_SERVER_ADDR, WEBSOCKRT_PORT, ( UINT8* ) WEBSOCKET_STATUS_NAVIGATION ) )
        {
            is_websocket_connect_navigation_ = true;
            YSOS_LOG_DEBUG ( "Web socket connect navigation status success." );
        }
        else
        {
            is_websocket_connect_navigation_ = false;
            YSOS_LOG_WARN ( "Web socket connect navigation status fail." );
        }
        if ( YSOS_ERROR_SUCCESS == websocket_connect_health_status ( ( UINT8* ) HTTP_SERVER_ADDR, WEBSOCKRT_PORT, ( UINT8* ) WEBSOCKET_STATUS_HEALTH ) )
        {
            is_websocket_connect_health_ = true;
            YSOS_LOG_DEBUG ( "Web socket connect health status success." );
        }
        else
        {
            is_websocket_connect_health_ = false;
            YSOS_LOG_WARN ( "Web socket connect health status fail." );
        }
        if ( YSOS_ERROR_SUCCESS == websocket_connect_greet_status ( ( UINT8* ) HTTP_SERVER_ADDR, WEBSOCKRT_PORT, ( UINT8* ) WEBSOCKET_STATUS_GREET ) )
        {
            is_websocket_connect_greet_ = true;
            YSOS_LOG_DEBUG ( "Web socket connect greet status success." );
        }
        else
        {
            is_websocket_connect_greet_ = false;
            YSOS_LOG_WARN ( "Web socket connect greet status fail." );
        }
        is_sdp_connected_ = true;
        YSOS_LOG_DEBUG ( "Connect Success." );
        return YSOS_ERROR_SUCCESS;
    }

    int GaussianChassisDriver::IoctlDisconnect ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlDisconnect" );
        if ( is_sdp_connected_ )
        {
            websocket_close_device_status();
            websocket_close_health_status();
            websocket_close_navigation_status();
            websocket_close_greet_status();
            is_sdp_connected_ = false;
        }
        return YSOS_ERROR_SUCCESS;
    }

    int GaussianChassisDriver::IoctlReconnect ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlReconnect" );
        int iret = YSOS_ERROR_FAILED;
        if ( YSOS_ERROR_SUCCESS == IoctlDisconnect ( input_buffer_ptr, output_buffer_ptr ) )
        {
            SLEEP ( 200 );
            iret = IoctlConnect ( input_buffer_ptr, output_buffer_ptr );
        }
        return iret;
    }

    int GaussianChassisDriver::IoctlRestart ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlRestart" );
        return YSOS_ERROR_FAILED;
    }

    /************************************************************************/
    /*output_buffer_ptr:
    {
      "protector":"111111",
      "ultrasonic":"111111",
       sensors: [{"id":"1", "pose":"", "type":"", "freq":"2.0", "kind":""}]
    }*/
    /************************************************************************/
    int GaussianChassisDriver::IoctlGetDeviceInfo ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlGetDeviceInfo" );
        const char* strhttpobjname = HTTP_OBJECT_NAME_CONFIG_STATUS;
        if ( !is_sdp_connected_ )
        {
            YSOS_LOG_WARN ( "Chassis is not connected." );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Input Or Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        if ( YSOS_ERROR_SUCCESS != http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) strhttpobjname, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_get Fail." );
            return YSOS_ERROR_FAILED;
        }
        UINT8 *outbufferptr = GetBufferUtility()->GetBufferData ( output_buffer_ptr );
        if ( NULL == outbufferptr )
        {
            YSOS_LOG_ERROR ( "Output Buffer is NULL." );
            return YSOS_ERROR_FAILED;
        }
        /// 解析获取数据
        std::string strresult = reinterpret_cast<char*> ( outbufferptr );
        if ( 0 == strresult.length() )
        {
            YSOS_LOG_WARN ( "HTTP Return Length is 0!" );
            return YSOS_ERROR_FAILED;
        }
        #ifdef _DEBUG
        YSOS_LOG_DEBUG ( "Http Get Result:" << strresult );
        #endif
        if ( strresult.find ( RESULT_SUCCESS ) == std::string::npos )
        {
            YSOS_LOG_WARN ( "HTTP Return Result Exception:" << strresult );
            return YSOS_ERROR_FAILED;
        }
        boost::property_tree::ptree ptjson;
        std::istringstream stream ( strresult );
        boost::property_tree::json_parser::read_json ( stream, ptjson );
        std::string ret_successed = ptjson.get<std::string> ( RESULT_SUCCESS, "" );
        if ( 0 != ret_successed.compare ( RESULT_TRUE ) )
        {
            std::string ret_errorcode = ptjson.get<std::string> ( RESULT_ERRCODE, "" );
            YSOS_LOG_WARN ( "Get Device Status Fail, the ErrorCode is:" << ret_errorcode.c_str() );
            return YSOS_ERROR_FAILED;
        }
        boost::property_tree::ptree ptjsonchild = ptjson.get_child ( RESULT_DATA );
        std::string strprotector = ptjsonchild.get<std::string> ( CHASSISS_CONFIG_PROTECTOR, "" );
        std::string strultrasonic = ptjsonchild.get<std::string> ( CHASSISS_CONFIG_ULTRASONIC, "" );
        /// 构造输出数据
        std::stringstream ssdest;
        boost::property_tree::ptree ptdest_root, ptdest_child;
        ptdest_root.put<std::string> ( CHASSISS_CONFIG_PROTECTOR, strprotector );
        ptdest_root.put<std::string> ( CHASSISS_CONFIG_ULTRASONIC, strultrasonic );
        ptdest_child.put<std::string> ( CHASSISS_DEVICE_ID, "" );
        ptdest_child.put<std::string> ( CHASSISS_DEVICE_POSE, "" );
        ptdest_child.put<std::string> ( CHASSISS_DEVICE_TYPE, "" );
        ptdest_child.put<std::string> ( CHASSISS_DEVICE_FREQ, "" );
        ptdest_child.put<std::string> ( CHASSISS_DEVICE_KIND, "" );
        ptdest_root.put_child ( CHASSISS_DEVICE_SENSORS, ptdest_child );
        boost::property_tree::write_json ( ssdest, ptdest_root );
        std::string strjsonresult = ssdest.str();
        GetBufferUtility()->CopyStringToBuffer ( strjsonresult, output_buffer_ptr );
        #ifdef _DEBUG
        YSOS_LOG_DEBUG ( "Output:" << strjsonresult );
        #endif
        return YSOS_ERROR_SUCCESS;
    }

    /************************************************************************/
    /*input_buffer_ptr:
    {
      "protector":"111111",
      "ultrasonic":"111111"
    }*/
    /************************************************************************/
    int GaussianChassisDriver::IoctlSetDeviceInfo ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlSetDeviceInfo" );
        UINT8 *inbufferptr = NULL;
        UINT32 inbuffersize = 0;
        if ( !is_sdp_connected_ )
        {
            YSOS_LOG_WARN ( "Chassis is not connected." );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == input_buffer_ptr || NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Input Or Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        input_buffer_ptr->GetBufferAndLength ( &inbufferptr, &inbuffersize );
        /// 解析输入数据
        std::string strbuffer = reinterpret_cast<char*> ( inbufferptr );
        if ( 0 == strbuffer.length() )
        {
            YSOS_LOG_ERROR ( "Input Length Error:0!!!!!!" );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        #ifdef _DEBUG
        YSOS_LOG_DEBUG ( "Input:" << strbuffer );
        #endif
        std::stringstream ss ( strbuffer );
        boost::property_tree::ptree pt;
        boost::property_tree::read_json ( ss, pt );
        std::string strprotector = pt.get<std::string> ( CHASSISS_CONFIG_PROTECTOR );
        std::string strultrasonic = pt.get<std::string> ( CHASSISS_CONFIG_ULTRASONIC );
        /// 设置PROTECTOR
        std::string strhttpobjname = HTTP_OBJECT_NAME_SET_PROTECTOR + strprotector;
        if ( YSOS_ERROR_SUCCESS != http_method_post ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) strhttpobjname.c_str(), NULL, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_post Fail." );
            return YSOS_ERROR_FAILED;
        }
        if ( YSOS_ERROR_SUCCESS != Check_Http_Result ( output_buffer_ptr, false ) )
        {
            YSOS_LOG_WARN ( "Check_Http_Result Fail." );
            return YSOS_ERROR_FAILED;
        }
        /// 设置UTRASONIC
        strhttpobjname = HTTP_OBJECT_NAME_SET_UTRASONIC + strultrasonic;
        if ( YSOS_ERROR_SUCCESS != http_method_post ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) strhttpobjname.c_str(), NULL, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_post Fail." );
            return YSOS_ERROR_FAILED;
        }
        if ( YSOS_ERROR_SUCCESS != Check_Http_Result ( output_buffer_ptr, false ) )
        {
            YSOS_LOG_WARN ( "Check_Http_Result Fail." );
            return YSOS_ERROR_FAILED;
        }
        return YSOS_ERROR_SUCCESS;
    }

    int GaussianChassisDriver::IoctlEnableHeartBeat ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlEnableHeartBeat" );
        const char* strhttpobjname = HTTP_OBJECT_NAME_STARTPINGCHECK;
        if ( !is_sdp_connected_ )
        {
            YSOS_LOG_WARN ( "Chassis is not connected." );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        if ( YSOS_ERROR_SUCCESS != http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) strhttpobjname, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_get Fail." );
            return YSOS_ERROR_FAILED;
        }
        if ( YSOS_ERROR_SUCCESS != Check_Http_Result ( output_buffer_ptr, false ) )
        {
            YSOS_LOG_WARN ( "Check_Http_Result Fail." );
            return YSOS_ERROR_FAILED;
        }
        is_enable_heart_ = true;
        return YSOS_ERROR_SUCCESS;
    }

    int GaussianChassisDriver::IoctlDisableHeartBeat ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlDisableHeartBeat" );
        const char* strhttpobjname = HTTP_OBJECT_NAME_STOPPINGCHECK;
        if ( !is_sdp_connected_ )
        {
            YSOS_LOG_WARN ( "Chassis is not connected." );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        if ( YSOS_ERROR_SUCCESS != http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) strhttpobjname, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_get Fail." );
            return YSOS_ERROR_FAILED;
        }
        if ( YSOS_ERROR_SUCCESS != Check_Http_Result ( output_buffer_ptr, false ) )
        {
            YSOS_LOG_WARN ( "Check_Http_Result Fail." );
            return YSOS_ERROR_FAILED;
        }
        is_enable_heart_ = false;
        return YSOS_ERROR_SUCCESS;
    }

    /************************************************************************************************************/
    /* input_buffer_ptr:
    {
      "mapname":"office",
      "positionname":"c1",
      "initdirect":true
    }*/
    /************************************************************************************************************/
    int GaussianChassisDriver::IoctlMapInitialize ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlMapInitialize" );
        if ( NULL == input_buffer_ptr || NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Input Or Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        char szhttpobjname[MAX_LENGTH256] = {'\0'};
        UINT8 *inbufferptr = NULL;
        UINT32 inbuffersize = 0;
        input_buffer_ptr->GetBufferAndLength ( &inbufferptr, &inbuffersize );
        Json::Reader js_reader;
        Json::Value js_value;
        if ( !js_reader.parse ( reinterpret_cast<char*> ( inbufferptr ), js_value, false ) )
        {
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        // load the map
        std::string strmapname = js_value[CHASSISS_MAP_NAME].asString();
        YSOS_LOG_DEBUG ( CHASSISS_MAP_NAME ": " << strmapname );
#ifdef _WIN32
        sprintf_s ( szhttpobjname, MAX_LENGTH256, HTTP_OBJECT_NAME_LOADMAP, strmapname.c_str() );
#else
        sprintf ( szhttpobjname, HTTP_OBJECT_NAME_LOADMAP, strmapname.c_str() );
#endif
        if ( YSOS_ERROR_SUCCESS != http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) szhttpobjname, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_get Fail." );
            return YSOS_ERROR_FAILED;
        }
        if ( YSOS_ERROR_SUCCESS != Check_Http_Result ( output_buffer_ptr, false ) )
        {
            YSOS_LOG_WARN ( "Check_Http_Result Fail." );
            return YSOS_ERROR_FAILED;
        }
        // wait to load the map
        SLEEP ( connect_sleep_time_ );
        std::string strpositionname = js_value[CHASSISS_MAP_POSITION_NAME].asString();
        YSOS_LOG_DEBUG ( CHASSISS_MAP_POSITION_NAME ": " << strpositionname );
        if ( strpositionname.empty() )
        {
            return YSOS_ERROR_SUCCESS;
        }
        // rolocate
        bool direct = js_value[CHASSISS_MAP_INITDIRECT].asBool();
#ifdef _WIN32
        sprintf_s ( szhttpobjname, MAX_LENGTH256, ( direct ? HTTP_OBJECT_NAME_DIRECT_INIT : HTTP_OBJECT_NAME_INIT ),
                    strmapname.c_str(), strpositionname.c_str() );
#else
        sprintf ( szhttpobjname, ( direct ? HTTP_OBJECT_NAME_DIRECT_INIT : HTTP_OBJECT_NAME_INIT ),
                    strmapname.c_str(), strpositionname.c_str() );
#endif
        if ( YSOS_ERROR_SUCCESS != http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) szhttpobjname, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_get Fail." );
            return YSOS_ERROR_FAILED;
        }
        if ( YSOS_ERROR_SUCCESS != Check_Http_Result ( output_buffer_ptr, false ) )
        {
            YSOS_LOG_WARN ( "Check_Http_Result Fail." );
            return YSOS_ERROR_FAILED;
        }
        // check init finish
        int i = 0;
        for ( i = 0; i < 30; i++ )
        {
            if ( YSOS_ERROR_SUCCESS == IoctlInitFinish ( input_buffer_ptr, output_buffer_ptr ) )
            {
                break;
            }
            SLEEP ( 2000 );
        }
        if ( i == 30 )
        {
            YSOS_LOG_WARN ( "Init Not Finish." );
            return YSOS_ERROR_FAILED;
        }
        return YSOS_ERROR_SUCCESS;
    }

    /************************************************************************/
    /*
    input_buffer_ptr:{"mapname":"office" }
    output_buffer_ptr:地图png数据
    */
    /************************************************************************/
    int GaussianChassisDriver::IoctlGetMapInfo ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlGetMapInfo" );
        int iret = YSOS_ERROR_FAILED;
        if ( NULL == input_buffer_ptr || NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Input Or Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        iret = IoctlGetMapPicture ( input_buffer_ptr, output_buffer_ptr );
        return iret;
    }

    /************************************************************************/
    /*
    input_buffer_ptr:
    {
      "type":"0",
      "method":"0",
      "mapname":"office",
      "positionname":"p",
      "newname":"p1",
      "originname":"p0"
    }
    */
    /************************************************************************/
    int GaussianChassisDriver::IoctlSetMapInfo ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlSetMapInfo" );
        int iret = YSOS_ERROR_FAILED;
        UINT8 *inbufferptr = NULL;
        UINT32 inbuffersize = 0;
        char szhttpobjname[MAX_LENGTH256] = {'\0'};
        if ( NULL == input_buffer_ptr || NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Input Or Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        input_buffer_ptr->GetBufferAndLength ( &inbufferptr, &inbuffersize );
        std::string strbuffer = reinterpret_cast<char*> ( inbufferptr );
        if ( 0 == strbuffer.length() )
        {
            YSOS_LOG_ERROR ( "Input Length Error:0!!!!!!" );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        #ifdef _DEBUG
        YSOS_LOG_DEBUG ( "Input:" << strbuffer );
        #endif
        std::stringstream ss ( strbuffer );
        boost::property_tree::ptree pt;
        boost::property_tree::read_json ( ss, pt );
        int itypeid = pt.get<int> ( CHASSISS_DEVICE_TYPE );
        int imethod = pt.get<int> ( CHASSISS_DEVICE_METHOD );
        std::string strmapname = pt.get<std::string> ( CHASSISS_MAP_NAME );
        std::string strpositionname = pt.get<std::string> ( CHASSISS_MAP_POSITION_NAME );
        std::string strnewname = pt.get<std::string> ( CHASSISS_MAP_NEW_NAME );
        std::string stroriginname = pt.get<std::string> ( CHASSISS_MAP_ORIGIN_NAME );
        if ( PROP_TYPE_MAP != itypeid || PROP_TYPE_MAP_SCAN != itypeid )
        {
            YSOS_LOG_ERROR ( "Parameter Error." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        switch ( itypeid )
        {
            case PROP_TYPE_MAP:
                {
                    if ( PROP_METHOD_DEL == imethod )
                    {
                        /// 构造输入Json数据
                        std::stringstream ssdest;
                        boost::property_tree::ptree ptdest_root;
                        ptdest_root.put<std::string> ( CHASSISS_MAP_NAME, strmapname );
                        boost::property_tree::write_json ( ssdest, ptdest_root );
                        std::string strjson_result = ssdest.str();
                        GetBufferUtility()->CopyStringToBuffer ( strjson_result, input_buffer_ptr );
                        iret = IoctlDelMap ( input_buffer_ptr, output_buffer_ptr );
                    }
                    else if ( PROP_METHOD_RENAME == imethod )
                    {
                        /// 构造输入Json数据
                        std::stringstream ssdest;
                        boost::property_tree::ptree ptdest_root;
                        ptdest_root.put<std::string> ( CHASSISS_MAP_NAME, strmapname );
                        ptdest_root.put<std::string> ( CHASSISS_MAP_ORIGIN_NAME, stroriginname );
                        boost::property_tree::write_json ( ssdest, ptdest_root );
                        std::string strjson_result = ssdest.str();
                        GetBufferUtility()->CopyStringToBuffer ( strjson_result, input_buffer_ptr );
                        iret = IoctlRenameMap ( input_buffer_ptr, output_buffer_ptr );
                    }
                }
                break;
            case PROP_TYPE_MAP_SCAN:
                {
                    if ( PROP_METHOD_START == imethod )
                    {
                        /// 构造输入Json数据
                        std::stringstream ssdest;
                        boost::property_tree::ptree ptdest_root;
                        ptdest_root.put<std::string> ( CHASSISS_MAP_NAME, strmapname );
                        boost::property_tree::write_json ( ssdest, ptdest_root );
                        std::string strjson_result = ssdest.str();
                        GetBufferUtility()->CopyStringToBuffer ( strjson_result, input_buffer_ptr );
                        iret = IoctlStartScanMap ( input_buffer_ptr, output_buffer_ptr );
                    }
                    else if ( PROP_METHOD_STOP == imethod )
                    {
                        iret = IoctlStopScanMap ( input_buffer_ptr, output_buffer_ptr );
                    }
                    else if ( PROP_METHOD_CANCEL == imethod )
                    {
                        iret = IoctlCancelScanMap ( input_buffer_ptr, output_buffer_ptr );
                    }
                }
                break;
            default:
                {
                    YSOS_LOG_WARN ( "In Default.No Support This Type!" );
                }
                break;
        }
        return iret;
    }

    /************************************************************************/
    /*
    input_buffer_ptr:
    {
      "mapname":"office"
    }
    */
    /************************************************************************/
    int GaussianChassisDriver::IoctlGetVirtualObstacle ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlGetVirtualObstacle" );
        UINT8 *inbufferptr = NULL;
        UINT32 inbuffersize = 0;
        std::string strhttpobjname = HTTP_OBJECT_NAME_GET_OBSTACLE;
        if ( !is_sdp_connected_ )
        {
            YSOS_LOG_WARN ( "Chassis is not connected." );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Input Or Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        if ( NULL != input_buffer_ptr )
        {
            input_buffer_ptr->GetBufferAndLength ( &inbufferptr, &inbuffersize );
            std::string strbuffer = reinterpret_cast<char*> ( inbufferptr );
            if ( 0 == strbuffer.length() )
            {
                YSOS_LOG_ERROR ( "Input Length Error:0!!!!!!" );
                return YSOS_ERROR_INVALID_ARGUMENTS;
            }
            #ifdef _DEBUG
            YSOS_LOG_DEBUG ( "Input:" << strbuffer );
            #endif
            std::stringstream ss ( strbuffer );
            boost::property_tree::ptree pt;
            boost::property_tree::read_json ( ss, pt );
            std::string strhttpmapname = pt.get<std::string> ( CHASSISS_MAP_NAME, connect_map_ );
            strhttpobjname += strhttpmapname;
        }
        else
        {
            strhttpobjname += connect_map_;
        }
        if ( YSOS_ERROR_SUCCESS != http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) strhttpobjname.c_str(), output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_get Fail." );
            return YSOS_ERROR_FAILED;
        }
        return YSOS_ERROR_SUCCESS;
    }

    int GaussianChassisDriver::IoctlSetVirtualObstacle ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlSetVirtualObstacle" );
        UINT8 *inbufferptr = NULL;
        UINT32 inbuffersize = 0;
        const char* strhttpobjname = HTTP_OBJECT_NAME_UPDATE_OBSTACLE;
        if ( !is_sdp_connected_ )
        {
            YSOS_LOG_WARN ( "Chassis is not connected." );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == input_buffer_ptr || NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Input Or Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        input_buffer_ptr->GetBufferAndLength ( &inbufferptr, &inbuffersize );
        if ( YSOS_ERROR_SUCCESS != http_method_post ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) strhttpobjname, inbufferptr, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_post Fail." );
            return YSOS_ERROR_FAILED;
        }
        return YSOS_ERROR_SUCCESS;
    }


    /************************************************************************/
    /*
    input_buffer_ptr:
    {
      "type":"0",
      "method":"0",
      "mapname":"office",
      "positionname":"p",
      "pathname":"p1",
      "originname":"p0"
    }
    */
    /************************************************************************/
    int GaussianChassisDriver::IoctlSetPredefinedPosition ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlSetPredefinedPosition" );
        UINT8 *inbufferptr = NULL;
        UINT32 inbuffersize = 0;
        char szhttpobjname[MAX_LENGTH256] = {'\0'};
        if ( !is_sdp_connected_ )
        {
            YSOS_LOG_WARN ( "Chassis is not connected." );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == input_buffer_ptr || NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Input Or Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        input_buffer_ptr->GetBufferAndLength ( &inbufferptr, &inbuffersize );
        /// 解析输入数据
        std::string strbuffer = reinterpret_cast<char*> ( inbufferptr );
        if ( 0 == strbuffer.length() )
        {
            YSOS_LOG_ERROR ( "Input Length Error:0!!!!!!" );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        #ifdef _DEBUG
        YSOS_LOG_DEBUG ( "Input:" << strbuffer );
        #endif
        std::stringstream ss ( strbuffer );
        boost::property_tree::ptree pt;
        boost::property_tree::read_json ( ss, pt );
        int itypeid = pt.get<int> ( CHASSISS_DEVICE_TYPE );
        int imethod = pt.get<int> ( CHASSISS_DEVICE_METHOD );
        std::string strmapname = pt.get<std::string> ( CHASSISS_MAP_NAME );
        std::string strpositionname = pt.get<std::string> ( CHASSISS_MAP_POSITION_NAME );
        std::string strpathname = pt.get<std::string> ( CHASSISS_MAP_PATH_NAME );
        std::string stroriginname = pt.get<std::string> ( CHASSISS_MAP_ORIGIN_NAME );
        if ( PROP_TYPE_POSITION == itypeid )   ///< POSITION
        {
#ifdef _WIN32            
            if ( PROP_METHOD_ADD == imethod )
            {
                sprintf_s ( szhttpobjname, MAX_LENGTH256, HTTP_OBJECT_NAME_ADD_POS, strpositionname.c_str() );
            }
            else if ( PROP_METHOD_DEL == imethod )
            {
                sprintf_s ( szhttpobjname, MAX_LENGTH256, HTTP_OBJECT_NAME_DEL_POS, strmapname.c_str(), strpositionname.c_str() );
            }
            else
            {
                YSOS_LOG_ERROR ( "Parameter Error." );
                return YSOS_ERROR_INVALID_ARGUMENTS;
            }
        }
        else if ( PROP_TYPE_PATH == itypeid )  ///< PATH
        {
            if ( PROP_METHOD_RENAME == imethod )
            {
                sprintf_s ( szhttpobjname, MAX_LENGTH256, HTTP_OBJECT_NAME_RENAME_PATH, strmapname.c_str(), stroriginname.c_str(), strpathname.c_str() );
            }
            else if ( PROP_METHOD_DEL == imethod )
            {
                sprintf_s ( szhttpobjname, MAX_LENGTH256, HTTP_OBJECT_NAME_DEL_PATH, strmapname.c_str(), strpathname.c_str() );
            }
            else
            {
                YSOS_LOG_ERROR ( "Parameter Error." );
                return YSOS_ERROR_INVALID_ARGUMENTS;
            }
        }
        else
        {
            YSOS_LOG_ERROR ( "Parameter Error." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
#else
        if ( PROP_METHOD_ADD == imethod )
            {
                sprintf ( szhttpobjname, HTTP_OBJECT_NAME_ADD_POS, strpositionname.c_str() );
            }
            else if ( PROP_METHOD_DEL == imethod )
            {
                sprintf ( szhttpobjname, HTTP_OBJECT_NAME_DEL_POS, strmapname.c_str(), strpositionname.c_str() );
            }
            else
            {
                YSOS_LOG_ERROR ( "Parameter Error." );
                return YSOS_ERROR_INVALID_ARGUMENTS;
            }
        }
        else if ( PROP_TYPE_PATH == itypeid )  ///< PATH
        {
            if ( PROP_METHOD_RENAME == imethod )
            {
                sprintf ( szhttpobjname, HTTP_OBJECT_NAME_RENAME_PATH, strmapname.c_str(), stroriginname.c_str(), strpathname.c_str() );
            }
            else if ( PROP_METHOD_DEL == imethod )
            {
                sprintf ( szhttpobjname, HTTP_OBJECT_NAME_DEL_PATH, strmapname.c_str(), strpathname.c_str() );
            }
            else
            {
                YSOS_LOG_ERROR ( "Parameter Error." );
                return YSOS_ERROR_INVALID_ARGUMENTS;
            }
        }
        else
        {
            YSOS_LOG_ERROR ( "Parameter Error." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
#endif
        if ( YSOS_ERROR_SUCCESS != http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) szhttpobjname, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_get Fail." );
            return YSOS_ERROR_FAILED;
        }
        if ( YSOS_ERROR_SUCCESS != Check_Http_Result ( output_buffer_ptr, false ) )
        {
            YSOS_LOG_WARN ( "Check_Http_Result Fail." );
            return YSOS_ERROR_FAILED;
        }
        return YSOS_ERROR_SUCCESS;
    }

    /************************************************************************/
    /*
    input_buffer_ptr:
    {
      "type":"0",
      "mapname":"office"
    }
    */
    /************************************************************************/
    int GaussianChassisDriver::IoctlGetPredefinedPosition ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlGetPredefinedPosition" );
        UINT8 *inbufferptr = NULL;
        UINT32 inbuffersize = 0;
        char szhttpobjname[MAX_LENGTH256] = {'\0'};
        if ( !is_sdp_connected_ )
        {
            YSOS_LOG_WARN ( "Chassis is not connected." );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == input_buffer_ptr || NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Input Or Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        input_buffer_ptr->GetBufferAndLength ( &inbufferptr, &inbuffersize );
        /// 解析输入数据
        std::string strbuffer = reinterpret_cast<char*> ( inbufferptr );
        if ( 0 == strbuffer.length() )
        {
            YSOS_LOG_ERROR ( "Input Length Error:0!!!!!!" );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        #ifdef _DEBUG
        YSOS_LOG_DEBUG ( "Input:" << strbuffer );
        #endif
        std::stringstream ss ( strbuffer );
        boost::property_tree::ptree pt;
        boost::property_tree::read_json ( ss, pt );
        int itypeid = pt.get<int> ( CHASSISS_DEVICE_TYPE );
        std::string strmapname = pt.get<std::string> ( CHASSISS_MAP_NAME );
#ifdef _WIN32
        if ( PROP_TYPE_POSITION == itypeid )
        {
            sprintf_s ( szhttpobjname, MAX_LENGTH256, HTTP_OBJECT_NAME_GET_POS, strmapname.c_str() );
        }
        else if ( PROP_TYPE_PATH == itypeid )
        {
            sprintf_s ( szhttpobjname, MAX_LENGTH256, HTTP_OBJECT_NAME_GET_PATH, strmapname.c_str() );
        }
        else
        {
            YSOS_LOG_ERROR ( "Parameter Error." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
#else
        if ( PROP_TYPE_POSITION == itypeid )
        {
            sprintf ( szhttpobjname, HTTP_OBJECT_NAME_GET_POS, strmapname.c_str() );
        }
        else if ( PROP_TYPE_PATH == itypeid )
        {
            sprintf ( szhttpobjname, HTTP_OBJECT_NAME_GET_PATH, strmapname.c_str() );
        }
        else
        {
            YSOS_LOG_ERROR ( "Parameter Error." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }

#endif
        if ( YSOS_ERROR_SUCCESS != http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) szhttpobjname, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_get Fail." );
            return YSOS_ERROR_FAILED;
        }
        if ( YSOS_ERROR_SUCCESS != Check_Http_Result ( output_buffer_ptr, true ) )
        {
            YSOS_LOG_WARN ( "Check_Http_Result Fail." );
            return YSOS_ERROR_FAILED;
        }
        return YSOS_ERROR_SUCCESS;
    }


    /************************************************************************/
    /* input_buffer_ptr: {"distance":"5.0","angle":"30.0"}
       distance:障碍物检测距离
       angle:障碍物检测角度
    */
    /************************************************************************/
    int GaussianChassisDriver::IoctlSetObstacleDetectionRange ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlSetObstacleDetectionRange" );
        UINT8 *inbufferptr = NULL;
        UINT32 inbuffersize = 0;
        if ( NULL == input_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Input Buffer Exception" );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        input_buffer_ptr->GetBufferAndLength ( &inbufferptr, &inbuffersize );
        std::string strbuffer = reinterpret_cast<char*> ( inbufferptr );
        if ( 0 == strbuffer.length() )
        {
            YSOS_LOG_ERROR ( "Input Length Error:0!!!!!!" );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        #ifdef _DEBUG
        YSOS_LOG_DEBUG ( "Input:" << strbuffer );
        #endif
        std::stringstream ss ( strbuffer );
        boost::property_tree::ptree pt;
        boost::property_tree::read_json ( ss, pt );
        std::string distance_str = pt.get<std::string> ( CHASSISS_POS_DISTANCE );
        std::string angle_str = pt.get<std::string> ( CHASSISS_POS_ANGLE );
        obstacle_detection_range_ = ( float ) atof ( distance_str.c_str() );
        obstacle_detection_angle_ = ( float ) atof ( angle_str.c_str() );
        #ifdef _DEBUG
        YSOS_LOG_DEBUG ( "obstacle_detection_range_:" << obstacle_detection_range_ << "obstacle_detection_angle_" << obstacle_detection_angle_ );
        #endif
        return YSOS_ERROR_SUCCESS;
    }


    /************************************************************************/
    /* output_buffer_ptr: {"distance":"5.0","angle":"30.0"}
       distance:障碍物检测距离
       angle:障碍物检测角度
    */
    /************************************************************************/
    int GaussianChassisDriver::IoctlGetObstacleDetectionRange ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlGetObstacleDetectionRange" );
        if ( NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Output Buffer Exception" );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        /// 构造输出数据
#ifdef _WIN32
        char szbuf[MAX_LENGTH16] = {'\0'};
        sprintf_s ( szbuf, MAX_LENGTH16, "%.1f", obstacle_detection_range_ );
        std::string distance_str = szbuf;
        sprintf_s ( szbuf, MAX_LENGTH16, "%.1f", obstacle_detection_angle_ );
#else
        char szbuf[MAX_LENGTH16] = {'\0'};
        sprintf ( szbuf, "%.1f", obstacle_detection_range_ );
        std::string distance_str = szbuf;
        sprintf ( szbuf, "%.1f", obstacle_detection_angle_ );
#endif
        std::string angle_str = szbuf;
        /// 构造输出Json数据
        std::stringstream ssdest;
        boost::property_tree::ptree ptdest_root;
        ptdest_root.put<std::string> ( CHASSISS_POS_DISTANCE, distance_str );
        ptdest_root.put<std::string> ( CHASSISS_POS_ANGLE, angle_str );
        boost::property_tree::write_json ( ssdest, ptdest_root );
        std::string strjson_result = ssdest.str();
        GetBufferUtility()->CopyStringToBuffer ( strjson_result, output_buffer_ptr );
        #ifdef _DEBUG
        YSOS_LOG_DEBUG ( "Get Obstacle Detection Output Data:" << strjson_result );
        #endif
        return YSOS_ERROR_SUCCESS;
    }


    /************************************************************************/
    /* output_buffer_ptr: {"speed":"high","linearSpeed":"0.6","angularSpeed":"0.1"}
       speed: 底盘速度，分为low/medium/high
    */
    /************************************************************************/
    int GaussianChassisDriver::IoctlGetMoveSpeed ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlGetMoveSpeed" );
        if ( NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Output Buffer Exception" );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        /// 构造输出数据
#ifdef _WIN32
        char szbuf[MAX_LENGTH16] = {'\0'};
        sprintf_s ( szbuf, MAX_LENGTH16, "%.1f", speed_line_ );
        std::string strspeedline = szbuf;
        sprintf_s ( szbuf, MAX_LENGTH16, "%.1f", speed_angular_ );
#else
        char szbuf[MAX_LENGTH16] = {'\0'};
        sprintf ( szbuf, "%.1f", speed_line_ );
        std::string strspeedline = szbuf;
        sprintf ( szbuf, "%.1f", speed_angular_ );
#endif
        std::string strspeedangular = szbuf;
        /// 构造输出Json数据
        std::stringstream ssdest;
        boost::property_tree::ptree ptdest_root;
        ptdest_root.put<std::string> ( CHASSISS_SPEED, "" );
        ptdest_root.put<std::string> ( CHASSISS_SPEED_LINE, strspeedline );
        ptdest_root.put<std::string> ( CHASSISS_SPEED_ANGULAR, strspeedangular );
        boost::property_tree::write_json ( ssdest, ptdest_root );
        std::string strjson_result = ssdest.str();
        GetBufferUtility()->CopyStringToBuffer ( strjson_result, output_buffer_ptr );
        #ifdef _DEBUG
        YSOS_LOG_DEBUG ( "Get Obstacle Detection Output Data:" << strjson_result );
        #endif
        return YSOS_ERROR_SUCCESS;
    }

    /************************************************************************/
    /* input_buffer_ptr: {"speed":"high","linearSpeed":"0.6","angularSpeed":"0.1"}
       speed: 底盘速度，分为low/medium/high
    */
    /************************************************************************/
    int GaussianChassisDriver::IoctlSetMoveSpeed ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlSetMoveSpeed" );
        UINT8 *inbufferptr = NULL;
        UINT32 inbuffersize = 0;
        const char* strhttpobjname = HTTP_OBJECT_NAME_SPEED;
        if ( !is_sdp_connected_ )
        {
            YSOS_LOG_WARN ( "Chassis is not connected." );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == input_buffer_ptr || NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Output Buffer Exception" );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        input_buffer_ptr->GetBufferAndLength ( &inbufferptr, &inbuffersize );
        /// 解析输入数据
        std::string strbuffer = reinterpret_cast<char*> ( inbufferptr );
        if ( 0 == strbuffer.length() )
        {
            YSOS_LOG_ERROR ( "Input Length Error:0!!!!!!" );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        #ifdef _DEBUG
        YSOS_LOG_DEBUG ( "Input:" << strbuffer );
        #endif
        std::stringstream ss ( strbuffer );
        boost::property_tree::ptree pt;
        boost::property_tree::read_json ( ss, pt );
        std::string strspeedline = pt.get<std::string> ( CHASSISS_SPEED_LINE );
        std::string strspeedangular = pt.get<std::string> ( CHASSISS_SPEED_ANGULAR );
        /// 构造发送Json数据
        /*std::stringstream ssource;
        boost::property_tree::ptree ptsource_root;
        boost::property_tree::ptree ptsource_child;
        ptsource_child.put<std::string>(CHASSISS_SPEED_LINE, strspeedline);
        ptsource_child.put<std::string>(CHASSISS_SPEED_ANGULAR, strspeedangular);
        ptsource_child.put<float>(CHASSISS_SPEED_LINE, 0);
        ptsource_child.put<float>(CHASSISS_SPEED_ANGULAR, 0.1);
        ptsource_root.put_child(CHASSISS_SPEED,ptsource_child);
        boost::property_tree::write_json(ssource, ptsource_root);
        std::string strposjson = ssource.str();*/
        Json::Value json_child_speed;
        Json::Value json_value;
        Json::FastWriter writer;
        json_child_speed[CHASSISS_SPEED_LINE] = atof ( strspeedline.c_str() );
        json_child_speed[CHASSISS_SPEED_ANGULAR] = atof ( strspeedangular.c_str() );;
        json_value[CHASSISS_SPEED] = json_child_speed;
        std::string strposjson = writer.write ( json_value );
        strposjson = GetUtility()->ReplaceAllDistinct ( strposjson, "\\r\\n", "" );
        if ( YSOS_ERROR_SUCCESS != http_method_post ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) strhttpobjname, ( UINT8* ) strposjson.c_str(), output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_post Fail." );
            return YSOS_ERROR_FAILED;
        }
        /// 解析接收Json数据
        UINT8 *outbufferptr = GetBufferUtility()->GetBufferData ( output_buffer_ptr );
        if ( NULL == outbufferptr )
        {
            YSOS_LOG_ERROR ( "Output Buffer is NULL." );
            return YSOS_ERROR_FAILED;
        }
        std::string strresult = reinterpret_cast<char*> ( outbufferptr );
        if ( 0 == strresult.length() )
        {
            YSOS_LOG_WARN ( "HTTP Return Length is 0!" );
            return YSOS_ERROR_FAILED;
        }
        #ifdef _DEBUG
        YSOS_LOG_DEBUG ( "Http Post Result:" << strresult );
        #endif
        boost::property_tree::ptree ptjson;
        std::istringstream stream ( strresult );
        boost::property_tree::json_parser::read_json ( stream, ptjson );
        std::string ret_successed = ptjson.get<std::string> ( RESULT_SUCCESS, "" );
        if ( 0 != ret_successed.compare ( RESULT_TRUE ) )
        {
            std::string ret_errorcode = ptjson.get<std::string> ( RESULT_ERRCODE, "" );
            YSOS_LOG_WARN ( "Set Move Speed Fail, the ErrorCode is:" << ret_errorcode.c_str() );
            return YSOS_ERROR_FAILED;
        }
        speed_line_ = ( float ) atof ( strspeedline.c_str() );
        speed_angular_ = ( float ) atof ( strspeedangular.c_str() );
        return YSOS_ERROR_SUCCESS;
    }

    int GaussianChassisDriver::IoctlSendHeartBeat ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlSendHeartBeat" );
        const char* strhttpobjname = HTTP_OBJECT_NAME_PINGCHASSIS;
        if ( !is_sdp_connected_ )
        {
            YSOS_LOG_WARN ( "Chassis is not connected." );
            return YSOS_ERROR_FAILED;
        }
        if ( !is_enable_heart_ )
        {
            YSOS_LOG_DEBUG ( "Chassis  disable heart." );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Output Buffer Exception" );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        if ( YSOS_ERROR_SUCCESS != http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) strhttpobjname, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_get Fail." );
            return YSOS_ERROR_FAILED;
        }
        if ( YSOS_ERROR_SUCCESS != Check_Http_Result ( output_buffer_ptr, false ) )
        {
            YSOS_LOG_WARN ( "Check_Http_Result Fail." );
            return YSOS_ERROR_FAILED;
        }
        return YSOS_ERROR_SUCCESS;
    }

    /*************************************************************************/
    /* output_buffer_ptr: {"battery":60,"charging":true}
    */
    /************************************************************************/
    int GaussianChassisDriver::IoctlGetDeviceStatus ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlGetDeviceStatus" );
        if ( !is_websocket_connect_device_ )
        {
            YSOS_LOG_WARN ( "WebSocket not connect device status" );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Output Buffer Exception" );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        /// 获取数据
        int ibattery = socket_endpoint_.get_metadata ( WEBSOCKET_DEVICE_ID )->get_battery_status();
        bool bcharge = socket_endpoint_.get_metadata ( WEBSOCKET_DEVICE_ID )->get_charge_status();
        bool bemergencystop = socket_endpoint_.get_metadata ( WEBSOCKET_DEVICE_ID )->get_emergencystopstatus_status();
        YSOS_LOG_DEBUG ( "bemergencystop:" << bemergencystop );
        if ( ibattery < 0 )
        {
            return YSOS_ERROR_FAILED;
        }
        /// 构造返回Json数据
        UINT8* buffer_ptr = NULL;
        UINT32 buffer_length = 0;
        output_buffer_ptr->GetBufferAndLength ( &buffer_ptr, &buffer_length );
        sprintf ( reinterpret_cast<char*> ( buffer_ptr ), "{\"battery\":%d,\"charging\":%s,\"emergencystop\":%s}", ibattery, bcharge ? "true" : "false", bemergencystop ? "true" : "false" );
        YSOS_LOG_DEBUG ( "Output:" << reinterpret_cast<char*> ( buffer_ptr ) );
        return YSOS_ERROR_SUCCESS;
    }


    /************************************************************************/
    /* output_buffer_ptr:
    {
      "MCUConnection":true,
      "laserConnection":true,
      "routerConnection":true,
      "error":[{"msg":"1|2|3|4|5|asdfafa"}]
    }
    */
    /************************************************************************/
    int GaussianChassisDriver::IoctlGetHealthStatus ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlGetHealthStatus" );
        if ( !is_websocket_connect_health_ )
        {
            YSOS_LOG_WARN ( "WebSocket not connect health status" );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Output Buffer Exception" );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        /// 获取数据
        bool bmcuconnetstatus = socket_endpoint_.get_metadata ( WEBSOCKET_SYSHEALTH_ID )->get_mcu_status();
        bool blaserconnetstatus = socket_endpoint_.get_metadata ( WEBSOCKET_SYSHEALTH_ID )->get_laser_status();
        bool brouterconnetstatus = socket_endpoint_.get_metadata ( WEBSOCKET_SYSHEALTH_ID )->get_router_status();
        /// 构造返回Json数据
        std::stringstream ssource;
        boost::property_tree::ptree ptsource_root, ptsource_child, ptsource_children;
        ptsource_root.put<bool> ( CHASSISS_HEALTH_STATUS_CONNECT_MCU, bmcuconnetstatus );
        ptsource_root.put<bool> ( CHASSISS_HEALTH_STATUS_CONNECT_LASER, blaserconnetstatus );
        ptsource_root.put<bool> ( CHASSISS_HEALTH_STATUS_CONNECT_ROUTER, brouterconnetstatus );
        ptsource_children.put ( RESULT_MESSAGE, "" );
        ptsource_child.push_back ( std::make_pair ( "", ptsource_children ) );
        ptsource_root.put_child ( RESULT_ERROR, ptsource_child );
        boost::property_tree::write_json ( ssource, ptsource_root );
        std::string strjsonresult = ssource.str();
        GetBufferUtility()->CopyStringToBuffer ( strjsonresult, output_buffer_ptr );
        #ifdef _DEBUG
        YSOS_LOG_DEBUG ( "Output:" << strjsonresult );
        #endif
        return YSOS_ERROR_SUCCESS;
    }


    /************************************************************************/
    /* output_buffer_ptr:
    {"x":3.1,"y":1.2,"angle":30.5,"quality":8}
    */
    /************************************************************************/
    int GaussianChassisDriver::IoctlGetLocalizationStatus ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlGetLocalizationStatus" );
        const char* strhttpobjname = HTTP_OBJECT_NAME_GETCURPOS;
        if ( !is_sdp_connected_ )
        {
            YSOS_LOG_WARN ( "Chassis is not connected." );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        if ( YSOS_ERROR_SUCCESS != http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) strhttpobjname, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_get Fail." );
            return YSOS_ERROR_FAILED;
        }
        /// 解析Json数据
        UINT8 *outbufferptr = GetBufferUtility()->GetBufferData ( output_buffer_ptr );
        if ( NULL == outbufferptr )
        {
            YSOS_LOG_ERROR ( "Output Buffer is NULL." );
            return YSOS_ERROR_FAILED;
        }
        std::string strresult = reinterpret_cast<char*> ( outbufferptr );
        if ( 0 == strresult.length() )
        {
            YSOS_LOG_WARN ( "HTTP Return Length is 0!" );
            return YSOS_ERROR_FAILED;
        }
        #ifdef _DEBUG
        YSOS_LOG_DEBUG ( "Http Get Result:" << strresult );
        #endif
        Json::Reader js_reader;
        Json::Value js_value;
        if ( !js_reader.parse ( strresult, js_value, false ) )
        {
            return YSOS_ERROR_FAILED;
        }
        double fposangle = js_value[CHASSISS_POS_ANGLE].asDouble();
        double fposx = js_value[CHASSISS_POS_GRID][CHASSISS_POS_X].asDouble();
        double fposy = js_value[CHASSISS_POS_GRID][CHASSISS_POS_Y].asDouble();
        /// 构造返回Json数据
        UINT8* buffer_ptr = NULL;
        UINT32 buffer_length = 0;
        output_buffer_ptr->GetBufferAndLength ( &buffer_ptr, &buffer_length );
        sprintf ( reinterpret_cast<char*> ( buffer_ptr ),
                  "{\"x\":%lf,\"y\":%lf,\"angle\":%lf,\"quality\":8}",
                  fposx, fposy, fposangle );
        #ifdef _DEBUG
        YSOS_LOG_DEBUG ( "Get Current Location:" << reinterpret_cast<char*> ( buffer_ptr ) );
        #endif
        return YSOS_ERROR_SUCCESS;
    }

    /************************************************************************/
    /* output_buffer_ptr: {"status":"moving/finish/unreach/null/warning"}           */
    /************************************************************************/

    int GaussianChassisDriver::IoctlGetNavigationStatus ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        if ( !is_websocket_connect_navigation_ )
        {
            YSOS_LOG_WARN ( "WebSocket not connect navigation status" );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Output Buffer Exception!" );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        std::string strnavigationstatus = socket_endpoint_.get_metadata ( WEBSOCKET_NAVIGATION_ID )->get_move_latest_status();
        YSOS_LOG_DEBUG ( "Device Return Navigation Status:" << strnavigationstatus << "!!!!!!!!" );
        /// 构造返回Json数据
        std::string strstatus;
        if ( strnavigationstatus == NOTICE_TYPE_INFO_PLANING )
        {
            strstatus = CHASSISS_NAVIGATION_STATUS_NULL;
        }
        else if ( strnavigationstatus == NOTICE_TYPE_INFO_HEADING )
        {
            strstatus = CHASSISS_NAVIGATION_STATUS_MOVING;
        }
        else if ( strnavigationstatus == NOTICE_TYPE_INFO_REACHED )
        {
            strstatus = CHASSISS_NAVIGATION_STATUS_FINISH;
        }
        else if ( strnavigationstatus == NOTICE_TYPE_INFO_UNREACHABLE || strnavigationstatus == NOTICE_TYPE_INFO_UNREACHED )
        {
            strstatus = CHASSISS_NAVIGATION_STATUS_UNREACH;
        }
        else if ( strnavigationstatus == NOTICE_TYPE_WARN_CLOSEOBSTACLE )
        {
            strstatus = CHASSISS_NAVIGATION_STATUS_WARN_OBS;
        }
        else if ( strnavigationstatus == NOTICE_TYPE_WARN_GOAL_NOSAFE )
        {
            strstatus = CHASSISS_NAVIGATION_STATUS_WARN_AIM;
        }
        else if ( strnavigationstatus == NOTICE_TYPE_WARN_LOCAL_FAIL )
        {
            strstatus = CHASSISS_NAVIGATION_STATUS_UNREACH;
        }
        else
        {
        }
        UINT8* buffer_ptr = NULL;
        UINT32 buffer_length = 0;
        output_buffer_ptr->GetBufferAndLength ( &buffer_ptr, &buffer_length );
        sprintf ( reinterpret_cast<char*> ( buffer_ptr ),
                  "{\"status\":\"%s\"}", strstatus.c_str() );
        #ifdef _DEBUG
        YSOS_LOG_DEBUG ( "Now Navigation Status:" << reinterpret_cast<char*> ( buffer_ptr ) << "!!!!!!!!" );
        #endif
        return YSOS_ERROR_SUCCESS;
    }


    /************************************************************************/
    /*
    input_buffer_ptr:
    {
      "type":"0",
      "id":"0"
    }
    */
    /************************************************************************/
    int GaussianChassisDriver::IoctlGetSensorData ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlGetSensorData" );
        int iret = YSOS_ERROR_FAILED;
        UINT8 *inbufferptr = NULL;
        UINT32 inbuffersize = 0;
        if ( !is_sdp_connected_ )
        {
            YSOS_LOG_WARN ( "Chassis is not connected." );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == output_buffer_ptr || NULL == input_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Input Or Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        input_buffer_ptr->GetBufferAndLength ( &inbufferptr, &inbuffersize );
        std::string strresult = reinterpret_cast<char*> ( inbufferptr );
        if ( 0 == strresult.length() )
        {
            YSOS_LOG_ERROR ( "Input Length Error:0!!!!!!" );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        #ifdef _DEBUG
        YSOS_LOG_DEBUG ( "Input:" << strresult );
        #endif
        boost::property_tree::ptree ptjson;
        std::istringstream stream ( strresult );
        boost::property_tree::json_parser::read_json ( stream, ptjson );
        int itype = ptjson.get<int> ( CHASSISS_DEVICE_TYPE, 0 );
        if ( TDSensor_Laser == itype )
        {
            iret = IoctlGetLaserInfo ( input_buffer_ptr, output_buffer_ptr );
        }
        else if ( TDSensor_Ultrasonic == itype )
        {
            iret = IoctlGetUltrasonicInfo ( input_buffer_ptr, output_buffer_ptr );
        }
        else if ( TDSensor_Protector == itype )
        {
            iret = IoctlGetProtectorInfo ( input_buffer_ptr, output_buffer_ptr );
        }
        return iret;
    }

    int GaussianChassisDriver::IoctlGoHome ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlGoHome" );
        int iret = IoctlStartMoveByPath ( input_buffer_ptr, output_buffer_ptr );
        return iret;
    }

    /************************************************************************/
    /* input_buffer_ptr: {"linearSpeed":0.2,"angularSpeed":0.3}
    */
    /************************************************************************/
    int GaussianChassisDriver::IoctlStartMoveByDirection ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlStartMoveByDirection" );
        UINT8 *inbufferptr = NULL;
        UINT32 inbuffersize = 0;
        // const char* strhttpobjname = HTTP_OBJECT_NAME_SPEED;
        const char* strhttpobjname = HTTP_OBJECT_NAME_MOVEBYDIR;  // safe mode
        if ( !is_sdp_connected_ )
        {
            YSOS_LOG_WARN ( "Chassis is not connected." );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == output_buffer_ptr || NULL == input_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Input Or Output Buffer Exception." );
            return YSOS_ERROR_FAILED;
        }
        input_buffer_ptr->GetBufferAndLength ( &inbufferptr, &inbuffersize );
        Json::Reader js_reader;
        Json::FastWriter writer;
        Json::Value js_value;
        Json::Value js_speed;
        if ( !js_reader.parse ( reinterpret_cast<char*> ( inbufferptr ), js_value, false ) )
        {
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        js_speed[CHASSISS_SPEED] = js_value;
        std::string strposjson = writer.write ( js_speed );
        strposjson = GetUtility()->ReplaceAllDistinct ( strposjson, "\\r\\n", "" );
        if ( YSOS_ERROR_SUCCESS != http_method_post ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) strhttpobjname, ( UINT8* ) strposjson.c_str(), output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_post Fail." );
            return YSOS_ERROR_FAILED;
        }
        if ( YSOS_ERROR_SUCCESS != Check_Http_Result ( output_buffer_ptr, false ) )
        {
            YSOS_LOG_WARN ( "Check_Http_Result Fail." );
            return YSOS_ERROR_FAILED;
        }
        return YSOS_ERROR_SUCCESS;
    }


    /************************************************************************/
    /* input_buffer_ptr:
    {
      "x":1.5,
      "y":2.5,
      "angle":5.2
    }
      The angle is optional.
    */
    /************************************************************************/
    int GaussianChassisDriver::IoctlStartMoveTo ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlStartMoveTo" );
        UINT8 *inbufferptr = NULL;
        UINT32 inbuffersize = 0;
        const char* strhttpobjname = HTTP_OBJECT_NAME_MOVETO;
        if ( !is_sdp_connected_ )
        {
            YSOS_LOG_WARN ( "Chassis is not connected." );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == input_buffer_ptr || NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Input Or Output Buffer Exception." );
            return YSOS_ERROR_FAILED;
        }
        input_buffer_ptr->GetBufferAndLength ( &inbufferptr, &inbuffersize );
        if ( NULL == inbufferptr )
        {
            YSOS_LOG_ERROR ( "Input Buffer Exception." );
            return YSOS_ERROR_FAILED;
        }
        ClearLatestNavigationStatus();
        /// 解析输入Json数据
        #ifdef _DEBUG
        YSOS_LOG_DEBUG ( "Input:" << reinterpret_cast<char*> ( inbufferptr ) );
        #endif
        Json::Reader js_reader;
        Json::Value js_value;
        if ( !js_reader.parse ( reinterpret_cast<char*> ( inbufferptr ), js_value, false ) )
        {
            YSOS_LOG_ERROR ( "parse exception." );
            return YSOS_ERROR_FAILED;
        }
        /// 构造发送Json数据
        Json::Value json_children;
        Json::Value json_child;
        Json::Value json_dest;
        Json::FastWriter writer;
        json_children[CHASSISS_POS_X] = js_value[CHASSISS_POS_X].asDouble();
        json_children[CHASSISS_POS_Y] = js_value[CHASSISS_POS_Y].asDouble();
        if ( js_value.isMember ( CHASSISS_POS_ANGLE ) )
        {
            json_child[CHASSISS_POS_ANGLE] = js_value[CHASSISS_POS_ANGLE].asDouble();
        }
        json_child[CHASSISS_POS_GRID] = json_children;
        json_dest[CHASSISS_POS_DESTINATION] = json_child;
        std::string strposjson = writer.write ( json_dest );
        strposjson = GetUtility()->ReplaceAllDistinct ( strposjson, "\\r\\n", "" );
        if ( YSOS_ERROR_SUCCESS != http_method_post ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) strhttpobjname, ( UINT8* ) strposjson.c_str(), output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_post Fail." );
            return YSOS_ERROR_FAILED;
        }
        if ( YSOS_ERROR_SUCCESS != Check_Http_Result ( output_buffer_ptr, false ) )
        {
            YSOS_LOG_WARN ( "Check_Http_Result Fail." );
            return YSOS_ERROR_FAILED;
        }
        return YSOS_ERROR_SUCCESS;
    }


    /************************************************************************/
    /* input_buffer_ptr:
    {
      "mapname":"office",
      "positionname","Current"
      "path":[{"x":"1.4","y":"1.1"},{"x":"1.3","y":"1.8"}],
      "isMilestone":"1"
    }
    */
    /************************************************************************/
    int GaussianChassisDriver::IoctlStartMoveByPath ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlStartMoveByPath" );
        UINT8 *inbufferptr = NULL;
        UINT32 inbuffersize = 0;
        char szhttpobjname[MAX_LENGTH256] = {'\0'};
        std::string strmapname;
        std::string strpositionname;
        if ( !is_sdp_connected_ )
        {
            YSOS_LOG_WARN ( "Chassis is not connected." );
            return YSOS_ERROR_FAILED;
        }
        if ( input_buffer_ptr == NULL || output_buffer_ptr == NULL )
        {
            YSOS_LOG_ERROR ( "Input Or Output Buffer Exception." );
            return YSOS_ERROR_FAILED;
        }
        input_buffer_ptr->GetBufferAndLength ( &inbufferptr, &inbuffersize );
        ClearLatestNavigationStatus();
        /// 解析输入数据
        if ( NULL != inbufferptr )
        {
            #ifdef _DEBUG
            YSOS_LOG_DEBUG ( "Input:" << reinterpret_cast<char*> ( inbufferptr ) );
            #endif
            Json::Reader js_reader;
            Json::Value js_value;
            if ( !js_reader.parse ( reinterpret_cast<char*> ( inbufferptr ), js_value, false ) )
            {
                return YSOS_ERROR_FAILED;
            }
            strmapname = js_value[CHASSISS_MAP_NAME].asString();
            strpositionname = js_value[CHASSISS_MAP_POSITION_NAME].asString();
        }
#ifdef _WIN32
        sprintf_s ( szhttpobjname, MAX_LENGTH256, HTTP_OBJECT_NAME_MOVETOBYNAME, strmapname.c_str(), strpositionname.c_str() );
#else
        sprintf ( szhttpobjname, HTTP_OBJECT_NAME_MOVETOBYNAME, strmapname.c_str(), strpositionname.c_str() );
#endif
        /// 发送数据到底盘
        if ( YSOS_ERROR_SUCCESS != http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) szhttpobjname, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_get Fail." );
            return YSOS_ERROR_FAILED;
        }
        if ( YSOS_ERROR_SUCCESS != Check_Http_Result ( output_buffer_ptr, false ) )
        {
            YSOS_LOG_WARN ( "Check_Http_Result Fail." );
            return YSOS_ERROR_FAILED;
        }
        return YSOS_ERROR_SUCCESS;
    }


    /************************************************************************/
    /* input_buffer_ptr:
    {
      "rotateto":"30.0"
    }
    */
    /************************************************************************/
    int GaussianChassisDriver::IoctlStartRotateTo ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlStartRotateTo" );
        UINT8 *inbufferptr = NULL;
        UINT32 inbuffersize = 0;
        double  frotatetoangle = 0.0;
        double  frotateangle = 0.0;
        double  fcurrentangle = 0.0;
        const char* strhttpobjname = HTTP_OBJECT_NAME_ROTATETO;
        if ( !is_sdp_connected_ )
        {
            YSOS_LOG_WARN ( "Chassis is not connected." );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == output_buffer_ptr || NULL == input_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Input Or Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        input_buffer_ptr->GetBufferAndLength ( &inbufferptr, &inbuffersize );
        /// 解析输入数据
        if ( NULL != inbufferptr )
        {
            #ifdef _DEBUG
            YSOS_LOG_DEBUG ( "Input:" << reinterpret_cast<char*> ( inbufferptr ) );
            #endif
            Json::Reader js_reader;
            Json::Value js_value;
            if ( !js_reader.parse ( reinterpret_cast<char*> ( inbufferptr ), js_value, false ) )
            {
                return YSOS_ERROR_FAILED;
            }
            frotatetoangle = js_value[CHASSISS_POS_ROTATETO].asDouble();
        }
        /// 获取当前位置
        if ( YSOS_ERROR_SUCCESS != IoctlGetLocalizationStatus ( input_buffer_ptr, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "Get Current Position Error" );
            return YSOS_ERROR_FAILED;
        }
        UINT8 *outbufferptr = GetBufferUtility()->GetBufferData ( output_buffer_ptr );
        if ( NULL != outbufferptr )
        {
            #ifdef _DEBUG
            YSOS_LOG_DEBUG ( "Input:" << reinterpret_cast<char*> ( outbufferptr ) );
            #endif
            Json::Reader js_reader;
            Json::Value js_value;
            if ( !js_reader.parse ( reinterpret_cast<char*> ( outbufferptr ), js_value, false ) )
            {
                return YSOS_ERROR_FAILED;
            }
            fcurrentangle = js_value[CHASSISS_POS_ROTATE].asDouble();
        }
        frotateangle = frotatetoangle - fcurrentangle;
        if ( frotateangle >= 180 )
        {
            frotateangle -= 360;
        }
        else if ( frotateangle <= -180 )
        {
            frotateangle += 360;
        }
        /// 构造发送数据
        Json::Value json_value;
        Json::FastWriter writer;
        json_value[CHASSISS_POS_ROTATAANGLE] = frotateangle;
        std::string strposjson = writer.write ( json_value );
        strposjson = GetUtility()->ReplaceAllDistinct ( strposjson, "\\r\\n", "" );
        if ( YSOS_ERROR_SUCCESS != http_method_post ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) strhttpobjname, ( UINT8* ) strposjson.c_str(), output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_post Fail." );
            return YSOS_ERROR_FAILED;
        }
        if ( YSOS_ERROR_SUCCESS != Check_Http_Result ( output_buffer_ptr, false ) )
        {
            YSOS_LOG_WARN ( "Check_Http_Result Fail." );
            return YSOS_ERROR_FAILED;
        }
        return YSOS_ERROR_SUCCESS;
    }


    /************************************************************************/
    /* input_buffer_ptr:
    {
      "rotate":"30.0"
    }
    */
    /************************************************************************/
    int GaussianChassisDriver::IoctlStartRotate ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlStartRotate" );
        UINT8 *inbufferptr = NULL;
        UINT32 inbuffersize = 0;
        double  frotateangle = 0;
        const char* strhttpobjname = HTTP_OBJECT_NAME_ROTATETO;
        if ( !is_sdp_connected_ )
        {
            YSOS_LOG_WARN ( "Chassis is not connected." );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == input_buffer_ptr || NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Input Or Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        input_buffer_ptr->GetBufferAndLength ( &inbufferptr, &inbuffersize );
        /// 解析输入数据
        if ( NULL != inbufferptr )
        {
            #ifdef _DEBUG
            YSOS_LOG_DEBUG ( "Input:" << reinterpret_cast<char*> ( inbufferptr ) );
            #endif
            Json::Reader js_reader;
            Json::Value js_value;
            if ( !js_reader.parse ( reinterpret_cast<char*> ( inbufferptr ), js_value, false ) )
            {
                return YSOS_ERROR_FAILED;
            }
            frotateangle = js_value[CHASSISS_POS_ROTATE].asDouble();
        }
        /// 构造发送数据
        Json::Value json_value;
        Json::FastWriter writer;
        json_value[CHASSISS_POS_ROTATAANGLE] = frotateangle;
        std::string strposjson = writer.write ( json_value );
        strposjson = GetUtility()->ReplaceAllDistinct ( strposjson, "\\r\\n", "" );
        if ( YSOS_ERROR_SUCCESS != http_method_post ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) strhttpobjname, ( UINT8* ) strposjson.c_str(), output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_post Fail." );
            return YSOS_ERROR_FAILED;
        }
        if ( YSOS_ERROR_SUCCESS != Check_Http_Result ( output_buffer_ptr, false ) )
        {
            YSOS_LOG_WARN ( "Check_Http_Result Fail." );
            return YSOS_ERROR_FAILED;
        }
        return YSOS_ERROR_SUCCESS;
    }

    int GaussianChassisDriver::IoctlPauseMove ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlPauseMove" );
        const char* strhttpobjname = HTTP_OBJECT_NAME_PAUSENAVIGATE;
        if ( !is_sdp_connected_ )
        {
            YSOS_LOG_WARN ( "Chassis is not connected." );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        if ( YSOS_ERROR_SUCCESS != http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) strhttpobjname, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_get Fail." );
            return YSOS_ERROR_FAILED;
        }
        if ( YSOS_ERROR_SUCCESS != Check_Http_Result ( output_buffer_ptr, false ) )
        {
            YSOS_LOG_WARN ( "Check_Http_Result Fail." );
            return YSOS_ERROR_FAILED;
        }
        return YSOS_ERROR_SUCCESS;
    }

    int GaussianChassisDriver::IoctlResumeMove ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlResumeMove" );
        const char* strhttpobjname = HTTP_OBJECT_NAME_RESUMENAVIGATE;
        if ( !is_sdp_connected_ )
        {
            YSOS_LOG_WARN ( "Chassis is not connected." );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        if ( YSOS_ERROR_SUCCESS != http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) strhttpobjname, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_get Fail." );
            return YSOS_ERROR_FAILED;
        }
        if ( YSOS_ERROR_SUCCESS != Check_Http_Result ( output_buffer_ptr, false ) )
        {
            YSOS_LOG_WARN ( "Check_Http_Result Fail." );
            return YSOS_ERROR_FAILED;
        }
        return YSOS_ERROR_SUCCESS;
    }

    int GaussianChassisDriver::IoctlStopMove ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlStopMove" );
        const char* strhttpobjname = HTTP_OBJECT_NAME_STOPMOVE;
        if ( !is_sdp_connected_ )
        {
            YSOS_LOG_WARN ( "Chassis is not connected." );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        if ( YSOS_ERROR_SUCCESS != http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) strhttpobjname, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_get Fail." );
            return YSOS_ERROR_FAILED;
        }
        if ( YSOS_ERROR_SUCCESS != Check_Http_Result ( output_buffer_ptr, false ) )
        {
            YSOS_LOG_WARN ( "Check_Http_Result Fail." );
            return YSOS_ERROR_FAILED;
        }
        return YSOS_ERROR_SUCCESS;
    }

    /************************************************************************/
    /* input_buffer_ptr:
    {
      "mapname":"office",
      "greetarea":"001"
    }
    */
    /************************************************************************/
    int GaussianChassisDriver::IoctlStartGreet ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlStartGreet" );
        UINT8 *inbufferptr = NULL;
        UINT32 inbuffersize = 0;
        char szhttpobjname[MAX_LENGTH256] = {'\0'};
        std::string strmapname;
        std::string greetarea;
        if ( !is_sdp_connected_ )
        {
            YSOS_LOG_WARN ( "Chassis is not connected." );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == input_buffer_ptr || NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Input Or Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        input_buffer_ptr->GetBufferAndLength ( &inbufferptr, &inbuffersize );
        if ( NULL != inbufferptr )
        {
            #ifdef _DEBUG
            YSOS_LOG_DEBUG ( "Input:" << reinterpret_cast<char*> ( inbufferptr ) );
            #endif
            Json::Reader js_reader;
            Json::Value js_value;
            if ( !js_reader.parse ( reinterpret_cast<char*> ( inbufferptr ), js_value, false ) )
            {
                return YSOS_ERROR_FAILED;
            }
            strmapname = js_value[CHASSISS_MAP_NAME].asString();
            greetarea = js_value[CHASSISS_MAP_GREETARER].asString();
        }
#ifdef _WIN32
        sprintf_s ( szhttpobjname, MAX_LENGTH256, HTTP_OBJECT_NAME_STARTGREET, strmapname.c_str(), greetarea.c_str() );
#else
        sprintf ( szhttpobjname, HTTP_OBJECT_NAME_STARTGREET, strmapname.c_str(), greetarea.c_str() );
#endif
        if ( YSOS_ERROR_SUCCESS != http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) szhttpobjname, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_get Fail." );
            return YSOS_ERROR_FAILED;
        }
        if ( YSOS_ERROR_SUCCESS != Check_Http_Result ( output_buffer_ptr, false ) )
        {
            YSOS_LOG_WARN ( "Check_Http_Result Fail." );
            return YSOS_ERROR_FAILED;
        }
        return YSOS_ERROR_SUCCESS;
    }

    int GaussianChassisDriver::IoctlPauseGreet ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlPauseGreet" );
        const char* strhttpobjname = HTTP_OBJECT_NAME_PAUSEGREET;
        if ( !is_sdp_connected_ )
        {
            YSOS_LOG_WARN ( "Chassis is not connected." );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        if ( YSOS_ERROR_SUCCESS != http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) strhttpobjname, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_get Fail." );
            return YSOS_ERROR_FAILED;
        }
        if ( YSOS_ERROR_SUCCESS != Check_Http_Result ( output_buffer_ptr, false ) )
        {
            YSOS_LOG_WARN ( "Check_Http_Result Fail." );
            return YSOS_ERROR_FAILED;
        }
        return YSOS_ERROR_SUCCESS;
    }

    int GaussianChassisDriver::IoctlResumeGreet ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlResumeGreet" );
        const char* strhttpobjname = HTTP_OBJECT_NAME_RESUMEGREET;
        if ( !is_sdp_connected_ )
        {
            YSOS_LOG_WARN ( "Chassis is not connected." );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        if ( YSOS_ERROR_SUCCESS != http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) strhttpobjname, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_get Fail." );
            return YSOS_ERROR_FAILED;
        }
        if ( YSOS_ERROR_SUCCESS != Check_Http_Result ( output_buffer_ptr, false ) )
        {
            YSOS_LOG_WARN ( "Check_Http_Result Fail." );
            return YSOS_ERROR_FAILED;
        }
        return YSOS_ERROR_SUCCESS;
    }

    int GaussianChassisDriver::IoctlStopGreet ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlStopGreet" );
        const char* strhttpobjname = HTTP_OBJECT_NAME_STOPGREET;
        if ( !is_sdp_connected_ )
        {
            YSOS_LOG_WARN ( "Chassis is not connected." );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        if ( YSOS_ERROR_SUCCESS != http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) strhttpobjname, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_get Fail." );
            return YSOS_ERROR_FAILED;
        }
        if ( YSOS_ERROR_SUCCESS != Check_Http_Result ( output_buffer_ptr, false ) )
        {
            YSOS_LOG_WARN ( "Check_Http_Result Fail." );
            return YSOS_ERROR_FAILED;
        }
        return YSOS_ERROR_SUCCESS;
    }

    /************************************************************************/
    /* input_buffer_ptr:
    {
      "freemode":0/1
      "importance":xxx
    }
    */
    /************************************************************************/
    int GaussianChassisDriver::IoctlSetGreetParam ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlSetGreetParam" );
        return YSOS_ERROR_SUCCESS;
    }

    /************************************************************************/
    /* input_buffer_ptr:
    {
      "mapname":"office",
      "positionname","Current"
    }
    */
    /************************************************************************/
    int GaussianChassisDriver::IoctlStartRecordPath ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlStartRecordPath" );
        UINT8 *inbufferptr = NULL;
        UINT32 inbuffersize = 0;
        char szhttpobjname[MAX_LENGTH256] = {'\0'};
        std::string strmanname;
        std::string strpositionname;
        struct ChassisPath struchassis;
        memset ( &struchassis, 0, sizeof ( struchassis ) );
        if ( !is_sdp_connected_ )
        {
            YSOS_LOG_WARN ( "Chassis is not connected." );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == input_buffer_ptr || NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Input Or Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        input_buffer_ptr->GetBufferAndLength ( &inbufferptr, &inbuffersize );
        /// 解析输入数据
        if ( NULL != inbufferptr )
        {
            std::string strresult = reinterpret_cast<char*> ( inbufferptr );
            if ( 0 == strresult.length() )
            {
                YSOS_LOG_ERROR ( "Input Length Error:0!!!!!!" );
                return YSOS_ERROR_INVALID_ARGUMENTS;
            }
            #ifdef _DEBUG
            YSOS_LOG_DEBUG ( "Input:" << strresult );
            #endif
            boost::property_tree::ptree ptjson;
            std::istringstream stream ( strresult );
            boost::property_tree::json_parser::read_json ( stream, ptjson );
            strmanname = ptjson.get<std::string> ( CHASSISS_MAP_NAME, "" );
            strpositionname = ptjson.get<std::string> ( CHASSISS_MAP_POSITION_NAME, "" );
        }
#ifdef _WIN32
        sprintf_s ( szhttpobjname, MAX_LENGTH256, HTTP_OBJECT_NAME_START_RECORDPATH, strmanname.c_str(), strpositionname.c_str() );
#else
        sprintf ( szhttpobjname, HTTP_OBJECT_NAME_START_RECORDPATH, strmanname.c_str(), strpositionname.c_str() );
#endif
        if ( YSOS_ERROR_SUCCESS != http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) szhttpobjname, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_get Fail." );
            return YSOS_ERROR_FAILED;
        }
        if ( YSOS_ERROR_SUCCESS != Check_Http_Result ( output_buffer_ptr, false ) )
        {
            YSOS_LOG_WARN ( "Check_Http_Result Fail." );
            return YSOS_ERROR_FAILED;
        }
        return YSOS_ERROR_SUCCESS;
    }

    int GaussianChassisDriver::IoctlStopRecordPath ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlStopRecordPath" );
        const char* strhttpobjname = HTTP_OBJECT_NAME_STOP_RECORDPATH;
        if ( !is_sdp_connected_ )
        {
            YSOS_LOG_WARN ( "Chassis is not connected." );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        if ( YSOS_ERROR_SUCCESS != http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) strhttpobjname, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_get Fail." );
            return YSOS_ERROR_FAILED;
        }
        if ( YSOS_ERROR_SUCCESS != Check_Http_Result ( output_buffer_ptr, false ) )
        {
            YSOS_LOG_WARN ( "Check_Http_Result Fail." );
            return YSOS_ERROR_FAILED;
        }
        return YSOS_ERROR_SUCCESS;
    }

    int GaussianChassisDriver::IoctlInitFinish ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlInitFinish" );
        const char* strhttpobjname = HTTP_OBJECT_NAME_INIT_FINISH;
        if ( NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        if ( YSOS_ERROR_SUCCESS != http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) strhttpobjname, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_get Fail." );
            return YSOS_ERROR_FAILED;
        }
        if ( YSOS_ERROR_SUCCESS != Check_Http_Result ( output_buffer_ptr, true ) )
        {
            YSOS_LOG_WARN ( "Check_Http_Result Fail." );
            return YSOS_ERROR_FAILED;
        }
        UINT8 *outbufferptr = GetBufferUtility()->GetBufferData ( output_buffer_ptr );
        if ( NULL != outbufferptr )
        {
            if ( 0 == strcmp ( reinterpret_cast<char*> ( outbufferptr ), RESULT_TRUE ) )
            {
                return YSOS_ERROR_SUCCESS;
            }
        }
        YSOS_LOG_DEBUG ( "Init Not Finish" );
        return YSOS_ERROR_FAILED;
    }


    int GaussianChassisDriver::IoctlGetVersion ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlGetVersion." );
        const char* strhttpobjname = HTTP_OBJECT_NAME_GETSDKVERSION;
        if ( NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        if ( YSOS_ERROR_SUCCESS != GetChassisVersion() )
        {
            YSOS_LOG_WARN ( "Get Chassis Version Fail." );
            return YSOS_ERROR_FAILED;
        }
        /// 构造输出Json
        boost::property_tree::ptree ptdest_root;
        ptdest_root.put ( "deviceID", chassis_productId_ );
        ptdest_root.put ( "manufacturerID", "" );
        ptdest_root.put ( "manufacturerName", "" );
        ptdest_root.put ( "modelID", chassis_module_type_ );
        ptdest_root.put ( "modelName", "" );
        ptdest_root.put ( "hardwareVersion", "" );
        ptdest_root.put ( "softwareVersion", version_sdk_ );
        std::stringstream ssdest;
        boost::property_tree::write_json ( ssdest, ptdest_root );
        std::string strjson_result = ssdest.str();
        GetBufferUtility()->CopyStringToBuffer ( strjson_result, output_buffer_ptr );
        return YSOS_ERROR_SUCCESS;
    }

    int GaussianChassisDriver::IoctlPowerOff ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlPowerOff" );
        const char* strhttpobjname = HTTP_OBJECT_NAME_POWEROFF;
        if ( NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        if ( YSOS_ERROR_SUCCESS != http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) strhttpobjname, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_get Fail." );
            return YSOS_ERROR_FAILED;
        }
        if ( YSOS_ERROR_SUCCESS != Check_Http_Result ( output_buffer_ptr, false ) )
        {
            YSOS_LOG_WARN ( "Check_Http_Result Fail." );
            return YSOS_ERROR_FAILED;
        }
        return YSOS_ERROR_SUCCESS;
    }

    /************************************************************************/
    /* output_buffer_ptr:
    {
      Gaussian define: "is_rotate_finish":"false"
      but we ues: {"status":"moving/finish"}
    }
    */
    /************************************************************************/
    int GaussianChassisDriver::IoctlGetRotateStatus ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlGetRotateStatus" );
        const char* strhttpobjname = HTTP_OBJECT_NAME_ROTATE_STATUS;
        if ( !is_sdp_connected_ )
        {
            YSOS_LOG_WARN ( "Chassis is not connected." );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        if ( YSOS_ERROR_SUCCESS != http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) strhttpobjname, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_get Fail." );
            return YSOS_ERROR_FAILED;
        }
        if ( YSOS_ERROR_SUCCESS != Check_Http_Result ( output_buffer_ptr, true ) )
        {
            YSOS_LOG_WARN ( "Check_Http_Result Fail." );
            return YSOS_ERROR_FAILED;
        }
        /// 构造返回Json数据
        UINT8 *outbufferptr = GetBufferUtility()->GetBufferData ( output_buffer_ptr );
        std::string strresult = reinterpret_cast<char*> ( outbufferptr );
        if ( 0 == strresult.length() )
        {
            YSOS_LOG_WARN ( "HTTP Return Data Error!" );
            return YSOS_ERROR_FAILED;
        }
        if ( 0 == strresult.compare ( RESULT_TRUE ) )
        {
            strcpy ( reinterpret_cast<char*> ( outbufferptr ), "{\"status\":\"finish\"}" );
        }
        else
        {
            strcpy ( reinterpret_cast<char*> ( outbufferptr ), "{\"status\":\"moving\"}" );
        }
        return YSOS_ERROR_SUCCESS;
    }

    int GaussianChassisDriver::IoctlGetLaserInfo ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlGetLaserInfo" );
        const char* strhttpobjname = HTTP_OBJECT_NAME_GETLASERRAW;
        if ( !is_sdp_connected_ )
        {
            YSOS_LOG_WARN ( "Chassis is not connected." );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        if ( YSOS_ERROR_SUCCESS != http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) strhttpobjname, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_get Fail." );
            return YSOS_ERROR_FAILED;
        }
        return YSOS_ERROR_SUCCESS;
    }

    /************************************************************************/
    /*
    input_buffer_ptr:
    {
      "type":"0",
      "id":"0"
    }
    */
    /************************************************************************/
    int GaussianChassisDriver::IoctlGetUltrasonicInfo ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlGetUltrasonicInfo" );
        UINT8 *inbufferptr = NULL;
        UINT32 inbuffersize = 0;
        char szhttpobjname[MAX_LENGTH256] = {'\0'};
        if ( !is_sdp_connected_ )
        {
            YSOS_LOG_WARN ( "Chassis is not connected." );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == input_buffer_ptr || NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Input Or Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        input_buffer_ptr->GetBufferAndLength ( &inbufferptr, &inbuffersize );
        std::string strresult = reinterpret_cast<char*> ( inbufferptr );
        if ( 0 == strresult.length() )
        {
            YSOS_LOG_ERROR ( "Input Length Error:0!!!!!!" );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        #ifdef _DEBUG
        YSOS_LOG_DEBUG ( "Input:" << strresult );
        #endif
        boost::property_tree::ptree ptjson;
        std::istringstream stream ( strresult );
        boost::property_tree::json_parser::read_json ( stream, ptjson );
        int iframeid  = ptjson.get<int> ( CHASSISS_DEVICE_ID, 0 );
#ifdef _WIN32
        sprintf_s ( szhttpobjname, MAX_LENGTH256, HTTP_OBJECT_NAME_GETULTRASONICRAW, iframeid );
#else
        sprintf ( szhttpobjname, HTTP_OBJECT_NAME_GETULTRASONICRAW, iframeid );
#endif
        if ( YSOS_ERROR_SUCCESS != http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) szhttpobjname, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_get Fail." );
            return YSOS_ERROR_FAILED;
        }
        return YSOS_ERROR_SUCCESS;
    }

    int GaussianChassisDriver::IoctlGetProtectorInfo ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlGetProtectorInfo" );
        const char* strhttpobjname = HTTP_OBJECT_NAME_GETPROTECTORRAW;
        if ( !is_sdp_connected_ )
        {
            YSOS_LOG_WARN ( "Chassis is not connected." );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        if ( YSOS_ERROR_SUCCESS != http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) strhttpobjname, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_get Fail." );
            return YSOS_ERROR_FAILED;
        }
        return YSOS_ERROR_SUCCESS;
    }

    int GaussianChassisDriver::IoctlStartCharge ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlStartCharge" );
        const char* strhttpobjname = HTTP_OBJECT_NAME_START_CHARGE;
        if ( !is_sdp_connected_ )
        {
            YSOS_LOG_WARN ( "Chassis is not connected." );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        if ( YSOS_ERROR_SUCCESS != http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) strhttpobjname, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_get Fail." );
            return YSOS_ERROR_FAILED;
        }
        if ( YSOS_ERROR_SUCCESS != Check_Http_Result ( output_buffer_ptr, false ) )
        {
            YSOS_LOG_WARN ( "Check_Http_Result Fail." );
            return YSOS_ERROR_FAILED;
        }
        return YSOS_ERROR_SUCCESS;
    }

    int GaussianChassisDriver::IoctlStopCharge ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlStopCharge" );
        const char* strhttpobjname = HTTP_OBJECT_NAME_STOP_CHARGE;
        if ( !is_sdp_connected_ )
        {
            YSOS_LOG_WARN ( "Chassis is not connected." );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        if ( YSOS_ERROR_SUCCESS != http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) strhttpobjname, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_get Fail." );
            return YSOS_ERROR_FAILED;
        }
        if ( YSOS_ERROR_SUCCESS != Check_Http_Result ( output_buffer_ptr, false ) )
        {
            YSOS_LOG_WARN ( "Check_Http_Result Fail." );
            return YSOS_ERROR_FAILED;
        }
        return YSOS_ERROR_SUCCESS;
    }


    /************************************************************************/
    /* input_buffer_ptr:
    {
      "mapname":"office"
    }
    */
    /************************************************************************/
    int GaussianChassisDriver::IoctlGetMapPicture ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlGetMapPicture" );
        UINT8 *inbufferptr = NULL;
        UINT32 inbuffersize = 0;
        char szhttpobjname[MAX_LENGTH256] = {'\0'};
        std::string strmapname;
        if ( NULL == input_buffer_ptr || NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Input Or Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        input_buffer_ptr->GetBufferAndLength ( &inbufferptr, &inbuffersize );
        if ( NULL != inbufferptr )
        {
            std::string strresult = reinterpret_cast<char*> ( inbufferptr );
            if ( 0 == strresult.length() )
            {
                YSOS_LOG_ERROR ( "Input Length Error:0!!!!!!" );
                return YSOS_ERROR_INVALID_ARGUMENTS;
            }
            #ifdef _DEBUG
            YSOS_LOG_DEBUG ( "Input:" << strresult );
            #endif
            boost::property_tree::ptree ptjson;
            std::istringstream stream ( strresult );
            boost::property_tree::json_parser::read_json ( stream, ptjson );
            strmapname = ptjson.get<std::string> ( CHASSISS_MAP_NAME, "" );
        }
#ifdef _WIN32
        sprintf_s ( szhttpobjname, MAX_LENGTH256, HTTP_OBJECT_NAME_MAP_GET_PNG, strmapname.c_str() );
#else
        sprintf ( szhttpobjname, HTTP_OBJECT_NAME_MAP_GET_PNG, strmapname.c_str() );
#endif
        if ( YSOS_ERROR_SUCCESS != http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) szhttpobjname, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_get Fail." );
            return YSOS_ERROR_FAILED;
        }
        return YSOS_ERROR_SUCCESS;
    }

    /************************************************************************/
    /* input_buffer_ptr:
    {
      "mapname":"office"
    }
    */
    /************************************************************************/
    int GaussianChassisDriver::IoctlDelMap ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlDelMap" );
        UINT8 *inbufferptr = NULL;
        UINT32 inbuffersize = 0;
        char szhttpobjname[MAX_LENGTH256] = {'\0'};
        std::string strmapname;
        if ( NULL == input_buffer_ptr || NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Input Or Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        input_buffer_ptr->GetBufferAndLength ( &inbufferptr, &inbuffersize );
        if ( NULL != inbufferptr )
        {
            std::string strresult = reinterpret_cast<char*> ( inbufferptr );
            if ( 0 == strresult.length() )
            {
                YSOS_LOG_ERROR ( "Input Length Error:0!!!!!!" );
                return YSOS_ERROR_INVALID_ARGUMENTS;
            }
            #ifdef _DEBUG
            YSOS_LOG_DEBUG ( "Input:" << strresult );
            #endif
            boost::property_tree::ptree ptjson;
            std::istringstream stream ( strresult );
            boost::property_tree::json_parser::read_json ( stream, ptjson );
            strmapname = ptjson.get<std::string> ( CHASSISS_MAP_NAME, "" );
        }
#ifdef _WIN32
        sprintf_s ( szhttpobjname, MAX_LENGTH256, HTTP_OBJECT_NAME_MAP_DEL, strmapname.c_str() );
#else
        sprintf ( szhttpobjname, HTTP_OBJECT_NAME_MAP_DEL, strmapname.c_str() );
#endif
        if ( YSOS_ERROR_SUCCESS != http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) szhttpobjname, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_get Fail." );
            return YSOS_ERROR_FAILED;
        }
        if ( YSOS_ERROR_SUCCESS != Check_Http_Result ( output_buffer_ptr, false ) )
        {
            YSOS_LOG_WARN ( "Check_Http_Result Fail." );
            return YSOS_ERROR_FAILED;
        }
        return YSOS_ERROR_SUCCESS;
    }

    /************************************************************************/
    /* input_buffer_ptr:
    {
      "mapname":"office",
      "originname":"house"
    }
    */
    /************************************************************************/
    int GaussianChassisDriver::IoctlRenameMap ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlRenameMap" );
        UINT8 *inbufferptr = NULL;
        UINT32 inbuffersize = 0;
        char szhttpobjname[MAX_LENGTH256] = {'\0'};
        std::string strmapname;
        std::string stroriginname;
        if ( NULL == input_buffer_ptr || NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Input Or Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        input_buffer_ptr->GetBufferAndLength ( &inbufferptr, &inbuffersize );
        /// 解析输入数据
        if ( NULL != inbufferptr )
        {
            std::string strresult = reinterpret_cast<char*> ( inbufferptr );
            if ( 0 == strresult.length() )
            {
                YSOS_LOG_ERROR ( "Input Length Error:0!!!!!!" );
                return YSOS_ERROR_INVALID_ARGUMENTS;
            }
            #ifdef _DEBUG
            YSOS_LOG_DEBUG ( "Input:" << strresult );
            #endif
            boost::property_tree::ptree ptjson;
            std::istringstream stream ( strresult );
            boost::property_tree::json_parser::read_json ( stream, ptjson );
            strmapname = ptjson.get<std::string> ( CHASSISS_MAP_NAME, "" );
            stroriginname = ptjson.get<std::string> ( CHASSISS_MAP_ORIGIN_NAME, "" );
        }
#ifdef _WIN32
        sprintf_s ( szhttpobjname, MAX_LENGTH256, HTTP_OBJECT_NAME_MAP_RENAME, stroriginname.c_str(), strmapname.c_str() );
#else
        sprintf ( szhttpobjname, HTTP_OBJECT_NAME_MAP_RENAME, stroriginname.c_str(), strmapname.c_str() );
#endif
        if ( YSOS_ERROR_SUCCESS != http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) szhttpobjname, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_get Fail." );
            return YSOS_ERROR_FAILED;
        }
        /// 解析输出Json数据
        if ( YSOS_ERROR_SUCCESS != Check_Http_Result ( output_buffer_ptr, false ) )
        {
            YSOS_LOG_WARN ( "Check_Http_Result Fail." );
            return YSOS_ERROR_FAILED;
        }
        return YSOS_ERROR_SUCCESS;
    }

    /************************************************************************/
    /* input_buffer_ptr:
    {
      "mapname":"office"
    }
    */
    /************************************************************************/
    int GaussianChassisDriver::IoctlStartScanMap ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlStartScanMap" );
        UINT8 *inbufferptr = NULL;
        UINT32 inbuffersize = 0;
        char szhttpobjname[MAX_LENGTH256] = {'\0'};
        std::string strmapname;
        if ( NULL == input_buffer_ptr || NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Input Or Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        input_buffer_ptr->GetBufferAndLength ( &inbufferptr, &inbuffersize );
        /// 解析输入数据
        if ( NULL != inbufferptr )
        {
            std::string strresult = reinterpret_cast<char*> ( inbufferptr );
            if ( 0 == strresult.length() )
            {
                YSOS_LOG_ERROR ( "Input Length Error:0!!!!!!" );
                return YSOS_ERROR_INVALID_ARGUMENTS;
            }
            #ifdef _DEBUG
            YSOS_LOG_DEBUG ( "Input:" << strresult );
            #endif
            boost::property_tree::ptree ptjson;
            std::istringstream stream ( strresult );
            boost::property_tree::json_parser::read_json ( stream, ptjson );
            strmapname = ptjson.get<std::string> ( CHASSISS_MAP_NAME, "" );
        }
#ifdef _WIN32
        sprintf_s ( szhttpobjname, MAX_LENGTH256, HTTP_OBJECT_NAME_MAP_SCAN_START, strmapname.c_str() );
#else
        sprintf ( szhttpobjname, HTTP_OBJECT_NAME_MAP_SCAN_START, strmapname.c_str() );
#endif
        if ( YSOS_ERROR_SUCCESS != http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) szhttpobjname, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_get Fail." );
            return YSOS_ERROR_FAILED;
        }
        /// 解析输出Json数据
        if ( YSOS_ERROR_SUCCESS != Check_Http_Result ( output_buffer_ptr, false ) )
        {
            YSOS_LOG_WARN ( "Check_Http_Result Fail." );
            return YSOS_ERROR_FAILED;
        }
        return YSOS_ERROR_SUCCESS;
    }

    int GaussianChassisDriver::IoctlCancelScanMap ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlCancelScanMap" );
        const char* strhttpobjname = HTTP_OBJECT_NAME_MAP_SCAN_CANCEL;
        if ( !is_sdp_connected_ )
        {
            YSOS_LOG_WARN ( "Chassis is not connected." );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        if ( YSOS_ERROR_SUCCESS != http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) strhttpobjname, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_get Fail." );
            return YSOS_ERROR_FAILED;
        }
        if ( YSOS_ERROR_SUCCESS != Check_Http_Result ( output_buffer_ptr, false ) )
        {
            YSOS_LOG_WARN ( "Check_Http_Result Fail." );
            return YSOS_ERROR_FAILED;
        }
        return YSOS_ERROR_SUCCESS;
    }

    int GaussianChassisDriver::IoctlStopScanMap ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlStopScanMap" );
        const char* strhttpobjname = HTTP_OBJECT_NAME_MAP_SCAN_STOP;
        if ( !is_sdp_connected_ )
        {
            YSOS_LOG_WARN ( "Chassis is not connected." );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        if ( YSOS_ERROR_SUCCESS != http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) strhttpobjname, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_get Fail." );
            return YSOS_ERROR_FAILED;
        }
        if ( YSOS_ERROR_SUCCESS != Check_Http_Result ( output_buffer_ptr, false ) )
        {
            YSOS_LOG_WARN ( "Check_Http_Result Fail." );
            return YSOS_ERROR_FAILED;
        }
        return YSOS_ERROR_SUCCESS;
    }

    int GaussianChassisDriver::IoctlMobileData ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlMobileData." );
        const char* strhttpobjname = HTTP_OBJECT_NAME_MOBILEDATA;
        if ( !is_sdp_connected_ )
        {
            YSOS_LOG_WARN ( "Chassis is not connected." );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        if ( YSOS_ERROR_SUCCESS != http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) strhttpobjname, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_post Fail." );
            return YSOS_ERROR_FAILED;
        }
        return YSOS_ERROR_SUCCESS;
    }

    /************************************************************************/
    /*
    input_buffer_ptr:
      {"angle_min":-30,"angle_max":30}
    output_buffer_ptr:
      {"data":[{"distance":0.55,"angle":2.0,"speed":0.08},{"distance":0.45,"angle":20,"speed":0.06}]}
    */
    /************************************************************************/
    int GaussianChassisDriver::IoctlGetObstacleData ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlGetObstacleData" );
        if ( !is_sdp_connected_ )
        {
            YSOS_LOG_WARN ( "Chassis is not connected." );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == input_buffer_ptr || NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Input Or Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        //parse input
        UINT8 *inputbufferptr = NULL;
        UINT32 inputbuffersize = 0;
        input_buffer_ptr->GetBufferAndLength ( &inputbufferptr, &inputbuffersize );
        double angle_fm = 0;
        double angle_to = 0;
        Json::Reader js_reader;
        Json::Value js_value;
        if ( !js_reader.parse ( reinterpret_cast<char*> ( inputbufferptr ), js_value, false ) )
        {
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        angle_fm = js_value[CHASSISS_OBSTANCLE_RANGE_MINANGLE].asDouble();
        angle_to = js_value[CHASSISS_OBSTANCLE_RANGE_MAXANGLE].asDouble();
        // get mobile data
        int iret = IoctlMobileData ( NULL, output_buffer_ptr );
        if ( YSOS_ERROR_SUCCESS != iret )
        {
            return iret;
        }
        //parse output
        UINT8 *outputbufferptr = NULL;
        UINT32 outputbuffersize = 0;
        output_buffer_ptr->GetBufferAndLength ( &outputbufferptr, &outputbuffersize );
        if ( outputbufferptr == NULL )
        {
            return YSOS_ERROR_FAILED;
        }
        if ( !js_reader.parse ( reinterpret_cast<char*> ( outputbufferptr ), js_value, false ) )
        {
            return YSOS_ERROR_FAILED;
        }
        js_value = js_value[RESULT_DATA][CHASSISS_OBSTANCLE_MOBILE];
        // data
        Json::Value js_out;
        Json::Value js_data;
        Json::Value js_unit;
        int cnt = js_value.size();
        for ( int i = 0; i < cnt; i++ )
        {
            double angle = js_value[i][CHASSISS_OBSTANCLE_OBJECT_ANGLE].asDouble();
            double distance = js_value[i][CHASSISS_POS_DISTANCE].asDouble();
            double speed = js_value[i][CHASSISS_OBSTANCLE_OBJECT_SPEED].asDouble();
            YSOS_LOG_DEBUG ( "MobileData:" << angle << " " << distance << " " << speed );
            if ( angle < angle_fm || angle > angle_to || distance <= CHECK_OBSTANCLE_MIN_DISTANCE )
            {
                continue;
            }
            js_unit[CHASSISS_POS_ANGLE] = angle;
            js_unit[CHASSISS_POS_DISTANCE] = distance;
            js_unit[CHASSISS_SPEED] = speed;
            js_data.append ( js_unit );
        }
        js_out[RESULT_DATA] = js_data;
        /// 构造输出Json
        Json::FastWriter writer;
        std::string strjson = writer.write ( js_out );
        strjson = GetUtility()->ReplaceAllDistinct ( strjson, "\\r\\n", "" );
        #ifdef _DEBUG
        YSOS_LOG_DEBUG ( "Output:" << strjson );
        #endif
        GetBufferUtility()->CopyStringToBuffer ( strjson, output_buffer_ptr );
        return YSOS_ERROR_SUCCESS;
    }

    /************************************************************************/
    /*
    input_buffer_ptr:{
      "ranges": [
        {"angle_min":-40,"angle_max":-15},
        {"angle_min":-15,"angle_max":15},
        {"angle_min":15,"angle_max":40}
      ]
    }
    output_buffer_ptr:{
      "distance": [1.1, 2.2, 3.3]
    }
    */
    /************************************************************************/
    int GaussianChassisDriver::IoctlGetNearestLaserInfoInAngleRange ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlGetNearestLaserInfoInAngleRange" );
        if ( !is_sdp_connected_ )
        {
            YSOS_LOG_WARN ( "Chassis is not connected." );
            return YSOS_ERROR_FAILED;
        }
        if ( NULL == input_buffer_ptr || NULL == output_buffer_ptr )
        {
            YSOS_LOG_ERROR ( "Input Or Output Buffer Exception." );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        UINT8 *inputbufferptr = NULL;
        UINT32 inputbuffersize = 0;
        input_buffer_ptr->GetBufferAndLength ( &inputbufferptr, &inputbuffersize );
        /// 解析输入Json
        struct RangeData
        {
            double angle_fm;
            double angle_to;
        };
        int range_num = 0;
        RangeData *range_data;
        Json::Reader js_reader;
        Json::Value js_value;
        if ( !js_reader.parse ( reinterpret_cast<char*> ( inputbufferptr ), js_value, false ) )
        {
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        js_value = js_value[CHASSISS_OBSTANCLE_RANGES];
        range_num = js_value.size();
        range_data = ( RangeData* ) malloc ( range_num * sizeof ( RangeData ) );
        for ( int i = 0; i < range_num; i++ )
        {
            range_data[i].angle_fm = js_value[i][CHASSISS_OBSTANCLE_RANGE_MINANGLE].asDouble();
            range_data[i].angle_to = js_value[i][CHASSISS_OBSTANCLE_RANGE_MAXANGLE].asDouble();
            #ifdef _DEBUG
            YSOS_LOG_DEBUG ( "Get Obstacle Angle Range " << i << ": " << range_data[i].angle_fm << "," << range_data[i].angle_to );
            #endif
        }
        // 读取激光数据
        int iret = IoctlGetLaserInfo ( NULL, output_buffer_ptr );
        if ( YSOS_ERROR_SUCCESS != iret )
        {
            YSOS_LOG_WARN ( "Get Laser Info Fail:" << iret );
            free ( range_data );
            return iret;
        }
        // 解析激光数据
        UINT8 *outbufferptr = GetBufferUtility()->GetBufferData ( output_buffer_ptr );
        if ( NULL == outbufferptr )
        {
            YSOS_LOG_ERROR ( "Output Buffer is NULL." );
            free ( range_data );
            return YSOS_ERROR_FAILED;
        }
        if ( !js_reader.parse ( reinterpret_cast<char*> ( outbufferptr ), js_value, false ) )
        {
            YSOS_LOG_ERROR ( "parse error" );
            free ( range_data );
            return YSOS_ERROR_FAILED;
        }
        double fminangle = js_value[CHASSISS_OBSTANCLE_RANGE_MINANGLE].asDouble() * 180 / M_PI;      ///< 数据开始角度
        double fmaxangle = js_value[CHASSISS_OBSTANCLE_RANGE_MAXANGLE].asDouble() * 180 / M_PI;      ///< 数据结束角度
        double fangle_increment = js_value[CHASSISS_OBSTANCLE_RANGE_INCREANGLE].asDouble() * 180 / M_PI; ///< 角度步长
        js_value = js_value[CHASSISS_OBSTANCLE_RANGES];
        Json::Value js_result;
        for ( int i = 0; i < range_num; i++ )
        {
            int idx_fm = ( range_data[i].angle_fm <= fminangle ) ? 0 : int ( ( range_data[i].angle_fm - fminangle ) / fangle_increment );
            int idx_tm = int ( ( range_data[i].angle_to - fminangle ) / fangle_increment );
            if ( idx_tm > js_value.size() - 1 )
            {
                idx_tm = js_value.size() - 1;
            }
            double min_dis = 100;
            for ( int j = idx_fm; j < idx_tm; j++ )
            {
                double curdis = js_value[j].asDouble();
                if ( curdis > CHECK_OBSTANCLE_MIN_DISTANCE && curdis < min_dis )
                {
                    min_dis = curdis;
                }
            }
            js_result[CHASSISS_POS_DISTANCE].append ( min_dis );
        }
        /// 构造输出Json
        Json::FastWriter writer;
        std::string strjson = writer.write ( js_result );
        strjson = GetUtility()->ReplaceAllDistinct ( strjson, "\\r\\n", "" );
        #ifdef _DEBUG
        YSOS_LOG_DEBUG ( "Output:" << strjson );
        #endif
        GetBufferUtility()->CopyStringToBuffer ( strjson, output_buffer_ptr );
        free ( range_data );
        return YSOS_ERROR_SUCCESS;
    }

    int GaussianChassisDriver::IoctlGetGreetStatusCode ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlGetGreetStatusCode" );
        if ( !is_websocket_connect_greet_ )
        {
            YSOS_LOG_WARN ( "WebSocket not connect greet status" );
            return YSOS_ERROR_FAILED;
        }
        int statuscode = socket_endpoint_.get_metadata ( WEBSOCKET_GREET_ID )->get_greet_status_code();
        #ifdef _DEBUG
        YSOS_LOG_DEBUG ( "Device Return Greet Status Code: " << statuscode << "!!!!!!!!" );
        #endif
        if ( output_buffer_ptr == NULL )
        {
            YSOS_LOG_ERROR ( "Output Buffer Exception!" );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        UINT8 *outbufferptr;
        UINT32 outbuffersize;
        output_buffer_ptr->GetBufferAndLength ( &outbufferptr, &outbuffersize );
#ifdef _WIN32
        sprintf_s ( ( char* ) outbufferptr, 1024, "{\"" CHASSISS_GREET_STATUS_CODE  "\":%d}", statuscode );
#else
        sprintf ( ( char* ) outbufferptr, "{\"" CHASSISS_GREET_STATUS_CODE  "\":%d}", statuscode );
#endif
        return YSOS_ERROR_SUCCESS;
    }

    int GaussianChassisDriver::IoctlCheckErrorStatus ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "IoctlCheckErrorStatus" );
        if ( !is_websocket_connect_greet_ )
        {
            YSOS_LOG_WARN ( "WebSocket not connect greet status" );
            return YSOS_ERROR_FAILED;
        }
        int statuscode = socket_endpoint_.get_metadata ( WEBSOCKET_GREET_ID )->get_error_status_code();
        if ( statuscode != 0 )
        {
            YSOS_LOG_ERROR ( "get_error_status_code: " << statuscode );
            switch ( statuscode )
            {
                case 1005:
                    SendStatusEvent ( "000100000802000005", "laser param not valid" );
                    break;
                case 1006:
                    SendStatusEvent ( "000100000802000006", "localization lost" );
                    break;
                case 1010:
                    SendStatusEvent ( "000100000802000007", "usb security not valid" );
                    break;
                default:
                    break;
            }
        }
        return YSOS_ERROR_SUCCESS;
    }

    int GaussianChassisDriver::GetChassisVersion()
    {
        YSOS_LOG_DEBUG ( "GetChassisVersion." );
        BufferInterfacePtr output_buffer_ptr = GetBufferUtility()->CreateBuffer ( 2 * MAX_LENGTH256 );
        const char* strhttpobjname = HTTP_OBJECT_NAME_GETSDKVERSION;
        if ( YSOS_ERROR_SUCCESS != http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) strhttpobjname, output_buffer_ptr ) )
        {
            YSOS_LOG_WARN ( "http_method_get Fail." );
            return YSOS_ERROR_FAILED;
        }
        /// 解析Http Get结果
        uint8_t *szBuffer = GetBufferUtility()->GetBufferData ( output_buffer_ptr );
        if ( NULL == szBuffer )
        {
            YSOS_LOG_ERROR ( "Output Buffer is NULL." );
            return YSOS_ERROR_FAILED;
        }
        std::string strresult = reinterpret_cast<char*> ( szBuffer );
        if ( 0 == strresult.length() )
        {
            YSOS_LOG_WARN ( "HTTP Return Length is 0!" );
            return YSOS_ERROR_FAILED;
        }
        #ifdef _DEBUG
        YSOS_LOG_DEBUG ( "Http Get Result:" << strresult );
        #endif
        if ( strresult.find ( RESULT_SUCCESS ) == std::string::npos )
        {
            YSOS_LOG_WARN ( "HTTP Return Result Exception:" << strresult );
            return YSOS_ERROR_FAILED;
        }
        boost::property_tree::ptree ptjson;
        std::istringstream stream ( strresult );
        boost::property_tree::json_parser::read_json ( stream, ptjson );
        std::string ret_successed = ptjson.get<std::string> ( RESULT_SUCCESS, "" );
        if ( 0 != ret_successed.compare ( RESULT_TRUE ) )
        {
            std::string ret_errorcode = ptjson.get<std::string> ( RESULT_ERRCODE, "" );
            YSOS_LOG_WARN ( "Get SDK Version Fail, the ErrorCode is:" << ret_errorcode.c_str() );
            return YSOS_ERROR_FAILED;
        }
        boost::property_tree::ptree ptjsonchild = ptjson.get_child ( RESULT_DATA );
        version_sdk_ = ptjsonchild.get<std::string> ( CHASSIS_SDK_VERSION, "" );
        chassis_module_type_ = ptjsonchild.get<std::string> ( CHASSIS_SDK_MODELTYPE, "" );
        chassis_productId_ = ptjsonchild.get<std::string> ( CHASSIS_SDK_PRODUCTID, "" );
        return YSOS_ERROR_SUCCESS;
    }

    int GaussianChassisDriver::GetDirectionPoint ( const unsigned int idirection, float &fposx, float &fposy, float &fangle )
    {
        YSOS_LOG_DEBUG ( "GetDirectionPoint Current Position: " << fposx << "," << fposy << "," << fangle );
        if ( idirection < 0 || idirection > 3 )
        {
            YSOS_LOG_ERROR ( "GetDirectionPoint direction error, the direction is " << idirection );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        if ( TDDirection_FORWARD == idirection )
        {
            fposx = ( float ) ( fposx + ( EXCHANGE_DIRECT_RATIO ) * cos ( fangle / 180 * M_PI ) ); ///< move 35cm
            fposy = ( float ) ( fposy + ( EXCHANGE_DIRECT_RATIO ) * sin ( fangle / 180 * M_PI ) );
        }
        else if ( TDDirection_BACKWARD == idirection )
        {
            fposx = ( float ) ( fposx - ( EXCHANGE_DIRECT_RATIO ) * cos ( fangle / 180 * M_PI ) );
            fposy = ( float ) ( fposy - ( EXCHANGE_DIRECT_RATIO ) * sin ( fangle / 180 * M_PI ) );
        }
        else if ( TDDirection_TURNLEFT == idirection )
        {
            fangle = ( ( fangle + 90 ) > 180 ) ? ( fangle - 270 ) : ( fangle + 90 );
        }
        else if ( TDDirection_TURNRIGHT == idirection )
        {
            fangle = ( ( fangle - 90 ) < -180 ) ? ( fangle + 270 ) : ( fangle - 90 );
        }
        YSOS_LOG_DEBUG ( "Get Direction Position: " << fposx << "," << fposy << "," << fangle );
        return YSOS_ERROR_SUCCESS;
    }

    int GaussianChassisDriver::ClearLatestNavigationStatus()
    {
        if ( !is_websocket_connect_navigation_ )
        {
            YSOS_LOG_WARN ( "WebSocket not connect navigation status" );
            return YSOS_ERROR_FAILED;
        }
        socket_endpoint_.get_metadata ( WEBSOCKET_NAVIGATION_ID )->clear_latest_status();
        return YSOS_ERROR_SUCCESS;
    }

    int GaussianChassisDriver::Check_Http_Result ( BufferInterfacePtr input_buffer_ptr, bool bhave_data )
    {
        UINT8 *buff_ptr = NULL;
        if ( input_buffer_ptr != NULL )
        {
            buff_ptr = GetBufferUtility()->GetBufferData ( input_buffer_ptr );
        }
        if ( NULL == buff_ptr )
        {
            YSOS_LOG_ERROR ( "Check HTTP Input Buffer Exception!" );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        #ifdef _DEBUG
        YSOS_LOG_DEBUG ( "HTTP Return Result:" << reinterpret_cast<char*> ( buff_ptr ) );
        #endif
        Json::Reader js_reader;
        Json::Value js_value;
        if ( !js_reader.parse ( reinterpret_cast<char*> ( buff_ptr ), js_value, false ) )
        {
            YSOS_LOG_ERROR ( "parse error" );
            return YSOS_ERROR_FAILED;
        }
        if ( !js_value.isMember ( RESULT_SUCCESS ) )
        {
            return YSOS_ERROR_FAILED;
        }
        if ( !js_value[RESULT_SUCCESS].asBool() )
        {
            YSOS_LOG_WARN ( "HTTP Return ErrorCode:" << js_value[RESULT_ERRCODE].asString() << "!" );
#ifdef _WIN32
            if ( stricmp ( js_value[RESULT_ERRCODE].asString().c_str(), "TM_TASK_MAP_NOT_SETTED" ) == 0 )
            {
                YSOS_LOG_WARN ( "GAUSSIAN CHASSIS NOT LOAD MAP" );
                //没有加载地图，需要重新初始化底盘
                // load the map
                char szhttpobjname[MAX_LENGTH256] = {'\0'};
                std::string strmapname = "office";
                YSOS_LOG_DEBUG ( CHASSISS_MAP_NAME ": " << strmapname );
                sprintf_s ( szhttpobjname, MAX_LENGTH256, HTTP_OBJECT_NAME_LOADMAP, strmapname.c_str() );
                http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) szhttpobjname, input_buffer_ptr ) ;
                // wait to load the map
                SLEEP ( connect_sleep_time_ );
                std::string strpositionname = "Current";
                // rolocate
                sprintf_s ( szhttpobjname, MAX_LENGTH256,  HTTP_OBJECT_NAME_DIRECT_INIT, strmapname.c_str(), strpositionname.c_str() );
                http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) szhttpobjname, input_buffer_ptr );
            }
#else
            if ( strcasecmp ( js_value[RESULT_ERRCODE].asString().c_str(), "TM_TASK_MAP_NOT_SETTED" ) == 0 )
            {
                YSOS_LOG_WARN ( "GAUSSIAN CHASSIS NOT LOAD MAP" );
                //没有加载地图，需要重新初始化底盘
                // load the map
                char szhttpobjname[MAX_LENGTH256] = {'\0'};
                std::string strmapname = "office";
                YSOS_LOG_DEBUG ( CHASSISS_MAP_NAME ": " << strmapname );
                sprintf ( szhttpobjname, HTTP_OBJECT_NAME_LOADMAP, strmapname.c_str() );
                http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) szhttpobjname, input_buffer_ptr ) ;
                // wait to load the map
                SLEEP ( connect_sleep_time_ );
                std::string strpositionname = "Current";
                // rolocate
                sprintf ( szhttpobjname,  HTTP_OBJECT_NAME_DIRECT_INIT, strmapname.c_str(), strpositionname.c_str() );
                http_method_get ( ( UINT8* ) HTTP_SERVER_ADDR, server_port_, ( UINT8* ) szhttpobjname, input_buffer_ptr );
            }
#endif
            return YSOS_ERROR_FAILED;
        }
        if ( bhave_data )
        {
            GetBufferUtility()->CopyStringToBuffer ( js_value[RESULT_DATA].asString(), input_buffer_ptr );
        }
        return YSOS_ERROR_SUCCESS;
    }




    /////////////////////////////////////////////////////////////////////////////
    //////////////////////////////// WebSocket //////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////
    int GaussianChassisDriver::websocket_connect_device_status ( UINT8 *server_addr_ptr, UINT32 iport, UINT8 *object_name_ptr )
    {
        return websocket_connect_status ( WEBSOCKET_DEVICE_ID, server_addr_ptr, iport, object_name_ptr );
    }

    int GaussianChassisDriver::websocket_close_device_status ( void )
    {
        return websocket_close_status ( WEBSOCKET_DEVICE_ID );
    }

    int GaussianChassisDriver::websocket_connect_navigation_status ( UINT8 *server_addr_ptr, UINT32 iport, UINT8 *object_name_ptr )
    {
        return  websocket_connect_status ( WEBSOCKET_NAVIGATION_ID, server_addr_ptr, iport, object_name_ptr );
    }

    int GaussianChassisDriver::websocket_close_navigation_status ( void )
    {
        return websocket_close_status ( WEBSOCKET_NAVIGATION_ID );
    }

    int GaussianChassisDriver::websocket_connect_health_status ( UINT8 *server_addr_ptr, UINT32 iport, UINT8 *object_name_ptr )
    {
        return websocket_connect_status ( WEBSOCKET_SYSHEALTH_ID, server_addr_ptr, iport, object_name_ptr );
    }

    int GaussianChassisDriver::websocket_close_health_status ( void )
    {
        return websocket_close_status ( WEBSOCKET_SYSHEALTH_ID );
    }

    int GaussianChassisDriver::websocket_connect_greet_status ( UINT8 *server_addr_ptr, UINT32 iport, UINT8 *object_name_ptr )
    {
        return  websocket_connect_status ( WEBSOCKET_GREET_ID, server_addr_ptr, iport, object_name_ptr );
    }

    int GaussianChassisDriver::websocket_close_greet_status ( void )
    {
        return websocket_close_status ( WEBSOCKET_GREET_ID );
    }


    /////////////////////////////////////////////////////////////////////////////
    ///////////////////////////// HTTP GET/POST API /////////////////////////////
    /////////////////////////////////////////////////////////////////////////////

    int GaussianChassisDriver::http_method_get ( UINT8 *server_addr_ptr, UINT32 iport, UINT8 *object_name_ptr, BufferInterfacePtr output_data_ptr )
    {
        int iret = YSOS_ERROR_SUCCESS;
        char szurl[HTTP_URL_LENGTH] = {'\0'};
        CURL *curl_ptr = NULL;
        //boost::lock_guard<boost::mutex> http_read_lock(mutex_lock_);
        if ( NULL == server_addr_ptr )
        {
            YSOS_LOG_ERROR ( "Server Address Exception!" );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        curl_ptr = curl_easy_init();
        if ( NULL == curl_ptr )
        {
            YSOS_LOG_WARN ( "curl_easy_init Fail" );
            return YSOS_ERROR_FAILED;
        }
#ifdef _WIN32        
        if ( NULL != object_name_ptr )
        {
            sprintf_s ( szurl, HTTP_URL_LENGTH, "http://%s:%d%s", server_addr_ptr, iport, object_name_ptr );
        }
        else
        {
            sprintf_s ( szurl, HTTP_URL_LENGTH, "http://%s:%d", server_addr_ptr, iport );
        }
#else
        if ( NULL != object_name_ptr )
        {
            sprintf ( szurl, "http://%s:%d%s", server_addr_ptr, iport, object_name_ptr );
        }
        else
        {
            sprintf ( szurl, "http://%s:%d", server_addr_ptr, iport );
        }
#endif
        YSOS_LOG_DEBUG ( "GET Url:" << szurl );
        #ifdef  TEST
        curl_easy_setopt ( curl_ptr, CURLOPT_VERBOSE, 1 );
        #ifdef _DEBUG
        curl_easy_setopt ( curl_ptr, CURLOPT_DEBUGFUNCTION, use_debug_info );
        #endif
        #endif
        GetBufferUtility()->SetBufferLength ( output_data_ptr, 0 );
        CURLcode curl_code = curl_easy_setopt ( curl_ptr, CURLOPT_URL, szurl );
        if ( CURLE_OK != curl_code )
        {
            YSOS_LOG_WARN ( "curl_easy_setopt Fail:" << curl_easy_strerror ( curl_code ) );
        }
        curl_code = curl_easy_setopt ( curl_ptr, CURLOPT_READFUNCTION, NULL );
        if ( CURLE_OK != curl_code )
        {
            YSOS_LOG_WARN ( "curl_easy_setopt Fail:" << curl_easy_strerror ( curl_code ) );
        }
        curl_code = curl_easy_setopt ( curl_ptr, CURLOPT_WRITEFUNCTION, &GaussianChassisDriver::http_receive_callback );
        if ( CURLE_OK != curl_code )
        {
            YSOS_LOG_WARN ( "curl_easy_setopt Fail:" << curl_easy_strerror ( curl_code ) );
        }
        curl_code = curl_easy_setopt ( curl_ptr, CURLOPT_WRITEDATA, ( void * ) &output_data_ptr );
        if ( CURLE_OK != curl_code )
        {
            YSOS_LOG_WARN ( "curl_easy_setopt Fail:" << curl_easy_strerror ( curl_code ) );
        }
        ///当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作
        ///如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出
        curl_code = curl_easy_setopt ( curl_ptr, CURLOPT_NOSIGNAL, 1 );
        if ( CURLE_OK != curl_code )
        {
            YSOS_LOG_WARN ( "curl_easy_setopt Fail:" << curl_easy_strerror ( curl_code ) );
        }
        curl_code = curl_easy_setopt ( curl_ptr, CURLOPT_CONNECTTIMEOUT, http_time_out_ );
        if ( CURLE_OK != curl_code )
        {
            YSOS_LOG_WARN ( "curl_easy_setopt Fail:" << curl_easy_strerror ( curl_code ) );
        }
        curl_code = curl_easy_setopt ( curl_ptr, CURLOPT_TIMEOUT, http_time_out_ );
        if ( CURLE_OK != curl_code )
        {
            YSOS_LOG_WARN ( "curl_easy_setopt Fail:" << curl_easy_strerror ( curl_code ) );
        }
        /*curl_code = curl_easy_setopt(curl_ptr, CURLOPT_FORBID_REUSE, 1);
        if(CURLE_OK!=curl_code){
          YSOS_LOG_WARN("curl_easy_setopt Fail:"<<curl_easy_strerror(curl_code));
        }*/
        try
        {
            curl_code = curl_easy_perform ( curl_ptr );
            if ( CURLE_OK != curl_code )
            {
                YSOS_LOG_WARN ( "curl_easy_perform Fail:" << curl_easy_strerror ( curl_code ) );
                iret = YSOS_ERROR_FAILED;
            }
            curl_easy_cleanup ( curl_ptr );
        }
        catch ( ... )
        {
            YSOS_LOG_WARN ( "curl_easy_perform Exception" );
            return YSOS_ERROR_FAILED;
        }
        return iret;
    }

    int GaussianChassisDriver::http_method_post ( UINT8 *server_addr_ptr, UINT32 iport, UINT8 *object_name_ptr, UINT8 *input_data_ptr, BufferInterfacePtr output_data_ptr )
    {
        int iret = YSOS_ERROR_SUCCESS;
        char szurl[HTTP_URL_LENGTH] = {'\0'};
        CURL *curl_ptr = NULL;
        //boost::lock_guard<boost::mutex> http_post_lock(mutex_lock_);
        if ( NULL == server_addr_ptr )
        {
            YSOS_LOG_ERROR ( "Server Address Exception!" );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        curl_ptr = curl_easy_init();
        if ( NULL == curl_ptr )
        {
            YSOS_LOG_WARN ( "curl_easy_init Fail" );
            return YSOS_ERROR_FAILED;
        }
#ifdef _WIN32
        if ( NULL != object_name_ptr )
        {
            sprintf_s ( szurl, HTTP_URL_LENGTH, "http://%s:%d%s", server_addr_ptr, iport, object_name_ptr );
        }
        else
        {
            sprintf_s ( szurl, HTTP_URL_LENGTH, "http://%s:%d", server_addr_ptr, iport );
        }
#else
        if ( NULL != object_name_ptr )
        {
            sprintf ( szurl, "http://%s:%d%s", server_addr_ptr, iport, object_name_ptr );
        }
        else
        {
            sprintf ( szurl, "http://%s:%d", server_addr_ptr, iport );
        }
#endif
        YSOS_LOG_DEBUG ( "POST Url:" << szurl );
        #ifdef _DEBUG
        if ( NULL != input_data_ptr )
        {
            char *input_ptr = ( char* ) input_data_ptr;
            YSOS_LOG_DEBUG ( "Http Post Data:" << input_ptr );
        }
        #endif
        #ifdef TEST
        curl_easy_setopt ( curl_ptr, CURLOPT_VERBOSE, 1 );
        #ifdef _DEBUG
        curl_easy_setopt ( curl_ptr, CURLOPT_DEBUGFUNCTION, use_debug_info );
        #endif
        #endif
        GetBufferUtility()->SetBufferLength ( output_data_ptr, 0 );
        CURLcode curl_code = curl_easy_setopt ( curl_ptr, CURLOPT_URL, szurl );
        if ( CURLE_OK != curl_code )
        {
            YSOS_LOG_WARN ( "curl_easy_setopt Fail:" << curl_easy_strerror ( curl_code ) );
        }
        curl_code = curl_easy_setopt ( curl_ptr, CURLOPT_POST, 1 );
        if ( CURLE_OK != curl_code )
        {
            YSOS_LOG_WARN ( "curl_easy_setopt Fail:" << curl_easy_strerror ( curl_code ) );
        }
        curl_code = curl_easy_setopt ( curl_ptr, CURLOPT_POSTFIELDS, input_data_ptr );
        if ( CURLE_OK != curl_code )
        {
            YSOS_LOG_WARN ( "curl_easy_setopt Fail:" << curl_easy_strerror ( curl_code ) );
        }
        curl_code = curl_easy_setopt ( curl_ptr, CURLOPT_READFUNCTION, NULL );
        if ( CURLE_OK != curl_code )
        {
            YSOS_LOG_WARN ( "curl_easy_setopt Fail:" << curl_easy_strerror ( curl_code ) );
        }
        curl_code = curl_easy_setopt ( curl_ptr, CURLOPT_WRITEFUNCTION, &GaussianChassisDriver::http_receive_callback );
        if ( CURLE_OK != curl_code )
        {
            YSOS_LOG_WARN ( "curl_easy_setopt Fail:" << curl_easy_strerror ( curl_code ) );
        }
        curl_code = curl_easy_setopt ( curl_ptr, CURLOPT_WRITEDATA, ( void * ) &output_data_ptr );
        if ( CURLE_OK != curl_code )
        {
            YSOS_LOG_WARN ( "curl_easy_setopt Fail:" << curl_easy_strerror ( curl_code ) );
        }
        /// 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作
        /// 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出
        curl_code = curl_easy_setopt ( curl_ptr, CURLOPT_NOSIGNAL, 1 );
        if ( CURLE_OK != curl_code )
        {
            YSOS_LOG_WARN ( "curl_easy_setopt Fail:" << curl_easy_strerror ( curl_code ) );
        }
        curl_code = curl_easy_setopt ( curl_ptr, CURLOPT_CONNECTTIMEOUT, http_time_out_ );
        if ( CURLE_OK != curl_code )
        {
            YSOS_LOG_WARN ( "curl_easy_setopt Fail:" << curl_easy_strerror ( curl_code ) );
        }
        curl_code = curl_easy_setopt ( curl_ptr, CURLOPT_TIMEOUT, http_time_out_ );
        if ( CURLE_OK != curl_code )
        {
            YSOS_LOG_WARN ( "curl_easy_setopt Fail:" << curl_easy_strerror ( curl_code ) );
        }
        /*curl_code = curl_easy_setopt(curl_ptr, CURLOPT_FORBID_REUSE, 1);
        if(CURLE_OK!=curl_code){
          YSOS_LOG_WARN("curl_easy_setopt Fail:"<<curl_easy_strerror(curl_code));
        }*/
        try
        {
            curl_code = curl_easy_perform ( curl_ptr );
            if ( CURLE_OK != curl_code )
            {
                YSOS_LOG_WARN ( "curl_easy_perform Fail:" << curl_easy_strerror ( curl_code ) );
                iret = YSOS_ERROR_FAILED;
            }
            curl_easy_cleanup ( curl_ptr );
        }
        catch ( ... )
        {
            YSOS_LOG_WARN ( "curl_easy_perform Exception" );
            return YSOS_ERROR_FAILED;
        }
        return iret;
    }

    size_t GaussianChassisDriver::http_receive_callback ( void *ptr, size_t size, size_t nmemb, void *response )
    {
        YSOS_LOG_DEBUG_CUSTOM ( s_logtag, "http_receive_callback size:" << size << ",nmemb:" << nmemb );
        if ( NULL == ptr )
        {
            YSOS_LOG_ERROR_CUSTOM ( s_logtag, "http_receive_callback pointer exception" );
            return YSOS_ERROR_FAILED;
        }
        #ifdef _DEBUG
        char *dataptr = ( char* ) ptr;
        YSOS_LOG_DEBUG_CUSTOM ( s_logtag, "Http Receive Result:" << dataptr );
        #endif
        size *= nmemb;
        BufferInterfacePtr* buff = reinterpret_cast<BufferInterfacePtr*> ( response );
        if ( NULL != buff )
        {
            UINT8* bufferptr = NULL;
            UINT32 bufferlength = 0;
            ( *buff )->GetBufferAndLength ( &bufferptr, &bufferlength );
            if ( NULL != bufferptr )
            {
                UINT32 buffermaxlength = GetBufferUtility()->GetBufferMaxLength ( *buff );
                if ( buffermaxlength > bufferlength + size )
                {
                    memcpy ( ( char* ) bufferptr + bufferlength, ptr, size );
                    bufferlength += size;
                    // add '\0' at the end
                    bufferptr[bufferlength] = '\0';
                    // but '\0' is not counted in the bufferlength
                    GetBufferUtility()->SetBufferLength ( *buff, bufferlength );
                }
                else
                {
                    YSOS_LOG_ERROR_CUSTOM ( s_logtag, "buffer size is not enough " << buffermaxlength << " " << ( bufferlength + size ) );
                }
            }
        }
        return size;
    }

    /////////////////////////////////////////////////////////////////////////////
    //////////////////////////////// Web Socket API /////////////////////////////
    /////////////////////////////////////////////////////////////////////////////

    int GaussianChassisDriver::websocket_connect_status ( int itype_id, UINT8 *server_addr_ptr, UINT32 iport, UINT8 *object_name_ptr )
    {
        char szurl[HTTP_URL_LENGTH] = {'\0'};
        if ( itype_id < 0 || NULL == server_addr_ptr || NULL == object_name_ptr )
        {
            YSOS_LOG_ERROR ( "Web Socket Connect Status Parameter Error" );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
#ifdef _WIN32
        sprintf_s ( szurl, HTTP_URL_LENGTH, "ws://%s:%d%s", server_addr_ptr, iport, object_name_ptr );
#else
        sprintf ( szurl, "ws://%s:%d%s", server_addr_ptr, iport, object_name_ptr );
#endif
        YSOS_LOG_DEBUG ( "Device Status Url:" << szurl );
        if ( socket_endpoint_.connect ( itype_id, szurl, NULL ) < 0 )
        {
            YSOS_LOG_WARN ( "Web Socket Connect Status " << itype_id << " Fail" );
            return YSOS_ERROR_FAILED;
        }
        return YSOS_ERROR_SUCCESS;
    }

    int GaussianChassisDriver::websocket_close_status ( int itype_id )
    {
        if ( itype_id < 0 )
        {
            YSOS_LOG_ERROR ( "Web Socket Close Connect Status Parameter Error" );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        int close_code = websocketpp::close::status::normal;
        socket_endpoint_.close ( itype_id, close_code, "Normal Close Status" );
        YSOS_LOG_DEBUG ( "Web Socket Close Connect Status " << itype_id << " Finish" );
        return YSOS_ERROR_SUCCESS;
    }



}
