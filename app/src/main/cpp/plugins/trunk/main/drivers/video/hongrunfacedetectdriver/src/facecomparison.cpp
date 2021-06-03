/**   //NOLINT
  *@file facecomparison.h
  *@brief Definition of 人脸比对 与服务器端请求相关的操作.把当前人脸特征发给服务进行比对
  *@version 0.1
  *@author wanfenfen
  *@date Created on: 2016:9:19   16:26
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  */

/// self headers
#include "../include/facecomparison.h"

#ifdef _WIN32
/// windows sdk
#include <Windows.h>
#else
//TODO:add for linux
#endif


/// stl headers
#include <string>
#include <map>

/// boost headers
#include <boost/scoped_ptr.hpp>
#include <boost/locale.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>

/// third party
#include <json/json.h>
#include "../../../public/include/httpclient/httpclient.h"

// ysos headers
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/common.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/driverinterface.h"

/// private headers
#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"

#include "../include/face_module_config.h"
#include "../include/common/mycommondefine.h"

namespace ysos {

    FaceDetectComparison::FaceDetectComparison() {
    }

    FaceDetectComparison::~FaceDetectComparison() {
    }

    int FaceDetectComparison::Comparison1NAccordBase64(const char *pTerminalId, const char *pOrgid, const std::vector<std::string> &vector_face_base64,
            const int iTopN, const int timeout_milliseconds, std::string *pstrComparisonReulstOriginal, std::vector<FaceComparisonResult1NPtr> &vec_comparisonresult, std::string *error_message) {
        YSOS_LOG_INFO_CUSTOM(MY_LOGGER, "in Comparison1N, terminalid:" << (pTerminalId ? pTerminalId :"null")  << ", orgnizeid:" << (pOrgid ? pOrgid : "null")  << ", face base64 count;" << vector_face_base64.size());
        assert(NULL != pTerminalId &&strlen(pTerminalId) >0 && NULL != pOrgid && strlen(pOrgid) >0 &&  vector_face_base64.size() >0);
        if(vector_face_base64.size() <=0) {
            if(error_message) {
                *error_message = "error, face base64 data is null";
            }
            return YSOS_ERROR_FAILED;
        }
        Json::Value json_request;
        json_request["termId"]= pTerminalId ? pTerminalId : "";
        json_request["orgId"] = pOrgid ? pOrgid : "";
        json_request["topN"] = iTopN;
        Json::Value json_list_base64;
        std::vector<std::string>::const_iterator it_face_base64 = vector_face_base64.begin();
        for (vector_face_base64; it_face_base64 != vector_face_base64.end(); ++it_face_base64) {
            json_list_base64.append(*it_face_base64);
        }
        json_request["datas"] = json_list_base64;
        boost::property_tree::ptree response_json_data;
        std::string recognize_url =FaceModuleConfig::GetInstance()->GetRecognizeServer();
        int http_post = HttpPostTDServer(recognize_url.c_str(), json_request, response_json_data);//add fro linux
        if (YSOS_ERROR_SUCCESS != http_post/* || response_json_data.empty()*/) {
            if(error_message) {
                *error_message = std::string("post http server error:") + recognize_url;
            }
            return http_post;
        }
        // 解析响应数据
        try {
            //得到响应是否成功
            std::string rspmsg = response_json_data.get<std::string>("REP_HEAD.TRAN_CODE", "-1");
            std::string rspmessage = boost::locale::conv::from_utf<char>(response_json_data.get<std::string>("REP_HEAD.TRAN_RSPMSG","").c_str(), "gbk");
            int retcode = atoi(rspmsg.c_str());
            if (0 != retcode) {
                if(error_message) {
                    *error_message = std::string("连接服务器返回错误:") + rspmessage;
                }
                YSOS_LOG_ERROR_CUSTOM(MY_LOGGER, "连接服务器返回错误:" << rspmessage.c_str());
                return YSOS_ERROR_FAILED;
            }
            {
                /// 经过一下处理
                response_json_data = response_json_data.get_child("REP_BODY");  ///<  只取body部分的数据
                // 添加一个字段 rc
                response_json_data.put("rc", 0);
            }
            if (pstrComparisonReulstOriginal) { /// 解析原始数据
                std::ostringstream ostring_stream;
                boost::property_tree::write_json(ostring_stream, response_json_data);
                *pstrComparisonReulstOriginal = boost::locale::conv::from_utf<char>(ostring_stream.str(), "gbk");
            }
            FaceComparisonResult1NPtr pComparisonResult(new FaceComparisonResult1N);
            if (pComparisonResult) {
                //解析结构
                boost::property_tree::ptree empty_tree,body_tree = response_json_data; // response_json_data.get_child("REP_BODY");
                pComparisonResult->group_id = ""; //  20170628 已取消此字段。boost::locale::conv::from_utf<char>(body_tree.get<std::string>("groupId").c_str(), "gbk");
                boost::property_tree::ptree faces_tree = body_tree.get_child("faces", empty_tree);
                BOOST_FOREACH(boost::property_tree::ptree::value_type &v, faces_tree) {
                    boost::property_tree::ptree p = v.second;
                    std::string utf8_score = p.get<std::string>("score", "");
                    std::string utf8_persionid = p.get<std::string>("personId", "");
                    std::string utf8_sex = p.get<std::string>("sex", "");
                    //std::string utf8_flag = p.get<std::string>("flag", "");
                    std::string utf8_age = p.get<std::string >("age", "");
                    //std::string utf8_smile = p.get<std::string>("smile", "");
                    std::string utf8_idcard = p.get<std::string>("idCard", "");
                    std::string ansi_personname =boost::locale::conv::from_utf<char>( p.get<std::string>("personName", "").c_str(), "gbk");
                    std::string utf8_mobile = p.get<std::string>("mobile", "");
                    std::string ansi_filed1 =boost::locale::conv::from_utf<char>( p.get<std::string>("field1", "").c_str(), "gbk");
                    //// 修正一下性别F 转为-1， M转为 1
                    // std::string source_sex = response_json_data.get("sex", "");
#ifdef _WIN32                    
                    if(stricmp(utf8_sex.c_str(), "F") ==0) {
                        utf8_sex = "-1";
                    }
                    else if(stricmp(utf8_sex.c_str(), "M") ==0) {
                        utf8_sex = "1";
                    }
                    else {
                        //assert(false);  ///< 后台有可能会传一个null
                        //return YSOS_ERROR_FAILED;
                        utf8_sex = "";
                    }
#else
                    if(strcasecmp(utf8_sex.c_str(), "F") ==0) {
                        utf8_sex = "-1";
                    }
                    else if(strcasecmp(utf8_sex.c_str(), "M") ==0) {
                        utf8_sex = "1";
                    }
                    else {
                        //assert(false);  ///< 后台有可能会传一个null
                        //return YSOS_ERROR_FAILED;
                        utf8_sex = "";
                    }
#endif
                    FaceCompareResultPtr face_result(new FaceCompareResult);
                    face_result->score = boost::locale::conv::from_utf<char>(utf8_score.c_str(), "gbk");
                    face_result->person_id = boost::locale::conv::from_utf<char>(utf8_persionid.c_str(), "gbk");
                    face_result->sex = boost::locale::conv::from_utf<char>(utf8_sex.c_str(), "gbk");
                    face_result->age = boost::locale::conv::from_utf<char>(utf8_age.c_str(), "gbk");
                    face_result->id_card = boost::locale::conv::from_utf<char>(utf8_idcard.c_str(), "gbk");
                    face_result->person_name = ansi_personname;
                    face_result->mobile = boost::locale::conv::from_utf<char>(utf8_mobile.c_str(), "gbk");
                    //           face_result->field1 = ansi_filed1;
                    //           face_result->field2 = boost::locale::conv::from_utf<char>(p.get<std::string>("field2", "").c_str(), "gbk");
                    //           face_result->vip_flag = boost::locale::conv::from_utf<char>(p.get<std::string>("flag", "").c_str(), "gbk");
                    //           face_result->user_image = boost::locale::conv::from_utf<char>(p.get<std::string>("image", "").c_str(), "gbk");
                    pComparisonResult->vec_faces.push_back(face_result);
                }
                vec_comparisonresult.push_back(pComparisonResult);
                YSOS_LOG_INFO_CUSTOM(MY_LOGGER, "解析数据结果成功");
            }
            return YSOS_ERROR_SUCCESS;
        }
        catch (...) {
            std::ostringstream out_stream;
            boost::property_tree::write_json(out_stream, response_json_data);
            std::string ansi_response_json_data = boost::locale::conv::from_utf<char>(out_stream.str().c_str(), "gbk");
            if(error_message) {
                *error_message = "解析数据结果异常" ;
            }
            YSOS_LOG_ERROR_CUSTOM(MY_LOGGER, "解析数据结果异常：" <<  ansi_response_json_data.c_str());
            return YSOS_ERROR_FAILED;
        }
        return YSOS_ERROR_FAILED;
    }



