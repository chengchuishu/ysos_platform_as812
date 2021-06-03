/**
*@file BaseAppManagerImpl.cpp
*@brief Definition of BaseAppManagerImpl
*@version 0.1
*@author dhongqian
*@date Created on: 2016-06-08 15:05:58
*@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
* 
*/

/// Private Headers
#include "../../../protect/include/sys_framework_package/baseappmanagerimpl.h"
/// Boost Headers        // NOLINT
#include <boost/make_shared.hpp>
/// Ysos Headers
#include "../../../protect/include/sys_framework_package/basesessionimpl.h"
#include "../../../public/include/core_help_package/utility.h"
#include "../../../public/include/sys_interface_package/messagetype.h"
#include "../../../protect/include/sys_framework_package/platformrpcentity.h"
#include "../../../public/include/sys_interface_package/ysos_enum.h"
#include "../../../public/include/os_hal_package/filepath.h"
#include "../../../public/include/core_help_package/xmlutil.h"
#include "../../../public/include/core_help_package/jsonutility.h"
#include "../../../protect/include/sys_framework_package/manager_declare.h"

namespace ysos {

#ifdef _DEBUG
static const char* g_ysos_win_name = "ysos_d.exe";
static const char* g_ysos_console_name = "system_manager_d.exe";
#else
static const char* g_ysos_win_name = "ysos.exe";
static const char* g_ysos_console_name = "system_manager.exe";
#endif

#define XML_EXTENSION ".xml"

BaseAppManagerImpl::BaseAppManagerImpl(const std::string& strClassName): BaseRPCCallbackImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.framework.agent");
}

BaseAppManagerImpl::~BaseAppManagerImpl() {

}

tinyxml2::XMLElement *BaseAppManagerImpl::GetRootElement(tinyxml2::XMLDocument &doc, const std::string &file_name, const std::string &next_element) {
  std::string xml_str = GetUtility()->ReadAllDataFromFile(file_name);
  int ret = doc.Parse(xml_str.c_str());
  if (0 != ret) {
    YSOS_LOG_DEBUG("Open configure file: " << file_name << " failed");
    return NULL;
  }

  if (next_element == "" || NULL == doc.RootElement()) {
    return doc.RootElement();
  }

  return doc.RootElement()->FirstChildElement(next_element.c_str());
}

BaseAppManagerImpl::ClientInfoPtr BaseAppManagerImpl::ParseClientInfo(const std::string &client_file) {
  tinyxml2::XMLDocument doc;
  tinyxml2::XMLElement *root = GetRootElement(doc, client_file, "appmanager");
  if (NULL == root) {
    YSOS_LOG_DEBUG("Parse file: " << client_file << " failed");
    return ClientInfoPtr();
  }

  ClientInfoPtr client_info_ptr = boost::make_shared<ClientInfo>();
  client_info_ptr->name = GetXmlUtil()->GetElementTextValue(root, "name");
  client_info_ptr->strategy_name  = GetXmlUtil()->GetElementTextValue(root, "strategy");
  client_info_ptr->strategy_callback  = GetXmlUtil()->GetElementTextValue(root, "strategy_callback");
  client_info_ptr->ip_address  = GetXmlUtil()->GetElementTextValue(root, "ip_address");
  client_info_ptr->ip_port  = GetXmlUtil()->GetElementInt32Value(root, "ip_port");

  return client_info_ptr;
}

int BaseAppManagerImpl::ParseConfigFile(const std::string &conf_dir) {
  if (!FilePath::IsDirectory(conf_dir)) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  const std::string xml_ext(XML_EXTENSION);
  std::vector<std::string> file_list = FilePath::GetAllFiles(conf_dir);
  for (std::vector<std::string>::iterator it = file_list.begin(); it!=file_list.end(); ++it) {
    FilePath file_path(*it);
    if (file_path.GetFileExtention() != xml_ext) {
      continue;
    }

    ClientInfoPtr client_info_ptr = ParseClientInfo(*it);
    if (NULL != client_info_ptr) {
      client_info_list_.push_back(client_info_ptr);
    }
  }

  return YSOS_ERROR_SUCCESS;
}

