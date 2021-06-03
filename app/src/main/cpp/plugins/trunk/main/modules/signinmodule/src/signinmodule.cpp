/**
 *@file signinmodule.cpp
 *@brief sign in module
 *@version 1.0
 *@author l.j..
 *@date Created on: 2016-10-20 14:50:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
/// Private Headers //  NOLINT
#include "../include/signinmodule.h"

#include <fstream>
#include <sstream>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/locale.hpp>
#include <boost/filesystem.hpp>
#include <json/json.h>
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"

#if _WIN32
#include <winsock2.h>
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")
#else

#endif

namespace ysos {
DECLARE_PLUGIN_REGISTER_INTERFACE(SignInModule, ModuleInterface);  ///<  插件的入口，一定要加上 //  NOLINT

SignInModule::SignInModule(const std::string &strClassName)
    : sign_in_time_(5),
      is_sign_in_(false),
      BaseThreadModuleImpl(strClassName) {
  robot_url_ = "";
  finance_url_ = "";
  store_path_ = "";
  org_id_ = "";
  term_id_ = "";
  proxy_ip_ = "";
  proxy_port_ = "";
  proxy_user_ = "";
  proxy_pwd_ = "";
  http_client_ = NULL;
  thread_data_->timeout = 3000;
  logger_ = GetUtility()->GetLogger("ysos.signin");
}

SignInModule::~SignInModule(void) {
}

int SignInModule::AlyAndWrite(std::string robot_data, std::string finance_data, std::string path) {
  int n_return = YSOS_ERROR_SUCCESS;

  do {
    /* TODO:add for linux
    try {
      Json::Reader json_reader;
      Json::Value rsp_robot;
      json_reader.parse(robot_data, rsp_robot, true);
      Json::Value robot_head = rsp_robot["REP_HEAD"];
      std::string str_trancode = robot_head.get("TRAN_CODE", "").asString();
      if ("000000" != str_trancode) {
        n_return = YSOS_ERROR_FAILED;
        break;
      }
      std::string str_tranrspmsg = robot_head.get("TRAN_RSPMSG", "").asString();
      Json::Value robot_body = rsp_robot["REP_BODY"];
      int int_orgstatus = robot_body.get("orgStatus", "").asInt();
      std::string str_timeofarrival = robot_body.get("timeOfArrival", "").asString();
      
      data_ptr_->SetData("signin_phone", str_phone);
      data_ptr_->SetData("signin_insadr", str_insadr);

      std::string str_pinkey = "";
      std::string str_mackey = "";
      if (!finance_data.empty()) {
        Json::Value rsp_finance;
        json_reader.parse(finance_data, rsp_finance, true);
        Json::Value finance_head = rsp_finance["REP_HEAD"];
        str_trancode = finance_head.get("TRAN_CODE", "").asString();
        if ("000000" != str_trancode) {
          n_return = YSOS_ERROR_FAILED;
          break;
        }
        Json::Value finance_body = rsp_finance["REP_BODY"];
        str_pinkey = finance_body.get("pinKey", "").asString();
        str_mackey = finance_body.get("macKey", "").asString();
        YSOS_LOG_DEBUG("str_pinkey = " << str_pinkey);
        YSOS_LOG_DEBUG("str_mackey = " << str_mackey);
      }

      boost::property_tree::ptree cfg_ini;
      
      cfg_ini.put<std::string>("SignIn.TRAN_CODE", str_trancode);
      cfg_ini.put<std::string>("SignIn.TRAN_RSPMSG", str_tranrspmsg);
      cfg_ini.put<int>("SignIn.orgStatus", int_orgstatus);
      cfg_ini.put<std::string>("SignIn.timeOfArrival", str_timeofarrival);
      write_ini(path, cfg_ini);
    } catch (std::exception e) {
      std::string strException = e.what();
      YSOS_LOG_DEBUG("strException = " << strException);
      n_return = YSOS_ERROR_FAILED;
      break;
    }
    */
  } while (0);

  return n_return;
}

