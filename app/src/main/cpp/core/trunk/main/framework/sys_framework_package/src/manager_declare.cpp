/**
 *@file manager_instance.cpp
 *@brief Put the implement of each manager here. One manager per Interface.
 *@version 0.1
 *@author XuLanyue
 *@date Created on: 2016-05-11 08:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

#include "../../../protect/include/sys_framework_package/manager_declare.h"

namespace ysos {
//  BaseInterface
//  IMPLEMENT_CREATEMANAGERINSTANCE(BaseInterfaceManager);

//  DriverInterface
IMPLEMENT_CREATEMANAGERINSTANCE(DriverInterfaceManager);

//  ProtocolInterface
IMPLEMENT_CREATEMANAGERINSTANCE(ProtocolInterfaceManager);

//  CallbackInterface
IMPLEMENT_CREATEMANAGERINSTANCE(CallbackInterfaceManager);

//  CallbackQueueInterface
IMPLEMENT_CREATEMANAGERINSTANCE(CallbackQueueInterfaceManager);

//  ModuleInterface
IMPLEMENT_CREATEMANAGERINSTANCE(ModuleInterfaceManager);

//  ModuleLinkInterface
IMPLEMENT_CREATEMANAGERINSTANCE(ModuleLinkInterfaceManager);

//  SessionInterface
IMPLEMENT_CREATEMANAGERINSTANCE(SessionInterfaceManager);

//  AgentInterface
IMPLEMENT_CREATEMANAGERINSTANCE(AgentInterfaceManager);

//  StrategyInterface
IMPLEMENT_CREATEMANAGERINSTANCE(StrategyInterfaceManager);

//  TransportInterface
IMPLEMENT_CREATEMANAGERINSTANCE(TransportInterfaceManager);

//  ConnectionInterface
IMPLEMENT_CREATEMANAGERINSTANCE(ConnectionInterfaceManager);

//  ConnectionCallbackInterface
IMPLEMENT_CREATEMANAGERINSTANCE(ConnectionCallbackInterfaceManager);

//  TransportCallbackInterface
IMPLEMENT_CREATEMANAGERINSTANCE(TransportCallbackInterfaceManager);

//  RPCInterface
IMPLEMENT_CREATEMANAGERINSTANCE(RPCInterfaceManager);

//  RPCCallbackInterface
IMPLEMENT_CREATEMANAGERINSTANCE(RPCCallbackInterfaceManager);

} // namespace ysos
