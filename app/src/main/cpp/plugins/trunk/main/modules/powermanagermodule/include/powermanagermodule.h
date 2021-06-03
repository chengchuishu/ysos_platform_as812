/**
*@file powermanagermodule.h
*@brief power manager
*@version 1.0
*@author Wang Xiaogui
*@date Created on: 2016-11-25 09:22:00
*@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
* 
*/

#ifndef YSOS_PLUGIN_MODULE_POWERMANAGER_H_  //NOLINT
#define YSOS_PLUGIN_MODULE_POWERMANAGER_H_  //NOLINT

/// Private Headers  //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemoduleimpl.h"          //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemodulecallbackimpl.h"  //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basethreadmoduleimpl.h"    //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basedriverimpl.h"          //NOLINT
#include "../../../../../../core/trunk/main/protect/include/os_hal_package/datainterfacemanager.h"

/// Public Headers
#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"

/// Boost Header //NOLINT
#include <boost/shared_ptr.hpp>                       //NOLINT
#include <boost/thread.hpp>                           //NOLINT
#include <boost/algorithm/string.hpp>                 //NOLINT
#include <boost/date_time/posix_time/posix_time.hpp>  //NOLINT
#include <boost/date_time/gregorian/gregorian.hpp>    //NOLINT

#ifdef _WIN32
  #include <Windows.h>
#else
#endif

namespace ysos {

    /**
    *@brief  PowerManagerModule  // NOLINT
    */
    class YSOS_EXPORT PowerManagerModule : public BaseThreadModuleImpl {
            DECLARE_PROTECT_CONSTRUCTOR ( PowerManagerModule );
            DISALLOW_COPY_AND_ASSIGN ( PowerManagerModule );
            DECLARE_CREATEINSTANCE ( PowerManagerModule );

        public:

            /**
            *@brief 获取接口的属性值                                   // NOLINT
            *@param type_id[Input]： 属性的ID                          // NOLINT
            *@param typeptr[Output]：属性值的值                        // NOLINT
            *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
            */
            virtual int GetProperty ( int type_id, void *typeptr );

            /**
            *@brief 设置接口的属性值                                   // NOLINT
            *@param type_id[Input]： 属性的ID                          // NOLINT
            *@param typeptr[input]： 属性值的值                        // NOLINT
            *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
            */
            virtual int SetProperty ( int type_id, void *typeptr );

            /**
            *@brief 配置函数，可对该接口进行详细配置                    // NOLINT
            *@param ctrl_id[Input]：详细配置ID号                        // NOLINT
            *@param param[Input/output]：详细配置的参数                 // NOLINT
            *@return：成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值    // NOLINT
            */
            int Ioctl ( INT32 ctrl_id, LPVOID param = NULL );


        protected:
            /**
            *@brief 基本初始化                                         // NOLINT
            *@param： 无                                               // NOLINT
            *@return：成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值   // NOLINT
            */
            int Initialize ( void *param = NULL );


            /**
            *@brief  配置文件中设置的Key、Value对，会在Initialized函数中调用 //NOLINT
            *@param key 配置参数中的Key                                      //NOLINT
            *@param value 与Key对应的Value                                   //NOLINT
            *@return 成功返回YSOS_ERROR_SUCCESS，否则失败                    //NOLINT
            */
            virtual int Initialized ( const std::string &key, const std::string &value );

            /**
            *@brief  初始化ModuleDataInfo                              // NOLINT
            *@return: 成功返回YSOS_ERROR_SUCCESS，否则失败             // NOLINT
            */
            int InitalDataInfo();

            /**
            *@brief 设置缓存
            *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值
            */
            int SetupModuleBuffer();

            /**
            *@brief 类似于模块的初始化功能，子类实现，只关注业务      // NOLINT
            *@param open需要的参数                                    // NOLINT
            *@return 成功返回0，否则失败                              // NOLINT
            */
            int RealOpen ( LPVOID param = NULL );

            /**
            *@brief 运行，子类实现，只关注业务                          // NOLINT
            *@param ： 无                                               // NOLINT
            *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值   // NOLINT
            */
            int RealRun();

            /**
            *@brief 暂停，子类实现，只关注业务                         // NOLINT
            *@param ： 无                                              // NOLINT
            *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
            */
            int RealPause();

            /**
            *@brief 停止运行，关闭模块，子类实现，只关注业务          // NOLINT
            *       只有当ref_cout为0时，才能正常关闭                 // NOLINT
            *@return 成功返回0，否则失败
            */
            int RealStop();

            /**
            *@brief 关闭，子类实现，只关注业务                        // NOLINT
            *@return 成功返回0，否则失败                              // NOLINT
            */
            int RealClose();

        private:
            /**
            *@brief 看门狗及电量定时查询线程
            *@return
            */
            static int Powermgr_Process_Thread ( LPVOID lpParam );

