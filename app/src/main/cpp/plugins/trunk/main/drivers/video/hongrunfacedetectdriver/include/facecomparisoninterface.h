/**   //NOLINT
  *@file facecomparisoninterface.h
  *@brief Definition of 
  *@version 0.1
  *@author wanfenfen
  *@date Created on: 2018:3:12   15:01
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  *@howto Usage:
  *@todo
  */

#ifndef TD_CAMERA_FACECOMPARISIONINTERFACE_H_
#define TD_CAMERA_FACECOMPARISIONINTERFACE_H_

/// stl headers
#include <string>
#include <vector>
#include <list>

/// boost headers
#include <boost/shared_ptr.hpp>

/// private headers
#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"

namespace ysos{

class FaceComparisonInterface
{
public:
  //virtual int Init(const char* pModuleFilePath) =0;
  //virtual int UnInit() =0;
  
  virtual ~FaceComparisonInterface(){};
  /**
    *@brief 读取配置项
    *@param 
    *@return   
    */
  //virtual int ReadConfig(const char* pModuleFilePath) =0;

  /**
   *@brief 1比n比对是否支持一次发送多个不同人脸，并且返回多个人脸的信息。
   *@param 
   *@return   
   */
  virtual bool IsSupportMulPersionOneTime1N() =0;

  /**
    *@brief 人脸识别（1比N特征比较）， 支持多个图片，每个图片的比对结果
    *@param vec_comparisonresult[Output] 如果1比n比对结果没有匹配到的人，则对应的位置上存储的是一个NULL指针
    *@return   
    */
  //virtual int Comparison1N(const std::vector<std::string>& vec_face_picture_base64,const long timeout_milliseconds,std::vector<FaceComparisonResult1NPtr>& vec_comparisonresult, std::string* error_message) =0;

  virtual int Comparison1N(const char* pTerminalId, const char* pOrgid, const char* pFaceFeature, const int iFaceFeatureDataLength,
    const int iTopN,int filter_user_mode, std::string* pstrComparisonReulstOriginal, FaceComparisonResult1N* pComparisonResult, std::string* error_message) {return -1;}
  virtual int Comparison1N(const char* pTerminalId, const char* pOrgid, std::list<std::string>& list_face_features,
    const int iTopN, int filter_user_mode,std::string* pstrComparisonReulstOriginal, FaceComparisonResult1N* pComparisonResult, std::string* error_message) {return -1;}

  virtual int Comparison1NAccordBase64(const char* pTerminalId, const char* pOrgid, const std::vector<std::string>& vector_face_base64,
    const int iTopN, const int timeout_milliseconds, std::string* pstrComparisonReulstOriginal, std::vector<FaceComparisonResult1NPtr>& vec_comparisonresult, std::string* error_message) {return -1;}

  virtual int Comparison11(const char* pTerminalId, const char* pOrgid, const char* pFaceFeatureA, const int iFaceFeatureDataLengthA,
    const char* pFaceFeatureB, const int iFaceFeatureDataLengthB, std::string* pstrComparisonReulstOriginal, FaceComparisonResult11* pCompareResult11, std::string* error_message) {return -1;};

  virtual int FaceImageRegister(const char* pTerminalId, const char* pOrgid, const char* pFaceImageData, const int iFaceImageDataLength,
    const char* pPersonName, const int iAge, std::string strSex, std::string strIdCard, std::string strMobileNum, std::string strAddress, 
    std::string* pstrComparisonReulstOriginal, FaceRegisterResult* pRegisterResult, std::string* error_message) {return -1;};

  virtual int FaceImageRegister2(const char* pTerminalId, const char* pOrgid,
    const char* pPersonName, const int iAge, std::string strSex, std::string strIdCard, std::string strMobileNum, std::string strAddress,
    std::string field1, std::string field2,
    std::list<std::string> list_image_base64,
    std::string person_id,
    std::string* pstrComparisonReulstOriginal, FaceRegisterResult* pRegisterResult, std::string* error_message) {return -1;}

protected:
private:
};

typedef boost::shared_ptr<FaceComparisonInterface> FaceComparisonInterfacePtr;


}
#endif
