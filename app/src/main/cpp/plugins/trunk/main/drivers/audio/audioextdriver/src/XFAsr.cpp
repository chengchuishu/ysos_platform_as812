/**   //NOLINT
  *@file XFAsr.h
  *@brief Definition of 迅飞语音接口模块, 此模块代码为原苏海鹏词音模块提取出来。
  *@version 0.1
  *@author 作者1：苏海鹏，作者2：wanfenfen
  *@date Created on: 2016:6:11   19:58
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  */


/// XFAsr.cpp : CXFAsr 的实现

/// self headers
/// #include "stdafx.h"
#include "../include/XFAsr.h"  //NOLINT

/// windows headers
#include <time.h>  //NOLINT
#include <math.h>

#include <stdexcept>
#include <assert.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <comutil.h>
#include <stdio.h>
#else
#define MAX_PATH 260
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#endif

using namespace std;

/// stl headers
#include <string>  //NOLINT
#include <vector>  //NOLINT

#include <boost/thread/mutex.hpp>  //NOLINT
#include <boost/thread/lock_guard.hpp>  //NOLINT
#include <boost/locale/conversion.hpp>
#include <boost/locale.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/atomic.hpp>

/// ysos headers
#include "../../../../../../core/trunk/main/public/include/core_help_package/jsonutility.h"

#include "../include/counterdeposit.h"

#pragma disable(warning:4018)



#ifdef _WIN32
#else

int GetTickCount()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    unsigned long timptt = (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
    return (int)timptt;
}

#endif

/*#pragma disable(warning:4482)*/
/**
  *@brief define of context, for save data
  */
class XFContext {
 public:
  char tts_sessionid_[1024];
  int tts_starttts_ownerid_;

  XFContext() { ///< 设置默认值
    memset(tts_sessionid_, '\0', sizeof(tts_sessionid_));
    tts_starttts_ownerid_ =0;
  }
};

/**
  *@brief define of context manager
  */
class XFContextManager {
 public:
  static XFContext GetXFContext() {
    XFContext temp_xfcontext;
    {
      boost::lock_guard<boost::mutex> lock_gurad(mutext_xfcontext_);
      temp_xfcontext = xfcontext_;
    }
    return temp_xfcontext;
  }

  static void Settts_sessionid(const char* pSessionID) {
    boost::lock_guard<boost::mutex> lock_gurad(mutext_xfcontext_);
    if (NULL == pSessionID)
      memset(xfcontext_.tts_sessionid_, '\0', sizeof(xfcontext_.tts_sessionid_));
    else
      //strncpy_s(xfcontext_.tts_sessionid_, sizeof(xfcontext_.tts_sessionid_) ,pSessionID, sizeof(xfcontext_.tts_sessionid_)-1);
      strncpy(xfcontext_.tts_sessionid_, pSessionID, sizeof(xfcontext_.tts_sessionid_)-1);
  }
  static void Settts_starttts_ownerid(const int iStartTTSOwnerid) {
    boost::lock_guard<boost::mutex> lock_gurad(mutext_xfcontext_);
    xfcontext_.tts_starttts_ownerid_ = iStartTTSOwnerid;
  }


  /**
   *@brief 初始化上下文
   *@return
   */
  //static void InitXFContextAsr() {
  //  boost::lock_guard<boost::mutex> lock_gurad(mutext_xfcontext_);
  //  memset(xfcontext_.asr_sessionid_, '\0', sizeof(xfcontext_.asr_sessionid_));
  //  xfcontext_.asr_audio_state_ = TDMSP_AUDIO_SAMPLE_FIRST;
  //  xfcontext_.asr_first_asr_getrsult_clock_count_ =0;
  //  xfcontext_.asr_startasr_ownerid =0;
  //  //warning ,not init asr_startasr_flag_ here.
  //}

 private:
  static XFContext xfcontext_;
  static boost::mutex mutext_xfcontext_;
};
XFContext XFContextManager::xfcontext_;
boost::mutex XFContextManager::mutext_xfcontext_;


bool ASRFlagNeedNlp(int iflag) {
  if (0 == iflag)
    return true;
  else
    return false;
}
/**
  *@brief statistics module, statistics time consuming of asr
  */
/////////////////////////////////////////////// ///////////////////////////
int ThreadFunProcNLP(ScenesInterfacePtr sence_ptr, int iVoiceServiceProvider, bool bSynchronize,SHORT iOwner,TDCharBufPtr TextNeedNlpPtr,int ResultMaxBufferSize, TDCharBufPtr ResultBufPtr);

boost::atomic<int> CXFAsr::construct_counter_atomic_;  ///<  构建对象的计数器
boost::atomic<int> CXFAsr::release_counter_atomic_;   ///< 析构对象的计数器
std::string CXFAsr::CreateVoiceFileName() {
  //std::string strTime = boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time());
  //strTime.replace(strTime.find('T'), 1, "_");
  //std::string sRet = config_save_vocie_filepath_ + "TDVoice_" + strTime + ".pcm";

  ///;保存到文件”wav/yyyymmdd/时间戳.pcm”,默认是"./wav"
  std::string strTime = boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time());
  int pos = strTime.find('T');
  log_date_ = strTime.substr(0,pos);
  log_time_ = strTime.substr(pos+1,strTime.length());

  strTime.replace(strTime.find('T'), 1, "");

  // 创建目录
  std::string voicepath = config_save_vocie_filepath_ + "/" + log_date_ + "/";
  if (!boost::filesystem::exists(voicepath)) {
    boost::system::error_code myecc;
    boost::filesystem::create_directories(voicepath, myecc);
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "create voice file path : "<<voicepath);
  }

  voice_log_filename_ = term_id_ + "_" + strTime + ".pcm";
  std::string sRet = voicepath + voice_log_filename_;

  return sRet;
}

std::string CXFAsr::CreateLocalVoiceFileName() {
  std::string strTime = boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time());
  strTime.replace(strTime.find('T'), 1, "_");
  std::string sRet = config_save_vocie_filepath_ + "TD_LocalVoice_" + strTime + ".pcm";
  return sRet;
}

void CXFAsr::AudioCaptureCallBackData(char* pData, UINT32 uDataLength) {
  if (0 == uDataLength)
    return;

  ///< 把录音数据保存起来
  TDVoiceDataPtr voice_data_ptr(new TDVoiceData);
  int save_ret = voice_data_ptr->SaveData(pData, uDataLength);
  if (YSOS_ERROR_SUCCESS != save_ret)
    return;

  {
    boost::lock_guard<boost::mutex> lock_gurad_list_voice_data(mutex_list_asr_voice_data_);
    ///<  按录音机录音设置，1秒会有 10个数据块，1分钟 375个数据块
    while (list_voice_data_.size() >= 60) {
      YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "in AudioCaptureCallBackData,录音数据保存超过了 6秒，部分数据会被清空, 以便及时响应");
      //list_voice_data_.clear();
      list_voice_data_.pop_front();
    }
    list_voice_data_.push_back(voice_data_ptr);
    return;
  }
}


void AudioPlayCallBackDone() {
  XFContext xfcontext = XFContextManager::GetXFContext();
}

int CXFAsr::TDStartASR(SHORT iOwner, SHORT iFlag) {

  YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "in StartASREx,ownerid: " << iOwner << ",flag: "<< iFlag);

  ///< 如果当前是打断模式，同时已经在录音识别操作，则不再响应此处理
  if (call_times_max_startasr_atomic_ > call_times_max_stopasr_atomic_) {
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "当前正在录音，所以不再响应此请求startasr");
    return YSOS_ERROR_SUCCESS;
  }

  int icalltimes = ++call_times_max_startasr_atomic_;
  if (call_times_max_stopasr_atomic_ < call_times_max_startasr_atomic_.load() -1) { ///< 结束以前所有startasr导致的语音识别，比如多次点击startasr
    call_times_max_stopasr_atomic_ = call_times_max_startasr_atomic_.load() -1;
  }
  return MsgProcStartASR(icalltimes, iOwner, iFlag, NULL);
}


