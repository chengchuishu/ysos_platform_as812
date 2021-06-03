/**   //NOLINT
  *@file mycommondefine.h
  *@brief Definition of
  *@version 0.1
  *@author wanfenfen
  *@date Created on: 2018:4:16   10:26
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  *@howto Usage:
  *@todo
  */

#ifndef YSOS_DRIVER_MYCOMMON_DEFINE_H_
#define YSOS_DRIVER_MYCOMMON_DEFINE_H_

namespace ysos {

#ifdef _DEBUG
  #define _WFF_DEBUG               ///< 一些自己 的测试代码，发布时注释掉
#endif


#ifdef _DEBUG
  #define MY_LOGGER GetUtility()->GetLogger("ysos.literobotcapacitystrategy")
#else
  #define MY_LOGGER GetUtility()->GetLogger("ysos.video")
#endif

#define ZERO_DOUBLE (1e-15)
#define Var2String(val)  (#val)
const double g_match_pi = 3.14159265358979323846;

}


#endif