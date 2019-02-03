#include "rightconfig.h"
#include <ac/xml/tinyxml.h>
#include <ac/log/log.h>
#include "roomtype.h"

int RightConfig::ReadCfg(const char* configfile)
{
	TiXmlDocument doc(configfile);
	//TiXmlDocument doc("../etc/rightconfig.xml");
	bool ret = doc.LoadFile();
	if (!ret)
	{
		AC_ERROR("RightConfig::ReadCfg: LoadFile error");
		return -1;
	}

	TiXmlElement *root = doc.FirstChildElement("roomright");
	if(!root)
	{
		AC_ERROR("RightConfig::ReadCfg: read load error");
		return -1;
	}

	TiXmlElement *child = root->FirstChildElement("unpasswd");
	if(child)
	{
		if(ReadDailyCfg(child, m_unpasswd)==-1)
		{
			AC_ERROR("RightConfig::ReadCfg: read unpasswd error");
			return -1;
		}
		
	}
	else
	{
		AC_ERROR("RightConfig::ReadCfg: TiXmlElement unpasswd is null");
		return -1;
	}

	child = root->FirstChildElement("closed");
	if(child)
	{
		if(ReadDailyCfg(child, m_closed)==-1)
		{
			AC_ERROR("RightConfig::ReadCfg: read closed error");
			return -1;
		}
		
	}
	else
	{
		AC_ERROR("RightConfig::ReadCfg: TiXmlElement closed is null");
		return -1;
	}
	
	child = root->FirstChildElement("private");
	if(child)
	{
		if(ReadDailyCfg(child, m_private)==-1)
		{
			AC_ERROR("RightConfig::ReadCfg: read private error");
			return -1;
		}
		
	}
	else
	{
		AC_ERROR("RightConfig::ReadCfg: TiXmlElement private is null");
		return -1;
	}

	child = root->FirstChildElement("fulllimit");
	if(child)
	{
		if(ReadDailyCfg(child, m_fulllimit)==-1)
		{
			AC_ERROR("RightConfig::ReadCfg: read fulllimit error");
			return -1;
		}
		
	}
	else
	{
		AC_ERROR("RightConfig::ReadCfg: TiXmlElement fulllimit is null");
		return -1;
	}

	child = root->FirstChildElement("onvjmic");
	if(child)
	{
		if(ReadDailyCfg(child, m_onvjmic)==-1)
		{
			AC_ERROR("RightConfig::ReadCfg: read onvjmic error");
			return -1;
		}
		
	}
	else
	{
		AC_ERROR("RightConfig::ReadCfg: TiXmlElement onvjmic is null");
		return -1;
	}

	child = root->FirstChildElement("updownwaitmic");
	if(child)
	{
		if(ReadDailyCfg(child, m_updownwaitmic)==-1)
		{
			AC_ERROR("RightConfig::ReadCfg: read updownwaitmic error");
			return -1;
		}
		
	}
	else
	{
		AC_ERROR("RightConfig::ReadCfg: TiXmlElement updownwaitmic is null");
		return -1;
	}

	child = root->FirstChildElement("freewaitmic");
	if(child)
	{
		if(ReadDailyCfg(child, m_freewaitmic)==-1)
		{
			AC_ERROR("RightConfig::ReadCfg: read freewaitmic error");
			return -1;
		}
		
	}
	else
	{
		AC_ERROR("RightConfig::ReadCfg: TiXmlElement freewaitmic is null");
		return -1;
	}

	child = root->FirstChildElement("forbiden");
	if(child)
	{
		if(ReadDailyCfg(child, m_forbiden)==-1)
		{
			AC_ERROR("RightConfig::ReadCfg: read forbiden error");
			return -1;
		}
		
	}
	else
	{
		AC_ERROR("RightConfig::ReadCfg: TiXmlElement forbiden is null");
		return -1;
	}
	
	child = root->FirstChildElement("black");
	if(child)
	{
		if(ReadDailyCfg(child, m_black)==-1)
		{
			AC_ERROR("RightConfig::ReadCfg: read black error");
			return -1;
		}
		
	}
	else
	{
		AC_ERROR("RightConfig::ReadCfg: TiXmlElement black is null");
		return -1;
	}

	child = root->FirstChildElement("kick");
	if(child)
	{
		if(ReadDailyCfg(child, m_kick)==-1)
		{
			AC_ERROR("RightConfig::ReadCfg: read kick error");
			return -1;
		}
		
	}
	else
	{
		AC_ERROR("RightConfig::ReadCfg: TiXmlElement kick is null");
		return -1;
	}

	child = root->FirstChildElement("giveonvjmic");
	if(child)
	{
		if(ReadDailyCfg(child, m_giveonvjmic)==-1)
		{
			AC_ERROR("RightConfig::ReadCfg: read giveonvjmic error");
			return -1;
		}
		
	}
	else
	{
		AC_ERROR("RightConfig::ReadCfg: TiXmlElement giveonvjmic is null");
		return -1;
	}

	child = root->FirstChildElement("giveoffvjmic");
	if(child)
	{
		if(ReadDailyCfg(child, m_giveoffvjmic)==-1)
		{
			AC_ERROR("RightConfig::ReadCfg: read giveoffvjmic error");
			return -1;
		}
		
	}
	else
	{
		AC_ERROR("RightConfig::ReadCfg: TiXmlElement giveonvjmic is null");
		return -1;
	}
	
	
	child = root->FirstChildElement("giveoffmic");
	if(child)
	{
		if(ReadDailyCfg(child, m_giveoffmic)==-1)
		{
			AC_ERROR("RightConfig::ReadCfg: read giveoffmic error");
			return -1;
		}
		
	}
	else
	{
		AC_ERROR("RightConfig::ReadCfg: TiXmlElement giveoffmic is null");
		return -1;
	}

	child = root->FirstChildElement("delwaitmic");
	if(child)
	{
		if(ReadDailyCfg(child, m_delwaitmic)==-1)
		{
			AC_ERROR("RightConfig::ReadCfg: read delwaitmic error");
			return -1;
		}
		
	}
	else
	{
		AC_ERROR("RightConfig::ReadCfg: TiXmlElement delwaitmic is null");
		return -1;
	}
	

	child = root->FirstChildElement("invitewaitmic");
	if(child)
	{
		if(ReadDailyCfg(child, m_invitewaitmic)==-1)
		{
			AC_ERROR("RightConfig::ReadCfg: read invitewaitmic error");
			return -1;
		}
		
	}
	else
	{
		AC_ERROR("RightConfig::ReadCfg: TiXmlElement invitewaitmic is null");
		return -1;
	}


	child = root->FirstChildElement("roomchat");
	if(child)
	{
		if(ReadDailyCfg(child, m_roomchat)==-1)
		{
			AC_ERROR("RightConfig::ReadCfg: read roomchat error");
			return -1;
		}
		
	}
	else
	{
		AC_ERROR("RightConfig::ReadCfg: TiXmlElement roomchat is null");
		return -1;
	}

	//房间管理
	child = root->FirstChildElement("getapplylist");
	if(child)
	{
		if(ReadMangerCfg(child, m_getapplylist)==-1)
		{
			AC_ERROR("RightConfig::ReadCfg: read getapplylist error");
			return -1;
		}
		
	}
	else
	{
		AC_ERROR("RightConfig::ReadCfg: TiXmlElement getapplylist is null");
		return -1;
	}

	child = root->FirstChildElement("auditapply");
	if(child)
	{
		if(ReadMangerCfg(child, m_auditapply)==-1)
		{
			AC_ERROR("RightConfig::ReadCfg: read auditapply error");
			return -1;
		}
		
	}
	else
	{
		AC_ERROR("RightConfig::ReadCfg: TiXmlElement auditapply is null");
		return -1;
	}
	
	child = root->FirstChildElement("getmemberlist");
	if(child)
	{
		if(ReadMangerCfg(child, m_getmemberlist)==-1)
		{
			AC_ERROR("RightConfig::ReadCfg: read getmemberlist error");
			return -1;
		}
		
	}
	else
	{
		AC_ERROR("RightConfig::ReadCfg: TiXmlElement getmemberlist is null");
		return -1;
	}

	child = root->FirstChildElement("getblacklist");
	if(child)
	{
		if(ReadMangerCfg(child, m_getblacklist)==-1)
		{
			AC_ERROR("RightConfig::ReadCfg: read getblacklist error");
			return -1;
		}
	}
	else
	{
		AC_ERROR("RightConfig::ReadCfg: TiXmlElement getblacklist is null");
		return -1;
	}

	child = root->FirstChildElement("setpassword");
	if(child)
	{
		if(ReadMangerCfg(child, m_setpassword)==-1)
		{
			AC_ERROR("RightConfig::ReadCfg: read setpassword error");
			return -1;
		}
	}
	else
	{
		AC_ERROR("RightConfig::ReadCfg: TiXmlElement setpassword is null");
		return -1;
	}	

	child = root->FirstChildElement("setroomclose");
	if(child)
	{
		if(ReadMangerCfg(child, m_setroomclose)==-1)
		{
			AC_ERROR("RightConfig::ReadCfg: read setroomclose error");
			return -1;
		}
	}
	else
	{
		AC_ERROR("RightConfig::ReadCfg: TiXmlElement setroomclose is null");
		return -1;
	}	

	child = root->FirstChildElement("setroomprivate");
	if(child)
	{
		if(ReadMangerCfg(child, m_setroomprivate)==-1)
		{
			AC_ERROR("RightConfig::ReadCfg: read setroomprivate error");
			return -1;
		}
	}
	else
	{
		AC_ERROR("RightConfig::ReadCfg: TiXmlElement setroomprivate is null");
		return -1;
	}	

	child = root->FirstChildElement("setroominout");
	if(child)
	{
		if(ReadMangerCfg(child, m_setroominout)==-1)
		{
			AC_ERROR("RightConfig::ReadCfg: read setroominout error");
			return -1;
		}
	}
	else
	{
		AC_ERROR("RightConfig::ReadCfg: TiXmlElement setroominout is null");
		return -1;
	}	

	child = root->FirstChildElement("setfreewaitmic");
	if(child)
	{
		if(ReadMangerCfg(child, m_setfreewaitmic)==-1)
		{
			AC_ERROR("RightConfig::ReadCfg: read setfreewaitmic error");
			return -1;
		}
	}
	else
	{
		AC_ERROR("RightConfig::ReadCfg: TiXmlElement setfreewaitmic is null");
		return -1;
	}

	child = root->FirstChildElement("setroomname");
	if(child)
	{
		if(ReadMangerCfg(child, m_setroomname)==-1)
		{
			AC_ERROR("RightConfig::ReadCfg: read setroomname error");
			return -1;
		}
	}
	else
	{
		AC_ERROR("RightConfig::ReadCfg: TiXmlElement setroomname is null");
		return -1;
	}

	child = root->FirstChildElement("setroomaffiche");
	if(child)
	{
		if(ReadMangerCfg(child, m_setroomaffiche)==-1)
		{
			AC_ERROR("RightConfig::ReadCfg: read setroomaffiche error");
			return -1;
		}
	}
	else
	{
		AC_ERROR("RightConfig::ReadCfg: TiXmlElement setroomaffiche is null");
		return -1;
	}

	child = root->FirstChildElement("sendnotice");
	if(child)
	{
		if(ReadMangerCfg(child, m_sendnotice)==-1)
		{
			AC_ERROR("RightConfig::ReadCfg: read sendnotice error");
			return -1;
		}
	}
	else
	{
		AC_ERROR("RightConfig::ReadCfg: TiXmlElement sendnotice is null");
		return -1;
	}

	child = root->FirstChildElement("setroomchat");
	if(child)
	{
		if(ReadMangerCfg(child, m_setroomchat)==-1)
		{
			AC_ERROR("RightConfig::ReadCfg: read setroomchat error");
			return -1;
		}
	}
	else
	{
		AC_ERROR("RightConfig::ReadCfg: TiXmlElement setroomchat is null");
		return -1;
	}

	child = root->FirstChildElement("setroomwelcome");
	if(child)
	{
		if(ReadMangerCfg(child, m_setroomwelcome)==-1)
		{
			AC_ERROR("RightConfig::ReadCfg: read setroomwelcome error");
			return -1;
		}
	}
	else
	{
		AC_ERROR("RightConfig::ReadCfg: TiXmlElement setroomwelcome is null");
		return -1;
	}

	child = root->FirstChildElement("setroomlogo");
	if(child)
	{
		if(ReadMangerCfg(child, m_setroomlogo)==-1)
		{
			AC_ERROR("RightConfig::ReadCfg: read setroomlogo error");
			return -1;
		}
	}
	else
	{
		AC_ERROR("RightConfig::ReadCfg: TiXmlElement setroomlogo is null");
		return -1;
	}

	child = root->FirstChildElement("delmember");
	if(child)
	{
		if(ReadMangerCfg(child, m_delmember)==-1)
		{
			AC_ERROR("RightConfig::ReadCfg: read delmember error");
			return -1;
		}
	}
	else
	{
		AC_ERROR("RightConfig::ReadCfg: TiXmlElement delmember is null");
		return -1;
	}

	child = root->FirstChildElement("setmember");
	if(child)
	{
		if(ReadMangerCfg(child, m_setmember)==-1)
		{
			AC_ERROR("RightConfig::ReadCfg: read setmember error");
			return -1;
		}
	}
	else
	{
		AC_ERROR("RightConfig::ReadCfg: TiXmlElement setmember is null");
		return -1;
	}

	child = root->FirstChildElement("setvja");
	if(child)
	{
		if(ReadMangerCfg(child, m_setvja)==-1)
		{
			AC_ERROR("RightConfig::ReadCfg: read setvja error");
			return -1;
		}
	}
	else
	{
		AC_ERROR("RightConfig::ReadCfg: TiXmlElement setvja is null");
		return -1;
	}
	
	child = root->FirstChildElement("setvj");
	if(child)
	{
		if(ReadMangerCfg(child, m_setvj)==-1)
		{
			AC_ERROR("RightConfig::ReadCfg: read setvj error");
			return -1;
		}
	}
	else
	{
		AC_ERROR("RightConfig::ReadCfg: TiXmlElement setvj is null");
		return -1;
	}

	child = root->FirstChildElement("setsubonwer");
	if(child)
	{
		if(ReadMangerCfg(child, m_setsubonwer)==-1)
		{
			AC_ERROR("RightConfig::ReadCfg: read setsubonwer error");
			return -1;
		}
	}
	else
	{
		AC_ERROR("RightConfig::ReadCfg: TiXmlElement setsubonwer is null");
		return -1;
	}
	
	return 0;
}