int CXFAsr::TDStopASR(SHORT iStopFlag) {
  if (0 == iStopFlag && 1 == config_voice_record_mode_) {
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "当前是 语音打断模式，普通打断flag4打断 不了。");
    return /*S_FALSE*/1;
  }

  call_times_max_stopasr_atomic_ = call_times_max_startasr_atomic_.load();
  return MsgProcStopASR();
}

int CXFAsr::TDNLP(SHORT iOwner, /*BSTR strTextNeedNlp*/const std::string strTextNeedNlp, SHORT iSynchronize, /*BSTR* pResult*/std::string* pResult) {
  ScenesInterfacePtr sence_ptr =  GetCurrentScenesInterfacePtr();
  if (NULL == sence_ptr) {
    return /*S_FALSE*/1;
  }
  if (YSOS_ERROR_FAILED == ProcNLP(/*sence_ptr->GetVoiceProviderID("nlp")*/ -1, iOwner, strTextNeedNlp, iSynchronize, pResult)) {
    return /*S_FALSE*/1;
  }
  return YSOS_ERROR_SUCCESS;

}

int CXFAsr::TDNLPEx(SHORT iVoiceServiceProvider, SHORT iOwner, std::string strTextNeedNlp, SHORT iSynchronize, std::string* pResult) {
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "in TDNLPEx");
  int iret = ProcNLP(iVoiceServiceProvider, iOwner, strTextNeedNlp, iSynchronize, pResult);
  if (YSOS_ERROR_SUCCESS == iret)
    return YSOS_ERROR_SUCCESS;
  else
    return /*S_FALSE*/1;
}

int CXFAsr::TDStartTTS(SHORT iOwnerid, std::string strText) {
  return MsgProcStartTTS(iOwnerid, /*strtext_tts*/strText);
}


int CXFAsr::TDStopTTS(void) {
  return MsgProcStopTTS();
}

int CXFAsr::TDOpenAudioCaptureDevice(void) {
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "in OpenDevice");
  return YSOS_ERROR_SUCCESS;
}

int CXFAsr::TDCloseAudioCaptureDevice(void) {
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "in TDCloseAudioCaptureDevice");
  return YSOS_ERROR_SUCCESS;
}

int CXFAsr::ProcNLP(int iVoiceServiceProvider, SHORT iOwner, std::string strTextNeedNlp, SHORT iSynchronize,std::string* pResult) {
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "in ProcNLP,voiceserviceprovider: " << iVoiceServiceProvider << ",ownerid: " << iOwner << ",iSynchronize:" << iSynchronize);

  int itext_need_nlp_size = strTextNeedNlp.length() +1; // strlen(ptext_need_nlp)+1;
  TDCharBufPtr text_nned_nlp_buf_ptr(new char[itext_need_nlp_size]);
  //strncpy_s(text_nned_nlp_buf_ptr.get(),itext_need_nlp_size,/*ptext_need_nlp*/strTextNeedNlp.c_str(),itext_need_nlp_size-1);
  strncpy(text_nned_nlp_buf_ptr.get(),/*ptext_need_nlp*/strTextNeedNlp.c_str(),itext_need_nlp_size-1);
  bool bissynchronize = (iSynchronize>0)?true:false;

  /// 使用一个单独的线程来操作
  int iresult_max_bufsize = 8*1024;
  TDCharBufPtr resultptr(new char[iresult_max_bufsize]);
  memset(resultptr.get(), '\0',iresult_max_bufsize);

  ScenesInterfacePtr sence_ptr =GetCurrentScenesInterfacePtr();
  if (NULL == sence_ptr) {
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "in ProcNLP ScenesInterfacePtr is not avaliable");
    return YSOS_ERROR_FAILED;
  }

  boost::thread* ptnlp = thread_group_all_.create_thread(boost::bind(ThreadFunProcNLP, sence_ptr, iVoiceServiceProvider,
                         bissynchronize,iOwner,
                         text_nned_nlp_buf_ptr, iresult_max_bufsize, resultptr));

  if (true == bissynchronize) { ///< 同步操作
    assert(true == ptnlp->joinable());
    bool isfinish = ptnlp->try_join_for(boost::chrono::milliseconds(config_nlp_timeout_millseconds_atomic_));  ///3秒后返回
    if (false == isfinish) { /// 线程体没有执行完
      //中断线程
      ptnlp->interrupt();  ///< 线程体没有执行完，则中断线程体

      YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "nlp timeout");
      return /*S_FALSE*/1;
    }
    assert(NULL != resultptr);
    assert(pResult);
    if (pResult)
      *pResult = resultptr.get();
    return YSOS_ERROR_SUCCESS;
  } else {
    //tnlp.detach();  /// 让线程自生自灭了
    return YSOS_ERROR_SUCCESS;
  }
}

/**
 *@brief 处理nlp请求
 *       应用场景：普通的NLP 调用操作。
 *@param pXFAsr[input]
 *@param iVoiceServiceProvider[Input] 第三方语音服务提供商
 *@param bSynchronize[Input] 是同步操作还是异步操作
 *@param TextNeedNlpPtr[Input] 需要进行Nlp解析的地址的指针
 *@param ResultMaxBufferSize[Input] 保存结果缓冲区的最大可用字节数
 *@param ResultBufPtr[Input] 保存结果的地址
 *@return success rturn YSOS_ERROR_SUCCESS
 *        fail return YSOS_ERROR_FAIL
 */
int ThreadFunProcNLP(ScenesInterfacePtr sence_ptr, int iVoiceServiceProvider, bool bSynchronize,SHORT iOwner,TDCharBufPtr TextNeedNlpPtr,int ResultMaxBufferSize, TDCharBufPtr ResultBufPtr) {
  assert(NULL != sence_ptr);
  assert(NULL != TextNeedNlpPtr);
  assert(NULL != ResultBufPtr);
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "in ThreadFunProcNLP, iVoiceServiceProvider: " << iVoiceServiceProvider);

  assert(NULL !=TextNeedNlpPtr);
  if (NULL == TextNeedNlpPtr ||NULL == ResultBufPtr || strlen(TextNeedNlpPtr.get())==0)
    return YSOS_ERROR_FAILED;


  std::string strnlp_result;
  {
    boost::this_thread::interruption_point();/// 设置中断点，用于网络超时时能及时中断
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "MSPSearch text need nlp: " << TextNeedNlpPtr.get());
    long before_call_nlp_clock_count = clock();
    int res = sence_ptr->NLP(TextNeedNlpPtr.get(), strnlp_result);
    //boost::this_thread::interruption_point();  ///< 这里不中断，后面要缓存数据后再中断
    if (YSOS_ERROR_SUCCESS != res) {
      char tempbuf[1024] = {'\0'};
#ifdef _WIN32      
      sprintf_s(tempbuf, sizeof(tempbuf),"{\"text\":\"error happens\",\"rc\":\"-21\",\"errordetail\":\"Nlp解析出错,iflyreturn:%d\"}", res);
#else
      sprintf(tempbuf,"{\"text\":\"error happens\",\"rc\":\"-21\",\"errordetail\":\"Nlp解析出错,iflyreturn:%d\"}", res);
#endif
      strnlp_result = tempbuf;
    }
  }

  boost::this_thread::interruption_point();
  if (false == bSynchronize) { ///<如果是异步操作，则向上层抛出事件
    assert(false);
    return YSOS_ERROR_SUCCESS;
  } else {
    //strncpy_s(ResultBufPtr.get(), ResultMaxBufferSize, strnlp_result.c_str(), ResultMaxBufferSize-1);
    strncpy(ResultBufPtr.get(), strnlp_result.c_str(), ResultMaxBufferSize-1);
  }
  return YSOS_ERROR_SUCCESS;
}

int CXFAsr::TDOpenPlayerDevice(void) {
  return YSOS_ERROR_SUCCESS;
}

int CXFAsr::TDUseScenes(SHORT iScenesID) {

  SetCurrentScenesType(iScenesID);

  return YSOS_ERROR_SUCCESS;
}

