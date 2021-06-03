/**   //NOLINT
  *@file commonstructdefine.h
  *@brief Definition of struct define
  *@version 0.1
  *@author wanfenfen
  *@date Created on: 2016:11:22   9:51
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  *@howto Usage
  @todo
  */
#ifndef YSOS_PLUGIN_COMMONSTRUCTDEFINE_H
#define YSOS_PLUGIN_COMMONSTRUCTDEFINE_H

/// stl headers
#include <vector>

/// boost headers
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>

/// ysos headers
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/callbackqueue.h"

namespace ysos {


    /**
      *@brief
      *ability_name:
      *             voicerecord
      *             voiceplay
      *             asr
      *             asrnlp
      *             tts
      *data_type_name：
      *             stream_pcmx1x
      *             text_pure
      *             text_json
      *             text_xml
      *
      */
    struct AbilityParam
    {
        char ability_name[256];
        char data_type_name[256];  ///< 数据类型，最多128个字符
        //bool have_data;   ///< 是否有数据，有时候input多次后才会一次输出数据.
        BufferInterfacePtr buf_interface_ptr;
        //int data_length;   ///< 数据的长度(包含协议头，如果有协议的话）， 改成由BufferInterfacePtr 获取（maxlength - prefixlength)。
        //int have_finish;
        int is_not_finish;  ///< 是不是没有传输完数据
        CallbackInterfacePtr callback_interface_ptr;  ///< 如果只提供输出，则需要指定callback， 比如录音机
        bool noresult_return_empty_;                   ///< 如果没有结果，则返回空结果(buffer里面的长度为0）（返回空后,callback会skip当前操作)，比如nlp能力，如果解析不了，则json数据也不会返回
        AbilityParam() {
            memset ( ability_name, '\0', sizeof ( ability_name ) );
            memset ( data_type_name, '\0', sizeof ( data_type_name ) );
            //data_length =0;
            buf_interface_ptr = NULL;
            is_not_finish = 0;
            callback_interface_ptr = NULL;
            noresult_return_empty_ = 0;
        }
    };

    /**
      *@brief fun call with only 2 params
      */
    struct FunObjectCommon2
    {
        void* pparam1;
        void* pparam2;
    };

    struct FunObjectCommon3
    {
        void* pparam1;
        void* pparam2;
        void* pparam3;
    };

    struct FunObjectCommon4
    {
        void* pparam1;
        void* pparam2;
        void* pparam3;
        void* pparam4;
    };

    struct FunObjectCommon5
    {
        void* pparam1;
        void* pparam2;
        void* pparam3;
        void* pparam4;
        void* pparam5;
    };



    /**
    *@brief use power manager
    */
    struct TimeParam
    {
        UINT16 nYear;      ///<  Year
        UINT8  cbMonth;    ///<  Month
        UINT8  cbDay;      ///<  Day
        UINT8  cbHour;     ///<  Hour
        UINT8  cbMin;      ///<  Minute
        UINT8  cbSecond;   ///<  Second
    };

    struct WatchParam
    {
        UINT16 nTime;		///< Time Interval
        UINT8  cbMode;	///< Reboot Mode
    };


    /**
     *  读命令结构体
    */
    struct ReadSet
    {
        UINT32 read_type;		//< 操作类型        //NOLINT
        UINT8 mode;			//< 读磁卡模式      //NOLINT
        UINT8 track;			//< 读磁道号        //NOLINT
    };

    /**
     *  Capdu命令结构体
    */
    struct CapduCmd
    {
        UINT32 mode;			//< 接触式卡类型 CPU(T=0/T=1)卡
        UINT32 len;			//< 命令长度
        UINT8* szData;		//< 命令数据

        //< pboc部分
        long trade_type;		//< 交易类型
        long money_type;		//< 货币类型
        long money;			//< 预授权金额（分为单位，不带小数点）   //NOLINT
    };

    /**
     *  进卡前准备操作结构体
    */
    struct GetReady
    {
        UINT8 ucFront;            //< 前端进卡设置
        UINT8 ucRear;             //< 后端进卡设置
        UINT8 ucPos;              //< 退卡位置
    };

    /**
     *  crt310004,t10em2初始化结构体
    */
    struct CardReaderSetting
    {
        UINT32 card_port;         //< 读卡机串口号
        UINT32 sender_port;       //< 发卡机串口号
        UINT32 card_baudrate;     //< 读卡机波特率
        UINT32 sender_baudrate;   //< 发卡机波特率
        UINT32 card_type;         //< 进卡类型
        UINT8 *device_type;       //< 读卡器设备类型
        UINT8 *sender_device_type;//< 发卡机设备类型
    };

