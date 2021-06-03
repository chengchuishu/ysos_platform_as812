/*@version 1.0
  *@author l.j..
  *@date Created on: 2016-10-20 13:48:23
  *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
  * 
*/

/// Private Headers //  NOLINT
#include "../include/resolutiondriver.h"


namespace ysos {
DECLARE_PLUGIN_REGISTER_INTERFACE(ResolutionDriver, DriverInterface);  ///<  插件的入口，一定要加上 //  NOLINT

ResolutionDriver::ResolutionDriver(const std::string &strClassName) : BaseInterfaceImpl(strClassName) {

  logger_ = GetUtility()->GetLogger("ysos.resolution");
}

ResolutionDriver::~ResolutionDriver(void) {
}

int ResolutionDriver::SetProperty(int type_id, void *type) {
  YSOS_LOG_DEBUG("driver SetProperty execute");
  int n_return = YSOS_ERROR_SUCCESS;

  YSOS_LOG_DEBUG("driver SetProperty done");

  return n_return;
}

int ResolutionDriver::Read(BufferInterfacePtr pBuffer, DriverContexPtr context_ptr/*=NULL*/) {
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

int ResolutionDriver::Write(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer/*=NULL*/, DriverContexPtr context_ptr/*=NULL*/) {
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

int ResolutionDriver::Ioctl(int iCtrlID, BufferInterfacePtr pInputBuffer, BufferInterfacePtr pOutputBuffer) {
  YSOS_LOG_DEBUG("driver Ioctl execute");
  int n_return = YSOS_ERROR_SUCCESS;

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

    Json::FastWriter js_writer;
    Json::Value js_value;
    Json::Value js_ret;
    std::string str_ret;
    char resolution_ratio[20];
    int n_result = 0;
    switch (iCtrlID) {
    case CMD_SCREEN_MODIFY: {
      std::string value((char *)buffer);
      int width = 0, height = 0;
      if (strcmp("768", value.c_str()) == 0) {
        width = 1366;
        height = 768;
      } else if (strcmp("1080", value.c_str()) == 0) {
        width = 1920;
        height = 1080;
      }
      n_return = ScreenResolutionModify(width, height);
      if (n_return == YSOS_ERROR_SUCCESS)
        data_ptr_->SetData("resolution", value.c_str());
      break;
    }
    case CMD_SCREEN_DELCACHE: {
      #ifdef _WIN32
        char szPath[MAX_PATH];
        DeleteUrlCache(File);
        if (SHGetSpecialFolderPath(NULL, szPath, CSIDL_INTERNET_CACHE, FALSE)) {
            //得到临时目录，并清空它.
            EmptyDirectory(szPath);
        }
      #else
        //TODO:add fro linux
      #endif
      YSOS_LOG_DEBUG("ie缓存清除成功");
      break;
    }
    case CMD_SCREEN_CALLBACKINFO: {
      #ifdef _WIN32
        width_ = GetSystemMetrics(SM_CXSCREEN);
        height_ = GetSystemMetrics(SM_CYSCREEN);
        sprintf_s(resolution_ratio, "%d", height_);
        js_value["type"] = "screen_info";
        js_ret["resolution_ratio"] = resolution_ratio;
        js_value["info"] = js_ret;
        str_ret = js_writer.write(js_value);
        str_ret = GetUtility()->ReplaceAllDistinct ( str_ret, "\\r\\n", "" );
        YSOS_LOG_DEBUG("str_ret = " << str_ret);
        memcpy(buffer, str_ret.c_str(), strlen(str_ret.c_str()));
      #else
        //TODO:add fro linux
      #endif
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

int ResolutionDriver::Open(void *pParams) {
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
    my_document_ = new tinyxml2::XMLDocument();
    my_document_->LoadFile(xml_path_.c_str());
    tinyxml2::XMLElement *root_element = my_document_->RootElement();
    tinyxml2::XMLElement *first_element = root_element->FirstChildElement("config_info");
    tinyxml2::XMLElement *second_element = first_element->FirstChildElement("common");
    tinyxml2::XMLElement *third_element = second_element->FirstChildElement("resolution");
    data_ptr_ = GetDataInterfaceManager()->GetData(GLOBAL_DATA_KEY);
#ifdef _WIN32
    width_ = GetSystemMetrics(SM_CXSCREEN);
    height_ = GetSystemMetrics(SM_CYSCREEN);
    if ((width_ != 1920) && (height_ != 1080)) {
      if (YSOS_ERROR_SUCCESS != ScreenResolutionModify(1920, 1080)) {
        ScreenResolutionModify(1366, 768);
        data_ptr_->SetData("resolution", "768");
      } else {
        data_ptr_->SetData("resolution", "1080");
      }
    }
#else
  //TODO:add fro linux
#endif
    data_ptr_->SetData("resolution", "1080");
  } while (0);
  YSOS_LOG_DEBUG("driver Open done");

  return n_return;
}

void ResolutionDriver::Close(void *pParams /* = nullptr */) {
  YSOS_LOG_DEBUG("driver Close execute");

  YSOS_LOG_DEBUG("driver Close done");
}

int ResolutionDriver::ScreenResolutionModify(UINT32 width, UINT32 height) {
#ifdef _WIN32
  DEVMODE dm_screen_settings;
  memset(&dm_screen_settings,0,sizeof(dm_screen_settings));
  EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm_screen_settings);
  dm_screen_settings.dmSize = sizeof(dm_screen_settings);
  dm_screen_settings.dmPelsWidth = width;
  dm_screen_settings.dmPelsHeight = height;
  if (ChangeDisplaySettings(&dm_screen_settings,CDS_UPDATEREGISTRY) != DISP_CHANGE_SUCCESSFUL) {
    YSOS_LOG_DEBUG("modify resolution failed");
    return YSOS_ERROR_FAILED;
  }
  YSOS_LOG_DEBUG("the current resolution is = " << width << "*" << height);
  tinyxml2::XMLElement *root_element = my_document_->RootElement();
  tinyxml2::XMLElement *first_element = root_element->FirstChildElement("config_info");
  tinyxml2::XMLElement *second_element = first_element->FirstChildElement("common");
  tinyxml2::XMLElement *third_element = second_element->FirstChildElement("resolution");
  third_element->SetText(height);
  my_document_->SaveFile(xml_path_.c_str());
#else
  //TODO:add fro linux
#endif
  return YSOS_ERROR_SUCCESS;
}

bool ResolutionDriver::DeleteUrlCache(DEL_CACHE_TYPE type) {
  return true;
}

bool ResolutionDriver::EmptyDirectory(std::string szPath, bool bDeleteDesktopIni /*= FALSE*/, bool bWipeIndexDat /*= FALSE*/) {
  //TODO:add for linux
#ifdef _WIN32
  WIN32_FIND_DATA wfd;
  HANDLE hFind;
  std::string sFullPath;
  std::string sFindFilter;
  DWORD dwAttributes = 0;

  sFindFilter = szPath;
  sFindFilter += "\\*.*";
  if ((hFind = FindFirstFile(sFindFilter.c_str(), &wfd)) == INVALID_HANDLE_VALUE) {
    return false;
  }

  do {
    if (strcmp(wfd.cFileName, ".") == 0 ||
        strcmp(wfd.cFileName, "..") == 0 ||
        (bDeleteDesktopIni == FALSE && strcmp(wfd.cFileName, "desktop.ini") == 0)) {
      continue;
    }

    sFullPath = szPath;
    sFullPath += "\\";
    sFullPath += wfd.cFileName;

    //去掉只读属性
    dwAttributes = GetFileAttributes(sFullPath.c_str());
    if (dwAttributes & FILE_ATTRIBUTE_READONLY) {
      dwAttributes &= ~FILE_ATTRIBUTE_READONLY;
      SetFileAttributes(sFullPath.c_str(), dwAttributes);
    }

    if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      EmptyDirectory(sFullPath, bDeleteDesktopIni, bWipeIndexDat);
      RemoveDirectory(sFullPath.c_str());
    } else {
      if (bWipeIndexDat && strcmp(wfd.cFileName, "index.dat") == 0) {
        WipeFile(szPath, wfd.cFileName);
      }
      DeleteFile(sFullPath.c_str());
    }
  } while (FindNextFile(hFind, &wfd));
  FindClose(hFind);
#else
  //TODO:add for linux
#endif
  return true;
}

bool ResolutionDriver::WipeFile(std::string szDir, std::string szFile) {
  std::string sPath;
#ifdef _WIN32
  HANDLE	hFile;
  DWORD	dwSize;
  DWORD	dwWrite;
  char	sZero[1000];
  memset(sZero, 0, 1000);

  sPath = szDir;
  sPath += "\\";
  sPath += szFile;

  hFile = CreateFile(sPath.c_str(), GENERIC_WRITE,
                     FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                     FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile == INVALID_HANDLE_VALUE) {
    return false;
  }

  dwSize = GetFileSize(hFile, NULL);

  //skip file header (actually, I don't know the file format of index.dat)
  dwSize -= 64;
  SetFilePointer(hFile, 64, NULL, FILE_BEGIN);

  while (dwSize > 0) {
    if (dwSize > 1000) {
      WriteFile(hFile, sZero, 1000, &dwWrite, NULL);
      dwSize -= 1000;
    } else {
      WriteFile(hFile, sZero, dwSize, &dwWrite, NULL);
      break;
    }
  }

  CloseHandle(hFile);
#else
  //TODO:add fro linux
#endif
  return true;
}

}