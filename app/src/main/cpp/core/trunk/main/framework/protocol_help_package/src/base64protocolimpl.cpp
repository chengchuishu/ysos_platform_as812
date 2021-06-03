/**
 *@file base64protocolimpl.cpp
 *@brief Definition of Base64ProtocolImpl
 *@version 0.1
 *@author jinchengzhe
 *@date Created on: 2016-05-09 13:12:58
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// Platform Headers
#include "../../../protect/include/protocol_help_package/base64protocolimpl.h"
#include "../../../public/include/sys_interface_package/ysos_enum.h"

namespace ysos {

Base64ProtocolImpl::Base64ProtocolImpl(const std::string& name) : BaseInterfaceImpl(name) {
  logger_ = GetUtility()->GetLogger("ysos.framework");

  YSOS_LOG_DEBUG("Base64ProtocolImpl::Base64ProtocolImpl()");
}

Base64ProtocolImpl::~Base64ProtocolImpl() {
  YSOS_LOG_DEBUG("Base64ProtocolImpl::~Base64ProtocolImpl()");
}

int Base64ProtocolImpl::GetLength(BufferInterfacePtr in_buffer, ProtocolFormatId format_id) {

  /// Check Parameter
  if (NULL == in_buffer) {
    return -1;
  }

  /// Get input buffer data
  UINT32 in_buffer_length = 0;
  UINT8* in_buffer_data_ptr = GetBuffer(in_buffer, &in_buffer_length);
  if (NULL == in_buffer_data_ptr && 0 == in_buffer_length) {
    return -1;
  }

  //std::string code;
  int buffer_length = 0;

  /// Check Format ID
  switch (format_id) {
  case PROP_ENCODE_TYPE: {
    //code = Encode((const unsigned char*)in_buffer_data_ptr, (int)in_buffer_length);

    buffer_length = GetEncodeBufferLength((const unsigned char*)in_buffer_data_ptr, (int)in_buffer_length);
  }
  break;
  case PROP_DECODE_TYPE: {
    //int out_buffer_length = 0;
    //code = Decode((const char*)in_buffer_data_ptr, (int)in_buffer_length, out_buffer_length);
	//int tt = code.length();

    buffer_length = GetDecodeBufferLength((const char*)in_buffer_data_ptr, (int)in_buffer_length);
  }
  break;
  default:
    break;
  }

  //return (code.length()+1);
  return buffer_length;
}

int Base64ProtocolImpl::ParseMessage(BufferInterfacePtr in_buffer, BufferInterfacePtr out_buffer, ProtocolFormatId format_id, void *context_ptr) {
  if (NULL == in_buffer && NULL == out_buffer) {
    return -1;
  }

  /// Get input buffer data
  UINT32 in_buffer_length = 0;
  UINT8* in_buffer_data_ptr = GetBuffer(in_buffer, &in_buffer_length);
  if (NULL == in_buffer_data_ptr && 0 == in_buffer_length) {
    return -1;
  }

  /// Get output buffer data
  UINT32 out_buffer_length = 0;
  UINT8* out_buffer_data_ptr = GetBuffer(out_buffer, &out_buffer_length);
  if (NULL == out_buffer_data_ptr && 0 == out_buffer_length) {
    return -1;
  }

  /// Decode
  return (Decode((char*)in_buffer_data_ptr, strlen((char*)in_buffer_data_ptr), out_buffer_length, (char*)out_buffer_data_ptr));
}

int Base64ProtocolImpl::FormMessage(BufferInterfacePtr in_buffer, BufferInterfacePtr out_buffer, ProtocolFormatId format_id, void *context_ptr) {

  if (NULL == in_buffer && NULL == out_buffer) {
    return -1;
  }

  //	Get input buffer data
  UINT32 in_buffer_length = 0;
  UINT8* in_buffer_data_ptr = GetBuffer(in_buffer, &in_buffer_length);
  if (NULL == in_buffer_data_ptr && 0 == in_buffer_length) {
    return -1;
  }

  /// Get output buffer data
  UINT32 out_buffer_length = 0;
  UINT8* out_buffer_data_ptr = GetBuffer(out_buffer, &out_buffer_length);
  if (NULL == out_buffer_data_ptr && 0 == out_buffer_length) {
    return -1;
  }

  /// Encode
  return (Encode((const unsigned char*)in_buffer_data_ptr, strlen((char*)in_buffer_data_ptr), out_buffer_length, (char*)out_buffer_data_ptr));
}

UINT8* Base64ProtocolImpl::GetBuffer(BufferInterfacePtr buffer_ptr, UINT32 * buffer_length_ptr) {
  UINT8 *ptr = NULL;
  buffer_ptr->GetBufferAndLength(&ptr, buffer_length_ptr);

  return ptr;
}

std::string Base64ProtocolImpl::Encode(const unsigned char* in_data_ptr, const int in_data_length) {
  //编码表
  const char encode_table[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  //返回值
  std::string encode;
  unsigned char temp[4]= {0};
  int line_length=0;

  for (int i=0; i<(int)(in_data_length / 3); i++) {
    temp[1] = *in_data_ptr++;
    temp[2] = *in_data_ptr++;
    temp[3] = *in_data_ptr++;
    encode+= encode_table[temp[1] >> 2];
    encode+= encode_table[((temp[1] << 4) | (temp[2] >> 4)) & 0x3F];
    encode+= encode_table[((temp[2] << 2) | (temp[3] >> 6)) & 0x3F];
    encode+= encode_table[temp[3] & 0x3F];

    if (line_length+=4, line_length==76) {
      encode+="\r\n";
      line_length=0;
    }

  }

  //对剩余数据进行编码
  int mod=in_data_length % 3;
  if (mod==1) {
    temp[1] = *in_data_ptr++;
    encode+= encode_table[(temp[1] & 0xFC) >> 2];
    encode+= encode_table[((temp[1] & 0x03) << 4)];
    encode+= "==";
  } else if (mod==2) {
    temp[1] = *in_data_ptr++;
    temp[2] = *in_data_ptr++;
    encode+= encode_table[(temp[1] & 0xFC) >> 2];
    encode+= encode_table[((temp[1] & 0x03) << 4) | ((temp[2] & 0xF0) >> 4)];
    encode+= encode_table[((temp[2] & 0x0F) << 2)];
    encode+= "=";
  }

  return encode;
}

int Base64ProtocolImpl::Encode(const unsigned char* in_data_ptr, const int in_data_length, const UINT32 out_data_length, char* out_data_ptr) {

  if (NULL == in_data_ptr &&
      0 == in_data_length &&
      NULL == out_data_ptr &&
      0 == out_data_length
     ) {
    return -1;
  }

  /// 编码表
  const char encode_table[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  /// 返回值
  std::string value;
  unsigned char temp[4]= {0};
  int line_length=0;

  UINT32 encode_data_length = 0;

  for (int ii = 0; ii < (int)(in_data_length / 3); ii++) {
    temp[1] = *in_data_ptr++;
    temp[2] = *in_data_ptr++;
    temp[3] = *in_data_ptr++;

    value = encode_table[temp[1] >> 2];
    if (encode_data_length == out_data_length - 1) {
      return -1;
    }
    /** need update for linux  **/
    #ifdef _WIN32
      strcat_s(out_data_ptr, out_data_length, value.c_str());
    #else
      strcat(out_data_ptr, value.c_str());
    #endif
    encode_data_length += value.length();

    value = encode_table[((temp[1] << 4) | (temp[2] >> 4)) & 0x3F];
    if (encode_data_length == out_data_length - 1) {
      return -1;
    }
    /** need update for linux  **/
    #ifdef _WIN32
      strcat_s(out_data_ptr, out_data_length, value.c_str());
    #else
      strcat(out_data_ptr, value.c_str());
    #endif
    //strcat_s(out_data_ptr, out_data_length, value.c_str());
    encode_data_length += value.length();

    value = encode_table[((temp[2] << 2) | (temp[3] >> 6)) & 0x3F];
    if (encode_data_length == out_data_length - 1) {
      return -1;
    }
    /** need update for linux  **/
    #ifdef _WIN32
      strcat_s(out_data_ptr, out_data_length, value.c_str());
    #else
      strcat(out_data_ptr, value.c_str());
    #endif
    //strcat_s(out_data_ptr, out_data_length, value.c_str());
    encode_data_length += value.length();

    value = encode_table[temp[3] & 0x3F];
    if (encode_data_length == out_data_length - 1) {
      return -1;
    }
    /** need update for linux  **/
    #ifdef _WIN32
      strcat_s(out_data_ptr, out_data_length, value.c_str());
    #else
      strcat(out_data_ptr, value.c_str());
    #endif
    //strcat_s(out_data_ptr, out_data_length, value.c_str());
    encode_data_length += value.length();

    if (line_length+=4, line_length==76) {
      value ="\r\n";
      if (encode_data_length == out_data_length - 1) {
        return -1;
      }
      /** need update for linux  **/
      #ifdef _WIN32
        strcat_s(out_data_ptr, out_data_length, value.c_str());
      #else
        strcat(out_data_ptr, value.c_str());
      #endif
      //strcat_s(out_data_ptr, out_data_length, value.c_str());
      encode_data_length += value.length();

      line_length=0;
    }

  }

  /// 对剩余数据进行编码
  int mod=in_data_length % 3;
  if (mod==1) {
    temp[1] = *in_data_ptr++;

    value = encode_table[(temp[1] & 0xFC) >> 2];
    if (encode_data_length == out_data_length - 1) {
      return -1;
    }
    /** need update for linux  **/
    #ifdef _WIN32
      strcat_s(out_data_ptr, out_data_length, value.c_str());
    #else
      strcat(out_data_ptr, value.c_str());
    #endif
    //strcat_s(out_data_ptr, out_data_length, value.c_str());
    encode_data_length += value.length();

    value = encode_table[((temp[1] & 0x03) << 4)];
    if (encode_data_length == out_data_length - 1) {
      return -1;
    }
    /** need update for linux  **/
    #ifdef _WIN32
      strcat_s(out_data_ptr, out_data_length, value.c_str());
    #else
      strcat(out_data_ptr, value.c_str());
    #endif
    //strcat_s(out_data_ptr, out_data_length, value.c_str());
    encode_data_length += value.length();

    value = "==";
    if (encode_data_length == out_data_length - 1) {
      return -1;
    }
    /** need update for linux  **/
    #ifdef _WIN32
      strcat_s(out_data_ptr, out_data_length, value.c_str());
    #else
      strcat(out_data_ptr, value.c_str());
    #endif
    //strcat_s(out_data_ptr, out_data_length, value.c_str());
    encode_data_length += value.length();

  } else if (mod==2) {
    temp[1] = *in_data_ptr++;
    temp[2] = *in_data_ptr++;

    value = encode_table[(temp[1] & 0xFC) >> 2];
    if (encode_data_length == out_data_length - 1) {
      return -1;
    }
    /** need update for linux  **/
    #ifdef _WIN32
      strcat_s(out_data_ptr, out_data_length, value.c_str());
    #else
      strcat(out_data_ptr, value.c_str());
    #endif
    //strcat_s(out_data_ptr, out_data_length, value.c_str());
    encode_data_length += value.length();

    value= encode_table[((temp[1] & 0x03) << 4) | ((temp[2] & 0xF0) >> 4)];
    if (encode_data_length == out_data_length - 1) {
      return -1;
    }
    /** need update for linux  **/
    #ifdef _WIN32
      strcat_s(out_data_ptr, out_data_length, value.c_str());
    #else
      strcat(out_data_ptr, value.c_str());
    #endif
    //strcat_s(out_data_ptr, out_data_length, value.c_str());
    encode_data_length += value.length();

    value= encode_table[((temp[2] & 0x0F) << 2)];
    if (encode_data_length == out_data_length - 1) {
      return -1;
    }
    /** need update for linux  **/
    #ifdef _WIN32
      strcat_s(out_data_ptr, out_data_length, value.c_str());
    #else
      strcat(out_data_ptr, value.c_str());
    #endif
    //strcat_s(out_data_ptr, out_data_length, value.c_str());
    encode_data_length += value.length();

    value = "=";
    if (encode_data_length == out_data_length - 1) {
      return -1;
    }
    /** need update for linux  **/
    #ifdef _WIN32
      strcat_s(out_data_ptr, out_data_length, value.c_str());
    #else
      strcat(out_data_ptr, value.c_str());
    #endif
    //strcat_s(out_data_ptr, out_data_length, value.c_str());
    encode_data_length += value.length();

  }

  return 0;
}

