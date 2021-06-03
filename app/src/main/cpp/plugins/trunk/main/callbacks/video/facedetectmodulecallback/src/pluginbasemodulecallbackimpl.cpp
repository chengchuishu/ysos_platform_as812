/**
 *@file pluginbasemodulecallbackimpl.cpp
 *@brief Definition of face detect module
 *@version 2.0
 *@author libaoguo
 *@date Created on: 2017-08-25
 *@copyright Copyright(c) 2017 YunShen Technology. All rights reserved.
**/

#include "../../../public/include/common/pluginbasemodulecallbackimpl.h"

#include <list>

// boost headers
#include <boost/thread/thread.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/locale.hpp>
#include <boost/thread/lock_guard.hpp>

// ysos headers
#include "../../../../../../core/trunk/main/public/include/os_hal_package/lock.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"

// third party ifly headers
//#include <ifly/include/qisr.h>
//#include <ifly/include/msp_cmn.h>
//#include <ifly/include/msp_errors.h>

// private headers
#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"
#include "../../../public/include/common/threadmsgqueue.h"

#define MAX_MSG_BUFFER_COUNT  3

namespace ysos {
#ifdef _WIN32
  //TODO: depand windows api
#else
  int memcpy_s(void *det, size_t detSize, const void * src, size_t srcSize)
  {
      uint8_t errorcode = 0;
      if (srcSize > detSize || src == NULL || det == NULL)
      {
          if (srcSize > detSize)
              errorcode = 1;
          else if (src == NULL)
              errorcode = 2;
          else if (det == NULL)
              errorcode = 3;
          printf("[waring] memcpy_s is error:%d [%ld---%ld]!\n", errorcode, srcSize, detSize);
          fflush(stdout);
          return -1;
      } else {
        memcpy(det, src, srcSize);
      }
      return 1;
  }
#endif
  
PluginBaseModuleCallbackImpl::PluginBaseModuleCallbackImpl(const std::string &strClassName):BaseModuleCallbackImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.video");
  output_type_to_tcallback_ = false;
  asynchronize_ = true;
  asynchronize_thread_ = NULL;
  asynchronize_stop_flag_ =0;
  nextdata_returnto_prebuf_ = false;
  log_result_ =0;
  max_loop_time_ =1;
}

int PluginBaseModuleCallbackImpl::IsReady() {
  return YSOS_ERROR_SUCCESS;
}

