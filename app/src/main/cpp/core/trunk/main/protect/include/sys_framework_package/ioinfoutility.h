/**
 *@file IoinfoUtility.h
 *@brief Definition of IoinfoUtility
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef SHP_IOINFO_UTILITY_H  //NOLINT
#define SHP_IOINFO_UTILITY_H  //NOLINT

/// stl Headers
#include <string>
#include <list>
/// boost Headers
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string/trim.hpp>

/// Private Ysos Headers
#include "../../../public/include/os_hal_package/lock.h"
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../public/include/core_help_package/utility.h"

namespace ysos {
struct IoInfo {
  std::string    name;
  std::string    data_type;
  std::string    operator_type;
  std::string    data;
  std::string    event_name;
  std::string    callback_name;
};
typedef  boost::shared_ptr<IoInfo>   IoInfoPtr;

/**
  *@brief  工具类，使用方式：  //  NOLINT
  *        GetIoinfoUtility()->ConvertHextStringToInt() ... //  NOLINT
  */
class IoinfoUtility;
typedef boost::shared_ptr<IoinfoUtility>  IoinfoUtilityPtr;
class YSOS_EXPORT IoinfoUtility: public BaseInterfaceImpl {
// DECLARE_CREATEINSTANCE(Utility);
  DISALLOW_COPY_AND_ASSIGN(IoinfoUtility);
  DECLARE_PROTECT_CONSTRUCTOR(IoinfoUtility);

 public:
  ~IoinfoUtility();

  /**
    *@brief  IoInfo的表达式只有如下三种：1 范围：[a,b] [a,b) (a,b) (a,b]        //  NOLINT
    *                                 2 a>=b a>b a<b a<=b a==b a!=b a||b a&&b   //  NOLINT
    *                                 3 !a                                      //  NOLINT
    *                                                                           //  NOLINT
    *@param  ioinfo 待解析的IoInfo//  NOLINT
    *@return 成功返回表达式，否则返回""  //  NOLINT
    */
  std::string ParseIoInfo(IoInfoPtr &ioinfo);
  /**
    *@brief  判断ioinfo的Name是否与ModuleName匹配   //  NOLINT
    *@param  module_name module的name // NOLINT
    *@param  ioinfo_name ioinfo的name  // NOLINT
    *@return 匹配返回true，否则返回false  //  NOLINT
    */
  bool IsNameMatch(const std::string &module_name, const std::string &ioinfo_name);
  /**
    *@brief  解析出表达式data中的所有数字，并存放在vec中//  NOLINT
    *        //  NOLINT
    *@param  data  含数字的表达式 //  NOLINT
    *@param  vec  存放解析的结果 //  NOLINT
    *@return 无  //  NOLINT
    */
  void parseDigit(const std::string &data, std::vector<std::string> &vec);
  /**
    *@brief  解析出表达式oper中的所有操作符，并存放在vec中//  NOLINT
    *        //  NOLINT
    *@param  data  含操作符的表达式 //  NOLINT
    *@param  vec  存放解析的结果 //  NOLINT
    *@return 无  //  NOLINT
    */
  void parseOperator(const std::string &oper, std::list<std::string> &vec);
  /**
    *@brief  解析[a,b] [a,b) (a,b) (a,b] 四种格式//  NOLINT
    *        //  NOLINT
    *@param  data  范围字符串 //  NOLINT
    *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
    */
  std::string parseRange(const std::string &data);
  /**
    *@brief  将XML里描述的表达式解析出来//  NOLINT
    *        //  NOLINT
    *@param  //  NOLINT
    *@param  //  NOLINT
    *@param  //  NOLINT
    *@param  //  NOLINT
    *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
    */
  std::string parseData(const std::string &data, std::list<std::string> &vec);
  /**
    *@brief  解析((a||b)>=(c<d))，支持+2.54 -58等格式 //  NOLINT
    *        //  NOLINT
    *@param  expression  表达式 //  NOLINT
    *@return 成功返回表达式计算结果，失败返回false  //  NOLINT
    */
  bool ParseExpression(const std::string &expression);
  /**
    *@brief  解析((a||b)&&(c&&d))，只解析||、&&操作符 //  NOLINT
    *        //  NOLINT
    *@param  expression  表达式 //  NOLINT
    *@return 成功返回表达式计算结果，失败返回false  //  NOLINT
    */
  int ParseAndOrExpression(const std::string &expression);