/**
 *@brief 获取实际的Buffer和最大长度  // NOLINT
 *       继续说明                 // NOLINT
 *@param iSupportAsrConcurrency[Input] 是否支持ASR并发，如果支持，则会创建多路sessionid(每次检测到一个语音尾端点后就开启一路sessionid)
 *                                     如果不支持，则只创建一个sessionid(每次检测到一个语音尾端点后，等拿到结果后再开启一路新的sessionid）
 *@param iInterruptionMode[Input] 是否是打断模式，如果是，则如果当前没有语音识别结果，则不往上抛出,每次录音前不会清空以前的音频数据（会使用以前保留的音频数据)
 *                                                如果不是，则如果当前没有语音识别结果，则也往上抛出空结果，每次录音前会清空以前的音频数据
 *@return
 */
int CXFAsr::ThreadStartAsrRecordingChild(ScenesInterfacePtr scenceptr, const int iInterruptionMode, const int iCurrentStartAsrCount, const int iOwnerid,const int lFlag) {

  assert(iCurrentStartAsrCount >0 && scenceptr);

  assert(scenceptr);
  if (NULL == scenceptr)
    return YSOS_ERROR_FAILED;


  int error_code = TDASRERROR_OPEN_RECORDING_DEVICE_FAIL;
  std::string error_msg = "ASR服务已关闭";
  std::string asr_result_all = "";
  std::string strsession_id = "";
  clock_t asr_first_asr_getrsult_clock_count_;
  std::string strsavefile_voice_recorder;


  if (iCurrentStartAsrCount <= call_times_max_stopasr_atomic_) {
    //goto FB_ASR_RECORDING_INTERRUPTION_RETURN;
    return FB_ASR_RECORDING_INTERRUPTION_RETURN(scenceptr, strsession_id, strsavefile_voice_recorder);
  }

  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "begin get asr");
  ///< 创建连接的session
  char session_begin_params[1024]= {'\0'};
  int isesson_begin_reult = scenceptr->ASRSessionBegin2(lFlag, iOwnerid, strsession_id);
  if (YSOS_ERROR_SUCCESS != isesson_begin_reult) {
    error_code = /*-14*/TDASRERROR_BUILD_ASR_CONNECTION_FAIL;
    error_msg = "建立语音识别连接失败";
    AddErrorResult(scenceptr, iOwnerid, lFlag, error_code, strsession_id, error_msg, strsavefile_voice_recorder);
    return YSOS_ERROR_FAILED;
  }

  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "开始尾端点检测");
  ///< 从录音数据队列中获取数据.检测尾端点
  int asr_audio_state_ = TDMSP_AUDIO_SAMPLE_FIRST;
  int voice_count = 0;
  int ep_stat =0;
  int rec_stat = TDMSP_REC_STATUS_SUCCESS;
  while (true) {
    voice_count++;
    if (iCurrentStartAsrCount <= call_times_max_stopasr_atomic_) { ///< stop asr 中断返回
      //goto FB_ASR_RECORDING_INTERRUPTION_RETURN;
      return FB_ASR_RECORDING_INTERRUPTION_RETURN(scenceptr, strsession_id, strsavefile_voice_recorder);
    }

    TDVoiceDataPtr voice_data_ptr = NULL;
    {
      for (;;) {  ///< 这里是一直循环
        if (iCurrentStartAsrCount <= call_times_max_stopasr_atomic_) { ///< stop asr 中断返回
          //goto FB_ASR_RECORDING_INTERRUPTION_RETURN;
          return FB_ASR_RECORDING_INTERRUPTION_RETURN(scenceptr, strsession_id, strsavefile_voice_recorder);
        }

        {
          ///< 注意，这个括号不能去掉
          boost::lock_guard<boost::mutex> lock_guard(mutex_list_asr_voice_data_);
          if (list_voice_data_.size() > 0) {
            voice_data_ptr = *(list_voice_data_.begin());
            list_voice_data_.pop_front();
            /*if (NULL == voice_data_ptr) { ///< 空音频，结束转写标记
              flag_asr_stop_current_transcribe = 1;
              continue;  ///< 获取下一个音频
            }*/
            break;
          }
        }
        //Sleep(100);
        boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
      }
      if (NULL == voice_data_ptr) {
        error_code = TDASRERROR_NO_RECORDING_DATA;
        error_msg = "当前没有录音数据";
        AddErrorResult(scenceptr, iOwnerid, lFlag, error_code, strsession_id, error_msg, strsavefile_voice_recorder);
        return YSOS_ERROR_FAILED;
      }
    }

    if (config_need_save_voice_file_) { ///< 需要保存到文件
      if (NULL == psavefile_voice_recorder_ && term_id_!="") {
        //strsavefile_voice_recorder = (boost::filesystem::path(config_save_vocie_filepath_).append(CreateVoiceFileName())).string();
        strsavefile_voice_recorder = CreateVoiceFileName();
        YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "createfile = " << strsavefile_voice_recorder);
        psavefile_voice_recorder_ = fopen(strsavefile_voice_recorder.c_str(), "wb");
        YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "psavefile_voice_recorder_ = " << psavefile_voice_recorder_);
      }
      if (NULL != psavefile_voice_recorder_)
        fwrite(voice_data_ptr->GetData(), voice_data_ptr->GetDataLength(), 1, psavefile_voice_recorder_);
    }

    //int ep_stat;
    // int rec_stat;
    int ret = scenceptr->ASRAudioWrite(strsession_id.c_str(), reinterpret_cast<const void *>(voice_data_ptr->GetData()),
                                       voice_data_ptr->GetDataLength(), asr_audio_state_, &ep_stat, &rec_stat);
    if (YSOS_ERROR_SUCCESS != ret) {  ///< 如果给迅飞传输数据失败，则关闭录音机,上报错误
      YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "录音结束，SessionEnd ");
      scenceptr-> ASRSessionEnd(strsession_id.c_str());
      strsession_id = "";
      error_code = /*-12*/TDASRERROR_SEND_VOICE_DATA_FAIL;
      error_msg = "发送音频数据失败";
      AddErrorResult(scenceptr, iOwnerid, lFlag, error_code, strsession_id, error_msg, strsavefile_voice_recorder);
      return YSOS_ERROR_FAILED;
    }
    ///< 如果当前是第一个音频，则下一个音频即为 MSP_AUDIO_SAMPLE_CONTINUE状态
    if (TDMSP_AUDIO_SAMPLE_FIRST == asr_audio_state_) {
      asr_audio_state_ = TDMSP_AUDIO_SAMPLE_CONTINUE;
    }

    ///< 如果检测到语音的尾结点
    if (TDMSP_EP_AFTER_SPEECH == ep_stat   ///< 如果检测到语音的尾结点
        || TDMSP_REC_STATUS_COMPLETE == rec_stat
        || TDMSP_AUDIO_SAMPLE_LAST == asr_audio_state_
       ) {  ///< MSP_REC_STATUS_COMPLETE识别结束
      YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "in WaveDataProc speech tail find，ep_stat: " << ep_stat << ", rec_stat: " << rec_stat);
      //pCXFAsr->voice_data_last_tail_ = voice_data_ptr;

      //为了防止当前最后一个音频块中有部分音频，尾端点的音频会被重新加入到下一次会话识别中
      boost::lock_guard<boost::mutex> lock_guard(mutex_list_asr_voice_data_);
      list_voice_data_.push_front(voice_data_ptr);

      break;
    }

  }  ///< 结束尾端点检测
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "开始尾端点检测 end");
  int ret = scenceptr->ASRAudioWrite(strsession_id.c_str(), NULL, 0, TDMSP_AUDIO_SAMPLE_LAST, &ep_stat, &rec_stat);
  if (YSOS_ERROR_SUCCESS != ret) {
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "录音结束，SessionEnd ");
    scenceptr-> ASRSessionEnd(strsession_id.c_str());
    strsession_id = "";
    error_code = /*-12*/TDASRERROR_SEND_VOICE_DATA_FAIL;
    error_msg = "发送音频数据失败";
    AddErrorResult(scenceptr, iOwnerid, lFlag, error_code, strsession_id, error_msg, strsavefile_voice_recorder);
    return YSOS_ERROR_FAILED;
  }


  if (NULL != psavefile_voice_recorder_) {  ///< 关闭录音文件
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "psavefile_voice_recorder_ end = " << psavefile_voice_recorder_);
    fclose(psavefile_voice_recorder_);
    psavefile_voice_recorder_ = NULL;
    voice_count = 0;
  }

  ///< 获取结果
  int time_begin = GetTickCount();
  asr_first_asr_getrsult_clock_count_ = clock();
  while (true) {
    if (iCurrentStartAsrCount <= call_times_max_stopasr_atomic_) { ///< stop asr 中断返回
      //goto FB_ASR_RECORDING_INTERRUPTION_RETURN;
      return FB_ASR_RECORDING_INTERRUPTION_RETURN(scenceptr, strsession_id, strsavefile_voice_recorder);
    }

    //INT32 rec_stat;
    std::string strtemp_result;
    UINT32 timeout_count = 0;
    int iresult_ret = scenceptr->ASRGetResult(strsession_id.c_str(), &rec_stat,strtemp_result);
    ///< 如果解析出错，则直接响应，返回上层结果错误。
    if (YSOS_ERROR_SUCCESS != iresult_ret) {
      YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "获取语音识别结果失败");
      error_code = /*-13*/TDASRERROR_GET_ASR_RESULT_FAIL;
      error_msg = "获取语音识别结果失败";
      AddErrorResult(scenceptr, iOwnerid, lFlag, error_code, strsession_id, error_msg, strsavefile_voice_recorder);
      return YSOS_ERROR_FAILED;
    }

    if (strtemp_result.length()>0) {
      //TODO:note string encode
      std::string temp_res = GetUtility()->Utf8ToGbk(strtemp_result);//GBK==gbk2312
      YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "asr: " << temp_res.c_str());
      // asr_result_all += strtemp_result;  ///< ASRGetResult 返回的结果是以前获取的所有的结果的总和
      asr_result_all = strtemp_result;
    }

    ///< 获取结果结束,则响应,返回上层结果
    if (TDMSP_REC_STATUS_COMPLETE == rec_stat || TDMSP_REC_STATUS_INCOMPLETE_NEED_NLP == rec_stat) {
      int time_end = GetTickCount();
      int time_interval = time_end - time_begin;
      AddAsrResult(scenceptr, iOwnerid, lFlag, strsession_id, asr_result_all, time_interval, strsavefile_voice_recorder);
      YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "开始尾端点检测 结果end");
      return YSOS_ERROR_SUCCESS;
    } else {
      ///< 否则继续获取数据
      clock_t now_clock_count = clock();
      if ((now_clock_count - asr_first_asr_getrsult_clock_count_) > config_asr_get_result_timeout_) {
        ///< 超时,关闭session
        ++timeout_count;
        YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "超时关掉SessionEnd ");
#ifdef _DEBUGVIEW
        ::OutputDebugString("ysos.asrext 超时关掉SessionEnd");
#endif
        scenceptr->ASRSessionEnd(strsession_id.c_str());
        strsession_id = "";
        if (asr_result_all.length() > 0) {
          ///< 但有部分结果，直接返回部分结果
          YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "超时但有部分结果返回");
          int time_end = GetTickCount();
          int time_interval = time_end - time_begin;
          AddAsrResult(scenceptr, iOwnerid, lFlag, strsession_id, asr_result_all, time_interval, strsavefile_voice_recorder);
          YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "开始尾端点检测 结果end2");
          return YSOS_ERROR_SUCCESS;
        } else {
          int time_end = GetTickCount();
          int time_interval = time_end - time_begin;
          error_code = /*-13*/TDASRERROR_TIMEOUT;
          error_msg = "获取语音识别结果超时";
          /*if (timeout_count < 4)
            asr_result_all = "...";
          else {
            asr_result_all = "....";
            timeout_count = 0;
          }
          if (is_broadcast_)
            AddAsrResult(scenceptr, iOwnerid, lFlag, strsession_id, asr_result_all, time_interval, strsavefile_voice_recorder);
          else*/
            AddErrorResult(scenceptr, iOwnerid, lFlag, error_code, strsession_id, error_msg, strsavefile_voice_recorder);
          YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "开始尾端点检测 结果end3");
          return YSOS_ERROR_SUCCESS;
          //return YSOS_ERROR_FAILED;
        }
      }
    }
  }  ///< 结束结果的获取

  if (strsession_id.length() >0) {
    scenceptr->ASRSessionEnd(strsession_id.c_str());
    strsession_id = "";
  }


  return YSOS_ERROR_SUCCESS;

