/**
 *@file pluginbasemoduleimpl.cpp
 *@brief Definition of plugin 下module的一般通用实现，主要实现功能
 *1. 读取配置文件，默认加载一个驱动 （Initialized 函数，RealOpen 函数）
 *2. 实现 PROP_FUN_CALLABILITY 功能，把参数从 module自动传给driver (GetProperty 函数）
 *3. 读取配置文件，callback传入的数据的数据类型的转换，在传到driver前生效（Initialized 函数，GetProperty 函数，NeedConvertDataTypeInfo 函数）
 *@version 2.0
 *@author libaoguo
 *@date Created on: 2017-08-25
 *@copyright Copyright(c) 2017 YunShen Technology. All rights reserved.
 *@how to Usage:
  *  继承自 PluginBaseModuleImpl 的类，在重写 PluginBaseModuleImpl类public函数之前 必须调用基类 PluginBaseModuleImpl的方法。
 *@todo
**/

#include "../../../public/include/common/pluginbasemoduleimpl.h"

// boost headers
#include <boost/thread/thread.hpp>

// ysos headers
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/manager_declare.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/ysos_enum.h"


namespace ysos {
PluginBaseModuleImpl::PluginBaseModuleImpl(const std::string &strClassName) : BaseModuleImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.audio");
  driver_ptr_ = NULL;
}

PluginBaseModuleImpl::~PluginBaseModuleImpl() {
}

int PluginBaseModuleImpl::RealUnInitialize(void *param) {
  YSOS_LOG_DEBUG("uninitialize logname:" <<logic_name_);
  driver_ptr_ = NULL;
  return YSOS_ERROR_SUCCESS;
}

int PluginBaseModuleImpl::Initialized(const std::string &key, const std::string &value)  {
  BaseModuleImpl::Initialized(key, value);
#ifdef _WIN32
  if (stricmp(key.c_str(), "relate_driver") ==0) {
    releate_driver_name_= value;
  } else if (stricmp(key.c_str(), "datatype_convert_info") ==0) {
    std::list<std::string> list_params;
    int iret= GetUtility()->SplitString(value, "|", list_params);
    if (YSOS_ERROR_SUCCESS !=iret || list_params.size() !=4) {
      YSOS_LOG_DEBUG("error datatype_convert config is error");
      assert(false);
      return YSOS_ERROR_FAILED;
    } else if (stricmp(key.c_str(), "driver_param_open") ==0) {
      driver_param_open_ = value;
    } else if (stricmp(key.c_str(), "driver_param_close") ==0) {
      driver_param_close_ = value;
    } else if (stricmp(key.c_str(), "driver_param_run") ==0) {
      driver_param_run_ = value;
    } else if (stricmp(key.c_str(), "driver_param_pause") ==0) {
      driver_param_pause_ = value;
    } else if (stricmp(key.c_str(), "driver_param_stop") ==0) {
      driver_param_stop_ = value;
    }

    ModuleDataTypeConvertInfo convert_info;
    std::list<std::string>::iterator it_list_params = list_params.begin();
    convert_info.source_input_datatype = *it_list_params;
    ++it_list_params;
    convert_info.source_output_datatype = *it_list_params;
    ++it_list_params;
    convert_info.dest_input_datatype = *it_list_params;
    ++it_list_params;
    convert_info.dest_output_datatype = *it_list_params;
    list_module_datatype_convert_info_.push_back(convert_info);
  } else if (stricmp(key.c_str(), "output_ability") ==0) { ///< 输出能力,如果有多个能力，则使用 ｜ 分开
    string_output_abilities_ = value;
    GetUtility()->SplitString(value, "|", list_output_abilities_);
  } else if (stricmp(key.c_str(), "thread_interval") ==0) { /// <!-- 线程module 的timeout 参数（单位毫秒) -->
    //int thread_interval = atoi(value.c_str());
    //assert(thread_interval >0);  ///< 此参数只有线程module才会使用到，所以这里设断言，值必大于0
    //thread_data_->timeout = thread_interval;
  }
#else
  if (strcasecmp(key.c_str(), "relate_driver") ==0) {
    releate_driver_name_= value;
  } else if (strcasecmp(key.c_str(), "datatype_convert_info") ==0) {
    std::list<std::string> list_params;
    int iret= GetUtility()->SplitString(value, "|", list_params);
    if (YSOS_ERROR_SUCCESS !=iret || list_params.size() !=4) {
      YSOS_LOG_DEBUG("error datatype_convert config is error");
      //assert(false);
      return YSOS_ERROR_FAILED;
    } else if (strcasecmp(key.c_str(), "driver_param_open") ==0) {
      driver_param_open_ = value;
    } else if (strcasecmp(key.c_str(), "driver_param_close") ==0) {
      driver_param_close_ = value;
    } else if (strcasecmp(key.c_str(), "driver_param_run") ==0) {
      driver_param_run_ = value;
    } else if (strcasecmp(key.c_str(), "driver_param_pause") ==0) {
      driver_param_pause_ = value;
    } else if (strcasecmp(key.c_str(), "driver_param_stop") ==0) {
      driver_param_stop_ = value;
    }

    ModuleDataTypeConvertInfo convert_info;
    std::list<std::string>::iterator it_list_params = list_params.begin();
    convert_info.source_input_datatype = *it_list_params;
    ++it_list_params;
    convert_info.source_output_datatype = *it_list_params;
    ++it_list_params;
    convert_info.dest_input_datatype = *it_list_params;
    ++it_list_params;
    convert_info.dest_output_datatype = *it_list_params;
    list_module_datatype_convert_info_.push_back(convert_info);
  } else if (strcasecmp(key.c_str(), "output_ability") ==0) { ///< 输出能力,如果有多个能力，则使用 ｜ 分开
    string_output_abilities_ = value;
    GetUtility()->SplitString(value, "|", list_output_abilities_);
  } else if (strcasecmp(key.c_str(), "thread_interval") ==0) { /// <!-- 线程module 的timeout 参数（单位毫秒) -->
    //int thread_interval = atoi(value.c_str());
    //assert(thread_interval >0);  ///< 此参数只有线程module才会使用到，所以这里设断言，值必大于0
    //thread_data_->timeout = thread_interval;
  }
#endif


  YSOS_LOG_DEBUG("test for log:logic name: "<< logic_name_ << " module_type: " << module_data_info_->module_type 
    << " input datatype: " << module_data_info_->in_datatypes << " output_datatype: " << module_data_info_->out_datatypes
    << " issel_allockattor: " << module_data_info_->is_self_allocator << " buffer_num: " << module_data_info_->buffer_number
    << " buffer_length: " <<  module_data_info_->buffer_length << " prefix: " << module_data_info_->prefix_length
    << " driver oepn param: " <<driver_param_open_ << " driver close param: " << driver_param_close_
    << " driver run param: " <<driver_param_open_ << " driver pause param: " << driver_param_pause_
    << " driver stop param: " << driver_param_stop_ /*<< " thread timeout: " << thread_data_->timeout*/);
  return YSOS_ERROR_SUCCESS;
}

