//#include "StdAfx.h"
#include "../include/scenesimp1.h"

#include <boost/thread/lock_guard.hpp>
#include <boost/thread/mutex.hpp>
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"


std::string GetVoiceTypeStr(unsigned int Type) {
  switch (Type) {
  case VoiceType_IFLY:
    return "科大迅飞";
  case VoiceType_JTHC:
    return "捷通华声";
  case 2:
    return "捷通客服系统";
  case 3:
    return "小问";
  case VoiceType_MICROSOFT:
    return "微软";
  case VoiceType_IFLY_PRIVATE:
    return "迅飞私有云";
  case VoiceType_TULING:
    return "图灵";
  case VoiceType_RGZX:
    return "人工座席";
  default:
    return "未知";
  }
}

std::string GetScenesTypeStr(unsigned int Type) {
  switch (Type) {
  case ScenesType_IFLY:
    return "科大迅飞场景";
  case ScenesType_JHTC:
    return "捷通华声场景";
  default:
    return "未知";
  }
}
int GetAsrErrorTypeCategory(const int iErrorType) {
  switch (iErrorType) {
  case TDASRERROR_SUCCESS:  ///< 0
    return TDASRERROR_CATEGORY_SUCCESS;
  case TDASRERROR_TIMEOUT:  ///< -1
    return TDASRERROR_CATEGORY_NETWORK;
  case TDASRERROR_OPEN_RECORDING_DEVICE_FAIL:  ///< -2
    return TDASRERROR_CATEGORY_RECORDING_DEVICE;
  case TDASRERROR_START_RECORDING_FAIL:  ///< -3
    return TDASRERROR_CATEGORY_RECORDING_DEVICE;
  case TDASRERROR_NO_RECORDING_DATA:  ///< -4
    return TDASRERROR_CATEGORY_RECORDING_DEVICE;
  case TDASRERROR_LOGIN_THIRD_VOICE_PROVIDER_FAIL:  ///< -11
    return TDASRERROR_CATEGORY_NETWORK;
  case TDASRERROR_SEND_VOICE_DATA_FAIL:  ///< -12
    return TDASRERROR_CATEGORY_NETWORK;
  case TDASRERROR_GET_ASR_RESULT_FAIL:  ///< -13
    return TDASRERROR_CATEGORY_NETWORK;
  case TDASRERROR_BUILD_ASR_CONNECTION_FAIL:  ///< -14
    return TDASRERROR_CATEGORY_NETWORK;
  case TDASRERROR_NO_VOICE_PROVIDER:  ///< -15
    return TDASRERROR_CATEGORY_RECORDING_DEVICE;
  case TDASRERROR_NLP_FAIL:  ///< -21
    return TDASRERROR_CATEGORY_NETWORK;
  default:
    return TDASRERROR_CATEGORY_NOTKNOWN;
  }
}



ScenesImp1::ScenesImp1(void) {
  config_nlp_result_max_caching_records_ =0;
}


ScenesImp1::~ScenesImp1(void) {

  /// 清空缓存的数据
  {
    boost::lock_guard<boost::mutex> lock_guard_caching_nlp_data(list_nlp_caching_data_mutex_);
    list_nlp_caching_data_temp_.clear();
    list_nlp_caching_data_permanent_.clear();
  }
}

int ScenesImp1::CheckDevice(const int iCheckItem, int& iErrorCode, std::string* pstrCheckErrorResult) {
  assert(false);
  iErrorCode =1;  ///< 错误码：异常
  if (NULL != pstrCheckErrorResult)
    *pstrCheckErrorResult = "当前场景不支持此操作";
  return YSOS_ERROR_FAILED;
}

int ScenesImp1::WriteCheckDeviceStatusToFile(const int iDeviceStatus) {
  assert(device_status_file_full_path_.length() >0);
  if (device_status_file_full_path_.length() ==0) {
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "没有配置当前设备状态的文件地址");
    return YSOS_ERROR_FAILED;
  }

  char status_buf[1024] = {'\0'};
  //sprintf_s(status_buf, sizeof(status_buf), "%d", iDeviceStatus);
  sprintf(status_buf, "%d", iDeviceStatus);
  return YSOS_ERROR_FAILED;
}


