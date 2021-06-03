/**   //NOLINT
  *@file videocapturedriver.h
  *@brief Definition of VideoCaptureDriver for windows.
  *@version 0.1
  *@author venucia
  *@date Created on: 2016/6/13   19:45
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  *
  */
/// self header
#include "../include/displaydriver.h"

//#pragma comment( lib, "gdiplus.lib" )
//#include <GdiPlus.h>
//using namespace Gdiplus;


//#include <GdiPlusGraphics.h>
//#include <GdiPlusBitmap.h>
//#include <atlcom.h>

/// Ysos Headers //  NOLINT
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"

/// third party opencv headers
/// opencv headers
#include "../../../../../thirdparty/opencv/include/opencv2/opencv.hpp"
#include "../../../../../thirdparty/opencv/include/opencv2/highgui/highgui.hpp"
#include "../../../../../thirdparty/opencv/include/opencv2/core/core.hpp"

/// private headers
#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"


namespace ysos {

DECLARE_PLUGIN_REGISTER_INTERFACE(DisplayDriver, DriverInterface);
DisplayDriver::DisplayDriver(const std::string &strClassName): BaseInterfaceImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.video");
  //* add for linux
  //pre_hwnd_ =  NULL;
  //pre_hdc_ = cur_hdc_ = NULL;
  left_ = top_ = 0;
  width_ = 120;
  height_ = 160;
  is_mirror_mode_ = true;
  //*/
}

DisplayDriver::~DisplayDriver() {
  // 所有的资源释放放到UnInitialize函数中
  // UnInitialize();
}

int DisplayDriver::RealUnInitialize(void *param) {
  YSOS_LOG_DEBUG("uninitialize logname:" <<logic_name_);
  /* add for linux
  if (NULL != pre_hdc_) {
    ::ReleaseDC((HWND)pre_hwnd_, pre_hdc_);
  }
  pre_hwnd_ = NULL;
  pre_hdc_ = NULL;
  */
  return YSOS_ERROR_SUCCESS;
}

int DisplayDriver::Read(BufferInterfacePtr pBuffer, DriverContexPtr context_ptr) {
  return YSOS_ERROR_SUCCESS;
}
/* add for linux
int DisplayDriver::CheckHDC(void *cur_hwnd) {
  if (pre_hwnd_ == cur_hwnd) {
    if (NULL != cur_hwnd) {
      cur_hdc_ = pre_hdc_;
      return YSOS_ERROR_SUCCESS;
    } else {
      YSOS_LOG_ERROR("############### DisplayDriver::Write cur_hwnd is null");
      return YSOS_ERROR_FAILED;
    }
  }

  if (NULL != pre_hdc_) {
    ::ReleaseDC((HWND)pre_hwnd_, pre_hdc_);
    pre_hwnd_ = NULL;
    pre_hdc_ = NULL;
  }

  BOOL is_window = IsWindow((HWND)cur_hwnd);
  if (!is_window) {
    YSOS_LOG_ERROR("############### DisplayDriver::Write hwnd_ is not a window");
    return YSOS_ERROR_FAILED;
  }

  cur_hdc_ = ::GetDC((HWND)cur_hwnd);
  if (NULL == cur_hdc_) {
    YSOS_LOG_ERROR("############### DisplayDriver::Write hdc is null");
    return YSOS_ERROR_FAILED;
  }

  pre_hwnd_ = cur_hwnd;
  pre_hdc_ = cur_hdc_;

  return YSOS_ERROR_SUCCESS;
}

static HWND g_hwnd_ = NULL;
*/
int DisplayDriver::Write(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, DriverContexPtr context_ptr) {
  if (NULL == input_buffer) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  uint8_t *data = GetBufferUtility()->GetBufferData(input_buffer);
  if (NULL == data) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  YSOS_LOG_DEBUG("############### DisplayDriver::Write begin");
  /* add for linux
  unsigned short *width = (unsigned short*)(data + 2);
  unsigned short *height = (unsigned short*)(data + 4);
  uint8_t *bmp_data = data + 6;
  
  g_hwnd_ = NULL;
  GetHwndByName(title_);
  int ret = CheckHDC(g_hwnd_);
  if(YSOS_ERROR_SUCCESS != ret) {
    YSOS_LOG_DEBUG("############### DisplayDriver::CheckHDC failed");
    return YSOS_ERROR_SKIP;
  }
  */
  /* add for linux
  if (is_mirror_mode_) {
    /// 设置显示方式
    int isetmode_ret =::SetGraphicsMode(cur_hdc_, GM_ADVANCED);  ///< If the function succeeds, the return value is the old graphics mode.If the function fails, the return value is zero.
    assert(0 != isetmode_ret);
    XFORM xForm;
    ///< x' = x * eM11 + y * eM21 + eDx,   ///< y' = x * eM12 + y * eM22 + eDy,
    ///水平翻转 x' = x * cose(180) + y * sin(0) + eDx,   ///< y' = x * sin(0) + y * cose(0) + eDy,
    xForm.eM11 = (FLOAT) -1.0;
    xForm.eM12 = (FLOAT) 0.0;
    xForm.eM21 = (FLOAT) 0.0;
    xForm.eM22 = (FLOAT) 1.0;
    xForm.eDx  = (FLOAT) width_;
    xForm.eDy  = (FLOAT) 0.0;
    bool set_ret = SetWorldTransform(cur_hdc_, &xForm);  ///< If the function succeeds, the return value is nonzero. If the function fails, the return value is zero.
    assert(set_ret);
  }
  */
  /* add for linux
  //BMP Header
  BITMAPINFO m_bmphdr= {0};
  DWORD dwBmpHdr = sizeof(BITMAPINFO);
  m_bmphdr.bmiHeader.biBitCount = 24;
  m_bmphdr.bmiHeader.biClrImportant = 0;
  m_bmphdr.bmiHeader.biSize = dwBmpHdr;
  m_bmphdr.bmiHeader.biSizeImage = 0;
  m_bmphdr.bmiHeader.biWidth = *width;
  //注意BMP在y方向是反着存储的，一次必须设置一个负值，才能使图像正着显示出来
  m_bmphdr.bmiHeader.biHeight = -(*height);
  m_bmphdr.bmiHeader.biXPelsPerMeter = 0;
  m_bmphdr.bmiHeader.biYPelsPerMeter = 0;
  m_bmphdr.bmiHeader.biClrUsed = 0;
  m_bmphdr.bmiHeader.biPlanes = 1;
  m_bmphdr.bmiHeader.biCompression = BI_RGB;
  ::SetStretchBltMode(cur_hdc_, HALFTONE);
  int nResult = ::StretchDIBits(cur_hdc_, top_, left_, width_, height_, 0, 0, *width, *height, bmp_data, &m_bmphdr, DIB_RGB_COLORS, SRCCOPY);

  /*if (NULL == hwnd_) {
  ::ReleaseDC((HWND)hwnd_, hdc);
  } else {
  YSOS_LOG_ERROR("############### DisplayDriver::Write hdc is null");
  }*/
  //*/ add for linux
  YSOS_LOG_DEBUG("############### DisplayDriver::Write end");

  return YSOS_ERROR_SUCCESS;
}

