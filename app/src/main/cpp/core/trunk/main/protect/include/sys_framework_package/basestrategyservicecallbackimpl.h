/**
 *@file BaseStrategyServiceCallbackImpl.h
 *@brief ״̬��ͨ��Callback����,����StateMachine��StateMachine�¼���Service�ص���
 *@version 1.0
 *@author dhongqian
 *@date Created on: 2016-05-11 10:06:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

#ifndef SFP_BASE_STRATEGY_SERVICE_CALLBACK_IMPL_H_  //NOLINT
#define SFP_BASE_STRATEGY_SERVICE_CALLBACK_IMPL_H_  //NOLINT

/// STL Headers
#include <map>
/// ysos private headers //  NOLINT
#include "../../../protect/include/sys_framework_package/basecallbackimpl.h"
#include "../../../public/include/sys_interface_package/moduleinterface.h"

namespace ysos {
typedef ModuleInterface   RepeaterInterface;
typedef ModuleInterfacePtr   RepeaterInterfacePtr;
class YSOS_EXPORT BaseStrategyServiceCallbackImpl : public  BaseCallbackImpl {
  DECLARE_CREATEINSTANCE(BaseStrategyServiceCallbackImpl);
  DISALLOW_COPY_AND_ASSIGN(BaseStrategyServiceCallbackImpl);
  DECLARE_PROTECT_CONSTRUCTOR(BaseStrategyServiceCallbackImpl);

 public:
  virtual ~BaseStrategyServiceCallbackImpl();

  struct RepeaterInfo {
    std::string           real_name;
    RepeaterInterfacePtr  repeater_ptr;
  };
  typedef boost::shared_ptr<RepeaterInfo>                RepeaterInfoPtr;
  typedef std::map<std::string, RepeaterInfoPtr >   RepeaterMap;

  /**
   *@brief �����Դ,Initialize�������  // NOLINT
   *@param param�� ��ʼ���Ĳ��� // NOLINT
   *@return�� �ɹ�����YSOS_ERROR_SUCCESS��ʧ�ܷ�����Ӧ����ֵ  // NOLINT
   */
  virtual int UnInitialize(void *param=NULL);

 protected:
  /**
   *@brief ͨ��service�����ƣ����Repeaterָ�� // NOLINT
   *@param service_name[input]�� ��Ҫ�ķ�����  // NOLINT
   *@return�� �ɹ�����Repeaterָ�룬ʧ�ܷ��ؿ�  // NOLINT
   */
  RepeaterInfoPtr GetRepeaterByServiceName(const std::string &service_name);

 protected:
  RepeaterMap                         repeater_map_;    ///< �洢���Calllback���õ���Repeater
};
}  /// namespace ysos //  NOLINT

#endif  ///<  SFP_BASE_STRATEGY_SERVICE_CALLBACK_IMPL_H_  //  NOLINT