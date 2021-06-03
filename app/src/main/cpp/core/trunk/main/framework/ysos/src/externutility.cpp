/*
# externutility.cpp
# Definition of extern utilities
# Created on: 2017-09-23 14:38:55
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20170923, created by JinChengZhe
*/

/// Private Headers
#include "externutility.h"
/// Platform Headers
//#include <sys_daemon_package/externcommon.h>
/// Windows Platform Headers
#if _WIN32
  #include <Shlwapi.h>
  #pragma comment(lib, "Shlwapi.lib")
  #include <atlstr.h>
#else
  #include <sys/mman.h> /* for mmap and munmap */
  #include <sys/types.h> /* for open */
  #include <sys/stat.h> /* for open */
  #include <fcntl.h>     /* for open */
  #include <unistd.h>    /* for lseek and write */
  //#include <stdio.h>
  #include <string.h>
#endif

//using namespace ysos::extern_common;


namespace ysos {

    namespace extern_utility {
        /**
        *@brief FileLoader 具体类实现
        */
        #if _WIN32
        FileLoader::FileLoader() :
            file_handle_ ( INVALID_HANDLE_VALUE ),
            file_map_handle_ ( INVALID_HANDLE_VALUE ),
            file_data_ ( NULL ),
            file_path_ ( _T ( "" ) ),
            file_size_ ( 0 ),
            is_opened_ ( FALSE )
        {
            #ifdef _YSOS_UNICODE
            PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::FileLoader()" ) );
            #else
            PrintDSA ( "extern_utility", "FileLoader::FileLoader()" );
            #endif
        }

        FileLoader::~FileLoader ( void )
        {
            #ifdef _YSOS_UNICODE
            PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::~FileLoader()" ) );
            #else
            PrintDSA ( "extern_utility", "FileLoader::~FileLoader()" );
            #endif
        }

