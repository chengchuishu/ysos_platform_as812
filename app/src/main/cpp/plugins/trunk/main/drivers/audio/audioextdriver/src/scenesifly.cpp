/// self headers
//#include "StdAfx.h"
#include "../include/scenesifly.h"

#ifdef _WIN32
/// windows headers
#include <Windows.h>
#else
#endif

/// stl ehaders
#include <string>

/// boost headers
#include <boost/locale/conversion.hpp>
#include <boost/locale.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/filesystem.hpp>
/// ysos headers
#include "../../../../../../core/trunk/main/public/include/core_help_package/jsonutility.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/xmlutil.h"

/// private headers
#include "../include/msgdefine.h"
//#include "stringext.h"

ScenesIFLY::ScenesIFLY(void) {
  asr_flag_atomic_ =0;
  asr_ownerid_atomic_ =0;
  asr_flag0_only_asr_ =0;
}

ScenesIFLY::~ScenesIFLY(void) {
}

int ScenesIFLY::Login() {  ///< 如果登录失败，则会尝试退出，再登录
  int ibase_ret = VoiceInterfaceIFLY::Login();
  if (YSOS_ERROR_SUCCESS == ibase_ret)
    return YSOS_ERROR_SUCCESS;

  YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "ifly Login fail , logout first then login");
  //先logout
  VoiceInterfaceIFLY::Logout();
  return VoiceInterfaceIFLY::Login();
}



int ScenesIFLY::ConvertNlpResultToStandasd(const char* pCallAbility, const char* pNlpResult, std::string& recvStandardNlpString) {
  assert(pNlpResult);
  assert(pCallAbility);
  if (NULL == pNlpResult) {
    recvStandardNlpString = "";
    return YSOS_ERROR_SUCCESS;
  }
  if (0 == strlen(pNlpResult)) {
    recvStandardNlpString = "";
    return YSOS_ERROR_SUCCESS;
  }

  try {
    std::string unicodestring = boost::locale::conv::to_utf<char>(pNlpResult, /*"gb2312"*/"gbk");
    boost::property_tree::ptree ptsource, ptdest;
    std::istringstream stream(unicodestring);
    boost::property_tree::json_parser::read_json(stream, ptsource);

    //standard:text
    std::string unicode_strasr_text = ptsource.get<std::string>("text", "");
    ptdest.put("text", unicode_strasr_text);

    //standard:rc
    std::string unicode_rc = ptsource.get<std::string>("rc", "4");  ///< 迅飞，如果rc为4,则表示正确返回结果，但结果为空
    if (strcasecmp("nlp", pCallAbility) ==0) {
      ptdest.put("rc", "0");
    } else { /*if(stricmp("asrnlp", pCallAbility) ==0) */
      ptdest.put("rc", "0");
    }

    //standard:rg
    ptdest.put("rg", "0");

    //standard:answer_best
    std::string unicode_answer_best = ptsource.get<std::string>("answer.text", "");
    ptdest.put("answer_best", unicode_answer_best);

    //standard:service
    std::string unicode_service = ptsource.get<std::string>("service", "");
    ptdest.put("service", unicode_service);

    //standard:other_answers
    boost::property_tree::ptree vec_other_answers_dest, other_answer_default;
    boost::property_tree::ptree vec_other_answers_source = ptsource.get_child("moreResults", other_answer_default);
    BOOST_FOREACH(boost::property_tree::ptree::value_type &v, vec_other_answers_source) {
      boost::property_tree::ptree other_answer_source = v.second;
      std::string unicode_other_answer_text = other_answer_source.get<std::string>("text", "");
      std::string unicode_other_answer_service = other_answer_source.get<std::string>("service", "");
      //std::string unicode_answer_operation = other_answer_source.get<std::string>("operation");
      std::string unicode_other_answer_answer = other_answer_source.get<std::string>("answer.text", "");

      boost::property_tree::ptree other_answer_dest;
      other_answer_dest.put("text", unicode_other_answer_text);
      other_answer_dest.put("service", unicode_other_answer_service);
      // other_answer_dest.put("operation", unicode_answer_operation);
      other_answer_dest.put("answer", unicode_other_answer_answer);
      vec_other_answers_dest.push_back(std::make_pair("", other_answer_dest));
    }
    ptdest.add_child("other_answers", vec_other_answers_dest);

    //如果是music
    if (strcasecmp("music", unicode_service.c_str()) ==0) { ///< 如果是音乐
      // standard:url
      {
        /// 取所有可用歌曲中的第一首歌的地址
        boost::property_tree::ptree tree_songs,  tree_empty;
        tree_songs = ptsource.get_child("data.result", tree_empty);
        BOOST_FOREACH(boost::property_tree::ptree::value_type& v_ii, tree_songs) {
          std::string unicode_url = v_ii.second.get<std::string>("downloadUrl", "");
          ptdest.put("content.url", unicode_url);
          break;
        }
      }

      //standard:argist
      std::string unicode_artist = ptsource.get<std::string>("semantic.slots.artist", "");
      ptdest.put("content.artist", unicode_artist);

      //standard:song
      std::string unicode_song = ptsource.get<std::string>("semantic.slots.song", "");
      ptdest.put("content.song", unicode_song);

    } else if (strcasecmp("weather", unicode_service.c_str()) ==0) { ///< 如果是天气
      // standard:url
      std::string unicode_url = ptsource.get<std::string>("webPage.url", "");
      ptdest.put("content.url", unicode_url);

      //standard:locatioin(city, datetime)
      std::string unicode_city = ptsource.get<std::string>("semantic.slots.location.city", "");
      ptdest.put("content.location.city", unicode_city);
      std::string unicode_datetime = ptsource.get<std::string>("semantic.slots.datetime.date", "");
      //if(stricmp("CURRENT_DAY", unicode_datetime))  ///< 这里要转为当前的日期,在下面判断，取天气列表的第一条日期数据
      //TODO
      ptdest.put("content.location.date", unicode_datetime);

      //standard:data
      boost::property_tree::ptree vec_datas_source, vec_datas_dest, ptree_data_default;
      vec_datas_source = ptsource.get_child("data.result", ptree_data_default);
      int i=-1;
      BOOST_FOREACH(boost::property_tree::ptree::value_type &v, vec_datas_source) {
        ++i;
        boost::property_tree::ptree p = v.second;
        std::string unicode_air_quality = p.get<std::string>("airQuality", "");
        std::string unicode_date_child = p.get<std::string>("date", "");
        std::string unicode_city_child = p.get<std::string>("city", "");
        std::string unicode_humidity = p.get<std::string>("humidity", "");
        std::string unicode_windlevel = p.get<std::string>("windLevel", "");
        std::string unicode_weather = p.get<std::string>("weather", "");
        std::string unicode_temp_range = p.get<std::string>("tempRange", "");
        std::string unicode_wind = p.get<std::string>("wind", "");

        boost::property_tree::ptree weather_data;
        weather_data.put("air_quality", unicode_air_quality);
        weather_data.put("date", unicode_date_child);
        weather_data.put("city", unicode_city_child);
        weather_data.put("humidity", unicode_humidity);
        weather_data.put("wind_level", unicode_windlevel);
        weather_data.put("weather", unicode_weather);
        weather_data.put("temp_range", unicode_temp_range);
        weather_data.put("wind", unicode_wind);
        vec_datas_dest.push_back(std::make_pair("", weather_data));

        if (0 ==i) {
          if (strcasecmp("CURRENT_DAY", unicode_datetime.c_str()) ==0) { ///< 如果 semantic.slots.datetime.date 为 CURRENT_DAY， 则提取天气结果中的第一个天气中的date字段
            unicode_datetime = unicode_date_child;
            ptdest.put("content.location.date", unicode_datetime);
          }
          if (strcasecmp("CURRENT_CITY", unicode_city.c_str()) ==0
              || unicode_city.length() <=0) {                      ///< 如果semantic.slots.location.city 为 CURRENT_CITY 或为空字符串，则提取天气结果中第一个天气中的city字段
            unicode_city = unicode_city_child;
            ptdest.put("content.location.city", unicode_city);
          }
        }
      }
      ptdest.add_child("data", vec_datas_dest);
    } else {  ///< 其他类型
      // 如果最佳答案为空，是从备选答案中选中一个
      if (unicode_answer_best.length() ==0 && false == vec_other_answers_dest.empty()) {
        BOOST_FOREACH(boost::property_tree::ptree::value_type &v, vec_other_answers_source) {
          boost::property_tree::ptree other_answer_source = v.second;
          //std::string unicode_other_answer_text = other_answer_source.get<std::string>("text", "");
          std::string unicode_other_answer_service = other_answer_source.get<std::string>("service", "");
          //std::string unicode_answer_operation = other_answer_source.get<std::string>("operation");
          std::string unicode_other_answer_answer = other_answer_source.get<std::string>("answer.text", "");
          if (unicode_other_answer_answer.length() <=0 || unicode_other_answer_service.length() <=0)
            continue;

          /// 使用备选答案
          ptdest.put("answer_best", unicode_other_answer_answer);
          ptdest.put("service", unicode_other_answer_service);
          break;
        }
      }
    }
    //ptdest.put("service", "asrdata");
    ptdest.put("asr_mode", "cloud");

    std::ostringstream ostream_result;
    boost::property_tree::json_parser::write_json(ostream_result, ptdest);
    recvStandardNlpString = boost::locale::conv::from_utf<char>(ostream_result.str(), "gbk");
    return YSOS_ERROR_SUCCESS;
  } catch (...) {
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "json decode error :ReceiveResult: " << pNlpResult);
    recvStandardNlpString="";
    return YSOS_ERROR_FAILED;
  }
}