/**********************************
*读取无被操作者的权限配置
**********************************/
int RightConfig::ReadDailyCfg(TiXmlElement *pElement,char right[16])
{

	if(!pElement)
		return -1;
	if(!right)
		return -1;
	
	TiXmlElement* child = pElement->FirstChildElement("GM");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadDailyCfg: read GM error");
		return -1;
	}
		
	const char* str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR("RightConfig::ReadMangerCfg: read property error");
		return -1;
	}
	right[USER_ID_GM]=atoi(str);


	child = pElement->FirstChildElement("owner");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadMangerCfg:  read owner error");
		return -1;
	}
		
	str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR("RightConfig::ReadMangerCfg:  read property error");
		return -1;
	}
	right[USER_ID_OWNER]=atoi(str);

	child = pElement->FirstChildElement("subowner");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadMangerCfg:  read subowner error");
		return -1;
	}
		
	str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR(" RightConfig::ReadMangerCfg: read property error");
		return -1;
	}
	right[USER_ID_OWNER_S]=atoi(str);

	child = pElement->FirstChildElement("VJ");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadMangerCfg:  read VJ error");
		return -1;
	}
		
	str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR(" RightConfig::ReadMangerCfg: read property error");
		return -1;
	}
	right[USER_ID_VJ]=atoi(str);

	child = pElement->FirstChildElement("VJA");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadMangerCfg:  read VJA error");
		return -1;
	}
		
	str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR(" RightConfig::ReadMangerCfg: read property error");
		return -1;
	}
	right[USER_ID_VJ_A]=atoi(str);

	child = pElement->FirstChildElement("MEMBER");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadMangerCfg:  read MEMBER error");
		return -1;
	}
		
	str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR(" RightConfig::ReadMangerCfg: read property error");
		return -1;
	}
	right[USER_ID_VIP]=atoi(str);

	child = pElement->FirstChildElement("VIP");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadMangerCfg:  read VIP error");
		return -1;
	}
		
	str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR(" RightConfig::ReadMangerCfg: read property error");
		return -1;
	}
	right[USER_ID_GVIP]=atoi(str);

	child = pElement->FirstChildElement("RED");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadMangerCfg:  read RED error");
		return -1;
	}
		
	str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR(" RightConfig::ReadMangerCfg: read property error");
		return -1;
	}
	right[USER_ID_RED]=atoi(str);

	child = pElement->FirstChildElement("PURPLE");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadMangerCfg:  read PURPLE error");
		return -1;
	}
		
	str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR(" RightConfig::ReadMangerCfg: read property error");
		return -1;
	}
	right[USER_ID_PURPLE]=atoi(str);

	child = pElement->FirstChildElement("SUPER");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadMangerCfg:  read SUPER error");
		return -1;
	}
		
	str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR(" RightConfig::ReadMangerCfg: read property error");
		return -1;
	}
	right[USER_ID_SUPER]=atoi(str);

	child = pElement->FirstChildElement("IMPERIAL");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadMangerCfg:  read IMPERIAL error");
		return -1;
	}
		
	str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR(" RightConfig::ReadMangerCfg: read property error");
		return -1;
	}
	right[USER_ID_IMPERIAL]=atoi(str);

	child = pElement->FirstChildElement("USER");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadMangerCfg:  read USER error");
		return -1;
	}
		
	str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR(" RightConfig::ReadMangerCfg: read property error");
		return -1;
	}
	right[USER_ID_NONE]=atoi(str);
	
	return 0;
}

