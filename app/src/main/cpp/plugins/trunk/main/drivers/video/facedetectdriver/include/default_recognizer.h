/**
 *@file default_recognizer.h
 *@brief 公版人脸识别头文件
 *@version 1.0
 *@author WangDaqian
 *@date Created on: 2018/1/19
 *@copyright Copyright(c) 2018 YunShen Technology. All rights reserved.
**/


#ifndef YSOS_DEFAULT_RECOGNIZER_H
#define YSOS_DEFAULT_RECOGNIZER_H

#include "face_recognizer.h"
#include "circle_queue.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#ifdef _WIN32
  #include <windows.h>
#else
  //TODO:add for linux
  #include "../../../public/include/common/event.h"
#endif

// boost headers
#include <boost/scoped_ptr.hpp>
#include <boost/thread/thread.hpp>

namespace ysos {

class DefaultRecognizer : public FaceRecognizer {
public:
  DefaultRecognizer();
  ~DefaultRecognizer();
  void Recognize(
      std::vector<boost::shared_ptr<FaceInfo> > &params) override;
  bool GetRecognizeResult(
      std::vector<boost::shared_ptr<FaceInfo> > &params) override;
  void Init();
  void Uninit();

private:
  void ThreadFunc();
  // 离线人脸识别
  bool LocalRecognize();
  // 请求服务器进行人脸识别
  void RequestRecognize();
  // 更新本地人脸库
  void UpdateLocalFaceInfo(boost::shared_ptr<FaceInfo> face);

  // 上一次的识别时间
  boost::posix_time::ptime last_recognize_time_;

  log4cplus::Logger logger_;
  // 任务通知事件
  #ifdef _WIN32
    HANDLE work_event_;
  #else
    event_handle work_event_;
  #endif
  // 工作线程
  boost::scoped_ptr<boost::thread> thread_;
  // 待识别的人脸
  boost::shared_ptr<FaceInfo> recog_face_;
  // 上一次识别的人脸
  boost::shared_ptr<FaceInfo> last_recog_face_;
  // 访问结果的锁
  boost::mutex result_lock_;
  // 云从人脸识别接口
  void* recog_handle_;
  // 云从参数
  std::string recognize_handle_file_;
  // 离线人脸信息
  CircleQueue<boost::shared_ptr<FaceInfo> > local_face_infos_;

  // 是不是正在进程人脸识别
  bool is_working_;
  // 线程退出
  bool is_thread_quit_;
  // 识别结果是否返回
  bool is_result_returned_;

};

} // namespace ysos

#endif  // YSOS_DEFAULT_RECOGNIZER_H