int PluginBaseModuleCallbackImpl::Initialized(const std::string &key, const std::string &value) {
  BaseModuleCallbackImpl::Initialized(key, value);
#ifdef _WIN32
  if (stricmp(key.c_str(), "input_type") == 0) {
    input_type_ = value;
  } else if (stricmp(key.c_str(), "output_type") == 0) {
    output_type_ = value;
  } else if (stricmp(key.c_str(), "input_ability") == 0) {
    input_ability_name_ = value;
  } else if (stricmp(key.c_str(), "output_ability") == 0) {
    output_ability_name_ = value;
  } else if (stricmp(key.c_str(), "output_type_to_tcallback") ==0) {
    if (value.length() > 0) {
      std::list<std::string> list_params;
      GetUtility()->SplitString(value, "|", list_params);
      while (list_params.size() >= 2) {
        std::string temp_string1 = *list_params.begin();
        list_params.pop_front();
        std::string temp_string2 = *list_params.begin();
        list_params.pop_front();
        if (stricmp("type", temp_string1.c_str()) == 0) {
          output_type_to_tcallback_ = true;
          output_type_to_tcallback_type_ = temp_string2;
        }
      }
    }
  } else if (stricmp(key.c_str(), "asynchronize") == 0) {
    if (value.length() > 0) {
      asynchronize_ = atoi(value.c_str()) > 0 ? true : false;
    } else {
      asynchronize_ = false;
    }

    if (asynchronize_) {
      boost::lock_guard<boost::mutex> lock_guard_asynchronize(mutex_asynchronize_thread_);
      assert(NULL == asynchronize_thread_);
      asynchronize_stop_flag_ =0;
      asynchronize_thread_ = boost::shared_ptr<boost::thread>(new boost::thread(
        boost::bind(&PluginBaseModuleCallbackImpl::ThreadFunRealCallback, this)));
      if (NULL == asynchronize_thread_)
        return YSOS_ERROR_FAILED;
    }
  } else if (stricmp(key.c_str(), "nextdata_returnto_prebuf") ==0) {
    if (value.length() >0) {
      nextdata_returnto_prebuf_ = atoi(value.c_str()) >0 ? true : false;
    } else {
      nextdata_returnto_prebuf_ = false;
    }
  } else if (stricmp(key.c_str(), "log_result") ==0) {
    if (value.length() >0) {
      log_result_ = atoi(value.c_str()) >0 ? true : false;
    } else {
      log_result_ = 0;
    }
  } else if (stricmp(key.c_str(), "max_loop_time") ==0) {
    if (value.length() >0) {
      max_loop_time_ = atoi(value.c_str());
    } else {
      max_loop_time_ = 1;
    }
  }
#else
  if (strcasecmp(key.c_str(), "input_type") == 0) {
    input_type_ = value;
  } else if (strcasecmp(key.c_str(), "output_type") == 0) {
    output_type_ = value;
  } else if (strcasecmp(key.c_str(), "input_ability") == 0) {
    input_ability_name_ = value;
  } else if (strcasecmp(key.c_str(), "output_ability") == 0) {
    output_ability_name_ = value;
  } else if (strcasecmp(key.c_str(), "output_type_to_tcallback") ==0) {
    if (value.length() > 0) {
      std::list<std::string> list_params;
      GetUtility()->SplitString(value, "|", list_params);
      while (list_params.size() >= 2) {
        std::string temp_string1 = *list_params.begin();
        list_params.pop_front();
        std::string temp_string2 = *list_params.begin();
        list_params.pop_front();
        if (strcasecmp("type", temp_string1.c_str()) == 0) {
          output_type_to_tcallback_ = true;
          output_type_to_tcallback_type_ = temp_string2;
        }
      }
    }
  } else if (strcasecmp(key.c_str(), "asynchronize") == 0) {
    if (value.length() > 0) {
      asynchronize_ = atoi(value.c_str()) > 0 ? true : false;
    } else {
      asynchronize_ = false;
    }

    if (asynchronize_) {
      boost::lock_guard<boost::mutex> lock_guard_asynchronize(mutex_asynchronize_thread_);
      assert(NULL == asynchronize_thread_);
      //printf("NULL == asynchronize_thread_ !!! \n");//add for linux
      asynchronize_stop_flag_ = 0;
      asynchronize_thread_ = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&PluginBaseModuleCallbackImpl::ThreadFunRealCallback, this)));
      if (NULL == asynchronize_thread_)
        return YSOS_ERROR_FAILED;
    }
  } else if (strcasecmp(key.c_str(), "nextdata_returnto_prebuf") ==0) {
    if (value.length() >0) {
      nextdata_returnto_prebuf_ = atoi(value.c_str()) >0 ? true : false;
    } else {
      nextdata_returnto_prebuf_ = false;
    }
  } else if (strcasecmp(key.c_str(), "log_result") ==0) {
    if (value.length() >0) {
      log_result_ = atoi(value.c_str()) >0 ? true : false;
    } else {
      log_result_ = 0;
    }
  } else if (strcasecmp(key.c_str(), "max_loop_time") ==0) {
    if (value.length() >0) {
      max_loop_time_ = atoi(value.c_str());
    } else {
      max_loop_time_ = 1;
    }
  }
#endif
  YSOS_LOG_DEBUG("logic name:" << logic_name_ << " input_type:" << input_type_ <<"  output_type:" << output_type_);
  return YSOS_ERROR_SUCCESS;
}

