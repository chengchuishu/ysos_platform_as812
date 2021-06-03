#ifndef TDVOICE_INTERFACE_IFLY_H
#define TDVOICE_INTERFACE_IFLY_H


/// stl headers
#include <map>
#include <string>
#include <vector>

/// boost headers
#include <boost/atomic/atomic.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/lock_guard.hpp>

/// private headers
#include "../include/voiceinterface.h"
#include "../../../public/include/common/commonstructdefine.h"

struct BuildGrammarStatusData {
  std::string using_grammar_filenames;

  int     build_fini;  //标识语法构建是否完成, 0 未知，1完成
  int     update_fini; //标识更新词典是否完成  0 未知，1完成
  int     errcode; //记录语法构建或更新词典回调错误码
  std::string    grammar_id; //保存语法构建返回的语法ID
  std::string build_result_message;

  boost::mutex mutex_member_;
  BuildGrammarStatusData() {
    build_fini =0;
    update_fini =0;
    errcode =0;
    grammar_id ="";
  }
  void ClearStatus() {
    boost::lock_guard<boost::mutex> guard(mutex_member_);
    build_fini =0;
    update_fini =0;
    errcode =0;
    grammar_id ="";
  }
};
typedef boost::shared_ptr<BuildGrammarStatusData> BuildGrammarStatusDataPtr;

struct BuildGrammarFiles {
  std::string grammar_file_path;
  FILE* pfile;
  int file_size;
  int seek_index;

  std::string ansi_text;
  BuildGrammarFiles() {
    pfile = NULL;
    file_size = 0;
    seek_index =0;
  }
  void ClearStatus() {

  }
};




class VoiceInterfaceIFLY:public virtual VoiceInterface{
 public:
  VoiceInterfaceIFLY();
  virtual ~VoiceInterfaceIFLY();
  virtual int Login();
  virtual int Logout();
  virtual bool IsHaveLoginIn();
  virtual int ReadConfig(const char* pFilePath);
  
  virtual int ASRSessionBegin(const int lFlag,std::string& ReceiveSeesionID);
  virtual int ASRAudioWrite(const char* sessionID, const void* waveData, unsigned int waveLen, int audioStatus, int *epStatus, int *recogStatus);
  virtual int ASRGetResult(const char* sessionID, int* rsltStatus,std::string& ReceiveResult);
  virtual int ASRSessionEnd(const char* sessionID);
  virtual int ASRSetAccent(const char* accent_name);
  virtual int ASRGetAccent(std::string& accent_name);
  virtual int ASRSetLanguage(const char* language);
  virtual int ASRGetLanguage(std::string& language);
  virtual int ASRSetEngineType(const char* enginetype);
  virtual int ASRGetEngineType(std::string& enginetype);
  virtual int ASRSetGrammarFile(const char* grammarfilename, std::string* error_message);
  virtual int ASRGetGrammarFile(std::string& grammarfilesname);
  
  virtual int NLP(const char* text,std::string& ReceiveNLPResult);

  virtual int TTSSessionBegin(std::string& ReceiveSeesionID);
  virtual int TTSTextPut( const char* pSessionID, const char* pTextString, unsigned int iTextLen, const char* pParams );
  virtual int TTSAudioGet( const char* pSessionID, TDCharBufPtr& RecviveAudioDataPtr, unsigned int* piAudioLen, int* piSynthStatus);
  virtual int TTSVoicePlayMode();
  virtual int TTSSessionEnd( const char* pSessionID, const char* pHints );
  virtual int TTSSetLanguage(const char* language);
  virtual int TTSGetLanguage(std::string& language);
  virtual int TTSSetEffect(const int effect);
  virtual int TTSGetEffect(int& effect);
  virtual int TTSSetSpeaker(const char* speaker);
  virtual int TTSGetSpeaker(std::string& speaker);
  virtual int TTSSetEngineType(const char* enginetype);
  virtual int TTSGetEngineType(std::string& enginetype);

 protected:
   /**
    *@brief 上传用户词表，为了添加特定词的识别率
    *@param 
    *@return 上传成功 return YSOS_ERROR_SUCCESS
    *        上传失败 return YSOS_ERROR_FAIL
    */
  virtual int UploadUserwords(std::string* pReceErrorMsg);

