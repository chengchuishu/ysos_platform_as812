/**
 *@file BaseCallbackImpl.cpp
 *@brief Definition of BaseCallbackImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// Self Header //  NOLINT
#include "../../../protect/include/sys_framework_package/statuseventmodulecallback.h"  // NOLINT
/// ysos private headers //  NOLINT
#include "../../../protect/include/sys_framework_package/basemoduleimpl.h"
#include "../../../public/include/core_help_package/utility.h"
#include "../../../protect/include/sys_framework_package/packageconfigimpl.h"

namespace ysos {
StatusEventModuleCallback::StatusEventModuleCallback(const std::string& name): BaseModuleCallbackImpl(name) {  // NOLINT
}

StatusEventModuleCallback::~StatusEventModuleCallback() {
}

/**
 *  context是该Callback所属的Module
 */
int StatusEventModuleCallback::Callback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, void *context) {
  BaseModuleImpl* module_ptr = GetCurModule(context);
  assert(NULL != module_ptr);

  // data_flow和ctrol_flow同步在这儿完成
  module_ptr->UpdateTeamParam();

  /*int *module_type = new int;
  int ret = module_ptr->GetProperty(PROP_MODULE_TYPE, module_type);

  assert(YSOS_ERROR_SUCCESS == ret);*/
  int ret = BaseModuleCallbackImpl::InvokeNextCallback(input_buffer, output_buffer, context);

  return ret;
}

int StatusEventModuleCallback::RealCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr next_input_buffer,
    BufferInterfacePtr pre_output_buffer, void *context) {
  return YSOS_ERROR_SUCCESS;
}
}  // NOLINT