int PluginBaseModuleCallbackImpl::UnInitialize(void *param) {
  boost::lock_guard<boost::mutex> lock_guard_asynchronize(mutex_asynchronize_thread_);
  if (NULL != asynchronize_thread_) {
    asynchronize_thread_->interrupt();
    asynchronize_stop_flag_ =1;
    if (asynchronize_thread_->joinable()) {
      asynchronize_thread_->join();
    }
  }

  return YSOS_ERROR_SUCCESS;
}

int PluginBaseModuleCallbackImpl::RealCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr next_output_buffer, 
  BufferInterfacePtr pre_output_buf, void *context) {
  BaseModuelCallbackContext* module_callback_context_ptr = NULL;
  module_callback_context_ptr = reinterpret_cast<BaseModuelCallbackContext*>(context);
  assert(module_callback_context_ptr);
  if (NULL == module_callback_context_ptr) {
    YSOS_LOG_DEBUG("error the conte is null");
    return YSOS_ERROR_FAILED;
  }

  if (!asynchronize_) {
    int iret= RealCallbackChild(input_buffer, next_output_buffer, pre_output_buf, /*context*/*module_callback_context_ptr);
    return iret;
  } else {
    BufferInterfacePtr temp_input_buffer = NULL;
    BufferInterfacePtr temp_next_output_buffer = NULL;

    UINT8* source_data = NULL;
    UINT32 source_data_size =0;
    UINT8 *temp_buffer_data =NULL;
    UINT32 temp_buffer_length =0;

    if (input_buffer) {
      input_buffer->GetBufferAndLength(&source_data,&source_data_size);
    }

    if (source_data_size >0) {
      temp_input_buffer = GetBufferUtility()->CreateBuffer(/*buffer_length*/source_data_size);
      temp_input_buffer->GetBufferAndLength(&temp_buffer_data, &temp_buffer_length);
      if (NULL == temp_buffer_data) {
        return YSOS_ERROR_FAILED;
      }

      memcpy_s(temp_buffer_data, temp_buffer_length, source_data, source_data_size);
    }

    source_data = NULL;
    source_data_size = 0;
    temp_buffer_data = NULL;
    temp_buffer_length = 0;
    if (next_output_buffer) {
      next_output_buffer->GetBufferAndLength(&source_data, &source_data_size);
    }

    if (source_data_size >0) {
      temp_next_output_buffer = GetBufferUtility()->CreateBuffer(source_data_size);
      temp_next_output_buffer->GetBufferAndLength(&temp_buffer_data, &temp_buffer_length);
      if (NULL == temp_buffer_data) {
        return YSOS_ERROR_FAILED;
      }

      memcpy_s(temp_buffer_data, temp_buffer_length, source_data, source_data_size);
    }

    MsgPluginBaseModuleCallback msg_value;
    msg_value.input_buffer = temp_input_buffer;
    msg_value.next_output_buffer = temp_next_output_buffer;
    msg_value.pre_output_buf = pre_output_buf;
    msg_value.context = *module_callback_context_ptr;

    if (queue_msg_.GetMsgCount() >= MAX_MSG_BUFFER_COUNT) {
      queue_msg_.RemoveMsgAccordMessageType(CALLBACK_MSG_REAL_CALLBACK);
    }

    queue_msg_.PushMsg(CALLBACK_MSG_REAL_CALLBACK, ThreadMsgPriorityCommon, &msg_value);
    return YSOS_ERROR_SKIP;
  }
}