int ScenesIFLY::ConvertLocalNlpResultToStandasd(const char* pCallAbility, const char* pNlpResult, std::string& recvStandardNlpString) {
  try {
    if (NULL == pCallAbility || NULL == pNlpResult) {
      return YSOS_ERROR_FAILED;
    }

    std::string answer_old;
    std::string service;
    std::string engine;

    boost::property_tree::ptree tree_xml;
    std::istringstream in_stream(pNlpResult);
    boost::property_tree::read_xml(in_stream, tree_xml);

    std::string question = tree_xml.get("nlp.rawtext", "");
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "local asr_result = " << question);
    if (!question.empty()) {
      service = tree_xml.get("nlp.result.focus", "");
      YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "local service = " << service);
      GetAsrInfo(service, answer_old);
      YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "GetAsrInfo service = " << service);
      engine = "local";
    } else {
      // question = GetXmlUtil()->GetElementTextValue(xml_element, "text");
      engine = "cloud";
    }

    Json::Value json_asr;
    Json::FastWriter json_writer;
    json_asr["rc"] = "0";
    json_asr["rg"] = "0";
    json_asr["text"] = question;
    json_asr["answer_best"] = "";
    json_asr["answer_old"] = answer_old;
    json_asr["other_answers"] = "";
    json_asr["service"] = service;
    json_asr["time_interval"] = 0;
    json_asr["asr_mode"] = engine;
    recvStandardNlpString = json_writer.write(json_asr);
    recvStandardNlpString = GetUtility()->ReplaceAllDistinct ( recvStandardNlpString, "\\r\\n", "" );
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "local recvStandardNlpString = " << recvStandardNlpString);

    return YSOS_ERROR_SUCCESS;
  } catch (...) {
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "json decode error :ReceiveResult: " << pNlpResult);
    recvStandardNlpString="";
    return YSOS_ERROR_FAILED;
  }
}


