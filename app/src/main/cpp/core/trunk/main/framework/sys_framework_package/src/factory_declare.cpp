/**
 *@file factory_instance.cpp
 *@brief Put the implement of each factory here. One factory per Interface.
 *@version 0.1
 *@author XuLanyue
 *@date Created on: 2016-05-11 08:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#include "../../../protect/include/sys_framework_package/factory_declare.h"

namespace ysos {

//  BaseInterface
IMPLEMENT_CREATEFACTORYINSTANCE(BaseInterfaceFactory);

//  DriverInterface
IMPLEMENT_CREATEFACTORYINSTANCE(DriverInterfaceFactory);

//  ProtocolInterface
IMPLEMENT_CREATEFACTORYINSTANCE(ProtocolInterfaceFactory);

//  CallbackInterface
IMPLEMENT_CREATEFACTORYINSTANCE(CallbackInterfaceFactory);

//  CallbackQueueInterface
IMPLEMENT_CREATEFACTORYINSTANCE(CallbackQueueInterfaceFactory);

//  ModuleInterface
IMPLEMENT_CREATEFACTORYINSTANCE(ModuleInterfaceFactory);

//  ModuleLinkInterface
IMPLEMENT_CREATEFACTORYINSTANCE(ModuleLinkInterfaceFactory);

//  SessionInterface
IMPLEMENT_CREATEFACTORYINSTANCE(SessionInterfaceFactory);

//  AgentInterface
IMPLEMENT_CREATEFACTORYINSTANCE(AgentInterfaceFactory);

//  StrategyInterface
IMPLEMENT_CREATEFACTORYINSTANCE(StrategyInterfaceFactory);

//  TransportInterface
IMPLEMENT_CREATEFACTORYINSTANCE(TransportInterfaceFactory);

//  ConnectionInterface
IMPLEMENT_CREATEFACTORYINSTANCE(ConnectionInterfaceFactory);

//  ConnectionCallbackInterface
IMPLEMENT_CREATEFACTORYINSTANCE(ConnectionCallbackInterfaceFactory);

//  TransportCallbackInterface
IMPLEMENT_CREATEFACTORYINSTANCE(TransportCallbackInterfaceFactory);

//  RPCInterface
IMPLEMENT_CREATEFACTORYINSTANCE(RPCInterfaceFactory);

//  RPCCallbackInterface
IMPLEMENT_CREATEFACTORYINSTANCE(RPCCallbackInterfaceFactory);

} // namespace ysos

