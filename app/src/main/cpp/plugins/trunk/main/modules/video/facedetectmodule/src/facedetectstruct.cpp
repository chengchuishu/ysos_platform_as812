/**
 *@file facedetectstruct.cpp
 *@brief Definition of 人脸检测相关结构体
 *@version 2.0
 *@author libaoguo
 *@date Created on: 2017-08-25
 *@copyright Copyright(c) 2017 YunShen Technology. All rights reserved.
**/

#include "../../../public/include/common/facedetectstruct.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/locale.hpp>
 
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"


namespace ysos{

#define FACE_DETECT_SKIP_DIS    1.5    // by estimate


  int DecodeSingleJson(boost::property_tree::ptree&p, TDFaceDetectInfo& detectinfo) {
    detectinfo.trackid = p.get<int>("trackid",0);
    detectinfo.x = p.get<int>("x", 0);
    detectinfo.y = p.get<int>("y", 0);
    detectinfo.width = p.get<int>("width", 0);
    detectinfo.hight = p.get<int>("hight", 0);
    detectinfo.face_quality_score = p.get<float>("facescore", 0.0);
    detectinfo.img_base64_data =boost::locale::conv::from_utf<char>( p.get<std::string>("imgbase64data", "").c_str(), "gbk");
    detectinfo.gender =boost::locale::conv::from_utf<char>( p.get<std::string>("gender", "-1").c_str(), "gbk");
    detectinfo.age = p.get<int>("age", 0);
    detectinfo.face_feature =boost::locale::conv::from_utf<char>( p.get<std::string>("face_feature", "").c_str(), "gbk");
    detectinfo.horizontal_angle = p.get<float>("horizontal_angle", 0);
    detectinfo.vertical_angle = p.get<float>("vertical_angle", 0);
    detectinfo.horizontal_ratio = p.get<float>("horizontal_ratio", 0);
    detectinfo.vertical_ratio = p.get<float>("vertical_ratio", 0);
    detectinfo.estimate_distance = p.get<float>("estimate_distance", 0);
    detectinfo.serial_num = p.get<int>("serial_num", 0);
    return YSOS_ERROR_SUCCESS;
  }
  int DecodeSingleJson(boost::property_tree::ptree& p, FaceCompareResult& faceinfo) {
    std::string utf8_score = p.get<std::string>("score", "");
    std::string utf8_persionid = p.get<std::string>("personId", "");
    std::string utf8_sex = p.get<std::string>("sex", "");
    //std::string utf8_flag = p.get<std::string>("flag");
    std::string utf8_age = p.get<std::string >("age", "");
    //std::string utf8_smile = p.get<std::string>("smile");
    std::string utf8_idcard = p.get<std::string>("id_card", "");
    std::string utf8_personname = p.get<std::string>("person_name", "");
    std::string utf8_mobile = p.get<std::string>("mobile", "");

    //FaceComparisonResultPtr face_result(new FaceCompareResult);
    faceinfo.score = boost::locale::conv::from_utf<char>(utf8_score.c_str(), "gbk");
    faceinfo.person_id = boost::locale::conv::from_utf<char>(utf8_persionid.c_str(), "gbk");
    faceinfo.sex = boost::locale::conv::from_utf<char>(utf8_sex.c_str(), "gbk");
    faceinfo.age = boost::locale::conv::from_utf<char>(utf8_age.c_str(), "gbk");
    faceinfo.id_card = boost::locale::conv::from_utf<char>(utf8_idcard.c_str(), "gbk");
    faceinfo.person_name = boost::locale::conv::from_utf<char>(utf8_personname.c_str(), "gbk");
    faceinfo.mobile = boost::locale::conv::from_utf<char>(utf8_mobile.c_str(), "gbk");

    //faceinfo.field1 = boost::locale::conv::from_utf<char>(p.get<std::string>("field1", "").c_str(), "gbk");
    //faceinfo.field2 = boost::locale::conv::from_utf<char>(p.get<std::string>("field2", "").c_str(), "gbk");
    //faceinfo.vip_flag = p.get<std::string>("vip_flag", "");
    //faceinfo.user_image = p.get<std::string>("user_image", "");
    //boost::property_tree::ptree empty_tree;
    //faceinfo.third_full_info = p.get_child("third_full_info", empty_tree);
    return YSOS_ERROR_SUCCESS;
  }


int DecodeJson(const char* pJsonString, TDFaceDetectInfoArray& ArrResult, int& nArrNumber) {
  //assert(pJsonString);
  if (NULL == pJsonString)
    return YSOS_ERROR_FAILED;

  try {
    std::string utf8string = boost::locale::conv::to_utf<char>(pJsonString, "gbk");
    std::istringstream istring(utf8string.c_str());
    boost::property_tree:: ptree ptroot;
    boost::property_tree::read_json(istring, ptroot);

    std::string string_facenumber = ptroot.get<std::string>("facenum");
    nArrNumber = atoi(string_facenumber.c_str());

    boost::property_tree::ptree faces_tree = ptroot.get_child("faces");
    ArrResult = TDFaceDetectInfoArray(new TDFaceDetectInfo[nArrNumber]);
    int i=-1;
    BOOST_FOREACH(boost::property_tree::ptree::value_type &v, faces_tree) {
      ++i;
      boost::property_tree::ptree p = v.second;
      TDFaceDetectInfo* face_result = &ArrResult[i];
      face_result->trackid = p.get<int>("trackid",0);
      face_result->x = p.get<int>("x", 0);
      face_result->y = p.get<int>("y", 0);
      face_result->width = p.get<int>("width", 0);
      face_result->hight = p.get<int>("hight", 0);
      face_result->face_quality_score = p.get<float>("facescore", 0.0);
      face_result->img_base64_data =boost::locale::conv::from_utf<char>( p.get<std::string>("imgbase64data", "").c_str(), "gbk");
      face_result->gender =boost::locale::conv::from_utf<char>( p.get<std::string>("gender", "-1").c_str(), "gbk");
      face_result->age = p.get<int>("age", 0);
      face_result->face_feature =boost::locale::conv::from_utf<char>( p.get<std::string>("face_feature", "").c_str(), "gbk");
      face_result->horizontal_angle = p.get<float>("horizontal_angle", 0);
      face_result->vertical_angle = p.get<float>("vertical_angle", 0);
      face_result->horizontal_ratio = p.get<float>("horizontal_ratio", 0);
      face_result->vertical_ratio = p.get<float>("vertical_ratio", 0);
      face_result->estimate_distance = p.get<float>("estimate_distance", 0);
      face_result->serial_num = p.get<int>("serial_num");
    }

    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"),  "error call ablicity , input/out alibityparam is null""解析数据结果成功");
    return YSOS_ERROR_SUCCESS;
  } catch (...) {
    //assert(false);
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"), "解析数据结果异常,json data:" << pJsonString);
    return YSOS_ERROR_FAILED;
  }

  return YSOS_ERROR_FAILED;
}

int EncodeJson(const TDFaceDetectInfoArray* pArrInfo, const int nArrNumber, std::string& strResult) {
  if(nArrNumber >0) {
  //assert(NULL !=pArrInfo);
    if(NULL == pArrInfo)
      return YSOS_ERROR_FAILED;
  }

  int detect_num= nArrNumber;
  try {
    boost::property_tree::ptree ptree_root;
    boost::property_tree::ptree tree_faces_array;
#ifdef _WIN32
    int face_num = 0;
    char tempbuf[1024] = {'\0'};
    for (int i=0; i<detect_num; ++i) {
      if ((*pArrInfo)[i].estimate_distance <= FACE_DETECT_SKIP_DIS) {
      ++face_num;
      boost::property_tree::ptree ptree_face;
      memset(tempbuf, '\0', sizeof(tempbuf));
      sprintf_s(tempbuf, "%d", (*pArrInfo)[i].trackid);
      ptree_face.put("trackid", tempbuf);

      memset(tempbuf, '\0', sizeof(tempbuf));
      sprintf_s(tempbuf, "%d", (*pArrInfo)[i].x);
      ptree_face.put("x", tempbuf);

      memset(tempbuf, '\0', sizeof(tempbuf));
      sprintf_s(tempbuf, "%d", (*pArrInfo)[i].y);
      ptree_face.put("y", tempbuf);

      memset(tempbuf, '\0', sizeof(tempbuf));
      sprintf_s(tempbuf, "%d", (*pArrInfo)[i].width);
      ptree_face.put("width", tempbuf);

      memset(tempbuf, '\0', sizeof(tempbuf));
      sprintf_s(tempbuf, "%d", (*pArrInfo)[i].hight);
      ptree_face.put("hight", tempbuf);

      memset(tempbuf, '\0', sizeof(tempbuf));
      sprintf_s(tempbuf, "%f", (*pArrInfo)[i].face_quality_score);
      ptree_face.put("facescore", tempbuf);
      ptree_face.put("imgbase64data", (*pArrInfo)[i].img_base64_data);
      ptree_face.put("gender", (*pArrInfo)[i].gender);

      memset(tempbuf, '\0', sizeof(tempbuf));
      sprintf_s(tempbuf, "%d", (*pArrInfo)[i].age);
      ptree_face.put("age", tempbuf);
      ptree_face.put("face_feature", (*pArrInfo)[i].face_feature);

      memset(tempbuf, '\0', sizeof(tempbuf));
      sprintf_s(tempbuf, "%f", (*pArrInfo)[i].horizontal_angle);
      ptree_face.put("horizontal_angle", tempbuf);

      memset(tempbuf, '\0', sizeof(tempbuf));
      sprintf_s(tempbuf, "%f", (*pArrInfo)[i].vertical_angle);
      ptree_face.put("vertical_angle", tempbuf);

      // 人脸所占摄像头视角的比例
      memset(tempbuf, '\0', sizeof(tempbuf));
      sprintf_s(tempbuf, "%f", (*pArrInfo)[i].horizontal_ratio);
      ptree_face.put("horizontal_ratio", tempbuf);

      memset(tempbuf, '\0',sizeof(tempbuf));
      sprintf_s(tempbuf, "%f", (*pArrInfo)[i].vertical_ratio);
      ptree_face.put("vertical_ratio", tempbuf);

      // 估计的人脸的距离
      memset(tempbuf, '\0',sizeof(tempbuf));
      sprintf_s(tempbuf, "%f", (*pArrInfo)[i].estimate_distance);
      ptree_face.put("estimate_distance", tempbuf);

      memset(tempbuf, '\0',sizeof(tempbuf));
      sprintf_s(tempbuf, "%d", (*pArrInfo)[i].serial_num);
      ptree_face.put("serial_num", tempbuf);

      tree_faces_array.push_back(std::make_pair("", ptree_face));
      }else {
        YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"), "skip the fase, dis: "<< (*pArrInfo)[i].estimate_distance);
      }
    }

    sprintf_s(tempbuf, sizeof(tempbuf), "%d", face_num);
    ptree_root.put("facenum", tempbuf);
#else
    int face_num = 0;
    char tempbuf[1024] = {'\0'};
    for (int i=0; i<detect_num; ++i) {
      if ((*pArrInfo)[i].estimate_distance <= FACE_DETECT_SKIP_DIS) {
      ++face_num;
      boost::property_tree::ptree ptree_face;
      memset(tempbuf, '\0', sizeof(tempbuf));
      snprintf(tempbuf, sizeof(tempbuf), "%d", (*pArrInfo)[i].trackid);
      ptree_face.put("trackid", tempbuf);

      memset(tempbuf, '\0', sizeof(tempbuf));
      snprintf(tempbuf, sizeof(tempbuf), "%d", (*pArrInfo)[i].x);
      ptree_face.put("x", tempbuf);

      memset(tempbuf, '\0', sizeof(tempbuf));
      snprintf(tempbuf, sizeof(tempbuf), "%d", (*pArrInfo)[i].y);
      ptree_face.put("y", tempbuf);

      memset(tempbuf, '\0', sizeof(tempbuf));
      snprintf(tempbuf, sizeof(tempbuf), "%d", (*pArrInfo)[i].width);
      ptree_face.put("width", tempbuf);

      memset(tempbuf, '\0', sizeof(tempbuf));
      snprintf(tempbuf, sizeof(tempbuf), "%d", (*pArrInfo)[i].hight);
      ptree_face.put("hight", tempbuf);

      memset(tempbuf, '\0', sizeof(tempbuf));
      snprintf(tempbuf, sizeof(tempbuf), "%f", (*pArrInfo)[i].face_quality_score);
      ptree_face.put("facescore", tempbuf);
      ptree_face.put("imgbase64data", (*pArrInfo)[i].img_base64_data);
      ptree_face.put("gender", (*pArrInfo)[i].gender);

      memset(tempbuf, '\0', sizeof(tempbuf));
      snprintf(tempbuf, sizeof(tempbuf), "%d", (*pArrInfo)[i].age);
      ptree_face.put("age", tempbuf);
      ptree_face.put("face_feature", (*pArrInfo)[i].face_feature);

      memset(tempbuf, '\0', sizeof(tempbuf));
      snprintf(tempbuf, sizeof(tempbuf), "%f", (*pArrInfo)[i].horizontal_angle);
      ptree_face.put("horizontal_angle", tempbuf);

      memset(tempbuf, '\0', sizeof(tempbuf));
      snprintf(tempbuf, sizeof(tempbuf), "%f", (*pArrInfo)[i].vertical_angle);
      ptree_face.put("vertical_angle", tempbuf);

      // 人脸所占摄像头视角的比例
      memset(tempbuf, '\0', sizeof(tempbuf));
      snprintf(tempbuf, sizeof(tempbuf), "%f", (*pArrInfo)[i].horizontal_ratio);
      ptree_face.put("horizontal_ratio", tempbuf);

      memset(tempbuf, '\0',sizeof(tempbuf));
      snprintf(tempbuf, sizeof(tempbuf), "%f", (*pArrInfo)[i].vertical_ratio);
      ptree_face.put("vertical_ratio", tempbuf);

      // 估计的人脸的距离
      memset(tempbuf, '\0',sizeof(tempbuf));
      snprintf(tempbuf, sizeof(tempbuf), "%f", (*pArrInfo)[i].estimate_distance);
      ptree_face.put("estimate_distance", tempbuf);

      memset(tempbuf, '\0',sizeof(tempbuf));
      snprintf(tempbuf, sizeof(tempbuf), "%d", (*pArrInfo)[i].serial_num);
      ptree_face.put("serial_num", tempbuf);

      tree_faces_array.push_back(std::make_pair("", ptree_face));
      }else {
        YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"), "skip the fase, dis: "<< (*pArrInfo)[i].estimate_distance);
      }
    }

    snprintf(tempbuf, sizeof(tempbuf), "%d", face_num);
    ptree_root.put("facenum", tempbuf);
#endif

    ptree_root.add_child("faces", tree_faces_array);
    std::ostringstream ostring_stream;
    boost::property_tree::write_json(ostring_stream, ptree_root);
    strResult = boost::locale::conv::from_utf<char>(ostring_stream.str(), "gbk");
    return YSOS_ERROR_SUCCESS;
  } catch (...) {
    //assert(false);
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"),  "error in TDGetCameraCurrentFace, json error");
    return YSOS_ERROR_FAILED;
  }

  return YSOS_ERROR_FAILED;
}

int EncodeJson(const TDFaceDetectInfoArray* pArrInfo, const int nArrNumber, Json::Value& json_object) {
  if(nArrNumber >0) {
    //assert(NULL !=pArrInfo);
    if(NULL == pArrInfo)
      return YSOS_ERROR_FAILED;
  }

  int detect_num = nArrNumber;
  try {
    Json::Value& ptree_root = json_object;
    char tempbuf[1024] = {'\0'};
#ifdef _WIN32
    sprintf_s(tempbuf, sizeof(tempbuf), "%d", detect_num);
#else
    snprintf(tempbuf, sizeof(tempbuf), "%d", detect_num); //add for linux
#endif
    ptree_root["facenum"] =tempbuf;

    Json::Value tree_faces_array;
    for (int i=0; i<detect_num; ++i) {
      Json::Value ptree_face;
      EncodeJson((*pArrInfo)[i], ptree_face);
      tree_faces_array.append(ptree_face);
    }

    ptree_root["faces"] = tree_faces_array;
    return YSOS_ERROR_SUCCESS;
  } catch (...) {
    //assert(false);
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"),  "error in TDGetCameraCurrentFace, json error");
    return YSOS_ERROR_FAILED;
  }

  return YSOS_ERROR_FAILED;
}

int EncodeJson(const FaceDetectInfo& face_info, Json::Value& json_object) {
  Json::Value& ptree_face = json_object;
  char tempbuf[1024] = {'\0'};
#ifdef _WIN32
  memset(tempbuf, '\0', sizeof(tempbuf));
  sprintf_s(tempbuf, "%d", face_info.trackid);
  ptree_face["trackid"] =tempbuf;

  memset(tempbuf, '\0', sizeof(tempbuf));
  sprintf_s(tempbuf, "%d", face_info.x);
  ptree_face["x"] =tempbuf;

  memset(tempbuf, '\0', sizeof(tempbuf));
  sprintf_s(tempbuf, "%d", face_info.y);
  ptree_face["y"] =tempbuf;

  memset(tempbuf, '\0', sizeof(tempbuf));
  sprintf_s(tempbuf, "%d", face_info.width);
  ptree_face["width"] =tempbuf;

  memset(tempbuf, '\0', sizeof(tempbuf));
  sprintf_s(tempbuf, "%d", face_info.hight);
  ptree_face["hight"] =tempbuf;

  memset(tempbuf, '\0', sizeof(tempbuf));
  sprintf_s(tempbuf, "%f", face_info.face_quality_score);
  ptree_face["facescore"] = tempbuf;
  ptree_face["imgbase64data"]= face_info.img_base64_data;
  ptree_face["gender"]= face_info.gender;

  memset(tempbuf, '\0', sizeof(tempbuf));
  sprintf_s(tempbuf, "%d",face_info.age);
  ptree_face["age"]= tempbuf;
  ptree_face["face_feature"]= face_info.face_feature;

  memset(tempbuf, '\0', sizeof(tempbuf));
  sprintf_s(tempbuf, "%f", face_info.horizontal_angle);
  ptree_face["horizontal_angle"]= tempbuf;

  memset(tempbuf, '\0', sizeof(tempbuf));
  sprintf_s(tempbuf, "%f", face_info.vertical_angle);
  ptree_face["vertical_angle"]= tempbuf;

  // 人脸所占摄像头视角的比例
  memset(tempbuf, '\0', sizeof(tempbuf));
  sprintf_s(tempbuf, "%f", face_info.horizontal_ratio);
  ptree_face["horizontal_ratio"] = tempbuf;

  memset(tempbuf, '\0',sizeof(tempbuf));
  sprintf_s(tempbuf, "%f", face_info.vertical_ratio);
  ptree_face["vertical_ratio"] = tempbuf;

  // 估计的人脸的距离
  memset(tempbuf, '\0',sizeof(tempbuf));
  sprintf_s(tempbuf, "%f", face_info.estimate_distance);
  ptree_face["estimate_distance"]= tempbuf;

  memset(tempbuf, '\0',sizeof(tempbuf));
  sprintf_s(tempbuf, "%d", face_info.serial_num);
  ptree_face["serial_num"] =tempbuf;
#else
  memset(tempbuf, '\0', sizeof(tempbuf));
  snprintf(tempbuf, sizeof(tempbuf), "%d", face_info.trackid);
  ptree_face["trackid"] =tempbuf;

  memset(tempbuf, '\0', sizeof(tempbuf));
  snprintf(tempbuf, sizeof(tempbuf), "%d", face_info.x);
  ptree_face["x"] =tempbuf;

  memset(tempbuf, '\0', sizeof(tempbuf));
  snprintf(tempbuf, sizeof(tempbuf), "%d", face_info.y);
  ptree_face["y"] =tempbuf;

  memset(tempbuf, '\0', sizeof(tempbuf));
  snprintf(tempbuf, sizeof(tempbuf), "%d", face_info.width);
  ptree_face["width"] =tempbuf;

  memset(tempbuf, '\0', sizeof(tempbuf));
  snprintf(tempbuf, sizeof(tempbuf), "%d", face_info.hight);
  ptree_face["hight"] =tempbuf;

  memset(tempbuf, '\0', sizeof(tempbuf));
  snprintf(tempbuf, sizeof(tempbuf), "%f", face_info.face_quality_score);
  ptree_face["facescore"] = tempbuf;
  ptree_face["imgbase64data"]= face_info.img_base64_data;
  ptree_face["gender"]= face_info.gender;

  memset(tempbuf, '\0', sizeof(tempbuf));
  snprintf(tempbuf, sizeof(tempbuf), "%d",face_info.age);
  ptree_face["age"]= tempbuf;
  ptree_face["face_feature"]= face_info.face_feature;

  memset(tempbuf, '\0', sizeof(tempbuf));
  snprintf(tempbuf, sizeof(tempbuf), "%f", face_info.horizontal_angle);
  ptree_face["horizontal_angle"]= tempbuf;

  memset(tempbuf, '\0', sizeof(tempbuf));
  snprintf(tempbuf, sizeof(tempbuf), "%f", face_info.vertical_angle);
  ptree_face["vertical_angle"]= tempbuf;

  // 人脸所占摄像头视角的比例
  memset(tempbuf, '\0', sizeof(tempbuf));
  snprintf(tempbuf, sizeof(tempbuf), "%f", face_info.horizontal_ratio);
  ptree_face["horizontal_ratio"] = tempbuf;

  memset(tempbuf, '\0',sizeof(tempbuf));
  snprintf(tempbuf, sizeof(tempbuf), "%f", face_info.vertical_ratio);
  ptree_face["vertical_ratio"] = tempbuf;

  // 估计的人脸的距离
  memset(tempbuf, '\0',sizeof(tempbuf));
  snprintf(tempbuf, sizeof(tempbuf), "%f", face_info.estimate_distance);
  ptree_face["estimate_distance"]= tempbuf;

  memset(tempbuf, '\0',sizeof(tempbuf));
  snprintf(tempbuf, sizeof(tempbuf), "%d", face_info.serial_num);
  ptree_face["serial_num"] =tempbuf;
#endif
  return YSOS_ERROR_SUCCESS;
}

int DecodeJson(const char* pJsonString, FaceComparisonResult1N& ArrResult) {
  //assert(pJsonString);
  if (NULL == pJsonString)
    return YSOS_ERROR_FAILED;

  try {
    std::string utf8string = boost::locale::conv::to_utf<char>(pJsonString, "gbk");
    std::istringstream istring(utf8string.c_str());
    boost::property_tree:: ptree ptroot;
    boost::property_tree::read_json(istring, ptroot);

    boost::property_tree::ptree& body_tree = ptroot;
    ArrResult.group_id =  boost::locale::conv::from_utf<char>(body_tree.get<std::string>("groupId").c_str(), "gbk");
    boost::property_tree::ptree faces_tree = body_tree.get_child("faces");
    BOOST_FOREACH(boost::property_tree::ptree::value_type &v, faces_tree) {
      boost::property_tree::ptree p = v.second;
      FaceCompareResultPtr face_result(new FaceCompareResult);
      std::string utf8_score = p.get<std::string>("score");
      std::string utf8_persionid = p.get<std::string>("personId");
      std::string utf8_sex = p.get<std::string>("sex");
      //std::string utf8_flag = p.get<std::string>("flag");
      std::string utf8_age = p.get<std::string >("age");
      //std::string utf8_smile = p.get<std::string>("smile");
      std::string utf8_idcard = p.get<std::string>("idCard");
      std::string utf8_personname = p.get<std::string>("personName");
      std::string utf8_mobile = p.get<std::string>("mobile");
      
      face_result->score = boost::locale::conv::from_utf<char>(utf8_score.c_str(), "gbk");
      face_result->person_id = boost::locale::conv::from_utf<char>(utf8_persionid.c_str(), "gbk");
      face_result->sex = boost::locale::conv::from_utf<char>(utf8_sex.c_str(), "gbk");
      face_result->age = boost::locale::conv::from_utf<char>(utf8_age.c_str(), "gbk");
      face_result->id_card = boost::locale::conv::from_utf<char>(utf8_idcard.c_str(), "gbk");
      face_result->person_name = boost::locale::conv::from_utf<char>(utf8_personname.c_str(), "gbk");
      face_result->mobile = boost::locale::conv::from_utf<char>(utf8_mobile.c_str(), "gbk");
      ArrResult.vec_faces.push_back(face_result);
    }

    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"),  "解析数据结果成功");
    return YSOS_ERROR_SUCCESS;
  } catch (...) {
    //assert(false);
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"), "解析数据结果异常,json data:" << pJsonString);
    return YSOS_ERROR_FAILED;
  }

  return YSOS_ERROR_SUCCESS;
}

int EncodeJson(const FaceComparisonResult1N* pComarisonResult, std::string& strResult) {
  //assert(pComarisonResult );
  if (NULL == pComarisonResult)
    return YSOS_ERROR_FAILED;

  try {
    boost::property_tree::ptree ptree_root;
    char tempbuf[1024] = {'\0'};
#ifdef _WIN32
    sprintf_s(tempbuf, sizeof(tempbuf), "%s", pComarisonResult->group_id.c_str());
#else
    snprintf(tempbuf, sizeof(tempbuf), "%s", pComarisonResult->group_id.c_str()); //add for linux
#endif
    ptree_root.put("groupId", tempbuf);

    boost::property_tree::ptree tree_faces_array;
    int detect_num = pComarisonResult->vec_faces.size();
    const std::vector<FaceCompareResultPtr>& vec_faces = pComarisonResult->vec_faces;
    for (int i=0; i<detect_num; ++i) {
      boost::property_tree::ptree ptree_face;
      ptree_face.put("score", vec_faces[i]->score);
      ptree_face.put("personId", vec_faces[i]->person_id);
      ptree_face.put("sex", vec_faces[i]->sex);
      ptree_face.put("age", vec_faces[i]->age);
      ptree_face.put("id_card", vec_faces[i]->id_card);
      std::string utf8_person_name = boost::locale::conv::to_utf<char>(vec_faces[i]->person_name.c_str(), "gbk");
      ptree_face.put("person_name", utf8_person_name);
      ptree_face.put("mobile", vec_faces[i]->mobile);
      tree_faces_array.push_back(std::make_pair("", ptree_face));
    }

    ptree_root.add_child("faces", tree_faces_array);
    std::ostringstream ostring_stream;
    boost::property_tree::write_json(ostring_stream, ptree_root);
    strResult = boost::locale::conv::from_utf<char>(ostring_stream.str(), "gbk");
    return YSOS_ERROR_SUCCESS;
  } catch (...) {
    //assert(false);
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"), "error in EncodeJson FaceComparisonResult1N, json error");
    return YSOS_ERROR_FAILED;
  }

  return YSOS_ERROR_FAILED;
}

int EncodeJson(const FaceComparisonResult1N* pComarisonResult, Json::Value& json_object) {
  if(NULL == pComarisonResult) {
    Json::Value empty_json;
    json_object = empty_json;
    return YSOS_ERROR_SUCCESS;
  }

  try {
    Json::Value& ptree_root = json_object;
    char tempbuf[1024] = {'\0'};
#ifdef _WIN32
    sprintf_s(tempbuf, sizeof(tempbuf), "%s", pComarisonResult->group_id.c_str());
#else
    snprintf(tempbuf, sizeof(tempbuf), "%s", pComarisonResult->group_id.c_str());
#endif
    ptree_root["groupId"] = tempbuf;

    Json::Value tree_faces_array;
    int detect_num = pComarisonResult->vec_faces.size();
    const std::vector<FaceCompareResultPtr>& vec_faces = pComarisonResult->vec_faces;
    for (int i=0; i<detect_num; ++i) {
      Json::Value ptree_face;
      ptree_face["score"]= vec_faces[i]->score;
      ptree_face["personId"]= vec_faces[i]->person_id;
      ptree_face["sex"]= vec_faces[i]->sex;
      ptree_face["age"]= vec_faces[i]->age;
      ptree_face["id_card"]= vec_faces[i]->id_card;
      ptree_face["person_name"]= vec_faces[i]->person_name;
      ptree_face["mobile"]=vec_faces[i]->mobile;
      tree_faces_array.append(ptree_face);
    }

    ptree_root["faces"] = tree_faces_array;
    return YSOS_ERROR_SUCCESS;
  } catch (...) {
    //assert(false);
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"), "error in EncodeJson FaceComparisonResult1N, json error");
    return YSOS_ERROR_FAILED;
  }

  return YSOS_ERROR_FAILED;
}

int EncodeJson(const TDFaceDetectUnit* puint, Json::Value& json_object) {
  //assert(puint);
  if(NULL == puint)
    return YSOS_ERROR_FAILED;
  Json::Value empty_value;
  Json::Value json_obejct_detect;
  if(puint->face_detect_info_ptr_) {
    EncodeJson(*(puint->face_detect_info_ptr_.get()), json_obejct_detect);
    json_object["face_detect_info"] = json_obejct_detect;
  }else {
    json_object["face_detect_info"] = empty_value;
  }

  // convert to string;
  char tempbuf[128]={'\0'};
#ifdef _WIN32
  sprintf_s(tempbuf, sizeof(tempbuf), "%d", puint->have_perfect_face_);
#else
  snprintf(tempbuf, sizeof(tempbuf), "%d", puint->have_perfect_face_);
#endif
  json_object["have_perfect_face"] = std::string(tempbuf);

  Json::Value json_object_perface_face;
  EncodeJson(puint->perfect_face_1n_ptr_.get(), json_object_perface_face);
  json_object["perfect_face"] = json_object_perface_face;
  return YSOS_ERROR_SUCCESS;
}

int EncodeJson(const TDFaceDetectUnit* puint, std::string& json_string) {
  Json::Value json_object;
  int iret = EncodeJson(puint, json_object);
  if(YSOS_ERROR_SUCCESS != iret)
    return iret;

  try{
    Json::FastWriter writer;
    json_string = writer.write(json_object);
    json_string = GetUtility()->ReplaceAllDistinct ( json_string, "\\r\\n", "" );
    return YSOS_ERROR_SUCCESS;
  }catch(...) {
    return YSOS_ERROR_FAILED;
  }
}

int EncodeJson(const std::vector<TDFaceDetectInfoPtr>& ArrInfo,  Json::Value* out_json, std::string* out_json_string) {
   
  Json::Value json_root_value;
  try {
    json_root_value["facenum"] = (int)ArrInfo.size();

    
    Json::Value json_face_arr;
    for (unsigned int i=0; i<ArrInfo.size(); ++i) {
      Json::Value face_value;
      face_value["trackid"] = ArrInfo[i]->trackid;
      face_value["x"] = ArrInfo[i]->x;
      face_value["y"] = ArrInfo[i]->y;
      face_value["width"] = ArrInfo[i]->width;
      face_value["hight"] = ArrInfo[i]->hight;
      face_value["facescore"] = ArrInfo[i]->face_quality_score;
      face_value["imgbase64data"] = ArrInfo[i]->img_base64_data;
      face_value["gender"] = ArrInfo[i]->gender;
      face_value["age"] = ArrInfo[i]->age;
      face_value["face_feature"] = ArrInfo[i]->face_feature;
      face_value["horizontal_angle"] = ArrInfo[i]->horizontal_angle;
      face_value["vertical_angle"] = ArrInfo[i]->vertical_angle;
      face_value["estimate_distance"] = ArrInfo[i]->estimate_distance;
      face_value["serial_num"] = ArrInfo[i]->serial_num;
      json_face_arr.append(face_value);
    }
    json_root_value["faces"] = json_face_arr;
 
    if(out_json) {
      *out_json = json_root_value;
    }
    if(out_json_string) {
      Json::FastWriter json_write;
      *out_json_string = json_write.write(json_root_value);
      *out_json_string = GetUtility()->ReplaceAllDistinct ( *out_json_string, "\\r\\n", "" );
    }
    return YSOS_ERROR_SUCCESS;
  } catch (...) {
    //assert(false);
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.video"),  "error in TDGetCameraCurrentFace, json error");
    return YSOS_ERROR_FAILED;
  }

  return YSOS_ERROR_FAILED;
}


int DecodeJson(boost::property_tree::ptree& ptroot, std::vector<TDFaceDetectUnit>& vec_faceunit) {
  vec_faceunit.clear();
  try {
    /*std::string utf8string = boost::locale::conv::to_utf<char>(pJsonString, "gbk");
    std::istringstream istring(utf8string.c_str());
    boost::property_tree:: ptree ptroot, empty_tree;
    boost::property_tree::read_json(istring, ptroot);*/

    std::string string_facenumber = ptroot.get<std::string>("facenum", "0");
    int nArrNumber = atoi(string_facenumber.c_str());

    //解析结构
    boost::property_tree:: ptree  empty_tree;
    boost::property_tree::ptree faces_tree = ptroot.get_child("faces", empty_tree);
    //ArrResult = TDFaceDetectInfoArray(new TDFaceDetectInfo[nArrNumber]);
    int i=-1;
    BOOST_FOREACH(boost::property_tree::ptree::value_type &v, faces_tree) {
      ++i;
      boost::property_tree::ptree p = v.second;

      TDFaceDetectUnit face_unit;
      face_unit.face_detect_info_ptr_.reset(new TDFaceDetectInfo);
      if(NULL == face_unit.face_detect_info_ptr_) {
        return YSOS_ERROR_FAILED;
      }
      DecodeSingleJson(p, *face_unit.face_detect_info_ptr_);
      face_unit.have_perfect_face_ = p.get<int>("have_perfect_face",0);

      boost::property_tree::ptree perfect_face_json ,empty_tree;
      perfect_face_json = p.get_child("perfect_face", empty_tree);

      face_unit.perfect_face_1n_ptr_.reset(new FaceComparisonResult1N);
      if(NULL == face_unit.perfect_face_1n_ptr_) {
        return YSOS_ERROR_FAILED;
      }
      BOOST_FOREACH(boost::property_tree::ptree::value_type &w, perfect_face_json) {
        boost::property_tree::ptree wp = w.second;
        
        FaceCompareResultPtr face_compare_ptr(new FaceCompareResult);
        if(NULL == face_compare_ptr) {
          return YSOS_ERROR_FAILED;
        }
        DecodeSingleJson(wp,*face_compare_ptr);
        face_unit.perfect_face_1n_ptr_->vec_faces.push_back(face_compare_ptr);
      }

      vec_faceunit.push_back(face_unit);
    }

    //g_Log.Trace_Print(TraceLogLevel_DEBUG, __FILE__, __LINE__, "解析数据结果成功");

    return YSOS_ERROR_SUCCESS;
  } catch (...) {
    //assert(false);
    //g_Log.Trace_Print(TraceLogLevel_INFO, __FILE__, __LINE__, "解析数据结果异常,json data:%s", pJsonString);
    return YSOS_ERROR_FAILED;
  }
  return YSOS_ERROR_FAILED;
}
 
int EncodeJson2(const std::vector<TDFaceDetectUnit>&arr_face_unit, std::string* strResult, Json::Value* josn_value) {
  return EncodeJson2(arr_face_unit, true, strResult, josn_value);
}
int EncodeJson2(const std::vector<TDFaceDetectUnit>&arr_face_unit, const bool encode_imagebase64, std::string* strResult, Json::Value* josn_value){

  int arr_number = arr_face_unit.size();

  try {
    //创建一个json返回给上层
    //boost::property_tree::ptree ptree_root;
    Json::Value root_json;

    char tempbuf[1024] = {'\0'};
#ifdef _WIN32
    sprintf_s(tempbuf, sizeof(tempbuf), "%d", arr_number);
#else
    snprintf(tempbuf, sizeof(tempbuf), "%d", arr_number);
#endif
    root_json["facenum"] = tempbuf;

    //boost::property_tree::ptree tree_faces_array;
    //ptree_root.add_child("faces", tree_faces_array);
    Json::Value faces_array;

#ifdef _WIN32
    for (int i=0; i<arr_number; ++i) {
      //boost::property_tree::ptree ptree_face;
      Json::Value face_json;
      TDFaceDetectInfoPtr detect_ptr = arr_face_unit[i].face_detect_info_ptr_;
      FaceComparisonResult1NPtr recog_ptr = arr_face_unit[i].perfect_face_1n_ptr_;

      memset(tempbuf, '\0', sizeof(tempbuf));
      sprintf_s(tempbuf, "%d", detect_ptr->trackid);
      face_json["trackid"] = tempbuf;

      memset(tempbuf, '\0', sizeof(tempbuf));
      sprintf_s(tempbuf, "%d", detect_ptr->x);
      face_json["x"] = tempbuf;

      memset(tempbuf, '\0', sizeof(tempbuf));
      sprintf_s(tempbuf, "%d",detect_ptr->y);
      face_json["y"] = tempbuf;

      memset(tempbuf, '\0', sizeof(tempbuf));
      sprintf_s(tempbuf, "%d", detect_ptr->width);
      face_json["width"] = tempbuf;

      memset(tempbuf, '\0', sizeof(tempbuf));
      sprintf_s(tempbuf, "%d", detect_ptr->hight);
      face_json["hight"] = tempbuf;

      memset(tempbuf, '\0', sizeof(tempbuf));
      sprintf_s(tempbuf, "%f", detect_ptr->face_quality_score);
      face_json["facescore"]= tempbuf;

      if(encode_imagebase64) {
        face_json["imgbase64data"] =detect_ptr->img_base64_data;
      }else {
        face_json["imgbase64data"] ="";
      }
      

      face_json["gender"] = detect_ptr->gender;

      memset(tempbuf, '\0', sizeof(tempbuf));
      sprintf_s(tempbuf, "%d", detect_ptr->age);
      face_json["age"] = tempbuf;

      face_json["face_feature"] =detect_ptr->face_feature;

      memset(tempbuf, '\0', sizeof(tempbuf));
      sprintf_s(tempbuf, "%f", detect_ptr->horizontal_angle);
      face_json["horizontal_angle"] = tempbuf;

      memset(tempbuf, '\0', sizeof(tempbuf));
      sprintf_s(tempbuf, "%f", detect_ptr->vertical_angle);
      face_json["vertical_angle"] = tempbuf;

      // 人脸所占摄像头视角的比例
      memset(tempbuf, '\0', sizeof(tempbuf));
      sprintf_s(tempbuf, "%f", detect_ptr->horizontal_ratio);
      face_json["horizontal_ratio"] = tempbuf;

      memset(tempbuf, '\0',sizeof(tempbuf));
      sprintf_s(tempbuf, "%f", detect_ptr->vertical_ratio);
      face_json["vertical_ratio"] = tempbuf;

      // 估计的人脸的距离
      memset(tempbuf, '\0',sizeof(tempbuf));
      sprintf_s(tempbuf, "%f",detect_ptr->estimate_distance);
      face_json["estimate_distance"] = tempbuf;

      memset(tempbuf, '\0',sizeof(tempbuf));
      sprintf_s(tempbuf, "%d", detect_ptr->serial_num);
      face_json["serial_num"] = tempbuf;

      // have perfect face
      {
        face_json["have_perfect_face"] = arr_face_unit[i].have_perfect_face_;
        //boost::property_tree::ptree tree_faces_array;
        Json::Value face_perfect_array;

        if(recog_ptr && recog_ptr->vec_faces.size() >0) {
          FaceCompareResultPtr  face_1n_ptr = recog_ptr->vec_faces[0];
          //boost::property_tree::ptree ptree_perfect_face;
          Json::Value face_perfect_data;
          face_perfect_data["score"] = face_1n_ptr->score;
          face_perfect_data["personId"] =face_1n_ptr->person_id.c_str();
          face_perfect_data["sex"] = face_1n_ptr->sex;
          face_perfect_data["age"] = face_1n_ptr->age;
          face_perfect_data["id_card"] = face_1n_ptr->id_card.c_str();

          face_perfect_data["person_name"] = face_1n_ptr->person_name.c_str();
          face_perfect_data["mobile"] = face_1n_ptr->mobile;

          /* ptree_perfect_face.put("field1", boost::locale::conv::to_utf<char>(face_1n_ptr->field1.c_str(), "gbk"));
          ptree_perfect_face.put("field2", boost::locale::conv::to_utf<char>(face_1n_ptr->field2.c_str(), "gbk"));
          ptree_perfect_face.put("vip_flag", face_1n_ptr->vip_flag);
          ptree_perfect_face.put("user_image", boost::locale::conv::to_utf<char>(face_1n_ptr->user_image.c_str(), "gbk"));
          ptree_perfect_face.put_child("third_full_info",  face_1n_ptr->third_full_info);*/

          //tree_faces_array.push_back(std::make_pair("", ptree_perfect_face));
          face_perfect_array.append(face_perfect_data);
        }
        face_json["perfect_face"] = face_perfect_array;
      }

      //tree_faces_array.push_back(std::make_pair("", ptree_face));
      faces_array.append(face_json);
    }
#else
    for (int i=0; i<arr_number; ++i) {
      //boost::property_tree::ptree ptree_face;
      Json::Value face_json;
      TDFaceDetectInfoPtr detect_ptr = arr_face_unit[i].face_detect_info_ptr_;
      FaceComparisonResult1NPtr recog_ptr = arr_face_unit[i].perfect_face_1n_ptr_;

      memset(tempbuf, '\0', sizeof(tempbuf));
      snprintf(tempbuf, sizeof(tempbuf), "%d", detect_ptr->trackid);
      face_json["trackid"] = tempbuf;

      memset(tempbuf, '\0', sizeof(tempbuf));
      snprintf(tempbuf, sizeof(tempbuf), "%d", detect_ptr->x);
      face_json["x"] = tempbuf;

      memset(tempbuf, '\0', sizeof(tempbuf));
      snprintf(tempbuf, sizeof(tempbuf), "%d",detect_ptr->y);
      face_json["y"] = tempbuf;

      memset(tempbuf, '\0', sizeof(tempbuf));
      snprintf(tempbuf, sizeof(tempbuf), "%d", detect_ptr->width);
      face_json["width"] = tempbuf;

      memset(tempbuf, '\0', sizeof(tempbuf));
      snprintf(tempbuf, sizeof(tempbuf), "%d", detect_ptr->hight);
      face_json["hight"] = tempbuf;

      memset(tempbuf, '\0', sizeof(tempbuf));
      snprintf(tempbuf, sizeof(tempbuf), "%f", detect_ptr->face_quality_score);
      face_json["facescore"]= tempbuf;

      if(encode_imagebase64) {
        face_json["imgbase64data"] =detect_ptr->img_base64_data;
      }else {
        face_json["imgbase64data"] ="";
      }
      

      face_json["gender"] = detect_ptr->gender;

      memset(tempbuf, '\0', sizeof(tempbuf));
      snprintf(tempbuf, sizeof(tempbuf), "%d", detect_ptr->age);
      face_json["age"] = tempbuf;

      face_json["face_feature"] =detect_ptr->face_feature;

      memset(tempbuf, '\0', sizeof(tempbuf));
      snprintf(tempbuf, sizeof(tempbuf), "%f", detect_ptr->horizontal_angle);
      face_json["horizontal_angle"] = tempbuf;

      memset(tempbuf, '\0', sizeof(tempbuf));
      snprintf(tempbuf, sizeof(tempbuf), "%f", detect_ptr->vertical_angle);
      face_json["vertical_angle"] = tempbuf;

      // 人脸所占摄像头视角的比例
      memset(tempbuf, '\0', sizeof(tempbuf));
      snprintf(tempbuf, sizeof(tempbuf), "%f", detect_ptr->horizontal_ratio);
      face_json["horizontal_ratio"] = tempbuf;

      memset(tempbuf, '\0',sizeof(tempbuf));
      snprintf(tempbuf, sizeof(tempbuf), "%f", detect_ptr->vertical_ratio);
      face_json["vertical_ratio"] = tempbuf;

      // 估计的人脸的距离
      memset(tempbuf, '\0',sizeof(tempbuf));
      snprintf(tempbuf, sizeof(tempbuf), "%f",detect_ptr->estimate_distance);
      face_json["estimate_distance"] = tempbuf;

      memset(tempbuf, '\0',sizeof(tempbuf));
      snprintf(tempbuf, sizeof(tempbuf), "%d", detect_ptr->serial_num);
      face_json["serial_num"] = tempbuf;

      // have perfect face
      {
        face_json["have_perfect_face"] = arr_face_unit[i].have_perfect_face_;
        //boost::property_tree::ptree tree_faces_array;
        Json::Value face_perfect_array;

        if(recog_ptr && recog_ptr->vec_faces.size() >0) {
          FaceCompareResultPtr  face_1n_ptr = recog_ptr->vec_faces[0];
          //boost::property_tree::ptree ptree_perfect_face;
          Json::Value face_perfect_data;
          face_perfect_data["score"] = face_1n_ptr->score;
          face_perfect_data["personId"] =face_1n_ptr->person_id.c_str();
          face_perfect_data["sex"] = face_1n_ptr->sex;
          face_perfect_data["age"] = face_1n_ptr->age;
          face_perfect_data["id_card"] = face_1n_ptr->id_card.c_str();

          face_perfect_data["person_name"] = face_1n_ptr->person_name.c_str();
          face_perfect_data["mobile"] = face_1n_ptr->mobile;

          /* ptree_perfect_face.put("field1", boost::locale::conv::to_utf<char>(face_1n_ptr->field1.c_str(), "gbk"));
          ptree_perfect_face.put("field2", boost::locale::conv::to_utf<char>(face_1n_ptr->field2.c_str(), "gbk"));
          ptree_perfect_face.put("vip_flag", face_1n_ptr->vip_flag);
          ptree_perfect_face.put("user_image", boost::locale::conv::to_utf<char>(face_1n_ptr->user_image.c_str(), "gbk"));
          ptree_perfect_face.put_child("third_full_info",  face_1n_ptr->third_full_info);*/

          //tree_faces_array.push_back(std::make_pair("", ptree_perfect_face));
          face_perfect_array.append(face_perfect_data);
        }
        face_json["perfect_face"] = face_perfect_array;
      }

      //tree_faces_array.push_back(std::make_pair("", ptree_face));
      faces_array.append(face_json);
    }
#endif
    
    root_json["faces"] = faces_array;

    if(strResult) {
      Json::FastWriter json_writer;
      *strResult = json_writer.write(root_json);
      *strResult = GetUtility()->ReplaceAllDistinct ( *strResult, "\\r\\n", "" );
    }
    if(josn_value) {
      *josn_value = root_json;
    }
    //ptree_root.add_child("faces", tree_faces_array);
    //std::ostringstream ostring_stream;
    //boost::property_tree::write_json(ostring_stream, ptree_root);
    //strResult = boost::locale::conv::from_utf<char>(ostring_stream.str(), "gbk");
    
    return YSOS_ERROR_SUCCESS;
  } catch (...) {
    //assert(false);
    //g_Log.Trace_Print(TraceLogLevel_ERROR, __FILE__, __LINE__, "error in TDGetCameraCurrentFace, json error");
    return YSOS_ERROR_FAILED;
  }
  return YSOS_ERROR_FAILED;
}



int DecodeJson2(boost::property_tree::ptree& ptroot, std::vector<TDFaceDetectUnit>& vec_faceunit) {
  //assert(pJsonString);
  //if (NULL == pJsonString)
  //  return YSOS_ERROR_FAILED;

  vec_faceunit.clear();
  try {
    /*std::string utf8string = boost::locale::conv::to_utf<char>(pJsonString, "gbk");
    std::istringstream istring(utf8string.c_str());
    boost::property_tree:: ptree ptroot, empty_tree;
    boost::property_tree::read_json(istring, ptroot);*/

    std::string string_facenumber = ptroot.get<std::string>("facenum", "0");
    int nArrNumber = atoi(string_facenumber.c_str());

    //解析结构
    boost::property_tree:: ptree  empty_tree;
    boost::property_tree::ptree faces_tree = ptroot.get_child("faces", empty_tree);
    //ArrResult = TDFaceDetectInfoArray(new TDFaceDetectInfo[nArrNumber]);
    int i=-1;
    BOOST_FOREACH(boost::property_tree::ptree::value_type &v, faces_tree) {
      ++i;
      boost::property_tree::ptree p = v.second;

      TDFaceDetectUnit face_unit;
      face_unit.face_detect_info_ptr_.reset(new TDFaceDetectInfo);
      if(NULL == face_unit.face_detect_info_ptr_) {
        return YSOS_ERROR_FAILED;
      }
      DecodeSingleJson(p, *face_unit.face_detect_info_ptr_);
      face_unit.have_perfect_face_ = p.get<int>("have_perfect_face",0);

      boost::property_tree::ptree perfect_face_json ,empty_tree;
      perfect_face_json = p.get_child("perfect_face", empty_tree);

      face_unit.perfect_face_1n_ptr_.reset(new FaceComparisonResult1N);
      if(NULL == face_unit.perfect_face_1n_ptr_) {
        return YSOS_ERROR_FAILED;
      }
      BOOST_FOREACH(boost::property_tree::ptree::value_type &w, perfect_face_json) {
        boost::property_tree::ptree wp = w.second;
        
        FaceCompareResultPtr face_compare_ptr(new FaceCompareResult);
        if(NULL == face_compare_ptr) {
          return YSOS_ERROR_FAILED;
        }
        DecodeSingleJson(wp,*face_compare_ptr);
        face_unit.perfect_face_1n_ptr_->vec_faces.push_back(face_compare_ptr);
      }

      vec_faceunit.push_back(face_unit);
    }

    //g_Log.Trace_Print(TraceLogLevel_DEBUG, __FILE__, __LINE__, "解析数据结果成功");

    return YSOS_ERROR_SUCCESS;
  } catch (...) {
    //assert(false);
    //g_Log.Trace_Print(TraceLogLevel_INFO, __FILE__, __LINE__, "解析数据结果异常,json data:%s", pJsonString);
    return YSOS_ERROR_FAILED;
  }
  return YSOS_ERROR_FAILED;
}
 

}