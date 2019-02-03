#ifndef ROOMTYPE_H_
#define ROOMTYPE_H_

#include "roomclient.h"
#include "netproxy.h"
#include "roomtimeout.h"

#define ROOMNUMPERPAGE 10
//add by jinguanfu 2011/8/10
#define READY_WAIT		30	//����׼����ʱʱ��
#define MUSIC_OFFSET	30	//����ʱ����ƫ��ֵ
//add by lihongwu 2011/9/20
#define MIC_REMAINTM    600  //ʣ��ʱ��+����ʱ��
#define SENDGIFT_SETTM    3  //����ʱ����ʱ

enum DBAGENTCMD
{
	CMD_CALLSP = 0x01,
};

enum PARAM_TYPE
{
	PT_UNKNOWN = 0,
	PT_INPUT = 1,
	PT_OUTPUT = 2,
};

enum PARAM_DATATYPE
{
	PDT_EMPTY = 0,
	PDT_INT = 1,
	PDT_VARCHAR = 2,
};

enum LOGIN_RET
{
	LOGIN_NONE     = 0,  //δ��¼
	LOGIN_SUCESS   = 1,  //��½�ɹ�
	LOGIN_AGAIN    = 2,  //�ظ���½   //������
	LOGIN_NOUIN    = 3,  //�Ƿ��û�	
	LOGIN_FULL     = 4,  //����������
	LOGIN_IFULL    = 5,  //�����Ѿ��ﵽ�����������û�����޷���½
	LOGIN_EPWD     = 6,  //�����������
	LOGIN_CLOSE    = 7,  //����ر�
	LOGIN_INBLACK  = 8,  //�ں�������
	LOGIN_INTBLACK = 9,  //����ʱ��������
	LOGIN_NOTEXIST = 10,	//���䲻���ڻ�ɾ��
	LOGIN_LOCKED= 11,	//���䶳��

};

enum ROOM_STATE
{
	ROOM_STATE_OPEN     = 0,  //���俪��״̬
	ROOM_STATE_CLOSE    = 1,  //����ر�״̬
};

