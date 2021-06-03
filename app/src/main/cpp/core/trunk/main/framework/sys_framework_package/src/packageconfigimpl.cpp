/**
 *@file PackageConfigImpl.cpp
 *@brief Definition of PackageConfigImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// Self Header //  NOLINT
#include "../../../protect/include/sys_framework_package/packageconfigimpl.h"
/// Stl Headers //  NOLINT
#include <algorithm>
#include <cassert>
#include <sstream>
/// Boost Headers //  NOLINT
#include <boost/make_shared.hpp>
#include <boost/algorithm/string.hpp>
/// Private Headers //  NOLINT
#include "../../../public/include/os_hal_package/filepath.h"
#include "../../../public/include/core_help_package/xmlutil.h"
// #include "../../../public/include/core_help_package/ysos_log.h"
#include "../../../public/include/sys_interface_package/baseinterface.h"
#include "../../../public/include/sys_framework_package/templatemanager.h"
#include "../../../public/include/sys_interface_package/moduleinterface.h"
#include "../../../public/include/sys_interface_package/ysos_enum.h"
#include "../../../protect/include/sys_framework_package/manager_declare.h"
#include "../../../protect/include/sys_framework_package/basemoduleimpl.h"
#include "../../../public/include/os_hal_package/lock.h"
#include "../../../public/include/sys_framework_package/messagetype.h"
#include "../../../protect/include/sys_framework_package/moduleutil.h"
#include "../../../protect/include/sys_framework_package/platformservice.h"

namespace ysos {
#define FILE_PATH_SEP  "/"
#define XML_EXTENSION  ".xml"

uint32_t ModuleLinkConfInfo::GetDepth() {
  uint32_t depth_temp = 0;
  LightLock lock__;
  AutoLockOper lock(&lock__);
  if (depth != 0xffffffffU) {
    depth_temp = depth;
  } else {
    for (ModuleInfoListInLinkIterator it = module_list.begin(); it != module_list.end(); ++it) {
      if (depth_temp < (*it)->instance_level) {
        depth_temp = (*it)->instance_level;
      }
    }
    depth = depth_temp;
  }
  return depth_temp;
}

/// service tag
static const char* SERVICE_ROOT_TAG = "service_list";
static const char* SERVICE_FIRST_LAYER_TAG = "service";
/// instance tag //  NOLINT
static const char* INSTANCE_ROOT_TAG = "instance_list";
static const char* INSTANCE_FIRST_LAYER_TAG = "instance";

boost::shared_ptr<LightLock> PackageConfigImpl::light_lock_ptr_
  = boost::make_shared<LightLock>();
PackageConfigImplPtr PackageConfigImpl::package_config_impl_ptr_ = NULL;

PackageConfigImpl::PackageConfigImpl() {
#if (CURRENT_PLATFORM == PLATFORM_WINDOWS)
  sys_conf_dir_ = "\\..\\conf\\";
#else   //CURRENT_PLATFORM != PLATFORM_WINDOWS
  sys_conf_dir_ = "/../conf/";
#endif
  sys_conf_dir_ = FilePath::GetCurPath() + sys_conf_dir_;

  logger_ = GetUtility()->GetLogger("ysos.framework");

  YSOS_LOG_DEBUG("sys_conf_dir is " << sys_conf_dir_);

  ip_address_ = "127.0.0.1";
  ip_port_ = 6002;

  /// module_conf_ptr_ = ConfMapPtr(new std::map<std::string, MapPtr >());
  /// callback_conf_ptr_ = ConfMapPtr(new std::map<std::string, MapPtr >());
}

PackageConfigImpl::~PackageConfigImpl() {
  module_conf_info_list_.clear();
  callback_conf_info_list_.clear();
  protocol_conf_info_list_.clear();
  module_conf_.clear();
  callback_conf_.clear();
  protocol_conf_.clear();
  driver_conf_.clear();
  connection_conf_.clear();

  if (0 != module_conf_info_list_.size()) {
    for (
      ModuleLinkConfInfoIterator iterator = module_link_conf_info_list_.begin();
      iterator != module_link_conf_info_list_.end();
      ++iterator
    ) {
      if (0 != (*iterator)->module_list.size()) {
        (*iterator)->module_list.clear();
      }
    }
    module_link_conf_info_list_.clear();
  }
}

const std::string PackageConfigImpl::GetConfPath(void) const {
  return sys_conf_dir_;
}

PackageConfigImplPtr PackageConfigImpl::instance() {
  if (NULL == package_config_impl_ptr_) {
    AutoLockOper lock(light_lock_ptr_.get());
    if (NULL == package_config_impl_ptr_) {
      package_config_impl_ptr_ = PackageConfigImplPtr(new PackageConfigImpl());
    }
  }

  return package_config_impl_ptr_;
}

MapPtr PackageConfigImpl::FindModuleConf(const std::string &module_name) {
  if (module_name.empty()) {
    return NULL;
  }

  ConfMap::iterator it = module_conf_.find(module_name);
  if (it == module_conf_.end()) {
    return NULL;
  }

  return it->second;
}

MapPtr PackageConfigImpl::FindCallbackConf(const std::string &callback_name) {
  if (callback_name.empty()) {
    return NULL;
  }

  ConfMap::iterator it = callback_conf_.find(callback_name);
  if (it == callback_conf_.end()) {
    return NULL;
  }

  return it->second;
}

MapPtr PackageConfigImpl::FindProtocolConf(const std::string &protocol_name) {
  if (protocol_name.empty()) {
    return NULL;
  }

  ConfMap::iterator it = protocol_conf_.find(protocol_name);
  if (it == protocol_conf_.end()) {
    return NULL;
  }

  return it->second;
}

MapPtr PackageConfigImpl::FindDriverConf(const std::string &driver_name) {
  if (driver_name.empty()) {
    return NULL;
  }

  ConfMap::iterator it = driver_conf_.find(driver_name);
  if (it == driver_conf_.end()) {
    return NULL;
  }

  return it->second;
}

MapPtr PackageConfigImpl::FindConnectionConf(const std::string &connection_name) {
  if (connection_name.empty()) {
    return NULL;
  }

  ConfMap::iterator it = connection_conf_.find(connection_name);
  if (it == connection_conf_.end()) {
    return NULL;
  }

  return it->second;
}

PackageConfigImpl::ModuleLinkConfInfoIterator PackageConfigImpl::ModuleLinkConfInfoHead(void) {
  return module_link_conf_info_list_.begin();
}
PackageConfigImpl::ModuleLinkConfInfoIterator PackageConfigImpl::ModuleLinkConfInfoEnd(void) {
  return module_link_conf_info_list_.end();
}
PackageConfigImpl::ModuleLinkConfInfoIterator PackageConfigImpl::FindModuleLinkConfInfo(const std::string &module_link_conf_info_name) {
  for (ModuleLinkConfInfoIterator it=module_link_conf_info_list_.begin();
       it != module_link_conf_info_list_.end(); ++it) {
    if ((*it)->name == module_link_conf_info_name) {
      return it;
    }
  }

  return module_link_conf_info_list_.end();
}

PackageConfigImpl::ModuleConfInfoIterator PackageConfigImpl::ModuleConfInfoHead(void) {
  return module_conf_info_list_.begin();
}
PackageConfigImpl::ModuleConfInfoIterator PackageConfigImpl::ModuleConfInfoEnd(void) {
  return module_conf_info_list_.end();
}
PackageConfigImpl::ModuleConfInfoIterator PackageConfigImpl::FindModuleConfInfo(const std::string &module_conf_info_name) {
  for (ModuleConfInfoIterator it=module_conf_info_list_.begin();
       it != module_conf_info_list_.end(); ++it) {
    if ((*it)->name == module_conf_info_name) {
      return it;
    }
  }

  return module_conf_info_list_.end();
}

PackageConfigImpl::CallbackConfInfoIterator PackageConfigImpl::CallbackConfInfoHead(void) {
  return callback_conf_info_list_.begin();
}
PackageConfigImpl::CallbackConfInfoIterator PackageConfigImpl::CallbackConfInfoEnd(void) {
  return callback_conf_info_list_.end();
}
PackageConfigImpl::CallbackConfInfoIterator PackageConfigImpl::FindCallbackConfInfo(const std::string &callback_conf_info_name) {
  for (CallbackConfInfoIterator it=callback_conf_info_list_.begin();
       it != callback_conf_info_list_.end(); ++it) {
    if ((*it)->name == callback_conf_info_name) {
      return it;
    }
  }

  return callback_conf_info_list_.end();
}

PackageConfigImpl::ProtocolConfInfoIterator PackageConfigImpl::ProtocolConfInfoHead(void) {
  return protocol_conf_info_list_.begin();
}

PackageConfigImpl::ProtocolConfInfoIterator PackageConfigImpl::ProtocolConfInfoEnd(void) {
  return protocol_conf_info_list_.end();
}

PackageConfigImpl::ProtocolConfInfoIterator PackageConfigImpl::FindProtocolConfInfo(const std::string &protocol_conf_info_name) {
  for (ProtocolConfInfoIterator it=protocol_conf_info_list_.begin();
       it != protocol_conf_info_list_.end(); ++it) {
    if ((*it)->name == protocol_conf_info_name) {
      return it;
    }
  }

  return protocol_conf_info_list_.end();
}

PackageConfigImpl::DriverConfInfoPtrIterator PackageConfigImpl::DriverConfInfoHead(void) {
  return driver_conf_info_list_.begin();
}

PackageConfigImpl::DriverConfInfoPtrIterator PackageConfigImpl::DriverConfInfoEnd(void) {
  return driver_conf_info_list_.end();
}

PackageConfigImpl::DriverConfInfoPtrIterator PackageConfigImpl::FindDriverConfInfo(const std::string &driver_conf_info_name) {
  for (DriverConfInfoPtrIterator it=driver_conf_info_list_.begin();
       it != driver_conf_info_list_.end(); ++it) {
    if ((*it)->name == driver_conf_info_name) {
      return it;
    }
  }

  return driver_conf_info_list_.end();
}

PackageConfigImpl::ConnectionConfInfoPtrIterator PackageConfigImpl::ConnectionConfInfoHead(void) {
  return connection_conf_info_list_.begin();
}

PackageConfigImpl::ConnectionConfInfoPtrIterator PackageConfigImpl::ConnectionConfInfoEnd(void) {
  return connection_conf_info_list_.end();
}

PackageConfigImpl::ConnectionConfInfoPtrIterator PackageConfigImpl::FindConnectionConfInfo(const std::string &connection_conf_info_name) {
  for (ConnectionConfInfoPtrIterator it=connection_conf_info_list_.begin();
       it != connection_conf_info_list_.end(); ++it) {
    if ((*it)->name == connection_conf_info_name) {
      return it;
    }
  }

  return connection_conf_info_list_.end();
}

InstanceInfoListIterator PackageConfigImpl::InstanceInfoHead(void) {
  return instance_info_list_.begin();
}

InstanceInfoListIterator PackageConfigImpl::InstanceInfoEnd(void) {
  return instance_info_list_.end();
}

InstanceInfoListIterator PackageConfigImpl::FindInstanceInfo(const std::string &instance_info_name) {
  for (InstanceInfoListIterator it=instance_info_list_.begin(); it != instance_info_list_.end(); ++it) {
    if ((*it)->logic_name == instance_info_name) {
      return it;
    }
  }

  return instance_info_list_.end();
}

tinyxml2::XMLElement *PackageConfigImpl::GetRootElement(
  tinyxml2::XMLDocument &doc,
  const std::string &file_name,
  const std::string &next_element) {
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

int PackageConfigImpl::ParseSysInfoFile(const std::string &configure_file) {
  tinyxml2::XMLDocument doc;

  tinyxml2::XMLElement *root = GetRootElement(doc, sys_conf_dir_ + configure_file);
  if (NULL == root) {
    YSOS_LOG_DEBUG("Parse file: " << configure_file << "failed");
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  sys_conf_info_.connection_info_ptr = ParseSysConfInfoElement(root->FirstChildElement("connection"));
  sys_conf_info_.driver_info_ptr = ParseSysConfInfoElement(root->FirstChildElement("driver"));
  sys_conf_info_.protocol_info_ptr = ParseSysConfInfoElement(root->FirstChildElement("protocol"));
  sys_conf_info_.callback_info_ptr = ParseSysConfInfoElement(root->FirstChildElement("callback"));
  sys_conf_info_.module_info_ptr = ParseSysConfInfoElement(root->FirstChildElement("module"));
  sys_conf_info_.module_link_info_ptr = ParseSysConfInfoElement(root->FirstChildElement("module_link"));
  sys_conf_info_.service_info_list_ptr = ParseSysConfInfoElement(root->FirstChildElement("service"));
  sys_conf_info_.event_info_ptr = ParseSysConfInfoElement(root->FirstChildElement("event"));
  sys_conf_info_.instance_info_ptr = ParseSysConfInfoElement(root->FirstChildElement("instance"));
  sys_conf_info_.appmanager_info_ptr = ParseSysConfInfoElement(root->FirstChildElement("appmanager"));
  ParseServerInfo(root->FirstChildElement("server"));

  ParseConfigureFile();

  return YSOS_ERROR_SUCCESS;
}

const std::string PackageConfigImpl::GetServerAddress(void) const {
  return ip_address_;
}
const uint16_t PackageConfigImpl::GetServerPort(void) const {
  return ip_port_;
}

std::string PackageConfigImpl::GetAppManagerConf(void) const {
  assert(NULL != sys_conf_info_.appmanager_info_ptr);

  return sys_conf_info_.appmanager_info_ptr->dir_name;
}

int PackageConfigImpl::ParseServerInfo(tinyxml2::XMLElement *element) {
  if (NULL == element) {
    YSOS_LOG_ERROR_DEFAULT("not exist server info");
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  std::string ip_address = GetXmlUtil()->GetElementTextValue(element, "ip_address");
  int32_t ip_port = (uint16_t)GetXmlUtil()->GetElementInt32Value(element, "ip_port");
  if (!ip_address.empty()) {
    ip_address_ = ip_address;
  }

  if (0 != ip_port) {
    ip_port_ = ip_port;
  }

  YSOS_LOG_DEBUG_DEFAULT("server ip address: " << ip_address_ << "   ip port: " << ip_port_);

  return YSOS_ERROR_SUCCESS;
}

SysConfInfoElementPtr  PackageConfigImpl::ParseSysConfInfoElement(tinyxml2::XMLElement *element) {
  SysConfInfoElementPtr conf_info;
  if (NULL == element) {
    return conf_info;
  }

  tinyxml2::XMLElement *dirname = element->FirstChildElement("dirname");
  if (NULL == dirname) {
    return conf_info;
  }

  conf_info = boost::make_shared<SysConfInfoElement>();
  if (NULL != dirname->Attribute("name")) {
    conf_info->dir_name
      = sys_conf_dir_ + dirname->Attribute("name") + FILE_PATH_SEP;
  }

  tinyxml2::XMLElement *exclude = dirname->FirstChildElement("exclude");
  while (NULL != exclude) {
    const char *text = exclude->GetText();
    if (NULL != text) {
      conf_info->exclude_file_list.push_back(text);
    }

    exclude = exclude->NextSiblingElement("exclude");
  }

  return conf_info;
}

int PackageConfigImpl::ParseConfigureFile(void) {
  int ret = ParseModuleConfInfo(sys_conf_info_.module_info_ptr);
  if (0 != ret) {
    YSOS_LOG_DEBUG(" ParseModuleConfInfo failed: " << ret);
  }

  ret = ParseCallbackConfInfo(sys_conf_info_.callback_info_ptr);
  if (0 != ret) {
    YSOS_LOG_DEBUG(" ParseCallbackConfInfo failed: " << ret);
  }

  ret = ParseProtocolConfInfo(sys_conf_info_.protocol_info_ptr);
  if (0 != ret) {
    YSOS_LOG_DEBUG(" ParseProtocolConfInfo failed: " << ret);
  }

  ret = ParseDriverConfInfo(sys_conf_info_.driver_info_ptr);
  if (0 != ret) {
    YSOS_LOG_DEBUG(" ParseDriverConfInfo failed: " << ret);
  }

  ret = ParseConnectionConfInfo(sys_conf_info_.connection_info_ptr);
  if (0 != ret) {
    YSOS_LOG_DEBUG(" ParseDriverConfInfo failed: " << ret);
  }

  /// ModuleLink中的知名Repeater依赖于Callback
  ret = ParseModuleLinkConfInfo(sys_conf_info_.module_link_info_ptr);
  if (0 != ret) {
    YSOS_LOG_DEBUG(" ParseModuleLinkConfInfo failed: " << ret);
  }

  ret = ParseServiceConfInfoList(sys_conf_info_.service_info_list_ptr);
  if (0 != ret) {
    YSOS_LOG_DEBUG(" ParseServiceConfInfo failed: " << ret);
  }

  ret = PaseEventConfInfo(sys_conf_info_.event_info_ptr);
  if (0 != ret) {
    YSOS_LOG_DEBUG(" PaseEventConfInfo failed: " << ret);
  }

  ret = PaseInstanceConfInfo(sys_conf_info_.instance_info_ptr);
  if (0 != ret) {
    YSOS_LOG_DEBUG(" PaseInstanceConfInfo failed: " << ret);
  }

  return YSOS_ERROR_SUCCESS;
}

int PackageConfigImpl::ParseModuleConfInfo(
  SysConfInfoElementPtr module_info_ptr) {
  if (NULL == module_info_ptr) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  if (!FilePath::IsDirectory(module_info_ptr->dir_name)) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  const std::string xml_ext(XML_EXTENSION);
  std::vector<std::string> file_list
    = FilePath::GetAllFiles(module_info_ptr->dir_name);
  for (std::vector<std::string>::iterator it = file_list.begin();
       it!=file_list.end(); ++it) {
    FilePath file_path(*it);
    if (file_path.GetFileExtention() != xml_ext) {
      continue;
    }

    boost::filesystem::path path(*it);
    if (std::find(module_info_ptr->exclude_file_list.begin(),
                  module_info_ptr->exclude_file_list.end(),
                  path.filename().string()) != module_info_ptr->exclude_file_list.end()) {
      continue;
    }

    ModuleConfInfoPtr module_conf_info_ptr = ParseModuleConfInfoImpl((*it));
    if (NULL != module_conf_info_ptr) {
      module_conf_info_list_.push_back(module_conf_info_ptr);
    }
  }

  return YSOS_ERROR_SUCCESS;
}

ModuleConfInfoPtr PackageConfigImpl::ParseModuleConfInfoImpl(const std::string &file_name) {
  tinyxml2::XMLDocument doc;
  tinyxml2::XMLElement *root = GetRootElement(doc, file_name, "module");
  if (NULL == root) {
    YSOS_LOG_DEBUG("Parse file: " << file_name << " failed");
    return ModuleConfInfoPtr();
  }

  ModuleConfInfoPtr module_conf_info_ptr = boost::make_shared<ModuleConfInfo>();
  module_conf_info_ptr->name = GetXmlUtil()->GetElementTextValue(root, "name");
  module_conf_info_ptr->type  = GetXmlUtil()->GetElementTextValue(root, "type");
  module_conf_info_ptr->version  = GetXmlUtil()->GetElementTextValue(root, "version");
  module_conf_info_ptr->is_local = GetXmlUtil()->GetElementBoolValue(root, "local");
  module_conf_info_ptr->uri = GetXmlUtil()->GetElementTextValue(root, "uri");
  module_conf_info_ptr->input_type = GetXmlUtil()->GetElementTextValue(root, "in_datatypes");
  module_conf_info_ptr->output_type = GetXmlUtil()->GetElementTextValue(root, "out_datatypes");
  module_conf_info_ptr->is_allocated = GetXmlUtil()->GetElementBoolValue(root, "allocator");
  module_conf_info_ptr->prefix = GetXmlUtil()->GetElementInt64Value(root, "prefix");
  module_conf_info_ptr->buffer_length = GetXmlUtil()->GetElementInt64Value(root, "buffer_length");
  module_conf_info_ptr->is_revised = GetXmlUtil()->GetElementBoolValue(root, "revise");
  module_conf_info_ptr->buffer_number = GetXmlUtil()->GetElementInt64Value(root, "buffer_number");
  module_conf_info_ptr->capacity = GetXmlUtil()->GetElementInt64Value(root, "capacity");
  module_conf_info_ptr->remark = GetXmlUtil()->GetElementTextValue(root, "remark");

  std::string module_name = GetXmlUtil()->GetElementTextValue(root, "name");
  if (module_conf_.end() != module_conf_.find(module_name)) {
    YSOS_LOG_DEBUG("module :" << module_name << " get more than one config xml");
    return module_conf_info_ptr;
  }
  MapPtr module_conf = MapPtr(new std::multimap<std::string, std::string>());
  tinyxml2::XMLElement *child = root->FirstChildElement();
  while (NULL != child) {
    const char *node_name = child->Name();
    const char *node_text = child->GetText();
    if (NULL == node_name || NULL == node_text) {
      YSOS_LOG_DEBUG("parse child element failed");
      child = child->NextSiblingElement();
      continue;
    }
//     if(module_conf->end() != module_conf->find(node_name)) {
//       YSOS_LOG << node_name << " already exist\n";
//       child = child->NextSiblingElement();
//       continue;
//     }
    module_conf->insert(std::make_pair(node_name, node_text));

    child = child->NextSiblingElement();
  }
  module_conf_.insert(std::make_pair(module_name, module_conf));

  return module_conf_info_ptr;
}

int PackageConfigImpl::ParseCallbackConfInfo(SysConfInfoElementPtr callback_info_ptr) {
  if (NULL == callback_info_ptr) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  if (!FilePath::IsDirectory(callback_info_ptr->dir_name)) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  const std::string xml_ext(XML_EXTENSION);
  std::vector<std::string> file_list = FilePath::GetAllFiles(callback_info_ptr->dir_name);
  for (std::vector<std::string>::iterator it = file_list.begin(); it != file_list.end(); ++it) {
    FilePath file_path(*it);
    if (file_path.GetFileExtention() != xml_ext) {
      continue;
    }

    boost::filesystem::path path(*it);
    std::list<std::string>::iterator callback_it = std::find(callback_info_ptr->exclude_file_list.begin(), callback_info_ptr->exclude_file_list.end(), path.filename().string());
    if (callback_it != callback_info_ptr->exclude_file_list.end()) {
      continue;
    }

    CallbackConfInfoPtr callback_conf_info_ptr = ParseCallbackConfInfoImpl((*it));
    if (NULL != callback_conf_info_ptr) {
      callback_conf_info_list_.push_back(callback_conf_info_ptr);
    }
  }

  return YSOS_ERROR_SUCCESS;
}

ProtocolConfInfoPtr PackageConfigImpl::ParseProtocolConfInfoImpl(const std::string &file_name) {
  tinyxml2::XMLDocument doc;
  tinyxml2::XMLElement *root = GetRootElement(doc, file_name, "protocol");
  if (NULL == root) {
    YSOS_LOG_DEBUG("Parse file: " << file_name << " failed");
    return NULL;
  }

  ProtocolConfInfoPtr protocol_conf_info_ptr = boost::make_shared<ProtocolConfInfo>();

  protocol_conf_info_ptr->name = GetXmlUtil()->GetElementTextValue(root, "name");
  protocol_conf_info_ptr->owner = GetXmlUtil()->GetElementTextValue(root, "owner");
  protocol_conf_info_ptr->version = GetXmlUtil()->GetElementTextValue(root, "version");
  protocol_conf_info_ptr->remark = GetXmlUtil()->GetElementTextValue(root, "remark");

  std::string protocol_name = GetXmlUtil()->GetElementTextValue(root, "name");
  if (protocol_conf_.end() != protocol_conf_.find(protocol_name)) {
    YSOS_LOG_DEBUG("Protocol :" << protocol_name << " get more than one config xml");
    return protocol_conf_info_ptr;
  }
  MapPtr protocol_conf = MapPtr(new std::multimap<std::string, std::string>());
  tinyxml2::XMLElement *child = root->FirstChildElement();
  while (NULL != child) {
    const char *node_name = child->Name();
    const char *node_text = child->GetText();
    if (NULL == node_name || NULL == node_text) {
      YSOS_LOG_DEBUG("parse child element failed");
      child = child->NextSiblingElement();
      continue;
    }

    protocol_conf->insert(std::make_pair(node_name, node_text));

    child = child->NextSiblingElement();
  }
  protocol_conf_.insert(std::make_pair(protocol_name, protocol_conf));

  return protocol_conf_info_ptr;
}

int PackageConfigImpl::ParseProtocolConfInfo(SysConfInfoElementPtr protocol_info_ptr) {
  if (NULL == protocol_info_ptr) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  if (!FilePath::IsDirectory(protocol_info_ptr->dir_name)) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  const std::string xml_ext(XML_EXTENSION);
  std::vector<std::string> file_list = FilePath::GetAllFiles(protocol_info_ptr->dir_name);
  for (std::vector<std::string>::iterator it = file_list.begin(); it != file_list.end(); ++it) {
    FilePath file_path(*it);
    if (file_path.GetFileExtention() != xml_ext) {
      continue;
    }

    boost::filesystem::path path(*it);
    std::list<std::string>::iterator callback_it = std::find(protocol_info_ptr->exclude_file_list.begin(), protocol_info_ptr->exclude_file_list.end(), path.filename().string());
    if (callback_it != protocol_info_ptr->exclude_file_list.end()) {
      continue;
    }

    ProtocolConfInfoPtr protocol_conf_info_ptr = ParseProtocolConfInfoImpl((*it));
    if (NULL != protocol_conf_info_ptr) {
      protocol_conf_info_list_.push_back(protocol_conf_info_ptr);
    }
  }

  return YSOS_ERROR_SUCCESS;
}

DriverConfInfoPtr PackageConfigImpl::ParseDriverConfInfoImpl(const std::string &file_name) {
  tinyxml2::XMLDocument doc;
  tinyxml2::XMLElement *root = GetRootElement(doc, file_name, "driver");
  if (NULL == root) {
    YSOS_LOG_DEBUG("Parse file: " << file_name << " failed");
    return NULL;
  }

  DriverConfInfoPtr driver_conf_info_ptr = boost::make_shared<DriverConfInfo>();

  driver_conf_info_ptr->name = GetXmlUtil()->GetElementTextValue(root, "name");
  driver_conf_info_ptr->version = GetXmlUtil()->GetElementTextValue(root, "version");
  driver_conf_info_ptr->remark = GetXmlUtil()->GetElementTextValue(root, "remark");

  std::string driver_name = GetXmlUtil()->GetElementTextValue(root, "name");
  if (driver_conf_.end() != driver_conf_.find(driver_name)) {
    YSOS_LOG_DEBUG("Driver :" << driver_name << " get more than one config xml");
    return driver_conf_info_ptr;
  }
  MapPtr driver_conf = MapPtr(new std::multimap<std::string, std::string>());
  tinyxml2::XMLElement *child = root->FirstChildElement();
  while (NULL != child) {
    const char *node_name = child->Name();
    const char *node_text = child->GetText();
    if (NULL == node_name || NULL == node_text) {
      YSOS_LOG_DEBUG("parse child element failed");
      child = child->NextSiblingElement();
      continue;
    }

    driver_conf->insert(std::make_pair(node_name, node_text));

    child = child->NextSiblingElement();
  }
  driver_conf_.insert(std::make_pair(driver_name, driver_conf));

  return driver_conf_info_ptr;
}

int PackageConfigImpl::ParseDriverConfInfo(SysConfInfoElementPtr driver_info_ptr) {
  if (NULL == driver_info_ptr) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  if (!FilePath::IsDirectory(driver_info_ptr->dir_name)) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  const std::string xml_ext(XML_EXTENSION);
  std::vector<std::string> file_list = FilePath::GetAllFiles(driver_info_ptr->dir_name);
  for (std::vector<std::string>::iterator it = file_list.begin(); it != file_list.end(); ++it) {
    FilePath file_path(*it);
    if (file_path.GetFileExtention() != xml_ext) {
      continue;
    }

    boost::filesystem::path path(*it);
    std::list<std::string>::iterator driver_it = std::find(driver_info_ptr->exclude_file_list.begin(), driver_info_ptr->exclude_file_list.end(), path.filename().string());
    if (driver_it != driver_info_ptr->exclude_file_list.end()) {
      continue;
    }

    DriverConfInfoPtr driver_conf_info_ptr = ParseDriverConfInfoImpl((*it));
    if (NULL != driver_conf_info_ptr) {
      driver_conf_info_list_.push_back(driver_conf_info_ptr);
    }
  }

  return YSOS_ERROR_SUCCESS;
}

ConnectionConfInfoPtr PackageConfigImpl::ParseConnectionConfInfoImpl(const std::string &file_name) {
  tinyxml2::XMLDocument doc;
  tinyxml2::XMLElement *root = GetRootElement(doc, file_name, "connection");
  if (NULL == root) {
    YSOS_LOG_DEBUG("Parse file: " << file_name << " failed");
    return NULL;
  }

  ConnectionConfInfoPtr connection_conf_info_ptr = boost::make_shared<ConnectionConfInfo>();

  connection_conf_info_ptr->name = GetXmlUtil()->GetElementTextValue(root, "name");
  connection_conf_info_ptr->version = GetXmlUtil()->GetElementTextValue(root, "version");
  connection_conf_info_ptr->remark = GetXmlUtil()->GetElementTextValue(root, "remark");

  std::string connection_name = GetXmlUtil()->GetElementTextValue(root, "name");
  if (connection_conf_.end() != connection_conf_.find(connection_name)) {
    YSOS_LOG_DEBUG("Connection :" << connection_name << " get more than one config xml");
    return connection_conf_info_ptr;
  }
  MapPtr connection_conf = MapPtr(new std::multimap<std::string, std::string>());
  tinyxml2::XMLElement *child = root->FirstChildElement();
  while (NULL != child) {
    const char *node_name = child->Name();
    const char *node_text = child->GetText();
    if (NULL == node_name || NULL == node_text) {
      YSOS_LOG_DEBUG("parse child element failed");
      child = child->NextSiblingElement();
      continue;
    }

    connection_conf->insert(std::make_pair(node_name, node_text));

    child = child->NextSiblingElement();
  }
  connection_conf_.insert(std::make_pair(connection_name, connection_conf));

  return connection_conf_info_ptr;
}

int PackageConfigImpl::ParseConnectionConfInfo(SysConfInfoElementPtr connection_info_ptr) {
  if (NULL == connection_info_ptr) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  if (!FilePath::IsDirectory(connection_info_ptr->dir_name)) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  const std::string xml_ext(XML_EXTENSION);
  std::vector<std::string> file_list = FilePath::GetAllFiles(connection_info_ptr->dir_name);
  for (std::vector<std::string>::iterator it = file_list.begin(); it != file_list.end(); ++it) {
    FilePath file_path(*it);
    if (file_path.GetFileExtention() != xml_ext) {
      continue;
    }

    boost::filesystem::path path(*it);
    std::list<std::string>::iterator connection_it = std::find(connection_info_ptr->exclude_file_list.begin(), connection_info_ptr->exclude_file_list.end(), path.filename().string());
    if (connection_it != connection_info_ptr->exclude_file_list.end()) {
      continue;
    }

    DriverConfInfoPtr connection_conf_info_ptr = ParseConnectionConfInfoImpl((*it));
    if (NULL != connection_conf_info_ptr) {
      connection_conf_info_list_.push_back(connection_conf_info_ptr);
    }
  }

  return YSOS_ERROR_SUCCESS;
}

int PackageConfigImpl::ParseServiceConfInfoList(const SysConfInfoElementPtr service_info_list_ptr) {
  if (NULL == service_info_list_ptr) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  if (!FilePath::IsDirectory(service_info_list_ptr->dir_name)) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  const std::string xml_ext(XML_EXTENSION);
  std::vector<std::string> file_list = FilePath::GetAllFiles(service_info_list_ptr->dir_name);
  for (std::vector<std::string>::iterator it = file_list.begin(); it != file_list.end(); ++it) {
    FilePath file_path(*it);
    if (file_path.GetFileExtention() != xml_ext) {
      continue;
    }

    boost::filesystem::path path(*it);
    if (std::find(service_info_list_ptr->exclude_file_list.begin(),
                  service_info_list_ptr->exclude_file_list.end(),
                  path.filename().string())
        != service_info_list_ptr->exclude_file_list.end()) {
      continue;
    }

    if (YSOS_ERROR_SUCCESS != GetPlatformService()->ParseService(*it)) {
      YSOS_LOG_DEBUG("Parse file: " << *it << " failed");
    }

    if (YSOS_ERROR_SUCCESS != ParseServiceConfInfoListImpl((*it))) {
      continue;
    }
  }

  GetPlatformService()->Dump();

  return YSOS_ERROR_SUCCESS;
}

int PackageConfigImpl::ParseServiceConfInfoListImpl(const std::string& file_name) {
  tinyxml2::XMLDocument doc;
  tinyxml2::XMLElement *root = GetRootElement(doc, file_name, SERVICE_ROOT_TAG);
  if (NULL == root) {
    YSOS_LOG_DEBUG("Parse file: " << file_name << " failed");
    return YSOS_ERROR_FAILED;
  }

  tinyxml2::XMLElement *first_layer = root->FirstChildElement(SERVICE_FIRST_LAYER_TAG);
  while (NULL != first_layer) {
    ServiceInfoTable::ServiceConfInfoPtr service_conf_info_ptr = ParseServiceConfInfoImpl(first_layer);
    if (NULL != service_conf_info_ptr) {
      ServiceInfoTable::Instance()->AddServiceConfInfo(service_conf_info_ptr);
    }

    first_layer = first_layer->NextSiblingElement(SERVICE_FIRST_LAYER_TAG);
  }

  return YSOS_ERROR_SUCCESS;
}

int PackageConfigImpl::PaseEventConfInfo(SysConfInfoElementPtr event_info_ptr) {
  if (NULL == event_info_ptr) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  if (!FilePath::IsDirectory(event_info_ptr->dir_name)) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  const std::string xml_ext(XML_EXTENSION);
  std::vector<std::string> file_list
    = FilePath::GetAllFiles(event_info_ptr->dir_name);
  for (std::vector<std::string>::iterator it = file_list.begin();
       it!=file_list.end(); ++it) {
    FilePath file_path(*it);
    if (file_path.GetFileExtention() != xml_ext) {
      continue;
    }

    boost::filesystem::path path(*it);
    if (std::find(event_info_ptr->exclude_file_list.begin(),
                  event_info_ptr->exclude_file_list.end(),
                  path.filename().string()) != event_info_ptr->exclude_file_list.end()) {
      continue;
    }

    if (YSOS_ERROR_SUCCESS != MessageType::Instance()->ParseMessageType((*it))) {
      return YSOS_ERROR_FAILED;
    }

  }

  return YSOS_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////////
/////////////       Parse Instance               //////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
int PackageConfigImpl::PaseInstanceConfInfo(SysConfInfoElementPtr instance_info_ptr) {
  if (NULL == instance_info_ptr) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  if (!FilePath::IsDirectory(instance_info_ptr->dir_name)) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  const std::string xml_ext(XML_EXTENSION);
  std::vector<std::string> file_list = FilePath::GetAllFiles(instance_info_ptr->dir_name);
  for (std::vector<std::string>::iterator it = file_list.begin();  it!=file_list.end(); ++it) {
    FilePath file_path(*it);
    if (file_path.GetFileExtention() != xml_ext) {
      continue;
    }

    boost::filesystem::path path(*it);
    std::list<std::string> &exclude_file_list = instance_info_ptr->exclude_file_list;
    std::list<std::string>::iterator info_it = std::find(exclude_file_list.begin(), exclude_file_list.end(), path.filename().string());
    if (info_it != instance_info_ptr->exclude_file_list.end()) {
      continue;
    }

    int ret = PaseInstanceConfInfoImpl(*it);
    if (YSOS_ERROR_SUCCESS != ret) {
      YSOS_LOG_DEBUG("Parse file: " << *it << " failed");
    }

  }

  return YSOS_ERROR_SUCCESS;
}

int PackageConfigImpl::PaseInstanceConfInfoImpl(const std::string& file_name) {
  tinyxml2::XMLDocument doc;
  tinyxml2::XMLElement *root = GetRootElement(doc, file_name, INSTANCE_ROOT_TAG);
  if (NULL == root) {
    YSOS_LOG_DEBUG("Parse file: " << file_name << " failed");
    return YSOS_ERROR_FAILED;
  }

  tinyxml2::XMLElement *first_layer = root->FirstChildElement(INSTANCE_FIRST_LAYER_TAG);
  while (NULL != first_layer) {
    InstanceInfoPtr instance_info_ptr = PaseInstanceConfInfoImpl(first_layer);
    if (NULL != instance_info_ptr) {
      instance_info_list_.push_back(instance_info_ptr);
    }

    first_layer = first_layer->NextSiblingElement(INSTANCE_FIRST_LAYER_TAG);
  }

  return YSOS_ERROR_SUCCESS;
}

InstanceInfoPtr PackageConfigImpl::PaseInstanceConfInfoImpl(tinyxml2::XMLElement* element) {
  if (NULL == element) {
    return NULL;
  }

  InstanceInfoPtr instance_info_ptr = boost::make_shared<InstanceInfo>();
  if (NULL == instance_info_ptr) {
    return NULL;
  }

  // type
  instance_info_ptr->type = GetXmlUtil()->GetElementTextAttribute(element, "type");
  // logic_name
  instance_info_ptr->logic_name = GetXmlUtil()->GetElementTextValue(element, "name");
  // class_name
  instance_info_ptr->class_name = GetXmlUtil()->GetElementTextValue(element, "class_name");

  return instance_info_ptr;
}

ServiceInfoTable::ServiceConfInfoPtr
PackageConfigImpl::ParseServiceConfInfoImpl(tinyxml2::XMLElement* element) {
  if (NULL == element) {
    return NULL;
  }

  ServiceInfoTable::ServiceConfInfoPtr service_conf_info_ptr
    = boost::make_shared<ServiceInfoTable::ServiceConfInfo>();

  //ID
  static UINT32 service_ID = 1;
  service_conf_info_ptr->ID = service_ID;
  ++service_ID;

  // name
  if (NULL != element->Attribute("name")) {
    service_conf_info_ptr->name = element->Attribute("name");
  }

  bool non_other_service_attribute = true;

  // type
  if (NULL != element->Attribute("type")) {
    service_conf_info_ptr->type = element->Attribute("type");
    non_other_service_attribute = false;
  }

  // callback
  if (NULL != element->Attribute("callback")) {
    service_conf_info_ptr->callback = element->Attribute("callback");
    non_other_service_attribute = false;
  }

  // is_default
  if (NULL != element->Attribute("default")) {
    service_conf_info_ptr->is_default = element->Attribute("default");
    non_other_service_attribute = false;
  }

  // description
  service_conf_info_ptr->description
    = GetXmlUtil()->GetElementTextValue(element, "description");

  // sub_services
  bool non_other_sub_service_attribute = true;
  tinyxml2::XMLElement *child = element->FirstChildElement("service");

  if (NULL != child) {
    while (NULL != child) {
      ServiceInfoTable::ServiceConfInfoPtr sub_service_conf_info_ptr
        = ParseServiceConfInfoImpl(child);

      if (true != sub_service_conf_info_ptr->type.empty() ||
          true != sub_service_conf_info_ptr->callback.empty() ||
          true != sub_service_conf_info_ptr->is_default.empty()) {
        non_other_sub_service_attribute = false;
      }

      // is module
      if (true == non_other_service_attribute
          && true == non_other_sub_service_attribute) {
        // instance_name
        service_conf_info_ptr->instance_name
          = SeekModuleInstanceName(sub_service_conf_info_ptr->name);
      }

      service_conf_info_ptr->sub_services.push_back(sub_service_conf_info_ptr);
      child = child->NextSiblingElement("service");
    }
  } else {

    // is module
    if (true == non_other_service_attribute) {
      // instance_name
      service_conf_info_ptr->instance_name
        = SeekModuleInstanceName(service_conf_info_ptr->name);
    }

    // is callback
    if (
      true != service_conf_info_ptr->type.empty() &&
      "callback" == service_conf_info_ptr->type) {
      // instance_name
      service_conf_info_ptr->instance_name
        = SeekCallbackInstanceName(service_conf_info_ptr->name);
    }
  }

  return service_conf_info_ptr;
}

std::string PackageConfigImpl::SeekModuleInstanceName(const std::string& sub_service_name) {
  std::string found_name = "";

  if (true == sub_service_name.empty()) {
    return found_name;
  }

  // Seek Module Link List
  for (
    ModuleLinkConfInfoIterator iterator1= module_link_conf_info_list_.begin();
    iterator1 != module_link_conf_info_list_.end();
    ++iterator1) {
    if (0 != (*iterator1)->module_list.size()) {
      for (
        ModuleLinkModuleConfInfoIterator iterator3
        = (*iterator1)->module_list.begin();
        iterator3 != (*iterator1)->module_list.end();
        ++iterator3) {
        if (sub_service_name == (*iterator3)->instance_name) {
          found_name = (*iterator3)->instance_name;
          return found_name;
        }
      }
    }
  }

  return found_name;
}

std::string PackageConfigImpl::SeekCallbackInstanceName(const std::string& service_name) {
  std::string found_name = "";

  if (true == service_name.empty()) {
    return found_name;
  }

  // Seek Callback List
  for (
    CallbackConfInfoIterator iterator2 = callback_conf_info_list_.begin();
    iterator2 != callback_conf_info_list_.end();
    ++iterator2) {
    if (service_name == (*iterator2)->instance_name) {
      found_name = (*iterator2)->instance_name;
      break;
    }
  }

  return found_name;
}

CallbackConfInfoPtr PackageConfigImpl::ParseCallbackConfInfoImpl(const std::string &file_name) {
  YSOS_LOG_DEBUG("ParseCallbackConfInfo: " << file_name);
  tinyxml2::XMLDocument doc;
  tinyxml2::XMLElement *root = GetRootElement(doc, file_name, "callback");
  if (NULL == root) {
    YSOS_LOG_DEBUG("Parse file: " << file_name << " failed");
    return CallbackConfInfoPtr();
  }

  CallbackConfInfoPtr callback_conf_info_ptr = boost::make_shared<CallbackConfInfo>();

  callback_conf_info_ptr->name = GetXmlUtil()->GetElementTextValue(root, "name");
  callback_conf_info_ptr->instance_name = GetXmlUtil()->GetElementTextValue(root, "instance_name");
  callback_conf_info_ptr->owner = GetXmlUtil()->GetElementTextValue(root, "owner");
  callback_conf_info_ptr->version = GetXmlUtil()->GetElementTextValue(root, "version");
  callback_conf_info_ptr->remark = GetXmlUtil()->GetElementTextValue(root, "remark");

  std::string callback_name = GetXmlUtil()->GetElementTextValue(root, "name");
  if (callback_conf_.end() != callback_conf_.find(callback_name)) {
    YSOS_LOG_DEBUG("callback :" << callback_name << " get more than one config xml");
    return callback_conf_info_ptr;
  }
  MapPtr callback_conf = MapPtr(new std::multimap<std::string, std::string>());
  tinyxml2::XMLElement *child = root->FirstChildElement();
  while (NULL != child) {
    const char *node_name = child->Name();
    const char *node_text = child->GetText();
    if (NULL == node_name || NULL == node_text) {
      // YSOS_LOG_DEBUG("parse child element failed");
      child = child->NextSiblingElement();
      continue;
    }
//     if(callback_conf->end() != callback_conf->find(node_name)) {
//       YSOS_LOG << node_name << " already exist\n";
//       child = child->NextSiblingElement();
//       continue;
//     }
    callback_conf->insert(std::make_pair(node_name, node_text));

    child = child->NextSiblingElement();
  }
  callback_conf_.insert(std::make_pair(callback_name, callback_conf));
  YSOS_LOG_DEBUG("ParseCallbackConfInfo add new callback config: " << callback_name);

  return callback_conf_info_ptr;
}

int PackageConfigImpl::ParseModuleLinkConfInfo(SysConfInfoElementPtr module_link_info_ptr) {
  if (NULL == module_link_info_ptr) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  if (!FilePath::IsDirectory(module_link_info_ptr->dir_name)) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  const std::string xml_ext(XML_EXTENSION);
  std::vector<std::string> file_list = FilePath::GetAllFiles(
                                         module_link_info_ptr->dir_name);
  for (std::vector<std::string>::iterator it
       = file_list.begin(); it!=file_list.end(); ++it) {
    FilePath file_path(*it);
    if (file_path.GetFileExtention() != xml_ext) {
      continue;
    }

    boost::filesystem::path path(*it);
    if (std::find(module_link_info_ptr->exclude_file_list.begin(),
                  module_link_info_ptr->exclude_file_list.end(),
                  path.filename().string())
        != module_link_info_ptr->exclude_file_list.end()) {
      continue;
    }

    ModuleLinkConfInfoPtr module_link_conf_info_ptr
      = ParseModuleLinkConfInfoImpl((*it));
    if (NULL != module_link_conf_info_ptr) {
      module_link_conf_info_list_.push_back(module_link_conf_info_ptr);
    }
  }

  return YSOS_ERROR_SUCCESS;
}

ModuleLinkConfInfoPtr PackageConfigImpl::ParseModuleLinkConfInfoImpl(const std::string &file_name) {
  tinyxml2::XMLDocument doc;
  tinyxml2::XMLElement *root = GetRootElement(doc, file_name, "module_link");
  if (NULL == root) {
    YSOS_LOG_DEBUG("Parse file: " << file_name << " failed");
    return ModuleLinkConfInfoPtr();
  }

  ModuleLinkConfInfoPtr module_link_conf_info_ptr
    = boost::make_shared<ModuleLinkConfInfo>();
  int id = 0;
  if (tinyxml2::XML_NO_ERROR == root->QueryIntAttribute("id", &id)) {
    module_link_conf_info_ptr->id = id;
  }
  module_link_conf_info_ptr->name = GetXmlUtil()->GetElementTextValue(root, "name");

  module_link_conf_info_ptr->remark = GetXmlUtil()->GetElementTextValue(root, "remark");

  module_link_conf_info_ptr->autorun = GetXmlUtil()->GetElementBoolValue(root, "autorun");

  tinyxml2::XMLElement *child = root->FirstChildElement("moduleinstance");
  while (NULL != child) {
    ModuleLinkModuleConfInfoPtr module_link_module_conf_info_ptr
      = ParseModuleLinkModuleConfInfoImpl(child);
    module_link_conf_info_ptr->module_list.push_back(
      module_link_module_conf_info_ptr);
    child = child->NextSiblingElement("moduleinstance");
  }

  /// 获取知名Repeater信息
  GetFamousRepeaterInfo(module_link_conf_info_ptr);

  /// 更新ModuleList为创建Repeater做准备
  UpdateModuleLinkModuleConfInfoImpl(module_link_conf_info_ptr->module_list);

  /// 生成内置Repeater信息
  GenerateInherentRepeaterList(module_link_conf_info_ptr);

  /// 由知名Repeater信息更新内置Repeater信息
  UpdateInherentRepeaterListByFamousRepeaterInfo(module_link_conf_info_ptr);

#if (CURRENT_PLATFORM_PHASE == PLATFORM_PHASE_DEBUG)
  //  Dump prev_modules
  for (InherentRepeaterConfInfoMapIterator it
       = module_link_conf_info_ptr->repeater_info_list.begin();
       it != module_link_conf_info_ptr->repeater_info_list.end(); ++it) {

    InherentRepeaterConfInfo& repeater_config_info = it->second;
    YSOS_LOG_DEBUG("The repeater after level " << (*it).first
                   << " with name " << repeater_config_info.name
                   << " with famous name " << repeater_config_info.famous_name
                   << " has prev module list: ");
    for (std::list<std::string>::iterator it2 = repeater_config_info.prev_module_name_list.begin();
         it2 != repeater_config_info.prev_module_name_list.end(); ++it2) {
      YSOS_LOG_DEBUG((*it2) << ";");
    }
    YSOS_LOG_DEBUG("has edge list: ");
    for (EdgeOfModuleNameListIterator it2 = repeater_config_info.edge_list.begin();
         it2 != repeater_config_info.edge_list.end(); ++it2) {
      YSOS_LOG_DEBUG("(" << (*it2).first << ", " << (*it2).second << ");");
    }
  }
#endif

  return module_link_conf_info_ptr;
}

/// 某FamousRepeater是否存在包含于InherentRepeaterMap中的InherentRepeater的边?
bool ExistEdgeOfFamousRepeaterInInherentRepeaterMap(
  EdgeOfModuleNameList &edge_list_of_famous_repeater,
  InherentRepeaterConfInfoMap &repeater_info_list,
  InherentRepeaterConfInfoMapIterator &the_inherent_repeater_it) {
  bool return_value = false;

  /// 对于该FamousRepeater的每一条边
  for (EdgeOfModuleNameListIterator it2 = edge_list_of_famous_repeater.begin();
       it2 != edge_list_of_famous_repeater.end(); ++it2) {

    for (InherentRepeaterConfInfoMapIterator it = repeater_info_list.begin();
         it != repeater_info_list.end(); ++it) {
      for (EdgeOfModuleNameListIterator it3 = (*it).second.edge_list.begin();
           it3 != (*it).second.edge_list.end(); ++it3) {
        if ((*it2).first == (*it3).first && (*it2).second == (*it3).second) {
          /// 已找到, 无需继续
          the_inherent_repeater_it = it;
          return_value = true;
          break;
        }
      }

      /// 已找到, 无需继续
      if (return_value) {
        break;
      }
    }

    /// 已找到, 无需继续
    if (return_value) {
      break;
    }
  }

  return return_value;
}

/// 将某FamousRepeater的全部边插入InherentRepeater中
bool InsertEdgeListOfFamousRepeaterToInherentRepeater(
  EdgeOfModuleNameList &edge_list_of_famous_repeater,
  InherentRepeaterConfInfo &the_inherent_repeater) {
  bool return_value = false;

  /// 对于该FamousRepeater的每一条边
  for (EdgeOfModuleNameListIterator it2 = edge_list_of_famous_repeater.begin();
       it2 != edge_list_of_famous_repeater.end(); ++it2) {
    bool is_the_edge_in_inherent_repeater = false;
    for (EdgeOfModuleNameListIterator it3 = the_inherent_repeater.edge_list.begin();
         it3 != the_inherent_repeater.edge_list.end(); ++it3) {
      if ((*it2).first == (*it3).first && (*it2).second == (*it3).second) {
        /// 无需插入
        is_the_edge_in_inherent_repeater = true;
        return_value = true;
        break;
      }
    }

    /// 不存在, 需要插入
    if (!is_the_edge_in_inherent_repeater) {
      the_inherent_repeater.edge_list.push_back((*it2));
    }
  }

  return return_value;
}

uint16_t GetLevelByModuleName(std::string &module_name, ModuleLinkConfInfoPtr &module_link_conf_info_ptr) {
  uint16_t level = 0;
  ModuleInfoListInLink& module_info = module_link_conf_info_ptr->module_list;
  for (ModuleInfoListInLinkIterator it = module_info.begin(); it != module_info.end(); ++it) {
    if ((*it)->instance_name == module_name) {
      level = (*it)->instance_level;
      break;
    }
  }

  return level;
}

int PackageConfigImpl::GetCallbackNamList(const std::string& module_name, std::list<std::string> &callback_name_list) {
  int return_value = YSOS_ERROR_SUCCESS;
  do {
    if (true == module_name.empty()) {
      return_value = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }
    std::string callback_owner;
    //  Seek Callback
    CallbackConfInfoIterator callback_info_iterator = CallbackConfInfoHead();
    for (; callback_info_iterator != CallbackConfInfoEnd(); ++callback_info_iterator) {
      /// 合法性检查
      if ((*callback_info_iterator)->owner.empty()) {
        continue;
      }
      callback_owner = (*callback_info_iterator)->owner;
      if (module_name != callback_owner) {
        continue;
      }

      /// 找到匹配的Callback
      std::string callback_instance_name = (*callback_info_iterator)->name;
      callback_name_list.push_back(callback_instance_name);
    }
  } while (false);
  return return_value;
}

/// 由知名Repeater信息更新内置Repeater信息
/// 对每一个知名Repeater,
///   查看其边表中是否有已存在于内置Repeater中的,
///   若其边表中有已存在于有内置Repeater i中的, 则
///     将其每一条边都加入, 并将i的名称改为知名Repeater名.
///   若其边表中没有任何一条边存在于任何内置Repeater中, 则
///     将该知名Repeater加入一个新建的内置Repeater中.
void PackageConfigImpl::UpdateInherentRepeaterListByFamousRepeaterInfo(
  ModuleLinkConfInfoPtr module_link_conf_info_ptr) {
  FamousRepeater_EdgeList_Map &map = module_link_conf_info_ptr->famous_repeater_edge_list_map;
  InherentRepeaterConfInfoMap &repeater_info_list = module_link_conf_info_ptr->repeater_info_list;
  uint32_t link_depth = module_link_conf_info_ptr->GetDepth();

  /// 对每一个知名Repeater,
  for (FamousRepeater_EdgeList_MapIterator it = map.begin(); it != map.end(); ++it) {
    InherentRepeaterConfInfoMapIterator the_inherent_repeater_it = repeater_info_list.end();
    EdgeOfModuleNameList &edge_list_of_famous_repeater = (*it).second;
    std::string callback_name = "";
    for (CallbackConfInfoIterator itcb = callback_conf_info_list_.begin();
         itcb != callback_conf_info_list_.end(); ++itcb) {
      if ((*itcb)->owner == (*it).first) {
        callback_name = /*"ysos::" + */(*itcb)->name;
        break;
      }
    }
    ///   查看其边表中是否有已存在于内置Repeater中的,
    bool exist_edge_in_inherent_repeater = ExistEdgeOfFamousRepeaterInInherentRepeaterMap(
        edge_list_of_famous_repeater, repeater_info_list, the_inherent_repeater_it);

    ///   若其边表中有已存在于某内置Repeater i中的, 则
    if (exist_edge_in_inherent_repeater) {
      ///   将其每一条边都加入,
      InsertEdgeListOfFamousRepeaterToInherentRepeater(
        edge_list_of_famous_repeater, (*the_inherent_repeater_it).second);
      ///   并将i的名称改为知名Repeater名.
      (*the_inherent_repeater_it).second.famous_name = (*it).first;
      // (*the_inherent_repeater_it).second.callback_name = callback_name;
    } else {
      ///   若其边表中没有任何一条边存在于任何内置Repeater中, 则
      int16_t curr_level = GetLevelByModuleName(edge_list_of_famous_repeater.front().first,
                           module_link_conf_info_ptr);
      InherentRepeaterConfInfo repeater_config_info;
      repeater_config_info.level = curr_level << 16;
      if (link_depth != curr_level) {
        repeater_config_info.level += curr_level + 1;
      }
      InherentRepeaterConfInfoMapIterator iter  = repeater_info_list.find(curr_level);
      if (iter != repeater_info_list.end()) {
        ///     若相同level的已存在, 则名称改为知名Repeater名.
        (*iter).second.famous_name = (*it).first;

        ///   将其每一条边都加入,
        InsertEdgeListOfFamousRepeaterToInherentRepeater(
          edge_list_of_famous_repeater, (*iter).second);

        /// Update famous callback name
        if (YSOS_ERROR_SUCCESS != GetCallbackNamList(
              repeater_config_info.famous_name, repeater_config_info.callback_name_list)) {
        }
        if (repeater_config_info.callback_name_list.size() == 0) {
          repeater_config_info.callback_name_list.push_back(BASE_REPEATER_CALLBACK_IMPL_CLASS_NAME);
        }

      } else {
        ///     否则新建一个内置Repeater
        repeater_config_info.name = MakeRepeaterName(module_link_conf_info_ptr->name, link_depth, curr_level, true, false);

        ///   将该知名Repeater加入内置Repeater中.
        repeater_config_info.famous_name = (*it).first;
        InsertEdgeListOfFamousRepeaterToInherentRepeater(
          edge_list_of_famous_repeater, repeater_config_info);

        /// Update famous callback name
        if (YSOS_ERROR_SUCCESS != GetCallbackNamList(
              repeater_config_info.famous_name, repeater_config_info.callback_name_list)) {
        }
        if (repeater_config_info.callback_name_list.size() == 0) {
          repeater_config_info.callback_name_list.push_back(BASE_REPEATER_CALLBACK_IMPL_CLASS_NAME);
        }
        repeater_info_list.insert(std::make_pair(curr_level, repeater_config_info));
      }
    }
  }
}

