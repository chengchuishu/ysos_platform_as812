/*
# winsvcwrapper.cpp
# Definition of winsvcwrapper
# Created on: 2017-03-14 10:43:49
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20170314, created by JinChengZhe
*/

/*************************************** add no used for linux  ************************************
/// Private Headers
#include "../../../protect/include/sys_daemon_package/winsvcwrapper.h"


namespace ysos {

    namespace win_svc_wrapper {

        YSOS_IMPLIMENT_SINGLETON ( WinSvcWrapper )
        WinSvcWrapper::WinSvcWrapper()
        {
        }

        WinSvcWrapper::~WinSvcWrapper()
        {
            if ( NULL != service_stop_event_handle_ )
            {
                CloseHandle ( service_stop_event_handle_ );
                service_stop_event_handle_ = NULL;
            }
        }

        BOOL WinSvcWrapper::Install ( LPCTSTR service_name_ptr, LPCTSTR display_name_ptr, LPCTSTR description_ptr, LPCTSTR module_file_path_ptr )
        {
            if (
                NULL == service_name_ptr ||
                0 == _tcslen ( service_name_ptr ) ||
                NULL == display_name_ptr ||
                0 == _tcslen ( display_name_ptr ) ||
                NULL == description_ptr ||
                0 == _tcslen ( description_ptr ) ||
                NULL == module_file_path_ptr ||
                0 == _tcslen ( module_file_path_ptr )
            )
            {
                return FALSE;
            }
            wprintf ( L"service_name_ptr[%s]\n", service_name_ptr );
            wprintf ( L"display_name_ptr[%s]\n", display_name_ptr );
            wprintf ( L"description_ptr[%s]\n", description_ptr );
            wprintf ( L"module_file_path_ptr[%s]\n", module_file_path_ptr );
            /// Get a handle to the SCM database
            SC_HANDLE sc_manager_handle = OpenSCManager (
                                              NULL,                           ///< local computer
                                              NULL,                           ///< ServiceActive database
                                              SC_MANAGER_ALL_ACCESS );        ///< full access rights
            if ( NULL == sc_manager_handle )
            {
                wprintf ( L"The function[OpenSCManager] fails.[%d]\n", GetLastError() );
                return FALSE;
            }
            /// Create the service
            SC_HANDLE service_handle = CreateService (
                                           sc_manager_handle,                 ///< SCM database
                                           service_name_ptr,                  ///< name of service
                                           display_name_ptr,                  ///< service name to display
                                           SERVICE_ALL_ACCESS,                ///< desired access
                                           SERVICE_WIN32_OWN_PROCESS,         ///< service type
                                           SERVICE_AUTO_START,                ///< start type
                                           SERVICE_ERROR_NORMAL,              ///< error control type
                                           module_file_path_ptr,              ///< path to service's binary
                                           NULL,                              ///< no load ordering group
                                           NULL,                              ///< no tag identifier
                                           NULL,                              ///< no dependencies
                                           NULL,                              ///< LocalSystem account
                                           NULL );                            ///< no password
            if ( NULL == service_handle )
            {
                wprintf ( L"The function[CreateService] fails.[%d]\n", GetLastError() );
                if ( !CloseServiceHandle ( sc_manager_handle ) )
                {
                    wprintf ( L"The function[CloseServiceHandle] fails.[%d]2\n", GetLastError() );
                }
                return FALSE;
            }
            SERVICE_DESCRIPTION service_description = {0,};
            service_description.lpDescription = const_cast<LPTSTR> ( description_ptr );
            if ( !ChangeServiceConfig2 ( service_handle, SERVICE_CONFIG_DESCRIPTION, &service_description ) )
            {
                wprintf ( L"The function[ChangeServiceConfig2] fails.[%d]\n", GetLastError() );
                if ( !CloseServiceHandle ( service_handle ) )
                {
                    wprintf ( L"The function[CloseServiceHandle] fails.[%d]1\n", GetLastError() );
                }
                if ( !CloseServiceHandle ( sc_manager_handle ) )
                {
                    wprintf ( L"The function[CloseServiceHandle] fails.[%d]2\n", GetLastError() );
                }
                return FALSE;
            }
            wprintf ( L"Service installed successfully\n" );
            if ( !CloseServiceHandle ( service_handle ) )
            {
                wprintf ( L"The function[CloseServiceHandle] fails.[%d]1\n", GetLastError() );
            }
            if ( !CloseServiceHandle ( sc_manager_handle ) )
            {
                wprintf ( L"The function[CloseServiceHandle] fails.[%d]2\n", GetLastError() );
            }
            return TRUE;
        }

        BOOL WinSvcWrapper::Uninstall ( LPCTSTR service_name_ptr )
        {
            if (
                NULL == service_name_ptr ||
                0 == _tcslen ( service_name_ptr )
            )
            {
                return FALSE;
            }
            if ( !Stop ( service_name_ptr ) )
            {
                wprintf ( L"The function[Stop] fails.\n" );
                return FALSE;
            }
            /// Get a handle to the SCM database
            SC_HANDLE sc_manager_handle = OpenSCManager (
                                              NULL,                          ///< local computer
                                              NULL,                          ///< ServiceActive database
                                              SC_MANAGER_ALL_ACCESS );       ///< full access rights
            if ( NULL == sc_manager_handle )
            {
                wprintf ( L"The function[OpenSCManager] fails.[%d]\n", GetLastError() );
                return FALSE;
            }
            //Open the service
            SC_HANDLE service_handle = OpenService (
                                           sc_manager_handle,
                                           service_name_ptr,
                                           DELETE );
            if ( NULL == service_handle )
            {
                wprintf ( L"The function[OpenService] fails.[%d]\n", GetLastError() );
                if ( !CloseServiceHandle ( sc_manager_handle ) )
                {
                    wprintf ( L"The function[CloseServiceHandle] fails.[%d]\n", GetLastError() );
                }
                return FALSE;
            }
            /// Delete the Service.
            if ( !DeleteService ( service_handle ) )
            {
                wprintf ( L"DeleteService failed (%d)\n", GetLastError() );
            }
            else
            {
                wprintf ( L"Service deleted successfully\n" );
            }
            if ( !CloseServiceHandle ( service_handle ) )
            {
                wprintf ( L"The function[CloseServiceHandle] fails.[%d]1\n", GetLastError() );
            }
            if ( !CloseServiceHandle ( sc_manager_handle ) )
            {
                wprintf ( L"The function[CloseServiceHandle] fails.[%d]2\n", GetLastError() );
            }
            return TRUE;
        }

        BOOL WinSvcWrapper::Start ( LPCTSTR service_name_ptr )
        {
            BOOL result = FALSE;
            /// Get a handle to the SCM database
            SC_HANDLE sc_manager_handle = OpenSCManager (
                                              NULL,                         ///< local computer
                                              NULL,                         ///< ServiceActive database
                                              SC_MANAGER_ALL_ACCESS );      ///< full access rights
            if ( NULL == sc_manager_handle )
            {
                wprintf ( L"The function[OpenSCManager] fails.[%d]\n", GetLastError() );
                return result;
            }
            //Open the service
            SC_HANDLE service_handle = OpenService (
                                           sc_manager_handle,
                                           service_name_ptr,
                                           SERVICE_ALL_ACCESS );
            if ( NULL == service_handle )
            {
                wprintf ( L"The function[OpenService] fails.[%d]\n", GetLastError() );
                if ( !CloseServiceHandle ( sc_manager_handle ) )
                {
                    wprintf ( L"The function[CloseServiceHandle] fails.[%d]\n", GetLastError() );
                }
                if ( ERROR_SERVICE_DOES_NOT_EXIST == GetLastError() )
                {
                    wprintf ( L"The specified service does not exist." );
                    return TRUE;
                }
                return result;
            }
            /// Check the status in case the service is not stopped.
            SERVICE_STATUS_PROCESS service_status_process = {0,};
            DWORD bytes_needed = 0;
            if ( !QueryServiceStatusEx (
                        service_handle,
                        SC_STATUS_PROCESS_INFO,
                        ( LPBYTE ) &service_status_process,
                        sizeof ( SERVICE_STATUS_PROCESS ),
                        &bytes_needed )
               )
            {
                wprintf ( L"The function[QueryServiceStatusEx] fails.[%d]\n", GetLastError() );
                goto START_CLEANUP;
            }
            /// Check if the service is already running. It would be possible
            /// to stop the service here, but for simplicity this example just returns.
            if (
                service_status_process.dwCurrentState != SERVICE_STOPPED &&
                service_status_process.dwCurrentState != SERVICE_STOP_PENDING
            )
            {
                wprintf ( L"Cannot start the service because it is already running\n" );
                result = TRUE;
                goto START_CLEANUP;
            }
            /// Save the tick count and initial checkpoint.
            DWORD start_time = GetTickCount();
            DWORD old_check_point = service_status_process.dwCheckPoint;
            DWORD wait_time;
            DWORD start_tick_count;
            /// Wait for the service to stop Before attempting to start it.
            while ( service_status_process.dwCurrentState == SERVICE_STOP_PENDING )
            {
                /// Do not wait longer than the wait hint. A good interval is
                /// one-tenth of the wait hint but not less than 1 second
                /// and not more than 10 seconds.
                wait_time = service_status_process.dwWaitHint / 10;
                if ( wait_time < 1000 )
                {
                    wait_time = 1000;
                }
                else if ( wait_time > 10000 )
                {
                    wait_time = 10000;
                }
                Sleep ( wait_time );
                /// Check the status until the service is no longer stop pending.
                if ( !QueryServiceStatusEx (
                            service_handle,                           ///< handle to service
                            SC_STATUS_PROCESS_INFO,                   ///< information level
                            ( LPBYTE ) &service_status_process,       ///< address of structure
                            sizeof ( SERVICE_STATUS_PROCESS ),        ///< size of structure
                            &bytes_needed ) )                         ///< size needed if buffer is too small
                {
                    wprintf ( L"QueryServiceStatusEx failed (%d)\n", GetLastError() );
                    goto START_CLEANUP;
                }
                if ( service_status_process.dwCheckPoint > old_check_point )
                {
                    /// Continue to wait and check.
                    start_tick_count = GetTickCount();
                    old_check_point = service_status_process.dwCheckPoint;
                }
                else
                {
                    if ( GetTickCount() - start_tick_count > service_status_process.dwWaitHint )
                    {
                        wprintf ( L"Timeout waiting for service to stop\n" );
                        goto START_CLEANUP;
                    }
                }
            }
            /// Attempt to start the service.
            if ( !StartService (
                        service_handle,                         ///< handle to service
                        0,                                      ///< number of arguments
                        NULL ) )                                ///< no arguments
            {
                wprintf ( L"StartService failed (%d)\n", GetLastError() );
                goto START_CLEANUP;
            }
            else
            {
                printf ( "Service start pending...\n" );
            }
            /// Check the status until the service is no longer start pending.
            if ( !QueryServiceStatusEx (
                        service_handle,                         ///< handle to service
                        SC_STATUS_PROCESS_INFO,                 ///< info level
                        ( LPBYTE ) &service_status_process,     ///< address of structure
                        sizeof ( SERVICE_STATUS_PROCESS ),      ///< size of structure
                        &bytes_needed ) )                       ///< if buffer too small
            {
                wprintf ( L"QueryServiceStatusEx failed (%d)\n", GetLastError() );
                goto START_CLEANUP;
            }
            /// Save the tick count and initial checkpoint.
            start_tick_count = GetTickCount();
            old_check_point = service_status_process.dwCheckPoint;
            while ( service_status_process.dwCurrentState == SERVICE_START_PENDING )
            {
                /// Do not wait longer than the wait hint. A good interval is
                /// one-tenth the wait hint, but no less than 1 second and no
                /// more than 10 seconds.
                wait_time = service_status_process.dwWaitHint / 10;
                if ( wait_time < 1000 )
                {
                    wait_time = 1000;
                }
                else if ( wait_time > 10000 )
                {
                    wait_time = 10000;
                }
                Sleep ( wait_time );
                /// Check the status again.
                if ( !QueryServiceStatusEx (
                            service_handle,                           ///< handle to service
                            SC_STATUS_PROCESS_INFO,                   ///< info level
                            ( LPBYTE ) &service_status_process,       ///< address of structure
                            sizeof ( SERVICE_STATUS_PROCESS ),        ///< size of structure
                            &bytes_needed ) )                         ///< if buffer too small
                {
                    wprintf ( L"QueryServiceStatusEx failed (%d)\n", GetLastError() );
                    break;
                }
                if ( service_status_process.dwCheckPoint > old_check_point )
                {
                    /// Continue to wait and check.
                    start_tick_count = GetTickCount();
                    old_check_point = service_status_process.dwCheckPoint;
                }
                else
                {
                    if ( GetTickCount() - start_tick_count > service_status_process.dwWaitHint )
                    {
                        /// No progress made within the wait hint.
                        break;
                    }
                }
            }
            /// Determine whether the service is running.
            if ( service_status_process.dwCurrentState == SERVICE_RUNNING )
            {
                result = TRUE;
                wprintf ( L"Service started successfully.\n" );
            }
            else
            {
                wprintf ( L"Service not started. \n" );
                wprintf ( L"  Current State: %d\n", service_status_process.dwCurrentState );
                wprintf ( L"  Exit Code: %d\n", service_status_process.dwWin32ExitCode );
                wprintf ( L"  Check Point: %d\n", service_status_process.dwCheckPoint );
                wprintf ( L"  Wait Hint: %d\n", service_status_process.dwWaitHint );
            }
        START_CLEANUP:
            if ( !CloseServiceHandle ( service_handle ) )
            {
                wprintf ( L"The function[CloseServiceHandle] fails.[%d]1\n", GetLastError() );
            }
            if ( !CloseServiceHandle ( sc_manager_handle ) )
            {
                wprintf ( L"The function[CloseServiceHandle] fails.[%d]2\n", GetLastError() );
            }
            return result;
        }

        BOOL WinSvcWrapper::Stop ( LPCTSTR service_name_ptr )
        {
            BOOL result = FALSE;
            /// Get a handle to the SCM database
            SC_HANDLE sc_manager_handle = OpenSCManager (
                                              NULL,                          ///< local computer
                                              NULL,                          ///< ServiceActive database
                                              SC_MANAGER_ALL_ACCESS );       ///< full access rights
            if ( NULL == sc_manager_handle )
            {
                wprintf ( L"The function[OpenSCManager] fails.[%d]\n", GetLastError() );
                return result;
            }
            //Open the service
            SC_HANDLE service_handle = OpenService (
                                           sc_manager_handle,
                                           service_name_ptr,
                                           SERVICE_STOP | SERVICE_QUERY_STATUS | SERVICE_ENUMERATE_DEPENDENTS );
            if ( NULL == service_handle )
            {
                wprintf ( L"The function[OpenService] fails.[%d]\n", GetLastError() );
                if ( !CloseServiceHandle ( sc_manager_handle ) )
                {
                    wprintf ( L"The function[CloseServiceHandle] fails.[%d]\n", GetLastError() );
                }
                if ( ERROR_SERVICE_DOES_NOT_EXIST == GetLastError() )
                {
                    wprintf ( L"The specified service does not exist." );
                    return TRUE;
                }
                return result;
            }
            /// Make sure the service is not already stopped.
            SERVICE_STATUS_PROCESS service_status_process = {0,};
            DWORD bytes_needed = 0;
            DWORD start_time = GetTickCount();
            DWORD timeout = 30000; /// 30-second time-out
            DWORD wait_time;
            if ( !QueryServiceStatusEx (
                        service_handle,
                        SC_STATUS_PROCESS_INFO,
                        ( LPBYTE ) &service_status_process,
                        sizeof ( SERVICE_STATUS_PROCESS ),
                        &bytes_needed )
               )
            {
                wprintf ( L"The function[QueryServiceStatusEx] fails.[%d]\n", GetLastError() );
                goto STOP_CLEANUP;
            }
            if ( service_status_process.dwCurrentState == SERVICE_STOPPED )
            {
                wprintf ( L"Service is already stopped.\n" );
                result = TRUE;
                goto STOP_CLEANUP;
            }
            /// If a stop is pending, wait for it.
            while ( service_status_process.dwCurrentState == SERVICE_STOP_PENDING )
            {
                wprintf ( L"Service stop pending...\n" );
                /// Do not wait longer than the wait hint. A good interval is
                /// one-tenth of the wait hint but not less than 1 second
                /// and not more than 10 seconds.
                wait_time = service_status_process.dwWaitHint / 10;
                if ( wait_time < 1000 )
                {
                    wait_time = 1000;
                }
                else if ( wait_time > 10000 )
                {
                    wait_time = 10000;
                }
                Sleep ( wait_time );
                if ( !QueryServiceStatusEx (
                            service_handle,
                            SC_STATUS_PROCESS_INFO,
                            ( LPBYTE ) &service_status_process,
                            sizeof ( SERVICE_STATUS_PROCESS ),
                            &bytes_needed ) )
                {
                    wprintf ( L"QueryServiceStatusEx failed (%d)\n", GetLastError() );
                    goto STOP_CLEANUP;
                }
                if ( service_status_process.dwCurrentState == SERVICE_STOPPED )
                {
                    wprintf ( L"Service stopped successfully.\n" );
                    result = TRUE;
                    goto STOP_CLEANUP;
                }
                if ( GetTickCount() - start_time > timeout )
                {
                    wprintf ( L"Service stop timed out.\n" );
                    goto STOP_CLEANUP;
                }
            }
            /// If the service is running, dependencies must be stopped first.
            //StopDependentServices();
            /// Send a stop code to the service.
            if ( !ControlService (
                        service_handle,
                        SERVICE_CONTROL_STOP,
                        ( LPSERVICE_STATUS ) &service_status_process )
               )
            {
                wprintf ( L"ControlService failed (%d)\n", GetLastError() );
                goto STOP_CLEANUP;
            }
            /// Wait for the service to stop.
            while ( service_status_process.dwCurrentState != SERVICE_STOPPED )
            {
                Sleep ( service_status_process.dwWaitHint );
                if ( !QueryServiceStatusEx (
                            service_handle,
                            SC_STATUS_PROCESS_INFO,
                            ( LPBYTE ) &service_status_process,
                            sizeof ( SERVICE_STATUS_PROCESS ),
                            &bytes_needed )
                   )
                {
                    wprintf ( L"QueryServiceStatusEx failed (%d)\n", GetLastError() );
                    goto STOP_CLEANUP;
                }
                if ( service_status_process.dwCurrentState == SERVICE_STOPPED )
                {
                    result = TRUE;
                    break;
                }
                if ( GetTickCount() - start_time > timeout )
                {
                    printf ( "Wait timed out\n" );
                    goto STOP_CLEANUP;
                }
            }
            wprintf ( L"Service stopped successfully\n" );
        STOP_CLEANUP:
            if ( !CloseServiceHandle ( service_handle ) )
            {
                wprintf ( L"The function[CloseServiceHandle] fails.[%d]1\n", GetLastError() );
            }
            if ( !CloseServiceHandle ( sc_manager_handle ) )
            {
                wprintf ( L"The function[CloseServiceHandle] fails.[%d]2\n", GetLastError() );
            }
            return result;
        }

        VOID WinSvcWrapper::PrintUsage2Console()
        {
            wprintf ( L"Description:\n" );
            wprintf ( L"\tCommand-line tool that controls a service.\n\n" );
            wprintf ( L"Usage:\n" );
            wprintf ( L"\tsvccontrol [command] [service_name]\n\n" );
            wprintf ( L"\t[command]\n" );
            wprintf ( L"\t  install/-install/-i\n" );
            wprintf ( L"\t  uninstall/-uninstall/-u\n" );
            wprintf ( L"\t  start/-start/-s\n" );
            wprintf ( L"\t  stop/-stop/-t\n" );
            return;
        }

        VOID WINAPI WinSvcWrapper::SvcMain ( DWORD argc, LPTSTR *argv_ptr )
        {
            if ( the_instance_ptr_ )
            {
                the_instance_ptr_->RealSvcMain ( argc, argv_ptr );
            }
            return;
        }

        VOID WINAPI WinSvcWrapper::SvcCtrlHandler ( DWORD ctrol_code )
        {
            if ( the_instance_ptr_ )
            {
                the_instance_ptr_->RealSvcCtrlHandler ( ctrol_code );
            }
            return;
        }

        VOID WINAPI WinSvcWrapper::ThreadProc ( PVOID param_ptr )
        {
            if ( NULL == the_instance_ptr_ )
            {
                return;
            }
            while ( TRUE != the_instance_ptr_->shutdown_flag_ )
            {
                the_instance_ptr_->do_job_callback_func_ptr_ ( the_instance_ptr_->shutdown_flag_ );
                Sleep ( the_instance_ptr_->perform_work_interval_time_ );
            }
            #ifdef _YSOS_UNICODE
            PrintDS ( _T ( "CYSOSWinSvc" ), _T ( "get out the thread..." ) );
            #else
            PrintDSA ( _T ( "CYSOSWinSvc" ), _T ( "get out the thread..." ) );
            #endif
            return;
        }

        VOID WinSvcWrapper::RealSvcMain ( DWORD argc, LPTSTR *argv_ptr )
        {
            if ( NULL == the_instance_ptr_ )
            {
                return;
            }
            /// Register the handler function for the service
            service_status_handle_ = RegisterServiceCtrlHandlerEx ( service_name_, ( LPHANDLER_FUNCTION_EX ) SvcCtrlHandler, the_instance_ptr_.get() );
            if ( !service_status_handle_ )
            {
                wprintf ( L"The function[RegisterServiceCtrlHandler] fails.[%d]2\n", GetLastError() );
                return;
            }
            /// These SERVICE_STATUS members remain as set here
            service_status_.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
            service_status_.dwServiceSpecificExitCode = 0;
            /// Report initial status to the SCM
            ReportSvcStatus ( SERVICE_START_PENDING, NO_ERROR, 3000 );
            /// Create an event. The control handler function, SvcCtrlHandler,
            /// signals this event when it receives the stop control code.
            service_stop_event_handle_ = CreateEvent (
                                             NULL,                      ///< default security attributes
                                             TRUE,                      ///< manual reset event
                                             FALSE,                     ///< not signaled
                                             NULL );                    ///< no name
            if ( service_stop_event_handle_ == NULL )
            {
                ReportSvcStatus ( SERVICE_STOPPED, NO_ERROR, 0 );
                return;
            }
            /// Report running status when initialization is complete.
            ReportSvcStatus ( SERVICE_RUNNING, NO_ERROR, 0 );
            /// TO dO: Do job until the service stops.
            #ifdef _YSOS_UNICODE
            PrintDS ( _T ( "YSOSWinSVC" ), _T ( "After function[perform_work_function_ptr_]...." ) );
            #else
            PrintDSA ( _T ( "YSOSWinSVC" ), _T ( "After function[perform_work_function_ptr_]...." ) );
            #endif
            DWORD thread_id = 0;
            HANDLE thread_handle = YSOSBEGINTHREADEX ( NULL, 0, ThreadProc, 0, 0, &thread_id );
            if ( NULL == thread_handle )
            {
                wprintf ( L"The function[RegisterServiceCtrlHandler] fails.[%d]2\n", GetLastError() );
            }
            WaitForSingleObject ( service_stop_event_handle_, INFINITE );
            Sleep ( 10 );
            long timeout = 0;
            do
            {
                if ( WAIT_OBJECT_0 == WaitForSingleObject ( thread_handle, 1000 ) )
                {
                    break;
                }
                else
                {
                    ++timeout;
                    Sleep ( 10 );
                }
            } while ( 3 > timeout );
            if ( 3 == timeout )
            {
                #ifdef _YSOS_UNICODE
                PrintDS ( _T ( "YSOSWinSVC" ), _T ( "[kill the thread.]" ) );
                #else
                PrintDSA ( _T ( "YSOSWinSVC" ), _T ( "[kill the thread.]" ) );
                #endif
                TerminateThread ( thread_handle, 0 );
            }
            if ( NULL != thread_handle )
            {
                #ifdef _YSOS_UNICODE
                PrintDS ( _T ( "YSOSWinSVC" ), _T ( "[thread_handle is not NULL.]" ) );
                #else
                PrintDSA ( _T ( "YSOSWinSVC" ), _T ( "[thread_handle is not NULL.]" ) );
                #endif
                CloseHandle ( thread_handle );
            }
            #ifdef _YSOS_UNICODE
            PrintDS ( _T ( "YSOSWinSVC" ), _T ( "After function[WaitForSingleObject]...." ) );
            #else
            PrintDSA ( _T ( "YSOSWinSVC" ), _T ( "After function[WaitForSingleObject]...." ) );
            #endif
            ReportSvcStatus ( SERVICE_STOPPED, NO_ERROR, 0 );
            #ifdef _YSOS_UNICODE
            PrintDS ( _T ( "YSOSWinSVC" ), _T ( "After function[ReportSvcStatus]...." ) );
            #else
            PrintDSA ( _T ( "YSOSWinSVC" ), _T ( "After function[ReportSvcStatus]...." ) );
            #endif
            return;
        }

        VOID WinSvcWrapper::RealSvcCtrlHandler ( DWORD ctrol_code )
        {
            /// Handle the requested control code.
            switch ( ctrol_code )
            {
                case SERVICE_CONTROL_STOP:
                    {
                        #ifdef _YSOS_UNICODE
                        PrintDS ( _T ( "YSOSWinSVC" ), _T ( "Event [SERVICE_CONTROL_STOP]...." ) );
                        #else
                        PrintDSA ( _T ( "YSOSWinSVC" ), _T ( "Event [SERVICE_CONTROL_STOP]...." ) );
                        #endif
                        ReportSvcStatus ( SERVICE_STOP_PENDING, NO_ERROR, 0 );
                        //Signal the thread to die
                        InterlockedExchange ( &shutdown_flag_, TRUE );
                        //Signal the service to stop.
                        SetEvent ( service_stop_event_handle_ );
                        #ifdef _YSOS_UNICODE
                        wprintf ( _T ( "dwCurrentState[%d]" ), service_status_.dwCurrentState );
                        #else
                        printf ( _T ( "dwCurrentState[%d]" ), service_status_.dwCurrentState );
                        #endif
                        ReportSvcStatus ( service_status_.dwCurrentState, NO_ERROR, 0 );
                    }
                    break;
                case SERVICE_CONTROL_INTERROGATE:
                    #ifdef _YSOS_UNICODE
                    PrintDS ( _T ( "YSOSWinSVC" ), _T ( "Event [SERVICE_CONTROL_INTERROGATE]...." ) );
                    #else
                    PrintDSA ( _T ( "YSOSWinSVC" ), _T ( "Event [SERVICE_CONTROL_INTERROGATE]...." ) );
                    #endif
                    break;
                default:
                    #ifdef _YSOS_UNICODE
                    PrintDS ( _T ( "YSOSWinSVC" ), _T ( "Event [default]...." ) );
                    #else
                    PrintDSA ( _T ( "YSOSWinSVC" ), _T ( "Event [default]...." ) );
                    #endif
                    break;
            }
            return;
        }

        VOID WinSvcWrapper::ReportSvcStatus ( DWORD current_state, DWORD win32_exit_code, DWORD wait_hint )
        {
            static DWORD dwCheckPoint = 1;
            /// Fill in the SERVICE_STATUS structure.
            service_status_.dwCurrentState = current_state;
            service_status_.dwWin32ExitCode = win32_exit_code;
            service_status_.dwWaitHint = wait_hint;
            if ( current_state == SERVICE_START_PENDING )
            {
                service_status_.dwControlsAccepted = 0;
            }
            else
            {
                service_status_.dwControlsAccepted = SERVICE_ACCEPT_STOP;
            }
            if (
                ( current_state == SERVICE_RUNNING ) ||
                ( current_state == SERVICE_STOPPED )
            )
            {
                service_status_.dwCheckPoint = 0;
            }
            else
            {
                service_status_.dwCheckPoint = dwCheckPoint++;
            }
            /// Report the status of the service to the SCM.
            SetServiceStatus ( service_status_handle_, &service_status_ );
            return;
        }

        BOOL WinSvcWrapper::Install()
        {
            if ( NULL == the_instance_ptr_ )
            {
                return FALSE;
            }
            /// Get module file path
            TCHAR module_file_path[MAX_PATH] = {0,};
            if ( !::GetModuleFileName ( NULL, module_file_path, MAX_PATH ) )
            {
                wprintf ( L"The function[GetModuleFileName] fails.[%d]\n", GetLastError() );
                return FALSE;
            }
            wprintf ( L"szPath[%s]\n", module_file_path );
            return WinSvcWrapper::Install ( service_name_, display_name_, description_, module_file_path );
        }

        BOOL WinSvcWrapper::Uninstall()
        {
            if ( NULL == the_instance_ptr_ )
            {
                return FALSE;
            }
            return WinSvcWrapper::Uninstall ( service_name_ );
        }

        BOOL WinSvcWrapper::Start()
        {
            if ( NULL == the_instance_ptr_ )
            {
                return FALSE;
            }
            return WinSvcWrapper::Start ( service_name_ );
        }

        BOOL WinSvcWrapper::Stop()
        {
            if ( NULL == the_instance_ptr_ )
            {
                return FALSE;
            }
            return WinSvcWrapper::Stop ( service_name_ );
        }

        BOOL WinSvcWrapper::Run()
        {
            if ( NULL == the_instance_ptr_ )
            {
                return FALSE;
            }
            /// This call returns when the service has stopped.
            /// The process should simply terminate when the call returns.
            #ifdef _YSOS_UNICODE
            PrintDS ( _T ( "YSOSWinSVC" ), _T ( "Before function[StartServiceCtrlDispatcher]..." ) );
            #else
            PrintDSA ( _T ( "YSOSWinSVC" ), _T ( "Before function[StartServiceCtrlDispatcher]..." ) );
            #endif
            if ( !StartServiceCtrlDispatcher ( service_table_entry_ ) )
            {
                DWORD dwLastError = GetLastError();
                wprintf ( L"The function[StartServiceCtrlDispatcher] fails.[%d]\n", dwLastError );
                switch ( dwLastError )
                {
                    case ERROR_FAILED_SERVICE_CONTROLLER_CONNECT:
                        #ifdef _YSOS_UNICODE
                        PrintDS ( _T ( "YSOSWinSVC" ), _T ( "Event[ERROR_FAILED_SERVICE_CONTROLLER_CONNECT]..." ) );
                        #else
                        PrintDSA ( _T ( "YSOSWinSVC" ), _T ( "Event[ERROR_FAILED_SERVICE_CONTROLLER_CONNECT]..." ) );
                        #endif
                        wprintf ( L"The function[StartServiceCtrlDispatcher] fails.[ERROR_FAILED_SERVICE_CONTROLLER_CONNECT]\n" );
                        break;
                    case ERROR_INVALID_DATA:
                        #ifdef _YSOS_UNICODE
                        PrintDS ( _T ( "YSOSWinSVC" ), _T ( "Event[ERROR_INVALID_DATA]..." ) );
                        #else
                        PrintDSA ( _T ( "YSOSWinSVC" ), _T ( "Event[ERROR_INVALID_DATA]..." ) );
                        #endif
                        wprintf ( L"The function[StartServiceCtrlDispatcher] fails.[ERROR_INVALID_DATA]\n" );
                        break;
                    case ERROR_SERVICE_ALREADY_RUNNING:
                        #ifdef _YSOS_UNICODE
                        PrintDS ( _T ( "YSOSWinSVC" ), _T ( "Event[ERROR_SERVICE_ALREADY_RUNNING]..." ) );
                        #else
                        PrintDSA ( _T ( "YSOSWinSVC" ), _T ( "Event[ERROR_SERVICE_ALREADY_RUNNING]..." ) );
                        #endif
                        wprintf ( L"The function[StartServiceCtrlDispatcher] fails.[ERROR_SERVICE_ALREADY_RUNNING]\n" );
                        break;
                    default:
                        wprintf ( L"Unknown Error[%d]\n", dwLastError );
                        break;
                }
                return FALSE;
            }
            #ifdef _YSOS_UNICODE
            PrintDS ( _T ( "YSOSWinSVC" ), _T ( "After function[StartServiceCtrlDispatcher]..." ) );
            #else
            PrintDSA ( _T ( "YSOSWinSVC" ), _T ( "After function[StartServiceCtrlDispatcher]..." ) );
            #endif
            return TRUE;
        }

        VOID WinSvcWrapper::SvcReportEvent ( LPTSTR function_name_ptr )
        {
            HANDLE event_source_handle;
            LPCTSTR strings_ptr[2];
            TCHAR buffer[80];
            event_source_handle = RegisterEventSource ( NULL, service_name_ );
            if ( NULL != event_source_handle )
            {
                StringCchPrintf ( buffer, 80, TEXT ( "%s failed with %d" ), function_name_ptr, GetLastError() );
                strings_ptr[0] = service_name_;
                strings_ptr[1] = buffer;
                ReportEvent ( event_source_handle,              ///< event log handle
                              EVENTLOG_ERROR_TYPE,                ///< event type
                              0,                                  ///< event category
                              SVC_ERROR,                          ///< event identifier
                              NULL,                               ///< no security identifier
                              2,                                  ///< size of lpszStrings array
                              0,                                  ///< no binary data
                              strings_ptr,                        ///< array of strings
                              NULL );                             ///< no binary data
                DeregisterEventSource ( event_source_handle );
            }
        }

        void WinSvcWrapper::Init ( LPCTSTR service_name_ptr, LPCTSTR display_name_ptr, LPCTSTR description_ptr, SVCDoJobCallBackFuncPtr do_job_callback_func_ptr, const long perform_work_interval_time )
        {
            if ( service_name_ptr )
            {
                StringCchCopy ( service_name_, MAX_PATH, service_name_ptr );
            }
            else
            {
                StringCchCopy ( display_name_, MAX_PATH, _T ( "Default Name of Service" ) );
            }
            if ( display_name_ptr )
            {
                StringCchCopy ( display_name_, MAX_PATH, display_name_ptr );
            }
            else
            {
                StringCchCopy ( display_name_, MAX_PATH, _T ( "Default Display Name of Service" ) );
            }
            if ( description_ptr )
            {
                StringCchCopy ( description_, MAX_PATH, description_ptr );
            }
            else
            {
                StringCchCopy ( description_, MAX_PATH, _T ( "Default Description of Service" ) );
            }
            do_job_callback_func_ptr_ = do_job_callback_func_ptr;
            perform_work_interval_time_ = 1000;
            ZeroMemory ( &service_status_handle_, sizeof ( SERVICE_STATUS_HANDLE ) );
            ZeroMemory ( &service_status_, sizeof ( SERVICE_STATUS ) );
            service_stop_event_handle_ = NULL;
            shutdown_flag_ = FALSE;
            //Add any additional services for the process to this table.
            service_table_entry_[0].lpServiceName = service_name_;
            service_table_entry_[0].lpServiceProc = ( LPSERVICE_MAIN_FUNCTION ) SvcMain;
            service_table_entry_[1].lpServiceName = NULL;
            service_table_entry_[1].lpServiceProc = NULL;
            return;
        }

    }

}


/*************************************** add no used for linux  ************************************/