#ifndef _FQ_MSG_H_
#define _FQ_MSG_H_

typedef struct _msg
{
	int buf_len;
	char buf[0];
} evg_msg_t;

#endif