enum ROOM_SERVER_CMD  //cmd����SBΪ�㲥��Ϣ
{
	ROOM_CMD_TOKEN = 1001,                  //token��֤
	ROOM_CMD_LOGIN,                         //��½
	ROOM_CMD_LEAVEROOM,                     //�뿪����     
	ROOM_CMD_KEEPALIVE,                     //������
	ROOM_CMD_SB_ENTERROOM,                  //ĳ���˽��뷿��
	ROOM_CMD_SB_LEAVEROOM,                  //ĳ���뿪����
	ROOM_CMD_GET_ALLINFO,                   //�õ��û��б� 
	ROOM_CMD_WAITMIC,                       //�û���������
	ROOM_CMD_CANCLE_WAITMIC,                //�û�����ȡ������                  
	ROOM_CMD_SB_WAITMIC,                    //ĳ������ɹ�10
	ROOM_CMD_SB_CANCLE_WAITMIC,             //ĳ��ȡ������ɹ�
	ROOM_CMD_WAITMIC_PK,                    //�û�����pk
	ROOM_CMD_CANCLE_WAITMIC_PK,             //�û�����ȡ��pk
	ROOM_CMD_SB_WAITMIC_PK,                 //ĳ�˼���pk�ɹ� 
	ROOM_CMD_SB_CANCLE_WAITMIC_PK,          //ĳ��ȡ��pk�ɹ� 15
	ROOM_CMD_UP_WAITMIC,                    //����ĳ�˵�����
	ROOM_CMD_SB_UP_WAITMIC,                 //����ĳ�˵�����ɹ�
	ROOM_CMD_DOWN_WAITMIC,                  //�½�ĳ�˵�����
	ROOM_CMD_SB_DOWN_WAITMIC,               //�½�ĳ�˵�����ɹ�
	ROOM_CMD_SB_ONMIC_READY,                //��������׼��20
	ROOM_CMD_ONMIC_READYOK,                 //����׼��OK
	ROOM_CMD_SB_ONMIC,                      //��������
	ROOM_CMD_OFFMIC_READY,                  //����׼��
	ROOM_CMD_SB_OFFMIC_READYOK,             //����׼��OK, �ͻ��˿��Կ�ʼ�����
	ROOM_CMD_SCORE,                         //���25
	ROOM_CMD_SB_OFFMIC,                     //��������
	ROOM_CMD_KICKOUT_SOMEONE,               //�߳�ĳ��
	ROOM_CMD_SB_KICKOUT,                    //ĳ�˱��߳�����
	ROOM_CMD_FORBIDEN_SOMEONE,              //����ĳ��
	ROOM_CMD_SB_FORBIDEN,                   //ĳ�˱�����30
	ROOM_CMD_UPDATE_BLACKLIST,              //���·���ĺ�����
	ROOM_CMD_SB_BLACKLIST_UPDATE,           //�����������������  31
	ROOM_CMD_UPDATE_MGRLIST,                //���·���Ĺ���Ա����
	ROOM_CMD_SB_MGRLIST_UPDATE,             //������������Ա������������     
	ROOM_CMD_ROOM_MGR_SUCCESS,              //������Ӧ�Ŀͻ��˷����������ɹ�
	ROOM_CMD_PRIVATE_CHAT,                  //˽��
	ROOM_CMD_PUBLIC_CHAT,                   //���� 37
	ROOM_CMD_ONVJ_MIC,                      //��vj��
	ROOM_CMD_SB_ONVJ_MIC,                   //������vj����
	ROOM_CMD_OFFVJ_MIC,                     //��vj��40
	ROOM_CMD_SB_OFFVJ_MIC,                  //������vj����    
	ROOM_CMD_SB_SCORE,                         //��ֽ���
	ROOM_CMD_GIVE_VJ_MIC,                   //����vj��  
	ROOM_CMD_GIVEOFF_VJ_MIC,                //��vj��
	ROOM_CMD_GIVEOFF_MIC,                   //����ͨ��45
	ROOM_CMD_SB_GIVEOFF_MIC,                 //���˱�����ͨ����
	ROOM_CMD_UPDATE_CONTENT,                //���·���Ĺ���
	ROOM_CMD_SB_CONTENT_UPDATE,             //���乫�汻������
	ROOM_CMD_SEND_GIFT,                   //�����������    				C=>S			NULL			int s_idx,r_idx,cate_idx,number  47
	ROOM_CMD_SB_SEND_GIFT,                //������㲥    				S=>C	50		NULL			int s_idx,r_idx,cate_idx,number
	//ROOM_CMD_BROAD_ALL,                     //������
	//ROOM_CMD_BROAD_LOBBY,                   //С����
	ROOM_CMD_SEND_NOTICE_TEMP,               //���ͷ��乫��    				C=>S			NULL			ResultData_SendFlowerInfo
	ROOM_CMD_SB_SEND_NOTICE_TEMP,            //�յ����乫��    				S=>C			NULL			ResultData_SendFlowerInfo   50
	ROOM_CMD_INVITE_MIC,                    //�����û��������������
	ROOM_CMD_SB_INVITE_MIC,                    //�����û��������������
	ROOM_CMD_USER_APP_JOINROOM,             //�û�������뷿��
	//ROOM_CMD_SB_USER_APP_JOINROOM,             //�û�������뷿��
	ROOM_CMD_VERIFY_USER_APP,               //����û���Ա����                 C=>S 
	ROOM_CMD_SB_VERIFY_USER_APP,            //����û���Ա����ظ�                 S=>C  
	ROOM_CMD_REFUSEMEMBER,
	ROOM_CMD_REMOVE_USER,                   //ɾ����Ա
	ROOM_CMD_SB_REMOVE_USER, 		//60
	ROOM_CMD_GIVE_VJ_A,                      //����Ϊ��������
	ROOM_CMD_SB_GIVE_VJ_A,                        //60
	ROOM_CMD_GIVE_VJ,                        //����Ϊ����
	ROOM_CMD_SB_GIVE_VJ,
	ROOM_CMD_GIVE_OUER_S,                    //����Ϊ������
	ROOM_CMD_SB_GIVE_OUER_S,
	ROOM_CMD_SET_ROOM_PWD,                   //��������
	ROOM_CMD_SB_SET_ROOM_PWD,
	ROOM_CMD_SET_ROOM_LOCK,                  //��������
	ROOM_CMD_SB_SET_ROOM_LOCK,			//70
	ROOM_CMD_SET_USER_ONLY,                 //���÷���ֻ�Ի�Ա����
	ROOM_CMD_SB_SET_USER_ONLY,                  // 
	ROOM_CMD_SET_USER_INOUT,                //���÷������ɽ���
	ROOM_CMD_SB_SET_USER_INOUT,
	ROOM_CMD_SET_MIC_UPDOWN,                //������������
	ROOM_CMD_SB_SET_MIC_UPDOWN,
	ROOM_CMD_SET_ROOM_NAME,                 //���÷�������
	ROOM_CMD_SB_SET_ROOM_NAME,
	ROOM_CMD_SET_CHAT_PUBLIC,              //���÷����Ƿ���
	ROOM_CMD_SB_SET_CHAT_PUBLIC,		//80
	ROOM_CMD_SET_ROOM_WELCOME,                //���÷��件ӭ��
	ROOM_CMD_SB_SET_ROOM_WELCOME,               //  
	ROOM_CMD_SET_ROOM_LOGO,                   //���÷���LOGO
	ROOM_CMD_SB_SET_ROOM_LOGO,
	ROOM_CMD_ROOMAPPLYLIST_C2R2C,            //��ȡ���������б�
	ROOM_CMD_GETBLACKLIST,                   //��ȡ����������б�
	ROOM_CMD_GETROOMMEMBERLIST,              //��ȡ�����Ա�б�
	/*add by jinguanfu 2010/2/25 �ɾ�ģ��*/
	ROOM_CMD_SB_SONGLEVEL,			//�ݳ��ȼ�֪ͨ
	ROOM_CMD_SB_RICHLEVEL,			//�Ƹ��ȼ�֪ͨ
	//add by jinguanfu 2010/4/7
	ROOM_CMD_KICKUSER = 1090,			//�û����ߵ�	90
	//add by jinguanfu 2010/4/11
	ROOM_CMD_EXITMEMBER,			//�û��˳������Ա

	ROOM_CMD_GETPANELINFO,			//ȡ�÷������������Ϣ