    /**
     *  TLV数据结构体
    */
    struct DataOfLv
    {
        int     length;         //TLV数据的L域
        UINT8   data[4096];     //TLV数据V域
    };

    /**
     *  串口初始化结构体
    */
    struct SerialPort
    {
        int com_port;
        int com_baudrate;
        int com_parity;
        int com_stop_bits;
        int com_byte_size;
        int com_mode;
        int invaltimeout;
    };

    /**
     *  串口通信数据的结构体
    */
    struct PortData
    {
        UINT32 size;
        UINT8* buffer;
    };

    /**
     *  IDTRYM9000MINI初始化结构体
    */
    struct IdtrymInit
    {
        UINT32 idtrym_port;                   //< 串口号
        UINT32 idtrym_baudrate;               //< 波特率
    };

    /**
     *  ET58初始化结构体
    */
    struct Et58Init
    {
        UINT32 et58_port;
        UINT32 code_port;
        UINT32 et58_baudrate;
        UINT8* et58_bmp_path;
        UINT8* et58_dat_path;
    };

    /**
     *  PinPad初始化结构体
    */
    struct PinPadInit
    {
        UINT32 pin_port;
        UINT32 pin_baudrate;
        UINT8 *pin_dll_path;
        UINT8 *device_type;
        UINT8 *auth_code;
        UINT8 *log_path;
    };

    /**
     *  ES4600AT初始化结构体
    */
    struct ES4600ATInit
    {
        UINT32 code_port;                   //< 串口号
        UINT32 code_baudrate;               //< 波特率
    };

    /**
     *  PinPad命令结构体
    */
    struct PinPadCmdParam
    {
        UINT32 pwd_type;          //< 密码接收模式

        UINT32 mas_key_type;      //< 主密钥长度模式
        UINT32 mas_key_no;        //< 主密钥号
        UINT8 *mas_key;           //< 主密钥

        UINT32 work_key_type;     //< 工作密钥长度模式
        UINT32 work_key_no;       //< 工作密钥号
        UINT8 *work_key;          //< 工作密钥

        UINT32 mac_mode;          //< MAC算法模式
        UINT8 *mac_value;         //< MAC算法数据

        UINT32 pin_mode;          //< 加密模式
        UINT32 pin_len;           //< 密码长度
        UINT8 *card_no;           //< 卡号

        UINT32 encrypt_mode;      //< 加密/解密
        UINT32 algorithm_mode;    //< 算法模式

        UINT8 *init_vector;       //< 初始向量

        UINT8 *data;              //< 加密的数据
    };

    /**
     *  签到参数结构体
    */
    struct SignInParam
    {
        UINT8 *org_id;            //< 机构号
        UINT8 *term_id;           //< 终端号
    };

    /**
     *  时间段结构体
    */
    struct TimePeriod
    {
        std::string time_begin;   //< 开始时间
        std::string time_end;     //< 结束时间
    };

    struct UserData
    {
        int  build_fini;  //< 标识语法构建是否完成
        int  update_fini; //< 标识更新词典是否完成
        int  error_code; //< 记录语法构建或更新词典回调错误码
        char grammar_id[32]; //< 保存语法构建返回的语法ID
    };

    /**
     *  HTTP服务请求结构体
    */
    struct HttpService
    {
        UINT32 time_out;          //< 超时
        std::string url;          //< 默认url地址
        std::string content;      //< http请求内容
        std::string key;          //< http请求对应key值
        std::string action;       //< http请求方式
        std::string response;     //< http应答
    };

    //////////////////////////////////////////////////////////////////////////
    struct TDPoint
    {
        float x;
        float y;
    };
    // 头部姿态
    struct TDHeadPose
    {
        float pitch;		// 抬头、低头,范围-90到90，越大表示越抬头
        float yaw;			// 左右转头
        float roll;			// 平面内偏头
    };

