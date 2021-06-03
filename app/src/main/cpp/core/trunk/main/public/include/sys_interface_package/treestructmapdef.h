/**
 *@file TreeStructMapDef.h
 *@brief Definition of TreeStructMap
 *@version 0.1
 *@author XuLanyue
 *@date Created on: 2016-05-03 18:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
/*
约定://NOLINT
不支持多重继承.//NOLINT
函数成员的声明应放到最后.//NOLINT
函数成员的实现应写到结构体定义之外.//NOLINT
小端排序.//NOLINT
以1字节为边界对齐.//NOLINT
//  #pragma pack(1)
数据成员需要指出其在对应的xml中是作为属性还是作为子节点(默认).//NOLINT
对于数组形式的内容需要指定是以std::vector还是普通的c++风格数组(默认)存储.//NOLINT
警告:Json会对结构体成员按Key进行排序.//NOLINT
*/
//  NextToDo:
//    考虑支持基类//NOLINT
//    考虑支持vector//NOLINT
//    GetBaseStructItemInfoList
//    对xml的支持//NOLINT
#ifndef TREE_STRUCT_MAP_DEF__H                                          //LINT
#define TREE_STRUCT_MAP_DEF__H                                          //LINT

#include "../../../public/include/sys_interface_package/common.h"

#ifdef _WIN32
  #include <string>
#else
  #include <cstring>
#endif

#include <map>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <stdio.h>
#include <stddef.h>

namespace ysos {

#define TREE_STRUCT_MAP_XML_MEMBER_AS_ATTRIBUTE     1
#define TREE_STRUCT_MAP_ARRAY_SAVE_AS_VECTOR        16

/*
根据json标准, 一个json是一个null, false, true, int, double, array或object.
为了使用TeamParams接口, 我们给json值的子项进行编号.
例如: 如下的json
{
  "name" :
  {
    "id" : 123987 ,
    "aka" : "T.E.S.T."
  },
  "count" : 1234,
  "attribute" :
  [
    "short",
    "random",
    "bold",
    12,
    {
      "height" : 7.2,
      "width" : 64.3
    }
  ],
  "test":
  {
    "1" :
    {
      "coord" :
      [
        1
        2
      ]
    }
  }
}
给其各子项的编号如下:
{
  "name" :                          //1_0_0_0, 0x0001000000000000
  {
    "id" : 123987 ,                 //1_1_0_0, 0x0001000100000000
    "aka" : "T.E.S.T."              //1_2_0_0, 0x0001000200000000
  },
  "count" : 1234,                   //2_0_0_0, 0x0002000000000000
  "attribute" :                     //3_0_0_0, 0x0003000000000000
  [
    "short",                        //3_1_0_0, 0x0003000100000000
    "random",                       //3_2_0_0, 0x0003000200000000
    "bold",                         //3_3_0_0, 0x0003000300000000
    12,                             //3_4_0_0, 0x0003000400000000
    {                               //3_5_0_0, 0x0003000500000000
      "height" : 7.2,               //3_5_1_0, 0x0003000500010000
      "width" : 64.3                //3_5_2_0, 0x0003000500020000
    }
  ],
  "test":                           //4_0_0_0, 0x0004000000000000
  {
    "1" :                           //4_1_0_0, 0x0004000100000000
    {
      "coord" :                     //4_1_1_0, 0x0004000100010000
      [
        1                           //4_1_1_1, 0x0004000100010001
        2                           //4_1_1_2, 0x0004000100010002
      ]
    }
  }
}
0x0003000500020000对应"width" : 64.3.
0_0_0_0, 0x0000000000000000, 对应根值.
这里, json最大层数暂定为4.且每层从数字1开始计数.
*/

/*
类似地,对于xml我们进行完全类似的规定.
XML最深只允许四层
每层从数字1开始计数, 如
  <a>                               // 0x0001000000000000
    <b/>                            // 0x0001000100000000
    <c>                             // 0x0001000200000000
      <d/>                          // 0x0001000200010000
    </c>
  </a>
  <h>
    <I/>
  </h>
0x0001000200010000对应<d/>
0x0000000000000000为Declaration
*/
//  NextToDo: 8, 动态, //NOLINT
struct HierarchyFormatIdStruct {
  uint16_t level_1;     // 第一层的位置//NOLINT
  uint16_t level_2;     // 第二层的位置//NOLINT
  uint16_t level_3;     // 第三层的位置//NOLINT
  uint16_t level_4;     // 第四层的位置//NOLINT

