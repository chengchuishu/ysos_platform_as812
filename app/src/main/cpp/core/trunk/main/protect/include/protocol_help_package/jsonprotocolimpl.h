/*
# JsonProtocolImpl.h
# Definition of JsonProtocolImpl
# Created on: 2016-04-21 13:59:20
# Original author: XuLanyue
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20160421, created by XuLanyue
*/
#ifndef JSON_PROTOCOL_IMPL_H                                        //LINT
#define JSON_PROTOCOL_IMPL_H                                        //LINT

#if 1   //  disable warning 4250 and 4996
#include "../../../public/include/os_hal_package/config.h"  // NOLINT
#if (CURRENT_PLATFORM == PLATFORM_WINDOWS)
#pragma warning(disable: 4250)      //  dominance
#pragma warning(disable: 4996)      //  std::_Fill_n
#endif
#endif
#include "../../../public/include/sys_interface_package/bufferpool.h"

#include "../../../public/include/core_help_package/utility.h"
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../public/include/sys_interface_package/treestructmapdef.h"
#include "../../../public/include/sys_interface_package/protocolinterface.h"
#include <json/json.h>
#include <boost/shared_ptr.hpp>

namespace ysos {
class YSOS_EXPORT JsonProtocolImpl :
  public ProtocolInterface,
  public BaseInterfaceImpl {
  DISALLOW_COPY_AND_ASSIGN(JsonProtocolImpl)
  DECLARE_PROTECT_CONSTRUCTOR(JsonProtocolImpl)
  DECLARE_CREATEINSTANCE(JsonProtocolImpl)
 public:
  enum JsonProperties {
    //  if skip any comment in json, set it 1,
    //  otherwise 0; default 1.
    IN_DATATYPES_SKIPCOMMENT    = 0x0100,

    //  if root must be value of an array or an object, set it 1,
    //  otherwise 0; default 1.
    IN_DATATYPES_STRICT,

    //  input data in pInBuffer: if it is from buffer, set it 0,
    //  else if it is from file, set it 1; default 0.
    IN_DATA_FROM,

    //  output data in pOutBuffer: if it is to buffer, set it 0,
    //  else if it is to file, set it 1; default 0.
    OUT_DATA_TO,
  };
  virtual ~JsonProtocolImpl();

  virtual int ParseMessage(
    BufferInterfacePtr in_buffer_ptr,
    BufferInterfacePtr out_buffer_ptr,
    ProtocolFormatId format_id,
    void* context);
  virtual int FormMessage(
    BufferInterfacePtr in_buffer_ptr,
    BufferInterfacePtr out_buffer_ptr,
    ProtocolFormatId format_id,
    void* context);
  virtual int GetLength(
    BufferInterfacePtr in_buffer_ptr,
    ProtocolFormatId format_id);
 private:
  int GetJsonMemberValue(
    bool from_file,
    std::string& path_or_json,
    ProtocolFormatId format_id,
    std::string& member_name);
  int TravelValueTreeForNode(
    const std::string &input,
    Json::Value &root,
    const Json::Features &features,
    uint64_t uFormatId,
    std::string& strMember);
  int TravelWholeValueTree(
    const std::string &input,
    Json::Value &root,
    const Json::Features &features,
    uint64_t uFormatId,
    std::string& strMember);
};

struct JsonContext {
  void* inner_context_ptr;
  void* inner_data_ptr;
  //  if skip any comment in json, set it 1,
  //  otherwise 0; default 1.
  int skip_comment;

  //  if root must be value of an array or an object, set it 1,
  //  otherwise 0; default 1.
  int strict;

  //  input data in pInBuffer: if it is from buffer, set it 0,
  //  else if it is from file, set it 1; default 0.
  int input_from;

  //  output data in pOutBuffer: if it is to buffer, set it 0,
  //  else if it is to file, set it 1; default 0.
  int output_to;
  JsonContext() {
      inner_context_ptr = NULL;
      inner_data_ptr = NULL;
      skip_comment = 1;
      strict = 1;
      input_from = 0;
      output_to = 0;
  }
  JsonContext(
    void* inner_context_ptr_ref,
    void* inner_data_ptr_ref,
    int skip_comment_ref,
    int strict_ref,
    int input_from_ref,
    int output_to_ref) {
      inner_context_ptr = inner_context_ptr_ref;
      inner_data_ptr = inner_data_ptr_ref;
      skip_comment = skip_comment_ref;
      strict = strict_ref;
      input_from = input_from_ref;
      output_to = output_to_ref;
  }
};

} // namespace ysos

#endif  //  JSON_PROTOCOL_IMPL_H//LINT