    // 人脸质量
    struct TDFaceQuality
    {
        float faceScore;    // 总分，0~1.0之间，越大则人脸质量越好.
        float brightness;   // 亮度 亮度，越大表示越亮，推荐范围0.2-0.8
        float clearness;    // 清晰度 清晰度，越大表示越清晰，推荐范围0.65-1.0
        //float symmetry;     // 对称性
        float glassness;    // 眼镜: 返回值0~1，值越大，越有可能没戴眼镜.
        float skiness;     // 肤色：返回值0~1, 肤色面积占人脸面积的比例 或肤色接近真人肤色程度，越大表示越真实，推荐范围0.5-1.0
        float mouthness;    // 嘴部: 自行设定阈值Thres，约在(0,1)区间，大于Thres为闭嘴.越大表示越可能张嘴，推荐范围0.0-0.5
        float eyeLeft;      // 左眼
        float eyeRight;     // 右眼
        float eye_block;    // 眼睛被遮挡的置信度，越大表示眼镜越可能被遮挡，推荐范围0.0-0.5
        // 这个值当前一直为空，从云从那拿不到，可改用glassness参数
        float extra_scores[8]; /* 质量分分数项，具体含义（根据数据下标顺序）:
													* 0 - 人脸角度，左转为正，右转为负
                          * 1 - 左右转程度，越大表示角度越正，推荐范围0.5-1.0
													*2 - 人脸角度，抬头为正，低头为负
													* 3 - 人脸角度，顺时针为正，逆时针为负
													* 4 - 抬低头程度，越大表示角度越正,推荐范围0.5-1.0
                          * 5~7 备用
													*/
    } ;
    struct TDKeyptPoints
    {
        TDPoint	  points[68];        // 关键点
        int				nkeypt;											// 关键点个数
        float			keyptScore;										// 关键点得分
    };

    struct TDAlignedFace
    {
        char   data[128 * 128];		     // 图像数据
        int    width;											 // 宽
        int    height;											 // 高
        int    nChannels;										 // 图像通道
    };


    // 人脸综合结构
    struct TDFaceDetectInfoDetail
    {
        TDKeyptPoints		     keypt;
        TDHeadPose       headPose;			// 头部姿态
        //cw_aligned_face_t	 faceAligned;		// 对齐人脸
        TDFaceQuality	     quality;			// 人脸质量
        TDAlignedFace	 face_align;		// 对齐人脸
        boost::shared_array<char> face_feature_field_arr;
        boost::shared_array<char> face_feature_prob_arr;


        TDFaceDetectInfoDetail() {
            face_feature_field_arr = NULL;
            face_feature_prob_arr = NULL;
        }
    };
    typedef boost::shared_ptr<TDFaceDetectInfoDetail> TDFaceDetectInfoDetailPtr;
    //////////////////////////////////////////////////////////////////////////
    struct FaceDetectRequest
    {
        // 设备号
        std::string terminal_id;
        //void* cvmat;           ///< a point to a cv mat;
        int frame_widht;
        int frame_height;
        int frame_channels;
        //int frame_type;
        void* frame_data;

        bool get_face_property;
        bool get_face_property_ex;
        bool get_face_feature;
        bool get_face_image_base64;
        FaceDetectRequest() {
            memset ( this, 0, sizeof ( FaceDetectRequest ) );
        }
    };

    typedef FaceDetectRequest  TDFaceDetectRequest;
    typedef boost::shared_ptr<FaceDetectRequest> FaceDetectRequestPtr;

    /**
      *@brief 人脸识别结果,版本1
      */
    typedef struct FaceDetectInfo
    {

        int trackid;// 人脸ID
        int x;//r人脸坐标
        int y;
        int width;
        int hight;
        float face_quality_score; //质量分数
        //unsigned char* szImgBase64Data;		//人脸图片数据
        //int nImgDataLength;
        std::string img_base64_data;

        std::string gender;  ///< 性别， -1 表示女性，1表示男性
        int age;
        std::string face_feature;   ///< 老版的face feature

        float horizontal_angle;  ///< 水平角度，以摄像头为视角，水平中间为原点，左边为正数角度，右边为负数角度
        float vertical_angle;    ///< 垂直角度，以摄像头为视频，垂直中间为原点，上面为正数角度，下边负正数角度

        float horizontal_ratio;   ///< (待废除）水平方向，人脸所占视角的大小，可用此参数来判断人脸离摄像头的远近。
        float vertical_ratio;    ///< (待废除）垂直方向，人脸所占视角的大小，
        float estimate_distance;   ///< 估计的人脸的距离

        int serial_num;           ///< 按序号的人脸ID. 根trackid的区别是，trackid是跟踪的人脸ID，是递增的，而serial_num是当前视频祯里人脸的按序的编号，比如有3个人，则按序有1，2，3的编号。

        int faceOrient;
        /// 兼容离线版云从 wff add 20170720
        TDFaceDetectInfoDetailPtr face_detect_detail_ptr;
        std::vector<char> vec_face_feature;
        std::vector<char> vec_face_prob_feature;

        FaceDetectInfo() {
            trackid = 0;
            x = 0;
            y = 0;
            width = 0;
            hight = 0;
            face_quality_score = 0;
            age = 0;
            face_feature = "";
            horizontal_angle = 0;
            vertical_angle = 0;
            horizontal_ratio = 0;
            vertical_ratio = 0;
            estimate_distance = 0;
            serial_num = 0;
            face_detect_detail_ptr = NULL;
        }

    } TDFaceDetectInfo;
    //typedef FaceDetectInfo TDFaceDetectInfo;
    typedef boost::shared_ptr<FaceDetectInfo> TDFaceDetectInfoPtr;
    typedef boost::shared_array<FaceDetectInfo> TDFaceDetectInfoArray;

