/**
 *@file pluginbasemodulecallbackimpl.h
 *@brief Definition of face detect module
 *@version 2.0
 *@author libaoguo
 *@date Created on: 2017-08-25
 *@copyright Copyright(c) 2017 YunShen Technology. All rights reserved.
**/

#ifndef CALLBACK_PLUGIN_BASE_MODULE_CALLBACK_IMPL_H  //NOLINT
#define CALLBACK_PLUGIN_BASE_MODULE_CALLBACK_IMPL_H  //NOLINT

/// stl headers
#include <string>

/// boost headers
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/atomic.hpp>
#include <boost/thread/mutex.hpp>

/// ysos headers
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemoduleimpl.h"         //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemodulecallbackimpl.h" //NOLINT

/// private headers
#include "../../../public/include/common/commonstructdefine.h"
#include "../../../public/include/common/threadmsgqueue.h"


namespace ysos {

#define CALLBACK_MSG_EXIT           0
#define CALLBACK_MSG_REAL_CALLBACK  1

class MsgPluginBaseModuleCallback {
public:
  BufferInterfacePtr input_buffer;
  BufferInterfacePtr next_output_buffer;
  BufferInterfacePtr pre_output_buf;
  BaseModuelCallbackContext context;
};

class YSOS_EXPORT PluginBaseModuleCallbackImpl : public BaseModuleCallbackImpl {
  DECLARE_PROTECT_CONSTRUCTOR(PluginBaseModuleCallbackImpl);

 public:
  /**
   *@brief 回调处理是否准备好  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  **/
  int IsReady();

 protected:
   virtual int Initialized(const std::string &key, const std::string &value);
   int RealCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr next_output_buffer, BufferInterfacePtr pre_output_buf, void *context);
   int ConvertToTranstionCallbackDataType(AbilityParam* ability);
   virtual int UnInitialize(void *param=NULL);

 protected:
   int ThreadFunRealCallback();
   int RealCallbackChild(BufferInterfacePtr& input_buffer, BufferInterfacePtr& next_output_buffer, BufferInterfacePtr& pre_output_buf, /*void *context*/BaseModuelCallbackContext& context);

 protected:
   std::string input_ability_name_;
   std::string output_ability_name_;

   int output_type_to_tcallback_;
   std::string output_type_to_tcallback_type_;

   /// 是否异步传输数据，如果是不是（表示同步），则等下一个callback完成后再返回
   /// 如果是，则表示异步，则对于inputbuf会创建一个临时拷贝，oubput不再创建临时拷贝，同时创建一个线程去处理数据
   /// 直到下一次本callback触发，此线程会被中断
   /// 默认值是0 表示是同步
   bool asynchronize_;  ///< 这里不使用atomic，为了提高性能
   boost::mutex mutex_asynchronize_thread_;
   boost::shared_ptr<boost::thread> asynchronize_thread_;
   boost::atomic<int> asynchronize_stop_flag_;

   ThreadMsgPriorityQueue<MsgPluginBaseModuleCallback> queue_msg_;   ///< 如果是异步处理，则使用线程队列来处理
   bool nextdata_returnto_prebuf_;  ///< callback输出结果是否回传到前面，默认是false， 如果是，则callback中会把outputbuf中的数据回传exteral_buf中
   int log_result_;                 ///< 打印日志,如果0 表示不打印,如果为 1 表示打断outputbuf中数据, 默认是0 ,注意，只有数据格式为text时 才输出日志，否则会出错
   int max_loop_time_;              ///< 最大callback次数，默认是1，只调一次，如果大于1，则表示多调一次，只到到达最大回调次数，或显式的说明数据传完了,0 表示无限制
};
 
} // namespace ysos

#endif    // CALLBACK_PLUGIN_BASE_MODULE_CALLBACK_IMPL_H  //NOLINT