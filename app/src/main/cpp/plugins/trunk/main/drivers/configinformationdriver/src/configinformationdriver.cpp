/**
 *@file configure_information_driverinterface.cpp
 *@brief Implement of configure information driver
 *@version 0.9.0.0
 *@author Xue xiaojun
 *@date Created on: 2017-7-27 12:40:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

#include "../include/configinformationdriver.h"

#include <json/json.h>
/// Public Header
#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"

#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/manager_declare.h"





namespace ysos {

/**
  *@brief 驱动接口，用于管理底层驱动封装实例，
  */
DECLARE_PLUGIN_REGISTER_INTERFACE(ConfigureInformationDriver, DriverInterface);
ConfigureInformationDriver::ConfigureInformationDriver(const std::string &strClassName): BaseInterfaceImpl(strClassName) 
	,seer_ptr(NULL)
	,connection_http_ptr(NULL)
	,connection_down_ptr(NULL)
	,connection_down_name_("default@HttpDownloadConnection1")
	,connection_http_name_("default@HttpClientConnection1")
	,is_update_page_(false)
	,is_update_source_(false)
{
	logger_ = GetUtility()->GetLogger("ysos.configinformation");
	YSOS_LOG_DEBUG("ConfigInformationDriver[driver] is ok");
}


ConfigureInformationDriver::~ConfigureInformationDriver() {
 
}


int ConfigureInformationDriver::SetProperty(int type_id, void *type) {
	YSOS_LOG_DEBUG("ConfigInformationDriver");
	int n_return = YSOS_ERROR_SUCCESS;
	YSOS_LOG_DEBUG("ConfigInformationDriver");
	return n_return;
}


int ConfigureInformationDriver::Read(BufferInterfacePtr pBuffer, DriverContexPtr context_ptr) {
  int return_value = YSOS_ERROR_SUCCESS;
  do {
    //  TODO:
  } while (false);
  return return_value;
}


/**
  *@brief 写数据到底层驱动中
  *@param input_buffer[Input]： 写入的数据缓冲
  *@param output_buffer[Output]： 读取的数据缓冲
  *@param context_ptr[In|Out]： Driver上下文
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值
  */
int ConfigureInformationDriver::Write(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, DriverContexPtr context_ptr) {
  int return_value = YSOS_ERROR_SUCCESS;
  do {
    //  TODO:
  } while (false);
  return return_value;
}

