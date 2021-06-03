/**
*@file powermanagermodule.cpp
*@brief power manager
*@version 1.0
*@author Wang Xiaogui
*@date Created on: 2016-11-25 10:15:00
*@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
* 
*/

/// MySelf Headers
#include "../include/powermanagermodule.h"

/// YSOS Header //NOLINT
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/manager_declare.h"
#include <json/json.h>

#ifndef SLEEP
    #define SLEEP(a)      (boost::this_thread::sleep_for(boost::chrono::milliseconds(a)))
#endif

/**
*@brief 通信信息
*/
const int MIN_MSG_LENGTH = 5;                 ///< 最小帧长度
const int MIN_COMMUNICATE_GAT = 500;          ///< 通信最小间隔
const int THREAD_SLEEP_TIME = 250;            ///< 通信线程睡眠间隔

const int MODE_VOLT_VAILD_CNT = 5;            ///< 连续获得电压模式，才使用电压模式

/**
*@brief 帧信息
*/
#define PROP_FRAME_HEADER  0x02                ///< 帧头               //NOLINT
#define PROP_FRAME_TAIL    0x03                ///< 帧尾               //NOLINT
#define PROP_FRAME_VERSION 0x01                ///< 版本信息             //NOLINT
#define PROP_FRAME_TRUN    0x1B                ///< 帧中的信息转义字节        //NOLINT

/**
*@brief 发送数据结长度
*/
#define PROP_SYNCH_SYSTEM_LEN  0x07    //NOLINT
#define PROP_WATCHDOG_OPEN_LEN 0x03    //NOLINT

/**
*@brief 模块代码
*/
enum DeviceCtrlCode
{
    PROP_DEVICECTRL_COMMON     = 0x00,        ///< 公共设备，所有设备都适用         //NOLINT
    PROP_DEVICECTRL_HEADER     = 0x01,        ///< 适用设备：机器人头部             //NOLINT
    PROP_DEVICECTRL_LED        = 0x04,        ///< 适用设备：led灯（耳朵上的led灯） //NOLINT
    PROP_DEVICECTRL_MICROARRAY = 0x05,        ///< 适用设备：麦克风阵列             //NOLINT
    PROP_DEVICECTRL_ELECTRIC   = 0x06,        ///< 适用设备：电源监控板             //NOLINT
};

/**
*@brief 看门狗
*/
enum WatchDogStatus
{
    PROP_WATCHDOG_DISENABLE       = 0x00,        ///< 未使能看门狗                 //NOLINT
    PROP_WATCHDOG_ENABLE          = 0x01,        ///< 使能看门狗                   //NOLINT
};

/**
*@brief 看门狗重启模式
*/
enum WatchDogRebootMode
{
    PROP_WATCHDOG_UPBODY           = 0x01,        ///< 重启上半身                 //NOLINT
    PROP_WATCHDOG_DOWNBODY         = 0x02,        ///< 重启下半身                 //NOLINT
    PROP_WATCHDOG_ALLBODY          = 0x04,        ///< 重启全身                  //NOLINT
};

/**
*@brief 发送数据命令码
*/
enum CtlCommand
{
    CMD_POWERMGR_TURN_OFF     = 0x24,     ///< 关机命令                     //NOLINT
    CMD_CLOSE_UPBODY_DOWNLOCK = 0x25,     ///< 上半身关机，下半身锁定       //NOLINT
    CMD_CLOSE_ALLBODY         = 0x26,     ///< 全身关机                     //NOLINT
    CMD_CLOSE_UPBODY          = 0x27,     ///< 上半身关机                   //NOLINT

    CMD_POWERMGR_BATTERY      = 0x28,     ///< 电量相关, @SubCmdBattery     //NOLINT

    CMD_POWERMGR_SYSTIME      = 0x29,     ///< 开关机相关, @SubCmdSystime   //NOLINT

    CMD_REBOOT_UPBODY         = 0x40,     ///< 重启上半身                   //NOLINT
    CMD_REBOOT_DOWNBODY       = 0x41,     ///< 重启下半身                   //NOLINT
    CMD_REBOOT_ALLBODY        = 0x42,     ///< 重启全身身                   //NOLINT

    CMD_POWERMGR_WATCHDOG     = 0x43,     ///< 看门狗相关, @SubCmdWatchdog  //NOLINT
};

enum SubCmdBattery
{
    CMD_POWER_MODEVOLT         = 0x00,    ///< 设置查询电量为读电压模式  //NOLINT
    CMD_POWER_MODEBATTERY      = 0x01,    ///< 设置查询电量为读电池模式  //NOLINT
    CMD_POWER_QUERYBATTERY     = 0x02,    ///< 查询电量                 //NOLINT
    CMD_POWER_QUERYMODE        = 0xaa,    ///< 查询电量模式，电压或电池  //NOLINT
};

enum SubCmdSystime
{
    CMD_SYNCH_CURRENTSYS_TIME  = 0x01,    ///< 同步当前系统时间            //NOLINT
    CMD_SET_OPENSYS_TIME       = 0x02,    ///< 设置开机时间              //NOLINT
    CMD_SET_CLOSESYS_TIME      = 0x03,    ///< 设置关机时间              //NOLINT
    CMD_CANCEL_OPENSYS_TIME    = 0x04,    ///< 取消开机时间              //NOLINT
    CMD_CANCEL_CLOSESYS_TIME   = 0x05,    ///< 取消关机时间              //NOLINT
};

enum SubCmdWatchdog
{
    CMD_WATCHDOG_CLOSE        = 0x00,     ///< 关闭看门狗                //NOLINT
    CMD_WATCHDOG_OPEN         = 0x01,     ///< 开启看门狗， 数据：左两个字节为时间间隔，单位为秒，最右边一个字节为重启 //NOLINT
    CMD_WATCHDOG_WATCH        = 0x02,     ///< 喂狗                 //NOLINT
    CMD_WATCHDOG_QUERY        = 0xaa,     ///< 查询看门狗状态            //NOLINT
};