        #if _YSOS_UNICODE
        BOOL FileLoader::OpenFile ( LPCTSTR file_path, const DWORD file_size )
        {
            PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::OpenFile[Enter]" ) );
        #else
        BOOL FileLoader::OpenFile ( LPCSTR file_path, const DWORD file_size )
        {
            PrintDSA ( "extern_utility", "FileLoader::OpenFile[Enter]" );
        #endif
            BOOL result = FALSE;
            do
            {
                if ( NULL == file_path || 0 == _tcslen ( file_path ) )
                {
                    #ifdef _YSOS_UNICODE
                    PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::OpenFile[Fail][file_path is NULL or empty]" ) );
                    #else
                    PrintDSA ( "extern_utility", "FileLoader::OpenFile[Fail][file_path is NULL or empty]" );
                    #endif
                    break;
                }
                #ifdef _YSOS_UNICODE
                PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::OpenFile[file_path][%s]" ), file_path );
                #else
                PrintDSA ( "extern_utility", "FileLoader::OpenFile[file_path][%s]", file_path );
                #endif
                file_path_ = file_path;
                #ifdef _YSOS_UNICODE
                PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::OpenFile[Check Point][0]" ) );
                #else
                PrintDSA ( "extern_utility", _T ( "FileLoader::OpenFile[Check Point][0]" ) );
                #endif
                file_handle_ = INVALID_HANDLE_VALUE;
                file_map_handle_ = INVALID_HANDLE_VALUE;
                file_handle_ = CreateFile (
                                   file_path_.c_str(),
                                   GENERIC_READ | GENERIC_WRITE,
                                   0,
                                   NULL,
                                   OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL,
                                   NULL );
                if ( INVALID_HANDLE_VALUE == file_handle_ )
                {
                    #ifdef _YSOS_UNICODE
                    PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::OpenFile[Fail][CreateFile()][last_error_code][%d]" ), GetLastError() );
                    #else
                    PrintDSA ( "extern_utility", "FileLoader::OpenFile[Fail][CreateFile()][last_error_code][%d]", GetLastError() );
                    #endif
                    break;
                }
                #ifdef _YSOS_UNICODE
                PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::OpenFile[Check Point][1]" ) );
                #else
                PrintDSA ( "extern_utility", "FileLoader::OpenFile[Check Point][1]" );
                #endif
                if ( 0 == file_size )
                {
                    #ifdef _YSOS_UNICODE
                    PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::OpenFile[go on file_size_]" ) );
                    #else
                    PrintDSA ( "extern_utility", "FileLoader::OpenFile[go on file_size_]" );
                    #endif
                    file_size_ = GetFileSize ( file_handle_, NULL );
                    if ( 0 == file_size_ )
                    {
                        #ifdef _YSOS_UNICODE
                        PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::OpenFile[Fail][GetFileSize()][last_error_code][%d]" ), GetLastError() );
                        #else
                        PrintDSA ( "extern_utility", "FileLoader::OpenFile[Fail][GetFileSize()][last_error_code][%d]", GetLastError() );
                        #endif
                        break;
                    }
                }
                else
                {
                    #ifdef _YSOS_UNICODE
                    PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::OpenFile[resize file_size_]" ) );
                    #else
                    PrintDSA ( "extern_utility", "FileLoader::OpenFile[resize file_size_]" );
                    #endif
                    file_size_ = file_size;
                }
                #ifdef _YSOS_UNICODE
                PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::OpenFile[file_size_][%ld]" ), file_size_ );
                #else
                PrintDSA ( "extern_utility", "FileLoader::OpenFile[file_size_][%ld]", file_size_ );
                #endif
                file_map_handle_ = CreateFileMapping (
                                       file_handle_,
                                       NULL,
                                       PAGE_READWRITE,
                                       0,
                                       file_size_ + sizeof ( TCHAR ),
                                       NULL );
                if ( file_map_handle_ == NULL )
                {
                    #ifdef _YSOS_UNICODE
                    PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::OpenFile[Fail][CreateFileMapping()][last_error_code][%d]" ), GetLastError() );
                    #else
                    PrintDSA ( "extern_utility", "FileLoader::OpenFile[Fail][CreateFileMapping()][last_error_code][%d]", GetLastError() );
                    #endif
                    break;
                }
                #ifdef _YSOS_UNICODE
                PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::OpenFile[Check Point][2]" ) );
                #else
                PrintDSA ( "extern_utility", "FileLoader::OpenFile[Check Point][2]" );
                #endif
                file_data_ = MapViewOfFile ( file_map_handle_, FILE_MAP_WRITE, 0, 0, 0 );
                if ( file_data_ == NULL )
                {
                    #ifdef _YSOS_UNICODE
                    PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::OpenFile[Fail][MapViewOfFile()][last_error_code][%d]" ), GetLastError() );
                    #else
                    PrintDSA ( "extern_utility", "FileLoader::OpenFile[Fail][MapViewOfFile()][last_error_code][%d]", GetLastError() );
                    #endif
                    break;
                }
                #ifdef _YSOS_UNICODE
                PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::OpenFile[Check Point][3]" ) );
                #else
                PrintDSA ( "extern_utility", "FileLoader::OpenFile[Check Point][3]" );
                #endif
                result = TRUE;
            } while ( 0 );
            if ( TRUE != result )
            {
                #ifdef _YSOS_UNICODE
                PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::OpenFile[Fail]" ) );
                #else
                PrintDSA ( "extern_utility", "FileLoader::OpenFile[Fail]" );
                #endif
                is_opened_ = FALSE;
            }
            else
            {
                #ifdef _YSOS_UNICODE
                PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::OpenFile[Sucess]" ) );
                #else
                PrintDSA ( "extern_utility", "FileLoader::OpenFile[Sucess]" );
                #endif
                is_opened_ = TRUE;
            }
            #ifdef _YSOS_UNICODE
            PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::OpenFile[Exit]" ) );
            #else
            PrintDSA ( "extern_utility", "FileLoader::OpenFile[Exit]" );
            #endif
            return result;
        }

        BOOL FileLoader::CloseFile ( BOOL is_resized )
        {
            #ifdef _YSOS_UNICODE
            PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::CloseFile[Enter]" ) );
            #else
            PrintDSA ( "extern_utility", "FileLoader::CloseFile[Enter]" );
            #endif
            BOOL result = FALSE;
            do
            {
                if ( TRUE != is_opened_ )
                {
                    #ifdef _YSOS_UNICODE
                    PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::CloseFile[Fail][is_opened_ is not TRUE]" ) );
                    #else
                    PrintDSA ( "extern_utility", "FileLoader::CloseFile[Fail][is_opened_ is not TRUE]" );
                    #endif
                    break;
                }
                #ifdef _YSOS_UNICODE
                PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::CloseFile[Check Point][0]" ) );
                #else
                PrintDSA ( "extern_utility", "FileLoader::CloseFile[Check Point][0]" );
                #endif
                if ( TRUE != UnmapViewOfFile ( file_data_ ) )
                {
                    #ifdef _YSOS_UNICODE
                    PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::CloseFile[Fail][UnmapViewOfFile()][last_error_code][%d]" ), GetLastError() );
                    #else
                    PrintDSA ( "extern_utility", "FileLoader::CloseFile[Fail][UnmapViewOfFile()][last_error_code][%d]", GetLastError() );
                    #endif
                    break;
                }
                #ifdef _YSOS_UNICODE
                PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::CloseFile[Check Point][1]" ) );
                #else
                PrintDSA ( "extern_utility", "FileLoader::CloseFile[Check Point][1]" );
                #endif
                if ( TRUE != CloseHandle ( file_map_handle_ ) )
                {
                    #ifdef _YSOS_UNICODE
                    PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::CloseFile[Fail][CloseHandle()][last_error_code][%d]" ), GetLastError() );
                    #else
                    PrintDSA ( "extern_utility", "FileLoader::CloseFile[Fail][CloseHandle()][last_error_code][%d]", GetLastError() );
                    #endif
                    break;
                }
                #ifdef _YSOS_UNICODE
                PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::CloseFile[Check Point][2]" ) );
                #else
                PrintDSA ( "extern_utility", "FileLoader::CloseFile[Check Point][2]" );
                #endif
                if ( TRUE == is_resized )
                {
                    #ifdef _YSOS_UNICODE
                    PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::CloseFile[Check Point][is_resized is TRUE]" ) );
                    #else
                    PrintDSA ( "extern_utility", "FileLoader::CloseFile[Check Point][is_resized is TRUE]" );
                    #endif
                    if ( INVALID_SET_FILE_POINTER == SetFilePointer ( file_handle_, file_size_, NULL, FILE_BEGIN ) )
                    {
                        #ifdef _YSOS_UNICODE
                        PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::CloseFile[Fail][SetFilePointer()][last_error_code][%d]" ), GetLastError() );
                        #else
                        PrintDSA ( "extern_utility", "FileLoader::CloseFile[Fail][SetFilePointer()][last_error_code][%d]", GetLastError() );
                        #endif
                        break;
                    }
                    #ifdef _YSOS_UNICODE
                    PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::CloseFile[Check Point][3]" ) );
                    #else
                    PrintDSA ( "extern_utility", "FileLoader::CloseFile[Check Point][3]" );
                    #endif
                    if ( TRUE != SetEndOfFile ( file_handle_ ) )
                    {
                        #ifdef _YSOS_UNICODE
                        PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::CloseFile[Fail][SetEndOfFile()][last_error_code][%d]" ), GetLastError() );
                        #else
                        PrintDSA ( "extern_utility", "FileLoader::CloseFile[Fail][SetEndOfFile()][last_error_code][%d]", GetLastError() );
                        #endif
                        break;
                    }
                }
                #ifdef _YSOS_UNICODE
                PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::CloseFile[Check Point][4]" ) );
                #else
                PrintDSA ( "extern_utility", "FileLoader::CloseFile[Check Point][4]" );
                #endif
                if ( TRUE != CloseHandle ( file_handle_ ) )
                {
                    #ifdef _YSOS_UNICODE
                    PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::CloseFile[Fail][CloseHandle()][last_error_code][%d]" ), GetLastError() );
                    #else
                    PrintDSA ( "extern_utility", "FileLoader::CloseFile[Fail][CloseHandle()][last_error_code][%d]", GetLastError() );
                    #endif
                    break;
                }
                #ifdef _YSOS_UNICODE
                PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::CloseFile[Check Point][End]" ) );
                #else
                PrintDSA ( "extern_utility", "FileLoader::CloseFile[Check Point][End]" );
                #endif
                result = TRUE;
            } while ( 0 );
            #ifdef _YSOS_UNICODE
            PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::CloseFile[Exit]" ) );
            #else
            PrintDSA ( "extern_utility", "FileLoader::CloseFile[Exit]" );
            #endif
            return result;
        }

        BOOL FileLoader::ResizeFile ( const DWORD file_data_size )
        {
            #ifdef _YSOS_UNICODE
            PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::ResizeFile[Enter]" ) );
            #else
            PrintDSA ( "extern_utility", "FileLoader::ResizeFile[Enter]" );
            #endif
            BOOL result = FALSE;
            do
            {
                if ( 0 == file_data_size )
                {
                    #ifdef _YSOS_UNICODE
                    PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::ResizeFile[Fail][file_data_size is 0]" ) );
                    #else
                    PrintDSA ( "extern_utility", "FileLoader::ResizeFile[Fail][file_data_size is 0]" );
                    #endif
                    break;
                }
                #ifdef _YSOS_UNICODE
                PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::ResizeFile[Check Point][0]" ) );
                #else
                PrintDSA ( "extern_utility", "FileLoader::ResizeFile[Check Point][0]" );
                #endif
                if ( TRUE != is_opened_ )
                {
                    #ifdef _YSOS_UNICODE
                    PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::ResizeFile[Fail][is_opened_ is not TRUE]" ) );
                    #else
                    PrintDSA ( "extern_utility", "FileLoader::ResizeFile[Fail][is_opened_ is not TRUE]" );
                    #endif
                    break;
                }
                #ifdef _YSOS_UNICODE
                PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::ResizeFile[Check Point][1]" ) );
                #else
                PrintDSA ( "extern_utility", "FileLoader::ResizeFile[Check Point][1]" );
                #endif
                if ( TRUE != CloseFile() )
                {
                    #ifdef _YSOS_UNICODE
                    PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::ResizeFile[Fail][CloseFile()]" ) );
                    #else
                    PrintDSA ( _T ( "extern_utility" ), _T ( "FileLoader::ResizeFile[Fail][CloseFile()]" ) );
                    #endif
                    break;
                }
                #ifdef _YSOS_UNICODE
                PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::ResizeFile[Check Point][2]" ) );
                #else
                PrintDSA ( "extern_utility", "FileLoader::ResizeFile[Check Point][2]" );
                #endif
                if ( TRUE != OpenFile ( file_path_.c_str(), file_data_size ) )
                {
                    #ifdef _YSOS_UNICODE
                    PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::ResizeFile[Fail][OpenFile()]" ) );
                    #else
                    PrintDSA ( "extern_utility", "FileLoader::ResizeFile[Fail][OpenFile()]" );
                    #endif
                    break;
                }
                #ifdef _YSOS_UNICODE
                PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::ResizeFile[Check Point][End]" ) );
                #else
                PrintDSA ( "extern_utility", "FileLoader::ResizeFile[Check Point][End]" );
                #endif
                result = TRUE;
            } while ( 0 );
            #ifdef _YSOS_UNICODE
            PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::ResizeFile[Exit]" ) );
            #else
            PrintDSA ( "extern_utility", "FileLoader::ResizeFile[Exit]" );
            #endif
            return result;
        }

        void* FileLoader::GetFileData ( void )
        {
            #ifdef _YSOS_UNICODE
            PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::GetFileData[Enter]" ) );
            #else
            PrintDSA ( "extern_utility", "FileLoader::GetFileData[Enter]" );
            #endif
            void* result = NULL;
            do
            {
                if ( TRUE != is_opened_ )
                {
                    #ifdef _YSOS_UNICODE
                    PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::GetFileData[Fail][is_opened_ is not TRUE]" ) );
                    #else
                    PrintDSA ( "extern_utility", "FileLoader::GetFileData[Fail][is_opened_ is not TRUE]" );
                    #endif
                    break;
                }
                #ifdef _YSOS_UNICODE
                PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::GetFileData[Check Point][0]" ) );
                #else
                PrintDSA ( "extern_utility", "FileLoader::GetFileData[Check Point][0]" );
                #endif
                result = file_data_;
                #ifdef _YSOS_UNICODE
                PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::GetFileData[Check Point][End]" ) );
                #else
                PrintDSA ( "extern_utility", "FileLoader::GetFileData[Check Point][End]" );
                #endif
            } while ( 0 );
            #ifdef _YSOS_UNICODE
            PrintDS ( _T ( "extern_utility" ), _T ( "FileLoader::GetFileData[Exit]" ) );
            #else
            PrintDSA ( "extern_utility", "FileLoader::GetFileData[Exit]" );
            #endif
            return result;
        }
        #else
          FileLoader::FileLoader() :
            file_handle_ ( -1 ),
            //file_map_handle_ ( INVALID_HANDLE_VALUE ),
            file_data_ ( NULL ),
            file_path_ ( "" ),
            file_size_ ( 0 ),
            is_opened_ ( FALSE )
        {
            //TODO:
            #ifdef _YSOS_UNICODE
            printf( "FileLoader::FileLoader()\n" );
            #else
            printf( "extern_utility::::FileLoader::FileLoader()\n" );
            #endif
        }

        FileLoader::~FileLoader ( void )
        {
            //TODO:
            #ifdef _YSOS_UNICODE
            printf( "FileLoader::~FileLoader()\n" );
            #else
            printf( "extern_utility::FileLoader::~FileLoader()\n" );
            #endif
        }

        #if _YSOS_UNICODE
        BOOL FileLoader::OpenFile ( LPCTSTR file_path, const DWORD file_size )
        {
            printf( "FileLoader::OpenFile[Enter]\n" );
        #else
        BOOL FileLoader::OpenFile ( LPCSTR file_path, const DWORD file_size )
        {
            printf( "extern_utility::--::FileLoader::OpenFile[Enter]\n" );
        #endif
            BOOL result = FALSE;
            do
            {
                if ( NULL == file_path || 0 == strlen ( file_path ) )
                {
                    //TODO:
                    #ifdef _YSOS_UNICODE
                    printf( "FileLoader::OpenFile[Fail][file_path is NULL or empty]\n" );
                    #else
                    printf( "extern_utility::FileLoader::OpenFile[Fail][file_path is NULL or empty]\n" );
                    #endif
                    break;
                }
                //TODO:
                #ifdef _YSOS_UNICODE
                printf( "FileLoader::OpenFile[file_path][%s]\n", file_path );
                #else
                printf( "extern_utility::FileLoader::OpenFile[file_path][%s]\n", file_path );
                #endif
                file_path_ = file_path;
                #ifdef _YSOS_UNICODE
                printf( "FileLoader::OpenFile[Check Point][0]\n" );
                #else
                printf( "extern_utility::FileLoader::OpenFile[Check Point][0]\n" );
                #endif
                //TODO: //add for linux
                //file_handle_ = INVALID_HANDLE_VALUE;
                file_handle_ = open(file_path_.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
                if ( -1 == file_handle_ )
                {
                    //TODO:
                    #ifdef _YSOS_UNICODE
                    printf( "FileLoader::OpenFile[Fail][CreateFile()][last_error_code][%s]\n" , "GetLastError()" );
                    #else
                    printf("extern_utility::FileLoader::OpenFile[Fail][CreateFile()][last_error_code][%s]\n", "GetLastError()" );
                    #endif
                    break;
                }
                //TODO: 
                #ifdef _YSOS_UNICODE
                printf( "FileLoader::OpenFile[Check Point][1]\n" );
                #else
                printf( "extern_utility::FileLoader::OpenFile[Check Point][1]\n" );
                #endif
                //add for linux
                write(file_handle_, "/0", 1); /* 在文件最后添加一个空字符，以便下面printf正常工作 */ //add for linux
                lseek(file_handle_, 0, SEEK_SET); //add for linux
                if ( 0 == file_size )
                {
                    //TODO:
                    #ifdef _YSOS_UNICODE
                    printf( "FileLoader::OpenFile[go on file_size_]\n" );
                    #else
                    printf( "extern_utility::FileLoader::OpenFile[go on file_size_]\n" );
                    #endif
                    file_size_ = lseek(file_handle_, 1, SEEK_END);//add for linux
                    if ( 0 == file_size_ )
                    {
                        //TODO:
                        #ifdef _YSOS_UNICODE
                        printf( "FileLoader::OpenFile[Fail][GetFileSize()][last_error_code][%s]\n" ), "GetLastError()" );
                        #else
                        printf( "extern_utility::FileLoader::OpenFile[Fail][GetFileSize()][last_error_code][%s]\n", "GetLastError()" );
                        #endif
                        break;
                    }
                    printf( "extern_utility::FileLoader::OpenFile[Fail][GetFileSize()][lseek][file_size_][%ld]\n", file_size_ );
                }
                else
                {
                    //TODO:
                    #ifdef _YSOS_UNICODE
                    printf( "FileLoader::OpenFile[resize file_size_]\n" ) );
                    #else
                    printf( "extern_utility::FileLoader::OpenFile[resize file_size_][%ld]\n", file_size );
                    #endif
                    file_size_ = file_size;
                }
                //TODO: 
                #ifdef _YSOS_UNICODE
                printf( "FileLoader::OpenFile[file_size_][%ld]\n", file_size_ );
                #else
                printf( "extern_utility::FileLoader::OpenFile[file_size_][%ld]\n", file_size_ );
                #endif
                //add fro linux
                //TODO:
                struct stat buf = {0};
                if (fstat(file_handle_, &buf) < 0)
                {
                    printf("extern_utility::FileLoader::OpenFile[Fail][fstat()][get file state error:][%d]\n", errno);
                    close(file_handle_);
                    break;
                }
                printf("extern_utility::FileLoader::OpenFile[mmap()][begin][file_data_][%p]\n", file_data_);
                file_data_ =(char *) mmap(NULL, /*(file_size_ + sizeof(char))*/buf.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, file_handle_, 0);//add fro linux
                //* valid address is never NULL since there's no MAP_FIXED */
                if (file_data_ == MAP_FAILED)
                {
                    printf("mmap failed\n");
                    close(file_handle_);
                    return -1;
                }
                printf("extern_utility::FileLoader::OpenFile[mmap()][begin][file_data_][%p]\n", file_data_);
                printf("extern_utility::FileLoader::OpenFile[mmap()][buf.st_size][%ld]\n", buf.st_size);
                file_size_ = buf.st_size;
                printf("extern_utility::FileLoader::OpenFile[mmap()][file_size_][%ld]\n", file_size_);
                //TODO:
                #ifdef _YSOS_UNICODE
                printf( "FileLoader::OpenFile[Check Point][2]\n" );
                #else
                printf( "extern_utility::FileLoader::OpenFile[Check Point][2]\n" );
                #endif
                if ( file_data_ == NULL )
                {
                    //TODO:
                    #ifdef _YSOS_UNICODE
                    printf( "FileLoader::OpenFile[Fail][mmap()][file_data_][is null]\n" );
                    #else
                    printf( "extern_utility::FileLoader::OpenFile[Fail][mmap()][file_data_][is null]\n" );
                    #endif
                    break;
                }
                //TODO:
                #ifdef _YSOS_UNICODE
                printf( "FileLoader::OpenFile[Check Point][3]\n" );
                #else
                printf( "extern_utility::FileLoader::OpenFile[Check Point][3]\n" );
                #endif
                result = TRUE;
            } while ( 0 );
            if ( TRUE != result )
            {
                //TODO:
                #ifdef _YSOS_UNICODE
                printf( "FileLoader::OpenFile[Fail]\n" );
                #else
                printf( "extern_utility::FileLoader::OpenFile[Fail]\n" );
                #endif
                is_opened_ = FALSE;
            }
            else
            {
                //TODO:
                #ifdef _YSOS_UNICODE
                printf( "FileLoader::OpenFile[Sucess]\n" );
                #else
                printf( "extern_utility::FileLoader::OpenFile[Sucess]\n" );
                #endif
                is_opened_ = TRUE;
            }
            //TODO:
            #ifdef _YSOS_UNICODE
            printf( "FileLoader::OpenFile[Exit]\n" );
            #else
            printf( "extern_utility::FileLoader::OpenFile[Exit]\n" );
            #endif
            return result;
        }

        BOOL FileLoader::CloseFile ( BOOL is_resized )
        {
           //TODO:
            #ifdef _YSOS_UNICODE
            printf( "FileLoader::CloseFile[Enter]\n" );
            #else
            printf( "extern_utility::FileLoader::CloseFile[Enter]\n" );
            #endif
            BOOL result = FALSE;
            do
            {
                if ( TRUE != is_opened_ )
                {
                    //TODO:
                    #ifdef _YSOS_UNICODE
                    printf( "FileLoader::CloseFile[Fail][is_opened_ is not TRUE]\n" );
                    #else
                    printf( "extern_utility::FileLoader::CloseFile[Fail][is_opened_ is not TRUE]\n" );
                    #endif
                    break;
                }
                //TODO:
                #ifdef _YSOS_UNICODE
                printf( "FileLoader::CloseFile[Check Point][0]\n" );
                #else
                printf( "extern_utility::FileLoader::CloseFile[Check Point][0]\n" );
                #endif
                munmap(file_data_, file_size_);
                //TODO:
                #ifdef _YSOS_UNICODE
                printf( "FileLoader::CloseFile[Check Point][4]\n" );
                #else
                printf( "extern_utility::FileLoader::CloseFile[Check Point][4]\n" );
                #endif
                if ( -1 == close ( file_handle_ ) )
                {
                    #ifdef _YSOS_UNICODE
                    printf( "FileLoader::CloseFile[Fail][CloseHandle()][last_error_code][%s]\n" ), "GetLastError()" );
                    #else
                    printf( "extern_utility::FileLoader::CloseFile[Fail][CloseHandle()][last_error_code][%s]\n", "GetLastError()" );
                    #endif
                    break;
                }
                //TODO:
                #ifdef _YSOS_UNICODE
                printf( "FileLoader::CloseFile[Check Point][End]\n" );
                #else
                printf( "extern_utility::FileLoader::CloseFile[Check Point][End]\n" );
                #endif
                result = TRUE;
            } while ( 0 );
            //TODO:
            #ifdef _YSOS_UNICODE
            printf( "FileLoader::CloseFile[Exit]\n" );
            #else
            printf( "extern_utility::FileLoader::CloseFile[Exit]\n" );
            #endif
            return result;
        }

