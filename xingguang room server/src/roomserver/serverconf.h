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

//�ⲿ��������Ϣ
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
		int m_Readtimeout;				//�ͻ������Ӷ�ȡ��ʱ
		ExServerCfg m_HallsvrCfg;		//������������Ϣ
		DbagentCfg m_DbagentCfg;		//���ݿ��������Ϣ
		DbagentCfg m_DbagentCfgGift;	//���������ݿ�
		DbagentCfg m_DbagentCfgRight;	//��ȡȨ������(�������������64K)
		ExServerCfg m_DbMonitor;			//��ط�������Ϣ
		ExServerCfg m_LogSvrCfg;			//��־��������Ϣ
		ExServerCfg m_FactorySvrCfg;		//������������Ϣ
};

#endif