	//add by jinguanfu 2010/5/13
	ROOM_CMD_GIVE_MEMBER,			//�ɹ���Ա��Ϊһ���Ա
	ROOM_CMD_SB_GIVE_MEMBER,		//��Ϊһ���Ա��֪ͨ
	//add by jinguanfu 2010/5/20
	ROOM_CMD_SB_CHANGEAVATAR,		//��������֪ͨ
	//add by jinguanfu 2010/6/2
	ROOM_CMD_REQUEST_GIVE_VJ_MIC,	//����������������Ӧ
	//add by jinguanfu 2010/7/14
	ROOM_CMD_SB_LEVEL,			//�û����������㲥
	//add by jinguanfu 2010/7/19
	ROOM_CMD_SB_VIP,				//VIP���ʹڸı�֪ͨ
	//add by jinguanfu 2010/8/5
	ROOM_CMD_UPDATEMONEY,		//����Ǯ��֪ͨ
	//add by jinguanfu 2010/8/13
	ROOM_CMD_SB_DELETEROOM=1100,	//����ɾ��֪ͨ
	//add by jinguanfu 2010/8/13
	ROOM_CMD_SB_LOCKROOM,		//���䶳��֪ͨ
	//add by jinguanfu 2010/8/17
	ROOM_CMD_SB_ROOMINFO_UPDATE,	//������Ϣ����֪ͨ
	//add by jinguanfu 2010/8/17
	ROOM_CMD_SB_GIVE_OWNER,			//������������֪ͨ
	//add by jinguanfu 2010/9/2
	ROOM_CMD_SEND_FIREWORKS,		//�����̻�
	//add by jinguanfu 2010/9/2
	ROOM_CMD_SB_SEND_FIREWORKS,	//�����̻�֪ͨ
	//add by jinguanfu 2010/9/2
	ROOM_CMD_RECV_FIREWORKS,		//�����̻�
	//add by jinguanfu 2010/9/3
	ROOM_CMD_SB_LUCKY,				//���������н�֪ͨ
	//add by jinguanfu 2010/9/8
	ROOM_CMD_SB_GIFT_INVALID,		//ӡ��ʧЧ֪ͨ
	//add by jinguanfu 2010/9/13
	ROOM_CMD_SB_GIFT_VALID,			//ӡ����Ч֪ͨ
	//add by jinguanfu 2010/11/2
	ROOM_CMD_VIEW_INCOME=1110,		//��ʾ����ȡ����
	
	ROOM_CMD_VIEW_INCOME_LOG,		//�鿴������ȡ��¼

	ROOM_CMD_GET_INCOME,			//��ȡ����
	//add by jinguanfu 2010/11/17
	ROOM_CMD_SB_GIFTINFO_CHANGE,	//������Ϣ����֪ͨ
	ROOM_CMD_SB_RIGHTCONF_CHANGE,	//Ȩ�����ø���֪ͨ
	//add by jinguanfu 2010/11/26
	ROOM_CMD_SB_AVSERVER_CHANGE,	//��������Ƶ����������
	//add by jinguanfu 2010/11/30
	ROOM_CMD_SB_UPDATE_GM,			//��̨����GM
	//add by jinguanfu 2010/12/8
	ROOM_CMD_SB_EXITMEMBER,			//�û��˳���Ա֪ͨ
	//add by jinguanfu 2011/2/23
	ROOM_CMD_UP_NETSTATUS,			//�û��ϴ�����״̬��Ϣ
	ROOM_CMD_SB_NETSTATUS,			//�򷿼����û��㲥����״̬��Ϣ
	//add by jinguanfu 2011/3/3
	ROOM_CMD_INVITE_MEMBER=1120,		//�����ԱC->S
	ROOM_CMD_INVITE_NOTIFY,			//����֪ͨS->C
	ROOM_CMD_INVITE_REPLY,			//����ظ�C->S

	ROOM_CMD_SEND_STAR,
	//add by jinguanfu 2011/4/12
	ROOM_CMD_GET_GIFTSEND,			//ȡ�õ����������ͼ�¼
	ROOM_CMD_GET_GIFTRECV,			//ȡ�õ���������ռ�¼
	//add by jinguanfu 2011/4/19
	ROOM_CMD_SET_AUTOONMIC,			//���÷�����ʱ����
	ROOM_CMD_SB_SET_AUTOONMIC,			//��ͣ����״̬֪ͨ
	//add by jinguanfu 2011/8/19
	ROOM_CMD_DISABLE_IPADDR,		//���öԷ���IP
	ROOM_CMD_DISABLE_MACADDR,		//���öԷ���MAC
	//add by lihongwu 2011-9-8
	ROOM_CMD_ADD_TM=1130,                //����ʱ���ӳ�
	ROOM_CMD_SB_ADDTM,             //����ʱ���ӳ�֪ͨ

	ROOM_CMD_LASTERROR,				//DEBUG�÷���Ȩ�޴���
	//add by jinguanfu 2010/11/9
	ROOM_CMD_UPDATE_MUSICINFO,		//�����ݿ���¸����ĵ㳪��
	ROOM_CMD_DB_LUCKCONF_CHANGE,		//�н����ʸ���
	//add by jinguanfu 2012/4/22
	ROOM_CMD_UPDATE_MUSICTIME,		//�ͻ����ϴ�����ʱ��


};

//begin by lihongwu 2011-9-8
enum MICTM_ADD_STATE
{
	ADDTM_SUCESS = 0,     //����ʱ���ӳ��ɹ�
	ADDTM_FAILURE = 1,     //����ʱ���ӳ�ʧ��
};
//end by lihong 2011-9-8

enum HALL_SERVER_CMD
{
	HALL_CMD_LEAVEROOM  = 0x01,
	HALL_CMD_ENTERROOM  = 0x02,
	HALL_CMD_CREATEROOM = 0x03,
	HALL_CMD_DELETEROOM = 0x04,
	HALL_CMD_LOCKROOM   = 0x05,
	HALL_CMD_GETUSERINFO=0X06,				//ȡ���û���Ϣ
	HALL_CMD_UPDATEROOM=0X07,				//������Ϣ����
	HALL_CMD_ROOMMANAGERCHG=0X08,			//�������Ա����
	HALL_CMD_ROOMLOGIN  =0x100,                       //���䱨��������Ϣ
	HALL_CMD_SYNC_ROOMINFO = 0x101,			//����ͬ��������Ϣ
	HALL_CMD_RETURN_SVERSERID =0x200,
	HALL_CMD_FLOWER_ALL       =300,      //֪ͨ��������������Ϣ
	HALL_CMD_FLOWER_LOBBY     =301,