/*
int ScenesIFLY::ConvertLocalNlpResultToStandasd(const char* pCallAbility, const char* pNlpResult, std::string& recvStandardNlpString) {
  try {
    if (NULL == pCallAbility || NULL == pNlpResult) {
      return YSOS_ERROR_FAILED;
    }

    std::string answer_old;
    std::string service;
    std::string engine;

    ysos::XmlDocument xml_doc;
    ysos::XMLError xml_error = xml_doc.Parse(pNlpResult);
    if (0 != xml_error) {
      YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "pNlpResult parse error");
      return YSOS_ERROR_FAILED;
    }
    ysos::XmlElement *xml_element = xml_doc.RootElement();
    std::string question = GetXmlUtil()->GetElementTextValue(xml_element, "rawtext");
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "local asr_result = " << question);
    if (!question.empty()) {
      ysos::XmlElement *xml_element2 = xml_element->FirstChildElement("result");
      service = GetXmlUtil()->GetElementTextValue(xml_element2, "focus");
      YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "local service = " << service);
      GetAsrInfo(service, answer_old);
      YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "GetAsrInfo service = " << service);
      engine = "local";
    } else {
      question = GetXmlUtil()->GetElementTextValue(xml_element, "text");
      engine = "cloud";
    }

    Json::Value json_asr;
    Json::FastWriter json_writer;
    json_asr["rc"] = "0";
    json_asr["rg"] = "0";
    json_asr["text"] = question;
    json_asr["answer_best"] = "";
    json_asr["answer_old"] = answer_old;
    json_asr["other_answers"] = "";
    json_asr["service"] = service;
    json_asr["time_interval"] = 0;
    json_asr["asr_mode"] = engine;
    recvStandardNlpString = json_writer.write(json_asr);
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "local recvStandardNlpString = " << recvStandardNlpString);

    return YSOS_ERROR_SUCCESS;
  } catch (...) {
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "json decode error :ReceiveResult: " << pNlpResult);
    recvStandardNlpString="";
    return YSOS_ERROR_FAILED;
  }
}*/


void ScenesIFLY::GetAsrInfo(std::string& service, std::string& answer_old) {
  do {
    if ("walkw" == service) {
      service = "walk_w";
      break;
    } else if ("walks" == service) {
      service = "walk_s";
      break;
    } else if ("walka" == service) {
      service = "walk_a";
      break;
    } else if ("walkd" == service) {
      service = "walk_d";
      break;
    } else if ("creditcard" == service) {
      service = "credit";
      break;
    } else if ("card|report" == service) {
      service = "report";
    } else if ("creditcard|repay" == service) {
      service = "repay";
    } else if ("account|query" == service) {
      service = "accounts";
    } else {
      std::string::size_type position;
      position = service.find("confirm");
      if (std::string::npos != position) {
        service = "comfirm";
        break;
      }
      position = service.find("noprint");
      if (std::string::npos != position) {
        service = "noprint";
        break;
      }
      position = service.find("print");
      if (std::string::npos != position) {
        service = "print";
        break;
      }
      position = service.find("currentdeposit");
      if (std::string::npos != position) {
        service = "currentdeposit";
        break;
      }
      position = service.find("fixedeposit");
      if (std::string::npos != position) {
        service = "fixedeposit";
        break;
      }
      position = service.find("currentflow");
      if (std::string::npos != position) {
        service = "currentflow";
        break;
      }
      position = service.find("historicalflow");
      if (std::string::npos != position) {
        service = "historicalflow";
        break;
      }
      position = service.find("transferin");
      if (std::string::npos != position) {
        service = "transferin";
        break;
      }
      position = service.find("transferout");
      if (std::string::npos != position) {
        service = "transferout";
        break;
      }
      std::string::size_type pos_query = service.find("query");
      std::string::size_type pos_account = service.find("account");
      std::string::size_type pos_balance = service.find("balance");
      if ((std::string::npos != pos_query) && (std::string::npos != pos_balance)) {
        service = "remainder";
        break;
      }
      std::string::size_type pos_flow = service.find("flow");
      if ((std::string::npos != pos_query) && (std::string::npos != pos_flow)) {
        service = "records";
        break;
      }
      std::string::size_type pos_credit = service.find("creditcard");
      std::string::size_type pos_bill = service.find("bill");
      if ((std::string::npos != pos_credit) && (std::string::npos != pos_account)) {
        service = "ledger";
        break;
      } else if ((std::string::npos != pos_credit) && (std::string::npos != pos_bill)) {
        service = "check";
        break;
      }
      if ((std::string::npos != pos_query) && (std::string::npos != pos_account)) {
        service = "accounts";
        break;
      }
      position = service.find("finance");
      if (std::string::npos != position) {
        service = "finance";
        break;
      }
      position = service.find("notid2");
      if (std::string::npos != position) {
        service = "notid2";
        break;
      } 
      position = service.find("id2");
      if (std::string::npos != position) {
        service = "id2";
        break;
      }
      position = service.find("telecom");
      if (std::string::npos != position) {
        service = "telecom";
        break;
      }
      position = service.find("traffic");
      if (std::string::npos != position) {
        service = "trafficfines";
        break;
      }
      position = service.find("unicom");
      if (std::string::npos != position) {
        service = "unicom";
        break;
      }
      position = service.find("mobile");
      if (std::string::npos != position) {
        service = "mobile";
        break;
      }
      position = service.find("electric");
      if (std::string::npos != position) {
        service = "electricity";
        break;
      }
      position = service.find("cardnumber");
      if (std::string::npos != position) {
        service = "cardnumber";
        break;
      }
      position = service.find("catv");
      if (std::string::npos != position) {
        service = "catv";
        break;
      }

      position = service.find("walkA1");
      if (std::string::npos != position) {
        service = "walkA";
        answer_old = "*yb";
        break;
      }
      position = service.find("walkA2");
      if (std::string::npos != position) {
        service = "walkA";
        answer_old = "*xxq";
        break;
      }
      position = service.find("walkA3");
      if (std::string::npos != position) {
        service = "walkA";
        answer_old = "*zzsb";
        break;
      }
      position = service.find("walkA4");
      if (std::string::npos != position) {
        service = "walkA";
        answer_old = "*gt";
        break;
      }
    }
  } while (0);
}


