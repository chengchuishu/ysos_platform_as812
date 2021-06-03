/*@version 1.0
  *@author l.j..
  *@date Created on: 2016-10-20 13:48:23
  *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
  * 
*/

/// Private Headers //  NOLINT
#include "../include/doconfigdriver.h"


namespace ysos {
DECLARE_PLUGIN_REGISTER_INTERFACE(DoconfigDriver, DriverInterface);  ///<  插件的入口，一定要加上 //  NOLINT

DoconfigDriver::DoconfigDriver(const std::string &strClassName) : BaseInterfaceImpl(strClassName) {

  my_document_ = NULL;
  xml_path_ = "";
  search_module_ = "info";
  logger_ = GetUtility()->GetLogger("ysos.doconfig");
}

DoconfigDriver::~DoconfigDriver(void) {
}

int DoconfigDriver::SetProperty(int type_id, void *type) {
  YSOS_LOG_DEBUG("driver SetProperty execute");
  int n_return = YSOS_ERROR_SUCCESS;

  YSOS_LOG_DEBUG("driver SetProperty done");

  return n_return;
}

int DoconfigDriver::Read(BufferInterfacePtr pBuffer, DriverContexPtr context_ptr/*=NULL*/) {
  YSOS_LOG_DEBUG("driver Read execute");
  int n_return = YSOS_ERROR_SUCCESS;
  do {
    UINT8* buffer;
    UINT32 buffer_size;
    n_return = pBuffer->GetBufferAndLength(&buffer, &buffer_size);
    if (YSOS_ERROR_SUCCESS != n_return) {
      break;
    }
  } while (0);
  YSOS_LOG_DEBUG("driver Read done");
  return n_return;
}

int DoconfigDriver::Write(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer/*=NULL*/, DriverContexPtr context_ptr/*=NULL*/) {
  YSOS_LOG_DEBUG("driver Write execute");
  int n_return = YSOS_ERROR_SUCCESS;
  do {
    UINT8* buffer;
    UINT32 buffer_size;
    n_return = input_buffer->GetBufferAndLength(&buffer, &buffer_size);
    if (YSOS_ERROR_SUCCESS != n_return) {
      break;
    }
  } while (0);
  YSOS_LOG_DEBUG("driver Write done");
  return n_return;
}

int DoconfigDriver::Ioctl(int iCtrlID, BufferInterfacePtr pInputBuffer, BufferInterfacePtr pOutputBuffer) {
  YSOS_LOG_DEBUG("driver Ioctl execute");
  int n_return = YSOS_ERROR_SUCCESS;

  std::string str_result = "";
  Json::Reader js_reader;
  Json::FastWriter js_writer;
  Json::Value js_value;
  Json::Value js_ret;
  Json::Value js_child;

  do {
    if (pOutputBuffer == NULL) {
      YSOS_LOG_DEBUG("driver pOutputBuffer is null");
      n_return = YSOS_ERROR_FAILED;
      break;
    }

    UINT8* buffer;
    UINT32 buffer_size;
    n_return = pOutputBuffer->GetBufferAndLength(&buffer, &buffer_size);
    if (YSOS_ERROR_SUCCESS != n_return) {
      YSOS_LOG_DEBUG("driver pOutputBuffer GetBufferAndLength failed");
      break;
    }

    int n_result = 0;
    switch (iCtrlID) {
    case CMD_CONFIG_LOG: {
      log_info = (LOG *)buffer;
      std::string name = log_info->log_name_;
      std::string level = log_info->log_level_;
      std::string info = "";

      //CreateThread(NULL, 0, My_ReadFile, this, 0, NULL);

      My_ReadFile(name, level, info);
      js_value["type"] = "log_info";
      js_value["info"] = info;


      break;
    }
    case CMD_CONFIG_ADD: {
      std::string key_value((char *)buffer);
      int pos1 = key_value.find_first_of("|");
      std::string key = key_value.substr(0, pos1);
      int pos2 = key_value.find_last_of("|");
      std::string value = key_value.substr(pos1 + 1, pos2 - pos1 - 1);
      n_return = ChangeXml(key, value, false);

      break;
    }
    case CMD_CONFIG_MODIFY: {
      std::string key_value((char *)buffer);
      int pos1 = key_value.find_first_of("|");
      std::string key = key_value.substr(0, pos1);
      int pos2 = key_value.find_last_of("|");
      std::string value = key_value.substr(pos1 + 1, pos2 - pos1 - 1);
      n_return = ChangeXml(key, value);
      break;
    }
    case CMD_CONFIG_CALLBACKINFO: {
      OpenXml();
      char buf[20];
      //sprintf_s(buf, "config_%s", search_module_.c_str());
      sprintf(buf, "config_%s", search_module_.c_str());//add for linux
      YSOS_LOG_DEBUG(search_module_.c_str());
      js_value["type"] = buf;
      tinyxml2::XMLElement *root_element = my_document_->RootElement();
      tinyxml2::XMLElement *first_child = root_element->FirstChildElement();
      tinyxml2::XMLElement *second_child;
      if (strcmp(search_module_.c_str(), "info") == 0)
        second_child = first_child->FirstChildElement();
      else
        second_child = first_child->FirstChildElement(search_module_.c_str());
      for (; second_child != NULL; second_child = second_child->NextSiblingElement()) {
        js_child.clear();
        if (second_child->FirstChildElement() == NULL) {
          YSOS_LOG_DEBUG(second_child->Value());
          js_ret[second_child->Value()] = second_child->GetText();
        } else {
          tinyxml2::XMLElement *third_child = second_child->FirstChildElement();
          for (; third_child != NULL; third_child = third_child->NextSiblingElement()) {
            YSOS_LOG_DEBUG(third_child->Value());
            const tinyxml2::XMLAttribute *attr = third_child->FirstAttribute();
            YSOS_LOG_DEBUG("node is = " << third_child->Value() << ", sign is = " << attr->Value());
            if (strcmp("1", attr->Value()) == 0) {
              data_ptr_->SetData(third_child->Value(), third_child->GetText());
            } else if (strcmp("2", attr->Value()) == 0) {
              js_child[third_child->Value()] = third_child->GetText();
            } else {
              data_ptr_->SetData(third_child->Value(), third_child->GetText());
              js_child[third_child->Value()] = third_child->GetText();
            }
          }
          js_ret[second_child->Value()] = js_child;
        }
      }
      std::string ip;
      GetIP(ip);
      js_ret["ip"] = ip.c_str();
      char version[20] = {0};
      //TODO:add for linux
      //GetPrivateProfileString("ysos", "product_version", "", version, 20, "../version/version.ini");
      js_ret["version"] = version;
      js_value["info"] = js_ret;
      str_result = js_writer.write(js_value);
      str_result = GetUtility()->ReplaceAllDistinct ( str_result, "\\r\\n", "" );
      memcpy(buffer, str_result.c_str(), strlen(str_result.c_str()));
      YSOS_LOG_DEBUG("str_result = " << str_result);
      break;
    }
    default: {
      YSOS_LOG_DEBUG("invalid command");
      n_return = YSOS_ERROR_FAILED;
      break;
    }
    } //switch
  } while (0);

  YSOS_LOG_DEBUG("driver Ioctl done");
  return n_return;
}

int DoconfigDriver::Open(void *pParams) {
  YSOS_LOG_DEBUG("driver Open execute");
  int n_return = YSOS_ERROR_SUCCESS;
  do {

    if (NULL == pParams) {
      YSOS_LOG_DEBUG("driver Open Param is NULL");
      n_return = YSOS_ERROR_FAILED;
      break;
    }
    YSOS_LOG_DEBUG("xml_path = " << (char *)pParams);
    xml_path_ = (char *)pParams;
    OpenXml();
    data_ptr_ = GetDataInterfaceManager()->GetData(GLOBAL_DATA_KEY);
  } while (0);
  YSOS_LOG_DEBUG("driver Open done");

  return n_return;
}

void DoconfigDriver::Close(void *pParams /* = nullptr */) {
  YSOS_LOG_DEBUG("driver Close execute");

  YSOS_LOG_DEBUG("driver Close done");
}

int DoconfigDriver::ChangeXml(const std::string &key, const std::string &value, bool is_modify/* = true*/) {
  int n_return = YSOS_ERROR_SUCCESS;
  YSOS_LOG_DEBUG("DoconfigDriver::ChangeXml()[Enter]");

  OpenXml();
  do {
    int position = 0, last_position = 0, end_position = 0;
    bool is_first = true;
    tinyxml2::XMLElement *root = my_document_->RootElement(), *child = NULL;
    while (key.find_first_of(".", position) != std::string::npos) {
      position = key.find_first_of(".", position);
      if (is_first) {
        child = my_document_->RootElement();
        if (child == NULL) {
          return YSOS_ERROR_FAILED;
        }
        is_first = false;
      } else {
        child = child->FirstChildElement((key.substr(last_position + 1, position - last_position - 1)).c_str());
        //search_module_ = key.substr(end_position + 1).c_str();
        if (child == NULL) {
          return YSOS_ERROR_FAILED;
        }
      }
      last_position = position;
      ++position;
    }
    end_position = key.find_last_of(".");
    if (!is_modify) {
      YSOS_LOG_DEBUG("DoconfigDriver::ChangeXml() add element execute");
      tinyxml2::XMLElement *newElement = my_document_->NewElement(key.substr(end_position + 1).c_str());
      if (NULL == newElement) {
        n_return = YSOS_ERROR_FAILED;
        break;
      }
      tinyxml2::XMLText *newText = my_document_->NewText(value.c_str());
      if (NULL == newText) {
        n_return = YSOS_ERROR_FAILED;
        break;
      }
      newElement->LinkEndChild(newText);
      child->LinkEndChild(newElement);
      YSOS_LOG_DEBUG("DoconfigDriver::ChangeXml()add element done");
    } else {
      YSOS_LOG_DEBUG("DoconfigDriver::ChangeXml()modify element execute");
      child = child->FirstChildElement(key.substr(end_position + 1).c_str());
      if (child == NULL) {
        n_return = YSOS_ERROR_FAILED;
        break;
      }
      child->SetText(value.c_str());
      YSOS_LOG_DEBUG("DoconfigDriver::ChangeXml()modify element done");
    }
    std::string con_str = GetXmlUtil()->ConvertToString(root);
    std::string asc_utf8 = GetUtility()->AsciiToUtf8(con_str);
    my_document_->Clear();
    my_document_->Parse(asc_utf8.c_str());
    my_document_->SaveFile(xml_path_.c_str());

    YSOS_LOG_DEBUG("DoconfigDriver::ChangeXml()[Exit]");
  } while (0);

  return n_return;
}

int DoconfigDriver::OpenXml() {
  if (NULL == my_document_) {
    my_document_ = new tinyxml2::XMLDocument();
  } else {
    my_document_->Clear();
  }

  std::string xml_str = GetUtility()->ReadAllDataFromFile(xml_path_.c_str());
  if ("" == xml_str) {
    YSOS_LOG_DEBUG("driver read_data_from file is NULL");
    return YSOS_ERROR_FAILED;
  }
  if (my_document_->Parse(xml_str.c_str())) {
    YSOS_LOG_DEBUG("driver parse read_data error");
    return YSOS_ERROR_FAILED;
  }
  return YSOS_ERROR_SUCCESS;
}

void DoconfigDriver::GetIP(std::string &ip) {
  Json::Value js_value;
  Json::FastWriter js_writer;
  //TODO:add for linux
  /*
  //PIP_ADAPTER_INFO结构体指针存储本机网卡信息
  PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO();
  //得到结构体大小,用于GetAdaptersInfo参数
  unsigned long stSize = sizeof(IP_ADAPTER_INFO);
  //调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量;其中stSize参数既是一个输入量也是一个输出量
  int nRel = GetAdaptersInfo(pIpAdapterInfo,&stSize);
  //记录网卡数量
  int netCardNum = 0;
  //记录每张网卡上的IP地址数量
  int IPnumPerNetCard = 0;
  if (ERROR_BUFFER_OVERFLOW == nRel) {
    //如果函数返回的是ERROR_BUFFER_OVERFLOW
    //则说明GetAdaptersInfo参数传递的内存空间不够,同时其传出stSize,表示需要的空间大小
    //这也是说明为什么stSize既是一个输入量也是一个输出量
    //释放原来的内存空间
    delete pIpAdapterInfo;
    //重新申请内存空间用来存储所有网卡信息
    pIpAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[stSize];
    //再次调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量
    nRel=GetAdaptersInfo(pIpAdapterInfo,&stSize);
  }
  if (ERROR_SUCCESS == nRel) {
    while (pIpAdapterInfo) {
      IP_ADDR_STRING *pIpAddrString =&(pIpAdapterInfo->IpAddressList);
      do {
        char s[10] = {0};
        //sprintf_s(s, "IP地址%d", ++IPnumPerNetCard);
        sprintf(s, "IP地址%d", ++IPnumPerNetCard);//add for linux
        js_value[s] = pIpAddrString->IpAddress.String;
        pIpAddrString=pIpAddrString->Next;
      } while (pIpAddrString);
      pIpAdapterInfo = pIpAdapterInfo->Next;
    }
  }
  ip = js_writer.write(js_value);
  if (pIpAdapterInfo)
    delete pIpAdapterInfo;
  */
  return ;
}

void DoconfigDriver::My_ReadFile(const std::string &name, const std::string &level, std::string &info) {
  std::ifstream s;
  int count = 0;
  char temp[50] = {0};
  char *temp_info = new char[1024 * 1024];
  std::string ss;
  memset(temp_info, 0, 1024 * 1024);
  //sprintf_s(temp, "../log/%s", name.c_str());
  sprintf(temp, "../log/%s", name.c_str());//add for linux
  s.open(temp);
  if (s) {
    while (s.getline(temp_info, 1024 * 1024)) {
      ss = temp_info;
      std::string::size_type i = ss.find("]");
      if (level == "bebug") {
        info += temp_info;
        info += "\r\n";
      } else if (level == "info" && (temp_info[i - 10] == 'I') ||
                 temp_info[i - 10] == 'W' || temp_info[i - 10] == 'E') {
        info += temp_info;
        info += "\r\n";
      } else if (level == "warn" && (temp_info[i - 10] == 'W' || temp_info[i - 10] == 'E')) {
        info += temp_info;
        info += "\r\n";
      } else if (level == "error" && (temp_info[i - 10] == 'E')) {
        info += temp_info;
        info += "\r\n";
      }
      ++count;
      if (count == 20) {
        count = 0;
        my_log_.push_back(info);
      }
    }
    if (count < 20 && count != 0) {
      my_log_.push_back(info);
    }
  } else {
    YSOS_LOG_ERROR("open file failed");
    info = "";
  }
  delete []temp_info;
}

}