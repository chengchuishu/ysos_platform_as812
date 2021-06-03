/**   //NOLINT
  *@file scenesimp1.h
  *@brief Definition of 场景的实现1
  *@version 0.1
  *@author wanfenfen
  *@date Created on: 2016:8:14   16:20
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  */
#pragma once
/// stl heaers
#include <list>

/// boost headers
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

/// ysos headers
#include "../../../../../../core/trunk/main/public/include/core_help_package/jsonutility.h"

/// private haders
#include "../include/scenesinterface.h"

enum VoiceType {
  VoiceType_IFLY =0,          ///< 迅飞公有云2.0接口返回的数据json格式
  VoiceType_JTHC =1,
  VoiceType_CUSTOMER =2, // 2  注意，2数字已分配给客服系统
  // 3 小问
  // 4 微软
  VoiceType_MICROSOFT =4, 
  VoiceType_IFLY_PRIVATE =5,   ///< 迅飞私有云
  VoiceType_TULING       =6,   ///< 图灵
  VoiceType_BAIDU        =7,   ///< 百度
  VoiceType_IFLY_LOCAL   =8,   ///< 迅飞本地离线命令词
  VoiceType_FILY_CLOUD3  =9,   ///< 迅飞公有云3.0接口返回的数据
  VoiceType_IFLY_XML     =10,   ///< 迅飞公有云2.0接口,xml格式

  /// 50-100 都属于外来答案
  VOICETYPE_EXTERNAL_BEGIN  =50,
  VoiceType_RGZX             =51,   ///< 人工座席
  VOICETYPE_EXTERNAL_END    =100,
};
std::string GetVoiceTypeStr(unsigned int Type);

/**
  *@brief 场景的类型
  */
enum ScenesType {
  ScenesType_IFLY =0,  ///< 场景，以科大迅飞为主
  ScenesType_JHTC =1,  ///< 场景，以捷通华声为主
  ScenesType_ARTIFICIAL_AGENTS =2, ///< 人工坐席
};

std::string GetScenesTypeStr(unsigned int Type);


/**
  *@brief
   0 正常无错误
  -1 timeout超时，检测到尾端点获取识别结果超时，默认超时时间是10秒
  -2 打开录音设备失败
  -3 打开录音机失败
  -4 录音机没有回调数据
  -11 登录第三方语音失败 使用场景的默认语音服务商前需要登录。
  -12 发送音频数据失败 向场景的默认语音服务商发送音频数据失败
  -13 获取语音识别结果失败
  -14 建立语音识别连接失败
  -15 获取三方服务提供商失败 设置的服务提供商Id 不正确导致
  -21 Nlp解析出错
  */
enum TDASR_ERROR_TYPE {  ///< ASR ERROR TYPE
  TDASRERROR_SUCCESS =0,
  TDASRERROR_TIMEOUT = -1,
  TDASRERROR_OPEN_RECORDING_DEVICE_FAIL =-2,
  TDASRERROR_START_RECORDING_FAIL= -3,
  TDASRERROR_NO_RECORDING_DATA  =-4,
  TDASRERROR_LOGIN_THIRD_VOICE_PROVIDER_FAIL = -11,
  TDASRERROR_SEND_VOICE_DATA_FAIL = -12,
  TDASRERROR_GET_ASR_RESULT_FAIL = -13,
  TDASRERROR_BUILD_ASR_CONNECTION_FAIL = -14,
  TDASRERROR_NO_VOICE_PROVIDER = -15,
  TDASRERROR_NLP_FAIL = -21
};

/**
  *@brief ASR 错误的大致类别
  [-1 未知]
  [0 无错误]
  [1 录音机设备]
  [2 程序配置]
  [3 网络]
  */
enum TDASR_ERROR_CATEGORY {
  TDASRERROR_CATEGORY_NOTKNOWN = -1,
  TDASRERROR_CATEGORY_SUCCESS =0,
  TDASRERROR_CATEGORY_RECORDING_DEVICE =1,
  TDASRERROR_CATEGORY_CONFIG = 2,
  TDASRERROR_CATEGORY_NETWORK =3,
};
/**
 *@brief 得到ASR 错误类型相关的 类型
 *@param 
 *@return   
 */