  /**
   *@brief 解析迅飞的参数字符串到map中
   *       迅飞参数的格式，以逗事情分开
   *       例如：ngine_type = local, text_encoding =GBK, 
   *@param 
   *@return   
   */
  int DecodeIflyKeyParam(const std::string set_param_string, std::map<std::string, std::string>& map_params);
  std::string GetIflyKeyParamString(const std::map<std::string, std::string>& map_params);

  int ASRSetKeyParam(const char* key, const char* param);
  int ASRGetKeyParam(const char* key, std::string& param);

  int TTSSetKeyParam(const char* key, const char* param);
  int TTSSetKeyParams(const std::map<std::string, std::string>& map_key_param);
  int TTSGetKeyParam(const char* key, std::string& param);

  //std::string GetAsrSessionConnString(const int flag);
  std::string SetAsrSessionConnMap(const int flag);
  int BuildGrammar(std::string grammarfiles, std::string& error_message);
  //int build_grammar_fily(const char* grm_content, const int grm_cnt_len, UserData *udata,std::string error_message);
  void LogGrammarToTempFile(const char* utf8_logstr, const int loglen, const char* grammar_new_filename);
  void MergeGrammar(char* grm_content, int& gram_all_seek_index, BuildGrammarFiles& gramfileinfo, std::string cmp_string, bool append_data, bool* hit_cmp_string = NULL, std::string* proc_line_string = NULL);

  std::string GetNewGrammarName(const char* grammar_files);

  /**
   *@brief 更新appleid,如果已登录，则重新登录
   *@param strAppID[Input] 新的appid
   *@return 上传成功 return YSOS_ERROR_SUCCESS
   *        上传失败 return YSOS_ERROR_FAIL
   */
  //virtual int SetAppID(std::string strAppID);

  // 这里呢不设置为static， 以便多个应用都可以使用此语音模块
  /*static*/ boost::atomic<int> have_login_result_atomic_;
  /*static*/ boost::mutex mutex_voice_service_provider_login_;  // mutex for login in voice service provider
 
  std::string read_config_file_path_;
  std::string voice_config_file_path_;

  boost::mutex mutex_member_;
  std::string login_params_;

  std::string asr_using_accent_;
  std::string asr_using_engine_type_;  ///, ;asr识别使用的类型，默认是cloud，;cloud 在线，local 离线，mixed 混合
  //BuildGrammarStatusData asr_using_grammar_status_data_;
  std::string asr_using_grammar_filename_;
  std::map<std::string, BuildGrammarStatusDataPtr> map_asr_using_grammar_data_;
  std::map<std::string, std::string> map_asr_using_sessionbegin_;

  boost::mutex mutex_asr_ilfy_interface_call_;
  std::map<std::string, std::string> map_asr_cloud_param_sessionbegin_common_;
  std::map<std::string, std::string> map_asr_cloud_param_sessionbegin_flag0_;
  std::map<std::string, std::string> map_asr_cloud_param_sessionbegin_flag2_;
  std::map<std::string, std::string> map_asr_cloud_param_sessionbegin_flag3_;
  std::map<std::string, std::string> map_asr_cloud_param_sessionbegin_flag4_;
  std::map<std::string, std::string> map_asr_cloud_param_sessionbegin_flag5_;
  std::map<std::string, std::string> map_asr_mixed_plus_param_;
  std::map<std::string, std::string> map_asr_local_plus_param_;

  std::map<std::string, std::string> map_asr_build_grammar_;
  std::string config_asr_default_grammar_filepath_;  
  std::string config_asr_default_grammar_filename_;

  std::string nlp_param_;

  //std::string tts_param_;
  std::map<std::string, std::string> map_tts_param_;
  std::map<std::string, std::string> map_tts_param_zh_; ///< 中文合成参数
  std::map<std::string, std::string> map_tts_param_en_; ///< 英文合成参数
  std::map<std::string, std::string> map_tts_param_zh_cantonese_;   ///< 粤词合成参数

  std::string tts_current_langluage_;
  std::string tts_using_engine_type_;
  std::string tts_using_speaker_;

  std::map<std::string, TTSSpeakerInfoPtr> map_tts_speaker_info_;

  boost::mutex mutex_tts_voice_blocks_;
  TTSVoiceDataBlock caching_voice_block;

  boost::timed_mutex nlp_mutex;
  //TDStatisticsInterfacePtr statistics_ptr_;  ///< 统计接口
};

#endif
