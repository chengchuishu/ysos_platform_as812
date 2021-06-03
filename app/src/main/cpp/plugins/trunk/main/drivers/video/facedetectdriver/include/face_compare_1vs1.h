/**
 *@file face_compare_1vs1.h
 *@brief 人脸1比1头文件
 *@version 1.0
 *@author WangDaqian
 *@date Created on: 2018/4/2
 *@copyright Copyright(c) 2018 YunShen Technology. All rights reserved.
**/


#ifndef YSOS_FACE_COMPARE_1VS1_H
#define YSOS_FACE_COMPARE_1VS1_H

#include "face_recognizer.h"
#include "circle_queue.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#ifdef _WIN32
  #include <windows.h>
#else
  //TODO:add for linux
#endif

// boost headers
#include <boost/scoped_ptr.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/thread.hpp>

namespace ysos {

class FaceCompare1vs1 {
public:
  FaceCompare1vs1();
  ~FaceCompare1vs1();
  void Init();
  void Uninit();

  // 人脸1比1对外接口
  void Compare(int track_id);
  // 获取人脸比较结果
  // @result: int数组, 0: track id; 1: 是不是同一个人
  bool GetCompareResult(int* result);
  void AddFaceInfo(boost::shared_ptr<FaceInfo>& face);
  // 人脸注册对外接口
  void Register(int track_id);
  // 取消注册
  void Unregister(int track_id);

private:
  void ThreadFunc();
  // 执行人脸1比1
  void DoCompare(int track_id);
  // 执行人脸注册
  void DoRegister(int track_id);
  
  log4cplus::Logger logger_;
  // 工作线程
  boost::scoped_ptr<boost::thread> thread_;
  // 线程锁
  boost::mutex mutex_;
  // 等待信号
  boost::condition thread_waiter_;
  // 云从参数
  std::string recognize_handle_file_;
  // 云从人脸识别接口
  void* recog_handle_;
  // 追踪过的人脸信息
  CircleQueue<boost::shared_ptr<FaceInfo> > faces_info_;
  // 注册的人脸
  boost::shared_ptr<FaceInfo> register_face_;
  // 比较得分
  float compare_score_;
  // 比较的track id
  int compare_track_id_;
  // 线程中执行的操作
  boost::function<void()> thread_operation_;

  // 线程退出
  bool is_thread_quit_;
  // 比较结果是否返回
  bool is_result_returned_;

};

} // namespace ysos

#endif  // YSOS_FACE_COMPARE_1VS1_H
