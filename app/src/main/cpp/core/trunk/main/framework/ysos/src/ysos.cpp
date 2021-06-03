/*
# ysos.cpp
# Definition of PlatformRPC
# Created on: 2017-01-19 14:23:23
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20170119, created by JinChengZhe
*/

// ysos.cpp : Defines the class behaviors for the application.
//

/// Private Headers
//#include "stdafx.h"
#include "ysos.h"
//#include "MainFrm.h"
#include <iostream>

#ifdef _WIN32
  //#
#else
  #include <stdlib.h>
  #include <dlfcn.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <fcntl.h>
  #include <unistd.h> //work direc
  #include <boost/filesystem.hpp>
#endif

#pragma region
/// Platform Headers
//#include <sys_daemon_package/minidump.h>
//#include "../../../protect/include/sys_daemon_package/externutility.h"
#include "externutility.h"
using namespace ysos::extern_utility;

#ifdef _WIN32
  #define YSOS_GUID                                 _T("Global\\{E346A65A-9755-48AA-B958-E08C4384EC23}") 
  #define YSOS_LOG_FILE_SPECIFIC_KEYWORD            _T("${ysos_log_path}$")
  #define YSOS_LOG_FILE_REPLACE_KEYWORD             _T("..")
  #define YSOS_LOG_FILE_SUB_FILE_PATH               _T("conf\\log\\log")
  #define YSOS_LOG_FILE_TEMPLATE_EXTENSION          _T(".properties.template")
  #define YSOS_LOG_FILE_EXTENSION                   _T(".properties")
  #define YSOS_SYS_PLATFORM_SDK_DLL_4_DEBUG         _T("libsys_platform_sdk_d.dll")
  #define YSOS_SYS_PLATFORM_SDK_DLL_4_RELEASE       _T("libsys_platform_sdk.dll")
#else
  #define YSOS_GUID                                 "Global/{E346A65A-9755-48AA-B958-E08C4384EC23}"
  #define YSOS_LOG_FILE_SPECIFIC_KEYWORD            "${ysos_log_path}$"
  #define YSOS_LOG_FILE_REPLACE_KEYWORD             ".."
  #define YSOS_LOG_FILE_SUB_FILE_PATH               "/conf/log/log"
  #define YSOS_LOG_FILE_TEMPLATE_EXTENSION          ".properties.template"
  #define YSOS_LOG_FILE_EXTENSION                   ".properties"
  #define YSOS_SYS_PLATFORM_SDK_DLL_4_DEBUG         "libsys_platform_sdk_d.so"
  #define YSOS_SYS_PLATFORM_SDK_DLL_4_RELEASE       "libsys_platform_sdk.so"
#endif

static const unsigned int g_buffer_size = 8192;

static const char* const YSOS_SDK_FUNC_NAME_OPENYSOSSDK = "OpenYSOSSDK";
static const char* const YSOS_SDK_FUNC_NAME_CLOSEYSOSSDK = "CloseYSOSSDK";
static const char* const YSOS_SDK_FUNC_NAME_INITYSOSSDKLOGGER = "InitYSOSSDKLogger";
static const char* const YSOS_SDK_FUNC_NAME_SETYSOSSDKOPENPARAMS = "SetYSOSSDKOpenParams";

#pragma endregion

#ifdef _DEBUG
    #define new DEBUG_NEW
#endif

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
    char fullpath[MAX_PATH];
    sprintf(fullpath,"%s%s", fpath, tfilename);
    strncpy(fpath,fullpath, MAX_PATH);
    return true;
}

