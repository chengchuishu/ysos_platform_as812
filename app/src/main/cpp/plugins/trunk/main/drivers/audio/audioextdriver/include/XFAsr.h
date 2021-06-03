/**   //NOLINT
  *@file XFAsr.h
  *@brief Definition of 迅飞语音接口模块, 此模块代码为原苏海鹏词音模块提取出来。
  *@version 0.1
  *@author 作者1：苏海鹏，作者2：wanfenfen
  *@date Created on: 2016:6:11   19:58
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  */
#ifndef XFASR_XFASR_H  //NOLINT
#define XFASR_XFASR_H  //NOLINT

/// stl headers
#include <string>

/// boost headers
#include <boost/atomic.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/filesystem.hpp>

/// ysos headers
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"

/// private headers
#include "../include/scenesinterface.h"
#include "../include/scenesifly.h"
#include "../include/msgdefine.h"
#include "../../../public/include/common/threadgroupex.hpp"

#ifdef _WIN32
#else
#include "../../../public/include/common/event.h"
typedef int SHORT;
#endif


struct TDVoiceData {
 public:
  TDVoiceData() {
    pdata = NULL;
    udata_length =0;
  }
  ~TDVoiceData() {
    ReleaseData();
  }
  int SaveData(char* pData, const UINT32 uDataLength) {
    assert(pData && uDataLength >0);
    ReleaseData();
    if (pData && uDataLength >0) {
      udata_length = uDataLength;
      pdata = new char[uDataLength];
      if (pdata) {
        //memcpy_s(pdata, uDataLength, pData, uDataLength);
        memcpy(pdata, pData, uDataLength);
        return YSOS_ERROR_SUCCESS;
      } else {
        return YSOS_ERROR_FAILED;
      }
    } else {
      return YSOS_ERROR_FAILED;
    }
  }
  int ReleaseData() {
    if (pdata) {
      delete [] pdata;
    }
    pdata = NULL;
    udata_length =0;
    return YSOS_ERROR_SUCCESS;
  }
  char* GetData() {
    return pdata;
  }
  int GetDataLength() {
    return udata_length;
  }
 private:
  char* pdata;
  UINT32 udata_length;
};
typedef boost::shared_ptr<TDVoiceData> TDVoiceDataPtr;

struct TDAsrResult {
 public:
  TDAsrResult(const int iOwnerid, const int iFlag, const char* presult) {
    iowner_id = iOwnerid;
    iflag = iFlag;
    if (presult)
      result = presult;
  }
 public:
  int iowner_id;
  int iflag;
  std::string result;
};
typedef boost::shared_ptr<TDAsrResult> TDAsrResultPtr;

/**
 * enum of asr status event id
 */
enum ASRStatusEventId {
  ASRStatusEventId_NULL =0,
  ASRStatusEventId_RECORD =1,  ///< 侦听中, 从打开录音机 到 检测到尾端点
  ASRStatusEventId_ASR =2,  ///< 识别中， 从检测到尾端点到识别到结果
  ASRStatusEventId_SUCCESS_FINISH =3,  ///< 识别完成
  ASRStatusEventId_ERROR =4,   ///<出错
  ASRStatusEventId_TIMEOUT =5,  ///<超时
  ASRStatusEventId_ASR_THEN_NLP =6,  ///<解析中， 识别到误音结果再次解析中
  ASRStatusEventId_STOP  = 7,   ///< 停止
};


struct TDSaveUploadVoiceRequest {
  std::string voice_file_path;
  std::string voice_asr_result;
  std::string voice_asrnlp_result;
  //boost::property_tree::ptree asrnlp_tree;
};
typedef boost::shared_ptr<TDSaveUploadVoiceRequest> TDSaveUploadVoiceRequestPtr;

//////////////////////////////////////////////////////////////////////////
class CXFAsr
{
 public:
  CXFAsr() {
    myFinalConstruct();
  }
  ~CXFAsr() {
    myFinalRelease();
  }

