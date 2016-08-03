/*
 * 充电控制描述结构
 * All rights reserved Hangzhou Power Equipment Co., Ltd.
 * author: LiJie
 * email:  bedreamer@163.com
 * 2014-08-22
*/

#ifndef _CHARGE_INCLUDED_H_
#define _CHARGE_INCLUDED_H_

struct charge_task;
#include "Hachiko.h"
#include "tcu.h"

// 无效时戳，初始化时默认赋值， 用于time_t默认值
#define INVALID_TIMESTAMP  0x00000000
// TCU 通信时的缓冲区
#define CAN_BUFF_SIZE         1024

#pragma pack(1)



// TCU与Charging交互阶段定义
typedef enum {
    TCU_STAGE_INVALID      		=0x00,     // TCU状态无效，可能是在初始化或者析构
    TCU_STAGE_CHECKVER      	=0x01,//版本校验
    TCU_STAGE_PARAMETER   	=0x02,//下发参数
    TCU_STAGE_CONNECT     	=0x03,//连接确认应答
    TCU_STAGE_START         		=0x04,// 启动充电
    TCU_STAGE_STATUS              =0X05,//启动状态完成ｏｒ失败
    TCU_STAGE_STOP      			=0X06,//停止充电
    TCU_STAGE_STOP_STATUS =0X07,//停止充电完成
    TCU_STAGE_HEAT					=0X08,//心跳
    TCU_STAGE_TIME					=0X09,//对时
    TCU_STAGE_ANY        			=0x0A // 任意阶段
}TCU_STAGE;


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

/*
 * 充电任务描述
 */
struct charge_task {
    // 充电任务所处阶段
    TCU_STAGE tcu_stage;
    TCU_STAGE tcu_tmp_stage;
    TCU_STAGE tcu_heartbeat_stage;
    TCU_STAGE tcu_time_stage;

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

    struct pgn2048_CRCV  crcv_info;//充电机版本
    struct pgn1792_TCV  tcv_info;//TCU版本
    struct pgn2304_TCP  tcp_info;//下发参数
    struct pgn256_TRC trc_info;////启动充电
    struct pgn768_TST tst_info;////停止充电
    struct pgn1280_TTS  tts_info;//下发对时
    struct pgn12544_THB  thb_info;//心跳
    struct pgn4608_TRSF trsf_info;//启动完成应答
    struct pgn5120_TRST trst_info;//停止完成应答
	struct pgn5632_TRCT trct_info;//连接确认应答
};

extern struct charge_task *task;

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

#endif /*_CHARGE_INCLUDED_H_*/