  /*
    每层用16位表示
  */
  HierarchyFormatIdStruct(uint64_t uFormatId) {
    level_1 = (uFormatId >> 48) & 0xFFFF;
    level_2 = (uFormatId >> 32) & 0xFFFF;
    level_3 = (uFormatId >> 16) & 0xFFFF;
    level_4 = uFormatId & 0xFFFF;
  }

  uint64_t FormatID(
    const uint16_t level_1,
    const uint16_t level_2,
    const uint16_t level_3,
    const uint16_t level_4) {
    return (((uint64_t)(level_1 & 0xFFFF) << 48)
      | ((uint64_t)(level_2 & 0xFFFF) << 32)
      | ((uint64_t)(level_3 & 0xFFFF) << 16)
      | (level_4 & 0xFFFF));
  }
};
}

//  STRINGIZE
#define MAKE_STR(Token) #Token
#define OFFSET_OF(StructName, member) (size_t)((int64_t)&reinterpret_cast<const volatile char&>((((StructName *)0)->member)))

template<typename T> void TreeStructMap_Json_AddStruct(
  T* struct_ptr,
  const char* struct_name,
  const char* mapped_name,
  std::string& json_string_of_struct,
  bool add_name,
  uint32_t index) {
}

// Macros for definition of structs
//
#define BEGIN_DEF_STRUCT_WITH0BASE___S(StructName)  \
struct StructName {

#define BEGIN_DEF_STRUCT_WITH1BASE___S(StructName, derivemode1, BaseClassName1)  \
struct StructName : derivemode1 BaseClassName1 {

#define ADD_BASIC_SINGLE_T___S(type, member, size, mappedname, options) \
  type member;

#define ADD_BASIC_SINGLE_N___S(type, member, size, mappedname, options) \
  type member;

#define ADD_BASIC_ARRAY_T___S(type, member, size, mappedname, options) \
  type member[size];

#define ADD_BASIC_ARRAY_N___S(type, member, size, mappedname, options) \
  type member[size];

#define ADD_STRUCT_SINGLE_T___S(type, member, size, mappedname, options) \
  type member;

#define ADD_STRUCT_SINGLE_N___S(type, member, size, mappedname, options) \
  type member;

#define ADD_STRUCT_ARRAY_T___S(type, member, size, mappedname, options) \
  type member[size];

#define ADD_STRUCT_ARRAY_N___S(type, member, size, mappedname, options) \
  type member[size];

#define ADD_FUNCTION_DECLARATION___S(FunctionDeclaration) \
  FunctionDeclaration;

#define END_DEF_STRUCT___S(StructName) \
};    /*End definition of StructName*/


struct StructItemInfoInTree {
  bool add_to_tree;
  std::string data_type_string;
  std::string member_name_string;
  size_t amount_of_element;
  bool is_struct;
  size_t offset_of_element;
  //  bool defined_in_base;
  StructItemInfoInTree() {
    add_to_tree = false;
    data_type_string = "";
    member_name_string = "";
    amount_of_element = 0;
    is_struct = false;
    offset_of_element = 0;
    //  defined_in_base = false;
  }

