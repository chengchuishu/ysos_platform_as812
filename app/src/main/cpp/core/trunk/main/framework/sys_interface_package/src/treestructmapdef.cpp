/*
# TreeStructMapDef.cpp
# Implement of TreeStructMap
# Created on: 2016-05-03 18:59:20
# Original author: XuLanyue
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
*/

#include "../../../public/include/sys_interface_package/treestructmapdef.h"
//#include <jsoncpp-src-0.5.0/include/json/json.h>
#include <json/json.h>

bool TreeStructMap_Json_isControlCharacter(char ch) {
  return (ch > 0 && ch <= 0x1F);
}

bool TreeStructMap_Json_containsControlCharacter(const char* str) {
  while (*str ) {
    if (TreeStructMap_Json_isControlCharacter(*(str++)))
      return true;
  }
  return false;
}

void TreeStructMap_Json_uintToString(unsigned int value, char* &current) {
  *--current = 0;
  do
  {
    *--current = (value % 10) + '0';
    value /= 10;
  }
  while (value != 0);
}

std::string TreeStructMap_Json_valueToString(int value) {
  char buffer[128];
  char *current = buffer + sizeof(buffer);
  bool isNegative = value < 0;
  if ( isNegative )
    value = -value;
  TreeStructMap_Json_uintToString((unsigned int)(value), current );
  if ( isNegative )
    *--current = '-';
  //  assert( current >= buffer );
  return current;
}

std::string TreeStructMap_Json_valueToString(unsigned int value) {
  char buffer[128];
  char *current = buffer + sizeof(buffer);
  TreeStructMap_Json_uintToString( value, current );
  //  assert( current >= buffer );
  return current;
}

std::string TreeStructMap_Json_valueToString(double value) {
  char buffer[128];
#if defined(_MSC_VER) && defined(__STDC_SECURE_LIB__)
  sprintf_s(buffer, sizeof(buffer), "%#.16g", value); 
#else	
  sprintf(buffer, "%#.16g", value); 
#endif
  char* ch = buffer + strlen(buffer) - 1;
  if (*ch != '0') return buffer;
  while(ch > buffer && *ch == '0'){
    --ch;
  }
  char* last_nonzero = ch;
  while(ch >= buffer){
    switch(*ch){
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      --ch;
      continue;
    case '.':
      *(last_nonzero+2) = '\0';
      return buffer;
    default:
      return buffer;
    }
  }
  return buffer;
}

std::string TreeStructMap_Json_valueToString(bool value) {
  return value ? "true" : "false";
}

std::string TreeStructMap_Json_valueToQuotedString(const char* value) {
  // NextToDo: unicode
  if (strpbrk(value, "\"\\\b\f\n\r\t") == NULL &&
    !TreeStructMap_Json_containsControlCharacter( value ))
    return std::string("\"") + value + "\"";
  unsigned maxsize = strlen(value)*2 + 3;
  std::string result;
  result.reserve(maxsize);
  result += "\"";
  for (const char* c = value; *c != 0; ++c)
  {
    switch(*c)
    {
    case '\"':
      result += "\\\"";
      break;
    case '\\':
      result += "\\\\";
      break;
    case '\b':
      result += "\\b";
      break;
    case '\f':
      result += "\\f";
      break;
    case '\n':
      result += "\\n";
      break;
    case '\r':
      result += "\\r";
      break;
    case '\t':
      result += "\\t";
      break;
      //case '/':
      // Even though \/ is considered a legal escape in JSON, a bare
      // slash is also legal, so I see no reason to escape it.
      // (I hope I am not misunderstanding something.
      // blep notes: actually escaping \/ may be useful in javascript to avoid </ 
      // sequence.
      // Should add a flag to allow this compatibility mode and prevent this 
      // sequence from occurring.
    default:
      if (TreeStructMap_Json_isControlCharacter( *c ) )
      {
        std::ostringstream oss;
        oss << "\\u" << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << static_cast<int>(*c);
        result += oss.str();
      }
      else
      {
        result += *c;
      }
      break;
    }
  }
  result += "\"";
  return result;
}

