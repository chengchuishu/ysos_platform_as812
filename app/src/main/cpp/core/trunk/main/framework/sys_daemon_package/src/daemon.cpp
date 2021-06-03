/*
# daemon.cpp
# Definition of Daemon
# Created on: 2017-03-20 15:53:55
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20170320, created by JinChengZhe
*/

/// Private Headers
#include "../../../protect/include/sys_daemon_package/daemon.h"
/// ThirdParty Headers
#include <boost/function.hpp>  /// NOLINT
/// Platform Headers
#include "../../../public/include/os_hal_package/filepath.h"
#include "../../../public/include/core_help_package/xmlutil.h"
#include "../../../public/include/core_help_package/utility.h"
#include "../../../public/include/sys_interface_package/ysossdkinterface.h"
#include "../../../public/include/sys_interface_package/messagetype.h"
#include "../../../protect/include/sys_framework_package/platformrpcentity.h"
#include "../../../protect/include/sys_daemon_package/externutility.h"
#include "../../../public/include/core_help_package/jsonutility.h"
#include "../../../protect/include/sys_daemon_package/processwrapper.h"
#include "../../../protect/include/sys_daemon_package/winpipeclient.h"
/// Windows Platform Headers
#if _WIN32
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include <atlstr.h>
#endif

using namespace ysos::extern_utility;
using namespace ysos::win_pipe;

namespace ysos {

namespace sys_daemon_package {

log4cplus::Logger logger_;  ///< 日志对象

static const std::string g_start_command = "Start";
static const std::string g_stop_command = "Stop";
#ifdef _DEBUG
static const char* g_ysos_win_name = "ysos_d.exe";
static const char* g_ysos_console_name = "system_manager_d.exe";
static const char* g_ysos_update_name = "ysos_update_d.exe";
static const char* g_ysos_service_name = "ysos_service_d.exe";
#else
static const char* g_ysos_win_name = "ysos.exe";
static const char* g_ysos_console_name = "system_manager.exe";
static const char* g_ysos_update_name = "ysos_update.exe";
static const char* g_ysos_service_name = "ysos_service.exe";
#endif

/**
*@brief ConnectionListener的具体实现  // NOLINT
*/
static const char* g_app_name = "ysos_daemon";
static const char* g_log_file_specific_keyword = "${ysos_log_path}$";
static const char* g_log_file_sub_file_path = "/conf/log/log";
static const char* g_log_file_template_extension = ".properties.template";
static const char* g_log_file_name = "log.properties";
static const char* g_log_folder_name = "/YSOS_DAEMON_LOG";
static const char* g_log_folder_sub_name = "/log";
static const unsigned int g_buffer_size = 8192;
static const char* g_cmd_check_new_version = "check_new_version_service";
static const char* g_cmd_get_current_version = "get_current_version_service";
static const char* g_cmd_get_all_available_version = "get_all_available_version_service";
static const char* g_cmd_update = "update_service";
static const char* g_cmd_kill_process = "kill_process_service";
static const char* g_cmd_prepare_update_complete = "prepare_update_complete_service";
static const char* g_cmd_exit_process = "ExitProcess";

int g_ysos_state = 0;

void YSOS_Run_State(int iYSOSState)
{
  g_ysos_state = iYSOSState;
  if(logger_.valid()) YSOS_LOG_DEBUG("YSOS_Run_State:"<< g_ysos_state);
}

//add for linux 
int GetModuleFileName( char* sModuleName, char* sFileName, int nSize)
{
 int ret = -1;
 //* one way
 char szBuf[nSize];
 int nCount;
 
 memset(szBuf, 0, sizeof(szBuf));
 nCount=readlink("/proc/self/exe", szBuf, nSize);
 if(nCount<0 || nCount>=nSize)
 {
    printf("failed\n");
 } else {
    ret = nCount;
 }
 szBuf[nCount]='\0';
 //printf("/proc/self/exe->[%s]\n", szBuf);
 //*/
 strncpy(sFileName,szBuf,nSize);
 return ret;
}

bool PathRemoveFileSpec (char* sMoudlePath)
{
    char* path;
    int MAX_PATH = 260;
    path = (char*)malloc(MAX_PATH + 1);
    char* p = strrchr(sMoudlePath,'/');
    ++p;
    char* it = sMoudlePath;
    int i = 0;
    for(i = 0; it < p-1; ++i, ++it)
        path[i] = *it;
    path[i] = '\0';
    //printf("..\\PathRemoveFileSpec() is :[%s]\n",path);
    strncpy(sMoudlePath,path,MAX_PATH);
    return true;
}

bool PathAppend(char* fpath, const char* tfilename)
{
    int MAX_PATH = 260;
    char fullpath[MAX_PATH];
    sprintf(fullpath,"%s%s", fpath, tfilename);
    strncpy(fpath,fullpath, MAX_PATH);
    return true;
}

bool PathAddExtension(char* fpath, const char* tfilename)
{
    int MAX_PATH = 260;
    char fullpath[MAX_PATH];
    sprintf(fullpath,"%s%s", fpath, tfilename);
    strncpy(fpath,fullpath, MAX_PATH);
    return true;
}

bool PathFileExists(const std::string& tfpath)
{
    bool ret = false;
    ret = boost::filesystem::exists(tfpath.c_str());
    return ret;
}

void PathRemoveExtension(char* tmpfpath)
{
    boost::filesystem::path path = tmpfpath; //add for linux
    strcpy(tmpfpath, path.replace_extension("").c_str());
    return;
}

bool CopyFile(const std::string& fromfpath, char* tofpath, boost::filesystem::copy_option coption, boost::system::error_code &ec)
{
    //TODO:can change other params for copy
    //boost::filesystem::copy_option option;
    //boost::system::error_code ec;
    boost::filesystem::copy_file ( fromfpath, tofpath, coption, ec);//add for linux  
    return true;
}

bool CreateDirectory(std::string _fpath, char* nopara)
{
    const char* path = _fpath.c_str();
    boost::filesystem::path dir(path);
    if(boost::filesystem::create_directory(dir))
    {
      printf("Directory Created:[%s]\n", _fpath.c_str());
    } else {
      return false;
    }
    return true;
}

int GetTempPath(int size, std::string &tmppath) 
{
  //char szTempFile[] = "/tmp/daemon";
  //char *pszTempFile = mktemp(szTempFile);
  tmppath = "/tmp";
  return tmppath.size();
}

//************************************
// Method:    string_replace
// FullName:  string_replace
// Access:    public 
// Returns:   void
// Qualifier: 把字符串的strsrc替换成strdst
// Parameter: std::string & strBig
// Parameter: const std::string & strsrc
// Parameter: const std::string & strdst
//************************************
void string_replace( std::string &strBig, const std::string &strsrc, const std::string &strdst)
{
    std::string::size_type pos = 0;
    std::string::size_type srclen = strsrc.size();
    std::string::size_type dstlen = strdst.size();

    while( (pos=strBig.find(strsrc, pos)) != std::string::npos )
    {
        strBig.replace( pos, srclen, strdst );
        pos += dstlen;
    }
}

//end add for linux

ConnectionListener* ConnectionListener::this_ptr_ = NULL;
ConnectionListener::ConnectionListener(const MediatorPtr& mediator_ptr) :
  Colleague(mediator_ptr),
  running_status_(IS_INITIAL) {
  this_ptr_ = this;
}

ConnectionListener::~ConnectionListener(void) {

}

int ConnectionListener::Send(const ExternMessage& message) {
  mediator_ptr_->Send(message, shared_from_this());
  return YSOS_ERROR_SUCCESS;
}

int ConnectionListener::Notify(const ExternMessage& message) {
  printf("Daemon::ConnectionListener::Notify[Enter]\n");
  YSOS_LOG_DEBUG("ConnectionListener::Notify[Enter]");
  int result = YSOS_ERROR_FAILED;

  do {
    switch (message.id_) {
    case Daemon::MSG_START: {
      printf("Daemon::ConnectionListener::Notify[message.id_][MSG_START]\n");
      YSOS_LOG_DEBUG("ConnectionListener::Notify[message.id_][MSG_START]");
      /// 开启模块
      if (YSOS_ERROR_SUCCESS != Start()) {
        printf("Daemon::ConnectionListener::Notify[Fail]The function[Start()] is failed\n");
        YSOS_LOG_DEBUG("ConnectionListener::Notify[Fail]The function[Start()] is failed");
        result = YSOS_ERROR_LOGIC_ERROR;
      }
      result = YSOS_ERROR_SUCCESS;
    }
    break;
    case Daemon::MSG_STOP: {
      /// 关闭模块
      printf("Daemon::ConnectionListener::Notify[message.id_][MSG_STOP]\n");
      YSOS_LOG_DEBUG("ConnectionListener::Notify[message.id_][MSG_STOP]");
      if (YSOS_ERROR_SUCCESS != Stop()) {
        printf("Daemon::ConnectionListener::Notify[Fail]The function[Stop()] is failed\n");
        YSOS_LOG_DEBUG("ConnectionListener::Notify[Fail]The function[Stop()] is failed");
        result = YSOS_ERROR_LOGIC_ERROR;
      }
      result = YSOS_ERROR_SUCCESS;
    }
    break;
    default:
      printf("Daemon::ConnectionListener::Notify[message.id_][%d]\n", message.id_);
      YSOS_LOG_DEBUG("ConnectionListener::Notify[message.id_]["<< message.id_ <<"]");
      result = YSOS_ERROR_NOT_SUPPORTED;
      break;
    }

    if (YSOS_ERROR_SUCCESS != result) {
      printf("Daemon::ConnectionListener::Notify[Fail]\n");
      YSOS_LOG_DEBUG("ConnectionListener::Notify[Fail]");
      break;
    }

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  printf("Daemon::ConnectionListener::Notify[Exit]\n");
  YSOS_LOG_DEBUG("ConnectionListener::Notify[Exit]");
  return result;
}

void ConnectionListener::UserHandleDoService(const char* session_id, const char* service_name, const char* service_param) {
  printf("Daemon::ConnectionListener::UserHandleDoService[Enter]\n");
  YSOS_LOG_DEBUG("ConnectionListener::UserHandleDoService[Enter]");
  int result = YSOS_ERROR_SUCCESS;

  do {
    if (NULL != session_id) {
      printf("Daemon::session_id[%s]\n", session_id);
      YSOS_LOG_DEBUG("ConnectionListener::UserHandleDoService[session_id]["<< session_id <<"]");
    }

    if (NULL != service_name) {
      printf("Daemon::service_name[%s]\n", service_name);
      YSOS_LOG_DEBUG("ConnectionListener::UserHandleDoService[service_name]["<< service_name <<"]");
    }

    if (NULL != service_param) {
      printf("Daemon::service_param[%s]\n", service_param);
      YSOS_LOG_DEBUG("ConnectionListener::UserHandleDoService[service_param]["<< service_param <<"]");
    }

    std::string duplicated_session_id(session_id), duplicated_service_name(service_name), duplicated_service_param(service_param);

    printf("Daemon::ConnectionListener::UserHandleDoService[Check Point][0]\n");
    YSOS_LOG_DEBUG("ConnectionListener::UserHandleDoService[Check Point][0]");

    /// 判断session id是否为空
    if (true == duplicated_session_id.empty()) {
      printf("Daemon::ConnectionListener::UserHandleDoService[Fail]duplicated_session_id is empty\n");
      YSOS_LOG_DEBUG("ConnectionListener::UserHandleDoService[Fail]duplicated_session_id is empty");
      result = YSOS_ERROR_LOGIC_ERROR;
      break;
    }

    printf("Daemon::ConnectionListener::UserHandleDoService[Check Point][1]\n");
    YSOS_LOG_DEBUG("ConnectionListener::UserHandleDoService[Check Point][1]");

    /// 初始化服务请求处理
    if (YSOS_AGENT_SERVICE_NAME_INIT == duplicated_service_name) {
      printf("Daemon::Handle YSOS_AGENT_SERVICE_NAME_INIT Service[Start]\n");
      YSOS_LOG_DEBUG("Handle YSOS_AGENT_SERVICE_NAME_INIT Service[Start]");

      /// 解析参数，判断进程类型
      if (true == duplicated_service_param.empty()) {
        printf("Daemon::ConnectionListener::UserHandleDoService[Fail]duplicated_service_param is empty\n");
        YSOS_LOG_DEBUG("ConnectionListener::UserHandleDoService[Fail]duplicated_service_param is empty");
        result = YSOS_ERROR_LOGIC_ERROR;
        break;
      }

      printf("Daemon::ConnectionListener::UserHandleDoService[Check Point][2]\n");
      YSOS_LOG_DEBUG("ConnectionListener::UserHandleDoService[Check Point][2]");

      /// 使用初始化消息结构体做解析
      ReqInitServiceParam req_service_param;
      if (true != req_service_param.FromString(duplicated_service_param)) {
        printf("Daemon::ConnectionListener::UserHandleDoService[Fail]The function[req_service_param.FromString()] is failed\n");
        YSOS_LOG_DEBUG("ConnectionListener::UserHandleDoService[Fail]The function[req_service_param.FromString()] is failed");
        result = YSOS_ERROR_LOGIC_ERROR;
        break;
      }

      printf("Daemon::ConnectionListener::UserHandleDoService[Check Point][3]\n");
      YSOS_LOG_DEBUG("ConnectionListener::UserHandleDoService[Check Point][3]");

      /// 保存session id
      {
        /// 加锁访问数据
        BoostSharedLock lock(this_ptr_->shared_mutex_);
        //this_ptr_->app_session_id_map_.insert(std::make_pair<std::string, std::string>(duplicated_session_id, req_service_param.app_name));
        this_ptr_->app_session_id_map_.insert(std::make_pair(duplicated_session_id, req_service_param.app_name));// add for linux  使用 std :: make_pair 你不应该明确指定模板参数
      }

      printf("Daemon::Check App Name\n");
      YSOS_LOG_DEBUG("Check App Name");

      /// 平台windows进程请求连接
      if (g_ysos_win_name == req_service_param.app_name) {
        printf("Daemon::~~~[%s] is connected\n", req_service_param.app_name.c_str());
        YSOS_LOG_DEBUG("~~~[" << req_service_param.app_name << "] is connected");

        /// 通知监测进程类，启动TDRobotMain进程
        if (YSOS_ERROR_SUCCESS != this_ptr_->Send(ExternMessage(Daemon::MSG_YSOS_IS_STARTED))) {
          printf("Daemon::ConnectionListener::UserHandleDoService[Fail]The function[Send()] is failed\n");
          YSOS_LOG_DEBUG("ConnectionListener::UserHandleDoService[Fail]The function[Send()] is failed");
          result = YSOS_ERROR_LOGIC_ERROR;
          break;
        }
      }
      /// 平台console进程请求连接
      else if (g_ysos_console_name == req_service_param.app_name) {
        printf("Daemon::~~~[%s] is connected\n", req_service_param.app_name.c_str());
        YSOS_LOG_DEBUG("~~~[" << req_service_param.app_name << "] is connected");

        /// 通知监测进程类，启动TDRobotMain进程
        if (YSOS_ERROR_SUCCESS != this_ptr_->Send(ExternMessage(Daemon::MSG_YSOS_IS_STARTED))) {
          printf("Daemon::ConnectionListener::UserHandleDoService[Fail]The function[Send()] is failed\n");
          YSOS_LOG_DEBUG("ConnectionListener::UserHandleDoService[Fail]The function[Send()] is failed");
          result = YSOS_ERROR_LOGIC_ERROR;
          break;
        }
      }
      /// 平台更新进程请求连接
      else if (g_ysos_update_name == req_service_param.app_name) {
        printf("Daemon::!!![%s] is connected\n", req_service_param.app_name.c_str());
        YSOS_LOG_DEBUG("!!![" << req_service_param.app_name << "] is connected");
      } else {
        printf("Daemon::@@@[%s] is connected. It is not supported.\n", req_service_param.app_name.c_str());
        YSOS_LOG_DEBUG("@@@[" << req_service_param.app_name << "] is connected. It is not supported.");
        result = YSOS_ERROR_NOT_SUPPORTED;
        break;
      }

      printf("Daemon::ConnectionListener::UserHandleDoService[Check Point][4]\n");
      YSOS_LOG_DEBUG("ConnectionListener::UserHandleDoService[Check Point][4]");

      /// 封装返回报文
      ResInitServiceParam handle_init_resp;
      handle_init_resp.status_code = GetUtility()->ConvertToString(0);
      handle_init_resp.description = "succeeded";
      std::string handle_init_resp_string;
      if (true != handle_init_resp.ToString(handle_init_resp_string)) {
        printf("Daemon::ConnectionListener::UserHandleDoService[Fail]session_id[%s]\n", session_id);
        YSOS_LOG_DEBUG("ConnectionListener::UserHandleDoService[Fail]session_id["<< session_id <<"]");
        result = YSOS_ERROR_LOGIC_ERROR;
        break;
      }

      printf("Daemon::ConnectionListener::UserHandleDoService[handle_init_resp_string][%s]\n", handle_init_resp_string.c_str());
      YSOS_LOG_DEBUG("ConnectionListener::UserHandleDoService[handle_init_resp_string]["<< handle_init_resp_string <<"]");

      /// 返回init结果
      if (YSOS_ERROR_SUCCESS != ysos::ysos_sdk::YSOSSDKOnDispatchMessage(session_id, service_name, handle_init_resp_string.c_str())) {
        printf("Daemon::ConnectionListener::UserHandleDoService[Fail]The function[ysos::ysos_sdk::YSOSSDKOnDispatchMessage()] is failed. session_id[%s]\n", session_id);
        YSOS_LOG_DEBUG("ConnectionListener::UserHandleDoService[Fail]The function[ysos::ysos_sdk::YSOSSDKOnDispatchMessage()] is failed. session_id["<< session_id <<"]");
        result = YSOS_ERROR_LOGIC_ERROR;
        break;
      }

      printf("Daemon::Handle YSOS_AGENT_SERVICE_NAME_INIT Service[End]\n");
      YSOS_LOG_DEBUG("Handle YSOS_AGENT_SERVICE_NAME_INIT Service[End]");
    }
    /// 其他服务请求处理
    else {
      printf("Daemon::Handle Common Service[Start]\n");
      YSOS_LOG_DEBUG("Handle Common Service[Start]");

      /// 根据session id调用平台进程/更新进程请求逻辑处理函数
      std::string app_name = this_ptr_->GetAppName(duplicated_session_id);
      /// 调平台进程请求逻辑处理函数
      if (app_name == g_ysos_win_name ||
          app_name == g_ysos_console_name) {
        if (YSOS_ERROR_SUCCESS != this_ptr_->HandleDoService4YSOS(duplicated_session_id, duplicated_service_name, duplicated_service_param)) {
          printf("Daemon::ConnectionListener::UserHandleDoService[Fail][The function[this_ptr_->HandleDoService4YSOS()] is failed]\n");
          YSOS_LOG_DEBUG("ConnectionListener::UserHandleDoService[Fail][The function[this_ptr_->HandleDoService4YSOS()] is failed]");
          result = YSOS_ERROR_LOGIC_ERROR;
          break;
        }
      }
      /// 调更新进程请求逻辑处理函数
      else if (app_name == g_ysos_update_name) {
        if (YSOS_ERROR_SUCCESS != this_ptr_->HandleDoService4Update(duplicated_session_id, duplicated_service_name, duplicated_service_param)) {
          printf("Daemon::ConnectionListener::UserHandleDoService[Fail][The function[this_ptr_->HandleDoService4Update()] is failed]\n");
          YSOS_LOG_DEBUG("ConnectionListener::UserHandleDoService[Fail][The function[this_ptr_->HandleDoService4Update()] is failed]");
          result = YSOS_ERROR_LOGIC_ERROR;
          break;
        }
      }
      /// 应用名为空或者没找到session id对应的应用名
      else {
        printf("Daemon::ConnectionListener::UserHandleDoService[Fail][It can't find app name[%s]]\n", app_name.c_str());
        YSOS_LOG_DEBUG("ConnectionListener::UserHandleDoService[Fail][It can't find app name]["<< app_name <<"]");
        result = YSOS_ERROR_LOGIC_ERROR;
        break;
      }

      printf("Daemon::Handle Common Service[End]\n");
      YSOS_LOG_DEBUG("Handle Common Service[End]");
    }

    printf("Daemon::ConnectionListener::UserHandleDoService[Check Point][End]\n");
    YSOS_LOG_DEBUG("ConnectionListener::UserHandleDoService[Check Point][End]");
  } while (0);

  if (YSOS_ERROR_SUCCESS != result) {
    printf("Daemon::ConnectionListener::UserHandleDoService[Error]\n");
    YSOS_LOG_DEBUG("ConnectionListener::UserHandleDoService[Error]");
  }

  printf("Daemon::ConnectionListener::UserHandleDoService[Exit]\n");
  YSOS_LOG_DEBUG("ConnectionListener::UserHandleDoService[Exit]");
  return;
}

std::string ConnectionListener::GetAppName(const std::string& session_id) {
  if (true == session_id.empty()) {
    return std::string("");
  }

  if (0 == app_session_id_map_.size()) {
    return std::string("");
  }

  AppSessionIDMapItr itr = app_session_id_map_.find(session_id);
  if (itr != app_session_id_map_.end()) {
    /// 找到session id对应的应用名
    return std::string(itr->second);
  }

  return std::string("");
}

std::string ConnectionListener::GetAppSessionID(const std::string& app_name) {
  if (true == app_name.empty()) {
    return std::string("");
  }

  if (0 == app_session_id_map_.size()) {
    return std::string("");
  }

  AppSessionIDMapItr itr = std::find_if(app_session_id_map_.begin(), app_session_id_map_.end(), AppSessionIDMapValueFinder(app_name));
  if (itr != app_session_id_map_.end()) {
    /// 找到应用名对应的session id
    return std::string(itr->first);
  }

  return std::string("");
}

int ConnectionListener::HandleDoService4YSOS(const std::string& session_id, const std::string& service_name, const std::string& service_param) {
  printf("Daemon::ConnectionListener::HandleDoService4YSOS[Enter]\n");
  YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Enter]");
  int result = YSOS_ERROR_SUCCESS;

  do {
    if (true == session_id.empty() ||
        true == service_name.empty()) {
      printf("Daemon::ConnectionListener::HandleDoService4YSOS[Fail][session_id/service_name is empty]\n");
      YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Fail][session_id/service_name is empty]");
      result = YSOS_ERROR_LOGIC_ERROR;
      break;
    }

    printf("Daemon::ConnectionListener::HandleDoService4YSOS[Check Point][0]\n");
    YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Check Point][0]");

    /// 自定义服务请求
    if (YSOS_AGENT_SERVICE_NAME_CUSTOM_EVENT == service_name) {
      printf("Daemon::ConnectionListener::HandleDoService4YSOS[Check Point][YSOS_AGENT_SERVICE_NAME_CUSTOM_EVENT][Enter]\n");
      if (true == service_param.empty()) {
        printf("Daemon::ConnectionListener::HandleDoService4YSOS[Fail][service_param is empty]\n");
        YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Fail][service_param is empty]");
        result = YSOS_ERROR_LOGIC_ERROR;
        break;
      }

      printf("Daemon::ConnectionListener::HandleDoService4YSOS[Check Point][1]\n");
      YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Check Point][1]");

