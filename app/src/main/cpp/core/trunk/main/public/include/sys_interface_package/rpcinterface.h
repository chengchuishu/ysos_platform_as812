/**
 *@file rpcinterface.h
 *@brief Definition of RPCInterface
 *@version 0.1
 *@author jinchengzhe
 *@date Created on: 2016-10-10 14:00:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef SIP_RPC_INTERFACE_H       //NOLINT
#define SIP_RPC_INTERFACE_H       //NOLINT

/// ThirdParty Headers
#include <boost/shared_ptr.hpp>
/// Platform Headers
#include "../../../public/include/sys_interface_package/baseinterface.h"
#include "../../../public/include/sys_interface_package/rpccallbackinterface.h"

namespace ysos {
/**
*@brief RPC接口
*/
class YSOS_EXPORT RPCInterface: virtual public BaseInterface {
  IMPLEMENT_PUBLIC_EMPTY_DESTRUCTOR(RPCInterface)
  virtual int Open(RPCCallbackInterface* params, std::string* session_id_Ptr = NULL) = 0;
  virtual int Close(const std::string* session_id_ptr = NULL) = 0;
  virtual int DoService(const std::string& session_id, const std::string& service_name, const std::string& service_param) = 0;
  virtual int OnDispatchMessage(const std::string& session_id, const std::string& service_name, const std::string& result) = 0;
};

typedef boost::shared_ptr<RPCInterface> RPCInterfacePtr;

} // namespace ysos

#endif  //  SIP_RPC_INTERFACE_H   //NOLINT