int SignInModule::Ioctl(INT32 control_id, LPVOID param /* = nullptr */) {
  YSOS_LOG_DEBUG("module Ioctl execute");
  int n_return = YSOS_ERROR_SUCCESS;

  do {
    switch (control_id) {
      case CMD_SIGN_IN: {
        YSOS_LOG_DEBUG("SIGNIN command");
        n_return = SignIn();
        break;
      }
      default: {
        YSOS_LOG_DEBUG("Ioctl id error, control_id = " << control_id);
        n_return = YSOS_ERROR_FAILED;
        break;
      }
    }
  } while (0);

  YSOS_LOG_DEBUG("module Ioctl done");

  return n_return;
}

int SignInModule::Flush(LPVOID param /* = nullptr */) {
  return YSOS_ERROR_SUCCESS;
}

int SignInModule::Initialize(LPVOID param /* = nullptr */) {
  YSOS_LOG_DEBUG("module Initialize execute");
  int n_return = YSOS_ERROR_SUCCESS;

  do {
    if (IsInitSucceeded()) {
      n_return = YSOS_ERROR_FINISH_ALREADY;
      break;
    }
#ifdef _WIN32
    std::string data_path = GetPackageConfigImpl()->GetConfPath() + "..\\data";
    if (!boost::filesystem::exists(data_path))
      boost::filesystem::create_directory(data_path);
    data_path = GetPackageConfigImpl()->GetConfPath() + "..\\data\\signinmodule";
    if (!boost::filesystem::exists(data_path))
      boost::filesystem::create_directory(data_path);

    store_path_ = GetPackageConfigImpl()->GetConfPath() + "..\\data\\signinmodule";

    if (!boost::filesystem::exists(store_path_)) {
      YSOS_LOG_DEBUG("store_path_ error, store_path_ = " << store_path_);
      n_return = YSOS_ERROR_FAILED;
      break;
    }
    store_path_ = store_path_ + "\\SignIn.ini";
    YSOS_LOG_DEBUG("store_path_ = " << store_path_);
#else
    std::string data_path = GetPackageConfigImpl()->GetConfPath() + "../data";
    if (!boost::filesystem::exists(data_path))
      boost::filesystem::create_directory(data_path);
    data_path = GetPackageConfigImpl()->GetConfPath() + "../data/signinmodule";
    if (!boost::filesystem::exists(data_path))
      boost::filesystem::create_directory(data_path);

    store_path_ = GetPackageConfigImpl()->GetConfPath() + "../data/signinmodule";

    if (!boost::filesystem::exists(store_path_)) {
      YSOS_LOG_DEBUG("store_path_ error, store_path_ = " << store_path_);
      n_return = YSOS_ERROR_FAILED;
      break;
    }
    store_path_ = store_path_ + "/SignIn.ini";
    YSOS_LOG_DEBUG("store_path_ = " << store_path_);
#endif
    
    n_return = BaseThreadModuleImpl::Initialize(param);
    if (n_return != YSOS_ERROR_SUCCESS) {
      YSOS_LOG_DEBUG("Initialize BaseThreadModuleImpl failed");
      break;
    }
  } while (0);

  YSOS_LOG_DEBUG("module Initialize done: "<< n_return);
  return n_return;
}

int SignInModule::UnInitialize(LPVOID param /* = nullptr */) {
  return BaseThreadModuleImpl::UnInitialize(param);
}

int SignInModule::GetProperty(int iTypeId, void *piType) {
  int ret = YSOS_ERROR_SUCCESS;

  switch (iTypeId) {
    case PROP_DATA: {
      if (data_.empty()) {
          //data_ = GetNetworkStatus();
      }

      YSOS_LOG_DEBUG("PROP_DATA: " << data_);
      if (data_.empty()) {
        ret = YSOS_ERROR_NOT_EXISTED;
        break;
      }
      BufferInterfacePtr *buffer_ptr_ptr = reinterpret_cast<BufferInterfacePtr*>(piType);
      if (NULL == buffer_ptr_ptr) {
        ret = YSOS_ERROR_INVALID_ARGUMENTS;
        break;
      }
      BufferInterfacePtr buffer_ptr = *buffer_ptr_ptr;
      if (NULL == buffer_ptr) {
        ret = YSOS_ERROR_INVALID_ARGUMENTS;
        break;
      }
      uint8_t *data = GetBufferUtility()->GetBufferData(buffer_ptr);
      size_t buffer_length = GetBufferUtility()->GetBufferLength(buffer_ptr);
      if (NULL == data || 0 == buffer_length || buffer_length <= data_.length()) {
        ret = YSOS_ERROR_INVALID_ARGUMENTS;
        break;
      }

      memcpy(data, data_.c_str(), data_.length());
      data[data_.length()] = '\0';
      data_.clear();
      break;
    }
    default:
      ret = BaseThreadModuleImpl::GetProperty(iTypeId, piType);
  }

  return ret;
}

