/**
 *@file ysos struct.h
 *@brief Definition of ysos struct 
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-26 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef SIP_YSOS_STRUCT_H_          //NOLINT
#define SIP_YSOS_STRUCT_H_          //NOLINT

namespace ysos {
	
/**
  *@brief 描述状态事件信息的结构
  */
struct StatusEventInfo {
  std::string   code;   ///<  status and event code
  std::string    detail;  ///< status and event detail message
  };

/**
  *@brief 通用的Param结构
  */
struct ParamInfo {
  int    param_type;   ///<  Param的类型
  void  *param;  ///< 具体的Param参数,类型由param_type决定
  };
  
}

#endif  // SIP_YSOS_STRUCT_H_       //NOLINT