  int myFinalConstruct() {
    //printf(" load xfasr.dll\n");
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "in FinalConstruct begin1");
    //MessageBoxA(NULL, "", "debug", 0);
    /// 构造和析构对象 互斥
    boost::lock_guard<boost::mutex> lock_guard_construct_release(mutex_construct_release_);
    construct_clock_count_ = clock();
    ++ construct_counter_atomic_;
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "in FinalConstruct begin2 real");

    static int iload_count =0;
    ++iload_count;
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "FinalConstruct 被调用第" << iload_count << "次");

    config_voice_record_mode_ = 0;
    psavefile_voice_recorder_ = NULL;
    //psavefile_voice_local_recorder_ = NULL;
    asr_filter_error_happens_atomic_ =0;
    call_times_max_startasr_atomic_ =0;
    call_times_max_stopasr_atomic_ =0;
    config_asr_allow_continuous_max_error_count_ =2;
    asr_current_continuous_error_count_ =0;
    config_asr_allow_continuous_max_error_miliseconds_ =20000;
    asr_current_error_clock_t_= clock();

#ifdef _WIN32
    hevent_module_optimize_finish_ =  CreateEvent(NULL, true, false, NULL);  ///< 注意，reset 是手动reset
#else
    //hevent_module_optimize_finish_ =  event_create(true, false);  ///< 注意，reset 是手动reset
