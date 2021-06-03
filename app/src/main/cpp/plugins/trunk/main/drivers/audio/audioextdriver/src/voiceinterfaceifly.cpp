
//#include "stdafx.h"
#include "../include/voiceinterfaceifly.h"

#ifdef _WIN32
/// windows headers
#include <Windows.h>
#else
#endif

/// boost headers
#include <boost/locale/conversion.hpp>
#include <boost/locale.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/shared_array.hpp>

//#include "error.h"  //NOLINT
#include "../../../../../thirdparty/ifly/include/qisr.h"  //NOLINT
#include "../../../../../thirdparty/ifly/include/msp_cmn.h"  //NOLINT
#include "../../../../../thirdparty/ifly/include/msp_errors.h"  //NOLINT
#include "../../../../../thirdparty/ifly/include/qtts.h"

/*#include "../include/tracelog.h"*/
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"

//#include "stringext.h"
//boost::atomic<int> VoiceInterfaceIFLY::have_login_result_atomic_(0);
//boost::mutex VoiceInterfaceIFLY::mutex_voice_service_provider_login_;


//////////////////////////////////////////////////////////////////////////
void my_recog_result_ntf_handler(const char *sessionID, const char *result, int resultLen, int resultStatus, void *userData) {
  //g_Log.Trace_Print(TraceLogLevel_INFO, __FILE__, __LINE__, "for test: result_ntf_handler result:%s",  result ? result :"");
}
void my_recog_status_ntf_handler(const char *sessionID, int type, int status, int param1, const void *param2, void *userData) {
  //g_Log.Trace_Print(TraceLogLevel_INFO, __FILE__, __LINE__, "for test: status_ntf_handler sessionid:%s", sessionID ? sessionID: "");
}
void my_recog_error_ntf_handler(const char *sessionID, int errorCode,	const char *detail, void *userData) {
  // g_Log.Trace_Print(TraceLogLevel_INFO, __FILE__, __LINE__, "for test: result_ntf_handler sessionid:%s,", sessionID ? sessionID: "");
}


//////////////////////////////////////////////////////////////////////////

int VoiceInterfaceIFLY::ASRSessionBegin(const int lFlag, std::string& ReceiveSeesionID) {
  std::string sessionbegin_params;

  sessionbegin_params = SetAsrSessionConnMap(lFlag);

  int errorCode = MSP_SUCCESS;
  YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "QISRSessionBegin param;" << sessionbegin_params.c_str());
#ifdef _DEBUGVIEW
  ::OutputDebugString((std::string("ysos.asrext QISRSessionBegin") + sessionbegin_params).c_str());
#endif

  clock_t begin_clock_for_statistics =0;  ///<  for statistics

  boost::lock_guard<boost::mutex> asr_call_guard(mutex_asr_ilfy_interface_call_);
  const char* psession_id = QISRSessionBegin(NULL, /*params*/sessionbegin_params.c_str(), &errorCode);

  if (NULL == psession_id || MSP_SUCCESS != errorCode) {
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "QISRSessionBegin errorcode:" << errorCode);
    assert(false);
    return YSOS_ERROR_FAILED;
  }

  ReceiveSeesionID = psession_id;
  return YSOS_ERROR_SUCCESS;
}


std::string VoiceInterfaceIFLY::SetAsrSessionConnMap(const int flag) {
  boost::lock_guard<boost::mutex> guard(mutex_member_);
  switch (flag) {
  case 0:
    map_asr_using_sessionbegin_  =map_asr_cloud_param_sessionbegin_flag0_;
    break;
  case 2:
    map_asr_using_sessionbegin_ = map_asr_cloud_param_sessionbegin_flag2_;
    break;
  case 3:
    map_asr_using_sessionbegin_ =map_asr_cloud_param_sessionbegin_flag3_;
    break;
  case 4:
    map_asr_using_sessionbegin_ = map_asr_cloud_param_sessionbegin_flag4_;
    break;
  case 5:
    map_asr_using_sessionbegin_ = map_asr_cloud_param_sessionbegin_flag5_;
    break;
  default:
    map_asr_using_sessionbegin_ = map_asr_cloud_param_sessionbegin_common_;
  }

  map_asr_using_sessionbegin_["language"] = TDLANGUAGE_ZHCN_STRING;
  map_asr_using_sessionbegin_["accent"] = asr_using_accent_;

  if (strcasecmp(asr_using_engine_type_.c_str(), "local") ==0) {
    std::map<std::string, std::string>::iterator it_plus = map_asr_local_plus_param_.begin();
    for (it_plus; it_plus != map_asr_local_plus_param_.end(); ++it_plus) {
      map_asr_using_sessionbegin_[it_plus->first] = it_plus->second;
    }
    ///<离线版本只支持普通话
    map_asr_using_sessionbegin_["accent"] = TDACCENT_MANDARIN_STRING;
  } else if (strcasecmp(asr_using_engine_type_.c_str(),"mixed") ==0) {
    std::map<std::string, std::string>::iterator it_plus = map_asr_mixed_plus_param_.begin();
    for (it_plus; it_plus != map_asr_mixed_plus_param_.end(); ++it_plus) {
      map_asr_using_sessionbegin_[it_plus->first] = it_plus->second;
    }

    if (2 == flag || 3 == flag || 5 == flag) {
      // remove nlp_version=0配置
      //map_asr_using_sessionbegin_["nlp_version"]="";
      //map_asr_using_sessionbegin_["scene"]="";
    }
  }

  // std::string temp_asr_session_string = GetIflyKeyParamString(tmp_using_asr_session_param);

  if ((strcasecmp(asr_using_engine_type_.c_str(), "local") ==0 ||strcasecmp(asr_using_engine_type_.c_str(),"mixed") ==0)
      && asr_using_grammar_filename_.length() >0) {
    //添加语法id
    BuildGrammarStatusDataPtr grammar_ptr = map_asr_using_grammar_data_[asr_using_grammar_filename_];
    if (grammar_ptr && grammar_ptr->grammar_id.length()) {
      //temp_asr_session_string += std::string(",local_grammar =") + grammar_ptr->grammar_id;
      map_asr_using_sessionbegin_["local_grammar"] = grammar_ptr->grammar_id;
    } else {
      assert(false);  ///< grammar file must exist and load correct
    }
  }
  return GetIflyKeyParamString(map_asr_using_sessionbegin_);
}



int VoiceInterfaceIFLY::ASRAudioWrite(const char* sessionID, const void* waveData, unsigned int waveLen, int audioStatus, int *epStatus, int *recogStatus) {

  boost::lock_guard<boost::mutex> asr_call_guard(mutex_asr_ilfy_interface_call_);
  int iret = QISRAudioWrite(sessionID, waveData, waveLen, audioStatus, epStatus, recogStatus);

  if (MSP_SUCCESS == iret) {
    return YSOS_ERROR_SUCCESS;
  } else {

    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "ASRAudioWrite errorcode:" << iret);

    if (iret >= 10200 && iret <10300) { /// Error codes of network
      for (int i=0; i<2; ++i) {
        iret = QISRAudioWrite(sessionID, waveData, waveLen, audioStatus, epStatus, recogStatus);
        if (MSP_SUCCESS == iret)
          return MSP_SUCCESS;
        //assert(false);
        YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "ifly ASRAudioWrite 迅飞10202 错误 , 已重发" << i << "次 retcode:" << iret);
      }
    }
#ifdef _DEBUGVIEW
    char msg_buf[1024] = {'\0'};
    sprintf_s(msg_buf, sizeof(msg_buf), "ysos.asrext QISRAudioWrite, audioStatus:%d, epStatus:%d,recogStatus:%d", audioStatus, (epStatus ? *epStatus : -1), (recogStatus ? *recogStatus :-1));
    ::OutputDebugString(msg_buf);
#endif
    return YSOS_ERROR_FAILED;
  }
}

int VoiceInterfaceIFLY::ASRGetResult(const char* sessionID, int* rsltStatus,std::string& ReceiveResult) {
  int errorCode = MSP_SUCCESS;
  boost::lock_guard<boost::mutex> asr_call_guard(mutex_asr_ilfy_interface_call_);
  const char* presult = QISRGetResult(sessionID,rsltStatus,/*waitTime*/0, &errorCode);
  if (NULL != presult) {
    ReceiveResult = presult;
  }
#ifdef _DEBUGVIEW
  if (ReceiveResult.length() >0) {
    ::OutputDebugString((std::string("ysos.asrext QISRGetResult:") + ReceiveResult).c_str());
  }
#endif

  if (MSP_SUCCESS == errorCode) {
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "QISRGetResult:" << errorCode);
    return YSOS_ERROR_SUCCESS;
  } else {
    assert(false);
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "QISRGetResult errorcode:" << errorCode);
    return YSOS_ERROR_FAILED;
  }
}


int VoiceInterfaceIFLY::ASRSessionEnd(const char* sessionID) {
  boost::lock_guard<boost::mutex> asr_call_guard(mutex_asr_ilfy_interface_call_);
  int ierror_code = QISRSessionEnd(sessionID, NULL);
  YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "QISRSessionEnd sessionid:" << sessionID << ", errorcode:" << ierror_code);
#ifdef _DEBUGVIEW
  ::OutputDebugString((std::string("ysos.asrext QISRSessionEnd") + (sessionID ? sessionID: "")).c_str());