    struct FaceDetectInfoGroup
    {
        int face_number;
        TDFaceDetectInfoArray arr_face_info;

        FaceDetectInfoGroup() {
            face_number = 0;
            arr_face_info = NULL;
        }

    };

    /**
      *@brief 人脸1比1比对请求
      */
    //struct FaceComparison11RequestV1 {
    //  char terminalid_buf[128];
    //  char orgind_buf[128];
    //  char facefeature_a_buf[1024*6];
    //  UINT32 facefeatue_datalength_a;
    //  char facefeature_b_buf[1024*6];
    //  UINT32 facefeatue_datalength_b;
    //  FaceComparison11RequestV1() {
    //    memset(this, 0, sizeof(FaceComparison11RequestV1));
    //  }
    //};

    struct FaceComparison11Request
    {
        std::string terminalid;
        std::string orgind;
        std::string facefeature_a;
        std::string facefeature_b;

        /// 为了扩展，添加两个字段，表示a数据的类型，数据类型有以下几种。(默认是0）
        /// 0 人脸特征（云从在线版）
        /// 1 图片地址
        /// 2 图片Base64
        /// 3 人脸prob特征的base64格式（云从离线版），此种格式数据只支持第1个人脸数据，不支持第2个脸脸数据。
        /// 4 人脸field特征base64格式（云从离线版）
        int data_a_type;
        int data_b_type;
        FaceComparison11Request() {
            memset ( this, 0, sizeof ( FaceComparison11Request ) );
        }
    };


    typedef boost::shared_ptr<FaceComparison11Request> FaceComparison11RequestPtr;
    /**
      *@brief 人脸1比1的比对结果,版本V1
      */
    struct FaceComparisonResult11
    {
        float score;   ///< 相似度，默认是0
        FaceComparisonResult11() {
            score = 0.0;
        }
    };
    typedef boost::shared_ptr<FaceComparisonResult11> FaceComparisonResult11Ptr;

    /**
    *@brief 人脸1比n的比对请求
    */
    //struct FaceComparision1nRequestV1 {
    //  char terminalid_buf[128];
    //  char orgid_buf[128];
    //  char facefeature_buf[1024*6];
    //  UINT32  facefeature_data_length;
    //  UINT32 topn;
    //};
    struct FaceComparision1nRequest
    {
        std::string terminalid;
        std::string orgid;
        std::string facefeature;
        UINT32 topn;

        /// 为了扩展，添加两个字段，表示a数据的类型，数据类型有以下几种。(默认是0）
        /// 0 人脸特征
        /// 1 图片地址
        /// 2 图片Base64
        /// 3 人脸prob特征的base64格式（云从离线版），此种格式数据只支持第1个人脸数据，不支持第2个脸脸数据。
        /// 4 人脸field特征base64格式（云从离线版）
        int data_type;

        FaceComparision1nRequest() {
            topn = 0;
            data_type = 0;
        }
    };
    typedef boost::shared_ptr<FaceComparision1nRequest> FaceComparision1nRequestPtr;