/**
*@brief 控制驱动状态/配置参数到驱动/读取状态等
*@param iCtrlID[Input]： ID
*@param pInputBuffer[Input]： 写入的数据缓冲
*@param pOutputBuffer[Output]： 读出的数据缓冲
*@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值
*/
int ConfigureInformationDriver::Ioctl(int iCtrlID, BufferInterfacePtr pInputBuffer, BufferInterfacePtr pOutputBuffer) {
 
	int return_value = YSOS_ERROR_SUCCESS;
   switch (iCtrlID) {
   case CMD_CONFIG_MESSAGE_ONE:
	  YSOS_LOG_DEBUG("ConfigInformation[Driver] CMD_CONFIG_MESSAGE_ONE is start");
 	  //return_value = connection_http_ptr->Open("http://jqrcs.tangdi.net:8088/tdcctp/BC0001.json?REQ_MESSAGE=");
	  //TODO: add for linux ignore
      //return_value = connection_http_ptr->Open("http://jqrcs.tangdi.net:8088/tdcctp/IR00001.json?REQ_MESSAGE={\"REQ_HEAD\":{\"TRAN_SUCCESS\":\"1\"},\"REQ_BODY\":{\"TransCode\":\"Tx000001\",\"termId\":\"T1000061\",\"orgId\":\"000000011\"}}");

	  YSOS_LOG_DEBUG("ConfigInformation[Driver] CMD_CONFIG_MESSAGE_ONE is ones");
	  if (YSOS_ERROR_SUCCESS!=return_value)
	  {
		  YSOS_LOG_DEBUG("ConfigInformation[Driver] CMD_CONFIG_MESSAGE_ONE open is error");
		  return YSOS_ERROR_FAILED;
	  }
 	return return_value;
   case CMD_CONFIG_MESSAGE_TWO:
	   YSOS_LOG_DEBUG("ConfigInformation[Driver] CMD_CONFIG_MESSAGE_ONE is start");
	   //return_value = connection_http_ptr->Open("http://jqrcs.tangdi.net:8088/tdcctp/BC0001.json?REQ_MESSAGE=");
	   //TODO: add for linux ignore
       //return_value = connection_http_ptr->Open("http://jqrcs.tangdi.net:8088/tdcctp/BC0001.json?REQ_MESSAGE={\"REQ_HEAD\":{\"TRAN_SUCCESS\":\"1\"},\"REQ_BODY\":{\"TransCode\":\"Tx000001\",\"termId\":\"T1000061\",\"orgId\":\"000000011\"}}");
    
	   YSOS_LOG_DEBUG("ConfigInformation[Driver] CMD_CONFIG_MESSAGE_ONE is ones");
	   if (YSOS_ERROR_SUCCESS!=return_value)
	   {
		   YSOS_LOG_DEBUG("ConfigInformation[Driver] CMD_CONFIG_MESSAGE_ONE open is error");
		   return YSOS_ERROR_FAILED;
	   }
 	  return return_value;
   case CMD_CONFIG_READ:
	  YSOS_LOG_DEBUG("ConfigInformation[Driver] CMD_CONFIG_READ is start");
 	  return_value = connection_http_ptr->Read(pOutputBuffer,4096,NULL);
	  YSOS_LOG_DEBUG("connection_http_ptr OutPutStr:[]"<<reinterpret_cast<const char*>(GetBufferUtility()->GetBufferData(pOutputBuffer)));
 	  return return_value;		
   case CMD_CONFIG_WIRTER:
 
 	return return_value;
   case CMD_CHECK_PAGE_UPDATE:
	   return_value = AnalyzePageData(pOutputBuffer);		
	   return return_value;
   case CMD_CHECK_SOURCE_UPDATE:
	   return_value = AnalyzeSourceData(pOutputBuffer);
	   return return_value;
   case CMD_DOWN_FILE:
      //connection_down_ptr->
 	  return return_value;
   default: {
 	YSOS_LOG_DEBUG("this cmd is not supported.");
 	return YSOS_ERROR_NOT_SUPPORTED;
   }
   }
   return YSOS_ERROR_SUCCESS;
 }
 
 
/**
  *@brief 打开底层驱动，并传入配置参数
  *@param pParams[Input]： 驱动所需的打开参数
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值
  */