int SignInModule::InitalDataInfo(void) {
  YSOS_LOG_DEBUG("module InitalDataInfo execute");
  int n_return = YSOS_ERROR_SUCCESS;

  YSOS_LOG_DEBUG("module InitalDataInfo done");

  return n_return;
}

int SignInModule::Initialized(const std::string &key, const std::string &value) {
  YSOS_LOG_DEBUG("module Initialized execute");
  int n_return = YSOS_ERROR_SUCCESS;

  do {
    if ("robot_url" == key) {
      robot_url_ = value;
    } else if ("financial_url" == key) {
      finance_url_ = value;
    } else if ("orgid" == key) {
      org_id_ = value;
    } else if ("termid" == key) {
      term_id_ = value;
    } else if ("proxy_ip" == key) {
      proxy_ip_ = value;
    } else if ("proxy_port" == key) {
      proxy_port_ = value;
    } else if ("proxy_user" == key) {
      proxy_user_ = value;
    } else if ("proxy_pwd" == key) {
      proxy_pwd_ = value;
    } else if (strcasecmp("sign_in_time", key.c_str()) == 0) {
      sign_in_time_ = atoi(value.c_str());
    }
  } while (0);
  YSOS_LOG_DEBUG("module Initialized done");

  return n_return;
}

int SignInModule::RealOpen(LPVOID param /* = nullptr */) {
  YSOS_LOG_DEBUG("module RealOpen execute");
  int n_return = YSOS_ERROR_SUCCESS;
  do {
    if (NULL == http_client_) {
      http_client_ = new HttpClient();
      if (http_client_ == NULL) {
        YSOS_LOG_DEBUG("get http_client_ failed");
        n_return = YSOS_ERROR_FAILED;
        break;
      }
    }
    n_return = BaseThreadModuleImpl::RealOpen(param);
    if (n_return != YSOS_ERROR_SUCCESS)
      break;
  } while (0);

  YSOS_LOG_DEBUG("module RealOpen done: " << n_return);
  return n_return;
}

int SignInModule::RealClose(void) {
  YSOS_LOG_DEBUG("module RealClose execute");
  int n_return = YSOS_ERROR_SUCCESS;
  do {
    n_return = BaseThreadModuleImpl::RealClose();
    if (n_return != YSOS_ERROR_SUCCESS)
      break;
  } while (0);

  YSOS_LOG_DEBUG("module RealClose done: " << n_return);
  return n_return;
}

int SignInModule::RealPause(void) {
  return YSOS_ERROR_SUCCESS;
}

int SignInModule::RealStop(void) {
  return YSOS_ERROR_SUCCESS;
}

int SignInModule::RealRun(void) {
  return YSOS_ERROR_SUCCESS;
}

