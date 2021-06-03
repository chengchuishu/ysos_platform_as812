/**
 *@file facedetectstruct.h
 *@brief Definition of 人脸检测相关结构体
 *@version 2.0
 *@author libaoguo
 *@date Created on: 2017-08-25
 *@copyright Copyright(c) 2017 YunShen Technology. All rights reserved.
**/

#ifndef TD_FACEDETECTSTRUCT_H     //NOLINT
#define TD_FACEDETECTSTRUCT_H     //NOLINT

/// stl headers
#include <string.h>
#include <vector>

/// boost headers
#include <boost/shared_array.hpp>
#include <boost/property_tree/ptree.hpp>

/// third party headers
#include <json/json.h>
#include <json/reader.h>
#include <json/writer.h>

/// private headers
#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"

namespace ysos{

/**
 *@brief 解析json数据到TDFaceDetectInfoArray结构中
 *@param 
 *@return   
 */
int DecodeJson(const char* pJsonString, TDFaceDetectInfoArray& ArrResult, int& nArrNumber);

/**
 *@brief 把TDFaceDetectInfoArray 组装到json字符串中
 *@param 
 *@return   
 */
int EncodeJson(const TDFaceDetectInfoArray* pArrInfo, const int nArrNumber, std::string& strResult);
int EncodeJson(const TDFaceDetectInfoArray* pArrInfo, const int nArrNumber, Json::Value& json_object);
int EncodeJson(const FaceDetectInfo& face_info, Json::Value& json_object);

/**
 *@brief 解析json数据到TDFaceDetectInfoArray结构中
 *@param 
 *@return   
 */
int DecodeJson(const char* pJsonString, FaceComparisonResult1N& ArrResult);

/**
 *@brief 把不同数据结构组装到json字符串中
 *@param 
 *@return   
 */
int EncodeJson(const FaceComparisonResult1N* pArrInfo, std::string& strResult);
int EncodeJson(const FaceComparisonResult1N* pComarisonResult, Json::Value& json_object);
int EncodeJson(const TDFaceDetectUnit* puint, std::string& json_string);
int EncodeJson(const TDFaceDetectUnit* puint, Json::Value& json_object);
int EncodeJson(const std::vector<TDFaceDetectInfoPtr>& ArrInfo, Json::Value* out_json, std::string* out_json_string);
int DecodeJson(boost::property_tree::ptree& p, std::vector<TDFaceDetectUnit>& vec_faceunit);

/**
 *@brief 组装成YSOS平台的数据格式
 *@param 
 *@return   
 */
int EncodeJson2(const std::vector<TDFaceDetectUnit>&arr_face_unit, std::string* strResult, Json::Value* josn_value);
int EncodeJson2(const std::vector<TDFaceDetectUnit>&arr_face_unit, const bool encode_imagebase64, std::string* strResult, Json::Value* josn_value);
int DecodeJson2(boost::property_tree::ptree& p, std::vector<TDFaceDetectUnit>& vec_faceunit);


}

#endif ///< TD_FACEDETECTSTRUCT_H
