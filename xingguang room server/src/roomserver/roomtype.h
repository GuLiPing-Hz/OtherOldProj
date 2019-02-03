#ifndef ROOMTYPE_H_
#define ROOMTYPE_H_

#include "roomclient.h"
#include "netproxy.h"
#include "roomtimeout.h"

#define ROOMNUMPERPAGE 10
//add by jinguanfu 2011/8/10
#define READY_WAIT		30	//上麦准备超时时间
#define MUSIC_OFFSET	30	//上麦超时歌曲偏移值
//add by lihongwu 2011/9/20
#define MIC_REMAINTM    600  //剩余时间+延麦时间
#define SENDGIFT_SETTM    3  //送礼定时器超时

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
	LOGIN_NONE     = 0,  //未登录
	LOGIN_SUCESS   = 1,  //登陆成功
	LOGIN_AGAIN    = 2,  //重复登陆   //不用了
	LOGIN_NOUIN    = 3,  //非法用户	
	LOGIN_FULL     = 4,  //房间人数满
	LOGIN_IFULL    = 5,  //房间已经达到允许人数，用户身份无法登陆
	LOGIN_EPWD     = 6,  //房间密码错误
	LOGIN_CLOSE    = 7,  //房间关闭
	LOGIN_INBLACK  = 8,  //在黑名单中
	LOGIN_INTBLACK = 9,  //在临时黑名单中
	LOGIN_NOTEXIST = 10,	//房间不存在或被删除
	LOGIN_LOCKED= 11,	//房间冻结

};

enum ROOM_STATE
{
	ROOM_STATE_OPEN     = 0,  //房间开发状态
	ROOM_STATE_CLOSE    = 1,  //房间关闭状态
};

