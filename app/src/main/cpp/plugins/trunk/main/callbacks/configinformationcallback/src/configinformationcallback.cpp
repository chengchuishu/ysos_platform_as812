/**
  *@file accountsstrategycallback.h
  *@brief Account Strategy Callback
  *@version 1.0
  *@author Xue Xiaojun
  *@date Created on: 2017-06-30 19:33:45
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * http://www.tangdi.com.cn
  */


/// Self Header //  NOLINT
#include "../include/configinformationcallback.h"
//#include "../include/accountsstrategycallback.h"
/// private headers
#include "../../../../../../core/trunk/main/public/include/plugins/commonenumdefine.h"
#include "../../../../../../core/trunk/main/public/include/plugins/commonstructdefine.h"
/// Ysos Sdk Headers
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/messageinterface.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/packageconfigimpl.h"
#include "../../../../../../core/trunk/main/protect/include/os_hal_package/datainterfacemanager.h"
#include "../../../../../../core/trunk/main/protect/include/protocol_help_package/platformprotocolimpl.h"

namespace ysos{

static unsigned int now_number_ = 0; ///<当前连续有人或者没有的次数 //NOLINT
static unsigned int now_people_state_ = 0; ///<当前的状态，0为无人，1为有人。初始默认无人

DECLARE_PLUGIN_REGISTER_INTERFACE(ConfigInformationCallback, CallbackInterface);
ConfigInformationCallback::ConfigInformationCallback(const std::string &strClassName):BaseThreadModuleCallbackImpl(strClassName) {
	logger_ = GetUtility()->GetLogger("ysos.configinformation");
	YSOS_LOG_DEBUG("ConfigInformationCallback is ok");
}


ConfigInformationCallback::~ConfigInformationCallback() {
}


int ConfigInformationCallback::RealCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr next_input_buffer,
	BufferInterfacePtr pre_output_buffer, void *context){
		YSOS_LOG_DEBUG("*************** Entry ConfigInformationCallback *************");

		if (NULL == context) {
			return YSOS_ERROR_INVALID_ARGUMENTS;
		}
		BaseModuelCallbackContext *module_callback_context_ptr = reinterpret_cast<BaseModuelCallbackContext*>(context);
		if (NULL == module_callback_context_ptr) {
			return YSOS_ERROR_INVALID_ARGUMENTS;
		}
		BaseModuleImpl *current_module_Interface_ptr = dynamic_cast<BaseModuleImpl*>(module_callback_context_ptr->cur_module_ptr);
		if (NULL == current_module_Interface_ptr) {
			return YSOS_ERROR_INVALID_ARGUMENTS;
		}
		if (next_input_buffer == NULL) {
			YSOS_LOG_WARN("GetBufferFromBufferPool failed");
			return YSOS_ERROR_FAILED;
		}
		// GET_PAGE_UPDATE and GET_SOURCE_UPDATE
		current_module_Interface_ptr->GetProperty(0, &next_input_buffer);
		return YSOS_ERROR_SUCCESS;
}

}