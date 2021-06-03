/**
 *@file XmlProtocolImpl.h
 *@brief Definition of XmlProtocolImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef SFP_YSOS_XML_IMPL_H_  // NOLINT
#define SFP_YSOS_XML_IMPL_H_  // NOLINT

#include "../../../public/include/sys_interface_package/common.h"  // NOLINT
#include "../../../public/include/core_help_package/xmlutil.h"  // NOLINT
#include "../../../public/include/sys_interface_package/baseinterface.h"  // NOLINT
#include "../../../public/include/sys_interface_package/treestructmapdef.h"  // NOLINT
#include "../../../public/include/sys_interface_package/protocolinterface.h"  // NOLINT

namespace ysos {
class YSOS_EXPORT XmlProtocolImpl : public ProtocolInterface, public BaseInterfaceImpl {
  DECLARE_CREATEINSTANCE(XmlProtocolImpl);
  DISALLOW_COPY_AND_ASSIGN(XmlProtocolImpl);
  DECLARE_PROTECT_CONSTRUCTOR(XmlProtocolImpl);

 public:
  ~XmlProtocolImpl();

  virtual int ParseMessage(
    BufferInterfacePtr in_buffer,
    BufferInterfacePtr out_buffer,
    ProtocolFormatId format_id,
    void *context_ptr);
  virtual int FormMessage(
    BufferInterfacePtr in_buffer,
    BufferInterfacePtr out_buffer,
    ProtocolFormatId format_id,
    void *context_ptr);
  virtual int GetLength(BufferInterfacePtr in_buffer, ProtocolFormatId format_id);

 protected:
  tinyxml2::XMLElement *GetSpecifyElement(tinyxml2::XMLElement *element, uint64_t num);  // NOLINT
  tinyxml2::XMLElement *GetSpecifyLevelElement(tinyxml2::XMLElement *parent, const uint64_t level);
  UINT8* GetBuffer(BufferInterfacePtr buffer_ptr, UINT32 *length);
  tinyxml2::XMLElement* ParseMesage(BufferInterfacePtr in_buffer, ProtocolFormatId format_id);

 private:
  tinyxml2::XMLDocument   *xml_doc_;
};
}
#endif // SFP_YSOS_XML_IMPL_H_  // NOLINT
