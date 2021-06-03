/*@version 1.0
  *@author l.j..
  *@date Created on: 2016-10-20 13:48:23
  *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
  * 
*/

/// self header
#include "../include/audioextdriver.h"

/// windows header
#include <assert.h>

/// stl headers
#include <string>

/// boost headers
#include <boost/thread/thread_guard.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/locale.hpp>

/// ysos headers
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/bufferpool.h"
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/callbackqueue.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/jsonutility.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/packageconfigimpl.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemoduleimpl.h"
#include "../../../../../../core/trunk/main/protect/include/os_hal_package/datainterfacemanager.h"

/// private headers
//#include "../include/stringext.h"

namespace ysos {
CXFAsr AudioExtDriver::cxfasr_;
DECLARE_PLUGIN_REGISTER_INTERFACE(AudioExtDriver, DriverInterface);
AudioExtDriver::AudioExtDriver(const std::string &strClassName) : BaseDriverImpl(strClassName) {
  tts_file_ = NULL;
  tts_pcm_dir_ = "";
  logger_ = GetUtility()->GetLogger("ysos.asrext");

#ifdef _DEBUG
  MessageBox(NULL, _T("AudioExtDriver alert window"), _T("alert"), MB_OK);
#endif
}

AudioExtDriver::~AudioExtDriver() {

}

int AudioExtDriver::Read(BufferInterfacePtr pBuffer, DriverContexPtr context_ptr) {
  return YSOS_ERROR_SUCCESS;
}

int AudioExtDriver::Write(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, DriverContexPtr context_ptr) {
  return YSOS_ERROR_SUCCESS;
}



int AudioExtDriver::Ioctl(int iCtrlID, BufferInterfacePtr pInputBuffer, BufferInterfacePtr pOutputBuffer) {
  int n_return = YSOS_ERROR_FAILED;

  do {
    Json::Value json_value;
    Json::Reader json_reader;

    switch (iCtrlID) {
    case CMD_ICC_START: {
      break;
    }
    case  CMD_ICC_STOP: { //< 停止
      YSOS_LOG_DEBUG("command CMD_ICC_STOP");
      if (NULL == pInputBuffer) {
        YSOS_LOG_DEBUG("error exectue CMD_ICC_STOP, the param is null ");
        break;
      }

      UINT8* pbuff = NULL;
      UINT32 buffer_size =0;
      pInputBuffer->GetBufferAndLength(&pbuff,&buffer_size);
      if (NULL == pbuff || buffer_size <=0) {
        YSOS_LOG_DEBUG("error exectue CMD_ICC_STOP, the param is null ");
        break;
      }
      std::string alibity_name = reinterpret_cast<char*>(pbuff);
      YSOS_LOG_DEBUG(" call command CMD_ICC_STOP: ability:" << alibity_name);
      if (strcasecmp(alibity_name.c_str(), ABSTR_ASR) ==0 || strcasecmp(alibity_name.c_str(),ABSTR_ASRNLP) ==0) {
        cxfasr_.TDStopASR(1);
        n_return = YSOS_ERROR_SUCCESS;
        break;
      } else if (strcasecmp(alibity_name.c_str(), ABSTR_NLP) ==0) {
        //< 这里停止不了，不调用
        n_return = YSOS_ERROR_SUCCESS;
        break;
      } else if (strcasecmp(alibity_name.c_str(), ABSTR_TTS) ==0) {
        cxfasr_.TDStopTTS();
        n_return = YSOS_ERROR_SUCCESS;
        break;
      }
    } //< CMD_ICC_STOP
    case CMD_AUDIO_ASR_MODE: {
      UINT8* pbuff = NULL;
      UINT32 buffer_size =0;
      pInputBuffer->GetBufferAndLength(&pbuff,&buffer_size);
      if (NULL == pbuff || buffer_size <=0) {
        YSOS_LOG_DEBUG("error exectue CMD_AUDIO_ASR_MODE, the param is null ");
        break;
      }
      YSOS_LOG_DEBUG("driver CMD_AUDIO_ASR_MODE input_param = " << (char*)pbuff );
      int mode =0;
      std::istringstream in_stream(boost::locale::conv::to_utf<char>((char*)pbuff, "gbk"));
      boost::property_tree::ptree request_tree, empty_tree;
      try {
        boost::property_tree::read_json(in_stream, request_tree);
        mode = request_tree.get("asr_mode", 0);
      }catch(...) {
        YSOS_LOG_ERROR("decode  json error = " << (char*)pbuff);
        break;
      }

      YSOS_LOG_DEBUG("driver CMD_AUDIO_ASR_MODE recv mode = " << mode);
      n_return = cxfasr_.ChangeConfig(mode);
      break;
    } //< CMD_AUDIO_ASR_MODE
    case CMD_AUDIO_ASR_ACCENT: {
      UINT8* pbuff = NULL;
      UINT32 buffer_size =0;
      pInputBuffer->GetBufferAndLength(&pbuff,&buffer_size);
      if (NULL == pbuff || buffer_size <=0) {
        YSOS_LOG_DEBUG("error exectue CMD_AUDIO_ASR_ACCENT, the param is null ");
        break;
      }
      YSOS_LOG_DEBUG("driver CMD_AUDIO_ASR_ACCENT input_param = " << (char*)pbuff);
      int accent =0;
      std::istringstream in_stream(boost::locale::conv::to_utf<char>((char*)pbuff, "gbk"));
      boost::property_tree::ptree request_tree, empty_tree;
      try {
        boost::property_tree::read_json(in_stream, request_tree);
        accent = request_tree.get("asr_accent", 0);
      }catch(...) {
        YSOS_LOG_ERROR("decode  json error = " << (char*)pbuff);
        break;
      }
      n_return = cxfasr_.AudioChangeAccent(accent);
      break;
    } //< CMD_AUDIO_ASR_ACCENT
    case CMD_AUDIO_TTS_MODE: {
      UINT8* pbuff = NULL;
      UINT32 buffer_size =0;
      pInputBuffer->GetBufferAndLength(&pbuff,&buffer_size);
      if (NULL == pbuff || buffer_size <=0) {
        YSOS_LOG_DEBUG("error exectue CMD_AUDIO_TTS_MODE, the param is null ");
        break;
      }
      YSOS_LOG_DEBUG("driver CMD_AUDIO_TTS_MODE input_param = " << (char*)pbuff);
      int mode =0;
      std::istringstream in_stream(boost::locale::conv::to_utf<char>((char*)pbuff, "gbk"));
      boost::property_tree::ptree request_tree, empty_tree;
      try {
        boost::property_tree::read_json(in_stream, request_tree);
        mode = request_tree.get("tts_mode", 0);
      } catch (...) {
        YSOS_LOG_ERROR("decode  json error = " << (char*)pbuff);
        break;
      }
      n_return = cxfasr_.AudioChangeTTSMode(mode);
      break;
    } //< CMD_AUDIO_TTS_MODE
    case CMD_AUDIO_TTS_SPEAKER: {
      UINT8* pbuff = NULL;
      UINT32 buffer_size =0;
      pInputBuffer->GetBufferAndLength(&pbuff,&buffer_size);
      if (NULL == pbuff || buffer_size <=0) {
        YSOS_LOG_DEBUG("error exectue CMD_AUDIO_TTS_SPEAKER, the param is null ");
        break;
      }
      YSOS_LOG_DEBUG("driver CMD_AUDIO_TTS_SPEAKER input_param = " << (char*)pbuff);
      std::string speaker = "";
      std::istringstream in_stream(boost::locale::conv::to_utf<char>((char*)pbuff, "gbk"));
      boost::property_tree::ptree request_tree, empty_tree;
      try {
        boost::property_tree::read_json(in_stream, request_tree);
        speaker = request_tree.get("tts_speaker", "mengmeng");
      } catch (...) {
        YSOS_LOG_ERROR("decode  json error = " << (char*)pbuff);
        break;
      }
      n_return = cxfasr_.AudioChangeTTSSpeaker(speaker);
      break;
    } //< CMD_AUDIO_TTS_SPEAKER
    }
  } while (0);

  return n_return;
}

int AudioExtDriver::Open(void *pParams) {
  return YSOS_ERROR_SUCCESS;
}

void AudioExtDriver::Close(void *pParams) {
}

int AudioExtDriver::GetProperty(int iTypeId, void *piType) {
  //YSOS_LOG_DEBUG("AudioExtDriver::GetProperty in");
  int n_return = YSOS_ERROR_FAILED;

  if (cxfasr_.term_id_ == ""){
    std::string term_id;
    DataInterfacePtr data_ptr = GetDataInterfaceManager()->GetData(GLOBAL_DATA_KEY);
    data_ptr->GetData("termId", term_id);
    cxfasr_.term_id_ = term_id;
    YSOS_LOG_ERROR("get term_id = " << term_id);
  }

  do {
    switch (iTypeId) {
    case PROP_FUN_CALLABILITY: {
      FunObjectCommon2* pobject = reinterpret_cast<FunObjectCommon2*>(piType);
      if (NULL == pobject) {
        YSOS_LOG_DEBUG("error call property PROP_Fun_CallAbility, the param is null");
        break;
      }
      AbilityParam* pin = reinterpret_cast<AbilityParam*>(pobject->pparam1);
      AbilityParam* pout = reinterpret_cast<AbilityParam*>(pobject->pparam2);
      n_return = CallAbility(pin, pout);
      break;
    }
    case PROP_REGISTER_ABILITY_EVENT:   ///< 能力事件的注册
    case PROP_UNREGISTER_ABILITY_EVENT: {
      FunObjectCommon2* pobject = reinterpret_cast<FunObjectCommon2*>(piType);
      if (NULL == pobject) {
        YSOS_LOG_DEBUG("error call property PROP_REGISTER_ABILITY_EVENT/PROP_UNREGISTER_ABILITY_EVENT, the param is null");
        break;
      }

      BaseModuleImpl* pmodule = reinterpret_cast<BaseModuleImpl*>(pobject->pparam1);
      std::string* pability_name = reinterpret_cast<std::string*>(pobject->pparam2);
      if (NULL == pmodule || NULL == pability_name) {
        YSOS_LOG_DEBUG("data error");
        break;
      }
      n_return = RegisterOrUnregisterAbilityEvent(PROP_REGISTER_ABILITY_EVENT == iTypeId ? true : false, pmodule, pability_name);
      break;
    }
    default:
      n_return = YSOS_ERROR_INVALID_ARGUMENTS;
    }
  } while (0);

  //YSOS_LOG_DEBUG("AudioExtDriver::GetProperty out");
  return n_return;
}

int AudioExtDriver::SetProperty(int iTypeId, void *piType) {
  return YSOS_ERROR_SUCCESS;
}

int AudioExtDriver::Initialized(const std::string &key, const std::string &value) {
  if ("tts_pcm_generate" == key) {
    int pcm_generate = boost::lexical_cast<int>(value);
    if (1 == pcm_generate) {
      tts_pcm_dir_ = GetPackageConfigImpl()->GetConfPath() + "../data/ttsextmodule/pcm/";
      YSOS_LOG_DEBUG("tts_pcm_dir = " << tts_pcm_dir_);
    }
  }

  return YSOS_ERROR_SUCCESS;
}

int AudioExtDriver::CallAbility(AbilityParam* pInput, AbilityParam* pOut) {
  int n_return = YSOS_ERROR_FAILED;
  do {
    if (NULL == pInput || NULL == pOut) {
      YSOS_LOG_DEBUG("error call ablicity , input/out alibityparam is null");
      break;
    }

    if (strcasecmp(ABSTR_ASR, pOut->ability_name) ==0) {
      if (strcasecmp(DTSTR_STREAM_PCMX1X, pInput->data_type_name) ==0
          && strcasecmp(DTSTR_TEXT_PURE, pOut->data_type_name) ==0) {
        n_return = AbilityASR_PCM2Textpure(pInput, pOut);
        break;
      }
    } else if (strcasecmp(ABSTR_ASRNLP, pOut->ability_name) ==0) {
      if (strcasecmp(DTSTR_STREAM_PCMX1X, pInput->data_type_name) ==0
          && strcasecmp(DTSTR_TEXT_JSON, pOut->data_type_name) ==0) {
        n_return = AbilityASRNLP_PCM2Json(pInput, pOut);
        break;
      }
    } else if (strcasecmp(ABSTR_NLP, pOut->ability_name) ==0) {
      if (strcasecmp(DTSTR_TEXT_PURE, pInput->data_type_name) ==0
          && strcasecmp(DTSTR_TEXT_JSON, pOut->data_type_name) ==0) {
        n_return = AbilityNLP_Text2Json(pInput, pOut);
        break;
      }
    } else if (strcasecmp(ABSTR_TTS, pOut->ability_name) ==0) { //< 使用TTS能力
      if ((strcasecmp(DTSTR_TEXT_PURE, pInput->data_type_name) ==0  //< 输入是文本类型
           || strcasecmp(DTSTR_TEXT_JSON, pInput->data_type_name) ==0
           || strcasecmp(DTSTR_TEXT_XML, pInput->data_type_name) ==0)) {
        if (strcasecmp(DTSTR_STREAM_PCMX1X_P2, pOut->data_type_name) ==0) {
          n_return = AbilityTTS_Text2PCMP2(pInput, pOut);
          break;
        } else if (strcasecmp(DTSTR_STREAM_PCMX1X, pOut->data_type_name) ==0) {
          n_return = AbilityTTS_Text2PCM(pInput, pOut);
          break;
        }
      } else { //< 其他类型数据
        //< 判断是不是text_json_extract@x类型
        std::vector<std::string> input_type_params = GetUtility()->SplitString2(pInput->data_type_name, "@"); //< 通过@切分
        if (input_type_params.size() >0) {
          if (strcasecmp(input_type_params[0].c_str(), "text_json_extract") ==0) {
            n_return = AbilityTTS_JsonExtract2PCMP2(pInput, input_type_params, pOut);
            break;
          }
        }
      }
    } else {
      YSOS_LOG_DEBUG("not support ability_name = " << pOut->ability_name);
      n_return = YSOS_ERROR_FAILED;
    }
  } while (0);

  return n_return;
}

int AudioExtDriver::AbilityASR_PCM2Textpure(AbilityParam* pInput, AbilityParam* pOut) {
  int n_return = YSOS_ERROR_FAILED;
  YSOS_LOG_DEBUG("AbilityASR_PCM2Textpure in");

  do {
    if (NULL == pInput->buf_interface_ptr || NULL == pOut->buf_interface_ptr) {
      YSOS_LOG_DEBUG("buffer error");
      break;
    }

    UINT8* wave_buffer = NULL;
    UINT32 buffer_size = 0;
    pInput->buf_interface_ptr->GetBufferAndLength(&wave_buffer,&buffer_size);
    cxfasr_.AudioCaptureCallBackData((char*)wave_buffer, buffer_size); //< 传入语音数据，
    cxfasr_.TDStartASR(0, 5); //< 开始语音转写
    TDAsrResultPtr resultptr = cxfasr_.GetAsrResult(); //< 获取结果
    if (resultptr && pOut) {
      UINT8* pout = NULL;
      UINT32 pout_size = 0;
      pOut->buf_interface_ptr->GetBufferAndLength(&pout, &pout_size);
      {
        UINT32 max_length = 0, prefix_length = 0;
        pOut->buf_interface_ptr->GetMaxLength(&max_length);
        pOut->buf_interface_ptr->GetPrefixLength(&prefix_length);
        pout_size = max_length - prefix_length;
      }
      if (pout_size < (resultptr->result.length()+1)) {
        YSOS_LOG_DEBUG("error call ability pcmx1x->asr, output buffer is to small");
        break;
      }
      //strcpy_s(reinterpret_cast<char*>(pout), (resultptr->result.length()+1), resultptr->result.c_str());
      strcpy(reinterpret_cast<char*>(pout), resultptr->result.c_str());
      pOut->buf_interface_ptr->SetLength(resultptr->result.length()+1);
      n_return = YSOS_ERROR_SUCCESS;
      break;
    } else {
      //no result;
      pOut->buf_interface_ptr->SetLength(0);
      n_return = YSOS_ERROR_SUCCESS;
      break;
    }
  } while (0);

  return YSOS_ERROR_FAILED;
}

int AudioExtDriver::AbilityASRNLP_PCM2Json(AbilityParam* pInput, AbilityParam* pOut) {
  int n_return = YSOS_ERROR_FAILED;
  YSOS_LOG_DEBUG("AudioExtDriver::AbilityASRNLP_PCM2Json in");

  do {
    if (NULL == pInput->buf_interface_ptr || NULL == pOut->buf_interface_ptr) {
      YSOS_LOG_DEBUG("buffer error");
      break;
    }

    UINT8* wave_buffer = NULL;
    UINT32 buffer_size = 0;
    pInput->buf_interface_ptr->GetBufferAndLength(&wave_buffer,&buffer_size);
    cxfasr_.AudioCaptureCallBackData((char*)wave_buffer, buffer_size);
    cxfasr_.TDStartASR(0, 4);
    TDAsrResultPtr resultptr = cxfasr_.GetAsrResult();
    if (resultptr && pOut) {
      UINT8* pout = NULL;
      UINT32 pout_size = 0;
      pOut->buf_interface_ptr->GetBufferAndLength(&pout, &pout_size);
      {
        UINT32 max_length = 0, prefix_length = 0;
        pOut->buf_interface_ptr->GetMaxLength(&max_length);
        pOut->buf_interface_ptr->GetPrefixLength(&prefix_length);
        pout_size = max_length - prefix_length;
      }
      if (pout_size < (resultptr->result.length()+1)) {
        YSOS_LOG_DEBUG("error call ability pcmx1x->asr, output buffer is to small");
        break;
      }
      //strcpy_s(reinterpret_cast<char*>(pout), (resultptr->result.length()+1), resultptr->result.c_str());
      strcpy(reinterpret_cast<char*>(pout), resultptr->result.c_str());
      pOut->buf_interface_ptr->SetLength(resultptr->result.length()+1);
      n_return =  YSOS_ERROR_SUCCESS;
      break;
    } else {
      // no result;
      pOut->buf_interface_ptr->SetLength(0);
      n_return = YSOS_ERROR_SUCCESS;
      break;
    }
  } while (0);

  return n_return;
}

int AudioExtDriver::AbilityNLP_Text2Json(AbilityParam* pInput, AbilityParam* pOut) {
  int n_return = YSOS_ERROR_FAILED;
  YSOS_LOG_DEBUG("AbilityNLP_Text2Json in");

  do {
    if (NULL == pInput->buf_interface_ptr || NULL == pOut->buf_interface_ptr) {
      YSOS_LOG_DEBUG("buffer error");
      break;
    }

    UINT8* ptext = NULL;
    UINT32 buffer_size = 0;
    pInput->buf_interface_ptr->GetBufferAndLength(&ptext,&buffer_size);
    std::string str_text_need_nlp = reinterpret_cast<char*>(ptext);
    std::string strnlp_result;
    int n_result = cxfasr_.TDNLP(0, str_text_need_nlp, 1, &strnlp_result);
    if (YSOS_ERROR_SUCCESS != n_result) {
      break;
    }
    // out put the data
    UINT8* pout = NULL;
    UINT32 pout_size = 0;
    pOut->buf_interface_ptr->GetBufferAndLength(&pout, &pout_size);
    {
      UINT32 max_length = 0, prefix_length = 0;
      pOut->buf_interface_ptr->GetMaxLength(&max_length);
      pOut->buf_interface_ptr->GetPrefixLength(&prefix_length);
      pout_size = max_length - prefix_length;
    }
    if (pout_size < (strnlp_result.length()+1)) {
      YSOS_LOG_DEBUG("error call ability pcmx1x->asr, output buffer is to small");
      break;
    }

    if (pOut->noresult_return_empty_ && strnlp_result.length() >0) { // 过滤没有结果的数据，连json也不返回
      //< 如果 "rc": "", 为4，则表示正常但无结果
      try {
        std::string utf8_result = boost::locale::conv::to_utf<char>(strnlp_result.c_str(), "gbk");
        std::istringstream istring_result(utf8_result);
        boost::property_tree::ptree ptree_json_input;
        boost::property_tree::read_json(istring_result, ptree_json_input);
        std::string utf8_rc = ptree_json_input.get<std::string>("rc");
        std::string str_rc = boost::locale::conv::from_utf<char>(utf8_rc.c_str(), "gbk");
        if (4 == atoi(str_rc.c_str())) {
          strnlp_result="";
        }
      } catch (...) {
        YSOS_LOG_DEBUG("error ecode hci custuomer resut json decode error");
        break;
      }
    }

    if (strnlp_result.length() >0) {
      //strcpy_s(reinterpret_cast<char*>(pout),(strnlp_result.length()+1), strnlp_result.c_str());
      strcpy(reinterpret_cast<char*>(pout), strnlp_result.c_str());
      pOut->buf_interface_ptr->SetLength(strnlp_result.length()+1);
    } else {
      pOut->buf_interface_ptr->SetLength(0);
    }
    n_return = YSOS_ERROR_SUCCESS;
  } while (0);

  return n_return;
}


int AudioExtDriver::AbilityNlp_Json2Json(AbilityParam* pInput, AbilityParam* pOut) {
  int n_return = YSOS_ERROR_FAILED;
  YSOS_LOG_DEBUG("AbilityNlp_Json2Json in");

  do {
    if (NULL == pInput->buf_interface_ptr || NULL == pOut->buf_interface_ptr) {
      YSOS_LOG_DEBUG("buffer error");
      break;
    }

    UINT8* ptext = NULL;
    UINT32 buffer_size =0;
    pInput->buf_interface_ptr->GetBufferAndLength(&ptext,&buffer_size);
    std::string str_text_need_nlp = reinterpret_cast<char*>(ptext);
    //< 提取json中的text字段
    if (str_text_need_nlp.length() >0) {
      try {
        std::string utf8_text_need_nlp = boost::locale::conv::to_utf<char>(str_text_need_nlp.c_str(), "gbk");
        std::istringstream istring_text_need_nlp(utf8_text_need_nlp);
        boost::property_tree::ptree ptree_json_input;
        boost::property_tree::read_json(istring_text_need_nlp, ptree_json_input);
        std::string utf8_text = ptree_json_input.get<std::string>("text");
        str_text_need_nlp = boost::locale::conv::from_utf<char>(utf8_text.c_str(), "gbk");
      } catch (...) {
        YSOS_LOG_DEBUG("error ablity json 2 json ,decode json error");
        break;
      }
    }

    std::string strnlp_result;
    int n_result = cxfasr_.TDNLP(0, str_text_need_nlp, 1, &strnlp_result);
    if (YSOS_ERROR_SUCCESS != n_result) {
      break;
    }
    // out put the data
    UINT8* pout = NULL;
    UINT32 pout_size =0;
    pOut->buf_interface_ptr->GetBufferAndLength(&pout, &pout_size);
    {
      UINT32 max_length = 0, prefix_length = 0;
      pOut->buf_interface_ptr->GetMaxLength(&max_length);
      pOut->buf_interface_ptr->GetPrefixLength(&prefix_length);
      pout_size = max_length - prefix_length;
    }
    if (pout_size < (strnlp_result.length()+1)) {
      YSOS_LOG_DEBUG("error call ability pcmx1x->asr, output buffer is to small");
      break;
    }

    if (pOut->noresult_return_empty_ && strnlp_result.length() >0) { // 过滤没有结果的数据，连json也不返回
      //< 如果 "rc": "", 为4，则表示正常但无结果
      try {
        std::string utf8_result = boost::locale::conv::to_utf<char>(strnlp_result.c_str(), "gbk");
        std::istringstream istring_result(utf8_result);
        boost::property_tree::ptree ptree_json_input;
        boost::property_tree::read_json(istring_result, ptree_json_input);
        std::string utf8_rc = ptree_json_input.get<std::string>("rc");
        std::string str_rc = boost::locale::conv::from_utf<char>(utf8_rc.c_str(), "gbk");
        if (4 == atoi(str_rc.c_str())) {
          strnlp_result="";
        }
      } catch (...) {
        YSOS_LOG_DEBUG("error ecode hci custuomer resut json decode error");
        break;
      }
    }

    if (strnlp_result.length() > 0) {
      //strcpy_s(reinterpret_cast<char*>(pout),(strnlp_result.length()+1), strnlp_result.c_str());
      strcpy(reinterpret_cast<char*>(pout), strnlp_result.c_str());
      pOut->buf_interface_ptr->SetLength(strnlp_result.length()+1);
    } else {
      pOut->buf_interface_ptr->SetLength(0);
    }

    n_return = YSOS_ERROR_SUCCESS;
  } while (0);

  return n_return;
}

int AudioExtDriver::AbilityTTS_Text2PCMP2(AbilityParam* pInput, AbilityParam* pOut) {
  int n_return = YSOS_ERROR_FAILED;
  YSOS_LOG_DEBUG("AbilityTTS_Text2PCMP2 in");

  do {
    if (NULL == pInput->buf_interface_ptr || NULL == pOut->buf_interface_ptr) {
      YSOS_LOG_DEBUG("buffer error");
      break;
    }

    UINT8* ptext = NULL;
    UINT32 buffer_size =0;
    pInput->buf_interface_ptr->GetBufferAndLength(&ptext,&buffer_size);
    int interrupt = *ptext;
    YSOS_LOG_DEBUG("tts get interrupt type = " << interrupt);
    std::string str_text_need_tts = reinterpret_cast<char*>(ptext+1);
    if (str_text_need_tts.length() <= 0) { //< 如果传入为空，则返回空数据
      pOut->buf_interface_ptr->SetLength(0);
      n_return = YSOS_ERROR_SUCCESS;
      break;
    }
    YSOS_LOG_DEBUG("str_text_need_tts = " << str_text_need_tts);

    static std::string last_tts_text;
    if (strcasecmp(last_tts_text.c_str(), str_text_need_tts.c_str()) != 0) {
      int n_result = cxfasr_.TDStartTTS(0, str_text_need_tts);
      if (YSOS_ERROR_SUCCESS != n_result)
        break;
      last_tts_text = str_text_need_tts;
    }
    // get audio data
    bool is_finish = false;
    UINT8* pout = NULL;
    UINT32 out_buffer_size = 0;
    pOut->buf_interface_ptr->GetBufferAndLength(&pout,&out_buffer_size);
    {
      UINT32 max_length = 0, prefix_length = 0;
      pOut->buf_interface_ptr->GetMaxLength(&max_length);
      pOut->buf_interface_ptr->GetPrefixLength(&prefix_length);
      out_buffer_size = max_length - prefix_length;
    }
    if (out_buffer_size < 2) { //< 预留两位作控制字段
      YSOS_LOG_DEBUG("error the buffer is to small");
      break;
    }
    int iaudio_len = 0;
    int n_result = cxfasr_.GetTTSAudio(reinterpret_cast<char*>(pout +2), out_buffer_size-2, &iaudio_len, &is_finish);
    if (YSOS_ERROR_SUCCESS != n_result) {
      break;
    }
    pOut->is_not_finish = is_finish ? false :true;

    if (iaudio_len <= 0) { //< 本次合成数据没有，有可能是本地合成数据比较小，需要跟后面合成的数据组成一个大块。
      pOut->buf_interface_ptr->SetLength(0);
      n_return = YSOS_ERROR_SUCCESS;
      break;
    }

    pOut->buf_interface_ptr->SetLength(iaudio_len + 2);
    *pout = is_finish;  ///< 填充协议头的第一个字节
    if (is_finish) {
      last_tts_text = "";
    }
    *(pout+1) = interrupt;

    n_return = YSOS_ERROR_SUCCESS;
  } while (0);

  return n_return;
}

int AudioExtDriver::AbilityTTS_Text2PCM(AbilityParam* input_param, AbilityParam* pOut) {
  int n_return = YSOS_ERROR_FAILED;
  YSOS_LOG_DEBUG("AbilityTTS_Text2PCM in");

  do {
    if (NULL == input_param->buf_interface_ptr || NULL == pOut->buf_interface_ptr) {
      YSOS_LOG_DEBUG("buffer error");
      break;
    }

    UINT8* ptext = NULL;
    UINT32 buffer_size =0;
    input_param->buf_interface_ptr->GetBufferAndLength(&ptext,&buffer_size);
    std::string str_text_need_tts = reinterpret_cast<char*>(ptext);
    static std::string last_tts_text;
    if (strcasecmp(last_tts_text.c_str(), str_text_need_tts.c_str()) !=0) {
      cxfasr_.TDStartTTS(0, str_text_need_tts);
      last_tts_text = str_text_need_tts;
    }
    /// get audio data
    bool is_finish = false;
    UINT8* pout =NULL;
    UINT32 out_buffer_size = 0;
    pOut->buf_interface_ptr->GetBufferAndLength(&pout,&out_buffer_size);
    {
      UINT32 max_length = 0, prefix_length =0;
      pOut->buf_interface_ptr->GetMaxLength(&max_length);
      pOut->buf_interface_ptr->GetPrefixLength(&prefix_length);
      out_buffer_size = max_length - prefix_length;
    }

    if (out_buffer_size < 0) {
      YSOS_LOG_DEBUG("error the buffer is to small");
      break;
    }
    int iaudiol_len = 0;
    int n_result = cxfasr_.GetTTSAudio(reinterpret_cast<char*>(pout), out_buffer_size, &iaudiol_len, &is_finish);
    if (YSOS_ERROR_SUCCESS != n_result)
      break;

    pOut->is_not_finish = is_finish ? false :true;
    if (iaudiol_len <= 0) { //< 本次合成数据没有，有可能是本地合成数据比较小，需要跟后面合成的数据组成一个大块
      pOut->buf_interface_ptr->SetLength(0);
      n_return = YSOS_ERROR_SUCCESS;
      break;
    }

    pOut->buf_interface_ptr->SetLength(iaudiol_len);
    if (is_finish) {
      last_tts_text = "";
    }

    n_return = YSOS_ERROR_SUCCESS;
  } while (0);

  return n_return;
}

int AudioExtDriver::AbilityTTS_JsonExtract2PCMP2(AbilityParam* pInput, std::vector<std::string>& extra_params, AbilityParam* pOut) {
  int n_return = YSOS_ERROR_FAILED;
  YSOS_LOG_DEBUG("AbilityTTS_JsonExtract2PCMP2 in");

  do {
    if (NULL == pInput->buf_interface_ptr || NULL == pOut->buf_interface_ptr) {
      YSOS_LOG_DEBUG("buffer error");
      break;
    }

    if (extra_params.size() < 2) {
      YSOS_LOG_DEBUG("error the extract param is null");
      break;
    }

    //int interrupt_type;
    UINT8* ptext = NULL;
    UINT32 buffer_size = 0;
    pInput->buf_interface_ptr->GetBufferAndLength(&ptext,&buffer_size); //< 里面包含两个字符串，得提取出来
    std::string str_text_need_tts = reinterpret_cast<char*>(ptext);
    std::string extract_field = extra_params.size()>=2 ? extra_params[1]: "";
    std::string str_service;
    if (extract_field.length()>0 && str_text_need_tts.length()>0) { //< 摄取json中特定的字段
      JsonValue json_value;
      JsonReader json_reader;
      json_reader.parse(str_text_need_tts, json_value, true);
      str_text_need_tts = json_value.get(extract_field, "").asString(); ///< 提取出来特定的字符串
      //interrupt_type = json_value.get("interrupt_type", 0).asInt();
      str_service = json_value.get("service", "").asString();
    }

    if (str_text_need_tts.length() <= 0) { //< 如果传入为空，则返回空数据
      pOut->buf_interface_ptr->SetLength(0);
      n_return = YSOS_ERROR_SKIP;
      break;
    }

    //< 医疗项目需求，指定的service不进行语音播报
    //TODO 按理不应该写在这的
    if (cxfasr_.list_tts_broadcast_filter_.size() > 0) {
      std::list<std::string>::iterator it = cxfasr_.list_tts_broadcast_filter_.begin();
      for (it; it != cxfasr_.list_tts_broadcast_filter_.end(); it++) {
        std::string filter_service = *it;
        if (str_service == filter_service) {
          YSOS_LOG_DEBUG("find key word filter_service = " << filter_service);
          pOut->buf_interface_ptr->SetLength(0);
          return YSOS_ERROR_SKIP;
        }
      }
    }

    static std::string last_tts_text;
    if (strcasecmp(last_tts_text.c_str(), str_text_need_tts.c_str()) !=0) {
      /*int tempType = GetUtility()->GetEncoding(str_text_need_tts);
      std::string temp_str;
      if ( Utility::UTF8 == tempType) { ///< utf-8 //  NOLINT
            temp_str = GetUtility()->Utf8ToAscii(str_text_need_tts);
      } else if (Utility::UTF16 == tempType) { ///<  utf-16 //  NOLINT
            temp_str = GetUtility()->UnicodeToAcsii((wchar_t*)str_text_need_tts.c_str());
      } else { ///<  gbk  //  NOLINT
            temp_str = str_text_need_tts;
      }*/
      //printf("str_text_need_tts 00 = [%s]\n",  GetUtility()->AsciiToUtf8(temp_str).c_str());
      
      YSOS_LOG_DEBUG("get TDStartTTS data [Utf-8] = " << GetUtility()->AsciiToUtf8(str_text_need_tts));
      int n_result = cxfasr_.TDStartTTS(0, str_text_need_tts);
      if (YSOS_ERROR_SUCCESS != n_result)
        break;
      last_tts_text = str_text_need_tts;
    }
    /// get audio data
    bool is_finish = false;
    UINT8* pout = NULL;
    UINT32 out_buffer_size = 0;
    pOut->buf_interface_ptr->GetBufferAndLength(&pout,&out_buffer_size);
    {
      UINT32 max_length = 0, prefix_length = 0;
      pOut->buf_interface_ptr->GetMaxLength(&max_length);
      pOut->buf_interface_ptr->GetPrefixLength(&prefix_length);
      out_buffer_size = max_length - prefix_length;
    }
    if (out_buffer_size < 2) {
      YSOS_LOG_DEBUG("error the buffer is to small");
      break;
    }
    int iaudio_len = 0;
    int n_result = cxfasr_.GetTTSAudio(reinterpret_cast<char*>(pout +2), out_buffer_size-2, &iaudio_len, &is_finish);
    if (YSOS_ERROR_SUCCESS != n_result) {
      break;
    }
    pOut->is_not_finish = is_finish ? false :true;

    if (iaudio_len <= 0) { //< 本次合成数据没有，有可能是本地合成数据比较小，需要跟后面合成的数据组成一个大块
      pOut->buf_interface_ptr->SetLength(0);
      n_return = YSOS_ERROR_SUCCESS;
      break;
    }

    if (!tts_pcm_dir_.empty()) { ///< pcm file
      if (tts_file_ == NULL) {
        std::string strTime = boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time());
        strTime.replace(strTime.find('T'), 1, "_");
        std::string pcm_dir = tts_pcm_dir_ + "TDVoice_" + strTime + ".pcm";
        tts_file_ = fopen(pcm_dir.c_str(), "wb");
        if (NULL == tts_file_) {
          YSOS_LOG_DEBUG("open file failed");
          break;
        }
      }
      fwrite(pout+2, iaudio_len, 1, tts_file_);
    }

    pOut->buf_interface_ptr->SetLength(iaudio_len + 2);
    *pout = is_finish; //< 填充协议头的第一个字节
    if (is_finish) {
      YSOS_LOG_DEBUG("get tts finish, last_tts_text = " << last_tts_text);
      last_tts_text = "";
      if (tts_file_ != NULL) { ///< pcm file
        fclose(tts_file_);
        tts_file_ = NULL;
      }
    }
    //itoa(interrupt_type, (char*)(pout+1), 10);

    n_return = YSOS_ERROR_SUCCESS;
  } while (0);

  YSOS_LOG_DEBUG("AbilityTTS_JsonExtract2PCMP2 out");
  return n_return;
}

int AudioExtDriver::RegisterOrUnregisterAbilityEvent(bool is_register, BaseInterface* module_interface, std::string* ability_name) {

  return YSOS_ERROR_SUCCESS;
}

}  /// end of namespace ysos