    /**
    *@brief 人脸1比n的比对结果， 版本V1
    */
    struct FaceCompareResult
    {
        //float score;   ///< 相似度
        //std::string person_id;  ///< 个人ID
        //UINT16 sex;   ///< ：-1  女， 1 男,性别Ｍ　Ｆ
        //// std::string lagl;   ///< ：是否存在（暂不用）
        //UINT16 age;    ///< ：年龄
        ////std::string smile;   ///< ：微笑度（暂不用）
        //std::string id_card;   ///< ：证件号
        //std::string person_name;   ///< ：个人姓名
        //std::string mobile;    ///< ：手机号
        std::string score;   ///< 相似度
        std::string person_id;  ///< 个人ID
        std::string sex;   ///< ：性别Ｍ　Ｆ
        // std::string lagl;   ///< ：是否存在（暂不用）
        std::string age;    ///< ：年龄
        std::string smile;   ///< ：微笑度（暂不用）
        std::string id_card;   ///< ：证件号
        std::string person_name;   ///< ：个人姓名
        std::string mobile;    ///< ：手机号
        std::string is_admin;  // 是否管理员 0 非，1 是
        std::string type; // 用户类型

        FaceCompareResult() {
            memset ( this, 0, sizeof ( FaceCompareResult ) );
        }
    };
    typedef FaceCompareResult TDFaceCompareResult;

    // struct FaceCompare1nResultV1 {
    //   float score;   ///< 相似度
    //   char person_id_buf[128];  ///< 个人ID
    //   UINT16 sex;   ///< ：-1  女， 1 男,性别Ｍ　Ｆ
    //   // std::string lagl;   ///< ：是否存在（暂不用）
    //   UINT16 age;    ///< ：年龄
    //   //std::string smile;   ///< ：微笑度（暂不用）
    //   char id_card_buf[128];   ///< ：证件号
    //   char person_name_buf[256];   ///< ：个人姓名
    //   char mobile_buf[128];    ///< ：手机号
    //
    //   FaceCompare1nResultV1() {
    //     memset(this, 0, sizeof(FaceCompare1nResultV1));
    //   }
    // };



    typedef boost::shared_ptr<FaceCompareResult> FaceCompareResultPtr;


    struct FaceComparisonResult1N
    {
        std::string group_id;  ///< 所属组ID 不为空
        // boost::shared_array<FaceComparisonResult> faces;
        std::vector<FaceCompareResultPtr>  vec_faces;

        int Clone ( FaceComparisonResult1N& obejct ) {
            obejct.group_id = group_id;
            int vec_size = vec_faces.size();
            obejct.vec_faces.clear();
            for ( int i = 0; i < vec_size; ++i ) {
                /*FaceComparisonResultPtr*/FaceCompareResultPtr data_ptr;
                if ( vec_faces[i] ) {
                    data_ptr = /*FaceComparisonResultPtr*/FaceCompareResultPtr ( new /*FaceCompareResult*/FaceCompareResult );
                    if ( NULL == data_ptr ) {
                        return YSOS_ERROR_FAILED;
                    }
                    *data_ptr = *vec_faces[i];
                }
                obejct.vec_faces.push_back ( data_ptr );
            }
            return YSOS_ERROR_SUCCESS;
        }
    };
    typedef FaceComparisonResult1N TDFaceComparisonResult1N;
    typedef boost::shared_ptr<FaceComparisonResult1N> FaceComparisonResult1NPtr;


    /**
      *@brief 人脸注册请求
      */
    struct FaceRegisterRequest
    {
        std::string terminalid;
        std::string orgid;
        char facebase64_buf[1092 * 1080 * 6]; ///< 人脸base64数据,最大可存储数据
        UINT32 facebase64_length;

        // std::string group_id;  ///< 所属组ID
        //std::string person_id;  ///<个人I
        UINT16 sex;  ///< 性别Ｍ　Ｆ
        UINT16 age;  ///< 所属组ID
        std::string simle;  ///< 响应信息
        std::string idcard;  ///< 身份证号
        std::string person_name;  ///< 个人姓名
        std::string mobile;  ///< 移动电话
        std::string address;   ///< 表示地址

        FaceRegisterRequest() {
            //memset(this, 0,sizeof(FaceRegisterRequestV1));
        }
    };
    typedef boost::shared_ptr<FaceRegisterRequest> FaceRegisterRequestPtr;


    /**
     *@brief 人脸注册的结果
     *@param
     *@return
     */
    struct FaceRegisterResult
    {
        std::string has_similar; // 相似标志 "true"存在相似，"false"不存在
        std::string group_id;    // 所属组ID
        std::string person_id;   // 个人I
        std::string sex;         // 性别 Ｍ/Ｆ
        std::string age;         // 所属组ID
        std::string simle;       // 响应信息
        std::string idcard;      // 身份证号
        std::string person_name; // 个人姓名
        std::string mobile;      // 移动电话
    };
    typedef FaceRegisterResult TDFaceRegisterResult;
    typedef boost::shared_ptr<FaceRegisterResult> FadeRegisterResultPtr;


