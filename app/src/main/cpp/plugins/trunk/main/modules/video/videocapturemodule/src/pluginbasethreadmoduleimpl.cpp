/**   //NOLINT
  *@file PluginBaseThreadModuleImpl.cpp
  *@brief Definition of plugin 下module的一般通用实现
  *@version 0.1
  *@author wanfenfen
  *@date Created on: 2016:12:14   10:53
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  *@howto Usage:
  *@todo
  */

/// self header
#include "../../../public/include/common/pluginbasethreadmoduleimpl.h"

/// boost headers
#include <boost/thread/thread.hpp>

/// ysos headers
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/manager_declare.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"


namespace ysos {
//DECLARE_PLUGIN_REGISTER_INTERFACE(PluginBaseThreadModuleImpl, ModuleInterface);
PluginBaseThreadModuleImpl::PluginBaseThreadModuleImpl(const std::string &strClassName /* =ASRModule */):/*BaseModuleImpl*/BaseThreadModuleImpl(strClassName) {
  YSOS_LOG_DEBUG("in PluginBaseThreadModuleImpl construct, logic name"<< logic_name_ );
  //InitalDataInfo();
  driver_ptr_ = NULL;
  logger_ = GetUtility()->GetLogger("ysos.audio");
}

PluginBaseThreadModuleImpl::~PluginBaseThreadModuleImpl() {
  YSOS_LOG_DEBUG("in PluginBaseThreadModuleImpl construct, logic name"<< logic_name_);
  driver_ptr_ = NULL;
}

int PluginBaseThreadModuleImpl::Initialized(const std::string &key, const std::string &value)  {
  BaseModuleImpl::Initialized(key, value);
#ifdef _WIN32
  if (stricmp(key.c_str(), "relate_driver") ==0) { ///< 关联的驱动 driver的逻辑名, 目前只支持一个driver
    releate_driver_name_= value;
  } else if (stricmp(key.c_str(), "datatype_convert_info") ==0) { ///< 数据类型转换
    std::list<std::string> list_params;
    int iret= GetUtility()->SplitString(value, "|", list_params);
    if (YSOS_ERROR_SUCCESS !=iret || list_params.size() !=4) {
      YSOS_LOG_DEBUG("error datatype_convert config is error");
      //assert(false);
      return YSOS_ERROR_FAILED;
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
  } else if (stricmp(key.c_str(), "thread_interval") ==0) { /// <!-- 线程module 的timeout 参数（单位毫秒) -->
    int thread_interval = atoi(value.c_str());
    assert(thread_interval >0);  ///< 此参数只有线程module才会使用到，所以这里设断言，值必大于0
    thread_data_->timeout = thread_interval;
  } 
#else
  if (strcasecmp(key.c_str(), "relate_driver") ==0) { ///< 关联的驱动 driver的逻辑名, 目前只支持一个driver
    releate_driver_name_= value;
  } else if (strcasecmp(key.c_str(), "datatype_convert_info") ==0) { ///< 数据类型转换
    std::list<std::string> list_params;
    int iret= GetUtility()->SplitString(value, "|", list_params);
    if (YSOS_ERROR_SUCCESS !=iret || list_params.size() !=4) {
      YSOS_LOG_DEBUG("error datatype_convert config is error");
      //assert(false);
      return YSOS_ERROR_FAILED;
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
  } else if (strcasecmp(key.c_str(), "thread_interval") ==0) { /// <!-- 线程module 的timeout 参数（单位毫秒) -->
    int thread_interval = atoi(value.c_str());
    assert(thread_interval >0);  ///< 此参数只有线程module才会使用到，所以这里设断言，值必大于0
    thread_data_->timeout = thread_interval;
  }
#endif

  {
    /// test 打印日志
    YSOS_LOG_DEBUG("test for log:logic name:"<< logic_name_ << " module_type:" <<module_data_info_->module_type << "  input datatype:" <<module_data_info_->in_datatypes
            <<"  output_datatype:" <<module_data_info_->out_datatypes << " issel_allockattor:" <<module_data_info_->is_self_allocator
            <<" buffer_num:" << module_data_info_->buffer_number << " buffer_length:" <<  module_data_info_->buffer_length
            <<"  prefix:" << module_data_info_->prefix_length
            <<" driver oepn param:" <<driver_param_open_
            <<" driver close param:" <<driver_param_close_
            <<" driver run param:" <<driver_param_open_
            <<" driver pause param:" <<driver_param_pause_
            <<" driver stop param:" << driver_param_stop_
            <<" thread timeout:" <<thread_data_->timeout
           );
  }

  return YSOS_ERROR_SUCCESS;
}

int PluginBaseThreadModuleImpl::RealOpen(LPVOID param) {
  /* int iret =  */BaseThreadModuleImpl::RealOpen(param);

  YSOS_LOG_DEBUG("in RealOpen, logic name:"<< logic_name_);

  //AllocateBufferPool();


  if (NULL == driver_ptr_) {
    //driver_ptr_.reset(iflyVoiceDriver::CreateInstance());
    YSOS_LOG_DEBUG("relate driver name is : " << releate_driver_name_.c_str());
    assert(releate_driver_name_.length() >0);
    if (releate_driver_name_.length() <=0) {
      YSOS_LOG_ERROR("relate driver name length is less than 0");
      return false;
    }
    driver_ptr_ = GetDriverInterfaceManager()->FindInterface(releate_driver_name_);
    if(NULL == driver_ptr_) {
      YSOS_LOG_ERROR("error, cant file driver:" << releate_driver_name_);
    }
    assert(driver_ptr_);
  }
  if (driver_ptr_) {
    // 添加事件回调地址为当前module
    /*FunObjectCommon2 fb2;
    fb2.pparam1 = this;
    fb2.pparam2 = const_cast<char*>(string_output_abilities_.c_str());
    int event_call_ret = driver_ptr_->GetProperty(PROP_REGISTER_ABILITY_EVENT, &fb2);*/
    std::string temp_string_output_abilities = string_output_abilities_; ///< 使用复本数据，防止被外部调用修改
    FunObjectCommon2 fb2;
    fb2.pparam1 = this;
    fb2.pparam2 = &temp_string_output_abilities;
    /* int event_call_ret =  */driver_ptr_->GetProperty(PROP_REGISTER_ABILITY_EVENT, &fb2);

    //int open_iret = driver_ptr_->Open(param);
    int open_iret = driver_ptr_->Open(&driver_param_open_);
    YSOS_LOG_DEBUG("open result:" << ((YSOS_ERROR_SUCCESS == open_iret) ? "success" : "fail"));
    return open_iret;
  } else {
    YSOS_LOG_ERROR("get driver failed: " << logic_name_);
  }

  return BaseThreadModuleImpl::RealOpen(param);
}


int PluginBaseThreadModuleImpl::RealRun() {
  //return /*driver_ptr_->Ioctl(CMD_ICC_START, NULL, NULL);*/ YSOS_ERROR_SUCCESS;
  /// 从输出能力列表遍历 开始运行所有的能力
  std::list<std::string>::iterator it = list_output_abilities_.begin();
  for (it; it!= list_output_abilities_.end(); ++it) {
    std::string ablity_name = *it;
    if (ablity_name.length() <=0)
      continue;;

    BufferInterfacePtr ablity_name_buffptr = GetBufferUtility()->CreateBuffer(ablity_name);
    if (driver_ptr_) {
      //std::cout<<" test ioctrl id:CMD_ICC_START" <<CMD_ICC_START);
      driver_ptr_->Ioctl(CMD_ICC_START, ablity_name_buffptr, NULL);
    }
  }
  return YSOS_ERROR_SUCCESS;
}

int PluginBaseThreadModuleImpl::RealPause() {
  return /*driver_ptr_->Ioctl(CMD_ICC_PAUSE, NULL, NULL);*/YSOS_ERROR_SUCCESS;
}



int PluginBaseThreadModuleImpl::Ioctl(INT32 control_id, LPVOID param) {
  return /*BaseModuleImpl::Ioctl(control_id,param);*/YSOS_ERROR_SUCCESS;
}


int PluginBaseThreadModuleImpl::RealClose() {

  //取消注册驱动的事件
  if (driver_ptr_) {
    FunObjectCommon2 fb2;
    BaseModuleImpl *cur_module_ptr = this;
    fb2.pparam1 = cur_module_ptr;
    fb2.pparam2 = &string_output_abilities_;
    /* int event_call_ret =  */driver_ptr_->GetProperty(PROP_UNREGISTER_ABILITY_EVENT, &fb2);

    // 关闭
    driver_ptr_->Close(&driver_param_close_);

    driver_ptr_ = NULL;
  }
  return BaseThreadModuleImpl::RealClose();;
}

int PluginBaseThreadModuleImpl::RealStop() {
  // BaseModuleImpl::RealStop();

  /// 从输出能力列表遍历 停止所有的能力
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

int PluginBaseThreadModuleImpl::GetProperty(int iTypeId, void *piType) {
  BaseModuleImpl::GetProperty(iTypeId, piType);

  switch (iTypeId) {
  case PROP_FUN_CALLABILITY: {

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
      if (PROP_RUN != module_state
          || YSOS_ERROR_SUCCESS != igetstate_ret) {
        ///  如果 当前module 没有在运行 或获取状态失败，则输出结果都是空   ， callbck 拿到结果为空后则不会向下一个callback处理
        YSOS_LOG_DEBUG("current is not run (current state:%" << module_state <<" ) getsatatus fun return:" << igetstate_ret << " , call ability return null");
        if (pout && pout->buf_interface_ptr) {
          pout->buf_interface_ptr->SetLength(0);  ///<  设置长度为空
        }
        return /*YSOS_ERROR_SUCCESS*/YSOS_ERROR_SKIP;  ///<  通知callback ,module 已经停了，callback 也不要再往下传了
      }
    }


    ///< 数据类型转换
    if (list_module_datatype_convert_info_.size() >0) {
      assert(pin &&pout);
      std::string input_datatype = pin ? pin->data_type_name : "";
      std::string output_dataype = pout ? pout->data_type_name :"";
      ModuleDataTypeConvertInfo convert_info;
    #ifdef _WIN32
      if (NeedConvertDataTypeInfo(input_datatype, output_dataype, convert_info)) {
        if (convert_info.dest_input_datatype.length() >0 && pin) {
          strcpy_s(pin->data_type_name, sizeof(pin->data_type_name), convert_info.dest_input_datatype.c_str());
        }
        if (convert_info.dest_output_datatype.length() >0 && pout) {
          strcpy_s(pout->data_type_name, sizeof(pout->data_type_name), convert_info.dest_output_datatype.c_str());
        }
      }
    #else
      if (NeedConvertDataTypeInfo(input_datatype, output_dataype, convert_info)) {
        if (convert_info.dest_input_datatype.length() >0 && pin) {
          //strncpy(pin->data_type_name, convert_info.dest_input_datatype.c_str(), sizeof(pin->data_type_name) );
          strcpy(pin->data_type_name, convert_info.dest_input_datatype.c_str());
        }
        if (convert_info.dest_output_datatype.length() >0 && pout) {
          //strncpy(pout->data_type_name, convert_info.dest_output_datatype.c_str(), sizeof(pout->data_type_name));
          strcpy(pout->data_type_name, convert_info.dest_output_datatype.c_str());
        }
      }
    #endif
    }
    assert(driver_ptr_);
    if (NULL == driver_ptr_) {
      return YSOS_ERROR_FAILED;   ///<  表示不支持此调用
    }
    // 这里全权传发。
    int driver_ret= driver_ptr_->GetProperty(iTypeId, piType);
    if(YSOS_ERROR_SKIP == driver_ret) {
      return driver_ret;
    }
    {
      ///  如果 当前module 没有在运行，则输出结果都是空，  在能力调以前 判断一次，在能力调之后判断一次
      INT32 module_state = 0;
      int igetstate_ret = GetState(3000, &module_state, NULL);  ///<  3秒超时，如果拿不到module的状态，则报错
      if (YSOS_ERROR_SUCCESS != igetstate_ret) {
        YSOS_LOG_DEBUG("error my cant get module status in 3000 miliseconds");
        assert(false);
      }
      if (PROP_RUN != module_state
          || YSOS_ERROR_SUCCESS != igetstate_ret) {
        ///  如果 当前module 没有在运行 或获取状态失败，则输出结果都是空   ， callbck 拿到结果为空后则不会向下一个callback处理
        YSOS_LOG_DEBUG("current is not run (current state:%d" << module_state <<" ) getsatatus fun return:" << igetstate_ret << " , call ability return null");
        if (pout && pout->buf_interface_ptr) {
          pout->buf_interface_ptr->SetLength(0);  ///<  设置长度为空
        }
        return /*YSOS_ERROR_SUCCESS*/YSOS_ERROR_SKIP;  ///<  通知callback ,module 已经停了，callback 也不要再往下传了
      }
    }

    return driver_ret;
  }
  break;
  default:
    int ret = /*BaseModuleImpl*/BaseThreadModuleImpl::GetProperty(iTypeId, piType);
    return ret;
  }

}


bool PluginBaseThreadModuleImpl::NeedConvertDataTypeInfo(const std::string source_input_datatype, const std::string source_output_datatypa, ModuleDataTypeConvertInfo& convert_info) {
  assert(source_input_datatype.length() >0 || source_output_datatypa.length() >0);
  if (source_input_datatype.length() <=0 && source_output_datatypa.length() <=0)
    return false;

  std::list<ModuleDataTypeConvertInfo>::iterator it = list_module_datatype_convert_info_.begin();
#ifdef _WIN32
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
    /// 匹配上了
    YSOS_LOG_DEBUG("datatype info convert succ, source_input:"<< source_input_datatype <<" source_output:" <<source_output_datatypa <<"  dest_input:"<< pinfo->dest_input_datatype <<"  dest_output:" <<pinfo->dest_output_datatype);
    convert_info = *pinfo;
    return true;
  }
#else
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
    /// 匹配上了
    YSOS_LOG_DEBUG("datatype info convert succ, source_input:"<< source_input_datatype <<" source_output:" <<source_output_datatypa <<"  dest_input:"<< pinfo->dest_input_datatype <<"  dest_output:" <<pinfo->dest_output_datatype);
    convert_info = *pinfo;
    return true;
  }
#endif  
  return false;
}

int PluginBaseThreadModuleImpl::InitalDataInfo() {
  return YSOS_ERROR_SUCCESS;
}


//int PluginBaseThreadModuleImpl::Initialize(void *param) {
//  thread_data_->timeout = 4000;
//  return /*YSOS_ERROR_SUCCESS*/BaseThreadModuleImpl::Initialize(param);
//}

}