int PluginBaseModuleCallbackImpl::ThreadFunRealCallback() {
  while (0 == asynchronize_stop_flag_) {
    boost::this_thread::interruption_point();
#ifdef _WIN32
    DWORD dwWaitRet = WaitForSingleObject(queue_msg_.event_thread(), 1000);// add for linux
    if (WAIT_OBJECT_0 != dwWaitRet){
        continue;
    }
#else
    //TODO:add for linux
    int ret = event_timedwait(queue_msg_.event_thread(), 1000);
    if (0 != ret){
        continue;
    }
#endif
      
    YSOS_LOG_DEBUG(this->logic_name_ << "ThreadFunRealCallback");
    boost::this_thread::interruption_point();
    unsigned int msg_type = 0;
    MsgPluginBaseModuleCallback msg_value;
    bool bhave_msg = queue_msg_.PopMsg(msg_type, msg_value);
    if (false == bhave_msg)
      continue;

    boost::this_thread::interruption_point();
    switch (msg_type) {
    case CALLBACK_MSG_EXIT: {
      YSOS_LOG_DEBUG(this->logic_name_ <<" exit msg thread 1");
      return YSOS_ERROR_SUCCESS;
    }

    case CALLBACK_MSG_REAL_CALLBACK: {
      YSOS_LOG_DEBUG(this->logic_name_ <<" CALLBACK_MSG_REAL_CALLBACK");
      RealCallbackChild(msg_value.input_buffer, msg_value.next_output_buffer, msg_value.pre_output_buf, msg_value.context);
    }
    break;
    }
  }

  YSOS_LOG_DEBUG(this->logic_name_ <<" exit msg thread 2");
  return YSOS_ERROR_SUCCESS;
}

int PluginBaseModuleCallbackImpl::RealCallbackChild(BufferInterfacePtr& input_buffer, BufferInterfacePtr& next_output_buffer, BufferInterfacePtr& pre_output_buf, /*void *context*/BaseModuelCallbackContext& context) {
  BaseModuelCallbackContext* module_callback_context_ptr = &context;
  BaseInterface* pModule= dynamic_cast<BaseInterface*>(module_callback_context_ptr->cur_module_ptr);
  YSOS_LOG_DEBUG("PluginBaseModuleCallbackImpl::RealCallbackChild [Enter] ");

  assert(pModule);
  
  AbilityParam input_param, out_param;
#ifdef _WIN32
  sprintf_s(input_param.ability_name, sizeof(input_param.ability_name), input_ability_name_.c_str());
  sprintf_s(input_param.data_type_name, sizeof(input_param.data_type_name), input_type_.c_str());
  input_param.buf_interface_ptr = input_buffer;
  sprintf_s(out_param.ability_name, sizeof(out_param.ability_name), output_ability_name_.c_str());
  sprintf_s(out_param.data_type_name, sizeof(out_param.data_type_name), output_type_.c_str());
  out_param.buf_interface_ptr = next_output_buffer;
#else
  snprintf(input_param.ability_name, sizeof(input_param.ability_name), "%10s", input_ability_name_.c_str());
  snprintf(input_param.data_type_name, sizeof(input_param.data_type_name), "%10s", input_type_.c_str());
  input_param.buf_interface_ptr = input_buffer;
  snprintf(out_param.ability_name, sizeof(out_param.ability_name), "%10s", output_ability_name_.c_str());
  snprintf(out_param.data_type_name, sizeof(out_param.data_type_name), "%10s", output_type_.c_str());
  out_param.buf_interface_ptr = next_output_buffer;
#endif

  FunObjectCommon2 funobject2;
  funobject2.pparam1 = &input_param;
  funobject2.pparam2 = &out_param;
  boost::this_thread::interruption_point();

  int iloop_max_times = max_loop_time_;
  for (int i = 0; 0 == iloop_max_times || i< iloop_max_times; ++i) {
    boost::this_thread::interruption_point();
    bool is_run = IsCurModuleRun(module_callback_context_ptr->cur_module_ptr, module_callback_context_ptr->prev_module_ptr);
    if (!is_run) {
      YSOS_LOG_ERROR("cur module is not run now: " << pModule->GetName() << " | " << logic_name_);
      return YSOS_ERROR_SUCCESS;
    }
    YSOS_LOG_DEBUG("PluginBaseModuleCallbackImpl::RealCallbackChild cur module is run now: " << pModule->GetName() << " | " << logic_name_);
    boost::this_thread::interruption_point();
    int iret= pModule->GetProperty(PROP_FUN_CALLABILITY, &funobject2);
    boost::this_thread::interruption_point();
    if (YSOS_ERROR_SUCCESS != iret) {
      YSOS_LOG_ERROR("error, call PROP_FUN_CALLABILITY ret: " << iret);
      return iret;
    }
    YSOS_LOG_DEBUG("PluginBaseModuleCallbackImpl::RealCallbackChild call PROP_FUN_CALLABILITY ret: " << iret);
    if (NULL != out_param.buf_interface_ptr) {
      UINT8* out_data = NULL;
      UINT32 data_size =0;
      out_param.buf_interface_ptr->GetBufferAndLength(&out_data, &data_size);
      if (data_size > 0) {
        if (1 == log_result_) {
          YSOS_LOG_DEBUG(this->logic_name_ <<"  data: [" << (char*)out_data << "]");
        }

        if (nextdata_returnto_prebuf_) {
          if (NULL != pre_output_buf) {
            uint8_t* buffer_data_ptr = GetBufferUtility()->GetBufferData(pre_output_buf);
            UINT32 buffer_length = GetBufferUtility()->GetBufferUsableLength(pre_output_buf);
            if (data_size <= buffer_length - 1) {
              memcpy(buffer_data_ptr, out_data, data_size);
              buffer_data_ptr[data_size] = '\0';
            }
          }
        }
      }
#ifdef _WIN32
      if (output_type_.length() <= 0 || stricmp(output_type_.c_str(), "null") == 0) {
        return YSOS_ERROR_SKIP;
      }
#else
      if (output_type_.length() <= 0 || strcasecmp(output_type_.c_str(), "null") == 0) {
        return YSOS_ERROR_SKIP;
      }
#endif
      if (0 == data_size) { ///< 表示解析成功，但当前没有数据，所以返因true
        if (0 == out_param.is_not_finish) ///< have finish all data
          break;
        else
          continue;
      }
    }

    boost::this_thread::interruption_point();
    if (output_type_to_tcallback_) {
      ConvertToTranstionCallbackDataType(&out_param);
    }

    boost::this_thread::interruption_point();
    iret = InvokeNextCallback(out_param.buf_interface_ptr, NULL, /*context*/module_callback_context_ptr);
    YSOS_LOG_DEBUG("cur module is run now: " << pModule->GetName() << " | InvokeNextCallback: [iret] = " << iret);
    if (0 == out_param.is_not_finish) {
      break;
    }
  }
  YSOS_LOG_DEBUG("PluginBaseModuleCallbackImpl::RealCallbackChild [End] ");
  return YSOS_ERROR_SKIP;
}

