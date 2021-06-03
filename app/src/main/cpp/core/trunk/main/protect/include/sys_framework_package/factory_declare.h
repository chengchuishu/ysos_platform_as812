/**
 *@file factory_instance.h
 *@brief Put the implement of each factory here. One factory per Interface.
 *@version 0.1
 *@author XuLanyue
 *@date Created on: 2016-05-11 08:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef FACTORY_DECLARE_H                                                       //NOLINT
#define FACTORY_DECLARE_H                                                       //NOLINT

#include "../../../public/include/sys_interface_package/common.h"
#include "../../../public/include/os_hal_package/driverinterface.h"
#include "../../../public/include/sys_interface_package/baseinterface.h"
#include "../../../public/include/sys_interface_package/agentinterface.h"
#include "../../../public/include/sys_interface_package/callbackqueue.h"
#include "../../../public/include/sys_interface_package/moduleinterface.h"
#include "../../../public/include/sys_interface_package/sessioninterface.h"
#include "../../../public/include/sys_interface_package/protocolinterface.h"
#include "../../../public/include/sys_interface_package/strategyinterface.h"
#include "../../../public/include/sys_interface_package/transportinterface.h"
#include "../../../public/include/sys_interface_package/connectioninterface.h"
#include "../../../public/include/sys_interface_package/connectioncallbackinterface.h"
#include "../../../public/include/sys_interface_package/transportcallbackinterface.h"
#include "../../../public/include/sys_interface_package/rpcinterface.h"
#include "../../../public/include/sys_interface_package/rpccallbackinterface.h"
#include "../../../public/include/sys_framework_package/templatefactory.h"

namespace ysos {
//  BaseInterface
DECLARE_INTERFACEFACTORY(BaseInterface);
#define RegisteBaseInterfaceClass(base_interface) \
  REGISTER_CLASS(ysos::GetBaseInterfaceFactory(), base_interface, ysos::BaseInterfaceFactory);

//  DriverInterface
DECLARE_INTERFACEFACTORY(DriverInterface);
#define RegisteDriverInterfaceClass(driver_interface) \
  REGISTER_CLASS(ysos::GetDriverInterfaceFactory(), driver_interface, ysos::DriverInterfaceFactory);

//  ProtocolInterface
DECLARE_INTERFACEFACTORY(ProtocolInterface);
#define RegisterProtocolClass(protocol)  \
  REGISTER_CLASS(ysos::GetProtocolInterfaceFactory(), protocol, ysos::ProtocolInterfaceFactory);

/// AgentInterface
DECLARE_INTERFACEFACTORY(AgentInterface);
#define RegisteAgentClass(agent) \
  REGISTER_CLASS(ysos::GetAgentInterfaceFactory(), agent, ysos::AgentInterfaceFactory);

//  CallbackInterface
DECLARE_INTERFACEFACTORY(CallbackInterface);
#define RegisterCallbackClass(callback)  \
  REGISTER_CLASS(ysos::GetCallbackInterfaceFactory(), callback, ysos::CallbackInterfaceFactory);

//  CallbackQueueInterface
DECLARE_INTERFACEFACTORY(CallbackQueueInterface);
#define RegisterCallbackQueueClass(callback_queue)  \
  REGISTER_CLASS(ysos::GetCallbackQueueInterfaceFactory(), callback_queue, ysos::CallbackQueueInterfaceFactory);

//  ModuleInterface
DECLARE_INTERFACEFACTORY(ModuleInterface);
#define RegisterModuleClass(module)  \
  REGISTER_CLASS(ysos::GetModuleInterfaceFactory(), module, ysos::ModuleInterfaceFactory);

//  ModuleLinkInterface
DECLARE_INTERFACEFACTORY(ModuleLinkInterface);
#define RegisterModuleLinkClass(module_link) \
  REGISTER_CLASS(ysos::GetModuleLinkInterfaceFactory(), module_link, ysos::ModuleLinkInterfaceFactory);

//  SessionInterface
DECLARE_INTERFACEFACTORY(SessionInterface);
#define RegisterSessionClass(session)  \
  REGISTER_CLASS(ysos::GetSessionInterfaceFactory(), session, ysos::SessionInterfaceFactory);

//  StrategyInterface
DECLARE_INTERFACEFACTORY(StrategyInterface);
#define RegisterStrategyClass(strategy) \
  REGISTER_CLASS(ysos::GetStrategyInterfaceFactory(), strategy, ysos::StrategyInterfaceFactory);

//  TransportInterface
DECLARE_INTERFACEFACTORY(TransportInterface);
#define RegisterTransportClass(transport)  \
  REGISTER_CLASS(ysos::GetTransportInterfaceFactory(), transport, ysos::TransportInterfaceFactory);

//  ConnectionInterface
DECLARE_INTERFACEFACTORY(ConnectionInterface);
#define RegisterConnectionClass(connection)  \
  REGISTER_CLASS(ysos::GetConnectionInterfaceFactory(), connection, ysos::ConnectionInterfaceFactory);

//  ConnectionCallbackInterface
DECLARE_INTERFACEFACTORY(ConnectionCallbackInterface);
#define RegisterConnectionCalllbackClass(connection_callback)  \
  REGISTER_CLASS(ysos::GetConnectionCallbackInterfaceFactory(), connection_callback, ysos::ConnectionCallbackInterfaceFactory);

//  TransportCallbackInterface
DECLARE_INTERFACEFACTORY(TransportCallbackInterface);
#define RegisterTransportCallbackClass(transport_callback)  \
  REGISTER_CLASS(ysos::GetTransportCallbackInterfaceFactory(), transport_callback, ysos::TransportCallbackInterfaceFactory);

//  RPCInterface
DECLARE_INTERFACEFACTORY(RPCInterface);
#define RegisterRPCClass(rpc)  \
 REGISTER_CLASS(ysos::GetRPCInterfaceFactory(), rpc, ysos::RPCInterfaceFactory);

//  RPCCallbackInterface
DECLARE_INTERFACEFACTORY(RPCCallbackInterface);
#define RegisterRPCCallbackClass(rpc_callback)  \
  REGISTER_CLASS(ysos::GetRPCCallbackInterfaceFactory(), rpc_callback, ysos::RPCCallbackInterfaceFactory);


} // namespace ysos

#endif  //  FACTORY_DECLARE_H                                                   //NOLINT