#endif

    char module_filepath_buf[1024]= {'\0'};
    if (true == GetCurrentModuleFilePath(sizeof(module_filepath_buf),module_filepath_buf)) {
      g_strModuleFilepath = module_filepath_buf;
      ReadConfig(module_filepath_buf);
      ///< 以使用核心平台，以下设置是必须设置
      config_voice_record_mode_ =1;  ///< 必须是打断模式
      config_voice_service_provide_id_ =0;  ///< 是迅飞第三方
    }

    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "in FinalConstruct");

    scenes_interface_ptr_ = NULL;
    scenes_interface_ptr_ifly_ = NULL;
    scenes_interface_ptr_jthc_ = NULL;
    {
      SetCurrentScenesType(config_voice_service_provide_id_);
    }


    {
      // 根据一些配置文件，创建好一些对应的目录 
      boost::system::error_code myecc;
      boost::filesystem::path path_current_moudle(module_filepath_buf);
      if(config_save_vocie_filepath_.length() >0) {
        boost::filesystem::path tmp_path(config_save_vocie_filepath_);
        if(tmp_path.is_relative()) {
          tmp_path = path_current_moudle.string() + tmp_path.relative_path().string();
          config_save_vocie_filepath_ = tmp_path.string();
        }
        boost::filesystem::create_directories(tmp_path, myecc);
        YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "create voice directory, config_save_vocie_filepath_ = " << tmp_path);
      }

      if(config_save_voicefile_asr_file_path_.length() >0) {
        boost::filesystem::path tmp_path(config_save_voicefile_asr_file_path_);
        if(tmp_path.is_relative()) {
          tmp_path = path_current_moudle.string() + tmp_path.relative_path().string();
          config_save_voicefile_asr_file_path_ = tmp_path.string();
        }
        boost::filesystem::create_directories(tmp_path, myecc);
        YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "create asr file directory, config_save_voicefile_asr_file_path_ = " << tmp_path);
      }
    }

    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "in FinalConstruct end");
    return YSOS_ERROR_SUCCESS;
  }
  void myFinalRelease() {
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "in FinalRelease begin1");
    boost::lock_guard<boost::mutex> lock_guard_construct_release(mutex_construct_release_);
    FinalReleaseWithNoLock();
  }

  void FinalReleaseWithNoLock() {
    ++ release_counter_atomic_;

    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "in FinalRelease begin2 real");

    {
      /// 等待模块加载完成
      YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "等待 优化加载完后才能释放");

      // 停止asr.
      MsgProcStopASR();

      //停止tts
      MsgProcStopTTS();

      {
        ///< 等待所有的线程退出
        YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "wait for all threads to exit begin");
        thread_group_all_.interrupt_all();
        thread_group_all_.join_all();
        YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "wait for all threads to exit end");
      }

      /// 退出第三方
      {
        boost::lock_guard<boost::mutex> lockguard_voicetype(mutex_set_voice_service_provider_);
        if (scenes_interface_ptr_ && true ==scenes_interface_ptr_->IsHaveLoginIn()) {
          YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "voice provider,ready to Logout");
          scenes_interface_ptr_->Logout();
        }
        ///< 释放场景资源
        scenes_interface_ptr_ifly_ = NULL;
        scenes_interface_ptr_jthc_ = NULL;
        scenes_interface_ptr_ = NULL;
      }

      YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "in FinalRelease end");
    }
  }

 public:
  //friend int ThreadStartAsrRecordingChild(const int iSupportAsrConcurrency, const int iInterruptionMode, const int iCurrentStartAsrCount, CXFAsr* pCXFAsr, const int iOwnerid,const int lFlag);
  //friend int ThreadStartAsrGetResult(const int iCurrentStartAsrCount, const int iInterruptionMode, CXFAsr* pCXFAsr, const int iOwnerid,const int lFlag, const std::string strSessionid);
  //friend int DetectAsrData(const int iSupportAsrConcurrency, const int iInterruptionMode, const int iCurrentStartAsrCount, CXFAsr* pCXFAsr, const int iOwnerid,const int lFlag);
  //friend int LocalAsrResult(const int iCurrentStartAsrCount, CXFAsr* pCXFAsr, const int iOwnerid, const int lFlag);
  void AddErrorResult(ScenesInterfacePtr scenceptr, const int iOwnerid, const int lFlag, int code, const std::string strSessionid, std::string msg, const std::string pcm_save_file_path);
  void AddAsrResult(ScenesInterfacePtr scenceptr, const int iOwnerid, const int lFlag, const std::string strSessionid, std::string result, int time, const std::string pcm_save_file_path);

  int ThreadStartAsr(const int interrupte_mode, const int iCurrentStartAsrCount, const int iOwnerid,const int lFlag);
  int ThreadStartAsrRecordingChild(ScenesInterfacePtr scenceptr, const int iInterruptionMode, const int iCurrentStartAsrCount, const int iOwnerid,const int lFlag);


  /**
   *@brief interface：start automatic speech recognition. asynchronous operation.
   *             open voice recorder first when voice recorder is close
   *@param iOwner[Input] 调用接口者ID, 抛出事件中会回传此参数。
   *@param lFlag[Input] 0 开始录音（支持语义解析），1 释放录音资源（相当于CloseDevice)，为了兼容留着。
   *             2 开始录音(不支持语义解析）
                 3 打开录音（人工坐席）（不支持语义解析）
                 4  原参数lFalg 0的增强版本，适用于打断模式,结束后即开启一轮新的会话，nlp异步操作）
   *@return success 0
   *        fail: non zero value
   */
  /*STDMETHOD*/ int TDStartASR(SHORT iOwner, SHORT iFlag);

  /**
  *@brief stop automatic speech recognition. asynchronous operation.
  *       如果当前在调用语音解析，则当前的解析结果返回空字符串的结果，如果当前没有在调用语音解析，则不操作
  *@param
  *@return success 0
  *        fail: non zero value
  */
  /*STDMETHOD*/int TDStopASR(SHORT iStopFlag);

  /**
   *@brief通过文本直接进行nlp解析。返回字符串的格式为第三方语音服务提供商的NLP解析格式
   *@param iOwner[Input] 调用接口者ID, 抛出事件中会回传此参数
   *@param strTextNeedNlp[Input]  需要进入nlp解析的文本
   *@param iSynchronize[Input] 是否同步操作，1 同步操作，0 异步操作。如果同步操作，则调用此接口直接返回结果。如果是异步操作，则调用此接口返回空，但会上抛出事件OnASRResultExEvent。
   *@param pResult[Output] nlp解析结果
   *@return success 0
   *        fail: non zero value
   */
  ///*STDMETHOD*/int TDNLP(SHORT iOwner, BSTR strTextNeedNlp, SHORT iSynchronize, BSTR* pResult);
  int TDNLP(SHORT iOwner, /*BSTR strTextNeedNlp*/const std::string strTextNeedNlp, SHORT iSynchronize, /*BSTR* pResult*/std::string* pResult);

  ///*STDMETHOD*/int TDNLPEx(SHORT iVoiceServiceProvider, SHORT iOwner, BSTR strTextNeedNlp, SHORT iSynchronize, BSTR* pResult);
  int TDNLPEx(SHORT iVoiceServiceProvider, SHORT iOwner, std::string strTextNeedNlp, SHORT iSynchronize, std::string* pResult);

  /**
   *@brief 开始语音合成。异步操作
   *@param iOwnerid[Input] 调用接口者ID, 抛出事件中会回传此参数。
   *@param strText[Input] 待合成语音的文本
   *@return success 0
   *        fail: non zero value
   */
  ///*STDMETHOD*/int TDStartTTS(SHORT iOwnerid, BSTR strText);
  int TDStartTTS(SHORT iOwnerid, std::string strText);

  /**
   *@brief 停止语音合成。同步操作。
   *@return success 0
   *        fail: non zero value
   */
  /*STDMETHOD*/int TDStopTTS(void);

  /**
  *@brief open voice recorder device, asynchronous operation.
  *@param
  *@return success 0
  *        fail: non zero value
  */
  /*STDMETHOD*/int TDOpenAudioCaptureDevice(void);

  /**
  *@brief close voice recorder device, asynchronous operation.
  *@param
  *@return success 0
  *        fail: non zero value
  */
  /*STDMETHOD*/int TDCloseAudioCaptureDevice(void);

  /**
   *@brief 打开录音设备。异步操作。
   *@param
   *@return success 0
   *        fail: non zero value
   */
  /*STDMETHOD*/int TDOpenPlayerDevice(void);

  /**
   *@brief 选择使用哪个第三方语音服务商,
   *@param iVoiceServiceProciderID[Input] 第三方语音服务商ID, 0 科大迅飞，1捷通华声
   *@param strServiceType[Input] 服务类型
   *@return success 0
   *        fail: non zero value
   */
  /*STDMETHOD*/int TDUseScenes(SHORT iScenesID);

  ///**
  // *@brief 检测设备是否正常
  // *@param iCheckItem[Input] checkitem
  // *                         0 录音设备串口通信是否正常。
  // *@param piCheckResult[Output] checkitem result
  // *@return success return YSOS_ERROR_SUCCESS
  // *        fail return S_FAIL
  // */
  ///*STDMETHOD*/int TDCheckDevice(SHORT iCheckItem, SHORT* piCheckResult);


  /**
   *@brief 设置ASR的方言
   *@param iAccentId[Input] 方言ID
   *@return success 0
   *        fail: non zero value
  // */
  ///*STDMETHOD*/int TDSetAsrAccent(SHORT iAccentId);

  ///**
  // *@brief 获取当前ASR的方言
  // *@param iAccentID[Output] 方言ID
  // *@return success 0
  // *        fail: non zero value
  // */
  ///*STDMETHOD*/ int TDGetAsrAccent(SHORT* iAccentID);

  /**
   *@brief 设置是否要显示音量的强度
   *@param iShowOrFalse 是否要显示，1 显示，0 不显示
   *                    如果显示，则表示当前的录音音量会通知事件 TDOnAudioCaptureVolumeStrength 返回
   *@return success 0
   *        fail: non zero value
   */
  /*STDMETHOD*/int TDSetAudioCaptureShowVolumeStrength(SHORT iShowOrFalse);

  /**
  *@brief local message process automatic speech recognition.
  *@param iCallNum[Input] 请求asr的id号
  *@param lFlag[Input] 0 开始录音（支持语义解析），1 释放录音资源（相当于CloseDevice)，为了兼容留着。
  *             2 开始录音(不支持语义解析）
  *@param strRecvErrorMsg[Onput] error message when speech recognition.
  *@return success rturn YSOS_ERROR_SUCCESS
  *        fail return YSOS_ERROR_FAIL
  */
  int MsgProcStartASR(const int iCallNum, const int iOwnerid,const int lFlag, std::string* pstrRecvErrorMsg);

  /**
   *@brief local message process stop asr
   *@param
   *@return success return YSOS_ERROR_SUCCESS
   *        fail return YSOS_ERROR_FAIL
   */
  int MsgProcStopASR(void);

  /**
   *@brief local message process open audio capture device
   *@param
   *@return success return YSOS_ERROR_SUCCESS
   *        fail return YSOS_ERROR_FAIL
   */
  int MsgProcOpenAudioCaptureDevice(void);

  /**
   *@brief local message process close audio capture device
   *@param
   *@return success return YSOS_ERROR_SUCCESS
   *        fail return YSOS_ERROR_FAIL
   */
  int MsgProcCloseAudioCaptureDevice(void);

  /**
   *@brief local message process, start tts
   *@param 调用接口者ID, 抛出事件中会回传此参数。
   *@return success return YSOS_ERROR_SUCCESS
   *        fail return YSOS_ERROR_FAIL
   */
  int MsgProcStartTTS(int iOwner, std::string strText);

  /**
   *@brief local messae process ,stop tts
   *@param
   *@return success return YSOS_ERROR_SUCCESS
   *        fail return YSOS_ERROR_FAIL
   */
  int MsgProcStopTTS();

  /**
  *@brief 处理nlp请求
  *       应用场景：普通的NLP 调用操作。
  *@param iVoiceServiceProvider[Input] 第三方语音服务提供商
  *@param iOwner[Input] 上层提供的id, 异步操作时连nlp结果一起返回
  *@param strTextNeedNlp[Input] 需要进行Nlp解析的文本
  *@param bSynchronize[Input] 是同步操作还是异步操作
  *@param pResult[Output] 得到的结果
  *@return success rturn YSOS_ERROR_SUCCESS
  *        fail return YSOS_ERROR_FAIL
  */
