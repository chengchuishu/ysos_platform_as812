/**
 *@file manager_declare.h
 *@brief Put the implement of each manager here. One manager per Interface.
 *@version 0.1
 *@author XuLanyue
 *@date Created on: 2016-05-11 08:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef MANAGER_DECLARE_H       //NOLINT
#define MANAGER_DECLARE_H       //NOLINT

#include "../../../protect/include/sys_framework_package/factory_declare.h"
#include "../../../public/include/sys_framework_package/templatemanager.h"

namespace ysos {
//  BaseInterface
//  DECLARE_INTERFACEMANAGER(BaseInterface);

//  DriverInterface
DECLARE_INTERFACEMANAGER(DriverInterface);

//  ProtocolInterface
DECLARE_INTERFACEMANAGER(ProtocolInterface);

/// AgentInterface
DECLARE_INTERFACEMANAGER(AgentInterface);

//  CallbackInterface
DECLARE_INTERFACEMANAGER(CallbackInterface);

//  CallbackQueueInterface
DECLARE_INTERFACEMANAGER(CallbackQueueInterface);

//  ModuleInterface
DECLARE_INTERFACEMANAGER(ModuleInterface);

//  ModuleLinkInterface
DECLARE_INTERFACEMANAGER(ModuleLinkInterface);

//  SessionInterface
DECLARE_INTERFACEMANAGER(SessionInterface);

//  StrategyInterface
DECLARE_INTERFACEMANAGER(StrategyInterface);

//  TransportInterface
DECLARE_INTERFACEMANAGER(TransportInterface);

//  ConnectionInterface
DECLARE_INTERFACEMANAGER(ConnectionInterface);

//  ConnectionCallbackInterface
DECLARE_INTERFACEMANAGER(ConnectionCallbackInterface);

//  TransportCallbackInterface
DECLARE_INTERFACEMANAGER(TransportCallbackInterface);

//  RPCInterface
DECLARE_INTERFACEMANAGER(RPCInterface);

//  RPCCallbackInterface
DECLARE_INTERFACEMANAGER(RPCCallbackInterface);

} // namespace ysos
#endif  //  MANAGER_DECLARE_H   //NOLINT
