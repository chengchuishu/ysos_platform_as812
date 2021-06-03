/**
 *@file configinformationmodule.h
 *@brief config information module interface
 *@version 1.0
 *@author XueXiaojun
 *@date Created on: 2017-07-28 09:18:00
 *@copyright Copyright (c) 2017 YunShen Technology. All rights reserved.
 * 
 */

/// self header
#include "../include/configinformationmodule.h"


#ifndef SLEEP
#define SLEEP(a)      (boost::this_thread::sleep_for(boost::chrono::milliseconds(a)))
#endif

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
DECLARE_PLUGIN_REGISTER_INTERFACE(ConfigInformationModule, ModuleInterface);
ConfigInformationModule::ConfigInformationModule(const std::string &strClassName):BaseThreadModuleImpl(strClassName) 
,module_driver_ptr_(NULL)
{
	logger_ = GetUtility()->GetLogger("ysos.configinformation");
	YSOS_LOG_DEBUG("ConfigInformationModule Is ok");

	config_input_buffer_ptr_ = GetBufferUtility()->CreateBuffer(8192);
	config_output_buffer_ptr_ = GetBufferUtility()->CreateBuffer(8192);
}

ConfigInformationModule::~ConfigInformationModule() {
	module_driver_ptr_ = NULL;
}

int ConfigInformationModule::Initialized(const std::string &key, const std::string &value)  {
	int n_return = YSOS_ERROR_FAILED;

	YSOS_LOG_DEBUG("ConfigModule Initialized start:"<<key);

	int iret = BaseModuleImpl::Initialized(key, value);

	if(key.compare("relate_driver") == 0 ) {           ///< 关联的驱动driver的逻辑名
		releate_driver_name_= value;
		YSOS_LOG_DEBUG("Config Driver Name: "<<releate_driver_name_);

	}/* else if(key.compare("relate_callback") == 0 ) {  ///< 关联的回调callback的逻辑名
		releate_callback_name_ = value;
		YSOS_LOG_DEBUG("Config Callback Name:"<<releate_callback_name_);

	}*/ 

	return n_return;
}

int ConfigInformationModule::RealOpen(LPVOID param) {
	YSOS_LOG_DEBUG("[configinformation]module RealOpen execute:"<<releate_driver_name_);
	int n_return = YSOS_ERROR_FAILED;

	do {
		if (NULL == module_driver_ptr_) {
			module_driver_ptr_ = GetDriverInterfaceManager()->FindInterface(releate_driver_name_);
			if (NULL == module_driver_ptr_) {
				YSOS_LOG_DEBUG("get config_prt_ failed");
				break;
			}
		}
		n_return = module_driver_ptr_->Open(param);
		if (YSOS_ERROR_SUCCESS != n_return) {
			break;
		}

		/*if (NULL == module_callback_ptr_)
		{
			module_callback_ptr_ = GetCallbackInterfaceManager()->FindInterface(releate_callback_name_);
			if (NULL == module_callback_ptr_) {
				YSOS_LOG_DEBUG("get config_prt_ failed");
				break;
			}
		}*/

		// start connect heart thread
		YSOS_LOG_DEBUG("start connect update thread");
		check_update_thread_ = boost::thread(boost::bind(Check_Update_Thread,this));
		YSOS_LOG_DEBUG("over connect update thread");

		n_return = BaseThreadModuleImpl::RealOpen(param);
		if (n_return != YSOS_ERROR_SUCCESS)
			break;
	} while (0);
	YSOS_LOG_DEBUG("module RealOpen done");

	return n_return;
}

int ConfigInformationModule::RealRun(void) {

	int n_return = YSOS_ERROR_SUCCESS;
	return n_return;
}

int ConfigInformationModule::RealPause(void) {
	return YSOS_ERROR_SUCCESS;
}

int ConfigInformationModule::Ioctl(INT32 control_id, LPVOID param) {
	return BaseThreadModuleImpl::Ioctl(control_id,param);
}

int ConfigInformationModule::Flush(LPVOID param /*= nullptr*/) {
	return YSOS_ERROR_SUCCESS;
}

int ConfigInformationModule::Initialize(LPVOID param /*= nullptr*/) {
	YSOS_LOG_DEBUG("ConfigModule Initialize is start");
	//config_input_buffer_ptr_ = GetBufferUtility()->CreateBuffer(4096);
	//config_output_buffer_ptr_ = GetBufferUtility()->CreateBuffer(4096);
	return BaseThreadModuleImpl::Initialize(param);
}

int ConfigInformationModule::UnInitialize(LPVOID param /*= nullptr*/) {
	return YSOS_ERROR_SUCCESS;
}

int ConfigInformationModule::RealClose() {

	int n_return = YSOS_ERROR_FAILED;

	module_driver_ptr_->Close();
	module_driver_ptr_ = NULL;

	// stop connect heart thread
	check_update_thread_.interrupt();
	check_update_thread_.join();

	n_return = BaseThreadModuleImpl::RealClose();

	return n_return;
}

int ConfigInformationModule::InitalDataInfo(void) {
	return YSOS_ERROR_SUCCESS;
}

int ConfigInformationModule::RealStop() {

	int n_return = YSOS_ERROR_FAILED;

	if (module_driver_ptr_) {
		n_return = module_driver_ptr_->Ioctl(CMD_ICC_STOP, NULL, NULL);
	}

	return n_return;
}


