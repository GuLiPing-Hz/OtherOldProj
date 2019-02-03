#pragma once

#include "network/eventhandler.h"
#include "network/reactor.h"
#include "ac/log/log.h"
#include "ac/util/objectmanager.h"
#include "ac/util/destroyable.h"
#include "roomtype.h"

class  ClientDataDecoder;
using namespace ac;


class roomtimeout : public TMEventHandler, public Destroyable
{
public: 
    roomtimeout() : m_roomid(0), m_idx(0), m_time(30)/*30s*/, m_type(0)/*黑名单*/,
        m_haspk(0), m_pClientDataDecoder(NULL) {}
	~roomtimeout(){}

	void init()
	{
		m_roomid=0;
		m_idx=0;
		m_time=30;
		m_type=0;
		m_haspk=0;
		m_pClientDataDecoder=NULL;
	};
	virtual void OnTimeOut();

public:
	int m_roomid;
	int m_idx;
	int m_time;
	char m_type;
	char m_haspk;
	ClientDataDecoder* m_pClientDataDecoder;
};

//add by jinguanfu 2010/1/15 异常超时<begin>
class Dbresultdata :public TMEventHandler, public Destroyable
{

public:
	Dbresultdata() : roomid(0), opidx(0), bopidx(0), cmd(0),badd(0),
		identity(0), cate_idx(0),number(0),seq(0),outseq(0)
	{
		memset(content,0,sizeof(content));
	}
	~Dbresultdata(){}
	
	void init()
	{
		roomid=0;
		opidx=0;
		bopidx=0;
		cmd=0;
		badd=0;
		identity=0;
		cate_idx=0;
		number=0;
		seq=0;
		outseq=0;
		memset(content,0,sizeof(content));
	};
	virtual void OnTimeOut();
public:
	int roomid;
	int opidx;
	int bopidx;
	short cmd;
	char badd;
	char identity;
	int cate_idx;
	int number;	
	int seq;	
	int outseq;			//索引标识
	char content[500];
	

};

class Flowerresultdata : public TMEventHandler, public Destroyable
{

public:
	Flowerresultdata():roomid(0)/*,s_idx(0),r_idx(0),
		cate_idx(0),number(0),havesend(0),seq(0)*/
	{
	
	}
	~Flowerresultdata(){}
	
	void init()
	{
		roomid=0;
		curpos=0;
		/*
		s_idx=0;
		r_idx=30;
		cate_idx=0;
		number=0;
		havesend=0;
		seq=0;
		*/
	};
		
	virtual void OnTimeOut();
public:
	int roomid;		//房间ID
	int curpos;		//任务执行位置
	/*
	int s_idx;		//送礼者idx
	int r_idx;		//接收礼物者idx
	int cate_idx;		//礼物id
	int number;		//礼物总数
	int havesend;	//礼物已发数量
	int seq;			//
	*/
};

/*
//add by jinguanfu 2010/9/8 礼物失效(印章)
class GiftTimeout : public TMEventHandler, public Destroyable
{
public:
	GiftTimeout():roomid(0),idx(0),ClientID(0),
		GiftID(0)
	{
	
	}
	~GiftTimeout(){}
	
	void init()
	{
		roomid=0;
		idx=0;
		ClientID=0;
		GiftID=0;
	};
	
	virtual void OnTimeOut();
	
public:
	int roomid;	//房间ID
	int idx;		//用户idx
	int ClientID;	//用户连接ID
	int GiftID;	//礼物ID
	
};
*/
class LobbyTimeout :public TMEventHandler, public Destroyable
{

public:
	LobbyTimeout() : roomid(0), idx(0),clientID(0),cmd(0),seq(0),outseq(0)
	{
		memset(sessionkey,0,sizeof(sessionkey));
	}
	~LobbyTimeout(){}
	
	void init()
	{
		roomid=0;
		idx=0;
		clientID=0;
		cmd=0;
		seq=0;
		outseq=0;
		Islogin=0;
		memset(sessionkey,0,sizeof(sessionkey));
		memset(enterpwd,0,sizeof(enterpwd));
	};
	virtual void OnTimeOut();
public:
	int roomid;
	int idx;
	int clientID;
	short cmd;
	int seq;	
	int outseq;			//索引标识
	char sessionkey[17];

	//add by jinguanfu 2011/1/18
	char enterpwd[64];	//进入房间密码

	char Islogin;		//是否已登陆其他房间
	

};

//add by jinguanfu 2010/11/8
//定时更新歌曲点歌率
class MusicInfoUpdate:public TMEventHandler, public Destroyable
{
public:
	MusicInfoUpdate()
		{}
	~MusicInfoUpdate()
		{}

	virtual void OnTimeOut();
	
};