#endif

  if (MSP_SUCCESS == ierror_code)
    return YSOS_ERROR_SUCCESS;
  else {
    assert(false);
    return YSOS_ERROR_FAILED;
  }
}

int VoiceInterfaceIFLY::ASRSetAccent(const char* accent_name) {
  //如果当前不是中文，则设置无效
  std::string current_language;
  int rc = ASRGetKeyParam("language", current_language);
  if (current_language == "") {
    current_language = TDLANGUAGE_ZHCN_STRING;
    ASRSetLanguage(TDLANGUAGE_ZHCN_STRING);
  }
  {
    boost::lock_guard<boost::mutex> guard(mutex_member_);
    if (current_language.length() >0 &&  strcasecmp(current_language.c_str(), TDLANGUAGE_ZHCN_STRING) !=0
        && accent_name
        && strlen(accent_name) >0) {
      return YSOS_ERROR_FAILED;
    }
  }

  rc = ASRSetKeyParam("accent", accent_name);
  if (rc == YSOS_ERROR_SUCCESS) {
    boost::lock_guard<boost::mutex> guard(mutex_member_);
    asr_using_accent_ = accent_name;
    // write to ini files
    //WritePrivateProfileString("IFLY", "asr_using_accent", accent_name, voice_config_file_path_.c_str());
  }
  return rc;
}


int VoiceInterfaceIFLY::ASRGetAccent(std::string& accent_name) {
  boost::lock_guard<boost::mutex> guard(mutex_member_);
  accent_name = asr_using_accent_;
  return YSOS_ERROR_SUCCESS;
}


int VoiceInterfaceIFLY::ASRSetLanguage(const char* language) {
  //如果不是中文，则方言为空
  if (strcasecmp(language, TDLANGUAGE_ZHCN_STRING) !=0) {
    ASRSetKeyParam("accent", "");
  }
  return ASRSetKeyParam("language", language);
}
int VoiceInterfaceIFLY::ASRGetLanguage(std::string& language) {
  int rc = ASRGetKeyParam("language", language);
  if (language.length() <=0) {
    language = TDLANGUAGE_ZHCN_STRING;
  }
  return rc;
}


int VoiceInterfaceIFLY::ASRSetEngineType(const char* enginetype) {
  YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "ifly set engine type:" << (enginetype ? enginetype: ""));
  if (NULL == enginetype
      || strcasecmp(enginetype, "local") ==0
      || strcasecmp(enginetype, "cloud") ==0
      || strcasecmp(enginetype, "mixed") ==0) {

  } else {
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "not support engine type:" << (enginetype ? enginetype : ""));
    return YSOS_ERROR_FAILED;
  }

  boost::lock_guard<boost::mutex> guard(mutex_member_);
  asr_using_engine_type_ = enginetype ? enginetype : "";

  // write to ini files
  //WritePrivateProfileString("IFLY", "asr_using_engine_type",(enginetype ? enginetype : ""), voice_config_file_path_.c_str());
  return YSOS_ERROR_SUCCESS;
}

int VoiceInterfaceIFLY::ASRGetEngineType(std::string& enginetype) {
  boost::lock_guard<boost::mutex> guard(mutex_member_);
  enginetype = asr_using_engine_type_;
  return YSOS_ERROR_SUCCESS;
}


int VoiceInterfaceIFLY::ASRSetGrammarFile(const char* grammarfilename, std::string* error_message) {
  YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "set grammar filename:" << (grammarfilename ? grammarfilename : ""));
  std::string tmp_grammarfiles;

  if (grammarfilename) {
    tmp_grammarfiles = grammarfilename;
  } else {
    boost::lock_guard<boost::mutex> guard(mutex_member_);
    tmp_grammarfiles = asr_using_grammar_filename_;
    YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "use default grammar file:" <<  tmp_grammarfiles.c_str());
  }
  std::string temp_error_message;

  int rc =  BuildGrammar(tmp_grammarfiles.c_str(), error_message ? *error_message : tmp_grammarfiles);

  if (YSOS_ERROR_SUCCESS == rc) {
    boost::lock_guard<boost::mutex> guard(mutex_member_);
    asr_using_grammar_filename_ = tmp_grammarfiles;

    // wirte to ini files
    //WritePrivateProfileString("IFLY", "asr_default_grammar_filename", tmp_grammarfiles.c_str(), voice_config_file_path_.c_str());
  }
  return rc;
}

int VoiceInterfaceIFLY::ASRGetGrammarFile(std::string& grammarfilesname) {
  boost::lock_guard<boost::mutex> guard(mutex_member_);
  grammarfilesname = asr_using_grammar_filename_;
  return YSOS_ERROR_SUCCESS;
}



int VoiceInterfaceIFLY::ASRSetKeyParam(const char* key, const char* param) {
  if (NULL == key || NULL ==param) {
    return YSOS_ERROR_FAILED;
  }
  if (strlen(key) ==0 /*|| strlen(param) ==0*/) {
    return YSOS_ERROR_FAILED;
  }

  boost::lock_guard<boost::mutex> lock_guard_accent(mutex_member_);
  map_asr_cloud_param_sessionbegin_common_[key] = param;
  map_asr_cloud_param_sessionbegin_flag0_[key] = param;
  map_asr_cloud_param_sessionbegin_flag2_[key] = param;
  map_asr_cloud_param_sessionbegin_flag3_[key] = param;
  map_asr_cloud_param_sessionbegin_flag4_[key] = param;
  map_asr_cloud_param_sessionbegin_flag5_[key] = param;
  map_asr_using_sessionbegin_[key] = param;

  return YSOS_ERROR_SUCCESS;
}

int VoiceInterfaceIFLY::ASRGetKeyParam(const char* key, std::string& param) {
  if (NULL == key || strlen(key) ==0) {
    return YSOS_ERROR_FAILED;
  }
  boost::lock_guard<boost::mutex> lock_guard_accent(mutex_member_);
  std::map<std::string, std::string>::iterator itfind = map_asr_using_sessionbegin_.find(key);
  if (itfind != map_asr_using_sessionbegin_.end()) {
    param = itfind->second;
    return YSOS_ERROR_SUCCESS;
  }
  return YSOS_ERROR_SUCCESS;
}


int build_grm_cb(int ecode, const char *info, void *udata) {
  BuildGrammarStatusData *grm_data = reinterpret_cast<BuildGrammarStatusData *>(udata);
  assert(grm_data);
  if (NULL == grm_data) {
    return -1;
  }
  boost::lock_guard<boost::mutex> guard(grm_data->mutex_member_);
  grm_data->build_fini = 1;
  grm_data->errcode = ecode;

  if (MSP_SUCCESS == ecode && NULL != info) {
    //printf("构建语法成功！ 语法ID:%s\n", info);
    YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),   "构建语法成功！ 语法ID:" <<  info);
    grm_data->build_result_message = std::string("构建语法成功！ 语法ID:") + info;
    if (NULL != grm_data) {
      // _snprintf(grm_data->grammar_id, MAX_GRAMMARID_LEN - 1, info);
      grm_data->grammar_id = info;
    }
  } else {
    assert(false);
    // printf("构建语法失败！%d\n", ecode);
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),   "构建语法失败！" << ecode);
    char tempbuf[1024] = {'\0'};
    //sprintf_s(tempbuf, sizeof(tempbuf), "构建语法失败！%d\n, filepath:", ecode);
    sprintf(tempbuf, "构建语法失败！%d\n, filepath:", ecode);
    grm_data->build_result_message = std::string(tempbuf) + grm_data->using_grammar_filenames;
  }
  return 0;
}

std::string VoiceInterfaceIFLY::GetNewGrammarName(const char* grammar_files) {
  if (NULL == grammar_files) {
    return "";
  }
  std::vector<std::string> vec_grammar_files;
  GetUtility()->SplitString2Vec(grammar_files, vec_grammar_files,';');

  std::string grammar_new_name;
  for (int i=0; i< vec_grammar_files.size(); ++i) {
    grammar_new_name += boost::filesystem::path(vec_grammar_files[i]).filename().replace_extension("").string();
  }
  return grammar_new_name;
}


void VoiceInterfaceIFLY::LogGrammarToTempFile(const char* utf8_logstr, const int loglen, const char* grammar_new_filename) {
  if (NULL == utf8_logstr || loglen <=0 || NULL == grammar_new_filename) {
    return;
  }

  std::string temp_parent_path = "msc/tmp";
  boost::system::error_code myecc;
  if (false == boost::filesystem::exists(temp_parent_path)) {
    boost::filesystem::create_directories(temp_parent_path, myecc);
  }

  // for test
  FILE* pfile_wirte_log =NULL;
  pfile_wirte_log = fopen(/*"call.bnf"*/(temp_parent_path + std::string("/tmp") + grammar_new_filename + ".bnf").c_str(), "w");
  if (pfile_wirte_log) {
    // // 写入UTF-8的BOM文件头
    char header[3] = {(char)0xEF, (char)0xBB, (char)0xBF};
    fwrite(header, sizeof(char), 3, pfile_wirte_log);
    fwrite(utf8_logstr, loglen, 1, pfile_wirte_log);
    fclose(pfile_wirte_log);
  }
  return;
}


