#ifndef ROOMCLIENT_H_
#define ROOMCLIENT_H_

#include <string.h>
#include "network/clientsocket.h"
#include "ac/util/protocolstream.h"
#include "ac/log/log.h"
#include "ac/util/crypt.h"

using namespace ac;

class RoomClient : public ClientSocket
{
public:
	RoomClient()
	{
		initAll();
	}
	virtual ~ RoomClient()
	{
	}

	virtual void Close();

    //void ErrorClose();

	void LeaveRoom();

	void initAll()
	{

		m_btoken = 0;
		memset(m_sessionkey, 0, sizeof(m_sessionkey));

		m_roomid = 0;
		m_identity = 0;

    	m_idx = 0; 
  		m_sex = 0;

		m_onmicflag = 0;
		m_starnum = 0;

		m_status = 0;	
		m_onlinetime = 0;
		m_logintime = 0;	

	}

	int SendEncryptBuf(BinaryWriteStream &stream, int xtea = 1, int bdirect = 0);
	int SendCompressBuf(BinaryWriteStream &stream, int bdirect = 0);
	int DirectSend(const char* buf,size_t buflen);

	//add by jinguanfu 2010/11/27
	int VLeaveRoom();	//虚拟用户退出房间

private:
	void clearRoomStat();

	void errorClear(int idx);


public:
	int m_roomid;
	int m_idx;
	char m_identity;
	char m_sessionkey[17];
	char m_btoken;
	char m_bforbiden;
	char m_viptype;	//VIP类型(等同m_identity的定义)
	int gold;			//金币数
	int silver;		//银币数
	char m_level;		//经验等级
	short m_viplevel;		//VIP值，用来区分VIP及皇冠等级
	char m_vipflag;		//VIP及皇冠是否有效
	char m_vipdate[32];	//VIP有效日期
	char m_sex;		//性别0:女1:男
	//void* gifttimeout;	//礼物有效期计时
	char m_onmicflag;	//0:未上麦 1:麦序中 2:表演麦 3:主持麦
	short m_starnum;		//用户星光数
	//add by jinguanfu 2011/8/10
	short m_status;		//用户防沉迷状态
	int m_onlinetime;		//用户当天在线时长
	int m_logintime;		//用户本次登陆时间

	
};

#endif // ROOMCLIENT_H_