        private:
            /**
            *@brief
            *@Structure Send Data
            *@param ctrl_id[Input] Structure Send Ctrl ID
            *@param param[Input] Structure Send Param
            *@param temp_buffer_ptr[Input] temp buffer
            *@param rst_buffer_ptr[Output] Send Data Buffer
            *@return success return YSOS_ERROR_SUCCESS,
            *        fail return YSOS_ERROR_FAILED
            */
            int StructureSendData ( int ctrl_id, std::string* param, BufferInterfacePtr temp_buffer_ptr, BufferInterfacePtr rst_buffer_ptr );

            /**
            *@brief
            *@Get command id
            *@param ctrl_id[Input] ctrl id
            *@param cmd[Output] command id
            *@param sub_cmd[Input] sub command id
            *@return only command id return 1
            *        command id and sub command id return 2
            *        fail return 0
            */
            int GetCommandId ( int ctrl_id, UINT8& cmd, UINT8& sub_cmd );

            /**
            *@brief
            *@Parser Time Json
            *@param strusystime[Input] time
            *@param senddata[Output] output buffer
            *@param sendsize[Input] output buffer max size
            *@return success return output used size,
            *        fail return 0
            */
            int GetTimeData ( const TimeParam &strusystime, UINT8* senddata, UINT32 sendsize );

            /**
            *@brief
            *@Parser Time Json
            *@param param[Input] time json
            *@param senddata[Output] output buffer
            *@param sendsize[Input] output buffer max size
            *@return success return output used size,
            *        fail return 0
            */
            int GetWatchdogData ( UINT8* senddata, UINT32 sendsize );

            /**
            *@brief
            *@Convert Send Msg
            *@param temp_buffer_ptr[Input] input buffer
            *@param rst_buffer_ptr[Output] output buffer
            *@return success return YSOS_ERROR_SUCCESS,
            *        fail return YSOS_ERROR_FAILED
            */
            int ConvertSendMsg ( BufferInterfacePtr temp_buffer_ptr, BufferInterfacePtr rst_buffer_ptr );

            /**
            *@brief
            *@to send command
            *@param input_buffer_ptr[Input] input buffer
            *@param output_buffer_ptr[Output] output buffer
            *@return success return YSOS_ERROR_SUCCESS,
            *        fail return YSOS_ERROR_FAILED
            */
            int IoctlEx ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr );

            /**
            *@brief
            *@check result
            *@param input_buffer_ptr[Input] input buffer
            *@param output_buffer_ptr[Output] output buffer
            *@return success return YSOS_ERROR_SUCCESS,
            *        fail return YSOS_ERROR_FAILED
            */
            int CheckResult ( BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr );

            /**
            *@brief
            *@to read data
            *@param output_buffer_ptr[Output] output buffer
            *@return success return YSOS_ERROR_SUCCESS,
            *        fail return YSOS_ERROR_FAILED
            */
            int ReadEx ( BufferInterfacePtr output_buffer_ptr );

            /**
            *@brief
            *@check whether power off
            *@param output_buffer_ptr[Output] output buffer
            *@return success return YSOS_ERROR_SUCCESS,
            *        fail return YSOS_ERROR_FAILED
            */
            int CheckPowerOff ( BufferInterfacePtr output_buffer_ptr );

        private:
            DriverInterfacePtr    powermanager_driver_ptr_; ///< Module模块的Driver指针                //NOLINT
            CallbackInterfacePtr  module_callback_ptr_;     ///< Module模块的Callback指针              //NOLINT
            // 线程
            boost::thread powmgr_judge_thread_;    ///< 线程句柄                                             //NOLINT

            static int Powermgr_Judge_Chassis_Thread ( LPVOID lpParam );

            DataInterfacePtr data_ptr_;

            std::string           releate_driver_name_;     ///< 底盘Driver对象名称                    //NOLINT
            std::string           releate_callback_name_;   ///< 底盘Callback对象名称                  //NOLINT

        private:
            int icom_port_;                  ///< 串口端口                                             //NOLINT

            // 看门狗
            int watchdog_enable_;            ///< 是否使能看门狗                                       //NOLINT
            UINT16 watchdog_time_;           ///< 看门狗等待时间，单位：秒，默认时间5分钟              //NOLINT
            UINT8 watchdog_reboot_mode_;     ///< 看门狗重启模式                                       //NOLINT

            // 电量
            UINT8 barrery_elect_;            ///< 电量
            UINT8 barrery_charge_status;     ///< 线充状态
            // 关机
            bool power_off_;                  ///< 关机

            // 线程
            boost::thread powmgr_thread_;    ///< 线程句柄                                             //NOLINT
            int thread_interval_;            ///< 查询电池电量的时间间隔，单位：毫秒                   //NOLINT
    };


} // namespace ysos

#endif    ///< YSOS_PLUGIN_MODULE_POWERMANAGER_H_  //NOLINT