      /// 解析服务参数
      ReqCustomEventServiceParam req_custom_event_service_param;
      if (true != req_custom_event_service_param.FromString(service_param)) {
        printf("Daemon::ConnectionListener::HandleDoService4YSOS[Fail]The function[req_custom_event_service_param.FromString()] is failed\n");
        YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Fail]The function[req_custom_event_service_param.FromString()] is failed");
        result = YSOS_ERROR_LOGIC_ERROR;
        break;
      }

      printf("Daemon::ConnectionListener::HandleDoService4YSOS[Check Point][2]\n");
      YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Check Point][2]");

      /// 根据具体自定义服务名称，进行逻辑分类处理
      if (g_cmd_check_new_version == req_custom_event_service_param.service_name ||
          g_cmd_get_all_available_version == req_custom_event_service_param.service_name ||
          g_cmd_update == req_custom_event_service_param.service_name) {
        printf("Daemon::ConnectionListener::HandleDoService4YSOS[Check Point][g_cmd_check_new_version/g_cmd_get_all_available_version/g_cmd_update]\n");
        YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Check Point][g_cmd_check_new_version/g_cmd_get_all_available_version/g_cmd_update]");
        /// 检测最新版本信息命令/获取所有可用版本信息命令/更新版本命令 转发给更新进程
        if (YSOS_ERROR_SUCCESS != DoService4Update(service_name, service_param)) {
          printf("Daemon::ConnectionListener::HandleDoService4YSOS[Fail]The function[DoService4Update()] is failed\n");
          YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Fail]The function[DoService4Update()] is failed");
          result = YSOS_ERROR_LOGIC_ERROR;
          break;
        }

        /// 拼装返回报文
        boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
        ResCustomEventServiceParam res_custem_event_service_param;
        res_custem_event_service_param.status_code = GetUtility()->ConvertToString(result);
        res_custem_event_service_param.description = (result == YSOS_ERROR_SUCCESS ? "succeeded" : "failed");
        std::string res_custem_event_service_param_string;
        if (true != res_custem_event_service_param.ToString(res_custem_event_service_param_string)) {
          printf("Daemon::ConnectionListener::HandleDoService4YSOS[Fail]session_id[%s]\n", session_id.c_str());
          YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Fail]session_id["<< session_id <<"]");
          result = YSOS_ERROR_LOGIC_ERROR;
          break;
        }

        printf("Daemon::ConnectionListener::HandleDoService4YSOS[res_custem_event_service_param_string][%s]\n", res_custem_event_service_param_string.c_str());
        YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[res_custem_event_service_param_string]["<< res_custem_event_service_param_string <<"]");

        /// 发送给平台进程
        if (YSOS_ERROR_SUCCESS != OnDispatchMessage4YSOS(service_name, res_custem_event_service_param_string)) {
          printf("Daemon::ConnectionListener::HandleDoService4YSOS[Fail]The function[OnDispatchMessage4YSOS()] is failed\n");
          YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Fail]The function[OnDispatchMessage4YSOS()] is failed");
          result = YSOS_ERROR_LOGIC_ERROR;
          break;
        }
      } else if (g_cmd_prepare_update_complete == req_custom_event_service_param.service_name) {
        printf("Daemon::ConnectionListener::HandleDoService4YSOS[Check Point][g_cmd_update_prepare_complete]\n");
        YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Check Point][g_cmd_update_prepare_complete]");

        /// 拼装返回报文
        boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
        ResCustomEventServiceParam res_custem_event_service_param;
        res_custem_event_service_param.status_code = GetUtility()->ConvertToString(result);
        res_custem_event_service_param.description = (result == YSOS_ERROR_SUCCESS ? "succeeded" : "failed");
        std::string res_custem_event_service_param_string;
        if (true != res_custem_event_service_param.ToString(res_custem_event_service_param_string)) {
          printf("Daemon::ConnectionListener::HandleDoService4YSOS[Fail]session_id[%s]\n", session_id.c_str());
          YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Fail]session_id["<< session_id <<"]");
          result = YSOS_ERROR_LOGIC_ERROR;
          break;
        }

        printf("Daemon::ConnectionListener::HandleDoService4YSOS[res_custem_event_service_param_string][%s]\n", res_custem_event_service_param_string.c_str());
        YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[res_custem_event_service_param_string]["<< res_custem_event_service_param_string <<"]");

        /// 发送给平台进程
        if (YSOS_ERROR_SUCCESS != OnDispatchMessage4YSOS(service_name, res_custem_event_service_param_string)) {
          printf("Daemon::ConnectionListener::HandleDoService4YSOS[Fail]The function[OnDispatchMessage4YSOS()] is failed\n");
          YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Fail]The function[OnDispatchMessage4YSOS()] is failed");
          result = YSOS_ERROR_LOGIC_ERROR;
          break;
        }

        printf("Daemon::ConnectionListener::HandleDoService4YSOS[Check Point][3]\n");
        YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Check Point][3]");

        /// 更新准备完成命令 转发给更新进程
        if (YSOS_ERROR_SUCCESS != DoService4Update(service_name, service_param)) {
          printf("Daemon::ConnectionListener::HandleDoService4YSOS[Fail]The function[DoService4Update()] is failed\n");
          YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Fail]The function[DoService4Update()] is failed");
          result = YSOS_ERROR_LOGIC_ERROR;
          break;
        }
      } else if (g_cmd_kill_process == req_custom_event_service_param.service_name) {
        printf("Daemon::ConnectionListener::HandleDoService4YSOS[Check Point][g_cmd_kill_process]\n");
        YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Check Point][g_cmd_kill_process]");
        /// 解析要杀死的进程名，结构为json结构体包数组 {"processes":["ysos.exe", "ysos_update.exe"]}

        ///开启pipe客户端
        if (0 != GetWinPipeClient()->StartA(std::string(g_ysos_service_pipe_server_name), NULL)) {
          printf("Daemon::ConnectionListener::HandleDoService4YSOS[Fail][The function[GetWinPipeClient()->StartA()] is failed]\n");
          YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Fail][The function[GetWinPipeClient()->StartA()] is failed]");
          result = YSOS_ERROR_LOGIC_ERROR;
          break;
        }

        printf("Daemon::ConnectionListener::HandleDoService4YSOS[Check Point][4]\n");
        YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Check Point]43]");

        /// 先发给服务进程去杀死指定进程，因为服务进程权限比较高
        DWORD cmd = WIN_SVC_PIPE_CMD_KILL_PROCESSES;
        std::string msg = req_custom_event_service_param.service_param;
        DWORD reply_cmd_result = 0;
        std::string reply_msg;
        result = GetWinPipeClient()->SendA(cmd, msg, &reply_cmd_result, reply_msg);

        printf("Daemon::ConnectionListener::HandleDoService4YSOS[Check Point][5]\n");
        YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Check Point][5]");

        /// 关闭pipe客户端
        if (0 != GetWinPipeClient()->Stop()) {
          printf("Daemon::ConnectionListener::HandleDoService4YSOS[Fail]The function[GetWinPipeClient()->Stop()] is failed\n");
          YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Fail]The function[GetWinPipeClient()->Stop()] is failed");
          result = YSOS_ERROR_LOGIC_ERROR;
          break;
        }

        printf("Daemon::ConnectionListener::HandleDoService4YSOS[Check Point][6]\n");
        YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Check Point][6]");

        if (0 == result && WinPipeInterface::CMD_RESULT_SUCCESS == reply_cmd_result) {
          printf("Daemon::ConnectionListener::HandleDoService4YSOS[Check Point][reply_cmd_result][%d]\n", (int)reply_cmd_result);
          printf("Daemon::ConnectionListener::HandleDoService4YSOS[Check Point][reply_msg][%s]\n", reply_msg.c_str());
          YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Check Point][reply_cmd_result][" << reply_cmd_result << "]");
          YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Check Point][reply_msg][" << reply_msg << "]");
          result = YSOS_ERROR_SUCCESS;
        }
        /// 如果服务进程pipe连接失败(有可能是没有起动服务进程，守护进程单进程调试)，守护进程自己负责杀死
        else {
          JsonValue json_value;
          if (YSOS_ERROR_SUCCESS != GetJsonUtil()->JsonObjectFromString(req_custom_event_service_param.service_param, json_value)) {
            printf("Daemon::ConnectionListener::HandleDoService4YSOS[Fail]The function[GetJsonUtil()->JsonObjectFromString()] is failed\n");
            YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Fail]The function[GetJsonUtil()->JsonObjectFromString()] is failed");
            result = YSOS_ERROR_LOGIC_ERROR;
            break;
          }

          printf("Daemon::ConnectionListener::HandleDoService4YSOS[Check Point][7]\n");
          YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Check Point][7]");

          if (true != json_value.isMember("processes") ||
              true != json_value["processes"].isArray()) {
            printf("Daemon::ConnectionListener::HandleDoService4YSOS[Fail][There is not processes in json_value]\n");
            YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Fail][There is not processes in json_value]");
            result = YSOS_ERROR_LOGIC_ERROR;
            break;
          }

          printf("Daemon::ConnectionListener::HandleDoService4YSOS[Check Point][8]\n");
          YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Check Point][8]");

          json_value = json_value["processes"];
          if (true == json_value.empty()) {
            printf("Daemon::ConnectionListener::HandleDoService4YSOS[Fail][json_value is empty]\n");
            YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Fail][json_value is empty]");
            result = YSOS_ERROR_LOGIC_ERROR;
            break;
          }

          printf("Daemon::ConnectionListener::HandleDoService4YSOS[Check Point][9]\n");
          YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Check Point][9]");

          ExternMessage extern_message(Daemon::MSG_KILL_PROCESS);
          for (unsigned ii = 0; ii < json_value.size(); ++ii) {
            extern_message.SetContent(json_value[ii].asString());
          }

          printf("Daemon::ConnectionListener::HandleDoService4YSOS[Check Point][10]\n");
          YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Check Point][10]");

          /// 请求杀死指定进程，通知监测进程类杀死指定进程
          result = Send(extern_message);
          if (YSOS_ERROR_SUCCESS != result) {
            printf("Daemon::ConnectionListener::HandleDoService4YSOS[Fail]The function[Send()] is failed\n");
            YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Fail]The function[Send()] is failed");
            result = YSOS_ERROR_LOGIC_ERROR;
            break;
          }
        }

        printf("Daemon::ConnectionListener::HandleDoService4YSOS[Check Point][11]\n");
        YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Check Point][11]");

        /// 拼装返回报文
        ResCustomEventServiceParam res_custem_event_service_param;
        res_custem_event_service_param.status_code = GetUtility()->ConvertToString(result);
        res_custem_event_service_param.description = (result == YSOS_ERROR_SUCCESS ? "succeeded" : "failed");
        res_custem_event_service_param.custom_event_service_unit.service_name = req_custom_event_service_param.service_name;
        res_custem_event_service_param.custom_event_service_unit.service_param = req_custom_event_service_param.service_param;
        std::string res_custem_event_service_param_string;
        if (true != res_custem_event_service_param.ToString(res_custem_event_service_param_string)) {
          printf("Daemon::ConnectionListener::HandleDoService4YSOS[Fail]session_id[%s]\n", session_id.c_str());
          YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Fail]session_id["<< session_id <<"]");
          result = YSOS_ERROR_LOGIC_ERROR;
          break;
        }

        printf("Daemon::ConnectionListener::HandleDoService4YSOS[res_custem_event_service_param_string][%s]\n", res_custem_event_service_param_string.c_str());
        YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[res_custem_event_service_param_string]["<< res_custem_event_service_param_string <<"]");

        /// 发送给平台进程
        if (YSOS_ERROR_SUCCESS != OnDispatchMessage4YSOS(service_name, res_custem_event_service_param_string)) {
          printf("Daemon::ConnectionListener::HandleDoService4YSOS[Fail]The function[OnDispatchMessage4YSOS()] is failed\n");
          YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Fail]The function[OnDispatchMessage4YSOS()] is failed");
          result = YSOS_ERROR_LOGIC_ERROR;
          break;
        }

        printf("Daemon::ConnectionListener::HandleDoService4YSOS[Check Point][YSOS_AGENT_SERVICE_NAME_CUSTOM_EVENT][End]\n");
        YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Check Point][YSOS_AGENT_SERVICE_NAME_CUSTOM_EVENT][End]");
      } else {
        /// 不支持服务名
        printf("Daemon::ConnectionListener::HandleDoService4YSOS[Not Suppoerted service_name][%s]\n", req_custom_event_service_param.service_name.c_str());
        YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Not Suppoerted service_name]["<< req_custom_event_service_param.service_name <<"]");
        result = YSOS_ERROR_NOT_SUPPORTED;
        break;
      }
    }
    /// 其他服务请求暂不处理
    else {
      printf("Daemon::ConnectionListener::HandleDoService4YSOS[Not Suppoerted ysos service_name][%s]\n", service_name.c_str());
      YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Not Suppoerted ysos service_name]["<< service_name <<"]");
      result = YSOS_ERROR_NOT_SUPPORTED;
      break;
    }

    printf("Daemon::ConnectionListener::HandleDoService4YSOS[Check Point][End]\n");
    YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Check Point][End]");
  } while (0);

  printf("Daemon::ConnectionListener::HandleDoService4YSOS[Exit]\n");
  YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4YSOS[Exit]");
  return result;
}

int ConnectionListener::HandleDoService4Update(const std::string& session_id, const std::string& service_name, const std::string& service_param) {
  printf("Daemon::ConnectionListener::HandleDoService4Update[Enter]\n");
  YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4Update[Enter]");
  int result = YSOS_ERROR_SUCCESS;

  do {
    if (true == session_id.empty() ||
        true == service_name.empty()) {
      printf("Daemon::ConnectionListener::HandleDoService4Update[Fail][session_id/service_name is empty]\n");
      YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4Update[Fail][session_id/service_name is empty]");
      result = YSOS_ERROR_LOGIC_ERROR;
      break;
    }

    printf("Daemon::ConnectionListener::HandleDoService4Update[Check Point][0]\n");
    YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4Update[Check Point][0]");

    /// EventNitify服务请求
    if (YSOS_AGENT_SERVICE_NAME_EVENT_NOTIFY == service_name) {
      /// 所有EventNotify服务请求都透传给平台
      if (YSOS_ERROR_SUCCESS != DoService4YSOS(service_name, service_param)) {
        printf("Daemon::ConnectionListener::HandleDoService4Update[Fail]The function[DoService4YSOS()] is failed\n");
        YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4Update[Fail]The function[DoService4YSOS()] is failed");
        result = YSOS_ERROR_LOGIC_ERROR;
        break;
      }
    }
    /// 其他服务请求暂不处理
    else {
      printf("Daemon::ConnectionListener::HandleDoService4Update[Not Suppoerted ysos service_name][%s]\n", service_name.c_str());
      YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4Update[Not Suppoerted ysos service_name]["<< service_name <<"]");
      result = YSOS_ERROR_NOT_SUPPORTED;
      break;
    }

    printf("Daemon::ConnectionListener::HandleDoService4Update[Check Point][End]\n");
    YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4Update[Check Point][End]");
  } while (0);

  printf("Daemon::ConnectionListener::HandleDoService4Update[Exit]\n");
  YSOS_LOG_DEBUG("ConnectionListener::HandleDoService4Update[Exit]");
  return result;
}

int ConnectionListener::DoService4YSOS(const std::string& service_name, const std::string& service_param) {
  printf("Daemon::ConnectionListener::DoService4YSOS[Enter]\n");
  YSOS_LOG_DEBUG("ConnectionListener::DoService4YSOS[Enter]");
  int result = YSOS_ERROR_FAILED;

  do {
    if (true == service_name.empty()) {
      printf("Daemon::ConnectionListener::DoService4YSOS[Fail][service_name is empty]\n");
      YSOS_LOG_DEBUG("ConnectionListener::DoService4YSOS[Fail][service_name is empty]");
      result = YSOS_ERROR_LOGIC_ERROR;
      break;
    }

    printf("Daemon::ConnectionListener::DoService4YSOS[Check Point][0]\n");
    YSOS_LOG_DEBUG("ConnectionListener::DoService4YSOS[Check Point][0]");

    /// 获取平台进程session id
    std::string session_id = GetAppSessionID(g_ysos_win_name);
    if (true == session_id.empty()) {
      printf("Daemon::ConnectionListener::DoService4YSOS[Fail][session_id is empty][0]\n");
      YSOS_LOG_DEBUG("ConnectionListener::DoService4YSOS[Fail][session_id is empty][0]");
      session_id = GetAppSessionID(g_ysos_console_name);
      if (true == session_id.empty()) {
        printf("Daemon::ConnectionListener::DoService4YSOS[Fail][session_id is empty][1]\n");
        YSOS_LOG_DEBUG("ConnectionListener::DoService4YSOS[Fail][session_id is empty][1]");
        result = YSOS_ERROR_LOGIC_ERROR;
        break;
      }
    }

    printf("Daemon::ConnectionListener::DoService4YSOS[Check Point][1]\n");
    YSOS_LOG_DEBUG("ConnectionListener::DoService4YSOS[Check Point][1]");

    if (YSOS_ERROR_SUCCESS != ysos::ysos_sdk::YSOSSDKDoService(session_id.c_str(), service_name.c_str(), service_param.c_str())) {
      printf("Daemon::ConnectionListener::DoService4YSOS[Fail]The function[ysos::ysos_sdk::YSOSSDKDoService()] is failed\n");
      YSOS_LOG_DEBUG("ConnectionListener::DoService4YSOS[Fail]The function[ysos::ysos_sdk::YSOSSDKDoService()] is failed");
      result = YSOS_ERROR_LOGIC_ERROR;
      break;
    }

    printf("Daemon::ConnectionListener::DoService4YSOS[Check Point][End]\n");
    YSOS_LOG_DEBUG("ConnectionListener::DoService4YSOS[Check Point][End]");
    result = YSOS_ERROR_SUCCESS;
  } while (0);

  printf("Daemon::ConnectionListener::DoService4YSOS[Exit]\n");
  YSOS_LOG_DEBUG("ConnectionListener::DoService4YSOS[Exit]");
  return result;
}

int ConnectionListener::DoService4Update(const std::string& service_name, const std::string& service_param) {
  printf("Daemon::ConnectionListener::DoService4Update[Enter]\n");
  YSOS_LOG_DEBUG("ConnectionListener::DoService4Update[Enter]");
  int result = YSOS_ERROR_FAILED;

  do {
    if (true == service_name.empty()) {
      printf("Daemon::ConnectionListener::DoService4Update[Fail][service_name is empty]\n");
      YSOS_LOG_DEBUG("ConnectionListener::DoService4Update[Fail][service_name is empty]");
      result = YSOS_ERROR_LOGIC_ERROR;
      break;
    }

    printf("Daemon::ConnectionListener::DoService4Update[Check Point][0]\n");
    YSOS_LOG_DEBUG("ConnectionListener::DoService4Update[Check Point][0]");

    /// 获取更新进程session id
    std::string session_id = GetAppSessionID(g_ysos_update_name);
    if (true == session_id.empty()) {
      printf("Daemon::ConnectionListener::DoService4Update[Fail][session_id is empty]\n");
      YSOS_LOG_DEBUG("ConnectionListener::DoService4Update[Fail][session_id is empty]");
      result = YSOS_ERROR_LOGIC_ERROR;
      break;
    }

    printf("Daemon::ConnectionListener::DoService4Update[Check Point][1]\n");
    YSOS_LOG_DEBUG("ConnectionListener::DoService4Update[Check Point][1]");

    if (YSOS_ERROR_SUCCESS != ysos::ysos_sdk::YSOSSDKDoService(session_id.c_str(), service_name.c_str(), service_param.c_str())) {
      printf("Daemon::ConnectionListener::DoService4Update[Fail]The function[ysos::ysos_sdk::YSOSSDKDoService()] is failed\n");
      YSOS_LOG_DEBUG("ConnectionListener::DoService4Update[Fail]The function[ysos::ysos_sdk::YSOSSDKDoService()] is failed");
      result = YSOS_ERROR_LOGIC_ERROR;
      break;
    }

    printf("Daemon::ConnectionListener::DoService4Update[Check Point][End]\n");
    YSOS_LOG_DEBUG("ConnectionListener::DoService4Update[Check Point][End]");
    result = YSOS_ERROR_SUCCESS;
  } while (0);

  printf("Daemon::ConnectionListener::DoService4Update[Exit]\n");
  YSOS_LOG_DEBUG("ConnectionListener::DoService4Update[Exit]");
  return result;
}

int ConnectionListener::DoService(const std::string& app_name, const std::string& service_name, const std::string& service_param) {
  printf("Daemon::ConnectionListener::DoService[Enter]\n");
  YSOS_LOG_DEBUG("ConnectionListener::DoService[Enter]");
  int result = YSOS_ERROR_FAILED;

  do {
    if (true == app_name.empty() ||
        true == service_name.empty()) {
      printf("Daemon::ConnectionListener::DoService[Fail][app_name/service_name is empty]\n");
      YSOS_LOG_DEBUG("ConnectionListener::DoService[Fail][app_name/service_name is empty]");
      result = YSOS_ERROR_LOGIC_ERROR;
      break;
    }

    printf("Daemon::ConnectionListener::DoService[Check Point][0]\n");
    YSOS_LOG_DEBUG("ConnectionListener::DoService[Check Point][0]");

    /// 获取进程session id
    std::string session_id = GetAppSessionID(app_name);
    if (true == session_id.empty()) {
      printf("Daemon::ConnectionListener::DoService[Fail][session_id is empty]\n");
      YSOS_LOG_DEBUG("ConnectionListener::DoService[Fail][session_id is empty]");
      result = YSOS_ERROR_LOGIC_ERROR;
      break;
    }

    printf("Daemon::ConnectionListener::DoService[Check Point][1]\n");
    YSOS_LOG_DEBUG("ConnectionListener::DoService[Check Point][1]");

    if (YSOS_ERROR_SUCCESS != ysos::ysos_sdk::YSOSSDKDoService(session_id.c_str(), service_name.c_str(), service_param.c_str())) {
      printf("Daemon::ConnectionListener::DoService[Fail]The function[ysos::ysos_sdk::YSOSSDKDoService()] is failed\n");
      YSOS_LOG_DEBUG("ConnectionListener::DoService[Fail]The function[ysos::ysos_sdk::YSOSSDKDoService()] is failed");
      result = YSOS_ERROR_LOGIC_ERROR;
      break;
    }

    printf("Daemon::ConnectionListener::DoService[Check Point][End]\n");
    YSOS_LOG_DEBUG("ConnectionListener::DoService[Check Point][End]");
    result = YSOS_ERROR_SUCCESS;
  } while (0);

  printf("Daemon::ConnectionListener::DoService[Exit]\n");
  YSOS_LOG_DEBUG("ConnectionListener::DoService[Exit]");
  return result;
}