int ConfigInformationModule::GetProperty(int iTypeId, void *piType) {
	int n_return = YSOS_ERROR_FAILED;
	do {
		switch (iTypeId) {
		case 0: {
			n_return = module_driver_ptr_->GetProperty(iTypeId, piType);
		}
				break;
		default: {
			n_return = BaseThreadModuleImpl::GetProperty(iTypeId, piType);
					}
		}
	} while (0);

	return n_return;
}

int ConfigInformationModule::Check_Update_Thread( LPVOID lpParam ) {
	ConfigInformationModule *config_module_ptr = (ConfigInformationModule*)lpParam;
	if (config_module_ptr == NULL) {
		return YSOS_ERROR_INVALID_ARGUMENTS;
	}

	YSOS_LOG_DEBUG_CUSTOM(config_module_ptr->logger_, "Check_Update_Thread start running");

	DriverInterfacePtr config_driver_ptr = config_module_ptr->module_driver_ptr_;
	BufferInterfacePtr input_buffer_ptr = config_module_ptr->config_input_buffer_ptr_; 
	BufferInterfacePtr output_buffer_ptr = config_module_ptr->config_output_buffer_ptr_; 
	if (config_driver_ptr==NULL || input_buffer_ptr==NULL || output_buffer_ptr==NULL) {
		YSOS_LOG_ERROR_CUSTOM(config_module_ptr->logger_, "ConfigModule Input Or Output Exception!");
		return YSOS_ERROR_INVALID_ARGUMENTS;
	}
	//std::string strconnect_string = config_module_ptr->Make_Connect_Json_String();

	bool benable_heart_flag = false;
	int exception_cnt = 0;
	//int chassis_connect_status = CHASSIS_STATUS_DISCONNECT;

	YSOS_LOG_ERROR_CUSTOM(config_module_ptr->logger_, "Check_Update_Thread is one!");
	while(true)
	{
		YSOS_LOG_ERROR_CUSTOM(config_module_ptr->logger_, "Check_Update_Thread is two!");
		for (int i = 0;i<1;i++)
		{
			///< 发送首页资源json报文到后台  // NOLINT
			if(YSOS_ERROR_SUCCESS != config_driver_ptr->Ioctl(CMD_CONFIG_MESSAGE_ONE,input_buffer_ptr,output_buffer_ptr))
			{
				YSOS_LOG_DEBUG_CUSTOM(config_module_ptr->logger_, "CMD_CONFIG_MESSAGE_ONE Is Error ");
				break;
			}
			YSOS_LOG_ERROR_CUSTOM(config_module_ptr->logger_, "Check_Update_Thread is 3333!");
			//YSOS_LOG_DEBUG_CUSTOM(config_module_ptr->logger_, "CMD_CONFIG_MESSAGE_ONE Is Error "<<);
			///< 获取首页资源信息  // NOLINT
			if(YSOS_ERROR_SUCCESS != config_driver_ptr->Ioctl(CMD_CONFIG_READ,input_buffer_ptr,output_buffer_ptr))
			{
				YSOS_LOG_DEBUG_CUSTOM(config_module_ptr->logger_, "CMD_CONFIG_READ Is Error ");
				break;
			}
			YSOS_LOG_ERROR_CUSTOM(config_module_ptr->logger_, "Check_Update_Thread is 4444!");
			///< 检查/更新 首页资源  // NOLINT
			if(YSOS_ERROR_SUCCESS != config_driver_ptr->Ioctl(CMD_CHECK_PAGE_UPDATE,input_buffer_ptr,output_buffer_ptr))
			{
				YSOS_LOG_DEBUG_CUSTOM(config_module_ptr->logger_, "CMD_CHECK_PAGE_UPDATE Is Error ");
				break;
			}
			YSOS_LOG_ERROR_CUSTOM(config_module_ptr->logger_, "Check_Update_Thread is 5555!");
			///< 发送程序资源json报文到后台  // NOLINT
			if(YSOS_ERROR_SUCCESS != config_driver_ptr->Ioctl(CMD_CONFIG_MESSAGE_TWO,input_buffer_ptr,output_buffer_ptr))
			{
				YSOS_LOG_DEBUG_CUSTOM(config_module_ptr->logger_, "CMD_CONFIG_MESSAGE_TWO Is Error ");
				break;
			}

			///< 获取程序资源信息  // NOLINT
			if(YSOS_ERROR_SUCCESS != config_driver_ptr->Ioctl(CMD_CONFIG_READ,input_buffer_ptr,output_buffer_ptr))
			{
				YSOS_LOG_DEBUG_CUSTOM(config_module_ptr->logger_, "CMD_CONFIG_READ Is Error ");
				break;
			}

			///< 检查/更新程序资源  // NOLINT
			if(YSOS_ERROR_SUCCESS != config_driver_ptr->Ioctl(CMD_CHECK_SOURCE_UPDATE,input_buffer_ptr,output_buffer_ptr))
			{
				YSOS_LOG_DEBUG_CUSTOM(config_module_ptr->logger_, "CMD_CHECK_SOURCE_UPDATE Is Error ");
				break;
			}
		}
		SLEEP(2000);
		break;
	}

	return YSOS_ERROR_SUCCESS;
}

}