void VoiceInterfaceIFLY::MergeGrammar(char* grm_content, int& gram_all_seek_index, BuildGrammarFiles& gramfileinfo, std::string cmp_string, bool append_data, bool* hit_cmp_string, std::string* proc_line_string) {
  int j =gramfileinfo.seek_index;
  bool is_begin_with_comments = false;  ///< 注释行
  bool is_begin_widh_comment_start = false;
  for (j ; j< gramfileinfo.ansi_text.length(); ++j) {
    //过滤开头的空白
    if (j == gramfileinfo.seek_index
        && (' ' == gramfileinfo.ansi_text[j] || '\r' == gramfileinfo.ansi_text[j] || '\t' == gramfileinfo.ansi_text[j]|| '\n' == gramfileinfo.ansi_text[j])) {
      gramfileinfo.seek_index =j+1;
      continue;
    }

    //过滤注释行
    if ('/' == gramfileinfo.ansi_text[j] && j < gramfileinfo.ansi_text.length() -1 && '/' == gramfileinfo.ansi_text[j+1]) {
      is_begin_with_comments = true;
    }
    if (is_begin_with_comments && '\n' == gramfileinfo.ansi_text[j]) {
      gramfileinfo.seek_index = j+1;
      continue;
      //break;
    }

    if ('/' == gramfileinfo.ansi_text[j] && j < gramfileinfo.ansi_text.length() -1 && '*' == gramfileinfo.ansi_text[j+1]) {
      is_begin_widh_comment_start = true;
    }
    if (is_begin_widh_comment_start && '/' == gramfileinfo.ansi_text[j]  && '*' == gramfileinfo.ansi_text[j-1]) {
      gramfileinfo.seek_index =j+2;
      continue;
    }

    if (';' == gramfileinfo.ansi_text[j]) {
      break;
    }
  }
  if ((j - gramfileinfo.seek_index > cmp_string.length())
      && strncasecmp(cmp_string.c_str(),  &gramfileinfo.ansi_text[gramfileinfo.seek_index], cmp_string.length()) ==0) {
    if (append_data) {
      memcpy(&grm_content[gram_all_seek_index], &gramfileinfo.ansi_text[gramfileinfo.seek_index], j - gramfileinfo.seek_index +1);
      gram_all_seek_index+= j - gramfileinfo.seek_index +1;
      grm_content[gram_all_seek_index] = '\n';
      grm_content[++gram_all_seek_index] = '\0';
    }

    if (proc_line_string) {
      *proc_line_string = gramfileinfo.ansi_text.substr(gramfileinfo.seek_index, j-gramfileinfo.seek_index +1);
    }

    gramfileinfo.seek_index = j+1;

    if (hit_cmp_string) {
      *hit_cmp_string = true;
    }
  } else {
    fseek(gramfileinfo.pfile, gramfileinfo.seek_index, SEEK_SET);
    if (hit_cmp_string) {
      *hit_cmp_string = false;
    }
  }
}


int VoiceInterfaceIFLY::BuildGrammar(std::string grammarfiles, std::string& error_message) {
  char *grm_content                        = NULL;
  //char grm_build_params[MAX_PARAMS_LEN]    = {NULL};
  int ret                                  = 0;
  int gram_all_seek_index=0;
  std::string add_string;
  std::string grammar_new_name;
  std::vector<std::string> vec_grammar_files;
  boost::system::error_code myecc;
  std::string utf8string;
  const char* gram_content_using_ptr = NULL;
  int gram_content_using_length =0;

  if (grammarfiles.length() <=0) {
    error_message = "error, the grammarfiles name is null";
    return YSOS_ERROR_FAILED;
  }

  GetUtility()->SplitString2Vec(grammarfiles.c_str(), vec_grammar_files,';');
  {
    //去掉空格
    std::vector<std::string> tmp_vec;
    std::string temp_string;
    for (int i=0; i< vec_grammar_files.size(); ++i) {
      temp_string = GetUtility()->Trim2(vec_grammar_files[i].c_str());
      if (temp_string.length() <=0) {
        continue;
      }
      tmp_vec.push_back(temp_string);
    }
    vec_grammar_files = tmp_vec;
  }

  //grammar file new name
  grammar_new_name = GetNewGrammarName(grammarfiles.c_str());


  BuildGrammarStatusDataPtr tmp_build_grammar_ptr= NULL;
  std::string grammar_build_params;
  {
    boost::lock_guard<boost::mutex> guard(mutex_member_);
    tmp_build_grammar_ptr = map_asr_using_grammar_data_[/*grammar_new_name*/grammarfiles];
    grammar_build_params = GetIflyKeyParamString(map_asr_build_grammar_);
  }

  //判断是否加载过此语法
  if (tmp_build_grammar_ptr) {
    boost::lock_guard<boost::mutex> jj_guard(tmp_build_grammar_ptr->mutex_member_);
    if (tmp_build_grammar_ptr->build_fini) {
      YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),   "have build grammar :" << grammarfiles.c_str());
      error_message = std::string("have build grammar :") + /*grammar_new_name*/grammarfiles;
      return MSP_SUCCESS == tmp_build_grammar_ptr->errcode ? YSOS_ERROR_SUCCESS : YSOS_ERROR_FAILED;
    } else {
      YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),   "is loading");
      error_message = std::string("is loading grammar") + /*grammar_new_name*/grammarfiles;
      return YSOS_ERROR_FAILED;
    }
  } else {
    tmp_build_grammar_ptr.reset(new BuildGrammarStatusData);
    tmp_build_grammar_ptr->using_grammar_filenames = /*grammarfiles*/grammarfiles;

    boost::lock_guard<boost::mutex> guard(mutex_member_);
    map_asr_using_grammar_data_[/*grammar_new_name*/grammarfiles] = tmp_build_grammar_ptr;
  }

  boost::filesystem::path path_grammar_file_parent; //(grammar_file_path);
  {
    boost::lock_guard<boost::mutex> guard(mutex_member_);
    path_grammar_file_parent.append(read_config_file_path_);
    path_grammar_file_parent.append(config_asr_default_grammar_filepath_);
  }

  int all_grm_size =0;
  std::vector<BuildGrammarFiles> vec_files;
  for (int i=0; i<vec_grammar_files.size(); ++i) {
    boost::filesystem::path path_full_grammar_files = path_grammar_file_parent;
    path_full_grammar_files.append(vec_grammar_files[i]);
    if (false == boost::filesystem::exists(path_full_grammar_files, myecc)) {
      error_message = std::string("file not exist:") + path_full_grammar_files.string();
      YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),   error_message.c_str());
      ret =-1;
      goto FB_BUILDGRAMMAR_RETURN;
    }

    BuildGrammarFiles tmp_files;
    vec_files.push_back(tmp_files);
    vec_files[i].pfile = fopen(path_full_grammar_files.string().c_str(), "rb");
    if (NULL == vec_files[i].pfile) {
      //printf("打开\"%s\"文件失败！[%s]\n", GRM_FILE, strerror(errno));
      error_message = std::string("打开文件失败！:") + path_full_grammar_files.string()+ " error code:" + strerror(errno);
      //printf("%s", error_message.c_str());
      YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),   error_message.c_str());
      ret =-1;
      goto FB_BUILDGRAMMAR_RETURN;
    }

    fseek(vec_files[i].pfile, 0, SEEK_END);
    vec_files[i].file_size = ftell(vec_files[i].pfile);
    all_grm_size += vec_files[i].file_size;
    fseek(vec_files[i].pfile, 0, SEEK_SET);
  }

  grm_content = (char *)malloc(all_grm_size + 1);
  //temp_file_read_content = (char*) malloc(1024*8);
  if (NULL == grm_content /*|| NULL == temp_file_read_content*/) {
    //printf("内存分配失败!\n");
    error_message = "no free memory";
    //printf("%s", error_message.c_str());
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),     error_message.c_str());
    ret = -1;
    goto FB_BUILDGRAMMAR_RETURN;
  }

  // only 1 bnffiles
  if (vec_files.size() ==1) {

    fread((void*)grm_content, 1, vec_files[0].file_size, vec_files[0].pfile);
    grm_content[all_grm_size] = '\0';
    fclose(vec_files[0].pfile);
    vec_files[0].pfile = NULL;

    //ret = build_grammar_fily(grm_content, all_grm_size, udata, error_message);
    //goto FB_BUILDGRAMMAR_RETURN;
    gram_content_using_ptr = grm_content;
    gram_content_using_length = all_grm_size;
    goto FB_BUILD_GRAMMAR_LAST;
  }

  for (int i=0; i< vec_files.size(); ++i) {
    int readcount = fread(grm_content, 1, vec_files[i].file_size, vec_files[i].pfile);
    if (readcount <= 0) {
      continue;
    }
    grm_content[readcount] = '\0';
    vec_files[i].ansi_text = boost::locale::conv::from_utf<char>(grm_content, "gbk");
  }


  // proc #BNF+IAT 1.0 UTF-8;
  for (int i=0; i<vec_files.size(); ++i) {
    MergeGrammar(grm_content, gram_all_seek_index, vec_files[i], "#BNF", /*i ==0 ? true : false*/false);
  }
  add_string = "#BNF+IAT 1.0 UTF-8;\n";
  memcpy(&grm_content[gram_all_seek_index], add_string.c_str(), add_string.length());
  gram_all_seek_index += add_string.length();


  //!grammar fu2cai3;
  for (int i=0; i<vec_files.size(); ++i) {
    MergeGrammar(grm_content, gram_all_seek_index, vec_files[i], "!grammar", /*i ==0 ? true : false*/false);
  }
  add_string = std::string("!grammar ") + grammar_new_name + ";\n";
  memcpy(&grm_content[gram_all_seek_index], add_string.c_str(), add_string.length());
  gram_all_seek_index += add_string.length();

  //!slot
  for (int i=0; i<vec_files.size(); ++i) {
    bool hit_cmp_string = false;
    do {
      MergeGrammar(grm_content, gram_all_seek_index, vec_files[i], "!slot", true, &hit_cmp_string);
    } while (hit_cmp_string);
  }

  utf8string = boost::locale::conv::to_utf<char>(grm_content, "gbk");
  //LogGrammarToTempFile(utf8string.c_str(), utf8string.length(), grammar_new_name.c_str());

  add_string = std::string("!start <") + grammar_new_name + "call>;\n<" + grammar_new_name + "call>:";
  memcpy(&grm_content[gram_all_seek_index], add_string.c_str(), add_string.length());
  gram_all_seek_index += add_string.length();
  grm_content[gram_all_seek_index] ='\0';

  add_string = "";
  // !start
  for (int i=0; i<vec_files.size(); ++i) {
    bool hit_cmp_string = false;
    std::string cmp_string = "!start";
    std::string proc_line_string;
    MergeGrammar(grm_content, gram_all_seek_index, vec_files[i], cmp_string, false, &hit_cmp_string, &proc_line_string);
    assert(hit_cmp_string);
    if (false == hit_cmp_string) {
      continue;
    }
    if (proc_line_string.length() <=0 && proc_line_string.length() <=cmp_string.length()) {
      continue;
    }
    std::string start_command = proc_line_string.substr(cmp_string.length(), proc_line_string.length() - cmp_string.length()-1); ///< 去掉;
    start_command = GetUtility()->Trim2(start_command.c_str());

    cmp_string = start_command;
    proc_line_string = "";
    MergeGrammar(grm_content, gram_all_seek_index, vec_files[i], cmp_string, false, &hit_cmp_string, &proc_line_string);
    if (false ==hit_cmp_string) {
      continue;
    }
    if (proc_line_string.length() <=0 && proc_line_string.length() <=cmp_string.length()) {
      continue;
    }
    /*std::string*/ start_command = proc_line_string.substr(cmp_string.length()+1, proc_line_string.length() - cmp_string.length()-2);  ///< 去掉: 和-1
    start_command = GetUtility()->Trim2(start_command.c_str());

    if (start_command.length() >0) {
      if (add_string.length() >0) {
        add_string += "|";
      }
      add_string += start_command;
    }
  }
  if (add_string.length() >0) {
    add_string += ";";
  }

  memcpy(&grm_content[gram_all_seek_index], add_string.c_str(), add_string.length());
  gram_all_seek_index += add_string.length();
  grm_content[gram_all_seek_index] ='\0';

  //// the last one
  for (int i=0; i<vec_files.size(); ++i) {
    if (vec_files[i].seek_index >= vec_files[i].ansi_text.length()) {
      continue;
    }
    memcpy(&grm_content[gram_all_seek_index], &vec_files[i].ansi_text[vec_files[i].seek_index], vec_files[i].ansi_text.length() - vec_files[i].seek_index);
    gram_all_seek_index += vec_files[i].ansi_text.length() - vec_files[i].seek_index;
    grm_content[gram_all_seek_index] ='\0';
  }

  utf8string = boost::locale::conv::to_utf<char>(grm_content, "gbk");
  LogGrammarToTempFile(utf8string.c_str(), utf8string.length(), grammar_new_name.c_str());
  gram_content_using_ptr = utf8string.c_str();
  gram_content_using_length = utf8string.length();

