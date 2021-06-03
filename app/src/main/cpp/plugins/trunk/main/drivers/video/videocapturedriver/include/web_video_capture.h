#ifndef YSOS_PLUGIN_WEB_VIDEO_CAPTURE_H_
#define YSOS_PLUGIN_WEB_VIDEO_CAPTURE_H_


#include <boost/scoped_ptr.hpp>
#include <boost/thread/thread.hpp>

#include "../../../../../thirdparty/opencv/include/opencv2/opencv.hpp"

#include <string>

#include "../../../../../../core/trunk/main/protect/include/core_help_package/log.h"

#ifdef _WIN32
#include <windows.h>
#else
#endif


namespace ysos {

class WebVideoCapture {
public:
  WebVideoCapture();
  virtual ~WebVideoCapture();

  // 开始显示视频
  virtual void Start(int x, int y, int width, int height);
  // 采集一张图片
  virtual void Pause();
  // 停止显示视频
  virtual void Stop();
  // 更新视频帧
  virtual void OnVideoFrame(const cv::Mat& frame);

  std::string GetCaptureResult();

private:
  void CloseWindow();
  void WindowThread(int x, int y);

  std::string capture_result_;
  // 视频显示窗口尺寸
  int width_, height_;
#ifdef _WIN32
  HWND display_wnd_;
  HDC hDc_;
  BITMAPINFO bmInfo_;
#endif
  // 采集的图片
  cv::Mat captured_frame_;
  // 窗口线程
  boost::scoped_ptr<boost::thread> wnd_thread_;
  boost::mutex frame_mutex_;
  
  log4cplus::Logger logger_;

  // 云从参数
  std::string face_detect_file_;
  // 人脸检测接口
  void* detect_handle_;

  // 是否开始
  bool is_start_;
  // 是否暂停
  bool is_paused_;

};

} // namespace ysos

#endif // YSOS_PLUGIN_WEB_VIDEO_CAPTURE_H_