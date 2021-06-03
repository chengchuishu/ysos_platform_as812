#ifndef TDVOICE_INTERFACE_H
#define TDVOICE_INTERFACE_H

#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>

/**
 *  MSPSampleStatus indicates how the sample buffer should be handled
 *  MSP_AUDIO_SAMPLE_FIRST  The sample buffer is the start of audio
 *  If recognizer was already recognizing, it will discard
 *  audio received to date and re-start the recognition
 *  MSP_AUDIO_SAMPLE_CONTINUE The sample buffer is continuing audio
 *  MSP_AUDIO_SAMPLE_LAST - The sample buffer is the end of audio
 * The recognizer will cease processing audio and
 *  return results
 *  Note that sample statii can be combined; for example, for file-based input
 *  the entire file can be written with SAMPLE_FIRST | SAMPLE_LAST as the
 *  status.
 *  Other flags may be added in future to indicate other special audio
 *  conditions such as the presence of AGC
 */
enum TDMSP_AUDIO_STATUS {
  TDMSP_AUDIO_SAMPLE_INIT           = 0x00,
  TDMSP_AUDIO_SAMPLE_FIRST          = 0x01,
  TDMSP_AUDIO_SAMPLE_CONTINUE       = 0x02,
  TDMSP_AUDIO_SAMPLE_LAST           = 0x04,
};

/*
 *  The enumeration MSPRecognizerStatus contains the recognition status
 *  MSP_REC_STATUS_SUCCESS - successful recognition with partial results
 *  MSP_REC_STATUS_NO_MATCH - recognition rejected
 *  MSP_REC_STATUS_INCOMPLETE  recognizer needs more time to compute results
 *  MSP_REC_STATUS_NON_SPEECH_DETECTED- discard status, no more in use
 *  MSP_REC_STATUS_SPEECH_DETECTED- recognizer has detected audio, this is delayed status
 *  MSP_REC_STATUS_COMPLETE- recognizer has return all result
 *  MSP_REC_STATUS_MAX_CPU_TIME- CPU time limit exceeded
 *  MSP_REC_STATUS_MAX_SPEECH- maximum speech length exceeded, partial results may be returned
 *  MSP_REC_STATUS_STOPPED- recognition was stopped
 *  MSP_REC_STATUS_REJECTED- recognizer rejected due to low confidence
 *  MSP_REC_STATUS_NO_SPEECH_FOUND- recognizer still found no audio, this is delayed status
 */
enum TDMSP_REC_STATUS {
  TDMSP_REC_STATUS_SUCCESS              = 0,
  TDMSP_REC_STATUS_NO_MATCH             = 1,
  TDMSP_REC_STATUS_INCOMPLETE= 2,
  TDMSP_REC_STATUS_INCOMPLETE_NEED_NLP  = 102,   ///< 还有结果待识别，待识别的结果是还需要再次nlp解析
  TDMSP_REC_STATUS_NON_SPEECH_DETECTED  = 3,
  TDMSP_REC_STATUS_SPEECH_DETECTED      = 4,
  TDMSP_REC_STATUS_COMPLETE = 5,
  TDMSP_REC_STATUS_MAX_CPU_TIME         = 6,
  TDMSP_REC_STATUS_MAX_SPEECH           = 7,
  TDMSP_REC_STATUS_STOPPED              = 8,
  TDMSP_REC_STATUS_REJECTED             = 9,
  TDMSP_REC_STATUS_NO_SPEECH_FOUND      = 10,
  TDMSP_REC_STATUS_FAILURE = TDMSP_REC_STATUS_NO_MATCH,
};

/**
 * The enumeration MSPepState contains the current endpointer state
 *  MSP_EP_LOOKING_FOR_SPEECH Have not yet found the beginning of speech
 *  MSP_EP_IN_SPEECH- Have found the beginning, but not the end of speech
 *  MSP_EP_AFTER_SPEECH- Have found the beginning and end of speech
 *  MSP_EP_TIMEOUT- Have not found any audio till timeout
 *  MSP_EP_ERROR- The endpointer has encountered a serious error
 *  MSP_EP_MAX_SPEECH- Have arrive the max size of speech
 */
