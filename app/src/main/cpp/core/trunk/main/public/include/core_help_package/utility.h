/**
 *@file Utility.h
 *@brief Definition of Utility
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef CHP_UTILITY_H  //NOLINT
#define CHP_UTILITY_H  //NOLINT

/// stl headers //  NOLINT
#include <string>
#include <list>
#include <fstream>
#include <iostream>
#include <sstream>
/// boost headers //  NOLINT
#include <boost/shared_ptr.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <boost/thread.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
/// ysos private headers //  NOLINT
#include "../../../public/include/sys_interface_package/baseinterface.h"
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../protect/include/core_help_package/singletontemplate.h"
#include "../../../protect/include/core_help_package/log.h"

namespace ysos {
// T must be: no-throw default constructible and no-throw destructible
// template <typename T>
// class Singleton {
//  private:
//   class object_creator {
//    public:
//     // This constructor does nothing more than ensure that instance()
//     //  is called before main() begins, thus creating the static
//     //  T object before multithreading race issues can come up.
//     object_creator() {
//       Singleton::Instance();
//     }
//     inline void do_nothing() const { }
//   };
//   static object_creator create_object;
//
//   Singleton();
//
//  public:
//   typedef T object_type;
//
//   // If, at any point (in user code), singleton_default<T>::instance()
//   //  is called, then the following function is instantiated.
//   static object_type *Instance() {
//     // This is the object that we return a reference to.
//     // It is guaranteed to be created before main() begins because of
//     //  the next line.
//     static object_type obj;
//
//     // The following line does nothing else than force the instantiation
//     //  of singleton_default<T>::create_object, whose constructor is
//     //  called before main() begins.
//     create_object.do_nothing();
//
//     return &obj;
//   }
// };
// template <typename T>
// typename Singleton<T>::object_creator  Singleton<T>::create_object;

/**
  *@brief  工具类，使用方式：  //  NOLINT
  *        GetUtility()->ConvertHextStringToInt() ... //  NOLINT
  */
class Utility;
typedef boost::shared_ptr<Utility>  UtilityPtr;
class YSOS_EXPORT Utility: public BaseInterfaceImpl {
  DISALLOW_COPY_AND_ASSIGN(Utility);
  DECLARE_PROTECT_CONSTRUCTOR(Utility);

// friend class Singleton<Utility>;

 public:
  ~Utility();

  enum EncodingType {
    ASCII,   ///<  ascii, gb2312. etc  //  NOLINT
    UTF8,    ///<  utf-8 //  NOLINT
    UTF16,   ///<  unicode //  NOLINT
    EncodingTypeEnd
  };

  /**
    *@brief  将16进制字符串转换成整型数 //  NOLINT
    *@param  hex_str 16进制字符串 //  NOLINT
    *@return 成功返回对应数值，失败返回0  //  NOLINT
    */
  int ConvertHextStringToInt(const std::string &hex_str);
  /**
    *@brief  将Escape值转换成有意义的字符 //  NOLINT
    *@param  escape_str Escape字符串 //  NOLINT
    *@return 成功返回对应数值，失败返回'\0'  //  NOLINT
    */
  char ConvertEscapeToChar(const std::string &escape_str);
  /**
    *@brief  input_str里是否有匹配的condition //  NOLINT
    *@param  input_str 16进制字符串 //  NOLINT
    *@param  condition    条件字符串
    *@return 成功返回true，失败返回false  //  NOLINT
    */
  bool Match(const std::string &input_str, const std::string &condition);
  /**
    *@brief  从Src_Str中找到第一个匹配的字符串 //  NOLINT
    *@param  src_str 要查找的字符串 //  NOLINT
    *@param  reg    正则表达式规则
    *@return 成功返回字符串，失败返回""  //  NOLINT
    */
  std::string GetFirstMatchedString(const std::string &src_str, boost::xpressive::sregex &reg);
  /**
    *@brief  从输入字符串中获得16进制字符串 //  NOLINT
    *@param  input_str 含16进制字符串的输入字符 //  NOLINT
    *@return 成功返回16进制字符串，失败返回“”  //  NOLINT
    */
  std::string GetHexString(const std::string &input_str);
  /**
    *@brief  从输入字符串中获得二进制字符串 //  NOLINT
    *@param  input_str 含二进制字符串的输入字符 //  NOLINT
    *@return 成功返回二进制字符串，失败返回“”  //  NOLINT
    */
  std::string GetStringBin(const std::string &input_str);
  /**
    *@brief  将输入字符串根据delimer分割成多个字符串 //  NOLINT
    *@param  input_str 待分割的字符串 //  NOLINT
    *@param  delimer   分割符 //  NOLINT
    *@param  str_list  分割的结果 //  NOLINT
    *@return 成功返回0，失败返回非0值  //  NOLINT
    */
  int SplitString(const std::string &input_str, const std::string &delimer, std::list<std::string> &str_list);
  /**
    *@brief  input_str里是否有匹配的condition //  NOLINT
    *@param  input_str 16进制字符串 //  NOLINT
    *@param  condition    条件字符串
    *@return 成功返回true，失败返回false  //  NOLINT
    */
  bool Search(const std::string &input_str, const std::string &condition);
  /**
    *@brief  一些操作符在传输保存时，无法直接存取，需要进行转化，函数GetOperator  //  NOLINT
    *    可实现如下操作符的转换：                                                 //  NOLINT
    *  GT:>  LT:< GE:>=  LE:<= EQ:== NE:!=                                        //  NOLINT
    *  其他操作符暂不变                                                           //  NOLINT
    *@param  oper 待转换的操作符 //  NOLINT
    *@return 成功返回操作符，失败返回""  //  NOLINT
    */
  std::string  GetOperator(const std::string &oper);
  /**
    *@brief  first_value & second_value //  NOLINT
    *@param  first_value  第一个操作数 //  NOLINT
    *@param  second_value  第二个操作数 //  NOLINT
    *@return 成功返回结果，失败返回0  //  NOLINT
    */
  template<typename T>
  bool And(const T &first_value, const T &second_value) {
    return first_value && second_value;
  }
  /**
    *@brief  first_value | second_value //  NOLINT
    *@param  first_value  第一个操作数 //  NOLINT
    *@param  second_value  第二个操作数 //  NOLINT
    *@return 成功返回结果，失败返回0  //  NOLINT
    */
  template<typename T>
  bool Or(const T &first_value, const T &second_value) {
    return first_value || second_value;
  }
  /**
    *@brief  ~input_value //  NOLINT
    *@param  input_value  操作数 //  NOLINT
    *@return 成功返回结果，失败返回0  //  NOLINT
    */
  template<typename T>
  bool Not(const T &input_value) {
    return !input_value;
  }