FB_BUILD_GRAMMAR_LAST:

  //BuildGrammarStatusData tmp_build_status;
  {
    boost::lock_guard<boost::mutex> asr_call_guard(mutex_asr_ilfy_interface_call_);
    ret = QISRBuildGrammar("bnf"/*path_grammar_file.extension().string().c_str()*/, gram_content_using_ptr, gram_content_using_length
                           , grammar_build_params.c_str(), build_grm_cb, tmp_build_grammar_ptr.get());
  }
  {
    //最多等待1分钟钟构建
    clock_t build_clock = clock();
    while ((clock() - build_clock) < 60000) {
      boost::this_thread::sleep_for(boost::chrono::milliseconds(100));

      boost::lock_guard<boost::mutex> guard(tmp_build_grammar_ptr->mutex_member_);
      if (tmp_build_grammar_ptr->build_fini) {

        if (MSP_SUCCESS != tmp_build_grammar_ptr->errcode) {
          YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),   tmp_build_grammar_ptr->build_result_message.c_str());
          error_message = tmp_build_grammar_ptr->build_result_message;
          return YSOS_ERROR_FAILED;
        }
        break;
      }
    }
  }
FB_BUILDGRAMMAR_RETURN:
  for (int i=0; i< vec_files.size(); ++i) {
    if (NULL == vec_files[i].pfile) {
      continue;
    }
    fclose(vec_files[i].pfile);
    vec_files[i].pfile = NULL;
  }

  if (grm_content) {
    free(grm_content);
    grm_content = NULL;
  }

  return ret;
}


int VoiceInterfaceIFLY::NLP(const char* text, std::string& ReceiveNLPResult) {
  /// 如果没有登录，则登录
  if (false == IsHaveLoginIn()) {
    int mspret = Login();
    if (YSOS_ERROR_SUCCESS != mspret) {
      YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "NLP MSPLogin fail");
      return YSOS_ERROR_FAILED;
    } else {
      YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "NLP MSPLogin true");
    }
  }
  // NLP 不支持并发操作，所以这里要作控制。
  //static boost::timed_mutex nlp_mutex;
  if (false == nlp_mutex.try_lock_for(boost::chrono::milliseconds(4000))) {
    //assert(false)
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "调用 ifly nlp 得到锁超时4秒，返回错误");
    return YSOS_ERROR_FAILED;
  }
  boost::lock_guard<boost::timed_mutex> lock_nlp_operation(nlp_mutex, boost::adopt_lock);
  const char* params = nlp_param_.c_str();

  int ierror_code = MSP_SUCCESS;
  unsigned int dataLen =0;
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "in ifly nlp mspsearch begin");

  const char* presult= MSPSearch(params, text, &dataLen,&ierror_code);

  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "in ifly nlp mspsearch end, MSPSearch error code:" << ierror_code);
  if (MSP_SUCCESS != ierror_code) {
    assert(false);
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "error, in ifly nlp mspsearch end, MSPSearch error code:" <<  ierror_code);
    return YSOS_ERROR_FAILED;
  } else {
    //from utf-8 to ansi
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "before ansi_string:" << presult);
    if (NULL != presult && strlen(presult) >0) {
      ReceiveNLPResult= boost::locale::conv::from_utf<char>(presult, /*"gb2312"*/"gbk");
    }

//  ReceiveNLPResult = boost::locale::conv::from_utf(presult,/*"gb2312"*/"gbk");  // GBK
    YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),"ifly nlp string:" << ReceiveNLPResult.c_str());

    //ReceiveNLPResult = presult;
    return YSOS_ERROR_SUCCESS;
  }
}



int VoiceInterfaceIFLY::Login(/*const char* usr, const char* pwd, const char* params*/) {
  boost::lock_guard<boost::mutex> lockguard_msp(mutex_voice_service_provider_login_);

  const char* params = login_params_.c_str();

  if (1 == have_login_result_atomic_) {
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "MSPLogin 已登录，不用再登录");
    return YSOS_ERROR_SUCCESS;
  }

  int iret = MSPLogin(NULL, NULL, params);

  if (MSP_SUCCESS == iret) {
    have_login_result_atomic_ = 1;
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "MSPLogin succ");
    return YSOS_ERROR_SUCCESS;
  } else {
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "MSPLogin fail errorcode:" << iret);
    have_login_result_atomic_ = 0;
    assert(false);
    return YSOS_ERROR_FAILED;
  }
}


int VoiceInterfaceIFLY::Logout() {
  boost::lock_guard<boost::mutex> lockguard_msp(mutex_voice_service_provider_login_);
  if (0 == have_login_result_atomic_) {
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "ifly MSPLogout没有登录或已退出，不用再logout");
    return YSOS_ERROR_SUCCESS;
  }

  int iret = MSPLogout();

  if (MSP_SUCCESS == iret) {
    have_login_result_atomic_ = 0;
    return YSOS_ERROR_SUCCESS;
  } else {
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "MSPLogout errorcode:" <<  iret);
    assert(false);
    return YSOS_ERROR_FAILED;
  }
}



bool VoiceInterfaceIFLY::IsHaveLoginIn() {
  boost::lock_guard<boost::mutex> lockguard_msp(mutex_voice_service_provider_login_);
  return have_login_result_atomic_>=1 ?true:false;
}