enum  TDMSP_EP_STATUS {
  TDMSP_EP_LOOKING_FOR_SPEECH   = 0,
  TDMSP_EP_IN_SPEECH            = 1,
  TDMSP_EP_AFTER_SPEECH         = 3,
  TDMSP_EP_TIMEOUT              = 4,
  TDMSP_EP_ERROR                = 5,
  TDMSP_EP_MAX_SPEECH           = 6,
  TDMSP_EP_IDLE                 = 7  // internal state after stop and before start
};

/* Synthesizing process flags */
enum {
  TDMSP_TTS_FLAG_ERROR         = -1,  ///< wff add type
  TDMSP_TTS_FLAG_STILL_HAVE_DATA        = 1,
  TDMSP_TTS_FLAG_DATA_END               = 2,
  TDMSP_TTS_FLAG_CMD_CANCELED           = 4,

};

/**
  *@brief TTS合成后声音，声音播放模式
  */
enum TDTTS_VOICE_PLAY_MODE {
  TDVOICE_PLAY_MODE_CACHING  =0,  ///< 先缓存再播
  TDVOICE_PLAY_MODE_IMMEDIATELY =1, ///< 立即播
};

//
//enum TDMSP_ERRORTYPE {
//  TDMSP_SUCCESS= 0,
//  TDMSP_ERROR_FAIL= -1,
//  TDMSP_ERROR_EXCEPTION= -2,
//};

/**
  *@brief 方言类型
  */
#define TDACCENT_MANDARIN_STRING    "mandarin"      ///< 普通语
#define TDACCENT_CANTONESE_STRING   "cantonese"     ///< 粤语
#define TDACCENT_SICHUAN_STRING     "lmz"           ///< 四川话
#define TDACCENT_HENAN_STRING       "henanese",     ///< 河南话
#define TDACCENT_DONGBEIESE_STRING  "dongbeiese",   ///< 东北话

/**
  *@brief 语言 
  */
#define TDLANGUAGE_ZHCN_STRING "zh_cn"    ///< 简体中文
#define TDLANGUAGE_ZHTW_STRING "zh_tw"    ///< 繁体中文
#define TDLANGUAGE_ENUS_STRING "en_us"    ///< 英文
#define TDLANGUAGE_ZHCANTONESE_STRING "zh_cantonese"     ///< 粤语


#define TDTTS_AGE_CHILD      "child"
#define TDTTS_AGE_YOUNG      "young"
#define TDTTS_AGE_OLD        "old"

#define TDTTS_GENDER_MALE     "male"
#define TDTTS_GENGER_FEMALE   "female"

#define TDTTS_ENGINETYPE_LOCAL  "local"
#define TDTTS_ENGINETYPE_CLOUD "cloud"

/**
  *@brief TTS合成音效
  */


typedef boost::shared_array<char> TDCharBufPtr;

struct TTSVoiceDataBlock {
 public:
  int tts_flag_;
  TDCharBufPtr data_ptr_;
  unsigned int data_size_;
  TTSVoiceDataBlock() {
    tts_flag_ = TDMSP_TTS_FLAG_ERROR;
    data_ptr_ = NULL;
    data_size_ =0;
  }
};

struct TTSSpeakerInfo{
  std::string name;
  std::string accent;
  std::string language;
  std::string age;
  std::string gender;
  std::string engingtype;
  std::string speed;
};
typedef boost::shared_ptr<TTSSpeakerInfo> TTSSpeakerInfoPtr;

class VoiceInterface {
 public:
  VoiceInterface() {}
  virtual ~VoiceInterface() {}
  virtual int ReadConfig(const char* pFilePath) =0;
  /**
   *@brief login in third party voice service provider
   *@param
   *@return success rturn YSOS_ERROR_SUCCESS
   *        fail return YSOS_ERROR_FAIL
   */
  virtual int Login() =0;
  /**
   *@brief logout third party voice service provider
   *@param
   *@return success rturn YSOS_ERROR_SUCCESS
   *        fail return YSOS_ERROR_FAIL
   */
  virtual int Logout() =0;

