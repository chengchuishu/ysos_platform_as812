/**
 *@file facedetectmodule.h
 *@brief Definition of face detect module
 *@version 2.0
 *@author libaoguo
 *@date Created on: 2017-08-25
 *@copyright Copyright(c) 2017 YunShen Technology. All rights reserved.
**/

#ifndef MODULE_FACE_DETECT_H_
#define MODULE_FACE_DETECT_H_

/// ysos headers
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemoduleimpl.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemodulecallbackimpl.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/driverinterface.h"

/// private headers
#include "../../../public/include/common/pluginbasemoduleimpl.h"
#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"
#include "../../../public/include/common/facedetectstruct.h"


namespace ysos {
class YSOS_EXPORT FaceDetectModule: public PluginBaseModuleImpl
{
  DECLARE_PROTECT_CONSTRUCTOR(FaceDetectModule);
  DISALLOW_COPY_AND_ASSIGN(FaceDetectModule);
  DECLARE_CREATEINSTANCE(FaceDetectModule);

public:
  /**
   *@brief inhert from PluginBaseModuleImpl;
   *      （只处理 输出能力为””调用，其他的转发给driver）
   *      others will be pass to driver to process
   *@param 
   *@return   
   */
  virtual int GetProperty(int iTypeId, void *piType);
  int Ioctl(INT32 control_id, LPVOID param) override;

protected:
  virtual int Initialized(const std::string &key, const std::string &value);
  int PickOnePerson(const int& face_number, const TDFaceDetectInfoArray& face_info_arr);
  bool IsInExcludeFace(const std::string* face_feature);
  virtual int AbilityFaceRecognition_StreamRGB24P6ToTextJson(AbilityParam* input_param, AbilityParam* output_param);
  int GenerateFaceInfo(TDFaceDetectUnit* puint, std::string& json_string);
  int RealOpen(LPVOID param = NULL) override;
  int RealRun() override;

  // 把人脸识别的结果转成json
  void GenerateRecognizeResult(const FaceComparisonResult1N &result,
                             AbilityParam* pout);

  // 人脸检测
  int DetectFace(AbilityParam* input_param, AbilityParam* output_param);

  // 处理PROP_FUN_CALLABILITY
  int OnPropCallAbility(void *piType);
  // 人脸检测和识别分开
  int OnPropCallAbility2(void *piType);
  // 处理PROP_FACE_SELECT_ONE
  int OnPropFaceSelectOne(void *piType);

private:
  float mp_select_one_min_score_;
  float min_face_score_;

  bool need_compare1n_;
  bool need_imgbase64_;
  std::string device_id_;
  boost::posix_time::ptime last_detect_time_;
  boost::posix_time::ptime last_recognize_time_;
  int detect_interval_;
  int recognize_interval_;
  // 服务器端最低识别分数
  float min_recognize_score_;
  // 本地端最低识别分数
  float min_local_recognize_score_;
  // 人脸识别服务器地址
  std::string recognize_server_;
  // 人脸检测器类型
  std::string face_detector_;
  // 人脸识别器类型
  std::string face_recognizer_;
  // 人脸的最小尺寸, 太小的不去识别
  int min_face_width_;
  int min_face_height_;

  BufferInterfacePtr buffer_ptr_;

  // 是否启用本地人脸识别
  bool enable_local_recognize_;
  // 是否可以进行人脸检测
  //bool is_detect_enable_;

};

} // namespace ysos

#endif    //MODULE_FACE_DETECT_H_