/*TODO: need update for linux
FB_ASR_RECORDING_INTERRUPTION_RETURN:   ///< 中断返回
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "则关闭session");
  if (strsession_id.length() >0) {
    scenceptr->ASRSessionEnd(strsession_id.c_str());
    strsession_id = "";
  }

  if (NULL != psavefile_voice_recorder_) {  ///< 关闭录音文件
    fclose(psavefile_voice_recorder_);
    psavefile_voice_recorder_ = NULL;
  }

  if (strsavefile_voice_recorder.length() >0) {
    boost::system::error_code myecc;
    boost::filesystem::remove(strsavefile_voice_recorder, myecc);
  }
  return YSOS_ERROR_FAILED;
*/
}

void CXFAsr::LogAccumulation(const std::string term_id, const std::string filepath,const std::string voicefilename,const std::string log_date, const std::string &asr_result) {
  std::string filename = term_id+"_"+log_date+"_asr.txt";
  std::string logmsg = voicefilename + "|" + "asr:"+ asr_result;
  std::string filefullname = filepath +"/"+filename;
  FILE *fp = fopen(filefullname.c_str(), "a+");
  if (fp != NULL) {
    //fprintf_s(fp, "%s\n", logmsg.c_str());
    fprintf(fp, "%s\n", logmsg.c_str());
    fclose(fp);
  }
}

void CXFAsr::AddErrorResult(ScenesInterfacePtr scenceptr, const int iOwnerid, const int lFlag, int code, const std::string strSessionid,std::string msg ,const std::string pcm_save_file_path) {

  if (strSessionid.length() > 0) {
    scenceptr->ASRSessionEnd(strSessionid.c_str());
  }


  if (NULL != psavefile_voice_recorder_) {  ///< 关闭录音文件
    fclose(psavefile_voice_recorder_);
    psavefile_voice_recorder_ = NULL;
  }

  ///<语料收集,仅在保存所有音频结果下保存错误识别
  if (voice_collection_mode_==1 && voice_log_filename_!="") {
    if (collection_voice_save_mode_ == 0 && code == TDASRERROR_GET_ASR_RESULT_FAIL || code == TDASRERROR_TIMEOUT) {
      LogAccumulation(term_id_, config_save_voicefile_asr_file_path_, voice_log_filename_, log_date_, "");
    } else {
      //// 清除本地保存的pcm文件
      if (pcm_save_file_path.length() >0) {
        boost::system::error_code myecc;
        boost::filesystem::remove(pcm_save_file_path, myecc);
      }
    }
  }

  {
    ///如果出错，则清空一下音频缓存
    ///< 注意，这个括号不能去掉
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "error 出错，则清空一下音频缓存");
    boost::lock_guard<boost::mutex> lock_guard(mutex_list_asr_voice_data_);
    list_voice_data_.clear();
  }

  static int error_time = 0;
  ++error_time;
  if (error_time < error_times_) {
    return;
  }
  error_time = 0;

  char error_msg_buf[1024] = {'\0'};
#ifdef _WIN32
  sprintf_s(error_msg_buf,sizeof(error_msg_buf),"{\"text\":\"抱歉网络不好，请再试一次\",\"rc\":\"%d\",\"rg\":\"%d\",\"errordetail\":\"%s\",\"answer_best\":\"\"}",code,code,msg.c_str());
#else
  sprintf(error_msg_buf,"{\"text\":\"抱歉网络不好，请再试一次\",\"rc\":\"%d\",\"rg\":\"%d\",\"errordetail\":\"%s\",\"answer_best\":\"\"}",code,code,msg.c_str());
#endif
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "asr error message = " << error_msg_buf);
  TDAsrResultPtr result_ptr(new TDAsrResult(iOwnerid, lFlag, error_msg_buf));
  if (NULL != result_ptr) {
    boost::lock_guard<boost::mutex> lock_guard_save_result(mutex_list_asr_result_);
    list_asr_result_.push_back(result_ptr);
  }

}