int VoiceInterfaceIFLY::ReadConfig(const char* pFilePath) {

//#ifdef _DEBUG
//  MessageBox(NULL, _T("VoiceInterfaceIFLY::ReadConfig alert window"), _T("alert"), MB_OK);
//#endif

  read_config_file_path_ = pFilePath ? pFilePath : "";
  char voide_detail_config_file_path[1024] = {'\0'};
  if (NULL == pFilePath) {
    //sprintf_s(voide_detail_config_file_path, sizeof(voide_detail_config_file_path), "%s", "C:/TDRobot/etc/voice.ini");
    sprintf(voide_detail_config_file_path, "%s", "C:/TDRobot/etc/voice.ini");
  } else {
    //sprintf_s(voide_detail_config_file_path, sizeof(voide_detail_config_file_path), "%setc/voice.ini", pFilePath);
    sprintf(voide_detail_config_file_path, "%setc/voice.ini", pFilePath);
  }
  YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),   "readconfig filepath:" <<  voide_detail_config_file_path);
  voice_config_file_path_ = voide_detail_config_file_path;

  //#登录参数
  char param_buf[1024*3]= {'\0'};
  /* ::GetPrivateProfileStringA ("IFLY", "login_params", "appid = 559527e6, work_dir = .",param_buf, sizeof(param_buf), config_file_full_path);
  login_params_ = param_buf;*/
  //login_params_ ="appid = 559527e6, work_dir = .";
  //login_params_ ="appid = 559527e6, work_dir = C:/TDRobot";

  //login_appid_ = "559527e6";
  //优先使用的appid
  memset(param_buf, '\0', sizeof(param_buf));
  //GetPrivateProfileStringA("IFLY", "priority_appid", "",param_buf, sizeof(param_buf), voide_detail_config_file_path);
  std::string strpriority_appid = param_buf;

  /// 获取资源路径
  memset(param_buf, '\0', sizeof(param_buf));
  //GetPrivateProfileStringA("IFLY", "default_dir", ".",param_buf, sizeof(param_buf), voide_detail_config_file_path);
  std::string default_dir = ".";

  //;网络代理参数设置，如果为空，则表示不设置
  memset(param_buf, '\0', sizeof(param_buf));
  //GetPrivateProfileStringA("IFLY", "net_agent_config", ".",param_buf, sizeof(param_buf), voide_detail_config_file_path);
  std::string  config_net_agent_config = "";

  std::string default_appid = "559527e6";
  if (strpriority_appid.length() >0) {
    default_appid = strpriority_appid;
  }

  // 设置登录参数
  memset(param_buf, '\0', sizeof(param_buf));
  //sprintf_s(param_buf, sizeof(param_buf), "appid = %s, work_dir = %s", default_appid.c_str(), default_dir.c_str());
  sprintf(param_buf, "appid = %s, work_dir = %s", default_appid.c_str(), default_dir.c_str());
  login_params_ = param_buf;
  if (config_net_agent_config.length() >0) {
    login_params_ += std::string(",") + config_net_agent_config;
  }
  std::string log_logparam = login_params_;
  {
    std::string log_default_appid = default_appid;
    if (log_default_appid.length() >2) {
      for (int i=2; i< log_default_appid.length(); ++i) {
        log_default_appid[i] = '*';
      }
    }
    GetUtility()->ReplaceAllDistinct(log_logparam, default_appid, log_default_appid);
  }
  YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),   "login params:" << log_logparam.c_str());


  //#ASR通用配置
  memset(param_buf, '\0', sizeof(param_buf));
  //::GetPrivateProfileString("IFLY", "asr_param_sessionbegin_common", "engine_type = cloud, domain = iat, ptt=0,rst=plain,rse=gb2312,nlp_version=2.0, vad_enable = 1, sch = 1, vad_bos=8000, vad_speech_tail =2000", param_buf, sizeof(param_buf), voide_detail_config_file_path);
  //asr_param_sessionbegin_common_ = param_buf;
  std::string ttmp = "engine_type = cloud, domain = iat, ptt=0,rst=plain,rse =gb2312,vad_enable = 1, sch = 1, vad_speech_tail =800";
  DecodeIflyKeyParam(ttmp, map_asr_cloud_param_sessionbegin_common_);

  //ASRStartFlag =0的配置[asr+nlp]
  memset(param_buf, '\0', sizeof(param_buf));
  //::GetPrivateProfileString("IFLY", "asr_param_sessionbegin_flag0","", param_buf, sizeof(param_buf), voide_detail_config_file_path);
  //asr_param_sessionbegin_flag0_ = param_buf;
  ttmp = "engine_type = cloud, domain = iat, ptt=0,rst=plain,rse =gb2312,nlp_version=2.0, vad_enable = 1, sch = 1, vad_speech_tail =800";
  DecodeIflyKeyParam(ttmp, map_asr_cloud_param_sessionbegin_flag0_);

  //#ASR StartFlag=2的配置[asr]
  memset(param_buf, '\0', sizeof(param_buf));
  //::GetPrivateProfileString("IFLY", "asr_param_sessionbegin_flag2","",param_buf, sizeof(param_buf), voide_detail_config_file_path);
  //asr_param_sessionbegin_flag2_ = param_buf;
  ttmp = "engine_type = cloud, domain = iat,  ptt=0,rst=plain,rse =gb2312, vad_enable = 1, sch = 0,  vad_speech_tail =800";
  DecodeIflyKeyParam(ttmp, map_asr_cloud_param_sessionbegin_flag2_);

  //#ASR StartFlag=3的配置[asr]
  memset(param_buf, '\0', sizeof(param_buf));
  //::GetPrivateProfileString("IFLY", "asr_param_sessionbegin_flag3","", param_buf, sizeof(param_buf), voide_detail_config_file_path);
  //asr_param_sessionbegin_flag3_ = param_buf;
  ttmp = "engine_type = cloud, domain = iat, ptt=0,rst=plain,rse =gb2312,vad_enable = 1, sch = 0, vad_speech_tail =500";
  DecodeIflyKeyParam(ttmp, map_asr_cloud_param_sessionbegin_flag3_);

  //#ASR StartFlag=4的配置[asr]
  memset(param_buf, '\0', sizeof(param_buf));
  //::GetPrivateProfileString("IFLY", "asr_param_sessionbegin_flag4", "", param_buf, sizeof(param_buf), voide_detail_config_file_path);
  //asr_param_sessionbegin_flag4_ = param_buf;
  ttmp = "engine_type = cloud, domain = iat, ptt=0,rst=plain,rse =gb2312,nlp_version=2.0, vad_enable = 1, sch = 0, vad_speech_tail =800";
  DecodeIflyKeyParam(ttmp, map_asr_cloud_param_sessionbegin_flag4_);

  memset(param_buf, '\0', sizeof(param_buf));
  //::GetPrivateProfileString("IFLY", "asr_param_sessionbegin_flag5", "",param_buf, sizeof(param_buf), voide_detail_config_file_path);
  //asr_param_sessionbegin_flag5_ = param_buf;
  ttmp = "engine_type = cloud, domain = iat, ptt=0,rst=plain,rse =gb2312,vad_enable = 1, sch = 0, vad_speech_tail =500";
  DecodeIflyKeyParam(ttmp, map_asr_cloud_param_sessionbegin_flag5_);

  memset(param_buf, '\0', sizeof(param_buf));
  //::GetPrivateProfileString("IFLY", "asr_using_engine_type", "cloud",param_buf, sizeof(param_buf), voide_detail_config_file_path);
  asr_using_engine_type_ = "";

  memset(param_buf, '\0', sizeof(param_buf));
  //::GetPrivateProfileString("IFLY", "asr_using_accent", "mandarin",param_buf, sizeof(param_buf), voide_detail_config_file_path);
  asr_using_accent_ = param_buf;
  if (asr_using_accent_ == "") {
    asr_using_accent_ = TDACCENT_MANDARIN_STRING;
  }

  memset(param_buf, '\0', sizeof(param_buf));
  //::GetPrivateProfileString("IFLY", "asr_mixed_plus_param", "",param_buf, sizeof(param_buf), voide_detail_config_file_path);
  DecodeIflyKeyParam("engine_type = mixed,asr_res_path = fo|res/asr/common.jet,grm_build_path = res/asr/GrmBuilld,rst=xml,nlp_version =2.0,sch=1,mixed_timeout=2000,mixed_threshold=18", map_asr_mixed_plus_param_);

  memset(param_buf, '\0', sizeof(param_buf));
  //::GetPrivateProfileString("IFLY", "asr_local_plus_param", "",param_buf, sizeof(param_buf), voide_detail_config_file_path);
  DecodeIflyKeyParam("engine_type = local,asr_res_path = fo|res/asr/common.jet,grm_build_path = res/asr/GrmBuilld,rst=xml,vad_bos=8000, vad_speech_tail =1000,asr_threshold=0", map_asr_local_plus_param_);

  memset(param_buf, '\0', sizeof(param_buf));
  //::GetPrivateProfileString("IFLY", "asr_build_grammar", "",param_buf, sizeof(param_buf), voide_detail_config_file_path);
  DecodeIflyKeyParam("engine_type = local, 	asr_res_path = fo|res/asr/common.jet, sample_rate = 16000,grm_build_path = res/asr/GrmBuilld", map_asr_build_grammar_);

  memset(param_buf, '\0', sizeof(param_buf));
  //::GetPrivateProfileString("IFLY", "asr_default_grammar_filepath", "",param_buf, sizeof(param_buf), voide_detail_config_file_path);
  config_asr_default_grammar_filepath_ = "msc/res/asr/gramfiles";

  memset(param_buf, '\0', sizeof(param_buf));
  //::GetPrivateProfileString("IFLY", "asr_default_grammar_filename", "",param_buf, sizeof(param_buf), voide_detail_config_file_path);
  config_asr_default_grammar_filename_ = "";
  asr_using_grammar_filename_ = config_asr_default_grammar_filename_;

  /// nlp param
  memset(param_buf, '\0', sizeof(param_buf));
  //::GetPrivateProfileString("IFLY", "nlp_param","nlp_version=2.0,tte=gb2312", param_buf, sizeof(param_buf), voide_detail_config_file_path);
  nlp_param_ = "nlp_version=2.0,tte=gb2312";

  //;合成参数
  ///< TTS mode
  memset(param_buf, '\0', sizeof(param_buf));
  //::GetPrivateProfileString("IFLY", "tts_using_engine_type", "local",param_buf, sizeof(param_buf), voide_detail_config_file_path);
  tts_using_engine_type_ = "local";
  if (tts_using_engine_type_ == "") {
    tts_using_engine_type_ = "local";
  }
  ///<TTS speaker
  memset(param_buf, '\0', sizeof(param_buf));
  //::GetPrivateProfileString("IFLY", "tts_using_speaker", "mengmeng",param_buf, sizeof(param_buf), voide_detail_config_file_path);
  tts_using_speaker_ = "mengmeng";
  if (tts_using_speaker_ == "") {
    tts_using_speaker_ = "mengmeng";
  }

  //TODO:add for linux   for example
  //const char* session_begin_params = "engine_type = local,voice_name=xiaoyan, text_encoding = UTF8, tts_res_path = fo|res/tts/xiaoyan.jet;fo|res/tts/common.jet, sample_rate = 16000, speed = 50, volume = 50, pitch = 50, rdn = 2";

  memset(param_buf, '\0', sizeof(param_buf));
  //::GetPrivateProfileString("IFLY", "TTSParam","", param_buf, sizeof(param_buf), voide_detail_config_file_path);
  //tts_param_ = param_buf;
  DecodeIflyKeyParam("engine_type = local, text_encoding = GBK, voice_name =mengmeng ,tts_res_path = fo|res/tts/mengmeng.jet;fo|res/tts/common.jet, sample_rate = 16000, spd=7 , volume = 100", map_tts_param_zh_);

  //::GetPrivateProfileString("IFLY", "TTSParam_en","", param_buf, sizeof(param_buf), voide_detail_config_file_path);
  DecodeIflyKeyParam("engine_type = cloud, text_encoding = GBK, voice_name =catherine ,tts_res_path = fo|res/tts/catherine.jet;fo|res/tts/common.jet, sample_rate = 16000, spd=6 , volume = 100,ttp=cssml", map_tts_param_en_);

  //::GetPrivateProfileString("IFLY", "TTSParam_zh_cantonese", "", param_buf, sizeof(param_buf), voide_detail_config_file_path);
  DecodeIflyKeyParam("engine_type = cloud, text_encoding = GBK, voice_name =xiaolin ,tts_res_path = fo|res/tts/xiaolin.jet;fo|res/tts/common.jet, sample_rate = 16000, spd=6 , volume = 50", map_tts_param_zh_cantonese_);

  map_tts_param_ = map_tts_param_zh_;  ///< 默认是中文配置

  //;tts本地合成，可用的离线资源有，多个资源用逗号分开,切换不同发音人时，如果本地有，则使用本地的发音人
  /*memset(param_buf, '\0', sizeof(param_buf));
  ::GetPrivateProfileString("IFLY", "TTSParam","", param_buf, sizeof(param_buf), voide_detail_config_file_path);
  splitString(param_buf, vec_tts_local_avaliable_speaker_);*/
  char tempnamebuf[256] = {'\0'};
  std::map<std::string, std::string> map_temp;
  std::vector<std::string> vtemp_vec;
  vtemp_vec.push_back("name=mengmeng,accent=mandarin,language=zh_cn,age=child,gender=male,spd=6");
  vtemp_vec.push_back("name=jiajia,accent=mandarin,language=zh_cn,age=young,gender=female,spd=5");
  vtemp_vec.push_back("name=xiaoyan,accent=mandarin,language=zh_cn,age=young,gender=female,spd=6");
  vtemp_vec.push_back("name=yanping,accent=mandarin,language=zh_cn,age=young,gender=female,spd=6");
  vtemp_vec.push_back("name=xiaofeng ,accent=mandarin,language=zh_cn,age=young,gender=male,spd=6");
  vtemp_vec.push_back("name=jinger,accent=mandarin,language=zh_cn,age=young,gender=female,spd=6");
  vtemp_vec.push_back("name=donaldduck,accent=mandarin,language=zh_cn,age=young,gender=male,spd=6");
  vtemp_vec.push_back("name=babyxu,accent=mandarin,language=zh_cn,age=young,gender=male,spd=6");
  vtemp_vec.push_back("name=nannan,accent=mandarin,language=zh_cn,age=young,gender=female,spd=6");
  vtemp_vec.push_back("name=catherine,accent=mandarin,language=en_us,age=young,gender=female,spd=5");
  for(int i=0; i<vtemp_vec.size(); ++i) {
    //sprintf_s(tempnamebuf, sizeof(tempnamebuf), "tts_speaker_info%d", i);
    map_temp.clear();

    //memset(param_buf, '\0', sizeof(param_buf));
    //::GetPrivateProfileString("IFLY", tempnamebuf,"", param_buf, sizeof(param_buf), voide_detail_config_file_path);
    //if (strlen(param_buf) ==0) {
    //  break;
    //}
    DecodeIflyKeyParam(vtemp_vec[i], map_temp);

    if (map_temp["name"].length() <=0) {
      continue;
    }
    TTSSpeakerInfoPtr temp_ptr(new TTSSpeakerInfo);
    if (!temp_ptr) {
      continue;
    }
    temp_ptr->engingtype = map_temp["engtype"];
    temp_ptr->age = map_temp["age"];
    temp_ptr->gender = map_temp["gender"];
    temp_ptr->accent = map_temp["accent"];
    temp_ptr->name = map_temp["name"];
    temp_ptr->language = map_temp["language"];
    temp_ptr->speed = map_temp["spd"];

    std::string tname = temp_ptr->name;
    map_tts_speaker_info_.insert(std::make_pair(tname, temp_ptr));
  }

  {
    //读取本地有哪些资源文件
    //msc\res\tts
    boost::system::error_code myecc;
    boost::filesystem::path root_path(read_config_file_path_);
    root_path.append("msc/res/tts");

    boost::filesystem::path temp_path;
    boost::filesystem::directory_iterator dit_begin(root_path, myecc), dit_end;
    for (dit_begin; dit_begin != dit_end; ++dit_begin) {
      if (strcasecmp(".jet", dit_begin->path().extension().string().c_str()) !=0) {
        continue;
      }
      std::string speaker_name = dit_begin->path().filename().replace_extension("").string();
      TTSSpeakerInfoPtr temp_ptr = map_tts_speaker_info_[speaker_name];
      if (NULL == temp_ptr) {
        continue;
      }
      if (temp_ptr->engingtype.length() >0) {
        continue;
      }
      temp_ptr->engingtype = TDTTS_ENGINETYPE_LOCAL;  ///< 本地有资源，所以设为本地合成
    }
  }

  TTSSetKeyParam("engine_type", tts_using_engine_type_.c_str());
  TTSSetSpeaker(tts_using_speaker_.c_str());

  //g_Log.Trace_Print(__FILE__, __LINE__ ,"login_params_:%s", login_params_.c_str()); 不要显示账号
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "asr_param_sessionbegin_common_:" << GetIflyKeyParamString(map_asr_cloud_param_sessionbegin_common_).c_str());
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "asr_param_sessionbegin_flag0_:" <<  GetIflyKeyParamString(map_asr_cloud_param_sessionbegin_flag0_).c_str());
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "asr_param_sessionbegin_flag2_:" << GetIflyKeyParamString(map_asr_cloud_param_sessionbegin_flag2_).c_str());
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "asr_param_sessionbegin_flag3_" << GetIflyKeyParamString(map_asr_cloud_param_sessionbegin_flag3_).c_str());
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "asr_param_sessionbegin_flag4_:" <<  GetIflyKeyParamString(map_asr_cloud_param_sessionbegin_flag4_).c_str());
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "asr_param_sessionbegin_flag5:" << GetIflyKeyParamString(map_asr_cloud_param_sessionbegin_flag5_).c_str());
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "asr_build_grammar:" << GetIflyKeyParamString(map_asr_build_grammar_).c_str());
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "nlp_param_:" <<  nlp_param_.c_str());
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "tts_param_:" << GetIflyKeyParamString(map_tts_param_).c_str());

  return YSOS_ERROR_SUCCESS;
}



