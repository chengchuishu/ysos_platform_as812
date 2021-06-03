/**
 *@file templatestrategycallback.cpp
 *@brief the manager self strategy call back
 *@version 1.0
 *@author z.gg
 *@date Created on: 2020-3-31 15:16:00
 *@copyright Copyright (c) 2020 YS Technology. All rights reserved.
 * 
 */
/// Private Headers //  NOLINT
#include "../include/templatestrategycallback.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "../../../public/include/common/commonenumdefine.h"

#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"


namespace ysos {
    DECLARE_PLUGIN_REGISTER_INTERFACE ( TemplateStrategyCallback,  CallbackInterface );  ///<  插件的入口，一定要加上 //  NOLINT
    TemplateStrategyCallback::TemplateStrategyCallback ( const std::string &strClassName ) : BaseStrategyCallbackImpl ( strClassName )
        , data_ptr_ ( GetDataInterfaceManager()->GetData ( GLOBAL_DATA_KEY ) )
    {
        logger_ = GetUtility()->GetLogger ( "ysos.templatestrategy" );
    }

    TemplateStrategyCallback::~TemplateStrategyCallback (  void )
    {
    }

    int TemplateStrategyCallback::HandleMessage ( const std::string &event_name, BufferInterfacePtr input_buffer, CallbackInterface *context )
    {
        YSOS_LOG_DEBUG ( "HandleMessage: " << event_name );
        if ( data_ptr_ == NULL )
        {
            YSOS_LOG_ERROR ( "data_ptr_ is NULL!" );
            return YSOS_ERROR_FAILED;
        }
        // template_change_state result
        std::string template_result;
		std::string change_state;
        data_ptr_->GetData ( "template_result", template_result );
        if ( template_result != "0" )
        {
            YSOS_LOG_DEBUG ( "template_result is not finish: " <<  template_result );
            return YSOS_ERROR_FAILED;
        } else {
			data_ptr_->GetData ( "new_state", change_state );
		}
		YSOS_LOG_DEBUG ( "****** HandleMessage [change_state] = " << change_state );
		//< change state
	    {
			int indx = change_state.find("@");
			if (indx <= 0 || indx >= change_state.length()) {
				indx = 0;
			} else {
			    indx = indx + 1;
			}
			std::string tmp_ch_state = change_state.substr(indx); 
			int rst = DoSwitchNotifyService ( tmp_ch_state, context );
			if ( YSOS_ERROR_SUCCESS != rst )
			{
				YSOS_LOG_DEBUG ( "execute DoSwitchNotifyService failed" );
			} else {
				YSOS_LOG_DEBUG ( "execute DoSwitchNotifyService success" );
				data_ptr_->SetData("machine_state", tmp_ch_state);
			}
        }
        
        return YSOS_ERROR_SUCCESS;
    }

    int TemplateStrategyCallback::Initialized (  const std::string &key,  const std::string &value )
    {
        YSOS_LOG_DEBUG ( "Initialized " << key << ": " << value );
        int n_return = YSOS_ERROR_SUCCESS;
        return n_return;
    }

    int TemplateStrategyCallback::RequestService ( std::string service_name, std::string id,  std::string type, std::string json_value, CallbackInterface *context )
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

}