enum ROOM_SERVER_CMD  //cmd中有SB为广播消息
{
	ROOM_CMD_TOKEN = 1001,                  //token验证
	ROOM_CMD_LOGIN,                         //登陆
	ROOM_CMD_LEAVEROOM,                     //离开房间     
	ROOM_CMD_KEEPALIVE,                     //心跳包
	ROOM_CMD_SB_ENTERROOM,                  //某人人进入房间
	ROOM_CMD_SB_LEAVEROOM,                  //某人离开房间
	ROOM_CMD_GET_ALLINFO,                   //得到用户列表 
	ROOM_CMD_WAITMIC,                       //用户请求排麦
	ROOM_CMD_CANCLE_WAITMIC,                //用户请求取消排麦                  
	ROOM_CMD_SB_WAITMIC,                    //某人排麦成功10
	ROOM_CMD_SB_CANCLE_WAITMIC,             //某人取消排麦成功
	ROOM_CMD_WAITMIC_PK,                    //用户请求pk
	ROOM_CMD_CANCLE_WAITMIC_PK,             //用户请求取消pk
	ROOM_CMD_SB_WAITMIC_PK,                 //某人加入pk成功 
	ROOM_CMD_SB_CANCLE_WAITMIC_PK,          //某人取消pk成功 15
	ROOM_CMD_UP_WAITMIC,                    //提升某人的麦序
	ROOM_CMD_SB_UP_WAITMIC,                 //提升某人的麦序成功
	ROOM_CMD_DOWN_WAITMIC,                  //下降某人的麦序
	ROOM_CMD_SB_DOWN_WAITMIC,               //下降某人的麦序成功
	ROOM_CMD_SB_ONMIC_READY,                //有人上麦准备20
	ROOM_CMD_ONMIC_READYOK,                 //上麦准备OK
	ROOM_CMD_SB_ONMIC,                      //有人上麦
	ROOM_CMD_OFFMIC_READY,                  //下麦准备
	ROOM_CMD_SB_OFFMIC_READYOK,             //下麦准备OK, 客户端可以开始打分了
	ROOM_CMD_SCORE,                         //打分25
	ROOM_CMD_SB_OFFMIC,                     //有人下麦
	ROOM_CMD_KICKOUT_SOMEONE,               //踢出某人
	ROOM_CMD_SB_KICKOUT,                    //某人被踢出房间
	ROOM_CMD_FORBIDEN_SOMEONE,              //禁言某人
	ROOM_CMD_SB_FORBIDEN,                   //某人被禁言30
	ROOM_CMD_UPDATE_BLACKLIST,              //更新房间的黑名单
	ROOM_CMD_SB_BLACKLIST_UPDATE,           //房间黑名单被更新了  31
	ROOM_CMD_UPDATE_MGRLIST,                //更新房间的管理员名单
	ROOM_CMD_SB_MGRLIST_UPDATE,             //房间名单管理员名单被更新了     
	ROOM_CMD_ROOM_MGR_SUCCESS,              //告诉相应的客户端房间管理操作成功
	ROOM_CMD_PRIVATE_CHAT,                  //私聊
	ROOM_CMD_PUBLIC_CHAT,                   //公聊 37
	ROOM_CMD_ONVJ_MIC,                      //上vj麦
	ROOM_CMD_SB_ONVJ_MIC,                   //有人上vj麦了
	ROOM_CMD_OFFVJ_MIC,                     //下vj麦40
	ROOM_CMD_SB_OFFVJ_MIC,                  //有人下vj麦了    
	ROOM_CMD_SB_SCORE,                         //打分结束
	ROOM_CMD_GIVE_VJ_MIC,                   //给与vj麦  
	ROOM_CMD_GIVEOFF_VJ_MIC,                //下vj麦
	ROOM_CMD_GIVEOFF_MIC,                   //下普通麦45
	ROOM_CMD_SB_GIVEOFF_MIC,                 //有人被下普通麦了
	ROOM_CMD_UPDATE_CONTENT,                //更新房间的公告
	ROOM_CMD_SB_CONTENT_UPDATE,             //房间公告被更新了
	ROOM_CMD_SEND_GIFT,                   //送礼物给好友    				C=>S			NULL			int s_idx,r_idx,cate_idx,number  47
	ROOM_CMD_SB_SEND_GIFT,                //送礼物广播    				S=>C	50		NULL			int s_idx,r_idx,cate_idx,number
	//ROOM_CMD_BROAD_ALL,                     //大喇叭
	//ROOM_CMD_BROAD_LOBBY,                   //小喇叭
	ROOM_CMD_SEND_NOTICE_TEMP,               //发送房间公告    				C=>S			NULL			ResultData_SendFlowerInfo
	ROOM_CMD_SB_SEND_NOTICE_TEMP,            //收到房间公告    				S=>C			NULL			ResultData_SendFlowerInfo   50
	ROOM_CMD_INVITE_MIC,                    //邀请用户进入表演麦排序
	ROOM_CMD_SB_INVITE_MIC,                    //邀请用户进入表演麦排序
	ROOM_CMD_USER_APP_JOINROOM,             //用户申请加入房间
	//ROOM_CMD_SB_USER_APP_JOINROOM,             //用户申请加入房间
	ROOM_CMD_VERIFY_USER_APP,               //审核用户会员申请                 C=>S 
	ROOM_CMD_SB_VERIFY_USER_APP,            //审核用户会员申请回复                 S=>C  
	ROOM_CMD_REFUSEMEMBER,
	ROOM_CMD_REMOVE_USER,                   //删除会员
	ROOM_CMD_SB_REMOVE_USER, 		//60
	ROOM_CMD_GIVE_VJ_A,                      //设置为助理主持
	ROOM_CMD_SB_GIVE_VJ_A,                        //60
	ROOM_CMD_GIVE_VJ,                        //设置为主持
	ROOM_CMD_SB_GIVE_VJ,
	ROOM_CMD_GIVE_OUER_S,                    //设置为副室主
	ROOM_CMD_SB_GIVE_OUER_S,
	ROOM_CMD_SET_ROOM_PWD,                   //设置密码
	ROOM_CMD_SB_SET_ROOM_PWD,
	ROOM_CMD_SET_ROOM_LOCK,                  //房间锁定
	ROOM_CMD_SB_SET_ROOM_LOCK,			//70
	ROOM_CMD_SET_USER_ONLY,                 //设置房间只对会员开放
	ROOM_CMD_SB_SET_USER_ONLY,                  // 
	ROOM_CMD_SET_USER_INOUT,                //设置房间自由进出
	ROOM_CMD_SB_SET_USER_INOUT,
	ROOM_CMD_SET_MIC_UPDOWN,                //设置自由排序
	ROOM_CMD_SB_SET_MIC_UPDOWN,
	ROOM_CMD_SET_ROOM_NAME,                 //设置房间名称
	ROOM_CMD_SB_SET_ROOM_NAME,
	ROOM_CMD_SET_CHAT_PUBLIC,              //设置房间是否公聊
	ROOM_CMD_SB_SET_CHAT_PUBLIC,		//80
	ROOM_CMD_SET_ROOM_WELCOME,                //设置房间欢迎词
	ROOM_CMD_SB_SET_ROOM_WELCOME,               //  
	ROOM_CMD_SET_ROOM_LOGO,                   //设置房间LOGO
	ROOM_CMD_SB_SET_ROOM_LOGO,
	ROOM_CMD_ROOMAPPLYLIST_C2R2C,            //获取房间申请列表
	ROOM_CMD_GETBLACKLIST,                   //获取房间黑名单列表
	ROOM_CMD_GETROOMMEMBERLIST,              //获取房间会员列表
	/*add by jinguanfu 2010/2/25 成就模块*/
	ROOM_CMD_SB_SONGLEVEL,			//演唱等级通知
	ROOM_CMD_SB_RICHLEVEL,			//财富等级通知
	//add by jinguanfu 2010/4/7
	ROOM_CMD_KICKUSER = 1090,			//用户被踢掉	90
	//add by jinguanfu 2010/4/11
	ROOM_CMD_EXITMEMBER,			//用户退出房间会员

	ROOM_CMD_GETPANELINFO,			//取得房间设置面板信息