void ConnectionListener::UserHandleOnDispatchMessage(const char* session_id, const char* service_name, const char* result) {
  printf("Daemon::ConnectionListener::UserHandleOnDispatchMessage[Enter]\n");
  YSOS_LOG_DEBUG("ConnectionListener::UserHandleOnDispatchMessage[Enter]");
  int return_value = YSOS_ERROR_FAILED;

  do {
    if (NULL != session_id) {
      printf("Daemon::session_id[%s]\n", session_id);
      YSOS_LOG_DEBUG("ConnectionListener::UserHandleOnDispatchMessage[session_id]["<< session_id <<"]");
    }

    if (NULL != service_name) {
      printf("Daemon::service_name[%s]\n", service_name);
      YSOS_LOG_DEBUG("ConnectionListener::UserHandleOnDispatchMessage[service_name]["<< service_name <<"]");
    }

    if (NULL != result) {
      printf("Daemon::result[%s]\n", result);
      YSOS_LOG_DEBUG("ConnectionListener::UserHandleOnDispatchMessage[result]["<< result <<"]");
    }

    std::string duplicated_session_id(session_id), duplicated_service_name(service_name), duplicated_result(result);

    printf("Daemon::ConnectionListener::UserHandleOnDispatchMessage[Check Point][0]\n");
    YSOS_LOG_DEBUG("ConnectionListener::UserHandleOnDispatchMessage[Check Point][0]");

    /// 判断session id是否为空
    if (true == duplicated_session_id.empty()) {
      printf("Daemon::ConnectionListener::UserHandleOnDispatchMessage[Fail]duplicated_session_id is empty\n");
      YSOS_LOG_DEBUG("ConnectionListener::UserHandleOnDispatchMessage[Fail]duplicated_session_id is empty");
      return_value = YSOS_ERROR_LOGIC_ERROR;
      break;
    }

    printf("Daemon::ConnectionListener::UserHandleOnDispatchMessage[Check Point][1]\n");
    YSOS_LOG_DEBUG("ConnectionListener::UserHandleOnDispatchMessage[Check Point][1]");

    /// 根据session id调用平台进程/更新进程请求逻辑处理函数
    std::string app_name = this_ptr_->GetAppName(duplicated_session_id);
    /// 调平台进程请求逻辑处理函数
    if (app_name == g_ysos_win_name ||
        app_name == g_ysos_console_name) {
      printf("Daemon::~~~[%s] is connected\n", app_name.c_str());
      if (YSOS_ERROR_SUCCESS != this_ptr_->HandleOnDispatchMessage4YSOS(duplicated_session_id, duplicated_service_name, duplicated_result)) {
        printf("Daemon::ConnectionListener::UserHandleOnDispatchMessage[Fail][The function[this_ptr_->HandleOnDispatchMessage4YSOS()] is failed]\n");
        YSOS_LOG_DEBUG("ConnectionListener::UserHandleOnDispatchMessage[Fail][The function[this_ptr_->HandleOnDispatchMessage4YSOS()] is failed]");
        return_value = YSOS_ERROR_LOGIC_ERROR;
        break;
      }
    }
    /// 调更新进程请求逻辑处理函数
    else if (app_name == g_ysos_update_name) {
      printf("Daemon::!!![%s] is connected\n", app_name.c_str());
      YSOS_LOG_DEBUG("!!!["<< app_name <<"] is connected");
      if (YSOS_ERROR_SUCCESS != this_ptr_->HandleOnDispatchMessage4Update(duplicated_session_id, duplicated_service_name, duplicated_result)) {
        printf("Daemon::ConnectionListener::UserHandleOnDispatchMessage[Fail][The function[this_ptr_->HandleOnDispatchMessage4Update(()] is failed]\n");
        YSOS_LOG_DEBUG("ConnectionListener::UserHandleOnDispatchMessage[Fail][The function[this_ptr_->HandleOnDispatchMessage4Update(()] is failed]");
        return_value = YSOS_ERROR_LOGIC_ERROR;
        break;
      }
    }
    /// 应用名为空或者没找到session id对应的应用名
    else {
      printf("Daemon::ConnectionListener::UserHandleOnDispatchMessage[Fail][It can't find app name[%s]]\n", app_name.c_str());
      YSOS_LOG_DEBUG("ConnectionListener::UserHandleOnDispatchMessage[Fail][It can't find app name]["<< app_name <<"]");
      return_value = YSOS_ERROR_LOGIC_ERROR;
      break;
    }

    return_value = YSOS_ERROR_SUCCESS;
  } while (0);

  if (YSOS_ERROR_SUCCESS != return_value) {
    printf("Daemon::ConnectionListener::UserHandleOnDispatchMessage[Error]\n");
    YSOS_LOG_DEBUG("onnectionListener::UserHandleOnDispatchMessage[Error]");
  }

  printf("Daemon::ConnectionListener::UserHandleOnDispatchMessage[Exit]\n");
  YSOS_LOG_DEBUG("ConnectionListener::UserHandleOnDispatchMessage[Exit]");
  return;
}

int ConnectionListener::HandleOnDispatchMessage4YSOS(const std::string& session_id, const std::string& service_name, const std::string& result) {
  printf("Daemon::ConnectionListener::HandleOnDispatchMessage4YSOS[Enter]\n");
  YSOS_LOG_DEBUG("ConnectionListener::HandleOnDispatchMessage4YSOS[Enter]");
  int return_value = YSOS_ERROR_FAILED;

  do {
    if (true == session_id.empty() ||
        true == service_name.empty()) {
      printf("Daemon::ConnectionListener::HandleOnDispatchMessage4YSOS[Fail][session_id/service_name is empty]\n");
      YSOS_LOG_DEBUG("ConnectionListener::HandleOnDispatchMessage4YSOS[Fail][session_id/service_name is empty]");
      return_value = YSOS_ERROR_LOGIC_ERROR;
      break;
    }

    printf("Daemon::ConnectionListener::HandleOnDispatchMessage4YSOS[Check Point][0]\n");
    YSOS_LOG_DEBUG("ConnectionListener::HandleOnDispatchMessage4YSOS[Check Point][0]");

    /// 自定义服务请求
    if (YSOS_AGENT_SERVICE_NAME_CUSTOM_EVENT == service_name) {
      printf("Daemon::ConnectionListener::HandleOnDispatchMessage4YSOS[Check Point][YSOS_AGENT_SERVICE_NAME_CUSTOM_EVENT][Enter]\n");
      YSOS_LOG_DEBUG("ConnectionListener::HandleOnDispatchMessage4YSOS[Check Point][YSOS_AGENT_SERVICE_NAME_CUSTOM_EVENT][Enter]");
      if (true == result.empty()) {
        printf("Daemon::ConnectionListener::HandleOnDispatchMessage4YSOS[Fail][result is empty]\n");
        YSOS_LOG_DEBUG("ConnectionListener::HandleOnDispatchMessage4YSOS[Fail][result is empty]");
        return_value = YSOS_ERROR_LOGIC_ERROR;
        break;
      }

      printf("Daemon::ConnectionListener::HandleOnDispatchMessage4YSOS[Check Point][1]\n");
      YSOS_LOG_DEBUG("ConnectionListener::HandleOnDispatchMessage4YSOS[Check Point][1]");

      /// 解析服务参数
      ResCustomEventServiceParam res_custom_event_service_param;
      if (true != res_custom_event_service_param.FromString(result)) {
        printf("Daemon::ConnectionListener::HandleOnDispatchMessage4YSOS[Fail]The function[res_custom_event_service_param.FromString()] is failed\n");
        YSOS_LOG_DEBUG("ConnectionListener::HandleOnDispatchMessage4YSOS[Fail]The function[res_custom_event_service_param.FromString()] is failed");
        return_value = YSOS_ERROR_LOGIC_ERROR;
        break;
      }

      printf("Daemon::ConnectionListener::HandleOnDispatchMessage4YSOS[service_name][%s]\n", res_custom_event_service_param.custom_event_service_unit.service_name.c_str());
      YSOS_LOG_DEBUG("ConnectionListener::HandleOnDispatchMessage4YSOS[service_name]["<< res_custom_event_service_param.custom_event_service_unit.service_name <<"]");
      printf("Daemon::ConnectionListener::HandleOnDispatchMessage4YSOS[status_code][%s]\n", res_custom_event_service_param.status_code.c_str());
      YSOS_LOG_DEBUG("ConnectionListener::HandleOnDispatchMessage4YSOS[status_code]["<< res_custom_event_service_param.status_code <<"]");

      printf("Daemon::ConnectionListener::HandleOnDispatchMessage4YSOS[Check Point][YSOS_AGENT_SERVICE_NAME_CUSTOM_EVENT][End]\n");
      YSOS_LOG_DEBUG("ConnectionListener::HandleOnDispatchMessage4YSOS[Check Point][YSOS_AGENT_SERVICE_NAME_CUSTOM_EVENT][End]");
    }
    /// 其他服务请求暂不处理
    else {
      printf("Daemon::ConnectionListener::HandleOnDispatchMessage4YSOS[Not Suppoerted ysos service_name][%s]\n", service_name.c_str());
      YSOS_LOG_DEBUG("ConnectionListener::HandleOnDispatchMessage4YSOS[Not Suppoerted ysos service_name]["<< service_name <<"]");
      return_value = YSOS_ERROR_NOT_SUPPORTED;
      break;
    }

    printf("Daemon::ConnectionListener::HandleOnDispatchMessage4YSOS[Check Point][End]\n");
    YSOS_LOG_DEBUG("ConnectionListener::HandleOnDispatchMessage4YSOS[Check Point][End]");
    return_value = YSOS_ERROR_SUCCESS;
  } while (0);

  printf("Daemon::ConnectionListener::HandleOnDispatchMessage4YSOS[Exit]\n");
  YSOS_LOG_DEBUG("ConnectionListener::HandleOnDispatchMessage4YSOS[Exit]");
  return return_value;
}

int ConnectionListener::HandleOnDispatchMessage4Update(const std::string& session_id, const std::string& service_name, const std::string& result) {
  printf("Daemon::ConnectionListener::HandleOnDispatchMessage4Update[Enter]\n");
  YSOS_LOG_DEBUG("ConnectionListener::HandleOnDispatchMessage4Update[Enter]");
  int return_value = YSOS_ERROR_FAILED;

  do {
    if (true == session_id.empty() ||
        true == service_name.empty()) {
      printf("Daemon::ConnectionListener::HandleOnDispatchMessage4Update[Fail][session_id/service_name is empty]\n");
      YSOS_LOG_DEBUG("ConnectionListener::HandleOnDispatchMessage4Update[Fail][session_id/service_name is empty]");
      return_value = YSOS_ERROR_LOGIC_ERROR;
      break;
    }

    printf("Daemon::ConnectionListener::HandleOnDispatchMessage4Update[Check Point][0]\n");
    YSOS_LOG_DEBUG("ConnectionListener::HandleOnDispatchMessage4Update[Check Point][0]");

    /// 自定义服务请求
    if (YSOS_AGENT_SERVICE_NAME_CUSTOM_EVENT == service_name) {
      printf("Daemon::ConnectionListener::HandleOnDispatchMessage4Update[Check Point][YSOS_AGENT_SERVICE_NAME_CUSTOM_EVENT][Enter]\n");
      YSOS_LOG_DEBUG("ConnectionListener::HandleOnDispatchMessage4Update[Check Point][YSOS_AGENT_SERVICE_NAME_CUSTOM_EVENT][Enter]");
      if (true == result.empty()) {
        printf("Daemon::ConnectionListener::HandleOnDispatchMessage4Update[Fail][result is empty]\n");
        YSOS_LOG_DEBUG("ConnectionListener::HandleOnDispatchMessage4Update[Fail][result is empty]");
        return_value = YSOS_ERROR_LOGIC_ERROR;
        break;
      }

      printf("Daemon::ConnectionListener::HandleOnDispatchMessage4Update[Check Point][1]\n");
      YSOS_LOG_DEBUG("ConnectionListener::HandleOnDispatchMessage4Update[Check Point][1]");

      /// 解析服务参数
      ResCustomEventServiceParam res_custom_event_service_param;
      if (true != res_custom_event_service_param.FromString(result)) {
        YSOS_LOG_DEBUG("ConnectionListener::HandleOnDispatchMessage4Update[Fail]The function[res_custom_event_service_param.FromString()] is failed");
        return_value = YSOS_ERROR_LOGIC_ERROR;
        break;
      }

      printf("Daemon::ConnectionListener::HandleOnDispatchMessage4Update[Check Point][2]\n");
      YSOS_LOG_DEBUG("ConnectionListener::HandleOnDispatchMessage4Update[Check Point][2]");

      printf("Daemon::ConnectionListener::HandleOnDispatchMessage4Update[service_name][%s]\n", res_custom_event_service_param.custom_event_service_unit.service_name.c_str());
      YSOS_LOG_DEBUG("ConnectionListener::HandleOnDispatchMessage4Update[service_name]["<< res_custom_event_service_param.custom_event_service_unit.service_name <<"]");
      printf("Daemon::ConnectionListener::HandleOnDispatchMessage4Update[status_code][%s]\n", res_custom_event_service_param.status_code.c_str());
      YSOS_LOG_DEBUG("ConnectionListener::HandleOnDispatchMessage4Update[status_code]["<< res_custom_event_service_param.status_code <<"]");

      printf("Daemon::ConnectionListener::HandleOnDispatchMessage4Update[Check Point][YSOS_AGENT_SERVICE_NAME_CUSTOM_EVENT][End]\n");
      YSOS_LOG_DEBUG("ConnectionListener::HandleOnDispatchMessage4Update[Check Point][YSOS_AGENT_SERVICE_NAME_CUSTOM_EVENT][End]");
    } else {
      printf("Daemon::ConnectionListener::HandleOnDispatchMessage4Update[Not Suppoerted ysos service_name][%s]\n", service_name.c_str());
      YSOS_LOG_DEBUG("ConnectionListener::HandleOnDispatchMessage4Update[Not Suppoerted ysos service_name]["<< service_name <<"]");
      return_value = YSOS_ERROR_NOT_SUPPORTED;
      break;
    }

    printf("Daemon::ConnectionListener::HandleOnDispatchMessage4Update[Check Point][End]\n");
    YSOS_LOG_DEBUG("ConnectionListener::HandleOnDispatchMessage4Update[Check Point][End]");
    return_value = YSOS_ERROR_SUCCESS;
  } while (0);

  printf("Daemon::ConnectionListener::HandleOnDispatchMessage4Update[Exit]\n");
  YSOS_LOG_DEBUG("ConnectionListener::HandleOnDispatchMessage4Update[Exit]");
  return return_value;
}

int ConnectionListener::OnDispatchMessage4YSOS(const std::string& service_name, const std::string& result) {
  printf("Daemon::ConnectionListener::OnDispatchMessage4YSOS[Enter]\n");
  YSOS_LOG_DEBUG("ConnectionListener::OnDispatchMessage4YSOS[Enter]");
  int return_value = YSOS_ERROR_FAILED;

  do {
    if (true == service_name.empty()) {
      printf("Daemon::ConnectionListener::OnDispatchMessage4YSOS[Fail][service_name is empty]\n");
      YSOS_LOG_DEBUG("ConnectionListener::OnDispatchMessage4YSOS[Fail][service_name is empty]");
      return_value = YSOS_ERROR_LOGIC_ERROR;
      break;
    }

    printf("Daemon::ConnectionListener::OnDispatchMessage4YSOS[Check Point][0]\n");
    YSOS_LOG_DEBUG("ConnectionListener::OnDispatchMessage4YSOS[Check Point][0]");

    /// 获取平台进程session id
    std::string session_id = GetAppSessionID(g_ysos_win_name);
    if (true == session_id.empty()) {
      printf("Daemon::ConnectionListener::OnDispatchMessage4YSOS[Fail][session_id is empty][0]\n");
      YSOS_LOG_DEBUG("ConnectionListener::OnDispatchMessage4YSOS[Fail][session_id is empty][0]");
      session_id = GetAppSessionID(g_ysos_console_name);
      if (true == session_id.empty()) {
        printf("Daemon::ConnectionListener::OnDispatchMessage4YSOS[Fail][session_id is empty][1]\n");
        YSOS_LOG_DEBUG("ConnectionListener::OnDispatchMessage4YSOS[Fail][session_id is empty][1]");
        return_value = YSOS_ERROR_LOGIC_ERROR;
        break;
      }
    }

    printf("Daemon::ConnectionListener::OnDispatchMessage4YSOS[Check Point][1]\n");
    YSOS_LOG_DEBUG("ConnectionListener::OnDispatchMessage4YSOS[Check Point][1]");

    if (YSOS_ERROR_SUCCESS != ysos::ysos_sdk::YSOSSDKOnDispatchMessage(session_id.c_str(), service_name.c_str(), result.c_str())) {
      printf("Daemon::ConnectionListener::OnDispatchMessage4YSOS[Fail]The function[ysos::ysos_sdk::YSOSSDKOnDispatchMessage()] is failed\n");
      YSOS_LOG_DEBUG("ConnectionListener::OnDispatchMessage4YSOS[Fail]The function[ysos::ysos_sdk::YSOSSDKOnDispatchMessage()] is failed");
      return_value = YSOS_ERROR_LOGIC_ERROR;
      break;
    }

    printf("Daemon::ConnectionListener::OnDispatchMessage4YSOS[Check Point][End]\n");
    YSOS_LOG_DEBUG("ConnectionListener::OnDispatchMessage4YSOS[Check Point][End]");
    return_value = YSOS_ERROR_SUCCESS;
  } while (0);

  printf("Daemon::ConnectionListener::OnDispatchMessage4YSOS[Exit]\n");
  YSOS_LOG_DEBUG("ConnectionListener::OnDispatchMessage4YSOS[Exit]");
  return return_value;
}

int ConnectionListener::OnDispatchMessage4Update(const std::string& service_name, const std::string& result) {
  printf("Daemon::ConnectionListener::OnDispatchMessage4Update[Enter]\n");
  YSOS_LOG_DEBUG("ConnectionListener::OnDispatchMessage4Update[Enter]");
  int return_value = YSOS_ERROR_FAILED;
  do {
    if (true == service_name.empty()) {
      printf("Daemon::ConnectionListener::OnDispatchMessage4Update[Fail][service_name is empty]\n");
      YSOS_LOG_DEBUG("ConnectionListener::OnDispatchMessage4Update[Fail][service_name is empty]");
      return_value = YSOS_ERROR_LOGIC_ERROR;
      break;
    }

    printf("Daemon::ConnectionListener::OnDispatchMessage4Update[Check Point][0]\n");
    YSOS_LOG_DEBUG("ConnectionListener::OnDispatchMessage4Update[Check Point][0]");

    /// 获取更新进程session id
    std::string session_id = GetAppSessionID(g_ysos_update_name);
    if (true == session_id.empty()) {
      printf("Daemon::ConnectionListener::OnDispatchMessage4Update[Fail][session_id is empty]\n");
      YSOS_LOG_DEBUG("ConnectionListener::OnDispatchMessage4Update[Fail][session_id is empty]");
      return_value = YSOS_ERROR_LOGIC_ERROR;
      break;
    }

    printf("Daemon::ConnectionListener::OnDispatchMessage4Update[Check Point][1]\n");
    YSOS_LOG_DEBUG("ConnectionListener::OnDispatchMessage4Update[Check Point][1]");

    if (YSOS_ERROR_SUCCESS != ysos::ysos_sdk::YSOSSDKOnDispatchMessage(session_id.c_str(), service_name.c_str(), result.c_str())) {
      printf("Daemon::ConnectionListener::OnDispatchMessage4Update[Fail]The function[ysos::ysos_sdk::YSOSSDKOnDispatchMessage()] is failed\n");
      YSOS_LOG_DEBUG("ConnectionListener::OnDispatchMessage4Update[Fail]The function[ysos::ysos_sdk::YSOSSDKOnDispatchMessage()] is failed");
      return_value = YSOS_ERROR_LOGIC_ERROR;
      break;
    }

    printf("Daemon::ConnectionListener::OnDispatchMessage4Update[Check Point][End]\n");
    YSOS_LOG_DEBUG("ConnectionListener::OnDispatchMessage4Update[Check Point][End]");
    return_value = YSOS_ERROR_SUCCESS;
  } while (0);

  printf("Daemon::ConnectionListener::OnDispatchMessage4Update[Exit]\n");
  YSOS_LOG_DEBUG("ConnectionListener::OnDispatchMessage4Update[Exit]");
  return return_value;
}

int ConnectionListener::OnDispatchMessage(const std::string& app_name, const std::string& service_name, const std::string& result) {
  printf("Daemon::ConnectionListener::OnDispatchMessage[Enter]\n");
  YSOS_LOG_DEBUG("ConnectionListener::OnDispatchMessage[Enter]");
  int return_value = YSOS_ERROR_FAILED;
  do {
    if (true == app_name.empty() ||
        true == service_name.empty()) {
      printf("Daemon::ConnectionListener::OnDispatchMessage[Fail][app_name/service_name is empty]\n");
      YSOS_LOG_DEBUG("ConnectionListener::OnDispatchMessage[Fail][app_name/service_name is empty]");
      return_value = YSOS_ERROR_LOGIC_ERROR;
      break;
    }

    printf("Daemon::ConnectionListener::OnDispatchMessage[Check Point][0]\n");
    YSOS_LOG_DEBUG("ConnectionListener::OnDispatchMessage[Check Point][0]");

    /// 获取进程session id
    std::string session_id = GetAppSessionID(app_name);
    if (true == session_id.empty()) {
      printf("Daemon::ConnectionListener::OnDispatchMessage[Fail][session_id is empty]\n");
      YSOS_LOG_DEBUG("ConnectionListener::OnDispatchMessage[Fail][session_id is empty]");
      return_value = YSOS_ERROR_LOGIC_ERROR;
      break;
    }

    printf("Daemon::ConnectionListener::OnDispatchMessage[Check Point][1]\n");
    YSOS_LOG_DEBUG("ConnectionListener::OnDispatchMessage[Check Point][1]");

    if (YSOS_ERROR_SUCCESS != ysos::ysos_sdk::YSOSSDKOnDispatchMessage(session_id.c_str(), service_name.c_str(), result.c_str())) {
      printf("Daemon::ConnectionListener::OnDispatchMessage[Fail]The function[ysos::ysos_sdk::YSOSSDKOnDispatchMessage()] is failed\n");
      YSOS_LOG_DEBUG("ConnectionListener::OnDispatchMessage[Fail]The function[ysos::ysos_sdk::YSOSSDKOnDispatchMessage()] is failed");
      return_value = YSOS_ERROR_LOGIC_ERROR;
      break;
    }

    printf("Daemon::ConnectionListener::OnDispatchMessage[Check Point][End]\n");
    YSOS_LOG_DEBUG("ConnectionListener::OnDispatchMessage[Check Point][End]");
    return_value = YSOS_ERROR_SUCCESS;
  } while (0);

  printf("Daemon::ConnectionListener::OnDispatchMessage[Exit]\n");
  YSOS_LOG_DEBUG("ConnectionListener::OnDispatchMessage[Exit]");
  return return_value;
}