	//add by jinguanfu 2010/4/8 �������������־�ʹ���ͬ��

	HALL_CMD_APPLYMEMBER_R2L = 304,		//֪ͨ�������������
	//HALL_CMD_KICKUSER_L2R = 305,			//����ת���߳��û���Ϣ
	HALL_CMD_DELMEMBER_R2L = 306,		//֪ͨ����ɾ����Ա
	HALL_CMD_EXITMEMBER_R2L = 307,		//֪ͨ������Ա�˳�
	

	HALL_CMD_KICKUSER_L2R = 400,			//����ת���߳��ظ��û���Ϣ
	HALL_CMD_CHANGENAME_R2L =401,		//֪ͨ��������������
	HALL_CMD_CHANGEPWD_R2L  =402,		//֪ͨ���������������
	HALL_CMD_CHANGECLOSEFLAG_R2L=403,		//֪ͨ�������俪��/�ر�״̬�ı�
	HALL_CMD_CHANGEUSERONLYFLAG_R2L=404,	//֪ͨ��������˽��״̬�ı�
	HALL_CMD_UPDATEGOLD_L2R=405,				//����ͬ���û��������������
	HALL_CMD_CHANGEAVATAR_L2R=406,			//�û��޸�AVATAR ֪ͨ
	//add by jinguanfu 2010/6/5
	HALL_CMD_CHANGERIGHT_R2L=407,			//Ȩ�޸���֪ͨ
	HALL_CMD_LEAVELOBBY_L2R=408,				//�û��뿪����֪ͨ
	//add by jinguanfu 2010/7/14
	HALL_CMD_CHANGELEVEL=409,			//�û���������
	

	HALL_CMD_OVERDUE_ITEM_L2R = 412,    // ֪ͨ���ڵĵ���
	HALL_CMD_OVERDUE_VIP_L2R = 413,	// ֪ͨ���ڵ�VIP
	HALL_CMD_EXP_CHG_R2L = 414, 		//����ı�֪ͨ
	//add by jinguanfu 2010/9/6
	HALL_CMD_LUCK_R2L = 415,			//�����н�֪ͨ
	//add by jinguanfu 2010/11/17
	HALL_CMD_RIGHT_CHANGE_L2R = 416,	//Ȩ�����ø���
	HALL_CMD_LUCKCONF_CHANGE_L2R = 417,	//���˳齱���ʸ���
	HALL_CMD_GIFTINFO_CHANGE_L2R = 418,	//������Ϣ����
	HALL_CMD_MOVE_ROOM_L2R = 419,		//�����ƶ� 
	//add by jinguanfu 2010/11/27
	HALL_CMD_UPDATE_AVSERVER_L2R=420,	//����Ƶ����������
	HALL_CMD_VUSER_LOGIN_L2R = 421,		//��̨��½�����û�
	HALL_CMD_VUSER_LOGOUT_L2R=422,		//��̨�˳������û�

	HALL_CMD_UPDATE_GM_L2R=424,			//��̨����GM
	//add by jinguanfu 2011/1/11
	HALL_CMD_ADD_SEEL_R2L = 425, 		//����ӡ��
	HALL_CMD_REMOVE_SEEL_L2R = 426, 	//ɾ��ӡ��

	//add by jinguanfu 2011/1/18
	HALL_CMD_LOGIN=427,			//����������Ƿ������û���¼

	//add by jinguanfu 2011/3/11
	HALL_CMD_FIREWORKS_R2L = 428,	//������������̻�֪ͨ

	//add by jinguanfu 2011/3/22
	HALL_CMD_CHANGELOGO_R2L = 429,//��������͸���logo֪ͨ
	//add by jinguanfu 2011/5/31
	HALL_CMD_ROOMBLACKMEMCHG_L2R = 430, //�������������

	//add by jinguanfu 2011/8/19
	HALL_CMD_UPDATE_ROOMCHAT_BLACKLIST_L2R =431,	//����֪ͨ������¹��ĺ�����
	HALL_CMD_KICKUSER_R2L = 432,		//����֪ͨ�����߳��û�
	//add by lihongwu 2011/9/20
	HALL_CMD_UPDATE_ROOMGIFT_REFRESH_TIMES_L2R =433,  //����֪ͨ�����������ˢ��Ƶ��

};

enum LOG_SERVER_CMD
{
	CMD_MUSIC_CHECK_R2LS		=0,	//���ֵ������
	CMD_ROOM_KEEPALIVE_R2LS	=1,	//�û�����״̬����
	CMD_ROOM_ONMIC_R2LS		=2,	//�ϱ����󱨸�
	CMD_ROOM_OFFMIC_R2LS		=3,	//�±����󱨸�
	CMD_ROOM_ONVJMIC_R2LS		=4,	//�������󱨸�
	CMD_ROOM_OFFVJMIC_R2LS	=5,	//�������󱨸�
	CMD_ROOM_ERRORINFO_R2LS	=6,	//���󱨸�
	CMD_ROOM_USERSTATUS_R2LS =7,	//�û�����״̬����
	CMD_ROOM_ENTERROOM_R2LS	=8,	//�û����뷿��
	CMD_ROOM_LEAVEROOM_R2LS	=9,	//�û��뿪����

};


enum MIC_INFO_STATE
{
	MIC_INFO_WAIT = 0,              //����s
	MIC_INFO_ONMIC_READY,           //����ʼ׼��
	MIC_INFO_ONMIC_READYOK,         //����׼�����
	MIC_INFO_ONMIC,                 //������
	MIC_INFO_OFFMIC_READY,          //����ʼ׼��
	MIC_INFO_OFFMIC_READYOK,        //����׼�����
	MIC_INFO_SCORE,                 //�ϴ��������
	MIC_INFO_OFFMIC,			//����
};

