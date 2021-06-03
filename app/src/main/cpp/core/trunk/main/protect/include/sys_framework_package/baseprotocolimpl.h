/**
 *@file baseprotocolimpl.h
 *@brief Definition of BaseProtocolImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-05-09 13:12:58
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef CFP_BASE_PROTOCOL_IMPL_H_  // NOLINT
#define CFP_BASE_PROTOCOL_IMPL_H_  // NOLINT

/// Platform Headers
#include "../../../public/include/sys_interface_package/common.h"  // NOLINT
#include "../../../public/include/core_help_package/baseinterfaceimpl.h" // NOLINT
#include "../../../public/include/sys_interface_package/protocolinterface.h" // NOLINT

namespace ysos {
/**
 *@brief BaseProtocolImpl�ľ���ʵ��  // NOLINT
 */
class YSOS_EXPORT BaseProtocolImpl : public ProtocolInterface, public BaseInterfaceImpl {
  DISALLOW_COPY_AND_ASSIGN(BaseProtocolImpl)  //  ��ֹ�����͸���
  DECLARE_PROTECT_CONSTRUCTOR(BaseProtocolImpl)  // ���캯������
  DECLARE_CREATEINSTANCE(BaseProtocolImpl)  // �������ȫ�־�̬��������

 public:
  virtual ~BaseProtocolImpl();

  /**
  *@brief ������ʼ��  // NOLINT
  *@param param�� ��ʼ���Ĳ���  // NOLINT
  *@return�� �ɹ�����YSOS_ERROR_SUCCESS��ʧ�ܷ�����Ӧ����ֵ  // NOLINT
  */
  virtual int Initialize(void *param=NULL);

  /**
   *@brief ������Ϣ  // NOLINT
   *@param in_buffer[Input]�� ����Ļ��壬�ڲ�������Э���ʽ����  // NOLINT
   *@param out_buffer[Output]������Ļ��壬�ڲ�ΪӦ����Ҫ�Ľ���������  // NOLINT
   *@param format_id[Input]���ɾ����������Զ���  // NOLINT
   *@param context_ptr[Input]��Э��ӿڵ������ģ������ʵ���������  // NOLINT
   *@return�� �ɹ�����YSOS_ERROR_SUCCESS��ʧ�ܷ�����Ӧ����ֵ  // NOLINT
   */
  virtual int ParseMessage(BufferInterfacePtr in_buffer, BufferInterfacePtr out_buffer, ProtocolFormatId format_id, void* context_ptr);

  /**
   *@brief ��װ��Ϣ  // NOLINT
   *@param in_buffer[Input]�� ����Ļ��壬�ڲ�ΪӦ������ľ�������  // NOLINT
   *@param out_buffer[Output]������Ļ��壬�ڲ�Ϊ���ո�Э���װ��ĸ�ʽ����  // NOLINT
   *@param format_id[Input]���ɾ����������Զ���  // NOLINT
   *@param context_ptr[Input]��Э��ӿڵ������ģ������ʵ���������  // NOLINT
   *@return�� �ɹ�����YSOS_ERROR_SUCCESS��ʧ�ܷ�����Ӧ����ֵ  // NOLINT
   */
  virtual int FormMessage(BufferInterfacePtr in_buffer, BufferInterfacePtr out_buffer, ProtocolFormatId format_id, void* context_ptr);

  /**
   *@brief ���Э����ĳ����Ϣ�ĳ��ȣ������ⲿ��ǰ���仺�峤��  //NOLINT
   *@param in_buffer[Input]�� ����Ļ��壬�ڲ�Ϊ��ʽ����  //NOLINT
   *@param format_id[Input]���ɾ����������Զ���  // NOLINT
   *@return�� �ɹ�����YSOS_ERROR_SUCCESS��ʧ�ܷ�����Ӧ����ֵ  //NOLINT
   */
  virtual int GetLength(BufferInterfacePtr in_buffer, ProtocolFormatId format_id);

 protected:
  /**
  *@brief  �����ļ������õ�Key��Value�ԣ�����Initialized�����е��� //  NOLINT
  *@param key ���ò����е�Key  // NOLINT
  *@param value ��Key��Ӧ��Value  // NOLINT
  *@return �ɹ�����YSOS_ERROR_SUCCESS������ʧ��  //  NOLINT
  */
  virtual int Initialized(const std::string &key, const std::string &value);

 private:
  /**
  *@brief  ��ȡ�����ļ��еĲ������������� //  NOLINT
  *@return �ɹ�����YSOS_ERROR_SUCCESS������ʧ��  //  NOLINT
  */
  int ConfigInitialize(void);

 protected:
  std::string                  remark_;        ///<  protocol����ı�ע  //  NOLINT
  std::string                  version_;       ///<  protocol����İ汾�� //  NOLINT
};

}
#endif
