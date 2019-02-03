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
		//��ȡ�ޱ������ߵ�����
		int ReadDailyCfg(TiXmlElement *pElement,char right[16]);
		//��ȡ�б������ߵ�����
		int ReadDailyCfg(TiXmlElement *pElement,char type,char right[16][16]);
		//��ȡ�ޱ������ߵ�����
		//int ReadMangerCfg(TiXmlElement *pElement,char right[16]);
		//��ȡ�б������ߵ�����
		//int ReadMangerCfg(TiXmlElement *pElement,char right[16][16]);
	public:
		/*
		//�����ճ�����
		char  m_unpasswd[16];		//��������������뷿
		char m_closed[16];			//����رշ���
		char m_private[16];			//������Ի�Ա���ŷ���
		char m_fulllimit[16];			//������Ա����
		char m_onvjmic[16];			//��������
		char m_updownwaitmic[16];	//��������
		char m_freewaitmic[16];		//����������������
		char m_invitewaitmic[16];		//��������������������
		char m_roomchat[16];		//��ֹ���ĺ����ڷ��乫��
		
		char m_forbiden[16][16];		//����
		char m_black[16][16];			//���ڲ��߳�����
		char m_kick[16][16];			//�߳�����
		char m_giveonvjmic[16][16];	//������������
		char m_giveoffmic[16][16];	//�±�����
		char m_giveoffvjmic[16][16];	//��������
		char m_delwaitmic[16][16];	//ɾ������
		
		//��������
		char m_getapplylist[16];		//ȡ�÷����Ա�����б�
		char m_auditapply[16];		//��˻�Ա����
		char m_getmemberlist[16];	//�鿴�����Ա�б�
		char m_getblacklist[16];		//�鿴���������
		char m_setpassword[16];		//���÷�������
		char m_setroomclose[16];		//���÷��俪���ر�
		char m_setroomprivate[16];	//���÷�����Ի�Ա����
		char m_setroominout[16];		//�û�������Ϣ�����ر�
		char m_setfreewaitmic[16];	//�����������ر�
		char m_setroomname[16];	//���������޸�
		char m_setroomaffiche[16];	//���÷��乫��
		char m_sendnotice[16];		//���÷�����ʱ����
		char m_setroomchat[16];		//�����乫��
		char m_setroomwelcome[16];	//���÷��件ӭ��
		char m_setroomlogo[16];		//���÷���LOGO

		char m_delmember[16][16];	//ɾ����Ա
		char m_setmember[16][16];	//��Ϊ��Ա
		char m_setvja[16][16];		//������������
		char m_setvj[16][16];			//����������
		char m_setsubonwer[16][16];	//���ø�����
		*/

		RIGHT_CONFIG m_right[64];		//Ȩ�޴洢[��������][������Ȩ��][��������Ȩ��]

};

#endif