enum _TIMEOUT_TYPE
{
	TIMEOUT_TB = 0,              //��ʱ������
	TIMEOUT_SCORE,             //���
	TIMEOUT_ONMIC,			//����
	TIMEOUT_FORBIDEN,		//����
	TIMEOUT_OFFMIC,		//����
	//add by jinguanfu 2011/8/10
	TIMEOUT_READY,			//����׼����ʱ
};
//add  by jinguanfu 2010/11/26
//������������Ϣ
class SEND_GIFT_INFO
{
public:
	SEND_GIFT_INFO():s_idx(0),r_idx(0),cate_idx(0),number(), havesend(0), seq(0)
		{
		 }
	~SEND_GIFT_INFO()
		{
		}

	void init()
	{
		s_idx=0;
		r_idx=0;
		cate_idx=0;
		number=0;
		havesend=0;
		seq=0;
		price=0;
	}
	//int roomid;		//����ID
	int s_idx;		//������idx
	int r_idx;		//����������idx
	int cate_idx;		//����id
	int number;		//��������
	int havesend;	//�����ѷ�����
	int seq;			//
	int price;		//���ﵥ��
	//add by lihongwu 2011/10/11
	int priceconf;  //����������м۸�����     
	int numpertime;     //ÿ��ˢ�������
	int interval;   //ˢ������ʱ��  

};

class MIC_INFO
{
public:
	MIC_INFO():idx(0),musicid(0),bk(0),state(MIC_INFO_WAIT), score(0), pkidx(0),pkmusicid(0),
	           pkbk(0), pkstate(MIC_INFO_WAIT), pkscore(0), level(0),musicspeed(0),reverseflag(0)
	           {
				memset(scoreinfo,0,sizeof(scoreinfo));		//add by jinguanfu 2010/6/5
				memset(pkscoreinfo,0,sizeof(pkscoreinfo));		//add by jinguanfu 2010/6/5
	           }
	~MIC_INFO()
		{
		}

	bool operator == (MIC_INFO const & rhs)
	{
		return (idx == rhs.idx) ? true : false;
	}

	bool operator == (int nidx)
	{
		return (pkidx == nidx) ? true : false;
	}

    void init()
    {
        idx = 0;                          
        musicid = 0;                    
        bk = 0;                        
        state = MIC_INFO_WAIT;                       
        score = 0;                        
        pkidx = 0;                        
        pkmusicid = 0;                  
        pkbk = 0;                       
        pkstate = MIC_INFO_WAIT;                     
        pkscore = 0;                     
        level = 0;    
        memset(scoreinfo,0,sizeof(scoreinfo));		//add by jinguanfu 2010/6/5
        memset(pkscoreinfo,0,sizeof(pkscoreinfo));		//add by jinguanfu 2010/6/5
        musictime=0;
		//add by jinguanfu 2012/2/24 �򻯰�
		memset(musicname,0,sizeof(musicname));
		musicspeed=0;
		reverseflag=0;
    }

	int idx;                          //�����˵�idx
	short musicid;                    //����id
	short bk;                         //����
	char state;                       //״̬
	int score;                        //����
	int pkidx;                        //pk�˵�idx
	short pkmusicid;                  //pk������id
	short pkbk;                       //pk�ı���
	char pkstate;                     //pk״̬
	int pkscore;                      //pk����
	char level;                       //�Ѷ�
	//add by jinguanfu 2010/6/5
	char scoreinfo[128];		//�����Ϣ
	char pkscoreinfo[128];		//pk�����Ϣ
	//add by jinguanfu 2011/8/10
	short musictime;					//����ʱ��(��)
	//add by jinguanfu 2012/2/23
	char musicname[128];		//������
	//add by jinguanfu 2012/2/27
	char musicspeed;			//�����ٶ�
	//add by jinguanfu 2012/4/5
	char reverseflag;			//������ʾλ��Ů���г�������Ů��
};
//add by jinguanfu 2010/4/19
//�������Ա
typedef struct _ROOM_MANAGER_
{
	int m_idx;		//����Աidx
	char m_identity;	//����Ա���
}ROOM_MANAGER;
//add by jinguanfu 2010/4/19
//�����Ա�����б�
typedef struct _ROOM_APPLY_
{
	int m_roomid;	//���뷿���
	int m_idx;		//������idx
	char m_time[32];	//����ʱ��
}ROOM_APPLY;