        BOOL FileLoader::ResizeFile ( const DWORD file_data_size )
        {
            //TODO:
            #ifdef _YSOS_UNICODE
            printf( "FileLoader::ResizeFile[Enter]\n" );
            #else
            printf( "extern_utility::FileLoader::ResizeFile[Enter]\n" );
            #endif
            BOOL result = FALSE;
            do
            {
                if ( 0 == file_data_size )
                {
                    //TODO:
                    #ifdef _YSOS_UNICODE
                    printf( "FileLoader::ResizeFile[Fail][file_data_size is 0]\n" );
                    #else
                    printf( "extern_utility::FileLoader::ResizeFile[Fail][file_data_size is 0]\n" );
                    #endif
                    break;
                }
                //TODO:
                #ifdef _YSOS_UNICODE
                printf( "FileLoader::ResizeFile[Check Point][0]\n" );
                #else
                printf( "extern_utility::FileLoader::ResizeFile[Check Point][0]\n" );
                #endif
                if ( TRUE != is_opened_ )
                {
                    //TODO:
                    #ifdef _YSOS_UNICODE
                    printf( "FileLoader::ResizeFile[Fail][is_opened_ is not TRUE]\n" );
                    #else
                    printf( "extern_utility::FileLoader::ResizeFile[Fail][is_opened_ is not TRUE]\n" );
                    #endif
                    break;
                }
                //TODO:
                #ifdef _YSOS_UNICODE
                printf( "FileLoader::ResizeFile[Check Point][1]\n" );
                #else
                printf( "extern_utility::FileLoader::ResizeFile[Check Point][1]\n" );
                #endif
                if ( TRUE != CloseFile() )
                {
                    //TODO:
                    #ifdef _YSOS_UNICODE
                    printf( "FileLoader::ResizeFile[Fail][CloseFile()]\n" );
                    #else
                    printf( "FileLoader::ResizeFile[Fail][CloseFile()]\n" );
                    #endif
                    break;
                }
                //TODO:
                #ifdef _YSOS_UNICODE
                printf( "FileLoader::ResizeFile[Check Point][2]\n" );
                #else
                printf( "extern_utility::FileLoader::ResizeFile[Check Point][2]\n" );
                #endif
                if ( TRUE != OpenFile ( file_path_.c_str(), file_data_size ) )
                {
                    //TODO:
                    #ifdef _YSOS_UNICODE
                    printf( "FileLoader::ResizeFile[Fail][OpenFile()]\n" );
                    #else
                    printf( "extern_utility::FileLoader::ResizeFile[Fail][OpenFile()]\n" );
                    #endif
                    break;
                }
                //TODO:
                #ifdef _YSOS_UNICODE
                printf( "FileLoader::ResizeFile[Check Point][End]\n" );
                #else
                printf( "extern_utility::FileLoader::ResizeFile[Check Point][End]\n" );
                #endif
                result = TRUE;
            } while ( 0 );
            //TODO:
            #ifdef _YSOS_UNICODE
            printf( "FileLoader::ResizeFile[Exit]\n" );
            #else
            printf( "extern_utility::FileLoader::ResizeFile[Exit]\n" );
            #endif
            return result;
        }

