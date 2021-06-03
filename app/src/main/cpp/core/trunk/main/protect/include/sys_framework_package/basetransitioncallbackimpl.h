/**
  *@file BaseTransitionCallbackImpl.h
  *@brief Transition Callback
  *@version 1.0
  *@author Donghongqian
  *@date Created on: 2016-05-31 18:03:27
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  */
#ifndef SFP_BASE_TRANSITION_CALLBACK_IMPL_H_
#define SFP_BASE_TRANSITION_CALLBACK_IMPL_H_

/// Ysos Headers //  NOLINT
#include "../../../protect/include/sys_framework_package/basestrategyservicecallbackimpl.h"
#include "../../../protect/include/sys_framework_package/basetransitioninfoimpl.h"

namespace ysos {

/**
  *@brief  Transition Callback //  NOLINT
  *        //  NOLINT
  */
class YSOS_EXPORT BaseTransitionCallbackImpl: public BaseStrategyServiceCallbackImpl {
  DECLARE_CREATEINSTANCE(BaseTransitionCallbackImpl);
  DISALLOW_COPY_AND_ASSIGN(BaseTransitionCallbackImpl);
  DECLARE_PROTECT_CONSTRUCTOR(BaseTransitionCallbackImpl);

 public:
  /**
   *@brief ��������
   */
  virtual ~BaseTransitionCallbackImpl();

  /**
  *@brief �ص��ӿڵ�ʵ�ֺ���  // NOLINT
  *@param input_buffer[Input]�� �������ݻ���  // NOLINT
  *@param output_buffer[Output]���ص���������������ݻ���  // NOLINT
  *@param context[Input]�� �ص�����������������,��ModuleInterface* ����  // NOLINT
  *@return�� �ɹ�����YSOS_ERROR_SUCCESS��ʧ�ܷ�����Ӧ����ֵ  // NOLINT
  */
  virtual int Callback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, void *context = NULL); // NOLINT

 protected:
  /**
  *@brief  �����ļ������õ�Key��Value�ԣ�����Initialized�����е��� //  NOLINT
  *@param key ���ò����е�Key  // NOLINT
  *@param value ��Key��Ӧ��Value  // NOLINT
  *@return �ɹ�����YSOS_ERROR_SUCCESS������ʧ��  //  NOLINT
  */
  virtual int Initialized(const std::string &key, const std::string &value);
  /**
  *@brief �ж�input_buffer�����Ϣ���Ƿ���������   // NOLINT
  *@param  input_ptr ��ƥ�������   //  NOLINT
  *@param  context_ptr �����ģ�����ΪNULL //  NOLINT
  *@return �ɹ�����true��ʧ�ܷ���false  //  NOLINT
  */
  virtual bool IsMatched(BufferInterfacePtr input_buffer, TransitionContext *context_ptr);
  /**
  *@brief ͨ��Context��Ϣ��ȡ��ȡ��Ϣͷ������
  *@param context_ptr �����ģ�����ΪNULL //  NOLINT
  *@return �ɹ�������Ϣͷ�����ͣ�ʧ�ܷ���0  //  NOLINT
  */
  virtual uint32_t GetMessageID(TransitionContext *context_ptr);
  /**
  *@brief ͨ��Context��Ϣ��ȡ��ȡ��Ϣͷ
  *@param  is_normal_data true: ��Normal_Data��Ϣ��false����Status_Event��Ϣ  //  NOLINT
  *@param context_ptr �����ģ�����ΪNULL //  NOLINT
  *@return �ɹ�������Ϣͷ��ʧ�ܷ���NULL  //  NOLINT
  */
 // virtual MessageHeadPtr GetMessageHeader(TransitionContext *context_ptr, bool is_normal_data=true);
  /**
  *@brief ����״̬�����л��ı�ʶ  //  NOLINT
  *@param  module_name ����������Module //  NOLINT
  *@param context_ptr �����ģ�����ΪNULL //  NOLINT
  *@return �ɹ�����0��ʧ�ܷ�������ֵ  //  NOLINT
  */
  virtual int SetSwitchFlag(const std::string &module_name, TransitionContext *context_ptr);

 protected:
  /**
  *@brief �ж�input_buffer�����Ϣ���Ƿ���������   // NOLINT
  *@param  status_code ״ֵ̬   //  NOLINT
  *@param  service_name ������ //  NOLINT
  *@param  input_ptr ��ƥ�������   //  NOLINT
  *@param  context_ptr �����ģ�����ΪNULL //  NOLINT
  *@return �ɹ�����true��ʧ�ܷ���false  //  NOLINT
  */
  virtual bool IsStatusEventMatched(std::string &status_code, std::string &service_name, BufferInterfacePtr input_buffer, TransitionContext *context_ptr);
  /**
  *@brief ��Message���ӵ�Strategy��Queue��   // NOLINT
  *@param  input_ptr �����ӵ�����   //  NOLINT
  *@param  is_normal_data true: ��Normal_Data��Ϣ��false����Status_Event��Ϣ  //  NOLINT
  *@param  context_ptr �����ģ�����ΪNULL //  NOLINT
  *@return �ɹ�����true��ʧ�ܷ���false  //  NOLINT
  */
  virtual bool AddMessageToQueue(BufferInterfacePtr input_buffer, bool is_normal_data, TransitionContext *context_ptr);

private:
  /**
  *@brief �ж�input_buffer�����Ϣ���Ƿ���������   // NOLINT
  *@param  input_ptr ��ƥ�������   //  NOLINT
  *@param  context_ptr �����ģ�����ΪNULL //  NOLINT
  *@return �ɹ�����true��ʧ�ܷ���false  //  NOLINT
  */
  bool IsStatusEventMatched(BufferInterfacePtr input_buffer, TransitionContext *context_ptr);

 protected:
  bool       status_event_msg_match_;  ///<  status event message�Ƿ����ƥ�䣬true: ƥ�� false: ���� //  NOLINT
  bool       normal_data_msg_match_;  ///<  normal data message�Ƿ����ƥ�䣬true: ƥ�� false: ���� //  NOLINT
};
}
#endif  /// SFP_BASE_TRANSITION_CALLBACK_IMPL_H_ 