typedef struct _ROOM_INFO_
{	
	int roomid;              //�����id��  
	int ownidx;              //���������idx
	int secondownidx;              //����ĸ�����idx
	int secondownidx2;              //����ĸ�����idx

	map<int, int> vjlist;             //������������б�
	map<int, int> vjlist_a;             //����������������б�
	int vjonmic;             //�����ϵ�������
	MIC_INFO onmic;                   //����������Ϣ
	short allowhuman;        //��������
	short maxhuman;          //�������
	char roomname[51];                //��������
	//char introduce[255];              //������
	char content[301];                //���乫��
	char passwd[25];                  //��������
	char state;                       //����״̬
	char type;                        //���������
	vector<MIC_INFO*> miclist;         //���������б�
	map<int, int> blacklist;          //���������
	map<int, int> tblacklist;         //������ʱ��������10���Ӻ��Զ�����
	//map<int, int> managerlistA;       //A��������������
	//map<int, int> managerlistB;       //B��������������
	map<int,int>  userlistVIP;        //�����Ա�б�
	map<int,ROOM_APPLY>  userlistAPP;        //�����û������б�
	map<int, RoomClient*> userlist;   //�����û��б�
	//add by jinguanfu 2010/4/12
	map<int,roomtimeout*>  forbidenlist;		//��������б�
	//map<int,Flowerresultdata*> giftlist;			//���������б�
	vector<SEND_GIFT_INFO*> sendgiftlist;		//���������
	//add by jinguanfu 2010/11/26 begin
	Flowerresultdata*  FlowerTimer;			//���������ʱ��
	//add by jinguanfu 2010/11/26 end
	map<int,ROOM_MANAGER> managerlist;		//�������Ա�б�(������������)
	char roomlogo[301];               //����LOGO
	char welcomeword[401];            //���件ӭ��
	bool isPublicChat;                //�����乫��
	bool isUserInOut;                 //�û�������Ϣ����/�ر�
	bool isMicUpdown;                 //����������/�ر�?
	bool isUserOnly;                  //���÷�����Ի�Ա����
	bool isClose;                      //����/�ر�
	//add by jinguanfu 2011/4/19
	bool isAutoOnmic;					//�Զ� ����

	map<int,int> userlistMale;		//��������
	map<int,int> userlistFemale;	//����Ů��
	//add by jinguanfu 2010/9/13
	map<int,ROOM_MANAGER> managerlist_online;		//�������߹���Ա(������������)
	//map<int,GiftTimeout*> seallist;	//ӡ���б�
	
	roomtimeout* m_pOnMicTimeout;	//����ʱ

	//add by jinguanfu 2010/11/25
	char isBlacklistOk;		//������ȡ�óɹ���־λ
	char isMemberlistOk;		//��Ա�б�ȡ�óɹ���־λ
	char isApplylistOk;		//��Ա�����б�ȡ�óɹ���־λ

	//add by jinguanfu 2010/11/25
	//modify by lihongwu 2011/11/18
	char AVServerIP_telcom[256];	//����Ƶ��������ַ--���� 
	short AVServerPort_telcom;	//����Ƶ�������˿�--����
	char AVServerIP_netcom[256];	//����Ƶ��������ַ--��ͨ
	short AVServerPort_netcom;	//����Ƶ�������˿�--��ͨ

	map<int, RoomClient*> vuserlist;   //�����û��б�
	//add by jinguanfu 2011/3/3
	map<int,int> invitelist;			//��Ա�����б�map<��������,������>
	
} ROOM_INFO;

typedef struct _ROOM_LIST_INFO_ //�������з����б�
{
	int hallid;                     //����id
	short roomnum;                  //�����������id���������
	map<int, ROOM_INFO*> roommap;    //�����������id�����б�,ʹ�÷���id���Ӧ   

} ROOM_LIST_INFO;

enum USER_IDENTITY
{
	USER_ID_NONE      = 0,  	//��ͨ�û�
	USER_ID_OWNER     = 1,  	//����
	USER_ID_A_MANAGER = 2, //A����Ա
	USER_ID_B_MANAGER = 3, //B����Ա
	USER_ID_VJ        = 4,  	//������
	USER_ID_VJ_A      = 5, 	//����������
	USER_ID_VIP       = 6,  	//�����Ա
	USER_ID_GM        = 7,  	//��Ϸ����Ա
	USER_ID_OWNER_S   = 8,  //������
	USER_ID_GVIP=9,		//��ϷVIP�û�
	USER_ID_RED=10,		//����û�
	USER_ID_PURPLE =11,	//�Ϲ��û�
	USER_ID_SUPER =12,		//�����û�
	USER_ID_IMPERIAL =13,	//�����û�

	USER_ID_GUEST = 14,	//�ο�

};

enum MIC_UP_DOWN_FLAG
{
	MIC_UP_DOWN_ONE      = 0,  //����һλ
	MIC_UP_DOWN_ALL      = 1,  //��������
};

enum WAITMIC_UP_DOWN_SEARCH_
{
	WAITMIC_UP_DOWN_IDX      = 0,  //idx����
	WAITMIC_UP_DOWN_PKIDX    = 1,  //pkidx����
};

enum USER_APP_REF
{
	SUCCESS      = 0, //�ɹ�
	ALREADY    	=-1,//���Ǳ������Ա�����������Ա
	LISTFULL 	=-2,	//��Ա�б�����������
	REFUSE		=-3,//�������ܾ������뱻�ܾ���24Сʱ�ڲ���������
	IAGAIN		=-4,//�ظ����꣬�ȴ�����Ա���
	APPLYFULL	=-5,//���������б�����

};

#define MAXBUFSIZETEA 256


/*
typedef struct _DB_RESULT_DATA
{
    int roomid;
    int opidx;
    int bopidx;
    short cmd;
    char badd;
    char identity;
    char content[200];
	int cate_idx;
	int number;	
	struct timeval regtime;
	

}DB_RESULT_DATA;

typedef struct _FLOWER_RESULT_DATA
{
	//int roomid;
	int s_idx;
	int r_idx;
	int cate_idx;
	int number;
	int havesend;
	struct timeval regtime;
}FLOWER_RESULT_DATA;
*/
typedef struct _MIC_OFF_DATA
{
	ROOM_INFO*  roominfo;
	struct timeval regtime;
}MIC_OFF_DATA;

enum ROOM_TYPE
{
	ROOM_TYPE_ROOM=0,	//�ݳ���
	ROOM_TYPE_SMALL=1,	//���跿

};

// �ݳ��÷���Ϣ
typedef struct _tNetSingScore
{
	_tNetSingScore()
	: score(0)
	, combo_num(0)
	, perfect_num(0)
	, great_num(0)
	, good_num(0)
	, poor_num(0)
	, miss_num(0)
	{
	}

	int score;// �ݳ�����
	int combo_num;// combo����
	int perfect_num;
	int great_num;
	int good_num;
	int poor_num;
	int miss_num;
}NETSINGSCORE;