int DisplayDriver::SetProperty(int type_id, void *type) {
  int ret = YSOS_ERROR_SUCCESS;
  //* add for linux
  switch (type_id) {
  case PROP_WIDTH: {
    int *width = static_cast<int*>(type);
    if (NULL == width) {
      ret = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }
    if (0 != *width) {
      width_ = *width;
    }
    break;
  }
  case PROP_HEIGHT: {
    int *height = static_cast<int*>(type);
    if (NULL == height) {
      ret = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }
    if (0 != *height) {
      height_ = *height;
    }
    break;
  }
  case PROP_LEFT: {
    int *left = static_cast<int*>(type);
    if (NULL == left) {
      ret = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }
    if (0 != *left) {
      left_ = *left;
    }
    break;
  }
  case PROP_TOP: {
    int *top = static_cast<int*>(type);
    if (NULL == top) {
      ret = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }
    if (0 != *top) {
      top_ = *top;
    }
    break;
  }
  case PROP_MIRROR_MODE: {
    bool *mirror_mode = static_cast<bool*>(type);
    if (NULL == mirror_mode) {
      ret = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }
    is_mirror_mode_ = *mirror_mode;
    break;
  }
  default:
    ret = BaseInterfaceImpl::SetProperty(type_id, type);
  }
  //*/add for linux
  return ret;
}

int DisplayDriver::Ioctl(int iCtrlID, BufferInterfacePtr pInputBuffer, BufferInterfacePtr pOutputBuffer) {
  return YSOS_ERROR_SUCCESS;
}

/* add for linux
///////////////////////////////////////////////////////////////////////////////////////////
static BOOL GetWindowWnd(HWND hWnd, LPARAM lParam) {
  char WindowTitle[100]= {0};
  ::GetWindowTextA(hWnd,WindowTitle,100);
  const char *windows_title = (const char*)lParam;
  if (NULL == lParam) {
    return TRUE;
  }

  if (0 == strlen(WindowTitle)) {
    return TRUE;
  }

  if (0 == strcmp(WindowTitle, windows_title)) {
    g_hwnd_ = hWnd;
    return TRUE;
  }

  return TRUE;
}

static BOOL CALLBACK EnumChildWindowsProc(HWND hWnd,LPARAM lParam) {
  GetWindowWnd(hWnd, lParam);

  return NULL==g_hwnd_;
}

static BOOL CALLBACK EnumWindowsProc(HWND hWnd,LPARAM lParam) {
  BOOL ret = TRUE;
  if (IsWindowVisible(hWnd)) {
    ret = GetWindowWnd(hWnd, lParam);
    if (NULL != g_hwnd_) return TRUE;

    ret = EnumChildWindows(hWnd,EnumChildWindowsProc,lParam); //获取父窗口的所有子窗口
  }

  return TRUE;
}
/////////////////////////////////////////////////////////////////////////
void DisplayDriver::GetHwndByName(const std::string name) {
  //if (NULL != hwnd_) {
  //  return;
  //}/

  if (name.empty()) {
    return;
  }

  EnumWindows(EnumWindowsProc, (LPARAM)name.c_str());
 // hwnd_ = g_hwnd_;
}*/

int DisplayDriver::Open(void *pParams) {
  YSOS_LOG_DEBUG("in open");
  if (NULL == pParams) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }
  /* add for linux
  std::string *name = static_cast<std::string*>(pParams);
  title_ = *name;
  GetHwndByName(*name);
  */
  return YSOS_ERROR_SUCCESS;
}

void DisplayDriver::Close(void *pParams) {
}
}  /// end of namespace ysos