int ScenesIFLY::IsNlpDataHaveResult(const  Json::Value& json_result, bool& is_have_nlp_result) {
  /// 如果有service字段，则表示是有nlp结果的
  try {
    std::string current_services = json_result.get("service", "").asString();
    if (current_services.length() >0) {
      is_have_nlp_result = true;
    } else {
      is_have_nlp_result = false;
    }

  } catch (...) {
    is_have_nlp_result = false;
  }
  return YSOS_ERROR_SUCCESS;
}


bool ScenesIFLY::IsFilterByService(/*Json::Value& result_json*/std::string& current_services) {
  bool is_filter = false;  ///< 是否被过滤
  if (vec_nlp_only_include_service_.size() <=0 && vec_nlp_exclude_service_.size() <=0)
    return false;   ///< 没有配置过滤的选项

  //std::string current_services = result_json.get("service", "").asString(); // result_json.get<std::string>("service", "");

  if (vec_nlp_only_include_service_.size() >0) {
    /// 必须保存在指点定的服务内
    if (current_services.length() <=0) {
      YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "被过滤，没有在被包含的服务内.当前服务:" << current_services.c_str());
      return true;   ///< 被过滤，没有在被包含的服务内.
    }
    std::vector<std::string>::iterator it_find= find(vec_nlp_only_include_service_.begin(), vec_nlp_only_include_service_.end(), current_services);
    if (it_find  == vec_nlp_only_include_service_.end()) {
      YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "被过滤，没有在被包含的服务内.当前服务:" << current_services.c_str());
      return true;
    } else {
      return false;  ///< 在被包含的服务内，不过滤
    }

  } else if (vec_nlp_exclude_service_.size() >0) {
    /// 如果命中则需要过滤,
    std::vector<std::string>::iterator it_find= find(vec_nlp_exclude_service_.begin(), vec_nlp_exclude_service_.end(), current_services);
    if (it_find != vec_nlp_exclude_service_.end()) {
      YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "被过滤，在被排除的服务中, 当前服务:" << current_services.c_str());
      return true;
    } else {
      return false;  ///< 没有在被排除的服务中
    }
  }
  assert(false); ///< not execute here
  return false;
}



int ScenesIFLY::DomSomethingBeforeAsr() {
  /// 如果没有登录，则登录
  if (false == IsHaveLoginIn()) {
    int mspret = Login();
    if (YSOS_ERROR_SUCCESS != mspret) {
      YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "ASRSessionBegin MSPLogin fail");
      assert(false);
      return YSOS_ERROR_FAILED;
    } else {
      YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "ASRSessionBegin MSPLogin true");
    }
  }

  {
    std::string temp_asr_using_engine_type;
    {
      boost::lock_guard<boost::mutex> guard(mutex_member_);
      temp_asr_using_engine_type = asr_using_engine_type_;
    }

    if (strcasecmp(temp_asr_using_engine_type.c_str(), "local") ==0
        || strcasecmp(temp_asr_using_engine_type.c_str(), "mixed") ==0) {
      std::string error_message;
      ASRSetGrammarFile(NULL, &error_message);
    }
  }
  return YSOS_ERROR_SUCCESS;
}


int ScenesIFLY::ASRGetResult(const char* sessionID, int* rsltStatus,std::string& ReceiveResult) {
  int ibaseret = VoiceInterfaceIFLY::ASRGetResult(sessionID, rsltStatus, ReceiveResult);
  if (YSOS_ERROR_SUCCESS != ibaseret) {
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "ASRGetResult error, ibaseret = " << ibaseret);
    return ibaseret;
  }

  if (ReceiveResult.length() >0) {
    strcurrent_sum_result_asr_ += ReceiveResult;
  }

  ReceiveResult = strcurrent_sum_result_asr_;  ///< 得到的结果是ASR+NLP
  if (TDMSP_REC_STATUS_COMPLETE != *rsltStatus) {
    return YSOS_ERROR_SUCCESS;
  }
  //TODO:note string encode
  //ReceiveResult = boost::locale::conv::between( ReceiveResult, "UTF-8", "GBK" );//GBK==gbk2312
  YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "ReceiveResult = " << ReceiveResult << "; asr_flag_atomic_ = " << asr_flag_atomic_);
  /// 如果所有的数据都拿到后，是获取其他数据
  if (0 == asr_flag_atomic_
      || 4 == asr_flag_atomic_) {  ///<  普通非打断模式（asr + nlp), 判断是否需要去获取客服系统的数据

    if (0 == ReceiveResult.length()) { ///< not asr result,return empty string
      return YSOS_ERROR_SUCCESS;
    }

    try {
      int iret = -1;
      if (ReceiveResult.length() >0 && '{' == ReceiveResult[0]) {
        YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "111111");
        iret = ConvertNlpResultToStandasd("asrnlp", ReceiveResult.c_str(), ReceiveResult);
      } else if (ReceiveResult.length() >0 && '<' == ReceiveResult[0]) {
        YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "1111");
        iret = ConvertLocalNlpResultToStandasd("asrnlp", ReceiveResult.c_str(), ReceiveResult);
      } else {
        iret = YSOS_ERROR_SUCCESS;
        Json::Value json_asr;
        json_asr["rc"] = "0";
        json_asr["rg"] = "0";
        json_asr["text"] = ReceiveResult;
        json_asr["answer_best"] = "";
        json_asr["answer_old"] = "";
        json_asr["other_answers"] = "";
        json_asr["service"] = "";
        json_asr["time_interval"] = 0;
        std::string engine_type;
        ASRGetEngineType(engine_type);
        json_asr["asr_mode"] = /*""*/ engine_type;
        Json::FastWriter json_writer;
        ReceiveResult = json_writer.write(json_asr);
        ReceiveResult = GetUtility()->ReplaceAllDistinct ( ReceiveResult, "\\r\\n", "" );
        YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "1111");
      }
      return iret;
    } catch (...) {
      YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "json decode error :ReceiveResult: " << ReceiveResult.c_str());
      ReceiveResult = "";
      return YSOS_ERROR_FAILED;
    }
  } else if (2 == asr_flag_atomic_) {
    return YSOS_ERROR_SUCCESS;
  } else if (3== asr_flag_atomic_
             || 5 == asr_flag_atomic_) {
    return YSOS_ERROR_SUCCESS;
  }  else {
    assert(false);
    YSOS_LOG_ERROR_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "error , not suport this asr flag: " << asr_flag_atomic_);
    return YSOS_ERROR_FAILED;
  }
  return YSOS_ERROR_SUCCESS;
}


