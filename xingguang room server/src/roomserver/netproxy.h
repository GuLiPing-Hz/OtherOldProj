#ifndef NETPROXY_H_
#define NETPROXY_H_

#include "network/listensocket.h"
#include "network/clientsocket.h"
#include "network/reactor.h"
#include "network/counter.h"
#include "ac/util/objectpool.h"
#include "network/datadecoder.h"
#include "network/package.h"
#include "serverconf.h"
#include "ac/util/protocolstream.h"
#include "roomtype.h"
#include "roomclient.h"
#include "roomlisten.h"
#include "roomtimeout.h"
#include "HallSvrClient.h"
#include <sys/time.h>
#include <errno.h>
#include "MonitorSvrClient.h"
#include "rightconfig.h"
#include "rightconfigEx.h"

//add by jinguanfu 2010/9/27
#define  DBAGENT_COUNT 	1	//数据库连接组数

#define NOT_USED(V) ((void) V)

using namespace ac;
class roomtimeout;
class NetProxy;
class RightConfig;
class RightConfigEx;
extern NetProxy *g_pNetProxy;
extern char g_configfilename[128];
extern char g_rightpath[128];


class ClientDataDecoder : public DataDecoder
{
	public:
		ClientDataDecoder(int pttype,size_t hdlen) : DataDecoder(pttype,hdlen){}
		virtual ~ClientDataDecoder(){}
		virtual int OnPackage(ClientSocketBase *pClient,const char* buf,size_t buflen);
	public:
		int DoToken(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		int DoLogin(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		int DoGetAllInfo(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		int DoWaitMic(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		int DoCancleWaitMic(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		int DoWaitMicPK(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		int DoCancleWaitMicPK(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		int DoUpWaitMic(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		int DoDownWaitMic(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		int DoOnMicReadyOK(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		int DoOffMicReady(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		int DoScore(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		int DoKickSb(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		int DoForbidenSb(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		int DoUpdateBlacklist(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		/*delete by jinguanfu 2010/8/27
		int DoUpdateMgr(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		int DoPrivateChat(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		*/
		int DoPublicChat(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		int DoOnVJMic(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		int DoOffVJMic(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		int DoGiveVJMic(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		int DoGiveOffVJMic(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		int DoGiveOffMic(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		int DoUpdateContent(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		int DoLeaveRoom(ClientSocketBase *pClient);
		int DoSendFlower(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		//int DoBroad(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		//new method
		int DoSendNotice(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		int DoInviteMic(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		int DoJoinRoomApp(ClientSocketBase *pClient,  short cmd, int seq/*, BinaryReadStream* pinstream*/);
		int DoVerifyUserApp(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		int DoRemoveUser(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		int DoGiveVJA(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		int DoGiveVJ(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		int DoGiveOwnS(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);		

		int DoSetRoomPwd(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		int DoSetRoomLock(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		int DoSetUserOnly(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		int DoSetUserInOut(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		int DoSetMicUpDown(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		int DoSetRoomName(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		int DoSetChatPublic(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		int DoSetRoomWelcome(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		int DoSetRoomLogo(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);

		int DoReturnRoomApplyList(ClientSocketBase *pClient,  short cmd, int seq/*, BinaryReadStream* pinstream*/);
		int DoReturnRoomBlackList(ClientSocketBase *pClient,  short cmd, int seq/*, BinaryReadStream* pinstream*/);
		int DoReturnRoomMemberList(ClientSocketBase *pClient,  short cmd, int seq/*, BinaryReadStream* pinstream*/);


		/*static  void *DoSBMICOff(void *arg);*/
		//检查miclist，如果持麦没有人，则走上麦流程
		int CheckMiclistOnmic(ROOM_INFO* proominfo);

		int DoExitRoomMember(ClientSocketBase *pClient,  short cmd, int seq/*, BinaryReadStream* pinstream*/);
		int DoGetPanelInfo(ClientSocketBase *pClient,  short cmd, int seq/*, BinaryReadStream* pinstream*/);
		//由管理员设为一般会员 add by jinguanfu 2010/5/13
		int DoGiveMember(ClientSocketBase *pClient,  short cmd, int seq,BinaryReadStream* pinstream);
		//邀请VJ麦后，被邀请用户的回应add by jinguanfu 2010/6/3
		int DoGiveVJMicRes(ClientSocketBase *pClient,  short cmd, int seq,BinaryReadStream* pinstream);
		//赠送礼物(烟花) add by jinguanfu 2010/8/30
		int DoSendFireworks(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		
		//有人离开清除排麦列表
		int ClearWaitmicList(ROOM_INFO* proominfo, int idx);

		//有人离开清除持麦列表
		int ClearOnmic(ROOM_INFO* proominfo, int idx);
		//显示房间可领取收益add by jinguanfu 2010/11/2
		int DoViewIncome(ClientSocketBase * pClient, short cmd, int seq, BinaryReadStream * pinstream);
		//显示收益领取记录add by jinguanfu 2010/11/2
		int DoViewIncomeLog(ClientSocketBase * pClient, short cmd, int seq, BinaryReadStream * pinstream);
		//领取房间收益add by jinguanfu 2010/11/2
		int DoGetIncome(ClientSocketBase * pClient, short cmd, int seq, BinaryReadStream * pinstream);
		//客户端心跳处理add by jinguanfu 2011/1/24
		int DoKeepAlive(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		//表演麦或主持麦上用户上传网络状态 add by jinguanfu 2011/2/23
		int DoBroadCastNetstatus(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		//add by jinguanfu 2011/3/3
		//房间管理员邀请用户成为会员 
		int DoInviteMember(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		//被邀请者回应会员邀请
		int DoReplyInvite(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);

		//add by jinguanfu 2011/4/12
		//取得当日赠送礼物记录
		int DoGetGiftSend(ClientSocketBase *pClient,  short cmd, int seq);
		//取得当日接收礼物记录
		int DoGetGiftRecv(ClientSocketBase *pClient,  short cmd, int seq);
		//add by jinguanfu 2011/4/19
		//设置房间暂停上麦
		int DoSetRoomAutoOnmic(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		//add by jinguanfu 2011/8/19
		//禁用用户IP
		int DoDisableIP(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		//禁用用户MAC
		int DoDisableMAC(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);
		//add by lihongwu 2011-9-8
		//麦序时间延长
		int DoOnMicAddTM(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);

		//add by jinguanfu 2012/4/22
		//更新歌曲时长
		int DoUpdateMusicTime(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream);

		//检查上麦的时候直接将下一个上麦,取消上麦准备过程
		int CheckMiclistOnmic2(ROOM_INFO* proominfo);
protected:
		int SendToHall(ClientSocketBase *pClient, short cmd,ROOM_INFO* pRoominfo);
		int SendToDBAgent(ClientSocketBase *pClient, short cmd, int idx = 0, char badd = 0);

		int ClearOnmicIdxEqualIdx(ROOM_INFO* proominfo);
		int ClearOnmicIdxEqualPKIdx(ROOM_INFO* proominfo);

       
		int CheckRight(char op_identity/*操作者房间内身份*/,
									char op_viplevel,/*操作者VIP身份*/
									char bop_identity/*被操作者房间内身份*/,
									char bop_viplevel/*被操作者VIP身份*/,
									short cmd);
		//add by jinguanfu 2011/2/21
		int CheckRightEx(char op_identity/*操作者房间内身份*/,
									char op_viplevel,/*操作者VIP身份*/
									char bop_identity/*被操作者房间内身份*/,
									char bop_viplevel/*被操作者VIP身份*/,
									short cmd);
				

		vector<MIC_INFO*>::iterator  SearchWaitMic(vector<MIC_INFO*>::iterator itbegin,
											vector<MIC_INFO*>::iterator itend,int idx);

		vector<MIC_INFO*>::iterator  SearchPKWaitMic(vector<MIC_INFO*>::iterator itbegin,
											vector<MIC_INFO*>::iterator itend,int idx);

		int LoginCheck(char op_identity/*操作者房间内身份*/,
									char op_viplevel/*操作者VIP身份*/,
									char type/*0:密码房间 1:房间关闭 2:会员房间*/);
		
		//add by jinguanfu 2011/2/22
		int LoginCheckEx(char op_identity/*操作者房间内身份*/,
									char op_viplevel/*操作者VIP身份*/,
									char type/*0:密码房间 1:房间关闭 2:会员房间*/);
		//向工厂服务器发送消息
		//int SendToFactory(ClientSocketBase *pClient, short cmd);
		//add by jinguanfu 2011/4/27
		//向日志服务器发送消息
		int SendToLogServer(ClientSocketBase *pClient, short cmd,int idx,int roomid);
		
};

class HallDataDecoder : public DataDecoder
{
public:
	HallDataDecoder(int pttype,size_t hdlen) : DataDecoder(pttype,hdlen){}
	virtual ~HallDataDecoder(){}
	virtual int OnPackage(ClientSocketBase *pClient,const char* buf,size_t buflen);
	//add by ym
	int DoCreateRoom(short cmd,int seq,BinaryReadStream *pinstream);
	int DoDeleteRoom(BinaryReadStream *pinstream);/*ClientSocketBase *pClient,short cmd,int seq,*/
	int DoBlockRoom(short cmd,int seq,BinaryReadStream *pinstream);
	//add by jinguanfu 2010/4/12
	int DoKickUser(BinaryReadStream *pinstream);

	int DoUpdateUserinfo(short cmd,int seq,BinaryReadStream *pinstream);
	//add by jinguanfu 2010/5/20
	int DoChangeAvatarNotice(BinaryReadStream *pinstream);	//更改形象通知
	//add by jinguanfu 2010/6/7
	int DoLeaveLobby(BinaryReadStream *pinstream);		//用户退出大厅

	int DoUpdateLevel(BinaryReadStream *pinstream);

	int DoUpdateVip(BinaryReadStream *pinstream);		//VIP/皇冠过期通知

	//int DoUpdateItem(BinaryReadStream * pinstream);	//道具过期通知
	
	int DoUpdateCoin(BinaryReadStream *pinstream);
	//add by jinguanfu 2010/8/13
	int DoUpdateRoomInfo(short cmd, int seq,BinaryReadStream *pinstream);
	//add by jinguanfu 2010/8/13
	//更新房间内用户权限功能
	int DoUpdateRoomManger(short cmd, int seq,BinaryReadStream *pinstream);
	
	//add by jinguanfu 2010/9/28
	int DoSyncRoomInfo(short cmd, int seq,BinaryReadStream *pinstream);
	//add by jinguanfu 2010/11/17
	int DoLuckConfChange(BinaryReadStream *pinstream);	//后台更改中奖概率配置信息
	int DoGiftInfoChange(BinaryReadStream *pinstream);	//后台更改礼物信息
	int DoRightConfChange(BinaryReadStream *pinstream);	//后台更改权限配置
	//add by jinguanfu 2010/11/24
	int DoMoveRoom(short cmd, int seq,BinaryReadStream *pinstream);	//迁移房间
	//add by jinguanfu 2010/11/27
	int DoUpdateAVServer(short cmd, int seq,BinaryReadStream *pinstream);	//音视频服务器更改
	int DoVUserLogin(short cmd, int seq,BinaryReadStream *pinstream);	//后台登陆虚拟用户
	int DoVUserLogout(short cmd, int seq,BinaryReadStream *pinstream);	//后台退出虚拟用户
	//add by jinguanfu 2010/11/30
	int DoUpDateGM(short cmd, int seq/*,BinaryReadStream *pinstream*/);		//后台更改GM
	//add by jinguanfu 2011/1/11
	int DoRemoveSeal(short cmd, int seq,BinaryReadStream *pinstream);	   //印章失效
	//add by jinguanfu 2011/5/31
	int DoUpdateRoomBlacklist(short cmd, int seq,BinaryReadStream *pinstream);//更新房间黑名单
	//add by jinguanfu 2011/8/19
	int DoUpdateChatBlacklist(short cmd, int seq,BinaryReadStream *pinstream);//更新公聊黑名单
	//add by lihongwu 2011/9/20
	int DoUpdateGiftConfig(short cmd, int seq);   //更新礼物刷新频率
};

class DbagentDataDecoder : public DataDecoder
{
public:
	DbagentDataDecoder(int pttype,size_t hdlen) : DataDecoder(pttype,hdlen){}
	virtual ~DbagentDataDecoder(){}
	virtual int OnPackage(ClientSocketBase *pClient,const char* buf,size_t buflen);

public:
	/* modify by jinguanfu 2010/1/19
 	int DoUpdateBlacklist(DB_RESULT_DATA* pdata);
	int DoUpdateMgr(DB_RESULT_DATA* pdata);
    int DoUpdateContent(DB_RESULT_DATA* pdata);
	int DoSendFlower(DB_RESULT_DATA* pdata);
	int DoVerifyUser(DB_RESULT_DATA* pdata);
	int DoRemoveUser(DB_RESULT_DATA* pdata);
	int DoGiveVJA(DB_RESULT_DATA* pdata);
	int DoGiveVJ(DB_RESULT_DATA* pdata);
	int DoGiveOwnS(DB_RESULT_DATA* pdata);
	int DoSetRoomLock(DB_RESULT_DATA* pdata);
	int DoSetRoomLock(DB_RESULT_DATA* pdata);
	int DoSetRoomUserOnly(DB_RESULT_DATA* pdata);
	int DoSetRoomUserOnly(DB_RESULT_DATA* pdata);
	int DoSetRoomInOut(DB_RESULT_DATA* pdata);
	int DoSetRoomInOut(DB_RESULT_DATA* pdata);
	int DoSetRoomMicUpDown(DB_RESULT_DATA* pdata);
	int DoSetRoomName(DB_RESULT_DATA* pdata);
	int DoSetChatPublic(DB_RESULT_DATA* pdata);
	int DoSetRoomWelcome(DB_RESULT_DATA* pdata);
	int DoSetRoomLogo(DB_RESULT_DATA* pdata);
	int DoReturnRoomApplyList(DB_RESULT_DATA* pdata,BinaryReadStream* infostream,short row,short col,short cmd);
	int DoRoomUserApp(DB_RESULT_DATA* pdata,BinaryReadStream* infostream);
	int DoReturnRoomBlackList(DB_RESULT_DATA* pdata,BinaryReadStream* infostream,short row,short col,short cmd);
	int DoReturnRoomMemberList(DB_RESULT_DATA* pdata,BinaryReadStream* infostream,short row,short col,short cmd);
	*/
	int DoUpdateBlacklist(Dbresultdata* pdata,BinaryReadStream* infostream);
	int DoUpdateMgr(Dbresultdata* pdate);
	int DoUpdateContent(Dbresultdata * pdata);
	int DoSendFlower(Dbresultdata* pdata,BinaryReadStream* infostream); 
	int DoVerifyUser(Dbresultdata* pdata,BinaryReadStream* infostream);
	int DoRemoveUser(Dbresultdata* pdata,BinaryReadStream* infostream);
	int DoGiveVJA(Dbresultdata* pdata,BinaryReadStream* infostream);
	int DoGiveVJ(Dbresultdata* pdata,BinaryReadStream* infostream);
	int DoGiveOwnS(Dbresultdata* pdata,BinaryReadStream* infostream);
	int DoSetRoomLock(Dbresultdata* pdata);
	int DoSetRoomUserOnly(Dbresultdata* pdata/*,BinaryReadStream* infostream*/);
	int DoSetRoomInOut(Dbresultdata* pdata/*,BinaryReadStream* infostream*/);
	int DoSetRoomMicUpDown(Dbresultdata* pdata/*,BinaryReadStream* infostream*/);
	int DoSetRoomName(Dbresultdata* pdata/*,BinaryReadStream* infostream*/);
	int DoSetChatPublic(Dbresultdata* pdata/*,BinaryReadStream* infostream*/);
	int DoSetRoomWelcome(Dbresultdata* pdata/*,BinaryReadStream* infostream*/);
	int DoSetRoomLogo(Dbresultdata* pdata/*,BinaryReadStream* infostream*/);
	int DoReturnRoomApplyList(Dbresultdata* pdata,BinaryReadStream* infostream,short row,short col,short cmd);
	int DoRoomUserApp(Dbresultdata* pdata,BinaryReadStream* infostream);
	int DoReturnRoomBlackList(Dbresultdata* pdata,BinaryReadStream* infostream,short row,short col,short cmd);
	int DoReturnRoomMemberList(Dbresultdata* pdata,BinaryReadStream* infostream,short row,short col,short cmd);

	int DoSetRoomPwd(Dbresultdata* pdata/*,BinaryReadStream* infostream*/);	//add by jinguanfu 2010/4/7

	int DoExitRoomMember(Dbresultdata* pdata,BinaryReadStream* infostream);	//add by jinguanfu 2010/4/11

	int DoGiveMember(Dbresultdata* pdata,BinaryReadStream* infostream);	//add by jinguanfu 2010/5/13
		
	int DoScore(Dbresultdata* pdata,BinaryReadStream* infostream);		//add by jinguanfu 2010/7/15
	//add by jinguanfu 2010/8/13
	int DoUpdateRoomInfo(Dbresultdata* pdata,short rownum,short colnum,BinaryReadStream* infostream);

	//更新房间内用户权限功能
	int DoUpdateRoomManger(Dbresultdata* pdata,BinaryReadStream* infostream,short rownum,short colnum);

	//add by jinguanfu 2010/9/2
	//赠送烟花
	int DoSendFireworks(Dbresultdata* pdata,BinaryReadStream* infostream);
	//接收烟花
	int DoRecvFireworks(Dbresultdata* pdata,short rownum,short colnum,BinaryReadStream* infostream);
	//add by jinguanfu 2010/9/6
	//幸运礼物抽奖反馈
	int DoRunLotte(Dbresultdata* pdata,BinaryReadStream* infostream);
	
	int DoRunLottery(Dbresultdata* pdata/*,BinaryReadStream* infostream*/);

	//add by jinguanfu 2010/9/18
	int DoLuckNotify(Dbresultdata* pdata,BinaryReadStream* infostream);
	//查看可领取收益add by jinguanfu 2010/11/2
	int DoViweIncome(Dbresultdata * pdata, BinaryReadStream * infostream);
	//查看领取收益记录add by jinguanfu 2010/11/2
	int DoViweIncomeLog(Dbresultdata * pdata, short rownum, short colnum, BinaryReadStream * infostream);
	//领取收益add by jinguanfu 2010/11/2
	int DoGetIncome(Dbresultdata * pdata, BinaryReadStream * infostream);
	//更新歌曲点唱率add by jinguanfu 2010/11/9
	int DoUpdateMusicInfo(Dbresultdata* pdata,BinaryReadStream* infostream);

	int DoUpdateGiftInfo(Dbresultdata* pdata,
							BinaryReadStream* infostream,
							short rownum,
							short colnum);

	int DoUpdateLuckConf(Dbresultdata* pdata,
							BinaryReadStream* infostream,
							short rownum,
							short colnum);

	int DoUpdateRightConf(Dbresultdata* pdata,
						BinaryReadStream* infostream,
						short rownum,
						short colnum);
	//add by jinguanfu 2010/11/24
	//迁移房间后取得房间信息
	int DoGetRoomInfo(Dbresultdata* pdata,
						BinaryReadStream* infostream,
						short rownum,
						short colnum);
	//迁移房间后取得房间会员申请列表
	int DoGetRoomApplyList(Dbresultdata* pdata,
						BinaryReadStream* infostream,
						short rownum,
						short colnum);
	//迁移房间后取得房间黑名单
	int DoGetRoomBlackList(Dbresultdata* pdata,
						BinaryReadStream* infostream,
						short rownum,
						short colnum);
	//迁移房间后取得房间会员列表
	int DoGetRoomMemberList(Dbresultdata* pdata,
						BinaryReadStream* infostream,
						short rownum,
						short colnum);
	//取得房间音视频服务器信息
	int DoGetRoomAVServer(Dbresultdata* pdata,
						BinaryReadStream* infostream,
						short rownum,
						short colnum);
	//后台更改GM
	int DoUpdateGM(Dbresultdata* pdata,
					BinaryReadStream* infostream,
					short rownum,
					short colnum);

	//add by jinguanfu 2011/3/3
	//用户同意会员邀请
	int DoReplyInvite(Dbresultdata* pdata,BinaryReadStream* infostream);

	//add by jinguanfu 2011/4/12
	//取得当日礼物赠送记录
	int	DoGetGiftSend(Dbresultdata* pdata,
					BinaryReadStream* infostream,
					short rownum,
					short colnum);
	
	//取得当日礼物接收记录
	int	DoGetGiftRecv(Dbresultdata* pdata,
					BinaryReadStream* infostream,
					short rownum,
					short colnum);

	//后台操作后，更新房间黑名单
	int DoUpdateRoomBlacklist(Dbresultdata* pdata,
									BinaryReadStream* infostream,
									short rownum,
									short colnum);
	//后台添加删除会员
	int DoUpdateRoomMemberlist(Dbresultdata* pdata,
									BinaryReadStream* infostream,
									short rownum,
									short colnum);

	//add by jinguanfu 2011/8/19
	//禁用MAC地址
	int DoDisableMAC(Dbresultdata* pdata,BinaryReadStream* infostream);
	//禁用IP地址
	int DoDisableIP(Dbresultdata* pdata,BinaryReadStream* infostream);
	//更新公聊黑名单
	int DoUpdateChatBlacklist(Dbresultdata *pdata,BinaryReadStream* infostream,short rownum,short colnum);
	//add by lihongwu 2011/9/20
	int DoUpdateGiftConfig(Dbresultdata *pdata,BinaryReadStream* infostream,short rownum,short colnum);   //更新礼物刷新频率

};

//add by jinguanfu 2010/7/15
class MonitorDataDecoder:public DataDecoder
{
public:
	MonitorDataDecoder(int pttype,size_t hdlen) : DataDecoder(pttype,hdlen){}
	virtual ~MonitorDataDecoder(){}
	virtual int OnPackage(ClientSocketBase *pClient,const char* buf,size_t buflen);
};

//add by jinguanfu 2011/3/10
class FactoryDataDecoder:public DataDecoder
{
public:
	FactoryDataDecoder(int pttype,size_t hdlen) : DataDecoder(pttype,hdlen){}
	virtual ~FactoryDataDecoder(){}
	virtual int OnPackage(ClientSocketBase *pClient,const char* buf,size_t buflen);

	int DoUpdateStarNum(short cmd, int seq,BinaryReadStream *pinstream);  //更新星光数量

};

class roomtoMgr : public ObjectManager<roomtimeout>
{
public:
    roomtoMgr(ObjectAllocator<roomtimeout> * pObjectAllocator) :
      ObjectManager<roomtimeout>(pObjectAllocator)
      {}
      virtual roomtimeout* CreateClient()
      {
          return Create();
      }
      virtual ~roomtoMgr(){}
};

class NetProxy
{
	public:
		NetProxy(char* sIP, int port,size_t maxclient,int pttype,size_t hdlen,int udpport,ExServerCfg* pHallsvrCfg,DbagentCfg* pDbagentCfg,DbagentCfg* pDbagentGiftCfg,DbagentCfg* pDbagentRightCfg,ExServerCfg* pDbMonitorCfg, ExServerCfg* pLogServerCfg ,/*ExServerCfg* pFactoryServerCfg,*/const char* serverkey,int runscan,int numberonescan,int readtimeout,int timeout = 3) :
			m_RoomSocketPool(maxclient),
			m_RoomtoPool(maxclient),
			//m_GiftTimeoutPool(maxclient),
			//add by jinguanfu 2010/1/19 <begin>
			m_DBResultPool(maxclient/5),
			//add by jinguanfu 2010/1/19 <end>
			m_FlowerResultPool(maxclient/10),
			m_SendGiftPool(maxclient),
			m_LobbyTimeoutPool(maxclient),
			//add by jinguanfu 2010/3/30 <begin>
			m_MicInfoPool(maxclient),
			//add by jinguanfu 2010/3/30 <end>
			m_ClientDataDecoder(pttype,hdlen),
			m_HallDataDecoder(pttype,hdlen),
			m_DbagentDataDecoder(pttype,hdlen),
			m_MonitorDataDecoder(pttype,hdlen),
			m_FactoryDataDecoder(pttype,hdlen),
			m_pDbagentCfg(pDbagentCfg),
			m_pDbagentGiftCfg(pDbagentGiftCfg),
			m_pDbagentRightCfg(pDbagentRightCfg),
			m_pHallsvrCfg(pHallsvrCfg),
			m_pDbMonitorCfg(pDbMonitorCfg),
			m_pLogSvrCfg(pLogServerCfg),
			//m_pFactoryServer(pFactoryServerCfg),
			m_listener(port,&m_Reactor,&m_RoomSocketPool,&m_counter,&m_ClientDataDecoder,&m_MapForClient,timeout),
			m_roomtomgr(&m_RoomtoPool),
			m_udplistener(&m_Reactor,udpport),
			m_counter(200,1000000){
				strncpy(m_sIP, sIP, sizeof(m_sIP));
				strncpy(m_ServerKey, serverkey, sizeof(m_ServerKey));
//				m_roomlistinfo.hallid = pHallsvrCfg->m_id;
				m_port = port;
				m_version = 0;
				gettimeofday(&m_time,NULL);//可以改成每10次取一下
				m_scantime=0;
				m_Runscan=runscan;
				m_Numberonescan=numberonescan;
				m_timeout=timeout;
				m_luckbase=10000;
				randseed=1;
				m_Readtimeout=readtimeout;

				for(int i=0;i<DBAGENT_COUNT;i++)
				{
					//m_HallMain[i]=new HallSvrClient(&m_Reactor,m_pHallsvrCfg->m_ip.c_str(),m_pHallsvrCfg->m_port);
					m_DbagentMain[i]= new BackClientSocketBase(&m_Reactor,m_pDbagentCfg->m_ipmain.c_str(),m_pDbagentCfg->m_portmain);
					m_DbagentBak[i] = new BackClientSocketBase(&m_Reactor,m_pDbagentCfg->m_ipbak.c_str(),m_pDbagentCfg->m_portbak);

					m_DbagentGiftMain[i] = new BackClientSocketBase(&m_Reactor,m_pDbagentGiftCfg->m_ipmain.c_str(),m_pDbagentGiftCfg->m_portmain);
					m_DbagentGiftBak[i] = new BackClientSocketBase(&m_Reactor,m_pDbagentGiftCfg->m_ipbak.c_str(),m_pDbagentGiftCfg->m_portbak);
				}
				m_DbagentRight=new BackClientSocketBase(&m_Reactor,m_pDbagentRightCfg->m_ipmain.c_str(),m_pDbagentRightCfg->m_portmain);

				DBAgentPos=0;
				

				//m_pMusicInfoUpdate=new MusicInfoUpdate();

				m_pRightCfg=NULL;

				m_LogServer=new BackClientSocketBase(&m_Reactor,m_pLogSvrCfg->m_ip.c_str(),m_pLogSvrCfg->m_port);
				
				//m_FactoryServer=new BackClientSocketBase(&m_Reactor,m_pFactoryServer->m_ip.c_str(),m_pFactoryServer->m_port);
  
				

		}
		virtual ~NetProxy()
			{ 
			//if(m_pRightCfg!=NULL)
				//delete m_pRightCfg;
			}
		virtual int Start();
		virtual int Stop();
		void GetInfo(char* str)
		{
			m_Reactor.GetInfo(str);
			char sz[50] = {0};
			sprintf(sz,"client count = %d\n",(int)m_MapForClient.Size());
			strcat(str,sz);
		}

		int GetRoomInfomation();
		int GetRoomNumByIpPort(int fd);
		int GetRoomDetailPrePage(int fd, int pagenum);
		int GetRoomsblacklist(int fd);
		int GetblacklistByRoomid(int fd, ROOM_INFO* proominfo);
		int GetRoomManagerlist(int fd);
		int GetManagerlistByRoomid(int fd, ROOM_INFO* proominfo);
		int GetGMlist(int fd);
		int DeleteRoomByID(ROOM_INFO* roominfo);
		int GetDBInfo(int fd, BinaryWriteStream* outsteam, char* infobuf, size_t insize, size_t& outsize);


		BackClientSocketBase* GetDBAgent();
		BackClientSocketBase* GetDBAgentGift();	//add by jinguanfu 2010/11/3
		BackClientSocketBase* GetDBAgentRight();	//add by jinguanfu 2010/11/26
		BackClientSocketBase* GetLogServer();	//add by jinguanfu 2010/12/29
		//BackClientSocketBase* GetFactoryServer();	//add by jinguanfu 2011/3/10
		BackClientSocketBase* GetDBMonitor(){return mDbMonitor;}
		HallSvrClient* GetHallSvr();

		inline Counter* GetCounter(){return &m_counter;}
		RoomClient* GetClient(int seq);

		EPReactor* GetReactor(){return &m_Reactor;}
		//modify by jinguanfu 2010/1/19
		//DB_RESULT_DATA* GetClientDBMap(int seq);
		Dbresultdata* GetClientDBMap(int seq);
		LobbyTimeout* GetLobbyTMMap(int seq);
		void ClearClientDBMap(int seq);
		void ClearClientDBMap(ClientSocketBase* pClient);

		ROOM_INFO* GetRoomInfo(ClientSocketBase *pClient); 
		ROOM_INFO* GetRoomInfo(int roomid);
		int SendToSrv(ClientSocketBase *pClient, BinaryWriteStream& outstream, int btea = 1, int bdirect = 0);
		int BroadcastSBCmd(ROOM_INFO* proominfo, BinaryWriteStream& outstream, int bdirect = 0);
		roomtoMgr* GettoMgr(){return &m_roomtomgr;}

		int ReportMyStatus(HALL_SERVER_CMD _st);
		//int PushBackFlower(FLOWER_RESULT_DATA fl);
		int PushBackFlower(Flowerresultdata* fl);
		int FlowerStatusScan(struct timeval _time);
		int ResultDataScan(struct timeval _time);
		int MonitorScan(struct timeval _time);

		//add by jinguanfu 2010/1/21
		Dbresultdata* CreateDBResultdata()
		{
			return (Dbresultdata*)m_DBResultPool.Create();
		};

		Flowerresultdata* CreateFlowerResult()
		{
			return (Flowerresultdata*)m_FlowerResultPool.Create();
		};
		//add by jinguanfu 2010/11/26
		SEND_GIFT_INFO* CreateSendGiftInfo()
		{
			return (SEND_GIFT_INFO*)m_SendGiftPool.Create();
		};

		//add by jinguanfu 2010/8/2
		roomtimeout* CreateRoomTimeout()
		{
			return (roomtimeout*)m_RoomtoPool.Create();
		};
		/*
		GiftTimeout* CreateGiftTimeout()
		{
			return (GiftTimeout*)m_GiftTimeoutPool.Create();
		};
		*/
		LobbyTimeout* CreateLobbyTimeout()
		{
			return (LobbyTimeout*)m_LobbyTimeoutPool.Create();
		};
		//void ClearFlowerResult(int seq);

		void ClearLobbyTMMap(int outseq);
		//add by jinguanfu 2010/4/7  <begin>
		//从缓存池中取得micinfo对象
		MIC_INFO* CreateMicInfo()
		{
			return (MIC_INFO*)m_MicInfoPool.Create();
		};
		
		//释放micinfo对象到缓存池
		void DestroyMicInfo(MIC_INFO* pMicInfo)
		{
			pMicInfo->init();	//清空信息
			return m_MicInfoPool.Destroy(pMicInfo);
		};

		//add by jinguanfu 2010/4/7  <end>
		//add by jinguanfu 2010/8/2 <begin>
		//释放DBResult对象到缓冲池
		void DestroyDBResult(Dbresultdata* pDbresult)
		{
			//pDbresult->UnRegisterTimer();
			pDbresult->init();
			return m_DBResultPool.Destroy(pDbresult);
		};	
		
		//释放FlowerResult对象到缓冲池
		void DestroyFlowerResult(Flowerresultdata* pData)
		{
			pData->UnRegisterTimer();
			pData->init();
			return m_FlowerResultPool.Destroy(pData);
		};
		//add by jinguanfu 2010/11/26
		void DestroySendGift(SEND_GIFT_INFO* pData)
		{
			pData->init();
			return m_SendGiftPool.Destroy(pData);
		};
		//释放roomtimeout对象到缓冲池
		void DestroyRoomtimeout(roomtimeout* pRoomtimeout)
		{
			pRoomtimeout->UnRegisterTimer();
			pRoomtimeout->init();
			return m_RoomtoPool.Destroy(pRoomtimeout);
		};
		/*
		void DestroyGifttimeout(GiftTimeout* pGifttimeout)
		{

			pGifttimeout->UnRegisterTimer();
			pGifttimeout->init();
			return m_GiftTimeoutPool.Destroy(pGifttimeout);
		};
		*/
		//add by jinguanfu 2010/8/2 <end>
		void DestroyLobbyTimeout(LobbyTimeout* pLobbyTimeout)
		{
			//pLobbyTimeout->UnRegisterTimer();
			pLobbyTimeout->init();
			return m_LobbyTimeoutPool.Destroy(pLobbyTimeout);
		};
		//add by jinguanfu 2010/4/19
		int GetApplylistByRoomid(int fd,ROOM_INFO* roominfo);
		//add by jinguanfu 2010/5/12
		int GetGiftInfo(int fd);

		MonitorSvrClient* GetMonitorSvr();

		int GetClientCount();

		void ReportStatusToMonitor();

		//add by jinguanfu 2010/7/19
		int LoadRightconfig(char* rightpath);		//加载权限配置文件
		//add by jinguanfu 2010/9/9
		int GetLuckConf(int fd);		//读取中奖配置信息
		//add by jinguanfu 2010/9/19
		int GetRandSeed()			//随机数生成种子
		{
			if(randseed>=m_luckbase)
				randseed=1;
			return randseed++;
		}
		
		int GetRightConf();		//从数据库取得房间权限信息

		//add by jinguanfu 2010/11/27
		//创建虚拟用户
		RoomClient* CreateClient()
		{
			 return (RoomClient*)m_RoomSocketPool.Create();
			
		};
		//释放虚拟用户
		void DestroyClient(RoomClient* pClient)
		{
			pClient->initAll();
			m_RoomSocketPool.Destroy(pClient);
		};

		//add by jinguanfu 2011/8/19
		//取得公聊黑名单列表
		int GetChatBlacklist(int fd);
		//add by lihongwu 2011/9/20
		//取得礼物刷新频率
		int GetGiftConfig(int fd);

	private:
		ObjectPoolAllocator<RoomClient> m_RoomSocketPool;
		ObjectPoolAllocator<roomtimeout> m_RoomtoPool;
		//ObjectPoolAllocator<GiftTimeout> m_GiftTimeoutPool;
		//add by jinguanfu 2010/1/19 <begin>
		ObjectPoolAllocator<Dbresultdata> m_DBResultPool;
		ObjectPoolAllocator<Flowerresultdata> m_FlowerResultPool;
		//add by jinguanfu 2010/1/19 <end>
		//add by jinguanfu 2010/11/26 <begin>
		ObjectPoolAllocator<SEND_GIFT_INFO> m_SendGiftPool;
		//add by jinguanfu 2010/11/26 <end>
		//add by jinguanfu 2010/11/3 <begin>
		ObjectPoolAllocator<LobbyTimeout> m_LobbyTimeoutPool;
		//add by jinguanfu 2010/11/3 <end>
		//add by jinguanfu 2010/3/30 <begin>
		ObjectPoolAllocator<MIC_INFO> m_MicInfoPool;	//排麦信息内存池
		//add by jinguanfu 2010/3/30 <end>
		ClientDataDecoder m_ClientDataDecoder;
		HallDataDecoder m_HallDataDecoder;
		DbagentDataDecoder m_DbagentDataDecoder;
		MonitorDataDecoder m_MonitorDataDecoder;
		FactoryDataDecoder m_FactoryDataDecoder;

		DbagentCfg* m_pDbagentCfg;
		DbagentCfg* m_pDbagentGiftCfg;
		DbagentCfg* m_pDbagentRightCfg;


		int DBAgentPos;			//取数据库连接时的当前位置
		int DBAgentGiftPos;		//取送礼DB连接时的当前位置

		
		
	public:	
		ExServerCfg* m_pHallsvrCfg;
		ExServerCfg* m_pDbMonitorCfg;
		//add by jinguanfu 2010/12/29
		ExServerCfg* m_pLogSvrCfg;	//日志服务器信息
		BackClientSocketBase* mDbMonitor;
		//add by jinguanfu 2010/7/20
		RightConfig* m_pRightCfg;			//房间权限配置
		RightConfigEx* m_pRightCfgEx;
		//add by jinguanfu 2011/3/10
		//ExServerCfg* m_pFactoryServer;
		
	public:	
    		int m_port;		
		int m_Runscan;
		int m_Numberonescan;
		int m_Readtimeout;
		
	private:	
		pair<int, HallSvrClient*>m_HallPair;
		RoomListen m_listener;
		roomtoMgr m_roomtomgr;
		UdpListenSocket m_udplistener;
		EPReactor m_Reactor;
		Counter m_counter;
		ClientMap m_MapForClient;
		int m_timeout;
		MonitorSvrClient* m_MonitorSrv;
		//add by jinguanfu 2010/9/27
		HallSvrClient* m_HallMain[DBAGENT_COUNT];		//大厅连接数
		BackClientSocketBase* m_DbagentMain[DBAGENT_COUNT];	 //数据库主连接组
		BackClientSocketBase* m_DbagentBak[DBAGENT_COUNT];	//数据库备用连接组
		//add by jinguanfu 2010/11/3
		BackClientSocketBase* m_DbagentGiftMain[DBAGENT_COUNT];	//送礼专用DB 主连接
		BackClientSocketBase* m_DbagentGiftBak[DBAGENT_COUNT];	//送礼专用DB 备用连接
		//add by jinguanfu 2010/11/26
		BackClientSocketBase* m_DbagentRight;		//读取权限专用DB 主连接
		//add by jinguanfu 2010/12/29
		BackClientSocketBase* m_LogServer;			//日志服务器连接
		//add by jinguanfu 2011/3/10
		//BackClientSocketBase* m_FactoryServer;		//工厂服务器连接(星光道具生成)
		
	public:

		int m_scantime; 
		char m_sIP[64];
		char m_ServerKey[16];
		ROOM_LIST_INFO m_roomlistinfo; 
		//map<int, DB_RESULT_DATA>m_DBMap;
		map<int,Dbresultdata*> m_DBMap;	//modify by jinguanfu 2010/1/18
		map<int,LobbyTimeout*> m_LobbyTMMap; //add by jinguanfu 2010/11/3
		float m_version;
		//vector<FLOWER_RESULT_DATA> m_flower;
		vector<Flowerresultdata*> m_flower;	//modify by jinguanfu 2010/1/18
		struct timeval m_time;
		map<int,GM_INFO> m_GM;//记录房间管理员列表
		//map<int,int> m_GiftPrice;//礼物价格<礼物ID，金币价格>
		map<int,GIFT_INFO> m_GiftInfo; //礼物信息 add by jinguanfu 2010/9/6
		//add by jinguanfu 2010/9/9
		int m_luckbase;				//中奖概率基数，默认10000
		map<int,int> m_LuckConf;		//中奖配置<概率,中奖倍数>
		map<int,int> m_Music;			//当日音乐点歌率
		//MusicInfoUpdate* m_pMusicInfoUpdate;	//点歌率更新任务 
		//add by jinguanfu 2011/8/19
		map<int,int> m_ChatBlacklist;  //禁止公聊黑名单
		//add by lihongwu 2011/10/10
		map<int,GIFT_CONFIG> m_giftconflist;        //礼物配置信息
		
	private:
		int randseed;				//随机数种子(幸运抽奖使用)
		
};

class MicInfoManager
{
	public:
		static MIC_INFO micinfo;
		static void Clean();
};

#endif