	//add by jinguanfu 2010/5/13
	ROOM_CMD_GIVE_MEMBER,			//由管理员降为一般会员
	ROOM_CMD_SB_GIVE_MEMBER,		//设为一般会员的通知
	//add by jinguanfu 2010/5/20
	ROOM_CMD_SB_CHANGEAVATAR,		//更改形象通知
	//add by jinguanfu 2010/6/2
	ROOM_CMD_REQUEST_GIVE_VJ_MIC,	//被邀请上主持麦后回应
	//add by jinguanfu 2010/7/14
	ROOM_CMD_SB_LEVEL,			//用户经验升级广播
	//add by jinguanfu 2010/7/19
	ROOM_CMD_SB_VIP,				//VIP、皇冠改变通知
	//add by jinguanfu 2010/8/5
	ROOM_CMD_UPDATEMONEY,		//更新钱币通知
	//add by jinguanfu 2010/8/13
	ROOM_CMD_SB_DELETEROOM=1100,	//房间删除通知
	//add by jinguanfu 2010/8/13
	ROOM_CMD_SB_LOCKROOM,		//房间冻结通知
	//add by jinguanfu 2010/8/17
	ROOM_CMD_SB_ROOMINFO_UPDATE,	//房间信息更改通知
	//add by jinguanfu 2010/8/17
	ROOM_CMD_SB_GIVE_OWNER,			//房间室主更改通知
	//add by jinguanfu 2010/9/2
	ROOM_CMD_SEND_FIREWORKS,		//赠送烟花
	//add by jinguanfu 2010/9/2
	ROOM_CMD_SB_SEND_FIREWORKS,	//赠送烟花通知
	//add by jinguanfu 2010/9/2
	ROOM_CMD_RECV_FIREWORKS,		//接收烟花
	//add by jinguanfu 2010/9/3
	ROOM_CMD_SB_LUCKY,				//幸运礼物中奖通知
	//add by jinguanfu 2010/9/8
	ROOM_CMD_SB_GIFT_INVALID,		//印章失效通知
	//add by jinguanfu 2010/9/13
	ROOM_CMD_SB_GIFT_VALID,			//印章有效通知
	//add by jinguanfu 2010/11/2
	ROOM_CMD_VIEW_INCOME=1110,		//显示可领取收益
	
	ROOM_CMD_VIEW_INCOME_LOG,		//查看收益领取记录

	ROOM_CMD_GET_INCOME,			//领取收益
	//add by jinguanfu 2010/11/17
	ROOM_CMD_SB_GIFTINFO_CHANGE,	//礼物信息更改通知
	ROOM_CMD_SB_RIGHTCONF_CHANGE,	//权限配置更新通知
	//add by jinguanfu 2010/11/26
	ROOM_CMD_SB_AVSERVER_CHANGE,	//房间音视频服务器更改
	//add by jinguanfu 2010/11/30
	ROOM_CMD_SB_UPDATE_GM,			//后台更改GM
	//add by jinguanfu 2010/12/8
	ROOM_CMD_SB_EXITMEMBER,			//用户退出会员通知
	//add by jinguanfu 2011/2/23
	ROOM_CMD_UP_NETSTATUS,			//用户上传网络状态信息
	ROOM_CMD_SB_NETSTATUS,			//向房间内用户广播网络状态信息
	//add by jinguanfu 2011/3/3
	ROOM_CMD_INVITE_MEMBER=1120,		//邀请会员C->S
	ROOM_CMD_INVITE_NOTIFY,			//邀请通知S->C
	ROOM_CMD_INVITE_REPLY,			//邀请回复C->S

	ROOM_CMD_SEND_STAR,
	//add by jinguanfu 2011/4/12
	ROOM_CMD_GET_GIFTSEND,			//取得当日礼物赠送记录
	ROOM_CMD_GET_GIFTRECV,			//取得当日礼物接收记录
	//add by jinguanfu 2011/4/19
	ROOM_CMD_SET_AUTOONMIC,			//设置房间暂时上麦
	ROOM_CMD_SB_SET_AUTOONMIC,			//暂停上麦状态通知
	//add by jinguanfu 2011/8/19
	ROOM_CMD_DISABLE_IPADDR,		//禁用对方的IP
	ROOM_CMD_DISABLE_MACADDR,		//禁用对方的MAC
	//add by lihongwu 2011-9-8
	ROOM_CMD_ADD_TM=1130,                //麦序时间延长
	ROOM_CMD_SB_ADDTM,             //麦序时间延长通知

	ROOM_CMD_LASTERROR,				//DEBUG用返回权限错误
	//add by jinguanfu 2010/11/9
	ROOM_CMD_UPDATE_MUSICINFO,		//向数据库更新歌曲的点唱率
	ROOM_CMD_DB_LUCKCONF_CHANGE,		//中奖概率更改
	//add by jinguanfu 2012/4/22
	ROOM_CMD_UPDATE_MUSICTIME,		//客户端上传歌曲时长


};

//begin by lihongwu 2011-9-8
enum MICTM_ADD_STATE
{
	ADDTM_SUCESS = 0,     //麦序时间延长成功
	ADDTM_FAILURE = 1,     //麦序时间延长失败
};
//end by lihong 2011-9-8