int VoiceInterfaceIFLY::TTSSessionBegin(/* const char* params,*/ std::string& ReceiveSeesionID) {
  /// 如果没有登录，则登录
  if (false == IsHaveLoginIn()) {
    int mspret = Login();
    if (YSOS_ERROR_SUCCESS != mspret) {
      YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "TTSSessionBegin fail");
      return YSOS_ERROR_FAILED;
    } else {
      YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "TTSSessionBegin true");
    }
  }
  /*const char* params = tts_param_.c_str();*/
  std::string tts_session_param;

  {
    tts_session_param = GetIflyKeyParamString(map_tts_param_);
    YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "QTTSSessionBegin param = " << tts_session_param);
  }
  int ierror_code=0;
  const char* psessionid = QTTSSessionBegin(/*params*/tts_session_param.c_str(), &ierror_code);
  if (0 != ierror_code) {
    YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "QTTSSessionBegin fail, error code:" << ierror_code);
    assert(false);
    return YSOS_ERROR_FAILED;
  }
  assert(NULL != psessionid);
  ReceiveSeesionID = psessionid;
  return YSOS_ERROR_SUCCESS;
}



int VoiceInterfaceIFLY::TTSTextPut(const char* pSessionID, const char* pTextString, unsigned int iTextLen, const char* pParams) {
  // assert(NULL != pSessionID && NULL != pTextString && iTextLen >0);
  if (NULL == pSessionID || NULL == pTextString || iTextLen <0) {
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "tts 内容长度为空?,不再合成");
    assert(false);
    return YSOS_ERROR_FAILED;
  }
