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
	ERR_QUEUE_FULL 					         = -8001,    //������
	ERR_PUSH_QUEUE_FAIL 			         = -8002,    //�������ʧ��
	ERR_INVALID_REQ 				         = -8003,    //���󴮸�ʽ�Ƿ�
	ERR_REACH_MAX_MSG 				         = -8004,    //���󴮴�����󳤶�
	ERR_REACH_MIN_MSG 				         = -8005,    //����С����С����
	ERR_SEND_FAIL 					         = -8006,    //����ʧ��
	ERR_RCV_FAIL 					         = -8007,    //����ʧ��
	ERR_BASE64_ENCODE_FAILED		         = -8018,    //BASE64 ����ʧ��
	ERR_BASE64_DECODE_FAILED		         = -8019,    //BASE64 ����ʧ��

	ERR_OBJECT_DETECT_FAILED                 = -9001,   //Ŀ����ʧ��
};


#endif