enum HALL_SERVER_CMD
{
	HALL_CMD_LEAVEROOM  = 0x01,
	HALL_CMD_ENTERROOM  = 0x02,
	HALL_CMD_CREATEROOM = 0x03,
	HALL_CMD_DELETEROOM = 0x04,
	HALL_CMD_LOCKROOM   = 0x05,
	HALL_CMD_GETUSERINFO=0X06,				//取得用户信息
	HALL_CMD_UPDATEROOM=0X07,				//房间信息更新
	HALL_CMD_ROOMMANAGERCHG=0X08,			//房间管理员更新
	HALL_CMD_ROOMLOGIN  =0x100,                       //房间报告自身信息
	HALL_CMD_SYNC_ROOMINFO = 0x101,			//大厅同步房间信息
	HALL_CMD_RETURN_SVERSERID =0x200,
	HALL_CMD_FLOWER_ALL       =300,      //通知大厅发送喇叭消息
	HALL_CMD_FLOWER_LOBBY     =301,

	//add by jinguanfu 2010/4/8 房间名与密码标志和大厅同步

	HALL_CMD_APPLYMEMBER_R2L = 304,		//通知大厅审核申请结果
	//HALL_CMD_KICKUSER_L2R = 305,			//大厅转发踢出用户消息
	HALL_CMD_DELMEMBER_R2L = 306,		//通知大厅删除会员
	HALL_CMD_EXITMEMBER_R2L = 307,		//通知大厅会员退出
	

	HALL_CMD_KICKUSER_L2R = 400,			//大厅转发踢出重复用户消息
	HALL_CMD_CHANGENAME_R2L =401,		//通知大厅房间名更改
	HALL_CMD_CHANGEPWD_R2L  =402,		//通知大厅房间密码更改
	HALL_CMD_CHANGECLOSEFLAG_R2L=403,		//通知大厅房间开启/关闭状态改变
	HALL_CMD_CHANGEUSERONLYFLAG_R2L=404,	//通知大厅房间私有状态改变
	HALL_CMD_UPDATEGOLD_L2R=405,				//大厅同步用户金币数与银币数
	HALL_CMD_CHANGEAVATAR_L2R=406,			//用户修改AVATAR 通知
	//add by jinguanfu 2010/6/5
	HALL_CMD_CHANGERIGHT_R2L=407,			//权限更改通知
	HALL_CMD_LEAVELOBBY_L2R=408,				//用户离开大厅通知
	//add by jinguanfu 2010/7/14
	HALL_CMD_CHANGELEVEL=409,			//用户经验升级
	

	HALL_CMD_OVERDUE_ITEM_L2R = 412,    // 通知过期的道具
	HALL_CMD_OVERDUE_VIP_L2R = 413,	// 通知过期的VIP
	HALL_CMD_EXP_CHG_R2L = 414, 		//经验改变通知
	//add by jinguanfu 2010/9/6
	HALL_CMD_LUCK_R2L = 415,			//幸运中奖通知
	//add by jinguanfu 2010/11/17
	HALL_CMD_RIGHT_CHANGE_L2R = 416,	//权限配置更改
	HALL_CMD_LUCKCONF_CHANGE_L2R = 417,	//幸运抽奖概率更改
	HALL_CMD_GIFTINFO_CHANGE_L2R = 418,	//礼物信息更改
	HALL_CMD_MOVE_ROOM_L2R = 419,		//房间移动 
	//add by jinguanfu 2010/11/27
	HALL_CMD_UPDATE_AVSERVER_L2R=420,	//视音频服务器更新
	HALL_CMD_VUSER_LOGIN_L2R = 421,		//后台登陆虚拟用户
	HALL_CMD_VUSER_LOGOUT_L2R=422,		//后台退出虚拟用户

	HALL_CMD_UPDATE_GM_L2R=424,			//后台更改GM
	//add by jinguanfu 2011/1/11
	HALL_CMD_ADD_SEEL_R2L = 425, 		//增加印章
	HALL_CMD_REMOVE_SEEL_L2R = 426, 	//删除印章

	//add by jinguanfu 2011/1/18
	HALL_CMD_LOGIN=427,			//向大厅请求是否允许用户登录

	//add by jinguanfu 2011/3/11
	HALL_CMD_FIREWORKS_R2L = 428,	//向大厅发送送烟花通知

	//add by jinguanfu 2011/3/22
	HALL_CMD_CHANGELOGO_R2L = 429,//向大厅发送更改logo通知
	//add by jinguanfu 2011/5/31
	HALL_CMD_ROOMBLACKMEMCHG_L2R = 430, //房间黑名单操作

	//add by jinguanfu 2011/8/19
	HALL_CMD_UPDATE_ROOMCHAT_BLACKLIST_L2R =431,	//大厅通知房间更新公聊黑名单
	HALL_CMD_KICKUSER_R2L = 432,		//房间通知大厅踢出用户
	//add by lihongwu 2011/9/20
	HALL_CMD_UPDATE_ROOMGIFT_REFRESH_TIMES_L2R =433,  //大厅通知房间更新礼物刷新频率

};

enum LOG_SERVER_CMD
{
	CMD_MUSIC_CHECK_R2LS		=0,	//音乐点击报告
	CMD_ROOM_KEEPALIVE_R2LS	=1,	//用户上麦状态报告
	CMD_ROOM_ONMIC_R2LS		=2,	//上表演麦报告
	CMD_ROOM_OFFMIC_R2LS		=3,	//下表演麦报告
	CMD_ROOM_ONVJMIC_R2LS		=4,	//上主持麦报告
	CMD_ROOM_OFFVJMIC_R2LS	=5,	//下主持麦报告
	CMD_ROOM_ERRORINFO_R2LS	=6,	//错误报告
	CMD_ROOM_USERSTATUS_R2LS =7,	//用户机器状态报告
	CMD_ROOM_ENTERROOM_R2LS	=8,	//用户进入房间
	CMD_ROOM_LEAVEROOM_R2LS	=9,	//用户离开房间

};