void CXFAsr::AddAsrResult(ScenesInterfacePtr scenceptr, const int iOwnerid, const int lFlag, const std::string strSessionid, std::string result, int time, const std::string pcm_save_file_path) {
  assert(scenceptr);
  if (strSessionid.length() > 0) {
    scenceptr->ASRSessionEnd(strSessionid.c_str());
  }
  if (NULL != psavefile_voice_recorder_) {  ///< 关闭录音文件
    fclose(psavefile_voice_recorder_);
    psavefile_voice_recorder_ = NULL;
  }

  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "get asr result = " << result);
  //if (result.length() <=0) {
  //  return;
  //}

  std::string text_value;
  if (result.length() >0 && '{' == result[0]) { ///< json格式处理
    Json::Value json_value;
    Json::Reader json_reader;
    Json::FastWriter json_writer;
    json_reader.parse(result, json_value, true);
    /*std::string*/ text_value = json_value.get("text", "").asString();
    //json_value["service"] = ""; ///< 电信
    json_value["time_interval"] = time;
    json_value["is_timeout"] = 0;
    //json_value["local_record_file"] = pcm_save_file_path;
    result = json_writer.write(json_value);
    result = GetUtility()->ReplaceAllDistinct ( result, "\\r\\n", "" );
  } /*else if (strcmp(result.c_str(), "...") == 0) {
    text_value = asr_timeout_broadcast_;
    Json::Value json_value;
    Json::Value json_v;
    Json::Reader json_reader;
    Json::FastWriter json_writer;
    json_value["rc"] = "0";
    json_value["rg"] = "0";
    json_value["text"] = asr_timeout_broadcast_;
    json_value["answer_best"] = asr_timeout_broadcast_;
    json_value["answer_old"] = "";
    json_value["other_answers"] = "";
    json_value["service"] = "";
    json_value["time_interval"] = 0;
    json_value["is_timeout"] = 1;
    result = json_writer.write(json_value);
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "json event = " << result);
  } else if (strcmp(result.c_str(), "....") == 0) {
    text_value = "网络信号差，待宝宝修复好了在听你说";
    Json::Value json_value;
    Json::Value json_v;
    Json::Reader json_reader;
    Json::FastWriter json_writer;
    json_value["rc"] = "0";
    json_value["rg"] = "0";
    json_value["text"] = "网络信号差，待宝宝修复好了在听你说";
    json_value["answer_best"] = "网络信号差，待宝宝修复好了在听你说";
    json_value["answer_old"] = "";
    json_value["other_answers"] = "";
    json_value["service"] = "";
    json_value["time_interval"] = 0;
    json_value["is_timeout"] = 1;
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "json event = " << result);
  }*/ else {
    // 普通格式,即纯文本
    text_value = result;
  }

  bool not_alert = false;
  // 过滤词
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "asr get result text_value = " << text_value);
  if (text_value.empty()) {
    //return;
    not_alert = true;
  } else if (text_value.length() <= /*3*/config_voice_filter_min_characters_atomic_) {
    not_alert = true;
    //

    std::list<std::string>::iterator it_find = std::find(list_asr_no_filter_.begin(), list_asr_no_filter_.end(), text_value);
    if (it_find != list_asr_no_filter_.end()) {
      // filter, not fine no filetr waord
      YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "asr get no filter words = " << text_value);
      //return;
      not_alert = false;
    }
  }

  ///<语料收集
  if (voice_collection_mode_ == 1 && voice_log_filename_!="") {
    bool voice_delete = true;
    ///<记录所有音频结果模式
    if (collection_voice_save_mode_ == 0) {
      LogAccumulation(term_id_, config_save_voicefile_asr_file_path_, voice_log_filename_, log_date_, text_value);
      voice_delete = false;
    }
    ///<只记录空结果
    if (collection_voice_save_mode_ == 1 && text_value.length() == 0) {
      LogAccumulation(term_id_, config_save_voicefile_asr_file_path_, voice_log_filename_, log_date_, text_value);
      voice_delete = false;
    }
    ///<只记录有转写结果
    if (collection_voice_save_mode_ == 2 && text_value.length() > 0) {
      LogAccumulation(term_id_, config_save_voicefile_asr_file_path_, voice_log_filename_, log_date_, text_value);
      voice_delete = false;
    }
    ///<记录空或小于设定长度的转写结果(不包含过滤词)
    if (collection_voice_save_mode_ == 3 && not_alert) {
      LogAccumulation(term_id_, config_save_voicefile_asr_file_path_, voice_log_filename_, log_date_, text_value);
      voice_delete = false;
    }
    //删除本地文件
    if (voice_delete && pcm_save_file_path.length() >0) {
      boost::system::error_code myecc;
      boost::filesystem::remove(pcm_save_file_path, myecc);
      YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "removefile = " << pcm_save_file_path);
    }
  }

  if (not_alert) {
    return;
  }
  TDAsrResultPtr result_ptr(new TDAsrResult(iOwnerid, lFlag, result.c_str()));
  if (NULL == result_ptr)
    return;
  boost::lock_guard<boost::mutex> lock_guard_save_result(mutex_list_asr_result_);
  list_asr_result_.push_back(result_ptr);
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "return asr result = " << result);

}


/**
 *@brief 打断模式的语音识别
 *@param
 *@return
 */
int CXFAsr::ThreadStartAsr(const int interrupte_mode, const int iCurrentStartAsrCount, const int iOwnerid,const int lFlag) {

  // 不支持并发，同时当前只能单线程执行
  // 这里改成 timer互斥
  static boost::timed_mutex mutex_exe_serial_asr_;
  if (false == mutex_exe_serial_asr_.try_lock_for(boost::chrono::milliseconds(2000))) {
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "error in MsgProcStartASR 出错，获取不到锁，当前已经在语音识别，不再开启语音识别");
    return YSOS_ERROR_FAILED;
  }
  boost::lock_guard<boost::timed_mutex> lock_guard_thread(mutex_exe_serial_asr_, boost::adopt_lock);

  int fun_ret = YSOS_ERROR_FAILED;
  ScenesInterfacePtr scenceptr =  GetCurrentScenesInterfacePtr(); // GetCurrentScenesInterfacePtr("asr");
  assert(scenceptr);
  if (NULL == scenceptr) {
    fun_ret= YSOS_ERROR_FAILED;
    goto FB_RETURN;
  }

  {
    ///< 清空以前的音频数据
    boost::lock_guard<boost::mutex> lock_guard(mutex_list_asr_voice_data_);
    list_voice_data_.clear();
  }

  scenceptr->DomSomethingBeforeAsr();

// is_asring_ =1;  ///< 在进行语音识别

  if (interrupte_mode) { ///< 打断模式
    bool bneed_asr_recording_loop = true;
    while (bneed_asr_recording_loop) {
      static int iloop_times = 0;
      iloop_times ++;
      YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "循环次数:" <<  iloop_times);
      int iret_recording = ThreadStartAsrRecordingChild(scenceptr, 1, iCurrentStartAsrCount,iOwnerid, lFlag);

      if (iCurrentStartAsrCount <= call_times_max_stopasr_atomic_) { ///< stop asr 中断返回
        fun_ret= YSOS_ERROR_FAILED;
        goto FB_RETURN;
      }

      //如果上次录音出错，为了避免重启抛出错误，这里停止200miliseconds;
      if (YSOS_ERROR_SUCCESS != iret_recording) {
        boost::this_thread::sleep_for(boost::chrono::milliseconds(200));
      }
    }  ///< 结束循环录音
  } else {
    /// 非打断模式
    int ret = ThreadStartAsrRecordingChild(scenceptr,0, iCurrentStartAsrCount, iOwnerid, lFlag);
  }

  fun_ret = YSOS_ERROR_SUCCESS;

FB_RETURN:
  // is_asring_ =0;    ///< 停止了语音识别
  return fun_ret;
}


