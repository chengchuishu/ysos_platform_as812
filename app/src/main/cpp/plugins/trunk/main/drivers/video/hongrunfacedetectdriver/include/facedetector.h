/**
 *@file face_detector.h
 *@brief 人脸检测接口
 *@version 1.0
 *@author WangDaqian
 *@date Created on: 2018/1/19
 *@copyright Copyright(c) 2018 YunShen Technology. All rights reserved.
**/


#ifndef YSOS_FACE_DETECTOR_H
#define YSOS_FACE_DETECTOR_H

/// stl headers
#include <vector>

/// boost headers
#include <boost/shared_ptr.hpp>


// private headers
#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"

#include "../../../public/include/common/facedetectstruct.h"

namespace ysos {

  //;评估人脸距离使用摄像头ID号. 0 表示未知， 1 logic HD720P, 3 realsense, 3 usb 广角摄像头
  enum FstimatePeopleDistanceUseCameraid {
    FPDUC_NULL              =0,
    FPDUC_LOGIC720P         =1,
    FPDUC_REALSENSE         =2,
    FPDUC_USE_WIDE_ANGLE    =3
  };

class FaceDetectorInterface {
public:
  
  /**
   *@brief 初始化
   *@param mouule_file_path 本模块的路径
   *@return success return YSOS_ERROR_SUCCESS
   *        fail return YSOS_ERROR_FAILED
   */
  virtual int Init(const char* mouule_file_path) =0;

      /**
   *@brief 释放资源
   *@param 
   *@return success return YSOS_ERROR_SUCCESS
   *        fail return YSOS_ERROR_FAILED
   */
  virtual int UnInit() =0;


    /**
   *@brief 读取配置项
   *@param
   *@return success return YSOS_ERROR_SUCCESS
   *        fail return YSOS_ERROR_FAILED
   */
   virtual int SetConfigKeyValue(const std::string &key, const std::string &value) =0;

   /**
   *@brief 人脸检测，检测到人。 人脸框，头部姿态（抬头、低头,左右转头，平面内偏头），对齐人脸，人脸质量（亮度，清晰度，对称性，眼镜，肤色，嘴部，左眼，右眼）
   *@param need_track[Input] 是否开启人脸跟踪，如果开启，则人脸不会过滤。速度快
   *                         如果开启，则会过滤低质量的人脸，与最近的人脸进行人脸1比1，从而定位到某个人。
   *@param need_face_qulity[Input] 是否获取人脸质量
   *@param need_key_point[Input] 是否获取关键点信息
   *@param need_face_attribute[Input] 是否获取人脸属性（人脸性别、年龄）
   *@param need_face_nationality[Input] 是否获取人脸国家属性（废除）
   *@param get_face_image_base64[Input] 是否获取人脸base64数据
   *@param get_field_feature[Input] 是否获取人脸特征
   *@param get_prob_feature[Input] 是否获取人脸prob特征（有的第三方会有这个特征，用这个特征去比对其他人脸特征）。
   *@param vec_face_detect_info[Output] 输出检测到的人脸信息
   *@return success return YSOS_ERROR_SUCCESS
   *        fail return YSOS_ERROR_FAILED
   */
  virtual int FaceDetect(const char* pdata, const int& widht, const int& heignt, const int& channels,
    const bool& need_track, const bool& need_face_qulity, const bool& need_key_point,
    const bool& need_face_attribute, const bool& need_face_nationality,const bool& get_face_image_base64,
    const bool& get_field_feature,const bool& get_prob_feature,
    std::vector<TDFaceDetectInfoPtr>& vec_face_detect_info) =0;


  /**
   *@brief 人脸比对（1:1 特征比对），会连接服务器
   *@param pTerminalId[Input] 机器人终端号,不能为空
   *@param pOrgid[Input] 机器人所属机构，不能为空
   *@param face_fetatue1[Input] 人脸特征值A，不能为空
   *@param face_fetatue1_length[Input] 人脸特征值A数据的长度
   *@param face_feature2[Input] 人脸特征值B，不能为空
   *@param face_feature2_length[Input] 人脸特征值B数据的长度
   *@param compare_result[Output] 比对结果，对服务端返回的结果放入对象中
   *@return success return YSOS_ERROR_SUCCESS,
   *        fail return one of(YSOS_ERROR_LOGIC_ERROR)
   */
  virtual int Comparison11Local(const char* face_fetatue1, const int face_fetatue1_length,
                        const char* face_feature2, const int face_feature2_length, FaceComparisonResult11* compare_result) =0;


};

typedef boost::shared_ptr<FaceDetectorInterface> FaceDetectorInterfacePtr;

} // namespace ysos

#endif  // YSOS_FACE_DETECTOR_H