  template<typename T>
  bool Greater(const T &first_value, const T &second_value) {
    return (first_value > second_value);
  }

  template<typename T>
  bool GreaterAndEqual(const T &first_value, const T &second_value) {
    return (first_value >= second_value);
  }

  template<typename T>
  bool Less(const T &first_value, const T &second_value) {
    return (first_value < second_value);
  }

  template<typename T>
  bool LessAndEqual(const T &first_value, const T &second_value) {
    return (first_value <= second_value);
  }

  template<typename T>
  bool NotEqual(const T &first_value, const T &second_value) {
    return (first_value != second_value);
  }

  template<typename T>
  bool Equal(const T &first_value, const T &second_value) {
    return (first_value == second_value);
  }

  /**
    *@brief  将file_name中的内容都读到内存中,且在内存中的编码为GBK2312类型 //  NOLINT
    *@param  file_name  要读取的文件名 //  NOLINT
    *@return 成功返回结果，失败返回""  //  NOLINT
    */
  std::string ReadAllDataFromFile(const std::string &file_name);
  /**
    *@brief  将data数据从encoding编码转化成utf编码 //  NOLINT
    *@param  data  要转化的数据 //  NOLINT
    *@param  encoding  源编码 //  NOLINT
    *@return 成功返回结果，失败返回""  //  NOLINT
    */
  std::string ConvertDataToUtf(const std::string &data, const std::string &encode);
  /**
    *@brief  将data数据从Ascii编码转化成utf-8编码 //  NOLINT
    *@param  data  要转化的数据 //  NOLINT
    *@return 成功返回结果，失败返回""  //  NOLINT
    */
  std::string AsciiToUtf8(const std::string &data);
  /**
    *@brief  将data数据从Utf-8编码转化成Ascii编码 //  NOLINT
    *@param  data  要转化的数据 //  NOLINT
    *@return 成功返回结果，失败返回""  //  NOLINT
    */
  std::string Utf8ToAscii(const std::string &data);
  /**
    *@brief  将data数据从Utf-8编码转化成Unicode编码 //  NOLINT
    *@param  data  要转化的数据 //  NOLINT
    *@return 成功返回结果，失败返回""  //  NOLINT
    */
  std::wstring Utf8ToUnicode(const std::string& data);
  /**
    *@brief  将data数据从Utf-8编码转化成GBK编码 //  NOLINT
    *@param  data  要转化的数据 //  NOLINT
    *@return 成功返回结果，失败返回""  //  NOLINT
    */
  std::string Utf8ToGbk(const std::string& data);
  /**
    *@brief  将data数据从GBK编码转化成Utf-8编码 //  NOLINT
    *@param  data  要转化的数据 //  NOLINT
    *@return 成功返回结果，失败返回""  //  NOLINT
    */
  std::string GbkToUtf8(const std::string& data);
  /**
    *@brief  将data数据从Unicode编码转化成Utf-8编码 //  NOLINT
    *@param  data  要转化的数据 //  NOLINT
    *@return 成功返回结果，失败返回""  //  NOLINT
    */
  std::string UnicodeToUtf8(const std::wstring& data);
  /**
    *@brief  将data数据从Unicode编码转化成Ascii编码 //  NOLINT
    *@param  data  要转化的数据 //  NOLINT
    *@return 成功返回结果，失败返回""  //  NOLINT
    */
  std::string UnicodeToAcsii(const std::wstring& data);
  /**
    *@brief  将data数据从Ascii编码转化成Unicode编码 //  NOLINT
    *@param  data  要转化的数据 //  NOLINT
    *@return 成功返回结果，失败返回""  //  NOLINT
    */
  std::wstring AcsiiToUnicode(const std::string& data);
  /**
    *@brief  将str转化为小写，以返回值的方式给出结果 //  NOLINT
    *@param  str  要转化的数据 //  NOLINT
    *@return 返回小写数据  //  NOLINT
    */
  std::string ToLower(const std::string &str);
  /**
    *@brief  将str转化为大写，以返回值的方式给出结果 //  NOLINT
    *@param  str  要转化的数据 //  NOLINT
    *@return 返回大写数据  //  NOLINT
    */
  std::string ToUpper(const std::string &str);
  /**
    *@brief  将data数据从utf编码转化成encoding编码 //  NOLINT
    *@param  data  要转化的数据 //  NOLINT
    *@param  encoding  目标编码 //  NOLINT
    *@return 成功返回结果，失败返回""  //  NOLINT
    */
  std::string ConvertDataFromUtf(const std::string &data, const std::string &encode);
  /**
    *@brief  将str转变为T类型 //  NOLINT
    *@param  str  要转化的数据 //  NOLINT
    *@param  default_value  默认值 //  NOLINT
    *@return 成功返回结果，失败返回默认值  //  NOLINT
    */
  template<typename T>
  T ConvertFromString(const std::string &str, T &default_value) {
    if (str.empty()) {
      return default_value;
    }

    std::stringstream ss;
    ss << str;
    T value;
    ss >> value;

    return value;
  }
  /**
    *@brief  将value转变为string类型 //  NOLINT
    *@param  str  要转化的数据 //  NOLINT
    *@return 成功返回结果，失败返回""  //  NOLINT
    */
  template<typename T>
  std::string ConvertToString(const T &value) {
    std::stringstream ss;
    ss << value;

    return ss.str();
  }
  /**
    *@brief  记录日志到当前目录的coreframework.log文件中 //  NOLINT
    *@return 成功返回文件输入流，失败返回默认值  //  NOLINT
    */
  std::ofstream & YsosLog(/*const std::string &log_file*/);
  /**
    *@brief  获得当前时间 //  NOLINT
    *@return 成功返回当前时间，失败返回空  //  NOLINT
    */
  std::string GetCurTime(void);
  /**
    *@brief  获得指定Boost线程的ID //  NOLINT
    *@param thread_ptr 待获取的线程指针  //  NOLINT
    *@return 成功返回线程ID，失败返回0  //  NOLINT
    */
  unsigned int GetThreadID(boost::thread *thread_ptr);
  /**
    *@brief  根据模块名，获得模块所在的DLL的完整文件名 //  NOLINT
    *@param module_name 要获取的模块名   //  NOLINT
    *@return 成功返回当前时间，失败返回空  //  NOLINT
    */
  std::string GetPluginName(const std::string &module_name);
  /**
    *@brief  获取指定的Logger，默认的Root Logger //  NOLINT
    *@param logger_name 要获取的logger名称   //  NOLINT
    *@return 成功返回logger，失败返回NULL  //  NOLINT
    */
  log4cplus::Logger GetLogger(const std::string &logger_name = "");
  /**
    *@brief  将输入的二进制数据转化成Base64字符串 //  NOLINT
    *@param data 待转化的字符串   //  NOLINT
    *@param data_length 要转换的数据长度   //  NOLINT
    *@return 成功返回字符串，失败返回空  //  NOLINT
    */
  std::string Base64Encode(const unsigned char* data,int data_length);
  /**
    *@brief  将输入的字符串还原成二进制数据 //  NOLINT
    *@param data 待转化的字符串   //  NOLINT
    *@param data_length 要转换的数据长度   //  NOLINT
    *@param binary_length 转换后的数据长度   //  NOLINT
    *@return 成功返回字符串，失败返回空  //  NOLINT
    */
  std::string Base64Decode(const char* data,int data_length,int& binary_length);
  /**
  *@brief  初始化全局log4plus配置文件 //  NOLINT
  *@param log_properties_file_path log配置文件路径   //  NOLINT
  *@return 成功返回true，失败返回false  //  NOLINT
  */
  bool InitLogger(const std::string &log_properties_file_path = "");

