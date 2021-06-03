/**
 *@file WindowControl.h
 *@brief Definition of WindowControl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef OHP_WINDOW_CONTROL_H_
#define OHP_WINDOW_CONTROL_H_

/// ysos private headers //  NOLINT
#include "../../../public/include/sys_interface_package/common.h"
/// c headers //  NOLINT
#ifdef WIN32
#include <Windows.h>
#endif
/// stl headers //  NOLINT
#include <string>
/// boost headers //  NOLINT
#include <boost/shared_ptr.hpp>

namespace ysos {
/**
 *@brief 窗口位置参数结构
 */
struct WindowRect {
  int left;  ///< 左上角X位置
  int top;   ///<  左上角Y位置 //  NOLINT
  int width; ///<  窗口宽度 //  NOLINT
  int height;  ///< 窗口高度

  WindowRect() {
    left = top = width = height = 0;
  }
};

/**
 *@brief 窗口控制操作类，实现了最前显示、隐藏和窗口裁剪
 */
class YSOS_EXPORT WindowControl { /*: public boost::enable_shared_from_this<WindowControl>*/
 public:

#ifdef WIN32
  typedef HWND           WindowHandleType;
  typedef LPARAM         LongParam;
  typedef DWORD          ProcessIdType;
#endif

  WindowControl();
  ~WindowControl(void);

  /**
    *@brief  初始化ShowWindows参数 //  NOLINT
    *@param  windows_name 窗体的标题//  NOLINT
    *@param  rec 窗体的显示坐标//  NOLINT
    *@return 无  //  NOLINT
    */
  void Init(const std::string &windows_name, WindowRect &rec, bool is_fuzzy = false);
  /**
    *@brief  通过当前的进程号查找主窗口 //  NOLINT
    *@param  is_parent true 获取父进程窗口的控制权， false 获取当前进程的窗口控制权//  NOLINT
    *@return 无  //  NOLINT
    */
  void Init(bool is_parent=true);

  /**
    *@brief  指定的窗体在最前端显示 //  NOLINT
    *@return true 成功， false 失败  //  NOLINT
    */
  bool Show(bool is_parent=true);
  /**
    *@brief  指定的窗体隐藏在其他窗体后面 //  NOLINT
    *@return true 成功， false 失败  //  NOLINT
    */
  bool Hide(bool is_parent=true);
  /**
    *@brief  控制当前进程的窗体，只显示Client区域内容 //  NOLINT
    *@return true 成功， false 失败  //  NOLINT
    */
  bool ClipWindow(bool is_parent=true);

 protected:
  /**
    *@brief  指定的窗体在最前端显示 //  NOLINT
    *@return true 成功， false 失败  //  NOLINT
    */
  bool Show(WindowControl::WindowHandleType window_handle);
  /**
    *@brief  指定的窗体隐藏在其他窗体后面 //  NOLINT
    *@return true 成功， false 失败  //  NOLINT
    */
  bool Hide(WindowControl::WindowHandleType window_handle);

 public:
  int EnumWindowsProc(WindowControl::WindowHandleType window_handle,WindowControl::LongParam long_param);
  int EnumChildWindowsProc(WindowControl::WindowHandleType window_handle,WindowControl::LongParam long_param);

 protected:
  void FindWindows(bool is_process=false, bool is_parent=true);
  ProcessIdType GetParentProcessID(void);
  ProcessIdType GetProcessIdByHandle(WindowHandleType window_handle);
  int GetWindowWnd(WindowHandleType window_handle);



 private:
  std::string windows_name_;  ///< 要查找的窗口的标题，如果标题为空，则通过进程ID查找
  WindowRect  rect_;         ///<  待控制窗口的大小，如果值为0，不改变窗口大小和位置 //  NOLINT
  bool        is_process_;  ///< 是否是子进程
  bool        is_fuzzy_;  ///<  是否模糊匹配 //  NOLINT
  ProcessIdType    parent_process_id_;  ///<  父进程的ID //  NOLINT
  ProcessIdType    cur_process_id_;     ///< 当前进程的ID
  WindowHandleType parent_window_handle_;  ///< 父进程的窗口Handle
  WindowHandleType child_window_handle_;   ///< 当前进程的窗口Handle
  WindowHandleType window_handle_;         ///< 通过窗口标题获取到的窗口Handle
};
}

#endif
