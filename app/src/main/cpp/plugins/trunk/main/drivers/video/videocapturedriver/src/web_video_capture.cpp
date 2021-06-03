#include "../include/web_video_capture.h"


//#include "include/ysoslog/ysoslog.h"
//#include <cloudwalk_sdk/Include/CWFaceDetection.h>
#include <json/json.h>
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/protect/include/os_hal_package/datainterfacemanager.h"

#ifdef _WIN32
// 视频显示窗口
#define WND_CLASS_NAME "WebDisplayWnd"

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  switch (msg) {
    case WM_SETCURSOR: {
      HINSTANCE hIns = GetModuleHandle(NULL);
      SetCursor(LoadCursor(hIns, MAKEINTRESOURCE(IDC_ARROW)));
      break;
    }
  }
  return DefWindowProc(hwnd, msg, wparam, lparam);
}
#else
//TODO:add for linux
#endif


namespace ysos {

// Class WebVideoCapture ------------------------------------------------------

WebVideoCapture::WebVideoCapture()
    : width_(0),
      height_(0),
      is_start_(false),
      is_paused_(false),
      detect_handle_(0)/*,
      hDc_(0),
      display_wnd_(0)*/ {
#ifdef _WIN32
  HINSTANCE hIns = GetModuleHandle(NULL);
  WNDCLASSEX wc = {
      sizeof(WNDCLASSEX), CS_HREDRAW|CS_VREDRAW, WndProc,
      0L, 0L, hIns, NULL,
      LoadCursor(NULL, IDC_ARROW), NULL, NULL,
      WND_CLASS_NAME, NULL
  };
  RegisterClassEx(&wc);
  logger_ = GetUtility()->GetLogger("ysos.video");
  
  face_detect_file_ =
      "..\\data\\facedetectdriver\\CWModels\\_configs_frontend_x86_arm.xml";
  cw_errcode_t errCode = CW_SDKLIT_OK;
  detect_handle_ = cwCreateDetHandle(&errCode, face_detect_file_.c_str(), 0);
  if (detect_handle_) {
    cw_det_param_t param;
    cwGetFaceParam(detect_handle_, &param);
    param.pConfigFile = face_detect_file_.c_str();
    cwSetFaceParam(detect_handle_, &param);
  } else {
    YSOS_LOG_ERROR("Create detect handle failed: " << errCode);
  }
#else
//TODO:add for linux
#endif
}

WebVideoCapture::~WebVideoCapture() {
  CloseWindow();
}

void WebVideoCapture::Start(int x, int y, int width, int height) {
  width_ = width;
  height_ = height;
  is_start_ = true;
  is_paused_ = false;
#ifdef _WIN32
  if (display_wnd_ == NULL) {
    wnd_thread_.reset(new boost::thread(boost::bind(&WebVideoCapture::WindowThread, this, x, y)));
  }
#else
//TODO:add for linux
#endif
}

void WebVideoCapture::Pause() {
  is_paused_ = true;
#ifdef _WIN32
  boost::lock_guard<boost::mutex> lock(frame_mutex_);
  int detect_flag = CW_OP_DET | CW_OP_QUALITY;
  cw_img_t srcImg;
  srcImg.data = (char*)captured_frame_.data;
  srcImg.width = width_;
  srcImg.height = height_;
  srcImg.angle = CW_IMAGE_ANGLE_0;
  srcImg.format = CW_IMAGE_BGR888;

  // 人脸检测
  if (!detect_handle_) {
    YSOS_LOG_ERROR("Face detect handle is null.");
    return;
  }
  cw_face_res_t face_buffers[5];
  memset(face_buffers, 0, sizeof(face_buffers));
  int detect_count = 0;
  cw_errcode_t errCode = CW_SDKLIT_OK;
  errCode = cwFaceDetection(detect_handle_, &srcImg, face_buffers, 5, &detect_count, detect_flag);
  if (errCode != CW_SDKLIT_OK) {
    YSOS_LOG_DEBUG ("Face detect failed, errorcode: " << errCode);
    return;
  }

  Json::Value root;
  root["type"] = "capture_result";
  root["count"] = detect_count;
  if (detect_count == 1) {
    root["quality"] = face_buffers[0].quality.scores[0];
  } else {
    root["quality"] = 0;
  }
  Json::FastWriter writer;
  std::string result = writer.write(root);
  result = GetUtility()->ReplaceAllDistinct ( result, "\\r\\n", "" );

  DataInterfacePtr ptr = GetDataInterfaceManager()->GetData(GLOBAL_DATA_KEY);
  ptr->SetData("capture_result", result);
#else
//TODO:add for linux
#endif
}

void WebVideoCapture::Stop() {
  is_paused_ = false;
  is_start_ = false;
  CloseWindow();
}

void WebVideoCapture::OnVideoFrame(const cv::Mat& frame) {
#ifdef _WIN32
  if (is_start_ && !is_paused_) {
    boost::lock_guard<boost::mutex> lock(frame_mutex_);
    cv::resize(frame, captured_frame_, cv::Size(width_, height_));

    memset(&bmInfo_, 0, sizeof(bmInfo_));
    bmInfo_.bmiHeader.biSize   = sizeof(BITMAPINFOHEADER);  
    bmInfo_.bmiHeader.biWidth  = width_;  
    bmInfo_.bmiHeader.biHeight = -height_;  
    bmInfo_.bmiHeader.biPlanes = 1;  
    bmInfo_.bmiHeader.biBitCount = 24;  
    bmInfo_.bmiHeader.biCompression = BI_RGB;
    StretchDIBits(hDc_,0, 0, width_, height_, 0, 0, width_, height_, captured_frame_.data, &bmInfo_, DIB_RGB_COLORS, SRCCOPY);
  }
#else
//TODO:add for linux
  
#endif
}

std::string WebVideoCapture::GetCaptureResult() {
  std::string data = capture_result_;
  capture_result_.clear();
  return data;
}

void WebVideoCapture::CloseWindow() {
#ifdef _WIN32
  if (hDc_ != NULL) {
    ReleaseDC(display_wnd_, hDc_);
    hDc_ = NULL;
  }

  if (display_wnd_ != NULL) {
    PostMessage(display_wnd_, WM_CLOSE, 0, 0);
    display_wnd_ = NULL;
  }
#else
//TODO:add for linux
#endif
}

void WebVideoCapture::WindowThread(int x, int y) {
#ifdef _WIN32
  HINSTANCE hIns = GetModuleHandle(NULL);
  display_wnd_ = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
                                WND_CLASS_NAME, "", WS_POPUP,
                                x, y, width_, height_, NULL, NULL, hIns, NULL);
  ShowWindow(display_wnd_, SW_SHOW);
  hDc_ = GetDC(display_wnd_);

  MSG msg;
  while (GetMessage(&msg, nullptr, 0, 0)) {
    if (!TranslateAccelerator(msg.hwnd, NULL, &msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
#else
//TODO:add for linux
#endif
}

} // namespace ysos
