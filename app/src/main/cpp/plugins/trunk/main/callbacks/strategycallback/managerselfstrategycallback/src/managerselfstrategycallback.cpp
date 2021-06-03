/**
 *@file managerselfstrategycallback.cpp
 *@brief manager self strategy call back
 *@version 1.0
 *@author l.j..
 *@date Created on: 2016-10-20 15:16:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
/// Private Headers //  NOLINT
#include "../include/managerselfstrategycallback.h"

#include <stdio.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "../../../public/include/common/commonenumdefine.h"

#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"

#define INFINITE          0x7fffffff
#define MAX_MULTIPLIER    6
#ifdef _WIN32
const char *TASK_FILE           = "C:\\TDRobot\\YSOS\\data\\robottask\\task.xml";
#else
const char *TASK_FILE           = "../data/robottask/task.xml"; //WORK TASK DIRECTIOR
#endif


enum ACTION_TYPE
{
    WORK_OFF      = 0,
    STATIC_SEARCH,
    LOCAL_SEARCH,
    SPOT_SEARCH
};

#ifdef _WIN32
#else
/**
 *原型：char *itoa( int value, char *string,int radix);
 *value：欲转换的数据；string：目标字符串的地址；radix：转换后的进制数，可以是10进制、16进制等。
 *功能：把一个整数转换成字符串
 **/
static void itoa( int value, char *str, int radix) {
   sprintf(str, "%d", value);
   return;
}
#endif

namespace ysos {
    DECLARE_PLUGIN_REGISTER_INTERFACE ( ManagerSelfStrategyCallback,  CallbackInterface );  ///<  插件的入口，一定要加上 //  NOLINT
    ManagerSelfStrategyCallback::ManagerSelfStrategyCallback ( const std::string &strClassName ) : BaseStrategyCallbackImpl ( strClassName )
        /*, battery_ ( 0xff )*/
        , data_ptr_ ( GetDataInterfaceManager()->GetData ( GLOBAL_DATA_KEY ) )
    {
        logger_ = GetUtility()->GetLogger ( "ysos.managerselfstrategy" );
    }

    ManagerSelfStrategyCallback::~ManagerSelfStrategyCallback (  void )
    {
    }

    int ManagerSelfStrategyCallback::HandleMessage ( const std::string &event_name, BufferInterfacePtr input_buffer, CallbackInterface *context )
    {
        YSOS_LOG_DEBUG ( "HandleMessage: " << event_name );
        if ( data_ptr_ == NULL )
        {
            YSOS_LOG_ERROR ( "data_ptr_ is NULL!" );
            return YSOS_ERROR_FAILED;
        }
        // common process
        if ( event_name == "go_home_chassis_event" )
        {
            // RequestService ( TTS_MODULE, TTS_IOCTL_ID, "text", ignore_charge_, context );
        }
        else if ( event_name == "task_updated_service" )
        {
            LoadTask ( boost::posix_time::microsec_clock::local_time().date() );
        }
        return YSOS_ERROR_SUCCESS;
    }

    int ManagerSelfStrategyCallback::Initialized (  const std::string &key,  const std::string &value )
    {
        YSOS_LOG_DEBUG ( "Initialized " << key << ": " << value );
        int n_return = YSOS_ERROR_SUCCESS;
        if ( key == "robot_init_mode" )
        {
            LoadTask ( boost::posix_time::microsec_clock::local_time().date() );
        } else {

        }

        return n_return;
    }

    int ManagerSelfStrategyCallback::RequestService ( std::string service_name, std::string id,  std::string type, std::string json_value, CallbackInterface *context )
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
        n_return = DoIoctlService ( req_service,  context );
        if ( YSOS_ERROR_SUCCESS != n_return )
        {
            YSOS_LOG_DEBUG ( "execute DoIoctlService failed, n_return = " << n_return );
        }
        YSOS_LOG_DEBUG ( "RequestService done and success" );
        return n_return;
    }

    int ManagerSelfStrategyCallback::LoadTask (  const boost::posix_time::ptime::date_type &day )
    {
        /* It is very bad not to read files in a new thread. However we do it first. */
        return YSOS_ERROR_SUCCESS;
    }

    bool timeCompare ( int cur_hour, int cur_minu, int tsk_hour, int tsk_minu )
    {
        if ( cur_hour > tsk_hour )
        {
            return true;
        }
        if ( cur_hour == tsk_hour )
        {
            return cur_minu >= tsk_minu;
        }
        return false;
    }


}