int ConfigureInformationDriver::Open(void *pParams) {
  YSOS_LOG_DEBUG("ConfigInformation[Driver] open is start");

  int return_value = YSOS_ERROR_SUCCESS;
  do {
	  YSOS_LOG_DEBUG(" connection_http_ptr Find Driver name:" << connection_http_name_);
	  connection_http_ptr = GetConnectionInterfaceManager()->FindInterface(connection_http_name_);
	  if(NULL == connection_http_ptr) {
		  YSOS_LOG_ERROR("connection_http_ptr Find Driver Fail");
		  return YSOS_ERROR_FAILED;
	  }
	  YSOS_LOG_ERROR("connection_http_ptr Find Driver successful");

	  YSOS_LOG_DEBUG(" connection_down_ptr Find Driver name:" << connection_down_name_);
	  connection_down_ptr = GetConnectionInterfaceManager()->FindInterface(connection_down_name_);
	  if(NULL == connection_down_ptr) {
		  YSOS_LOG_ERROR("connection_down_ptr Find Driver Fail");
		  return YSOS_ERROR_FAILED;
	  }
	  YSOS_LOG_ERROR("connection_down_ptr Find Driver successful");
    //  TODO:
  } while (false);
  YSOS_LOG_DEBUG("ConfigInformation[Driver] open is successful");
  return return_value;
}


 int ConfigureInformationDriver::AnalyzePageData(BufferInterfacePtr input_buffer_ptr)
 {
 	YSOS_LOG_DEBUG("AnalyzePageData[driver] is run");
 	Json::Reader js_reader;
 	Json::Value root;
 	YSOS_LOG_DEBUG("Page Data json:["<<reinterpret_cast<const char*>(GetBufferUtility()->GetBufferData(input_buffer_ptr)));
 	if(!js_reader.parse(reinterpret_cast<const char*>(GetBufferUtility()->GetBufferData(input_buffer_ptr)),root,false)){
 		YSOS_LOG_ERROR("page data json error");
 		return YSOS_ERROR_FAILED;
 	}
	///< json解析  // NOLINT
	if (root.isNull()) 
	{	
		YSOS_LOG_DEBUG("AnalyzePageData函数json解析数据为空.");
		return YSOS_ERROR_FAILED;
	}
	if (true != root.isMember("REP_HEAD"))
	{
		YSOS_LOG_DEBUG("[REP_HEAD]不是json字符串成员");
		return YSOS_ERROR_FAILED;
	}
	Json::Value json_value_data = root["REP_HEAD"];

	if (true != json_value_data.isMember("TRAN_CODE"))
	{
		YSOS_LOG_DEBUG("[TRAN_CODE]不是json字符串成员");
		return YSOS_ERROR_FAILED;
	}
	if (0!=strcmp("000000",json_value_data["TRAN_CODE"].asString().c_str()))
	{
		YSOS_LOG_DEBUG("收到后台的首页资源信息失败!");
		return YSOS_ERROR_FAILED;
	}
	if (true != root.isMember("REP_BODY"))
	{
		YSOS_LOG_DEBUG("[REP_BODY]不是json字符串成员");
		return YSOS_ERROR_FAILED;
	}
	json_value_data.clear();
	json_value_data = root["REP_BODY"];
	if (true != json_value_data.isMember("stepNum"))
	{
		YSOS_LOG_DEBUG("[REP_BODY]不是json字符串成员");
		return YSOS_ERROR_FAILED;
	}
	///< 资源数量  // NOLINT
	std::string pic_number = json_value_data["stepNum"].asString();
	if (true != json_value_data.isMember("pathPre"))
	{
		YSOS_LOG_DEBUG("[REP_BODY]不是json字符串成员");
		return YSOS_ERROR_FAILED;
	}
	///< 服务器地址  // NOLINT
	std::string server_path = json_value_data["pathPre"].asString();
					
	int file_size = json_value_data["files"].size();  ///< 得到"files"的数组个数    //NOLINT

	BufferInterfacePtr input_path_ptr;
	input_path_ptr = GetBufferUtility()->CreateBuffer(1024);
	for(int i = 0; i < file_size; ++i)    
	{  
		Json::Value val_image = json_value_data["files"][i];  
		std::string curl_file_name_str = val_image["filePath"].asString();  
		std::string server_md5_str  = val_image["md5"].asString(); 
		std::string server_file_path = server_path + curl_file_name_str;
		///< 获取文件的md5码值
		std::string base_path = "D:\\picture\\";
		base_path += curl_file_name_str;
		YSOS_LOG_DEBUG("curl_file_name_str:["<<base_path.c_str());

		std::string curl_file_md5 = GetMD5(base_path.c_str());
		YSOS_LOG_DEBUG("server_md5:["<<server_md5_str.c_str());
		YSOS_LOG_DEBUG("curl_md5:["<<curl_file_md5.c_str());
		if (0==strcmp(curl_file_md5.c_str(),server_md5_str.c_str()))
		{
			YSOS_LOG_DEBUG("AnalyzePageData[driver] MD5 Is Same");
			continue;
		}
		///< 开始下载需要更新的文件  // NOLINT
		YSOS_LOG_DEBUG("AnalyzePageData[driver] File Is Down");
		Json::Value val_write;
		Json::FastWriter writer;
		val_write["HTTP_PATH"] = server_file_path.c_str();
		std::string curl_temp_str = "D:\\picture\\" + curl_file_name_str;
		val_write["LOCAL_PATH"] = curl_temp_str.c_str();
		GetBufferUtility()->InitialBuffer(input_path_ptr);
		GetBufferUtility()->CopyStringToBuffer(writer.write(val_write).c_str(), input_path_ptr);
		YSOS_LOG_DEBUG("down_file_name:["<<reinterpret_cast<const char*>(GetBufferUtility()->GetBufferData(input_path_ptr)));
		connection_down_ptr->Write(input_path_ptr,0);
	}  
	YSOS_LOG_DEBUG("AnalyzePageData[driver] is successful");
	is_update_page_ = true;
	return YSOS_ERROR_SUCCESS;
 }
 