int PluginBaseModuleCallbackImpl::ConvertToTranstionCallbackDataType(AbilityParam* ability) {
  assert(ability);

  if (NULL == ability)
    return YSOS_ERROR_FAILED;

  assert(output_type_to_tcallback_ && output_type_to_tcallback_type_.length() > 0);
  YSOS_LOG_DEBUG("PluginBaseModuleCallbackImpl::ConvertToTranstionCallbackDataType  output_type_to_tcallback_ is:"<< output_type_to_tcallback_);
  YSOS_LOG_DEBUG("PluginBaseModuleCallbackImpl::ConvertToTranstionCallbackDataType  output_type_to_tcallback_type_.length() is:"<< output_type_to_tcallback_type_.length());

  try {
    UINT8* ptext_u8 = NULL;
    UINT32 buffer_size = 0;
    ability->buf_interface_ptr->GetBufferAndLength(&ptext_u8,&buffer_size);
    char* ptext = reinterpret_cast<char*>(ptext_u8);
#ifdef _WIN32    
    assert(ptext);
    boost::property_tree::ptree ptree_root;
    ptree_root.put("type", output_type_to_tcallback_type_);
    if (NULL == ptext) {
      ptree_root.put("data", "");
    } else if (stricmp(ability->data_type_name, DTSTR_TEXT_JSON) == 0) {
      std::istringstream istring(boost::locale::conv::to_utf<char>(ptext ? ptext : "", "gbk"));
      boost::property_tree::ptree tree_soure, tree_empty;
      boost::property_tree::read_json(istring, tree_soure);
      ptree_root.add_child("data", tree_soure);
    } else if (stricmp(ability->data_type_name, DTSTR_TEXT_PURE) == 0 || 
      stricmp(ability->data_type_name, DTSTR_TEXT_XML) == 0) {
      ptree_root.put("data", boost::locale::conv::to_utf<char>(ptext ? ptext : "", "gbk"));
    } else {
      //assert(false);
      YSOS_LOG_DEBUG("error not support convert to transition callback type ,current is output datatype is: " 
        << ability->data_type_name);
      return YSOS_ERROR_FAILED;
    }
#else
  assert(ptext);
  boost::property_tree::ptree ptree_root;
  ptree_root.put("type", output_type_to_tcallback_type_);
  if (NULL == ptext) {
    ptree_root.put("data", "");
  } else if (strcasecmp(ability->data_type_name, DTSTR_TEXT_JSON) == 0) {
    std::istringstream istring(boost::locale::conv::to_utf<char>(ptext ? ptext : "", "gbk"));
    boost::property_tree::ptree tree_soure, tree_empty;
    boost::property_tree::read_json(istring, tree_soure);
    ptree_root.add_child("data", tree_soure);
  } else if (strcasecmp(ability->data_type_name, DTSTR_TEXT_PURE) == 0 || 
    strcasecmp(ability->data_type_name, DTSTR_TEXT_XML) == 0) {
    ptree_root.put("data", boost::locale::conv::to_utf<char>(ptext ? ptext : "", "gbk"));
  } else {
    assert(false);
    YSOS_LOG_DEBUG("error not support convert to transition callback type ,current is output datatype is: " << ability->data_type_name);
    return YSOS_ERROR_FAILED;
  }
#endif
    YSOS_LOG_DEBUG("PluginBaseModuleCallbackImpl::ConvertToTranstionCallbackDataType  data_type_name is:"<< output_type_to_tcallback_);
    std::ostringstream omystring;
    boost::property_tree::write_json(omystring, ptree_root);
    std::string ansi_string = boost::locale::conv::from_utf(omystring.str(), "gbk");
    YSOS_LOG_DEBUG("PluginBaseModuleCallbackImpl::ConvertToTranstionCallbackDataType  ansi_string is:"<< ansi_string);
    size_t out_buffer_size = 0;
    UINT32 max_length = 0, prefix_length =0;
    ability->buf_interface_ptr->GetMaxLength(&max_length);
    ability->buf_interface_ptr->GetPrefixLength(&prefix_length);
    out_buffer_size = max_length - prefix_length;
    if (ansi_string.length() > out_buffer_size -1) {
      YSOS_LOG_DEBUG("error output buf is to small, cant package all the string, outbufsize:" << out_buffer_size << " need store string;"<< ansi_string);
      //assert(false);
      return YSOS_ERROR_FAILED;
    }
    YSOS_LOG_DEBUG("PluginBaseModuleCallbackImpl::ConvertToTranstionCallbackDataType   outbufsize:" << out_buffer_size << " need store string;"<< ansi_string);
#ifdef _WIN32
    strcpy_s(ptext, out_buffer_size, ansi_string.c_str());
#else
    strcpy(ptext, ansi_string.c_str());
#endif    
    ability->buf_interface_ptr->SetLength(ansi_string.length()+1);
    YSOS_LOG_DEBUG("PluginBaseModuleCallbackImpl::ConvertToTranstionCallbackDataType [End] ");
    return YSOS_ERROR_SUCCESS;
  } catch (...) {
    YSOS_LOG_DEBUG("error decode json ");
    return YSOS_ERROR_FAILED;
  }
}

}
