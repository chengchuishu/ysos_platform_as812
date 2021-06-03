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
#include "../include/facestrategycallback.h"
//#include "../include/accountsstrategycallback.h"
/// private headers
#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"
/// Ysos Sdk Headers
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/messageinterface.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/packageconfigimpl.h"
#include "../../../../../../core/trunk/main/protect/include/os_hal_package/datainterfacemanager.h"
#include "../../../../../../core/trunk/main/protect/include/protocol_help_package/platformprotocolimpl.h"

namespace ysos{

static unsigned int now_number_ = 0; ///<当前连续有人或者没有的次数 //NOLINT
static unsigned int now_people_state_ = 0; ///<当前的状态，0为无人，1为有人。初始默认无人

DECLARE_PLUGIN_REGISTER_INTERFACE(FaceStrategyCallback, CallbackInterface);
FaceStrategyCallback::FaceStrategyCallback(const std::string &strClassName):BaseStrategyCallbackImpl(strClassName) {
}


FaceStrategyCallback::~FaceStrategyCallback() {
}


int FaceStrategyCallback::HandleMessage(const std::string &event_name, BufferInterfacePtr input_buffer, CallbackInterface *context)
{
	YSOS_LOG_DEBUG("HandleMessage " << event_name);
	do {
		if (
			true == event_name.empty() ||
			NULL == input_buffer ||
			NULL == context
			) {
				YSOS_LOG_DEBUG("Invalid Parameters.");
				break;
		}

		if (0 != strcasecmp(event_name.c_str(),event_name_.c_str())) {
			YSOS_LOG_DEBUG("event_name is error. [" << event_name << "]");
			break;
		}
		UINT8* buffer_ptr = NULL;
		UINT32 buffer_length = 0;
		input_buffer->GetBufferAndLength(&buffer_ptr, &buffer_length);     
		if (NULL==buffer_ptr)
		{
			YSOS_LOG_DEBUG("Input Buffer Change Is Error");
			break;
		}

		std::string temp_buffer_str = reinterpret_cast<char*>(buffer_ptr);
		std::string people_info;
		///< 将人脸信息进行连续次数的判断，做出有人/没人的状态转化  // NOLINT
		YSOS_LOG_DEBUG("[有人/无人] Function Is Run ");
		if(YSOS_ERROR_SUCCESS !=ChangePeopleState(temp_buffer_str,people_info)){
      //YSOS_LOG_DEBUG("[有人/无人]的识别状态无改变");
			break;
		}

		YSOS_LOG_DEBUG("[DoEventNotifyService()] Function Is Run ");
		///< 上抛人脸判断事件数据 // NOLINT
		if (YSOS_ERROR_SUCCESS != DoEventNotifyService(event_name_, event_callback_name_, people_info, context)) {
			YSOS_LOG_DEBUG("Execute failure[DoEventNotifyService()]");
			break;
		}
		YSOS_LOG_DEBUG("[有人/无人]消息改变：" << people_info);
	} while (0);

	return YSOS_ERROR_SUCCESS;
}


int FaceStrategyCallback::Initialized(const std::string &key, const std::string &value) {
	YSOS_LOG_DEBUG(key << ": " << value);

	if ("nobodynumber" == key)
	{
		base_nobody_number_ = atoi(value.c_str());
	}
	else if ("somebodynumber" == key)
	{
		base_somebody_number_ = atoi(value.c_str());
	}
	else if ("eventname" == key)
	{
		event_name_ = value;
	}
	else if ("callbackname" == key)
	{
		event_callback_name_ = value;
	}
	else
	{
		return BaseStrategyCallbackImpl::Initialized(key,value);
	}

	return YSOS_ERROR_SUCCESS;
}


int FaceStrategyCallback::ChangePeopleState(const std::string& fece_info,std::string &output_str)
{
	YSOS_LOG_DEBUG("ChangePeopleState " << fece_info);
	Json::Features features = Json::Features::strictMode();                                       
	Json::Reader reader(features);
	Json::Value root;
	bool parsingSuccessful = reader.parse(fece_info, root, 1);
	if (!parsingSuccessful) 
	{	
		YSOS_LOG_DEBUG("GetProperty函数json解析出错.");
		return YSOS_ERROR_FAILED;
	}
	if (root.isNull()) 
	{	
		YSOS_LOG_DEBUG("GetProperty函数json解析数据为空.");
		return YSOS_ERROR_FAILED;
	}
	if (true != root.isMember("data") &&
		true != root["data"].isObject()
		)
	{
		YSOS_LOG_DEBUG("[data]函数json解析数据为空.");
		return YSOS_ERROR_FAILED;
	}

	Json::Value json_value_data = root["data"];
	int temp_number = 0;
	if (true != json_value_data.isMember("face_detect_info"))
	{
		YSOS_LOG_DEBUG("[face_detect_info]json解析数据为空.");
		return YSOS_ERROR_FAILED;
	}
	if (json_value_data["face_detect_info"].isObject())
	{
		temp_number = 1;
	}

	if (now_people_state_ != temp_number)
	{
		now_number_++;
		if (1 == temp_number&&
			now_number_ >= base_somebody_number_
			)
		{
			now_number_ = 0;
			now_people_state_ = temp_number;
			output_str = "1";
			YSOS_LOG_DEBUG("来人了，来人了.");
			return YSOS_ERROR_SUCCESS;
		}
		else if (0 == temp_number&&
			now_number_ >= base_nobody_number_
			)
		{
			now_number_ = 0;
			now_people_state_ = temp_number;
			output_str = "0";
			YSOS_LOG_DEBUG("人走了，人走了.");
			return YSOS_ERROR_SUCCESS;
		}
	}
	else
	{
		now_number_ = 0;
	}

	return YSOS_ERROR_NO_OUTPUT;
}

}