void PackageConfigImpl::GenerateInherentRepeaterList(ModuleLinkConfInfoPtr& link_config_info) {
  do {
    if (link_config_info->module_list.size() == 0) {
      break;
    }
    ModuleInfoListInLink& module_info = link_config_info->module_list;
    InherentRepeaterConfInfoMap& repeater_info_list = link_config_info->repeater_info_list;
    uint32_t link_depth = link_config_info->GetDepth();

    //  对列表中的每一个元素A://NOLINT
    for (ModuleInfoListInLinkIterator it = module_info.begin();
         it != module_info.end(); ++it) {
      //  若有下游模块在内置的Repeater中则//NOLINT
      if ((*it)->next_module_concerned_with_inherent_repeater.size() > 0) {
        //  取出其level//NOLINT
        uint32_t curr_level = (*it)->instance_level;
        bool concerned_repeater_existing = false;
        InherentRepeaterConfInfoMapIterator iter
          = repeater_info_list.end();
        {
          //  对应的Repeater信息是否已存在?
          iter  = repeater_info_list.find(curr_level);
          if (iter != repeater_info_list.end()) {
            concerned_repeater_existing = true;
          } else {
            //  创建一个Repeater信息
            InherentRepeaterConfInfo repeater_config_info;
            repeater_config_info.level = curr_level << 16;
            if (link_depth != curr_level) {
              repeater_config_info.level += curr_level + 1;
            }
            repeater_config_info.name = MakeRepeaterName(
                                          link_config_info->name,
                                          link_depth,
                                          curr_level,
                                          true,
                                          false);
            repeater_info_list.insert(std::make_pair(curr_level, repeater_config_info));
            iter  = repeater_info_list.find(curr_level);
            if (iter != repeater_info_list.end()) {
              concerned_repeater_existing = true;
            }
          }
        }
        //  异常情况//NOLINT
        if (!concerned_repeater_existing) {
          break;
        }
        InherentRepeaterConfInfo& repeater_config_info = iter->second;

        //  对其中的每一个元素B//NOLINT
        for (std::list<std::string>::iterator it2
             = (*it)->next_module_concerned_with_inherent_repeater.begin();
             it2 != (*it)->next_module_concerned_with_inherent_repeater.end();
             ++it2) {
          push_back_if_not_in_list<std::string>(
            repeater_config_info.prev_module_name_list, (*it)->instance_name);
          //  repeater_config_info.prev_module_name_list.push_back(
          //    (*it)->instance_name);

          auto tmpETM = std::make_pair((*it)->instance_name, (*it2));  //need add for linux
          push_back_if_not_in_list<EdgeOfModuleName>(
            repeater_config_info.edge_list,
            /*std::make_pair((*it)->instance_name, (*it2))*/tmpETM);   //need update for linux
          //  repeater_config_info.edge_list.push_back(
          //    std::make_pair((*it)->instance_name, (*it2)));
        }
      }
    }

#if (CURRENT_PLATFORM_PHASE == PLATFORM_PHASE_DEBUG)
    //  Dump prev_modules
    for (InherentRepeaterConfInfoMapIterator it
         = repeater_info_list.begin();
         it != repeater_info_list.end(); ++it) {

      InherentRepeaterConfInfo& repeater_config_info = it->second;
      YSOS_LOG_DEBUG("The repeater after level " << (*it).first
                     << " with name " << repeater_config_info.name
                     << " has prev module list: ");
      for (std::list<std::string>::iterator it2
           = repeater_config_info.prev_module_name_list.begin();
           it2 != repeater_config_info.prev_module_name_list.end(); ++it2) {
        YSOS_LOG_DEBUG((*it2) << ";");
      }
      YSOS_LOG_DEBUG("has edge list: ");
      for (EdgeOfModuleNameListIterator it2
           = repeater_config_info.edge_list.begin();
           it2 != repeater_config_info.edge_list.end(); ++it2) {
        YSOS_LOG_DEBUG("(" << (*it2).first << ", " << (*it2).second << ");");
      }
    }
#endif
  } while (false);
}