int PluginBaseModuleImpl::RealOpen(LPVOID param) {
  YSOS_LOG_DEBUG("in RealOpen logic name:"<< logic_name_);
  if (NULL == driver_ptr_ && releate_driver_name_.length() >0) {
    assert(releate_driver_name_.length() >0);
    if (releate_driver_name_.length() <=0)
      return false;

    driver_ptr_ = GetDriverInterfaceManager()->FindInterface(releate_driver_name_);
    if (NULL == driver_ptr_) {
      YSOS_LOG_ERROR("error, cant find driver,in RealOpen logic name:["<< logic_name_ << "] releate_driver_name_:[" << releate_driver_name_<<"]");
    }

    assert(driver_ptr_);
  }

  if (driver_ptr_) {
    std::string temp_string_output_abilities = string_output_abilities_;
    FunObjectCommon2 fb2;
    fb2.pparam1 = this;
    fb2.pparam2 = &temp_string_output_abilities;
    driver_ptr_->GetProperty(PROP_REGISTER_ABILITY_EVENT, &fb2);

    int open_iret = driver_ptr_->Open(&driver_param_open_);
    YSOS_LOG_DEBUG("open result:" << (YSOS_ERROR_SUCCESS == open_iret) ? "success" : "fail");
                   return open_iret;
  }

  return YSOS_ERROR_SUCCESS;
}

int PluginBaseModuleImpl::RealRun() {
  std::list<std::string>::iterator it = list_output_abilities_.begin();
  for (it; it!= list_output_abilities_.end(); ++it) {
    std::string ablity_name = *it;
    if (ablity_name.length() <=0)
      continue;;

    BufferInterfacePtr ablity_name_buffptr = GetBufferUtility()->CreateBuffer(ablity_name);
    if (driver_ptr_)
      driver_ptr_->Ioctl(CMD_ICC_START, ablity_name_buffptr, NULL);
  }

  return YSOS_ERROR_SUCCESS;
}