enum VIP_LEVEL				//vip�ȼ�
{
	VIP_LEVEL_NONE		=0,		//��ͨ�û�
	VIP_LEVEL_VIP		=1,		//VIP�û�
	VIP_LEVEL_LIFEVIP	=2,		//����VIP�û�
	VIP_LEVEL_RED		=100,	//����û�
	VIP_LEVEL_PURPLE	=200,	//�Ϲ��û�
	VIP_LEVEL_SUPER	=300,	//�����û�
	VIP_LEVEL_IMPERIAL	=400,	//�����û�
};

enum MONITOR_CMD
{
	MONITOR_CMD_REPORT=1,
	MONITOR_CMD_REPLY=4,		//�������������
};

//add by jinguanfu 2010/9/6
typedef struct _GIFT_INFO_
{
	int GiftID;		//����ID
	int type;			//����  1:һ�� 2:���� 3:���̻� 4:С�̻�
	int price;		//���ﵥ��
	int vtime;		//������Чʱ��(����)������ӡ����Ч
	//add by lihongwu 2011/10/11
	int priceconf;  //����������м۸�����    //��ؼ��ֶ�û�õ�
	int number;     //ÿ��ˢ�������
	int interval;   //ˢ������ʱ��
}GIFT_INFO;

//add by jinguanfu 2010/9/18
#define  DB_TIMEOUT	10	//���ݿ������ʱ(��)

enum DBRESULT_GIFT	//�����������
{
	DBRESULT_GIFT_NOMONEY	=-1,		//����
	DBRESULT_GIFT_SUCCESS		=0,		//�ɹ�
	DBRESULT_GIFT_NULL		=1,		//���׽��Ϊ0
	DBRESULT_GIFT_NOGIFT		=2,		//�޴˽�������
	DBRESULT_GIFT_OFFLINE		=3,		//��������߲�������

};

enum DBRESULT_MEMBER	//��Ա����
{
	DBRESULT_MEMBER_FULL		=-1,		//��������
	DBRESULT_MEMBER_SUCCESS	=0,		//�ɹ�
};

enum DBRESULT_VERIFY		//�������
{

	DBRESULT_VERIFY_SUCCESS	=0,		//�ɹ�
	DBRESULT_VERIFY_ALREADY	=1,		//���Ǳ���������������Ա
	DBRESULT_VERIFY_FULL		=2,		//��Ա������
};

enum DBRESULT_APPLY		//�����Ա
{
	DBRESULT_APPLY_FULL		=-1,		//��������
	DBRESULT_APPLY_SUCCESS	=0,		//�ɹ�
	DBRESULT_APPLY_NONE		=1,		//
	DBRESULT_APPLY_AGAGN		=2,		//�ظ�����
	DBRESULT_APPLY_REFUSE	=3,		//�������ܾ�(����Ա�ܾ�24Сʱ�ڲ���������)
	DBRESULT_APPLY_ALREADY	=4,		//���Ǳ���������������Ա
};

enum DBRESULT_SCORE		//��ֺ�Ӿ������
{

	DBRESULT_SCORE_SUCCESS	=0,		//�ɹ�
	DBRESULT_SCORE_FALIED		=1,		//ʧ��
};

enum GIFT_TYPE
{
	GIFT_TYPE_NONE		=1,		//һ������
	GIFT_TYPE_LUCK		=2,		//��������
	GIFT_TYPE_BFIRE	=3,		//���̻�
	GIFT_TYPE_SFIRE	=4,		//С�̻�
	GIFT_TYPE_SEAL		=5,		//ӡ��
	
};

enum DBRESULT_INCOME		//��ȡ��������
{
	DBRESULT_INCOME_SUCCESS	=0,		//�ɹ�
	DBRESULT_INCOME_FALIED	=1,		//ʧ��
};

enum DBRESULT_BLACKLIST	//�������������
{
	DBRESULT_BLACKLIST_SUCCESS	=0,
	DBRESULT_BLACKLIST_FALIED	=1,	//���ں������л��ѴӺ�������ɾ��
};

//add by jinguanfu 2010/11/23
#define  OFFMIC_WAIT	5	//�������һ������ȴ�ʱ��(��)

//����������ڲ�������
enum INNER_CMD
{
	INNER_CMD_GETROOMINFO=2000,	//ȡ�÷��������Ϣ
	INNER_CMD_GETMEMBERLIST,			//ȡ�÷����Ա�б�
	INNER_CMD_GETBLACKLIST,		//ȡ�÷��������
	INNER_CMD_GETAPPLYLIST,		//ȡ�÷����Ա�����б�
	//add by jinguanfu 2011/5/31
	INNER_CMD_UPDATEROOMMANGER,		//���·������Ա�б�
	INNER_CMD_UPDATEROOMBLACKLIST,	//���·��������

};

//add by jinguanfu 2010/12/1
typedef struct _GM_INFO_
{
	int idx;			//�û�idx
	char addflag;		//0:ȡ��GM 1:��ΪGM
}GM_INFO;

//add by jinguanfu 2011/1/25
//�û�����״̬
enum USER_STATE
{
	USER_STATE_NONE		=0,		//δ������
	USER_STATE_WAITMIC	=1,		//������
	USER_STATE_ONMIC		=2,		//��������
	USER_STATE_ONVJMIC	=3,		//��������
};

//add by jinguanfu 2011/2/21  
typedef struct _RIGHT_CONFIG_
{
	char optiontype;
	char rightdetail[16][16];
}RIGHT_CONFIG;