/**********************************
*读取有被操作者的权限配置
**********************************/
int RightConfig::ReadDailyCfg(TiXmlElement *pElement,char right[16][16])
{

	if(!pElement)
		return -1;
	if(!right)
		return -1;
	
	TiXmlElement* child = pElement->FirstChildElement("GM");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadDailyCfg: read GM error");
		return -1;
	}
		
	const char* str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR("RightConfig::ReadDailyCfg: read property error");
		return -1;
	}
	char property=atoi(str);

	if(property==1)
	{
		if(ReadDailyCfg(child,right[USER_ID_GM])==-1)
		{
			AC_ERROR("RightConfig::ReadDailyCfg: read property error");
			return -1;
		}
	}
	else
	{
		memset(right[USER_ID_GM],0,16);
	}

	
	child = pElement->FirstChildElement("owner");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadDailyCfg:  read owner error");
		return -1;
	}
		
	str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR("RightConfig::ReadDailyCfg:  read property error");
		return -1;
	}
	property=atoi(str);
	if(property==1)
	{
		if(ReadDailyCfg(child,right[USER_ID_OWNER])==-1)
		{
			AC_ERROR("RightConfig::ReadDailyCfg: read property error");
			return -1;
		}
	}	
	else
	{
		memset(right[USER_ID_OWNER],0,16);
	}	

	child = pElement->FirstChildElement("subowner");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadDailyCfg:  read subowner error");
		return -1;
	}
		
	str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR(" RightConfig::ReadDailyCfg: read property error");
		return -1;
	}
	property=atoi(str);
	if(property==1)
	{
		if(ReadDailyCfg(child,right[USER_ID_OWNER_S])==-1)
		{
			AC_ERROR("RightConfig::ReadDailyCfg: read property error");
			return -1;
		}
	}else
	{
		memset(right[USER_ID_OWNER_S],0,16);
	}


	child = pElement->FirstChildElement("VJ");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadDailyCfg:  read VJ error");
		return -1;
	}
	str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR(" RightConfig::ReadDailyCfg: read property error");
		return -1;
	}
	property=atoi(str);
	if(property==1)
	{
		if(ReadDailyCfg(child,right[USER_ID_VJ])==-1)
		{
			AC_ERROR("RightConfig::ReadDailyCfg: read property error");
			return -1;
		}
	}else
	{
		memset(right[USER_ID_VJ],0,16);
	}


	child = pElement->FirstChildElement("VJA");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadDailyCfg:  read VJA error");
		return -1;
	}
	str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR(" RightConfig::ReadDailyCfg: read property error");
		return -1;
	}
	
	property=atoi(str);
	if(property==1)
	{
		if(ReadDailyCfg(child,right[USER_ID_VJ_A])==-1)
		{
			AC_ERROR("RightConfig::ReadDailyCfg: read property error");
			return -1;
		}
	}else
	{
		memset(right[USER_ID_VJ_A],0,16);
	}


	child = pElement->FirstChildElement("MEMBER");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadDailyCfg:  read MEMBER error");
		return -1;
	}
		
	str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR(" RightConfig::ReadDailyCfg: read property error");
		return -1;
	}
	property=atoi(str);
	if(property==1)
	{
		if(ReadDailyCfg(child,right[USER_ID_VIP])==-1)
		{
			AC_ERROR("RightConfig::ReadDailyCfg: read property error");
			return -1;
		}
	}else
	{
		memset(right[USER_ID_VIP],0,16);
	}
	

	child = pElement->FirstChildElement("VIP");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadDailyCfg:  read VIP error");
		return -1;
	}
		
	str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR(" RightConfig::ReadMangerCfg: read property error");
		return -1;
	}
	property=atoi(str);
	if(property==1)
	{
		if(ReadDailyCfg(child,right[USER_ID_GVIP])==-1)
		{
			AC_ERROR("RightConfig::ReadDailyCfg: read property error");
			return -1;
		}
	}else
	{
		memset(right[USER_ID_GVIP],0,16);
	}
	

	child = pElement->FirstChildElement("RED");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadDailyCfg:  read RED error");
		return -1;
	}
		
	str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR(" RightConfig::ReadMangerCfg: read property error");
		return -1;
	}
	property=atoi(str);
	if(property==1)
	{
		if(ReadDailyCfg(child,right[USER_ID_RED])==-1)
		{
			AC_ERROR("RightConfig::ReadDailyCfg: read property error");
			return -1;
		}
	}else
	{
		memset(right[USER_ID_RED],0,16);
	}


	child = pElement->FirstChildElement("PURPLE");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadDailyCfg:  read PURPLE error");
		return -1;
	}
		
	str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR(" RightConfig::ReadDailyCfg: read property error");
		return -1;
	}
	property=atoi(str);
	if(property==1)
	{
		if(ReadDailyCfg(child,right[USER_ID_PURPLE])==-1)
		{
			AC_ERROR("RightConfig::ReadDailyCfg: read property error");
			return -1;
		}
	}else
	{
		memset(right[USER_ID_PURPLE],0,16);
	}

	child = pElement->FirstChildElement("SUPER");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadDailyCfg:  read SUPER error");
		return -1;
	}
		
	str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR(" RightConfig::ReadDailyCfg: read property error");
		return -1;
	}
	property=atoi(str);
	if(property==1)
	{
		if(ReadDailyCfg(child,right[USER_ID_SUPER])==-1)
		{
			AC_ERROR("RightConfig::ReadDailyCfg: read property error");
			return -1;
		}
	}else
	{
		memset(right[USER_ID_SUPER],0,16);
	}

	child = pElement->FirstChildElement("IMPERIAL");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadDailyCfg:  read IMPERIAL error");
		return -1;
	}
		
	str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR(" RightConfig::ReadDailyCfg: read property error");
		return -1;
	}
	property=atoi(str);
	if(property==1)
	{
		if(ReadDailyCfg(child,right[USER_ID_IMPERIAL])==-1)
		{
			AC_ERROR("RightConfig::ReadDailyCfg: read property error");
			return -1;
		}
	}else
	{
		memset(right[USER_ID_IMPERIAL],0,16);
	}
	

	child = pElement->FirstChildElement("USER");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadDailyCfg:  read USER error");
		return -1;
	}
		
	str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR(" RightConfig::ReadDailyCfg: read property error");
		return -1;
	}
	property=atoi(str);
	if(property==1)
	{
		if(ReadDailyCfg(child,right[USER_ID_NONE])==-1)
		{
			AC_ERROR("RightConfig::ReadDailyCfg: read property error");
			return -1;
		}
	}else
	{
		memset(right[USER_ID_NONE],0,16);
	}
	
	return 0;
}