  StructItemInfoInTree(
    bool add_to_tree_ref,
    std::string data_type_string_ref,
    size_t amount_of_element_ref,
    std::string member_name_string_ref,
    bool is_struct_ref,
    size_t offset_of_element_ref
    //  , size_t offset_of_element_ref
  ) {
    add_to_tree = add_to_tree_ref;
    data_type_string = data_type_string_ref;
    member_name_string = member_name_string_ref;
    amount_of_element = amount_of_element_ref;
    is_struct = is_struct_ref;
    offset_of_element = offset_of_element_ref;
    //  defined_in_base = defined_in_base_ref;
  }

  StructItemInfoInTree(
    const StructItemInfoInTree& ref) {
    add_to_tree = ref.add_to_tree;
    data_type_string = ref.data_type_string;
    member_name_string = ref.member_name_string;
    amount_of_element = ref.amount_of_element;
    is_struct = ref.is_struct;
    offset_of_element = ref.offset_of_element;
    //defined_in_base = ref.defined_in_base;
  }
};

bool TreeStructMap_Json_isControlCharacter(char ch);
bool TreeStructMap_Json_containsControlCharacter(const char* str);
void TreeStructMap_Json_uintToString(unsigned int value, char* &current);
std::string TreeStructMap_Json_valueToString(int value);
std::string TreeStructMap_Json_valueToString(unsigned int value);
std::string TreeStructMap_Json_valueToString(double value);
std::string TreeStructMap_Json_valueToString(bool value);
std::string TreeStructMap_Json_valueToQuotedString(const char* value);

void GetAddedElementAmountOfStruct(
  std::vector<StructItemInfoInTree>* item_info_ptr,
  uint32_t* added_amount_ptr);
void GetElementAmountOfStruct(
  std::vector<StructItemInfoInTree>* item_info_ptr,
  uint32_t* total_amount_ptr,
  uint32_t* added_amount_ptr);
bool IsSeparatorRequired(
  uint32_t element_index_based_1,
  std::vector<StructItemInfoInTree>* self,
  std::vector<StructItemInfoInTree>* base = NULL);

void TreeStructMap_Json_BeginArray(
  const char* array_name,
  std::string& json_string_of_struct);
void TreeStructMap_Json_BeginStruct(
  const char* struct_name,
  std::string& json_string_of_struct);
void TreeStructMap_Json_BeginArray(std::string& json_string_of_struct);
void TreeStructMap_Json_BeginStruct(std::string& json_string_of_struct);
void TreeStructMap_Json_SeparateNextForElement(
  std::string& json_string_of_struct);
void TreeStructMap_Json_EndArray(
  std::string& json_string_of_struct);
void TreeStructMap_Json_EndStruct(
  std::string& json_string_of_struct);

// Macros for definitions of the corresponding item detail info of a struct
//

#define BEGIN_STRUCT_INFO_LIST(StructName)  \
static std::vector<StructItemInfoInTree> g_StructItemInfoList_##StructName;  \
std::vector<StructItemInfoInTree>* getStructItemInfoList_##StructName() {  \
  if (g_StructItemInfoList_##StructName.size() == 0) { 

#define END_STRUCT_INFO_LIST(StructName)  \
  } \
  return &g_StructItemInfoList_##StructName; \
}

#define BEGIN_DEF_STRUCT_WITH0BASE___D(StructName)  \
std::vector<StructItemInfoInTree>* getBaseStructItemInfoListOf_##StructName() { \
  return (std::vector<StructItemInfoInTree>*)NULL;} \
BEGIN_STRUCT_INFO_LIST(StructName)

#define BEGIN_DEF_STRUCT_WITH1BASE___D(StructName, derivemode1, BaseClassName1)  \
std::vector<StructItemInfoInTree>* getBaseStructItemInfoListOf_##StructName() { \
  return getStructItemInfoListOf_##BaseClassName1();} \
BEGIN_STRUCT_INFO_LIST(StructName)

#define ADD_BASIC_SINGLE_T___D(StructName, type, member, size, mappedname, options) \
    { \
      StructItemInfoInTree item(true, MAKE_STR(type), 0, MAKE_STR(mappedname), false, OFFSET_OF(StructName, member)); \
      g_StructItemInfoList_##StructName.push_back(item); \
    }