//  调用前提: instance_id已解析//NOLINT
void PackageConfigImpl::UpdateModuleLinkModuleConfInfoImpl(
  ModuleInfoListInLink& module_info) {
  if (module_info.size() == 0) {
    return;
  }
  //  设置prev_modules实例逻辑名//NOLINT
  UpdateModuleLinkModuleConfInfoImpl_FillPrevModuleList(module_info);
  //  更新边表//NOLINT
  UpdateModuleLinkModuleConfInfoImpl_UpdateNextModuleList(module_info);
}

/// 设置prev_modules实例逻辑名//NOLINT
void PackageConfigImpl::UpdateModuleLinkModuleConfInfoImpl_FillPrevModuleList(
  ModuleInfoListInLink& module_info) {
  if (module_info.size() == 0) {
    return;
  }

  //  对列表中的每一个元素A://NOLINT
  for (ModuleInfoListInLinkIterator it = module_info.begin();
       it != module_info.end(); ++it) {
    //  若有下游模块则//NOLINT
    if ((*it)->next_modules.size() > 0) {
      //  对其中的每一个元素B//NOLINT
      for (std::list<std::string>::iterator it2 = (*it)->next_modules.begin();
           it2 != (*it)->next_modules.end(); ++it2) {
        //  取出其ID号//NOLINT
        int64_t curr_next_id = std::stoi((*it2));
        //  试图找到对应元素C的实例名, 若找到则将A的实例名推入C的上游模块列表//NOLINT
        LightLock lock__;
        AutoLockOper lock(&lock__);
        for (ModuleInfoListInLinkIterator it3 = module_info.begin();
             it3 != module_info.end(); ++it3) {
          if (curr_next_id == (*it3)->instance_id) {
            (*it3)->prev_modules.push_back((*it)->instance_name);
          }
        }
      }
    }
  }

#if (CURRENT_PLATFORM_PHASE == PLATFORM_PHASE_DEBUG)
  // Dump prev_modules
  for (ModuleInfoListInLinkIterator it = module_info.begin();
       it != module_info.end(); ++it) {
    YSOS_LOG_DEBUG("The Module " << (*it)->instance_name << "\' prev module list:");
    if ((*it)->prev_modules.size() > 0) {
      // 对其中的每一个元素B//NOLINT
      for (std::list<std::string>::iterator it2 = (*it)->prev_modules.begin();
           it2 != (*it)->prev_modules.end(); ++it2) {
        YSOS_LOG_DEBUG(" " << (*it2));
      }
    } else {
      YSOS_LOG_DEBUG(" (null).");
    }
  }
#endif
}

