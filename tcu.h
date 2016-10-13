/*
 * TCU 通信数据包定义
 * 版本 V 1.0
 * 遵循标准： 《计费控制单元与充电控制器通信协议》  SAE J1939-21
 */
#ifndef _TCU_PACKAGE_INCLUDED_H_
#define _TCU_PACKAGE_INCLUDED_H_

#ifdef __cplusplus
extern "C" {
#endif

// 充电机地址
#define CAN_ADDR_CHARGER   0xF6  // 242
// TCU地址
#define CAN_ADDR_TCU       0x8A  // 138

#define CAN_TX_ID_MASK    ((CAN_ADDR_CHARGER<<8)|CAN_ADDR_TCU) //发送  F68A
#define CAN_RCV_ID_MASK   ((CAN_ADDR_CHARGER)|CAN_ADDR_TCU<<8)//接收  8AF6

#ifndef u8
#define u8 unsigned char
#endif
#ifndef u16
#define u16 unsigned short
#endif
#ifndef u32
#define u32 unsigned int
#endif


// 无效时戳，初始化时默认赋值， 用于time_t默认值
#define INVALID_TIMESTAMP  0x00000000
// TCU 通信时的缓冲区
#define CAN_BUFF_SIZE         1024


#pragma pack(1)


// TCU与Charging交互阶段定义
typedef enum {
    TCU_STAGE_INVALID               =0x00,     // TCU状态无效，可能是在初始化或者析构
    TCU_STAGE_CHECKVER      	=0x01,//版本校验
    TCU_STAGE_PARAMETER   	=0x02,//下发参数
    TCU_STAGE_WAITCONNECT     	=0x03,//等待连接确认
    TCU_STAGE_CONNECT            =0x04,//连接确认应答
    TCU_STAGE_WAITSTART         =0x05,// 等待启动充电
    TCU_STAGE_START                   =0X06,// 启动充电
    TCU_STAGE_STARTING            =0X07,//启动充电...
    TCU_STAGE_STATUS                 =0X08,//启动状态完成ｏｒ失败
    TCU_STAGE_CHARGING          =0X09,//充电中...
    TCU_STAGE_STOP                     =0X0A,//停止充电
    TCU_STAGE_STOP_STATUS    =0X0B,//停止充电...
    TCU_STAGE_STOP_END          =0X0C,//停止充电完成
    TCU_STAGE_HEAT                     =0X0D,//心跳
    TCU_STAGE_TIME					     =0x0E,//对时
    TCU_STAGE_ANY        			     =0x0F // 任意阶段
}TCU_STAGE;

typedef enum {
    TCU_ERR_STAGE_INVALID      		=0x00,     // TCU状态无效，可能是在初始化或者析构
    TCU_ERR_STAGE_CHECKVER      	=0x01,//版本校验
    TCU_ERR_STAGE_PARAMETER   	=0x02,//下发参数
    TCU_ERR_STAGE_WAITCONNECT     	=0x03,//等待连接确认
    TCU_ERR_STAGE_CONNECT     	=0x04,//连接确认应答
    TCU_ERR_STAGE_WAITSTART      =0x05,// 等待启动充电
    TCU_ERR_STAGE_START         		=0X06,// 启动充电
    TCU_ERR_STAGE_STARTING         =0X07,//启动充电...
    TCU_ERR_STAGE_STATUS              =0X08,//启动状态完成ｏｒ失败
    TCU_ERR_STAGE_CHARGING        =0X09,//充电中...
    TCU_ERR_STAGE_STOP      			    =0X0A,//停止充电
    TCU_ERR_STAGE_STOP_STATUS =0X0B,//停止充电...
    TCU_ERR_STAGE_STOP_END       =0x0C,//停止充电完成
    TCU_ERR_STAGE_HEAT					=0X0D,//心跳
    TCU_ERR_STAGE_TIME					=0X0E,//对时
    TCU_ERR_STAGE_ANY        			    =0x0F // 任意阶段
}TCU_ERR_STAGE;

// TCU 状态定义
typedef enum {
    CC_WAIT          =0x00,
    CC_WORK              =0x01,
    CC_FULL                 =0x02,
    CC_WARN              =0x03,
    CC_ERROR             =0x04
}CC_STATUS;

// TCU CAN通信状态定义
typedef enum {
    // 无效模式
    CAN_INVALID               =0x00,
    // 普通模式
    CAN_NORMAL                =0x01,
    // 连接管理读模式, 多数据包读
    CAN_TP_RD                 =0x02,
    // 连接管理写模式，多数据包写，当前的协议标准没有用到
    CAN_TP_WR                 =0x04
}CAN_TCU_STATUS;

// CAN 传输链接状态
typedef enum {
    // 链接请求发送
    CAN_TP_RTS = 0x10,
    // 链接准备发送
    CAN_TP_CTS = 0x20,
    // 数据发送中
    CAN_TP_TX  = 0x30,
    // 数据接收中
    CAN_TP_RX  = 0x40,
    // 链接发送完成
    CAN_TP_ACK = 0x50,
    // 链接发送中止
    CAN_TP_ABRT= 0x60
}CAN_TP_STATUS;

struct can_pack_generator;
// 通信报文生成依据
struct can_pack_generator {
    // 所属阶段
   TCU_STAGE stage;
    // 生成PGN
    unsigned int pgn;
    // 数据包优先级
    unsigned int prioriy;
    // 数据包长度
    unsigned int datalen;
    // 数据包发送周期
    unsigned int period;
    // 心跳计数
    unsigned int heartbeat;
    // 数据包名称
    const char *mnemonic;
};



struct pgn256_TRC{//启动充电
		u8 spn256_port;//0-255
		u8 spn256_load_control;//负荷控制开关 1启用，2关闭，其他无效  根据用户类型提供不同功率输出
};
struct pgn512_CRRC{//应答启动充电
		u8 spn512_port;//0-255
		u8 spn512_load_control;//负荷控制开关 1启用，2关闭，其他无效  根据用户类型提供不同功率输出
		u8 spn512_status;//0成功；1失败
};

struct pgn768_TST{//停止充电
		u8 spn768_port;//0-255
		u8 spn768_status;//0x01：计费控制单元正常停止 		0x02: 计费控制单元故障终止
};

struct pgn1024_CRST{//应答停止充电
		u8 spn1024_port;//0-255
		u8 spn1024_status;//0成功；1失败
};

struct pgn1280_TTS{//下发对时
		u8 spn1280_Immediately;//0立即执行　１自行选择时间更新
		// 压缩的BCD码
		u8 spn1280_bcd_sec_l;
	    u8 spn1280_bcd_sec_h;
	    u8 spn1280_bcd_min;
	    u8 spn1280_bcd_hour;
	    u8 spn1280_bcd_day;
	    u8 spn1280_bcd_mon;
	    u8 spn1280_bcd_year;
};

struct pgn1536_CRTS{//应答对时
		u8 spn1536_Immediately;//0立即执行　１自行选择时间更新
		u8 spn1536_confirm;//0-对时确认 1-对时拒绝
};


struct pgn1792_TCV{//版本校验
	 // TCU 通信协议版本号, 12.10
	    unsigned char spn_tcu_version[2];
};
struct pgn2048_CRCV{//版本确认
	 // Charging 通信协议版本号, 12.10
	    unsigned char spn_charging_version[2];
};
struct pgn2304_TCP{//下发参数
	   // 充电机编号， 范围0-oxFFFFFFFF
	    u8 spn2304_charger_sn[4];
	    // 充电机/充电站所在区域编码 ASCII  3BYTE
	    u8 spn2304_charger_region_code[3];
};
struct pgn2560_CRCP{//应答参数
		u8 spn2560_charger_status;//0 成功　1 失败
};

struct pgn4352_CSF{//Charging启动完成状态信息
		u8 spn4352_port;//0-255
		u8 spn4352_load_control;//负荷控制开关 1启用，2关闭，其他无效  根据用户类型提供不同功率输出
		u8 spn4352_status;//0成功；1失败
};
struct pgn4608_TRSF{//TCU应答启动完成
		u8 spn4608_port;//0-255
		u8 spn4608_load_control;//负荷控制开关 1启用，2关闭，其他无效  根据用户类型提供不同功率输出
		u8 spn4608_status;//0成功；1失败
};
struct pgn4864_CST{//Charging停止充电完成状态信息
		u8 spn4864_port;//0-255
		u8 spn4864_Stop_reason;//停止原因 Stop reason
		u8 spn4864_status;//0成功；1失败
};
struct pgn5120_TRST{//TCU应答停止完成
		u8 spn5120_port;//0-255
		u8 spn5120_Stop_reason;//停止原因 Stop reason
		u8 spn5120_status;//0成功；1失败
};
struct pgn5376_CCT{//Charging连接确认
		u8 spn5376_port;//0-255
};
struct pgn5632_TRCT{//TCU应答连接确认
		u8 spn5632_port;//0-255
		u8 spn5632_status;//0成功；1失败
};
struct pgn8448_CRF{//Charging遥信帧
		u8 spn8448_port;//0-255
        char spn8448_status[4];//工作状态  0000-待机0001-工作 0010-充满 0011-告警0100故障
		u8 connect_confirm_switch_status;//连接确认开关状态 布尔型, 0连接，1未连接
		u8 emergency_stop;//急停动作告警  布尔型, 0正常，1异常
		u8 arrester_fault;//避雷器故障  布尔型, 0正常，1异常
		u8 gun_fault;//充电枪未归位  布尔型, 0正常，1异常
		u8 Over_temperature_fault;//过温故障  布尔型, 0正常，1异常
		u8 In_vol_over;//输入电压过压  布尔型, 0正常，1异常
		u8 In_vol_under;//输入电压欠压  布尔型, 0正常，1异常
		u8 Out_contactor_status;//输出接触器状态  布尔型, 0分断，1闭合
		u8 Vehicle_control_guid_alarm;//充电中车辆控制导引告警  布尔型, 0正常，1异常
		u8 AC_contactor_fault;//交流接触器故障  布尔型, 0正常，1异常
		u8 Out_over_current;//输出过流告警  布尔型, 0正常，1保护
		u8 Out_over_current_protection_action;//输出过流保护动作  布尔型, 0正常，1异常
		u8 Other_faults[2];//其它类型故障  充电机私有告警和故障信息，第1个八位组为告警信息编号，第2个八位组为告警信息值（布尔型, 变化上传；0正常，1异常）
};
struct pgn8704_CTF{//Charging遥测帧
		u8 spn8704_port;//0-255
        u8  spn8704_out_vol[2];//输出电压    精确到小数点后1位0V - 450V
        u8  spn8704_out_cur[2];//输出电流   精确到小数点后2位0A - 70A
        u8  spn8704_guid_vol[2];//导引电压  精确到小数点后2位0A - 12V
};
struct pgn12544_THB{//TCU心跳帧
		u8 spn12544_port;//0-255
		u8 spn12544_status;//计费控制单元状态信息 0-正常 1-故障
		u8 spn12544_rev;//保留
		u8 spn12544_ele[2];//数据分辨率：0.1 kWh/位，0 kWh偏移量；数据范围：0~1000 kWh;	（待机过程中此数据项为0）
		u8 spn12544_time[2];//数据分辨率：1 min/位，0 min偏移量；数据范围：0~6000 min；		（待机过程中此数据项为0）

};

#pragma pack()

typedef enum {
    PGN_TRC = 0x000100,//TCU启动Charging
    PGN_CRTR = 0x000200,//Charging应答TCU
    PGN_TST = 0x000300,//TCU停止充电
    PGN_CRST = 0x000400,//Charging应答TCU停止充电
    PGN_TTS = 0x000500,//TCU下发对时
    PGN_CRTS = 0x000600,//Charging应答对时
    PGN_TCV = 0x000700,//TCU校验版本    check-ver
    PGN_CRCV = 0x000800,//Charging 应答版本校验
    PGN_TCP = 0x000900,//TCU下发充电参数  change parameter
    PGN_CRCP = 0x000A00,//Charging 应答充电参数
    PGN_CSF = 0x001100,//Charging启动完成  start finish
    PGN_TRSF = 0x001200,//TCU应答启动完成
    PGN_CST = 0x001300,//Charging停止充电完成  stop
    PGN_TRST = 0x001400,//TCU应答停止完成
    PGN_CCT = 0x001500,//Charging连接确认 connect
    PGN_TRCT = 0x001600,//TCU应答连接确认
    PGN_CRF = 0x002100,//Charging遥信帧 remote frame
    PGN_CTF = 0x002200,//Charging遥测帧  telemetry frame
    PGN_THB = 0x003100//TCU心跳帧   heartbeat
}CAN_PGN;

#include "Hachiko.h"
#include "global.h"

/*
 * 读到一个完整数据包后调用该函数
 */
typedef enum {
    // 无效事件，初始化完成前收到该消息
    EVENT_INVALID    = 0,


    /* 数据包接受完成。系统中存在两个和CAN接口相关的线程，一个
     * 是CAN数据读取线程，还有一个是CAN数据发送线程，这两个线程
     * 对CAN的操作都是同步的，因此，在读取数据包时读线程会被阻塞
     * 因此需要通过该事件来通知系统，已经完成了数据包的读取。 */
    EVENT_RX_DONE    = 1,



    /* 数据包接收错误。在阻塞读取CAN数据包时，发生数据包读取错误，
     * 会收到该事件，表征数据包有问题，应该作相应处理。 */
    EVENT_RX_ERROR   = 2,



    /* 数据包发送请求。系统中CAN数据的发送线程是同步的，可以通过
     * 该事件来提醒系统，目前可以进行数据包的发送，若系统有数据包发送
     * 则设定发送数据，交由发送线程进行发送，若没有数据要发送，则返回一个
     * 不需要发送的结果即可。 */
    EVENT_TX_REQUEST = 3,



    /* 连接管理模式下的请求发送数据包，进行连接数据控制。 */
    EVENT_TX_TP_RTS = 4,



    /* 连接管理模式下的准备发送数据包，进行连接数据控制。 */
    EVENT_TX_TP_CTS = 5,



    /* 连接管理模式下的接收数据包完成应答，进行连接数据控制。  */
    EVENT_TX_TP_ACK = 6,



    /* 连接管理模式下的传输中止，进行连接数据控制。*/
    EVENT_TX_TP_ABRT= 7,


    /* 数据包准备发送。当EVENT_TX_REQUEST返回结果是需要发送时，经发送线程
     * 经发送线程确认后，将会受到该消息，表示发送线程已经准备发送该消息了，此时
     * 可以返回取消发送指令，实现数据包的取消发送。 */
    EVENT_TX_PRE     = 8,




    /* 数据包发送完成。当确认后的数据包发送完成后，将会受到该消息，表征数据包
     * 已经正确的发送完成。 */
    EVENT_TX_DONE    = 9,




    /* 数据包发送失败。当确认后的数据包发送失败后，将会受到改小。*/
    EVENT_TX_FAILS   = 10,



    // CAN 消息函数初始化。当第一次运行函数时将会收到该消息，可重复发送。
    EVENT_CAN_INIT   = 11,

    // CAN 消息复位。再次执行初始化操作。
    EVENT_CAN_RESET  = 12,
    EVENT_CAN_HEART = 13,
    EVENT_CAN_TIME  =  14
}EVENT_CAN;

// 事件通知返回/传入参数
typedef enum {
    // 无效
    EVT_RET_INVALID  = 0,
    // 一切OK
    EVT_RET_OK       = 1,
    // 失败了
    EVT_RET_ERR      = 2,
    // 超时
    EVT_RET_TIMEOUT  = 3,
    // 终止发送，EVENT_CAN.EVENT_TX_PRE的可用参数
    EVT_RET_TX_ABORT = 4
}EVT_PARAM;

// CAN 链接临时参数
struct can_tp_param {
    // 传输的数据包PGN
    unsigned int tp_pgn;
    // 即将传输的数据包大小
    unsigned int tp_size;
    // 即将传输的数据包个数
    unsigned int tp_pack_nr;

    // 已经接收的数据字节数
    unsigned int tp_rcv_bytes;
    // 已经接收的数据包个数
    unsigned int tp_rcv_pack_nr;
};

// 事件通知结构
struct event_struct {
    // 事件参数
    EVT_PARAM evt_param;

    union {
        // 发送缓冲区地址， 针对EVENT_TX_REQUEST设置
        unsigned char *tx_buff;
        // 接收缓冲区地址，针对EVENT_RX_DONE设置
        const unsigned char* rx_buff;
    }buff;
    // 缓冲区大小
    unsigned int buff_size;
    // CAN ID
    unsigned int can_id;
    // 缓冲区载荷大小
    unsigned int buff_payload;
};

// TCU通信统计技术结构
struct tcu_statistics {
    // 数据包PGN
    CAN_PGN can_pgn;
    // 数据包静默时间，只针对接受数据包有效
    unsigned int can_silence;
    // 接受数据包容忍的沉默时常，针对接收数据有效
    unsigned int can_tolerate_silence;
    // 计数器，接收或发送的次数
    unsigned int can_counter;
};

typedef enum {
    I_TRC = 0,
    I_CRTR = 1,
    I_TST = 2,
    I_CRST = 3,
    I_TTS = 4,
    I_CRTS = 5,
    I_TCV = 6,
    I_CRCV = 7,
    I_TCP = 8,
    I_CRCP = 9,
    I_CSF = 10,
    I_TRSF = 11,
    I_CST = 12,
    I_TRST = 13,
    I_CCT = 14,
    I_TRCT = 15,
    I_CRF = 16,
    I_CTF = 17,
    I_THB = 18
}CAN_PGN_STATISTICS;

typedef enum {
    TCU_TRC = 0,
    TCU_TST = 1,
    TCU_TTS = 2,
    TCU_TCV = 3,
    TCU_TCP = 4,
    TCU_TRSF = 5,
    TCU_TRST = 6,
    TCU_TRCT = 7,
    TCU_THB = 8
}CAN_TCU_GENERATOR;

typedef enum {
    C_CRTR = 0,
    C_CRST = 1,
    C_CRTS = 2,
    C_CRCV = 3,
    C_CRCP = 4,
    C_CSF = 5,
    C_CST = 6,
    C_CCT = 7,
    C_CRF = 8,
    C_CTF = 9
}CAN_C_GENERATOR;


/*
 * 充电任务描述
 */
struct charge_task {
    // 充电任务所处阶段
    TCU_STAGE tcu_stage;
    TCU_STAGE tcu_tmp_stage;
    TCU_STAGE tcu_heartbeat_stage;
    TCU_STAGE tcu_time_stage;
    TCU_STAGE tcu_cct_stage;
    TCU_STAGE tcu_wait_stage;

     // 充电出错所处阶段
    TCU_ERR_STAGE tcu_err_stage;

    CC_STATUS cc_status;

    // CAN TCU 通信所处状态
    CAN_TCU_STATUS can_tcu_status;
    // CAN 通信输入缓冲区
    unsigned char can_buff_in[CAN_BUFF_SIZE];
    volatile unsigned int can_buff_in_nr;
    unsigned int can_packet_pgn;
    // CAN 通信输出缓冲区
    unsigned char can_buff_out[CAN_BUFF_SIZE];
    volatile unsigned int can_buff_out_nr;
    // 标识CAN写缓冲是否满，若缓冲区满，则CAN线程向外写出数据, 写完后置0
    volatile unsigned int can_buff_out_magic;

    // 连接管理时的输出数据包
    unsigned char can_tp_buff_tx[8];
    // 连接管理时的输出数据包大小
    unsigned int can_tp_buff_nr;

    // 连接管理的传输控制参数
    struct can_tp_param can_tp_param;

    // 连接管理超时控制器
    struct Hachiko_food can_tp_bomb;
    struct Hachiko_CNA_TP_private can_tp_private;
    // CAN数据包心跳
    struct Hachiko_food can_heart_beat;
    //TCU心跳
    struct Hachiko_food tcu_heartbeat;

    struct pgn1792_TCV  tcv_info;//TCU版本
    struct pgn2304_TCP  tcp_info;//下发参数
    struct pgn256_TRC trc_info;////启动充电
    struct pgn768_TST tst_info;////停止充电
    struct pgn1280_TTS  tts_info;//下发对时
    struct pgn12544_THB  thb_info;//心跳
    struct pgn4608_TRSF trsf_info;//启动完成应答
    struct pgn5120_TRST trst_info;//停止完成应答
	struct pgn5632_TRCT trct_info;//连接确认应答

	struct pgn512_CRRC crrc_info;//应答启动充电
	struct pgn1024_CRST crst_info;//应答停止充电
	struct pgn1536_CRTS  crts_info;//应答对时
	struct pgn2048_CRCV crcv_info;//充电机版本确认
	struct pgn2560_CRCP crcp_info;//应答参数
	struct pgn4352_CSF csf_info;//Charging启动完成状态信息
	struct pgn4864_CST cst_info;//Charging停止充电完成状态信息
	struct pgn5376_CCT cct_info;//Charging连接确认
	struct pgn8448_CRF crf_info;//Charging遥信帧
	struct pgn8704_CTF ctf_info;//Charging遥测帧
};

//struct charge_task tom;
//struct charge_task *task = &tom;

int about_packet_reciev_done(struct charge_task *thiz,
                             struct event_struct *param);
int about_packet_transfer_done(struct charge_task *thiz,
                             struct event_struct *param);
int gen_packet_tcu_PGN256(struct charge_task * thiz,
                       struct event_struct* param);
int gen_packet_tcu_PGN1792(struct charge_task * thiz,
                        struct event_struct* param);
int gen_packet_tcu_PGN2304(struct charge_task * thiz,
                        struct event_struct* param);
int gen_packet_tcu_PGN768(struct charge_task * thiz,
                        struct event_struct* param);
int gen_packet_tcu_PGN1280(struct charge_task * thiz,
                        struct event_struct* param);
int gen_packet_tcu_PGN12544(struct charge_task * thiz,
                        struct event_struct* param);
int gen_packet_tcu_PGN4608(struct charge_task * thiz,
                        struct event_struct* param);
int gen_packet_tcu_PGN5120(struct charge_task * thiz,
                        struct event_struct* param);
int gen_packet_tcu_PGN5632(struct charge_task * thiz,
						struct event_struct* param);

int set_data_tcu_PGN1792(struct charge_task * thiz);
int set_data_tcu_PGN2304(struct charge_task * thiz);
int set_data_tcu_PGN5632(struct charge_task * thiz);
int set_data_tcu_PGN256(struct charge_task * thiz);
int set_data_tcu_PGN768(struct charge_task * thiz);
int set_data_tcu_PGN1280(struct charge_task * thiz);
int set_data_tcu_PGN4608(struct charge_task * thiz);
int set_data_tcu_PGN5120(struct charge_task * thiz);
int set_data_tcu_PGN12544(struct charge_task * thiz);

int recv_data_tcu_PGN512(struct charge_task * thiz,struct event_struct* param);
int recv_data_tcu_PGN1024(struct charge_task * thiz,struct event_struct* param);
int recv_data_tcu_PGN1536(struct charge_task * thiz,struct event_struct* param);
int recv_data_tcu_PGN2048(struct charge_task * thiz,struct event_struct* param);
int recv_data_tcu_PGN2560(struct charge_task * thiz,struct event_struct* param);
int recv_data_tcu_PGN4352(struct charge_task * thiz,struct event_struct* param);
int recv_data_tcu_PGN4864(struct charge_task * thiz,struct event_struct* param);
int recv_data_tcu_PGN5376(struct charge_task * thiz,struct event_struct* param);
int recv_data_tcu_PGN8448(struct charge_task * thiz,struct event_struct* param);
int recv_data_tcu_PGN8704(struct charge_task * thiz,struct event_struct* param);


int analysis_data_tcu_PGN512(struct charge_task * thiz);
int analysis_data_tcu_PGN1024(struct charge_task * thiz);
int analysis_data_tcu_PGN1536(struct charge_task * thiz);
int analysis_data_tcu_PGN2048(struct charge_task * thiz);
int analysis_data_tcu_PGN2560(struct charge_task * thiz);
int analysis_data_tcu_PGN4352(struct charge_task * thiz);
int analysis_data_tcu_PGN4864(struct charge_task * thiz);
int analysis_data_tcu_PGN5376(struct charge_task * thiz);
int analysis_data_tcu_PGN8448(struct charge_task * thiz);
int analysis_data_tcu_PGN8704(struct charge_task * thiz);

//1字节crc16计算
static inline void calc_crc16(unsigned short *crc, unsigned short  crcbuf)
{
     unsigned char  i,TT;

    *crc=*crc^crcbuf;
    for(i=0;i<8;i++)
    {
        TT=*crc&1;
        *crc=*crc>>1;
        *crc=*crc&0x7fff;
        if (TT==1)
            (*crc)=(*crc)^0xa001;
        *crc=*crc&0xffff;
    }
}

//多字节CRC16计算
static inline unsigned short load_crc(unsigned short cnt, char *dat)
{
     unsigned short i;
     unsigned short crc=0xffff;
    for(i=0;i<cnt&&i<0xff;i++)
    {
        calc_crc16(&crc,dat[i]);
    }
    return crc;
}

// 大小端转换
static inline unsigned short swap_hi_lo_bytes(unsigned short b)
{
    unsigned char h = b >> 8, l = b & 0xFF;
    return (l << 8 | h);
}
#define b2l swap_hi_lo_bytes
#define l2b swap_hi_lo_bytes

/////////////////////////////////////////////////////
//
//功能：二进制取反
//
//输入：const unsigned char *src 二进制数据
// int length 待转换的二进制数据长度
//
//输出：unsigned char *dst 取反后的二进制数据
//
//返回：0 success
//
//////////////////////////////////////////////////////
int convert(unsigned char *dst, const unsigned char *src, int length);

//////////////////////////////////////////////////////////
//
//功能：十六进制转为十进制
//
//输入：const unsigned char *hex 待转换的十六进制数据
// int length 十六进制数据长度
//
//输出：
//
//返回：int rslt 转换后的十进制数据
//
//思路：十六进制每个字符位所表示的十进制数的范围是0 ~255，进制为256
// 左移8位(<<8)等价乘以256
//
/////////////////////////////////////////////////////////
unsigned long HextoDec(const unsigned char *hex, int length) ;

/////////////////////////////////////////////////////////
//
//功能：十进制转十六进制
//
//输入：int dec 待转换的十进制数据
// int length 转换后的十六进制数据长度
//
//输出：unsigned char *hex 转换后的十六进制数据
//
//返回：0 success
//
//思路：原理同十六进制转十进制
//////////////////////////////////////////////////////////
int DectoHex(int dec, unsigned char *hex, int length) ;

/////////////////////////////////////////////////////////
//
//功能：求权
//
//输入：int base 进制基数
// int times 权级数
//
//输出：
//
//返回：unsigned long 当前数据位的权
//
//////////////////////////////////////////////////////////
unsigned long power(int base, int times);

/////////////////////////////////////////////////////////
//
//功能：BCD转10进制
//
//输入：const unsigned char *bcd 待转换的BCD码
// int length BCD码数据长度
//
//输出：
//
//返回：unsigned long 当前数据位的权
//
//思路：压缩BCD码一个字符所表示的十进制数据范围为0 ~ 99,进制为100
// 先求每个字符所表示的十进制值，然后乘以权
//////////////////////////////////////////////////////////
unsigned long BCDtoDec(const unsigned char *bcd, int length) ;

/////////////////////////////////////////////////////////
//
//功能：十进制转BCD码
//
//输入：int Dec 待转换的十进制数据
// int length BCD码数据长度
//
//输出：unsigned char *Bcd 转换后的BCD码
//
//返回：0 success
//
//思路：原理同BCD码转十进制
//
//////////////////////////////////////////////////////////
int DectoBCD(int Dec, unsigned char *Bcd, int length) ;

void bcd2str(const char* inBCD, char *outStr, int len);
void str2bcd(const char *pstr, u8 *pbcd, int len);

#endif /*_TCU_PACKAGE_INCLUDED_H_*/

#ifdef __cplusplus
}
#endif