TDAsrResultPtr CXFAsr::GetAsrResult() {
  boost::lock_guard<boost::mutex> lock_guard_save_result(mutex_list_asr_result_);
  if (list_asr_result_.size() <=0)
    return NULL;
  //TDAsrResultPtr front_ptr = *(list_asr_result_.begin());
  //list_asr_result_.pop_front();
  //return front_ptr;

  // 只返回最新的一份识别结果
  TDAsrResultPtr end_ptr = *(list_asr_result_.rbegin());
  list_asr_result_.clear();
  return end_ptr;
}

///< 创建一个线程去执行语音识别操作
int CXFAsr::MsgProcStartASR(const int iCallNum, const int iOwnerid,const int lFlag, std::string* pstrRecvErrorMsg) {

  {
    ///< 加入一个计时器,以更调试用
    static int t_callcount2 = 0;
    ++t_callcount2;
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "StartASR被调用次数: " << t_callcount2);
    /////< 清空以前的音频数据
    //boost::lock_guard<boost::mutex> lock_guard(mutex_list_voice_data_);
    //list_voice_data_.clear();
  }

  // 直接使用打断 模式
  boost::thread* ptemp_thread = thread_group_all_.create_thread(boost::bind(&CXFAsr::ThreadStartAsr, this, 1, iCallNum, iOwnerid, lFlag));
  thread_group_all_.add_this_thread_to_unfinish_remove_list(ptemp_thread);
  return YSOS_ERROR_SUCCESS;
}


int CXFAsr::MsgProcStopASR(void) {

  ////++ call_times_max_stopasr_atomic_;
  //int tempint = call_times_max_startasr_atomic_;
  //call_times_max_stopasr_atomic_ = tempint;

  {
    ///< 清空以前的音频数据
    boost::lock_guard<boost::mutex> lock_guard(mutex_list_asr_voice_data_);
    list_voice_data_.clear();
  }

  return YSOS_ERROR_SUCCESS;
}

int CXFAsr::MsgProcOpenAudioCaptureDevice(void) {
  return YSOS_ERROR_FAILED;
}

int CXFAsr::MsgProcCloseAudioCaptureDevice(void) {
  return YSOS_ERROR_SUCCESS;
}

int CXFAsr::MsgProcStartTTS(int iOwner, std::string strText) {
  {
    /// 结束当前播报
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "in MsgProcPlaySound,ownerid: " << iOwner << ", text; " << strText.c_str());
    MsgProcStopTTS();
  }

  boost::lock_guard<boost::mutex> guard(mutex_tts_oper_);

  if (strText.length() > 1024*7) {
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "字符超过2048，不支持超过这么长文本的tts");
    return YSOS_ERROR_FAILED;
  }

  /// 如果上一路会话没有结束，结束上一次会话
  XFContext xfcontext = XFContextManager::GetXFContext();
  if ('\0' != xfcontext.tts_sessionid_[0]) {
    GetCurrentScenesInterfacePtr()->TTSSessionEnd(xfcontext.tts_sessionid_, NULL);
    XFContextManager::Settts_sessionid(NULL);
  }

  XFContextManager::Settts_starttts_ownerid(iOwner);

  std::string strSessionID="";
  if (YSOS_ERROR_SUCCESS != GetCurrentScenesInterfacePtr()->TTSSessionBegin(strSessionID)) {
    return YSOS_ERROR_FAILED;
  }
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "TTSSessionBegin sessionid: " << strSessionID.c_str());
  XFContextManager::Settts_sessionid(strSessionID.c_str());
  if (YSOS_ERROR_SUCCESS != GetCurrentScenesInterfacePtr()->TTSTextPut(strSessionID.c_str(), strText.c_str(),strText.length(),NULL)) {
    return YSOS_ERROR_FAILED;
  }

  return YSOS_ERROR_SUCCESS;
}

int CXFAsr::GetTTSAudio(char* pBuf, int iBufMaxSize, int* pAudioLength, bool* pIsFinish) {
  boost::lock_guard<boost::mutex> guard(mutex_tts_oper_);

  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "proc GetTTSAudio");
  char* pout = pBuf; //  NULL;
  int buffer_size =iBufMaxSize;  // 0;

  assert(pout && buffer_size >0 && pAudioLength);
  if (NULL == pout || buffer_size<=0 || NULL == pAudioLength) {
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "error, the receiver buf is null");
    return YSOS_ERROR_FAILED;
  }
  XFContext xfcontext = XFContextManager::GetXFContext();
  if ('\0' == xfcontext.tts_sessionid_[0]) {
    return YSOS_ERROR_FAILED;
  }
  TDCharBufPtr AudioDataPtr;
  unsigned int iaudio_len=0;
  int synth_status = TDMSP_TTS_FLAG_ERROR;
  int audioget_ret = GetCurrentScenesInterfacePtr()->TTSAudioGet(xfcontext.tts_sessionid_,AudioDataPtr, &iaudio_len, &synth_status);
  YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "TTSAudioGet:sessionid: " << xfcontext.tts_sessionid_ << ",audiolen: " << iaudio_len << ",synth_status: " << synth_status);
  if (YSOS_ERROR_SUCCESS != audioget_ret) {
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "TTSAudioGet fail");
    return YSOS_ERROR_FAILED;
  }
  *pAudioLength = iaudio_len;  ///< 传出数据长度
  int voice_play_mode = GetCurrentScenesInterfacePtr()->TTSVoicePlayMode();

  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "voice play mode immediately");
  // 通过立即播放的模式
  if (iaudio_len>0) {
    assert(NULL != AudioDataPtr);
    assert(buffer_size >= iaudio_len);
    if (buffer_size < iaudio_len) {
      YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "error ,store tts voice data,buffer_size is to small");
      assert(false);
      return YSOS_ERROR_FAILED;
    }
    //memcpy_s(pout, buffer_size, AudioDataPtr.get(), iaudio_len);
    memcpy(pout, AudioDataPtr.get(), iaudio_len);
  }
  if (TDMSP_TTS_FLAG_DATA_END == synth_status) {
    *pIsFinish = true;
  } else {
    *pIsFinish =0;
  }

  if (TDMSP_TTS_FLAG_DATA_END == synth_status) {
    GetCurrentScenesInterfacePtr()->TTSSessionEnd(xfcontext.tts_sessionid_,NULL);
    XFContextManager::Settts_sessionid(NULL);
  }
  return YSOS_ERROR_SUCCESS;
}

int CXFAsr::MsgProcStopTTS() {
  boost::lock_guard<boost::mutex> guard(mutex_tts_oper_);

  XFContextManager::Settts_starttts_ownerid(0);

  /// 如果上一路会话没有结束，结束上一次会话
  XFContext xfcontext = XFContextManager::GetXFContext();
  if ('\0' != xfcontext.tts_sessionid_[0]) {
    GetCurrentScenesInterfacePtr()->TTSSessionEnd(xfcontext.tts_sessionid_, NULL);
    XFContextManager::Settts_sessionid(NULL);
  }

  return YSOS_ERROR_SUCCESS;
}


void CXFAsr::ReadConfig(const char* pModuleFilePath) {

//#ifdef _DEBUG
//  MessageBox(NULL, _T("CXFAsr::ReadConfig alert window"), _T("alert"), MB_OK);
//#endif

  char config_file_full_path[1024] = {'\0'};
#ifdef _WIN32
  if (NULL == pModuleFilePath) {
    sprintf_s(config_file_full_path, sizeof(config_file_full_path), "%s", /*"C:\\TDRobot\\etc\\TDRobotInfo.ini"*/".\\etc\\TDRobotInfo.ini");
  } else {
    sprintf_s(config_file_full_path, sizeof(config_file_full_path), "%setc\\TDRobotInfo.ini", pModuleFilePath);
  }
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "readconfig filepath: " << config_file_full_path);
  char voide_detail_config_file_path[1024]= {'\0'};
  if (NULL == pModuleFilePath) {
    sprintf_s(voide_detail_config_file_path, sizeof(config_file_full_path), "%s", /*"C:\\TDRobot\\etc\\voice.ini"*/".\\etc\\voice.ini");
  } else {
    sprintf_s(voide_detail_config_file_path, sizeof(config_file_full_path), "%setc\\voice.ini", pModuleFilePath);
  }