/// 获取知名Repeater的信息.
/// 在NextModuleList生成后, 即可生成知名Repeater和边的映射.
///   遍历Module节点
///   若repeater_after_it不空,
///   则
///     若NextModuleList空
///     则
///       当前Module加入知名Repeater.
///     否则
///       查看NextModule中,将repeater_before_it与当前Module的repeater_after_it相同的,
///       形成边加入知名Repeater.
void PackageConfigImpl::GetFamousRepeaterInfo(ModuleLinkConfInfoPtr& module_link_conf_info_ptr) {
  bool broken = false;
  if (NULL == module_link_conf_info_ptr) {
    return;
  }
  ModuleInfoListInLink& module_info = module_link_conf_info_ptr->module_list;
  if (module_info.size() == 0) {
    return;
  }

  uint16_t prev_level = 1;
  uint16_t curr_level = 1;
  std::string famous_repeater_name;
  EdgeOfModuleNameList edge_list_of_repeater;

  ///   遍历Module节点
  ///   对列表中的每一个元素A:
  for (ModuleInfoListInLinkIterator it = module_info.begin(); it != module_info.end(); ++it) {
    curr_level = (*it)->instance_level;
    ///   一个level的FamousRepeater已经扫描完毕
    if (prev_level != curr_level) {
      ///   如果存在FamousRepeater, 那么加入Map
      if (famous_repeater_name.length() > 0 && edge_list_of_repeater.size() > 0) {
        FamousRepeater_EdgeList_Pair pair = std::make_pair(famous_repeater_name, edge_list_of_repeater);
        module_link_conf_info_ptr->famous_repeater_edge_list_map.insert(pair);
      }
      ///   更新临时变量
      prev_level += 1;
      famous_repeater_name = "";
      edge_list_of_repeater.clear();
    }

    if ((*it)->repeater_after_it.length() == 0) {
      continue;
    }

    ///   若repeater_after_it不空,
    ///   则
    if (famous_repeater_name.length() == 0) {
      famous_repeater_name = (*it)->repeater_after_it;
    } else if (famous_repeater_name.length() > 0 && famous_repeater_name != (*it)->repeater_after_it) {
      YSOS_LOG_DEBUG("Unexpected famous_repeater_name " << (*it)->repeater_after_it);
      broken = true;
      break;
    }

    std::string prev_module_name = (*it)->instance_name;
    if ((*it)->next_modules.size() == 0) {      ///   若NextModuleList空则
      std::string next_module_name = "";
      ///   当前Module加入知名Repeater.
      EdgeOfModuleName edge = std::make_pair(prev_module_name, next_module_name);
      edge_list_of_repeater.push_back(edge);
    } else {                                    ///   否则
      ///   对其中的每一个元素B
      ///   查看NextModule中,
      bool exist_matched_module = false;
      for (std::list<std::string>::iterator it2 = (*it)->next_modules.begin();
           it2 != (*it)->next_modules.end(); ++it2) {
        int64_t next_module_id = std::stoll((*it2));
        LightLock lock__;
        AutoLockOper lock(&lock__);
        ///   将repeater_before_it与当前Module的repeater_after_it相同的,
        for (ModuleInfoListInLinkIterator it3 = module_info.begin();
             it3 != module_info.end(); ++it3) {
          if (next_module_id == (*it3)->instance_id && famous_repeater_name == (*it3)->repeater_before_it) {
            ///   形成边加入知名Repeater.
            std::string next_module_name = (*it3)->instance_name;
            EdgeOfModuleName edge = std::make_pair(prev_module_name, next_module_name);
            edge_list_of_repeater.push_back(edge);
            exist_matched_module = true;
            break;
          }
        }
      }
      if (!exist_matched_module) {
        YSOS_LOG_DEBUG("No matched next module for famous_repeater_name " << (*it)->repeater_after_it);
        broken = true;
        break;
      }
    }
  }

  // 整个link的FamousRepeater已经扫描完毕
  {
    //  如果存在FamousRepeater, 那么加入Map
    if (famous_repeater_name.length() > 0 && edge_list_of_repeater.size() > 0) {
      FamousRepeater_EdgeList_Pair pair = std::make_pair(famous_repeater_name, edge_list_of_repeater);
      module_link_conf_info_ptr->famous_repeater_edge_list_map.insert(pair);
    }
  }

#if (CURRENT_PLATFORM_PHASE == PLATFORM_PHASE_DEBUG)
  FamousRepeater_EdgeList_Map &map = module_link_conf_info_ptr->famous_repeater_edge_list_map;
  for (FamousRepeater_EdgeList_MapIterator it = map.begin(); it != map.end(); ++it) {
    YSOS_LOG_DEBUG("The original edge list of repeater " << (*it).first << ":");
    if ((*it).second.size() > 0) {
      //  对其中的每一个元素B//NOLINT
      for (EdgeOfModuleNameListIterator it2 = (*it).second.begin(); it2 != (*it).second.end(); ++it2) {
        YSOS_LOG_DEBUG("(" << (*it2).first << "," << (*it2).second << ")");
      }
    } else {
      YSOS_LOG_DEBUG("(null).");
    }
  }
#endif
}