  /**
   *@brief is  had successes login in third party voice service provider
   *@param
   *@return  true if had success login in
   *         false if not had success login in
   */
  virtual bool IsHaveLoginIn() =0;
  /**
   *@brief begin a asr session
   *@param lFlag[Input]  begin a asr session in which way
   *@param ReceiveSeesionID[Output] receive a asr sessionid if begin asr session successful
   *@return success rturn YSOS_ERROR_SUCCESS
   *        fail return YSOS_ERROR_FAIL
   */
  virtual int ASRSessionBegin(const int lFlag, std::string& ReceiveSeesionID) =0;

  /**
   *@brief send local record voice data to third party voice service provider
   *@param sessionID[Input] asr begin session. you can get it from function ASRSessionBegin;
   *@param waveData[Input] local record voice data pointer
   *@param waveLen[Input] local record voice data length
   *@param audioStatus[Input] local record voice date status, you can reference enum TDMSP_AUDIO_STATUS to find out specific instructions
   *@param epStatus[Output] receive third party voice service provider speed end point detection status. you can reference enum TDMSP_EP_STATUS to find out specific instructions
   *@param recogStatus[Output] receive third party voice service provider voice recognition status, you can reference enum TDMSP_REC_STATUS to find out specific instructions
   *@return success return YSOS_ERROR_SUCCESS
   *        fail return YSOS_ERROR_FAIL
   */
  virtual int ASRAudioWrite(const char* pSessionID, const void* pWaveData, unsigned int iWaveLen, int iAudioStatus, int *pEpStatus, int *pRecogStatus) =0;
  /**
   *@brief get result
   *@param rsltStatus [Output] enum TDMSP_REC_STATUS type value
   *@param errorCode[Output] enum TDMSP_ERRORTYPE type value
   *@param ReceiveResult[Output] 语音识别的结果，注意，此参数返回的结果不仅仅是当次获取的部分结果，而是此次获取前获取到的所有的结果的总合。
   *@return success rturn YSOS_ERROR_SUCCESS
   *        fail return YSOS_ERROR_FAIL
   */
  virtual int ASRGetResult(const char* pSessionID, int* piResultStatus, std::string& ReceiveResult) =0;

  /**
   *@brief end a asr session
   *@param sessionID[Input] end the specific sessionID session, which you get it from function ASRSessionBegin
   *@return success return YSOS_ERROR_SUCCESS
   *        fail return YSOS_ERROR_FAIL
   */
  virtual int ASRSessionEnd(const char* pSessionID) =0;

  /**
   *@brief 使用ASR的方言，默认是普通语
   *@param iAccentID 方言的ID, 参考 方言类型字符串
   *@return 设置成功 return YSOS_ERROR_SUCCESS
   *        设置失败 return YSOS_ERROR_FAIL
   */
  virtual int ASRSetAccent(const char* accent_name) =0;

  /**
   *@brief 获取当前的方言
   *@param
   *@return  当前使用的方言
   */
  virtual int ASRGetAccent(std::string& accent_name) =0;

  /**
   *@brief 设置识别语言，可以参考宏定义相关：TDLANGUAGE_ZHCN_STRING
   *@param 
   *@return   
   */
  virtual int ASRSetLanguage(const char* language) =0;
  virtual int ASRGetLanguage(std::string& language)=0;

  /**
   *@brief 设置识别引擎类型，主要有：local,cloud,mixed
   *@param 
   *@return   
   */
  virtual int ASRSetEngineType(const char* enginetype) =0;
  virtual int ASRGetEngineType(std::string& enginetype) =0;

  virtual int ASRSetGrammarFile(const char* grammarfilename, std::string* error_message) =0;
  virtual int ASRGetGrammarFile(std::string& grammarfilesname) =0;

  /**
   *@brief NLP process
   *@param pTextNeedNlp[Input] text need to nlp process
   *@param filter_null_result[Input] 是否过滤无效结果，如果是，则如果nlp无效，则返回空字符串
   *@param ReceiveNLPResult[Output] receive nlp result
   *@return success rturn YSOS_ERROR_SUCCESS
   *        fail return YSOS_ERROR_FAIL
   */
  virtual int NLP(const char* pTextNeedNlp, std::string& ReceiveNLPResult) =0;