int SignInModule::SignIn() {
  int ret = YSOS_ERROR_SUCCESS;
  Json::Value json_body;
  json_body["type"] = "signin_result_yes";
  json_body["signin_orgid"] = org_id_;
  json_body["signin_termid"] = term_id_;

  do {
    if (org_id_.empty() || term_id_.empty()) {
      YSOS_LOG_DEBUG("str_orgid or str_termid is null");
      ret = YSOS_ERROR_FAILED;
      json_body["signin_result"] = 3;
      break;
    }

    data_ptr_ = GetDataInterfaceManager()->GetData(GLOBAL_DATA_KEY);
    if (!data_ptr_) {
      YSOS_LOG_DEBUG("get data_ptr failed.");
      ret = YSOS_ERROR_FAILED;
      json_body["signin_result"] = 3;
      break;
    }

    data_ptr_->DeleteData("signin_phone");
    data_ptr_->DeleteData("signin_insadr");
    data_ptr_->DeleteData("signin_orgid");
    data_ptr_->DeleteData("signin_termid");
    data_ptr_->SetData("orgId", org_id_);
    data_ptr_->SetData("termId", term_id_);
    YSOS_LOG_DEBUG("set orgId = " << org_id_ << " termId = " << term_id_);

    std::string check_value;
    data_ptr_->GetData("orgId", check_value);
    if (check_value != org_id_) {
      data_ptr_->SetData("orgId", org_id_);
      YSOS_LOG_DEBUG("orgId = " << check_value);
    }
    data_ptr_->GetData("termId", check_value);
    if (check_value != term_id_) {
      data_ptr_->SetData("termId", term_id_);
      YSOS_LOG_DEBUG("termId = " << check_value);
    }

    std::vector<std::string> mac_addresses;
    // 先离线授权, 失败后再进行在线授权
    bool time_out = false;
    /*
    int i = 0;
    for (; i < sign_in_time_; ++i) {
      if (SignInOnce(mac_addresses) == YSOS_ERROR_SUCCESS)
        break;
    }
    if (i == sign_in_time_) {
      time_out = true;
      YSOS_LOG_DEBUG("Sign in " << i << " times, all failed.");
    }

    if (is_sign_in_) {
      json_body["signin_result"] = 0;
      data_ptr_->SetData("signin_result", "0");
      data_ptr_->SetData("signin_orgid", org_id_);
      data_ptr_->SetData("signin_termid", term_id_);
      YSOS_LOG_DEBUG("set signin_orgid = " << org_id_ << " signin_termid = " << term_id_);
    } else if (time_out) {
      json_body["signin_result"] = 0;
      is_sign_in_ = true;
      //data_ptr_->SetData("signin_result", "2");
      data_ptr_->SetData("signin_result", "0");
      data_ptr_->SetData("signin_orgid", org_id_);
      data_ptr_->SetData("signin_termid", term_id_);
    } else {
      json_body["signin_result"] = 1;
      data_ptr_->SetData("signin_result", "1");
      YSOS_LOG_INFO("Authorize failed.");
    }
    */
    //add  for test_debug
    if (!time_out) {
      json_body["type"] = "signin_result_yes";
    } else {
      json_body["type"] = "signin_result_no";
    }

  } while (0);

  

  // 签到成功不发事件给前端
  /*if (!is_sign_in_)*/ 
  {
    Json::FastWriter writer;
    data_ = writer.write(json_body);
    data_ = GetUtility()->ReplaceAllDistinct ( data_, "\\r\\n", "" );
  } 
  
  YSOS_LOG_DEBUG("SignInModule::SignIn  [data_] = " << data_ );

  SetProperty(PROP_THREAD_NOTIFY, NULL);
  return YSOS_ERROR_SUCCESS;
}

