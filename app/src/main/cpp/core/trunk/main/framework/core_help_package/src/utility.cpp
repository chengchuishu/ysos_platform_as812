/**
 *@file Utility.cpp
 *@brief Definition of Utility
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// ysos private headers //  NOLINT
#include "../../../public/include/core_help_package/utility.h"
/// c headers //  NOLINT
#include <cstring>
#include <cstdio>
#include <cstdlib>
/// stl headers //  NOLINT
#include <sstream>
#include <fstream>
/// 3rdparty Headers
#include <log4cplus/configurator.h>

/// boost headers //  NOLINT
#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>
#include <boost/locale/conversion.hpp>
#include <boost/locale/encoding.hpp>
#include <boost/locale/conversion.hpp>
#include <boost/locale/encoding.hpp>
#include <boost/thread.hpp>
#include <boost/thread/recursive_mutex.hpp>
/// Ysos Headers
#include "../../../public/include/sys_interface_package/common.h"

#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#include <codecvt>
#endif

namespace ysos {

    namespace ysos_utility {
        static log4cplus::ConfigureAndWatchThread* g_configure_and_watch_thread_ptr_ = NULL;
    }

    static std::ofstream* s_ofstream_log_ = NULL;
    static boost::recursive_mutex mutex_;

    DEFINE_SINGLETON(Utility);
    Utility::~Utility() {
        delete s_ofstream_log_;
        s_ofstream_log_ = NULL;

        if (NULL != ysos_utility::g_configure_and_watch_thread_ptr_) {
            delete ysos_utility::g_configure_and_watch_thread_ptr_;
            ysos_utility::g_configure_and_watch_thread_ptr_ = NULL;
        }
    }

    Utility::Utility(const std::string& strClassName) : BaseInterfaceImpl(strClassName) {
    }

    int Utility::ConvertHextStringToInt(const std::string& hex_str) {
        return (int)std::strtol(hex_str.c_str(), NULL, 0);
    }

    char Utility::ConvertEscapeToChar(const std::string& escape_str) {
        return YSOS_ERROR_SUCCESS;
    }

    bool Utility::Match(const std::string& input_str, const std::string& condition) {
        boost::xpressive::sregex reg = boost::xpressive::sregex::compile(condition);

        return boost::xpressive::regex_match(input_str, reg);
    }

    std::string Utility::GetFirstMatchedString(const std::string& src_str, boost::xpressive::sregex& reg) {
        boost::xpressive::smatch result;
        if (boost::xpressive::regex_search(src_str, result, reg)) {
            assert(result.size() > 0);

            return result[0];
        }

        return "";
    }

    std::string Utility::GetHexString(const std::string& input_str) {
        std::string condition = "0[xX]\\d+";
        boost::xpressive::cregex reg = boost::xpressive::cregex::compile(condition);

        boost::xpressive::cmatch result;
        boost::xpressive::regex_search(input_str.c_str(), result, reg);

        if (result.size() > 0) {
            return result[0];
        }

        return "";
    }

    std::string Utility::GetStringBin(const std::string& input_str) {
        return "";
    }

    int Utility::SplitString(const std::string& input_str, const std::string& delimer, std::list<std::string>& str_list) {

        std::vector<std::string> cmd_list;
        //  boost::is_any_of这里相当于分割规则了//NOLINT
        boost::split(str_list, input_str, boost::algorithm::is_any_of((delimer.c_str())));

        return YSOS_ERROR_SUCCESS;
    }

    bool Utility::Search(const std::string& input_str,
        const std::string& condition) {
        boost::xpressive::sregex reg = boost::xpressive::sregex::compile(condition);

        return boost::xpressive::regex_search(input_str, reg);
    }

    std::string  Utility::GetOperator(const std::string& oper) {
        std::string result;
        std::string logic = boost::to_upper_copy(oper);
        if ("AND" == logic) {
            result = "&&";
        }
        else if ("OR" == logic) {
            result = "||";
        }
        else if ("GT" == logic) {
            result = ">";
        }
        else if ("LT" == logic) {
            result = "<";
        }
        else if ("GE" == logic) {
            result = ">=";
        }
        else if ("LE" == logic) {
            result = ">=";
        }
        else if ("EQ" == logic) {
            result = "==";
        }
        else if ("NE" == logic) {
            result = "!=";
        }
        else {
            result = logic;
        }

        return result;
    }

    static Utility::EncodingType GetStringEncoding(const uint8_t* character_ptr) {
        if (NULL == character_ptr) {
            return Utility::EncodingTypeEnd;
        }

        if (*character_ptr == 0xff) {
            if (*(++character_ptr) == 0xfe) {
                return Utility::UTF16;
            }
        } /*else if (*character_ptr == 0xfe) {
          if (*(++character_ptr) == 0xff) {
          return UNICODE_BIGENDIAN;
          }
          }*/ else if (*character_ptr == 0xef) {
            if (*(++character_ptr) == 0xbb) {
                if (*(++character_ptr) == 0xbf) {
                    return Utility::UTF8;
                }
            }
        }

        return Utility::ASCII;
    }

    static Utility::EncodingType GetStringEncoding(std::ifstream& file_stream) {
        const int encode_tpe_len = 3;
        uint8_t encode_type[encode_tpe_len] = { 0 };

        file_stream.seekg(0, file_stream.beg);
        file_stream.read((char*)encode_type, encode_tpe_len);

        return GetStringEncoding(encode_type);
    }

    std::string Utility::ReadAllDataFromFile(const std::string& file_name) {
        std::ifstream in_ifile;
        in_ifile.open(file_name.c_str(), std::ios_base::in | std::ios_base::binary);
        if (!in_ifile.good()) {
            return "";
        }

        Utility::EncodingType type = GetStringEncoding(in_ifile);
        if (Utility::EncodingTypeEnd == type) {
            return "";
        }

        int start_pos = 0, end_len = 1;
        if (Utility::UTF16 == type) {
            start_pos = 2;
            end_len = 2;
        }
        else if (Utility::UTF8 == type) {
            start_pos = 3;
        }

        in_ifile.seekg(0, in_ifile.end);
        int length = static_cast<int>(in_ifile.tellg()) - start_pos;
        in_ifile.seekg(start_pos, in_ifile.beg);

        char* buffer = new char[length + end_len];
        std::memset(buffer, 0, length + end_len);
        in_ifile.read(buffer, length);
        if (in_ifile) {
            YSOS_LOG_DEBUG("all characters read successfully.");  // need update for linux
        }
        else {
            YSOS_LOG_ERROR("error: only " << in_ifile.gcount() << " could be read");  // need update for linux
        }
        in_ifile.close();

        std::string result_str;

        if (Utility::UTF8 == type) { ///< utf-8 //  NOLINT
            result_str = Utf8ToAscii(buffer);
        }
        else if (Utility::UTF16 == type) { ///<  utf-16 //  NOLINT
            result_str = UnicodeToAcsii((wchar_t*)buffer);
        }
        else { ///<  gbk  //  NOLINT
            result_str = buffer;
        }
        delete[]buffer;

        return result_str;
    }

    std::string Utility::ConvertDataToUtf(
        const std::string& data, const std::string& encode) {
        return boost::locale::conv::to_utf<char>(data, encode);
    }

    std::string Utility::ConvertDataFromUtf(
        const std::string& data, const std::string& encode) {
        return boost::locale::conv::from_utf(data, encode);
    }

    std::string Utility::AsciiToUtf8(const std::string& data) {
#ifdef WIN32
        //先把 ascii 转为 unicode
        std::wstring wstr = AcsiiToUnicode(data);
        //最后把 unicode 转为 utf8
        return UnicodeToUtf8(wstr);
#else
        std::string const& to_encoding("UTF-8");
        std::string const& from_encoding("GBK");
        boost::locale::conv::method_type how = boost::locale::conv::default_method;

        return boost::locale::conv::between(data.c_str(), to_encoding, from_encoding, how);
#endif
    }

    std::string Utility::Utf8ToAscii(const std::string& data) {
#ifdef WIN32
        //先把 utf8 转为 unicode
        std::wstring wstr = Utf8ToUnicode(data);
        //最后把 unicode 转为 ascii
        return UnicodeToAcsii(wstr);
#else
        std::string const& to_encoding("GBK");
        std::string const& from_encoding("UTF-8");
        boost::locale::conv::method_type how = boost::locale::conv::default_method;

        return boost::locale::conv::between(data.c_str(), to_encoding, from_encoding, how);
#endif
    }

    std::wstring Utility::Utf8ToUnicode(const std::string& data) {
#ifdef WIN32
        int widesize = ::MultiByteToWideChar(CP_UTF8, 0, data.c_str(), data.size(), NULL, 0);
        if (widesize == ERROR_NO_UNICODE_TRANSLATION) {
            return std::wstring();
        }
        if (widesize == 0) {
            return std::wstring();
        }
        std::vector<wchar_t> resultstring(widesize + 1);
        int convresult = ::MultiByteToWideChar(CP_UTF8, 0, data.c_str(), data.size(), &resultstring[0], widesize);
        if (convresult != widesize) {
            return std::wstring();
        }
        return std::wstring(&resultstring[0]);
#else
        using convert_typeX = std::codecvt_utf8<wchar_t>;
        std::wstring_convert<convert_typeX, wchar_t> converterX;

        return converterX.from_bytes(data);
        //return "";
#endif
    }

    std::string Utility::Utf8ToGbk(const std::string& data) {
        std::string const& to_encoding("UTF-8");
        std::string const& from_encoding("GBK");
        boost::locale::conv::method_type how = boost::locale::conv::default_method;
        return boost::locale::conv::between(data.c_str(), to_encoding, from_encoding, how);
    }

    std::string Utility::GbkToUtf8(const std::string& data) {
        std::string const& to_encoding("GBK");
        std::string const& from_encoding("UTF-8");
        boost::locale::conv::method_type how = boost::locale::conv::default_method;
        return boost::locale::conv::between(data.c_str(), to_encoding, from_encoding, how);
    }

    std::string Utility::UnicodeToUtf8(const std::wstring& data) {
#ifdef WIN32
        int utf8size = ::WideCharToMultiByte(CP_UTF8, 0, data.c_str(), data.size(), NULL, 0, NULL, NULL);
        if (utf8size == 0) {
            return std::string("");
        }
        std::vector<char> resultstring(utf8size + 1 + 3);
        int convresult = ::WideCharToMultiByte(CP_UTF8, 0, data.c_str(), data.size(), &resultstring[3], utf8size, NULL, NULL);
        if (convresult != utf8size) {
            return std::string("");
        }
        resultstring[0] = (char)0xEF;
        resultstring[1] = (char)0xBB;
        resultstring[2] = (char)0xBF;
        return std::string(&resultstring[0]);
#else
        using convert_typeX = std::codecvt_utf8<wchar_t>;
        std::wstring_convert<convert_typeX, wchar_t> converterX;

        return converterX.to_bytes(data);
        //return "";
#endif
    }

    std::string Utility::UnicodeToAcsii(const std::wstring& data) {
#ifdef WIN32
        int asciisize = ::WideCharToMultiByte(CP_OEMCP, 0, data.c_str(), data.size(), NULL, 0, NULL, NULL);
        if (asciisize == ERROR_NO_UNICODE_TRANSLATION) {
            return std::string("");
        }
        if (asciisize == 0) {
            return std::string("");
        }
        std::vector<char> resultstring(asciisize + 1);
        int convresult = ::WideCharToMultiByte(CP_OEMCP, 0, data.c_str(), data.size(), &resultstring[0], asciisize, NULL, NULL);
        if (convresult != asciisize) {
            return std::string("");
        }
        resultstring[convresult] = '\0';
        return std::string(&resultstring[0]);
#else
        std::string ret;
        std::mbstate_t state = {};
        const wchar_t* src = data.data();
        size_t len = std::wcsrtombs(nullptr, &src, 0, &state);
        if (static_cast<size_t>(-1) != len) {
            std::unique_ptr< char[] > buff(new char[len + 1]);
            len = std::wcsrtombs(buff.get(), &src, len, &state);
            if (static_cast<size_t>(-1) != len) {
                ret.assign(buff.get(), len);
            }
        }
        return ret;
        //return "";
#endif
    }

    std::wstring Utility::AcsiiToUnicode(const std::string& data) {
#ifdef WIN32
        int widesize = MultiByteToWideChar(CP_ACP, 0, data.c_str(), data.size(), NULL, 0);
        if (widesize == ERROR_NO_UNICODE_TRANSLATION) {
            return std::wstring();
        }
        if (widesize == 0) {
            return std::wstring();
        }
        std::vector<wchar_t> resultstring(widesize + 1);
        int convresult = MultiByteToWideChar(CP_ACP, 0, data.c_str(), data.size(), &resultstring[0], widesize);
        if (convresult != widesize) {
            return std::wstring();
        }
        return std::wstring(&resultstring[0]);
#else
        std::wstring ret;
        std::mbstate_t state = {};
        const char* src = data.data();
        size_t len = std::mbsrtowcs(nullptr, &src, 0, &state);
        if (static_cast<size_t>(-1) != len) {
            std::unique_ptr< wchar_t[] > buff(new wchar_t[len + 1]);
            len = std::mbsrtowcs(buff.get(), &src, len, &state);
            if (static_cast<size_t>(-1) != len) {
                ret.assign(buff.get(), len);
            }
        }
        return ret;
        //return "";
#endif
    }

    std::string Utility::ToLower(const std::string& str) {
        return boost::algorithm::to_lower_copy(str);
    }

    std::string Utility::ToUpper(const std::string& str) {
        return boost::algorithm::to_upper_copy(str);
    }

    std::ofstream& Utility::YsosLog(/*const std::string &log_file*/) {
        if (NULL == s_ofstream_log_) {
            mutex_.lock();
            if (NULL == s_ofstream_log_) {
                s_ofstream_log_ = new std::ofstream();
                s_ofstream_log_->open("coreframework.log", std::ios_base::out | std::ios_base::app | std::ios_base::binary);
            }
            mutex_.unlock();
        }

        return (*s_ofstream_log_);
    }

    std::string Utility::GetCurTime(void) {
        struct timeval tv;
        long tv_usec = 0;
#ifdef WIN32
        SYSTEMTIME wtm;
        GetLocalTime(&wtm);
        tv_usec = wtm.wMilliseconds;
        tv.tv_usec = 0;
#else
        struct timezone tz;
        gettimeofday(&tv, &tz);
#endif

        time_t timep;
        struct tm* p;
        time(&timep);
        p = gmtime(&timep);

        std::stringstream oss;
        oss << (1900 + p->tm_year) << "-"
            << std::setw(2) << std::setfill('0') << (1 + p->tm_mon) << "-"
            << std::setw(2) << std::setfill('0') << p->tm_mday << " "
            << std::setw(2) << std::setfill('0') << (p->tm_hour + 8) << ":"
            << std::setw(2) << std::setfill('0') << p->tm_min << ":"
            << std::setw(2) << std::setfill('0') << p->tm_sec << ":"
            << std::setw(3) << std::setfill('0') << tv_usec
            << "|" << boost::this_thread::get_id();

        return oss.str();
    }

    unsigned int Utility::GetThreadID(boost::thread* thread_ptr) {
        if (NULL == thread_ptr) {
            return 0;
        }

        boost::thread::id thread_id = thread_ptr->get_id();
        std::stringstream ss;
        ss << thread_id;

        unsigned int thread_id_value = 0;
        ss >> thread_id_value;

        return thread_id_value;
    }

    std::string Utility::GetPluginName(const std::string& module_name) {
        if (module_name.empty()) {
            return "";
        }

        std::stringstream ss;
#ifdef WIN32
        ss << "lib" << module_name << ".dll";
#else
        ss << "lib" << module_name << ".so";
#endif

        return ss.str();
    }

    log4cplus::Logger Utility::GetLogger(const std::string& logger_name) {
        if (logger_name.empty()) {
            // return log4cplus::Logger::getRoot();
            return log4cplus::Logger::getInstance("ysos");
        }

        return log4cplus::Logger::getInstance(logger_name.c_str());
    }

    std::string Utility::Base64Encode(const unsigned char* data, int data_length) {
        //编码表
        const char EncodeTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        //返回值
        std::string strEncode;
        unsigned char Tmp[4] = { 0 };
        int LineLength = 0;
        for (int i = 0; i < (int)(data_length / 3); i++) {
            Tmp[1] = *data++;
            Tmp[2] = *data++;
            Tmp[3] = *data++;
            strEncode += EncodeTable[Tmp[1] >> 2];
            strEncode += EncodeTable[((Tmp[1] << 4) | (Tmp[2] >> 4)) & 0x3F];
            strEncode += EncodeTable[((Tmp[2] << 2) | (Tmp[3] >> 6)) & 0x3F];
            strEncode += EncodeTable[Tmp[3] & 0x3F];
            if (LineLength += 4, LineLength == 76) {
                strEncode += "\r\n";
                LineLength = 0;
            }
        }
        //对剩余数据进行编码
        int Mod = data_length % 3;
        if (Mod == 1) {
            Tmp[1] = *data++;
            strEncode += EncodeTable[(Tmp[1] & 0xFC) >> 2];
            strEncode += EncodeTable[((Tmp[1] & 0x03) << 4)];
            strEncode += "==";
        }
        else if (Mod == 2) {
            Tmp[1] = *data++;
            Tmp[2] = *data++;
            strEncode += EncodeTable[(Tmp[1] & 0xFC) >> 2];
            strEncode += EncodeTable[((Tmp[1] & 0x03) << 4) | ((Tmp[2] & 0xF0) >> 4)];
            strEncode += EncodeTable[((Tmp[2] & 0x0F) << 2)];
            strEncode += "=";
        }

        return strEncode;
    }

    std::string Utility::Base64Decode(const char* data, int data_length, int& binary_length) {
        //解码表
        const char DecodeTable[] = {
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
        //返回值
        std::string strDecode;
        int nValue;
        int i = 0;
        while (i < data_length) {
            if (*data != '\r' && *data != '\n') {
                nValue = DecodeTable[*data++] << 18;
                nValue += DecodeTable[*data++] << 12;
                strDecode += (nValue & 0x00FF0000) >> 16;
                binary_length++;
                if (*data != '=') {
                    nValue += DecodeTable[*data++] << 6;
                    strDecode += (nValue & 0x0000FF00) >> 8;
                    binary_length++;
                    if (*data != '=') {
                        nValue += DecodeTable[*data++];
                        strDecode += nValue & 0x000000FF;
                        binary_length++;
                    }
                }
                i += 4;
            }
            else { // 回车换行,跳过
                data++;
                i++;
            }
        }
        return strDecode;
    }

    bool Utility::InitLogger(const std::string& log_properties_file_path) {
        if (true == log_properties_file_path.empty()) {
            return false;
        }

        ysos_utility::g_configure_and_watch_thread_ptr_ = new log4cplus::ConfigureAndWatchThread(log_properties_file_path.c_str(), 30 * 1000);
        assert(NULL != ysos_utility::g_configure_and_watch_thread_ptr_);
        // log4cplus::ConfigureAndWatchThread configureThread("log4cplus.properties", 5 * 1000);

        return true;
    }

    /**
     * ==================================================================
     *           stringUtil for string special process 
     * ===================================================================
     **/
    //////////////////////////////////////////////////////////////////////////
    ///@brief 安全拷贝字符串,从给定目标地址处开始拷贝
    ///
    ///如果要拷贝的字符串超过限定的长度，则只拷贝在（限定长度－1）个字符。尾字符为字符串'\0'
    ///@param[in] dest 目标字符数组
    ///@param[in] max_copy_size 限定拷贝的字符数量
    ///@param[in] source 需要拷贝的字符串
    void Utility::SafeStringCopy(char* dest, const unsigned int max_copy_size, const char* source) {
        unsigned int length_source =0;

        if (dest==NULL || source==NULL || max_copy_size<1) return;
        length_source = strlen(source);
        //assert(max_copy_size>length_source); //debug assert

        if (max_copy_size>length_source) {
            //strcpy(dest,source);
            //strcpy_s(dest,max_copy_size,source);
            strncpy(dest, source, max_copy_size);//add for linux
        } else {
            memcpy(dest,source,max_copy_size-1);
        }
    }

    ///@brief 安全附加拷贝字符串，从给定目标字符串结尾处开始拷贝
    ///
    ///限定拷贝的字符串附加到目标串结尾后不能超过目标数组的大小，如果超过了，则只拷贝一部分字符
    ///@param[in] dest 目标字符数组
    ///@param[in] dest_size目标数组总的大小
    ///@param[in] source 需要拷贝的字符串
    //////////////////////////////////////////////////////////////////////////
    void Utility::SafeStringCat(char* dest, const unsigned int dest_size, const char* source) {
        int length_dest = 0;
        int length_source = 0;
        int can_copy_length = 0;

        if (dest==NULL || source==NULL|| dest_size<1) return;

        length_dest = strlen(dest);
        length_source = strlen(source);
        can_copy_length = dest_size-length_dest;
        if (can_copy_length<0) return;

        //	assert(can_copy_length>length_source); //在debug模式下断言提醒

        if (can_copy_length>length_source) {
            //strcpy(&dest[length_dest],source);
            //strcpy_s(&dest[length_dest],can_copy_length,source);
            strncpy(&dest[length_dest], source, can_copy_length);
        } else { //can_copy_length<=length_source
            memcpy(&dest[length_dest], source, can_copy_length-1);
            dest[dest_size-1]='\0'; // add a terminal char
        }
    }

    char* Utility::StrReplace(char* source, char* sub, char* rep) {
        char* result;
        /*pc1　 是复制到结果result的扫描指针*/
        /*pc2　 是扫描 source 的辅助指针*/
        /*pc3　 寻找子串时,为检查变化中的source是否与子串相等,是指向sub的扫描指针 */
        /*找到匹配后,为了复制到结果串,是指向rep的扫描指针*/
        char *pc1, *pc2, *pc3;
        int isource, isub, irep;
        if (NULL == sub)
            return source;

        isub = strlen(sub); /*对比字符串的长度*/
        irep = strlen(rep); /*替换字符串的长度*/
        isource = strlen(source); /*源字符串的长度*/
        if (0 == isub)
            return source;

        /*申请结果串需要的空间*/
        result = (char*)malloc(((irep > isub) ? ((long)((double)isource / isub * irep) + 2):(isource + 1)) * sizeof(char));
        pc1 = result; /*为pc1依次复制结果串的每个字节作准备*/
        if (result == NULL)
            return NULL;
        while (*source != '\0') {
            /*为检查source与sub是否相等作准备,为pc2,pc3 赋初值*/
            pc2 = source;
            pc3 = sub;
            /* 出循环的（任一）条件是：　
            *　 *pc2　不等于 *pc3　（与子串不相等）　
            *　 pc2　 到源串结尾　
            *　 pc3　 到源串结尾　（此时,检查了全部子串,source处与sub相等）　
            *****************************************************/
            while (*pc2==*pc3 && *pc3!=0 && *pc2!=0)
            pc2++, pc3++;
            /* 如果找到了子串,进行以下处理工作*/
            if ('\0' == *pc3) {
            pc3 = rep;
            /*将替代串追加到结果串*/
            while (*pc3 != 0)
                *pc1++ = *pc3++;
            pc2--;
            source = pc2;
            /* 检查 source与sub相等的循环结束后，　
            　　* pc2 对应的位置是在 sub 中串结束符处。该是源串中下一个位置。　
            　　* 将　source 指向其前面一个字符。　
            　　***************************************************/
            } else {
            /*如果没找到子串,下面复制source所指的字节到结果串*/
            *pc1++ = *source;
            }
            source++; /* 将source向后移一个字符*/
        }
        *pc1 = '\0';
        return result;
    }

    std::string& Utility::ReplaceAll(std::string& str, const std::string& old_value, const std::string& new_value) {
        while (true)   {
            std::string::size_type pos(0);
            if ((pos = str.find(old_value)) != std::string::npos) {
                str.replace(pos,old_value.length(),new_value);
            } else {
                break;
            }
        }
        return str;
    }

    std::string& Utility::ReplaceAllDistinct(std::string& str, const std::string& old_value, const std::string& new_value) {
        for (std::string::size_type pos(0); pos!=std::string::npos; pos+=new_value.length())   {
            if ((pos = str.find(old_value,pos)) != std::string::npos) {
                str.replace(pos, old_value.length(), new_value);
            } else {
                break;
            }        
        }
        return str;
    }
    /**
     * 字符串分割函数
     **/
    std::vector<std::string> Utility::SplitString2(std::string str, std::string pattern) {
        std::string::size_type pos;
        std::vector<std::string> result;
        str += pattern;//扩展字符串以方便操作
        int size = str.size();

        for (int i=0; i<size; i++) {
            pos=str.find(pattern,i);
            if (pos<size) {
                std::string s = str.substr(i,pos-i);
                result.push_back(s);
                i = pos + pattern.size() - 1;
            }
        }
        return result;
    }

    std::string Utility::Trim(const char * src, char ch /*= ' '*/) {
        int startpos = 0;
        //int endpos = _tcslen(src) -1;
        int endpos = strlen(src) -1;

        while (startpos<=endpos && ((src[startpos] == ch) || (src[startpos] == '\n') || (src[startpos] == '\r'))) {
            ++startpos;
        }
        if (startpos>endpos) {
            return ("");
        }
        while (endpos>=startpos && ((src[startpos] == ch) || (src[startpos] == '\n') || (src[startpos] == '\r'))) {
            --endpos;
        }
        return std::string(src+startpos, endpos-startpos+1);
    }

    std::string Utility::Trim2(const char * src) {
        int startpos = 0;
        //int endpos = _tcslen(src) -1;
        int endpos = strlen(src) -1;
        char ch = ' ';

        while (startpos<=endpos && ((src[startpos] == ch) || (src[startpos] == '\n') || (src[startpos] == '\r'))) {
            ++startpos;
        }
        if (startpos>endpos) {
            return ("");
        }
        while (endpos>=startpos && ((src[startpos] == ch) || (src[startpos] == '\n') || (src[startpos] == '\r'))) {
            --endpos;
        }
        return std::string(src+startpos, endpos-startpos+1);
    }

    bool Utility::SBase64Encode(const std::string& input, std::string* output) {
        typedef boost::archive::iterators::base64_from_binary<boost::archive::iterators::transform_width<std::string::const_iterator, 6, 8> > Base64EncodeIterator;
        std::stringstream result;
        copy(Base64EncodeIterator(input.begin()) , Base64EncodeIterator(input.end()), std::ostream_iterator<char>(result));
        size_t equal_count = (3 - input.length() % 3) % 3;
        for (size_t i = 0; i < equal_count; i++) {
            result.put('=');
        }
        *output = result.str();
        return output->empty() == false;
    }

    bool Utility::SBase64Decode(const std::string& input, std::string* output) {
        typedef boost::archive::iterators::transform_width<boost::archive::iterators::binary_from_base64<std::string::const_iterator>, 8, 6> Base64DecodeIterator;
        std::stringstream result;
        try {
            copy(Base64DecodeIterator(input.begin()) , Base64DecodeIterator(input.end()), std::ostream_iterator<char>(result));
        } catch (...) {
            return false;
        }
        *output = result.str();
        return output->empty() == false;
    }
    
    //template <typename T1>
    int Utility::SplitToInt(const char * src, std::vector<int> & dest, char ch /*= ','*/) {
        int item_count = 0;
        int last_offset = 0,current_pos, src_len;

        dest.clear();

        src_len = strlen(src);
        if (src_len == 0) return 0;

        current_pos = 0;
        do {
            if (src[current_pos] == ch || current_pos==src_len) {
            if (current_pos == last_offset) { //null string

            } else {
                std::string item = Trim2(std::string(src+last_offset, current_pos - last_offset).c_str());
                if (item.length()) {
                  //dest.push_back((T1) atoi(item.c_str()));
                  dest.push_back(atoi(item.c_str()));
                }
            }
            item_count++;
            last_offset = current_pos+1;
            }
        } while ((++current_pos)<=src_len);


        return item_count;
    }


    /**
     *@brief 
    *@param 
    *@return  splite count
    */
    //template <typename TT>
    int Utility::SplitString2Vec(const char * src, std::vector<std::string> & dest, char ch /*= ','*/) {
        int item_count = 0;
        int last_offset = 0,current_pos, src_len;

        dest.clear();

        src_len = strlen(src);
        if (src_len == 0) return 0;

        current_pos = 0;
        do {
            if (src[current_pos] == ch || current_pos==src_len) {
            if (current_pos == last_offset) { //null string

            } else {
                std::string item = Trim2(std::string(src+last_offset, current_pos - last_offset).c_str());
                if (item.length()) {
                  //dest.push_back((TT) (item.c_str()));
                  dest.push_back(item.c_str());
                }
            }
            item_count++;
            last_offset = current_pos+1;
            }
        } while ((++current_pos)<=src_len);

        return item_count;
    }

    static int preNUm(unsigned char byte) {
        unsigned char mask = 0x80;
        int num = 0;
        for (int i = 0; i < 8; i++) {
            if ((byte & mask) == mask) {
                mask = mask >> 1;
                num++;
            } else {
                break;
            }
        }
        return num;
    }

    static bool isUtf8(unsigned char* data, int len) {
        int num = 0;
        int i = 0;
        while (i < len) {
            if ((data[i] & 0x80) == 0x00) {
                // 0XXX_XXXX
                i++;
                continue;
            }
            else if ((num = preNUm(data[i])) > 2) {
            // 110X_XXXX 10XX_XXXX
            // 1110_XXXX 10XX_XXXX 10XX_XXXX
            // 1111_0XXX 10XX_XXXX 10XX_XXXX 10XX_XXXX
            // 1111_10XX 10XX_XXXX 10XX_XXXX 10XX_XXXX 10XX_XXXX
            // 1111_110X 10XX_XXXX 10XX_XXXX 10XX_XXXX 10XX_XXXX 10XX_XXXX
            // preNUm() 返回首个字节8个bits中首�?0bit前面1bit的个数，该数量也是该字符所使用的字节数        
            i++;
            for(int j = 0; j < num - 1; j++) {
                //判断后面num - 1 个字节是不是都是10开
                if ((data[i] & 0xc0) != 0x80) {
                        return false;
                    }
                    i++;
            }
        } else {
            //其他情况说明不是utf-8
            return false;
        }
        } 
        return true;
    }

    static bool isGBK(unsigned char* data, int len)  {
        int i  = 0;
        while (i < len)  {
            if (data[i] <= 0x7f) {
                //编码小于等于127,只有一个字节的编码，兼容ASCII
                i++;
                continue;
            } else {
                //大于127的使用双字节编码
                if 	(data[i] >= 0x81 &&
                    data[i] <= 0xfe &&
                    data[i + 1] >= 0x40 &&
                    data[i + 1] <= 0xfe &&
                    data[i + 1] != 0xf7) {
                    i += 2;
                    continue;
                } else {
                    return false;
                }
            }
        }
        return true;
    }
    /**
      *需要说明的是，isGBK()是通过双字节是否落在gbk的编码范围内实现的，
      *而utf-8编码格式的每个字节都是落在gbk的编码范围内
      *所以只有先调用isUtf8()先判断不是utf-8编码，再调用isGBK()才有意义
    **/
    Utility::EncodingType Utility::GetEncoding(unsigned char* data, int len) {
        Utility::EncodingType coding;
        if (isUtf8(data, len) == true) {
            coding = UTF8;
        } else if (isGBK(data, len) == true) {
            coding = ASCII;
        } else {
            coding = EncodingTypeEnd;
        }
        return coding;
    }

    Utility::EncodingType Utility::GetEncoding(std::string data) {
        return GetStringEncoding((unsigned char*)(data.c_str()));
    }
    
}
