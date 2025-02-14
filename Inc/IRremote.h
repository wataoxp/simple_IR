/*
 * IRremote.h
 *
 *  Created on: Nov 16, 2024
 *      Author: wataoxp
 */

#ifndef INC_IRREMOTE_H_
#define INC_IRREMOTE_H_

#include "main.h"

#define IR_DECODE_READY 0
#define IR_ACK 0x06
#define IR_NACK 0x15
#define IR_REPEAT 0x20

/* Falling Edge Exti Timing */
//#define IR_MIN_TIMING 10000 //10000us.RepeatCode Limit
//#define IR_MAX_TIMING 14000 //14000us.LeaderCode Limit
//#define IR_LEADER_TIMING 13000 //13000us.LeaderCode
//#define IR_REPEAT_TIMING 11000 //11000us.RepeatCode
//#define NEC_HIGH 2000
//#define NEC_LOW 1400

#define IR_MIN_TIMING 1000 //10000us.RepeatCode Limit
#define IR_MAX_TIMING 1400 //14000us.LeaderCode Limit
#define IR_LEADER_TIMING 1300 //13000us.LeaderCode
#define IR_REPEAT_TIMING 1100 //11000us.RepeatCode
#define NEC_HIGH 200
#define NEC_LOW 140

/* Polling Status */
/* 割り込みとは異なり、StartCodeのLow期間の後を測る */
#define POLLING_REP	200		//ShortPause.2ms
#define POLLING_DATA 400	//LongPause.4ms

typedef enum{
	StartLeader,
	EndLeader,
	ReadTime,
	StopIR,
}IRstatus;

typedef struct{
	TIM_TypeDef *TIM;
	IRQn_Type IRQnumber;
	uint16_t *Pulse;
	uint8_t *Flag;
	uint32_t ExtiLine;
}IR_Parameter;

typedef struct{
	uint8_t bit31;
	uint8_t bit24;
	uint8_t bit16;
	uint8_t bit8;
}ConvertLSB;

void SetPolling(TIM_TypeDef *TIM,GPIO_TypeDef *port,uint32_t pin);
void SetISR(IR_Parameter *IR);
void EnableIR(void);
void DisableIR(void);
uint32_t RecieveIR(void);
void RecieveIR_IT(void);
uint32_t DecodeIR(uint16_t *Pulse);
void BinaryToHex(ConvertLSB *LSB,uint32_t Binary);
void DataReset(ConvertLSB *LSB,uint32_t *Binary);

/* ***NECフォーマットについて ***
 * 本プログラムはNECフォーマットの赤外線リモコンを想定したものになっています。
 * また赤外線受信においてはOSRB38C99Aを利用しています。
 * 同モジュールにおいては送信側の赤外線が「点灯」したときに「Low」が出力されます。
 * ですので特に割り込みを使う場合、立ち下がりエッジを検出することで1周期の時間をチェックするようにします。
 *
 * NECフォーマットは以下の形で信号が送信されます。
 *
 * LeaderCode	通信の開始を伝える
 * 9000us(9ms)間のLow、4000us(4ms)のHigh
 * 割り込みを使う場合、初回の立ち下がりから次の立ち下がりまでの時間が13000us程度であれば正しいデータと判断できます。
 *
 * AｄｄｒｅｓｓCｏｄｅ 送信側個体のアドレスを伝える。16ビット
 *
 * DataCode リモコンであればどのボタンが押されたかの情報を伝える。16ビット
 *
 * Address、Dataともに0と1は以下の周期で判定できます。
 * Low時間は共通で約600us
 * High時間は 0のとき500us。1のとき1600us。
 * なので1ビットずつ周期を測り、2000us以上であれば1であると判断するようにしています。
 * */

#endif /* INC_IRREMOTE_H_ */
