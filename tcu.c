/*
 * TCU - CAN 通信过程
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <time.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include "charge.h"
#include "Hachiko.h"
#include "tcu.h"
#include "config.h"
#include "log.h"
#include "error.h"
//计费控制单元ＴＣＵ　　　充电机Ｃ
// 数据包生成器信息
struct can_pack_generator generator[] = {

	{//充电启动   4   pgn256   0
	.stage      =  TCU_STAGE_START,
	.pgn        =  0x00100,
	.prioriy    =  4,
	.datalen    =  8,
	.period     =  250,
	.heartbeat   =  0,
	.mnemonic   =  "TRC"
	},
	{//充电停止  6   pgn768  1
	.stage      =  TCU_STAGE_STOP,
	.pgn        =  0x000300,
	.prioriy    =  4,
	.datalen    =  8,
	.period     =  250,
	.heartbeat   =  0,
	.mnemonic   =  "TST"
	},
	{//下发对时  8  pgn1280  2
	.stage      =  TCU_STAGE_TIME,
	.pgn        =  0x000500,
	.prioriy    =  6,
	.datalen    =  8,
	.period     =  510,
	.heartbeat   =  0,
	.mnemonic   =  "TTS"
	},
	{//校验版本  1　pgn1792   3
	.stage      =  TCU_STAGE_CHECKVER,
	.pgn        =  0x000700,
	.prioriy    =  6,
	.datalen    =  2,
	.period     =  500,
	.heartbeat   =  0,
	.mnemonic   =  "TCV"
	},
	{//下发充电参数  2   pgn2304    4
	.stage      =  TCU_STAGE_PARAMETER,
	.pgn        =  0x000900,
	.prioriy    =  6,
	.datalen    =  8,
	.period     =  500,
	.heartbeat   =  0,
	.mnemonic   =  "TCP"
	},
	{//启动完成应答  5 pgn4608   5
	.stage      =  TCU_STAGE_STATUS,
	.pgn        =  0x001200,
	.prioriy    =  4,
	.datalen    =  8,
	.period     =  250,
	.heartbeat   =  0,
	.mnemonic   =  "TRSF"
	},
	{//停机完成应答  7 pgn5120   6
	.stage      =  TCU_STAGE_STOP_STATUS,
	.pgn        =  0x001400,
	.prioriy    =  4,
	.datalen    =  8,
	.period     =  250,
	.heartbeat   =  0,
	.mnemonic   =  "TRST"
	},
    {//连接确认采集  3  pgn5632   7
    .stage      =  TCU_STAGE_CONNECT,
    .pgn        =  0x001600,
    .prioriy    =  4,
    .datalen    =  8,
    .period     =  250,
    .heartbeat   =  0,
    .mnemonic   =  "TRCT"
    },
    {//心跳  9   pgn12544   8
    .stage      =  TCU_STAGE_HEAT,
    .pgn        =  0x003100,
    .prioriy    =  6,
    .datalen    =  8,
    .period     =  1010,
    .heartbeat   =  0,
    .mnemonic   =  "THB"
    }
};

// CAN 数据包统计结构
struct tcu_statistics statistics[] = {
    {//0
    .can_pgn = PGN_TRC,
    .can_silence = 0,
    .can_tolerate_silence = 250,
    .can_counter = 0
    },
    {//1
    .can_pgn = PGN_CRTR,
    .can_silence = 0,
    .can_tolerate_silence = 250,
    .can_counter = 0
    },
    {//2
    .can_pgn = PGN_TST,
    .can_silence = 0,
    .can_tolerate_silence = 250,
    .can_counter = 0
    },
    {//3
    .can_pgn = PGN_CRST,
    .can_silence = 0,
    .can_tolerate_silence = 250,
    .can_counter = 0
    },
    {//4
    .can_pgn = PGN_TTS,
    .can_silence = 0,
    .can_tolerate_silence = 500,
    .can_counter = 0
    },
    {//5
    .can_pgn = PGN_CRTS,
    .can_silence = 0,
    .can_tolerate_silence = 500,
    .can_counter = 0
    },
    {//6
    .can_pgn = PGN_TCV,
    .can_silence = 0,
    .can_tolerate_silence = 500,
    .can_counter = 0
    },
    {//7
    .can_pgn = PGN_CRCV,
    .can_silence = 0,
    .can_tolerate_silence = 500,
    .can_counter = 0
    },
    {//8
    .can_pgn = PGN_TCP,
    .can_silence = 0,
    .can_tolerate_silence = 500,
    .can_counter = 0
    },
    {//9
    .can_pgn = PGN_CRCP,
    .can_silence = 0,
    .can_tolerate_silence = 500,
    .can_counter = 0
    },
    {//10
    .can_pgn = PGN_CSF,
    .can_silence = 0,
    .can_tolerate_silence = 250,
    .can_counter = 0
    },
    {//11
    .can_pgn = PGN_TRSF,
    .can_silence = 0,
    .can_tolerate_silence = 250,
    .can_counter = 0
    },
    {//12
    .can_pgn = PGN_CST,
    .can_silence = 0,
    .can_tolerate_silence = 250,
    .can_counter = 0
    },
    {//13
    .can_pgn = PGN_TRST,
    .can_silence = 0,
    .can_tolerate_silence = 250,
    .can_counter = 0
    },
    {//14
    .can_pgn = PGN_CCT,
    .can_silence = 0,
    .can_tolerate_silence = 250,
    .can_counter = 0
    },
    {//15
    .can_pgn = PGN_TRCT,
    .can_silence = 0,
    .can_tolerate_silence = 250,
    .can_counter = 0
    },
    {//16
    .can_pgn = PGN_CRF,
    .can_silence = 0,
    .can_tolerate_silence = 250,
    .can_counter = 0
    },
    {//17
    .can_pgn = PGN_CTF,
    .can_silence = 0,
    .can_tolerate_silence = 1000,
    .can_counter = 0
    },
    {//18
    .can_pgn = PGN_THB,
    .can_silence = 0,
    .can_tolerate_silence = 1000,
    .can_counter = 0
    }
   };
//void Hachiko_packet_tcu_heart_beart_notify_proc(Hachiko_EVT evt, void *private,
//                            const struct Hachiko_food *self)
//{
//	 if (evt == HACHIKO_TIMEOUT ) {
//	        int i = 0;
//	        struct can_pack_generator *thiz;
//	        struct tcu_statistics *me;
//	        for ( i = 0;
//	              (unsigned int)i < sizeof(generator) / sizeof(struct can_pack_generator); i++ ) {
//	            thiz = &generator[i];
//	            if ( thiz->stage == 0x08/*task->tcu_heartbeat_stage*/ ){
//	            	if ( thiz->heartbeat < thiz->period ) {
//						thiz->heartbeat += 1;
//					} else {
//						thiz->heartbeat = thiz->period;
//						task->tcu_heartbeat_stage  = TCU_STAGE_HEAT;
//						task->tcu_tmp_stage = task->tcu_stage;
//						task->tcu_stage  = TCU_STAGE_HEAT;
//					}
//	            } else {
//	                thiz->heartbeat = 0;
//	            }
//	        }
//	 }
//}

// 数据包超时心跳包, 定时器自动复位, 一个单位时间一次
void Hachiko_packet_heart_beart_notify_proc(Hachiko_EVT evt, void *private,
                            const struct Hachiko_food *self)
{
    if (evt == HACHIKO_TIMEOUT ) {
        int i = 0;
        struct can_pack_generator *thiz;
        struct tcu_statistics *me;
        for ( i = 0;
              (unsigned int)i < sizeof(generator) / sizeof(struct can_pack_generator); i++ ) {
            thiz = &generator[i];
            if ( thiz->stage == task->tcu_stage ) {
                if ( thiz->heartbeat < thiz->period ) {
                   // thiz->heartbeat += 10;
                    thiz->heartbeat += 1;
                } else {
                    thiz->heartbeat = thiz->period;
                }
            }else if ( thiz->stage == 0x08/*task->tcu_heartbeat_stage*/ ){
            	if ( thiz->heartbeat < thiz->period ) {
					thiz->heartbeat += 1;
				} else {
					thiz->heartbeat = thiz->period;
					task->tcu_heartbeat_stage  = TCU_STAGE_HEAT;
					task->tcu_stage  = TCU_STAGE_HEAT;
				}
            } else if ( thiz->stage == 0x09/*task->tcu_time_stage*/ ){
            	if ( thiz->heartbeat < thiz->period ) {
					thiz->heartbeat += 1;
				} else {
					thiz->heartbeat = thiz->period;
					task->tcu_time_stage  = TCU_STAGE_TIME;
					task->tcu_stage  = TCU_STAGE_TIME;
				}
            } else {
                thiz->heartbeat = 0;
            }
        }

        /*
         * 为了能够侦探到接受数据包的超时事件，需要在这里进行一个计数操作
         * 当can_silence 计数大于等于 can_tolerate_silence 时认为对应数据包接收超时，需要在BMS逻辑主线程
         * 中做相应处理.
         *
         * BEM和CEM不在超时统计范围内
         */
#if 0
        for ( i = 0;
              (unsigned int)i < (sizeof(statistics) / sizeof(struct tcu_statistics) ) - 2; i++ ) {
            me = &statistics[i];
            me->can_silence ++;
            if ( me->can_tolerate_silence < me->can_silence ) {
                switch (task->charge_stage) {
                case TCU_STAGE_CHECKVER:
                    if (me->can_pgn != PGN_CRCV) break;
                    if (bit_read(task, F_GUN_1_PHY_CONN_STATUS)) {
                        if ( !bit_read(task, S_BMS_COMM_DOWN) ) {
                            bit_set(task, S_BMS_COMM_DOWN);
                            log_printf(WRN, "BMS: 握手阶段BMS通信"RED("故障"));
                            task->charge_stage = TCU_STAGE_CHECKVER;
                        }
                    }
                    break;
#if 0
                case CHARGE_STAGE_CONFIGURE:
                    if (me->can_pgn != PGN_BCP) break;
                    if (bit_read(task, F_GUN_1_PHY_CONN_STATUS)) {
                        if ( !bit_read(task, S_BMS_COMM_DOWN) ) {
                            bit_set(task, S_BMS_COMM_DOWN);
                            log_printf(WRN, "BMS: 配置阶段BMS通信"RED("故障"));
                            task->charge_stage = CHARGE_STAGE_HANDSHACKING;
                        }
                    }
                    break;
                case CHARGE_STAGE_CHARGING:
                    if (me->can_pgn != PGN_BCL) break;
                    if (bit_read(task, F_GUN_1_PHY_CONN_STATUS)) {
                        if ( !bit_read(task, S_BMS_COMM_DOWN) ) {
                            bit_set(task, S_BMS_COMM_DOWN);
                            log_printf(WRN, "BMS: 充电阶段BMS通信"RED("故障"));
                            task->charge_stage = CHARGE_STAGE_HANDSHACKING;
                        }
                    }
                    break;
                case CHARGE_STAGE_DONE:
                    if (me->can_pgn != PGN_BSD) break;
                    if (bit_read(task, F_GUN_1_PHY_CONN_STATUS)) {
                        if ( !bit_read(task, S_BMS_COMM_DOWN) ) {
                            bit_set(task, S_BMS_COMM_DOWN);
                            log_printf(WRN, "BMS: 充电完成阶段BMS通信"RED("故障"));
                            task->charge_stage = CHARGE_STAGE_HANDSHACKING;
                        }
                    }
                    break;
#endif
                default:
                    break;
                }
            }
        }
#endif
    }
}

/*
 * 本可以将串口通信，SOCKET通信方式也纳入该事件函数，但本着CAN通信优先的原则，暂时将
 * CAN通信时间数据独立出来进行处理。
 *
 * CAN数据包消息处理函数，必须是可重入函数
 * 该函数处于所有需要互斥对象的临界点，有多个线程会直接调用该函数。
 */
static int can_packet_callback(
        struct charge_task * thiz, EVENT_CAN ev, struct event_struct* param)
{
    switch ( ev ) {
    case EVENT_CAN_TIME:
    	log_printf(INF, "TCU: CHARGER now stage to "RED("TCU_STAGE_TIME"));
		thiz->tcu_time_stage = TCU_STAGE_TIME;
		thiz->tcu_stage = TCU_STAGE_TIME;
    	break;
    case EVENT_CAN_HEART:
        thiz->can_tcu_status = CAN_NORMAL;
        //thiz->tcu_heartbeat.Hachiko_notify_proc=
        //		Hachiko_packet_tcu_heart_beart_notify_proc;
		log_printf(INF, "TCU: CHARGER now stage to "RED("TCU_STAGE_HEAT"));
		thiz->tcu_heartbeat_stage = TCU_STAGE_HEAT;
		thiz->tcu_stage = TCU_STAGE_HEAT;
    	break;
    case EVENT_CAN_INIT:
        // 事件循环函数初始化
        thiz->can_tcu_status = CAN_NORMAL;
        thiz->can_heart_beat.Hachiko_notify_proc=
                Hachiko_packet_heart_beart_notify_proc;
        Hachiko_new(&thiz->can_heart_beat, HACHIKO_AUTO_FEED, 1, NULL);
        log_printf(INF, "TCU: CHARGER change stage to "RED("TCU_STAGE_CHECKVER"));
        thiz->tcu_stage = TCU_STAGE_CHECKVER;
        thiz->tcu_tmp_stage = TCU_STAGE_CHECKVER;
        break;
    case EVENT_CAN_RESET:
        // 事件循环函数复位
        // 当发生通信中断时会收到该事件
        break;
    case EVENT_INVALID:
        // 无效事件，事件填充
        break;
    case EVENT_RX_DONE:
        // 数据包接受成功了
    	//printf("EVENT_RX_DONE\n");
        about_packet_reciev_done(thiz, param);
        break;
    case EVENT_RX_ERROR:
        // 数据包接受失败了
        break;
    case EVENT_TX_FAILS:
    	//printf("EVENT_TX_FAILS\n");
        // 数据包发送失败了
        break;
    case EVENT_TX_DONE:
        // 数据包发送完成了
    	//printf("EVENT_TX_DONE\n");
    	param->can_id = param->can_id >> 8;
        about_packet_transfer_done(thiz,param);
        break;
    case EVENT_TX_PRE:
        // 决定是否要发送刚刚准备发送的数据包
        param->evt_param = EVT_RET_OK;
        break;
    case EVENT_TX_REQUEST:
        /*
         * 在这里进行CAN数据包的发送处理
         * 进行数据包发送的条件是：充电枪物理连接正常，进入车辆识别过程，或充电过程。
         *
         * 数据包的发送，优先级最高的是错误报文输出，若是遇到周期性发送的数据包在发送
         * 时序上有重叠的问题，那么在这里的处理方式是，先到先处理，例如在若干个循环内
         * 数据包A，B的发送周期是10ms, 30ms，那么A，B的发送时时序应该是如下方式
         * T (ms)      数据包
         * |             |
         * 0             A
         * 0  + $$       B
         * |             |
         * |             |
         * 10            A
         * |             |
         * |             |
         * 20            A
         * |             |
         * |             |
         * 30            A
         * 30 + $$       B
         * |             |
         * |             |
         * ...         ...
         *
         * $$ 表示最小的循环控制周期，一般来说是绝对小于数据包的发送周期的
         * 所以会有如下的控制逻辑结构
         * if ( ... ) {
         * } else if ( ... ) {
         * } else if ( ... ) {
         * } else ;
         * 在若干个循环控制周期内，数据包都能按照既定的周期发送完成.
         */
        switch ( thiz->tcu_stage ) {
			case TCU_STAGE_INVALID:
				param->evt_param = EVT_RET_ERR;
				break;
			case TCU_STAGE_TIME:
				if ( generator[TCU_TTS].heartbeat >= generator[TCU_TTS].period ) {
					gen_packet_tcu_PGN1280(thiz, param);
					generator[TCU_TTS].heartbeat = 0;
				}
				break;
			case TCU_STAGE_HEAT:
				if ( generator[TCU_THB].heartbeat >= generator[TCU_THB].period ) {
					gen_packet_tcu_PGN12544(thiz, param);
					generator[TCU_THB].heartbeat = 0;
					//thiz->tcu_stage = thiz->tcu_heartbeat_stage ;
				}
				break;
			case TCU_STAGE_CHECKVER:
				//printf( "now  CHARGE_STAGE_HANDSHACKING generator[TCU_TCV].heartbeat= %d  generator[TCU_TCV].period= %d\n",generator[TCU_TCV].heartbeat,generator[TCU_TCV].period);
				if ( generator[TCU_TCV].heartbeat >= generator[TCU_TCV].period ) {
					//printf( "now  CHARGE_STAGE_HANDSHACKING generator[0].heartbeat= %d  generator[0].period= %d\n",generator[0].heartbeat,generator[0].period);
					gen_packet_tcu_PGN1792(thiz, param);
					generator[TCU_TCV].heartbeat = 0;
				}
				break;
			case TCU_STAGE_PARAMETER:
				if ( generator[TCU_TCP].heartbeat >= generator[TCU_TCP].period ) {
					gen_packet_tcu_PGN2304(thiz, param);
					generator[TCU_TCP].heartbeat = 0;
				}
				break;
			case TCU_STAGE_CONNECT:
				if ( generator[TCU_TRCT].heartbeat >= generator[TCU_TRCT].period ) {
					gen_packet_tcu_PGN5632(thiz, param);
					generator[TCU_TRCT].heartbeat = 0;
				}
				break;
			case TCU_STAGE_START:
				if ( generator[TCU_TRC].heartbeat >= generator[TCU_TRC].period ) {
					gen_packet_tcu_PGN256(thiz, param);
					generator[TCU_TRC].heartbeat = 0;
				}
				break;
			case TCU_STAGE_STATUS:
				if ( generator[TCU_TRSF].heartbeat >= generator[TCU_TRSF].period ) {
					gen_packet_tcu_PGN4608(thiz, param);
					generator[TCU_TRSF].heartbeat = 0;
				}
				break;
			case TCU_STAGE_STOP:
				if ( generator[TCU_TST].heartbeat >= generator[TCU_TST].period ) {
					gen_packet_tcu_PGN768(thiz, param);
					generator[TCU_TST].heartbeat = 0;
				}
				break;
			case TCU_STAGE_STOP_STATUS:
				if ( generator[TCU_TRST].heartbeat >= generator[TCU_TRST].period ) {
					gen_packet_tcu_PGN5120(thiz, param);
					generator[TCU_TRST].heartbeat = 0;
				}
				break;
			default:
				break;
        }
        break;
    case EVENT_TX_TP_RTS: // 本系统中TCU通信暂时不会使用
        //串口处于连接管理状态时，将会收到该传输数据报请求。
        break;
    case EVENT_TX_TP_CTS:
    {
        /*串口处于连接管理状态时，将会收到该传输数据报请求。
         * 当数据包接收完成后向TCU发送消息结束应答数据包
         *
         * byte[1]: 0x13
         * byte[2:3]: 消息大小，字节数目
         * byte[4]: 全部数据包数目
         * byte[5]: 0xFF
         * byte[6:8]: PGN
         */

        param->buff.tx_buff[0] = 0x11;
        // 目前的多数据包发送策略是： 无论要发送多少数据包，都一次传输完成
        param->buff.tx_buff[1] = thiz->can_tp_param.tp_pack_nr;
        param->buff.tx_buff[2] = 1;
        param->buff.tx_buff[3] = 0xFF;
        param->buff.tx_buff[4] = 0xFF;
        param->buff.tx_buff[5] = (thiz->can_tp_param.tp_pgn >> 16) & 0xFF;
        param->buff.tx_buff[6] = (thiz->can_tp_param.tp_pgn >> 8 ) & 0xFF;
        param->buff.tx_buff[7] = thiz->can_tp_param.tp_pgn & 0xFF;
        param->buff_payload = 8;
        param->can_id = 0x1cecf456 | CAN_EFF_FLAG;
        param->evt_param = EVT_RET_OK;
    }
        break;
    case EVENT_TX_TP_ACK:
    {
        //串口处于连接管理状态时，将会收到该传输数据报请求。
        param->buff.tx_buff[0] = 0x13;
        // 目前的多数据包发送策略是： 无论要发送多少数据包，都一次传输完成
        param->buff.tx_buff[1] = thiz->can_tp_param.tp_size & 0xFF;
        param->buff.tx_buff[2] = (thiz->can_tp_param.tp_size >> 8) & 0xFF;
        param->buff.tx_buff[3] = thiz->can_tp_param.tp_pack_nr;
        param->buff.tx_buff[4] = 0xFF;
        param->buff.tx_buff[5] = (thiz->can_tp_param.tp_pgn >> 16) & 0xFF;
        param->buff.tx_buff[6] = (thiz->can_tp_param.tp_pgn >> 8 ) & 0xFF;
        param->buff.tx_buff[7] = thiz->can_tp_param.tp_pgn & 0xFF;
        param->buff_payload = 8;
        param->can_id = 0x1cecf456 | CAN_EFF_FLAG;
        param->evt_param = EVT_RET_OK;
    }
        break;
    case EVENT_TX_TP_ABRT:
        //串口处于连接管理状态时，将会收到该传输数据报请求。
        break;
    default:
        break;
    }

    return 0;
}



// CAN数据包接受完成
int about_packet_reciev_done(struct charge_task *thiz,
                             struct event_struct *param)
{
	//log_printf(DBG_LV0, "TCU: packet receive. %08X", param->can_id);
    switch ( param->can_id & 0x00FF00) {
    case PGN_CRCV :// 0x000800, Charging 应答版本校验
        statistics[I_CRCV].can_counter ++;
        statistics[I_CRCV].can_silence = 0;
        log_printf(INF, "TCU: TCU  now  "GRN("PGN_CRCV"));

        if ( thiz->tcu_stage == TCU_STAGE_CHECKVER) {
			thiz->tcu_stage = TCU_STAGE_PARAMETER;
			thiz->tcu_tmp_stage = TCU_STAGE_PARAMETER;
			log_printf(INF, "TCU: TCU change stage to "RED("TCU_STAGE_PARAMETER"));
		}
        break;
    case PGN_CRCP:
    	statistics[I_CRCP].can_counter ++;
		statistics[I_CRCP].can_silence = 0;
		log_printf(INF, "TCU: TCU  now  "GRN("PGN_CRCP"));
		 if ( thiz->tcu_stage == TCU_STAGE_PARAMETER) {
			//thiz->charge_stage = TCU_STAGE_CONNECT;
			log_printf(INF, "TCU: TCU now stage to "RED("TCU_STAGE_PARAMETER"));
		}
    	break;
    case PGN_CCT:
    	statistics[I_CCT].can_counter ++;
		statistics[I_CCT].can_silence = 0;
		log_printf(INF, "TCU: TCU  now  "GRN("PGN_CCT"));
		 if ( thiz->tcu_stage == TCU_STAGE_PARAMETER) {
			thiz->tcu_stage = TCU_STAGE_CONNECT;
			thiz->tcu_tmp_stage = TCU_STAGE_CONNECT;
			log_printf(INF, "TCU: TCU change stage to "RED("TCU_STAGE_CONNECT"));
		}
    	break;
    case PGN_CRTR:
    	statistics[I_CRTR].can_counter ++;
		statistics[I_CRTR].can_silence = 0;
		log_printf(INF, "TCU: TCU  now  "GRN("PGN_CRTR"));
		 if ( thiz->tcu_stage == TCU_STAGE_START) {
			//thiz->charge_stage = TCU_STAGE_STATUS;
			log_printf(INF, "TCU: TCU now stage to "RED("TCU_STAGE_START"));
		}
    	break;
    case PGN_CSF:
    	statistics[I_CSF].can_counter ++;
		statistics[I_CSF].can_silence = 0;
		log_printf(INF, "TCU: TCU  now  "GRN("PGN_CSF"));
		 if ( thiz->tcu_stage == TCU_STAGE_START) {
			thiz->tcu_stage = TCU_STAGE_STATUS;
			thiz->tcu_tmp_stage = TCU_STAGE_STATUS;
			log_printf(INF, "TCU: TCU now stage to "RED("TCU_STAGE_START"));
		}
    	break;
    case PGN_CRST:
    	statistics[I_CRST].can_counter ++;
		statistics[I_CRST].can_silence = 0;
		log_printf(INF, "TCU: TCU  now  "GRN("PGN_CRST"));
		 if ( thiz->tcu_stage == TCU_STAGE_STOP) {
			//thiz->charge_stage = TCU_STAGE_STOP_STATUS;
			log_printf(INF, "TCU: TCU now stage to "RED("TCU_STAGE_STOP"));
		}
    	break;
    case PGN_CST:
    	statistics[I_CST].can_counter ++;
		statistics[I_CST].can_silence = 0;
		log_printf(INF, "TCU: TCU  now  "GRN("PGN_CST"));
		 if ( thiz->tcu_stage == TCU_STAGE_STOP) {
			thiz->tcu_stage = TCU_STAGE_STOP_STATUS;
			thiz->tcu_tmp_stage = TCU_STAGE_STOP_STATUS;
			log_printf(INF, "TCU: TCU change stage to "RED("TCU_STAGE_STOP_STATUS"));
		}
    	break;
    case PGN_CRF:
    	break;
    case PGN_CTF:
    	break;
    default:
        log_printf(WRN, "TCU: un-recognized PGN %08X",
                   param->can_id);
        break;
    }
    return ERR_OK;
}
// CAN数据包发送完成
int about_packet_transfer_done(struct charge_task *thiz,
                             struct event_struct *param)
{
    //log_printf(DBG_LV0, "TCU: packet sent. %08X", param->can_id);
    //param->can_id = (param->can_id & 0x00FF0000) >> 8;
	switch (param->can_id & 0x00FF00) {
    	case PGN_TCV:
    		break;
    	case PGN_TCP:
    		break;
    	case PGN_TRC:
    		break;
    	case PGN_TST:
    		break;
    	case PGN_TTS:
    		thiz->tcu_stage = thiz->tcu_tmp_stage;
    		break;
    	case PGN_TRCT:
    		log_printf(INF, "TCU: TCU  now  "GRN("PGN_TRCT"));
    		 if ( thiz->tcu_stage == TCU_STAGE_CONNECT) {
				thiz->tcu_stage = TCU_STAGE_START;
				thiz->tcu_tmp_stage = TCU_STAGE_START;
				log_printf(INF, "TCU: TCU change stage to "RED("TCU_STAGE_START"));
				//task->can_tcu_status = CAN_NORMAL;
				//can_packet_callback(task, EVENT_TX_PRE, &param);
				//can_packet_callback(task, EVENT_TX_REQUEST, &param);
			}
    		break;
    	case PGN_TRSF:
    		log_printf(INF, "TCU: TCU  now  "GRN("PGN_TRSF"));
    		if ( thiz->tcu_stage == TCU_STAGE_STATUS) {
				thiz->tcu_stage = TCU_STAGE_STOP;
				thiz->tcu_tmp_stage = TCU_STAGE_STOP;
				log_printf(INF, "TCU: TCU change stage to "RED("TCU_STAGE_STOP"));
				can_packet_callback(task, EVENT_TX_REQUEST, &param);
			}
    		break;
    	case  PGN_THB:
    		//thiz->tcu_tmp_stage = thiz->tcu_stage;
    		//thiz->tcu_heartbeat_stage = TCU_STAGE_ANY;
    		thiz->tcu_stage = thiz->tcu_tmp_stage;
    		break;
    	default:
    		break;

    }
	return ERR_OK;
}


// tcu 通信 写 服务线程
// 提供tcu通信服务
void *thread_tcu_write_service(void *arg) ___THREAD_ENTRY___
{
    int *done = (int *)arg;
    int mydone = 0;
    int s;
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct can_frame frame;
    struct event_struct param;
    unsigned char txbuff[32];
    int nbytes;
    if ( done == NULL ) done = &mydone;

    s = socket(PF_CAN, SOCK_RAW, CAN_RAW);

    strcpy(ifr.ifr_name, "can1" );
    ioctl(s, SIOCGIFINDEX, &ifr);

    addr.can_family = PF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    bind(s, (struct sockaddr *)&addr, sizeof(addr));

    log_printf(INF, "%s running...s=%d", __FUNCTION__, s);

    param.buff_payload = 0;
    param.evt_param = EVT_RET_INVALID;


    // 进行数据结构的初始化操作
    can_packet_callback(task, EVENT_CAN_INIT, &param);
    can_packet_callback(task, EVENT_CAN_HEART, &param);
    can_packet_callback(task, EVENT_CAN_TIME, &param);
    while ( ! *done ) {
        usleep(5000);

        /*
         * 写线程同时负责写数据和进行连接管理时的控制数据写出，这里需要对当前CAN的
         * 状态进行判定，当CAN处于CAN_NORMAL时进行普通的写操作，当CAN处于CAN_TP_RD
         * 时，采用EVENT_TX_REQUEST 当CAN处于CAN_TP_RD时采用EVENT_TX_TP_REQUEST
         */
        param.buff.tx_buff = txbuff;
        param.buff_size = sizeof(txbuff);
        param.evt_param = EVT_RET_INVALID;
        if ( task->can_tcu_status & CAN_NORMAL ) {
            can_packet_callback(task, EVENT_TX_REQUEST, &param);
        } else if ( task->can_tcu_status & CAN_TP_RD ) { // 连接管理读模式, 多数据包读
            	switch ( task->can_tcu_status & 0xF0 ) {
						case CAN_TP_CTS: // 链接准备发送
							can_packet_callback(task, EVENT_TX_TP_CTS, &param);
							break;
						case CAN_TP_TX:
						case CAN_TP_RX:
							break;
						case CAN_TP_ACK:
							can_packet_callback(task, EVENT_TX_TP_ACK, &param);
							break;
						case CAN_TP_ABRT:
							can_packet_callback(task, EVENT_TX_TP_ABRT, &param);
							break;
						default:
							log_printf(WRN, "TCU: can_tcu_status crashed(%d).",
									   task->can_tcu_status);
							continue;
							break;
            }
        } else if ( task->can_tcu_status & CAN_TP_WR ) { // 连接管理写模式，多数据包写，当前的协议标准没有用到
            log_printf(WRN, "TCU: CAN_TP_WRITE not implement.");
            continue;
        } else if ( task->can_tcu_status == CAN_INVALID ) { // 无效模式
            log_printf(DBG_LV0, "TCU: invalid can_tcu_status: %d.",
                       task->can_tcu_status);
            continue;
        } else {
            log_printf(DBG_LV0, "TCU: invalid can_tcu_status: %d.",
                       task->can_tcu_status);
            continue;
        }

        if ( EVT_RET_OK != param.evt_param ) {
            continue;
        }

        param.evt_param = EVT_RET_INVALID;
        // 链接模式下的数据包发送不需要确认, 并且也不能被中止
        if ( task->can_tcu_status == CAN_NORMAL ) {
            can_packet_callback(task, EVENT_TX_PRE, &param);
            if ( EVT_RET_TX_ABORT == param.evt_param ) {
                // packet sent abort.
                continue;
            } else if ( EVT_RET_OK != param.evt_param ) {
                continue;
            } else {
                // confirm to send.
            }
        }

        /* 根据GB/T 27930-2011 中相关规定，充电机向BMS发送数据包都没有超过
         * 8字节限制，因此这里不用进行连接管理通信。
         */
        if ( param.buff_payload <= 8 && param.buff_payload > 0 ) {
					frame.can_id = param.can_id;
					frame.can_dlc= param.buff_payload;
					memcpy(frame.data, param.buff.tx_buff, 8);
					nbytes = write(s, &frame, sizeof(struct can_frame));
					if ( (unsigned int)nbytes < param.buff_payload ) {printf("EVT_RET_ERR");
							param.evt_param = EVT_RET_ERR;
							can_packet_callback(task, EVENT_TX_FAILS, &param);
					} else {
							param.evt_param = EVT_RET_OK;
							can_packet_callback(task, EVENT_TX_DONE, &param);
					}
        } else if ( param.buff_payload > 8 ) {
            // 大于8字节的数据包在这里处理，程序向后兼容
					static unsigned int notimplement = 0;
					if ( notimplement % 10000 == 0 ) {
						log_printf(INF,
								   "TCU: Connection manage for send has not implemented.");
					}
					notimplement ++;
        } else {
        }

        param.buff_payload = 0;

        // 准备接收完成
        if ( task->can_tcu_status == (CAN_TP_RD | CAN_TP_CTS) ) {
				task->can_tcu_status = (CAN_TP_RD | CAN_TP_RX);
				log_printf(DBG_LV3, "TCU: ready for data transfer.");
        }
        // 应答结束
        if ( task->can_tcu_status == (CAN_TP_RD | CAN_TP_ACK) ) {
				task->can_tcu_status = CAN_NORMAL;
				log_printf(DBG_LV0, "TCU: connection closed normally.");
        }
        // 传输终止
        if ( task->can_tcu_status == (CAN_TP_RD | CAN_TP_ABRT) ) {
				task->can_tcu_status = CAN_NORMAL;
				log_printf(DBG_LV2, "TCU: connection aborted.");
        }
    }
    return NULL;
}

// CAN 数据发送报文
void Hachiko_CAN_TP_notify_proc(Hachiko_EVT evt, void *private,
                            const struct Hachiko_food *self)
{
    if ( evt == HACHIKO_TIMEOUT ) {
        log_printf(WRN, "TCU: CAN data transfer terminal due to time out.");
        task->can_tcu_status = CAN_NORMAL;
    } else if ( evt == HACHIKO_DIE ) {

    }
}


// tcu 通信 读 服务线程
// 提供tcu通信服务
void *thread_tcu_read_service(void *arg) ___THREAD_ENTRY___
{
    int *done = (int *)arg;
    int mydone = 0;
    int s;
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct can_frame frame;
    int nbytes;
    struct event_struct param;
    // 用于链接管理的数据缓冲
    unsigned char tp_buff[2048];

    // 缓冲区数据字节数
    unsigned int tp_cnt = 0;
    // 数据包个数
    unsigned int tp_packets_nr = 0;
    // 数据包字节总数
    unsigned int tp_packets_size = 0;
    // 数据包对应的PGN
    unsigned int tp_packet_PGN = 0;

    unsigned int dbg_packets = 0;

    task->can_tp_private.status = PRIVATE_INVALID;
    task->can_tp_bomb.private = (void *)&task->can_tp_private;

    if ( done == NULL ) done = &mydone;
    s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    strcpy(ifr.ifr_name, "can1" );
    ioctl(s, SIOCGIFINDEX, &ifr);
    addr.can_family = PF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    bind(s, (struct sockaddr *)&addr, sizeof(addr));
    log_printf(INF, "TCU: %s running...s=%d", __FUNCTION__, s);

    param.buff.rx_buff = tp_buff;
    param.buff_size = sizeof(tp_buff);
    param.buff_payload = 0;
    while ( ! *done ) {
        usleep(5000);

        if ( task->can_tcu_status  == CAN_INVALID ) {
        	log_printf(DBG_LV0, "TCU: can_tcu_status CAN_INVALID");
        	//log_printf(INF, "BMS: can_tcu_status CAN_INVALID");
            //continue;
        }

        memset(&frame, 0, sizeof(frame));
        nbytes = read(s, &frame, sizeof(struct can_frame));
        if ( (frame.can_id & 0xFFFF) != CAN_RCV_ID_MASK ) {
            #if 0
            log_printf(DBG_LV0, "TCU: id not accept %x", frame.can_id);
            log_printf(INF, "TCU: id not accept %x  %lu", frame.can_id&CAN_EFF_MASK,frame.can_id);
            #endif
            continue;
        }

        dbg_packets ++;

        if ( nbytes != sizeof(struct can_frame) ) {
            param.evt_param = EVT_RET_ERR;
            log_printf(DBG_LV3, "TCU: read frame error %x", frame.can_id);
            log_printf(INF, "TCU: read frame error %x", frame.can_id&CAN_EFF_MASK);
            can_packet_callback(task, EVENT_RX_ERROR, &param);
            continue;
        }

        debug_log(INF,/*DBG_LV1,*/
                   "TCU: get %dst packet %08X:%02X%02X%02X%02X%02X%02X%02X%02X",
                   dbg_packets,
                   frame.can_id&CAN_EFF_MASK,
                   frame.data[0],
                   frame.data[1],
                   frame.data[2],
                   frame.data[3],
                   frame.data[4],
                   frame.data[5],
                   frame.data[6],
                   frame.data[7]);

        /*
         * CAN通信处于普通模式
         *
         * SAE J1939-21 Revised December 2006 版协议中规定
         * TP.DT.PGN 为 0x00EB00  连接管理
         * TP.CM.PGN 为 0x00EC00  数据传输
         * CAN 的数据包大小最大为8字节，因此需要传输大于等于9字节的数据包
         * 时需要使用连接管理功能来传输数据包。
         * 首先由数据发送方，发送一个数据发送请求包，请求包中包含的数据内容有
         * 消息的总长度，需要发送的数据包个数（必须大于1），最大的数据包编号，
         * 这个消息的PGN等
         */
        if ( ((frame.can_id & 0x00FF0000) >> 16) == 0xEB ) {
            /* Data transfer
             * byte[1]: 数据包编号
             * byte[2:8]: 数据
             */
            if ( (task->can_tcu_status & CAN_TP_RD) != CAN_TP_RD ) {
                task->can_tcu_status = CAN_NORMAL;
                log_printf(WRN, "TCU: timing crashed.");
                continue;
            }
            Hachiko_feed(&task->can_tp_bomb);
            memcpy(&tp_buff[ (frame.data[0] - 1) * 7 ], &frame.data[1], 7);
            log_printf(DBG_LV1, "TCU data tansfer fetch the %dst packet.",
                       frame.data[0]);
            task->can_tp_param.tp_rcv_pack_nr ++;
            if ( task->can_tp_param.tp_rcv_pack_nr >=
                 task->can_tp_param.tp_pack_nr ) {
                // 数据接收完成后即可关闭定时器
                Hachiko_kill(&task->can_tp_bomb);

                param.buff_payload = task->can_tp_param.tp_size;
                param.evt_param = EVT_RET_INVALID;
                param.can_id = task->can_tp_param.tp_pgn;
                log_printf(DBG_LV3,
                           "TCU: data transfer complete PGN=%08X change to ACK",
                           task->can_tp_param.tp_pgn);
                can_packet_callback(task, EVENT_RX_DONE, &param);
                // 数据链接接受完成
                task->can_tcu_status = CAN_TP_RD | CAN_TP_ACK;
            }
        } else if ( ((frame.can_id & 0x00FF0000) >> 16 ) == 0xEC ) {
            // Connection managment
					if ( 0x10 == frame.data[0] ) {
						if ( task->can_tp_buff_nr ) {
							/*
							 * 数据传输太快，还没将缓冲区的数据发送出去
							 */
							log_printf(WRN, "TCU: CAN data transfer too fast.");
							continue;
						}
						/* request a connection. TP.CM_RTS
						 * byte[1]: 0x10
						 * byte[2:3]: 消息大小，字节数目
						 * byte[4]: 全部数据包的数目
						 * byte[5]: 0xFF
						 * byte[6:8]: PGN
						 */
						tp_cnt = 0;
						tp_packets_size = frame.data[2] * 256 + frame.data[1];
						tp_packets_nr = frame.data[3];
						tp_packet_PGN = frame.data[5] +
								(frame.data[6] << 8) + (frame.data[7] << 16);
						/*
						 * 接收到这个数据包后向TCU发送准备发送数据包
						 *
						 * byte[1]: 0x11
						 * byte[2]: 可发送的数据包个数
						 * byte[3]: 下一个要发送的数据包编号
						 * byte[4:5]: 0xFF
						 * byte[6:8]: PGN
						 */
						if ( tp_packets_size <= 8 ) {
							log_printf(WRN,
								 "TCU: detect a TCU transfer error, pack size < 8 bytes");
							continue;
						}
						if ( tp_packets_nr <= 1 ) {
							log_printf(WRN,
									   "TCU: detect a TCU transfer error, pack count < 2");
							continue;
						}

						if ( task->can_tp_private.status == PRIVATE_BUSY ) {
							log_printf(WRN, "TCU: previous connection not exit,"
									   " do new connection instead.");
							Hachiko_feed( &task->can_tp_bomb );
						} else {
							task->can_tp_bomb.Hachiko_notify_proc =
									Hachiko_CAN_TP_notify_proc;
							// 根据SAE J1939-21中关于CAN总线数据传输链接的说明，中间传输
							// 过程最大不超过1250ms
							int ret = Hachiko_new( & task->can_tp_bomb,
												   HACHIKO_ONECE, 1250,
												   &task->can_tp_private);
							if ( ret == (int)ERR_WRONG_PARAM ) {
								log_printf(ERR,
										   "TCU: set new timer error, with code:%d",
										   ret);
								continue;
							}
							if ( ret == (int)ERR_TIMER_BEMAX ) {
								log_printf(ERR,
										   "TCU: set new timer error, with code:%d",
										   ret);
								continue;
							}
						}

						task->can_tp_param.tp_pack_nr = tp_packets_nr;
						task->can_tp_param.tp_size = tp_packets_size;
						task->can_tp_param.tp_pgn = tp_packet_PGN;
						task->can_tp_param.tp_rcv_bytes = 0;
						task->can_tp_param.tp_rcv_pack_nr = 0;
						task->can_tcu_status = CAN_TP_RD | CAN_TP_CTS;
						log_printf(DBG_LV2,
								   "TCU: data connection accepted, rolling..."
								   "PGN: %X, total: %d packets, %d bytes",
								   tp_packet_PGN, tp_packets_nr, tp_packets_size);
						} else if ( 0xFF == frame.data[0] ) {
						/* connection abort.
						 * byte[1]: 0xFF
						 * byte[2:5]: 0xFF
						 * byte[6:8]: PGN
						 */
						int *d = (int *)&frame.data[0];
						log_printf(DBG_LV2, "TCU: %08X", *d);
						} else {
						//omited.
						int *d = (int *)&frame.data[0];
						log_printf(DBG_LV3, "TCU: %08X", *d);
						}
        } else {
				param.can_id = (frame.can_id & 0x00FF0000) >> 8;
				param.buff_payload = frame.can_dlc;
				printf(" param.buff_payload==%d\n", param.buff_payload);
				param.evt_param = EVT_RET_INVALID;
				memcpy((void * __restrict__)param.buff.rx_buff, frame.data, 8);
				can_packet_callback(task, EVENT_RX_DONE, &param);
				log_printf(DBG_LV0, "TCU: read a frame done. %08X", frame.can_id);
				log_printf(INF, "TCU: read a frame done. %08X %08X", frame.can_id,param.can_id);
        }

        if ( task->can_tcu_status == CAN_NORMAL ) {
        } else if ( task->can_tcu_status == CAN_TP_RD ) {
            // CAN通信处于连接管理模式
        }
    }

    return NULL;
}

//----------------------------------------------------------------------------------------------------------------------------------
// TCU校验版本
int gen_packet_tcu_PGN1792(struct charge_task * thiz, struct event_struct* param)
{
    struct can_pack_generator *gen = &generator[TCU_TCV];

    memset(param->buff.tx_buff, 0xFF, sizeof(struct pgn1792_TCV));
	memset(&thiz->tcv_info, 0xFF, sizeof(struct pgn1792_TCV));
    param->buff.tx_buff[0] = 12;
    param->buff.tx_buff[1] = 10;
    thiz->tcv_info.spn_tcu_version[0] = 12;
    thiz->tcv_info.spn_tcu_version[1] = 10;
    param->buff_payload = gen->datalen;
    param->can_id = gen->prioriy << 26 | gen->pgn << 8 | CAN_TX_ID_MASK | CAN_EFF_FLAG;
    param->evt_param = EVT_RET_OK;
    statistics[I_TCV].can_counter ++;

    return 0;
}
// TCU下发参数
int gen_packet_tcu_PGN2304(struct charge_task * thiz, struct event_struct* param)
{
    struct can_pack_generator *gen = &generator[TCU_TCP];

    memset(param->buff.tx_buff, 0xFF,  gen->datalen);
	memset(&thiz->tcp_info, 0xFF, sizeof(struct pgn2304_TCP));
	thiz->tcp_info.spn2304_charger_sn[0] = 0x00;
	thiz->tcp_info.spn2304_charger_sn[1] = 0x00;
	thiz->tcp_info.spn2304_charger_sn[2] = 0x00;
	thiz->tcp_info.spn2304_charger_sn[3] = 0x00;

	thiz->tcp_info.spn2304_charger_region_code[0] = '1';
	thiz->tcp_info.spn2304_charger_region_code[1] = '1';
	thiz->tcp_info.spn2304_charger_region_code[2] = '1';

    param->buff.tx_buff[0] = 0x00;
    param->buff.tx_buff[1] = 0x00;
    param->buff.tx_buff[2] = 0x00;
    param->buff.tx_buff[3] = 0x00;
    param->buff.tx_buff[4] = '1';
    param->buff.tx_buff[5] = '1';
    param->buff.tx_buff[6] = '1';


    param->buff_payload = gen->datalen;
    param->can_id = gen->prioriy << 26 | gen->pgn << 8 | CAN_TX_ID_MASK | CAN_EFF_FLAG;
    param->evt_param = EVT_RET_OK;
    statistics[I_TCP].can_counter ++;

    return 0;
}

// TCU启动充电
int gen_packet_tcu_PGN256(struct charge_task * thiz, struct event_struct* param)
{
    struct can_pack_generator *gen = &generator[TCU_TRC];

    memset(param->buff.tx_buff, 0xFF, sizeof(struct pgn256_TRC));
	memset(&thiz->tcv_info, 0xFF, sizeof(struct pgn256_TRC));
    param->buff.tx_buff[0] = '1';
    param->buff.tx_buff[1] = 0;
    thiz->trc_info.spn256_port = 1;
    thiz->trc_info.spn256_load_control = 0;
    param->buff_payload = gen->datalen;
    param->can_id = gen->prioriy << 26 | gen->pgn << 8 | CAN_TX_ID_MASK | CAN_EFF_FLAG;
    param->evt_param = EVT_RET_OK;
    statistics[I_TRC].can_counter ++;

    return 0;
}

// TCU停止充电
int gen_packet_tcu_PGN768(struct charge_task * thiz, struct event_struct* param)
{
    struct can_pack_generator *gen = &generator[TCU_TST];

    memset(param->buff.tx_buff, 0xFF, sizeof(struct pgn768_TST));
	memset(&thiz->tcv_info, 0xFF, sizeof(struct pgn768_TST));
    param->buff.tx_buff[0] = '1';
    param->buff.tx_buff[1] = 0x00;
    thiz->tst_info.spn768_port = 1;
    thiz->tst_info.spn768_status = 0x00;
    param->buff_payload = gen->datalen;
    param->can_id = gen->prioriy << 26 | gen->pgn << 8 | CAN_TX_ID_MASK | CAN_EFF_FLAG;
    param->evt_param = EVT_RET_OK;
    statistics[I_TST].can_counter ++;

    return 0;
}

// TCU下发对时
int gen_packet_tcu_PGN1280(struct charge_task * thiz, struct event_struct* param)
{
    struct can_pack_generator *gen = &generator[TCU_TTS];
    time_t timep;
    struct tm *p;

    time(&timep);
    p =localtime(&timep);
    if ( p == NULL ) {
        param->evt_param = EVT_RET_ERR;
        return 0;
    }
    thiz->tts_info.spn1280_Immediately = 0;
    thiz->tts_info.spn1280_bcd_sec = (((p->tm_sec / 10 ) & 0x0F ) << 4) |
               ((p->tm_sec % 10) & 0x0F);
    thiz->tts_info.spn1280_bcd_min = (((p->tm_min / 10 ) & 0x0F ) << 4) |
               ((p->tm_min % 10) & 0x0F);
    thiz->tts_info.spn1280_bcd_hour = (((p->tm_hour / 10 ) & 0x0F ) << 4) |
               ((p->tm_hour % 10) & 0x0F);
    thiz->tts_info.spn1280_bcd_day = (((p->tm_mday / 10 ) & 0x0F ) << 4) |
               ((p->tm_mday % 10) & 0x0F);
    thiz->tts_info.spn1280_bcd_mon = (((p->tm_mon / 10 ) & 0x0F ) << 4) |
               ((p->tm_mon % 10) & 0x0F);
    thiz->tts_info.spn1280_bcd_year_h = (((p->tm_year / 100 ) & 0x0F ) << 4) |
               ((p->tm_year % 100) & 0x0F);
    thiz->tts_info.spn1280_bcd_year_l = (((p->tm_year / 10 ) & 0x0F ) << 4) |
               ((p->tm_year % 10) & 0x0F);

	memset(&thiz->tts_info, 0xFF, sizeof(struct pgn1280_TTS));
	memset(param->buff.tx_buff, 0xFF, sizeof(struct pgn1280_TTS));
	memcpy(param->buff.tx_buff, &thiz->tts_info, sizeof(struct pgn1280_TTS));

    param->buff_payload = gen->datalen;
    param->can_id = gen->prioriy << 26 | gen->pgn << 8 | CAN_TX_ID_MASK | CAN_EFF_FLAG;
    param->evt_param = EVT_RET_OK;
    statistics[I_TTS].can_counter ++;

    return 0;
}

// TCU心跳帧
int gen_packet_tcu_PGN12544(struct charge_task * thiz, struct event_struct* param)
{
    struct can_pack_generator *gen = &generator[TCU_THB];

    memset(param->buff.tx_buff, 0xFF, sizeof(struct pgn12544_THB));
	memset(&thiz->thb_info, 0xFF, sizeof(struct pgn12544_THB));

    thiz->thb_info.spn12544_port = 1;
    thiz->thb_info.spn12544_status = 0x00;
    thiz->thb_info.spn12544_ele[0] = '0';
    thiz->thb_info.spn12544_ele[1] = '2';
    thiz->thb_info.spn12544_time[0] = '0';
    thiz->thb_info.spn12544_time[1] = '2';
    memcpy(param->buff.tx_buff, &thiz->thb_info, sizeof(struct pgn12544_THB));

    param->buff_payload = gen->datalen;
    param->can_id = gen->prioriy << 26 | gen->pgn << 8 | CAN_TX_ID_MASK | CAN_EFF_FLAG;
    param->evt_param = EVT_RET_OK;
    statistics[I_THB].can_counter ++;

    return 0;
}

// TCU启动完成应答帧
int gen_packet_tcu_PGN4608(struct charge_task * thiz, struct event_struct* param)
{
    struct can_pack_generator *gen = &generator[TCU_TRSF];

    memset(param->buff.tx_buff, 0xFF, sizeof(struct pgn4608_TRSF));
	memset(&thiz->thb_info, 0xFF, sizeof(struct pgn4608_TRSF));

    thiz->trsf_info.spn4608_port = 1;
    thiz->trsf_info.spn4608_load_control = 0;
    thiz->trsf_info.spn4608_status = 0x00;
    memcpy(param->buff.tx_buff, &thiz->thb_info, sizeof(struct pgn4608_TRSF));

    param->buff_payload = gen->datalen;
    param->can_id = gen->prioriy << 26 | gen->pgn << 8 | CAN_TX_ID_MASK | CAN_EFF_FLAG;
    param->evt_param = EVT_RET_OK;
    statistics[I_TRSF].can_counter ++;

    return 0;
}

// TCU停止完成应答帧
int gen_packet_tcu_PGN5120(struct charge_task * thiz, struct event_struct* param)
{
    struct can_pack_generator *gen = &generator[TCU_TRST];

    memset(param->buff.tx_buff, 0xFF, sizeof(struct pgn5120_TRST));
	memset(&thiz->thb_info, 0xFF, sizeof(struct pgn5120_TRST));

    thiz->trst_info.spn5120_port = 1;
    thiz->trst_info.spn5120_Stop_reason = 0;
    thiz->trst_info.spn5120_status = 0x00;
    memcpy(param->buff.tx_buff, &thiz->thb_info, sizeof(struct pgn5120_TRST));

    param->buff_payload = gen->datalen;
    param->can_id = gen->prioriy << 26 | gen->pgn << 8 | CAN_TX_ID_MASK | CAN_EFF_FLAG;
    param->evt_param = EVT_RET_OK;
    statistics[I_TRST].can_counter ++;

    return 0;
}

// TCU连接确认应答帧
int gen_packet_tcu_PGN5632(struct charge_task * thiz, struct event_struct* param)
{
    struct can_pack_generator *gen = &generator[TCU_TRCT];

    memset(param->buff.tx_buff, 0xFF, sizeof(struct pgn5632_TRCT));
	memset(&thiz->trct_info, 0xFF, sizeof(struct pgn5632_TRCT));

    thiz->trct_info.spn5632_port = 1;
    thiz->trct_info.spn5632_status = 0x00;
    memcpy(param->buff.tx_buff, &thiz->thb_info, sizeof(struct pgn5632_TRCT));

    param->buff_payload = gen->datalen;
    param->can_id = gen->prioriy << 26 | gen->pgn << 8 | CAN_TX_ID_MASK | CAN_EFF_FLAG;
    param->evt_param = EVT_RET_OK;
    statistics[I_TRCT].can_counter ++;

    return 0;
}