int RightConfig::ReadMangerCfg(TiXmlElement *pElement,char right[16])
{

	if(!pElement)
		return -1;
	if(!right)
		return -1;
	
	TiXmlElement* child = pElement->FirstChildElement("GM");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadMangerCfg: read GM error");
		return -1;
	}
		
	const char* str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR("RightConfig::ReadMangerCfg: read property error");
		return -1;
	}
	right[USER_ID_GM]=atoi(str);


	child = pElement->FirstChildElement("owner");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadMangerCfg:  read owner error");
		return -1;
	}
		
	str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR("RightConfig::ReadMangerCfg:  read property error");
		return -1;
	}
	right[USER_ID_OWNER]=atoi(str);

	child = pElement->FirstChildElement("subowner");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadMangerCfg:  read subowner error");
		return -1;
	}
		
	str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR(" RightConfig::ReadMangerCfg: read property error");
		return -1;
	}
	right[USER_ID_OWNER_S]=atoi(str);

	child = pElement->FirstChildElement("VJ");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadMangerCfg:  read VJ error");
		return -1;
	}
		
	str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR(" RightConfig::ReadMangerCfg: read property error");
		return -1;
	}
	right[USER_ID_VJ]=atoi(str);
	
	child = pElement->FirstChildElement("VJA");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadMangerCfg:  read VJA error");
		return -1;
	}
		
	str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR(" RightConfig::ReadMangerCfg: read property error");
		return -1;
	}
	right[USER_ID_VJ_A]=atoi(str);
	
	child = pElement->FirstChildElement("MEMBER");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadMangerCfg:  read MEMBER error");
		return -1;
	}
		
	str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR(" RightConfig::ReadMangerCfg: read property error");
		return -1;
	}
	right[USER_ID_VIP]=atoi(str);
	
	child = pElement->FirstChildElement("USER");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadMangerCfg:  read USER error");
		return -1;
	}
		
	str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR(" RightConfig::ReadMangerCfg: read property error");
		return -1;
	}
	right[USER_ID_NONE]=atoi(str);
	
	return 0;
}