std::string Base64ProtocolImpl::Decode(const char* in_data_ptr, const int in_data_length, int& out_data_length) {

  /// 解码表
  const char decode_table[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    62, // '+'
    0, 0, 0,
    63, // '/'
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, // '0'-'9'
    0, 0, 0, 0, 0, 0, 0,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
    13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, // 'A'-'Z'
    0, 0, 0, 0, 0, 0,
    26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
    39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, // 'a'-'z'
  };

  /// 返回值
  std::string decode;
  int value = 0;
  int ii = 0;
  while (ii < in_data_length) {
    if (*in_data_ptr != '\r' &&  *in_data_ptr!='\n') {
      value = decode_table[*in_data_ptr++] << 18;
      value += decode_table[*in_data_ptr++] << 12;
      decode +=(value & 0x00FF0000) >> 16;

      out_data_length++;

      if (*in_data_ptr != '=') {
        value += decode_table[*in_data_ptr++] << 6;
        decode +=(value & 0x0000FF00) >> 8;

        out_data_length++;

        if (*in_data_ptr != '=') {
          value += decode_table[*in_data_ptr++];
          decode += value & 0x000000FF;

          out_data_length++;
        }
      }

      ii += 4;

    } else { /// 回车换行,跳过
      in_data_ptr++;
      ii++;
    }

  }

  return decode;

}