#ifdef _DEBUGVIEW
  ::OutputDebugString((std::string("ysos.asrext TTSTextPut") + pTextString).c_str());
#endif

  int iret = QTTSTextPut(pSessionID,pTextString,iTextLen,pParams);
  if (0 != iret) {
    YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "QTTSTextPut errorcode :" << iret);
    assert(false);
    return YSOS_ERROR_FAILED;
  }

  boost::lock_guard<boost::mutex> lock_guard_voice_blocks(mutex_tts_voice_blocks_);
  TTSVoiceDataBlock empty_block;
  caching_voice_block = empty_block;

  return YSOS_ERROR_SUCCESS;
}


int VoiceInterfaceIFLY::TTSAudioGet(const char* pSessionID, TDCharBufPtr& RecviveAudioDataPtr, unsigned int* piAudioLen, int* piSynthStatus) {
  assert(NULL != pSessionID && NULL != piAudioLen);
  if (NULL == pSessionID || NULL == piAudioLen)
    return YSOS_ERROR_FAILED;

  int ierror_code =0;
  int synth_status =MSP_TTS_FLAG_STILL_HAVE_DATA;
  const void* paudio_data = QTTSAudioGet(pSessionID,piAudioLen, &synth_status, &ierror_code);
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "QTTSAudioGet,errorcode:" <<ierror_code);
  if (0 != ierror_code) {
    YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),"QTTSAudioGet,errorcode:" << ierror_code);
    assert(false);
    return YSOS_ERROR_FAILED;
  }

  boost::lock_guard<boost::mutex> lock_guard_voice_blocks(mutex_tts_voice_blocks_);
  // 由于每次产生的tts片段很小3.7KB左右，所以这里作一下控制，只有100KB才放入缓存队列
  // 如果音频数据块大小，则播报的时候会一卡一卡的感觉
  int max_caching_data_lenght = 1024*200;  ///< 每次最大缓存

  if ((caching_voice_block.data_ptr_ && caching_voice_block.data_size_ >=0
       && (caching_voice_block.data_size_ + *piAudioLen) > max_caching_data_lenght) ///< 数据放不下，需要返回
      ||MSP_TTS_FLAG_DATA_END == synth_status
      ||MSP_TTS_FLAG_CMD_CANCELED == synth_status) { ///< 最后一个数据块，需要返回
    //{
    //tts_voice_blocks_.push_back(caching_voice_block);
    RecviveAudioDataPtr.reset(new char[/**piAudioLen*/caching_voice_block.data_size_ + *piAudioLen]);
    memset(RecviveAudioDataPtr.get(), 0,/**piAudioLen*/ caching_voice_block.data_size_ + *piAudioLen);
    if (caching_voice_block.data_size_ >0)
      memcpy(RecviveAudioDataPtr.get(), caching_voice_block.data_ptr_.get(),/**piAudioLen*/caching_voice_block.data_size_);
    if (*piAudioLen >0) {
      char* pdata = RecviveAudioDataPtr.get();
      memcpy(pdata + caching_voice_block.data_size_, paudio_data, *piAudioLen);
    }
    *piAudioLen = caching_voice_block.data_size_ + *piAudioLen;
    caching_voice_block.data_ptr_ = NULL;
    caching_voice_block.data_size_ =0;
    // }
  } else {  ///< 数据块合并
    if (NULL == caching_voice_block.data_ptr_) {
      if (*piAudioLen > max_caching_data_lenght) {
        caching_voice_block.data_ptr_.reset(new char[*piAudioLen]);
      } else {
        caching_voice_block.data_ptr_.reset(new char[max_caching_data_lenght]);
      }
      caching_voice_block.data_size_ =0;
      // caching_voice_block.tts_flag_ = TDMSP_TTS_FLAG_STILL_HAVE_DATA;
    }

    if (*piAudioLen >0) {
      char* pdata = caching_voice_block.data_ptr_.get();
      memcpy(pdata + caching_voice_block.data_size_, paudio_data, *piAudioLen);
      caching_voice_block.data_size_ += *piAudioLen;
    }
    RecviveAudioDataPtr = NULL;
    *piAudioLen =0;
  }

  /*if (piAudioLen >0) {
  RecviveAudioDataPtr.reset(new char[*piAudioLen]);
  memset(RecviveAudioDataPtr.get(),0,*piAudioLen);
  memcpy(RecviveAudioDataPtr.get(),paudio_data,*piAudioLen);
  }*/

  switch (synth_status) {
  case  MSP_TTS_FLAG_STILL_HAVE_DATA:
    *piSynthStatus = TDMSP_TTS_FLAG_STILL_HAVE_DATA;
    break;
  case MSP_TTS_FLAG_DATA_END:
    *piSynthStatus = TDMSP_TTS_FLAG_DATA_END;
    break;
  case MSP_TTS_FLAG_CMD_CANCELED:
    *piSynthStatus = TDMSP_TTS_FLAG_CMD_CANCELED;
    break;
  default:
    assert(false);
    *piSynthStatus = TDMSP_TTS_FLAG_ERROR;
    return YSOS_ERROR_FAILED;
  }

  return YSOS_ERROR_SUCCESS;
}


int VoiceInterfaceIFLY::TTSVoicePlayMode() {
  // return TDVOICE_PLAY_MODE_CACHING;
  return TDVOICE_PLAY_MODE_IMMEDIATELY;
}

int VoiceInterfaceIFLY::TTSSessionEnd(const char* pSessionID, const char* pHints) {
  int iret = QTTSSessionEnd(pSessionID, pHints);
  if (0 != iret) {
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "QTTSSessionEnd errorcode:" << iret <<", sessionid:" << (pSessionID ? pSessionID : ""));
    assert(false);
    return YSOS_ERROR_FAILED;
  }
  return YSOS_ERROR_SUCCESS;
}


/// 迅飞TTS设置不了tts语言，只能是对应不同发音人
int VoiceInterfaceIFLY::TTSSetLanguage(const char* language) {
  if (NULL == language || strlen(language) <=0) {
    return YSOS_ERROR_FAILED;
  }

  if (NULL == language || strlen(language) <=0) {
    return YSOS_ERROR_FAILED;
  }

  boost::lock_guard<boost::mutex> guard(mutex_member_);
  if (strcasecmp(tts_current_langluage_.c_str(), language) ==0) {
    /// save language
    return YSOS_ERROR_SUCCESS;
  }

  if (strcasecmp(language, TDLANGUAGE_ZHCN_STRING) ==0) {
    tts_current_langluage_ = language;
    map_tts_param_ = map_tts_param_zh_;
    return YSOS_ERROR_SUCCESS;
  } else if (strcasecmp(language, TDLANGUAGE_ENUS_STRING) ==0) {
    tts_current_langluage_ = language;
    map_tts_param_ = map_tts_param_en_;
    return YSOS_ERROR_SUCCESS;
  } else if (strcasecmp(language, TDLANGUAGE_ZHCANTONESE_STRING) ==0) {
    tts_current_langluage_ = language;
    map_tts_param_ = map_tts_param_zh_cantonese_;
    return YSOS_ERROR_SUCCESS;
  } else {
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "microsoft not support language:" << language);
    return YSOS_ERROR_FAILED;
  }

  return YSOS_ERROR_SUCCESS;
}
int VoiceInterfaceIFLY::TTSGetLanguage(std::string& language) {
  //获取当前发音人对应的语言
  boost::lock_guard<boost::mutex> guard(mutex_member_);
  language = tts_current_langluage_;
  return YSOS_ERROR_SUCCESS;
}


int VoiceInterfaceIFLY::TTSSetEffect(const int effect) {
  char tempbuf[64] = {'\0'};
  //sprintf_s(tempbuf, sizeof(tempbuf), "%d", effect);
  sprintf(tempbuf, "%d", effect);
  return TTSSetKeyParam("effect", tempbuf);
}
int VoiceInterfaceIFLY::TTSGetEffect(int& effect) {
  std::string str_effect;
  int iget = TTSGetKeyParam("effect", str_effect);
  if (YSOS_ERROR_SUCCESS != iget) {
    effect =0;
    return iget;
  }
  effect = str_effect.length() >0 ? atoi(str_effect.c_str()) : 0;
  return YSOS_ERROR_SUCCESS;
}


