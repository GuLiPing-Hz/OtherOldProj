#include "serverconf.h"
#include <ac/xml/tinyxml.h>
#include <ac/log/log.h>

int ServerConf::ReadCfg(const char* configfile)
{
	TiXmlDocument doc(configfile);
	//TiXmlDocument doc("../etc/roomserver.xml");
	bool ret = doc.LoadFile();
	if (!ret)
		return -1;

	TiXmlElement *root = doc.FirstChildElement("config");
	if(!root)
		return -1;

	TiXmlElement *servercfg = root->FirstChildElement("servercfg");
	if(!servercfg)
		return -1;

	const char* str = servercfg->Attribute("logdir");
	if(!str)
	{
		AC_ERROR("read logdir error");
		return -1;
	}
	AC_INFO("logdir = %s",str);
	strncpy(m_Logdir,str,sizeof(m_Logdir));

	str = servercfg->Attribute("ip");
	if(!str)
	{
		AC_ERROR("read ip error");
		return -1;
	}
	AC_INFO("ip = %s",str);
	strncpy(m_sIP,str,sizeof(m_sIP));

	str = servercfg->Attribute("listen", &m_Listenport);
	if(!str)
	{
		AC_ERROR("read listen error");
		return -1;
	}
	AC_INFO("listen = %d",m_Listenport);

	str = servercfg->Attribute("udp", &m_Udpport);
	if(!str)
	{
		AC_ERROR("read udp error");
		return -1;
	}
	AC_INFO("udp = %d",m_Udpport);
	
	str = servercfg->Attribute("maxclient", &m_Maxclientcount);
	if(!str)
	{
		AC_ERROR("read maxclient error");
		return -1;
	}
	AC_INFO("maxclient = %d",m_Maxclientcount);
	
	str = servercfg->Attribute("conntimeout",&m_Conntimeout);
	if(!str)
	{
		AC_ERROR("read conntimeout error");
		return -1;
	}
	AC_INFO("conntimeout = %d",m_Conntimeout);

	str = servercfg->Attribute("serverkey");
	if(!str)
	{
		AC_ERROR("read serverkey error");
		return -1;
	}
	strncpy(m_ServerKey, str,  sizeof(m_ServerKey));
	AC_INFO("ServerKey = %s",m_ServerKey);

	str = servercfg->Attribute("runscan",&m_Runscan);
	if(!str)
	{
		AC_ERROR("read runscan error");
		return -1;
	}
	AC_INFO("runscan = %d",m_Runscan);

	str = servercfg->Attribute("numberonescan",&m_Numberonescan);
	if(!str)
	{
		AC_ERROR("read numberonescan error");
		return -1;
	}
	AC_INFO("numberonescan = %d",m_Numberonescan);

	str = servercfg->Attribute("readtimeout",&m_Readtimeout);
	if(!str)
	{
		AC_ERROR("read readtimeout error");
		return -1;
	}
	AC_INFO("readtimeout = %d",m_Readtimeout);
	

	TiXmlElement *hallsvrcfg = root->FirstChildElement("hallsvrcfg");
	if(!hallsvrcfg)
	{
		AC_ERROR("read hallsvrcfg error");
		return -1;
	}
	str = hallsvrcfg->Attribute("ip");
	if(!str)
	{
		AC_ERROR("read hallsvr ip error");
		return -1;
	}
	m_HallsvrCfg.m_ip.append(str);

	str = hallsvrcfg->Attribute("port",&m_HallsvrCfg.m_port);
	if(!str)
	{
		AC_ERROR("read hallsvr port error");
		return -1;
	}

	str = hallsvrcfg->Attribute("id",&m_HallsvrCfg.m_id);
	if(!str)
	{
		AC_ERROR("read hallsvr id error");
		return -1;
	}
	AC_INFO("hallsvr ip = %s,port = %d,id = %d",
		m_HallsvrCfg.m_ip.c_str(),m_HallsvrCfg.m_port,m_HallsvrCfg.m_id);

	TiXmlElement *dbagentcfg = root->FirstChildElement("dbagentcfg");
	if(!dbagentcfg)
	{
		AC_ERROR("read dbagent error");
		return -1;
	}
	str = dbagentcfg->Attribute("ipmain");
	if(!str)
	{
		AC_ERROR("read dbagent ipmain error");
		return -1;
	}
	m_DbagentCfg.m_ipmain.append(str);

	str = dbagentcfg->Attribute("portmain",&m_DbagentCfg.m_portmain);
	if(!str)
	{
		AC_ERROR("read dbagent portmain error");
		return -1;
	}

	str = dbagentcfg->Attribute("ipbak");
	if(!str)
	{
		AC_ERROR("read dbagent ipbak error");
		return -1;
	}
	m_DbagentCfg.m_ipbak.append(str);

	str = dbagentcfg->Attribute("portbak",&m_DbagentCfg.m_portbak);
	if(!str)
	{
		AC_ERROR("read dbagent portbak error");
		return -1;
	}
	AC_INFO("dbagent ipmain = %s,portmain = %d,ipbak = %s,portbak = %d",
		m_DbagentCfg.m_ipmain.c_str(),m_DbagentCfg.m_portmain,m_DbagentCfg.m_ipbak.c_str(),
		m_DbagentCfg.m_portbak);

	//送礼物专用DB
	TiXmlElement *dbagentcfgGift = root->FirstChildElement("dbagentcfgGift");
	if(!dbagentcfgGift)
	{
		AC_ERROR("read dbagentcfgGift error");
		return -1;
	}
	str = dbagentcfgGift->Attribute("ipmain");
	if(!str)
	{
		AC_ERROR("read dbagentcfgGift ipmain error");
		return -1;
	}
	m_DbagentCfgGift.m_ipmain.append(str);

	str = dbagentcfgGift->Attribute("portmain",&m_DbagentCfgGift.m_portmain);
	if(!str)
	{
		AC_ERROR("read dbagentcfgGift portmain error");
		return -1;
	}

	str = dbagentcfgGift->Attribute("ipbak");
	if(!str)
	{
		AC_ERROR("read dbagentcfgGift ipbak error");
		return -1;
	}
	m_DbagentCfgGift.m_ipbak.append(str);

	str = dbagentcfgGift->Attribute("portbak",&m_DbagentCfgGift.m_portbak);
	if(!str)
	{
		AC_ERROR("read dbagentcfgGift portbak error");
		return -1;
	}
	AC_INFO("dbagentcfgGift ipmain = %s,portmain = %d,ipbak = %s,portbak = %d",
		m_DbagentCfgGift.m_ipmain.c_str(),m_DbagentCfgGift.m_portmain,m_DbagentCfgGift.m_ipbak.c_str(),
		m_DbagentCfgGift.m_portbak);

	//读取权限配置数据库连接信息
	TiXmlElement *dbagentcfgRight = root->FirstChildElement("dbagentcfgRight");
	if(!dbagentcfgRight)
	{
		AC_ERROR("read dbagentcfgRight error");
		return -1;
	}
	str = dbagentcfgRight->Attribute("ipmain");
	if(!str)
	{
		AC_ERROR("read dbagentcfgRight ipmain error");
		return -1;
	}
	m_DbagentCfgRight.m_ipmain.append(str);

	str = dbagentcfgRight->Attribute("portmain",&m_DbagentCfgRight.m_portmain);
	if(!str)
	{
		AC_ERROR("read dbagentcfgRight portmain error");
		return -1;
	}

	AC_INFO("dbagentcfgRight ipmain = %s,portmain = %d",
		m_DbagentCfgRight.m_ipmain.c_str(),m_DbagentCfgRight.m_portmain);
	
	TiXmlElement *dbmonitor = root->FirstChildElement("dbmonitor");
	if(!dbmonitor)
	{
		AC_ERROR("read dbmonitor error");
		return -1;
	}
	str = dbmonitor->Attribute("ip");
	if(!str)
	{
		AC_ERROR("read dbmonitor ip error");
		return -1;
	}
	m_DbMonitor.m_ip.append(str);

	str = dbmonitor->Attribute("port",&m_DbMonitor.m_port);
	if(!str)
	{
		AC_ERROR("read dbmonitor port error");
		return -1;
	}

	str = dbmonitor->Attribute("id",&m_DbMonitor.m_id);
	if(!str)
	{
		AC_ERROR("read dbmonitor id error");
		return -1;
	}
	AC_INFO("dbmonitor ip = %s,port = %d,id = %d",
		m_DbMonitor.m_ip.c_str(),m_DbMonitor.m_port,m_DbMonitor.m_id);

	//add by jinguanfu 2010/12/29
	TiXmlElement *LogServer = root->FirstChildElement("logserver");
	if(!LogServer)
	{
		AC_ERROR("read logserver error");
		return -1;
	}
	str = LogServer->Attribute("ip");
	if(!str)
	{
		AC_ERROR("read logserver ip error");
		return -1;
	}
	m_LogSvrCfg.m_ip.append(str);

	str = LogServer->Attribute("port",&m_LogSvrCfg.m_port);
	if(!str)
	{
		AC_ERROR("read logserver port error");
		return -1;
	}

	AC_INFO("logserver ip = %s,port = %d",
		m_LogSvrCfg.m_ip.c_str(),m_LogSvrCfg.m_port);

	//add by jinguanfu 2011/3/10
	TiXmlElement *FactoryServer = root->FirstChildElement("factoryserver");
	if(!FactoryServer)
	{
		AC_ERROR("read factoryserver error");
		return -1;
	}
	str = FactoryServer->Attribute("ip");
	if(!str)
	{
		AC_ERROR("read factoryserver ip error");
		return -1;
	}
	m_FactorySvrCfg.m_ip.append(str);

	str = FactoryServer->Attribute("port",&m_FactorySvrCfg.m_port);
	if(!str)
	{
		AC_ERROR("read factoryserver port error");
		return -1;
	}

	AC_INFO("factoryserver ip = %s,port = %d",
		m_FactorySvrCfg.m_ip.c_str(),m_FactorySvrCfg.m_port);
	
	return 0;
}