int Base64ProtocolImpl::Decode(const char* in_data_ptr, const int in_data_length, const UINT32 out_data_length, char* out_data_ptr) {
  if (NULL == in_data_ptr &&
      0 == in_data_length &&
      NULL == out_data_ptr &&
      0 == out_data_length
     ) {
    return -1;
  }

  /// 解码表
  const char decode_table[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    62, // '+'
    0, 0, 0,
    63, // '/'
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, // '0'-'9'
    0, 0, 0, 0, 0, 0, 0,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
    13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, // 'A'-'Z'
    0, 0, 0, 0, 0, 0,
    26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
    39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, // 'a'-'z'
  };

  int value = 0;
  int ii = 0;
  UINT32 decode_data_length = 0;
  std::string temp;
  while (ii < in_data_length) {

    if (*in_data_ptr != '\r' && *in_data_ptr != '\n') {

      value = decode_table[*in_data_ptr++] << 18;
      value += decode_table[*in_data_ptr++] << 12;
      temp = (value & 0x00FF0000) >> 16;

      if (decode_data_length == out_data_length - 1) {
        return -1;
      }
      /** need update for linux  **/
      #ifdef _WIN32
        strcat_s(out_data_ptr, out_data_length, temp.c_str());
      #else
        strcat(out_data_ptr, temp.c_str());
      #endif
      //strcat_s(out_data_ptr, out_data_length, temp.c_str());
      decode_data_length += temp.length();

      if (*in_data_ptr != '=') {
        value += decode_table[*in_data_ptr++] << 6;
        temp = (value & 0x0000FF00) >> 8;

        if (decode_data_length == out_data_length - 1) {
          return -1;
        }
        /** need update for linux  **/
        #ifdef _WIN32
          strcat_s(out_data_ptr, out_data_length, temp.c_str());
        #else
          strcat(out_data_ptr, temp.c_str());
        #endif
        //strcat_s(out_data_ptr, out_data_length, temp.c_str());
        decode_data_length += temp.length();

        if (*in_data_ptr != '=') {
          value += decode_table[*in_data_ptr++];
          temp = value & 0x000000FF;

          if (decode_data_length == out_data_length - 1) {
            return -1;
          }
          /** need update for linux  **/
          #ifdef _WIN32
            strcat_s(out_data_ptr, out_data_length, temp.c_str());
          #else
            strcat(out_data_ptr, temp.c_str());
          #endif
          //strcat_s(out_data_ptr, out_data_length, temp.c_str());
          decode_data_length += temp.length();
        }

      }

      ii += 4;

    } else { /// 回车换行,跳过

      in_data_ptr++;
      ii++;
    }

  }

  return 0;
}

