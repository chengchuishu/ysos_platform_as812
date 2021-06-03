/**
 *@file strategyinterface.h
 *@brief Definition of strategy
 *@version 0.1
 *@author Steven.Shi
 *@date Created on: 2016-04-13 08:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef SFP_STRATEGY_INTERFACE_H  //NOLINT
#define SFP_STRATEGY_INTERFACE_H  //NOLINT

#include <boost/shared_ptr.hpp>
#include "../../../public/include/sys_interface_package/common.h"
#include "../../../public/include/sys_interface_package/baseinterface.h"
#include "../../../public/include/sys_interface_package/msginterface.h"
#include "../../../public/include/sys_interface_package/moduleinterface.h"
/*#include "../../../public/include/sys_interface_package/messagehead.h"*/

namespace ysos {
class ControlInterface;
class BaseInterface;
class YSOS_EXPORT IoInfoInterface: virtual public BaseInterface {
  IMPLEMENT_PUBLIC_EMPTY_DESTRUCTOR(IoInfoInterface)
 public:
  enum IoInfoInterface_ProPerty {
    IOINFO_CALLBACK = 0,
    IoInfoInterface_ProPerty_End
  };
  /**
    *@brief  对输入的数据进行匹配如果匹配，将数据添加进Msg_Queue_，同时返回True，否则返回False   //  NOLINT
    *@param  input_ptr 待匹配的数据   //  NOLINT
    *@param  output_ptr 过滤后的结果，如果不需要返回值，可以为NULL  //  NOLINT
    *@param  context_ptr 上下文，可以为NULL //  NOLINT
    *@return 成功返回true，否则返回false  //  NOLINT
    */
  virtual bool IsMatched(BufferInterfacePtr input_buffer, BufferInterfacePtr ouput_buffer, void *context_ptr) = 0;
  //virtual int PreProcess(BufferInterfacePtr input_buffer, BufferInterfacePtr ouput_buffer, void *context_ptr) = 0;
};
typedef boost::shared_ptr<IoInfoInterface> IoInfoInterfacePtr;

class YSOS_EXPORT TransitionInfoInterface: virtual public BaseInterface {
  IMPLEMENT_PUBLIC_EMPTY_DESTRUCTOR(TransitionInfoInterface)
 public:
  /**
    *@brief  对输入的数据进行过滤   //  NOLINT
    *@param  input_ptr 待过滤的数据   //  NOLINT
    *@param  output_ptr 过滤后的结果，如果不需要返回值，可以为NULL  //  NOLINT
    *@param  context_ptr 上下文，可以为NULL //  NOLINT
    *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
    */
  virtual int Filter(BufferInterfacePtr input_ptr, BufferInterfacePtr output_ptr=NULL, void *context_ptr=NULL) = 0;
  /**
    *@brief  对输入的数据进行匹配   //  NOLINT
    *@param  input_ptr 待匹配的数据   //  NOLINT
    *@param  output_ptr 过滤后的结果，如果不需要返回值，可以为NULL  //  NOLINT
    *@param  context_ptr 上下文，可以为NULL //  NOLINT
    *@return 成功返回true，否则返回false  //  NOLINT
    */
  virtual bool IsMatched(BufferInterfacePtr input_ptr, BufferInterfacePtr output_ptr=NULL, void *context_ptr=NULL) = 0;
  /**
    *@brief 获取消息头，由ioinfo用消息头封闭数据
    *@param  input_ptr 待匹配的数据   //  NOLINT
    *@param  output_ptr 过滤后的结果，如果不需要返回值，可以为NULL  //  NOLINT
    *@param  context_ptr 上下文，可以为NULL //  NOLINT
    *@return 成功返回消息ID，失败返回0  //  NOLINT
    */
  virtual uint32_t GetMessageID(BufferInterfacePtr input_ptr, BufferInterfacePtr output_ptr=NULL, void *context_ptr=NULL) = 0;
  /**
  *@brief 添加消息进Strategy MessageQueue中
  *@param  message_ptr 待添加的数据   //  NOLINT
  *@param  context_ptr 上下文，可以为NULL //  NOLINT
  *@return 成功返回头，失败返回NULL  //  NOLINT
  */
  virtual int AddMessageToQueue(MsgInterfacePtr message_ptr, void *context_ptr=NULL) = 0;
  ///**
  //*@brief 注册Repeater到TransitionInfo中对应的IoInfo里
  //*@param  Repeater_Ptr 待注册的Repeater   //  NOLINT
  //*@param  context_ptr 上下文，可以为NULL //  NOLINT
  //*@return 成功返回0，失败返回其他L  //  NOLINT
  //*/
  //virtual int RegisterRepeater(RepeaterInterfacePtr &repeater_ptr, void *context_ptr=NULL) = 0;
};
typedef boost::shared_ptr<TransitionInfoInterface> TransitionInfoInterfacePtr;

/**
 *@brief 策略接口，主要针对用户场景实现应对策略
   策略接口，继承自Module_Interface，
   用于实现某一种场景下的状态，
   基本实现为内部维持一个状态图模式，
   复杂情况下可实现其他决策类算法逻辑。
   内部会维护多个ModuleLink_Interface，接收其结果的同时，对这些接口进行控制
 */
class YSOS_EXPORT StrategyInterface : virtual public ControlInterface, virtual public BaseInterface {
  IMPLEMENT_PUBLIC_EMPTY_DESTRUCTOR(StrategyInterface)
 public:

  /**
   *@brief 加载策略逻辑  // NOLINT
   *@param strategy_name[Input]： 策略名称  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int LoadStrategy(const std::string &strategy_name) = 0;

  /**
   *@brief 创建策略逻辑  // NOLINT
   *@param strategy_name[Input]： 策略名称  // NOLINT
   *@param params[Input]： 策略所需的参数  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int CreateStrategy(const std::string &strategy_name, void *params) = 0;

  /**
   *@brief 销毁策略逻辑  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int DestroyStrategy() = 0;

  /**
   *@brief 保存策略逻辑  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int SaveStrategy() = 0;
};

typedef boost::shared_ptr<StrategyInterface> StrategyInterfacePtr;

} // namespace ysos

#endif  // STRATEGY_INTERFACE_H  //NOLINT