int ScenesImp1::CachingNlpData(const int iVoiceServiceProvider, const std::string strTextNeedNlp, const std::string strNlpResult, const bool permanent_caching) {
  if (strTextNeedNlp.length() <=0)
    return YSOS_ERROR_FAILED;
  if ("error happens" == strTextNeedNlp) {
    return YSOS_ERROR_FAILED;
  }
  /*if (VoiceType_IFLY != iVoiceServiceProvider) { ///< 暂时只缓存科大迅飞的，其他的不缓存
    return YSOS_ERROR_FAILED;
  }*/
  {
    boost::lock_guard<boost::mutex> lock_guard_caching_nlp_data(list_nlp_caching_data_mutex_);
    NLPCachingDataPtr caching_data_ptr(new NLPCachingData);
    caching_data_ptr->voice_type = iVoiceServiceProvider;
    caching_data_ptr->need_nlp_string = strTextNeedNlp;
    caching_data_ptr->nlp_result_string = strNlpResult;
    caching_data_ptr->permanent_preservation = permanent_caching;
    if (permanent_caching) {
      list_nlp_caching_data_permanent_.push_back(caching_data_ptr);  ///< 放入永久表中
    } else {
      list_nlp_caching_data_temp_.push_back(caching_data_ptr);  ///< 放入临时表中

      if (list_nlp_caching_data_temp_.size() > config_nlp_result_max_caching_records_) {
        list_nlp_caching_data_temp_.pop_front();  ///< 只保存最近10份的数据
      }
    }

    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "缓存数据,服务商:" << iVoiceServiceProvider << ", asr:" << strTextNeedNlp.c_str());
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "缓存数据,服务商:" << iVoiceServiceProvider <<", asr:" << strTextNeedNlp.c_str() <<", nlp:" <<  strNlpResult.c_str());

  }
  return YSOS_ERROR_SUCCESS;
}


int ScenesImp1::GetCachingNlpData(const int iVoiceServiceProvider, const std::string strTextNeedNlp, std::string& strNlpResult) {
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),   "in GetCachingNlpData, voiceprovider:" << iVoiceServiceProvider <<" strtextneednlp:" << strTextNeedNlp.c_str());
  if (strTextNeedNlp.length() <=0)
    return YSOS_ERROR_FAILED;
  /*if (VoiceType_IFLY != iVoiceServiceProvider) { ///< 暂时只缓存科大迅飞的，其他的不缓存
    return YSOS_ERROR_FAILED;
  }*/
  std::string* pstr_result = NULL;

  /// 如果1秒内没从缓存中拿到结果，再去重新获取一次
  //int current_times =0;
  //while(current_times <=4) {  /// 最多获取3次，每次间隔500毫秒
  //  ++ current_times;
  {
    boost::lock_guard<boost::mutex> lock_guad_caching_data(list_nlp_caching_data_mutex_);
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "caching records:" << list_nlp_caching_data_temp_.size());


    /// 先临时表中获取 从最新的一份获取
    std::list<NLPCachingDataPtr>::reverse_iterator rit = list_nlp_caching_data_temp_.rbegin();
    for (rit; rit != list_nlp_caching_data_temp_.rend(); ++rit) {
      NLPCachingDataPtr& nlp_ptr = (*rit);
      if (nlp_ptr->need_nlp_string == strTextNeedNlp
          && nlp_ptr->voice_type == iVoiceServiceProvider) {
        /// 找到了最近的一条缓冲数据
        pstr_result = & nlp_ptr->nlp_result_string;
        break;  ///< 不再查找其他结果
      }
    }

    /// 如果临时表没有数据，则从永久表中获取数据
    if (NULL == pstr_result) {
      std::list<NLPCachingDataPtr>::reverse_iterator rit = list_nlp_caching_data_permanent_.rbegin();
      for (rit; rit != list_nlp_caching_data_permanent_.rend(); ++rit) {
        NLPCachingDataPtr& nlp_ptr = (*rit);
        if (nlp_ptr->need_nlp_string == strTextNeedNlp
            && nlp_ptr->voice_type == iVoiceServiceProvider) {
          /// 找到了最近的一条缓冲数据
          pstr_result = & nlp_ptr->nlp_result_string;
          break;  ///< 不再查找其他结果
        }
      }
    }

    if (NULL != pstr_result) {
      strNlpResult = *pstr_result;
      YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),   "使用 缓存数据");
      YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),   "缓存数据 获取一条数据,服务提供商：" << iVoiceServiceProvider <<", 缓存数据：" << pstr_result->c_str());
      return YSOS_ERROR_SUCCESS;
    } else {
      YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),   "缓存数据 未 获取一条数据,服务提供商：" << iVoiceServiceProvider <<"， seleep 500 miliseconds");
      return YSOS_ERROR_FAILED;
    }
  }
  return YSOS_ERROR_FAILED;
}

/*
int ScenesImp1::SetInvokeWindow(HWND hWnd) {
  hwnd_msg_invoke_ = hWnd;
  return YSOS_ERROR_SUCCESS;
}*/