//  int ProcNLP(int iVoiceServiceProvider, SHORT iOwner, BSTR strTextNeedNlp, SHORT iSynchronize, BSTR* pResult);
  int ProcNLP(int iVoiceServiceProvider, SHORT iOwner, std::string strTextNeedNlp, SHORT iSynchronize,std::string* pResult);

  /**
   *@brief set current use voice service provider
   *@param iVoiceServiceProvider[Input] set voice service provider; 0 科大迅飞，1 捷通华声
   *@return success return YSOS_ERROR_SUCCESS
   *        fail return YSOS_ERROR_FAIL
   */
  int SetCurrentScenesType(int iVoiceServiceProvider);

  /**
   *@brief get current voice service provider
   *@param piScenesid[Output] receive voice service type providerid
   *@return   voice service type pointer
   */
  /*VoiceInterfacePtr*/ScenesInterfacePtr GetCurrentScenesInterfacePtr(int* piScenesid = NULL);

  /**
   *@brief get specific voice server provider id
   *@param strType[Input] service type, can be one of "asr","nlp","tts"
   *@return  current voice provider id
   */
  int GetVoiceProviderID(const std::string strType);


  void AudioCaptureCallBackData(char* pData, UINT32 uDataLength);
  /**
   *@brief 得到一个Asr结果，如果没有结果，则返回空
   *@param
   *@return
   */
  TDAsrResultPtr GetAsrResult();

  /**
   *@brief 得到TTS合成的音频数据
   *@param RecvAudioBuff[Input/Outpu] 用于接收合成的音频数据
   *@param pIsFinish[Output] 是否合成完成
   *@return
   */
  //int GetTTSAudio(ysos::BufferInterfacePtr RecvAudioBuff, bool* pIsFinish);
  int GetTTSAudio(char* pBuf, int iBufMaxSize, int* pAudioLength, bool* pIsFinish);

  std::string CreateVoiceFileName();

  std::string CreateLocalVoiceFileName();

  int ChangeConfig(const int &mode = 0);
  int AudioChangeAccent(const int &accent = 0);

  int AudioChangeTTSMode(const int &mode = 0);
  int AudioChangeTTSSpeaker(const std::string &speaker = "");

  void LogAccumulation(const std::string term_id, const std::string filepath,const std::string voicefilename,const std::string log_date, const std::string &asr_result);

  //add for linux
  int FB_ASR_RECORDING_INTERRUPTION_RETURN(ScenesInterfacePtr scenceptr, std::string strsession_id, std::string strsavefile_voice_recorder);

  /* ThreadMsgPriorityQueue thread_msgqueue_asr_request_;  ///< message priority queue for request
   ThreadMsgPriorityQueue thread_msgqueue_tts_request_;  ///< message priority queue for request
   ThreadMsgPriorityQueue thread_msgqueue_response_;     ///< message priority queue for response
   HWND hwnd_msg_invoke_;                                ///< window handle for invoke message
   ysos::AudioCaptureDriver audio_capture_driver_;       ///< voice recorder driver
   ysos::AudioPlayDriver audio_play_driver_;             ///< voice player deriver*/
  boost::atomic<int> config_voice_record_mode_;  ///< 录音模式，0 普通模式，录完音ASR后会关闭录音机. 1打断模式，默认0

  //boost::atomic<int> have_startasr_flag4_;   ///< 是否已经startasrt了，如果是，则不再开启新的startasr, 只适应于打断模式。 默认值0

  boost::mutex mutex_set_voice_service_provider_;
  //boost::atomic<int> config_linyun_kf_enable_atomic_;  ///< 启用灵云客服系统,在得到ASR后，同时调灵云客服系统和第三方NLP,默认0
  //boost::atomic<int> config_jthc_version_mode_atomic_;  ///< 捷通华声版本，0 正式版，1银行演示版，默认0

  ScenesInterfacePtr scenes_interface_ptr_;
  ScenesInterfacePtr scenes_interface_ptr_ifly_;  ///< 在这里添加变量，是为了方便调试
  ScenesInterfacePtr scenes_interface_ptr_jthc_;  ///< 在这里添加变量，是为了方便调试

  boost::atomic<int> config_need_save_voice_file_;  ///< 是否保存音频文件
  std::string config_save_vocie_filepath_;  ///< 保存音频文件的目录
  std::string config_save_voicefile_asr_file_path_;  ///< 保存音频文件对应的asr结果
  FILE* psavefile_voice_recorder_;  ///< 为了测试用，输出录音音频文件
  boost::mutex mutex_file_voice_corresponse_asr_result_;
  //;保存空语音识别的音频的模式，默认是0，0表示不保存语音识别为空的结果，1表示不保存小于3个字符的识别结果(参数voice_filter_min_character控制），2保存所有的音频，包含空语音识别结果
  int  config_save_empty_voice_mode_;
  // ;是否保存asrnlp结果，默认是0，如果1则除了会保存关联的asr，还会保存asrnlp结果
  int config_save_asrnlp_result_;
  std::string term_id_;///<终端号，用于文件保存
  int voice_collection_mode_;///<语料收集模式0-关闭 1-开启
  std::string log_date_;///<语料记录日期
  std::string log_time_;///<语料记录时间
  std::string voice_log_filename_;///<语料记录音频文件名称
  int collection_voice_save_mode_;///<语料记录音频结果模式 0-所有 1-只记录空结果 2-只记录有识别结果 3-记录空和小于3个字符的结果
  //FILE* psavefile_voice_local_recorder_;

  boost::atomic<int> call_times_max_startasr_atomic_;  ///< 调用startasr中最大的值,默认0
  boost::atomic<int> call_times_max_stopasr_atomic_;    ///<调用STOPASR中最大的值，默认0
  boost::atomic<int> asr_filter_error_happens_atomic_;  ///< 是否过滤错误，如果是，则出错，也不上报error happens,如果是1过滤，0 不过滤。默认是0
  int config_asr_get_result_timeout_;   ///< asr获取结果超时时间（单位毫秒），默认10000毫秒。
  int is_broadcast_;                    ///< 超时是否播报信息
  char asr_timeout_broadcast_[1024];    ///< asr超时播报信息
  //boost::mutex mutex_module_optimize_;   ///<  加载模块优化，框架优化加载时使用的是另一个线程，释放资源的时候得等此线程结束才能释放