/// 更新边表:生成将被包含在内置Repeater里的边表,并将这些边从原有边表中移除//NOLINT
void PackageConfigImpl::UpdateModuleLinkModuleConfInfoImpl_UpdateNextModuleList(
  ModuleInfoListInLink& module_info) {
  if (module_info.size() == 0) {
    return;
  }

  //  对列表中的每一个元素A://NOLINT
  for (ModuleInfoListInLinkIterator it = module_info.begin();
       it != module_info.end(); ++it) {
    //  若有>1个上游模块则//NOLINT
    if ((*it)->prev_modules.size() > 1) {
      //  对其中的每一个元素B//NOLINT
      for (std::list<std::string>::iterator it2 = (*it)->prev_modules.begin();
           it2 != (*it)->prev_modules.end(); ++it2) {
        //  取出其实例名//NOLINT
        //  试图找到对应元素C, 若找到则将A从C的下游模块列表中移除//NOLINT
        //  并添加A的实例逻辑名到C下游的某个内置Repeater的边表//NOLINT
        LightLock lock__;
        AutoLockOper lock(&lock__);
        for (ModuleInfoListInLinkIterator it3 = module_info.begin();
             it3 != module_info.end(); ++it3) {
          if ((*it2) == (*it3)->instance_name) {
            std::string id_string = std::to_string((*it)->instance_id);
            (*it3)->next_modules.remove(id_string);
            (*it3)->next_module_concerned_with_inherent_repeater.push_back(
              (*it)->instance_name);
          }
        }
      }
    }
  }

#if (CURRENT_PLATFORM_PHASE == PLATFORM_PHASE_DEBUG)
  //  Dump prev_modules
  for (ModuleInfoListInLinkIterator it = module_info.begin();
       it != module_info.end(); ++it) {
    YSOS_LOG_DEBUG("The Module " << (*it)->instance_name << "\' next module list: ");
    if ((*it)->next_modules.size() > 0) {
      //  对其中的每一个元素B//NOLINT
      for (std::list<std::string>::iterator it2 = (*it)->next_modules.begin();
           it2 != (*it)->next_modules.end(); ++it2) {
        YSOS_LOG_DEBUG((*it2) << ";");
      }
    } else {
      YSOS_LOG_DEBUG("(null);");
    }
    YSOS_LOG_DEBUG("Next module concerned with repeater list:");
    if ((*it)->next_module_concerned_with_inherent_repeater.size() > 0) {
      //  对其中的每一个元素B//NOLINT
      for (std::list<std::string>::iterator it2
           = (*it)->next_module_concerned_with_inherent_repeater.begin();
           it2 != (*it)->next_module_concerned_with_inherent_repeater.end();
           ++it2) {
        YSOS_LOG_DEBUG((*it2) << ";");
      }
    } else {
      YSOS_LOG_DEBUG("(null);");
    }
  }
#endif
}

