/*
# httpprotocolimpl.cpp
# Definition of HTTPProtocolImpl
# Created on: 2016-04-26 13:18:58
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20160912, created by JinChengZhe
*/

/// ThirdParty Headers
#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
/// Platform Headers
#include "../../../protect/include/protocol_help_package/httpprotocolimpl.h"
#include "../../../public/include/core_help_package/utility.h"
#include "../../../public/include/os_hal_package/bufferutility.h"
#include "../../../public/include/sys_interface_package/ysos_enum.h"

namespace ysos {

const char* g_crlf = "\r\n";  ///< 换行符
const char* g_2crlf = "\r\n\r\n";  ///< 双换行符
const char* g_space = " ";  ///< 空格符

const char* g_request_header_tag_method = "METHOD";
const char* g_request_header_tag_request_url = "REQUEST_URL";
const char* g_request_header_tag_http_version = "HTTP_VERSION";
const char* g_response_header_tag_status_code = "STATUS_CODE";
const char* g_response_header_tag_reason_phrase = "REASON_PHRASE";

HTTPProtocolImpl::HTTPProtocolImpl(const std::string& name) : BaseInterfaceImpl(name) {
}

HTTPProtocolImpl::~HTTPProtocolImpl() {
}

int HTTPProtocolImpl::ParseMessage(BufferInterfacePtr in_buffer, BufferInterfacePtr out_buffer, ProtocolFormatId format_id, void *context_ptr) {

  if (NULL == in_buffer ||
      NULL == out_buffer) {
    return YSOS_ERROR_FAILED;
  }

  ///  Get input buffer data
  UINT32 in_buffer_length = GetBufferUtility()->GetBufferLength(in_buffer);
  if (0 == in_buffer_length) {
    return YSOS_ERROR_FAILED;
  }

  UINT8* in_buffer_data_ptr = GetBufferUtility()->GetBufferData(in_buffer);
  if (NULL == in_buffer_data_ptr) {
    return YSOS_ERROR_FAILED;
  }

  if (YSOS_ERROR_SUCCESS != GetBufferUtility()->InitialBuffer(out_buffer)) {
    return YSOS_ERROR_FAILED;
  }

  ///  Get output buffer data
  UINT32 out_buffer_length = GetBufferUtility()->GetBufferLength(out_buffer);
  if (0 == out_buffer_length) {
    return YSOS_ERROR_FAILED;
  }

  UINT8* out_buffer_data_ptr = GetBufferUtility()->GetBufferData(out_buffer);
  if (NULL == out_buffer_data_ptr) {
    return YSOS_ERROR_FAILED;
  }

  std::string input_data = (char*)in_buffer_data_ptr;
  if (true == input_data.empty()) {
    return YSOS_ERROR_FAILED;
  }

  /// Split Header and Data
  std::string header_only;
  std::string data_only;

  std::size_t found = input_data.find(g_2crlf);
  if (std::string::npos != found) {
    header_only = input_data.substr(0, found);
    data_only = input_data.substr(found+4, input_data.length());
  }

  if (true == header_only.empty()) {
    return YSOS_ERROR_FAILED;
  }

  HTTPProtocolParams* http_protocol_params_ptr = static_cast<HTTPProtocolParams*>(context_ptr);
  std::string token, tag, value;

  boost::char_separator<char> line_separator(g_crlf);
  boost::tokenizer<boost::char_separator<char>> line_tokenizer(header_only, line_separator);
  for (
    boost::tokenizer<boost::char_separator<char>>::iterator line_iterator = line_tokenizer.begin();
    line_iterator != line_tokenizer.end();
    ++line_iterator
  ) {
    token = *line_iterator;
    if (line_iterator == line_tokenizer.begin()) {
      boost::char_separator<char> first_line_separator(g_space);
      boost::tokenizer<boost::char_separator<char>> first_line_tokenizer(token, first_line_separator);
      for (
        boost::tokenizer<boost::char_separator<char>>::iterator first_line_iterator = first_line_tokenizer.begin();
        first_line_iterator != first_line_tokenizer.end();
      ) {
        if (PROP_GET == format_id ||
            PROP_POST == format_id
           ) {
          tag = g_request_header_tag_method;
          value = *first_line_iterator;
          http_protocol_params_ptr->insert(std::make_pair(tag, value));

          tag = g_request_header_tag_request_url;
          value = *(++first_line_iterator);
          http_protocol_params_ptr->insert(std::make_pair(tag, value));

          tag = g_request_header_tag_http_version;
          value = *(++first_line_iterator);
          http_protocol_params_ptr->insert(std::make_pair(tag, value));

          break;
        }  else if (PROP_RESPONSE == format_id) {
          tag = g_request_header_tag_http_version;
          value = *first_line_iterator;
          http_protocol_params_ptr->insert(std::make_pair(tag, value));

          tag = g_response_header_tag_status_code;
          value = *(++first_line_iterator);
          http_protocol_params_ptr->insert(std::make_pair(tag, value));

          tag = g_response_header_tag_reason_phrase;
          value = *(++first_line_iterator);
          ++first_line_iterator;

          while (first_line_iterator != first_line_tokenizer.end()) {
            value.append(g_space);
            value.append(*(first_line_iterator));
            ++first_line_iterator;
          }

          http_protocol_params_ptr->insert(std::make_pair(tag, value));

          break;
        }
      }
    } else {
      found = token.find(':');
      if (found != std::string::npos) {
        tag = token.substr(0, found);
        value = token.substr(found+2, token.length());
        http_protocol_params_ptr->insert(std::make_pair(tag, value));
      }
    }
  }/// end of for

  if (true != data_only.empty()) {
    std::memcpy(out_buffer_data_ptr, data_only.c_str(), out_buffer_length > strlen(data_only.c_str()) ? strlen(data_only.c_str()) : out_buffer_length-1);
  }

  return YSOS_ERROR_SUCCESS;
}

int HTTPProtocolImpl::FormMessage(BufferInterfacePtr in_buffer, BufferInterfacePtr out_buffer, ProtocolFormatId format_id, void *context_ptr) {

  if (NULL == out_buffer) {
    return YSOS_ERROR_FAILED;
  }

  UINT32 in_buffer_length = 0;
  UINT8* in_buffer_data_ptr = NULL;
  if (NULL != in_buffer) {
    ///  Get input buffer data
    in_buffer_length = BufferUtility::Instance()->GetBufferLength(in_buffer);
    if (0 == in_buffer_length) {
      return YSOS_ERROR_FAILED;
    }

    in_buffer_data_ptr = BufferUtility::Instance()->GetBufferData(in_buffer);
    if (NULL == in_buffer_data_ptr) {
      return YSOS_ERROR_FAILED;
    }
  }

  if (YSOS_ERROR_SUCCESS != BufferUtility::Instance()->InitialBuffer(out_buffer)) {
    return YSOS_ERROR_FAILED;
  }

  ///  Get output buffer data
  UINT32 out_buffer_length = BufferUtility::Instance()->GetBufferLength(out_buffer);
  if (0 == out_buffer_length) {
    return YSOS_ERROR_FAILED;
  }

  UINT8* out_buffer_data_ptr = BufferUtility::Instance()->GetBufferData(out_buffer);
  if (NULL == out_buffer_data_ptr) {
    return YSOS_ERROR_FAILED;
  }

  /// Get Param
  FormMessageContextPtr form_message_context_ptr = static_cast<FormMessageContextPtr>(context_ptr);
  if (NULL == form_message_context_ptr) {
    return YSOS_ERROR_FAILED;
  }

  /// Parse URL
  std::string protocol;
  std::string domain;
  std::string port;
  std::string path;
  std::string query;
  std::string fragment;

  ParseURL(form_message_context_ptr->url, protocol, domain, port, path, query, fragment);

  std::string temp;

  if (PROP_RESPONSE == format_id) {

    /// Set HTTP Version
    temp = "HTTP/1.1";
    #ifdef _WIN32
      strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());
    #else
      strcat((char*)out_buffer_data_ptr, temp.c_str());
    #endif

    /// Add Space
    temp = g_space;
    #ifdef _WIN32
      strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());
    #else
      strcat((char*)out_buffer_data_ptr, temp.c_str());
    #endif