#else
  if (NULL == pModuleFilePath) {
    sprintf(config_file_full_path, "%s", /*"C:\\TDRobot\\etc\\TDRobotInfo.ini"*/"./etc/TDRobotInfo.ini");
  } else {
    sprintf(config_file_full_path, "%setc/TDRobotInfo.ini", pModuleFilePath);
  }
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "readconfig filepath: " << config_file_full_path);
  char voide_detail_config_file_path[1024]= {'\0'};
  if (NULL == pModuleFilePath) {
    sprintf(voide_detail_config_file_path, "%s", /*"C:\\TDRobot\\etc\\voice.ini"*/"./etc/voice.ini");
  } else {
    sprintf(voide_detail_config_file_path, "%setc/voice.ini", pModuleFilePath);
  }
#endif
  // 记取日志级别
  int iloglevel = 3; /// 默认是错误级别
  /// 使用语音模块类型,0 科大迅飞，1 捷通华声
  config_voice_service_provide_id_ = 0;
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "读取配置文件：voice service providerid: " << config_voice_service_provide_id_);

  //读取;录音模式，0 普通模式，录完音ASR后会关闭录音机. 1打断模式，默认0
  config_voice_record_mode_ = 1;
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "读取配置文件：voice_record_mode: " << config_voice_record_mode_);

  //;nlp超时时间设置（单位秒）,默认5秒。
  config_nlp_timeout_millseconds_atomic_ = 5000;

  //是否过滤错误，如果是，则底盘出错，也不上报error happens,如果是1过滤，0 不过滤。默认是0
  asr_filter_error_happens_atomic_ = 0;

  //是否保存pcm文件
  char param_buf[1024]= {'\0'};
  memset(param_buf, '\0', sizeof(param_buf));
  config_need_save_voice_file_ = 1;
  YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "need_save_voice_file:" << config_need_save_voice_file_);

  if (config_need_save_voice_file_ == 1) {
    //保存pcm文件路径
    //::GetPrivateProfileString("VOICEFILE", "voicefilepath","./wav", param_buf, sizeof(param_buf), voide_detail_config_file_path);
    config_save_vocie_filepath_ = param_buf;
    if (config_save_vocie_filepath_ != "") {
      YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "voicefilepath:" << config_save_vocie_filepath_);
    } else {
      config_save_vocie_filepath_ = "../wav";
      YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "voicefilepath default:" << config_save_vocie_filepath_);
    }
  }

  //语料收集模式
  voice_collection_mode_ = 0;
  YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "voice_collection_mode:" << voice_collection_mode_);
  //语料收集记录音频模式
  collection_voice_save_mode_ = 0;
  YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "collection_voice_save_mode:" << collection_voice_save_mode_);

  if (voice_collection_mode_ == 1) {
    //保存语料收集记录文件路径
    memset(param_buf, '\0', sizeof(param_buf));
    //::GetPrivateProfileString("VOICEFILE", "voicefile_asr_file_path","./log/accumulation/", param_buf, sizeof(param_buf), voide_detail_config_file_path);
    config_save_voicefile_asr_file_path_ = param_buf;
    if (config_save_voicefile_asr_file_path_ != "") {
      YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "voicefile_asr_file_path:" << config_save_voicefile_asr_file_path_);
    } else {
      config_save_voicefile_asr_file_path_ = "./log/accumulation/";
      YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "voicefile_asr_file_path default:" << config_save_voicefile_asr_file_path_);
    }
  }

  //asr获取结果超时时间（单位毫秒），默认10000毫秒。
  config_asr_get_result_timeout_ = 10000;//::GetPrivateProfileInt("VOICEDETAILCONFIG", "asr_get_result_timeout", 10000, voide_detail_config_file_path);
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "asr获取结果超时时间（单位毫秒）: " << config_asr_get_result_timeout_);

  //asr获取结果超时时间（单位毫秒），默认10000毫秒。
  is_broadcast_ = 1;//::GetPrivateProfileInt("VOICEDETAILCONFIG", "is_broadcast", 1, voide_detail_config_file_path);
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "超时播报 " << is_broadcast_);

  //asr获取结果超时时间播报内容。
  //::GetPrivateProfileString("VOICEDETAILCONFIG", "asr_timeout_broadcast", "宝宝没听清楚，能不能再说一遍", asr_timeout_broadcast_, sizeof(asr_timeout_broadcast_), voide_detail_config_file_path);
  strcpy(asr_timeout_broadcast_, "宝宝没听清楚，能不能再说一遍");
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "asr获取结果超时播报: " << asr_timeout_broadcast_);

  //;如果是打断模式下，则过滤的字符数。默认是2（即1个中文字）
  config_voice_filter_min_characters_atomic_ = 2;//::GetPrivateProfileInt("VOICEDETAILCONFIG", "voice_filter_min_character", 2, voide_detail_config_file_path);
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "读取配置文件：config_voice_filter_min_characters_atomic_:" << config_voice_filter_min_characters_atomic_);


  config_asr_allow_continuous_max_error_count_ = 2;//::GetPrivateProfileInt("VOICEDETAILCONFIG", "asr_allow_continuous_max_error_count", 2, voide_detail_config_file_path);
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "asr_allow_max_error_count: " << config_asr_allow_continuous_max_error_count_);

  config_asr_allow_continuous_max_error_miliseconds_ = 2;//::GetPrivateProfileInt("VOICEDETAILCONFIG", "asr_allow_continuous_max_error_miliseconds", 2, voide_detail_config_file_path);
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "asr_allow_continuous_max_error_miliseconds: " << config_asr_allow_continuous_max_error_miliseconds_);

  memset(param_buf, '\0', sizeof(param_buf));
  //::GetPrivateProfileStringA("IFLY", "asr_no_filter", "", param_buf, sizeof(param_buf), voide_detail_config_file_path);
  //GetPrivateProfileStringA("IFLY", "asr_no_filter", voide_detail_config_file_path);
  std::string asr_no_filter(param_buf);
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "asr_no_filter: " << asr_no_filter);
  if (!asr_no_filter.empty()) {
    GetUtility()->SplitString(asr_no_filter, "|", list_asr_no_filter_);
  }

  memset(param_buf, '\0', sizeof(param_buf));
  //::GetPrivateProfileStringA("IFLY", "tts_broadcast_filter", "", param_buf, sizeof(param_buf), voide_detail_config_file_path);
  //GetPrivateProfileStringA("IFLY", "tts_broadcast_filter", voide_detail_config_file_path);
  std::string tts_broadcast_filter(param_buf);
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "tts_broadcast_filter: " << tts_broadcast_filter);
  if (!tts_broadcast_filter.empty()) {
    GetUtility()->SplitString(tts_broadcast_filter, "|", list_tts_broadcast_filter_);
  }

  //error_times_ = ::GetPrivateProfileInt("IFLY", "asr_error_times", 5, voide_detail_config_file_path);
  error_times_ = 5; //::GetPrivateProfileInt("IFLY", "asr_error_times", voide_detail_config_file_path);
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "error_times_: " << error_times_);
}


bool CXFAsr::GetCurrentModuleFilePath(int iMaxBufSize, char* pFilePath) {
  //for test
  //strcpy_s(pFilePath, iMaxBufSize, /*"C:\\TDRobot\\"*/"./");
  strcpy(pFilePath, "./");
  return true;

  /* add for linux
  assert(iMaxBufSize>0 && NULL != pFilePath);
  HANDLE hOCXHandle = NULL;
  unsigned int itrunkcount =0;
  hOCXHandle =  GetModuleHandle("XFAsr.dll"); //如果为null,则在IE中加开时会加载C:\Program Files (x86)\Internet Explorer\iexplorer.exe
  if (NULL == hOCXHandle) {
    assert(false);
    return false;
  }
  if (GetModuleFileName((HMODULE)hOCXHandle,pFilePath,iMaxBufSize)<=0) { //得到结果 C:\TDRobot\XFAsr.dll
    assert(false); //can't load module
    return false;
  }
  //得到去掉最后文件名的路径
  itrunkcount = strlen("XFAsr.dll");
  assert(strlen(pFilePath)>itrunkcount);
  if (strlen(pFilePath)>itrunkcount) {
    pFilePath[(strlen(pFilePath)-itrunkcount)] ='\0';
  }
 
  return true;
  */
}