int PluginBaseModuleImpl::RealPause() {
  return YSOS_ERROR_SUCCESS;
}

int PluginBaseModuleImpl::Ioctl(INT32 control_id, LPVOID param) {
  return BaseModuleImpl::Ioctl(control_id,param);
}

int PluginBaseModuleImpl::RealClose() {
  if (driver_ptr_) {
    std::string temp_string_output_abilities = string_output_abilities_;
    FunObjectCommon2 fb2;
    BaseModuleImpl *cur_module_ptr = this;
    fb2.pparam1 = cur_module_ptr;
    fb2.pparam2 = &temp_string_output_abilities;
    driver_ptr_->GetProperty(PROP_UNREGISTER_ABILITY_EVENT, &fb2);
    driver_ptr_->Close(&driver_param_close_);
    driver_ptr_ = NULL;
  }

  return YSOS_ERROR_SUCCESS;
}

int PluginBaseModuleImpl::RealStop() {
  std::list<std::string>::iterator it = list_output_abilities_.begin();
  for (it; it!= list_output_abilities_.end(); ++it) {
    std::string ablity_name = *it;
    if (ablity_name.length() <=0)
      continue;;

    BufferInterfacePtr ablity_name_buffptr = GetBufferUtility()->CreateBuffer(ablity_name);
    if (driver_ptr_)
      driver_ptr_->Ioctl(CMD_ICC_STOP, ablity_name_buffptr, NULL);
  }

  return YSOS_ERROR_SUCCESS;
}

int PluginBaseModuleImpl::GetProperty(int iTypeId, void *piType) {
  switch (iTypeId) {
  case PROP_FUN_CALLABILITY: {
    // 如果当前状态 是 realstop 状态，则callback输出为空
    /*
    PROP_CLOSE,
    PROP_STOP,
    PROP_PAUSE,
    PROP_RUN,
    */

    assert(piType);
    FunObjectCommon2* pobject = reinterpret_cast<FunObjectCommon2*>(piType);
    assert(pobject);
    if (NULL == pobject) {
      YSOS_LOG_DEBUG("error call property PROP_Fun_CallAbility, the param is null");
      return YSOS_ERROR_FAILED;
    }
    AbilityParam* pin = reinterpret_cast<AbilityParam*>(pobject->pparam1);
    AbilityParam* pout = reinterpret_cast<AbilityParam*>(pobject->pparam2);

    {
      ///  如果 当前module 没有在运行，则输出结果都是空，  在能力调以前 判断一次，在能力调之后判断一次
      INT32 module_state = 0;
      int igetstate_ret = GetState(3000, &module_state, NULL);  ///<  3秒超时，如果拿不到module的状态，则报错
      if (YSOS_ERROR_SUCCESS != igetstate_ret) {
        YSOS_LOG_DEBUG("error my cant get module status in 3000 miliseconds");
        assert(false);
      }

      if (PROP_RUN != module_state || YSOS_ERROR_SUCCESS != igetstate_ret) {
        YSOS_LOG_DEBUG("current is not run (current state:%" << module_state << " ) getsatatus fun return:" << igetstate_ret << " , call ability return null");
        if (pout && pout->buf_interface_ptr) {
          pout->buf_interface_ptr->SetLength(0);
        }

        return YSOS_ERROR_SKIP;
      }
    }

#ifdef _WIN32
    if (list_module_datatype_convert_info_.size() >0) {
      //assert(pin &&pout);
      std::string input_datatype = pin ? pin->data_type_name : "";
      std::string output_dataype = pout ? pout->data_type_name :"";
      ModuleDataTypeConvertInfo convert_info;
      if (NeedConvertDataTypeInfo(input_datatype, output_dataype, convert_info)) {   
        if (convert_info.dest_input_datatype.length() >0 && pin) {
          strcpy_s(pin->data_type_name, sizeof(pin->data_type_name), convert_info.dest_input_datatype.c_str());
        }
        if (convert_info.dest_output_datatype.length() >0 && pout) {
          strcpy_s(pout->data_type_name, sizeof(pout->data_type_name), convert_info.dest_output_datatype.c_str());
        }
      }
    }
#else
if (list_module_datatype_convert_info_.size() >0) {
      assert(pin &&pout);
      std::string input_datatype = pin ? pin->data_type_name : "";
      std::string output_dataype = pout ? pout->data_type_name :"";
      ModuleDataTypeConvertInfo convert_info;
      if (NeedConvertDataTypeInfo(input_datatype, output_dataype, convert_info)) {   
        if (convert_info.dest_input_datatype.length() >0 && pin) {
          strcpy(pin->data_type_name, convert_info.dest_input_datatype.c_str());
        }
        if (convert_info.dest_output_datatype.length() >0 && pout) {
          strcpy(pout->data_type_name, convert_info.dest_output_datatype.c_str());
        }
      }
    }
#endif
    assert(driver_ptr_);
    if (NULL == driver_ptr_) {
      return YSOS_ERROR_FAILED;
    }

    // 这里全权传发。
    int driver_ret =  driver_ptr_->GetProperty(iTypeId, piType);

    ///  如果 当前module 没有在运行，则输出结果都是空，  在能力调以前 判断一次，在能力调之后判断一次
    INT32 module_state = 0;
    int igetstate_ret = GetState(3000, &module_state, NULL);  ///<  3秒超时，如果拿不到module的状态，则报错
    if (YSOS_ERROR_SUCCESS != igetstate_ret) {
      YSOS_LOG_DEBUG("error my cant get module status in 3000 miliseconds");
      assert(false);
    }

    if (PROP_RUN != module_state || YSOS_ERROR_SUCCESS != igetstate_ret) {
      YSOS_LOG_DEBUG("current is not run (current state:%d" << module_state << " ) getsatatus fun return:" << igetstate_ret << " , call ability return null");
      if (pout && pout->buf_interface_ptr) {
        pout->buf_interface_ptr->SetLength(0);
      }

      return YSOS_ERROR_SKIP;
    }

    return driver_ret;
  }
  break;

  default: {
    int ret = BaseModuleImpl::GetProperty(iTypeId, piType);
    return ret;
  }
  }

  return YSOS_ERROR_SUCCESS;
}