int ScenesIFLY::NLP2Child(const char* pTextNeedNlp, bool filter_null_result,std::string& ReceiveNLPResult, const bool permanent_caching) {
  if (NULL == pTextNeedNlp || 0 == strlen(pTextNeedNlp)) {
    ReceiveNLPResult = "";
    return YSOS_ERROR_FAILED;
  }

  //获取chching 中的数据
  int have_get_caching_data =GetCachingNlpData(VoiceType_IFLY,pTextNeedNlp, ReceiveNLPResult);
  YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "get caching nlpdata:" << ReceiveNLPResult.c_str());
  if (YSOS_ERROR_SUCCESS != have_get_caching_data) {
    int ibaseret = VoiceInterfaceIFLY::NLP(pTextNeedNlp, ReceiveNLPResult);
    if (YSOS_ERROR_SUCCESS != ibaseret) {
      //nlp_success = false;
      YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "nlp error, nlptext:" << pTextNeedNlp);
      ReceiveNLPResult = "";
      return YSOS_ERROR_FAILED;
    }
  }

  if (ReceiveNLPResult.length() <=0) { ///< 结果为空
    return YSOS_ERROR_SUCCESS;
  }
  try {

    Json::Value root_json;
    Json::Reader json_reader;
    json_reader.parse(ReceiveNLPResult, root_json, 0);
    if (filter_null_result) { ///< 过滤无效数据
      int rc_code = root_json.get("rc", -1).asInt();
      if (0 != rc_code) {
        YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "由于迅飞返回的结果表示没有，当前被过滤,不保存到缓存中");
        ReceiveNLPResult = "";
        return YSOS_ERROR_SUCCESS;
      }
    }

    std::string current_services = root_json.get("service", "").asString();
    if (IsFilterByService(/*root_json*/current_services)) {
      YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "被配置的服务被过滤");
      ReceiveNLPResult = "";
      return YSOS_ERROR_SUCCESS;
    }

    std::string temp_nlp_version;
    ASRGetKeyParam("nlp_version", temp_nlp_version);
    if (strcasecmp("3.0", temp_nlp_version.c_str()) ==0) {
      if (YSOS_ERROR_SUCCESS != have_get_caching_data) { ///< 只有不被命中的才会转
        ConvertNlp3To2(root_json);
      }
    }
    root_json["nlptype"] =0;

    Json::FastWriter json_writer;
    ReceiveNLPResult = json_writer.write(root_json);
    ReceiveNLPResult = GetUtility()->ReplaceAllDistinct ( ReceiveNLPResult, "\\r\\n", "" );
    if (YSOS_ERROR_SUCCESS != have_get_caching_data) {
      YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "结果保存到缓存中：" << ReceiveNLPResult.c_str());
      CachingNlpData(VoiceType_IFLY, pTextNeedNlp, ReceiveNLPResult, permanent_caching);
    }

  } catch (...) {
    // printf("json decode error");
    YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "in VoiceInterfaceIFLY nlp json decode error, 结果被过滤");
    //ReceiveNLPResult = "";
    char error_msg_buf[1024] = {'\0'};
    int code = TDASRERROR_NLP_FAIL;
    std::string error_msg = "解析数据失败";
    //sprintf_s(error_msg_buf,sizeof(error_msg_buf),"{\"text\":\"抱歉网络不好，请再试一次\",\"rc\":\"%d\",\"rg\":\"%d\",\"errordetail\":\"%s\",\"answer_best\":\"\"}",code,code,error_msg.c_str());
    sprintf(error_msg_buf,"{\"text\":\"抱歉网络不好，请再试一次\",\"rc\":\"%d\",\"rg\":\"%d\",\"errordetail\":\"%s\",\"answer_best\":\"\"}",code,code,error_msg.c_str());
    ReceiveNLPResult = error_msg_buf;
    return YSOS_ERROR_FAILED;
  }

  ///<转换标准格式
  int iret = ConvertNlpResultToStandasd("asrnlp", ReceiveNLPResult.c_str(), ReceiveNLPResult);

  if (YSOS_ERROR_SUCCESS != iret) {
    char error_msg_buf[1024] = {'\0'};
    int code = TDASRERROR_NLP_FAIL;
    std::string error_msg = "解析数据失败";
    //sprintf_s(error_msg_buf,sizeof(error_msg_buf),"{\"text\":\"抱歉网络不好，请再试一次\",\"rc\":\"%d\",\"rg\":\"%d\",\"errordetail\":\"%s\",\"answer_best\":\"\"}",code,code,error_msg.c_str());
    sprintf(error_msg_buf,"{\"text\":\"抱歉网络不好，请再试一次\",\"rc\":\"%d\",\"rg\":\"%d\",\"errordetail\":\"%s\",\"answer_best\":\"\"}",code,code,error_msg.c_str());
    ReceiveNLPResult = error_msg_buf;
    return YSOS_ERROR_FAILED;
  }

  return YSOS_ERROR_SUCCESS;
}

int ScenesIFLY::NLP2(const char* pTextNeedNlp, bool filter_null_result,std::string& ReceiveNLPResult) {
  return NLP2Child(pTextNeedNlp, filter_null_result, ReceiveNLPResult, false);
}