enum MIC_INFO_STATE
{
	MIC_INFO_WAIT = 0,              //排麦s
	MIC_INFO_ONMIC_READY,           //上麦开始准备
	MIC_INFO_ONMIC_READYOK,         //上麦准备完成
	MIC_INFO_ONMIC,                 //上麦中
	MIC_INFO_OFFMIC_READY,          //下麦开始准备
	MIC_INFO_OFFMIC_READYOK,        //下麦准备完成
	MIC_INFO_SCORE,                 //上传分数完成
	MIC_INFO_OFFMIC,			//下麦
};

enum _TIMEOUT_TYPE
{
	TIMEOUT_TB = 0,              //临时黑名单
	TIMEOUT_SCORE,             //打分
	TIMEOUT_ONMIC,			//上麦
	TIMEOUT_FORBIDEN,		//禁言
	TIMEOUT_OFFMIC,		//下麦
	//add by jinguanfu 2011/8/10
	TIMEOUT_READY,			//上麦准备超时
};
//add  by jinguanfu 2010/11/26
//送礼物任务信息
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
	//int roomid;		//房间ID
	int s_idx;		//送礼者idx
	int r_idx;		//接收礼物者idx
	int cate_idx;		//礼物id
	int number;		//礼物总数
	int havesend;	//礼物已发数量
	int seq;			//
	int price;		//礼物单价
	//add by lihongwu 2011/10/11
	int priceconf;  //礼物可配置中价格配置     
	int numpertime;     //每次刷礼物个数
	int interval;   //刷礼物间隔时间  

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
		//add by jinguanfu 2012/2/24 简化版
		memset(musicname,0,sizeof(musicname));
		musicspeed=0;
		reverseflag=0;
    }

	int idx;                          //排麦人的idx
	short musicid;                    //音乐id
	short bk;                         //背景
	char state;                       //状态
	int score;                        //分数
	int pkidx;                        //pk人的idx
	short pkmusicid;                  //pk的音乐id
	short pkbk;                       //pk的背景
	char pkstate;                     //pk状态
	int pkscore;                      //pk分数
	char level;                       //难度
	//add by jinguanfu 2010/6/5
	char scoreinfo[128];		//打分信息
	char pkscoreinfo[128];		//pk打分信息
	//add by jinguanfu 2011/8/10
	short musictime;					//歌曲时长(秒)
	//add by jinguanfu 2012/2/23
	char musicname[128];		//歌曲名
	//add by jinguanfu 2012/2/27
	char musicspeed;			//歌曲速度
	//add by jinguanfu 2012/4/5
	char reverseflag;			//反调标示位。女声男唱，男声女唱
};
//add by jinguanfu 2010/4/19
//房间管理员
typedef struct _ROOM_MANAGER_
{
	int m_idx;		//管理员idx
	char m_identity;	//管理员身份
}ROOM_MANAGER;
//add by jinguanfu 2010/4/19
//房间会员申请列表
typedef struct _ROOM_APPLY_
{
	int m_roomid;	//申请房间号
	int m_idx;		//申请人idx
	char m_time[32];	//申请时间
}ROOM_APPLY;