  template<typename T>
  bool CalculateExpression(const std::string &expression, T default_value) {
    const std::string condition = "[+-]?\\d+(\\.\\d+)?\\s*(>|<|>=|<=|==|!=|&&|\\|\\|)\\s*[+-]?\\d+(\\.\\d+)?|\\(\\s*[+-]?\\d+(\\.\\d+)?\\s*\\)";
    std::string result_str = expression;
    boost::replace_all(result_str, " ", "");
    std::string::const_iterator start = result_str.begin(), end = result_str.end();
    boost::xpressive::sregex reg = boost::xpressive::sregex::compile(condition);

    boost::xpressive::smatch result;
    while (boost::xpressive::regex_search(result_str, result, reg)) {
      if (result.size() > 0) {
        std::string ret = RealCalculate(result[0], default_value);
        boost::algorithm::replace_first(result_str, result[0], ret);
      } else {
        break;
      }
    }

    return result_str=="1" ? true : false;
  }

  // 计算 ((2.5>=1)||(3<2))&&(8>5) 表达式
  template<typename T>
  std::string RealCalculate(const std::string &expression, T default_value) {
    T first_value, second_value;

    std::string input_str = expression;
    std::string condition = "\\s*[+-]?\\d+|\\s*[+-]?\\d+\\.\\d+";
    boost::xpressive::smatch result;
    boost::xpressive::sregex reg = boost::xpressive::sregex::compile(condition);
    if (boost::xpressive::regex_search(input_str, result, reg)) {
      std::string str = result[0];
      first_value = GetUtility()->ConvertFromString(expression, default_value);
      boost::algorithm::replace_first(input_str, str, "");
      if (boost::xpressive::regex_search(input_str, result, reg)) {
        std::string str = result[0];
        second_value = GetUtility()->ConvertFromString(str, default_value);
        boost::algorithm::replace_first(input_str, str, "");
        boost::trim(input_str);
        return Caculate(first_value, second_value, input_str) ? "1" : "0";
      } else {
        input_str = expression;
        boost::algorithm::replace_first(input_str, "(", "");
        boost::algorithm::replace_first(input_str, ")", "");
      }
    } else {
      return "0";
    }

    return input_str;
  }

  bool Caculate(std::string first, std::string second, std::string &oper) {
    if (">" == oper) {
      return GetUtility()->Greater(first, second);
    } else if (">=" == oper) {
      return GetUtility()->GreaterAndEqual(first, second);
    } else if ("<" == oper) {
      return GetUtility()->Less(first, second);
    } else if ("<=" == oper) {
      return GetUtility()->LessAndEqual(first, second);
    } else if ("==" == oper) {
      return GetUtility()->Equal(first, second);
    } else if ("!=" == oper) {
      return GetUtility()->NotEqual(first, second);
    } /*else if ("||" == oper) {
      return GetUtility()->Or(first, second);
    } else if ("&&" == oper) {
      return GetUtility()->And(first, second);
    } else if("!" == oper) {
      return GetUtility()->Not(first);
    }*/

    return false;
  }

  template<typename T>
  bool Caculate(T first, T second, std::string &oper) {
    if (">" == oper) {
      return GetUtility()->Greater(first, second);
    } else if (">=" == oper) {
      return GetUtility()->GreaterAndEqual(first, second);
    } else if ("<" == oper) {
      return GetUtility()->Less(first, second);
    } else if ("<=" == oper) {
      return GetUtility()->LessAndEqual(first, second);
    } else if ("==" == oper) {
      return GetUtility()->Equal(first, second);
    } else if ("!=" == oper) {
      return GetUtility()->NotEqual(first, second);
    } else if ("||" == oper) {
      return GetUtility()->Or(first, second);
    } else if ("&&" == oper) {
      return GetUtility()->And(first, second);
    } else if("!" == oper) {
      return GetUtility()->Not(first);
    }

    return false;
  }

  /**
   *@brief 静态函数，对外使用，如：UtilityPtrInstance()->ReadAllDataFromFile(file_name);  // NOLINT
   *@return： 成功返回XmlUtil指针，失败返回NULL  // NOLINT
   */
  static const IoinfoUtilityPtr Instance(void);

 private:
  static IoinfoUtilityPtr   s_utility_;
  static boost::shared_ptr<LightLock> s_utility_lock_;
  std::string               input_str_;
};
}
#define GetIoinfoUtility  ysos::IoinfoUtility::Instance
#endif /* INCLUDE_UTILITY_ */
