#ifndef RIGHT_CONFEX_H_
#define RIGHT_CONFEX_H_

#include <vector>
#include <string>
#include <ac/xml/tinyxml.h>
#include "netproxy.h"
using namespace std;


class RightConfigEx
{
	public:
		RightConfigEx()
		{}
		~RightConfigEx(){}
		int ReadCfg(const char* configfile);

		void Init()
		{
		/*
			memset(m_unpasswd,0,sizeof(m_unpasswd));
			memset(m_closed,0,sizeof(m_closed));
			memset(m_private,0,sizeof(m_private));
			memset(m_fulllimit,0,sizeof(m_fulllimit));
			memset(m_onvjmic,0,sizeof(m_onvjmic));
			memset(m_updownwaitmic,0,sizeof(m_updownwaitmic));
			memset(m_freewaitmic,0,sizeof(m_freewaitmic));
			memset(m_invitewaitmic,0,sizeof(m_invitewaitmic));
			memset(m_roomchat,0,sizeof(m_roomchat));

			memset(m_forbiden,0,sizeof(m_forbiden));
			memset(m_black,0,sizeof(m_black));
			memset(m_kick,0,sizeof(m_kick));
			memset(m_giveonvjmic,0,sizeof(m_giveonvjmic));
			memset(m_giveoffmic,0,sizeof(m_giveoffmic));
			memset(m_giveoffvjmic,0,sizeof(m_giveoffvjmic));
			memset(m_delwaitmic,0,sizeof(m_delwaitmic));
			
			memset(m_getapplylist,0,sizeof(m_getapplylist));
			memset(m_auditapply,0,sizeof(m_auditapply));
			memset(m_getmemberlist,0,sizeof(m_getmemberlist));
			memset(m_getblacklist,0,sizeof(m_getblacklist));
			memset(m_setpassword,0,sizeof(m_setpassword));
			memset(m_setroomclose,0,sizeof(m_setroomclose));
			memset(m_setroomprivate,0,sizeof(m_setroomprivate));
			memset(m_setroominout,0,sizeof(m_setroominout));
			memset(m_setfreewaitmic,0,sizeof(m_setfreewaitmic));
			memset(m_setroomname,0,sizeof(m_setroomname));
			memset(m_setroomaffiche,0,sizeof(m_setroomaffiche));
			memset(m_sendnotice,0,sizeof(m_sendnotice));
			memset(m_setroomchat,0,sizeof(m_setroomchat));
			memset(m_setroomwelcome,0,sizeof(m_setroomwelcome));
			memset(m_setroomlogo,0,sizeof(m_setroomlogo));

			memset(m_delmember,0,sizeof(m_delmember));
			memset(m_setmember,0,sizeof(m_setmember));
			memset(m_setvja,0,sizeof(m_setvja));
			memset(m_setvj,0,sizeof(m_setvj));
			memset(m_setsubonwer,0,sizeof(m_setsubonwer));
			*/
			memset(m_right,0,sizeof(m_right));

		};
	private:
		//读取无被操作者的配置
		int ReadDailyCfg(TiXmlElement *pElement,char right[16]);
		//读取有被操作者的配置
		int ReadDailyCfg(TiXmlElement *pElement,char type,char right[16][16]);
		//读取无被操作者的配置
		//int ReadMangerCfg(TiXmlElement *pElement,char right[16]);
		//读取有被操作者的配置
		//int ReadMangerCfg(TiXmlElement *pElement,char right[16][16]);
	public:
		/*
		//房间日常管理
		char  m_unpasswd[16];		//无需密码进入密码房
		char m_closed[16];			//进入关闭房间
		char m_private[16];			//进入仅对会员开放房间
		char m_fulllimit[16];			//进入满员房间
		char m_onvjmic[16];			//上主持麦
		char m_updownwaitmic[16];	//调整麦序
		char m_freewaitmic[16];		//非自由排麦下排麦
		char m_invitewaitmic[16];		//非自由排麦下邀请排麦
		char m_roomchat[16];		//禁止公聊后能在房间公聊
		
		char m_forbiden[16][16];		//禁言
		char m_black[16][16];			//拉黑并踢出房间
		char m_kick[16][16];			//踢出房间
		char m_giveonvjmic[16][16];	//邀请上主持麦
		char m_giveoffmic[16][16];	//下表演麦
		char m_giveoffvjmic[16][16];	//下主持麦
		char m_delwaitmic[16][16];	//删除排序
		
		//房间设置
		char m_getapplylist[16];		//取得房间会员申请列表
		char m_auditapply[16];		//审核会员申请
		char m_getmemberlist[16];	//查看房间会员列表
		char m_getblacklist[16];		//查看房间黑名单
		char m_setpassword[16];		//设置房间密码
		char m_setroomclose[16];		//设置房间开启关闭
		char m_setroomprivate[16];	//设置房间仅对会员开放
		char m_setroominout[16];		//用户进出信息开启关闭
		char m_setfreewaitmic[16];	//自由上麦开启关闭
		char m_setroomname[16];	//房间名称修改
		char m_setroomaffiche[16];	//设置房间公告
		char m_sendnotice[16];		//设置房间临时公告
		char m_setroomchat[16];		//允许房间公聊
		char m_setroomwelcome[16];	//设置房间欢迎词
		char m_setroomlogo[16];		//设置房间LOGO

		char m_delmember[16][16];	//删除会员
		char m_setmember[16][16];	//降为会员
		char m_setvja[16][16];		//设置助理主持
		char m_setvj[16][16];			//设置主持人
		char m_setsubonwer[16][16];	//设置副室主
		*/

		RIGHT_CONFIG m_right[64];		//权限存储[操作类型][操作者权限][被操作者权限]

};

#endif