ModuleLinkModuleConfInfoPtr
PackageConfigImpl::ParseModuleLinkModuleConfInfoImpl(
  tinyxml2::XMLElement *element) {
  if (NULL == element) {
    return ModuleLinkModuleConfInfoPtr();
  }

  ModuleLinkModuleConfInfoPtr module_link_module_conf_info_ptr
    = boost::make_shared<ModuleLinkModuleConfInfo>();
  module_link_module_conf_info_ptr->module_name = GetXmlUtil()->
      GetElementTextValue(element, "module_name");
  module_link_module_conf_info_ptr->instance_name = GetXmlUtil()->
      GetElementTextValue(element, "instance_name");

  module_link_module_conf_info_ptr->repeater_before_it = GetXmlUtil()->
      GetElementTextValue(element, "repeater_before_it");
  module_link_module_conf_info_ptr->repeater_after_it = GetXmlUtil()->
      GetElementTextValue(element, "repeater_after_it");

  tinyxml2::XMLElement *next_module = element->FirstChildElement("next_module");
  while (NULL != next_module) {
    const char *text = next_module->GetText();
    if (NULL != text) {
      YSOS_LOG_DEBUG("next module: " << text);
      module_link_module_conf_info_ptr->next_modules.push_back(text);
    }

    next_module = next_module->NextSiblingElement("next_module");
  }

  int id = 0;
  if (tinyxml2::XML_NO_ERROR == element->QueryIntAttribute("id", &id)) {
    module_link_module_conf_info_ptr->instance_id = id;
  }

  int level = 0;
  if (tinyxml2::XML_NO_ERROR == element->QueryIntAttribute("level", &level)) {
    module_link_module_conf_info_ptr->instance_level = level;
  }

  return module_link_module_conf_info_ptr;
}