enum SubCmdChange
{
    CMD_NOT_CHARGING          = 0x10,     ///< 没有充电中            //NOLINT
    CMD_CHARGING              = 0x20,     ///< 充电中                //NOLINT
};

#ifdef _DEBUG
std::string bytestohexstring ( char* bytes, int bytelength )
{
    std::string str ( "" );
    std::string str2 ( "0123456789abcdef" );
    for ( int i = 0; i < bytelength; i++ )
    {
        int b;
        b = 0x0f & ( bytes[i] >> 4 );
        char s1 = str2.at ( b );
        str.append ( 1, str2.at ( b ) );
        b = 0x0f & bytes[i];
        str.append ( 1, str2.at ( b ) );
        char s2 = str2.at ( b );
    }
    return str;
}
#endif

namespace ysos {

    DECLARE_PLUGIN_REGISTER_INTERFACE ( PowerManagerModule, ModuleInterface );
    PowerManagerModule::PowerManagerModule ( const std::string &strClassName ) : BaseThreadModuleImpl ( strClassName )
        , powermanager_driver_ptr_ ( NULL )
        , module_callback_ptr_ ( NULL )
        , releate_driver_name_ ( "default@PowerManagerDriver1" )
        , releate_callback_name_ ( "default@PowerManagerModuleCallback1" )
        , icom_port_ ( 0 )

        , watchdog_enable_ ( PROP_WATCHDOG_ENABLE )
        , watchdog_time_ ( 300 )
        , watchdog_reboot_mode_ ( PROP_WATCHDOG_UPBODY )

        , barrery_elect_ ( 0xff )
        , barrery_charge_status ( 0xff )
        , power_off_ ( false )

        , powmgr_thread_()
        , thread_interval_ ( 10000 )
    {
        logger_ = GetUtility()->GetLogger ( "ysos.powermgr" );
    }


    int PowerManagerModule::Initialize ( void *param )
    {
        if ( IsInitialized() )
        {
            return YSOS_ERROR_SUCCESS;
        }
        InitalDataInfo();
        if ( YSOS_ERROR_SUCCESS != SetupModuleBuffer() )
        {
            YSOS_LOG_WARN ( "PowerManagerModule Initialize Setup Buffer Fail" );
        }
        if ( NULL != thread_data_ )
        {
            thread_data_->timeout = thread_interval_;
            YSOS_LOG_DEBUG ( "PowerManagerModule Initialize Time Interval:" << thread_interval_ );
        }
        return BaseThreadModuleImpl::Initialize ( param );
    }

    int PowerManagerModule::Initialized ( const std::string &key, const std::string &value )
    {
        YSOS_LOG_DEBUG ( "PowerManagerModule Initialized start:" << key );
        int iret = BaseModuleImpl::Initialized ( key, value );
        if ( key.compare ( "com_port" ) == 0 )                 ///< 串口端口
        {
            icom_port_ = atoi ( value.c_str() );
            YSOS_LOG_DEBUG ( "PowerManager Com Port:" << icom_port_ );
        }
        else if ( key.compare ( "relate_driver" ) == 0 )      ///< 关联的驱动driver的逻辑名
        {
            releate_driver_name_ = value;
            YSOS_LOG_DEBUG ( "PowerManager Name: " << releate_driver_name_ );
        }
        else if ( key.compare ( "relate_callback" ) == 0 )    ///< 关联的回调callback的逻辑名
        {
            releate_callback_name_ = value;
            YSOS_LOG_DEBUG ( "PowerManager Callback Name:" << releate_callback_name_ );
        }
        else if ( key.compare ( "enable_watchdog" ) == 0 )    ///< 是否使能看门狗
        {
            if ( value.compare ( "true" ) == 0 || value.compare ( "TRUE" ) == 0 )
            {
                watchdog_enable_ = PROP_WATCHDOG_ENABLE;
            }
            else
            {
                watchdog_enable_ = PROP_WATCHDOG_DISENABLE;
            }
        }
        else if ( key.compare ( "watchdog_interval" ) == 0 )    ///< 喂狗时间间隔
        {
            int itime = atoi ( value.c_str() );
            watchdog_time_ = itime;
            YSOS_LOG_DEBUG ( "PowerManager Watch Time:" << itime );
        }
        else if ( key.compare ( "watchdog_reboot_mode" ) == 0 )  ///< 看门狗异常重启模式
        {
            int imode = atoi ( value.c_str() );
            watchdog_reboot_mode_ = imode;
            YSOS_LOG_DEBUG ( "PowerManager Watchdog Reboot Mode:" << imode );
        }
        YSOS_LOG_DEBUG ( "PowerManagerModule Initialized Finish" );
        return iret;
    }


    int PowerManagerModule::InitalDataInfo()
    {
        if ( NULL != module_data_info_ )
        {
            module_data_info_->module_type = PROP_SOURCE;
            module_data_info_->in_datatypes = DTSTR_NULL;
            module_data_info_->out_datatypes = DTSTR_TEXT_JSON;
            module_data_info_->is_self_allocator = true;
            // module_data_info_->buffer_length = MAX_BUFFER_SIZE;
            // module_data_info_->buffer_number = MAX_BUFFER_NUM;
            // module_data_info_->prefix_length = 0;
            return YSOS_ERROR_SUCCESS;
        }
        return YSOS_ERROR_FAILED;
    }

    int PowerManagerModule::SetupModuleBuffer()
    {
        return YSOS_ERROR_SUCCESS;
    }

