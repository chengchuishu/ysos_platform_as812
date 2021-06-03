/**   //NOLINT
  *@file facecomparison.h
  *@brief Definition of 人脸比对 与服务器端请求相关的操作.把当前人脸特征发给服务进行比对
  *@version 0.1
  *@author wanfenfen
  *@date Created on: 2016:9:19   16:26
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  */
/// stl headers
#include <string>
#include <vector>
#include <list>

/// boost headers
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/thread/mutex.hpp>


/// private headers
#include "facecomparisoninterface.h"

/// third party headers
#include <json/json.h>

namespace ysos {

 class FaceDetectComparison : public FaceComparisonInterface {
 public:
    
   FaceDetectComparison();
   virtual ~FaceDetectComparison();

   
  /**
    *@brief 读取配置项
    *@param 
    *@return   
    */
   //virtual int ReadConfig(const char* pModuleFilePath);
 
   virtual bool IsSupportMulPersionOneTime1N() {return false;}

   /**
    *@brief 人脸识别（1比N特征比较），对应于后台接口 /tdcctp/0A00035.json。 topn参数有效，可以返回多个相似人脸，同时可以一次发送多全个人脸特征
    *@param list_face_base64[Input] 人脸的base64数据，而不是人脸特征。
    *@return   
    */
  /* virtual int Comparison1NAccordBase64(const char* pTerminalId, const char* pOrgid, std::list<std::string>& list_face_base64,
     const int iTopN, int filter_user_mode,std::string* pstrComparisonReulstOriginal, FaceComparisonResult1N* pComparisonResult, std::string* error_message);*/
   virtual int Comparison1NAccordBase64(const char* pTerminalId, const char* pOrgid, const std::vector<std::string>& vector_face_base64,
     const int iTopN, const int timeout_milliseconds, std::string* pstrComparisonReulstOriginal, std::vector<FaceComparisonResult1NPtr>& vec_comparisonresult, std::string* error_message);


 protected:

   /**
    *@brief 与棠棣服务器连接里使用的请求样式, 会把参数 strRequestBodyJson 封闭到 "REQ_BODY"字段中，同时会增加一个 head字段
    * 组装后的实际发送的数据为：
    * {
    * "REQ_HEAD": { },
    * "REQ_BODY":参数：strRequestBodyJson
    * }
    *@param pRequestUrl[Input] 请求的url
    *@param strRequestBodyJson[Input] 请求的json
    *@param strResponseJsonData[Output] 响应获取到的json数据
    *@return success return YSOS_ERROR_SUCCESS,
    *        fail return one of(YSOS_ERROR_LOGIC_ERROR)
    */
   int HttpPostTDServer(const char* pRequestUrl, const boost::property_tree::ptree& strRequestBodyJson, boost::property_tree::ptree& ResponseJsonData);

   int HttpPostTDServer(const char* pRequestUrl, Json::Value& RequestBodyJson, boost::property_tree::ptree& ResponseJsonData);

 private: 
   //std::string config_url_request_comparison11_;   ///< 人脸识别（1：1特征比较）请求地址
   //std::string config_url_request_comparison1n_;   ///< 人脸识别（1：N特征比较）请求地址
   //std::string config_url_request_faceimage_register_;  ///< 人脸注册的地址
 };

}