#ifdef _WIN32
  void* hevent_module_optimize_finish_;
#else
  event_handle hevent_module_optimize_finish_;
#endif 

  boost::mutex mutex_list_asr_voice_data_;
  std::list<TDVoiceDataPtr> list_voice_data_;   ///<  保存录音数据

  boost::mutex mutex_list_asr_result_;
  std::list<TDAsrResultPtr> list_asr_result_;   ///< temporary fro caching asr result;
  //boost::thread_group asr_thread_group_;

  boost::mutex mutex_asr_count_oper_;
  boost::mutex mutex_nlp_count_oper_;
  boost::mutex mutex_tts_oper_;
  /*boost::atomic<int> concurrency_asr_canuse_count_;  ///< 当前可并发的使用的数量
  boost::atomic<int> concurrency_nlp_canuse_count_;*/
 // boost::atomic<int> support_concurrency_asr_max_count_;  ///< 可以并发的最大的数量
 // boost::atomic<int> support_concurrency_nlp_max_count_;
 private:
  /**
   *@brief window (for invoke message） message callback function;
   *@param hwnd[Input] window handle
   *@param message[Input] message type
   *@param wParam[Input] callback param
   *@param lParam[Input] callback param
   *@return success:0
   *        fail:-1
   */
  //static LRESULT CALLBACK MessageWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

  /**
   *@brief get current module filepath
   *@param iMaxBufSize[Input] receive filepath buffer max size
   *@param pFilePath[Output] receive filepath buffer, "C:\TDRobot\" for example
   *@return true:get file path success
   *        false:get file path fail
   */
  bool GetCurrentModuleFilePath(int iMaxBufSize, char* pFilePath);

  /**
   *@brief read voice config
   *@param pFilePath[Input] the config file path
   *@return
   */
  void ReadConfig(const char* pFilePath);

  /**
   *@brief 模块的优化工作
   *@param iParam[Input] 阶段：1 加载OCX最后时，提前登录第三方
   *@return
   */
  void Module0ptimization(int iParam);

  int config_voice_service_provide_id_;  /// 配置文件中使用的语音服务商,  0 科大迅飞，1 捷通华声  默认是0

  boost::atomic<int> config_nlp_timeout_millseconds_atomic_;  ///< nlp默认的超时时间（同步情况下，默认5秒)

  boost::mutex mutex_construct_release_;  ///<  互斥量，构建对象和析构对象的时候使用
  static boost::atomic<int> construct_counter_atomic_;  ///<  构建对象的计数器
  static boost::atomic<int> release_counter_atomic_;   ///< 析构对象的计数器
  clock_t construct_clock_count_;  ///< 初始化时的时间。

  //boost::mutex mutex_asr_count_oper_;
  //boost::mutex mutex_nlp_count_oper_;
  //boost::atomic<int> concurrency_asr_canuse_count_;  ///< 当前可并发的使用的数量
  //boost::atomic<int> concurrency_nlp_canuse_count_;
  //boost::atomic<int> support_concurrency_asr_max_count_;  ///< 可以并发的最大的数量
  //boost::atomic<int> support_concurrency_nlp_max_count_;
  ///< 打断模式下，最大可允许的出错次数，超过这个次数，则上报错误,默认值为2
  boost::atomic<int> config_asr_allow_continuous_max_error_count_;
  boost::atomic<int > asr_current_continuous_error_count_;
  boost::atomic<int> config_asr_allow_continuous_max_error_miliseconds_;
  boost::atomic<int> asr_current_error_clock_t_;

  /*boost::thread_group*/boostex::thread_group thread_group_all_;

  std::string g_strModuleFilepath;
  std::list<std::string> list_asr_no_filter_;

  boost::atomic<int> config_voice_filter_min_characters_atomic_;  ///< 如果是打断模式下，则过滤的字符数。默认是6（即3个中文字）
  
 public:
 
  int error_times_;
   std::list<std::string> list_tts_broadcast_filter_;
 };

//OBJECT_ENTRY_AUTO(__uuidof(XFAsr), CXFAsr)
//
#endif  ///< XFASR_XFASR_H  //NOLINT