int ScenesIFLY::NLP(const char* pTextNeedNlp,std::string& ReceiveNLPResult) {  ///<  在迅飞结果基础上添加 字段 nlptype
#ifdef _WIN32
  std::string sch_value;
  int sch_ret = ASRGetKeyParam("sch", sch_value);
  YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "sch = " << sch_ret);
  if("0" == sch_value) {
    ///<封装成json格式
    int iret = YSOS_ERROR_SUCCESS;
    Json::Value json_asr;
    json_asr["rc"] = "0";
    json_asr["rg"] = "0";
    json_asr["text"] = pTextNeedNlp;
    json_asr["answer_best"] = "";
    json_asr["answer_old"] = "";
    json_asr["other_answers"] = "";
    json_asr["service"] = "";
    json_asr["time_interval"] = 0;
    std::string engine_type;
    ASRGetEngineType(engine_type);
    json_asr["asr_mode"] = /*""*/ engine_type;

    Json::FastWriter json_writer;
    ReceiveNLPResult = json_writer.write(json_asr);
    YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "nlp result = " << ReceiveNLPResult);
    return iret;
  }else {
    return NLP2Child(pTextNeedNlp, true, ReceiveNLPResult, false);
  }
#else
  std::string sTextNeedNlp = pTextNeedNlp;
  ReceiveNLPResult = "{\"answer_best\":\"\",\"answer_old\":\"\",\"asr_mode\":\"\",\"other_answers\":\"\",\"rc\":\"0\",\"rg\":\"0\",\"service\":\"\",\"text\":\""+ sTextNeedNlp +"\",\"time_interval\":0}";
  YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "nlp result = " << ReceiveNLPResult);
  return 0;
#endif
}

int ScenesIFLY::ASRSessionBegin2(const int lFlag, const int iOwnerid, std::string& ReceiveSeesionID) {
  asr_flag_atomic_ = lFlag;
  asr_ownerid_atomic_ = iOwnerid;
  strcurrent_sum_result_asr_ = "";

  int tempflag = lFlag;
  if (asr_flag0_only_asr_) {  ///< asrnlp中只需要asr,nlp的部分由其他第三方nlp服务商提供
    if (0 == tempflag || 4 == tempflag) {
      tempflag =2;
    }
  }
  return VoiceInterfaceIFLY::ASRSessionBegin(/*lFlag*/tempflag, ReceiveSeesionID);
}

//int ScenesIFLY::ASRSessionBegin2(const int lFlag, const int iOwnerid, std::string& ReceiveSeesionID, const int mode) {
//  asr_flag_atomic_ = lFlag;
//  asr_ownerid_atomic_ = iOwnerid;
//  strcurrent_sum_result_asr_ = "";
//  return VoiceInterfaceIFLY::ASRSessionBegin(lFlag, ReceiveSeesionID, mode);
//}