    int FaceDetectComparison::HttpPostTDServer(const char *pRequestUrl, const boost::property_tree::ptree &RequestBodyJson, boost::property_tree::ptree &ResponseJsonData) {
        assert(pRequestUrl);
        if (NULL == pRequestUrl) {
            YSOS_LOG_ERROR_CUSTOM(MY_LOGGER, "HttpPostTDServer 请求的url地址没有设置");
            return YSOS_ERROR_FAILED;
        }
        try {
            boost::property_tree::ptree request_params_tree;
            boost::property_tree::ptree head_tree;
            head_tree.put("caller", "test");
            //boost::property_tree::ptree body_tree;
            request_params_tree.add_child("REQ_HEAD", head_tree);
            request_params_tree.add_child("REQ_BODY", RequestBodyJson);
            std::ostringstream str_stream;
            boost::property_tree::json_parser::write_json(str_stream, request_params_tree);
            // std::string json_request_string = boost::locale::conv::from_utf<char>(str_stream.str(), "gbk");
            std::string utf8_request_data = boost::locale::conv::to_utf<char>("REQ_MESSAGE=", "gbk") +  str_stream.str();
            {
                /// 由于URL传值的时候 符号 +  会被过滤成 空格，在 boost里作json转换的时候不会对这个字符处理，所以这里处理一下，把 + 转为 %2B ，服务端接收到后会转为+
                utf8_request_data = GetUtility()->ReplaceAllDistinct(utf8_request_data, "+", "%2B");
            }
            //     std::string error_message;
            //     std::string utf8_result;
            //     ysos::HTTPRequest tmp_http_request;
            //     int http_post_ret = tmp_http_request.HTTPPostSyn(pRequestUrl, json_request_string, "utf8", 30000, NULL,"utf8",utf8_result,&error_message);
            //     if(YSOS_ERROR_SUCCESS != http_post_ret) {
            //       return YSOS_ERROR_NETWORK_FAIL;
            //     }
            boost::scoped_ptr<HttpClientInterface> http_client(new HttpClient());
            if(NULL ==http_client) {
                return YSOS_ERROR_FAILED;
            }
            ///* add for debug to ignore
            http_client->SetHttpUrl(pRequestUrl);
            http_client->SetHttpHeader("Content-Type", "application/x-www-form-urlencoded");
            http_client->SetHttpContent(utf8_request_data.c_str(), utf8_request_data.length());  ///< 这里是传utf8数据还是ansi呢
            http_client->SetTimeOut(1, 1);
            //*/
            std::string response_data; //="{\"REP_HEAD\":{\"TRAN_RSPMSG\":\"\",\"TRAN_CODE\":\"000001\"},\"REP_BODY\":{}}";//add for debug linux
            int http_post_ret = http_client->Request(response_data, HTTP_POST);
            if(YSOS_ERROR_SUCCESS != http_post_ret) {
                std::string ansi_request_data = boost::locale::conv::from_utf<char>(utf8_request_data.c_str(), "gbk");  ///< for log
                YSOS_LOG_ERROR_CUSTOM(MY_LOGGER, "conn server error, url:" << pRequestUrl << "   rquest:" << ansi_request_data.c_str() << "   response:" << response_data.c_str());
                return YSOS_ERROR_FAILED;
            }
            std::string utf8_result = boost::locale::conv::from_utf<char>(response_data.c_str(), "gbk");
            std::istringstream istream_response_data(utf8_result);
            boost::property_tree::read_json(istream_response_data, ResponseJsonData);
            {
                /// 以下代码只是为了输出日志， 如果成功则只打返回数据日志，如果失败，则打所有的日志
                std::string ansi_response_data = boost::locale::conv::from_utf<char>(utf8_result.c_str(), "gbk");  ///< for log
                int ret_code = ResponseJsonData.get<int>("REP_HEAD.TRAN_CODE", -1);
                if(0 != ret_code) {
                    YSOS_LOG_ERROR_CUSTOM(MY_LOGGER, "发送TDserver出错，url:" << pRequestUrl << "   request:" <<  ansi_response_data.c_str() << "    response" <<response_data.c_str());
                }
                else {
                    // 如果成功返回，也打一下日志，
                    YSOS_LOG_INFO_CUSTOM(MY_LOGGER, "获取响应数据:" <<  response_data.c_str());
                }
            }
            return http_post_ret;
        }
        catch (...) {
            //assert(false);
            YSOS_LOG_ERROR_CUSTOM(MY_LOGGER, "in HttpPostTDServer json oper error, URL:" << pRequestUrl);
            return YSOS_ERROR_FAILED;
        }
        return YSOS_ERROR_FAILED;
    }