        char* FileLoader::GetFileData ( void )
        {
            //TODO:
            #ifdef _YSOS_UNICODE
            printf( "FileLoader::GetFileData[Enter]\n" );
            #else
            printf( "extern_utility::FileLoader::GetFileData[Enter]\n" );
            #endif
            char* result = NULL;
            do
            {
                if ( TRUE != is_opened_ )
                {
                    //TODO:
                    #ifdef _YSOS_UNICODE
                    printf( "FileLoader::GetFileData[Fail][is_opened_ is not TRUE]\n" );
                    #else
                    printf( "extern_utility::FileLoader::GetFileData[Fail][is_opened_ is not TRUE]\n" );
                    #endif
                    break;
                }
                //TODO:
                #ifdef _YSOS_UNICODE
                printf( "FileLoader::GetFileData[Check Point][0]\n" );
                #else
                printf( "extern_utility::FileLoader::GetFileData[Check Point][0]\n" );
                #endif
                printf( "extern_utility::FileLoader::GetFileData[Check Point][begin][result][%p]\n", result );
                result = file_data_;
                printf( "extern_utility::FileLoader::GetFileData[Check Point][enddd][file_data_][%p]\n", file_data_ );
                printf( "extern_utility::FileLoader::GetFileData[Check Point][enddd][result][%p]\n", result );
                //TODO:
                #ifdef _YSOS_UNICODE
                printf( "FileLoader::GetFileData[Check Point][End]\n" );
                #else
                printf( "extern_utility::FileLoader::GetFileData[Check Point][End]\n" );
                #endif
            } while ( 0 );
            //TODO:
            #ifdef _YSOS_UNICODE
            printf( "FileLoader::GetFileData[Exit]\n" );
            #else
            printf( "extern_utility::FileLoader::GetFileData[Exit]\n" );
            #endif
            return result;
        }
        #endif
    }
}