void PackageConfigImpl::PrintfModuleConfInfo(
  const ModuleConfInfoPtr &module_conf_info_ptr,
  bool is_module_link) {
  std::string pad = "    ";
  if (is_module_link) {
    pad += pad;
    YSOS_LOG_DEBUG(pad << " module instance_id: "
                   << module_conf_info_ptr->instance_id);
    YSOS_LOG_DEBUG(pad <<" module instance_name: "
                   << module_conf_info_ptr->instance_name);
    return;
  }

  YSOS_LOG_DEBUG(pad <<" module name: " << module_conf_info_ptr->name);
  YSOS_LOG_DEBUG(pad <<" module version: " << module_conf_info_ptr->version);
  YSOS_LOG_DEBUG(pad <<" module local: " << module_conf_info_ptr->is_local);
  YSOS_LOG_DEBUG(pad <<" module uri: " << module_conf_info_ptr->uri);
  YSOS_LOG_DEBUG(pad <<" module type: " << module_conf_info_ptr->type);
  YSOS_LOG_DEBUG(pad <<" module input_type: " << module_conf_info_ptr->input_type);
  YSOS_LOG_DEBUG(pad <<" module output_type: " << module_conf_info_ptr->output_type);
  YSOS_LOG_DEBUG(pad <<" module allocated: " << module_conf_info_ptr->is_allocated);
  YSOS_LOG_DEBUG(pad <<" module prefix: " << module_conf_info_ptr->prefix);
  YSOS_LOG_DEBUG(pad <<" module instance_id: " << module_conf_info_ptr->buffer_length);
  YSOS_LOG_DEBUG(pad <<" module is_revised: " << module_conf_info_ptr->is_revised);
  YSOS_LOG_DEBUG(pad <<" module buffer_number: " << module_conf_info_ptr->buffer_number);
  YSOS_LOG_DEBUG(pad <<" module capacity: " << module_conf_info_ptr->capacity);
}

void  PackageConfigImpl::PrintfModuleLinkModuleConfInfo(
  const ModuleLinkModuleConfInfoPtr &module_link_module_conf_info_ptr) {
  YSOS_LOG_DEBUG(" instance id: " << module_link_module_conf_info_ptr->instance_id);
  YSOS_LOG_DEBUG(" instance level: " << module_link_module_conf_info_ptr->instance_level);
  YSOS_LOG_DEBUG(" module name: " << module_link_module_conf_info_ptr->module_name);
  YSOS_LOG_DEBUG(" instance name: " << module_link_module_conf_info_ptr->instance_name);

  if (0 != module_link_module_conf_info_ptr->next_modules.size()) {
    for (std::list<std::string>::iterator iterator
         = module_link_module_conf_info_ptr->next_modules.begin();
         iterator != module_link_module_conf_info_ptr->next_modules.end();
         ++iterator) {
      YSOS_LOG_DEBUG(" next module: " << *iterator);
    }
  }
}

