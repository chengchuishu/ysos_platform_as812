/*
# winsvcwrapper.h
# Definition of winsvcwrapper
# Created on: 2017-03-14 10:43:49
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20170314, created by JinChengZhe
*/

/*************************************** add no used for linux  ************************************/

#ifndef SPS_WIN_SVC_WRAPPER_H_
#define SPS_WIN_SVC_WRAPPER_H_

/// ThirdParty Headers
#include <sys_daemon_package/externcommon.h>
using namespace ysos::extern_common;

namespace ysos {

namespace win_svc_wrapper {
//
// MessageId: SVC_ERROR
//
// MessageText:
//
// An error has occurred (%2).
//
#define SVC_ERROR                        ((DWORD)0xC0020001L)

typedef VOID (CALLBACK *SVCDoJobCallBackFuncPtr)(const long shutdown_flag);

/**
 *@brief CWinSvcWrapper的具体实现，封装了Windows平台的服务程序 // NOLINT
 */
class WinSvcWrapper;
typedef std::tr1::shared_ptr<WinSvcWrapper> CWinSvcWrapperPtr;
class WinSvcWrapper {
  YSOS_DECLARE_SINGLETON(WinSvcWrapper)

 public:
  /**
  *@brief 析构函数  // NOLINT
  *@param 无  // NOLINT
  *@return： 无  // NOLINT
  */
  ~WinSvcWrapper();
  /**
  *@brief 打印使用方法  // NOLINT
  *@param 无  // NOLINT
  *@return： 无  // NOLINT
  */
  static VOID PrintUsage2Console();
  /**
  *@brief 安装服务  // NOLINT
  *@param service_name_ptr[Input]： 在任务管理器"名称"标签上显示名称  // NOLINT
  *@param display_name_ptr[Input]： 在服务"名称"标签上显示名称  // NOLINT
  *@param description_ptr[Input]： 在"描述"标签上显示名称  // NOLINT
  *@return： 成功返回TRUE，失败返回FALSE  // NOLINT
  */
  static BOOL Install(LPCTSTR service_name_ptr, LPCTSTR display_name_ptr, LPCTSTR description_ptr, LPCTSTR module_file_path_ptr);
  /**
  *@brief 卸载服务  // NOLINT
  *@param service_name_ptr[Input]： 在任务管理器"名称"标签上显示名称  // NOLINTNOLINT
  *@return： 成功返回TRUE，失败返回FALSE  // NOLINT
  */
  static BOOL Uninstall(LPCTSTR service_name_ptr);
  /**
  *@brief 开始服务  // NOLINT
  *@param service_name_ptr[Input]： 在任务管理器"名称"标签上显示名称  // NOLINTNOLINT
  *@return： 成功返回TRUE，失败返回FALSE  // NOLINT
  */
  static BOOL Start(LPCTSTR service_name_ptr);
  /**
  *@brief 终止服务  // NOLINT
  *@param service_name_ptr[Input]： 在任务管理器"名称"标签上显示名称  // NOLINT
  *@return： 成功返回TRUE，失败返回FALSE  // NOLINT
  */
  static BOOL Stop(LPCTSTR service_name_ptr);
  /**
  *@brief 安装服务  // NOLINT
  *@param 无
  *@return： 成功返回TRUE，失败返回FALSE  // NOLINT
  */
  BOOL Install();
  /**
  *@brief 删除服务  // NOLINT
  *@param 无
  *@return： 成功返回TRUE，失败返回FALSE  // NOLINT
  */
  BOOL Uninstall();
  /**
  *@brief 运行服务  // NOLINT
  *@param 无  // NOLINT
  *@return： 成功返回TRUE，失败返回FALSE  // NOLINT
  */
  BOOL Start();
  /**
  *@brief 运行服务  // NOLINT
  *@param 无  // NOLINT
  *@return： 成功返回TRUE，失败返回FALSE  // NOLINT
  */
  BOOL Stop();
  /**
  *@brief 运行服务  // NOLINT
  *@param 无  // NOLINT
  *@return： 成功返回TRUE，失败返回FALSE  // NOLINT
  */
  BOOL Run();
  /**
  *@brief 初始化参数  // NOLINT
  *@param service_name_ptr[Input]： 在任务管理器"名称"标签上显示名称  // NOLINT
  *@param display_name_ptr[Input]： 在服务"名称"标签上显示名称  // NOLINT
  *@param description_ptr[Input]： 在"描述"标签上显示名称  // NOLINT
  *@param do_job_callback_func_ptr[Input]： 需要执行的回调函数指针  // NOLINT
  *@param perform_work_interval_time[Input]：设置回调函数延迟时间  // NOLINT
  *@return： 成功返回单例指针，失败返回NULL  // NOLINT
  */
  void Init(LPCTSTR service_name_ptr, LPCTSTR display_name_ptr, LPCTSTR description_ptr, SVCDoJobCallBackFuncPtr do_job_callback_func_ptr, const long perform_work_interval_time = 1000);

 private:
  WinSvcWrapper(const WinSvcWrapper&);
  WinSvcWrapper& operator= (const WinSvcWrapper&);
  WinSvcWrapper();

  static VOID WINAPI SvcMain(DWORD argc, LPTSTR *argv_ptr);
  static VOID WINAPI SvcCtrlHandler(DWORD ctrol_code);

  VOID RealSvcMain(DWORD argc, LPTSTR *argv_ptr);
  VOID RealSvcCtrlHandler(DWORD ctrol_code);
  VOID ReportSvcStatus(DWORD current_state, DWORD win32_exit_code, DWORD wait_hint);
  VOID SvcReportEvent(LPTSTR function_name_ptr);
  static VOID WINAPI ThreadProc(PVOID param_ptr);

  volatile long shutdown_flag_;
  TCHAR service_name_[MAX_PATH];
  TCHAR display_name_[MAX_PATH];
  TCHAR description_[MAX_PATH];

  SERVICE_TABLE_ENTRY service_table_entry_[2];

  SERVICE_STATUS_HANDLE service_status_handle_;
  SERVICE_STATUS service_status_;
  HANDLE service_stop_event_handle_;

  long perform_work_interval_time_;
  SVCDoJobCallBackFuncPtr do_job_callback_func_ptr_;
};

}

}

#define GetWinSvcWrapper ysos::win_svc_wrapper::WinSvcWrapper::GetInstancePtr

#endif


*************************************** add no used for linux  ************************************/