  /**
   *@brief MSPLogin成功后开始一路TTS会话，并在参数中指明本路会话用到的参数。
   *@param params[Input] 本路TTS会话使用的参数，可以设置的参数及其取值范围请参考《可设置参数列表_MSC50.xls》。
   *@param ReceiveSeesionID[Output] 合建会话成功后的sessionid
   *@return success rturn YSOS_ERROR_SUCCESS
   *        fail return YSOS_ERROR_FAIL
   */
  virtual int TTSSessionBegin(std::string& ReceiveSeesionID) =0;

  /**
   *@brief 写入要合成的文本
   *@param pSessionID[Input] 由TTSSessionBegin返回过来的会话ID。
   *@param pTextString[Input] 将要进行合成的文本。
   *@paaram iTextLen[Input] 合成文本长度，单位字节。
   *@param pParams[Input] 本次合成所用的参数，只对本次合成的文本有效。
   *@return success rturn YSOS_ERROR_SUCCESS
   *        fail return YSOS_ERROR_FAIL
   */
  virtual int TTSTextPut(const char* pSessionID, const char* pTextString, unsigned int iTextLen, const char* pParams)=0;

  /**
   *@brief 获取合成音频。
   *@param pSessionID[Input] 由TTSSessionBegin返回过来的会话ID
   *@param ppRecvAudioData[Onput] 接收音频数据
   *@param iAudioLen[Onput] 合成音频长度，单位字节。
   *@param iSynthStatus[Onput] 合成音频状态，可能的值如下：
   *                            TDMSP_TTS_FLAG_STILL_HAVE_DATA = 1 音频还没取完，还有后继的音频
   *                            TDMSP_TTS_FLAG_DATA_END = 2 音频已经取完
   *@return success rturn YSOS_ERROR_SUCCESS
   *        fail return YSOS_ERROR_FAIL
   */
  virtual int TTSAudioGet(const char* pSessionID, TDCharBufPtr& RecviveAudioDataPtr, unsigned int* piAudioLen, int* piSynthStatus) =0;

  /**
   *@brief TTS 得到的音频数据的 播放模式，
   *@param
   *@return 播放模式，可以参考 enum TDTTS_VOICE_PLAY_MODE
   */
  virtual int TTSVoicePlayMode() =0;


  /**
   *@brief 结束一路TTS会话
   *@param pSessionID[Input]
   *@param pHints[Input]结束本次会话的原因描述，用于记录日志，便于用户查阅或者跟踪某些问题。
   *@return success rturn YSOS_ERROR_SUCCESS 由QTTSSessionBegin返回过来的会话ID
   *        fail return YSOS_ERROR_FAIL
   */
  virtual int TTSSessionEnd(const char* pSessionID, const char* pHints) =0;

  ///**
  // *@brief 是否支持并发
  // *@param pServiceType[Input] 服务类型，可以为“asr","nlp","tts"
  // *@return return true if support
  // *        return false if not support
  // */
  //virtual bool IsSupportConcurrency(const char* pServiceType) =0;
  ///**
  // *@brief 支持并发的数量
  // *@param
  // *@return 支持的并发数
  // */
  //virtual int SupportConcurrencyCount(const char* pServiceType) =0;


  //virtual int SetStatisticsInterface(TDStatisticsInterfacePtr statistics_ptr) {return YSOS_ERROR_FAILED;}

  virtual int TTSSetLanguage(const char* language) =0;
  virtual int TTSGetLanguage(std::string& language) =0;
  virtual int TTSSetEffect(const int effect) =0;
  virtual int TTSGetEffect(int& effect) =0;
  virtual int TTSSetSpeaker(const char* speaker) =0;
  virtual int TTSGetSpeaker(std::string& speaker) =0;
  virtual int TTSSetEngineType(const char* enginetype) = 0;
  virtual int TTSGetEngineType(std::string& enginetype) = 0;
};

typedef boost::shared_ptr<VoiceInterface> VoiceInterfacePtr;


#endif
