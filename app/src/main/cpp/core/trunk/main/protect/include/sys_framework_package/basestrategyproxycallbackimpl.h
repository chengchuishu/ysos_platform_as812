/**
 *@file BaseStrategyProxyCallbackImpl.h
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
#include "../../../public/include/sys_interface_package/strategyinterface.h"
#include "../../../public/include/sys_interface_package/agentinterface.h"

namespace ysos {

class YSOS_EXPORT BaseStrategyProxyCallbackImpl : public BaseStrategyServiceCallbackImpl {
  DECLARE_CREATEINSTANCE(BaseStrategyProxyCallbackImpl);
  DISALLOW_COPY_AND_ASSIGN(BaseStrategyProxyCallbackImpl);
  DECLARE_PROTECT_CONSTRUCTOR(BaseStrategyProxyCallbackImpl);

 public:
  virtual ~BaseStrategyProxyCallbackImpl();

  enum Strategy_Service_Property {LOCAL_SERVICE, CLIENT_SERVICE, INTERNAL_SERVICE, STRATEGY_SERVICE_END};
  typedef std::map<std::string, ReqIOCtlServiceParam>       IoctlMap;

  /**
  *@brief �ص��ӿڵ�ʵ�ֺ���  // NOLINT
  *@param input_buffer[Input]�� �������ݻ���  // NOLINT
  *@param output_buffer[Output]���ص���������������ݻ���  // NOLINT
  *@param context[Input]�� �ص�����������������  // NOLINT
  *@return�� �ɹ�����YSOS_ERROR_SUCCESS��ʧ�ܷ�����Ӧ����ֵ  // NOLINT
  */
  virtual int Callback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, void *context = NULL);

  /**
   *@brief ���ýӿڵ�����ֵ  // NOLINT
   *@param type_id[Input]�� ���Ե�ID  // NOLINT
   *@param type[input]�� ����ֵ��ֵ  // NOLINT
   *@return�� �ɹ�����YSOS_ERROR_SUCCESS��ʧ�ܷ�����Ӧ����ֵ  // NOLINT
   */
  virtual int SetProperty(int type_id, void *type);

 protected:
  /**
  *@brief  �����ļ������õ�Key��Value�ԣ�����Initialized�����е��� //  NOLINT
  *@param key ���ò����е�Key  // NOLINT
  *@param value ��Key��Ӧ��Value  // NOLINT
  *@return �ɹ�����YSOS_ERROR_SUCCESS������ʧ��  //  NOLINT
  */
  virtual int Initialized(const std::string &key, const std::string &value);
  /**
  *@brief �����¼�����Ϣ  // NOLINT
  *@param event_name[Input]�� �¼�������  // NOLINT
  *@param input_buffer[input]��������������  // NOLINT
  *@param context[Input]�� �ص�����������������  // NOLINT
  *@return�� �ɹ�����YSOS_ERROR_SUCCESS��ʧ�ܷ�����Ӧ����ֵ  // NOLINT
  */
  virtual int HandleMessage(const std::string &event_name, BufferInterfacePtr input_buffer, CallbackInterface *context);
  /**
   *@brief ��Agent�����¼�֪ͨ����  // NOLINT
   *@param event_name[Input]�� �¼�������  // NOLINT
   *@param input_buffer[input]��������������  // NOLINT
   *@param context[Input]�� �ص�����������������  // NOLINT
   *@return�� �ɹ�����YSOS_ERROR_SUCCESS��ʧ�ܷ�����Ӧ����ֵ  // NOLINT
   */
  virtual int DoCustomEventService(const std::string &event_name, BufferInterfacePtr input_buffer, CallbackInterface *context);
  /**
  *@brief ������Strategy��������  // NOLINT
  *@param event_name[Input]�� �¼�������  // NOLINT
  *@param input_buffer[input]��������������  // NOLINT
  *@param context[Input]�� �ص�����������������  // NOLINT
  *@return�� �ɹ�����YSOS_ERROR_SUCCESS��ʧ�ܷ�����Ӧ����ֵ  // NOLINT
  */
  virtual int DoInternalService(const std::string &event_name, BufferInterfacePtr input_buffer, CallbackInterface *context);
  /**
  *@brief ��״̬������Ioctl����  // NOLINT
  *@param event_name[Input]�� �¼�������  // NOLINT
  *@param input_buffer[input]��������������  // NOLINT
  *@param context[Input]�� �ص�����������������  // NOLINT
  *@return�� �ɹ�����YSOS_ERROR_SUCCESS��ʧ�ܷ�����Ӧ����ֵ  // NOLINT
  */
  virtual int DoIoctlService(const std::string &event_name, BufferInterfacePtr input_buffer, CallbackInterface *context);
  /**
  *@brief ����Ioctl���ò���  // NOLINT
  *@param ioctl_param[Input]�� ioctl�����ò���  // NOLINT
  *@return�� �ɹ�����YSOS_ERROR_SUCCESS��ʧ�ܷ�����Ӧ����ֵ  // NOLINT
  */
  virtual int ParseIoctlParam(const std::string &ioctl_param);

 protected:
  AgentInterface*              agent_ptr_;   ///< client �ͻ���
  Strategy_Service_Property    service_type_;   ///< ֧�ַ��������
  StrategyInterfacePtr         strategy_ptr_;  ///< internal��Strategyָ��
  bool                         is_init_strategy_;   ///< �Ƿ��ʼ������
  std::string                  strategy_name_;  ///< internal��Strategy����
  IoctlMap  ioctl_map_;   ///< �¼���Ӧ��Ioctl����
};
}  /// namespace ysos //  NOLINT

#endif  ///<  SFP_BASE_STRATEGY_CALLBACK_IMPL_H_  //  NOLINT