typedef struct _ROOM_INFO_
{	
	int roomid;              //房间的id号  
	int ownidx;              //房间的室主idx
	int secondownidx;              //房间的副室主idx
	int secondownidx2;              //房间的副室主idx

	map<int, int> vjlist;             //房间的主持人列表
	map<int, int> vjlist_a;             //房间的助理主持人列表
	int vjonmic;             //在麦上的主持人
	MIC_INFO onmic;                   //房间持麦的信息
	short allowhuman;        //允许人数
	short maxhuman;          //最大人数
	char roomname[51];                //房间名称
	//char introduce[255];              //房间简介
	char content[301];                //房间公告
	char passwd[25];                  //房间密码
	char state;                       //房间状态
	char type;                        //房间的类型
	vector<MIC_INFO*> miclist;         //房间排麦列表
	map<int, int> blacklist;          //房间黑名单
	map<int, int> tblacklist;         //房间临时黑名单，10分钟后自动消除
	//map<int, int> managerlistA;       //A级别主持人名单
	//map<int, int> managerlistB;       //B级别主持人名单
	map<int,int>  userlistVIP;        //房间会员列表
	map<int,ROOM_APPLY>  userlistAPP;        //房间用户申请列表
	map<int, RoomClient*> userlist;   //房间用户列表
	//add by jinguanfu 2010/4/12
	map<int,roomtimeout*>  forbidenlist;		//房间禁言列表
	//map<int,Flowerresultdata*> giftlist;			//房间送礼列表
	vector<SEND_GIFT_INFO*> sendgiftlist;		//送礼任务表
	//add by jinguanfu 2010/11/26 begin
	Flowerresultdata*  FlowerTimer;			//礼物计数计时器
	//add by jinguanfu 2010/11/26 end
	map<int,ROOM_MANAGER> managerlist;		//房间管理员列表(助理主持以上)
	char roomlogo[301];               //房间LOGO
	char welcomeword[401];            //房间欢迎词
	bool isPublicChat;                //允许房间公聊
	bool isUserInOut;                 //用户进出信息开启/关闭
	bool isMicUpdown;                 //自由上麦开启/关闭?
	bool isUserOnly;                  //设置房间仅对会员开放
	bool isClose;                      //开启/关闭
	//add by jinguanfu 2011/4/19
	bool isAutoOnmic;					//自动 上麦

	map<int,int> userlistMale;		//在线男生
	map<int,int> userlistFemale;	//在线女生
	//add by jinguanfu 2010/9/13
	map<int,ROOM_MANAGER> managerlist_online;		//房间在线管理员(助理主持以上)
	//map<int,GiftTimeout*> seallist;	//印章列表
	
	roomtimeout* m_pOnMicTimeout;	//上麦超时

	//add by jinguanfu 2010/11/25
	char isBlacklistOk;		//黑名单取得成功标志位
	char isMemberlistOk;		//会员列表取得成功标志位
	char isApplylistOk;		//会员申请列表取得成功标志位

	//add by jinguanfu 2010/11/25
	//modify by lihongwu 2011/11/18
	char AVServerIP_telcom[256];	//音视频服务器地址--电信 
	short AVServerPort_telcom;	//音视频服务器端口--电信
	char AVServerIP_netcom[256];	//音视频服务器地址--网通
	short AVServerPort_netcom;	//音视频服务器端口--网通

	map<int, RoomClient*> vuserlist;   //虚拟用户列表
	//add by jinguanfu 2011/3/3
	map<int,int> invitelist;			//会员邀请列表map<被邀请者,邀请者>
	
} ROOM_INFO;

typedef struct _ROOM_LIST_INFO_ //大厅所有房间列表
{
	int hallid;                     //大厅id
	short roomnum;                  //属于这个大厅id房间的数量
	map<int, ROOM_INFO*> roommap;    //属于这个大厅id房间列表,使用房间id相对应   

} ROOM_LIST_INFO;

enum USER_IDENTITY
{
	USER_ID_NONE      = 0,  	//普通用户
	USER_ID_OWNER     = 1,  	//室主
	USER_ID_A_MANAGER = 2, //A管理员
	USER_ID_B_MANAGER = 3, //B管理员
	USER_ID_VJ        = 4,  	//主持人
	USER_ID_VJ_A      = 5, 	//助理主持人
	USER_ID_VIP       = 6,  	//房间会员
	USER_ID_GM        = 7,  	//游戏管理员
	USER_ID_OWNER_S   = 8,  //副室主
	USER_ID_GVIP=9,		//游戏VIP用户
	USER_ID_RED=10,		//红冠用户
	USER_ID_PURPLE =11,	//紫冠用户
	USER_ID_SUPER =12,		//超冠用户
	USER_ID_IMPERIAL =13,	//至尊用户

	USER_ID_GUEST = 14,	//游客

};

enum MIC_UP_DOWN_FLAG
{
	MIC_UP_DOWN_ONE      = 0,  //升降一位
	MIC_UP_DOWN_ALL      = 1,  //升降到顶
};

enum WAITMIC_UP_DOWN_SEARCH_
{
	WAITMIC_UP_DOWN_IDX      = 0,  //idx搜索
	WAITMIC_UP_DOWN_PKIDX    = 1,  //pkidx搜索
};

enum USER_APP_REF
{
	SUCCESS      = 0, //成功
	ALREADY    	=-1,//已是本房间会员或其他房间会员
	LISTFULL 	=-2,	//会员列表或黑名单已满
	REFUSE		=-3,//服务器拒绝，申请被拒绝后24小时内不能再申请
	IAGAIN		=-4,//重复重申，等待管理员审核
	APPLYFULL	=-5,//房间申请列表已满

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
	ROOM_TYPE_ROOM=0,	//演唱房
	ROOM_TYPE_SMALL=1,	//练歌房

};

// 演唱得分信息
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

	int score;// 演唱分数
	int combo_num;// combo数量
	int perfect_num;
	int great_num;
	int good_num;
	int poor_num;
	int miss_num;
}NETSINGSCORE;

enum VIP_LEVEL				//vip等级
{
	VIP_LEVEL_NONE		=0,		//普通用户
	VIP_LEVEL_VIP		=1,		//VIP用户
	VIP_LEVEL_LIFEVIP	=2,		//终生VIP用户
	VIP_LEVEL_RED		=100,	//红冠用户
	VIP_LEVEL_PURPLE	=200,	//紫冠用户
	VIP_LEVEL_SUPER	=300,	//超冠用户
	VIP_LEVEL_IMPERIAL	=400,	//至尊用户
};

enum MONITOR_CMD
{
	MONITOR_CMD_REPORT=1,
	MONITOR_CMD_REPLY=4,		//房间服务器代号
};