#define ADD_BASIC_SINGLE_N___D(StructName, type, member, size, mappedname, options) \
    { \
      StructItemInfoInTree item(false, MAKE_STR(type), 0, "", false, OFFSET_OF(StructName, member)); \
      g_StructItemInfoList_##StructName.push_back(item); \
    }

#define ADD_BASIC_ARRAY_T___D(StructName, type, member, size, mappedname, options) \
    { \
      StructItemInfoInTree item(true, MAKE_STR(type), size, MAKE_STR(mappedname), false, OFFSET_OF(StructName, member)); \
      g_StructItemInfoList_##StructName.push_back(item); \
    }

#define ADD_BASIC_ARRAY_N___D(StructName, type, member, size, mappedname, options) \
    { \
      StructItemInfoInTree item(false, MAKE_STR(type), size, "", false, OFFSET_OF(StructName, member)); \
      g_StructItemInfoList_##StructName.push_back(item); \
    }

#define ADD_STRUCT_SINGLE_T___D(StructName, type, member, size, mappedname, options) \
    { \
      StructItemInfoInTree item(true, MAKE_STR(type), 0, MAKE_STR(mappedname), true, OFFSET_OF(StructName, member)); \
      g_StructItemInfoList_##StructName.push_back(item); \
    }

#define ADD_STRUCT_SINGLE_N___D(StructName, type, member, size, mappedname, options) \
    { \
      StructItemInfoInTree item(false, MAKE_STR(type), 0, "", true, OFFSET_OF(StructName, member)); \
      g_StructItemInfoList_##StructName.push_back(item); \
    }

#define ADD_STRUCT_ARRAY_T___D(StructName, type, member, size, mappedname, options) \
    { \
      StructItemInfoInTree item(true, MAKE_STR(type), size, MAKE_STR(mappedname), true, OFFSET_OF(StructName, member)); \
      g_StructItemInfoList_##StructName.push_back(item); \
    }

#define ADD_STRUCT_ARRAY_N___D(StructName, type, member, size, mappedname, options) \
    { \
      StructItemInfoInTree item(false, MAKE_STR(type), size, "", true, OFFSET_OF(StructName, member)); \
      g_StructItemInfoList_##StructName.push_back(item); \
    }

#define ADD_FUNCTION_DECLARATION___D(FunctionDeclaration) 

#define END_DEF_STRUCT___D(StructName) \
    { \
      StructItemInfoInTree item(false, "", 0, "", false, 0xffffffff); \
      g_StructItemInfoList_##StructName.push_back(item); \
    }  \
END_STRUCT_INFO_LIST(StructName)  \
void getElementAmountOfStruct_##StructName(uint32_t* element_amount_ptr, uint32_t* element_amount_add_ptr) { \
  GetElementAmountOfStruct( \
  getStructItemInfoList_##StructName(), element_amount_ptr, element_amount_add_ptr); \
} \
bool IsSeparatorRequired_##StructName(uint32_t element_index_based_1) { \
  return IsSeparatorRequired( \
    element_index_based_1, \
    getStructItemInfoList_##StructName(), \
    getBaseStructItemInfoListOf_##StructName()); \
} \
void TreeStructMap_Json_AddSeparatorIfRequired_##StructName(uint32_t element_index, std::string& json_string_of_struct) { \
  if (IsSeparatorRequired_##StructName(element_index)) TreeStructMap_Json_SeparateNextForElement(json_string_of_struct); \
}

int TreeStructMap_Json_GetStructFromString(
  const std::string& json_string,
  std::vector<StructItemInfoInTree>* struct_item_info_list_ptr,
  void* struct_ptr);

//  element_level: 0, top

