#ifndef __COMMON_H__
#define __COMMON_H__


//#define CAL_TIME 1

#define DEBUG 1

#ifdef DEBUG
#define dbg(fmt, args...)  printf(fmt, ##args)
#else
#define dbg(fmt, args...) do{}while(0)
#endif

enum EnErrCode
{
	ERR_SUCCESS = 0,
	ERR_SYSTEM = -1,

	//Common
	ERR_COMMON_BEGIN 				         = -8000,
	ERR_QUEUE_FULL 					         = -8001,    //队列满
	ERR_PUSH_QUEUE_FAIL 			         = -8002,    //插入队列失败
	ERR_INVALID_REQ 				         = -8003,    //请求串格式非法
	ERR_REACH_MAX_MSG 				         = -8004,    //请求串大于最大长度
	ERR_REACH_MIN_MSG 				         = -8005,    //请求串小于最小长度
	ERR_SEND_FAIL 					         = -8006,    //发送失败
	ERR_RCV_FAIL 					         = -8007,    //接收失败
	ERR_BASE64_ENCODE_FAILED		         = -8018,    //BASE64 编码失败
	ERR_BASE64_DECODE_FAILED		         = -8019,    //BASE64 解码失败

	ERR_OBJECT_DETECT_FAILED                 = -9001,   //目标检测失败
};


#endif