int ConfigureInformationDriver::AnalyzeSourceData(BufferInterfacePtr input_buffer_ptr)
{
	YSOS_LOG_DEBUG("AnalyzeSourceData[driver] is run");
	Json::Reader js_reader;
	Json::Value root;
	YSOS_LOG_DEBUG("Source Data json:["<<reinterpret_cast<const char*>(GetBufferUtility()->GetBufferData(input_buffer_ptr)));
	if(!js_reader.parse(reinterpret_cast<const char*>(GetBufferUtility()->GetBufferData(input_buffer_ptr)),root,false)){
		YSOS_LOG_ERROR("page data json error");
		return YSOS_ERROR_FAILED;
	}
	///< json解析  // NOLINT
	if (root.isNull()) 
	{	
		YSOS_LOG_DEBUG("AnalyzeSourceData函数json解析数据为空.");
		return YSOS_ERROR_FAILED;
	}
	if (true != root.isMember("REP_HEAD"))
	{
		YSOS_LOG_DEBUG("[REP_HEAD]不是json字符串成员");
		return YSOS_ERROR_FAILED;
	}
	Json::Value json_value_data = root["REP_HEAD"];

	if (true != json_value_data.isMember("TRAN_CODE"))
	{
		YSOS_LOG_DEBUG("[TRAN_CODE]不是json字符串成员");
		return YSOS_ERROR_FAILED;
	}
	if (0!=strcmp("000000",json_value_data["TRAN_CODE"].asString().c_str()))
	{
		YSOS_LOG_DEBUG("收到后台的首页资源信息失败!");
		return YSOS_ERROR_FAILED;
	}
	if (true != root.isMember("REP_BODY"))
	{
		YSOS_LOG_DEBUG("[REP_BODY]不是json字符串成员");
		return YSOS_ERROR_FAILED;
	}
	json_value_data.clear();
	json_value_data = root["REP_BODY"];

	int file_size = json_value_data["elementList"].size();  ///< 得到"files"的数组个数    //NOLINT

	BufferInterfacePtr input_path_ptr;
	input_path_ptr = GetBufferUtility()->CreateBuffer(1024);
	for(int i = 0; i < file_size; ++i)    
	{  
		Json::Value val_image = json_value_data["elementList"][i];  
		std::string curl_file_name_str = val_image["elementFileId"].asString();  
		std::string server_md5_str  = val_image["elementMD5"].asString(); 
		std::string server_file_path = val_image["elementUrl"].asString(); 
		///< 获取文件的md5码值
		std::string base_path = "D:\\picture\\";
		base_path += curl_file_name_str;
		YSOS_LOG_DEBUG("curl_file_name_str:["<<base_path.c_str());

		std::string curl_file_md5 = GetMD5(base_path.c_str());
		YSOS_LOG_DEBUG("server_md5:["<<server_md5_str.c_str());
		YSOS_LOG_DEBUG("curl_md5:["<<curl_file_md5.c_str());
		if (0==strcmp(curl_file_md5.c_str(),server_md5_str.c_str()))
		{
			YSOS_LOG_DEBUG("AnalyzeSourceData[driver] MD5 Is Same");
			continue;
		}
		///< 开始下载需要更新的文件  // NOLINT
		YSOS_LOG_DEBUG("AnalyzeSourceData[driver] File Is Down");
		Json::Value val_write;
		Json::FastWriter writer;
		val_write["HTTP_PATH"] = server_file_path.c_str();
		std::string curl_temp_str = "D:\\picture\\" + curl_file_name_str;
		val_write["LOCAL_PATH"] = curl_temp_str.c_str();
		GetBufferUtility()->InitialBuffer(input_path_ptr);
		GetBufferUtility()->CopyStringToBuffer(writer.write(val_write).c_str(), input_path_ptr);
		YSOS_LOG_DEBUG("[AnalyzeSourceData]down_file_name:["<<reinterpret_cast<const char*>(GetBufferUtility()->GetBufferData(input_path_ptr)));
		if(0!=connection_down_ptr->Write(input_path_ptr,0))
		{
			YSOS_LOG_DEBUG("AnalyzeSourceData[driver] Down Is Faild");
			continue;
		}
	}  
	YSOS_LOG_DEBUG("AnalyzeSourceData[driver] is successful");
	is_update_source_ = true;
	return YSOS_ERROR_SUCCESS;
}

 // 获得MD5码   魏伽妮
 std::string ConfigureInformationDriver::GetMD5(const char* cul_file_path)
 {
     //TODO: add for linux temp ignore this code
     /* add for linux
	 ///< 打开文件  
	 HANDLE pfile;
	 pfile = ::CreateFile(cul_file_path,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

	 if ( pfile == INVALID_HANDLE_VALUE)
	 {
		 return "";
	 }

	 DWORD filesize=GetFileSize(pfile,NULL);

	 if (filesize<=0)
	 {
		 return "";
	 }

	 BYTE* buffer=new BYTE[filesize];																// 最后一位为 '/0',C-Style 字符串的结束符。

	 DWORD readsize;

	 if(0 == ReadFile(pfile,buffer,filesize,&readsize,NULL))
	 {
		 return "";
	 }
	 md5_ptr_.GenerateMD5(buffer,filesize);
	 std::string strMy ="";
	 strMy = md5_ptr_.ToString();
	 delete[] buffer;							///< 注意是delete[] 而不是 delete
	 CloseHandle(pfile);						///< 关闭句柄。
	 return strMy;		
     */
    return "";
 }