    if (true == form_message_context_ptr->use_default_header) {
      /// Set Status Code
      temp = "200";
      #ifdef _WIN32
        strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());

        /// Add Space
        temp = g_space;
        strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());

        /// Set Reason Phrase
        temp = "OK";
        strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());

        temp = g_crlf;
        strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());
      #else
        strcat((char*)out_buffer_data_ptr, temp.c_str());

        /// Add Space
        temp = g_space;
        strcat((char*)out_buffer_data_ptr, temp.c_str());

        /// Set Reason Phrase
        temp = "OK";
        strcat((char*)out_buffer_data_ptr, temp.c_str());

        temp = g_crlf;
        strcat((char*)out_buffer_data_ptr, temp.c_str());
      #endif
    } else {
      if (0 < form_message_context_ptr->header_info.size()) {
        /// Set Status Code
        HTTPProtocolParamsIterator iterator = form_message_context_ptr->header_info.find(g_response_header_tag_status_code);
        if (iterator != form_message_context_ptr->header_info.end()) {
          temp = iterator->second;
          #ifdef _WIN32
            strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());
          #else
            strcat((char*)out_buffer_data_ptr, temp.c_str());
          #endif
        }

        /// Add Space
        temp = g_space;
        #ifdef _WIN32
          strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());
        #else
          strcat((char*)out_buffer_data_ptr, temp.c_str());
        #endif

        /// Set Reason Phrase
        iterator = iterator = form_message_context_ptr->header_info.find(g_response_header_tag_reason_phrase);
        if (iterator != form_message_context_ptr->header_info.end()) {
          temp = iterator->second;
          #ifdef _WIN32
            strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());
          #else
            strcat((char*)out_buffer_data_ptr, temp.c_str());
          #endif
        }

        temp = g_crlf;
        #ifdef _WIN32
          strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());
        #else
          strcat((char*)out_buffer_data_ptr, temp.c_str());
        #endif
      }
    }

    /// Add Data Length
    if (0 < strlen((char*)in_buffer_data_ptr)) {
      temp = "Content-Length: ";
      #ifdef _WIN32  
        strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());

        temp = boost::lexical_cast<std::string>(strlen((char*)in_buffer_data_ptr));
        strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());

        temp = g_crlf;
        strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());
      #else
        strcat((char*)out_buffer_data_ptr, temp.c_str());

        temp = boost::lexical_cast<std::string>(strlen((char*)in_buffer_data_ptr));
        strcat((char*)out_buffer_data_ptr, temp.c_str());

        temp = g_crlf;
        strcat((char*)out_buffer_data_ptr, temp.c_str());
      #endif
    }
  } else {
    if (PROP_GET == format_id) {
      temp = "GET";
      #ifdef _WIN32
        strcpy_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());

        /// Add Space
        temp = g_space;
        strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());
      #else
        strcpy((char*)out_buffer_data_ptr, temp.c_str());

        /// Add Space
        temp = g_space;
        strcat((char*)out_buffer_data_ptr, temp.c_str());
      #endif

      /// Add Request Path
      if (true == path.empty()) {
        temp = "/";
      } else {
        temp = path;
      }
      
      #ifdef _WIN32
        strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());
      #else
        strcat((char*)out_buffer_data_ptr, temp.c_str());
      #endif

      /// Add Data
      if (0 != form_message_context_ptr->data_info.size()) {
        temp = "?";
        #ifdef _WIN32
          strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());
        #else
          strcat((char*)out_buffer_data_ptr, temp.c_str());
        #endif

        for (
          HTTPProtocolParamsIterator iterator = form_message_context_ptr->data_info.begin();
          iterator != form_message_context_ptr->data_info.end();
        ) {
          temp = iterator->first;
          #ifdef _WIN32
            strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());

            temp = "=";
            strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());

            temp = iterator->second;
            strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());
          #else
            strcat((char*)out_buffer_data_ptr, temp.c_str());

            temp = "=";
            strcat((char*)out_buffer_data_ptr, temp.c_str());

            temp = iterator->second;
            strcat((char*)out_buffer_data_ptr, temp.c_str());
          #endif

          ++iterator;

          if (iterator != form_message_context_ptr->data_info.end()) {
            temp = "&";
            #ifdef _WIN32
              strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());
            #else
              strcat((char*)out_buffer_data_ptr, temp.c_str());
            #endif
          }
        }
      } /// end of if (0 != form_message_context_ptr->data_info.size())

      temp = g_space;
      #ifdef _WIN32
        strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());

        /// Set HTTP Version
        temp = "HTTP/1.1";
        strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());

        temp = g_crlf;
        strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());
      #else
        strcat((char*)out_buffer_data_ptr, temp.c_str());

        /// Set HTTP Version
        temp = "HTTP/1.1";
        strcat((char*)out_buffer_data_ptr, temp.c_str());

        temp = g_crlf;
        strcat((char*)out_buffer_data_ptr, temp.c_str());
      #endif

    } else if (PROP_POST == format_id) {
      temp = "POST";
      #ifdef _WIN32
        strcpy_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());

        /// Add Space
        temp = g_space;
        strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());
      #else
        strcpy((char*)out_buffer_data_ptr, temp.c_str());

        /// Add Space
        temp = g_space;
        strcat((char*)out_buffer_data_ptr, temp.c_str());
      #endif

      /// Add Request Path
      if (true == path.empty()) {
        temp = "/";
      } else {
        temp = path;
      }

      #ifdef _WIN32
        strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());

        temp = g_space;
        strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());

        /// Set HTTP Version
        temp = "HTTP/1.1";
        strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());

        temp = g_crlf;
        strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());
      #else
        strcat((char*)out_buffer_data_ptr, temp.c_str());

        temp = g_space;
        strcat((char*)out_buffer_data_ptr, temp.c_str());

        /// Set HTTP Version
        temp = "HTTP/1.1";
        strcat((char*)out_buffer_data_ptr, temp.c_str());

        temp = g_crlf;
        strcat((char*)out_buffer_data_ptr, temp.c_str());
      #endif
      /// Add Data Length
      if (0 < strlen((char*)in_buffer_data_ptr)) {
        temp = "Content-Length: ";
        #ifdef _WIN32
          strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());

          temp = boost::lexical_cast<std::string>(strlen((char*)in_buffer_data_ptr));
          strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());

          temp = g_crlf;
          strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());
        #else
          strcat((char*)out_buffer_data_ptr, temp.c_str());

          temp = boost::lexical_cast<std::string>(strlen((char*)in_buffer_data_ptr));
          strcat((char*)out_buffer_data_ptr, temp.c_str());

          temp = g_crlf;
          strcat((char*)out_buffer_data_ptr, temp.c_str());
        #endif
      }
    }

    /// Set Host
    temp = "Host: ";
    #ifdef _WIN32
      strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());

      temp = domain;
      strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());
    #else
      strcat((char*)out_buffer_data_ptr, temp.c_str());

      temp = domain;
      strcat((char*)out_buffer_data_ptr, temp.c_str());
    #endif  

    /// Set Port
    if (false == port.empty()) {
      temp = ":";
      #ifdef _WIN32
        strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());

        temp = port;
        strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());
      #else
        strcat((char*)out_buffer_data_ptr, temp.c_str());

        temp = port;
        strcat((char*)out_buffer_data_ptr, temp.c_str());
      #endif
    }

    temp = g_crlf;
    #ifdef _WIN32
      strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());
    #else
      strcat((char*)out_buffer_data_ptr, temp.c_str());
    #endif
  }

  if (true == form_message_context_ptr->use_default_header) {
    /// Set Connection Option
    temp = "Connection: keep-alive";
    #ifdef _WIN32
      strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());

      temp = g_crlf;
      strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());

      /// Set Acception Option
      temp = "Accept: */*";
      strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());

      temp = g_crlf;
      strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());
    #else
      strcat((char*)out_buffer_data_ptr, temp.c_str());

      temp = g_crlf;
      strcat((char*)out_buffer_data_ptr, temp.c_str());

      /// Set Acception Option
      temp = "Accept: */*";
      strcat((char*)out_buffer_data_ptr, temp.c_str());

      temp = g_crlf;
      strcat((char*)out_buffer_data_ptr, temp.c_str());
    #endif
  } else {
    /// Set Other Header Info
    if (0 < form_message_context_ptr->header_info.size()) {
      for (
        HTTPProtocolParamsIterator iterator = form_message_context_ptr->header_info.begin();
        iterator != form_message_context_ptr->header_info.end();
        ++iterator) {

        temp = iterator->first;

        if (
          g_request_header_tag_http_version == temp ||
          g_response_header_tag_status_code == temp ||
          g_response_header_tag_reason_phrase == temp
        ) {
          continue;
        }

        #ifdef _WIN32
          strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());
          temp = ":";
          strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());
          temp = g_space;
          strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());
          temp = iterator->second;
          strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());
          temp = g_crlf;
          strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());
        #else
          strcat((char*)out_buffer_data_ptr, temp.c_str());
          temp = ":";
          strcat((char*)out_buffer_data_ptr, temp.c_str());
          temp = g_space;
          strcat((char*)out_buffer_data_ptr, temp.c_str());
          temp = iterator->second;
          strcat((char*)out_buffer_data_ptr, temp.c_str());
          temp = g_crlf;
          strcat((char*)out_buffer_data_ptr, temp.c_str());
        #endif
      }
    }
  }

  /// Set End Tag
  temp = g_crlf;
  #ifdef _WIN32
    strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());
    temp = g_crlf;
    strcat_s((char*)out_buffer_data_ptr, out_buffer_length, temp.c_str());
  #else
    strcat((char*)out_buffer_data_ptr, temp.c_str());
    temp = g_crlf;
    strcat((char*)out_buffer_data_ptr, temp.c_str());
  #endif

  if (
    PROP_POST == format_id ||
    PROP_RESPONSE == format_id
  ) {
    if (0 < strlen((char*)in_buffer_data_ptr)) {
      #ifdef _WIN32
        strcat_s((char*)out_buffer_data_ptr, out_buffer_length, (char*)in_buffer_data_ptr);
      #else
        strcat((char*)out_buffer_data_ptr, (char*)in_buffer_data_ptr);
      #endif

    }
  }

  return YSOS_ERROR_SUCCESS;
}

