#pragma once

#ifdef __cpluscplus
extern "C" {
#endif



typedef enum
{
	ELed_R,	//红灯
	ELed_Y,	//黄灯
	ELed_G,	//绿灯
} ELed_t;

typedef enum
{
	ELED_OP_ON = 0x0,		//亮（默认）
	ELED_OP_FLASH_SLOW = 0x1,	//慢闪
	ELED_OP_FLASH_FAST = 0x2,	//快闪
	ELED_OP_OFF = 0x3,		//灭
} ELedOp_t;




#ifdef __cpluscplus
}
#endif