template <typename T>
void TreeStructMap_Json_AddBasicSingle(
  T* element_value_ptr,
  const std::string& data_type_string,
  const std::string& member_name_string,
  std::string& json_string_of_struct,
  bool add_name) {
  if (add_name) {
    json_string_of_struct += TreeStructMap_Json_valueToQuotedString(member_name_string.c_str());
    json_string_of_struct += ":";
  }
  if (data_type_string == "int") {
    json_string_of_struct += TreeStructMap_Json_valueToString(*(int*)element_value_ptr);
  } else if (data_type_string == "unsigned int") {
    json_string_of_struct += TreeStructMap_Json_valueToString(*(int*)element_value_ptr);
  } else if (data_type_string == "double") {
    json_string_of_struct += TreeStructMap_Json_valueToString(*(double*)element_value_ptr);
  } else if (data_type_string == "bool") {
    json_string_of_struct += TreeStructMap_Json_valueToString(*(bool*)element_value_ptr);
  } else if (data_type_string == "std::string") {
    json_string_of_struct += TreeStructMap_Json_valueToQuotedString(
      ((std::string*)element_value_ptr)->c_str());
  }
}

template <typename T>
void TreeStructMap_Json_AddBasicArray(
  void* element_value_ptr,
  const std::string& data_type_string,
  const std::string& member_name_string,
  uint32_t amount_of_element,
  std::string& json_string_of_struct,
  bool add_name) {
    if (add_name) {
      json_string_of_struct += TreeStructMap_Json_valueToQuotedString(member_name_string.c_str());
      json_string_of_struct += ":";
    }
    TreeStructMap_Json_BeginArray(json_string_of_struct);
    //  添加数组成员//NOLINT
    for (uint32_t i = 0; i < amount_of_element; ++i) {
      if (i > 0) {
        TreeStructMap_Json_SeparateNextForElement(json_string_of_struct);
      }
      TreeStructMap_Json_AddBasicSingle<T>(
        &((T*)element_value_ptr)[i],
        data_type_string,
        member_name_string,
        json_string_of_struct,
        false);
    }
    TreeStructMap_Json_EndArray(json_string_of_struct);
}

template <typename T> void TreeStructMap_Json_AddStructArray(
  T* element_value_ptr,
  const std::string& member_name_string,
  const char* struct_name,
  uint32_t amount_of_element,
  std::string& json_string_of_struct,
  bool add_name) {
  if (add_name) {
    json_string_of_struct += TreeStructMap_Json_valueToQuotedString(member_name_string.c_str());
    json_string_of_struct += ":";
  }
  TreeStructMap_Json_BeginArray(json_string_of_struct);
  //  添加数组成员//NOLINT
  for (uint32_t i = 0; i < amount_of_element; ++i) {
    if (i > 0) {
      TreeStructMap_Json_SeparateNextForElement(json_string_of_struct);
    }
    TreeStructMap_Json_AddStruct<T>(
      &((T*)element_value_ptr)[i],
      struct_name,
      "",
      json_string_of_struct,
      false,
      0);
  }
  TreeStructMap_Json_EndArray(json_string_of_struct);
}

