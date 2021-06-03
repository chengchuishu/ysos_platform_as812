/*@version 1.0
  *@author l.j..
  *@date Created on: 2016-10-20 13:48:23
  *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
  * 
*/

/// Private Headers //  NOLINT
#include "../include/nlpextdriver.h"

// boost headers
#include <boost/thread/thread_guard.hpp>
#include <boost/locale.hpp>
#include <boost/filesystem.hpp>
#include <boost/atomic.hpp>
#include <json/json.h>

namespace ysos {

#ifdef _WIN32
  #include <Windows.h>
  #include <process.h>
  #define CreateThreadEx(tid,threadFun,args) _beginthreadex(tid, 0, threadFun, args, 0, NULL)
/*
 HANDLE WINAPI _beginthreadex(
   //线程内核对象的安全属性，一般传入NULL表示使用默认设置。
   LPSECURITY_ATTRIBUTES lpThreadAttributes,
   //线程栈空间大小。传入0表示使用默认大小（1MB）。
   SIZE_T dwStackSize,
   //新线程所执行的线程函数地址，多个线程可以使用同一个函数地址。
   LPTHREAD_START_ROUTINE lpStartAddress,
   //传给线程函数的参数。
   LPVOID lpParameter,
   //指定额外的标志来控制线程的创建，为0表示线程创建之后立即就可以进行调度，如果为CREATE_SUSPENDED则表示线程创建后暂停运行，这样它就无法调度，直到调用ResumeThread()。
   DWORD dwCreationFlags,
   //返回线程的ID号，传入NULL表示不需要返回该线程ID号。
   LPDWORD lpThreadId
 );*/
#else
//TODO:add for linux 
int strcpy_s(char *strDestination, size_t numberOfElements, const char *strSource)
{
   strncpy(strDestination, strSource, numberOfElements);//add for linux
   return 0;
}
// 返回自系统开机以来的毫秒数（tick）
unsigned long GetTickCount()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}
#endif

    DECLARE_PLUGIN_REGISTER_INTERFACE ( NlpExtDriver, DriverInterface ); ///<  插件的入口，一定要加上 //  NOLINT
    NlpExtDriver::NlpExtDriver ( const std::string &strClassName ) : BaseDriverImpl ( strClassName )
    {
        //interrupt_type_ = 0;
        thread_exit_ = false;
        nlp_http_client_ = NULL;
        filter_asr_local_engine_query_ = 1;
        config_local_record_file_filter_mode_ = 0;
        config_local_record_file_path_ = "./log/accumulation";
        config_local_record_file_dialog_ = 0;
        config_local_record_file_dialog_path_ = "./log/dialog";
        logger_ = GetUtility()->GetLogger ( "ysos.audio.nlpext" );
        bussiness_switch_ = 0;
        hand_ = nullptr;
        is_clear_ = false;
        init_connect_timeout_ = 2;
        init_read_timeout_ = 3;
        renewal_connect_timeout_ = 2;
        renewal_read_timeout_ = 3;
        query_connect_timeout_ = 2;
        query_read_timeout_ = 3;
        clear_connect_timeout_ = 2;
        clear_read_timeout_ = 3;
    }

    NlpExtDriver::~NlpExtDriver ( void )
    {
    }

    int NlpExtDriver::SetProperty ( int type_id, void *type )
    {
        YSOS_LOG_DEBUG ( "driver SetProperty execute" );
        int n_return = YSOS_ERROR_SUCCESS;
        do
        {
        } while ( 0 );
        YSOS_LOG_DEBUG ( "driver SetProperty done" );
        return n_return;
    }

    int NlpExtDriver::GetProperty ( int type_id, void *type )
    {
        int n_return = YSOS_ERROR_FAILED;
        do
        {
            if ( NULL == type )
            {
                YSOS_LOG_DEBUG ( "param error" );
                break;
            }
            switch ( type_id )
            {
                case PROP_FUN_CALLABILITY:
                    FunObjectCommon2* pobject = reinterpret_cast<FunObjectCommon2*> ( type );
                    if ( NULL == pobject )
                    {
                        YSOS_LOG_ERROR ( "error call property PROP_Fun_CallAbility, the param is null" );
                        break;
                    }
                    AbilityParam* pin = reinterpret_cast<AbilityParam*> ( pobject->pparam1 );
                    AbilityParam* pout = reinterpret_cast<AbilityParam*> ( pobject->pparam2 );
                    n_return = CallAbility ( pin, pout );
                    break;
            }
        } while ( 0 );
        return n_return;
    }

    int NlpExtDriver::Read ( BufferInterfacePtr pBuffer, DriverContexPtr context_ptr/*=NULL*/ )
    {
        YSOS_LOG_DEBUG ( "driver Read execute" );
        int n_return = YSOS_ERROR_SUCCESS;
        do
        {
            UINT8* buffer;
            UINT32 buffer_size;
            n_return = pBuffer->GetBufferAndLength ( &buffer, &buffer_size );
            if ( YSOS_ERROR_SUCCESS != n_return )
            {
                break;
            }
        } while ( 0 );
        YSOS_LOG_DEBUG ( "driver Read done" );
        return n_return;
    }

    int NlpExtDriver::Write ( BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer/*=NULL*/, DriverContexPtr context_ptr/*=NULL*/ )
    {
        YSOS_LOG_DEBUG ( "driver Write execute" );
        int n_return = YSOS_ERROR_SUCCESS;
        do
        {
            UINT8* buffer;
            UINT32 buffer_size;
            n_return = input_buffer->GetBufferAndLength ( &buffer, &buffer_size );
            if ( YSOS_ERROR_SUCCESS != n_return )
            {
                break;
            }
        } while ( 0 );
        YSOS_LOG_DEBUG ( "driver Write done" );
        return n_return;
    }

    int NlpExtDriver::Ioctl ( int iCtrlID, BufferInterfacePtr pInputBuffer, BufferInterfacePtr pOutputBuffer )
    {
        YSOS_LOG_DEBUG ( "driver Ioctl execute" );
        int n_return = YSOS_ERROR_SUCCESS;
        int n_result = -1;
        Json::FastWriter js_writer;
        Json::Value js_value;
        std::string str_result;
        do
        {
            if ( pInputBuffer == NULL )
            {
                YSOS_LOG_ERROR ( "driver pOutputBuffer is null" );
                n_return = YSOS_ERROR_FAILED;
                break;
            }
            YSOS_LOG_INFO ( "iCtrlID = " << iCtrlID );
            UINT8* buffer;
            UINT32 buffer_size;
            n_return = pInputBuffer->GetBufferAndLength ( &buffer, &buffer_size );
            if ( YSOS_ERROR_SUCCESS != n_return )
            {
                YSOS_LOG_ERROR ( "GetBufferAndLength failed, n_return = " << n_return );
                break;
            }
            switch ( iCtrlID )
            {
                case CMD_AUDIO_JSON_NLP:
                    {
                        break;
                    }
                case CMD_AUDIO_CONTEXT_CLEAR:
                    {
                        //TODO: need add for linux
                        break;
                    }
                default:
                    {
                        n_return = YSOS_ERROR_FAILED;
                        break;
                    }
            } //switch
        } while ( 0 );
        YSOS_LOG_DEBUG ( "driver Ioctl done" );
        return n_return;
    }

    int NlpExtDriver::Open ( void *pParams )
    {
        YSOS_LOG_DEBUG ( "driver Open execute" );
        int n_return = YSOS_ERROR_FAILED;
        do
        {
            data_ptr_ = GetDataInterfaceManager()->GetData ( GLOBAL_DATA_KEY );
            if ( !data_ptr_ )
            {
                YSOS_LOG_ERROR ( "get data_ptr failed" );
                break;
            }
            /*
            if ( nlp_http_client_ == NULL )
            {
                nlp_http_client_ = new HttpClient();
                nlp_http_client_->SetHttpHeader("mall_code","8000000001");
                nlp_http_client_->SetHttpHeader("machine_code","HX-00066");
                nlp_http_client_->SetHttpHeader("floor",std::to_string(3));
                nlp_http_client_->SetHttpHeader("session_id","");
                nlp_http_client_->SetHttpHeader("Content-Type","application/json;charset=UTF-8");
                if ( nlp_http_client_ == NULL )
                {
                    YSOS_LOG_ERROR ( "get nlp_http_client_ failed" );
                    break;
                }
            }*/
            //TODO:start get tokenid info in time
            /* add for linux    
            HANDLE handle = ( HANDLE ) _beginthreadex ( NULL, 0, QueryThread, this, 0, &thread_id_ );
            if ( handle == NULL )
            {
                YSOS_LOG_ERROR ( "create work thread failed" );
                break;
            }
            CloseHandle ( handle );
            */
            n_return = YSOS_ERROR_SUCCESS;
        } while ( 0 );
        YSOS_LOG_DEBUG ( "driver Open done" );
        return n_return;
    }

    void NlpExtDriver::Close ( void *pParams /* = nullptr */ )
    {
        YSOS_LOG_DEBUG ( "driver Close execute" );
        thread_exit_ = true;
        if ( nlp_http_client_ )
        {
            delete nlp_http_client_;
        }
        YSOS_LOG_DEBUG ( "driver Close done" );
    }

    int NlpExtDriver::CallAbility ( AbilityParam *pInput, AbilityParam *pOut )
    {
        int n_return = YSOS_ERROR_FAILED;
        do
        {
            if ( NULL == pInput || NULL == pOut )
            {
                YSOS_LOG_ERROR ( "error call ablicity , input/out alibityparam is null" );
                break;
            }
            YSOS_LOG_INFO ( "c" << pOut->ability_name << ", datatype from: " << pInput->data_type_name << "to: " << pOut->ability_name );
#ifdef _WIN32
            if ( stricmp ( ABSTR_NLP, pOut->ability_name ) == 0 )
            {
                n_return = AblityNlp_JsonExtract2Json ( pInput, pOut );
                break;
            }
#else
            if ( strcasecmp ( ABSTR_NLP, pOut->ability_name ) == 0 )
            {
                n_return = AblityNlp_JsonExtract2Json ( pInput, pOut );
                break;
            }
#endif
            YSOS_LOG_DEBUG ( "error not support ability" );
        } while ( 0 );
        return n_return;
    }

    void NlpExtDriver::LogDialog ( const std::string log_date, const std::string log_time, std::string logpath, const std::string term_id, int mode, const std::string msg )
    {
        if ( term_id == "" )
        {
            return;
        }
        std::string strTime = log_date + "_" + log_time;
        std::string filename = term_id + "_" + log_date + "_dialog.txt";
        std::string filefullname = logpath + "/" + filename;
        ///<创建记录文件夹
        if ( !boost::filesystem::exists ( logpath ) )
        {
            boost::system::error_code myecc;
            boost::filesystem::create_directories ( logpath, myecc );
        }
        std::string logmsg = strTime;
        if ( mode == 0 )
        {
            logmsg += " 问题：" + msg;
        }
        else
        {
            logmsg += " 答案：" + msg;
        }
        FILE *fp = fopen ( filefullname.c_str(), "a+" );
        if ( fp != NULL )
        {
            //fprintf_s ( fp, "%s\n", logmsg.c_str() );
            fprintf ( fp, "%s\n", logmsg.c_str() );//add for linux
            fclose ( fp );
        }
    }

    void NlpExtDriver::LogNlpAccumulation ( const std::string log_date, const std::string log_time, const std::string filepath, const std::string term_id, const std::string &asr_result, const std::string &nlp_result )
    {
        if ( term_id == "" )
        {
            return;
        }
        std::string strTime = log_date + "_" + log_time;
        std::string filename = term_id + "_" + log_date + "_nlp.txt";
        std::string logmsg = strTime + "|" + "nlp问题:" + asr_result + "|" + "nlp答案:" + nlp_result ;
        std::string filefullname = filepath + "/" + filename;
        ///<创建记录文件夹
        if ( !boost::filesystem::exists ( filepath ) )
        {
            boost::system::error_code myecc;
            boost::filesystem::create_directories ( filepath, myecc );
        }
        FILE *fp = fopen ( filefullname.c_str(), "a+" );
        if ( fp != NULL )
        {
            //fprintf_s ( fp, "%s\n", logmsg.c_str() );
            fprintf ( fp, "%s\n", logmsg.c_str() );//add for linux
            fclose ( fp );
        }
    }

    int NlpExtDriver::AblityNlp_JsonExtract2Json ( AbilityParam *pInput, AbilityParam *pOut )
    {
        int n_return = YSOS_ERROR_FAILED;
        bool is_result_ok = false;
        std::string recv_asr_result = "";
        std::string log_date = "";
        std::string log_time = "";
        YSOS_LOG_DEBUG ( "AblityNlp_JsonExtract2Json in" );
        ///<获取终端号
        if ( term_id_ == "" )
        {
            DataInterfacePtr data_ptr = GetDataInterfaceManager()->GetData ( GLOBAL_DATA_KEY );
            data_ptr->GetData ( "termId", term_id_ );
        }
        do
        {
            if ( NULL == pInput->buf_interface_ptr || NULL == pOut->buf_interface_ptr )
            {
                YSOS_LOG_ERROR ( "param is null" );
                break;
            }
            int time_interval;
            UINT8* ptext = NULL;
            UINT32 buffer_size = 0;
            pInput->buf_interface_ptr->GetBufferAndLength ( &ptext, &buffer_size ); //< 里面包含两个字符串，得提取出来
            std::string asr_result = reinterpret_cast<char*> ( ptext );
            ioctl_flag_ = false;
            std::string str_text_need_nlp = asr_result;
            YSOS_LOG_INFO ( "asr_data = " << GetUtility()->AsciiToUtf8(str_text_need_nlp));
            //TODO:add for linux
            int ind = str_text_need_nlp.find_first_of('{');
            if ( ind < 0 ) {
              str_text_need_nlp = "{\"answer_best\":\"\",\"answer_old\":\"\",\"asr_mode\":\"\",\"is_timeout\":0,\"other_answers\":\"\",\"rc\":\"0\",\"rg\":\"0\",\"service\":\"\",\"text\":\"" + asr_result + "\",\"time_interval\":3}";
            }
            // {"answer_best":"","other_answers":"","rc":"0","rg":"0","serialNo":113646,"service":"","time_interval":0,"uniqueCode":"20171109155116"}
            if ( str_text_need_nlp.length() > 0 ) //< 摄取json中特定的字段
            {
                Json::Reader json_reader;
                Json::Value ptree_json;
                json_reader.parse ( str_text_need_nlp, ptree_json, true );
                str_text_need_nlp = ptree_json["text"].asString();
                time_interval = ptree_json["time_interval"].asInt();
                ioctl_flag_ = ptree_json["ioctl"].asBool();
                std::string rsp_code = ptree_json["rc"].asString();
                //add for tips error info
                if ( "0" != rsp_code && "-1" != rsp_code )
                {
                    YSOS_LOG_ERROR ( "rsp_code = " << rsp_code );
                    break;
                }
                
                if ("-1" == rsp_code) {
                    str_text_need_nlp = GetUtility()->GbkToUtf8(str_text_need_nlp);
                }

                recv_asr_result = str_text_need_nlp;
            }
            if ( str_text_need_nlp.empty() && !ioctl_flag_ ) //< 如果传入为空，则返回空数据
            {
                pOut->buf_interface_ptr->SetLength ( 0 );
                n_return = YSOS_ERROR_SUCCESS;
                break;
            }
            /*  plase replace this block
            int time_begin = GetTickCount();
            std::string tang_di_result = "";
            
            n_return = TdQuery ( asr_result, tang_di_result );
            if ( YSOS_ERROR_SUCCESS != n_return )
            {
                break;
            }
            int time_end = GetTickCount();
            time_interval += time_end - time_begin;
            //< 转换成规范后的数据
            std::string convert_result = "";
            std::string nlp_service = "";
            ConvertNlpResultToStandasd ( asr_result, tang_di_result, convert_result, nlp_service, time_interval );
            */
            int time_begin = GetTickCount();
            std::string tang_di_result = "";
            n_return = TdQuery ( str_text_need_nlp, tang_di_result );
            if ( YSOS_ERROR_SUCCESS != n_return )
            {
                break;
            }
            int time_end = GetTickCount();
            time_interval += time_end - time_begin;
            //< 转换成规范后的数据
            std::string convert_result = "";
            std::string nlp_service = "";
            ConvertNlpResultToStandasd ( str_text_need_nlp, tang_di_result, convert_result, nlp_service, time_interval );
            //TODO:add for debug
            //{\"answer_best\":\"请咨询导诊台。\",\"commands\":[null],\"orign_rspbody\":null,\"parameterSet\":null,\"rc\":\"0\",\"recommendQs\":[],\"rg\":\"0\",\"service\":\"chat\",\"showContent\":\"\",\"text\":\"帅哥你好很高兴为您服务\",\"time_interval\":1234}
            //std::string convert_result = "{\"answer_best\":\"" + str_text_need_nlp + "\",\"commands\":[null],\"orign_rspbody\":null,\"parameterSet\":null,\"rc\":\"0\",\"recommendQs\":[],\"rg\":\"0\",\"service\":\"chat\",\"showContent\":\"\",\"text\":\"" + str_text_need_nlp + "\",\"time_interval\":100}";
            YSOS_LOG_INFO ( "convert_result [before] = " << convert_result);
            convert_result = GetUtility()->Utf8ToAscii(convert_result);
            YSOS_LOG_INFO ( "convert_result [after] = " << convert_result);
            //YSOS_LOG_INFO ( "convert_result = " << GetUtility()->AsciiToUtf8(convert_result));
            UINT8* pout = NULL;
            UINT32 pout_size = 0;
            pOut->buf_interface_ptr->GetBufferAndLength ( &pout, &pout_size );
            {
                UINT32 max_length = 0, prefix_length = 0;
                pOut->buf_interface_ptr->GetMaxLength ( &max_length );
                pOut->buf_interface_ptr->GetPrefixLength ( &prefix_length );
                pout_size = max_length - prefix_length;
            }
            if ( NULL == pout )
            {
                YSOS_LOG_ERROR ( "error call ability pcmx1x->asr, pout is null" );
                break;
            }
            if ( pout_size < ( convert_result.length() + 1 ) )
            {
                YSOS_LOG_ERROR ( "error call ability pcmx1x->asr, output buffer is too small" );
                break;
            }
            if ( convert_result.length() > 0 )
            {
                strcpy_s ( reinterpret_cast<char*> ( pout ), ( convert_result.length() + 1 ), convert_result.c_str() );
                pOut->buf_interface_ptr->SetLength ( convert_result.length() + 1 );
                is_result_ok = true;
            }
            else
            {
                pOut->buf_interface_ptr->SetLength ( 0 ); //< 结果为空
            }
            n_return = YSOS_ERROR_SUCCESS;
        } while ( 0 );
        return n_return;
    }

    int NlpExtDriver::TdQuery ( const std::string &asr_text, std::string &rsp_data )
    {
        int n_return = YSOS_ERROR_FAILED;
        /* no token
        if ( token_.length() <= 0 )
        {
            if ( n_return != YSOS_ERROR_SUCCESS )
            {
                YSOS_LOG_ERROR ( "token_ is null" );
                return n_return;
            }
        }*/
        do
        {
            /*
            Json::Value json_asr;
            json_reader_.parse ( asr_result, json_asr, true );
            std::string cur_time = boost::posix_time::to_iso_string ( boost::posix_time::second_clock::local_time() );
            cur_time.replace ( cur_time.find ( 'T' ), 1, "" );
            int serial_no = json_asr["serialNo"].asInt();
            std::string question = json_asr["text"].asString();
            */
            
            Json::Value business_set;
            business_set["nav_switch"] = config_nav_switch_;
            business_set["is_wake"] = config_is_wake_;
            business_set["location"] =  GetUtility()->GbkToUtf8(config_location_);

            Json::Value req_body;
            req_body["sentence"] = GetUtility()->AsciiToUtf8(asr_text);
            req_body["business"] = business_set;
            
            std::string req_data = json_writer_.write ( req_body );
            YSOS_LOG_INFO ( "req_data = " << req_data );
            SymbolClear ( req_data );
            
            if ( nlp_http_client_ == NULL )
            {
                nlp_http_client_ = new HttpClient();
                nlp_http_client_->SetHttpHeader("Content-Type", "application/json;charset=UTF-8");
                nlp_http_client_->SetHttpHeader("mall_code", config_mall_code_);
                nlp_http_client_->SetHttpHeader("machine_code", config_machine_code_);
                nlp_http_client_->SetHttpHeader("floor", config_floor_);
                nlp_http_client_->SetHttpHeader("session_id","");
                if ( nlp_http_client_ == NULL )
                {
                    YSOS_LOG_ERROR ( "get nlp_http_client_ failed" );
                    break;
                }
            }

            int timeout_count = 0;
            //n_return = HTTPPostExt(config_td_query_url_, req_data, rsp_data);
            nlp_http_client_->SetHttpUrl ( config_zj_query_url_ );
            
            nlp_http_client_->SetHttpContent ( req_data.c_str(), req_data.length() );
            nlp_http_client_->SetTimeOut ( query_connect_timeout_, query_read_timeout_ );
            n_return = nlp_http_client_->Request ( rsp_data, HTTP_POST);
            if ( YSOS_ERROR_SUCCESS != n_return )
            {
                YSOS_LOG_ERROR ( "tangd di http post fail, n_return = " << n_return << " URL:" << config_td_query_url_ << "  reqdata:" << req_data << " rspdata:" << rsp_data );
                break;
            }
            if ( rsp_data.empty() )
            {
                break;
            }
            //rsp_data = boost::locale::conv::from_utf<char> ( rsp_data, "gbk" );
            YSOS_LOG_INFO ( "get response data = " << rsp_data );
            n_return = YSOS_ERROR_SUCCESS;
        } while ( 0 );
        return n_return;
    }

    int NlpExtDriver::ConvertNlpResultToStandasd ( const std::string &asr_text, const std::string &nlp_result, std::string &convert_result, std::string& rsp_service, const int &time )
    {
        int n_return = YSOS_ERROR_FAILED;
        YSOS_LOG_DEBUG ( "ConvertNlpResultToStandasd in" );
        do
        {
            if (nlp_result.empty()) {
              YSOS_LOG_DEBUG("nlpResult empty");
              break;
            }
            Json::Reader json_reader;
            Json::Value nlp_value, std_nlp_value;
            Json::Value analysis, ndata;

            std::string token;
            std::string session_id;
            int state;
            int flag;
            std::string reply;
            std::string erorr_msg;

            json_reader.parse ( nlp_result, nlp_value, false );
            token = nlp_value["token"].asString();
            session_id = nlp_value["session_id"].asString();
            state = nlp_value["state"].asInt();
            flag = nlp_value["flag"].asInt();
            reply = nlp_value["reply"].asString();
            erorr_msg = nlp_value["erorr_msg"].asString();
            ndata = nlp_value["data"];
            analysis = nlp_value["analysis"];
             
            std_nlp_value["rc"] = "0";
            std_nlp_value["rg"] = "0";
            std_nlp_value["text"] = GetUtility()->AsciiToUtf8(asr_text);
            std_nlp_value["service"] = "chat";
            std_nlp_value["time_interval"] = time;
            std_nlp_value["showContent"] = "";
            std_nlp_value["recommendQs"] = "";
            std_nlp_value["parameterSet"] = "";
            std_nlp_value["commands"] = "";
            std_nlp_value["orign_rspbody"] = "";
            std_nlp_value["answer_old"] = reply;
            std_nlp_value["answer_best"] = reply;

            convert_result = json_writer_.write ( std_nlp_value );
            n_return = YSOS_ERROR_SUCCESS;
        } while ( 0 );
        return n_return;
    }

    int NlpExtDriver::Initialize ( void *param/*=NULL*/ )
    {
        return BaseDriverImpl::Initialize ( param );
    }

    int NlpExtDriver::Initialized ( const std::string &key, const std::string &value )
    {
        if ( "result_type" == key )
        {
            result_type_ = boost::lexical_cast<int> ( value );
        }
        else if ( "warn_switch" == key )
        {
            warn_switch_ = boost::lexical_cast<int> ( value );
        }
        else if ( "empty_times" == key )
        {
            empty_times_ = boost::lexical_cast<int> ( value );
        }
        else if ( "warn_words_chat" == key )
        {
            warn_words_chat_ = boost::locale::conv::from_utf ( value, "GBK" );
        }
        else if ( "warn_words_busi" == key )
        {
            //warn_words_busi_ = value;
            warn_words_busi_ = boost::locale::conv::from_utf ( value, "GBK" );
        }
        else if ( "zj_query_url" == key )
        {
            config_zj_query_url_ = value;
        }
        else if ( "mall_code" == key )
        {
            config_mall_code_ = value;
        }
        else if ( "machine_code" == key )
        {
            config_machine_code_ = value;
        }
        else if ( "floor" == key )
        {
            config_floor_ = value;
        }
        else if ( "location" == key )
        {
            config_location_ = boost::locale::conv::from_utf ( value, "GBK" );
        }
        else if ( "nav_switch" == key )
        {
            config_nav_switch_ = atoi ( value.c_str() );
        }
        else if ( "is_wake" == key )
        {
            config_is_wake_ = atoi ( value.c_str() );
        }
        else if ( "filter_asr_local_engine_query" == key )
        {
            filter_asr_local_engine_query_ = atoi ( value.c_str() );
        }
        else if ( "local_record_file_filter_mode" == key )
        {
            config_local_record_file_filter_mode_ = atoi ( value.c_str() );
            YSOS_LOG_DEBUG ( "local_record_file_filter_mode = " << config_local_record_file_filter_mode_ );
        }
        else if ( "local_record_file_dialog" == key )
        {
            config_local_record_file_dialog_ = atoi ( value.c_str() );
            YSOS_LOG_DEBUG ( "local_record_file_dialog = " << config_local_record_file_dialog_ );
        }
        else if ( "local_record_file_path" == key )
        {
            if ( value == "" )
            {
                config_local_record_file_path_ = "./log/dialog";
            }
            else
            {
                config_local_record_file_path_ = value;
            }
            YSOS_LOG_DEBUG ( "local_record_file_path = " << config_local_record_file_path_ );
        }
        else if ( "local_record_file_dialog_path" == key )
        {
            if ( value == "" )
            {
                config_local_record_file_dialog_path_ = "./log/accumulation";
            }
            else
            {
                config_local_record_file_dialog_path_ = value;
            }
            YSOS_LOG_DEBUG ( "local_record_file_dialog_path = " << config_local_record_file_dialog_path_ );
        }
        else if ( "bussiness_mode_switch" == key )
        {
            if ( value != "" )
            {
                bussiness_switch_ = atoi ( value.c_str() );
            }
            YSOS_LOG_DEBUG ( "bussiness_switch_ = " << bussiness_switch_ );
        }
        else if ( "init_connect_timeout" == key )
        {
            init_connect_timeout_ = atoi ( value.c_str() );
        }
        else if ( "init_read_timeout" == key )
        {
            init_read_timeout_ = atoi ( value.c_str() );
        }
        else if ( "renewal_connect_timeout" == key )
        {
            renewal_connect_timeout_ = atoi ( value.c_str() );
        }
        else if ( "renewal_read_timeout" == key )
        {
            renewal_read_timeout_ = atoi ( value.c_str() );
        }
        else if ( "query_connect_timeout" == key )
        {
            query_connect_timeout_ = atoi ( value.c_str() );
        }
        else if ( "query_read_timeout" == key )
        {
            query_read_timeout_ = atoi ( value.c_str() );
        }
        else if ( "clear_connect_timeout" == key )
        {
            clear_connect_timeout_ = atoi ( value.c_str() );
        }
        else if ( "clear_read_timeout" == key )
        {
            clear_read_timeout_ = atoi ( value.c_str() );
        }
        else if ( "timeout_broadcast" == key )
        {
            timeout_broadcast_ = value;
        }
        return YSOS_ERROR_SUCCESS;
    }

    int NlpExtDriver::UnInitialize ( void *param/*=NULL*/ )
    {
        return BaseDriverImpl::UnInitialize ( param );
    }

    void NlpExtDriver::CleanToken ( void )
    {
        token_ = "";
        data_ptr_->SetData ( "token_id", "" );
        YSOS_LOG_DEBUG ( "token clean success" );
    }

    void NlpExtDriver::SymbolClear ( std::string &data )
    {
        data = GetUtility()->ReplaceAllDistinct ( data, "\\r\\n", "" );
    }

}