bool PathAddExtension(char* fpath, const char* tfilename)
{
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

int GetEnvironmentVariable(const char* vname,char* system_path, int g_buffer_size)
{
    int ret = 0;
    char tgbuf[g_buffer_size];
    memset(tgbuf, 0, sizeof(tgbuf));
    char* vpath = getenv(vname);
    printf("%s\n",vpath);
    strncpy(system_path, tgbuf, g_buffer_size);
    ret = 1;
    return ret;
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

log4cplus::Logger logger_;

FP_OpenYSOSSDK g_fp_open_ysos_sdk = NULL;
FP_CloseYSOSSDK g_fp_close_ysos_sdk = NULL;
FP_InitYSOSSDKLogger g_fp_init_ysos_sdk_logger = NULL;
FP_SetYSOSSDKOpenParams g_fp_set_ysos_sdk_open_params = NULL;

//end add fro linux



// CysosApp


// CysosApp construction

CysosApp::CysosApp() :
    mutex_handle_ ( NULL ),
    module_handle_ ( NULL ),
    ysos_conf_info_ptr_ ( NULL ),
    is_run_ ( false )
{
    printf( "CysosApp::CysosApp[Enter]\n" );
    printf( "CysosApp::CysosApp[Exit]\n"  );
}

CysosApp::~CysosApp()
{
    if ( NULL != ysos_conf_info_ptr_ )
    {
        delete ysos_conf_info_ptr_;
        ysos_conf_info_ptr_ = NULL;
    }
}

// The one and only CysosApp object

CysosApp theApp;


// CysosApp initialization

int CysosApp::InitInstance()
{
    /// for debug
    //  ::MessageBoxA(NULL, "OK", "OK", MB_OK);
    printf( "CysosApp::InitInstance[Enter]\n" );
    /* for del linux
    //PrintDSA ( _T ( "ysos_exe" ), _T ( "CysosApp::InitInstance[Enter]" ) );
    ///  防止重复运行
    mutex_handle_ = OpenMutex ( MUTEX_ALL_ACCESS, FALSE, YSOS_GUID );
    if ( NULL != mutex_handle_ )
    {
        PrintDSA ( _T ( "YSOS_EXE" ), _T ( "ysos.exe is already running." ) );
        CloseHandle ( mutex_handle_ );
        mutex_handle_ = NULL;
        return TRUE;
    }
    else
    {
        mutex_handle_ = CreateMutex ( NULL, FALSE, YSOS_GUID );
        if ( NULL == mutex_handle_ )
        {
            PrintDSA ( _T ( "YSOS_EXE" ), _T ( "CysosApp::InitInstance[Fail][mutext_handle_ is NULL][last_error_code][%d]" ), GetLastError() );
            return FALSE;
        }
    }
    */
    printf( "CysosApp::InitInstance[Check Point][0]\n" );
    /* //  设置dump机制
    if ( 0 != GetMiniDump()->WriteDump() )
    {
        PrintDSA ( _T ( "YSOS_EXE" ), _T ( "CysosApp::InitInstance[Fail][WriteDump()]" ) );
        return FALSE;
    }
    */
    printf("CysosApp::InitInstance[Check Point][1]\n");
    ///  初始化日志
    TCHAR module_file_name[MAX_PATH] = {0};
    if ( -1 == GetModuleFileName ( NULL, module_file_name, MAX_PATH ) )
    {
        printf("CysosApp::InitInstance[Fail][The function[GetModuleFileName()] is failed][last_error_code][...]\n");
        return FALSE;
    }
    printf( "CysosApp::InitInstance[module_file_name][%s]\n", module_file_name );
    
    //*  ignore del for linux
    if ( TRUE != PathRemoveFileSpec ( module_file_name ) )
    {
        printf("CysosApp::InitInstance[Fail][The function[PathRemoveFileSpec()] is failed][last_error_code][...]\n");
        return FALSE;
    }
    printf( "CysosApp::InitInstance[module_file_name][%s]", module_file_name );
    if ( TRUE != PathRemoveFileSpec ( module_file_name ) )
    {
        printf("CysosApp::InitInstance[Fail][The function[PathRemoveFileSpec()] is failed][last_error_code][...]\n");
        return FALSE;
    }
    //*/
    printf("CysosApp::InitInstance[module_file_name][%s]\n", module_file_name);
    if ( TRUE != PathAppend ( module_file_name, "/log/ysos_exe.log" ) )
    {
        printf("CysosApp::InitInstance[Fail][The function[PathRemoveFileSpec()] is failed][last_error_code][...]\n");
        return FALSE;
    }
    printf("CysosApp::InitInstance[module_file_name][%s]\n", module_file_name);
    if ( true != ysos::log::InitLoggerExt ( module_file_name, "ysos_exe" ) )
    {
        printf("CysosApp::InitInstance[Fail]The function[ysos::log::InitLogger()] is failed\n");
        return FALSE;
    }
    printf("CysosApp::InitInstance[Check Point][2]\n");
    ///  判断logger是否可用
    logger_ = ysos::log::GetLogger ( "ysos_exe" );
    if ( true != logger_.valid() )
    {
        printf("CysosApp::InitInstance[Fail]The function[ysos::log::GetLogger()] is failed\n");
        return FALSE;
    }
    YSOS_LOG_DEBUG ( "CysosApp::InitInstance[Check Point][start to write log]" );
    printf("CysosApp::InitInstance[Check Point][3]\n");
    YSOS_LOG_DEBUG ( "CysosApp::InitInstance[Check Point][3]" );
    ///  设置平台环境变量
    if ( TRUE != SetYSOSEnvironmentVariable() )
    {
        printf("CysosApp::InitInstance[Fail][SetYSOSEnvironmentVariable()]\n");
        YSOS_LOG_DEBUG ( "CysosApp::InitInstance[Fail][SetYSOSEnvironmentVariable()]" );
        return FALSE;
    }
    printf("CysosApp::InitInstance[Check Point][4]\n");
    YSOS_LOG_DEBUG ( "CysosApp::InitInstance[Check Point][4]" );
    ///  加载平台sdk dll并获取公有函数指针
    if ( TRUE != LoadLibraryAndSetGetFunctionPtr() )
    {
        printf("CysosApp::InitInstance[Fail][LoadLibraryAndSetGetFunctionPtr()]\n");
        YSOS_LOG_DEBUG ( "CysosApp::InitInstance[Fail][LoadLibraryAndSetGetFunctionPtr()]" );
        return FALSE;
    }
    printf("CysosApp::InitInstance[Check Point][5]\n");
    YSOS_LOG_DEBUG ( "CysosApp::InitInstance[Check Point][5]" );
    ///  设置平台log属性
    if ( TRUE != SetYSOSLogProperty() )
    {
        printf("CysosApp::InitInstance[Fail][SetYSOSLogProperty()]\n");
        YSOS_LOG_DEBUG ( "CysosApp::InitInstance[Fail][SetYSOSLogProperty()]" );
        return FALSE;
    }
    printf("CysosApp::InitInstance[Check Point][6]\n");
    YSOS_LOG_DEBUG ( "CysosApp::InitInstance[Check Point][6]" );
    ///  用于电信赛特斯项目，与npapi框架并用时，删除框架目录下debug.log文件
    ///  delete todo action
    ///  Todo: 后期实现
    #pragma region
    #pragma endregion
    printf("CysosApp::InitInstance[Exit]\n");
    YSOS_LOG_DEBUG ( "CysosApp::InitInstance[Exit]" );
    return TRUE;
}

int CysosApp::ExitInstance()
{
    //TODO: handle additional resources you may have added
    printf( "CysosApp::ExitInstance[Enter]\n" );
    YSOS_LOG_DEBUG ( "CysosApp::ExitInstance[Enter]" );
    //AfxOleTerm ( FALSE );
    if ( NULL != module_handle_ )
    {
        printf( "CysosApp::ExitInstance[module_handle_ is not NULL]\n" );
        YSOS_LOG_DEBUG ( "CysosApp::ExitInstance[module_handle_ is not NULL]" );
        //if ( TRUE !=  FreeLibrary ( module_handle_ ) )
        if ( TRUE !=  dlclose ( module_handle_ ) )//add for linux
        {
            printf( "CysosApp::ExitInstance[Fail][FreeLibrary()][last_error_code][%s]\n", dlerror() );
            YSOS_LOG_DEBUG ( "CysosApp::ExitInstance[Fail][FreeLibrary()][last_error_code][ << GetLastError() << ]" );
        }
        module_handle_ = NULL;
    }
    if ( NULL != mutex_handle_ )
    {
        printf( "CysosApp::ExitInstance[Check Point][mutex_handle_ is not NULL]\n" );
        YSOS_LOG_DEBUG ( "CysosApp::ExitInstance[Check Point][mutex_handle_ is not NULL]" );
        //if ( TRUE != CloseHandle ( mutex_handle_ ) )
        if ( TRUE != dlclose ( mutex_handle_ ) )
        {
            printf( "CysosApp::ExitInstance[Fail][CloseHandle()][last_error_code][%s]\n", dlerror() );
            YSOS_LOG_DEBUG ( "CysosApp::ExitInstance[Fail][CloseHandle()][last_error_code][ << GetLastError() << ]" );
        }
        mutex_handle_ = NULL;
    }
    printf( "CysosApp::ExitInstance[Exit]\n" );
    YSOS_LOG_DEBUG ( "CysosApp::ExitInstance[Exit]" );
    return ExitInstance();
}

// CysosApp message handlers


// CAboutDlg dialog used for App About

#pragma region
int CysosApp::SetYSOSLogProperty()
{
    printf( "CysosApp::SetYSOSLogProperty[Enter]\n" );
    int result = FALSE;
    do
    {
        /// 获取日志配置文件模板路径
        std::string log_template_properties_file_path;
        if ( TRUE != GetYSOSLogTemplatePropertiesFilePath ( log_template_properties_file_path ) )
        {
            printf( "CysosApp::SetYSOSLogProperty[Fail][GetYSOSLogTemplatePropertiesFilePath()]\n" );
            break;
        }
        printf( "CysosApp::SetYSOSLogProperty[log_template_properties_file_path][%s]\n", log_template_properties_file_path.c_str() );
        /// 重新生成日志配置文件
        std::string log_properties_file_path;
        if ( TRUE != CreateYSOSLogPropertiesFile ( log_template_properties_file_path, log_properties_file_path ) )
        {
            printf( "CysosApp::SetYSOSLogProperty[Fail][CreateYSOSLogPropertiesFile()]\n" );
            break;
        }
        printf( "CysosApp::SetYSOSLogProperty[log_properties_file_path][%s]\n", log_properties_file_path.c_str() );
        /// 加载log文件夹下log.properties文件
        FileLoader file_loader;
        if ( TRUE != file_loader.OpenFile ( log_properties_file_path.c_str() ) )
        {
            printf( "CysosApp::SetYSOSLogProperty[Fail][file_loader.OpenFile()]\n" );
            break;
        }
        printf( "CysosApp::SetYSOSLogProperty[Check Point][0]\n" );
        /// 获取文件内容
        char* file_data = file_loader.GetFileData();//add for linux
        printf( "CysosApp::SetYSOSLogProperty[Check Point][0][file_data][%p]\n", file_data );
        //printf( "CysosApp::SetYSOSLogProperty[Check Point][0][file_loader.file_data_][%s]\n", file_data );
        if ( NULL == file_data )
        {
            printf( "CysosApp::SetYSOSLogProperty[Fail][file_data is NULL]\n" );
            break;
        }
        printf( "CysosApp::SetYSOSLogProperty[Check Point][1]\n" );
        /// 替换字符
        std::string temp_string = file_data;
        //temp_string.Replace ( YSOS_LOG_FILE_SPECIFIC_KEYWORD, YSOS_LOG_FILE_REPLACE_KEYWORD );
        //temp_string.Replace ( "\\", "/" );
        string_replace(temp_string, YSOS_LOG_FILE_SPECIFIC_KEYWORD, YSOS_LOG_FILE_REPLACE_KEYWORD );
        string_replace(temp_string, "\\", "/" );
        printf( "CysosApp::SetYSOSLogProperty[Check Point][2]\n" );
        /// 重新加载log.properties文件，把替换的内容写到文件里
        if ( TRUE != file_loader.ResizeFile ( temp_string.length() ) )
        {
            printf( "CysosApp::SetYSOSLogProperty[Fail][file_loader.ResizeFile()]\n" );
            break;
        }
        printf( "CysosApp::SetYSOSLogProperty[Check Point][3]\n" );
        /// 获取文件内容
        char* new_file_data = file_loader.GetFileData();//add for linux
        if ( NULL == new_file_data )
        {
            printf( "CysosApp::SetYSOSLogProperty[Fail][new_file_data is NULL]\n" );
            break;
        }
        
        printf( "CysosApp::SetYSOSLogProperty[Check Point][4]\n" );
        printf( "CysosApp::SetYSOSLogProperty[Check Point][4][%p]\n",  new_file_data );
        //ZeroMemory ( new_file_data, sizeof ( new_file_data ) );
        printf( "CysosApp::SetYSOSLogProperty[Check Point][4][%ld]\n",  file_loader.file_size_);
        memset(new_file_data, 0, file_loader.file_size_);//add for linux
        printf( "CysosApp::SetYSOSLogProperty[Check Point][4][%ld]\n",  strlen(temp_string.c_str()));
        memcpy ( new_file_data, temp_string.c_str(), strlen(temp_string.c_str()));
        if ( NULL == new_file_data )
        {
            printf( "CysosApp::SetYSOSLogProperty[Fail][memcpy_s()][new_file_data IS NULL]\n");
            break;
        }
        printf( "CysosApp::SetYSOSLogProperty[Check Point][5]\n" );
        /// 释放加载的new_file_data
        if ( TRUE != file_loader.CloseFile ( TRUE ) )
        {
            printf( "CysosApp::SetYSOSLogProperty[Fail][file_loader.CloseFile()]\n" );
            break;
        }
        printf( "CysosApp::SetYSOSLogProperty[Check Point][6]\n" );
        ///  初始化平台日志
        if ( NULL == g_fp_init_ysos_sdk_logger )
        {
            printf( "CysosApp::SetYSOSLogProperty[Fail][g_fp_init_ysos_sdk_logger_ is NULL]\n" );
            break;
        }
        printf( "CysosApp::SetYSOSLogProperty[Check Point][7]\n" );
        if ( true != g_fp_init_ysos_sdk_logger ( log_properties_file_path.c_str() ) )
        {
            printf( "CysosApp::SetYSOSLogProperty[Fail][g_fp_init_ysos_sdk_logger_()]\n" );
            break;
        }
        printf( "CysosApp::SetYSOSLogProperty[Check Point][End]\n" );
        result = TRUE;
    } while ( 0 );
    printf( "CysosApp::SetYSOSLogProperty[Exit]\n" );
    return result;
}

int CysosApp::GetYSOSLogTemplatePropertiesFilePath ( std::string& log_template_properties_file_path )
{
    printf( "CysosApp::GetYSOSLogTemplatePropertiesFilePath[Enter]\n" );
    int result = FALSE;
    do
    {
        /// 获取模块所在路径
        TCHAR temp_log_template_properties_file_path[MAX_PATH * 2] = {0};
        if ( 0 == GetModuleFileName ( NULL, temp_log_template_properties_file_path, MAX_PATH * 2 ) )
        {
            printf( "CysosApp::GetYSOSLogTemplatePropertiesFilePath[Fail][GetModuleFileName()][last_error_code][%s]\n", "GetLastError()");
            break;
        }
        printf( "CysosApp::GetYSOSLogTemplatePropertiesFilePath[temp_log_template_properties_file_path][0][%s]\n", temp_log_template_properties_file_path );
        /// 从模块所在路径中删除模块名称
        //* add for linux
        if ( TRUE != PathRemoveFileSpec ( temp_log_template_properties_file_path ) )
        {
            printf( "CysosApp::GetYSOSLogTemplatePropertiesFilePath[Fail][PathRemoveFileSpec()][last_error_code][%s]\n", "GetLastError()" );
            break;
        }
        //*/
        boost::filesystem::path ttpath = temp_log_template_properties_file_path; //add for linux
        char ttmp_module_file_name[MAX_PATH*2] = {0,}; //add for linux
        strcpy(ttmp_module_file_name, ttpath.parent_path().c_str()); //add for linux
        printf( "CysosApp::GetYSOSLogTemplatePropertiesFilePath[temp_log_template_properties_file_path][1][%s]\n", temp_log_template_properties_file_path );
        /// 从模块所在路径中删除lib
        //* add for linux
        if ( TRUE != PathRemoveFileSpec ( temp_log_template_properties_file_path ) )
        {
            printf( "CysosApp::GetYSOSLogTemplatePropertiesFilePath[Fail][PathRemoveFileSpec()][last_error_code][%s]\n", "GetLastError()" );
            break;
        }
        //*/
        boost::filesystem::path tttpath = ttmp_module_file_name; //add for linux
        char tttmp_module_file_name[MAX_PATH*2] = {0,}; //add for linux
        strcpy(tttmp_module_file_name, tttpath.parent_path().c_str()); //add for linux
        printf( "CysosApp::GetYSOSLogTemplatePropertiesFilePath[temp_log_template_properties_file_path][2][%s]\n", temp_log_template_properties_file_path );
        /// 在模块所在路径中添加模板文件名
        //* add for linux
        if ( TRUE != PathAppend ( temp_log_template_properties_file_path, YSOS_LOG_FILE_SUB_FILE_PATH ) )
        {
            printf( "CysosApp::GetYSOSLogTemplatePropertiesFilePath[Fail][PathAppend()][last_error_code][%s]\n", "GetLastError()");
            break;
        }
        //*/
        strcat(tttmp_module_file_name, YSOS_LOG_FILE_SUB_FILE_PATH); //add for linux
        printf( "CysosApp::GetYSOSLogTemplatePropertiesFilePath[temp_log_template_properties_file_path][3][%s]\n", temp_log_template_properties_file_path );
        /// 添加日志模板文件扩展名
        //* add for linux
        if ( TRUE != PathAddExtension ( temp_log_template_properties_file_path, YSOS_LOG_FILE_TEMPLATE_EXTENSION ) )
        {
            printf( "CysosApp::GetYSOSLogTemplatePropertiesFilePath[Fail][PathAddExtension()][last_error_code][%s]\n", "GetLastError()" );
            break;
        }
        //*/
        //strcat(tttmp_module_file_name, YSOS_LOG_FILE_TEMPLATE_EXTENSION); //no used ///add for linux
        printf( "CysosApp::GetYSOSLogTemplatePropertiesFilePath[temp_log_template_properties_file_path][4][%s]\n", temp_log_template_properties_file_path );
        /// 拷贝拼装的日志模板文件路径
        log_template_properties_file_path = temp_log_template_properties_file_path;
        printf( "CysosApp::GetYSOSLogTemplatePropertiesFilePath[Check Point][End][log_template_properties_file_path][%s]\n", log_template_properties_file_path.c_str() );
        result = TRUE;
    } while ( 0 );
    printf( "CysosApp::GetYSOSLogTemplatePropertiesFilePath[Exit]\n" );
    return result;
}

int CysosApp::CreateYSOSLogPropertiesFile ( const std::string& log_template_properties_file_path, std::string& log_properties_file_path )
{
    printf( "CysosApp::CreateYSOSLogPropertiesFile[Enter]\n" );
    int result = FALSE;
    do
    {
        if ( true == log_template_properties_file_path.empty() )
        {
            printf( "CysosApp::CreateYSOSLogPropertiesFile[Fail][log_template_properties_file_path is empty]\n" );
            break;
        }
        printf( "CysosApp::CreateYSOSLogPropertiesFile[log_template_properties_file_path][%s]\n", log_template_properties_file_path.c_str() );
        /// 判断日志模板文件是否存在
        if ( TRUE != PathFileExists ( log_template_properties_file_path.c_str() ) )
        {
            printf( "CysosApp::CreateYSOSLogPropertiesFile[Fail][PathFileExists()][last_error_code][%s]\n", "no exists log file" );
            break;
        }
        printf( "CysosApp::CreateYSOSLogPropertiesFile[Check Point][0]\n" );
        /// 拼装一个新的日志配置文件路径
        TCHAR temp_log_properties_file_path[MAX_PATH * 2] = {0,};
        //StringCchCopy ( temp_log_properties_file_path, MAX_PATH * 2, log_template_properties_file_path.c_str() );
        strncpy( temp_log_properties_file_path, log_template_properties_file_path.c_str(), MAX_PATH * 2);//add for linux 
        PathRemoveExtension ( temp_log_properties_file_path );
        printf( "CysosApp::CreateYSOSLogPropertiesFile[temp_log_properties_file_path][%s]\n", temp_log_properties_file_path );
        /// 重新拷贝一个新的log.properties
        ///* //add for linux
        boost::system::error_code errinfo;// add for linux ///define copy file error info
        if ( TRUE != CopyFile ( log_template_properties_file_path.c_str(), temp_log_properties_file_path, boost::filesystem::copy_option::overwrite_if_exists, errinfo))
        {
            printf( "CysosApp::CreateYSOSLogPropertiesFile[Fail][CopyFile()][last_error_code][%d]--[%s]\n", errinfo.value(), errinfo.category().name() );
            break;
        }
        //*/
        /// 拷贝拼装的日志配置文件路径
        log_properties_file_path = temp_log_properties_file_path;
        printf( "CysosApp::CreateYSOSLogPropertiesFile[Check Point][End][log_properties_file_path][%s]\n", log_properties_file_path.c_str() );
        result = TRUE;
    } while ( 0 );
    printf( "CysosApp::CreateYSOSLogPropertiesFile[Exit]\n" );
    return result;
}

int CysosApp::SetYSOSEnvironmentVariable()
{
    printf("CysosApp::SetYSOSEnvironmentVariable[Enter]\n");
    int result = FALSE;
    do
    {
        /// 获取系统path
        TCHAR system_path[g_buffer_size] = {0,};
        //if ( 0 == GetEnvironmentVariable ( _T ( "PATH" ), system_path, g_buffer_size ) )
        char* tval = getenv("PATH");
        printf("[%s]\n", getenv("PATH"));
        if ( NULL == tval ) //add for linux
        {
            printf( "CysosApp::SetYSOSEnvironmentVariable[Fail][GetEnvironmentVariable()][val][%s]\n",tval);
            break;
        } else {
            strcpy(system_path, tval);
        }
        printf("CysosApp::SetYSOSEnvironmentVariable[Check Point][0][system_path][%s]\n", system_path );
        /// 获取模块文件路径
        TCHAR module_file_name[MAX_PATH * 2] = {0};
        if ( 0 == GetModuleFileName ( NULL, module_file_name, MAX_PATH * 2 ) )
        {
            printf( "CysosApp::SetYSOSEnvironmentVariable[Fail][GetModuleFileName()][last_error_code][...]\n");
            break;
        }
        printf( "CysosApp::SetYSOSEnvironmentVariable[Check Point][1][module_file_name][%s]\n", module_file_name );
        ///* //add for linux
        if ( TRUE != PathRemoveFileSpec ( module_file_name ) )
        {
            printf( "CysosApp::SetYSOSEnvironmentVariable[Fail][PathRemoveFileSpec()][last_error_code][...]\n");
            break;
        }//*/
        //boost::filesystem::path tpath = module_file_name; //add for linux
        //char tmp_module_file_name[MAX_PATH*2] = {0,}; //add for linux
        //strcpy(tmp_module_file_name, tpath.parent_path().c_str()); //add for linux
        printf( "CysosApp::SetYSOSEnvironmentVariable[Check Point][2][module_file_name][%s]\n", module_file_name );
        //if ( TRUE != SetCurrentDirectory ( module_file_name ) )
        if ( -1 == chdir ( module_file_name ) )//add for linux
        {
            printf( "CysosApp::SetYSOSEnvironmentVariable[Fail][SetCurrentDirectory()][last_error_code][...]\n");
            break;
        }
        printf( "CysosApp::SetYSOSEnvironmentVariable[Check Point][3][module_file_name][%s]\n", module_file_name );
        //TCHAR variable_name[] = _T ( "path" );
        TCHAR variable_name[] = "path";
        TCHAR variable_value[g_buffer_size] = {0};
        //StringCchPrintf ( variable_value, g_buffer_size, _T ( "%s;%s\\..\\os_lib;%s\\..\\lib;%s\\..\\extern_lib;%s;" ), tmp_module_file_name, tmp_module_file_name, tmp_module_file_name, tmp_module_file_name, system_path );
        //char *tbuf; //add for linux
        sprintf(variable_value, "%s;%s/../lib;%s/../extern_lib;%s;", module_file_name, module_file_name, module_file_name, system_path );//add for linux
        //strcpy(variable_value, tbuf);//add for linux
        printf( "CysosApp::SetYSOSEnvironmentVariable[Check Point][4][variable_value][%s]\n", variable_value );
        //if ( TRUE != SetEnvironmentVariable ( variable_name, variable_value ) )
        if ( 0 != setenv ( variable_name, variable_value, 1) )
        {
            printf( "CysosApp::SetYSOSEnvironmentVariable[Fail][SetEnvironmentVariable()][last_error_code][...]\n");
            break;
        }
        printf( "CysosApp::SetYSOSEnvironmentVariable[Check Point][End]\n" );
        result = TRUE;
    } while ( 0 );
    printf( "CysosApp::SetYSOSEnvironmentVariable[Exit]\n" );
    return result;
}

int CysosApp::LoadLibraryAndSetGetFunctionPtr()
{
    printf( "CysosApp::LoadLibraryAndSetGetFunctionPtr[Enter]\n");
    int result = FALSE;
    do
    {
        #ifdef _WIN32
          #ifdef _DEBUG
            module_handle_ = LoadLibrary ( YSOS_SYS_PLATFORM_SDK_DLL_4_DEBUG );
          #else
            module_handle_ = LoadLibrary ( YSOS_SYS_PLATFORM_SDK_DLL_4_RELEASE );
          #endif
        #else
          //open so lib
          module_handle_ = dlopen ( YSOS_SYS_PLATFORM_SDK_DLL_4_RELEASE, RTLD_LAZY);
          printf("CysosApp::LoadLibraryAndSetGetFunctionPtr[module_handle_][%p]\n", module_handle_);
        #endif
        if ( !module_handle_ )
        {
            printf( "CysosApp::LoadLibraryAndSetGetFunctionPtr[Fail][module_handle_ is NULL][dlerror][%s]\n", dlerror());
            //fprintf (stderr, "%s\n", dlerror());
            //exit(EXIT_FAILURE);//add for linux
            break;
        }
        printf( "CysosApp::LoadLibraryAndSetGetFunctionPtr[Check Point][0]\n");
        dlerror();
        #ifdef _WIN32
          g_fp_open_ysos_sdk = ( FP_OpenYSOSSDK ) GetProcAddress ( module_handle_, YSOS_SDK_FUNC_NAME_OPENYSOSSDK );
        #else
          //清除之前存在的错误
          dlerror(); //add for linux
          g_fp_open_ysos_sdk = ( FP_OpenYSOSSDK ) dlsym ( module_handle_, YSOS_SDK_FUNC_NAME_OPENYSOSSDK );
        #endif
        if ( NULL == g_fp_open_ysos_sdk )
        {
            printf( "CysosApp::LoadLibraryAndSetGetFunctionPtr[Fail][g_fp_open_ysos_sdk is NULL][last_error_code][%s]\n", dlerror());
            break;
        }
        printf( "CysosApp::LoadLibraryAndSetGetFunctionPtr[Check Point][1]\n");
        #ifdef _WIN32
          g_fp_close_ysos_sdk = ( FP_CloseYSOSSDK ) GetProcAddress ( module_handle_, YSOS_SDK_FUNC_NAME_CLOSEYSOSSDK );
        #else
          g_fp_close_ysos_sdk = ( FP_CloseYSOSSDK ) dlsym ( module_handle_, YSOS_SDK_FUNC_NAME_CLOSEYSOSSDK );
        #endif
        if ( NULL == g_fp_close_ysos_sdk )
        {
            printf( "CysosApp::LoadLibraryAndSetGetFunctionPtr[Fail][g_fp_close_ysos_sdk is NULL][last_error_code][...]\n");
            break;
        }
        printf( "CysosApp::LoadLibraryAndSetGetFunctionPtr[Check Point][2]\n");
        #ifdef _WIN32
          g_fp_init_ysos_sdk_logger = ( FP_InitYSOSSDKLogger ) GetProcAddress ( module_handle_, YSOS_SDK_FUNC_NAME_INITYSOSSDKLOGGER );
        #else
          g_fp_init_ysos_sdk_logger = ( FP_InitYSOSSDKLogger ) dlsym ( module_handle_, YSOS_SDK_FUNC_NAME_INITYSOSSDKLOGGER );
        #endif
        if ( NULL == g_fp_init_ysos_sdk_logger )
        {
            printf( "CysosApp::LoadLibraryAndSetGetFunctionPtr[Fail][g_fp_init_ysos_sdk_logger is NULL][last_error_code][...]\n");
            break;
        }
        printf( "CysosApp::LoadLibraryAndSetGetFunctionPtr[Check Point][3]\n");
        #ifdef _WIN32
          g_fp_set_ysos_sdk_open_params = ( FP_SetYSOSSDKOpenParams ) GetProcAddress ( module_handle_, YSOS_SDK_FUNC_NAME_SETYSOSSDKOPENPARAMS );
        #else
          g_fp_set_ysos_sdk_open_params = ( FP_SetYSOSSDKOpenParams ) dlsym ( module_handle_, YSOS_SDK_FUNC_NAME_SETYSOSSDKOPENPARAMS );
        #endif  
        if ( NULL == g_fp_set_ysos_sdk_open_params )
        {
            printf( "CysosApp::LoadLibraryAndSetGetFunctionPtr[Fail][g_fp_set_ysos_sdk_open_params is NULL][last_error_code][...]\n");
            break;
        }
        printf( "CysosApp::LoadLibraryAndSetGetFunctionPtr[Check Point][End]\n");
        result = TRUE;
    } while ( 0 );
    printf( "CysosApp::LoadLibraryAndSetGetFunctionPtr[Exit]\n");
    return result;
}
#pragma endregion


// User Define Functions
#pragma region
bool CysosApp::LoadConfig()
{
    bool result = false;
    do
    {
        if ( NULL == ysos_conf_info_ptr_ )
        {
            ysos_conf_info_ptr_ = new YSOSConfInfo();
            if ( NULL == ysos_conf_info_ptr_ )
            {
                break;
            }
        }
        result = true;
    } while ( 0 );
    return result;
}

bool CysosApp::RunYSOSPlatform()
{
    printf( "CysosApp::RunYSOSPlatform[Enter]\n" );
    YSOS_LOG_DEBUG ( "CysosApp::RunYSOSPlatform[Enter]" );
    
    bool result = false;
    do
    {
        if ( NULL == g_fp_open_ysos_sdk )
        {
            printf( "CysosApp::RunYSOSPlatform[Fail][g_fp_open_ysos_sdk is NULL]\n" );
            YSOS_LOG_DEBUG ( "CysosApp::RunYSOSPlatform[Fail][g_fp_open_ysos_sdk is NULL]" );
            break;
        }
        printf( "CysosApp::RunYSOSPlatform[Check Point][0]\n" );
        YSOS_LOG_DEBUG ( "CysosApp::RunYSOSPlatform[Check Point][0]" );
        /// 配置sdk参数
        if ( NULL == g_fp_set_ysos_sdk_open_params )
        {
            printf( "CysosApp::RunYSOSPlatform[Check Point][fp_set_ysos_sdk_config_params_ is NULL]\n" );
            YSOS_LOG_DEBUG ( "CysosApp::RunYSOSPlatform[Check Point][fp_set_ysos_sdk_config_params_ is NULL]" );
            break;
        }
        printf( "CysosApp::RunYSOSPlatform[Check Point][1]\n" );
        YSOS_LOG_DEBUG ( "CysosApp::RunYSOSPlatform[Check Point][1]" );
        ysos::ysos_sdk::OpenParams open_params;
        if ( 0 != g_fp_set_ysos_sdk_open_params ( open_params, ysos::ysos_sdk::OpenParams::SET_CONF_FILE_NAME, "ysos.conf" ) )
        {
            printf( "CysosApp::RunYSOSPlatform[Fail][The function[g_fp_set_ysos_sdk_open_params_()] is failed]\n" );
            YSOS_LOG_DEBUG ( "CysosApp::RunYSOSPlatform[Fail][The function[g_fp_set_ysos_sdk_open_params_()] is failed]" );
            break;
        }
        printf( "CysosApp::RunYSOSPlatform[Check Point][2]\n" );
        YSOS_LOG_DEBUG ( "CysosApp::RunYSOSPlatform[Check Point][2]" );
        open_params.type_ = ysos::ysos_sdk::OpenParams::PLATFORM;
        printf( "CysosApp::RunYSOSPlatform[Check Point][3]\n" );
        YSOS_LOG_DEBUG ( "CysosApp::RunYSOSPlatform[Check Point][3]" );
        if ( 0 != g_fp_open_ysos_sdk ( &open_params, NULL ) )
        {
            printf( "CysosApp::RunYSOSPlatform[Fail][The function[fp_open_ysos_sdk_()] is failed]\n" );
            YSOS_LOG_DEBUG ( "CysosApp::RunYSOSPlatform[Fail][The function[fp_open_ysos_sdk_()] is failed]" );
            break;
        }
        printf( "CysosApp::RunYSOSPlatform[Check Point][End]\n" );
        YSOS_LOG_DEBUG ( "CysosApp::RunYSOSPlatform[Check Point][End]" );
        result = true;
    } while ( 0 );
    printf( "CysosApp::RunYSOSPlatform[Exit]\n" );
    YSOS_LOG_DEBUG ( "CysosApp::RunYSOSPlatform[Exit]" );
    return result;
}

bool CysosApp::StopYSOSPlatform()
{
    printf( "CysosApp::StopYSOSPlatform[Enter]\n" );
    YSOS_LOG_DEBUG ( "CysosApp::StopYSOSPlatform[Enter]" );
    bool result = false;
    do
    {
        if ( NULL == g_fp_close_ysos_sdk )
        {
            printf( "CysosApp::RunYSOSPlatform[Fail][g_fp_close_ysos_sdk_ is NULL]\n" );
            YSOS_LOG_DEBUG ( "CysosApp::RunYSOSPlatform[Fail][g_fp_close_ysos_sdk_ is NULL]" );
            break;
        }
        if ( 0 != g_fp_close_ysos_sdk ( NULL ) )
        {
            printf( "CysosApp::StopYSOSPlatform[Fail][The function[g_fp_close_ysos_sdk_()] is failed]\n" );
            YSOS_LOG_DEBUG ( "CysosApp::StopYSOSPlatform[Fail][The function[g_fp_close_ysos_sdk_()] is failed]" );
            break;
        }
        printf( "CysosApp::StopYSOSPlatform[Check Point][0]\n" );
        YSOS_LOG_DEBUG ( "CysosApp::StopYSOSPlatform[Check Point][0]" );
        result = true;
    } while ( 0 );
    printf( "CysosApp::StopYSOSPlatform[Exit]\n" );
    YSOS_LOG_DEBUG ( "CysosApp::StopYSOSPlatform[Exit]" );
    return result;
}

bool CysosApp::NotifyDaemonYSOSState()
{
    bool bDaemonRuning = false;
    //#ifdef _DEBUG
    //	if (TRUE == GetProcessWrapper()->IsRuningProcessEx()) {
    //		bDaemonRuning = TRUE;
    //	}
    //#else
    //	if (TRUE == GetProcessWrapper()->IsRuningProcessEx()) {
    //		bDaemonRuning = TRUE;
    //	}
    //#endif
    if ( !bDaemonRuning )
    {
        //TODO:
        /*
        Client objClient;
        objClient.Run();
        Sleep ( 100 );
        */
    }
    return true;
}
#pragma endregion


// CysosApp message handlers
#pragma region
/**
 *  auto start sdk service 
 **/
int CysosApp::OnAutoRun()
{
    printf( "CysosApp::OnAutoRun[Enter]\n" );
    
    printf( "CysosApp::OnAutoRun[Check Point][1]\n" );
    ///  获取配置属性值
    if ( true != LoadConfig() )
    {
        printf( "CysosApp::OnAutoRun[Fail][0]\n" );
        return -1;
    }
    printf( "CysosApp::OnAutoRun[Check Point][2]\n" );
    ///  初始化PopupMenu
    if ( NULL == ysos_conf_info_ptr_ )
    {
        printf( "CysosApp::OnAutoRun[Fail][1]\n" );
        return -1;
    }
    printf( "CysosApp::OnAutoRun[Check Point][3]\n" );
    //NOTHING TODO:
    printf( "CysosApp::OnAutoRun[Check Point][4]\n" );
    ///  设置平台sdk interface 指针
    if ( NULL == g_fp_open_ysos_sdk )
    {
        printf( "CysosApp::OnAutoRun[Fail][g_fp_open_ysos_sdk is NULL]\n" );
        return -1;
    }
    printf( "CysosApp::OnAutoRun[Check Point][5]\n" );
    if ( NULL == g_fp_close_ysos_sdk )
    {
        printf( "CysosApp::OnAutoRun[Fail][g_fp_close_ysos_sdk is NULL]\n" );
        return -1;
    }
    printf( "CysosApp::OnAutoRun[Check Point][6]\n" );
    if ( NULL == g_fp_set_ysos_sdk_open_params )
    {
        printf( "CysosApp::OnAutoRun[Fail][g_fp_set_ysos_sdk_open_params is NULL]\n" );
        return -1;
    }
    printf( "CysosApp::OnAutoRun[Check Point][7]\n" );
    ///Test
    //NotifyDaemonYSOSState();
    ///  启动平台
    if ( NULL != ysos_conf_info_ptr_ )
    {
        printf( "CysosApp::OnAutoRun[Check Point][8]\n" );
        if ( true == ysos_conf_info_ptr_->auto_run )
        {
            printf( "CysosApp::OnAutoRun[Check Point][9]\n" );
            //TODO:start all sdk services
            OnRealRun();
        }
        printf( "CysosApp::OnAutoRun[Check Point][10]\n" );
    }
    printf( "CysosApp::OnAutoRun[Exit]\n" );
    return 0;
}

/**
 *  hand operate sdk service start run 
 **/
void CysosApp::OnRealRun()
{
    // TODO: Add your command handler code here
    printf( "CysosApp::OnRealRun[Enter]\n" );
    do
    {
        if ( true != RunYSOSPlatform() )
        {
            printf( "CysosApp::OnRealRun[Fail][0]\n" );
            break;
        }
        printf( "CysosApp::OnRealRun[Check Point][0]\n" );
        is_run_ = true;
        NotifyDaemonYSOSState();
        printf( "CysosApp::OnRealRun[Check Point][3]\n" );
    } while ( 0 );
    printf( "CysosApp::OnRealRun[Exit]\n" );
    return;
}

/**
 *  hand operate sdk service stop run but not exit process
 **/
void CysosApp::OnHandStop()
{
    // TODO: Add your command handler code here
    printf( "CysosApp::OnHandStop[Enter]\n" );
    do
    {
        if ( true != StopYSOSPlatform() )
        {
            printf( "CysosApp::OnHandStop[Fail][0]\n" );
            break;
        }
        printf( "CysosApp::OnHandStop[Check Point][0]\n" );
        is_run_ = false;
        printf( "CysosApp::OnHandStop[Check Point][2]\n" );
    } while ( 0 );
    printf( "CysosApp::OnHandStop[Exit]\n" );
    return;
}

/**
 *  hand operate sdk service stop run and exit process 
 **/
void CysosApp::OnHandExit()
{
    // TODO: Add your command handler code here
    printf( "CysosApp::OnHandExit[Enter]\n" );
    if ( true == is_run_ )
    {
        printf( "CysosApp::OnHandExit[Check Point][0]\n" );
        if ( true != StopYSOSPlatform() )
        {
            printf( "CysosApp::OnHandExit[Fail][0]\n" );
        }
        printf( "CysosApp::OnHandExit[Check Point][1]\n" );
        is_run_ = false;
        printf( "CysosApp::OnHandExit[Check Point][2]\n" );
        printf( "CysosApp::OnHandExit[Check Point][3]\n" );
    }
    
    printf( "CysosApp::OnHandExit[Exit]\n" );
}
#pragma endregion
// CysosApp message handlers
