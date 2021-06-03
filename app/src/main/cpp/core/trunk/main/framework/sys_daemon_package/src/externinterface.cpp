/*
# externinterface.cpp
# Definition of extern interface
# Created on: 2017-03-20 15:53:55
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20170320, created by JinChengZhe
*/


#include "../../../protect/include/sys_daemon_package/externinterface.h"


namespace ysos {

namespace extern_interface {
/**
*@brief 全局日志对象  // NOLINT
*/
log4cplus::Logger logger_;  ///< 日志对象
}  /// namespace extern_interface

}  /// namespace ysos
