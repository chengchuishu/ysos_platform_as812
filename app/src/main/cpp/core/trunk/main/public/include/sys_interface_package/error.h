/*
# common.h
# Definition of multimap
# Created on: 2016-04-13 13:59:20
# Original author: dhongqian
# Copyright (c) 2015 YunShen Technology. All rights reserved.
# 
# version 0.1, 20160413, created by dhongqian
*/
#ifndef CHP_ERROR_H     //NOLINT
#define CHP_ERROR_H     //NOLINT

namespace ysos {
//  Error range [3000 - 500, 3000 + 500) = [2500, 3500),
//  for Transport use
#define YSOS_ERROR_TRANSPORT_ERROR_BASE                    3000
#define YSOS_ERROR_TRANSPORT_ERROR_OPERATION_ABORTED       3001
#define YSOS_ERROR_TRANSPORT_ERROR_EOF                     3002
#define YSOS_ERROR_TRANSPORT_ERROR_BAD_DESCRIPTOR          3003
#define YSOS_ERROR_TRANSPORT_ERROR_CONNECTION_RESET        3004
#define YSOS_ERROR_TRANSPORT_ERROR_OTHER                   3005

//  Error range [2000 - 500, 2000 + 500) = [1500, 2500),
//  for Callback/CallbackQueue/Module/ModuleLink use
#define YSOS_ERROR_MODULE_ERROR_BASE         2000

//  driver errors
//  Error range [1000 - 500, 1000 + 500) = [500, 1500),
#define YSOS_DRVERR_OK                       1000 
#define YSOS_DRVERR_FAIL                     1001
#define YSOS_DRVERR_ACTIONNOTSUPPORTED       1002
#define YSOS_DRVERR_NOFEATURE                1003


#define YSOS_ERROR_FINISH_ALREADY           10                  //  已经执行过了
#define YSOS_ERROR_SKIP                      9                  //  当前条件不满足，跳过当前操作，继续执行后续的条件   //NOLINT
#define YSOS_ERROR_PASS                      8                  //  当前条件满足，不需要执行后续的条件   //NOLINT
#define YSOS_ERROR_NO_OUTPUT                 7                  //  没有输出                  //NOLINT
#define YSOS_ERROR_NOT_EXISTED               6                  //  指定的资源，不存在          //NOLINT
#define YSOS_ERROR_NOT_SUPPORTED             5                  //  属性、配置不支持当前操作     //NOLINT
#define YSOS_ERROR_HAS_EXISTED               4                  //  已存在                   //NOLINT
#define YSOS_ERROR_NOT_ENOUGH_RESOURCE       3                  //  资源不足                  //NOLINT
#define YSOS_ERROR_LOGIC_ERROR               2                  //  逻辑错误                  //NOLINT
#define YSOS_ERROR_IS_BUSY                   1                  //  当前忙                   //NOLINT
#define YSOS_ERROR_SUCCESS                   0                  //  成功                     //NOLINT
#define YSOS_ERROR_FAILED                   -1                  //  未知错误                  //NOLINT
#define YSOS_ERROR_OUTOFMEMORY              -2                  //  内存溢出                  //NOLINT
#define YSOS_ERROR_INVALID_ARGUMENTS        -3                  //  非法参数                  //NOLINT
#define YSOS_ERROR_MEMORY_ALLOCATED_FAILED  -4                  //  内存分配失败              //NOLINT
#define YSOS_ERROR_INVALID_OPERATION        -5                  //  非法操作                  //NOLINT
#define YSOS_ERROR_TIMEOUT                  -6                  //  超时                  //NOLINT

}   //  namespace ysos

#endif // CHP_ERROR_H   //NOLINT