    int PowerManagerModule::GetProperty ( int type_id, void *typeptr )
    {
        int nret = YSOS_ERROR_FAILED;
        if ( NULL == typeptr )
        {
            YSOS_LOG_DEBUG ( "PowerManagerModule GetProperty Arguments Error!" );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        switch ( type_id )
        {
            case PROP_POWERMGR_BATTERY_INFO:
                if ( barrery_elect_ != 0xff )
                {
                    * ( int* ) typeptr = barrery_elect_;
                    nret = YSOS_ERROR_SUCCESS;
                }
                break;
            case PROP_POWERMGR_CHARGE_INFO:
                if ( barrery_charge_status != 0xff )
                {
                    * ( int* ) typeptr = barrery_charge_status;
                    nret = YSOS_ERROR_SUCCESS;
                }
                break;
            case PROP_POWERMGR_POWEROFF_INFO:
                * ( bool* ) typeptr = power_off_;
                nret = YSOS_ERROR_SUCCESS;
                break;
            case PROP_POWERMGR_SERIAL_PORT:
            case PROP_POWERMGR_SERIAL_BAUDRATE:
            case PROP_POWERMGR_SERIAL_PARITY:
            case PROP_POWERMGR_SERIAL_BYTESIZE:
            case PROP_POWERMGR_SERIAL_STOPBITS:
            case PROP_POWERMGR_SERIAL_READ_TIMEOUT:
                if ( powermanager_driver_ptr_ )
                {
                    nret = powermanager_driver_ptr_->GetProperty ( type_id, typeptr );
                }
                break;
            default:
                nret = BaseThreadModuleImpl::GetProperty ( type_id, typeptr );
        }
        return nret;
    }

    int PowerManagerModule::SetProperty ( int type_id, void *typeptr )
    {
        int nret = YSOS_ERROR_SUCCESS;
        switch ( type_id )
        {
            case PROP_POWERMGR_SERIAL_PORT:
            case PROP_POWERMGR_SERIAL_BAUDRATE:
            case PROP_POWERMGR_SERIAL_PARITY:
            case PROP_POWERMGR_SERIAL_BYTESIZE:
            case PROP_POWERMGR_SERIAL_STOPBITS:
            case PROP_POWERMGR_SERIAL_READ_TIMEOUT:
                if ( powermanager_driver_ptr_ )
                {
                    nret = powermanager_driver_ptr_->SetProperty ( type_id, typeptr );
                }
                break;
            default:
                nret = BaseThreadModuleImpl::SetProperty ( type_id, typeptr );
        }
        return nret;
    }

    int PowerManagerModule::Ioctl ( INT32 ctrl_id, LPVOID param /* = NULL */ )
    {
        YSOS_LOG_DEBUG ( "PowerManagerModule::Ioctl " << ctrl_id );
        // temporary code
        if ( ctrl_id == CMD_POWERMGR_THREAD_STOP )
        {
            powmgr_thread_.interrupt();
            powmgr_thread_.join();
            return YSOS_ERROR_SUCCESS;
        }
        if ( ctrl_id == CMD_POWERMGR_THREAD_START )
        {
            powmgr_thread_ = boost::thread ( boost::bind ( Powermgr_Process_Thread, this ) );
            return YSOS_ERROR_SUCCESS;
        }
        // temporary code end
        std::string* stringBuff = reinterpret_cast<std::string*> ( param );
        if ( stringBuff != NULL )
        {
            YSOS_LOG_DEBUG ( *stringBuff );
        }
        BufferInterfacePtr input_buffer_ptr = NULL;
        BufferInterfacePtr output_buffer_ptr = NULL;
        if ( buffer_pool_ptr_ != NULL )
        {
            buffer_pool_ptr_->GetBuffer ( &input_buffer_ptr );
            buffer_pool_ptr_->GetBuffer ( &output_buffer_ptr );
        }
        if ( input_buffer_ptr == NULL || output_buffer_ptr == NULL )
        {
            YSOS_LOG_ERROR ( "Ioctl get buff error" );
            return YSOS_ERROR_FAILED;
        }
#ifdef _WIN32
        int nRet = StructureSendData ( ctrl_id, reinterpret_cast<std::string*> ( param ), output_buffer_ptr, input_buffer_ptr );
        if ( YSOS_ERROR_SUCCESS != nRet )
        {
            return YSOS_ERROR_FAILED;
        }
        nRet = IoctlEx ( input_buffer_ptr, output_buffer_ptr );
        if ( YSOS_ERROR_SUCCESS != nRet )
        {
            return YSOS_ERROR_FAILED;
        }
#else
        //TODO:add for linux
#endif
        if ( NULL != param )
        {
            UINT8 *outbufferptr = NULL;
            UINT32 outbufferlength = 0;
            output_buffer_ptr->GetBufferAndLength ( &outbufferptr, &outbufferlength );
            stringBuff->append ( ( const char* ) outbufferptr, outbufferlength );
        }
        return YSOS_ERROR_SUCCESS;
    }

    int PowerManagerModule::RealOpen ( LPVOID param /*= NULL*/ )
    {
        YSOS_LOG_DEBUG ( "PowerManagerModule RealOpen Function Start!" );
        powermanager_driver_ptr_ = GetDriverInterfaceManager()->FindInterface ( releate_driver_name_ );
        if ( powermanager_driver_ptr_ == NULL )
        {
            YSOS_LOG_ERROR ( "PowerManagerModule RealOpen Find Driver Interface Fail " << releate_driver_name_ );
            return YSOS_ERROR_FAILED;
        }
        /// initialize
        powermanager_driver_ptr_->Initialize ( NULL );
#ifdef _WIN32
        /// 打开串口
        if ( NULL != param )
        {
            if ( YSOS_ERROR_SUCCESS != powermanager_driver_ptr_->Open ( param ) )
            {
                YSOS_LOG_ERROR ( "PowerManagerModule RealOpen Function Open Serial Fail!" );
                return YSOS_ERROR_FAILED;
            }
        }
        else
        {
            if ( YSOS_ERROR_SUCCESS != powermanager_driver_ptr_->Open ( &icom_port_ ) )
            {
                YSOS_LOG_ERROR ( "PowerManagerModule RealOpen Function Open Serial Fail!" );
                return YSOS_ERROR_FAILED;
            }
        }
        data_ptr_ = GetDataInterfaceManager()->GetData ( GLOBAL_DATA_KEY );
        if ( !data_ptr_ )
        {
            YSOS_LOG_DEBUG ( "get data_ptr_ failed" );
            return YSOS_ERROR_FAILED;
        }
        // 同步时间
        Ioctl ( CMD_POWERMGR_SYNCH_CURSYS_TIME, NULL );
        SLEEP ( 350 );
        // 开启线程
        powmgr_thread_ = boost::thread ( boost::bind ( Powermgr_Process_Thread, this ) );
        powmgr_judge_thread_ = boost::thread ( boost::bind ( Powermgr_Judge_Chassis_Thread, this ) );
#else
        data_ptr_ = GetDataInterfaceManager()->GetData ( GLOBAL_DATA_KEY );
        if ( !data_ptr_ )
        {
            YSOS_LOG_DEBUG ( "get data_ptr_ failed" );
            return YSOS_ERROR_FAILED;
        }
        // 同步时间
        //Ioctl ( CMD_POWERMGR_SYNCH_CURSYS_TIME, NULL );
        SLEEP ( 350 );
        // 开启线程
        powmgr_thread_ = boost::thread ( boost::bind ( Powermgr_Process_Thread, this ) );
       //powmgr_judge_thread_ = boost::thread ( boost::bind ( Powermgr_Judge_Chassis_Thread, this ) );
#endif
        return BaseThreadModuleImpl::RealOpen ( param );
    }

    int PowerManagerModule::RealRun()
    {
        return YSOS_ERROR_SUCCESS;
    }

    int PowerManagerModule::RealPause()
    {
        return YSOS_ERROR_SUCCESS;
    }

    int PowerManagerModule::RealStop()
    {
        return YSOS_ERROR_SUCCESS;
    }

    int PowerManagerModule::RealClose()
    {
        YSOS_LOG_DEBUG ( "PowerManagerModule RealClose Start" );
        powmgr_thread_.interrupt();
        powmgr_thread_.join();
        if ( PROP_WATCHDOG_ENABLE == watchdog_enable_ )
        {
            Ioctl ( CMD_POWERMGR_WATCHDOG_CLOSE, NULL );
        }
        int iret = BaseThreadModuleImpl::RealClose();
        if ( YSOS_ERROR_SUCCESS != iret )
        {
            return iret;
        }
        if ( powermanager_driver_ptr_ )
        {
            powermanager_driver_ptr_->Close ( NULL );
        }
        YSOS_LOG_DEBUG ( "PowerManagerModule RealClose End" );
        return YSOS_ERROR_SUCCESS;
    }

    int PowerManagerModule::Powermgr_Process_Thread ( LPVOID lpParam )
    {
        PowerManagerModule *powermgrmodule_ptr = ( PowerManagerModule* ) lpParam;
        if ( lpParam == NULL )
        {
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        BufferInterfacePtr watchdog_buffer_ptr = NULL;
        BufferInterfacePtr battery_buffer_ptr = NULL;
        BufferInterfacePtr output_buffer_ptr = NULL;
        if ( powermgrmodule_ptr->buffer_pool_ptr_ != NULL )
        {
            powermgrmodule_ptr->buffer_pool_ptr_->GetBuffer ( &watchdog_buffer_ptr );
            powermgrmodule_ptr->buffer_pool_ptr_->GetBuffer ( &battery_buffer_ptr );
            powermgrmodule_ptr->buffer_pool_ptr_->GetBuffer ( &output_buffer_ptr );
        }
        if ( watchdog_buffer_ptr == NULL || battery_buffer_ptr == NULL || output_buffer_ptr == NULL )
        {
            YSOS_LOG_DEBUG_CUSTOM ( powermgrmodule_ptr->logger_, "Process Thread get buff error" );
            return YSOS_ERROR_FAILED;
        }
        UINT8 *bufferptr = NULL;
        UINT32 buffersize = 0;
        output_buffer_ptr->GetBufferAndLength ( &bufferptr, &buffersize );
        if ( bufferptr == NULL )
        {
            YSOS_LOG_DEBUG_CUSTOM ( powermgrmodule_ptr->logger_, "get output buff error" );
            return YSOS_ERROR_FAILED;
        }
        int rst = 0;
#ifdef _WIN32
        if ( PROP_WATCHDOG_ENABLE == powermgrmodule_ptr->watchdog_enable_ )
        {
            // enable watchdog
            powermgrmodule_ptr->StructureSendData ( CMD_POWERMGR_WATCHDOG_OPEN, NULL, output_buffer_ptr, watchdog_buffer_ptr );
            rst = powermgrmodule_ptr->IoctlEx ( watchdog_buffer_ptr, output_buffer_ptr );
            YSOS_LOG_DEBUG_CUSTOM ( powermgrmodule_ptr->logger_, "WATCHDOG_OPEN " << rst );
            // structure watchdog watch (ignore the result of WATCHDOG_OPEN)
            powermgrmodule_ptr->StructureSendData ( CMD_POWERMGR_WATCHDOG_WATCH, NULL, output_buffer_ptr, watchdog_buffer_ptr );
        }
        else
        {
            // disable watchdog
            powermgrmodule_ptr->StructureSendData ( CMD_POWERMGR_WATCHDOG_CLOSE, NULL, output_buffer_ptr, watchdog_buffer_ptr );
            rst = powermgrmodule_ptr->IoctlEx ( watchdog_buffer_ptr, output_buffer_ptr );
            YSOS_LOG_DEBUG_CUSTOM ( powermgrmodule_ptr->logger_, "WATCHDOG_CLOSE " << rst );
        }
        SLEEP ( MIN_COMMUNICATE_GAT );
        {
            #if 0
            // set mode battery
            powermgrmodule_ptr->StructureSendData ( CMD_POWERMGR_POWER_SET_MODE_BATTERY, NULL, output_buffer_ptr, battery_buffer_ptr );
            rst = powermgrmodule_ptr->IoctlEx ( battery_buffer_ptr, output_buffer_ptr );
            YSOS_LOG_DEBUG_CUSTOM ( powermgrmodule_ptr->logger_, "SET_MODE_BATTERY " << rst );
            #endif
            // structure query battery
            powermgrmodule_ptr->StructureSendData ( CMD_POWERMGR_POWER_QUERY_BATTERY, NULL, output_buffer_ptr, battery_buffer_ptr );
        }
        SLEEP ( MIN_COMMUNICATE_GAT );
        int vol_cnt = MODE_VOLT_VAILD_CNT - 1; // initially, just wait twice
        boost::posix_time::ptime time_begin;
        boost::posix_time::ptime time_now;
        powermgrmodule_ptr->barrery_elect_ = 0x65;//101%电量	，保证线充时，也可以有电量 的变化，这样，就可以上送电量状态
        while ( true )
        {
            time_begin = boost::posix_time::microsec_clock::universal_time();
            if ( powermgrmodule_ptr->watchdog_enable_ == PROP_WATCHDOG_ENABLE )
            {
                rst = powermgrmodule_ptr->IoctlEx ( watchdog_buffer_ptr, output_buffer_ptr );
                YSOS_LOG_DEBUG_CUSTOM ( powermgrmodule_ptr->logger_, "WATCHDOG_WATCH " << rst );
                SLEEP ( MIN_COMMUNICATE_GAT );
            }
            rst = powermgrmodule_ptr->IoctlEx ( battery_buffer_ptr, output_buffer_ptr );
            /*电源监控版查询电量结果返回=[02010628110703]
            转义后(去掉帧头帧尾)=[0106281107]
            最后两位十六进制：OX07=代表当前电量
                倒数三四位十六进制：OX11=代表线充挂在机器人上，）OX01=代表线充未挂在机器人上*/
            if ( rst == YSOS_ERROR_SUCCESS )
            {
                //判断线充状态
                if ( ( bufferptr[4] & 0x10 ) == 0x10 )
                {
                    powermgrmodule_ptr->barrery_charge_status = 1;
                    YSOS_LOG_DEBUG_CUSTOM ( powermgrmodule_ptr->logger_, "CHASSIS IS Changing " );
                }
                else
                {
                    powermgrmodule_ptr->barrery_charge_status = 0;
                    YSOS_LOG_DEBUG_CUSTOM ( powermgrmodule_ptr->logger_, "CHASSIS IS NOT Changing " );
                }
                //判断电量模式
                if ( ( bufferptr[4] & 0x01 ) == 0x01 )
                {
                    powermgrmodule_ptr->barrery_elect_ = bufferptr[5];
                    vol_cnt = 0;
                    YSOS_LOG_DEBUG_CUSTOM ( powermgrmodule_ptr->logger_, "MODE_BATTERY " << ( int ) bufferptr[5] << "MODE_CHARGE" << ( int ) bufferptr[4] );
                }
                else
                {
                    if ( vol_cnt == MODE_VOLT_VAILD_CNT )
                    {
                        switch ( bufferptr[5] )
                        {
                            case 1:
                                powermgrmodule_ptr->barrery_elect_ = 7;
                                break;
                            case 2:
                                powermgrmodule_ptr->barrery_elect_ = 15;
                                break;
                            case 3:
                                powermgrmodule_ptr->barrery_elect_ = 49;
                                break;
                            case 4:
                                powermgrmodule_ptr->barrery_elect_ = 69;
                                break;
                        }
                    }
                    else
                    {
                        ++vol_cnt;
                    }
                    YSOS_LOG_DEBUG_CUSTOM ( powermgrmodule_ptr->logger_, "MODE_VOLT " << ( int ) bufferptr[5] );
                }
            }
            else
            {
                YSOS_LOG_DEBUG_CUSTOM ( powermgrmodule_ptr->logger_, "QUERY_BATTERY " << rst );
            }
            SLEEP ( MIN_COMMUNICATE_GAT - THREAD_SLEEP_TIME );
            do
            {
                SLEEP ( THREAD_SLEEP_TIME );
                if ( YSOS_ERROR_SUCCESS == powermgrmodule_ptr->ReadEx ( output_buffer_ptr ) )
                {
                    if ( YSOS_ERROR_SUCCESS == powermgrmodule_ptr->CheckPowerOff ( output_buffer_ptr ) )
                    {
                        powermgrmodule_ptr->power_off_ = true;
                        powermgrmodule_ptr->SetProperty ( PROP_THREAD_NOTIFY, NULL );
                        return YSOS_ERROR_SUCCESS;
                    }
                }
                time_now = boost::posix_time::microsec_clock::universal_time();
            } while ( ( time_now - time_begin ).total_milliseconds() < powermgrmodule_ptr->thread_interval_ );
        }
#else    
        SLEEP ( MIN_COMMUNICATE_GAT );
        int vol_cnt = MODE_VOLT_VAILD_CNT - 1; // initially, just wait twice
        boost::posix_time::ptime time_begin;
        boost::posix_time::ptime time_now;
        powermgrmodule_ptr->barrery_elect_ = 0x65;//101%电量	，保证线充时，也可以有电量 的变化，这样，就可以上送电量状态
        powermgrmodule_ptr->barrery_charge_status = 0;
        YSOS_LOG_DEBUG_CUSTOM ( powermgrmodule_ptr->logger_, "MODE_BATTERY " << ( int ) powermgrmodule_ptr->barrery_elect_ );
#endif
        YSOS_LOG_DEBUG_CUSTOM ( powermgrmodule_ptr->logger_, "Powermgr_Process_Thread End" );
        return YSOS_ERROR_SUCCESS;
    }

    int PowerManagerModule::StructureSendData ( int ctrl_id, std::string* param, BufferInterfacePtr temp_buffer_ptr, BufferInterfacePtr rst_buffer_ptr )
    {
        UINT8 *bufferptr = NULL;
        UINT32 buffersize = 0;
        if ( temp_buffer_ptr )
        {
            temp_buffer_ptr->GetBufferAndLength ( &bufferptr, &buffersize );
            temp_buffer_ptr->GetMaxLength ( &buffersize );
        }
        if ( bufferptr == NULL )
        {
            YSOS_LOG_ERROR ( "StructureSendData, but temp memory error" );
            return YSOS_ERROR_FAILED;
        }
        int num = GetCommandId ( ctrl_id, bufferptr[0], bufferptr[1] );
        if ( num == 0 )
        {
            return YSOS_ERROR_FAILED;
        }
        if ( bufferptr[0] == CMD_POWERMGR_SYSTIME )
        {
            if ( bufferptr[1] == CMD_SYNCH_CURRENTSYS_TIME )
            {
                boost::posix_time::ptime localtm = boost::posix_time::microsec_clock::local_time();
                struct TimeParam strusystime;
                strusystime.nYear = ( int ) localtm.date().year();
                strusystime.cbMonth = ( int ) localtm.date().month();
                strusystime.cbDay = ( int ) localtm.date().day();
                strusystime.cbHour = localtm.time_of_day().hours();
                strusystime.cbMin = localtm.time_of_day().minutes();
                strusystime.cbSecond = localtm.time_of_day().seconds();
                int datanum = GetTimeData ( strusystime, bufferptr + num, buffersize - num );
                if ( datanum == 0 )
                {
                    return YSOS_ERROR_FAILED;
                }
                num += datanum;
            }
            else if ( bufferptr[1] == CMD_SET_OPENSYS_TIME || bufferptr[1] == CMD_SET_CLOSESYS_TIME )
            {
                Json::Reader js_reader;
                Json::Value js_value;
                if ( !js_reader.parse ( *param, js_value, false ) )
                {
                    YSOS_LOG_ERROR ( "ParserTimeJson failed!" );
                    return YSOS_ERROR_FAILED;
                }
                struct TimeParam strusystime;
                strusystime.nYear = js_value["year"].asInt();
                strusystime.cbMonth = js_value["month"].asInt();
                strusystime.cbDay = js_value["day"].asInt();
                strusystime.cbHour = js_value["hour"].asInt();
                strusystime.cbMin = js_value["minute"].asInt();
                strusystime.cbSecond = js_value["second"].asInt();
                int datanum = GetTimeData ( strusystime, bufferptr + num, buffersize - num );
                if ( datanum == 0 )
                {
                    return YSOS_ERROR_FAILED;
                }
                num += datanum;
            }
        }
        else if ( bufferptr[0] == CMD_POWERMGR_WATCHDOG && bufferptr[1] == CMD_WATCHDOG_OPEN )
        {
            int datanum = GetWatchdogData ( bufferptr + num, buffersize - num );
            if ( datanum == 0 )
            {
                return YSOS_ERROR_FAILED;
            }
            num += datanum;
        }
        else
        {
        }
        temp_buffer_ptr->SetLength ( num );
        return ConvertSendMsg ( temp_buffer_ptr, rst_buffer_ptr );
    }

    int PowerManagerModule::GetCommandId ( int ctrl_id, UINT8& cmd, UINT8& sub_cmd )
    {
        int cmdnum = 0;
        switch ( ctrl_id )
        {
            /// 关机
            case CMD_POWERMGR_CLOSE_UPBODY_DOWNLOCK:
                cmd = CMD_CLOSE_UPBODY_DOWNLOCK;
                cmdnum = 1;
                break;
            case CMD_POWERMGR_CLOSE_ALLBODY:
                cmd = CMD_CLOSE_ALLBODY;
                cmdnum = 1;
                break;
            case CMD_POWERMGR_CLOSE_UPBODY:
                cmd = CMD_CLOSE_UPBODY;
                cmdnum = 1;
                break;
            /// 电池
            case CMD_POWERMGR_POWER_SET_MODE_VOLT:
                cmd = CMD_POWERMGR_BATTERY;
                sub_cmd = CMD_POWER_MODEVOLT;
                cmdnum = 2;
                break;
            case CMD_POWERMGR_POWER_SET_MODE_BATTERY:
                cmd = CMD_POWERMGR_BATTERY;
                sub_cmd = CMD_POWER_MODEBATTERY;
                cmdnum = 2;
                break;
            case CMD_POWERMGR_POWER_QUERY_BATTERY:
                cmd = CMD_POWERMGR_BATTERY;
                sub_cmd = CMD_POWER_QUERYBATTERY;
                cmdnum = 2;
                break;
            case CMD_POWERMGR_POWER_QUERY_MODE:
                cmd = CMD_POWERMGR_BATTERY;
                sub_cmd = CMD_POWER_QUERYMODE;
                cmdnum = 2;
                break;
            /// 同步开关机时间
            case CMD_POWERMGR_SYNCH_CURSYS_TIME:
                cmd = CMD_POWERMGR_SYSTIME;
                sub_cmd = CMD_SYNCH_CURRENTSYS_TIME;
                cmdnum = 2;
                break;
            case CMD_POWERMGR_SYNCH_OPENSYS_TIME:
                cmd = CMD_POWERMGR_SYSTIME;
                sub_cmd = CMD_SET_OPENSYS_TIME;
                cmdnum = 2;
                break;
            case CMD_POWERMGR_SYNCH_CLOSESYS_TIME:
                cmd = CMD_POWERMGR_SYSTIME;
                sub_cmd = CMD_SET_CLOSESYS_TIME;
                cmdnum = 2;
                break;
            case CMD_POWERMGR_SYNCH_CANCEL_OPENSYS:
                cmd = CMD_POWERMGR_SYSTIME;
                sub_cmd = CMD_CANCEL_OPENSYS_TIME;
                cmdnum = 2;
                break;
            case CMD_POWERMGR_SYNCH_CANCEL_CLOSESYS:
                cmd = CMD_POWERMGR_SYSTIME;
                sub_cmd = CMD_CANCEL_CLOSESYS_TIME;
                cmdnum = 2;
                break;
            /// 重启
            case CMD_POWERMGR_REBOOT_UPBODY:
                cmd = CMD_REBOOT_UPBODY;
                cmdnum = 1;
                break;
            case CMD_POWERMGR_REBOOT_DOWNBODY:
                cmd = CMD_REBOOT_DOWNBODY;
                cmdnum = 1;
                break;
            case CMD_POWERMGR_REBOOT_ALLBODY:
                cmd = CMD_REBOOT_ALLBODY;
                cmdnum = 1;
                break;
            /// 看门狗
            case CMD_POWERMGR_WATCHDOG_CLOSE:
                cmd = CMD_POWERMGR_WATCHDOG;
                sub_cmd = CMD_WATCHDOG_CLOSE;
                cmdnum = 2;
                break;
            case CMD_POWERMGR_WATCHDOG_OPEN:
                cmd = CMD_POWERMGR_WATCHDOG;
                sub_cmd = CMD_WATCHDOG_OPEN;
                cmdnum = 2;
                break;
            case CMD_POWERMGR_WATCHDOG_WATCH:
                cmd = CMD_POWERMGR_WATCHDOG;
                sub_cmd = CMD_WATCHDOG_WATCH;
                cmdnum = 2;
                break;
            case CMD_POWERMGR_WATCHDOG_QUERY:
                cmd = CMD_POWERMGR_WATCHDOG;
                sub_cmd = CMD_WATCHDOG_QUERY;
                cmdnum = 2;
                break;
            /// 充电状态
            case CMD_POWERMGR_NOTIFY_CHARGING:
                cmd = CMD_POWERMGR_TURN_OFF;
                sub_cmd = CMD_CHARGING;
                cmdnum = 2;
                break;
            case CMD_POWERMGR_NOTIFY_NOT_CHARGING:
                cmd = CMD_POWERMGR_TURN_OFF;
                sub_cmd = CMD_NOT_CHARGING;
                cmdnum = 2;
                break;
            default:
                YSOS_LOG_DEBUG ( "ctrl_id is not supported" );
                break;
        }
        return cmdnum;
    }

    int PowerManagerModule::GetTimeData ( const TimeParam &strusystime, UINT8* senddata, UINT32 sendsize )
    {
        if ( senddata == NULL || sendsize < PROP_SYNCH_SYSTEM_LEN )
        {
            YSOS_LOG_ERROR ( "ParserTimeJson buffer error" );
            return 0;
        }
        senddata[0] = ( ( strusystime.nYear / 1000 ) << 4 ) + strusystime.nYear / 100 % 10;
        senddata[1] = ( ( strusystime.nYear / 10 % 10 ) << 4 ) + strusystime.nYear % 10;
        senddata[2] = ( ( strusystime.cbMonth / 10 ) << 4 ) + strusystime.cbMonth % 10;
        senddata[3] = ( ( strusystime.cbDay / 10 ) << 4 ) + strusystime.cbDay % 10;
        senddata[4] = ( ( strusystime.cbHour / 10 ) << 4 ) + strusystime.cbHour % 10;
        senddata[5] = ( ( strusystime.cbMin / 10 ) << 4 ) + strusystime.cbMin % 10;
        senddata[6] = ( ( strusystime.cbSecond / 10 ) << 4 ) + strusystime.cbSecond % 10;
        return PROP_SYNCH_SYSTEM_LEN;
    }

    int PowerManagerModule::GetWatchdogData ( UINT8* senddata, UINT32 sendsize )
    {
        if ( senddata == NULL || sendsize < PROP_WATCHDOG_OPEN_LEN )
        {
            YSOS_LOG_ERROR ( "ParserTimeJson buffer error" );
            return 0;
        }
        senddata[0] = watchdog_time_ >> 8;
        senddata[1] = watchdog_time_ & 0xff;
        senddata[2] = watchdog_reboot_mode_;
        return PROP_WATCHDOG_OPEN_LEN;
    }

    int PowerManagerModule::ConvertSendMsg ( BufferInterfacePtr temp_buffer_ptr, BufferInterfacePtr rst_buffer_ptr )
    {
        UINT8 *tmpbufferptr = NULL;
        UINT32 tmpbuffersize = 0;
        UINT8 *rstbufferptr = NULL;
        UINT32 rstbuffersize = 0;
        if ( temp_buffer_ptr )
        {
            temp_buffer_ptr->GetBufferAndLength ( &tmpbufferptr, &tmpbuffersize );
        }
        if ( rst_buffer_ptr )
        {
            rst_buffer_ptr->GetBufferAndLength ( &rstbufferptr, &rstbuffersize );
            rst_buffer_ptr->GetMaxLength ( &rstbuffersize );
        }
        if ( tmpbufferptr == NULL || NULL == rstbufferptr || rstbuffersize < tmpbuffersize * 2 + 4 )
        {
            YSOS_LOG_ERROR ( "ConvertSendMsg buffer error" );
            return YSOS_ERROR_FAILED;
        }
        UINT32 size = 3;
        rstbufferptr[0] = PROP_FRAME_HEADER;
        rstbufferptr[1] = PROP_FRAME_VERSION;
        rstbufferptr[2] = PROP_DEVICECTRL_ELECTRIC;
        for ( UINT32 i = 0; i < tmpbuffersize; i++ )
        {
            if ( PROP_FRAME_HEADER == tmpbufferptr[i] || PROP_FRAME_TAIL == tmpbufferptr[i] || PROP_FRAME_TRUN == tmpbufferptr[i] )
            {
                rstbufferptr[size++] = PROP_FRAME_TRUN;
                rstbufferptr[size++] = tmpbufferptr[i] - PROP_FRAME_TRUN;
            }
            else
            {
                rstbufferptr[size++] = tmpbufferptr[i];
            }
        }
        rstbufferptr[size++] = PROP_FRAME_TAIL;
        rst_buffer_ptr->SetLength ( size );
        return YSOS_ERROR_SUCCESS;
    }

    int PowerManagerModule::IoctlEx ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        int iretresult = YSOS_ERROR_FAILED;
        if ( powermanager_driver_ptr_ )
        {
            iretresult = powermanager_driver_ptr_->Ioctl ( 3, input_buffer_ptr, output_buffer_ptr ); //读取电量
            if ( YSOS_ERROR_SUCCESS != iretresult )
            {
                YSOS_LOG_ERROR ( "driver->Ioctl failed " << iretresult );
                return YSOS_ERROR_FAILED;
            }
            iretresult = CheckResult ( input_buffer_ptr, output_buffer_ptr );
            if ( YSOS_ERROR_SUCCESS != iretresult )
            {
                YSOS_LOG_ERROR ( "CheckResult failed " << iretresult );
                return YSOS_ERROR_FAILED;
            }
        }
        return iretresult;
    }

    /// 检查返回结果 //NOLINK
    int PowerManagerModule::CheckResult ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr )
    {
        UINT32 isendbufferlength = 0;
        UINT32 irecvbufferlength = 0;
        UINT8 *sendbufferptr = NULL;
        UINT8 *recvbufferptr = NULL;
        if ( input_buffer_ptr )
        {
            input_buffer_ptr->GetBufferAndLength ( &sendbufferptr, &isendbufferlength );
        }
        if ( output_buffer_ptr )
        {
            output_buffer_ptr->GetBufferAndLength ( &recvbufferptr, &irecvbufferlength );
        }
        if ( irecvbufferlength < MIN_MSG_LENGTH )
        {
            YSOS_LOG_ERROR ( "PowerManagerModule CheckResult Function ReceiveBuffer Not Enough!" );
            return YSOS_ERROR_FAILED;
        }
        if ( PROP_FRAME_HEADER == recvbufferptr[0] && PROP_FRAME_VERSION == recvbufferptr[1] &&
                PROP_DEVICECTRL_ELECTRIC == recvbufferptr[2] && sendbufferptr[3] == recvbufferptr[3] )
        {
            return YSOS_ERROR_SUCCESS;
        }
        YSOS_LOG_ERROR ( "PowerManagerModule CheckResult Function Check Receive Error!" );
        return YSOS_ERROR_FAILED;
    }