bool PluginBaseModuleImpl::NeedConvertDataTypeInfo(const std::string source_input_datatype, 
  const std::string source_output_datatypa, ModuleDataTypeConvertInfo& convert_info) {
  assert(source_input_datatype.length() >0 || source_output_datatypa.length() >0);
  if (source_input_datatype.length() <=0 && source_output_datatypa.length() <=0)
    return false;

#ifdef _WIN32
  std::list<ModuleDataTypeConvertInfo>::iterator it = list_module_datatype_convert_info_.begin();
  for (it; it != list_module_datatype_convert_info_.end(); ++it) {
    ModuleDataTypeConvertInfo* pinfo = &(*it);
    if (source_input_datatype.length() >0 && pinfo->source_input_datatype.length() >0) {
      if (stricmp(source_input_datatype.c_str(), pinfo->source_input_datatype.c_str()) !=0)
        continue;
    }

    if (source_output_datatypa.length() >0 && pinfo->source_output_datatype.length() >0) {
      if (stricmp(source_output_datatypa.c_str(), pinfo->source_output_datatype.c_str()) !=0)
        continue;
    }

    YSOS_LOG_DEBUG("datatype info convert succ, source_input:"<< source_input_datatype << " source_output:" 
      <<source_output_datatypa <<"  dest_input:"<< pinfo->dest_input_datatype <<"  dest_output:" <<pinfo->dest_output_datatype);
    convert_info = *pinfo;
    return true;
  }
#else
  std::list<ModuleDataTypeConvertInfo>::iterator it = list_module_datatype_convert_info_.begin();
  for (it; it != list_module_datatype_convert_info_.end(); ++it) {
    ModuleDataTypeConvertInfo* pinfo = &(*it);
    if (source_input_datatype.length() >0 && pinfo->source_input_datatype.length() >0) {
      if (strcasecmp(source_input_datatype.c_str(), pinfo->source_input_datatype.c_str()) !=0)
        continue;
    }

    if (source_output_datatypa.length() >0 && pinfo->source_output_datatype.length() >0) {
      if (strcasecmp(source_output_datatypa.c_str(), pinfo->source_output_datatype.c_str()) !=0)
        continue;
    }

    YSOS_LOG_DEBUG("datatype info convert succ, source_input:"<< source_input_datatype << " source_output:" <<source_output_datatypa <<"  dest_input:"<< pinfo->dest_input_datatype <<"  dest_output:" <<pinfo->dest_output_datatype);
    convert_info = *pinfo;
    return true;
  }
#endif  

  return false;
}

int PluginBaseModuleImpl::InitalDataInfo() {
  return YSOS_ERROR_SUCCESS;
}

}
