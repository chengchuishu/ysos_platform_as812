/**
 *@file BaseCommmonStrategyImpl.h
 *@brief Definition of BaseCommmonStrategyImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef SFP_YSOS_DAEMON_STRATEGY_IMPL_H_  // NOLINT
#define SFP_YSOS_DAEMON_STRATEGY_IMPL_H_  // NOLINT

/// STL Headers
#include <map>
#include <list>
/// ThirdParty Headers
#include <tinyxml2/tinyxml2.h>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
/// Private Ysos Headers
#include "../../../public/include/os_hal_package/lock.h"
#include "../../../public/include/sys_interface_package/strategyinterface.h"
#include "../../../public/include/sys_interface_package/msginterface.h"
#include "../../../protect/include/sys_framework_package/serviceinfotable.h"
#include "../../../protect/include/sys_framework_package/basecommonstrategyimpl.h"
#include "../../../protect/include/sys_framework_package/ysosdaemonconnectioncallback.h"

namespace ysos {
/*************************************************************************************************************
 *                      YsosDaemonStrategyImpl                                                               *
 *************************************************************************************************************/
/**
  *@brief BaseCommonStrategyImpl是独立于BaseStrategyImpl的又一个更通用的Strategy基类    //  NOLINT
     它首先用于状态收集器和Daemon与YSOS的通信逻辑处理           //  NOLINT
  */
class YSOS_EXPORT YsosDaemonStrategyImpl : BaseCommonStrategyImpl {
  DECLARE_CREATEINSTANCE(YsosDaemonStrategyImpl);
  DISALLOW_COPY_AND_ASSIGN(YsosDaemonStrategyImpl);
  DECLARE_PROTECT_CONSTRUCTOR(YsosDaemonStrategyImpl);

 public:
  virtual ~YsosDaemonStrategyImpl(void);
  /**
   *@brief 基本初始化  // NOLINT
   *@param param： 初始化的参数 // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Initialize(void *param=NULL);
  /**
   *@brief 清除资源,Initialize的逆操作  // NOLINT
   *@param param： 初始化的参数 // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int UnInitialize(void *param=NULL);
};

typedef boost::shared_ptr<YsosDaemonStrategyImpl>   YsosDaemonStrategyImplPtr;

}
#endif  // SFP_YSOS_DAEMON_STRATEGY_IMPL_H_  // NOLINT