int CXFAsr::SetCurrentScenesType(int iScenesID) {
  boost::lock_guard<boost::mutex> lockguard_voicetype(mutex_set_voice_service_provider_);

  switch (iScenesID) {
  case VoiceType_IFLY:
    if (NULL == scenes_interface_ptr_ifly_) {
      scenes_interface_ptr_ifly_.reset(new ScenesIFLY);
      scenes_interface_ptr_ifly_->ReadConfig(g_strModuleFilepath.c_str());
    }
    scenes_interface_ptr_ = scenes_interface_ptr_ifly_;
    break;
  default: {
    assert(false);
  }
  }
  return YSOS_ERROR_SUCCESS;
}
ScenesInterfacePtr CXFAsr::GetCurrentScenesInterfacePtr(int* piScenesid) {
  boost::lock_guard<boost::mutex> lockguard_voicetype(mutex_set_voice_service_provider_);
  assert(scenes_interface_ptr_);
  if (NULL == scenes_interface_ptr_) {
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "error GetCurrentScenesInterfacePtr is null");
    return NULL;
  }
  if (NULL != piScenesid)
    *piScenesid = config_voice_service_provide_id_;
  return scenes_interface_ptr_;

}

/**
*@brief get specific voice server provider id
*@param strType[Input] service type, can be one of "asr","nlp","tts"
*@return  current voice provider id
*/
int CXFAsr::GetVoiceProviderID(const std::string strType) {
  boost::lock_guard<boost::mutex> lockguard_voicetype(mutex_set_voice_service_provider_);
  assert(scenes_interface_ptr_);
  if (NULL == scenes_interface_ptr_)
    return -1;
  return scenes_interface_ptr_->GetVoiceProviderID(strType);
}

void ThreadFunCheckDevice(boost::mutex* pMutex, CXFAsr* pXFasr, short iCheckItem) {
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "in ThreadFunCheckDevice: checkitem: " << iCheckItem);
  assert(pMutex && pXFasr);
  if (NULL == pMutex || NULL == pXFasr)
    return;

  boost::lock_guard<boost::mutex> lock_guard_check_device(*pMutex);  ///< 注意，这里会一直等待

  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "TDCheckDevice: item: " << iCheckItem);
  ScenesInterfacePtr scence_ptr = pXFasr->GetCurrentScenesInterfacePtr();
  assert(scence_ptr);
  std::string check_error_string;
  int ierror_code=0;
  scence_ptr->CheckDevice(iCheckItem, ierror_code , &check_error_string);
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "TDCheckDevice checkitem: " << iCheckItem << ", ierrorcode: " << ierror_code);
}


//int CXFAsr::TDSetAsrAccent(SHORT iAccentId) {
//  // TODO: Add your implementation code here
//  int iret = GetCurrentScenesInterfacePtr()->ASRSetAccent(iAccentId);
//  return YSOS_ERROR_SUCCESS ?YSOS_ERROR_SUCCESS :S_FALSE;
//}
//
//
//int CXFAsr::TDGetAsrAccent(SHORT* iAccentID) {
//  // TODO: Add your implementation code here
//  *iAccentID = GetCurrentScenesInterfacePtr()->ASRGetAccentID();
//  return YSOS_ERROR_SUCCESS;
//}

int CXFAsr::TDSetAudioCaptureShowVolumeStrength(SHORT iShowOrFalse) {
  return YSOS_ERROR_SUCCESS;
}

int CXFAsr::AudioChangeAccent(const int &accent) {
  // accent   : 0-普通话 1-粤语
  int n_return = YSOS_ERROR_FAILED;
  ScenesInterfacePtr scene_ptr = GetCurrentScenesInterfacePtr();
  assert(scene_ptr);
  if (NULL == scene_ptr) {
    YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "scene_ptr is null");
    return n_return;
  }
  switch (accent) {
  case 0:
    YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "AudioChangeAccent in, accent = " << TDACCENT_MANDARIN_STRING);
    n_return = scene_ptr->ASRSetAccent(TDACCENT_MANDARIN_STRING);
    break;
  case 1:
    YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "AudioChangeAccent in, accent = "<<TDACCENT_CANTONESE_STRING);
    n_return = scene_ptr->ASRSetAccent(TDACCENT_CANTONESE_STRING);
    break;
  default:
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "AudioChangeAccent in, accent not known, accent:" << accent);
    //assert(false);
  }
  if (n_return != YSOS_ERROR_SUCCESS) {
    YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "AudioChangeAccent failed");
  }
  return n_return;
}

int CXFAsr::ChangeConfig(const int &mode) {
  int n_return = YSOS_ERROR_FAILED;
  // 0在线， 1 离线
  std::string error_message;
  ScenesInterfacePtr scene_ptr = GetCurrentScenesInterfacePtr();
  assert(scene_ptr);
  if (NULL == scene_ptr) {
    return n_return;
  }
  switch (mode) {
  case 0:
    YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "ChangeConfig in, asr engine type = cloud");
    n_return = scene_ptr->ASRSetEngineType("cloud");
    break;
  case 1:
    YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "ChangeConfig in, asr engine type = local");
    n_return = scene_ptr->ASRSetEngineType("local");
    if (n_return == YSOS_ERROR_SUCCESS) {
      n_return = scene_ptr->ASRSetGrammarFile(NULL, &error_message);
      if (n_return == YSOS_ERROR_SUCCESS) {
        YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "ASRSetGrammarFile failed");
      }
    }
    break;
  default:
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "ChangeConfig in, mode not known, mode:" << mode);
    assert(false);
  }
  if (n_return != YSOS_ERROR_SUCCESS) {
    YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "ChangeConfig failed");
  }
  return n_return;
}

int CXFAsr::AudioChangeTTSMode(const int &mode) {
  int n_return = YSOS_ERROR_FAILED;
  // 0在线， 1 离线
  ScenesInterfacePtr scene_ptr = GetCurrentScenesInterfacePtr();
  assert(scene_ptr);
  if (NULL == scene_ptr) {
    return n_return;
  }
  switch (mode) {
  case 0:
    YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "AudioChangeTTSMode in, tts engine type = cloud");
    n_return = scene_ptr->TTSSetEngineType("cloud");
    break;
  case 1:
    YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "AudioChangeTTSMode in, tts engine type = local");
    n_return = scene_ptr->TTSSetEngineType("local");
    break;
  default:
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "AudioChangeTTSMode in, mode not known, mode:" << mode);
    assert(false);
  }
  if (n_return != YSOS_ERROR_SUCCESS) {
    YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "AudioChangeTTSMode failed");
  }
  return n_return;
}

int CXFAsr::AudioChangeTTSSpeaker(const std::string &speaker) {
  int n_return = YSOS_ERROR_FAILED;
  ScenesInterfacePtr scene_ptr = GetCurrentScenesInterfacePtr();
  assert(scene_ptr);
  if (NULL == scene_ptr) {
    return n_return;
  }

  YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "AudioChangeTTSSpeaker in, speaker = "<<speaker);
  n_return = scene_ptr->TTSSetSpeaker(speaker.c_str());
  if (n_return != YSOS_ERROR_SUCCESS) {
    YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "AudioChangeTTSSpeaker failed");
  }
  return n_return;
}

int CXFAsr::FB_ASR_RECORDING_INTERRUPTION_RETURN(ScenesInterfacePtr scenceptr, std::string strsession_id, std::string strsavefile_voice_recorder)
{   ///< 中断返回
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "则关闭session");
  if (strsession_id.length() >0) {
    scenceptr->ASRSessionEnd(strsession_id.c_str());
    strsession_id = "";
  }

  if (NULL != psavefile_voice_recorder_) {  ///< 关闭录音文件
    fclose(psavefile_voice_recorder_);
    psavefile_voice_recorder_ = NULL;
  }

  if (strsavefile_voice_recorder.length() >0) {
    boost::system::error_code myecc;
    boost::filesystem::remove(strsavefile_voice_recorder, myecc);
  }
  return YSOS_ERROR_FAILED;
}