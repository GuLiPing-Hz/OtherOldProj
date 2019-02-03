#ifndef SERVER_CONF_H_
#define SERVER_CONF_H_

#include <vector>
#include <string>
using namespace std;


struct DbagentCfg
{
	string m_ipmain;
	int m_portmain;
	string m_ipbak;
	int m_portbak;
};

//外部服务器信息
struct ExServerCfg
{
	string m_ip;
	int m_port;
	int m_id;
};


class ServerConf
{
	public:
		int ReadCfg(const char* configfile);
	public:
		char m_Logdir[128];
		char m_sIP[64];
		int m_Listenport;
		int m_Udpport;
		int m_Maxclientcount;
		int m_Conntimeout;
		char m_ServerKey[17];
		int m_Runscan;
		int m_Numberonescan;
		int m_Readtimeout;				//客户端连接读取超时
		ExServerCfg m_HallsvrCfg;		//大厅服务器信息
		DbagentCfg m_DbagentCfg;		//数据库服务器信息
		DbagentCfg m_DbagentCfgGift;	//送礼用数据库
		DbagentCfg m_DbagentCfgRight;	//读取权限连接(最大数据量限制64K)
		ExServerCfg m_DbMonitor;			//监控服务器信息
		ExServerCfg m_LogSvrCfg;			//日志服务器信息
		ExServerCfg m_FactorySvrCfg;		//工厂服务器信息
};

#endif