    /**
     *@brief 最佳人脸对象
    **/
    struct TDFaceDetectUnit
    {
        TDFaceDetectInfoPtr face_detect_info_ptr_;  ///< 模糊人脸信息
        int have_perfect_face_;  ///< 是否有最佳人脸，默认是0 表示未知， -1 表示没有， 1表示有, 5 表示在查找中
        FaceComparisonResult1NPtr perfect_face_1n_ptr_;

        TDFaceDetectUnit() {
            ClearContent();
        }

        int Clone ( TDFaceDetectUnit& object ) {
            if ( face_detect_info_ptr_ ) {
                object.face_detect_info_ptr_ = TDFaceDetectInfoPtr ( new FaceDetectInfo );
                if ( NULL == object.face_detect_info_ptr_ ) {
                    return YSOS_ERROR_FAILED;
                }
                *object.face_detect_info_ptr_ = *face_detect_info_ptr_;
            }
            else {
                object.face_detect_info_ptr_ = NULL;
            }
            object.have_perfect_face_ = have_perfect_face_;
            if ( perfect_face_1n_ptr_ ) {
                object.perfect_face_1n_ptr_ = FaceComparisonResult1NPtr ( new FaceComparisonResult1N );
                if ( NULL == object.perfect_face_1n_ptr_ ) {
                    return YSOS_ERROR_FAILED;
                }
                if ( YSOS_ERROR_SUCCESS != perfect_face_1n_ptr_->Clone ( *object.perfect_face_1n_ptr_ ) ) {
                    return YSOS_ERROR_FAILED;
                }
            }
            else {
                object.perfect_face_1n_ptr_ = NULL;
            }
            return YSOS_ERROR_SUCCESS;
        }

        void ClearContent() {
            face_detect_info_ptr_ = NULL;
            //perfect_face_ptr_ = NULL;
            perfect_face_1n_ptr_ = NULL;
            have_perfect_face_ = 0;
        }
    };


    /**
     *@brief 当前摄像头拍照请求
     * 默认支持的格式

     */
    struct VideoTakeAPictureRequestV1
    {
        UINT32 width;  ///< 图片的长宽，如果小于或等于0，则表示不设置
        UINT32 hegiht;   ///<
        std::string img_fromat_extension;  ///<  图片格式扩展名，比如".jpg",".png"
        UINT8 base64_convert;   ///<  是否需要进行base64转码，如果需要则为1，否则为0
        UINT8 image_quality;   ///<  图片质量，比1到100（值越高，图片质量越好 压缩率越低，文件大小越大）， 0 表示使用默认值, 预留字段，暂时不可用，只使用默认值
    };

    /**
     *@brief 当前视频拍照结果
     */
    struct VideoTakeAPictureResultV1
    {
        VideoTakeAPictureRequestV1 source_request;
        //UINT8 have_a_pictue;   ///< 是否有结果
        // char picture_data_buf[1092*1080*6];
        // char picture_data_length;
        std::vector<unsigned char> vec_picture_data;   ///< 如果是图片数据（没有经过base64转换），则结果存在在这里面
        //std::string
    };

    /**
      *@brief 数据转换说明, 对应于module xml中配置 datatype_convert
      */
    struct ModuleDataTypeConvertInfo
    {
        std::string source_input_datatype;  ///< callback输入格式
        std::string source_output_datatype;  ///< callback 输出格式
        std::string dest_input_datatype;  ///< 转换后输入的格式
        std::string dest_output_datatype;  ///< 转换后的输出格式
    };


    ////////////////////////////底盘开始///////////////////////////////////
    /**
    *@brief 加载地图
    */
    struct ChassisLoadMap
    {
        bool binitdirect;         ///< 是否转圈
        char szmapname[64];       ///< 地图名称
        char szinitpointname[64]; ///< 初始点
    };

    /**
    *@brief 底盘版本信息
    */
    struct ChassisVersion
    {
        char szversionsdp[32];
        char szversionsdk[32];
        char szmoduletype[32];
        char szproductId[64];
    };

    /**
    *@brief 底盘位置坐标
    */
    struct ChassisPosition
    {
        float fposx;
        float fposy;
        float fposangle;
    };