int GetAsrErrorTypeCategory(const int iErrorType);

/**
 *@brief 得到ASR 错误类型的说明
 *@param 
 *@return   
 */
//std::string GetAsrErrorTypeString(const int iErrorType);

struct NLPCachingData {
  int voice_type;
  std::string need_nlp_string;
  std::string nlp_result_string;
  bool permanent_preservation;  ///< 是否是永久保存，如果是，则不删除

  NLPCachingData() {
    voice_type =0;
    permanent_preservation = false;
  }
};
typedef boost::shared_ptr<NLPCachingData> NLPCachingDataPtr;



class ScenesImp1: public virtual ScenesInterface {
 public:
  ScenesImp1(void);
  virtual ~ScenesImp1(void);

  ///inherirt from VoiceInterface
  //virtual int ReadConfig(const char* pFilePath);

  /// inherit from ScenesInterface
  virtual int CheckDevice(const int iCheckItem, int& iErrorCode, std::string* pstrCheckErrorResult);
  virtual int WriteCheckDeviceStatusToFile(const int iDeviceStatus);
  virtual std::string ExeScenceFun(const int iFunid, std::string strParam) {return "";}


  /**
  *@brief 缓存NLP结果数据，为了加速NLP结果, all nlpreuslt will be caching in caching list except following examples:
  *       1. strTextNeedNlp is empty,
  *       2. strTextNeedNlp equal to string "error happens"
  *       if caching data have reached the defined maximum size, it will remove some have caching data.
  *       default caching data size is up to 10 records
  *@param iVoiceServiceProvider[Input] voice Service Provider
  *@param strTextNeedNlp[Input] text need send to nlp
  *@Param strNlpResult[Input] nlp result.
  *@param permanent_caching[Input] 是否永久保存
  *@return success caching return YSOS_ERROR_SUCCESS
  *        fail to cachins return YSOS_ERROR_FAIL
  */
  virtual int CachingNlpData(const int iVoiceServiceProvider, const std::string strTextNeedNlp, const std::string strNlpResult, const bool permanent_caching = false);


  /**
   *@brief 获取缓存的nlp结果数据
   *@param iVoiceServiceProvider[Input] voice Service Provider
   *@param strTextNeedNlp[Input] text need send to nlp
   *@Param strNlpResult[Output] receive nlp result.
   *@return success get caching data return YSOS_ERROR_SUCCESS
   *        fail to get caching data return YSOS_ERROR_FAIL
   */
  virtual int GetCachingNlpData(const int iVoiceServiceProvider, const std::string strTextNeedNlp, std::string& strNlpResult);

  //virtual int SetInvokeWindow(HWND hWnd);

  //virtual int IsNlpDataHaveResult(const boost::property_tree::ptree& json_result, bool& is_have_nlp_result) {assert(false); return YSOS_ERROR_FAILED;}
  virtual int IsNlpDataHaveResult(const Json::Value& json_result, bool& is_have_nlp_result) {assert(false); return YSOS_ERROR_FAILED;}

  //HWND hwnd_msg_invoke_;                                ///< window handle for invoke message
 protected:
  boost::mutex list_nlp_caching_data_mutex_;
  std::list<NLPCachingDataPtr> list_nlp_caching_data_temp_;  ///< 临时保存数据的列表，使用此列表前，得使用 list_nlp_caching_data_mutex 进行锁定
  std::list<NLPCachingDataPtr> list_nlp_caching_data_permanent_;  ///< 永久保存的列表，使用此列表前，得使用 list_nlp_caching_data_mutex 进行锁定

  int config_nlp_result_max_caching_records_;   ///< caching nlp records maxminum. default is 10;
  std::string device_status_file_full_path_;  ///< 设备状态的文件目录

  

private:
 
};