void  PackageConfigImpl::PrintfModuleLinkConfInfo(const ModuleLinkConfInfoPtr &module_link_conf_info_ptr) {
  YSOS_LOG_DEBUG(" module link id: " << module_link_conf_info_ptr->id);
  YSOS_LOG_DEBUG(" module link name: " << module_link_conf_info_ptr->name);

  for (ModuleLinkModuleConfInfoIterator it
       = module_link_conf_info_ptr->module_list.begin();
       it != module_link_conf_info_ptr->module_list.end(); ++it) {
    YSOS_LOG_DEBUG("module info: *********************");
    PrintfModuleLinkModuleConfInfo(*it);
  }
}

void PackageConfigImpl::PrintfCallbackConfInfo(const CallbackConfInfoPtr &callback_conf_info_ptr) {
  std::string pad = "    ";
  YSOS_LOG_DEBUG(pad <<" callback name: " << callback_conf_info_ptr->name);
  YSOS_LOG_DEBUG(pad <<" callback owner: " << callback_conf_info_ptr->owner);
  YSOS_LOG_DEBUG(pad <<" callback version: " << callback_conf_info_ptr->version);
}

void PackageConfigImpl::PrintServiceConfInfo(
  const ServiceInfoTable::ServiceConfInfoPtr &service_conf_info_ptr) {
  YSOS_LOG_DEBUG(" service ID: " << service_conf_info_ptr->ID);
  YSOS_LOG_DEBUG(" service name: " << service_conf_info_ptr->name);
  YSOS_LOG_DEBUG(" service type: " << service_conf_info_ptr->type);
  YSOS_LOG_DEBUG(" service callback: " << service_conf_info_ptr->callback);
  YSOS_LOG_DEBUG(" service instance_name: " << service_conf_info_ptr->instance_name);
  YSOS_LOG_DEBUG(" service is_default: " << service_conf_info_ptr->is_default);
  YSOS_LOG_DEBUG(" service description: " << service_conf_info_ptr->description);

  for (ServiceInfoTable::ServiceConfInfoIterator iterator
       = service_conf_info_ptr->sub_services.begin();
       iterator != service_conf_info_ptr->sub_services.end();
       ++iterator) {
    YSOS_LOG_DEBUG("sub service info: *********************");
    PrintServiceConfInfo((*iterator));
  }
}

void  PackageConfigImpl::DumpCallbackConfInfoList(void) {
  for (CallbackConfInfoIterator it = callback_conf_info_list_.begin();
       it != callback_conf_info_list_.end(); ++it) {
    YSOS_LOG_DEBUG("callback info: *********************");
    PrintfCallbackConfInfo(*it);
  }
}

void PackageConfigImpl::DumpModuleConfInfoList(void) {
  for (ModuleConfInfoIterator it=module_conf_info_list_.begin();
       it != module_conf_info_list_.end(); ++it) {
    YSOS_LOG_DEBUG("module info: *********************");
    PrintfModuleConfInfo(*it);
  }
}

void PackageConfigImpl::DumpModuleLinkConfInfoList(void) {
  for (ModuleLinkConfInfoIterator it=module_link_conf_info_list_.begin();
       it != module_link_conf_info_list_.end(); ++it) {
    YSOS_LOG_DEBUG("module link info: *********************");
    PrintfModuleLinkConfInfo(*it);
  }
}

void PackageConfigImpl::DumpServiceConfInfoList(void) {
  ServiceInfoTable::ServiceConfInfoList service_conf_info_list
    = ServiceInfoTable::Instance()->GetServiceConfInfoList();
  for (ServiceInfoTable::ServiceConfInfoIterator iterator
       = service_conf_info_list.begin();
       iterator != service_conf_info_list.end();
       ++iterator) {
    YSOS_LOG_DEBUG("service info info: *********************");
    PrintServiceConfInfo(*iterator);
  }
}

int PackageConfigImpl::CreateModuleLinkObject(ModuleLinkInterface* module_link_interface_ptr,
    std::string &module_link_name) {
  int return_value = YSOS_ERROR_FAILED;
  bool will_break = false;
  do {
    YSOS_LOG_DEBUG("Try to CreateModuleLinkObject " << module_link_name);

    if (NULL == module_link_interface_ptr) {
      YSOS_LOG_ERROR("module link not exist: " << module_link_name);
      break;
    }

    //  查找对应的ModuleLinkConfInfo
    ModuleLinkConfInfoIterator module_link_info_iterator = ModuleLinkConfInfoHead();
    if (0 == (*module_link_info_iterator)->module_list.size()) {
      YSOS_LOG_ERROR("module link conf size is 0: " << module_link_name);
      break;
    }

    for (/**/; module_link_info_iterator != ModuleLinkConfInfoEnd(); ++module_link_info_iterator) {
      if (module_link_name == (*module_link_info_iterator)->name) {
        YSOS_LOG_DEBUG("get module link conf info success");
        break;
      }
    }

    //  若没有找到
    if (module_link_info_iterator == ModuleLinkConfInfoEnd()) {
      YSOS_LOG_ERROR("get module link conf info failed");
      break;
    }

    ysos::ModuleInterfacePtr module_interface_ptr = NULL;
    std::string module_instance_name;

    //  Seek Module Info
    for (ModuleLinkModuleConfInfoIterator module_info_iterator = (*module_link_info_iterator)->module_list.begin();
         module_info_iterator != (*module_link_info_iterator)->module_list.end(); ++module_info_iterator) {

      //  Get Module Instance Name
      module_instance_name = (*module_info_iterator)->instance_name;
      if (module_instance_name.empty()) {
        YSOS_LOG_ERROR("Exist empty name module info in " << module_link_name << ".");
        will_break = true;
        break;
      }

      YSOS_LOG_DEBUG("module instance name: " << module_instance_name << " | " << module_link_name);

      module_interface_ptr = ysos::GetModuleInterfaceManager()->FindInterface(module_instance_name);

      if (NULL == module_interface_ptr) {
        YSOS_LOG_ERROR("found module " << module_instance_name << " failed!");
        will_break = true;
        break;
      }

      //  Add Module
      uint32_t level_in_link = (*module_info_iterator)->instance_level * 0x10001;
      module_interface_ptr->SetProperty(PROP_MODULE_LINK_LEVEL, &level_in_link);
      module_link_interface_ptr->AddModule(module_interface_ptr);
      YSOS_LOG_DEBUG("add module " << module_instance_name << " to module link: " << module_link_name << " level: " << (*module_info_iterator)->instance_level);
    }

    if (will_break) {
      break;
    }

    ysos::ModuleInterfacePtr current_module_interface_ptr = NULL;
    ysos::ModuleInterfacePtr next_module_interface_ptr = NULL;
    std::string current_module_instance_name;
    std::string next_module_instance_name;

    //  Seek Module Info
    for (PackageConfigImpl::ModuleLinkModuleConfInfoIterator module_info_iterator = (*module_link_info_iterator)->module_list.begin();
         module_info_iterator != (*module_link_info_iterator)->module_list.end(); ++module_info_iterator) {
      //  Get Module Instance Name
      if ((*module_info_iterator)->instance_name .empty()) {
        YSOS_LOG_ERROR("exist empty instance name: " << module_link_name);
        will_break = true;
        break;
      }

      current_module_instance_name = (*module_info_iterator)->instance_name;
      YSOS_LOG_DEBUG("current module instance name: " << current_module_instance_name);
      current_module_interface_ptr
        = ysos::GetModuleInterfaceManager()->FindInterface(current_module_instance_name);
      if (NULL == current_module_interface_ptr) {
        YSOS_LOG_ERROR("Could not found module " << current_module_instance_name << ".");
        will_break = true;
        break;
      }

      //  Source or Destination directly add
      if (0 == (*module_info_iterator)->next_modules.size()) {
        continue;
      }

      for (PackageConfigImpl::ModuleLinkModuleConfInfoNextModulesIterator
           iterator = (*module_info_iterator)->next_modules.begin();
           iterator != (*module_info_iterator)->next_modules.end(); ++iterator) {
        next_module_interface_ptr = FindNextModule(*iterator, module_link_info_iterator);

        //  Add Edge
        if (NULL != next_module_interface_ptr) {
          YSOS_LOG_DEBUG("found " << current_module_instance_name << "'s next module " << next_module_interface_ptr->GetName() << " sucess: " << module_link_name);
          module_link_interface_ptr->AddModule(next_module_interface_ptr, current_module_interface_ptr);
          YSOS_LOG_DEBUG("add " << current_module_instance_name << "'s next module " << next_module_interface_ptr->GetName() << " : " << module_link_name);
        } else {
          YSOS_LOG_ERROR("found " << current_module_instance_name << "'s next module failed: " << module_link_name);
        }
      }

      if (will_break) {
        break;
      }
    }  // end for

    // 创建Repeater并链入.
    if ((*module_link_info_iterator)->repeater_info_list.size() != 0) {
      YSOS_LOG_DEBUG("Create repeater(s) in link.: " << module_link_name);
      module_link_interface_ptr->Ioctl(CMD_MODULE_LINK_ADD_INHERENT_REPEATER, &(*module_link_info_iterator)->repeater_info_list);
    }

    return_value = YSOS_ERROR_SUCCESS;
  } while (false);

  return return_value;
}

boost::shared_ptr<ModuleInterface> PackageConfigImpl::FindNextModule(
  std::string& next_module_id, ModuleLinkConfInfoIterator& module_link_info_iterator) {
  if (true == next_module_id.empty()) {
    return NULL;
  }

  ysos::ModuleInterfacePtr found_module_interface_ptr = NULL;
  std::string found_module_instance_name;

  {
    {

      if (0 != (*module_link_info_iterator)->module_list.size()) {
        //  Seek Module Info
        for (PackageConfigImpl::ModuleLinkModuleConfInfoIterator
             module_info_iterator
             = (*module_link_info_iterator)->module_list.begin();
             module_info_iterator
             != (*module_link_info_iterator)->module_list.end();
             ++module_info_iterator) {
          //  Get Module Instance Name
          if (true == (*module_info_iterator)->instance_name .empty()) {
            continue;
          }
          if (std::stoi(next_module_id)
              != (*module_info_iterator)->instance_id) {
            continue;
          }

          found_module_instance_name = (*module_info_iterator)->instance_name;
          if (true == found_module_instance_name.empty()) {
            continue;
          }
          found_module_instance_name = found_module_instance_name;
          YSOS_LOG_DEBUG("found module instance name: " << found_module_instance_name);
          found_module_interface_ptr
            = ysos::GetModuleInterfaceManager()->FindInterface(found_module_instance_name);
          return found_module_interface_ptr;
        }  // end for
      }
    }
  }  // end for

  return NULL;
}

void PackageConfigImpl::Dump(void) {
  DumpCallbackConfInfoList();
  DumpModuleConfInfoList();
  DumpModuleLinkConfInfoList();
  DumpServiceConfInfoList();
}
}   //  end of namespace//NOLINT