    /**
    *@brief 底盘移动速度
    */
    struct ChassisMoveSpeed
    {
        float flinearspeed;
        float fangularspeed;
        int   iLevel;
    };

    /**
    *@brief 检查障碍物距离角度
      */
    struct ChassisObstacleDetection
    {
        float fobstacledetectiondistance;
        float fobstacledetectionangle;
    };

    /**
    *@brief 检查障碍物结果
    */
    struct ChassisObstacleDetectionResult
    {
        bool bHaveObstacle;
        float fobstacleavgdistance;
    };


    /**
    *@brief 底盘状态
    */
    struct ChassisDeviceStatus
    {
        int   ibattery;
        bool  bcharge;
        bool  bemergencystop;
        float fspeed;
        float fmillage;
    };

    /**
    *@brief 底盘健康状态
    */
    struct ChassisHealthStatus
    {
        int   imcuconnetstatus;
        int   ilaserconnetstatus;
        int   irouterconnetstatus;
    };


    /**
    *@brief 底盘配置信息，防撞条和超声信息
    */
    struct ChassisConfigStatus
    {
        char   szprotector[16];
        char   szultrasonic[16];
    };

    /**
    *@brief 底盘传感器信息
    */
    struct ChassisSensorInfo
    {
        char   szprotector[256];      /// 防碰撞数据
        char   szultrasonic[256];     /// 声呐数据
        char   szlaser[2048];         /// 激光数据
        char   szrealsenser[256];     /// 激光数据
    };


    /**
    *@brief 增加预定义点/路径/扫地图
    */
    enum TypeID
    {
        PROP_TYPE_MAP      = 0x00,
        PROP_TYPE_POSITION = 0x01,
        PROP_TYPE_PATH     = 0x02,
        PROP_TYPE_MAP_SCAN = 0x03,
    };

    enum MethodID
    {
        PROP_METHOD_GET    = 0x00,
        PROP_METHOD_ADD    = 0x01,
        PROP_METHOD_DEL    = 0x02,
        PROP_METHOD_START  = 0x03,
        PROP_METHOD_STOP   = 0x04,
        PROP_METHOD_CANCEL = 0x05,
        PROP_METHOD_RENAME = 0x06,
    };

    struct ChassisPredefine
    {
        int  itypeid;                 ///< 类型id,见enum TypeID        //NOLINK
        int  imethod;                 ///< 方法，enum MethodID         //NOLINK
        char szmapname[64];           ///< 地图名称                    //NOLINK
        char szpositionname[64];      ///< 位置名称                    //NOLINK
        char szname[64];              ///< 重命名时新名称              //NOLINK
        char szoriginname[64];        ///< 重命名时原名称              //NOLINK
    };

    /**
    *@brief 底盘开始录制路径, 手动控制机器人移动或者按位置名导航
    */
    struct ChassisPath
    {
        char szmapname[64];
        char szpathname[64];
    };


    /**
    *@brief 底盘障碍物分类信息
    */
    struct ObstacleInfo
    {
        double avg_obstacle_distance;  ///< 障碍物的平均距离,单位为米
        double obstacle_size;          ///< 障碍物的尺寸, 单位为米
        double angle_front;            ///< 相对于机器人的障碍物的角度
        double angle_paranomic;        ///< 相对于世界坐标系的障碍物的角度
        int    obstacle_type;          ///< 障碍物的类型
        bool   active_or_not_in_map;   ///< 是否是活动障碍物
        ChassisPosition dest_location; ///< 障碍物的坐标
    };

    /**
    *@brief 底盘最近障碍物信息，通过激光获取
    */
    struct NearestObstacleInfo
    {
        double obstacle_distance;  ///< 障碍物的平均距离,单位为米
        double obstacle_angle;     ///< 障碍物的角度
    };


    /**
    *@brief 底盘Callback 输出
    */
    struct ChassisOutPut
    {
        bool   bheart_lost;
        int    icurise_mode;
        char   sznavigation_status[128];
        struct ChassisPosition      struposition;
        struct ChassisMoveSpeed     strumovespeed;
        struct ChassisDeviceStatus  strudevstatus;
        struct ChassisConfigStatus  struconfigstatus;
        struct ChassisHealthStatus  struhealthstatus;
        struct NearestObstacleInfo  strunearestobstacleinfo;
        struct ObstacleInfo         struObstacleinfo[32];
    };


