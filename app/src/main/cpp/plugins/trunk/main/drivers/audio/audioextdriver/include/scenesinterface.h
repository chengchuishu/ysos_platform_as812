/**   //NOLINT
  *@file scenesinterface.h
  *@brief Definition of 场景
  *@version 0.1
  *@author wanfenfen
  *@date Created on: 2016:8:14   0:24
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  */
#pragma once
#pragma warning (disable:4250)

#ifdef _WIN32
#include <Windows.h>
#else

#endif
/// stl headers
#include <vector>

/// boost headers
#include <boost/property_tree/ptree.hpp>


/// ysos headers
#include "../../../../../../core/trunk/main/public/include/core_help_package/jsonutility.h"

/// self headers
#include "../include/voiceinterface.h"



/**
  * enum of mic array device error code.
  * 此错误码与数据库中的数据码同步
  * 麦克风阵列	状态：0正常，1异常，4串口不对，5无响应
  */
enum MICArrayCheckDeviceErrorCode {
  MIC_ERROR_SUCC    =0,  ///< 正常
  MIC_ERROR_ERROR   =1,  ///< 异常
  MIC_ERROR_SERIAL_ERROR    =4,  ///< 串口不对
  MIC_ERROR_SERIAL_NO_RESPONSE    =5,   ///< 无响应
};


class ScenesInterface: public virtual VoiceInterface {
 public:
   ScenesInterface(void){};
   virtual ~ScenesInterface(void){};


   virtual int DomSomethingBeforeAsr() =0;
  /**
   *@brief 相比 VoiceInterface 接口多了一个参数 iOwner
   *@param lFlag[Input]  begin a asr session in which way
   *@param iOwnerid[Input] begin a asr session's owner id
   *@param ReceiveSeesionID[Output] receive a asr sessionid if begin asr session successful
   *@return success rturn YSOS_ERROR_SUCCESS
   *        fail return YSOS_ERROR_FAIL
   */
  virtual int ASRSessionBegin2(const int lFlag, const int iOwnerid, std::string& ReceiveSeesionID) =0;

    /**
   *@brief NLP process
   *@param pTextNeedNlp[Input] text need to nlp process
   *@param filter_null_result[Input] 是否过滤无效结果，如果是，则如果nlp无效，则返回空字符串
   *@param ReceiveNLPResult[Output] receive nlp result
   *@return success rturn YSOS_ERROR_SUCCESS
   *        fail return YSOS_ERROR_FAIL
   */
  virtual int NLP2(const char* pTextNeedNlp, bool filter_null_result,std::string& ReceiveNLPResult) =0;

  /**
   *@brief 模块的优化工作
   *@param iParam[Input] 阶段：1 加载OCX最后时，提前登录第三方
   *@return
   */
  virtual void Scenes0ptimization(int iParam) =0;

  /**
  *@brief get specific voice server provider id
  *@param strType[Input] service type, can be one of "asr","nlp","tts"
  *@return  current voice provider id
  */
  virtual int GetVoiceProviderID(const std::string strType) =0;

  /**
   *@brief NLP process
   *@param iVoiceServiceProvider[Input] 第三方语音服务提供商
   *@param pTextNeedNlp[Input] text need to nlp process
   *@param ReceiveNLPResult[Output] receive nlp result
   *@return success return YSOS_ERROR_SUCCESS
   *        fail return YSOS_ERROR_FAIL
   */
  //virtual int NLP2(int iVoiceServiceProvider, const char* pTextNeedNlp, std::string& ReceiveNLPResult) =0;

  /**
   *@brief save windows hwnd that will be used to alert message
   *@param hWnd[Input] windows hwnd
   *@return success return YSOS_ERROR_SUCCESS
   *        fail return YSOS_ERROR_FAIL
   */
  //virtual int SetInvokeWindow(HWND hWnd) =0;

    /**
   *@brief 检测设备
   *@param iCheckItem[Input] 检测设备项
   *                         0 默认录音设备串口通信是否正常，操作：连接串口，发送一条串口命令
   *@param iErrorCode[Output] 出错码
   *@param pstrCheckErrorResult[Output] 出错信息
   *@return success get caching data return YSOS_ERROR_SUCCESS
   *        fail to get caching data return YSOS_ERROR_FAIL
   */
  virtual int CheckDevice(const int iCheckItem, int& iErrorCode, std::string* pstrCheckErrorResult) =0;

  /**
   *@brief 把当前设备检测的结果保存到文件
   *@param iDeviceStatus[Input] 当前设备的状态
   *                            状态：0正常，1异常，4串口不对，5无响应
   *@return success get caching data return YSOS_ERROR_SUCCESS
   *        fail to get caching data return YSOS_ERROR_FAIL
   */
  virtual int WriteCheckDeviceStatusToFile(const int iDeviceStatus) =0;

  /**
   *@brief 中断场景，场景从头开始
   *@param strQuery[Input] 中断场景
   *@param strInterruptResult[Input] 中断后的结果
   *@return success return YSOS_ERROR_SUCCESS
   *        fail return YSOS_ERROR_FAIL
   */
  //virtual int LingYunQuery(const std::string strQuery, std::string& strInterruptResult) =0;

  /**
   *@brief  执行场景中的函数
   *@param iFunid[Input] 函数id号
   *@param strParam[Input] 函数需要的参数
   *@return  函数执行结果
   */
  virtual std::string ExeScenceFun(const int iFunid, std::string strParam) =0;

  /**
   *@brief asr后使用哪家的nlp作为asr的二次结果。如果是多家，则使用｜分开, 如果有值，则表示asr后获取二次结果
   *@param 
   *@return   
   */
  virtual int GetNlpProvidersAfterAsr(std::vector<int>& vec_nlpproviders) =0;
  
  /**
   *@brief nlp服务默认同时使用哪几家.如果是多家，则使用｜分开
   *@param 
   *@return   
   */
  virtual int GetNlpDefaultProviders(std::vector<int>& vec_nlpproviders) =0;


  /**
   *@brief 当前nlp是否是有结果的结构
   *@param 
   *@return 
   */
  //virtual int IsNlpDataHaveResult(const boost::property_tree::ptree& json_result, bool& is_have_nlp_result) =0;
  virtual int IsNlpDataHaveResult(const Json::Value& json_result, bool& is_have_nlp_result) =0;

};



typedef boost::shared_ptr<ScenesInterface> ScenesInterfacePtr;