  //////////////////////////////////////////////////////////////////////////
  ///@brief 安全拷贝字符串,从给定目标地址处开始拷贝
  ///
  ///如果要拷贝的字符串超过限定的长度，则只拷贝在（限定长度－1）个字符。尾字符为字符串'\0'
  ///@param[in] dest 目标字符数组
  ///@param[in] max_copy_size 限定拷贝的字符数量
  ///@param[in] source 需要拷贝的字符串
  void SafeStringCopy(char* dest, const unsigned int max_copy_size, const char* source);

  ///@brief 安全附加拷贝字符串，从给定目标字符串结尾处开始拷贝
  ///
  ///限定拷贝的字符串附加到目标串结尾后不能超过目标数组的大小，如果超过了，则只拷贝一部分字符
  ///@param[in] dest 目标字符数组
  ///@param[in] dest_size目标数组总的大小
  ///@param[in] source 需要拷贝的字符串
  //////////////////////////////////////////////////////////////////////////
  void SafeStringCat(char* dest, const unsigned int dest_size, const char* source);

  //////////////////////////////////////////////////////////////////////////
  char* StrReplace(char* source, char* sub, char* rep);

  std::string& ReplaceAll(std::string& str, const std::string& old_value, const std::string& new_value);
  std::string& ReplaceAllDistinct(std::string& str, const std::string& old_value, const std::string& new_value);
  