    ////////////////////////////底盘结束///////////////////////////////////


    //////////////////////////机械手臂开始/////////////////////////////////
    /**
     *	机械手臂初始化结构体
    */
    struct MechanicalArmInit
    {
        UINT32 arm_port;         //< 串口号
        UINT32 arm_baudrate;     //< 波特率
    };

    /**
    *@brief 手臂舵机角度，Module输出
    */
    struct Angle
    {
        UINT32 gear_num;				            //< 手臂舵机数量
        std::vector<int> u2angle;				//< 手臂舵机各自的角度
    };

    /**
    *@brief 机械手臂信息
    */
    struct ArmPosition
    {
        std::vector<int> u2angle;			//< 手臂舵机各自的角度
        UINT32 deley_time;				        //< 一个舵机两次旋转的时延
    };

    /**
    *@brief 手臂动作分解手势
    */
    struct ResolveArmCommon
    {
        ArmPosition final_position;					      //< 手臂动作最终角度
        ArmPosition arm_position;		              //< 手臂动作分解角度
    };

    typedef ResolveArmCommon	ResolveArmEnter,	    //< 手臂进入动作分解手势
               ResolveArmLeave,	                      //< 手臂离开动作分解手势
               ResolveArmHandShake,                 	//< 手臂握手动作分解手势
               ResolveArmTouch,	                      //< 手臂摸后脑勺动作分解手势
               ResolveArmPlease,	                    //< 手臂请进动作分解手势
               ResolveArmQuietness;	                  //< 手臂请进动作分解手势

    //////////////////////////机械手臂结束/////////////////////////////////

    //////////////////////////头部开始/////////////////////////////////////
    /**
     *	头部初始化结构体
    */
    struct HeadInit
    {
        UINT32 arm_port;         //< 串口号
        UINT32 arm_baudrate;     //< 波特率
    };

    //////////////////////////头部结束/////////////////////////////////////

    //////////////////////////百度机器人手臂开始/////////////////////////////////////
    /**
     *	百度机器人手臂初始化结构体
    */
    struct BaiduInit
    {
        UINT32 arm_port;         //< 上半身控制板串口号
        UINT32 chassis_port;     //< 底盘控制板串口号
        UINT32 arm_baudrate;     //< 波特率
    };

    //////////////////////////百度机器人手臂开始/////////////////////////////////////

    //////////////////////////t10em2读卡器开始/////////////////////////////////////
    /**
     *	t10em2读卡器初始化结构体
    */
    struct T10em2Init
    {
        UINT32 card_port;         //< 串口号
        UINT32 card_baudrate;     //< 波特率
    };

    //////////////////////////t10em2读卡器开始/////////////////////////////////////

    //////////////////////////金融业务开始/////////////////////////////////
    /**
     *@brief 金融定制配置文件中的Atttribute属性
     */
    struct FinancialAttributeInfo
    {
        std::string name;         ///< 属性名称
        std::string type;         ///< 属性类别
        std::string value;        ///< 属性值
        std::string default_str;  ///< 默认值
        std::string format;       ///< 格式化的Protocol名
        std::string row_no;       /// 行值
        std::string col_no;       ///< 列值
        std::string length;       ///< 允许的长度范围
        std::string log_format;   ///< 日志的输出格式
    };
    typedef boost::shared_ptr<FinancialAttributeInfo>  FinancialAttributeInfoPtr;
    /**
     *@brief 金融定制配置文件信息
     */
    struct FinancialProcessInfo
    {
        std::string content_type;    ///< 类别
        std::string description;     ///< 详细描述
        std::string encoding;        ///< 编码类型
        bool is_pin_block_check;     ///< 是否是密码类型
        bool is_uc;                  ///< 是否是UC
        int log_switch;              ///< log的级别
        std::string pid;             ///< 配置文件的ID值
        std::string head;            ///< 配置文件中的Head信息
        std::string request;        ///< 配置文件中的request信息
        std::string response;        ///< 配置文件中的response信息
        std::string request_end_function;  ///< request的信息尾部处理Protocol名称
        std::string response_head_function;  ///< response的信息头处理Protocol名称
        //std::list<FinancialAttributeInfoPtr > attribute_list; ///< 配置文件中的属性信息
    };
    typedef boost::shared_ptr<FinancialProcessInfo>  FinancialProcessInfoPtr;
    //////////////////////////金融业务结束/////////////////////////////////
}

#endif