void HTTPProtocolImpl::ParseURL(const std::string url, std::string& protocol, std::string& domain, std::string& port, std::string& path, std::string& query, std::string& fragment) {

  //"https://www.google.com:443/webhp?gws_rd=ssl#q=cpp";
  boost::regex ex("(http|https)://([^/ :]+):?([^/ ]*)(/?[^ #?]*)\\x3f?([^ #]*)#?([^ ]*)");
  boost::cmatch what;

  if (boost::regex_match(url.c_str(), what, ex)) {
    protocol = std::string(what[1].first, what[1].second);//https
    domain = std::string(what[2].first, what[2].second);//www.google.com
    port = std::string(what[3].first, what[3].second);//443
    path = std::string(what[4].first, what[4].second);//webhp
    query = std::string(what[5].first, what[5].second);//gws_rd=ssl
    fragment = std::string(what[6].first, what[6].second);//q=cpp
  }

  return;
}

} /// namespace ysos

/* GET方式请求报文示例 */
//     GET /css/news070130.css HTTP/1.1 \r\n
//     Accept: */* \r\n
//     Referer: http://news.qq.com/a/20081224/000088.htm \r\n
//     Accept-Language: zh-cn \r\n
//     Accept-Encoding: gzip, deflate \r\n
//     If-Modified-Since: Thu, 29 Mar 2007 02:05:43 GMT; length=11339 \r\n
//     User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; .NET CLR 1.1.4322; .NET CLR 2.0.50727) \r\n
//     Host: news.qq.com \r\n
//     Connection: Keep-Alive \r\n
//     Cookie: ssid=idlsesels; flv=9.0; icache=ADLFMBGC; adid=adsdifids; isflucky_50829637=0; \r\n
//     \r\n

