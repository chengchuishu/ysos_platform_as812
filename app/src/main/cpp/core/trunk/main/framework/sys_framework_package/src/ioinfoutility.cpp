/**
 *@file IoinfoUtility.cpp
 *@brief Definition of IoinfoUtility
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// ysos private headers //  NOLINT
#include "../../../protect/include/sys_framework_package/ioinfoutility.h"
/// c headers //  NOLINT
#include <cstring>
#include <cstdio>
#include <cstdlib>
/// stl headers //  NOLINT
#include <sstream>
#include <fstream>
/// boost headers //  NOLINT
#include <boost/algorithm/string.hpp>
#include <boost/xpressive/xpressive.hpp>
/// Ysos Headers
#include "../../../public/include/sys_interface_package/common.h"
#include "../../../public/include/core_help_package/utility.h"

namespace ysos {
IoinfoUtilityPtr  IoinfoUtility::s_utility_ = NULL;
boost::shared_ptr<LightLock> IoinfoUtility::s_utility_lock_ = boost::shared_ptr<LightLock>(new LightLock());

IoinfoUtility::~IoinfoUtility() {
}

IoinfoUtility::IoinfoUtility(const std::string &strClassName): BaseInterfaceImpl(strClassName) {
  input_str_ = "$(ysos)";
}

const IoinfoUtilityPtr IoinfoUtility::Instance(void) {
  if (NULL == s_utility_) {
    AutoLockOper lock(s_utility_lock_.get());
    if(NULL == s_utility_) {
      s_utility_ = IoinfoUtilityPtr(new IoinfoUtility());
    }
  }

  return s_utility_;
}

std::string IoinfoUtility::ParseIoInfo(IoInfoPtr &ioinfo) {
  std::list<std::string> vec;
  parseOperator(ioinfo->operator_type, vec);
  std::string str = parseData(ioinfo->data, vec);

  return str;
}

bool IoinfoUtility::IsNameMatch(const std::string &module_name, const std::string &ioinfo_name) {
  std::string condition = std::string("^") + module_name + "(_\\d*)?$";
  boost::xpressive::smatch result;
  boost::xpressive::sregex name_reg = boost::xpressive::sregex::compile(condition);
  
  return boost::xpressive::regex_match(ioinfo_name, result, name_reg);
}

std::string IoinfoUtility::parseData(const std::string &data, std::list<std::string> &vec) {
  if (vec.size() == 0) {
    return parseRange(data);
  }

  std::vector<std::string> digit_vec;
  parseDigit(data, digit_vec);

  std::string str;
  //std::string input_str = "$(ysos)";
  std::string::size_type j=0;
  for (std::list<std::string>::iterator it=vec.begin(); it!=vec.end(); ++it) {
    if (*it == "!" && j<digit_vec.size()) {
      str = (*it) + digit_vec[j++];
    } else if (*it == "(" || *it == ")" || *it == "||" || *it == "&&") {
      str += (*it);
    } else if (j<digit_vec.size()) {
      str += input_str_ + (*it) + digit_vec[j++];
    } else {
      std::cout <<"parse error" << std::endl;
    }
  }

  return str;
}

std::string IoinfoUtility::parseRange(const std::string &data) {
  if (data.empty()) {
    return "";
  }

  std::string condition_open_open =  "\\([+-]?\\d+,\\s*[+-]?\\d+\\)|\\([+-]?\\d+,\\s*[+-]?\\d+\\.\\d+\\)|\\([+-]?\\d+\\.\\d+,\\s*[+-]?\\d+\\.\\d+\\)|\\([+-]?\\d+,\\s*[+-]?\\d+\\.\\d+\\)";
  std::string condition_open_close = "\\([+-]?\\d+,\\s*[+-]?\\d+\\]|\\([+-]?\\d+,\\s*[+-]?\\d+\\.\\d+\\]|\\([+-]?\\d+\\.\\d+,\\s*[+-]?\\d+\\.\\d+\\]|\\([+-]?\\d+,\\s*[+-]?\\d+\\.\\d+\\]";;
  std::string condition_close_close = "\\[[+-]?\\d+,\\s*[+-]?\\d+\\]|\\[[+-]?\\d+,\\s*[+-]?\\d+\\.\\d+\\]|\\[[+-]?\\d+\\.\\d+,\\s*[+-]?\\d+\\.\\d+\\]|\\[[+-]?\\d+,\\s*[+-]?\\d+\\.\\d+\\]";
  std::string condition_close_open = "\\[[+-]?\\d+,\\s*[+-]?\\d+\\)|\\[[+-]?\\d+,\\s*[+-]?\\d+\\.\\d+\\)|\\[[+-]?\\d+\\.\\d+,\\s*[+-]?\\d+\\.\\d+\\)|\\[[+-]?\\d+,\\s*[+-]?\\d+\\.\\d+\\)";
  boost::xpressive::smatch result;
  boost::xpressive::sregex reg_open_open = boost::xpressive::sregex::compile(condition_open_open);
  boost::xpressive::sregex reg_open_close = boost::xpressive::sregex::compile(condition_open_close);
  boost::xpressive::sregex reg_close_close = boost::xpressive::sregex::compile(condition_close_close);
  boost::xpressive::sregex reg_close_open = boost::xpressive::sregex::compile(condition_close_open);

  std::string oper1, oper2;
  if (boost::xpressive::regex_search(data, result, reg_open_open)) {
    oper1 = ">";
    oper2 = "<";
  } else if (boost::xpressive::regex_search(data, result, reg_open_close)) {
    oper1 = ">";
    oper2 = "<=";
  } else if (boost::xpressive::regex_search(data, result, reg_close_close)) {
    oper1 = ">=";
    oper2 = "<=";
  } else if (boost::xpressive::regex_search(data, result, reg_close_open)) {
    oper1 = ">=";
    oper2 = "<";
  } else {
    std::cout << " wrong expression: " << data << std::endl;
    return "";
  }

  for (boost::xpressive::smatch::size_type i=0; i<result.size(); ++i) {
    std::string str = result[i];
    std::cout << str << std::endl;
  }

  std::string str;
  //std::string input_str = "$(ysos)";
  for (boost::xpressive::smatch::size_type i=0; i<result.size(); ++i) {
    std::string result_str = result[i];
    std::vector<std::string> vec;
    parseDigit(result_str, vec);
    if (vec.size() != 2) {
      std::cout <<"not equal 2 parameters" << vec.size() << std::endl;
      continue;
    }

    if (!str.empty()) {
      str += " && ";
    }
    str = "((" + input_str_ + oper1 + vec[0] + ") && (" + input_str_ + oper2 + vec[1] + "))";
  }

  return str;
}

void  IoinfoUtility::parseDigit(const std::string &data, std::vector<std::string> &vec) {
  std::string condition = "\\s*[+-]?\\d+|\\s*[+-]?\\d+\\.\\d+";
  boost::xpressive::smatch result;
  boost::xpressive::sregex reg = boost::xpressive::sregex::compile(condition);

  std::string::const_iterator start=data.begin(), end=data.end();
  while (boost::xpressive::regex_search(start, end, result, reg)) {
    std::string str = result[0];
    vec.push_back(str);
    start = result[0].second;
  }
}

void IoinfoUtility::parseOperator(const std::string &oper, std::list<std::string> &vec) {
  std::string oper_str = GetUtility()->GetOperator(oper);
  if (!oper_str.empty()) {
    vec.push_back(oper_str);
  }
}

bool IoinfoUtility::ParseExpression(const std::string &expression) {
  const std::string condition = "[01]&&[01]|[01]\\|\\|[01]|![01]|\\(1\\)|\\(0\\)";
  std::string result_str = expression;

  std::string::const_iterator start = result_str.begin(), end = result_str.end();
  boost::xpressive::sregex reg = boost::xpressive::sregex::compile(condition);

  boost::xpressive::smatch result;
  int idx = 0;
  while (boost::xpressive::regex_search(start, end, result, reg)) {

    for (boost::xpressive::smatch::size_type i = 0; i < result.size(); i++) {
      int ret = ParseAndOrExpression(result[0]);
      boost::algorithm::replace_first(result_str, result[0], ret==0?"0":"1");
      start = result_str.begin(), end = result_str.end();
    }
  }

  bool ret = result_str=="1";
  return ret;
}

int IoinfoUtility::ParseAndOrExpression(const std::string &expression) {
  if (expression.empty()) {
    return 0;
  }

  if ("0&&1" == expression || "1&&0" == expression || "0||0" == expression || "!1" == expression || "(0)" == expression) {
    return 0;
  }

  if ("1&&1" == expression || "1||0" == expression || "0||1" == expression || "1||1" == expression ||"!0" == expression || "(1)" == expression) {
    return 1;
  }

  YSOS_LOG_DEBUG("unsupport expression: " << expression);

  return 0;
}
}
