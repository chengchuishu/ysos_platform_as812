/**
 *@file nlpextdriver.h
 *@brief nlpext interface
 *@version 1.0
 *@author l.j..
 *@date Created on: 2016-10-20 13:48:23
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

#ifndef NLPEXT_INTERFACE_H_    //NOLINT
#define NLPEXT_INTERFACE_H_    //NOLINT

#include <json/json.h>
#include "../../../public/include/httpclient/httpclient.h"

//ysos Headers    //NOLINT
#include "../../../../../../core/trunk/main/public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/driverinterface.h"
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/common.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/protect/include/os_hal_package/datainterfacemanager.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basedriverimpl.h"
//com ctrl Headers    //NOLINT

#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"

#ifdef _WIN32
  #include <windows.h>
#else
  #include <time.h>
  #define  __stdcall
  #define WINAPI 
#endif
  
namespace ysos {

/**
 *@brief  语音模块驱动层 //NOLINT
*/
class YSOS_EXPORT NlpExtDriver : public BaseDriverImpl {
  DECLARE_CREATEINSTANCE(NlpExtDriver);
  DECLARE_PROTECT_CONSTRUCTOR(NlpExtDriver);

 public:
  virtual ~NlpExtDriver(void);
  /**
   *@brief 设置接口的属性值  // NOLINT
   *@param type_id[Input]： 属性的ID  // NOLINT
   *@param type[input]： 属性值的值  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int SetProperty(int type_id, void *type);

  /**
   *@brief 获取接口的属性值  // NOLINT
   *@param type_id[Input]： 属性的ID  // NOLINT
   *@param type[Input/Output]：属性值的值  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int GetProperty(int type_id, void *type);

  /**
   *@brief 从底层驱动中读取数据  // NOLINT
   *@param pBuffer[Output]： 读取的数据缓冲  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Read(BufferInterfacePtr pBuffer, DriverContexPtr context_ptr=NULL);

  /**
   *@brief 写数据到底层驱动中  // NOLINT
   *@param pBuffer[Input]： 写入的数据缓冲  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Write(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer=NULL, DriverContexPtr context_ptr=NULL);

  /**
   *@brief 控制驱动状态/配置参数到驱动/读取状态等  // NOLINT
   *@param iCtrlID[Input]： ID  // NOLINT
   *@param pInputBuffer[Input]： 写入的数据缓冲  // NOLINT
   *@param pOutputBuffer[Output]： 读出的数据缓冲  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Ioctl(int iCtrlID, BufferInterfacePtr pInputBuffer, BufferInterfacePtr pOutputBuffer);

  /**
   *@brief 打开底层驱动，并传入配置参数  // NOLINT
   *@param pParams[Input]： 驱动所需的打开参数  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Open(void *pParams);

  /**
   *@brief 打开底层驱动，并传入配置参数  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 无  // NOLINT
   */
  virtual void Close(void *pParams = nullptr);

  /**
   *@brief 基本初始化  // NOLINT
   *@param param： 初始化的参数  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Initialize(void *param=NULL);

  /**
   *@brief 清除资源,Initialize的逆操作  // NOLINT
   *@param param： 初始化的参数 // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int UnInitialize(void *param=NULL);

 protected:
  /**
  *@brief  配置文件中设置的Key、Value对，会在Initialized函数中调用 //  NOLINT
  *@param key 配置参数中的Key  // NOLINT
  *@param value 与Key对应的Value  // NOLINT
  *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
  */
  virtual int Initialized(const std::string &key, const std::string &value);

 public:
  int CallAbility(AbilityParam *pInput, AbilityParam *pOut);

  int AblityNlp_JsonExtract2Json(AbilityParam *pInput, AbilityParam *pOut);

  int TdQuery(const std::string &asr_result, std::string &rsp_data);

  int ConvertNlpResultToStandasd(const std::string &asr_result, const std::string &nlp_result, std::string &convert_result, std::string &rsp_service, const int &time);

  int WordsConvert(int kind, std::string &query);

  std::string GetAUniqueNum();

  void SymbolClear(std::string &data);

  int GetToken(void);
  void LogDialog(const std::string log_date, const std::string log_time, std::string logpath, const std::string term_id, int mode, const std::string msg);
  void LogNlpAccumulation(const std::string log_date, const std::string log_time, const std::string filepath, const std::string term_id, const std::string &asr_result, const std::string &nlp_result);
  void CleanToken(void);
 public:
  //< 灵云后台服务系统的地址
  int result_type_;                                       //< 结果类型 0：神聊 1：业务
  int warn_switch_;                                       //< 是否提示
  int empty_times_;                                       //< 空返回次数
  //int interrupt_type_;                                    //< 打断类型 0：打断 1：不打断
  std::string warn_words_chat_;                           //< 神聊提示语
  std::string warn_words_busi_;                           //< 业务提示语
  std::string config_zj_query_url_;               //< 获取nlp结果的地址
  std::string config_mall_code_;
  std::string config_machine_code_;
  std::string config_floor_;
  std::string config_location_;
  int config_nav_switch_;
  int config_is_wake_;


  Json::Reader json_reader_;
  Json::FastWriter json_writer_;
  std::string config_td_init_url_;
  std::string config_td_renewal_url_;
  std::string config_td_query_url_;
  std::string config_td_clear_url_;
  std::string init_tag_;
  std::string init_version_;
  std::string support_model_;
  std::string token_;
  
  bool ioctl_flag_;
  
  int bussiness_switch_;

  bool is_clear_;

  // 是否过滤本地语音识别发过来的查询请求，0 不过滤，1过滤，默认是1。  即 查询请求json中字段 asr_mode = local，则透传不处理
  int filter_asr_local_engine_query_;

  UINT32 thread_id_;
  bool thread_exit_;
  HttpClientInterface *nlp_http_client_;

  DataInterfacePtr data_ptr_;                             //< 内存指针

 private:
  static UINT32 WINAPI QueryThread(LPVOID param); //add for linux

  ///< 语料收集记录音频有转写结果但未被智能知识库正常识别的音频
  ///< 0. 不记录
  ///< 1. 记录音频有转写结果但未被智能知识库正常识别的结果
  int config_local_record_file_filter_mode_;
  std::string config_local_record_file_path_;///<记录保存目录
  ///<会话保存　0-不保存　1-保存
  int config_local_record_file_dialog_;
  std::string config_local_record_file_dialog_path_;///<会话保存目录
  std::string term_id_;///<用于记录和会话的终端号

public:
  HANDLE hand_;

  UINT32 init_connect_timeout_;
  UINT32 init_read_timeout_;
  UINT32 renewal_connect_timeout_;
  UINT32 renewal_read_timeout_;
  UINT32 query_connect_timeout_;
  UINT32 query_read_timeout_;
  UINT32 clear_connect_timeout_;
  UINT32 clear_read_timeout_;
  std::string timeout_broadcast_;
};

};

#endif  //NLPEXT_INTERFACE_H_  //NOLINT