/* POST方式请求报文示例 */
//     POST /Login.php HTTP/1.1 \r\n
//     Accept: image/gif, */* \r\n
//     Accept-language: zh-cn \r\n
//     Accept-encodeing: gzip \r\n
//     User-Agent: MSIE6.0 \r\n
//     Host: www.some.com \r\n
//     Connection: Keep-Alive \r\n
//     Content-Length: 7 \r\n
//     Cache-Control:no-cache \r\n
//     Cookie: name1=value1; name2=value2; \r\n
//     \r\n
//     username=b&passwd=d
//     \r\n

/* 响应报文示例 */
//     HTTP/1.1 200 OK \r\n
//     Cache-Control: private, max-age=0 \r\n
//     Date: Fri, 02 Jan 2009 12:26:17 GMT \r\n
//     Expires: -1 \r\n
//     Content-Type: text/html; charset=GB2312 \r\n
//     Set-Cookie: PREF=ID=7bbe374f53b6c6a8:NW=1:TM=1230899177:LM=1230899177:S=2EGHuZJnrtdQUB_A; expires=Sun, 02-Jan-2011 12:26:17 GMT; path=/; domain=.google.com \r\n
//     Server: gws \r\n
//     Transfer-Encoding: chunked \r\n
//     Connection: Close \r\n
//     \r\n
//     <html>
//     html data
//     </html>