int ScenesIFLY::ReadConfig(const char* pFilePath) {
  assert(pFilePath);
  if (NULL == pFilePath)
    return YSOS_ERROR_FAILED;
  ocx_file_path_ = pFilePath;
  if (NULL == pFilePath)
    ocx_file_path_ = "C:\\TDRobot\\";


  VoiceInterfaceIFLY::ReadConfig(pFilePath);
  // customer_system_jths_.ReadConfig(pFilePath);
  device_status_file_full_path_  = ocx_file_path_  + "etc\\DevStatus.ini";

  //读取板子串口号
  char config_devctrl_file_full_path[2048] = {'\0'};

  //sprintf_s(config_devctrl_file_full_path, sizeof(config_devctrl_file_full_path), "%setc\\DEV_CTRL.ini", ocx_file_path_.c_str());
  sprintf(config_devctrl_file_full_path, "%setc\\DEV_CTRL.ini", ocx_file_path_.c_str());
  YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"), "config_devctrl_file_full_path filepath:" << config_devctrl_file_full_path);


  char voide_detail_config_file_path[2048] = {'\0'};
  if (NULL == pFilePath) {
    //sprintf_s(voide_detail_config_file_path, sizeof(voide_detail_config_file_path), "%s", "C:\\TDRobot\\etc\\voice.ini");
    sprintf(voide_detail_config_file_path, "%s", "C:\\TDRobot\\etc\\voice.ini");
  } else {
    //sprintf_s(voide_detail_config_file_path, sizeof(voide_detail_config_file_path), "%setc\\voice.ini", pFilePath);
    sprintf(voide_detail_config_file_path, "%setc/voice.ini", pFilePath);
  }
  YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "readconfig filepath:" << voide_detail_config_file_path);

  char config_robot_file_full_path[1024] = {'\0'};
  if (NULL == pFilePath) {
    //sprintf_s(config_robot_file_full_path, sizeof(config_robot_file_full_path), "%s", "C:\\TDRobot\\etc\\TDRobotInfo.ini");
    sprintf(config_robot_file_full_path, "%s", "C:\\TDRobot\\etc\\TDRobotInfo.ini");
  } else {
    //sprintf_s(config_robot_file_full_path, sizeof(config_robot_file_full_path), "%setc\\TDRobotInfo.ini", pFilePath);
    sprintf(config_robot_file_full_path, "%setc/TDRobotInfo.ini", pFilePath);
  }
  YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "readconfig filepath:" << config_robot_file_full_path);

  //启用灵云客服系统,在得到ASR后，同时调灵云客服系统和第三方NLP,默认0
  /*config_linyun_kf_enable_atomic_ = ::GetPrivateProfileInt("VOICEDETAILCONFIG", "linyun_kf_enable", 0, voide_detail_config_file_path);*/
  // g_Log.Trace_Print(__FILE__, __LINE__, "读取配置文件：config_linyun_kf_enable_atomic_:%d", config_linyun_kf_enable_atomic_);

  /// ;优化，提前加载天气(迅飞),默认0
  config_optimization_load_ahead_weather_ = 0;//::GetPrivateProfileInt("VOICEDETAILCONFIG", "optimization_load_ahead_weather", 0, voide_detail_config_file_path);
  YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "读取配置文件：optimization_load_ahead_weather:" << config_optimization_load_ahead_weather_);


  // 读取默认的城市
  char tempbuf[1024]= {'\0'};
  //::GetPrivateProfileString("TDRobotINFO","defaultCity","",tempbuf, sizeof(tempbuf),config_robot_file_full_path);
  config_optimixation_default_weather_city_ = "上海";
  YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "读取配置文件：defaultCity:" << config_optimixation_default_weather_city_.c_str());

  //;默认nlp缓存记录数（最大），默认10
  config_nlp_result_max_caching_records_ = 10; //::GetPrivateProfileInt("VOICEDETAILCONFIG", "nlp_result_max_caching_records", 10, voide_detail_config_file_path);

  //;如果是打断模式下，则过滤的字符数。默认是2（即1个中文字）
  //config_voice_filter_min_characters_atomic_ = ::GetPrivateProfileInt("VOICEDETAILCONFIG", "voice_filter_min_character", 3, voide_detail_config_file_path);

  ///// 读取迅飞端口配置
  //config_ifly_port_ = ::GetPrivateProfileInt("DEVCONFIG","IFLY_COM",1,config_devctrl_file_full_path);
  //config_ifly_type_ = ::GetPrivateProfileInt("DEVCONFIG","IFLY_TYPE",0,config_devctrl_file_full_path);
  //config_ifly_baud_ = ::GetPrivateProfileInt("DEVCONFIG","IFLY_BAUD",9600,config_devctrl_file_full_path);

  ////  使用普通麦克风还是阵列，0普通麦克风，1阵; 默认0，如果是麦克风阵列，则会在优化时去唤醒阵列
  //config_use_device_mode_ = ::GetPrivateProfileInt("IFLY","use_device_mode",0,voide_detail_config_file_path);

  //;asr后使用哪家的nlp作为asr的二次结果。如果是多家，则使用｜分开, 如果有值，则表示asr后获取二次结果
  //char tempbuf[1024]= {'\0'};
  memset(tempbuf, 0, sizeof(tempbuf));
  //::GetPrivateProfileString("IFLY","nlp_after_startasr_providers","",tempbuf, sizeof(tempbuf),voide_detail_config_file_path);
  YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "IFLY：nlp_after_startasr_providers:" << tempbuf);
  GetUtility()->SplitToInt("", vec_nlp_after_startasr_providers_, '|');
  if (vec_nlp_after_startasr_providers_.size() >0) {
    int find_index = -1;
    // 检测是否有-1，如果有，则表示asr中不带nlp。
    std::vector<int>::iterator it_find = vec_nlp_after_startasr_providers_.begin();
    for (int i=0; it_find != vec_nlp_after_startasr_providers_.end(); ++it_find, ++i) {
      if (*it_find == -1) {
        find_index = i;
        // vec_nlp_after_startasr_providers_.erase(it_find);
        break;
      }
    }
    if (find_index <0) {
      asr_flag0_only_asr_ = true;   ///< 没有找到-1标识，表示只用asr部分，不需要nlp部分
    }
  }

  memset(tempbuf, 0, sizeof(tempbuf));
  //::GetPrivateProfileString("IFLY","nlp_default_providers","",tempbuf, sizeof(tempbuf),voide_detail_config_file_path);
  YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "IFLY：nlp_default_providers:" << tempbuf);
  GetUtility()->SplitToInt("2|0", vec_nlp_default_providers_, '|');

  //////////////////////////////////////////////////////////////////////////
  //memset(tempbuf, 0, sizeof(tempbuf));
  //::GetPrivateProfileString("IFLY","nlp_only_include_service","",tempbuf, sizeof(tempbuf),voide_detail_config_file_path);
  conifg_nlp_only_include_service_ = "";
  //YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "IFLY：nlp_only_include_service:" <<  tempbuf);
  //GetUtility()->SplitString2Vec(conifg_nlp_only_include_service_.c_str(), vec_nlp_only_include_service_, '|');

  //memset(tempbuf, 0, sizeof(tempbuf));
  //::GetPrivateProfileString("IFLY","nlp_exclude_service","",tempbuf, sizeof(tempbuf),voide_detail_config_file_path);
  config_nlp_exclude_service_ = "";
  //YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "IFLY：nlp_exclude_service:" << tempbuf);
  //GetUtility()->SplitString2Vec(config_nlp_exclude_service_.c_str(), vec_nlp_exclude_service_, '|');

  return YSOS_ERROR_SUCCESS;
}
int ScenesIFLY::GetVoiceProviderID(const std::string strType) {
  return VoiceType_IFLY;
}


void ThreadFunScenes0ptimization(ScenesIFLY* pScenesIFLY) {
  assert(pScenesIFLY);
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "in ThreadFunScenes0ptimization");


  pScenesIFLY->Login();

  //;优化，提前加载天气(迅飞),默认0
  if (pScenesIFLY->config_optimization_load_ahead_weather_ >0 && pScenesIFLY->config_optimixation_default_weather_city_.length() >0) {
    /// 加载迅飞的天气
    std::string strtext_need_nlp = pScenesIFLY->config_optimixation_default_weather_city_ + "的天气";
    YSOS_LOG_INFO_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "提前加载nlp:" <<  strtext_need_nlp.c_str());
    std::string nlpresult;
    pScenesIFLY->NLP2Child(strtext_need_nlp.c_str(), true, nlpresult, true);  ///< 永久保存
  }
}


void ScenesIFLY::Scenes0ptimization(int iParam) {
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.asrext"),  "in Module0ptimization");
  switch (iParam) {
  case  1: {  ///< 加载OCX时

    //改成同步，不用异步，用异步会导致释放资源的时候不同步
    ThreadFunScenes0ptimization(this);

  }
  break;
  }
}




/**
 *@brief
 *@param iFunid[Input]
 *             0 上传关键传，参数空，返回结果，返回执行结果说明字符串，如果成功，则返回“成功”，否则返回“失败及错误信息"
 *@return
 */