//add by jinguanfu 2010/9/6
typedef struct _GIFT_INFO_
{
	int GiftID;		//礼物ID
	int type;			//类型  1:一般 2:幸运 3:大烟花 4:小烟花
	int price;		//礼物单价
	int vtime;		//礼物有效时间(分钟)，仅对印章有效
	//add by lihongwu 2011/10/11
	int priceconf;  //礼物可配置中价格配置    //这关键字都没用到
	int number;     //每次刷礼物个数
	int interval;   //刷礼物间隔时间
}GIFT_INFO;

//add by jinguanfu 2010/9/18
#define  DB_TIMEOUT	10	//数据库操作超时(秒)

enum DBRESULT_GIFT	//礼物、道具赠送
{
	DBRESULT_GIFT_NOMONEY	=-1,		//余额不足
	DBRESULT_GIFT_SUCCESS		=0,		//成功
	DBRESULT_GIFT_NULL		=1,		//交易金额为0
	DBRESULT_GIFT_NOGIFT		=2,		//无此交易类型
	DBRESULT_GIFT_OFFLINE		=3,		//礼物接收者不房间内

};

enum DBRESULT_MEMBER	//会员操作
{
	DBRESULT_MEMBER_FULL		=-1,		//人数已满
	DBRESULT_MEMBER_SUCCESS	=0,		//成功
};

enum DBRESULT_VERIFY		//审核申请
{

	DBRESULT_VERIFY_SUCCESS	=0,		//成功
	DBRESULT_VERIFY_ALREADY	=1,		//已是本房间或其他房间会员
	DBRESULT_VERIFY_FULL		=2,		//会员数已满
};

enum DBRESULT_APPLY		//申请会员
{
	DBRESULT_APPLY_FULL		=-1,		//人数已满
	DBRESULT_APPLY_SUCCESS	=0,		//成功
	DBRESULT_APPLY_NONE		=1,		//
	DBRESULT_APPLY_AGAGN		=2,		//重复申请
	DBRESULT_APPLY_REFUSE	=3,		//服务器拒绝(管理员拒绝24小时内不能再申请)
	DBRESULT_APPLY_ALREADY	=4,		//已是本房间或其他房间会员
};

enum DBRESULT_SCORE		//打分后加经验操作
{

	DBRESULT_SCORE_SUCCESS	=0,		//成功
	DBRESULT_SCORE_FALIED		=1,		//失败
};

enum GIFT_TYPE
{
	GIFT_TYPE_NONE		=1,		//一般礼物
	GIFT_TYPE_LUCK		=2,		//幸运礼物
	GIFT_TYPE_BFIRE	=3,		//大烟花
	GIFT_TYPE_SFIRE	=4,		//小烟花
	GIFT_TYPE_SEAL		=5,		//印章
	
};

enum DBRESULT_INCOME		//领取房间收益
{
	DBRESULT_INCOME_SUCCESS	=0,		//成功
	DBRESULT_INCOME_FALIED	=1,		//失败
};

enum DBRESULT_BLACKLIST	//操作房间黑名单
{
	DBRESULT_BLACKLIST_SUCCESS	=0,
	DBRESULT_BLACKLIST_FALIED	=1,	//已在黑名单中或已从黑名单里删除
};

//add by jinguanfu 2010/11/23
#define  OFFMIC_WAIT	5	//下麦后，下一个上麦等待时间(秒)

//房间服务器内部用命令
enum INNER_CMD
{
	INNER_CMD_GETROOMINFO=2000,	//取得房间基本信息
	INNER_CMD_GETMEMBERLIST,			//取得房间会员列表
	INNER_CMD_GETBLACKLIST,		//取得房间黑名单
	INNER_CMD_GETAPPLYLIST,		//取得房间会员申请列表
	//add by jinguanfu 2011/5/31
	INNER_CMD_UPDATEROOMMANGER,		//更新房间管理员列表
	INNER_CMD_UPDATEROOMBLACKLIST,	//更新房间黑名单

};

//add by jinguanfu 2010/12/1
typedef struct _GM_INFO_
{
	int idx;			//用户idx
	char addflag;		//0:取消GM 1:成为GM
}GM_INFO;

//add by jinguanfu 2011/1/25
//用户在麦状态
enum USER_STATE
{
	USER_STATE_NONE		=0,		//未在麦上
	USER_STATE_WAITMIC	=1,		//麦序中
	USER_STATE_ONMIC		=2,		//表演麦上
	USER_STATE_ONVJMIC	=3,		//主持麦上
};

//add by jinguanfu 2011/2/21  
typedef struct _RIGHT_CONFIG_
{
	char optiontype;
	char rightdetail[16][16];
}RIGHT_CONFIG;

