/**
 *@file nlpextcallback.h
 *@brief the nlp ext call back
 *@version 1.0
 *@author l.j..
 *@date Created on: 2016-10-20 15:16:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

#ifndef NLPEXT_CALLBACK_H_
#define NLPEXT_CALLBACK_H_

/// boost headers
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/atomic.hpp>
#include <boost/thread/mutex.hpp>

/// Private Headers //  NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemoduleimpl.h"  //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemodulecallbackimpl.h"  //NOLINT
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"

namespace ysos {

/**
 *@brief  语音模块回调层 //NOLINT
*/
class YSOS_EXPORT NlpExtCallback : public BaseModuleCallbackImpl {
  DECLARE_CREATEINSTANCE(NlpExtCallback);
  DISALLOW_COPY_AND_ASSIGN(NlpExtCallback);
  DECLARE_PROTECT_CONSTRUCTOR(NlpExtCallback);

 public:
  virtual ~NlpExtCallback(void);
  /**
   *@brief 回调处理是否准备好  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  int IsReady();

 protected:
  /**
  *@brief 回调接口的实现函数  // NOLINT
  *@param input_buffer[Input]： 输入数据缓冲  // NOLINT
  *@param output_buffer[Output]：回调处理后输出的数据缓冲  // NOLINT
  *@param context[Input]： 回调处理的上下文数据  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int RealCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr cur_output_buffer, BufferInterfacePtr output_buffer, void *context);

  /**
  *@brief  配置文件中设置的Key、Value对，会在Initialized函数中调用 //  NOLINT
  *@param key 配置参数中的Key  // NOLINT
  *@param value 与Key对应的Value  // NOLINT
  *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
  */
  virtual int Initialized(const std::string &key, const std::string &value);

private:
    std::string input_ability_name_;
    std::string output_ability_name_;

    int output_type_to_tcallback_;  ///< 是否需要转成transition callback的格式
    std::string output_type_to_tcallback_type_;

    //< 是否异步传输数据，如果是不是（表示同步），则等下一个callback完成后再返回
    //< 如果是，则表示异步，则对于inputbuf会创建一个临时拷贝，oubput不再创建临时拷贝，同时创建一个线程去处理数据
    //< 直到下一次本callback触发，此线程会被中断
    //< 默认值是0 表示是同步
    bool asynchronize_;  ///< 这里不使用atomic，为了提高性能
    boost::mutex mutex_asynchronize_thread_;
    boost::shared_ptr<boost::thread> asynchronize_thread_;
    boost::atomic<int> asynchronize_stop_flag_;

    bool nextdata_returnto_prebuf_;  //< callback输出结果是否回传到前面，默认是false， 如果是，则callback中会把outputbuf中的数据回传exteral_buf中
    int log_result_;                 //< 打印日志,如果0 表示不打印,如果为 1 表示打断outputbuf中数据, 默认是0 ,注意，只有数据格式为text时 才输出日志，否则会出错
    int max_loop_time_;              //< 最大callback次数，默认是1，只调一次，如果大于1，则表示多调一次，只到到达最大回调次数，或显式的说明数据传完了,0 表示无限制

};

}///< namespace ysos    //NOLINT

#endif///< NLPEXT_CALLBACK_H_    //NOLINT