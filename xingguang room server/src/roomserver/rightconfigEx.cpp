#include "rightconfigEx.h"
#include <ac/xml/tinyxml.h>
#include <ac/log/log.h>
#include "roomtype.h"

int RightConfigEx::ReadCfg(const char* configfile)
{
	char type=0;
	int optionid=0;
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
	
	TiXmlElement *rightdetail = root->FirstChildElement("rightdetail");
	//TiXmlElement *child =NULL;
	while(rightdetail)
	{
		
		const char* str = rightdetail->Attribute("type");
		if(!str)
		{
			AC_ERROR("RightConfig::ReadCfg: read type error,type=%d,id=%d",type,optionid);
			return -1;
		}
		type=atoi(str);
		
		str = rightdetail->Attribute("optionid");
		if(!str)
		{
			AC_ERROR("RightConfig::ReadCfg: read id error,type=%d,id=%d",type,optionid);
			return -1;
		}
		optionid=atoi(str);

		m_right[optionid].optiontype=type;

		//child =rightdetail->FirstChildElement();
		if(ReadDailyCfg(rightdetail,type,m_right[optionid].rightdetail)==-1)
		{
			AC_ERROR("RightConfig::ReadCfg: read id=%d element error",optionid);
			return -1;
		}


		rightdetail=rightdetail->NextSiblingElement("rightdetail");
		
	}
	
	
	return 0;
}

/**********************************
*读取无被操作者的权限配置
**********************************/
int RightConfigEx::ReadDailyCfg(TiXmlElement *pElement,char right[16])
{

	if(!pElement)
		return -1;
	if(!right)
		return -1;
	
	TiXmlElement* child = pElement->FirstChildElement("USER");
	const char* str;
	char userid=0;
	char property=0;
	
	while(child)
	{
		userid=0;
		property=0;
		
		str= child->Attribute("userid");
		if(!str)
		{
			AC_ERROR("RightConfig::ReadDailyCfg: read userid error");
			return -1;
		}
		userid=atoi(str);
		
		str = child->Attribute("property");
		if(!str)
		{
			AC_ERROR("RightConfig::ReadDailyCfg: read property error");
			return -1;
		}
		property=atoi(str);

		right[userid]=property;

		child=child->NextSiblingElement();
	}
	
	return 0;
}

/**********************************
*读取有被操作者的权限配置
**********************************/
int RightConfigEx::ReadDailyCfg(TiXmlElement *pElement,char type,char right[16][16])
{

	if(!pElement)
		return -1;
	if(!right)
		return -1;
	if(type!=1&&type!=2)
		return -1;

	const char* str;
	char userid=0;
	char property=0;

	TiXmlElement* child = pElement->FirstChildElement("USER");

	while(child)
	{
		userid=0;
		property=0;
		
		str= child->Attribute("userid");
		if(!str)
		{
			AC_ERROR("RightConfig::ReadDailyCfg: read userid error");
			return -1;
		}
		userid=atoi(str);
		
		str = child->Attribute("property");
		if(!str)
		{
			AC_ERROR("RightConfig::ReadDailyCfg: read property error");
			return -1;
		}
		property=atoi(str);
		
		if(property==1&&type==2)
		{
			if(ReadDailyCfg(child,right[userid])==-1)
			{
				AC_ERROR("RightConfig::ReadDailyCfg: read sub element error");
				return -1;
			}
		}
		else
		{
			memset(right[userid],property,16);
		}

		child=child->NextSiblingElement();
	
	}
	
	return 0;
}


