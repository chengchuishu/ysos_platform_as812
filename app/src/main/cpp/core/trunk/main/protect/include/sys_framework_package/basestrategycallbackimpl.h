/**
 *@file BaseStrategyCallbackImpl.h
 *@brief ״̬��ͨ��Callback����,����StateMachine���¼���Service�ص�
 *@version 1.0
 *@author dhongqian
 *@date Created on: 2016-05-11 10:06:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

#ifndef SFP_BASE_STRATEGY_CALLBACK_IMPL_H_  //NOLINT
#define SFP_BASE_STRATEGY_CALLBACK_IMPL_H_  //NOLINT

#include "../../../protect/include/sys_framework_package/basestrategyservicecallbackimpl.h"
#include "../../../protect/include/sys_framework_package/platformrpcentity.h"

namespace ysos {

class YSOS_EXPORT BaseStrategyCallbackImpl : public  BaseStrategyServiceCallbackImpl {
  DECLARE_CREATEINSTANCE(BaseStrategyCallbackImpl);
  DISALLOW_COPY_AND_ASSIGN(BaseStrategyCallbackImpl);
  DECLARE_PROTECT_CONSTRUCTOR(BaseStrategyCallbackImpl);

 public:
  virtual ~BaseStrategyCallbackImpl();

  /**
  *@brief �ص��ӿڵ�ʵ�ֺ���  // NOLINT
  *@param input_buffer[Input]�� �������ݻ���  // NOLINT
  *@param output_buffer[Output]���ص���������������ݻ���  // NOLINT
  *@param context[Input]�� �ص�����������������  // NOLINT
  *@return�� �ɹ�����YSOS_ERROR_SUCCESS��ʧ�ܷ�����Ӧ����ֵ  // NOLINT
  */
  virtual int Callback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, void *context = NULL);

 protected:
  /**
  *@brief �����¼�����Ϣ  // NOLINT
  *@param event_name[Input]�� �¼�������  // NOLINT
  *@param input_buffer[input]��������������  // NOLINT
  *@param context[Input]�� �ص�����������������  // NOLINT
  *@return�� �ɹ�����YSOS_ERROR_SUCCESS��ʧ�ܷ�����Ӧ����ֵ  // NOLINT
  */
  virtual int HandleMessage(const std::string &event_name, BufferInterfacePtr input_buffer, CallbackInterface *context);
  /**
  *@brief ͨ��EventName����ö�Ӧ��Agentע�������CallbackName  // NOLINT
  *@param event_name[Input]�� �¼�������  // NOLINT
  *@param context[Input]�� �ص�����������������  // NOLINT
  *@return�� �ɹ�����Callback���ƣ�ʧ�ܷ��ؿ�  // NOLINT
  */
  virtual std::string GetCallbackNameByEventName(const std::string &event_name, CallbackInterface *context);
  /**
   *@brief ��Agent�����¼�֪ͨ����  // NOLINT
   *@param event_name[Input]�� ֪ͨ��Agent���¼�����  // NOLINT
   *@param callback_name[Input]�� Agent�������ݵĺ�����  // NOLINT
   *@param content[input]��������������  // NOLINT
   *@param context[Input]�� �ص�����������������  // NOLINT
   *@return�� �ɹ�����YSOS_ERROR_SUCCESS��ʧ�ܷ�����Ӧ����ֵ  // NOLINT
   */
  virtual int DoEventNotifyService(const std::string &event_name, const std::string &callback_name, const std::string &content, CallbackInterface *context);
  /**
  *@brief ��Agent����״̬�л�����  // NOLINT
  *@param next_state_name[Input]�� Ŀ��״̬  // NOLINT
  *@param context[Input]�� �ص�����������������  // NOLINT
  *@return�� �ɹ�����YSOS_ERROR_SUCCESS��ʧ�ܷ�����Ӧ����ֵ  // NOLINT
  */
  virtual int DoSwitchNotifyService(const std::string &next_state_name, CallbackInterface *context);
  /**
  *@brief ��Agent����״̬�л�����  // NOLINT
  *@param switch_req[Input]�� switch����  // NOLINT
  *@param context[Input]�� �ص�����������������  // NOLINT
  *@return�� �ɹ�����YSOS_ERROR_SUCCESS��ʧ�ܷ�����Ӧ����ֵ  // NOLINT
  */
  virtual int DoSwitchService(ReqSwitchServiceParam &switch_req, CallbackInterface *context);
  /**
  *@brief ��״̬������Ioctl����  // NOLINT
  *@param ioctl_req[Input]�� Ioctl����  // NOLINT
  *@param context[Input]�� �ص�����������������  // NOLINT
  *@return�� �ɹ�����YSOS_ERROR_SUCCESS��ʧ�ܷ�����Ӧ����ֵ  // NOLINT
  */
  virtual int DoIoctlService(ReqIOCtlServiceParam &ioctl_req, CallbackInterface *context);
  /**
  *@brief ��״̬�������¼�ע�����  // NOLINT
  *@param reg_service_event_req[Input]�� �¼�ע��Ĳ���  // NOLINT
  *@param context[Input]�� �ص�����������������  // NOLINT
  *@return�� �ɹ�����YSOS_ERROR_SUCCESS��ʧ�ܷ�����Ӧ����ֵ  // NOLINT
  */
  virtual int DoRegEventService(ReqRegServiceEventServiceParam &reg_service_event_req, CallbackInterface *context);
  /**
  *@brief ��״̬������ע���¼�����  // NOLINT
  *@param un_reg_service_event_req[Input]�� ��ע���¼��Ĳ���  // NOLINT
  *@param context[Input]�� �ص�����������������  // NOLINT
  *@return�� �ɹ�����YSOS_ERROR_SUCCESS��ʧ�ܷ�����Ӧ����ֵ  // NOLINT
  */
  virtual int DoUnRegEventService(ReqUnregServiceEventServiceParam &un_reg_service_event_req, CallbackInterface *context);
  /**
  *@brief ��״̬������OnReady����  // NOLINT
  *@param ready_req[Input]�� on_ready�Ĳ���  // NOLINT
  *@param context[Input]�� �ص�����������������  // NOLINT
  *@return�� �ɹ�����YSOS_ERROR_SUCCESS��ʧ�ܷ�����Ӧ����ֵ  // NOLINT
  */
  virtual int DoReadyService(ReqReadyServiceParam &ready_req, CallbackInterface *context);
};
}  /// namespace ysos //  NOLINT

#endif  ///<  SFP_BASE_STRATEGY_CALLBACK_IMPL_H_  //  NOLINT