    int FaceDetectComparison::HttpPostTDServer(const char *pRequestUrl, Json::Value &RequestBodyJson, boost::property_tree::ptree &ResponseJsonData) {
        assert(pRequestUrl);
        if (NULL == pRequestUrl) {
            YSOS_LOG_ERROR_CUSTOM(MY_LOGGER, "HttpPostTDServer 请求的url地址没有设置");
            return YSOS_ERROR_FAILED;
        }
        try {
            Json::Value  json_root, json_header;
            json_header["caller"] = "module camera";
            json_root["REQ_HEAD"] = json_header;
            json_root["REQ_BODY"] = RequestBodyJson;
            std::string request_data = "REQ_MESSAGE=";
            Json::FastWriter writer;
            request_data += writer.write(json_root);
            request_data = GetUtility()->ReplaceAllDistinct ( request_data, "\\r\\n", "" );
            {
                // for test log
                YSOS_LOG_DEBUG_CUSTOM(MY_LOGGER, "url:" <<  pRequestUrl);
                YSOS_LOG_DEBUG_CUSTOM(MY_LOGGER, "request data:" <<  request_data.c_str());
            }
            std::string utf8_request_data = boost::locale::conv::to_utf<char>(request_data.c_str(), "gbk");
            {
                /// 由于URL传值的时候 符号 +  会被过滤成 空格，在 boost里作json转换的时候不会对这个字符处理，所以这里处理一下，把 + 转为 %2B ，服务端接收到后会转为+
                /*request_data*/utf8_request_data = GetUtility()->ReplaceAllDistinct(utf8_request_data, "+", "%2B");
            }
            //     std::string error_message;
            //     std::string utf8_result;
            //     ysos::HTTPRequest tmp_http_request;
            //     int http_post_ret = tmp_http_request.HTTPPostSyn(pRequestUrl, utf8_request_data, "utf8", 30000, NULL,"utf8",utf8_result,&error_message);
            //     if(YSOS_ERROR_SUCCESS != http_post_ret) {
            //       return YSOS_ERROR_NETWORK_FAIL;
            //     }
            std::string recognize_url = FaceModuleConfig::GetInstance()->GetRecognizeServer();
            if (recognize_url.empty()) {
                YSOS_LOG_ERROR_CUSTOM(MY_LOGGER, "Recognize server url is empty.");
                return YSOS_ERROR_FAILED;
            }
            boost::scoped_ptr<HttpClientInterface> http_client(new HttpClient());
            if(NULL ==http_client) {
                return YSOS_ERROR_FAILED;
            }
            //*
            http_client->SetHttpUrl(pRequestUrl);
            http_client->SetHttpHeader("Content-Type", "application/x-www-form-urlencoded");
            //http_client->SetHttpHeader("Content-Type", "application/json");
            http_client->SetHttpContent(utf8_request_data.c_str(), utf8_request_data.length());  ///< 这里是传utf8数据还是ansi呢
            //http_client->SetTimeOut(1, 1);
            //*/
            std::string response_data;// = "{\"REP_HEAD\":{\"TRAN_RSPMSG\":\"\",\"TRAN_CODE\":\"000001\"},\"REP_BODY\":{}}";//add for debug linux
            int http_post_ret = http_client->Request(response_data, HTTP_POST);
            if(YSOS_ERROR_SUCCESS != http_post_ret) {
                YSOS_LOG_ERROR_CUSTOM(MY_LOGGER, "conn server error, url:" << recognize_url.c_str() << "   rquest:" << request_data.c_str() << "   response:" << response_data.c_str());
                return YSOS_ERROR_FAILED;
            }
            std::string &utf8_result = response_data; //boost::locale::conv::from_utf<char>(response_data.c_str(), "gbk");
            std::istringstream istream_response_data(utf8_result);
            boost::property_tree::read_json(istream_response_data, ResponseJsonData);
            {
                /// 以下代码只是为了输出日志， 如果成功则只打返回数据日志，如果失败，则打所有的日志
                std::string ansi_response_data = boost::locale::conv::from_utf<char>(utf8_result.c_str(), "gbk");  ///< for log
                int ret_code = ResponseJsonData.get<int>("REP_HEAD.TRAN_CODE", -1);
                if(0 != ret_code) {
                    YSOS_LOG_ERROR_CUSTOM(MY_LOGGER, "发送TDserver出错，url:" << pRequestUrl << "   request:" <<  ansi_response_data.c_str() << "    response" <<response_data.c_str());
                }
                else {
                    // 如果成功返回，也打一下日志，
                    YSOS_LOG_INFO_CUSTOM(MY_LOGGER, "获取响应数据:" << ansi_response_data.c_str());
                }
            }
            return http_post_ret;
        }
        catch(...) {
            //assert(false && "in HttpPostTDServer json oper error");
            YSOS_LOG_ERROR_CUSTOM(MY_LOGGER, "in HttpPostTDServer json oper error, URL:" << pRequestUrl);
            return YSOS_ERROR_FAILED;
        }
    }

}