int Base64ProtocolImpl::GetEncodeBufferLength(const unsigned char* in_data_ptr, const int in_data_length) {

  int line_length = 0;
  int encode_data_length = 0;

  for (int i=0; i<(int)(in_data_length / 3); i++) {
    *in_data_ptr++;
    *in_data_ptr++;
    *in_data_ptr++;

    encode_data_length += 4;

    if (line_length+=4, line_length==76) {
      encode_data_length += 2;
      line_length=0;
    }

  }

  /// 对剩余数据进行编码
  int mod=in_data_length % 3;
  if (mod==1) {
    *in_data_ptr++;
    encode_data_length += 4;
  } else if (mod==2) {
    *in_data_ptr++;
    *in_data_ptr++;
    encode_data_length += 4;
  }

  return (encode_data_length+1);
}

int Base64ProtocolImpl::GetDecodeBufferLength(const char* in_data_ptr, const int in_data_length) {

  int decode_data_length = 0;
  int ii = 0;
  while (ii < in_data_length) {
    if (*in_data_ptr != '\r' &&  *in_data_ptr!='\n') {

      *in_data_ptr++;
      *in_data_ptr++;

      decode_data_length++;

      if (*in_data_ptr != '=') {
        *in_data_ptr++;

        decode_data_length++;

        if (*in_data_ptr != '=') {
          *in_data_ptr++;

          decode_data_length++;
        }
      }

      ii += 4;

    } else { /// 回车换行,跳过
      in_data_ptr++;
      ii++;
    }

  }

  return (decode_data_length+1);

}

Base64ProtocolImplPtr CreateInstanceOfBase64ProtocolImplInterface() {
  return Base64ProtocolImplPtr(Base64ProtocolImpl::CreateInstance());
}

};
