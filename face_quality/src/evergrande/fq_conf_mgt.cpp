#include "base_os.h"
#include "base_convert.h"
#include "base_string.h"
#include "base_xml_parser.h"
#include "base_logger.h"
#include "fq_conf_mgt.h"

using namespace std;

Logger g_logger;
StSysInfo g_sysInfo;


Conf_Mgt::Conf_Mgt(): _cfg("")
{
}


Conf_Mgt::~Conf_Mgt()
{

}


int Conf_Mgt::init(const std::string &cfg)
{
	int nRet = 0;

	_cfg = cfg;
	nRet = refresh();
	if(nRet != 0)
	{
		XCP_LOGGER_INFO(&g_logger, "refresh conf mgt falied, ret:%d\n", nRet);
		return nRet;
	}

	//启动conf mgt timer
// 	XCP_LOGGER_INFO(&g_logger, "--- prepare to start conf mgt timer ---\n");
// 	Select_Timer *timer_conf = new Select_Timer;
// 	Conf_Timer_handler *conf_thandler = new Conf_Timer_handler;
// 	nRet = timer_conf->register_timer_handler(conf_thandler, 10000000);
// 	if(nRet != 0)
// 	{
// 		XCP_LOGGER_INFO(&g_logger, "register conf mgt timer handler falied, ret:%d\n", nRet);
// 		return nRet;
// 	}
//
// 	nRet = timer_conf->init();
// 	if(nRet != 0)
// 	{
// 		XCP_LOGGER_ERROR(&g_logger, "int conf mgt timer failed, ret:%d\n", nRet);
// 		return nRet;
// 	}
//
// 	nRet = timer_conf->run();
// 	if(nRet != 0)
// 	{
// 		XCP_LOGGER_ERROR(&g_logger, "conf mgt timer run failed, ret:%d\n", nRet);
// 		return nRet;
// 	}
// 	XCP_LOGGER_INFO(&g_logger, "=== complete to start conf mgt timer ===\n");


	return nRet;

}


int Conf_Mgt::refresh()
{
	int nRet = 0;

	XML_Parser _parser;
	nRet = _parser.parse_file(_cfg);
	if(nRet != 0)
	{
		printf("init conf mgt failed, ret:%d, cfg:%s\n", nRet, _cfg.c_str());
		XCP_LOGGER_INFO(&g_logger, "init conf mgt failed, ret:%d, cfg:%s\n", nRet, _cfg.c_str());
		return nRet;
	}

	//---------------------- sysinfo ---------------------

	StSysInfo sysInfo;

	//id
	XML_Node node;
	nRet = _parser.get_node("face_quality/system/id", node);
	if(nRet != 0)
	{
		printf("get id failed, ret:%d\n", nRet);
		XCP_LOGGER_INFO(&g_logger, "get id failed, ret:%d\n", nRet);
		return -1;
	}
	sysInfo._id = node.get_text();
	trim(sysInfo._id);
	if(sysInfo._id == "")
	{
		printf("id is empty\n");
		XCP_LOGGER_INFO(&g_logger, "id is empty\n");
		return -1;
	}

	//ip
	nRet = _parser.get_node("face_quality/system/ip", node);
	if(nRet != 0)
	{
		printf("get ip failed, ret:%d\n", nRet);
		XCP_LOGGER_INFO(&g_logger, "get ip failed, ret:%d\n", nRet);
		return -1;
	}
	sysInfo._ip = node.get_text();
	trim(sysInfo._ip);
	if(sysInfo._ip == "")
	{
		printf("ip is empty\n");
		XCP_LOGGER_INFO(&g_logger, "ip is empty\n");
		return -1;
	}


	//ip_out
	nRet = _parser.get_node("face_quality/system/ip_out", node);
	if(nRet != 0)
	{
		printf("get ip_out failed, ret:%d\n", nRet);
		XCP_LOGGER_INFO(&g_logger, "get ip_out failed, ret:%d\n", nRet);
		sysInfo._ip_out = sysInfo._ip;
	}
	else
	{
		sysInfo._ip_out = node.get_text();
		trim(sysInfo._ip_out);
		if(sysInfo._ip_out == "")
		{
			printf("ip_out is empty\n");
			XCP_LOGGER_INFO(&g_logger, "ip_out is empty\n");
			return -1;
		}
	}


	//port
	nRet = _parser.get_node("face_quality/system/port", node);
	if(nRet != 0)
	{
		printf("get port failed, ret:%d\n", nRet);
		XCP_LOGGER_INFO(&g_logger, "get port failed, ret:%d\n", nRet);
		return -1;
	}
	else
	{
		sysInfo._port = (unsigned short)atoll(node.get_text().c_str());
		if(sysInfo._port == 0)
		{
			printf("port is 0\n");
			XCP_LOGGER_INFO(&g_logger, "port is 0\n");
			return -1;
		}
	}

	//Message ID命名规则： [svr id]_[ip_out]_[port]
	sysInfo._log_id = base::format("%s_%s_%u", sysInfo._id.c_str(), sysInfo._ip_out.c_str(), sysInfo._port);
	sysInfo._new_id = base::format("%s_%llu_%d", sysInfo._log_id.c_str(), getTimestamp(), get_pid());

	//thr_num
	nRet = _parser.get_node("face_quality/system/thr_num", node);
	if(nRet != 0)
	{
		printf("get thr_num failed, ret:%d\n", nRet);
		XCP_LOGGER_INFO(&g_logger, "get thr_num failed, ret:%d\n", nRet);
		get_cpu_number_proc(sysInfo._thr_num);
	}
	else
	{
		sysInfo._thr_num = (unsigned short)atoll(node.get_text().c_str());
		if(sysInfo._thr_num == 0)
		{
			printf("thr_num is 0\n");
			XCP_LOGGER_INFO(&g_logger, "thr_num is 0\n");
			return -1;
		}
	}


	//max_queue_size
	nRet = _parser.get_node("face_quality/system/max_queue_size", node);
	if(nRet != 0)
	{
		printf("get max_queue_size failed, ret:%d\n", nRet);
		XCP_LOGGER_INFO(&g_logger, "get max_queue_size failed, ret:%d\n", nRet);
	}
	sysInfo._max_queue_size = (unsigned int)atoll(node.get_text().c_str());
	if(sysInfo._max_queue_size < 100000)
	{
		printf("max_queue_size < 100000\n");
		XCP_LOGGER_INFO(&g_logger, "max_queue_size < 100000\n");
		sysInfo._max_queue_size = 100000;
	}

	//TZ
	nRet = _parser.get_node("face_quality/system/TZ", node);
	if(nRet != 0)
	{
		printf("get TZ failed, ret:%d\n", nRet);
	}
	else
	{
		sysInfo._TZ = node.get_text();
	}

	if(true)
	{
		Thread_Mutex_Guard guard(_mutex);
		_sysInfo = sysInfo;
	}

	return 0;
}


//获取副本
StSysInfo Conf_Mgt::get_sysinfo()
{
	Thread_Mutex_Guard guard(_mutex);
	return _sysInfo;
}