int RightConfig::ReadMangerCfg(TiXmlElement *pElement,char right[16][16])
{

	if(!pElement)
		return -1;
	if(!right)
		return -1;
	
	TiXmlElement* child = pElement->FirstChildElement("GM");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadMangerCfg: read GM error");
		return -1;
	}
		
	const char* str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR("RightConfig::ReadMangerCfg: read property error");
		return -1;
	}
	char property=atoi(str);
	if(property==1)
	{
		if(ReadMangerCfg(child,right[USER_ID_GM])==-1)
		{
			AC_ERROR("RightConfig::ReadMangerCfg: read property error");
			return -1;
		}
	}
	else
	{
		memset(right[USER_ID_GM],0,16);
	}
	

	child = pElement->FirstChildElement("owner");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadMangerCfg:  read owner error");
		return -1;
	}
		
	str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR("RightConfig::ReadMangerCfg:  read property error");
		return -1;
	}
	property=atoi(str);
	if(property==1)
	{
		if(ReadMangerCfg(child,right[USER_ID_OWNER])==-1)
		{
			AC_ERROR("RightConfig::ReadMangerCfg: read property error");
			return -1;
		}
	}
	else
	{
		memset(right[USER_ID_OWNER],0,16);
	}
	

	child = pElement->FirstChildElement("subowner");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadMangerCfg:  read subowner error");
		return -1;
	}
		
	str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR(" RightConfig::ReadMangerCfg: read property error");
		return -1;
	}
	property=atoi(str);
	if(property==1)
	{
		if(ReadMangerCfg(child,right[USER_ID_OWNER_S])==-1)
		{
			AC_ERROR("RightConfig::ReadMangerCfg: read property error");
			return -1;
		}
	}
	else
	{
		memset(right[USER_ID_OWNER_S],0,16);
	}


	child = pElement->FirstChildElement("VJ");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadMangerCfg:  read VJ error");
		return -1;
	}
		
	str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR(" RightConfig::ReadMangerCfg: read property error");
		return -1;
	}
	property=atoi(str);
	if(property==1)
	{
		if(ReadMangerCfg(child,right[USER_ID_VJ])==-1)
		{
			AC_ERROR("RightConfig::ReadMangerCfg: read property error");
			return -1;
		}
	}
	else
	{
		memset(right[USER_ID_VJ],0,16);
	}


	child = pElement->FirstChildElement("VJA");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadMangerCfg:  read VJA error");
		return -1;
	}
		
	str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR(" RightConfig::ReadMangerCfg: read property error");
		return -1;
	}
	property=atoi(str);
	if(property==1)
	{
		if(ReadMangerCfg(child,right[USER_ID_VJ_A])==-1)
		{
			AC_ERROR("RightConfig::ReadMangerCfg: read property error");
			return -1;
		}
	}
	else
	{
		memset(right[USER_ID_VJ_A],0,16);
	}


	child = pElement->FirstChildElement("MEMBER");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadMangerCfg:  read MEMBER error");
		return -1;
	}
		
	str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR(" RightConfig::ReadMangerCfg: read property error");
		return -1;
	}
	property=atoi(str);
	if(property==1)
	{
		if(ReadMangerCfg(child,right[USER_ID_VIP])==-1)
		{
			AC_ERROR("RightConfig::ReadMangerCfg: read property error");
			return -1;
		}
	}
	else
	{
		memset(right[USER_ID_VIP],0,16);
	}
	

	child = pElement->FirstChildElement("USER");
	if(!child)
	{
		AC_ERROR("RightConfig::ReadMangerCfg:  read USER error");
		return -1;
	}
		
	str = child->Attribute("property");
	if(!str)
	{
		AC_ERROR(" RightConfig::ReadMangerCfg: read property error");
		return -1;
	}
	property=atoi(str);
	if(property==1)
	{
		if(ReadMangerCfg(child,right[USER_ID_NONE])==-1)
		{
			AC_ERROR("RightConfig::ReadMangerCfg: read property error");
			return -1;
		}
	}
	else
	{
		memset(right[USER_ID_NONE],0,16);
	}
	
	return 0;
	
}