/**
  *@brief 关闭底层驱动，并传入配置参数
  *@param ： 无
  *@return： 无
  */
void ConfigureInformationDriver::Close(void *pParams) {
  YSOS_LOG_DEBUG("ConfigInformationDriver close.");
}


int ConfigureInformationDriver::GetProperty(int type_id, void *pi_type_ptr) { 
	YSOS_LOG_ERROR("ConfigureInformationDriver[driver] GetProperty is run.");
	if (NULL == pi_type_ptr){
		YSOS_LOG_ERROR("Get chassis system info Error.");
		return YSOS_ERROR_INVALID_ARGUMENTS;
	}

	BufferInterfacePtr* out_put_buffer_str = reinterpret_cast<BufferInterfacePtr*>(pi_type_ptr);
	UINT8* bufferptr = NULL;
	UINT32 bufferlength = 0;
	UINT32 buffermaxlength = 0;
	(*out_put_buffer_str)->GetBufferAndLength(&bufferptr, &bufferlength);
	(*out_put_buffer_str)->GetMaxLength(&buffermaxlength);
	switch (type_id){
	case 0:
		{
			YSOS_LOG_ERROR("ConfigureInformationDriver[driver] GetProperty is 111111111:"<<bufferlength<<",max:"<<buffermaxlength);
			/// 构造输出Json
			Json::Value js_data;
			js_data["service"] = "update_app_source";
			if (is_update_page_){
				js_data["PAGE_UPDATE"] = "YES";
			}else{
				js_data["PAGE_UPDATE"] = "NO";
			}
			if (is_update_source_){
				js_data["SOURCE_UPDATE"] = "YES";
			}else{
				js_data["SOURCE_UPDATE"] = "NO";
			}
			js_data["PAGE_PATH"] = "D:\\picture\\";
			js_data["SOURCE_PATH"] = "D:\\picture\\";
			Json::FastWriter js_writer;
			std::string strjson_result = js_writer.write(js_data);
			strjson_result = GetUtility()->ReplaceAllDistinct ( strjson_result, "\\r\\n", "" );
			YSOS_LOG_ERROR("ConfigureInformationDriver[driver] GetProperty is:"<<strjson_result.c_str()<<",length:"<<strjson_result.length());
			memcpy(bufferptr,strjson_result.c_str(),strjson_result.length());

			if (is_update_page_)
			{
				is_update_page_ = false;
			}
			if (is_update_source_)
			{
				is_update_source_ = false;
			}
		}
		break;
	default:
		YSOS_LOG_WARN("This property is not support.");
		return YSOS_ERROR_NOT_SUPPORTED;
	}

	return YSOS_ERROR_SUCCESS;
}


}