/*
# daemoninterface.cpp
# Definition of daemoninterface
# Created on: 2017-11-16 20:09:11
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20171116, created by JinChengZhe
*/

#include "../../../protect/include/sys_daemon_package/daemoninterface.h"
#include "../../../protect/include/sys_daemon_package/daemon.h"

namespace ysos {

namespace sys_daemon_package {

#ifdef __cplusplus
extern "C" {
#endif

int StartYSOSDaemonModule(void) {
  if (NULL == GetDaemon()) {
    return YSOS_ERROR_FAILED;
  }

  return (GetDaemon()->Start());
}

int StopYSOSDaemonModule(void) {
  if (NULL == GetDaemon()) {
    return YSOS_ERROR_FAILED;
  }

  return (GetDaemon()->Stop());
}


#ifdef __cplusplus
}  /// end of extern "C"
#endif

}  /// end of namespace sys_daemon_package

}  /// end of namespace ysos
