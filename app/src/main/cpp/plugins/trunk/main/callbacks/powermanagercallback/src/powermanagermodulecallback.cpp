/**
*@file powermanagermodulecallback.cpp
*@brief power manager module callback
*@version 1.0
*@author wangxg
*@date Created on: 2016-11-28 14:05:00
*@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
* 
*/

/// Myself Headers  //NOLINT
#include "../include/powermanagermodulecallback.h"

/// Public Headers
#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"

/// YSOS Header     //NOLINT
#include "../../../../../../core/trunk/main/public/include/os_hal_package/lock.h"             //NOLINT
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"       //NOLINT
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"
#include "../../../../../../core/trunk/main/protect/include/os_hal_package/datainterfacemanager.h"

/// BOOST Header     //NOLINT
#include <boost/typeof/typeof.hpp>                    //NOLINT
#include <boost/date_time/posix_time/posix_time.hpp>  //NOLINT
#include <boost/thread.hpp>                           //NOLINT
#include <json/json.h>

#define POWERMGR_OUTPUT_TYPE                     "type"
#define POWERMGR_OUTPUT_TYPE_VALUE               "powermgr_info"
#define POWERMGR_OUTPUT_DTAT                     "data"
#define POWERMGR_OUTPUT_PREFIX                   "prefix"
#define POWERMGR_OUTPUT_BATTERY_MODE             "mode"
#define POWERMGR_OUTPUT_BATTERY_ELE              "battery"

const char* ELECTRICITY = "electricity";
const char* CLECTCHARGE = "electcharge";
#ifndef SLEEP
    #define SLEEP(a)      (boost::this_thread::sleep_for(boost::chrono::milliseconds(a)))
#endif

namespace ysos {

    DECLARE_PLUGIN_REGISTER_INTERFACE ( PowerManagerModuleCallback, CallbackInterface );
    PowerManagerModuleCallback::PowerManagerModuleCallback ( const std::string &strClassName ) : BaseThreadModuleCallbackImpl ( strClassName )
    {
        logger_ = GetUtility()->GetLogger ( "ysos.powermgr" );
    }

    PowerManagerModuleCallback::~PowerManagerModuleCallback()
    {
    }

    int PowerManagerModuleCallback::IsReady()
    {
        return YSOS_ERROR_SUCCESS;
    }


    /************************************************************************************************************/
    /* cur_output_buffer_ptr:
    {
      "prefix":"2017.1.18 14:06:25",///< 时间前缀
      "mode":1,                     ///< 0:表示电压模式，1：表示电池模式
      "battery":80                  ///< 电池电量：如果mode为0，则分1、2、3、4四个等级
    } */
    /************************************************************************************************************/

    int PowerManagerModuleCallback::RealCallback ( BufferInterfacePtr input_buffer, BufferInterfacePtr cur_output_buffer, BufferInterfacePtr output_buffer, void *context )
    {
        YSOS_LOG_DEBUG ( "###############PowerManagerModuleCallback###############" );
        if ( NULL == context )
        {
            YSOS_LOG_ERROR ( "PowerManagerModuleCallback Parameter Error!" );
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        BaseModuelCallbackContext *module_callback_context_ptr = reinterpret_cast<BaseModuelCallbackContext*> ( context );
        if ( module_callback_context_ptr == NULL )
        {
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        BaseModuleImpl *current_module_Interface_ptr = dynamic_cast<BaseModuleImpl*> ( module_callback_context_ptr->cur_module_ptr );
        if ( current_module_Interface_ptr == NULL )
        {
            return YSOS_ERROR_INVALID_ARGUMENTS;
        }
        bool power_off = false;
        current_module_Interface_ptr->GetProperty ( PROP_POWERMGR_POWEROFF_INFO, &power_off );
        if ( power_off )
        {
            UINT8 *curbufferptr = NULL;
            UINT32 icurbuffersize = 0;
            cur_output_buffer->GetBufferAndLength ( &curbufferptr, &icurbuffersize );
            sprintf ( ( char* ) curbufferptr, "{\"type\":\"powermgr_power_off\",\"data\":\"\"}" );
            InvokeNextCallback ( cur_output_buffer, NULL, context );
        }
        int barrery_elect = 0;
        if ( YSOS_ERROR_SUCCESS != current_module_Interface_ptr->GetProperty ( PROP_POWERMGR_BATTERY_INFO, &barrery_elect ) )
        {
            return YSOS_ERROR_SKIP;
        }
        int barrery_charge_status = 0;
        if ( YSOS_ERROR_SUCCESS != current_module_Interface_ptr->GetProperty ( PROP_POWERMGR_CHARGE_INFO, &barrery_charge_status ) )
        {
            return YSOS_ERROR_SKIP;
        }
        // 电量放大
        if ( barrery_elect > 30 && barrery_elect < 90 )
        {
            barrery_elect = barrery_elect * 7 / 6.5;
        }
        else if ( barrery_elect >= 90 )
        {
            barrery_elect = 100;
        }
        // write buffer
        DataInterfacePtr data_ptr_ = GetDataInterfaceManager()->GetData ( GLOBAL_DATA_KEY );
        if ( data_ptr_ != NULL )
        {
            char elect[4];
            sprintf ( elect, "%3d", barrery_elect );
            char electcharge[4];
            sprintf ( electcharge, "%3d", barrery_charge_status );
            data_ptr_->SetData ( ELECTRICITY, elect );
            data_ptr_->SetData ( CLECTCHARGE, electcharge );
            YSOS_LOG_DEBUG ( "CLECTCHARGE electcharge:" << electcharge );
        }
        return YSOS_ERROR_SKIP;
        #if 0
        /// 构造Json输出数据
        std::string strresult;
        if ( YSOS_ERROR_SUCCESS != GetJsonResult ( 1, barrery_elect, strresult ) )
        {
            YSOS_LOG_ERROR ( "Get Json Result Fail" );
            return YSOS_ERROR_FAILED;
        }
        GetBufferUtility()->CopyStringToBuffer ( strresult, cur_output_buffer );
        return YSOS_ERROR_SUCCESS;
        #endif
    }

    int PowerManagerModuleCallback::GetJsonResult ( int barrery_mode, int barrery_elect, std::string &strjson_result )
    {
        Json::Value js_root;
        Json::Value js_value;
        js_value[POWERMGR_OUTPUT_PREFIX] = to_simple_string ( boost::posix_time::microsec_clock::local_time() );
        js_value[POWERMGR_OUTPUT_BATTERY_MODE] = barrery_mode;
        js_value[POWERMGR_OUTPUT_BATTERY_ELE] = barrery_elect;
        js_root[POWERMGR_OUTPUT_TYPE] = POWERMGR_OUTPUT_TYPE_VALUE;
        js_root[POWERMGR_OUTPUT_DTAT] = js_value;
        Json::FastWriter js_writer;
        strjson_result = js_writer.write ( js_root );
        strjson_result = GetUtility()->ReplaceAllDistinct ( strjson_result, "\\r\\n", "" );
        YSOS_LOG_DEBUG ( "Json Result:" << strjson_result );
        return YSOS_ERROR_SUCCESS;
    }


}