enum RIGHT_OPTION
{
	RIGHT_OPTION_CHKPWD			=1,		//验证进入房间密码
	RIGHT_OPTION_CHKCLOSE			=2,		//验证进入关闭房间
	RIGHT_OPTION_CHKPRIVATE		=3,		//验证进入会员房间
	RIGHT_OPTION_CHKFULL			=4,		//验证进入满员房间
	RIGHT_OPTION_ONVJMIC			=5,		//上主持麦
	RIGHT_OPTION_CHGWAITMIC		=6,		//调整麦序
	RIGHT_OPTION_FREEWAITMIC		=7,		//非自由排麦下排麦
	RIGHT_OPTION_FORBIDEN			=8,		//禁言
	RIGHT_OPTION_BLACK			=9,		//操作黑名单
	RIGHT_OPTION_KICKOUT			=10,		//踢用户出房间
	RIGHT_OPTION_GIVEVJMIC		=11,		//邀请主持麦
	RIGHT_OPTION_OFFVJMIC			=12,		//下别人主持麦
	RIGHT_OPTION_OFFMIC			=13,		//下别人表演麦
	RIGHT_OPTION_DELWAITMIC		=14,		//删除麦序
	RIGHT_OPTION_INVITEMIC		=15,		//邀请别人排麦
	RIGHT_OPTION_SENDCHAT			=16,		//发送房间公聊
	
	RIGHT_OPTION_GETAPPLYLIST	=17,		//取得会员审请列表
	RIGHT_OPTION_AUDITAPPLY		=18,		//审核会员申请
	RIGHT_OPTION_MEMBERLIST		=19,		//查看会员列表
	RIGHT_OPTION_DELMEMBER		=20,		//删除房间会员
	RIGHT_OPTION_GIVEMEMBER		=21,		//管理员降为会员
	RIGHT_OPTION_GIVEVJA			=22,		//设置为助理主持
	RIGHT_OPTION_GIVEVJ			=23,		//设置为主持
	RIGHT_OPTION_GIVESUBOWNER	=24,		//设置为副室主
	RIGHT_OPTION_GETBLACK			=25,		//取得黑名单
	RIGHT_OPTION_SETROOMPWD		=26,		//设置房间密码
	RIGHT_OPTION_SETROOMCLOSE	=27,		//关闭、开放房间
	RIGHT_OPTION_SETROOMPRIVATE	=28,		//设置会员房间
	RIGHT_OPTION_SETROOMINOUT	=29,		//设置房间进出信息
	RIGHT_OPTION_SETFREEMIC		=30,		//设置房间内自由上麦
	RIGHT_OPTION_SETROOMNAME		=31,		//设置房间名
	RIGHT_OPTION_SETROOMCONTENT	=32,		//设置房间公告
	RIGHT_OPTION_SENDCONTENT		=33,		//发送房间临时公告
	RIGHT_OPTION_SETROOMCHAT		=34,		//设置房间公聊
	RIGHT_OPTION_SETWELCOMEWORD	=35,		//设置房间欢迎词
	RIGHT_OPTION_SETROOMLOGO		=36,		//设置房间LOGO
	RIGHT_OPTION_SETAUTOONMIC	=37,		//设置房间自动 上麦
	//add by jinguanfu 2011/8/19
	RIGHT_OPTTON_DISABLEMACIP   =38,		//禁用IP或MAC
	//add by lihongwu 2011/9/30
	RIGHT_OPTION_ADDTM    =39,              //麦序时间延长

};


//add by jinguanfu 2011/3/9
//星光工厂服务器
enum FACTORY_CMD
{
	
	FACTORY_CMD_LOGIN_R2F			=0,		//用户进入房间
	FACTORY_CMD_LOGOUT_R2F,				//用户退出房间
	FACTORY_CMD_KEEPALIVE_R2F,			//用户心跳
	FACTORY_CMD_STAR_NUM_F2R,		//生成星光道具通知
};

//add by jinguanfu 2011/3/22 
//邀请主持麦后回应
enum RESULT_GIVEVJMIC
{
	RESULT_GIVEVJMIC_ACCEPT	=0,	//被邀请者同意上麦
	RESULT_GIVEVJMIC_REFUSED,	//被邀请者拒绝上麦
	RESULT_GIVEVJMIC_ONVJMIC,	//有人在主持麦上
	RESULT_GIVEVJMIC_ONMIC,		//被邀请者在表演麦上
};

//add by jinguanfu 2011/8/11
//用户防沉迷状态
enum USER_STATUS
{
	USER_STATUS_NONE 	=0,		//未认证
	USER_STATUS_JUST 	=1,		//暂时非防沉迷
	USER_STATUS_BAD 	=2,		//防沉迷
	USER_STATUS_GOOD 	=3,		//非防沉迷

};

//add by jinguanfu 2011/9/27
//用户TOKEN认证返回值
enum TOKENLOGIN_RET
{
	TOKENLOGIN_SUCCESS =0,	//成功
	TOKENLOGIN_TIMEOUT =-1,	//失败，token超时
	TOKENLOGIN_INVALID =-2,	//失败，该用户未登陆大厅
};

//add by lihongwu 2011/10/10
//礼物可配置的配置信息
typedef struct _GIFT_CONFIG
{
	_GIFT_CONFIG()
		: price(0)
		, number(0)
		, interval(0)
	{
	}

	int price;       //价格配置
	int number;   //每次刷礼物个数
	int interval;    //刷礼物时间间隔

}GIFT_CONFIG;

//add by lihongwu 2011/10/25
//幸运礼物中奖广播方式
enum LUCKINFO_FLAG
{
	LUCK_BUGLE   =0,  //50倍以上喇叭广播
	LUCK_CONTENT =1,  //500倍喇叭和公告
};
#endif //ROOMTYPE_H_