  std::vector<std::string> SplitString2(std::string str, std::string pattern);

  std::string Trim(const char* src, char tch);
  std::string Trim2(const char* src);

  //template <typename T1>
  int SplitToInt(const char * src, std::vector<int> & dest, char ch /*= ','*/);
  //template <typename TT>
  int SplitString2Vec(const char * src, std::vector<std::string> & dest, char ch /*= ','*/);

  bool SBase64Encode(const std::string& input, std::string* output);
  bool SBase64Decode(const std::string& input, std::string* output);
   
  EncodingType GetEncoding(unsigned char* data, int len);
  EncodingType GetEncoding(std::string data);

  DECLARE_SINGLETON_VARIABLE(Utility);
};
}
#define GetUtility  ysos::Utility::Instance
// #define GetUtility  ysos::Singleton<ysos::Utility>::Instance

//#define  YSOS_LOG   GetUtility()->YsosLog()<<"[["<<__FILE__<<"|"<<__FUNCTION__<<"|"<<__LINE__<<"|"<<GetUtility()->GetCurTime()<<"]]------"

// #define  YSOS_LOG_INFO_DEFAULT(msg)    LOG4CPLUS_INFO(GetUtility()->GetLogger(), msg)
// #define  YSOS_LOG_WARN_DEFAULT(msg)    LOG4CPLUS_WARN(GetUtility()->GetLogger(), msg)
// #define  YSOS_LOG_DEBUG_DEFAULT(msg)   LOG4CPLUS_DEBUG(GetUtility()->GetLogger(), msg)
// #define  YSOS_LOG_ERROR_DEFAULT(msg)   LOG4CPLUS_ERROR(GetUtility()->GetLogger(), msg)
// #define  YSOS_LOG_FATAL_DEFAULT(msg)   LOG4CPLUS_FATAL(GetUtility()->GetLogger(), msg)
// 
// #define  YSOS_LOG_INFO(msg)   LOG4CPLUS_INFO(logger_, msg)
// #define  YSOS_LOG_WARN(msg)   LOG4CPLUS_WARN(logger_, msg)
// #define  YSOS_LOG_DEBUG(msg)   LOG4CPLUS_DEBUG(logger_, msg)
// #define  YSOS_LOG_ERROR(msg)   LOG4CPLUS_ERROR(logger_, msg)
// #define  YSOS_LOG_FATAL(msg)   LOG4CPLUS_FATAL(logger_, msg)
// 
// #define  YSOS_LOG_INFO_CUSTOM(logger, msg)   LOG4CPLUS_INFO(logger, msg)
// #define  YSOS_LOG_WARN_CUSTOM(logger, msg)   LOG4CPLUS_WARN(logger, msg)
// #define  YSOS_LOG_DEBUG_CUSTOM(logger, msg)   LOG4CPLUS_DEBUG(logger, msg)
// #define  YSOS_LOG_ERROR_CUSTOM(logger, msg)   LOG4CPLUS_ERROR(logger, msg)
// #define  YSOS_LOG_FATAL_CUSTOM(logger, msg)   LOG4CPLUS_FATAL(logger, msg)

#endif /* INCLUDE_UTILITY_ */