int SignInModule::SignInOnce(const std::vector<std::string>& mac_addresses) {
  int n_return = YSOS_ERROR_FAILED;
  if ((!proxy_ip_.empty()) && (!proxy_port_.empty())) { ///< 代理
    std::string proxy_address = proxy_ip_ + ":" + proxy_port_;
    std::string proxy_auth = "";
    if ((!proxy_user_.empty()) && (!proxy_pwd_.empty())) {
      proxy_auth = proxy_user_ + ":" + proxy_pwd_;
    }
    YSOS_LOG_DEBUG("proxy_address = " << proxy_address);
    YSOS_LOG_DEBUG("proxy_auth = " << proxy_auth);
    http_client_->SetProxyInfo(proxy_address, proxy_auth);
  }

  std::string cur_time = boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time());
  std::string trans_date = cur_time.substr(0, 8);
  std::string trans_time = cur_time.substr(9);
  YSOS_LOG_DEBUG("trans_date = " << trans_date);
  YSOS_LOG_DEBUG("trans_time = " << trans_time);

  // mac地址
  std::string mac_string;
  std::vector<std::string>::const_iterator itr = mac_addresses.begin();
  for (; itr != mac_addresses.end(); ++itr) {
    if (!mac_string.empty())
      mac_string.append(";");
    mac_string.append(*itr);
  }

  Json::Value req_head;
  req_head["termId"] = "";
  req_head["TRAN_PROCESS"] = "CM0001";
  Json::Value req_body;
  req_body["orgId"] = org_id_;
  req_body["termId"] = term_id_;
  req_body["TransDate"] = trans_date;
  req_body["TransTime"] = trans_time;
  req_body["mac"] = mac_string;
  req_body["version"] = "2003";
  Json::Value req_value;
  req_value["REQ_HEAD"] = req_head;
  req_value["REQ_BODY"] = req_body;
  Json::FastWriter json_writer;
  std::string req_data = json_writer.write(req_value);
  req_data = GetUtility()->ReplaceAllDistinct ( req_data, "\\r\\n", "" );
  req_data = "REQ_MESSAGE=" + req_data;
  YSOS_LOG_DEBUG("robot sign in req_data = " << req_data);
  req_data = boost::locale::conv::to_utf<char>(req_data, "gbk");
  std::string rsp_robot;
  http_client_->SetHttpUrl(robot_url_);
  http_client_->SetHttpContent(req_data.c_str(), req_data.length());
  http_client_->SetTimeOut(10, 10);
  n_return = YSOS_ERROR_SUCCESS; /*http_client_->Request(rsp_robot);*/
  rsp_robot = "{\"REP_HEAD\":{\"PINKEY\":\"\",\"TRAN_RSPMSG\":\"签到成功\",\"MACKEY\":\"\",\"TRAN_CODE\":\"000000\"},\"REP_BODY\":{\"ORG_ID\":\"000000006\",\"termId\":\"00000248\",\"orgNo\":\"000000006\",\"orgName\":\"医疗机构\"}}";
  YSOS_LOG_DEBUG("robot 请求, n_return = " << n_return);
  YSOS_LOG_DEBUG("返回数据, n_return = " << rsp_robot);
  if (YSOS_ERROR_SUCCESS != n_return) {
    YSOS_LOG_DEBUG("robot sign in fail, n_return = " << n_return);
    return n_return;
  }
  YSOS_LOG_DEBUG("1111111, n_return = " << rsp_robot);
  rsp_robot = boost::locale::conv::from_utf<char>(rsp_robot, "gbk");
  YSOS_LOG_DEBUG("rsp_robot = " << rsp_robot);

  if (rsp_robot == "") {
    YSOS_LOG_DEBUG("rsp_robot is null");
    return YSOS_ERROR_FAILED;
  }

  // 解析签到结果
  if (!ParseSigninResponse(rsp_robot))
    return YSOS_ERROR_FAILED;
  // 授权失败直接返回
  if (!is_sign_in_)
    return YSOS_ERROR_SUCCESS;

  req_value.clear();
  req_head.clear();
  req_body.clear();

  ///< TODO: Analzy the response content for author
  n_return = AlyAndWrite(rsp_robot, "", store_path_);
  if (n_return != YSOS_ERROR_SUCCESS) {
    YSOS_LOG_DEBUG("AlyAndWrite not success, return = " << n_return);
    return n_return;
  }

  YSOS_LOG_DEBUG("CMD_SIGN_IN success");
  return YSOS_ERROR_SUCCESS;
}

bool SignInModule::ParseSigninResponse(const std::string& json_string) {
  Json::Reader json_reader;
  Json::Value root;
  try {
    json_reader.parse(json_string, root, 1);
    Json::Value head = root["REP_HEAD"];
    if (head["TRAN_CODE"] == "000000") {
      is_sign_in_ = true;
    } else {
      is_sign_in_ = false;
    }
  } catch (std::exception& ex) {
    YSOS_LOG_ERROR("Parse json exception: " << ex.what());
    return false;
  }

  return true;
}

std::string SignInModule::GetNetworkStatus() {
  boost::shared_ptr<HttpClient> http_client(new HttpClient());
  http_client->SetHttpUrl("http://www.baidu.com");
  http_client->SetHttpHeader("Content-Type", "application/x-www-form-urlencoded");
  http_client->SetTimeOut(2, 2);
  std::string response_data;
  int ret = http_client->Request(response_data, HTTP_POST);
  response_data = boost::locale::conv::from_utf<char>(response_data.c_str(), "gbk");
  YSOS_LOG_INFO("Network check response " << response_data.size() << " bytes.");

  std::string result = "{\"type\":\"network_status\",\"data\":{\"status\":";
  if (response_data.empty())
    result.append("0}}");
  else
    result.append("1}}");
  return result;
}

}