enum RIGHT_OPTION
{
	RIGHT_OPTION_CHKPWD			=1,		//��֤���뷿������
	RIGHT_OPTION_CHKCLOSE			=2,		//��֤����رշ���
	RIGHT_OPTION_CHKPRIVATE		=3,		//��֤�����Ա����
	RIGHT_OPTION_CHKFULL			=4,		//��֤������Ա����
	RIGHT_OPTION_ONVJMIC			=5,		//��������
	RIGHT_OPTION_CHGWAITMIC		=6,		//��������
	RIGHT_OPTION_FREEWAITMIC		=7,		//����������������
	RIGHT_OPTION_FORBIDEN			=8,		//����
	RIGHT_OPTION_BLACK			=9,		//����������
	RIGHT_OPTION_KICKOUT			=10,		//���û�������
	RIGHT_OPTION_GIVEVJMIC		=11,		//����������
	RIGHT_OPTION_OFFVJMIC			=12,		//�±���������
	RIGHT_OPTION_OFFMIC			=13,		//�±��˱�����
	RIGHT_OPTION_DELWAITMIC		=14,		//ɾ������
	RIGHT_OPTION_INVITEMIC		=15,		//�����������
	RIGHT_OPTION_SENDCHAT			=16,		//���ͷ��乫��
	
	RIGHT_OPTION_GETAPPLYLIST	=17,		//ȡ�û�Ա�����б�
	RIGHT_OPTION_AUDITAPPLY		=18,		//��˻�Ա����
	RIGHT_OPTION_MEMBERLIST		=19,		//�鿴��Ա�б�
	RIGHT_OPTION_DELMEMBER		=20,		//ɾ�������Ա
	RIGHT_OPTION_GIVEMEMBER		=21,		//����Ա��Ϊ��Ա
	RIGHT_OPTION_GIVEVJA			=22,		//����Ϊ��������
	RIGHT_OPTION_GIVEVJ			=23,		//����Ϊ����
	RIGHT_OPTION_GIVESUBOWNER	=24,		//����Ϊ������
	RIGHT_OPTION_GETBLACK			=25,		//ȡ�ú�����
	RIGHT_OPTION_SETROOMPWD		=26,		//���÷�������
	RIGHT_OPTION_SETROOMCLOSE	=27,		//�رա����ŷ���
	RIGHT_OPTION_SETROOMPRIVATE	=28,		//���û�Ա����
	RIGHT_OPTION_SETROOMINOUT	=29,		//���÷��������Ϣ
	RIGHT_OPTION_SETFREEMIC		=30,		//���÷�������������
	RIGHT_OPTION_SETROOMNAME		=31,		//���÷�����
	RIGHT_OPTION_SETROOMCONTENT	=32,		//���÷��乫��
	RIGHT_OPTION_SENDCONTENT		=33,		//���ͷ�����ʱ����
	RIGHT_OPTION_SETROOMCHAT		=34,		//���÷��乫��
	RIGHT_OPTION_SETWELCOMEWORD	=35,		//���÷��件ӭ��
	RIGHT_OPTION_SETROOMLOGO		=36,		//���÷���LOGO
	RIGHT_OPTION_SETAUTOONMIC	=37,		//���÷����Զ� ����
	//add by jinguanfu 2011/8/19
	RIGHT_OPTTON_DISABLEMACIP   =38,		//����IP��MAC
	//add by lihongwu 2011/9/30
	RIGHT_OPTION_ADDTM    =39,              //����ʱ���ӳ�

};


//add by jinguanfu 2011/3/9
//�ǹ⹤��������
enum FACTORY_CMD
{
	
	FACTORY_CMD_LOGIN_R2F			=0,		//�û����뷿��
	FACTORY_CMD_LOGOUT_R2F,				//�û��˳�����
	FACTORY_CMD_KEEPALIVE_R2F,			//�û�����
	FACTORY_CMD_STAR_NUM_F2R,		//�����ǹ����֪ͨ
};

//add by jinguanfu 2011/3/22 
//������������Ӧ
enum RESULT_GIVEVJMIC
{
	RESULT_GIVEVJMIC_ACCEPT	=0,	//��������ͬ������
	RESULT_GIVEVJMIC_REFUSED,	//�������߾ܾ�����
	RESULT_GIVEVJMIC_ONVJMIC,	//��������������
	RESULT_GIVEVJMIC_ONMIC,		//���������ڱ�������
};

//add by jinguanfu 2011/8/11
//�û�������״̬
enum USER_STATUS
{
	USER_STATUS_NONE 	=0,		//δ��֤
	USER_STATUS_JUST 	=1,		//��ʱ�Ƿ�����
	USER_STATUS_BAD 	=2,		//������
	USER_STATUS_GOOD 	=3,		//�Ƿ�����

};

//add by jinguanfu 2011/9/27
//�û�TOKEN��֤����ֵ
enum TOKENLOGIN_RET
{
	TOKENLOGIN_SUCCESS =0,	//�ɹ�
	TOKENLOGIN_TIMEOUT =-1,	//ʧ�ܣ�token��ʱ
	TOKENLOGIN_INVALID =-2,	//ʧ�ܣ����û�δ��½����
};

//add by lihongwu 2011/10/10
//��������õ�������Ϣ
typedef struct _GIFT_CONFIG
{
	_GIFT_CONFIG()
		: price(0)
		, number(0)
		, interval(0)
	{
	}

	int price;       //�۸�����
	int number;   //ÿ��ˢ�������
	int interval;    //ˢ����ʱ����

}GIFT_CONFIG;

//add by lihongwu 2011/10/25
//���������н��㲥��ʽ
enum LUCKINFO_FLAG
{
	LUCK_BUGLE   =0,  //50���������ȹ㲥
	LUCK_CONTENT =1,  //500�����Ⱥ͹���
};
#endif //ROOMTYPE_H_
