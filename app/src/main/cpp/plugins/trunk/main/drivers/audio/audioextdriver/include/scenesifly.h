/**   //NOLINT
  *@file scenesifly.h
  *@brief Definition of 应用迅飞的场景
  *@version 0.1
  *@author wanfenfen
  *@date Created on: 2016:8:13   19:00
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  */
#pragma once

#include "../include/voiceinterfaceifly.h"
//#include "customersystemjthc.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"

#include "../include/scenesinterface.h"
#include "../include/scenesimp1.h"
//#include "CnComm.h"


class ScenesIFLY: public virtual VoiceInterfaceIFLY, public virtual /*ScenesInterface*/ScenesImp1 {
 public:
  ScenesIFLY(void);
  virtual ~ScenesIFLY(void);

  //inherit from VoiceInterfaceIFLY
  //virtual int ASRSessionBegin(const int lFlag,std::string& ReceiveSeesionID);
  virtual int Login();  ///< 如果登录失败，则会尝试退出，再登录
  virtual int DomSomethingBeforeAsr();
  virtual int ASRGetResult(const char* sessionID, int* rsltStatus,std::string& ReceiveResult);  ///<   ///在迅飞结果基础上添加 字段 nlptype
  virtual int NLP(const char* pTextNeedNlp,std::string& ReceiveNLPResult);  ///<  在迅飞结果基础上添加 字段 nlptype，
  virtual int ReadConfig(const char* pFilePath);

  //inherit from ScenesInterface
  virtual void Scenes0ptimization(int iParam);
  virtual int GetVoiceProviderID(const std::string strType);
  //virtual int NLP2(int iVoiceServiceProvider, const char* pTextNeedNlp, std::string& ReceiveNLPResult);
  virtual int ASRSessionBegin2(const int lFlag, const int iOwnerid, std::string& ReceiveSeesionID);
  //virtual int CheckDevice(const int iCheckItem, int& iErrorCode, std::string* pstrCheckErrorResult);
  virtual std::string ExeScenceFun(const int iFunid, std::string strParam);
// virtual int LingYunQuery(const std::string strQuery, std::string& strInterruptResult);
  virtual int GetNlpProvidersAfterAsr(std::vector<int>& vec_nlpproviders);
  virtual int GetNlpDefaultProviders(std::vector<int>& vec_nlpproviders);
  virtual int NLP2(const char* pTextNeedNlp, bool filter_null_result,std::string& ReceiveNLPResult);
  //virtual int IsNlpDataHaveResult(const boost::property_tree::ptree& json_result, bool& is_have_nlp_result);
  virtual int IsNlpDataHaveResult(const Json::Value& json_result, bool& is_have_nlp_result);

  /**
   *@brief 是否被过滤，通过配置参数nlp_only_include_service，nlp_exclude_service 来确定是否要被过滤
   *@param
   *@return
   */
  //bool IsFilterByService(boost::property_tree::ptree& result_json);
  //bool IsFilterByService(Json::Value& result_json);
  bool IsFilterByService(std::string& result_service_name);

  //int ConvertNlpResultToStandasd(const char* pNlpResult, std::string& recvStandardNlpString);

  //boost::atomic<int> config_voice_filter_min_characters_atomic_;  ///< 如果是打断模式下，则过滤的字符数。默认是6（即3个中文字）
  //boost::atomic<int> config_linyun_kf_enable_atomic_;  ///< 启用灵云客服系统,在得到ASR后，同时调灵云客服系统和第三方NLP,默认0
  //CustomerSystemJTHS customer_system_jths_;   ///< 捷通华声的客服系统


  /// 优化方面
  boost::atomic<int> config_optimization_load_ahead_weather_;  ///< ;优化，提前加载天气(迅飞),默认0
  std::string config_optimixation_default_weather_city_;   ///< 优化，提前加载天气的城市，默认为空

  int config_use_device_mode_;  ///< 使用普通麦克风还是阵列，0普通麦克风，1阵列
 protected:
   int ConvertNlpResultToStandasd(const char* pCallAbility, const char* pNlpResult, std::string& recvStandardNlpString);
   int ConvertLocalNlpResultToStandasd(const char* pCallAbility, const char* pNlpResult, std::string& recvStandardNlpString);
   void GetAsrInfo(std::string& service, std::string& answer_old);

  friend void ThreadFunScenes0ptimization(ScenesIFLY* pScenesIFLY);
  int NLP2Child(const char* pTextNeedNlp, bool filter_null_result,std::string& ReceiveNLPResult, const bool permanent_caching);


  /**
  *@brief 转换一下天气数据格式，从3.0转为2.0
  *@param
  *@return
  */
  int ConvertNlp3To2(boost::property_tree::ptree& pt_3);
  int ConvertNlp3To2(Json::Value& pt_3);

  /**
   *@brief 解析迅飞离线命令词数据
   *@param 
   *@return   
   */
  int DecodeIflyLocalResult(const std::string& rawresult, std::string* rawtext);
 private:
  std::string strcurrent_sum_result_asr_;   ///< asr所有的结果
  boost::atomic<int> asr_flag_atomic_;
  boost::atomic<int> asr_ownerid_atomic_;
  std::string ocx_file_path_;

  std::vector<int> vec_nlp_after_startasr_providers_;
  std::vector<int> vec_nlp_default_providers_;

  bool asr_flag0_only_asr_;    ///< asr flag0 是否只是asr,不需要nlp，如果是，则只需要asr，再根据其他第三方来填充nlp的部分(默认是0）

  /*;以下两个配置只能配置一个，一个是只包含某些服务 而排除其他，一个是是排除特定的某些服务
    ;如果两个都配置了，则使用第1个配置
    ;以下两参数会返回的结果产生影响（asrnlp的结果，nlp的结果）*/
  std::string  conifg_nlp_only_include_service_;
  std::string  config_nlp_exclude_service_;
  std::vector<std::string> vec_nlp_only_include_service_;
  std::vector<std::string> vec_nlp_exclude_service_;

  };