int VoiceInterfaceIFLY::TTSSetSpeaker(const char* speaker) {
  assert(speaker);
  if (NULL == speaker || strlen(speaker) <=0) {
    return YSOS_ERROR_FAILED;
  }

  TTSSpeakerInfoPtr speak_ptr = NULL;
  {
    boost::lock_guard<boost::mutex> guard(mutex_member_);
    speak_ptr = map_tts_speaker_info_[speaker];
  }

  std::string old_voice_name;
  TTSGetKeyParam("voice_name", old_voice_name);
  if (strcasecmp(speaker, old_voice_name.c_str()) ==0) { ///< the save speaker
    return YSOS_ERROR_SUCCESS;
  }
  std::string old_tts_res_path;
  TTSGetKeyParam("tts_res_path", old_tts_res_path);

  std::string old_engine_type;
  TTSGetKeyParam("engtype", old_engine_type);

  std::map<std::string, std::string> map_tts_new_param;
  map_tts_new_param["engine_type"] = tts_using_engine_type_;//speak_ptr && speak_ptr->engingtype.length() >0 ? speak_ptr->engingtype : "cloud";
  map_tts_new_param["voice_name"] = speaker;
  map_tts_new_param["tts_res_path"] = std::string("fo|res/tts/") + speaker + ".jet;fo|res/tts/common.jet";
  map_tts_new_param["spd"] = speak_ptr && speak_ptr->speed.length()>0 ? speak_ptr->speed : "6";

  int n_return = TTSSetKeyParams(map_tts_new_param);
  if (n_return == YSOS_ERROR_SUCCESS) {
    // write to ini files
    //WritePrivateProfileString("IFLY", "tts_using_speaker", speaker, voice_config_file_path_.c_str());
  }
  return n_return;
}

int VoiceInterfaceIFLY::TTSGetSpeaker(std::string& speaker) {
  return TTSGetKeyParam("voice_name", speaker);
}

int VoiceInterfaceIFLY::TTSSetEngineType(const char* enginetype) {
  YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "ifly tts set engine type:" << (enginetype ? enginetype: ""));
  if (NULL == enginetype
      || strcasecmp(enginetype, "local") ==0
      || strcasecmp(enginetype, "cloud") ==0) {

  } else {
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "not support tts engine type:" << (enginetype ? enginetype : ""));
    return YSOS_ERROR_FAILED;
  }
  {
    boost::lock_guard<boost::mutex> guard(mutex_member_);
    tts_using_engine_type_ = enginetype ? enginetype : "local";
  }
  TTSSetKeyParam("engine_type", tts_using_engine_type_.c_str());
  // write to ini files
  //WritePrivateProfileString("IFLY", "tts_using_engine_type", enginetype, voice_config_file_path_.c_str());
  return YSOS_ERROR_SUCCESS;
}

int VoiceInterfaceIFLY::TTSGetEngineType(std::string& enginetype) {
  boost::lock_guard<boost::mutex> tts_guard(mutex_member_);
  enginetype = tts_using_engine_type_;
  return YSOS_ERROR_SUCCESS;
}

VoiceInterfaceIFLY::VoiceInterfaceIFLY() {
  have_login_result_atomic_ =0;
}

VoiceInterfaceIFLY::~VoiceInterfaceIFLY() {}



int VoiceInterfaceIFLY::TTSSetKeyParam(const char* key, const char* param) {
  if (NULL == key || NULL ==param) {
    return YSOS_ERROR_FAILED;
  }
  if (strlen(key) ==0 || strlen(param) ==0) {
    return YSOS_ERROR_FAILED;
  }

  boost::lock_guard<boost::mutex> lock_guard_accent(mutex_member_);
  map_tts_param_[key] = param;

  return YSOS_ERROR_SUCCESS;
}

int VoiceInterfaceIFLY::TTSSetKeyParams(const std::map<std::string, std::string>& map_key_param) {
  boost::lock_guard<boost::mutex> lock_guard_accent(mutex_member_);
  std::map<std::string, std::string>::const_iterator cit = map_key_param.begin();
  for (cit; cit != map_key_param.end(); ++cit) {
    map_tts_param_[cit->first] = cit->second;
  }
  return YSOS_ERROR_SUCCESS;
}

int VoiceInterfaceIFLY::TTSGetKeyParam(const char* key, std::string& param) {
  if (NULL == key || strlen(key) ==0) {
    return YSOS_ERROR_FAILED;
  }
  boost::lock_guard<boost::mutex> lock_guard_accent(mutex_member_);
  std::map<std::string, std::string>::iterator itfind = map_tts_param_.find(key);
  if (itfind != map_tts_param_.end()) {
    param = itfind->second;
    return YSOS_ERROR_SUCCESS;
  }
  return YSOS_ERROR_SUCCESS;
}


int VoiceInterfaceIFLY::UploadUserwords(std::string* pReceErrorMsg) {
  char* userwords = NULL;
  unsigned int len = 0;
  unsigned int read_len = 0;
  FILE* fp = NULL;
  int ret = -1;
  std::string utf8_userwords ;

  fp = fopen("userwords.txt", "rb");
  if (NULL == fp) {
    // printf("\nopen [userwords.txt] failed! \n");
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "open [userwords.txt] failed!");
    if (pReceErrorMsg)
      *pReceErrorMsg = "open [userwords.txt] failed!";
    goto upload_exit;
  }

  fseek(fp, 0, SEEK_END);
  len = ftell(fp); //获取音频文件大小
  fseek(fp, 0, SEEK_SET);

  userwords = (char*)malloc(len + 1);
  if (NULL == userwords) {
    // printf("\nout of memory! \n");
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "out of memory! ");
    if (pReceErrorMsg)
      *pReceErrorMsg = "out of memory! ";
    goto upload_exit;
  }

  read_len = fread((void*)userwords, 1, len, fp); //读取用户词表内容
  if (read_len != len) {
    //printf("\nread [userwords.txt] failed!\n");
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "read [userwords.txt] failed!");
    if (pReceErrorMsg)
      *pReceErrorMsg = "read [userwords.txt] failed!";
    goto upload_exit;
  }
  userwords[len] = '\0';

  // 转成utf8格式
  utf8_userwords = boost::locale::conv::to_utf<char>(userwords, /*"gb2312"*/"gbk");

  YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "upload words:" << userwords);
  //MSPUploadData("userwords", userwords, len, "sub = uup, dtt = userword", &ret); //上传用户词表
  MSPUploadData("userwords", (char*)utf8_userwords.c_str(), utf8_userwords.length(), "sub = uup, dtt = userword", &ret); //上传用户词表
  if (MSP_SUCCESS != ret) {
    //printf("\nMSPUploadData failed ! errorCode: %d \n", ret);
    char error_buf[1024]= {'\0'};
    //sprintf_s(error_buf, sizeof(error_buf),"MSPUploadData failed ! errorCode: %d ", ret);
    sprintf(error_buf, "MSPUploadData failed ! errorCode: %d ", ret);
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), error_buf);
    if (pReceErrorMsg)
      *pReceErrorMsg = error_buf;
    goto upload_exit;
  }

  if (pReceErrorMsg)
    *pReceErrorMsg = "上传关键词成功";  ///< 注意这个字符串不能变，其他地方会以这个 字符串来判断是否 上传成功

upload_exit:
  if (NULL != fp) {
    fclose(fp);
    fp = NULL;
  }
  if (NULL != userwords) {
    free(userwords);
    userwords = NULL;
  }

  YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "上传结果:" << (0==ret ? "success" : "fail") << ", 错误原因:" << (pReceErrorMsg? pReceErrorMsg->c_str():""));
  return 0 ==ret ?YSOS_ERROR_SUCCESS :YSOS_ERROR_FAILED;
}



int VoiceInterfaceIFLY::DecodeIflyKeyParam(const std::string set_param_string, std::map<std::string, std::string>& map_params) {
  std::vector<std::string> vec_keysvalues;
  GetUtility()->SplitString2Vec(set_param_string.c_str(), vec_keysvalues, ',');
  std::vector<std::string>::iterator it = vec_keysvalues.begin();

  std::vector<std::string> vec_tmep;
  std::string temps1, temps2;
  for (it; it != vec_keysvalues.end(); ++it) {
    vec_tmep.clear();
    GetUtility()->SplitString2Vec((*it).c_str(), vec_tmep, '=');
    assert(vec_tmep.size() ==2);
    if (vec_tmep.size() !=2) {
      YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "decode ifly key param error, is not a pair:" << (*it).c_str());
      continue;
    }
    // fortest
    //{
    //  std::string mys=" sssss   ";
    //  std::string mys2 = trim(mys.c_str());
    //  printf("%s", mys2.c_str());
    //}
    temps1 = GetUtility()->Trim2(vec_tmep[0].c_str());
    temps2 = GetUtility()->Trim2(vec_tmep[1].c_str());
    if (temps1.length() <=0 || temps2.length() <=0) {
      YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "decode ifly key param error, have blank param:" << (*it).c_str());
      continue;
    }
    map_params[temps1] = temps2;
  }

  return YSOS_ERROR_SUCCESS;
}
std::string VoiceInterfaceIFLY::GetIflyKeyParamString(const std::map<std::string, std::string>& map_params) {
  std::string string_params;
  std::map<std::string, std::string>::const_iterator cit = map_params.begin();
  for (cit; cit != map_params.end(); ++cit) {
    if (string_params.length() >0) {
      string_params += ",";
    }
    if (cit->first.length() <=0 || cit->second.length() <=0) {
      continue;
    }
    string_params += cit->first + "=" + cit->second;
  }
  return string_params;
}