#define BEGIN_DEF_STRUCT_WITH0BASE___J(StructName)  \
void getElementAmountOfBaseOfStruct_##StructName(uint32_t* element_amount_ptr, uint32_t* element_amount_add_ptr) { \
  if (element_amount_ptr) *element_amount_ptr = 0; if (element_amount_add_ptr) *element_amount_add_ptr = 0; \
} \
template<> void TreeStructMap_Json_AddStruct<StructName>( \
  StructName* struct_ptr, \
  const char* struct_name, \
  const char* mapped_name, \
  std::string& json_string_of_struct, \
  bool add_name, \
  uint32_t index) { \
  if (struct_ptr) { \
  if (add_name) { \
  json_string_of_struct += TreeStructMap_Json_valueToQuotedString(struct_name); \
  json_string_of_struct += ":"; \
}  \
TreeStructMap_Json_BeginStruct(json_string_of_struct); \
  /*Add each element of base to the struct*/

#define BEGIN_DEF_STRUCT_WITH1BASE___J(StructName)  \
void getElementAmountOfBaseOfStruct_##StructName(uint32_t* element_amount_ptr, uint32_t* element_amount_add_ptr) { \
  GetElementAmountOfStruct( \
  getBaseStructItemInfoListOf_##StructName(), element_amount_ptr, element_amount_add_ptr); \
} \
template<> void TreeStructMap_Json_AddStruct<StructName>( \
  StructName* struct_ptr, \
  const char* struct_name, \
  const char* mapped_name, \
  std::string& json_string_of_struct, \
  bool add_name, \
  uint32_t index) { \
  if (struct_ptr) { \
  if (add_name) { \
  json_string_of_struct += TreeStructMap_Json_valueToQuotedString(struct_name); \
  json_string_of_struct += ":"; \
}  \
TreeStructMap_Json_BeginStruct(json_string_of_struct); \
  /*Add each element of base to the struct*/

#define ADD_BASIC_SINGLE_T___J(struct_ptr, StructName, type, member, size, mappedname, options, index) \
  TreeStructMap_Json_AddSeparatorIfRequired_##StructName(index, json_string_of_struct); \
  TreeStructMap_Json_AddBasicSingle<type>( \
  (type*)&((StructName*)struct_ptr)->member, \
  MAKE_STR(type), \
  MAKE_STR(mappedname), \
  json_string_of_struct, true);

#define ADD_BASIC_SINGLE_N___J(struct_ptr, StructName, type, member, size, mappedname, options, index)  

#define ADD_BASIC_ARRAY_T___J(struct_ptr, StructName, type, member, size, mappedname, options, index) \
  TreeStructMap_Json_AddSeparatorIfRequired_##StructName(index, json_string_of_struct); \
  TreeStructMap_Json_AddBasicArray<type>( \
  (type*)&((StructName*)struct_ptr)->member, \
  MAKE_STR(type), \
  MAKE_STR(mappedname), \
  size, \
  json_string_of_struct, true);

#define ADD_BASIC_ARRAY_N___J(struct_ptr, StructName, type, member, size, mappedname, options, index)  

#define ADD_STRUCT_SINGLE_T___J(struct_ptr, StructName, type, member, size, mappedname, options, index) \
  TreeStructMap_Json_AddSeparatorIfRequired_##StructName(index, json_string_of_struct); \
  TreeStructMap_Json_AddStruct<type>( \
  (type*)&((StructName*)struct_ptr)->member, \
  MAKE_STR(StructName), \
  MAKE_STR(mappedname), \
  json_string_of_struct, true, index);

#define ADD_STRUCT_SINGLE_N___J(struct_ptr, StructName, type, member, size, mappedname, options, index)  

#define ADD_STRUCT_ARRAY_T___J(struct_ptr, StructName, type, member, size, mappedname, options, index) \
  TreeStructMap_Json_AddSeparatorIfRequired_##StructName(index, json_string_of_struct); \
  TreeStructMap_Json_AddStructArray<type>( \
  (type*)&((StructName*)struct_ptr)->member, \
  MAKE_STR(mappedname), \
  MAKE_STR(member), \
  size, \
  json_string_of_struct, true);

#define ADD_STRUCT_ARRAY_N___J(struct_ptr, StructName, type, member, size, mappedname, options, index)  

#define ADD_FUNCTION_DECLARATION___J(FunctionDeclaration)  

#define END_DEF_STRUCT___J(StructName) \
  TreeStructMap_Json_EndStruct(json_string_of_struct);}}


// Macros for add json info of structs
//
typedef const char** (*GetJsonInfoOfStructFunc)(void);

#define BEGIN_ADD_JSON_INFO_OF_STRUCT_TO_MAP \
  std::map<std::string, GetJsonInfoOfStructFunc> g_mapJsonInfoOfStruct; \
  std::map<std::string, GetJsonInfoOfStructFunc>* getJsonInfoOfStructMap() {

#define ADD_JSON_INFO_OF_STRUCT(StructName) \
  g_mapJsonInfoOfStruct[MAKE_STR(StructName)] = &getJsonInfoOfStruct_##StructName;

#define END_ADD_JSON_INFO_OF_STRUCT_TO_MAP \
  return &g_mapJsonInfoOfStruct;}

#define GET_MEMBER_VALUE_OF_STRUCT(StructName, StructInstance, Type, iMemberIdx) \
  *reinterpret_cast<Type*>((char*)&StructInstance + getOffsetOfMemberInStruct_##StructName[iMemberIdx])

#define DEF_STRUCT_WITH0BASE_2(BEGIN_DEF_STRUCT_WITH0BASE, StructName, \
  _0011, _0012, _0013, _0014, _0015, _0016, \
  _0021, _0022, _0023, _0024, _0025, _0026, ...) \
  BEGIN_DEF_STRUCT_WITH0BASE##___S(StructName) \
  _0011##___S(_0012, _0013, _0014, _0015, _0016) \
  _0021##___S(_0022, _0023, _0024, _0025, _0026) \
  __VA_ARGS__ \
  END_DEF_STRUCT___S(StructName) \
  BEGIN_DEF_STRUCT_WITH0BASE##___D(StructName) \
  _0011##___D(StructName, _0012, _0013, _0014, _0015, _0016) \
  _0021##___D(StructName, _0022, _0023, _0024, _0025, _0026) \
  END_DEF_STRUCT___D(StructName) \
  BEGIN_DEF_STRUCT_WITH0BASE##___J(StructName) \
  _0011##___J(struct_ptr, StructName, _0012, _0013, _0014, _0015, _0016, 1) \
  _0021##___J(struct_ptr, StructName, _0022, _0023, _0024, _0025, _0026, 2) \
  END_DEF_STRUCT___J(StructName)

#define DEF_STRUCT_WITH0BASE_3(BEGIN_DEF_STRUCT_WITH0BASE, StructName, \
  _0011, _0012, _0013, _0014, _0015, _0016, \
  _0021, _0022, _0023, _0024, _0025, _0026, \
  _0031, _0032, _0033, _0034, _0035, _0036, ...) \
  BEGIN_DEF_STRUCT_WITH0BASE##___S(StructName) \
  _0011##___S(_0012, _0013, _0014, _0015, _0016) \
  _0021##___S(_0022, _0023, _0024, _0025, _0026) \
  _0031##___S(_0032, _0033, _0034, _0035, _0036) \
  __VA_ARGS__ \
  END_DEF_STRUCT___S(StructName) \
  BEGIN_DEF_STRUCT_WITH0BASE##___D(StructName) \
  _0011##___D(StructName, _0012, _0013, _0014, _0015, _0016) \
  _0021##___D(StructName, _0022, _0023, _0024, _0025, _0026) \
  _0031##___D(StructName, _0032, _0033, _0034, _0035, _0036) \
  END_DEF_STRUCT___D(StructName) \
  BEGIN_DEF_STRUCT_WITH0BASE##___J(StructName) \
  _0011##___J(struct_ptr, StructName, _0012, _0013, _0014, _0015, _0016, 1) \
  _0021##___J(struct_ptr, StructName, _0022, _0023, _0024, _0025, _0026, 2) \
  _0031##___J(struct_ptr, StructName, _0032, _0033, _0034, _0035, _0036, 3) \
  END_DEF_STRUCT___J(StructName)

#define DEF_STRUCT_WITH0BASE_4(BEGIN_DEF_STRUCT_WITH0BASE, StructName, \
  _0011, _0012, _0013, _0014, _0015, _0016, \
  _0021, _0022, _0023, _0024, _0025, _0026, \
  _0031, _0032, _0033, _0034, _0035, _0036, \
  _0041, _0042, _0043, _0044, _0045, _0046, ...) \
  BEGIN_DEF_STRUCT_WITH0BASE##___S(StructName) \
  _0011##___S(_0012, _0013, _0014, _0015, _0016) \
  _0021##___S(_0022, _0023, _0024, _0025, _0026) \
  _0031##___S(_0032, _0033, _0034, _0035, _0036) \
  _0041##___S(_0042, _0043, _0044, _0045, _0046) \
  __VA_ARGS__ \
  END_DEF_STRUCT___S(StructName) \
  BEGIN_DEF_STRUCT_WITH0BASE##___D(StructName) \
  _0011##___D(StructName, _0012, _0013, _0014, _0015, _0016) \
  _0021##___D(StructName, _0022, _0023, _0024, _0025, _0026) \
  _0031##___D(StructName, _0032, _0033, _0034, _0035, _0036) \
  _0041##___D(StructName, _0042, _0043, _0044, _0045, _0046) \
  END_DEF_STRUCT___D(StructName) \
  BEGIN_DEF_STRUCT_WITH0BASE##___J(StructName) \
  _0011##___J(struct_ptr, StructName, _0012, _0013, _0014, _0015, _0016, 1) \
  _0021##___J(struct_ptr, StructName, _0022, _0023, _0024, _0025, _0026, 2) \
  _0031##___J(struct_ptr, StructName, _0032, _0033, _0034, _0035, _0036, 3) \
  _0041##___J(struct_ptr, StructName, _0042, _0043, _0044, _0045, _0046, 4) \
  END_DEF_STRUCT___J(StructName)