void GetAddedElementAmountOfStruct(
  std::vector<StructItemInfoInTree>* item_info_ptr,
  uint32_t* added_amount_ptr) {
    uint32_t element_amount = 0;
    uint32_t element_amount_add = 0;
    if (item_info_ptr && item_info_ptr->size() > 0) {
      element_amount = item_info_ptr->size() - 1;
      for (uint32_t i = 0; i <= element_amount; ++i)
      {
        if ((*item_info_ptr)[i].add_to_tree) {
          ++element_amount_add;
        }
      }
    }
    if (added_amount_ptr) *added_amount_ptr = element_amount_add;
}

void GetElementAmountOfStruct(
  std::vector<StructItemInfoInTree>* item_info_ptr,
  uint32_t* total_amount_ptr,
  uint32_t* added_amount_ptr) {
    uint32_t element_amount = 0;
    uint32_t element_amount_add = 0;
    if (item_info_ptr && item_info_ptr->size() > 0) {
      element_amount = item_info_ptr->size() - 1;
      for (uint32_t i = 0; i <= element_amount; ++i)
      {
        if ((*item_info_ptr)[i].add_to_tree) {
          ++element_amount_add;
        }
      }
    }
    if (total_amount_ptr) *total_amount_ptr = element_amount;
    if (added_amount_ptr) *added_amount_ptr = element_amount_add;
}

bool IsSeparatorRequired(
  uint32_t element_index_based_1,
  std::vector<StructItemInfoInTree>* self,
  std::vector<StructItemInfoInTree>* base) {
    bool return_value = false;
    if (self) {
      uint32_t base_element_amount_add = 0;
      uint32_t current_element_amount = 0;
      uint32_t element_amount = 0;
      uint32_t element_amount_add = 0;
      bool current_existing_added = false;
      GetAddedElementAmountOfStruct(base, &base_element_amount_add);
      if (base_element_amount_add == 0) {
        if (self && self->size() > 0) {
          element_amount = self->size();
          while (current_element_amount < element_amount) {
            if ((*self)[current_element_amount].add_to_tree) {
              ++element_amount_add;
            }
            ++current_element_amount;
            if (current_element_amount == element_index_based_1) {
              if (element_amount_add > 1) {
                return_value = true;
              }
            }
          }
        }
      } else {
        return_value = true;
      }
    }
    return return_value;
}

void TreeStructMap_Json_BeginArray(
  const char* array_name,
  std::string& json_string_of_struct) {
  json_string_of_struct += TreeStructMap_Json_valueToQuotedString(array_name);
  json_string_of_struct += ":[";
}

void TreeStructMap_Json_BeginStruct(
  const char* struct_name,
  std::string& json_string_of_struct) {
  json_string_of_struct += TreeStructMap_Json_valueToQuotedString(struct_name);
  json_string_of_struct += ":{";
}

void TreeStructMap_Json_BeginArray(std::string& json_string_of_struct) {
  json_string_of_struct += "[";
}

void TreeStructMap_Json_BeginStruct(std::string& json_string_of_struct) {
  json_string_of_struct += "{";
}

void TreeStructMap_Json_SeparateNextForElement(
  std::string& json_string_of_struct) {
  json_string_of_struct += ",";
}

void TreeStructMap_Json_EndArray(
  std::string& json_string_of_struct) {
  json_string_of_struct += "]";
}

void TreeStructMap_Json_EndStruct(
  std::string& json_string_of_struct) {
  json_string_of_struct += "}";
}

//  NextToDo:
int TreeStructMap_Json_GetStructFromString(
  const std::string& json_string,
  std::vector<StructItemInfoInTree>* struct_item_info_list_ptr,
  void* struct_ptr) {
  int return_value = YSOS_ERROR_FAILED;
  do {
    //  参数检测//NOLINT
    if (json_string.size() == 0
      || !struct_item_info_list_ptr
      || struct_item_info_list_ptr->size() <= 1
      || struct_ptr) {
        return_value = YSOS_ERROR_INVALID_ARGUMENTS;
        break;
    }
    Json::Value root;
    Json::Features features = Json::Features::strictMode();

    for (Json::Value::iterator it = root.begin(); it != root.end(); ++it) {
      //  it->key()
    }
    return_value = YSOS_ERROR_SUCCESS;
  } while (false);
  return return_value;
}