std::string ScenesIFLY::ExeScenceFun(const int iFunid, std::string strParam) {
  switch (iFunid) {
  case 0: {
    // 得先登录
    VoiceInterfaceIFLY::Login();

    std::string error_msg;
    int iret =UploadUserwords(&error_msg);
    return error_msg;
  }
  break;
  /*case 1: {
  int iret = SetAppID(strParam);
  return YSOS_ERROR_SUCCESS == iret?"更新appid成功":"更新appid失败";
  }
  break;*/
  default: {
    return "不支持些操作";
  }
  }
  return "";
}

int ScenesIFLY::GetNlpProvidersAfterAsr(std::vector<int>& vec_nlpproviders) {
  vec_nlpproviders = vec_nlp_after_startasr_providers_;
  return YSOS_ERROR_SUCCESS;
}
int ScenesIFLY::GetNlpDefaultProviders(std::vector<int>& vec_nlpproviders) {
  vec_nlpproviders = vec_nlp_default_providers_;
  return YSOS_ERROR_SUCCESS;
}



int ScenesIFLY::ConvertNlp3To2(boost::property_tree::ptree& pt) {
  /// 为了兼容,修改字段格式为2.0版的字段格式
  boost::property_tree::ptree tree_empty;
  boost::property_tree::ptree tree_semantic2, slot2;  ///< 2.0版的是一个json，3.0版的是一个list
  std::string city_address_ansi;
  // 得到城市信息
  boost::property_tree::ptree tree_semantic3= pt.get_child("semantic", tree_empty);
  BOOST_FOREACH(boost::property_tree::ptree::value_type &v, tree_semantic3) {
    //boost::property_tree::ptree other_answer_source = v.second;
    boost::property_tree::ptree slots3 = v.second.get_child("slots", tree_empty);
    BOOST_FOREACH(boost::property_tree::ptree::value_type &vslot, slots3) {
      std::string slot_name = vslot.second.get("name", "");
      if ("location.city" == slot_name) {
        slot2.put("location.city", vslot.second.get("value", ""));
      } else if ("location.cityAddr" == slot_name) {
        slot2.put("location.cityAddr", vslot.second.get("value", ""));
        // city_address_ansi = boost::locale::conv::from_utf<char>(vslot.second.get("value", "").c_str, "gbk");
      } else if ("location.type" == slot_name) {
        slot2.put("location.type", vslot.second.get("value", ""));
      }
    }
    break; ///< 只遍历第1个
  }

  // 得到日期信息
  /// 还是知道是今天，明天，后天，大后天
  //int day_index =0;
  //{
  //  std::string answer_text = pt.get("answer.text", "");
  //  std::string middle_string = "";
  //  if(answer_text.length() >0) {
  //   //  "text": "\"南昌后天多云转雷阵雨\"，\"29℃~37℃\"，\"无持续风向微风\"，出门记得带伞，淋雨感冒心情也不好呢",
  //   answer_text = replace_all_distinct(answer_text, city_address_ansi, "");
  //
  //   std::string end_string = "天";
  //   string::size_type  begin_pos =0, end_pose =0;

  //   for(string::size_type   pos(0);   pos!=string::npos;   pos+=std::string("").length())   {
  //     if(   (pos=answer_text.find(city_address_ansi,pos))!=string::npos   )   {
  //       //str.replace(pos,old_value.length(),new_value);
  //       begin_pos = pos;
  //     }
  //     else   {break; }
  //   }

  //   for(string::size_type   pos = begin_pos;   pos!=string::npos;   pos+=std::string("").length())   {
  //     if(   (pos=answer_text.find(end_string,pos))!=string::npos   )   {
  //       //str.replace(pos,old_value.length(),new_value);
  //       end_pose = pos;
  //     }
  //     else   {break; }
  //   }

  //   if(begin_pos <= end_pose) {
  //     middle_string = answer_text.substr(begin_pos, end_pose);
  //   }

  //  }

  //  if(middle_string == "今") {

  //  }else if(middle_string == "明") {

  //  }else if(middle_string == "后") {

  //  }else if(middle_string == "大后") {

  //  }
  //}
  boost::property_tree::ptree tree_result3 = pt.get_child("data.result", tree_empty);
  BOOST_FOREACH(boost::property_tree::ptree::value_type &v, tree_result3) {
    slot2.put("datetime.date", v.second.get("date", ""));
    slot2.put("datetime.type", v.second.get("date", "DT_BASIC"));  ///< 设为固定值
    break;  ///< 只获取第1个
  }
  tree_semantic2.put_child("slots", slot2);
  pt.put_child("semantic", tree_semantic2);

  // 设置answer字段
  //boost::property_tree::ptree tree_answer2;
  pt.put("answer.content.location",slot2.get("location.city", ""));

  return YSOS_ERROR_SUCCESS;
}


int ScenesIFLY::ConvertNlp3To2(Json::Value& pt) {
  return YSOS_ERROR_FAILED;
}


int ScenesIFLY::DecodeIflyLocalResult(const std::string& rawresult, std::string* rawtext) {
  if (rawresult.length() <=0) {
    return YSOS_ERROR_FAILED;
  }

  //decode xml
  std::istringstream in_stream(rawresult /*boost::locale::conv::to_utf<char>(rawresult.c_str(), "gbk")*/);
  boost::property_tree::ptree tree_xml;
  boost::property_tree::read_xml(in_stream, tree_xml);
  if (rawtext) {
    //*rawtext = boost::locale::conv::from_utf<char>(tree_xml.get("nlp.rawtext", "").c_str(), "gbk");
    std::string text1 = tree_xml.get("nlp.text", "");
    if (text1.length() >0) {
      *rawtext = text1;
      return YSOS_ERROR_SUCCESS;
    }
    *rawtext =tree_xml.get("nlp.rawtext", "");
  }
  return YSOS_ERROR_SUCCESS;
}