    int PowerManagerModule::ReadEx ( BufferInterfacePtr output_buffer_ptr )
    {
        YSOS_LOG_DEBUG ( "ReadEx" );
        int iretresult = YSOS_ERROR_FAILED;
        if ( powermanager_driver_ptr_ )
        {
            iretresult = powermanager_driver_ptr_->Ioctl ( 2, NULL, output_buffer_ptr );
        }
        return iretresult;
    }

    int PowerManagerModule::CheckPowerOff ( BufferInterfacePtr output_buffer_ptr )
    {
        UINT32 irecvbufferlength = 0;
        UINT8 *recvbufferptr = NULL;
        if ( output_buffer_ptr )
        {
            output_buffer_ptr->GetBufferAndLength ( &recvbufferptr, &irecvbufferlength );
        }
        if ( irecvbufferlength == 5 &&
                recvbufferptr[0] == PROP_FRAME_HEADER &&
                recvbufferptr[1] == PROP_FRAME_VERSION &&
                recvbufferptr[2] == PROP_DEVICECTRL_ELECTRIC &&
                recvbufferptr[3] == CMD_POWERMGR_TURN_OFF )
        {
            YSOS_LOG_WARN ( "time to turn off" );
            return YSOS_ERROR_SUCCESS;
        }
        return YSOS_ERROR_FAILED;
    }

    int PowerManagerModule::Powermgr_Judge_Chassis_Thread ( LPVOID lpParam )
    {
        PowerManagerModule *power_mgr = ( PowerManagerModule* ) lpParam;
        std::string is_chassis_connect;
        while ( 1 )
        {
            power_mgr->data_ptr_->GetData ( "chassis_connect", is_chassis_connect );
            YSOS_LOG_DEBUG_CUSTOM ( power_mgr->logger_, "chassis_connect : " << is_chassis_connect.c_str() );
            if ( is_chassis_connect.compare ( "0" ) == 0 )
            {
                power_mgr->powermanager_driver_ptr_->Ioctl ( PROP_POWERMGR_REBOOT_DOWN, nullptr, nullptr );
            }
            else if ( is_chassis_connect.compare ( "1" ) == 0 )
            {
                break;
            }
            else if ( is_chassis_connect.compare ( " " ) == 0 )
            {
                continue;
            }
            boost::this_thread::sleep ( boost::posix_time::seconds ( 2 ) );
        }
    }

}