void ConnectionListener::UserHandleError(const char* session_id, const char* error_param) {
  printf("Daemon::ConnectionListener::UserHandleError[Enter]\n");
  YSOS_LOG_DEBUG("ConnectionListener::UserHandleError[Enter]");
  int result = YSOS_ERROR_SUCCESS;

  do {
    if (NULL != session_id) {
      printf("Daemon::session_id[%s]\n", session_id);
      YSOS_LOG_DEBUG("ConnectionListener::UserHandleError[Check Point][session_id]["<< session_id <<"]");
    }

    if (NULL != error_param) {
      printf("Daemon::error_param[%s]\n", error_param);
      YSOS_LOG_DEBUG("ConnectionListener::UserHandleError[Check Point][error_param]["<< error_param <<"]");
    }

    std::string duplicated_session_id(session_id), duplicated_error_param(error_param);

    printf("Daemon::ConnectionListener::UserHandleError[Check Point][0]\n");
    YSOS_LOG_DEBUG("ConnectionListener::UserHandleError[Check Point][0]");

    /// 判断session id是否为空
    if (true == duplicated_session_id.empty()) {
      printf("Daemon::ConnectionListener::UserHandleError[Fail]duplicated_session_id is empty\n");
      result = YSOS_ERROR_LOGIC_ERROR;
      break;
    }

    printf("Daemon::ConnectionListener::UserHandleError[Check Point][1]\n");
    YSOS_LOG_DEBUG("ConnectionListener::UserHandleError[Check Point][1]");

    /// 根据session id调用平台进程/更新进程错误处理
    std::string app_name = this_ptr_->GetAppName(duplicated_session_id);
    /// 调平台进程错误处理
    if (app_name == g_ysos_win_name ||
        app_name == g_ysos_console_name) {
      if (YSOS_ERROR_SUCCESS != this_ptr_->HandleError4YSOS(duplicated_session_id, duplicated_error_param)) {
        printf("Daemon::ConnectionListener::UserHandleError[Fail][The function[this_ptr_->HandleError4YSOS()] is failed]\n");
        YSOS_LOG_DEBUG("ConnectionListener::UserHandleError[Fail][The function[this_ptr_->HandleError4YSOS()] is failed]");
        result = YSOS_ERROR_LOGIC_ERROR;
        break;
      }
    }
    /// 调更新进程错误处理
    else if (app_name == g_ysos_update_name) {
      if (YSOS_ERROR_SUCCESS != this_ptr_->HandleError4Update(duplicated_session_id, duplicated_error_param)) {
        printf("Daemon::ConnectionListener::UserHandleError[Fail][The function[this_ptr_->HandleError4Update(()] is failed]\n");
        YSOS_LOG_DEBUG("ConnectionListener::UserHandleError[Fail][The function[this_ptr_->HandleError4Update(()] is failed]");
        result = YSOS_ERROR_LOGIC_ERROR;
        break;
      }
    }
    /// 应用名为空或者没找到session id对应的应用名
    else {
      printf("Daemon::ConnectionListener::UserHandleError[Fail][It can't find app name[%s]]\n", app_name.c_str());
      YSOS_LOG_DEBUG("ConnectionListener::UserHandleError[Fail][It can't find app name]["<< app_name <<"]");
      result = YSOS_ERROR_LOGIC_ERROR;
      break;
    }

    printf("Daemon::ConnectionListener::UserHandleError[Check Point][2]\n");
    YSOS_LOG_DEBUG("ConnectionListener::UserHandleError[Check Point][2]");

    /// 删除出错的连接
    {
      printf("Daemon::ConnectionListener::UserHandleError[Check Point][Delete session_id][%s]\n", duplicated_session_id.c_str());
      YSOS_LOG_DEBUG("ConnectionListener::UserHandleError[Check Point][Delete session_id]["<< duplicated_session_id <<"]");

      /// 加锁访问数据
      BoostSharedLock lock(this_ptr_->shared_mutex_);
      AppSessionIDMapItr itr = this_ptr_->app_session_id_map_.find(duplicated_session_id);
      this_ptr_->app_session_id_map_.erase(itr);
    }

    printf("Daemon::ConnectionListener::UserHandleError[Check Point][End]\n");
    YSOS_LOG_DEBUG("ConnectionListener::UserHandleError[Check Point][End]");
  } while (0);

  if (YSOS_ERROR_SUCCESS != result) {
    printf("Daemon::ConnectionListener::UserHandleError[Error]\n");
    YSOS_LOG_DEBUG("ConnectionListener::UserHandleError[Error]");
  }

  printf("Daemon::ConnectionListener::UserHandleError[Exit]\n");
  YSOS_LOG_DEBUG("ConnectionListener::UserHandleError[Exit]");
  return;
}

int ConnectionListener::HandleError4YSOS(const std::string& session_id, const std::string& error_param) {
  printf("Daemon::ConnectionListener::HandleError4YSOS[Enter]\n");
  YSOS_LOG_DEBUG("ConnectionListener::HandleError4YSOS[Enter]");
  int result = YSOS_ERROR_SUCCESS;

  do {
    if (true == session_id.empty() ||
        true == error_param.empty()) {
      YSOS_LOG_DEBUG("ConnectionListener::HandleError4YSOS[Fail][session_id or error_param is empty]");
      result = YSOS_ERROR_LOGIC_ERROR;
      break;
    }

    YSOS_LOG_DEBUG("ConnectionListener::HandleError4YSOS[session_id]["<< session_id <<"]");
    YSOS_LOG_DEBUG("ConnectionListener::HandleError4YSOS[error_param]["<< error_param <<"]");
  } while (0);

  printf("Daemon::ConnectionListener::HandleError4YSOS[Exit]\n");
  YSOS_LOG_DEBUG("ConnectionListener::HandleError4YSOS[Exit]");
  return result;
}

int ConnectionListener::HandleError4Update(const std::string& session_id, const std::string& error_param) {
  printf("Daemon::ConnectionListener::HandleError4Update[Enter]\n");
  YSOS_LOG_DEBUG("ConnectionListener::HandleError4Update[Enter]");
  int result = YSOS_ERROR_SUCCESS;

  do {
    if (true == session_id.empty() ||
        true == error_param.empty()) {
      YSOS_LOG_DEBUG("ConnectionListener::HandleError4Update[Fail][session_id or error_param is empty]");
      result = YSOS_ERROR_LOGIC_ERROR;
      break;
    }

    YSOS_LOG_DEBUG("ConnectionListener::HandleError4Update[session_id]["<< session_id <<"]");
    YSOS_LOG_DEBUG("ConnectionListener::HandleError4Update[error_param]["<< error_param <<"]");
  } while (0);

  printf("Daemon::ConnectionListener::HandleError4Update[Exit]\n");
  YSOS_LOG_DEBUG("ConnectionListener::HandleError4Update[Exit]");
  return result;
}

int ConnectionListener::Init(void) {
  printf("Daemon::ConnectionListener::Init[Enter]\n");
  int result = YSOS_ERROR_FAILED;

  do {
    /// 初始化本地日志
    if (true != logger_.valid()) {
      printf("Daemon::ConnectionListener::Init[Check Point][The function[logger_.valid()] is failed]\n");
      /// 初始化日志文件路径
      if (true != log::InitLoggerExt("../log/daemon.log", "daemon")) {
        printf("Daemon::ConnectionListener::Init[Fail][The function[log::InitLoggerExt()] is failed]\n");
        return YSOS_ERROR_FAILED;
      }

      printf("Daemon::ConnectionListener::Init[Check Point][0]\n");

      /// 判断logger是否可用
      logger_ = ysos::log::GetLogger("daemon");
      if (true != logger_.valid()) {
        printf("Daemon::ConnectionListener::Init[Fail][The function[logger_.valid()] is failed]\n");
        return YSOS_ERROR_FAILED;
      }

      printf("Daemon::ConnectionListener::Init[Check Point][1]\n");
    }

    printf("Daemon::ConnectionListener::Init[Check Point][2]\n");

#ifdef _WIN32
    /// 创建平台日志文件路径
    std::string log_properties_file_path;
    if (YSOS_ERROR_SUCCESS != SetYSOSLogProperty(log_properties_file_path)) {
      printf("Daemon::The function[SetYSOSLogProperty()] fails.\n");
      result = YSOS_ERROR_LOGIC_ERROR;
      break;
    }

    printf("Daemon::ConnectionListener::Init[Check Point][3]\n");

    /// 初始化平台类日志配置
    if (true != ysos::ysos_sdk::InitYSOSSDKLogger(log_properties_file_path.c_str())) {
      printf("Daemon::The function[log::InitYSOSSDKLogger()] fails\n");
      result = YSOS_ERROR_LOGIC_ERROR;
      break;
    }
#else
    /// 创建平台日志文件路径
    std::string log_properties_file_path;
    if (YSOS_ERROR_SUCCESS != SetYSOSLogProperty(log_properties_file_path)) {
      printf("Daemon::The function[SetYSOSLogProperty()] fails.\n");
      result = YSOS_ERROR_LOGIC_ERROR;
      break;
    }

    printf("Daemon::ConnectionListener::Init[Check Point][3]\n");

    /// 初始化平台类日志配置
    if (true != ysos::ysos_sdk::InitYSOSSDKLogger(log_properties_file_path.c_str())) {
      printf("Daemon::The function[log::InitYSOSSDKLogger()] fails\n");
      result = YSOS_ERROR_LOGIC_ERROR;
      break;
    }

#endif

    printf("Daemon::ConnectionListener::Init[Check Point][End]\n");
    result = YSOS_ERROR_SUCCESS;
  } while (0);

  printf("Daemon::ConnectionListener::Init[Exit]\n");
  return result;
}