int BaseAppManagerImpl::Start(const BaseAppManagerImpl::AppManagerParamPtr &app_param_ptr, const std::string &conf_dir) {
  YSOS_LOG_DEBUG_DEFAULT("BaseAppManagerImpl::Start");
  CHECK_FINISH(GetBaseSessionImpl()->SetProperty(BaseSessionImpl::MANAGER, this));

  /// 解析配置文件
  ParseConfigFile(conf_dir);

  /// 连接客户端
  boost::thread start_client_thread = boost::thread(boost::bind(&BaseAppManagerImpl::StartClient, this));
  if (start_client_thread.joinable()) {
    start_client_thread.join();
  }

  /// 启动服务器
  return StartServer(app_param_ptr);
}

int BaseAppManagerImpl::AddSession(SessionInterface *session_ptr) {
  if (NULL == session_ptr) {
    YSOS_LOG_DEBUG_DEFAULT("invalid session");
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  if (session_map_.find(session_ptr->GetSessionId()) != session_map_.end()) {
    YSOS_LOG_DEBUG_DEFAULT(session_ptr->GetSessionId() << " already exist");
    return YSOS_ERROR_HAS_EXISTED;
  }

  session_map_.insert(std::make_pair(session_ptr->GetSessionId(), session_ptr));

  return YSOS_ERROR_SUCCESS;
}

SessionInterface *BaseAppManagerImpl::FindSession(const std::string &session_id) {
  SessionMapType::iterator it=session_map_.find(session_id);
  if (session_map_.end() == it) {
    YSOS_LOG_DEBUG_DEFAULT(session_id << " does not exist");
    return NULL;
  }

  return it->second;
}

int BaseAppManagerImpl::RemoveSession(const std::string &session_id) {
  SessionMapType::iterator it=session_map_.find(session_id);
  if (session_map_.end() == it) {
    YSOS_LOG_DEBUG_DEFAULT(session_id << " does not exist");
    return YSOS_ERROR_NOT_EXISTED;
  }

  BaseSessionImpl *session_ptr = static_cast<BaseSessionImpl*>(it->second);
  session_map_.erase(it);

  if (NULL == session_ptr) {
    YSOS_LOG_DEBUG_DEFAULT("session_ptr is null : " << session_id);
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }
  session_ptr->UnInitialize();
  GetBaseSessionImpl()->Destroy(session_ptr);

  return YSOS_ERROR_SUCCESS;
}

int BaseAppManagerImpl::Stop(void) {
  YSOS_LOG_DEBUG_DEFAULT("stop");

  /// 清理session
  SessionMapType::iterator it = session_map_.begin();
  for (; session_map_.end() !=it; ++it) {
    SessionInterface *session_ptr = it->second;
    session_ptr->UnInitialize();
    GetBaseSessionImpl()->Destroy(session_ptr);
  }

  session_map_.clear();

  /// 关闭客户端
  if (true != client_platform_rpc_list_.empty()) {
    /// 依次关闭客户端
    for (ClientPlatformRPCList::iterator itr = client_platform_rpc_list_.begin();
         itr != client_platform_rpc_list_.end();
         ++itr) {
      if (YSOS_ERROR_SUCCESS != (*itr)->Close()) {
        continue;
      }
    }
  }

  client_platform_rpc_list_.clear();

  /// 关闭服务器
  return StopServer();
}

int BaseAppManagerImpl::StartServer(const BaseAppManagerImpl::AppManagerParamPtr &app_param_ptr) {
  if (NULL == app_param_ptr) {
    YSOS_LOG_DEBUG_DEFAULT("invalid param");
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  SetAddress(app_param_ptr->address);
  SetPort(app_param_ptr->port);
  SetMode(PROP_SERVER);
  YSOS_LOG_DEBUG_DEFAULT("address:" << app_param_ptr->address << " | port:" << app_param_ptr->port << " | mode:SERVER");

  platform_rpc_ = PlatformRPCPtr(PlatformRPC::CreateInstance());
  if (NULL == platform_rpc_) {
    YSOS_LOG_DEBUG_DEFAULT("allocate platform rpc failed");
    return YSOS_ERROR_MEMORY_ALLOCATED_FAILED;
  }

  return platform_rpc_->Open(this);
}

void BaseAppManagerImpl::StartClient(void) {
  int result = YSOS_ERROR_FAILED;

  do {
    /// 查看客户端列表
    if (true == client_info_list_.empty()) {
      result = YSOS_ERROR_LOGIC_ERROR;
      break;
    }

    /// 依次创建客户端连接
    for (ClientInfoList::iterator itr = client_info_list_.begin();
         itr != client_info_list_.end();
         ++itr) {
      /// 创建策略callback实例
      RPCCallbackInterfacePtr connection_callback = GetRPCCallbackInterfaceManager()->FindInterface((*itr)->strategy_callback);
      if (NULL == connection_callback) {
        continue;
      }

      /// 设置ip
      connection_callback->SetAddress((*itr)->ip_address);

      /// 设置port
      connection_callback->SetPort((*itr)->ip_port);

      /// 设置模式
      connection_callback->SetMode(PROP_CLIENT);

      /// 设置状态机名称
      connection_callback->SetStrategyName((*itr)->strategy_name);

      /// 设置应用名
      connection_callback->SetAplicationName((*itr)->name);

      /// 设置错误处理回调函数
      connection_callback->SetErrorHandler(boost::bind(&BaseAppManagerImpl::HandleError, this, _1, _2));

      /// 创建rpc
      PlatformRPCPtr platform_rpc_ptr = PlatformRPCPtr(PlatformRPC::CreateInstance());
      if (NULL == platform_rpc_ptr) {
        continue;
      }

      /// 打开rpc
      std::string session_id;
      if (YSOS_ERROR_SUCCESS != platform_rpc_ptr->Open(connection_callback.get(), &session_id)) {
        continue;
      }

      /// 保存已打开的rpc
      client_platform_rpc_list_.push_back(platform_rpc_ptr);

      /// 拼装初始化状态机参数字符窜
      JsonValue json_value;
      json_value["app_name"] = (*itr)->name;
      json_value["strategy_name"] = (*itr)->strategy_name;

      std::string service_param;
      if (YSOS_ERROR_SUCCESS != GetJsonUtil()->JsonValueToString(json_value, service_param)) {
        continue;
      }

      /// 创建session
      if (YSOS_ERROR_SUCCESS != GetBaseSessionImpl()->Accept(session_id, platform_rpc_ptr, &service_param)) {
        continue;
      }

      /// 获取新生成的session指针
      SessionInterface *session_ptr = FindSession(session_id);
      if (NULL == session_ptr) {
        YSOS_LOG_DEBUG_DEFAULT("Find session failed: " << session_id);
        continue;
      }

      /// 将新生成的session指针绑定到新创建策略callback实例
      if (YSOS_ERROR_SUCCESS != connection_callback->SetProperty(PROP_SESSIONPTR, session_ptr)) {
        continue;
      }
    }

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  if (YSOS_ERROR_SUCCESS != result) {
  }

  return;
}

int BaseAppManagerImpl::StopServer(void) {
  assert(NULL != platform_rpc_);
  return platform_rpc_->Close();
}

void BaseAppManagerImpl::HandleDoService(const std::string& session_id, const std::string& service_name, const std::string& service_param) {

  if (true != session_id.empty()) {
    /// 初始化服务处理
#if _CERTUSNET
    if (YSOS_AGENT_SERVICE_NAME_INIT_CONNECTION == service_name) {
#else
    if (YSOS_AGENT_SERVICE_NAME_INIT == service_name) {
#endif
      YSOS_LOG_DEBUG_DEFAULT(" Handle connected");
      int ret = HandleConnected(session_id, platform_rpc_, service_param);
      ResInitServiceParam handle_init_resp;
      handle_init_resp.status_code = GetUtility()->ConvertToString(ret);
      handle_init_resp.description = YSOS_ERROR_SUCCESS == ret ? "succeeded" : "failed";
      std::string handle_init_resp_string;
      if (true != handle_init_resp.ToString(handle_init_resp_string)) {
        YSOS_LOG_DEBUG_DEFAULT("[handle_init_resp.ToString()] failed. session_id: " << session_id);
      }

      /// 返回init结果
      if (YSOS_ERROR_SUCCESS != platform_rpc_->OnDispatchMessage(session_id, service_name, handle_init_resp_string)) {
        YSOS_LOG_DEBUG_DEFAULT("[platform_rpc_->DoService()] failed. handle_init_resp: " << handle_init_resp_string);
        return;
      }

      boost::this_thread::sleep_for(boost::chrono::milliseconds(50));

      /// 获取首个默认切换状态
      SessionInterface *session_ptr = FindSession(session_id);
      if (NULL == session_ptr) {
        YSOS_LOG_DEBUG_DEFAULT("Find session failed: " << session_id);
        return;
      }

#if _CERTUSNET
      /// 直接切换为首个状态
      if (YSOS_ERROR_SUCCESS != session_ptr->SetProperty(PROP_STATE, NULL)) {
        YSOS_LOG_DEBUG_DEFAULT("[platform_rpc_->DoService()] failed");
        return;
      }
#else
      /// 主动发送switchnotify
      std::string default_state;
      if (YSOS_ERROR_SUCCESS != session_ptr->GetProperty(PROP_STATE, &default_state)) {
        YSOS_LOG_DEBUG_DEFAULT("Get default state failed: " << session_id);
        return;
      }

      if (true == default_state.empty()) {
        YSOS_LOG_DEBUG_DEFAULT("default_state is empty");
        return;
      }

      ReqSwitchNotifyServiceParam req_switch_notify_service_param;
      req_switch_notify_service_param.state_name = default_state;
      std::string switch_notify_service_param;
      if (true != req_switch_notify_service_param.ToString(switch_notify_service_param)) {
        YSOS_LOG_DEBUG_DEFAULT("[req_switch_notify_service_param.ToString()] failed. session_id: " << session_id);
        return;
      }

      if (true == switch_notify_service_param.empty()) {
        YSOS_LOG_DEBUG_DEFAULT("switch_notify_service_param is empty");
        return;
      }

      if (YSOS_ERROR_SUCCESS != platform_rpc_->DoService(session_id, YSOS_AGENT_SERVICE_NAME_SWITCH_NOTIFY, switch_notify_service_param)) {
        YSOS_LOG_DEBUG_DEFAULT("[platform_rpc_->DoService()] failed. switch_notify_service_param: " << switch_notify_service_param);
      }
#endif

      return;
    }
    /// 注销服务处理
#if _CERTUSNET
    else if (YSOS_AGENT_SERVICE_NAME_UNINIT_CONNECTION == service_name) {
#else
    else if (YSOS_AGENT_SERVICE_NAME_UNINIT == service_name) {
#endif
      YSOS_LOG_DEBUG_DEFAULT(" Handle disconnected");

      /// 查找session
      SessionInterface *session_ptr = FindSession(session_id);
      if (NULL == session_ptr) {
        YSOS_LOG_DEBUG_DEFAULT("BaseAppManagerImpl::HandleDoService[Fail][0]");
        YSOS_LOG_DEBUG_DEFAULT("Find session failed: " << session_id);
        return;
      }

      YSOS_LOG_DEBUG_DEFAULT("BaseAppManagerImpl::HandleDoService[Check Point][0]");

      BaseSessionImpl::SessionAgentInfo session_agent_info("", "");
      if (YSOS_ERROR_SUCCESS != (dynamic_cast<BaseInterfaceImpl*>(session_ptr))->SetProperty(BaseSessionImpl::ERROR, &session_agent_info)) {
        YSOS_LOG_DEBUG_DEFAULT("BaseAppManagerImpl::HandleDoService[Fail][1]");
        return;
      }

      YSOS_LOG_DEBUG_DEFAULT("BaseAppManagerImpl::HandleDoService[Check Point][1]");

      /// 删除session
      if (YSOS_ERROR_SUCCESS != RemoveSession(session_id)) {
        YSOS_LOG_DEBUG_DEFAULT("BaseAppManagerImpl::HandleDoService[Fail][2]");
        return;
      }

      YSOS_LOG_DEBUG_DEFAULT("BaseAppManagerImpl::HandleDoService[Check Point][2]");

      /// 发送注销服务响应
      ResUninitServiceParam handle_uninit_resp;
      handle_uninit_resp.status_code = GetUtility()->ConvertToString(0);
      handle_uninit_resp.description = "succeeded";
      std::string handle_uninit_resp_string;
      if (true != handle_uninit_resp.ToString(handle_uninit_resp_string)) {
        YSOS_LOG_DEBUG_DEFAULT("[handle_uninit_resp.ToString()] failed. session_id: " << session_id);
        return;
      }

      YSOS_LOG_DEBUG_DEFAULT("BaseAppManagerImpl::HandleDoService[Check Point][3]");

      if (YSOS_ERROR_SUCCESS != platform_rpc_->OnDispatchMessage(session_id, service_name, handle_uninit_resp_string)) {
        YSOS_LOG_DEBUG_DEFAULT("[platform_rpc_->DoService()] failed. handle_uninit_resp: " << handle_uninit_resp_string);
        return;
      }

      YSOS_LOG_DEBUG_DEFAULT("BaseAppManagerImpl::HandleDoService[Check Point][End]");

      return;
    }
  }

  SessionInterface *session_ptr = FindSession(session_id);
  if (NULL == session_ptr) {
    YSOS_LOG_DEBUG_DEFAULT("Find session failed: " << session_id);
    return;
  }

  BaseSessionImpl::SessionAgentInfo session_agent_info(service_name, service_param);
  BaseSessionImpl* base_session_impl_ptr = dynamic_cast<BaseSessionImpl*>(session_ptr);
  assert(NULL != base_session_impl_ptr);
  base_session_impl_ptr->SetProperty(BaseSessionImpl::DOSERVICE, &session_agent_info);

  return;
}

void BaseAppManagerImpl::HandleOnDispatchMessage(const std::string& session_id, const std::string& service_name, const std::string& result) {
  SessionInterface *session_ptr = FindSession(session_id);
  if (NULL == session_ptr) {
    YSOS_LOG_DEBUG_DEFAULT("Find session failed: " << session_id);
    return;
  }
  BaseSessionImpl::SessionAgentInfo session_agent_info(service_name, result);

  (dynamic_cast<BaseInterfaceImpl*>(session_ptr))->SetProperty(BaseSessionImpl::DISPATCHSERVICE, &session_agent_info);
  return;
}

void BaseAppManagerImpl::HandleError(const std::string& session_id, const std::string& error_param) {
/// std::cout << "BaseAppManagerImpl::HandleError[Enter]" << std::endl;
  YSOS_LOG_DEBUG_DEFAULT("BaseAppManagerImpl::HandleError[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {
    if (true == session_id.empty() || true == error_param.empty()) {
      /// std::cout << "BaseAppManagerImpl::HandleError[Fail][0]" << std::endl;
      YSOS_LOG_DEBUG_DEFAULT("BaseAppManagerImpl::HandleError[Fail][0]");
      break;
    }

    /// std::cout << "BaseAppManagerImpl::HandleError[Check Point][0]" << std::endl;
    YSOS_LOG_DEBUG_DEFAULT("BaseAppManagerImpl::HandleError[Check Point][0]");

    RPCCallbackErrorParam rpc_callback_error_param;
    if (true != rpc_callback_error_param.FromString(error_param)) {
      /// std::cout << "BaseAppManagerImpl::HandleError[Fail][1]" << std::endl;
      YSOS_LOG_DEBUG_DEFAULT("BaseAppManagerImpl::HandleError[Fail][1]");
      break;
    }

    /// std::cout << "BaseAppManagerImpl::HandleError[Check Point][1]" << std::endl;
    YSOS_LOG_DEBUG_DEFAULT("BaseAppManagerImpl::HandleError[Check Point][1]");

    /// std::cout << "BaseAppManagerImpl::HandleError[rpc_callback_error_param.error_code][" << rpc_callback_error_param.error_code << "]" << std::endl;
    YSOS_LOG_DEBUG_DEFAULT("BaseAppManagerImpl::HandleError[rpc_callback_error_param.error_code][" << rpc_callback_error_param.error_code << "]");

    /// 暂时设置所有的错误都删除资源
    if (YSOS_ERROR_TRANSPORT_ERROR_CONNECTION_RESET == rpc_callback_error_param.error_code) {
      /// std::cout << "BaseAppManagerImpl::HandleError[Check Point][2]" << std::endl;
      YSOS_LOG_DEBUG_DEFAULT("BaseAppManagerImpl::HandleError[Check Point][2]");
    }

    /// std::cout << "BaseAppManagerImpl::HandleError[Check Point][2]" << std::endl;
    YSOS_LOG_DEBUG_DEFAULT("BaseAppManagerImpl::HandleError[Check Point][2]");

    SessionInterface *session_ptr = FindSession(session_id);
    if (NULL == session_ptr) {
      /// std::cout << "BaseAppManagerImpl::HandleError[Fail][2]" << std::endl;
      YSOS_LOG_DEBUG_DEFAULT("BaseAppManagerImpl::HandleError[Fail][2]");
      /// std::cout << "Find session failed: " << session_id << std::endl;
      YSOS_LOG_DEBUG_DEFAULT("Find session failed: " << session_id);
      break;
    }

    /// std::cout << "BaseAppManagerImpl::HandleError[Check Point][3]" << std::endl;
    YSOS_LOG_DEBUG_DEFAULT("BaseAppManagerImpl::HandleError[Check Point][3]");

    BaseSessionImpl::SessionAgentInfo session_agent_info("", error_param);
    if (YSOS_ERROR_SUCCESS != (dynamic_cast<BaseInterfaceImpl*>(session_ptr))->SetProperty(BaseSessionImpl::ERROR, &session_agent_info)) {
      /// std::cout << "BaseAppManagerImpl::HandleError[Fail][3]" << std::endl;
      YSOS_LOG_DEBUG_DEFAULT("BaseAppManagerImpl::HandleError[Fail][3]");
      break;
    }

    /// std::cout << "BaseAppManagerImpl::HandleError[Check Point][4]" << std::endl;
    YSOS_LOG_DEBUG_DEFAULT("BaseAppManagerImpl::HandleError[Check Point][4]");

    if (YSOS_ERROR_SUCCESS != RemoveSession(session_id)) {
      /// std::cout << "BaseAppManagerImpl::HandleError[Fail][4]" << std::endl;
      YSOS_LOG_DEBUG_DEFAULT("BaseAppManagerImpl::HandleError[Fail][4]");
      break;
    }

    /// std::cout << "BaseAppManagerImpl::HandleError[Check Point][6]" << std::endl;
    YSOS_LOG_DEBUG_DEFAULT("BaseAppManagerImpl::HandleError[Check Point][6]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  /// std::cout << "BaseAppManagerImpl::HandleError[Exit]" << std::endl;
  YSOS_LOG_DEBUG_DEFAULT("BaseAppManagerImpl::HandleError[Exit]");

  return;
}

int BaseAppManagerImpl::HandleConnected(const std::string& session_id, const PlatformRPCPtr& platform_sdk_ptr, const std::string& service_param) {
  std::string duplicated_service_param = service_param;
  return GetBaseSessionImpl()->Accept(session_id, platform_sdk_ptr, &duplicated_service_param);
}
}  /// end of namespace