#define DEF_STRUCT_WITH0BASE_5(BEGIN_DEF_STRUCT_WITH0BASE, StructName, \
  _0011, _0012, _0013, _0014, _0015, _0016, \
  _0021, _0022, _0023, _0024, _0025, _0026, \
  _0031, _0032, _0033, _0034, _0035, _0036, \
  _0041, _0042, _0043, _0044, _0045, _0046, \
  _0051, _0052, _0053, _0054, _0055, _0056, ...) \
  BEGIN_DEF_STRUCT_WITH0BASE##___S(StructName) \
  _0011##___S(_0012, _0013, _0014, _0015, _0016) \
  _0021##___S(_0022, _0023, _0024, _0025, _0026) \
  _0031##___S(_0032, _0033, _0034, _0035, _0036) \
  _0041##___S(_0042, _0043, _0044, _0045, _0046) \
  _0051##___S(_0052, _0053, _0054, _0055, _0056) \
  __VA_ARGS__ \
  END_DEF_STRUCT___S(StructName) \
  BEGIN_DEF_STRUCT_WITH0BASE##___D(StructName) \
  _0011##___D(StructName, _0012, _0013, _0014, _0015, _0016) \
  _0021##___D(StructName, _0022, _0023, _0024, _0025, _0026) \
  _0031##___D(StructName, _0032, _0033, _0034, _0035, _0036) \
  _0041##___D(StructName, _0042, _0043, _0044, _0045, _0046) \
  _0051##___D(StructName, _0052, _0053, _0054, _0055, _0056) \
  END_DEF_STRUCT___D(StructName) \
  BEGIN_DEF_STRUCT_WITH0BASE##___J(StructName) \
  _0011##___J(struct_ptr, StructName, _0012, _0013, _0014, _0015, _0016, 1) \
  _0021##___J(struct_ptr, StructName, _0022, _0023, _0024, _0025, _0026, 2) \
  _0031##___J(struct_ptr, StructName, _0032, _0033, _0034, _0035, _0036, 3) \
  _0041##___J(struct_ptr, StructName, _0042, _0043, _0044, _0045, _0046, 4) \
  _0051##___J(struct_ptr, StructName, _0052, _0053, _0054, _0055, _0056, 5) \
  END_DEF_STRUCT___J(StructName)

#endif  //  TREE_STRUCT_MAP_DEF__H                                      //NOLINT