#if _WIN32
int ConnectionListener::SetYSOSLogProperty(std::string& log_properties_file_path) {
  printf("Daemon::ConnectionListener::SetYSOSLogProperty[Enter]\n");
  int result = YSOS_ERROR_FAILED;

  do {
    /// 获取日志配置文件模板路径
    std::string log_template_properties_file_path;
    if (YSOS_ERROR_SUCCESS != GetYSOSLogTemplatePropertiesFilePath(log_template_properties_file_path)) {
      printf("Daemon::ConnectionListener::SetYSOSLogProperty[Fail][GetYSOSLogTemplatePropertiesFilePath()]\n");
      break;
    }

    printf("Daemon::ConnectionListener::SetYSOSLogProperty[log_template_properties_file_path][%s]\n", log_template_properties_file_path.c_str());

    /// 重新生成日志配置文件
    if (YSOS_ERROR_SUCCESS != CreateYSOSLogPropertiesFile(log_template_properties_file_path, log_properties_file_path)) {
      printf("Daemon::ConnectionListener::SetYSOSLogProperty[Fail][CreateYSOSLogPropertiesFile()]\n");
      break;
    }

    printf("Daemon::ConnectionListener::SetYSOSLogProperty[log_properties_file_path][%s]\n", log_properties_file_path.c_str());

    /// 加载log文件夹下log.properties文件
    FileLoader file_loader;
    if (TRUE != file_loader.OpenFile(log_properties_file_path.c_str())) {
      printf("Daemon::ConnectionListener::SetYSOSLogProperty[Fail][file_loader.OpenFile()]\n");
      break;
    }

    printf("Daemon::ConnectionListener::SetYSOSLogProperty[Check Point][0]\n");

    /// 获取文件内容
    LPVOID file_data = file_loader.GetFileData();
    if (NULL == file_data) {
      printf("Daemon::ConnectionListener::SetYSOSLogProperty[Fail][file_data is NULL]\n");
      break;
    }

    printf("Daemon::ConnectionListener::SetYSOSLogProperty[Check Point][1]\n");

    /// 获取替换字符
    std::string alternate_string;
    if (YSOS_ERROR_SUCCESS != GetYSOSLogPropertiesAlternateString(alternate_string)) {
      printf("Daemon::ConnectionListener::SetYSOSLogProperty[Fail][GetYSOSLogPropertiesAlternateString()]\n");
      break;
    }

    if (true == alternate_string.empty()) {
      printf("Daemon::ConnectionListener::SetYSOSLogProperty[Fail][alternate_string is empty]\n");
      break;
    }

    printf("Daemon::ConnectionListener::SetYSOSLogProperty[Check Point][2]\n");

    /// 替换字符
    CString temp_string = (char*)file_data;
    temp_string.Replace(g_log_file_specific_keyword, alternate_string.c_str());
    temp_string.Replace("\\", "/");

    printf("Daemon::ConnectionListener::SetYSOSLogProperty[Check Point][3]\n");

    /// 重新加载log.properties文件，把替换的内容写到文件里
    if (TRUE != file_loader.ResizeFile(temp_string.GetLength())) {
      printf("Daemon::ConnectionListener::SetYSOSLogProperty[Fail][file_loader.ResizeFile()]\n");
      break;
    }

    printf("Daemon::ConnectionListener::SetYSOSLogProperty[Check Point][4]\n");

    /// 获取文件内容
    LPVOID new_file_data = file_loader.GetFileData();
    if (NULL == new_file_data) {
      printf("Daemon::ConnectionListener::SetYSOSLogProperty[Fail][new_file_data is NULL]\n");
      break;
    }

    printf("Daemon::ConnectionListener::SetYSOSLogProperty[Check Point][5]\n");

    ZeroMemory(new_file_data, sizeof(new_file_data));
    if (0 != memcpy_s(new_file_data, temp_string.GetLength(), temp_string.GetBuffer(), temp_string.GetLength())) {
      printf("Daemon::ConnectionListener::SetYSOSLogProperty[Fail][memcpy_s()][last_error_code][%d]\n", GetLastError());
      break;
    }

    printf("Daemon::ConnectionListener::SetYSOSLogProperty[Check Point][6]\n");

    /// 释放加载的new_file_data
    if (TRUE != file_loader.CloseFile(TRUE)) {
      printf("Daemon::ConnectionListener::SetYSOSLogProperty[Fail][file_loader.CloseFile()]\n");
      break;
    }

    printf("Daemon::ConnectionListener::SetYSOSLogProperty[Check Point][End]\n");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  if (YSOS_ERROR_SUCCESS != result) {
    printf("Daemon::ConnectionListener::SetYSOSLogProperty[Fail]\n");
    log_properties_file_path.clear();
  }

  printf("Daemon::ConnectionListener::SetYSOSLogProperty[Exit]\n");
  return result;
}

int ConnectionListener::GetYSOSLogTemplatePropertiesFilePath(std::string& log_template_properties_file_path) {
  printf("Daemon::ConnectionListener::GetYSOSLogTemplatePropertiesFilePath[Enter]\n");
  int result = YSOS_ERROR_FAILED;

  do {
    /// 获取模块所在路径
    TCHAR temp_log_template_properties_file_path[MAX_PATH*2] = {0};
    if (0 == GetModuleFileName(NULL, temp_log_template_properties_file_path, MAX_PATH*2)) {
      printf("Daemon::ConnectionListener::GetYSOSLogTemplatePropertiesFilePath[Fail][GetModuleFileName()][last_error_code][%s]\n", "GetLastError()");
      break;
    }

    printf("Daemon::ConnectionListener::GetYSOSLogTemplatePropertiesFilePath[temp_log_template_properties_file_path][0][%s]\n", temp_log_template_properties_file_path);

    /// 从模块所在路径中删除模块名称
    if (TRUE != PathRemoveFileSpec(temp_log_template_properties_file_path)) {
      printf("Daemon::ConnectionListener::GetYSOSLogTemplatePropertiesFilePath[Fail][PathRemoveFileSpec()][last_error_code][%s]\n", "GetLastError()");
      break;
    }

    printf("Daemon::ConnectionListener::GetYSOSLogTemplatePropertiesFilePath[temp_log_template_properties_file_path][1][%s]\n", temp_log_template_properties_file_path);

    /// 从模块所在路径中删除lib
    if (TRUE != PathRemoveFileSpec(temp_log_template_properties_file_path)) {
      printf("Daemon::ConnectionListener::GetYSOSLogTemplatePropertiesFilePath[Fail][PathRemoveFileSpec()][last_error_code][%s]\n", "GetLastError()");
      break;
    }

    printf("Daemon::ConnectionListener::GetYSOSLogTemplatePropertiesFilePath[temp_log_template_properties_file_path][2][%s]\n", temp_log_template_properties_file_path);

    /// 在模块所在路径中添加模板文件名
    if (TRUE != PathAppend(temp_log_template_properties_file_path, g_log_file_sub_file_path)) {
      printf("Daemon::ConnectionListener::GetYSOSLogTemplatePropertiesFilePath[Fail][PathAppend()][last_error_code][%s]\n", "GetLastError()");
      break;
    }

    printf("Daemon::ConnectionListener::GetYSOSLogTemplatePropertiesFilePath[temp_log_template_properties_file_path][3][%s]\n", temp_log_template_properties_file_path);

    /// 添加日志模板文件扩展名
    if (TRUE != PathAddExtension(temp_log_template_properties_file_path, g_log_file_template_extension)) {
      printf("Daemon::ConnectionListener::GetYSOSLogTemplatePropertiesFilePath[Fail][PathAddExtension()][last_error_code][%s]\n", "GetLastError()");
      break;
    }

    printf("Daemon::ConnectionListener::GetYSOSLogTemplatePropertiesFilePath[temp_log_template_properties_file_path][4][%s]\n", temp_log_template_properties_file_path);

    /// 拷贝拼装的日志模板文件路径
    log_template_properties_file_path = temp_log_template_properties_file_path;

    printf("Daemon::ConnectionListener::GetYSOSLogTemplatePropertiesFilePath[Check Point][End][log_template_properties_file_path][%s]\n", log_template_properties_file_path.c_str());

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  printf("Daemon::ConnectionListener::GetYSOSLogTemplatePropertiesFilePath[Exit]\n");
  return result;
}

int ConnectionListener::CreateYSOSLogPropertiesFile(const std::string& log_template_properties_file_path, std::string& log_properties_file_path) {
  printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[Enter]\n");
  int result = YSOS_ERROR_FAILED;

  do {
    if (true == log_template_properties_file_path.empty()) {
      printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[Fail][log_template_properties_file_path is empty]\n");
      break;
    }

    printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[log_template_properties_file_path][%s]\n", log_template_properties_file_path.c_str());

    /// 判断日志模板文件是否存在
    if (TRUE != PathFileExists(log_template_properties_file_path.c_str())) {
      printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[Fail][PathFileExists()][last_error_code][%s]\n", "GetLastError()");
      break;
    }

    printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[Check Point][0]\n");

    /// 获取系统temp文件路径
    TCHAR temp_path[MAX_PATH] = {0,};
    DWORD return_value = GetTempPath(MAX_PATH, temp_path);
    if (return_value > MAX_PATH || (return_value == 0)) {
      printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[Fail][GetTempPath()][last_error_code][%s]\n", "GetLastError()");
      break;
    }

    printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[temp_path][0][%s]\n", temp_path);

    /// 创建YSOS_DAEMON_LOG文件夹
    if (TRUE != PathAppend(temp_path, g_log_folder_name)) {
      printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[Fail][PathAppend()][last_error_code][%s]\n", "GetLastError()");
      break;
    }

    printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[temp_path][1][%s]\n", temp_path);

    if (TRUE != PathFileExists(temp_path)) {
      printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[Check Point][There is not [%s]]\n", temp_path);
      if (TRUE != CreateDirectory(temp_path, NULL)) {
        printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[Fail][CreateDirectory()][last_error_code][%s]\n", "GetLastError()");
        break;
      }
    }

    printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[temp_path][2][%s]\n", temp_path);

    /// 在YSOS_DAEMON_LOG下创建Log文件夹
    if (TRUE != PathAppend(temp_path, g_log_folder_sub_name)) {
      printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[Fail][The function[PathAppend] is failed][last_error_code][%s]\n", "GetLastError()");
      break;
    }

    printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[temp_path][3][%s]\n", temp_path);

    if (TRUE != PathFileExists(temp_path)) {
      printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[Check Point][There is not [%s]]\n", temp_path);
      if (TRUE != CreateDirectory(temp_path, NULL)) {
        printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[Fail][CreateDirectory()][last_error_code][%s]\n", "GetLastError()");
        break;
      }
    }

    printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[temp_path][4][%s]\n", temp_path);

    if (TRUE != PathRemoveFileSpec(temp_path)) {
      printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[Fail][The function[PathRemoveFileSpec] is failed][last_error_code][%s]\n", "GetLastError()");
      break;
    }

    printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[temp_path][5][%s]\n", temp_path);

    /// 拼装一个新的日志配置文件路径
    TCHAR temp_log_properties_file_path[MAX_PATH] = {0,};
    StringCchPrintf(temp_log_properties_file_path, MAX_PATH, _T("%s\\%s"), temp_path, g_log_file_name);

    printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[temp_log_properties_file_path][%s]\n", temp_log_properties_file_path);

    /// YSOS_DAEMON_LOG下重新拷贝一个新的log.properties
    if (TRUE != CopyFile(log_template_properties_file_path.c_str(), temp_log_properties_file_path, FALSE)) {
      printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[Fail][CopyFile()][last_error_code][%s]\n", "GetLastError()");
      break;
    }

    /// 拷贝拼装的日志配置文件路径
    log_properties_file_path = temp_log_properties_file_path;

    printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[Check Point][End][log_properties_file_path][%s]\n", log_properties_file_path.c_str());

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[Exit]\n");
  return result;
}

BOOL ConnectionListener::GetYSOSLogPropertiesAlternateString(std::string& alternate_string) {
  printf("Daemon::ConnectionListener::GetYSOSLogPropertiesAlternateString[Enter]\n");
  YSOS_LOG_DEBUG("ConnectionListener::GetYSOSLogPropertiesAlternateString[Enter]");
  int result = YSOS_ERROR_FAILED;

  do {
    TCHAR temp_path[MAX_PATH] = {0,};
    DWORD return_value = GetTempPath(MAX_PATH, temp_path);
    if (return_value > MAX_PATH || (return_value == 0)) {
      printf("Daemon::ConnectionListener::GetYSOSLogPropertiesAlternateString[Fail][GetTempPath()][last_error_code][%d]\n", GetLastError());
      YSOS_LOG_DEBUG("ConnectionListener::GetYSOSLogPropertiesAlternateString[Fail][GetTempPath()][last_error_code]["<< GetLastError() <<"]");
      break;
    }

    printf("Daemon::ConnectionListener::GetYSOSLogPropertiesAlternateString[temp_path][0][%s]\n", temp_path);
    YSOS_LOG_DEBUG("ConnectionListener::GetYSOSLogPropertiesAlternateString[temp_path][0]["<< temp_path <<"]");

    if (TRUE != PathAppend(temp_path, g_log_folder_name)) {
      printf("Daemon::ConnectionListener::GetYSOSLogPropertiesAlternateString[Fail][PathAppend()][last_error_code][%s]\n", "GetLastError()");
      YSOS_LOG_DEBUG("ConnectionListener::GetYSOSLogPropertiesAlternateString[Fail][PathAppend()][last_error_code]["<< GetLastError() <<"]");
      break;
    }

    printf("Daemon::ConnectionListener::GetYSOSLogPropertiesAlternateString[temp_path][1][%s]\n", temp_path);
    YSOS_LOG_DEBUG("ConnectionListener::GetYSOSLogPropertiesAlternateString[temp_path][1]["<< temp_path <<"]");

    TCHAR long_temp_path[MAX_PATH] = {0,};
    if (0 == GetLongPathName(temp_path, long_temp_path, MAX_PATH*sizeof(TCHAR))) {
      printf("Daemon::ConnectionListener::GetYSOSLogPropertiesAlternateString[Fail][GetLongPathName()][last_error_code][%s]\n", "GetLastError()");
      YSOS_LOG_DEBUG("ConnectionListener::GetYSOSLogPropertiesAlternateString[Fail][GetLongPathName()][last_error_code]["<< GetLastError() <<"]");
      break;
    }

    printf("Daemon::ConnectionListener::GetYSOSLogPropertiesAlternateString[long_temp_path][%s]\n", long_temp_path);
    YSOS_LOG_DEBUG("ConnectionListener::GetYSOSLogPropertiesAlternateString[long_temp_path]["<< long_temp_path <<"]");

    alternate_string = long_temp_path;

    printf("Daemon::ConnectionListener::GetYSOSLogPropertiesAlternateString[Check Point][End][alternate_string][%s]\n", alternate_string.c_str());
    YSOS_LOG_DEBUG("ConnectionListener::GetYSOSLogPropertiesAlternateString[Check Point][End][alternate_string]["<< alternate_string <<"]");
    result = YSOS_ERROR_SUCCESS;
  } while (0);

  printf("Daemon::ConnectionListener::GetYSOSLogPropertiesAlternateString[Exit]\n");
  YSOS_LOG_DEBUG("ConnectionListener::GetYSOSLogPropertiesAlternateString[Exit]");
  return result;
}
#else
//TODO: add for linux
int ConnectionListener::SetYSOSLogProperty(std::string& log_properties_file_path) {
  printf("Daemon::ConnectionListener::SetYSOSLogProperty[Enter]\n");
  int result = YSOS_ERROR_FAILED;

  do {
    /// 获取日志配置文件模板路径
    std::string log_template_properties_file_path;
    if (YSOS_ERROR_SUCCESS != GetYSOSLogTemplatePropertiesFilePath(log_template_properties_file_path)) {
      printf("Daemon::ConnectionListener::SetYSOSLogProperty[Fail][GetYSOSLogTemplatePropertiesFilePath()]\n");
      break;
    }

    printf("Daemon::ConnectionListener::SetYSOSLogProperty[log_template_properties_file_path][%s]\n", log_template_properties_file_path.c_str());

    /// 重新生成日志配置文件
    if (YSOS_ERROR_SUCCESS != CreateYSOSLogPropertiesFile(log_template_properties_file_path, log_properties_file_path)) {
      printf("Daemon::ConnectionListener::SetYSOSLogProperty[Fail][CreateYSOSLogPropertiesFile()]\n");
      break;
    }

    printf("Daemon::ConnectionListener::SetYSOSLogProperty[log_properties_file_path][%s]\n", log_properties_file_path.c_str());

    /// 加载log文件夹下log.properties文件
    FileLoader file_loader;
    if (TRUE != file_loader.OpenFile(log_properties_file_path.c_str())) {
      printf("Daemon::ConnectionListener::SetYSOSLogProperty[Fail][file_loader.OpenFile()]\n");
      break;
    }

    printf("Daemon::ConnectionListener::SetYSOSLogProperty[Check Point][0]\n");

    /// 获取文件内容
    LPVOID file_data = file_loader.GetFileData();
    if (NULL == file_data) {
      printf("Daemon::ConnectionListener::SetYSOSLogProperty[Fail][file_data is NULL]\n");
      break;
    }

    printf("Daemon::ConnectionListener::SetYSOSLogProperty[Check Point][1]\n");

    /// 获取替换字符
    std::string alternate_string;
    if (YSOS_ERROR_SUCCESS != GetYSOSLogPropertiesAlternateString(alternate_string)) {
      printf("Daemon::ConnectionListener::SetYSOSLogProperty[Fail][GetYSOSLogPropertiesAlternateString()]\n");
      break;
    }

    if (true == alternate_string.empty()) {
      printf("Daemon::ConnectionListener::SetYSOSLogProperty[Fail][alternate_string is empty]\n");
      break;
    }

    printf("Daemon::ConnectionListener::SetYSOSLogProperty[Check Point][2]\n");

    /// 替换字符
    std::string temp_string = (char*)file_data;
    //temp_string.Replace(g_log_file_specific_keyword, alternate_string.c_str());
    //temp_string.Replace("\\", "/");
    string_replace(temp_string, g_log_file_specific_keyword, alternate_string.c_str() );
    string_replace(temp_string, "\\", "/" );
    printf("Daemon::ConnectionListener::SetYSOSLogProperty[Check Point][3]\n");

    /// 重新加载log.properties文件，把替换的内容写到文件里
    if (TRUE != file_loader.ResizeFile(temp_string.length())) {
      printf("Daemon::ConnectionListener::SetYSOSLogProperty[Fail][file_loader.ResizeFile()]\n");
      break;
    }

    printf("Daemon::ConnectionListener::SetYSOSLogProperty[Check Point][4]\n");

    /// 获取文件内容
    LPVOID new_file_data = file_loader.GetFileData();
    if (NULL == new_file_data) {
      printf("Daemon::ConnectionListener::SetYSOSLogProperty[Fail][new_file_data is NULL]\n");
      break;
    }

    printf("Daemon::ConnectionListener::SetYSOSLogProperty[Check Point][5]\n");

    //ZeroMemory(new_file_data, sizeof(new_file_data));
    //if (0 != memcpy_s(new_file_data, temp_string.GetLength(), temp_string.GetBuffer(), temp_string.GetLength())) {
    //TODO: add for linux
    printf( "CysosApp::SetYSOSLogProperty[Check Point][4][%ld]\n",  file_loader.file_size_);
    memset(new_file_data, 0, file_loader.file_size_);//add for linux
    printf( "CysosApp::SetYSOSLogProperty[Check Point][4][%ld]\n",  strlen(temp_string.c_str()));
    memcpy ( new_file_data, temp_string.c_str(), strlen(temp_string.c_str()));
    if ( NULL == new_file_data ) {
      printf("Daemon::ConnectionListener::SetYSOSLogProperty[Fail][memcpy_s()][new_file_data IS NULL]\n");
      break;
    }

    printf("Daemon::ConnectionListener::SetYSOSLogProperty[Check Point][6]\n");

    /// 释放加载的new_file_data
    if (TRUE != file_loader.CloseFile(TRUE)) {
      printf("Daemon::ConnectionListener::SetYSOSLogProperty[Fail][file_loader.CloseFile()]\n");
      break;
    }

    printf("Daemon::ConnectionListener::SetYSOSLogProperty[Check Point][End]\n");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  if (YSOS_ERROR_SUCCESS != result) {
    printf("Daemon::ConnectionListener::SetYSOSLogProperty[Fail]\n");
    log_properties_file_path.clear();
  }

  printf("Daemon::ConnectionListener::SetYSOSLogProperty[Exit]\n");
  return result;
}

int ConnectionListener::GetYSOSLogTemplatePropertiesFilePath(std::string& log_template_properties_file_path) {
  printf("Daemon::ConnectionListener::GetYSOSLogTemplatePropertiesFilePath[Enter]\n");
  int result = YSOS_ERROR_FAILED;
  int MAX_PATH = 260; //add for linux
  do {
    /// 获取模块所在路径
    char temp_log_template_properties_file_path[MAX_PATH*2] = {0};
    if (0 == GetModuleFileName(NULL, temp_log_template_properties_file_path, MAX_PATH*2)) {
      printf("Daemon::ConnectionListener::GetYSOSLogTemplatePropertiesFilePath[Fail][GetModuleFileName()][last_error_code][%s]\n", "GetLastError()");
      break;
    }

    printf("Daemon::ConnectionListener::GetYSOSLogTemplatePropertiesFilePath[temp_log_template_properties_file_path][0][%s]\n", temp_log_template_properties_file_path);

    /// 从模块所在路径中删除模块名称
    if (TRUE != PathRemoveFileSpec(temp_log_template_properties_file_path)) {
      printf("Daemon::ConnectionListener::GetYSOSLogTemplatePropertiesFilePath[Fail][PathRemoveFileSpec()][last_error_code][%s]\n", "GetLastError()");
      break;
    }

    printf("Daemon::ConnectionListener::GetYSOSLogTemplatePropertiesFilePath[temp_log_template_properties_file_path][1][%s]\n", temp_log_template_properties_file_path);

    /// 从模块所在路径中删除lib
    if (TRUE != PathRemoveFileSpec(temp_log_template_properties_file_path)) {
      printf("Daemon::ConnectionListener::GetYSOSLogTemplatePropertiesFilePath[Fail][PathRemoveFileSpec()][last_error_code][%s]\n", "GetLastError()");
      break;
    }

    printf("Daemon::ConnectionListener::GetYSOSLogTemplatePropertiesFilePath[temp_log_template_properties_file_path][2][%s]\n", temp_log_template_properties_file_path);

    /// 在模块所在路径中添加模板文件名
    if (TRUE != PathAppend(temp_log_template_properties_file_path, g_log_file_sub_file_path)) {
      printf("Daemon::ConnectionListener::GetYSOSLogTemplatePropertiesFilePath[Fail][PathAppend()][last_error_code][%s]\n", "GetLastError()");
      break;
    }

    printf("Daemon::ConnectionListener::GetYSOSLogTemplatePropertiesFilePath[temp_log_template_properties_file_path][3][%s]\n", temp_log_template_properties_file_path);

    /// 添加日志模板文件扩展名
    if (TRUE != PathAddExtension(temp_log_template_properties_file_path, g_log_file_template_extension)) {
      printf("Daemon::ConnectionListener::GetYSOSLogTemplatePropertiesFilePath[Fail][PathAddExtension()][last_error_code][%s]\n", "GetLastError()");
      break;
    }

    printf("Daemon::ConnectionListener::GetYSOSLogTemplatePropertiesFilePath[temp_log_template_properties_file_path][4][%s]\n", temp_log_template_properties_file_path);

    /// 拷贝拼装的日志模板文件路径
    log_template_properties_file_path = temp_log_template_properties_file_path;

    printf("Daemon::ConnectionListener::GetYSOSLogTemplatePropertiesFilePath[Check Point][End][log_template_properties_file_path][%s]\n", log_template_properties_file_path.c_str());

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  printf("Daemon::ConnectionListener::GetYSOSLogTemplatePropertiesFilePath[Exit]\n");
  return result;
}

int ConnectionListener::CreateYSOSLogPropertiesFile(const std::string& log_template_properties_file_path, std::string& log_properties_file_path) {
  printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[Enter]\n");
  int result = YSOS_ERROR_FAILED;
  int MAX_PATH = 260; //add fro linux
  do {
    if (true == log_template_properties_file_path.empty()) {
      printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[Fail][log_template_properties_file_path is empty]\n");
      break;
    }

    printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[log_template_properties_file_path][%s]\n", log_template_properties_file_path.c_str());

    /// 判断日志模板文件是否存在
    if (TRUE != PathFileExists(log_template_properties_file_path.c_str())) {
      printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[Fail][PathFileExists()][last_error_code][%s]\n", "GetLastError()");
      break;
    }

    printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[Check Point][0]\n");

    /// 获取系统temp文件路径
    char temp_path[MAX_PATH] = {0,};
    std::string stemp_path = "";
    DWORD return_value = GetTempPath(MAX_PATH, stemp_path);
    if (return_value > MAX_PATH || (return_value == 0)) {
      printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[Fail][GetTempPath()][last_error_code][%s]\n", "GetLastError()");
      break;
    }
    strncpy(temp_path, stemp_path.c_str(), stemp_path.length() + 1);
    printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[temp_path][0][%s]\n", temp_path);

    /// 创建YSOS_DAEMON_LOG文件夹
    if (TRUE != PathAppend(temp_path, g_log_folder_name)) {
      printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[Fail][PathAppend()][last_error_code][%s]\n", "GetLastError()");
      break;
    }

    printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[temp_path][1][%s]\n", temp_path);

    if (TRUE != PathFileExists(temp_path)) {
      printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[Check Point][There is not [%s]]\n", temp_path);

      if (TRUE != CreateDirectory(temp_path, NULL)) {
        printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[Fail][CreateDirectory()][last_error_code][%s]\n", "GetLastError()");
        break;
      }
    }

    printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[temp_path][2][%s]\n", temp_path);

    /// 在YSOS_DAEMON_LOG下创建Log文件夹
    if (TRUE != PathAppend(temp_path, g_log_folder_sub_name)) {
      printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[Fail][The function[PathAppend] is failed][last_error_code][%s]\n", "GetLastError()");
      break;
    }

    printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[temp_path][3][%s]\n", temp_path);

    if (TRUE != PathFileExists(temp_path)) {
      printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[Check Point][There is not [%s]]\n", temp_path);
      if (TRUE != CreateDirectory(temp_path, NULL)) {
        printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[Fail][CreateDirectory()][last_error_code][%s]\n", "GetLastError()");
        break;
      }
    }

    printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[temp_path][4][%s]\n", temp_path);

    if (TRUE != PathRemoveFileSpec(temp_path)) {
      printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[Fail][The function[PathRemoveFileSpec] is failed][last_error_code][%s]\n", "GetLastError()");
      break;
    }

    printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[temp_path][5][%s]\n", temp_path);

    /// 拼装一个新的日志配置文件路径
    char temp_log_properties_file_path[MAX_PATH] = {0,};
    //StringCchPrintf(temp_log_properties_file_path, MAX_PATH, _T("%s\\%s"), temp_path, g_log_file_name);
    sprintf(temp_log_properties_file_path, "%s/%s", temp_path, g_log_file_name); //add for linux

    printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[temp_log_properties_file_path][%s]\n", temp_log_properties_file_path);

    /// YSOS_DAEMON_LOG下重新拷贝一个新的log.properties
    ///* //add for linux
    boost::system::error_code errinfo;// add for linux ///define copy file error info
    if ( TRUE != CopyFile ( log_template_properties_file_path.c_str(), temp_log_properties_file_path, boost::filesystem::copy_option::overwrite_if_exists, errinfo))
    {
        printf( "Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[Fail][CopyFile()][last_error_code][%d]--[%s]\n", errinfo.value(), errinfo.category().name() );
        break;
    }

    /// 拷贝拼装的日志配置文件路径
    log_properties_file_path = temp_log_properties_file_path;

    printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[Check Point][End][log_properties_file_path][%s]\n", log_properties_file_path.c_str());

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  printf("Daemon::ConnectionListener::CreateYSOSLogPropertiesFile[Exit]\n");
  return result;
}

BOOL ConnectionListener::GetYSOSLogPropertiesAlternateString(std::string& alternate_string) {
  printf("Daemon::ConnectionListener::GetYSOSLogPropertiesAlternateString[Enter]\n");
  YSOS_LOG_DEBUG("ConnectionListener::GetYSOSLogPropertiesAlternateString[Enter]");
  int result = YSOS_ERROR_FAILED;
  int MAX_PATH = 260;
  do {
    char temp_path[MAX_PATH] = {0,};
    std::string stemp_path = "";
    DWORD return_value = GetTempPath(MAX_PATH, stemp_path);
    strncpy(temp_path, stemp_path.c_str(), stemp_path.length()+1);
    //TODO:add for linux 
    //strcpy(temp_path, "/tmp");
    //DWORD return_value = sizeof(temp_path)/sizeof(temp_path[0]);
    if (return_value > MAX_PATH || (return_value == 0)) {
      printf("Daemon::ConnectionListener::GetYSOSLogPropertiesAlternateString[Fail][GetTempPath()][last_error_code][%s]\n", "GetLastError()");
      YSOS_LOG_DEBUG("ConnectionListener::GetYSOSLogPropertiesAlternateString[Fail][GetTempPath()][last_error_code]["<< "GetLastError()" <<"]");
      break;
    }

    printf("Daemon::ConnectionListener::GetYSOSLogPropertiesAlternateString[temp_path][0][%s]\n", temp_path);
    YSOS_LOG_DEBUG("ConnectionListener::GetYSOSLogPropertiesAlternateString[temp_path][0]["<< temp_path <<"]");

    if (TRUE != PathAppend(temp_path, g_log_folder_name)) {
      printf("Daemon::ConnectionListener::GetYSOSLogPropertiesAlternateString[Fail][PathAppend()][last_error_code][%s]\n", "GetLastError()");
      YSOS_LOG_DEBUG("ConnectionListener::GetYSOSLogPropertiesAlternateString[Fail][PathAppend()][last_error_code]["<< "GetLastError()" <<"]");
      break;
    }

    printf("Daemon::ConnectionListener::GetYSOSLogPropertiesAlternateString[temp_path][1][%s]\n", temp_path);
    YSOS_LOG_DEBUG("ConnectionListener::GetYSOSLogPropertiesAlternateString[temp_path][1]["<< temp_path <<"]");

    char long_temp_path[MAX_PATH] = {0,};
    //TODO: add for linux
    strcpy(long_temp_path, temp_path);
    /* 
    if (0 == GetLongPathName(temp_path, long_temp_path, MAX_PATH*sizeof(char))) {
      printf("Daemon::ConnectionListener::GetYSOSLogPropertiesAlternateString[Fail][GetLongPathName()][last_error_code][%s]\n", "GetLastError()");
      YSOS_LOG_DEBUG("ConnectionListener::GetYSOSLogPropertiesAlternateString[Fail][GetLongPathName()][last_error_code]["<< "GetLastError()" <<"]");
      break;
    }
    */

    printf("Daemon::ConnectionListener::GetYSOSLogPropertiesAlternateString[long_temp_path][%s]\n", long_temp_path);
    YSOS_LOG_DEBUG("ConnectionListener::GetYSOSLogPropertiesAlternateString[long_temp_path]["<< long_temp_path <<"]");

    alternate_string = long_temp_path;

    printf("Daemon::ConnectionListener::GetYSOSLogPropertiesAlternateString[Check Point][End][alternate_string][%s]\n", alternate_string.c_str());
    YSOS_LOG_DEBUG("ConnectionListener::GetYSOSLogPropertiesAlternateString[Check Point][End][alternate_string]["<< alternate_string <<"]");
    result = YSOS_ERROR_SUCCESS;
  } while (0);

  printf("Daemon::ConnectionListener::GetYSOSLogPropertiesAlternateString[Exit]\n");
  YSOS_LOG_DEBUG("ConnectionListener::GetYSOSLogPropertiesAlternateString[Exit]");
  return result;
}
#endif

int ConnectionListener::RunServer(void) {
  printf("Daemon::ConnectionListener::RunServer[Enter]\n");
  YSOS_LOG_DEBUG("ConnectionListener::RunServer[Enter]");
  int result = YSOS_ERROR_FAILED;

  do {
    /// 平台sdk初始化设置参数
    ysos::ysos_sdk::OpenParams open_params;

    printf("Daemon::ConnectionListener::RunServer[Check Point][0]\n");
    YSOS_LOG_DEBUG("ConnectionListener::RunServer[Check Point][0]");

    /// 修改为服务器模式并设置端口
    open_params.type_ = ysos::ysos_sdk::OpenParams::JSON_RPC;
    open_params.mode_ = YSOS_SDK_SERVER_MODE;
    open_params.port_ = SERVER_PORT;

    printf("Daemon::ConnectionListener::RunServer[Check Point][1]\n");
    YSOS_LOG_DEBUG("ConnectionListener::RunServer[Check Point][1]");

    /// 设置回调函数指针
    open_params.do_service_handler_ = std::bind(&ConnectionListener::UserHandleDoService, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    open_params.on_dispatch_message_handler_ = std::bind(&ConnectionListener::UserHandleOnDispatchMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    open_params.error_handler_ = std::bind(&ConnectionListener::UserHandleError, this, std::placeholders::_1, std::placeholders::_2);

    printf("Daemon::ConnectionListener::RunServer[Check Point][2]\n");
    YSOS_LOG_DEBUG("ConnectionListener::RunServer[Check Point][2]");

    /// 打开服务器
    if (YSOS_ERROR_SUCCESS != ysos::ysos_sdk::OpenYSOSSDK(&open_params)) {
      printf("Daemon::The function[ysos::ysos_sdk::OpenYSOSSDK()] fails.\n");
      YSOS_LOG_DEBUG("ConnectionListener::RunServer[Fail]The function[ysos::ysos_sdk::OpenYSOSSDK()] is failed");
      result = YSOS_ERROR_LOGIC_ERROR;
      break;
    }

    printf("Daemon::ConnectionListener::RunServer[Check Point][End]\n");
    YSOS_LOG_DEBUG("ConnectionListener::RunServer[Check Point][End]");
    result = YSOS_ERROR_SUCCESS;
  } while (0);

  if (YSOS_ERROR_SUCCESS != result) {
    printf("Daemon::ConnectionListener::RunServer[Error]\n");

    /// 关闭服务器
    if (YSOS_ERROR_SUCCESS != StopServer()) {
      printf("Daemon::The function[StopServer()] fails.\n");
    }
  }

  printf("Daemon::ConnectionListener::RunServer[Exit]\n");
  YSOS_LOG_DEBUG("ConnectionListener::RunServer[Exit]");
  return result;
}

int ConnectionListener::StopServer(void) {
  printf("Daemon::ConnectionListener::StopServer[Enter]\n");
  YSOS_LOG_DEBUG("ConnectionListener::StopServer[Enter]");
  int result = YSOS_ERROR_FAILED;

  do {
    /// 关闭服务器
    if (YSOS_ERROR_SUCCESS != ysos::ysos_sdk::CloseYSOSSDK()) {
      printf("Daemon::ConnectionListener::StopServer[Fail]The function[ysos::ysos_sdk::CloseYSOSSDK()] is failed\n");
      YSOS_LOG_DEBUG("ConnectionListener::StopServer[Fail]The function[ysos::ysos_sdk::CloseYSOSSDK()] is failed");
      result = YSOS_ERROR_LOGIC_ERROR;
    }

    printf("Daemon::ConnectionListener::StopServer[Check Point][End]\n");
    printf("Daemon::ConnectionListener::StopServer[Check Point][End]");
    result = YSOS_ERROR_SUCCESS;
  } while (0);

  printf("Daemon::ConnectionListener::StopServer[Exit]\n");
  YSOS_LOG_DEBUG("ConnectionListener::StopServer[Exit]");
  return result;
}

int ConnectionListener::Start(void) {
  printf("Daemon::ConnectionListener::Start[Enter]\n");
  int result = YSOS_ERROR_FAILED;

  do {
    /// 判断运行状态
    if (IS_RUNNING == GetRunningStatus()) {
      result = YSOS_ERROR_SUCCESS;
      printf("Daemon::ConnectionListener::Start[Check Point]ConnectionListener is already running\n");
      break;
    }

    printf("Daemon::ConnectionListener::Start[Check Point][0]\n");

    /// 初始化参数
    if (YSOS_ERROR_SUCCESS != Init()) {
      printf("Daemon::ConnectionListener::Start[Fail]The function[Init()] is failed\n");
      result = YSOS_ERROR_LOGIC_ERROR;
      break;
    }

    printf("Daemon::ConnectionListener::Start[Check Point][1]\n");
    YSOS_LOG_DEBUG("ConnectionListener::Start[Check Point][1]");

    /// 开启服务器模式
    if (YSOS_ERROR_SUCCESS != RunServer()) {
      printf("Daemon::ConnectionListener::Start[Fail]The function[RunServer()] is failed\n");
      YSOS_LOG_DEBUG("ConnectionListener::Start[Fail]The function[RunServer()] is failed");
      result = YSOS_ERROR_LOGIC_ERROR;
      break;
    }

    printf("Daemon::ConnectionListener::Start[Check Point][2]\n");
    YSOS_LOG_DEBUG("ConnectionListener::Start[Check Point][2]");

    /// 设置为运行状态
    SetRunningStatus(IS_RUNNING);

    printf("Daemon::ConnectionListener::Start[Check Point][End]\n");
    YSOS_LOG_DEBUG("ConnectionListener::Start[Check Point][End]");
    result = YSOS_ERROR_SUCCESS;
  } while (0);

  printf("Daemon::ConnectionListener::Start[Exit]\n");
  YSOS_LOG_DEBUG("ConnectionListener::Start[Exit]");
  return result;
}

int ConnectionListener::Stop(void) {
  printf("Daemon::ConnectionListener::Stop[Enter]\n");
  YSOS_LOG_DEBUG("ConnectionListener::Stop[Enter]");
  int result = YSOS_ERROR_FAILED;

  do {
    /// 判断运行状态
    if (IS_STOPPED == GetRunningStatus()) {
      result = YSOS_ERROR_SUCCESS;
      printf("Daemon::ConnectionListener::Stop[Check Point]ProcessMonitor is already stopped\n");
      YSOS_LOG_DEBUG("ConnectionListener::Stop[Check Point]ProcessMonitor is already stopped");
      break;
    }

    printf("Daemon::ConnectionListener::Stop[Check Point][0]\n");
    YSOS_LOG_DEBUG("ConnectionListener::Stop[Check Point][0]");

    /// 设置为停止状态
    SetRunningStatus(IS_STOPPED);

    printf("Daemon::ConnectionListener::Stop[Check Point][1]\n");
    YSOS_LOG_DEBUG("ConnectionListener::Stop[Check Point][1]");

    /// 关闭服务器
    if (YSOS_ERROR_SUCCESS != StopServer()) {
      printf("Daemon::ConnectionListener::Stop[Fail]The function[StopServer()] is failed\n");
      YSOS_LOG_DEBUG("ConnectionListener::Stop[Fail]The function[StopServer()] is failed");
    }

    printf("Daemon::ConnectionListener::Stop[Check Point][End]\n");
    YSOS_LOG_DEBUG("ConnectionListener::Stop[Check Point][End]");
    result = YSOS_ERROR_SUCCESS;
  } while (0);

  printf("Daemon::ConnectionListener::Stop[Exit]\n");
  YSOS_LOG_DEBUG("ConnectionListener::Stop[Exit]");
  return result;
}


/**
*@brief ProcessMonitor的具体实现  // NOLINT
*/
#ifdef _DEBUG
static const char* g_ysos_daemon_xml_file_name = "ysos_daemon_d.xml";
#else
static const char* g_ysos_daemon_xml_file_name = "ysos_daemon.xml";
#endif
static const char* g_xml_root_element_name = "process_info_list";
static const char* g_xml_child_element_name = "process_info";
static const char* g_xml_child_element_attribute_tag = "tag";
static const char* g_xml_child_element_attribute_dependency = "dependency";
static const char* g_xml_child_element_attribute_path = "path";
static const char* g_xml_child_element_attribute_command_line = "command_line";
static const char* g_xml_child_element_attribute_delay_time = "delay_time";
static const char* g_xml_child_element_attribute_hide_window = "hide_window";
static const char* g_xml_child_element_attribute_call_times = "call_times";
static const char* g_xml_child_element_attribute_type = "type";

//
static const char* g_xml_child_element_flash_name = "process_flash_info";
static const char* g_xml_child_element_flash_attribute_enable = "enable";
static const char* g_xml_child_element_flash_attribute_address = "ip";
static const char* g_xml_child_element_flash_attribute_port = "port";

//
static const char* g_xml_child_element_explorer_name = "process_explorer_info";

ProcessMonitor::ProcessMonitor(const MediatorPtr& mediator_ptr) :
  Colleague(mediator_ptr),
  running_status_(IS_INITIAL),
  monitoring_status_(IS_INITIAL) {
  enable_flash_server_ = true;
  kill_explorer_ = false;
  flash_listen_port_ = 843;
  listen_address_ = "127.0.0.1";
  flash_listen_server_ = NULL;
  kill_explorer_path_ ="";
}

ProcessMonitor::~ProcessMonitor(void) {

}

int ProcessMonitor::Send(const ExternMessage& message) {
  printf("Daemon::ProcessMonitor::Send[Enter]\n");
  YSOS_LOG_DEBUG("ProcessMonitor::Send[Enter]");
  if (YSOS_ERROR_SUCCESS != mediator_ptr_->Send(message, shared_from_this())) {
    printf("Daemon::ProcessMonitor::Send[Fail]The function[mediator_ptr_->Send()] is failed\n");
    YSOS_LOG_DEBUG("ProcessMonitor::Send[Fail][The function[mediator_ptr_->Send()] is failed]");
    return YSOS_ERROR_FAILED;
  }
  printf("Daemon::ProcessMonitor::Send[Exit]\n");
  YSOS_LOG_DEBUG("ProcessMonitor::Send[Exit]");
  return YSOS_ERROR_SUCCESS;
}

int ProcessMonitor::Notify(const ExternMessage& message) {
  printf("Daemon::ProcessMonitor::Notify[Enter]\n");
  YSOS_LOG_DEBUG("ProcessMonitor::Notify[Enter]");
  int result = YSOS_ERROR_FAILED;

  do {
    switch (message.id_) {
    case Daemon::MSG_START: {
      printf("Daemon::ProcessMonitor::Notify[message.id_][MSG_START]\n");
      YSOS_LOG_DEBUG("ProcessMonitor::Notify[message.id_][MSG_START]");
      /// 启动模块
      if (YSOS_ERROR_SUCCESS != Start()) {
        printf("Daemon::ProcessMonitor::Notify[Fail]The function[Start()] is failed\n");
        YSOS_LOG_DEBUG("ProcessMonitor::Notify[Fail]The function[Start()] is failed");
        result = YSOS_ERROR_LOGIC_ERROR;
      }
      result = YSOS_ERROR_SUCCESS;
    }
    break;
    case Daemon::MSG_STOP: {
      printf("Daemon::ProcessMonitor::Notify[message.id_][MSG_STOP]\n");
      YSOS_LOG_DEBUG("ProcessMonitor::Notify[message.id_][MSG_STOP]");
      /// 关闭模块
      if (YSOS_ERROR_SUCCESS != Stop()) {
        printf("Daemon::ProcessMonitor::Notify[Fail]The function[Stop()] is failed\n");
        YSOS_LOG_DEBUG("ProcessMonitor::Notify[Fail]The function[Stop()] is failed");
        result = YSOS_ERROR_LOGIC_ERROR;
      }
      result = YSOS_ERROR_SUCCESS;
    }
    break;
    case Daemon::MSG_YSOS_IS_STARTED: {
      printf("Daemon::ProcessMonitor::Notify[message.id_][MSG_YSOS_IS_STARTED]\n");
      YSOS_LOG_DEBUG("ProcessMonitor::Notify[message.id_][MSG_YSOS_IS_STARTED]");

      result = YSOS_ERROR_SUCCESS;
    }
    break;
    case Daemon::MSG_KILL_PROCESS: {
      printf("Daemon::ProcessMonitor::Notify[message.id_][MSG_KILL_PROCESS]\n");
      YSOS_LOG_DEBUG("ProcessMonitor::Notify[message.id_][MSG_KILL_PROCESS]");

      /// 杀死指定进程
      for (unsigned ii = 0; ii < message.contents_.size(); ++ii) {
        if (YSOS_ERROR_SUCCESS != TerminateProcess(message.contents_[ii])) {
          printf("Daemon::ProcessMonitor::Notify[Fail]The function[TerminateProcess()] is failed\n");
          YSOS_LOG_DEBUG("ProcessMonitor::Notify[Fail]The function[TerminateProcess()] is failed");
          result = YSOS_ERROR_LOGIC_ERROR;
        }
      }
      result = YSOS_ERROR_SUCCESS;
    }
    break;
    default:
      printf("Daemon::ProcessMonitor::Notify[message.id_][%d]\n", message.id_);
      YSOS_LOG_DEBUG("ProcessMonitor::Notify[message.id_]["<< message.id_ <<"]");
      result = YSOS_ERROR_NOT_SUPPORTED;
      break;
    }

    if (YSOS_ERROR_SUCCESS != result) {
      printf("Daemon::ProcessMonitor::Notify[Fail]\n");
      YSOS_LOG_DEBUG("ProcessMonitor::Notify[Fail]");
      break;
    }

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  printf("Daemon::ProcessMonitor::Notify[Exit]\n");
  YSOS_LOG_DEBUG("ProcessMonitor::Notify[Exit]");
  return result;
}

int ProcessMonitor::Init(const std::string& conf_file_path /* = "" */) {
  printf("Daemon::ProcessMonitor::Init[Enter]\n");
  int result = YSOS_ERROR_FAILED;

  do {
    /// 初始化本地日志
    if (true != logger_.valid()) {
      printf("Daemon::Daemon::Init[Check Point][The function[logger_.valid()] is failed]\n");
      /// 初始化日志文件路径
      if (true != log::InitLoggerExt("../log/daemon.log", "daemon")) {
        printf("Daemon::Daemon::Init[Fail][The function[log::InitLoggerExt()] is failed]\n");
        return YSOS_ERROR_FAILED;
      }

      printf("Daemon::Daemon::Init[Check Point][0]\n");

      /// 判断logger是否可用
      logger_ = ysos::log::GetLogger("daemon");
      if (true != logger_.valid()) {
        printf("Daemon::Daemon::Init[Fail][The function[logger_.valid()] is failed]\n");
        return YSOS_ERROR_FAILED;
      }

      printf("Daemon::Daemon::Init[Check Point][1]\n");
    }

    printf("Daemon::Daemon::Init[Check Point][2]\n");

    /// 解析配置文件
    if (true == process_info_map_.empty()) {
      BoostLockGuard lock(shared_mutex_);
      if (YSOS_ERROR_SUCCESS != ParseConfFile(conf_file_path)) {
        printf("Daemon::The function[ParseConfFile()] fails.\n");
        result = YSOS_ERROR_LOGIC_ERROR;
        break;
      }
    }

    printf("Daemon::Daemon::Init[Check Point][End]\n");
    result = YSOS_ERROR_SUCCESS;
  } while (0);

  printf("Daemon::ProcessMonitor::Init[Exit]\n");

  return result;
}

int ProcessMonitor::ParseConfFile(const std::string& conf_file_path) {
  printf("Daemon::ProcessMonitor::ParseConfFile[Enter]\n");
  YSOS_LOG_DEBUG("ProcessMonitor::ParseConfFile[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {
    std::string temp_conf_file_path;
    if (true == conf_file_path.empty()) {
      YSOS_LOG_DEBUG("ProcessMonitor::ParseConfFile[Check Point][conf_file_path is empty]");
#if _WIN32
      TCHAR module_file_name[MAX_PATH*2] = {0};
      if (0 == GetModuleFileName(NULL, module_file_name, MAX_PATH*2)) {
        PrintDSA("ysos_daemon", "InitInstance[Fail][GetModuleFileName()][last_error_code][%d]", GetLastError());
        YSOS_LOG_DEBUG("ProcessMonitor::InitInstance[Fail][The function[GetModuleFileName()] is failed]["<< GetLastError() <<"]");
        break;
      }

      YSOS_LOG_DEBUG("ProcessMonitor::ParseConfFile[Check Point][0]");

      if (TRUE != PathRemoveFileSpec(module_file_name)) {
        PrintDSA("ysos_daemon", "InitInstance[Fail][PathRemoveFileSpec()][last_error_code][%d]", GetLastError());
        YSOS_LOG_DEBUG("ProcessMonitor::InitInstance[Fail][The function[PathRemoveFileSpec()] is failed]["<< GetLastError() <<"]");
        break;
      }

      YSOS_LOG_DEBUG("ProcessMonitor::ParseConfFile[Check Point][1]");

      temp_conf_file_path = module_file_name;
      temp_conf_file_path.append("\\");
      temp_conf_file_path.append(g_ysos_daemon_xml_file_name);
#else
//TODO: add for linux
      char module_file_name[260*2] = {0};
      if (0 == GetModuleFileName(NULL, module_file_name, 260*2)) {
        printf("ysos_daemon::InitInstance[Fail][GetModuleFileName()][last_error_code][%s]\n", "GetLastError()");
        YSOS_LOG_DEBUG("ProcessMonitor::InitInstance[Fail][The function[GetModuleFileName()] is failed]["<< "GetLastError()" <<"]");
        break;
      }

      YSOS_LOG_DEBUG("ProcessMonitor::ParseConfFile[Check Point][0]");

      if (TRUE != PathRemoveFileSpec(module_file_name)) {
        printf("ysos_daemon::InitInstance[Fail][PathRemoveFileSpec()][last_error_code][%s]\n", "GetLastError()");
        YSOS_LOG_DEBUG("ProcessMonitor::InitInstance[Fail][The function[PathRemoveFileSpec()] is failed]["<< "GetLastError()" <<"]");
        break;
      }

      YSOS_LOG_DEBUG("ProcessMonitor::ParseConfFile[Check Point][1]");

      temp_conf_file_path = module_file_name;
      temp_conf_file_path.append("/");
      temp_conf_file_path.append(g_ysos_daemon_xml_file_name);
#endif
    } else {
      YSOS_LOG_DEBUG("ProcessMonitor::ParseConfFile[Check Point][conf_file_path is not empty]");
      temp_conf_file_path = conf_file_path;
    }

    printf("Daemon::temp_conf_file_path[%s]\n", temp_conf_file_path.c_str());
    YSOS_LOG_DEBUG("ProcessMonitor::ParseConfFile[Check Point][temp_conf_file_path]["<< temp_conf_file_path <<"]");

    FilePath file_path(temp_conf_file_path);
    if (!file_path.IsFileExist()) {
      printf("Daemon::The file is not exist[%s]\n", temp_conf_file_path.c_str());
      YSOS_LOG_DEBUG("ProcessMonitor::ParseConfFile[Fail][The file is not exist][temp_conf_file_path]["<< temp_conf_file_path <<"]");
      break;
    }

    YSOS_LOG_DEBUG("ProcessMonitor::ParseConfFile[Check Point][2]");

	tinyxml2::XMLDocument xml_document_flashserver;
	tinyxml2::XMLElement* xml_element_flashserver = GetXmlUtil()->GetRootElement(&xml_document_flashserver, GetXmlUtil()->GetXmlStringFromFile(temp_conf_file_path), g_xml_child_element_flash_name);
	if(NULL != xml_element_flashserver) {
		std::string strenable  = GetXmlUtil()->GetElementTextAttribute(xml_element_flashserver, g_xml_child_element_flash_attribute_enable);
		if (true == strenable.empty()) {
			printf("Daemon::enable flash is empty.\n");
			YSOS_LOG_DEBUG("ProcessMonitor::ParseConfFile[Check Point][enable flash is empty.]");
		} else {
			YSOS_LOG_DEBUG("ProcessMonitor::ParseConfFile[Check Point][Enable Flash]:"<<strenable.c_str());
			if( strenable == "false"|| strenable == "FALSE") {
				enable_flash_server_ = false;
			}else {
				enable_flash_server_ = true;
			}
		}

		std::string straddress = GetXmlUtil()->GetElementTextAttribute(xml_element_flashserver, g_xml_child_element_flash_attribute_address);
		if (true == straddress.empty()) {
			printf("Daemon::straddress is empty.\n");
			YSOS_LOG_DEBUG("ProcessMonitor::ParseConfFile[Check Point][straddress is empty]");
		} else{
			listen_address_ = straddress;
			YSOS_LOG_DEBUG("ProcessMonitor::ParseConfFile[Check Point][Flash Address]:"<<listen_address_.c_str());
        }

		std::string strport = GetXmlUtil()->GetElementTextAttribute(xml_element_flashserver, g_xml_child_element_flash_attribute_port);
		if (true == strport.empty()) {
			printf("Daemon::port is empty.\n");
			YSOS_LOG_DEBUG("ProcessMonitor::ParseConfFile[Check Point][port is empty]");
		} else {
			flash_listen_port_ = atoi(strport.c_str());
			YSOS_LOG_DEBUG("ProcessMonitor::ParseConfFile[Check Point][Flash Port]:"<<flash_listen_port_);
		}
	}
	YSOS_LOG_DEBUG("ProcessMonitor::ParseConfFile[Check Point][3]");

	//
	tinyxml2::XMLDocument xml_document_explorer;
	tinyxml2::XMLElement* xml_element_explorer = GetXmlUtil()->GetRootElement(&xml_document_explorer, GetXmlUtil()->GetXmlStringFromFile(temp_conf_file_path), g_xml_child_element_explorer_name);
	if(NULL != xml_element_explorer) {
		std::string strenable  = GetXmlUtil()->GetElementTextAttribute(xml_element_explorer, g_xml_child_element_flash_attribute_enable);
		if (true == strenable.empty()) {
			printf("Daemon::enable kill explorer is empty.\n");
			YSOS_LOG_DEBUG("ProcessMonitor::ParseConfFile[Check Point][enable kill explorer is empty.]");
		} else {
			YSOS_LOG_DEBUG("ProcessMonitor::ParseConfFile[Check Point][Enable Flash]:"<<strenable.c_str());
			if( strenable == "false"|| strenable == "FALSE") {
				kill_explorer_ = false;
			}else {
				kill_explorer_ = true;
			}
		}

		std::string strpath = GetXmlUtil()->GetElementTextAttribute(xml_element_explorer, g_xml_child_element_attribute_path);
		if (true == strpath.empty()) {
			printf("Daemon::path is empty.\n");
			YSOS_LOG_DEBUG("ProcessMonitor::ParseConfFile[Check Point][path is empty]");
		} else{
			kill_explorer_path_ = strpath;
			YSOS_LOG_DEBUG("ProcessMonitor::ParseConfFile[Check Point][Kill Explorer Path]:"<<kill_explorer_path_.c_str());
		}
	}
	YSOS_LOG_DEBUG("ProcessMonitor::ParseConfFile[Check Point][4]");

    /// 解析ysos_daemon.xml
    process_info_map_.clear();

    ProcessInfo process_info;
    tinyxml2::XMLDocument xml_document;
    tinyxml2::XMLElement* xml_element = GetXmlUtil()->GetRootElement(&xml_document, GetXmlUtil()->GetXmlStringFromFile(temp_conf_file_path), g_xml_child_element_name);
    while (NULL != xml_element) {
      process_info.Clear();

      process_info.tag = GetXmlUtil()->GetElementTextAttribute(xml_element, g_xml_child_element_attribute_tag);
      if (true == process_info.tag.empty()) {
        printf("Daemon::tag is empty.\n");
        YSOS_LOG_DEBUG("ProcessMonitor::ParseConfFile[Check Point][tag is empty]");
      }

      process_info.dependency = GetXmlUtil()->GetElementTextAttribute(xml_element, g_xml_child_element_attribute_dependency);
      if (true == process_info.dependency.empty()) {
        printf("Daemon::dependency is empty.\n");
        YSOS_LOG_DEBUG("ProcessMonitor::ParseConfFile[Check Point][dependency is empty]");
      }

      process_info.path = GetXmlUtil()->GetElementTextAttribute(xml_element, g_xml_child_element_attribute_path);
      if (true == process_info.path.empty()) {
        printf("Daemon::path is empty.\n");
        YSOS_LOG_DEBUG("ProcessMonitor::ParseConfFile[Check Point][path is empty]");
      }

      process_info.command_line = GetXmlUtil()->GetElementTextAttribute(xml_element, g_xml_child_element_attribute_command_line);
      if (true == process_info.command_line.empty()) {
        printf("Daemon::command_line is empty.\n");
        YSOS_LOG_DEBUG("ProcessMonitor::ParseConfFile[Check Point][command_line is empty]");
      }

      process_info.delay_time = GetXmlUtil()->GetElementTextAttribute(xml_element, g_xml_child_element_attribute_delay_time);
      if (true == process_info.delay_time.empty()) {
        printf("Daemon::delay_time is empty.\n");
        YSOS_LOG_DEBUG("ProcessMonitor::ParseConfFile[Check Point][delay_time is empty]");
      }

      process_info.hide_window = GetXmlUtil()->GetElementTextAttribute(xml_element, g_xml_child_element_attribute_hide_window);
      if (true == process_info.hide_window.empty()) {
        printf("Daemon::hide_window is empty.\n");
        YSOS_LOG_DEBUG("ProcessMonitor::ParseConfFile[Check Point][hide_window is empty]");
      }

      process_info.call_times = GetXmlUtil()->GetElementTextAttribute(xml_element, g_xml_child_element_attribute_call_times);
      if (true == process_info.call_times.empty()) {
        printf("Daemon::call_times is empty.\n");
        YSOS_LOG_DEBUG("ProcessMonitor::ParseConfFile[Check Point][call_times is empty]");
      }

      process_info.type = GetXmlUtil()->GetElementTextAttribute(xml_element, g_xml_child_element_attribute_type);
      if (true == process_info.type.empty()) {
        printf("Daemon::type is empty.\n");
        YSOS_LOG_DEBUG("ProcessMonitor::ParseConfFile[Check Point][type is empty]");
      }

      //process_info_map_.insert(std::make_pair<std::string, ProcessInfo>(process_info.tag, process_info));//
      process_info_map_.insert(std::make_pair(process_info.tag, process_info));//add for linux  //使用 std :: make_pair 你不应该明确指定模板参数

      xml_element = xml_element->NextSiblingElement(g_xml_child_element_name);
    }

    printf("Daemon::ProcessMonitor::ParseConfFile[End]\n");
    result = YSOS_ERROR_SUCCESS;
  } while (0);

  printf("Daemon::ProcessMonitor::ParseConfFile[Exit]\n");
  YSOS_LOG_DEBUG("ProcessMonitor::ParseConfFile[Exit]");
  return result;
}

void ProcessMonitor::MonitorProcess() {
  printf("Daemon::ProcessMonitor::MonitorProcess[Enter]\n");
  YSOS_LOG_DEBUG("ProcessMonitor::MonitorProcess[Enter]");

  bool all_is_running = false;
  unsigned main_process_call_times = 0;
  unsigned main_process_failed_call_times = 0;
  std::string main_process_path;
  while (IS_STOPPED != GetRunningStatus()) {

    /// 查找所有有依赖的进程信息
    for (ProcessInfoMapIterator itr = process_info_map_.begin();itr != process_info_map_.end(); ++itr) {
      /// 没有依赖的进程跳过
      if (true == itr->second.dependency.empty()) {
        // printf("Daemon::Skip non dependency process[%s]", itr->second.path.c_str());
        // YSOS_LOG_DEBUG("Skip non dependency process["<< itr->second.path.c_str() <<"]");
        continue;
      }

      /// 检测是否所有依赖进程已运行
      all_is_running = false;
      main_process_call_times = 0;
      main_process_failed_call_times = 0;
      main_process_path.clear();
      if (0 != CheckDependencyProcessRunningStatus(itr->second.dependency, all_is_running, main_process_call_times, main_process_failed_call_times, main_process_path)) {
        printf("Daemon::ProcessMonitor::MonitorProcess[Fail][The function[CheckDependencyProcessRunningStatus()] is failed]\n");
        YSOS_LOG_DEBUG("ProcessMonitor::MonitorProcess[Fail][The function[CheckDependencyProcessRunningStatus()] is failed]");
        continue;
      }

      /// 依赖进程没有全部已运行
      if (false == all_is_running) {

		g_ysos_state = 0;

        // printf("Daemon::ProcessMonitor::MonitorProcess[dependency process is not running][%s]", itr->second.dependency.c_str());
        // YSOS_LOG_DEBUG("ProcessMonitor::MonitorProcess[dependency process is not running][" << itr->second.dependency << "]");

        /// 判断是否是ui进程，如果是ui进程，判断是否主进程超出最大拉起次数，超出将不再杀死ui进程，将通过ui进程展示错误
        if (true != itr->second.type.empty()) {
          unsigned process_type = std::stoul(itr->second.type);
          if (0 == process_type) {
            /// 不再杀死
            if (main_process_failed_call_times == (main_process_call_times+2)) {
              // printf("Daemon::ProcessMonitor::MonitorProcess[don't kill ui process][%s]", itr->second.path.c_str());
              // YSOS_LOG_DEBUG("ProcessMonitor::MonitorProcess[don't kill ui process][" << itr->second.path << "]");
              boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
              continue;
            }
            /// 最后一次拉起后被杀，则再杀一次
            else if (main_process_failed_call_times == main_process_call_times+1) {
              printf("Daemon::ProcessMonitor::MonitorProcess[kill ui process][%s]\n", itr->second.path.c_str());
              YSOS_LOG_DEBUG("ProcessMonitor::MonitorProcess[kill ui process][" << itr->second.path << "]");
              if (0 != SetMainProcessFailedCallTimes(main_process_failed_call_times+1)) {
                printf("Daemon::ProcessMonitor::MonitorProcess[Fail][The function[CheckDependencyProcessRunningStatus()] is failed]\n");
                YSOS_LOG_DEBUG("ProcessMonitor::MonitorProcess[Fail][The function[CheckDependencyProcessRunningStatus()] is failed]");
                boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
                continue;
              }
            }
          }
        }

        /// 杀死该有依赖的进程
        //  printf("Daemon::Before kill the process[%s].", itr->second.path.c_str());
        //  YSOS_LOG_DEBUG("Before kill the process["<< itr->second.path.c_str() <<"]");

        if (TRUE == GetProcessWrapper()->IsRuningProcessEx(itr->second.path.c_str())) {
          if (0 != GetProcessWrapper()->KillProcessEx(itr->second.path.c_str())) {
            printf("Daemon::Can't kill the process[%s].\n", itr->second.path.c_str());
            YSOS_LOG_DEBUG("Can't kill the process["<< itr->second.path.c_str() <<"]");
          }
        }

        //printf("Daemon::After kill the process[%s].", itr->second.path.c_str());
        //YSOS_LOG_DEBUG("After kill the process["<< itr->second.path.c_str() <<"]");
      }

    } /// for (ProcessInfoMapIterator itr = process_info_map_.begin(); itr != process_info_map_.end(); ++itr)

    /// 线程切换
    boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
  }

  printf("Daemon::ProcessMonitor::MonitorProcess[Exit]\n");
  YSOS_LOG_DEBUG("ProcessMonitor::MonitorProcess[Exit]");
  return;
}

void ProcessMonitor::RunMonitorThreades() {
  printf("Daemon::ProcessMonitor::RunMonitorThreades[Enter]\n");
  YSOS_LOG_DEBUG("ProcessMonitor::RunMonitorThreades[Enter]");

  for (ProcessInfoMapIterator itr = process_info_map_.begin();
       itr != process_info_map_.end();
       ++itr) {
    YSOS_LOG_DEBUG("[process_info][" << std::hex << &(itr->second) << "]");
    run_process_thread_group_.create_thread(boost::bind(&ProcessMonitor::RunProcess, this, itr->second));
  }

  monitor_thread_ = boost::thread(boost::bind(&ProcessMonitor::MonitorProcess, this));

  printf("Daemon::ProcessMonitor::RunMonitorThreades[Exit]\n");
  YSOS_LOG_DEBUG("ProcessMonitor::RunMonitorThreades[Exit]");
  return;
}

void ProcessMonitor::RunProcess(ProcessInfo& process_info) {
  printf("Daemon::ProcessMonitor::RunProcess[Enter]\n");
  YSOS_LOG_DEBUG("ProcessMonitor::RunProcess[Enter]");
  YSOS_LOG_DEBUG("ProcessMonitor::RunProcess[process][" << process_info.path << "]");
  YSOS_LOG_DEBUG("[process_info][" << std::hex << &(process_info) << "]");

  HANDLE* process_handle_ptr = new HANDLE;
  if (NULL == process_handle_ptr) {
    printf("Daemon::ProcessMonitor::RunProcess[process_handle_ptr is NULL]\n");
    YSOS_LOG_DEBUG("ProcessMonitor::RunProcess[process_handle_ptr is NULL]");
    return;
  }

  bool process_is_running = false;
  bool all_is_running = false;
  unsigned main_process_call_times = 0;
  unsigned main_process_failed_call_times = 0;
  std::string main_process_path;
  int result = -1;
  int check_Cnt = 0;

  while (IS_STOPPED != GetRunningStatus()) {
    //printf("Daemon::ProcessMonitor::RunProcess[process][%s]", process_info.path.c_str());
    //YSOS_LOG_DEBUG("ProcessMonitor::RunProcess[process][" << process_info.path << "]");

    /// 该进程是有依赖的进程
    if (true != process_info.dependency.empty()) {
      //YSOS_LOG_DEBUG("ProcessMonitor::RunProcess[process] int dependency!");

      /// 检测是否该进程的所有依赖进程已运行
      all_is_running = false;
      main_process_call_times = 0;
      main_process_failed_call_times = 0;
      main_process_path.clear();
      if (0 != CheckDependencyProcessRunningStatus(process_info.dependency, all_is_running, main_process_call_times, main_process_failed_call_times, main_process_path)) {
        printf("Daemon::ProcessMonitor::RunProcess[Fail][The function[CheckDependencyProcessRunningStatus()] is failed]\n");
        YSOS_LOG_DEBUG("ProcessMonitor::RunProcess[Fail][The function[CheckDependencyProcessRunningStatus()] is failed]");
        boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
        continue;
      }

      /// 该进程的依赖进程没有全部已运行
      if (false == all_is_running) {
        //printf("Daemon::ProcessMonitor::RunProcess[dependency process is not running][%s]", process_info.dependency.c_str());
        YSOS_LOG_DEBUG("ProcessMonitor::RunProcess[dependency process is not running][" << process_info.dependency << "]");

        /// 判断是ui程序
        if (true != process_info.type.empty()) {
          unsigned process_type = std::stoul(process_info.type);
          if (0 == process_type) {
            ///  判断主进程死掉的次数是最大拉起次数+2，即如果最大拉起次数是2，则最后拉起后该失败次数将是2+1=3，这时会被MonitorProcess线程杀死
            if (main_process_failed_call_times == (main_process_call_times+2)) {
              printf("Daemon::ProcessMonitor::RunProcess[call ui process][%s]\n", process_info.path.c_str());
              YSOS_LOG_DEBUG("ProcessMonitor::RunProcess[call ui process][" << process_info.path << "]");
              goto PP_RUN;
            }
          }
        }

        boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
        continue;
      }

	  if( g_ysos_state == 0 && check_Cnt < 50 ){
        if(check_Cnt == 0)YSOS_LOG_DEBUG("ProcessMonitor::RunProcess[YSOS Not Run Finish]");
        check_Cnt++;
	    boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
		continue;
	  }
    }

    /// 该进程是无依赖进程
PP_RUN:
	YSOS_LOG_DEBUG("ProcessMonitor::RunProcess[process] 0!");

    /// 设置了重新拉起次数
    if (true != process_info.call_times.empty()) {
      unsigned call_times = std::stoul(process_info.call_times);
      /// 判断次数是否已超出重新拉起次数，如果是2，则0，1，2 运行3次，0次是默认拉起，1，2是重新拉起次数
      if (call_times < process_info.failed_call_times) {
        // printf("Daemon::ProcessMonitor::RunProcess[call_times][%d][failed_call_times][%d][path][%s]", call_times, process_info.failed_call_times, process_info.path);
        // YSOS_LOG_DEBUG("ProcessMonitor::RunProcess[call_times][" << call_times << "] failed_call_times[" << process_info.failed_call_times << "] path[" << process_info.path << "]");
        boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
        continue;
      }
    }

	YSOS_LOG_DEBUG("ProcessMonitor::RunProcess[ExecuteProcess][" << process_info.path << "]");

	process_is_running = false;
	if(GetProcessWrapper()->IsRuningProcessEx(process_info.path.c_str())){
      process_is_running = true;
	}

	if(process_is_running) {   ///< 程序已经在运行
      while(TRUE){
        boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
        if(GetProcessWrapper()->IsRuningProcessEx(process_info.path.c_str())) {
	     continue;
        }
        YSOS_LOG_DEBUG("ProcessMonitor::RunProcess[process exit]["<<process_info.path << "]");
        break;
      }
	} else {
	  /// 运行进程
    YSOS_LOG_DEBUG("ProcessMonitor::RunProcess  开始执行EXPLORER.EXE Start");
    /* add for linux
    HANDLE* process_handle_ptr_1 = new HANDLE;
    int _result = GetProcessWrapper()->ExecuteProcess(
      "C:\\Windows\\Explorer.exe",
      "",
      "",
      TRUE,
      0,
      TRUE,
      process_handle_ptr);*/
	  YSOS_LOG_DEBUG("ProcessMonitor::RunProcess  结束执行EXPLORER.EXE  Start");
    //* add for linux
	  result = GetProcessWrapper()->ExecuteProcess(
					process_info.path.c_str(),
					process_info.command_line.c_str(),
					NULL,
					(process_info.hide_window == "1" ? true : false),
					(boost::lexical_cast<long>(process_info.delay_time.c_str())),
					true,
					process_handle_ptr);
      if (0 != result) {
        printf("Daemon::ProcessMonitor::RunProcess[Fail][The function[GetProcessWrapper()->ExecuteProcess()] is failed]result[%d] path[%s]\n", result, process_info.path.c_str());
        YSOS_LOG_DEBUG("ProcessMonitor::RunProcess[Fail][The function[GetProcessWrapper()->ExecuteProcess()] is failed]result["<< result <<"] path[" << process_info.path << "]");

        /// 设置了重新拉起次数，记录拉起次数
        if (true != process_info.call_times.empty()) {
          process_info.failed_call_times += 1;

          if (0 != SetProcessInfoMap(process_info)) {
            printf("Daemon::ProcessMonitor::RunProcess[Fail][The function[SetProcessInfoMap()] is failed]\n");
            YSOS_LOG_DEBUG("ProcessMonitor::RunProcess[Fail][The function[SetProcessInfoMap()] is failed]");
            boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
            continue;
          }
          /// for debug
          if(true != process_info.call_times.empty()) {
            unsigned call_times = std::stoul(process_info.call_times);
            YSOS_LOG_DEBUG("ProcessMonitor::RunProcess[Check Point][call_times][" << call_times << "] [failed_call_times][" << process_info.failed_call_times << "] [path][" << process_info.path << "]");
            if (call_times < process_info.failed_call_times) {
              printf("Daemon::ProcessMonitor::RunProcess[call_times][%d][failed_call_times][%d][path][%s]\n", call_times, process_info.failed_call_times, process_info.path.c_str());
              YSOS_LOG_DEBUG("ProcessMonitor::RunProcess[call_times][" << call_times << "] [failed_call_times][" << process_info.failed_call_times << "] [path][" << process_info.path << "]");
            }
          }   
		    }
        boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
        continue;
      }
      //*/ //end add for linux

      YSOS_LOG_DEBUG("ProcessMonitor::RunProcess[process] 1!");
      if (NULL == *process_handle_ptr) {
        printf("Daemon::ProcessMonitor::RunProcess[Fail][process_handle_ptr is NULL]\n");
        YSOS_LOG_DEBUG("ProcessMonitor::RunProcess[Fail][process_handle_ptr is NULL]");
        boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
        continue;
      }

      printf("Daemon::ProcessMonitor::RunProcess[Check Point][Before][path][%s]\n", process_info.path.c_str());
      YSOS_LOG_DEBUG("ProcessMonitor::RunProcess[Check Point][Before][path][" << process_info.path << "]");

      /// 持续等待线程结束
      //TODO: add for linux
      //WaitForSingleObject(*process_handle_ptr, INFINITE);
      YSOS_LOG_DEBUG("ProcessMonitor::RunProcess[process exit]["<<process_info.path << "]");
	}

    printf("Daemon::ProcessMonitor::RunProcess[Check Point][After][path][%s]\n", process_info.path.c_str());
    YSOS_LOG_DEBUG("ProcessMonitor::RunProcess[Check Point][After][path][" << process_info.path << "]");

	GetProcessWrapper()->RefreshTaskbarIcon();

    /// 设置了重新拉起次数，记录拉起次数
    if (true != process_info.call_times.empty()) {
      process_info.failed_call_times += 1;

      if (0 != SetProcessInfoMap(process_info)) {
        printf("Daemon::ProcessMonitor::RunProcess[Fail][The function[SetProcessInfoMap()] is failed]\n");
        YSOS_LOG_DEBUG("ProcessMonitor::RunProcess[Fail][The function[SetProcessInfoMap()] is failed]");
        boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
        continue;
      }

      /* add for linux -------> for temp ignore
      // for debug
	    if (true != process_info.call_times.empty() ) {
        unsigned call_times = std::stoul(process_info.call_times);
        YSOS_LOG_DEBUG("ProcessMonitor::RunProcess[Check Point][call_times][" << call_times << "] [failed_call_times][" << process_info.failed_call_times << "] [path][" << process_info.path << "]");
        if (call_times < process_info.failed_call_times) {
          printf("Daemon::ProcessMonitor::RunProcess[call_times][%d][failed_call_times][%d][path][%s]\n", call_times, process_info.failed_call_times, process_info.path.c_str());
          YSOS_LOG_DEBUG("ProcessMonitor::RunProcess[call_times][" << call_times << "] [failed_call_times][" << process_info.failed_call_times << "] [path][" << process_info.path << "]");
        }
	    }
      */
    }

    boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
  }  //// end while (IS_STOPPED != GetRunningStatus())

  printf("Daemon::ProcessMonitor::RunProcess[Exit]\n");
  YSOS_LOG_DEBUG("ProcessMonitor::RunProcess[Exit]");
  return;
}

int ProcessMonitor::SetProcessInfoMap(const ProcessInfo& process_info) {
  printf("Daemon::ProcessMonitor::SetProcessInfoMap[Enter]\n");
  YSOS_LOG_DEBUG("ProcessMonitor::SetProcessInfoMap[Enter]");

  int result = 0;
  do {
    BoostLockGuard lock(shared_mutex_);

    if (true == process_info_map_.empty()) {
      printf("Daemon::ProcessMonitor::SetProcessInfoMap[Fail][process_info_map_ is empty]\n");
      YSOS_LOG_DEBUG("ProcessMonitor::SetProcessInfoMap[Fail][process_info_map_ is empty]");
      result = -1;
      break;
    }

    ProcessInfoMapIterator process_info_itr = process_info_map_.find(process_info.tag);
    if (process_info_itr == process_info_map_.end()) {
      printf("Daemon::ProcessMonitor::SetProcessInfoMap[Fail][can't find process][tag][%s][path][%s]\n", process_info.tag.c_str(), process_info.path.c_str());
      YSOS_LOG_DEBUG("ProcessMonitor::SetProcessInfoMap[Fail][can't find process][tag][" << process_info.tag << "] [path][" << process_info.path << "]");
      result = -1;
      break;
    }

    process_info_itr->second = process_info;
  } while (0);

  printf("Daemon::ProcessMonitor::SetProcessInfoMap[Exit]\n");
  YSOS_LOG_DEBUG("ProcessMonitor::SetProcessInfoMap[Exit]");
  return result;
}

int ProcessMonitor::SetMainProcessFailedCallTimes(const unsigned failed_call_times) {
  printf("Daemon::ProcessMonitor::SetMainProcessFailedCallTimes[Enter]\n");
  YSOS_LOG_DEBUG("ProcessMonitor::SetMainProcessFailedCallTimes[Enter]");

  int result = 0;
  bool is_found = false;

  do {
    BoostLockGuard lock(shared_mutex_);

    if (true == process_info_map_.empty()) {
      printf("Daemon::ProcessMonitor::SetMainProcessFailedCallTimes[Fail][process_info_map_ is empty]\n");
      YSOS_LOG_DEBUG("ProcessMonitor::SetMainProcessFailedCallTimes[Fail][process_info_map_ is empty]");
      result = -1;
      break;
    }

    is_found = false;
    for (ProcessInfoMapIterator itr = process_info_map_.begin();
         itr != process_info_map_.end();
         ++itr) {
      /// 查找主进程
      if (true != itr->second.type.empty() && "1" == itr->second.type) {
        printf("Daemon::ProcessMonitor::SetMainProcessFailedCallTimes[Fail][find process][tag][%s][path][%s]\n", itr->second.tag.c_str(), itr->second.path.c_str());
        YSOS_LOG_DEBUG("ProcessMonitor::SetMainProcessFailedCallTimes[Fail][find process][tag][" << itr->second.tag << "] [path][" << itr->second.path << "]");
        itr->second.failed_call_times = failed_call_times;
        is_found = true;
        break;
      }
    }
  } while (0);

  /// 没找到主进程
  if (false == is_found) {
    printf("Daemon::ProcessMonitor::SetMainProcessFailedCallTimes[Check Point][Can't find main process\n");
    YSOS_LOG_DEBUG("ProcessMonitor::SetMainProcessFailedCallTimes[Can't find main process]");
    result = -2;
  }

  printf("Daemon::ProcessMonitor::SetMainProcessFailedCallTimes[Exit]\n");
  YSOS_LOG_DEBUG("ProcessMonitor::SetMainProcessFailedCallTimes[Exit]");
  return result;
}

int ProcessMonitor::CheckDependencyProcessRunningStatus(const std::string& dependency_info, bool& is_running, unsigned& main_process_call_times, unsigned& main_process_failed_call_times, std::string& main_process_path) {
//   printf("Daemon::ProcessMonitor::CheckDependencyProcessRunningStatus[Enter]");
//   YSOS_LOG_DEBUG("ProcessMonitor::CheckDependencyProcessRunningStatus[Enter]");

  int result = 0;
  is_running = true;
  main_process_path.clear();

  do {
    if (true == dependency_info.empty()) {
      printf("Daemon::ProcessMonitor::CheckDependencyProcessRunningStatus[Fail][dependency_info is empty]\n");
      YSOS_LOG_DEBUG("ProcessMonitor::CheckDependencyProcessRunningStatus[Fail][dependency_info is empty]");
      result = -1;
      break;
    }

//     printf("Daemon::ProcessMonitor::CheckDependencyProcessRunningStatus[Check Point][0]");
//     YSOS_LOG_DEBUG("ProcessMonitor::CheckDependencyProcessRunningStatus[Check Point][0]");

    /// 解析依赖进程tag字符串
    std::list<std::string> dependency_info_tag_list;
    if (YSOS_ERROR_SUCCESS != GetUtility()->SplitString(dependency_info, "|", dependency_info_tag_list)) {
      printf("Daemon::ProcessMonitor::CheckDependencyProcessRunningStatus[Fail][The function[GetUtility()->SplitString()] is failed]\n");
      YSOS_LOG_DEBUG("ProcessMonitor::CheckDependencyProcessRunningStatus[Fail][The function[GetUtility()->SplitString()] is failed]");
      result = -1;
      break;
    }

//     printf("Daemon::ProcessMonitor::CheckDependencyProcessRunningStatus[Check Point][1]");
//     YSOS_LOG_DEBUG("ProcessMonitor::CheckDependencyProcessRunningStatus[Check Point][1]");

    /// 列表为空跳过
    if (true == dependency_info_tag_list.empty()) {
      printf("Daemon::ProcessMonitor::CheckDependencyProcessRunningStatus[Fail][dependency_info_tag_list is empty]\n");
      YSOS_LOG_DEBUG("ProcessMonitor::CheckDependencyProcessRunningStatus[Fail][dependency_info_tag_list is empty]");
      result = -1;
      break;
    }

//     printf("Daemon::ProcessMonitor::CheckDependencyProcessRunningStatus[Check Point][2]");
//     YSOS_LOG_DEBUG("ProcessMonitor::CheckDependencyProcessRunningStatus[Check Point][2]");

    /// 查询所有依赖进程，判断所有依赖进程是否都已运行
    std::string expand_variable_value;
    for (std::list<std::string>::iterator itr = dependency_info_tag_list.begin();
         itr != dependency_info_tag_list.end();
         ++itr) {

      /// 以tag查询进程名称
      ProcessInfoMapIterator process_info_itr = process_info_map_.find(*itr);
      if (process_info_itr == process_info_map_.end()) {
        printf("Daemon::ProcessMonitor::CheckDependencyProcessRunningStatus[Fail][Can't find the process info][%s].\n", itr->c_str());
        YSOS_LOG_DEBUG("ProcessMonitor::CheckDependencyProcessRunningStatus[Fail][Can't find the process info]["<< itr->c_str() <<"]");
        result = -1;
        break;
      }

      /// 判断所有依赖程序都运行
      if (TRUE != GetProcessWrapper()->IsRuningProcessEx(process_info_itr->second.path.c_str())) {
//         printf("Daemon::ProcessMonitor::CheckDependencyProcessRunningStatus[Check Point][The process is not running][%s]", process_info_itr->second.path.c_str());
//         YSOS_LOG_DEBUG("ProcessMonitor::CheckDependencyProcessRunningStatus[Check Point][The process is not running]["<< process_info_itr->second.path.c_str() <<"]");
        is_running = false;
      }

      /// 查看该进程是否是主进程
	  if (true != process_info_itr->second.type.empty() ) {
        unsigned process_type = std::stoul(process_info_itr->second.type);
        //YSOS_LOG_DEBUG("ProcessMonitor::CheckDependencyProcessRunningStatus[process_type][" << process_type << "] [path][" << process_info_itr->second.path);
        if (1 == process_type) {
          /// 查看是否设置了最大拉起次数
          if (true != process_info_itr->second.call_times.empty()) {
            unsigned call_times = std::stoul(process_info_itr->second.call_times);

            /// 记录主进程最大拉起次数
            main_process_call_times = call_times;

            /// 记录主进程当前运行失败次数
            main_process_failed_call_times = process_info_itr->second.failed_call_times;

            /// for debug
            //  if (call_times + 1 == process_info_itr->second.failed_call_times) {
            //    printf("Daemon::ProcessMonitor::CheckDependencyProcessRunningStatus[call_times][%d][failed_call_times][%d][path][%s]", call_times, process_info_itr->second.failed_call_times, process_info_itr->second.path);
            //    YSOS_LOG_DEBUG("ProcessMonitor::CheckDependencyProcessRunningStatus[call_times][" << call_times << "] [failed_call_times][" << process_info_itr->second.failed_call_times << "] [path][" << process_info_itr->second.path << "]");
            // }
          }

          /// 保存进程执行路径
          main_process_path =  process_info_itr->second.path;
	    }
      }
    }  ///end for
  } while (0);

//   printf("Daemon::ProcessMonitor::CheckDependencyProcessRunningStatus[Exit]");
//   YSOS_LOG_DEBUG("ProcessMonitor::CheckDependencyProcessRunningStatus[Exit]");
  return result;
}

std::string ProcessMonitor::GetExpandEnvironmentVariableString(void) {
  printf("Daemon::GetExpandEnvironmentVariableString[Enter]\n");

  std::string result;
  int MAX_PATH = 260;
  do {
    /// 获取系统path
    char system_path[g_buffer_size] = {0,};
    //if (0 == GetEnvironmentVariable(_T("PATH"), system_path, g_buffer_size)) {
    char* tval = getenv("PATH");
    printf("[%s]\n", getenv("PATH"));
    if ( NULL == tval ) {//add for linux
      printf("Daemon::GetExpandEnvironmentVariableString[Fail][GetEnvironmentVariable()][val][%s]\n",tval);
      break;
    } else {
      strcpy(system_path, tval);
    }

    printf("Daemon::GetExpandEnvironmentVariableString[Check Point][0][system_path][%s]\n", system_path);

    /// 获取模块文件路径
    char module_file_name[MAX_PATH*2] = {0};
    if (0 == GetModuleFileName(NULL, module_file_name, MAX_PATH*2)) {
      printf("Daemon::GetExpandEnvironmentVariableString[Fail][2][last_error_code][%s]\n", "GetLastError()");
      break;
    }

    if (TRUE != PathRemoveFileSpec(module_file_name)) {
      printf("Daemon::GetExpandEnvironmentVariableString[Fail][3][last_error_code][%s]\n", "GetLastError()");
      break;
    }

    printf("Daemon::GetExpandEnvironmentVariableString[Check Point][1][module_file_name][%s]\n", module_file_name);

    //if (TRUE != SetCurrentDirectory(module_file_name)) {
    if ( -1 == chdir ( module_file_name ) )//add for linux
    {
      printf("Daemon::GetExpandEnvironmentVariableString[Fail][SetCurrentDirectory()][last_error_code][%s]\n", "GetLastError()");
      break;
    }

    printf("Daemon::GetExpandEnvironmentVariableString[Check Point][2][module_file_name][%s]\n", module_file_name);

    char variable_value[g_buffer_size] = {0};
    //StringCchPrintf(variable_value, g_buffer_size, _T("%s\\..\\..;%s;%s\\..\\os_lib;%s\\..\\lib;%s\\..\\extern_lib;%s;"), module_file_name, module_file_name, module_file_name, module_file_name, module_file_name, system_path);
    sprintf(variable_value, "%s/../..;%s;%s/../lib;%s/../extern_lib;%s;", module_file_name, module_file_name, module_file_name, module_file_name, system_path);

    printf("Daemon::GetExpandEnvironmentVariableString[Check Point][3][variable_value][%s]\n", variable_value);

    result = variable_value;

    printf("Daemon::GetExpandEnvironmentVariableString[Check Point][End]\n");

  } while (0);

  printf("Daemon::GetExpandEnvironmentVariableString[Exit]\n");

  return std::string(result);
}

int ProcessMonitor::TerminateProcess(const std::string& process_name) {
  YSOS_LOG_DEBUG("ProcessMonitor::TerminateProcess[Enter]");
  int result = YSOS_ERROR_FAILED;

  do {
    YSOS_LOG_DEBUG("ProcessMonitor::TerminateProcess[Check Point][terminate_all is false]");
    if (true == process_name.empty()) {
      YSOS_LOG_DEBUG("ProcessMonitor::TerminateProcess[Fail][process_name is empty]");
      result = YSOS_ERROR_LOGIC_ERROR;
      break;
    }

    YSOS_LOG_DEBUG("ProcessMonitor::TerminateProcess[Check Point][0]");

    result = GetProcessWrapper()->KillProcess(process_name.c_str());
    if (0 != result) {
      printf("Daemon::The function[GetProcessWrapper()->KillProcess()] fails.[1][%d]\n", result);
      YSOS_LOG_DEBUG("ProcessMonitor::TerminateProcess[Fail][The process[GetProcessWrapper()->KillProcess()] is failed]["<< result <<"]");
      break;
    }

    YSOS_LOG_DEBUG("ProcessMonitor::TerminateProcess[Check Point][End]");
    result = YSOS_ERROR_SUCCESS;
  } while (0);

  YSOS_LOG_DEBUG("ProcessMonitor::TerminateProcess[Exit]");
  return result;
}

int ProcessMonitor::ExecuteProcessWithCMD(const std::string& params, const std::string& environment) {
  printf("Daemon::ProcessMonitor::ExecuteProcessWithCMD[Enter]\n");
  YSOS_LOG_DEBUG("ProcessMonitor::ExecuteProcessWithCMD[Enter]");
  int result = YSOS_ERROR_FAILED;

  do {
    if (true == params.empty()) {
      YSOS_LOG_DEBUG("ProcessMonitor::ExecuteProcessWithCMD[Fail][params is empty]");
      result = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }

    YSOS_LOG_DEBUG("ProcessMonitor::ExecuteProcessWithCMD[Check Point][0]");

    /// 拼装cmd参数 taskkill /f /t /im ysos_service.exe
    std::string new_params = "/C ";
    new_params.append(params);
    new_params.append(" -t");
    printf("Daemon::[new_params][%s]\n", new_params.c_str());
    YSOS_LOG_DEBUG("ProcessMonitor::ExecuteProcessWithCMD[Check Point][new_params][" << new_params.c_str() <<"]");

    if (0 != GetProcessWrapper()->ExecuteProcess(
          "C:\\Windows\\System32\\cmd.exe",
          new_params.c_str(),
          const_cast<LPTSTR>(environment.c_str()),
          TRUE,
          0)) {
      printf("Daemon::The function[ExecuteProcess] fails.\n");
      YSOS_LOG_DEBUG("ProcessMonitor::ExecuteProcessWithCMD[Fail][The function[GetProcessWrapper()->ExecuteProcess()] is failed]");
      result = YSOS_ERROR_LOGIC_ERROR;
      break;
    }

    YSOS_LOG_DEBUG("ProcessMonitor::ExecuteProcessWithCMD[Check Point][End]");
    result = YSOS_ERROR_SUCCESS;
  } while (0);

  printf("Daemon::ProcessMonitor::ExecuteProcessWithCMD[Exit]\n");
  YSOS_LOG_DEBUG("ProcessMonitor::ExecuteProcessWithCMD[Exit]");
  return result;
}

int ProcessMonitor::Start(void) {
  printf("Daemon::ProcessMonitor::Start[Enter]\n");
  int result = YSOS_ERROR_FAILED;

  do {
    /// 判断运行状态
    if (IS_RUNNING == GetRunningStatus()) {
      result = YSOS_ERROR_SUCCESS;
      printf("Daemon::ProcessMonitor::Start[Check Point]ProcessMonitor is already running\n");
      break;
    }

    printf("Daemon::ProcessMonitor::Start[Check Point][0]\n");

    /// 初始化参数
    if (YSOS_ERROR_SUCCESS != Init()) {
      result = YSOS_ERROR_LOGIC_ERROR;
      printf("Daemon::ProcessMonitor::Start[Fail]The function[Init()] is failed\n");
      break;
    }

    printf("Daemon::ProcessMonitor::Start[Check Point][1]\n");
    YSOS_LOG_DEBUG("ProcessMonitor::Start[Check Point][1]");

  //TODO: add for linux
  /* // kill explorer process after start ysos.exe for hide explorer
	if(kill_explorer_ && kill_explorer_path_.length() > 0 ) {
		if (0 != GetProcessWrapper()->ExecuteProcessWithShell(kill_explorer_path_.c_str(),NULL,NULL,TRUE,FALSE,NULL)){
			YSOS_LOG_DEBUG("ProcessMonitor::Kill Explorer Fail");
		}		
	}*/

  //TODO: add for linux 
  /* //del this logic process  because it's for flex-front appcation
	if(enable_flash_server_) {
		flash_listen_server_ = new Server(listen_address_.c_str());
		if(flash_listen_server_ != NULL){
			flash_listen_server_->Register_CallBack_Function(YSOS_Run_State);
			flash_listen_server_->Run();
		}
	}*/

    /// 依次运行需要守护的进程
    RunMonitorThreades();

    printf("Daemon::ProcessMonitor::Start[Check Point][2]\n");
    YSOS_LOG_DEBUG("ProcessMonitor::Start[Check Point][2]");

    /// 设置为运行状态
    SetRunningStatus(IS_RUNNING);

    printf("Daemon::ProcessMonitor::Start[Check Point][End]\n");
    YSOS_LOG_DEBUG("ProcessMonitor::Start[Check Point][End]");
    result = YSOS_ERROR_SUCCESS;
  } while (0);

  printf("Daemon::ProcessMonitor::Start[Exit]\n");
  YSOS_LOG_DEBUG("ProcessMonitor::Start[Exit]");
  return result;
}

int ProcessMonitor::Stop(void) {
  printf("Daemon::ProcessMonitor::Stop[Enter]\n");
  YSOS_LOG_DEBUG("ProcessMonitor::Stop[Enter]");
  int result = YSOS_ERROR_FAILED;

  do {
	if(flash_listen_server_ != NULL) 
	{
		flash_listen_server_->Close();
		delete flash_listen_server_;
		flash_listen_server_ = NULL;
	}

    /// 判断运行状态
    if (IS_STOPPED == GetRunningStatus()) {
      result = YSOS_ERROR_SUCCESS;
      printf("Daemon::ProcessMonitor::Stop[Check Point]ProcessMonitor is already stopped\n");
      YSOS_LOG_DEBUG("UHExecuteCommand::Stop[Check Point][ProcessMonitor is already stopped]");
      break;
    }

    printf("Daemon::ProcessMonitor::Stop[Check Point][0]\n");
    YSOS_LOG_DEBUG("ProcessMonitor::Stop[Check Point][0]");

    /// 设置为停止状态
    SetRunningStatus(IS_STOPPED);

    printf("Daemon::ProcessMonitor::Stop[Check Point][1]\n");
    YSOS_LOG_DEBUG("ProcessMonitor::Stop[Check Point][1]");

    /// 等待监控线程结束
    if (true == monitor_thread_.joinable()) {
      YSOS_LOG_DEBUG("ProcessMonitor::Stop[Check Point][monitor_thread_.joinable()][Start]");
      monitor_thread_.join();
      YSOS_LOG_DEBUG("ProcessMonitor::Stop[Check Point][monitor_thread_.joinable()][End]");
    }

    /// 等待执行进程线程组结束
    run_process_thread_group_.join_all();

    printf("Daemon::ProcessMonitor::Stop[Check Point][End]\n");
    YSOS_LOG_DEBUG("ProcessMonitor::Stop[Check Point][End]");
    result = YSOS_ERROR_SUCCESS;
  } while (0);

  printf("Daemon::ProcessMonitor::Stop[Exit]\n");
  YSOS_LOG_DEBUG("ProcessMonitor::Stop[Exit]");
  return result;
}


/**
*@brief Daemon 具体类实现
*/
DEFINE_SINGLETON(Daemon)
Daemon::Daemon(const std::string &strClassName /* =YSOSDaemon */)
  : running_status_(IS_INITIAL),
    connection_listener_ptr_(NULL),
    process_monitor_ptr_(NULL) {
}

Daemon::~Daemon(void) {
  /// 重置运行状态
  SetRunningStatus(IS_INITIAL);

  /// 重置相关类
  if (NULL != connection_listener_ptr_) {
    connection_listener_ptr_ = NULL;
  }

  if (NULL != process_monitor_ptr_) {
    process_monitor_ptr_ = NULL;
  }
}

int Daemon::Send(const ExternMessage& message, const ColleaguePtr& colleague_ptr) {
  printf("Daemon::Daemon::Send[Enter]\n");
  YSOS_LOG_DEBUG("Daemon::Send[Enter]");

  /// 消息中转，监听连接类会发送给监测进程类，监测进程类会发送给监听连接类
  if (colleague_ptr == connection_listener_ptr_) {
    /// 通知监测进程类
    if (YSOS_ERROR_SUCCESS != process_monitor_ptr_->Notify(message)) {
      printf("Daemon::Daemon::Notify[Fail]The function[process_monitor_ptr_->Notify()] is failed\n");
      YSOS_LOG_DEBUG("Daemon::Notify[Fail][The function[process_monitor_ptr_->Notify()] is failed]");
      return YSOS_ERROR_FAILED;
    }
  } else {
    /// 通知监听连接类
    if (YSOS_ERROR_SUCCESS != connection_listener_ptr_->Notify(message)) {
      printf("Daemon::Daemon::Notify[Fail]The function[connection_listener_ptr_->Notify()] is failed\n");
      YSOS_LOG_DEBUG("Daemon::Notify[Fail][The function[connection_listener_ptr_->Notify()] is failed]");
      return YSOS_ERROR_FAILED;
    }
  }

  printf("Daemon::Daemon::Send[Exit]\n");
  YSOS_LOG_DEBUG("Daemon::Send[Exit]");
  return YSOS_ERROR_SUCCESS;
}

int Daemon::Notify(const ExternMessage& message, const ColleaguePtr& colleague_ptr) {
  printf("Daemon::Daemon::Notify[Enter]\n");
  YSOS_LOG_DEBUG("Daemon::Notify[Enter]");

  /// 通知监听连接类
  if (colleague_ptr == connection_listener_ptr_) {
    if (YSOS_ERROR_SUCCESS != connection_listener_ptr_->Notify(message)) {
      printf("Daemon::Daemon::Notify[Fail]The function[connection_listener_ptr_->Notify()] is failed\n");
      YSOS_LOG_DEBUG("Daemon::Notify[Fail][The function[connection_listener_ptr_->Notify()] is failed]");
      return YSOS_ERROR_FAILED;
    }
  }
  /// 通知监测进程类
  else {
    if (YSOS_ERROR_SUCCESS != process_monitor_ptr_->Notify(message)) {
      printf("Daemon::Daemon::Notify[Fail]The function[process_monitor_ptr_->Notify()] is failed\n");
      YSOS_LOG_DEBUG("Daemon::Notify[Fail][The function[process_monitor_ptr_->Notify()] is failed]");
      return YSOS_ERROR_FAILED;
    }
  }

  printf("Daemon::Daemon::Notify[Exit]\n");
  YSOS_LOG_DEBUG("Daemon::Notify[Exit]");
  return YSOS_ERROR_SUCCESS;
}

int Daemon::NotifyAll(const ExternMessage& message) {
  printf("Daemon::Daemon::NotifyAll[Enter]\n");
  YSOS_LOG_DEBUG("Daemon::NotifyAll[Enter]");

  /// 通知监听连接类
  if (YSOS_ERROR_SUCCESS != connection_listener_ptr_->Notify(message)) {
    printf("Daemon::Daemon::NotifyAll[Fail]The function[connection_listener_ptr_->Notify()] is failed\n");
    YSOS_LOG_DEBUG("Daemon::NotifyAll[Fail][The function[connection_listener_ptr_->Notify()] is failed]");
    return YSOS_ERROR_FAILED;
  }

  printf("Daemon::Daemon::NotifyAll[Check Point][0]\n");

  /// 通知监测进程类
  if (YSOS_ERROR_SUCCESS != process_monitor_ptr_->Notify(message)) {
    printf("Daemon::Daemon::NotifyAll[Fail]The function[process_monitor_ptr_->Notify()] is failed\n");
    YSOS_LOG_DEBUG("Daemon::NotifyAll[Fail][The function[process_monitor_ptr_->Notify()] is failed]");
    return YSOS_ERROR_FAILED;
  }

  printf("Daemon::Daemon::NotifyAll[Exit]\n");
  YSOS_LOG_DEBUG("Daemon::NotifyAll[Exit]");
  return YSOS_ERROR_SUCCESS;
}

int Daemon::Start(void) {
  printf("Daemon::Daemon::Start[Enter]\n");
  int result = YSOS_ERROR_FAILED;

  do {
    /// 判断运行状态
    if (IS_RUNNING == GetRunningStatus()) {
      result = YSOS_ERROR_SUCCESS;
      printf("Daemon::Daemon::Start[Check Point]It is already running\n");
      break;
    }

    printf("Daemon::Daemon::Start[Check Point][0]\n");

    /// 判断logger对象是否可用
    if (true != logger_.valid()) {
      printf("Daemon::Daemon::Start[Check Point][The function[logger_.valid()] is failed]\n");
      /// 初始化日志文件路径
      if (true != log::InitLoggerExt("../log/daemon.log", "daemon")) {
        printf("Daemon::Daemon::Start[Fail][The function[log::InitLoggerExt()] is failed]\n");
        return YSOS_ERROR_FAILED;
      }

      printf("Daemon::Daemon::Start[Check Point][1]\n");

      /// 初始化logger
      logger_ = ysos::log::GetLogger("daemon");
      if (true != logger_.valid()) {
        printf("Daemon::Daemon::Start[Fail][The function[logger_.valid()] is failed]\n");
        return YSOS_ERROR_FAILED;
      }

      printf("Daemon::Daemon::Start[Check Point][2]\n");
    }

    printf("Daemon::Daemon::Start[Check Point][3]\n");
    YSOS_LOG_DEBUG("Daemon::Start[Check Point][###Logging Start###]");

    /// 创建监听连接类
    connection_listener_ptr_ = ConnectionListenerPtr(new ConnectionListener(shared_from_this()));
    if (NULL == connection_listener_ptr_) {
      result = YSOS_ERROR_LOGIC_ERROR;
      printf("Daemon::Daemon::Start[Fail]connection_listener_ptr_ is NULL\n");
      YSOS_LOG_DEBUG("Daemon::Start[Fail][connection_listener_ptr_ is NULL]");
      break;
    }

    printf("Daemon::Daemon::Start[Check Point][4]\n");
    YSOS_LOG_DEBUG("Daemon::Start[Check Point][0]");

    /// 创建监测进程类
    process_monitor_ptr_ = ProcessMonitorPtr(new ProcessMonitor(shared_from_this()));
    if (NULL == process_monitor_ptr_) {
      result = YSOS_ERROR_LOGIC_ERROR;
      printf("Daemon::Daemon::Start[Fail]process_monitor_ptr_ is NULL\n");
      YSOS_LOG_DEBUG("Daemon::Start[Fail][process_monitor_ptr_ is NULL]");
      break;
    }

    printf("Daemon::Daemon::Start[Check Point][5]\n");
    YSOS_LOG_DEBUG("Daemon::Start[Check Point][1]");

    /// 设置监听连接类
    SetConnectionListener(connection_listener_ptr_);
    printf("Daemon::Daemon::Start[Check Point][6]\n");
    YSOS_LOG_DEBUG("Daemon::Start[Check Point][2]");

    /// 设置监测进程类
    SetProcessMonitor(process_monitor_ptr_);
    printf("Daemon::Daemon::Start[Check Point][7]\n");
    YSOS_LOG_DEBUG("Daemon::Start[Check Point][3]");

    /// 通知所有类开始工作
    if (YSOS_ERROR_SUCCESS != NotifyAll(ExternMessage(Daemon::MSG_START))) {
      result = YSOS_ERROR_LOGIC_ERROR;
      printf("Daemon::Daemon::Start[Fail]The function[Notify()] is failed\n");
      YSOS_LOG_DEBUG("Daemon::Start[Fail]The function[Notify()] is failed");
      break;
    }

    printf("Daemon::Daemon::Start[Check Point][End]\n");
    YSOS_LOG_DEBUG("Daemon::Start[Check Point][End]");
    result = YSOS_ERROR_SUCCESS;
  } while (0);

  printf("Daemon::Daemon::Start[Exit]\n");
  YSOS_LOG_DEBUG("Daemon::Start[Exit]");
  return result;
}

int Daemon::Stop(bool terminate_all) {
  printf("Daemon::Daemon::Stop[Enter]\n");
  YSOS_LOG_DEBUG("Daemon::Stop[Enter]");
  int result = YSOS_ERROR_FAILED;

  do {
    /// 判断运行状态
    if (IS_STOPPED == GetRunningStatus()) {
      result = YSOS_ERROR_SUCCESS;
      printf("Daemon::Daemon::Stop[Check Point]It is already stopped\n");
      YSOS_LOG_DEBUG("Daemon::Stop[Check Point]It is already stopped");
      break;
    }

    printf("Daemon::Daemon::Stop[Check Point][0]\n");
    YSOS_LOG_DEBUG("Daemon::Stop[Check Point][0]");

    /// 设置为停止状态
    SetRunningStatus(IS_STOPPED);

    printf("Daemon::Daemon::Stop[Check Point][1]\n");
    YSOS_LOG_DEBUG("Daemon::Stop[Check Point][1]");

    /// 分别发送停止命令
    if (true == terminate_all) {
      printf("Daemon::Daemon::Stop[Check Point][terminate_all is true]\n");
      YSOS_LOG_DEBUG("Daemon::Stop[Check Point][terminate_all is true]");

      /// 通知监听连接类停止
      if (YSOS_ERROR_SUCCESS != Notify(ExternMessage(Daemon::MSG_STOP), connection_listener_ptr_)) {
        result = YSOS_ERROR_LOGIC_ERROR;
        printf("Daemon::Daemon::Stop[Fail]The function[Notify()] is failed\n");
        YSOS_LOG_DEBUG("Daemon::Stop[Fail][The function[Notify()] is failed]");
        break;
      }

      YSOS_LOG_DEBUG("Daemon::Stop[Check Point][2]");

      /// 通知监测进程类停止，杀死所有守护的进程
      if (YSOS_ERROR_SUCCESS != Notify(ExternMessage(Daemon::MSG_STOP, std::string("terminate_all")), process_monitor_ptr_)) {
        result = YSOS_ERROR_LOGIC_ERROR;
        printf("Daemon::Daemon::Stop[Fail]The function[Notify()] is failed\n");
        YSOS_LOG_DEBUG("Daemon::Stop[Fail][The function[Notify()] is failed]");
        break;
      }
    }
    /// 通知所有类停止工作
    else {
      printf("Daemon::Daemon::Stop[Check Point][terminate_all is false]\n");
      YSOS_LOG_DEBUG("Daemon::Stop[Check Point][terminate_all is false]");
      if (YSOS_ERROR_SUCCESS != NotifyAll(ExternMessage(Daemon::MSG_STOP))) {
        result = YSOS_ERROR_LOGIC_ERROR;
        printf("Daemon::Daemon::Stop[Fail]The function[NotifyAll()] is failed\n");
        YSOS_LOG_DEBUG("Daemon::Stop[Fail][The function[NotifyAll()] is failed]");
        break;
      }
    }

    printf("Daemon::Daemon::Stop[Check Point][End]\n");
    YSOS_LOG_DEBUG("Daemon::Stop[Check Point][End]");
    result = YSOS_ERROR_SUCCESS;
  } while (0);

  printf("Daemon::Daemon::Stop[Exit]\n");
  YSOS_LOG_DEBUG("Daemon::Stop[Exit]");
  return result;
}

}  /// namespace sys_daemon_package

}  /// namespace ysos
