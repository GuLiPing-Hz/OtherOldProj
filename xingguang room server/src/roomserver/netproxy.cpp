#include "netproxy.h"
#include "ac/log/log.h"
#include "ac/util/md5.h"
#include "ac/util/protocolstream.h"
#include "ac/util/crypt.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "StreamFactory.h"

void UdpListenSocket::OnFDRead()
{
	int n;
	socklen_t len;
	struct sockaddr_in pcliaddr;
	char buf[1024];

	memset(buf,0,sizeof(buf));
	len = sizeof(pcliaddr);

	n = recvfrom(m_fd, buf, sizeof(buf), 0, (struct sockaddr *)&pcliaddr, &len);
	AC_INFO("recv n=%d,message=%s",n,buf);

	//sendto(m_fd,buf,n,0,(struct sockaddr *)&pcliaddr,len);
	if(strcmp(buf,"info\n") == 0)
	{
		char str[512] = {0};
		g_pNetProxy->GetInfo(str);
		sendto(m_fd,str,strlen(str),0,(struct sockaddr *)&pcliaddr,len);
	}
}
		
int NetProxy::Start()
{

	if(GetRightConf()!=0)
	{
		AC_ERROR("GetRightConf info error");
		return -1;
	}
	if(GetRoomInfomation() != 0)
	{
		AC_ERROR("getroom info error");
		return -1;
	}

	if(m_Reactor.Init() != 0)
	{
		AC_ERROR("reactor init error");
		return -1;
	}
	
	HallSvrClient *pHallSvr = new HallSvrClient(&m_Reactor,m_pHallsvrCfg->m_ip.c_str(),m_pHallsvrCfg->m_port);
	pHallSvr->SetDecoder(&m_HallDataDecoder);
	pHallSvr->SetTimeout(2);
	pHallSvr->Connect();
	m_HallPair = make_pair(m_pHallsvrCfg->m_id, pHallSvr);
	/* del by jinguanfu 2010/9/27
	BackClientSocketBase *pDbagent = new BackClientSocketBase(&m_Reactor,m_pDbagentCfg->m_ipmain.c_str(),m_pDbagentCfg->m_portmain);
	pDbagent->SetDecoder(&m_DbagentDataDecoder);
	pDbagent->SetTimeout(2); // 2秒读写超时，2秒重连
	pDbagent->Connect();

	BackClientSocketBase *pDbagentBak = new BackClientSocketBase(&m_Reactor,m_pDbagentCfg->m_ipbak.c_str(),m_pDbagentCfg->m_portbak);
	pDbagentBak->SetDecoder(&m_DbagentDataDecoder);
	pDbagentBak->SetTimeout(2);
	pDbagentBak->Connect();
	m_DbagentPair = make_pair(pDbagent,pDbagentBak);
	*/
	//add by jinguanfu 2010/9/27
	for(int i=0;i<DBAGENT_COUNT;i++)
	{
	/*
		m_HallMain[i]->SetDecoder(&m_HallDataDecoder);
		m_HallMain[i]->SetTimeout(2);
		m_HallMain[i]->Connect();
	*/
		m_DbagentMain[i]->SetDecoder(&m_DbagentDataDecoder);
		m_DbagentMain[i]->SetTimeout(2);
		m_DbagentMain[i]->Connect();

		m_DbagentBak[i]->SetDecoder(&m_DbagentDataDecoder);
		m_DbagentBak[i]->SetTimeout(2);
		m_DbagentBak[i]->Connect();

		m_DbagentGiftMain[i]->SetDecoder(&m_DbagentDataDecoder);
		m_DbagentGiftMain[i]->SetTimeout(2);
		m_DbagentGiftMain[i]->Connect();

		m_DbagentGiftBak[i]->SetDecoder(&m_DbagentDataDecoder);
		m_DbagentGiftBak[i]->SetTimeout(2);
		m_DbagentGiftBak[i]->Connect();

	}

	m_DbagentRight->SetDecoder(&m_DbagentDataDecoder);
	m_DbagentRight->SetTimeout(2);
	m_DbagentRight->Connect();
/*
	BackClientSocketBase *pDbMonitor = new BackClientSocketBase(&m_Reactor,m_pDbMonitorCfg->m_ip.c_str(),m_pDbMonitorCfg->m_port);
	pDbMonitor->SetDecoder(&m_DbagentDataDecoder);
	pDbMonitor->SetTimeout(2); // 2秒读写超时，2秒重连
	pDbMonitor->Connect();
	mDbMonitor=pDbMonitor;

	MonitorSvrClient* pMonitor = new MonitorSvrClient(&m_Reactor,m_pDbMonitorCfg->m_ip.c_str(),m_pDbMonitorCfg->m_port);
	pMonitor->SetDecoder(&m_MonitorDataDecoder);
	pMonitor->SetTimeout(2);
	pMonitor->Connect();
	//add by jinguanfu 2010/12/28 
	//pMonitor->RegisterRead(600); //读取超时10分钟后断开连接
	m_MonitorSrv=pMonitor;
*/	

/*
	//add by jinguanfu 2010/11/9
	//点唱率更新任务50 分钟运行一次
	m_pMusicInfoUpdate->SetReactor(g_pNetProxy->GetReactor());
	m_pMusicInfoUpdate->RegisterTimer(10*60);		//10分钟检查一次
*/
	//add by jinguanfu 2010/12/29
	m_LogServer->SetTimeout(2);
	m_LogServer->Connect();

	//add by jinguanfu 2011/3/10
	/*
	m_FactoryServer->SetDecoder(&m_FactoryDataDecoder);
	m_FactoryServer->SetTimeout(2);
	m_FactoryServer->Connect();
	*/
	
	if(m_listener.Listen() != 0)
	{
		AC_ERROR("listen error");
		return -1;
	}
	if(m_udplistener.Listen() != 0)
	{
		AC_ERROR("udp listen error");
		return -1;
	}
	//TimerTest *pTimerTest = new TimerTest(&m_Reactor);
	//struct timeval timeout = {3,0};
	//pTimerTest->Register(&timeout);

	m_Reactor.Run();
	    /*{
	    AC_ERROR("reactor run error");
	    return -1;
	    }*/
	return 0;
}

int NetProxy::Stop()
{
	m_listener.Close();
	m_udplistener.Close();

	HallSvrClient* pHallSvr = m_HallPair.second;
	if(pHallSvr)
	{
		if(pHallSvr->IsConnect())
			pHallSvr->RealClose();
		//delete pHallSvr;
	}
	/* del by jinguanfu 2010/9/27
	BackClientSocketBase* pDbagentMain = m_DbagentPair.first;
	if(pDbagentMain && pDbagentMain->IsConnect())
	{
		pDbagentMain->RealClose();
	}
	BackClientSocketBase* pDbagentBak = m_DbagentPair.second;
	if(pDbagentBak && pDbagentBak->IsConnect())
	{
		pDbagentBak->RealClose();
	}
	*/
	//add by jinguanfu 2010/9/27
	for(int i =0;i<DBAGENT_COUNT;i++)
	{
		//m_HallMain[i]->RealClose();
		//delete m_HallMain[i];
		if(m_DbagentMain[i])
		{
			if(m_DbagentMain[i]->IsConnect())
				m_DbagentMain[i]->RealClose();
			//delete m_DbagentMain[i];
		}
		if(m_DbagentBak[i])
		{
			if(m_DbagentBak[i]->IsConnect())
				m_DbagentBak[i]->RealClose();
			//delete m_DbagentBak[i];
		}
		if(m_DbagentGiftMain[i])
		{
			if(m_DbagentGiftMain[i]->IsConnect())
				m_DbagentGiftMain[i]->RealClose();
			//delete m_DbagentGiftMain[i];
		}
		if(m_DbagentGiftBak[i])
		{
			if(m_DbagentGiftBak[i]->IsConnect())
				m_DbagentGiftBak[i]->RealClose();
			//delete m_DbagentGiftBak[i];
		}
	}
	
	m_Reactor.Stop();
	return 0;
}

RoomClient* NetProxy::GetClient(int seq)
{
	if(m_MapForClient.Get(seq) == NULL)
		return NULL;
	return (RoomClient*)(*(m_MapForClient.Get(seq)));
}

BackClientSocketBase* NetProxy::GetDBAgent()
{
	/* del by jinguanfu 
	BackClientSocketBase* pDbagentMain = m_DbagentPair.first;
	if(pDbagentMain->IsConnect())
		return pDbagentMain;
	BackClientSocketBase* pDbagentBak = m_DbagentPair.second;
	if(pDbagentBak->IsConnect())
		return pDbagentBak;
	*/
	//add by jinguanfu 2010/9/27
	BackClientSocketBase* pDBAgent=NULL;
	
	if(DBAgentPos>=DBAGENT_COUNT)
		DBAgentPos=0;

	while(DBAgentPos<DBAGENT_COUNT)
	{
		if(m_DbagentMain[DBAgentPos]->IsConnect())
		{
			pDBAgent= m_DbagentMain[DBAgentPos];
			break;
		}
		if(m_DbagentBak[DBAgentPos]->IsConnect())
		{
			pDBAgent= m_DbagentBak[DBAgentPos];
			break;
		}
		DBAgentPos++;
	}
	DBAgentPos++;
	
	return pDBAgent;
	
}

BackClientSocketBase* NetProxy::GetDBAgentGift()
{

	BackClientSocketBase* pDBAgent=NULL;
	
	if(DBAgentGiftPos>=DBAGENT_COUNT)
		DBAgentGiftPos=0;

	while(DBAgentGiftPos<DBAGENT_COUNT)
	{
		if(m_DbagentGiftMain[DBAgentGiftPos]->IsConnect())
		{
			pDBAgent= m_DbagentGiftMain[DBAgentGiftPos];
			break;
		}
		if(m_DbagentGiftBak[DBAgentGiftPos]->IsConnect())
		{
			pDBAgent= m_DbagentGiftBak[DBAgentGiftPos];
			break;
		}
		DBAgentGiftPos++;
	}
	DBAgentGiftPos++;
	
	return pDBAgent;
	
}
//add by jinguanfu 2010/11/26
BackClientSocketBase* NetProxy::GetDBAgentRight()
{
	BackClientSocketBase* pDBAgent=NULL;
	if(m_DbagentRight->IsConnect())
		pDBAgent=m_DbagentRight;

	return pDBAgent;
}

//add by jinguanfu 2010/12/29
BackClientSocketBase* NetProxy::GetLogServer()
{
	BackClientSocketBase* pLogSvr=NULL;
	if(m_LogServer->IsConnect())
		pLogSvr=m_LogServer;

	return pLogSvr;
}

/*
//add by jinguanfu 2011/3/10
BackClientSocketBase* NetProxy::GetFactoryServer()
{
	BackClientSocketBase* pFactorySvr=NULL;
	if(m_FactoryServer->IsConnect())
		pFactorySvr=m_FactoryServer;

	return pFactorySvr;
}
*/

HallSvrClient* NetProxy::GetHallSvr()
{
	return m_HallPair.second;
	//add by jinguanfu 2010/9/27
	srand(time(NULL));
	int index = rand() % DBAGENT_COUNT;
	if(m_HallMain[index]->IsConnect())
		return m_HallMain[index];
	return NULL;
}

int NetProxy::GetRoomInfomation()
{
	int fd;
	if((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		AC_ERROR("NetProxy::GetRoomInfomation():create socket error");
		return -1;
	}

	sockaddr_in addr;
	memset(&addr, 0, sizeof(sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(m_pDbagentCfg->m_ipmain.c_str());
	addr.sin_port = htons(m_pDbagentCfg->m_portmain);

	if(connect(fd, (sockaddr*)(&addr), sizeof(sockaddr_in)) == -1)
	{
		AC_ERROR("NetProxy::GetRoomInfomation():connect maindbagent error");

		sockaddr_in addrbak;
		memset(&addrbak, 0, sizeof(sockaddr_in));
		addrbak.sin_family = AF_INET;
		addrbak.sin_addr.s_addr = inet_addr(m_pDbagentCfg->m_ipbak.c_str());
		addrbak.sin_port = htons(m_pDbagentCfg->m_portbak);

		if(connect(fd, (sockaddr*)(&addrbak), sizeof(sockaddr_in)) == -1)
		{
			AC_ERROR("NetProxy::GetRoomInfomation():connect bakdbagent error");
			close(fd);
			return -1;
		}
		
	}

	int roomnum = 0;
	if((roomnum = GetRoomNumByIpPort(fd)) == -1)
	{
		AC_ERROR("NetProxy::GetRoomInfomation():GetRoomNumByIpPort error");
		close(fd);
		return -1;
	}

	if(roomnum == 0)
	{
		/*
		AC_ERROR("NetProxy::GetRoomInfomation():roomnum error");
		close(fd);
		return -1;
		*/
		AC_DEBUG("NetProxy::GetRoomInfomation():roomnum =%d",roomnum);
	}

	m_roomlistinfo.roomnum = roomnum;
	int pagesize = roomnum / ROOMNUMPERPAGE;
	pagesize++;


	for(int i = 0; i< pagesize; i++)
	{
		if(GetRoomDetailPrePage(fd, i) == -1)
		{
			AC_ERROR("NetProxy::GetRoomInfomation():GetRoomDetailPrePage error");
			close(fd);
			return -1;
		}
	}

	AC_DEBUG("NetProxy::GetRoomInfomation():GetRoomDetailPrePage success");
	if(GetRoomsblacklist(fd) == -1)
	{
		AC_ERROR("NetProxy::GetRoomInfomation():GetRoomsblacklist error");
		close(fd);
		return -1;
	}
	AC_DEBUG("NetProxy::GetRoomInfomation():GetRoomsblacklist success");

	if(GetRoomManagerlist(fd) == -1)
	{
		AC_ERROR("NetProxy::GetRoomInfomation():GetRoomsManagerlist error");
		close(fd);
		return -1;
	}
	AC_DEBUG("NetProxy::GetRoomInfomation():GetRoomsManagerlist success");

	//add by jinguanfu 2010/5/12
	if(GetGiftInfo(fd)==-1)
	{
		AC_ERROR("NetProxy::GetRoomInfomation():GetGiftInfo error");
		close(fd);
		return -1;
	}
	AC_DEBUG("NetProxy::GetRoomInfomation():GetGiftInfo success");

	//add by jinguanfu 2010/9/9
	if(GetLuckConf(fd)== -1)
	{
		AC_ERROR("NetProxy::GetRoomInfomation():GetLuckConf error");
		close(fd);
		return -1;
	}
	AC_DEBUG("NetProxy::GetRoomInfomation():GetLuckConf success");	

	//add by jinguanfu 2011/8/19
	if(GetChatBlacklist(fd)==-1)
	{
		AC_ERROR("NetProxy::GetRoomInfomation():GetChatBlacklist error");
		close(fd);
		return -1;
	}
	AC_DEBUG("NetProxy::GetRoomInfomation():GetChatBlacklist success");
	
	//add by lihongwu 2011/10/11
	if(GetGiftConfig(fd)==-1)
	{
		AC_ERROR("NetProxy::GetRoomInfomation():GetGiftConfig error");
		close(fd);
		return -1;
	}
	AC_DEBUG("NetProxy::GetRoomInfomation():GetGiftConfig success");

	close(fd);

	return 0;
}

int NetProxy::GetDBInfo(int fd, BinaryWriteStream* outstream, char* infobuf, size_t insize, size_t& outsize)
{
    if(send(fd, outstream->GetData(), outstream->GetSize(), 0) == -1)
    {
        AC_ERROR("NetProxy::GetDBInfo():send error,%s",strerror(errno));
        return -1;
    }

    int len = 0;
    char buf[65535];
    if((len = recv(fd, buf, sizeof(buf), 0)) <= 0)
    {
        AC_ERROR("NetProxy::GetDBInfo():recv error");
        return -1;
    }

    short packlen = 0;
    memcpy(&packlen, buf, sizeof(short));
    packlen = ntohs(packlen);
    AC_DEBUG("NetProxy::GetDBInfo():packlen = %d", packlen);

    if(!packlen || sizeof(buf)<(size_t)packlen)
    {
        AC_ERROR("NetProxy::GetDBInfo():read packlen error");
        return -1;
    }

    char* ptr = buf + len;
    int ptrlen = packlen - len;
    while(ptrlen)
    {
        //没有收到一个整包，重复接收
        if((len = recv(fd, ptr, sizeof(buf), 0)) == -1)
        {
            AC_ERROR("NetProxy::GetDBInfo():recv again error");
            return -1;
        }
        ptr += len;
        ptrlen -= len;
    }

    BinaryReadStream instream(buf, sizeof(buf));	
    short cmd;
    if(!instream.Read(cmd))
    {
        AC_ERROR("NetProxy::GetDBInfo():read cmd error");
        return -1;
    }
    int inseq;
    if(!instream.Read(inseq))
    {
        AC_ERROR("NetProxy::GetDBInfo():read seq error");
        return -1;
    }
    int ret;
    if(!instream.Read(ret))
    {
        AC_ERROR("NetProxy::GetDBInfo():read ret error");
        return -1;
    }
    if(ret < 0)
    {
        AC_ERROR("NetProxy::GetDBInfo():db ret error");
        return -1;
    }
    short argnum;
    if(!instream.Read(argnum))
    {
        AC_ERROR("NetProxy::GetDBInfo():read argnum error");
        return -1;
    }
    if(argnum != 0)//这个版本统一为0
    {
        AC_ERROR("NetProxy::GetDBInfo():argnum error");	
        return -1;
    }

    if(!instream.Read(infobuf, insize, outsize))
    {
        AC_ERROR("NetProxy::GetDBInfo():read info error");
        return -1;
    }

    return 0;
}

int NetProxy::GetRoomNumByIpPort(int fd)
{
	AC_DEBUG("NetProxy::GetRoomNumByIpPort():IP = %s port = %d", m_sIP, m_port);

	int outseq = g_pNetProxy->GetCounter()->Get();
	BinaryWriteStream* outstream = StreamFactory::InstanceWriteStream();
	static const char* spname = {"DBMW_GetRoomNum_ByIpPort"};
	outstream->Write((short)CMD_CALLSP);
	outstream->Write(outseq);
	outstream->Write(spname,strlen(spname));
	outstream->Write((short)2);
	outstream->Write((char)PT_INPUT);
	outstream->Write((char)PDT_VARCHAR);
	outstream->Write(m_sIP, strlen(m_sIP));
	outstream->Write((char)PT_INPUT);
	outstream->Write((char)PDT_INT);
	outstream->Write(m_port);
	outstream->Flush();

	char info[65535];
	size_t infolen;
	if(GetDBInfo(fd, outstream, info, sizeof(info), infolen) == -1)
	{
		AC_ERROR("NetProxy::GetRoomNumByIpPort():read info error");
		return -1;
	}

	BinaryReadStream infostream(info,infolen);
	short rownum;
	if(!infostream.Read(rownum))
	{
		AC_ERROR("NetProxy::GetRoomNumByIpPort():read row num error");
		return -1;
	}

	short colnum;
	if(!infostream.Read(colnum))
	{
		AC_ERROR("NetProxy::GetRoomNumByIpPort():read col num error");
		return -1;
	}

	char infobuf[16] = {0};
	size_t curlen;
	int roomnum = 0;

	if(colnum != 1 || rownum != 1)
	{
		AC_ERROR("NetProxy::GetRoomNumByIpPort():colnum rownum error");
		return -1;
	}

	if(!infostream.Read(infobuf,sizeof(infobuf),curlen))
	{
		AC_ERROR("NetProxy::GetRoomNumByIpPort():read ptr error");
		return -1;
	}

	roomnum = atoi(infobuf);

	return roomnum;

}

int NetProxy::GetRoomDetailPrePage(int fd, int pagenum)
{
	int outseq = g_pNetProxy->GetCounter()->Get();	
	BinaryWriteStream* outstream = StreamFactory::InstanceWriteStream();
	static const char* spname = {"DBMW_GetRoomInfo_ByIpPort"};
	outstream->Write((short)CMD_CALLSP);
	outstream->Write(outseq);
	outstream->Write(spname,strlen(spname));
	outstream->Write((short)4);
	outstream->Write((char)PT_INPUT);
	outstream->Write((char)PDT_INT);
	outstream->Write(pagenum);
	outstream->Write((char)PT_INPUT);
	outstream->Write((char)PDT_INT);
	outstream->Write((int)ROOMNUMPERPAGE);
	outstream->Write((char)PT_INPUT);
	outstream->Write((char)PDT_VARCHAR);
	outstream->Write(m_sIP, strlen(m_sIP));
	outstream->Write((char)PT_INPUT);
	outstream->Write((char)PDT_INT);
	outstream->Write(m_port);
	outstream->Flush();	

	char info[65535];
	size_t infolen;
	if(GetDBInfo(fd, outstream, info, sizeof(info), infolen) == -1)
	{
		AC_ERROR("NetProxy::GetRoomDetailPrePage():read info error");
		return -1;
	}

	BinaryReadStream infostream(info,infolen);
	short rownum;
	if(!infostream.Read(rownum))
	{
		AC_ERROR("NetProxy::GetRoomDetailPrePage():read row num error");
		return -1;
	}

	short colnum;
	if(!infostream.Read(colnum))
	{
		AC_ERROR("NetProxy::GetRoomDetailPrePage():read col num error");
		return -1;
	}

	char infobuf[1024];
	size_t curlen;
	for(int i = 0;i < rownum;i++)
	{
		ROOM_INFO* proominfo = new ROOM_INFO();
		int roomid = 0;
		for(int j = 0;j < colnum;j++)
		{
			if(!infostream.Read(infobuf,sizeof(infobuf),curlen))
			{
				AC_ERROR("NetProxy::GetRoomDetailPrePage():read ptr error");
				return -1;
			}
			infobuf[curlen] = 0;
			switch(j)
			{
			case 0:{}break;	//pos
			case 1:
			{
				roomid = atoi(infobuf);
				proominfo->roomid = roomid;
			}
			break;
			case 2:
			{
				int hallid = atoi(infobuf);
 				if(hallid != m_pHallsvrCfg->m_id)
 				{
 					AC_ERROR("NetProxy::GetRoomDetailPrePage():hallid error [%d],[%d]",hallid,m_pHallsvrCfg->m_id);
					return -1;
				}
				
               		 m_roomlistinfo.hallid = hallid;
			}
			break;
			case 3:
			{
            			if(infobuf!=NULL)
					strcpy(proominfo->roomname, infobuf);
			}
			break;
			case 4:
			{
            			if(infobuf!=NULL)
					strcpy(proominfo->passwd, infobuf);
				else 
					memset(proominfo->passwd,0,sizeof(proominfo->passwd));		
			}
			break;
			case 5:
			{
				proominfo->type = atoi(infobuf);
			}
			break;
			case 6:
			{
				proominfo->state = atoi(infobuf);
			}
			break;
 			case 7:
			{
 				proominfo->ownidx = atoi(infobuf);
 			}
			break;
			case 8:{}break;	//房间创建时间
			case 9:				//房间自由排序标志
			{
				int freeflag = atoi(infobuf);
				if(freeflag ==1)
					proominfo->isMicUpdown = true;
				else if(freeflag == 0)
					proominfo->isMicUpdown = false;
				
			}
			break;
			case 10:
			{
				proominfo->maxhuman = atoi(infobuf);
			}
			break;
			case 11:{}break;	//房间到期时间
			case 12:{}break;	//自动审核会员申请
			case 13:	//房间公告
			{
				if(infobuf!=NULL)
					strcpy(proominfo->content, infobuf);
				else
					memset(proominfo->content,0,sizeof(proominfo->content));
			}
			break;
			case 14:{}break;	//房间IP地址--电信
			case 15:{}break;	//房间端口
			case 16:	//欢迎词
			{
				            			//modify by jinguanfu 2010/1/27 房间公告改欢迎词
				//strcpy(proominfo->introduce, infobuf);
				if(infobuf!=NULL)
					strcpy(proominfo->welcomeword,infobuf);
				else
					memset(proominfo->welcomeword,0,sizeof(proominfo->welcomeword));
			}
			break;
			case 17:	{}break;//房间更新时间
			//add by jinguanfu 2010/1/27 <begin>
			case 18:
			{
				if(infobuf!=NULL)
			 		strcpy(proominfo->roomlogo,infobuf);
				else
					memset(proominfo->roomlogo,0,sizeof(proominfo->roomlogo));	
			}
			break;
			//add by jinguanfu 2010/1/27 <end>
			//add by jinguanfu 2010/4/1 <begin>
			case  19://是否允许公聊标志位
			{
				int  chatflag =atoi(infobuf);
				if(chatflag == 1)
					proominfo->isPublicChat=true;
				else if(chatflag == 0)
					proominfo->isPublicChat=false;
			}
			break;
			case 20://用户进出信息显示标志位
			{
				int inoutflag = atoi(infobuf);
				if(inoutflag ==1)
					proominfo->isUserInOut = true;
				else if(inoutflag == 0)
					proominfo->isUserInOut = false;
			}
			break;
			case 21://房间是否公开标志位
			{
				int useronlyflag = atoi(infobuf);
				if(useronlyflag ==1)
					proominfo->isUserOnly = true;
				else if(useronlyflag == 0)
					proominfo->isUserOnly = false;
			}
			break;
			case 22://房间是否关闭标志位
			{
				int closeflag = atoi(infobuf);
				if(closeflag ==1)
					proominfo->isClose = true;
				else if(closeflag == 0)
					proominfo->isClose = false;
			}
			break;
			//add by jinguanfu 2010/4/1 <end>
			case 23:
			{
				proominfo->allowhuman = atoi(infobuf);
			}
			break;
			case 24:	//房间IP地址--网通
				{}
				break;
			case 25:	//音视频服务器地址--电信
			{
				if(curlen<sizeof(proominfo->AVServerIP_telcom))
				{
					strncpy(proominfo->AVServerIP_telcom,infobuf,curlen);
					proominfo->AVServerIP_telcom[curlen]=0;
				}
				else
				{
				 	AC_ERROR("NetProxy::GetRoomDetailPrePage():AVServerIP_telcom len=%d",curlen);
					return -1;
				}
				
			}
			break;
			case 26://音视频服务器端口--电信
			{
				proominfo->AVServerPort_telcom=atoi(infobuf);
			}
			break;
			case 27://音视频服务器地址--网通
			{
				if(curlen<sizeof(proominfo->AVServerIP_netcom))
				{
					strncpy(proominfo->AVServerIP_netcom,infobuf,curlen);
					proominfo->AVServerIP_netcom[curlen]=0;
				}
				else
				{
				 	AC_ERROR("NetProxy::GetRoomDetailPrePage():AVServerIP_netcom len=%d",curlen);
					return -1;
				}
			}
			break;
			case 28://音视频服务器端口--网通
			{
				proominfo->AVServerPort_netcom=atoi(infobuf);
			}
			/*
			break;
			case 29://中继服务器地址--电信
			{
				if(curlen<sizeof(proominfo->RelayIP_telcom))
				{
					strncpy(proominfo->RelayIP_telcom,infobuf,curlen);
					proominfo->RelayIP_telcom[curlen]=0;
				}
				else
				{
				 	AC_ERROR("NetProxy::GetRoomDetailPrePage():RelayIP_telcom len=%d",curlen);
					return -1;
				}
			}
			break;
			case 30://中继服务器端口--电信
			{
				proominfo->RelayPort_telcom=atoi(infobuf);
			}
			break;
			case 31://中继服务器地址--网通
			{
				if(curlen<sizeof(proominfo->RelayIP_netcom))
				{
					strncpy(proominfo->RelayIP_netcom,infobuf,curlen);
					proominfo->RelayIP_netcom[curlen]=0;
				}
				else
				{
				 	AC_ERROR("NetProxy::GetRoomDetailPrePage():RelayIP_netcom len=%d",curlen);
					return -1;
				}
			}
			break;
			case 32://中继服务器端口--网通
			{
				proominfo->RelayPort_telcom=atoi(infobuf);
			}
			*/
			default:
			{
				if(j==colnum-1)
				{
	              	proominfo->vjonmic = 0;
	                proominfo->secondownidx = 0;
	                proominfo->secondownidx2 = 0;
					proominfo->isAutoOnmic=true;
					//add by jinguanfu 2010/2/3 <begin> 上麦超时
					proominfo->m_pOnMicTimeout=g_pNetProxy->CreateRoomTimeout();
					if(proominfo->m_pOnMicTimeout!=NULL)
					{
						proominfo->m_pOnMicTimeout->SetReactor(g_pNetProxy->GetReactor());
						proominfo->m_pOnMicTimeout->m_type=TIMEOUT_ONMIC;
						proominfo->m_pOnMicTimeout->m_roomid=roomid;
						proominfo->m_pOnMicTimeout->m_idx= proominfo->ownidx;
					}
					else
					{
				 		AC_ERROR("NetProxy::GetRoomDetailPrePage():CreateRoomTimeout error");
						return -1;
					}
					//((roomtimeout*)(proominfo->m_pOnMicTimeout))->RegisterTimer(0);
					//add by jinguanfu 2010/2/3 <end> 上麦超时
					proominfo->FlowerTimer=g_pNetProxy->CreateFlowerResult();
					if(proominfo->FlowerTimer!=NULL)
					{
						proominfo->FlowerTimer->SetReactor(g_pNetProxy->GetReactor());
						proominfo->FlowerTimer->roomid=roomid;
						proominfo->FlowerTimer->RegisterTimer(SENDGIFT_SETTM);		// modify by lihongwu 2011/9/30
					}
					else
					{
				 		AC_ERROR("NetProxy::GetRoomDetailPrePage():CreateFlowerResult error");
						return -1;
					}

					proominfo->userlist.clear();
					proominfo->userlistAPP.clear();
					proominfo->userlistFemale.clear();
					proominfo->userlistMale.clear();
					proominfo->userlistVIP.clear();
					proominfo->vuserlist.clear();
					
					proominfo->blacklist.clear();
					proominfo->tblacklist.clear();
					proominfo->managerlist.clear();
					proominfo->managerlist_online.clear();
					proominfo->vjlist.clear();
					proominfo->vjlist_a.clear();
					proominfo->miclist.clear();

					proominfo->invitelist.clear();
					proominfo->forbidenlist.clear();
					proominfo->sendgiftlist.clear();
					
					m_roomlistinfo.roommap.insert(make_pair(roomid, proominfo));
					AC_DEBUG("NetProxy::GetRoomDetailPrePage():proominfo->roomid = %d proominfo->roomname = %s proominfo->passwd = %s proominfo->type = %d proominfo->state = %d proominfo->ownidx = %d proominfo->maxhuman = %d proominfo->allowhuman=%d proominfo->welcomeword = %s proominfo->content = %s",
						proominfo->roomid,proominfo->roomname, proominfo->passwd, proominfo->type, proominfo->state, proominfo->ownidx,
						proominfo->maxhuman,proominfo->allowhuman, proominfo->welcomeword, proominfo->content);
				}
			}
			}
		}
	}
	
	return 0;
}

int NetProxy::GetRoomsblacklist(int fd)
{
	map<int, ROOM_INFO*>::iterator it = m_roomlistinfo.roommap.begin();
	for(;it != m_roomlistinfo.roommap.end();it++)
	{
		ROOM_INFO* proominfo = (ROOM_INFO*)((*it).second);
		if(proominfo)
		{
			if(GetblacklistByRoomid(fd, proominfo) == -1)
			{
				AC_ERROR("NetProxy::GetRoomsblacklist():GetblacklistByRoomid error");
				return -1;
			}
		}
		else
		{
			AC_ERROR("proominfo == NULL");
			return -1;
		}
	}
	return 0;
}

int NetProxy::GetblacklistByRoomid(int fd, ROOM_INFO* proominfo)
{
	int outseq = g_pNetProxy->GetCounter()->Get();
	BinaryWriteStream* outstream = StreamFactory::InstanceWriteStream();
	static const char* spname = {"DBMW_GetMultiblacklist_byroomid"};
	outstream->Write((short)CMD_CALLSP);
	outstream->Write(outseq);
	outstream->Write(spname,strlen(spname));
	outstream->Write((short)1);
	outstream->Write((char)PT_INPUT);
	outstream->Write((char)PDT_INT);
	outstream->Write((int)proominfo->roomid);
	outstream->Flush();

	char info[65535];
	size_t infolen;
	if(GetDBInfo(fd, outstream, info, sizeof(info), infolen) == -1)
	{
		AC_ERROR("NetProxy::GetblacklistByRoomid():read info error");
		return -1;
	}

	BinaryReadStream infostream(info,infolen);
	short rownum;
	if(!infostream.Read(rownum))
	{
		AC_ERROR("NetProxy::GetblacklistByRoomid():read row num error");
		return -1;
	}

	short colnum;
	if(!infostream.Read(colnum))
	{
		AC_ERROR("NetProxy::GetblacklistByRoomid():read col num error");
		return -1;
	}

	proominfo->blacklist.clear();

	AC_DEBUG("NetProxy::GetblacklistByRoomid():rownum = %d colnum = %d", rownum, colnum);
	char infobuf[1024];
	size_t curlen;
	for(int i = 0;i < rownum;i++)
	{
		for(int j = 0;j < colnum;j++)
		{
			if(!infostream.Read(infobuf,sizeof(infobuf),curlen))
			{
				AC_ERROR("NetProxy::GetblacklistByRoomid():read ptr error");
				return -1;
			}
			infobuf[curlen] = 0;

			if(j == colnum - 1)
			{
				int idx = atoi(infobuf);
				proominfo->blacklist.insert(make_pair(idx, idx));
				AC_DEBUG("NetProxy::GetblacklistByRoomid():rownum = %d colnum = %d idx = %d",i,j,idx);
			}
			//AC_DEBUG("NetProxy::GetblacklistByRoomid():rownum = %d colnum = %d infobuf = %s",i,j,infobuf);
		}
	}

	return 0;
}

int NetProxy::GetRoomManagerlist(int fd)
{
	map<int, ROOM_INFO*>::iterator it = m_roomlistinfo.roommap.begin();
	for(;it != m_roomlistinfo.roommap.end();it++)
	{
		ROOM_INFO* proominfo = (ROOM_INFO*)((*it).second);
		if(proominfo)
		{
			if(GetManagerlistByRoomid(fd, proominfo) == -1)
			{
				AC_ERROR("NetProxy::GetRoomManagerlist():GetManagerlistByRoomid error");
				return -1;
			}
			//add by jinguanfu 2010/4/19 取房间申请列表
			if(GetApplylistByRoomid(fd, proominfo)==-1)
			{
				AC_ERROR("NetProxy::GetRoomManagerlist():GetApplylistByRoomid error");
				return -1;
			}
		}
		else
		{
			AC_ERROR("proominfo == NULL");
		}
	}

	//取GM
	if(GetGMlist(fd) == -1)
	{
		AC_ERROR("NetProxy::GetRoomManagerlist():GetGMlist error");
		return -1;
	}
	return 0;
}

int NetProxy::GetManagerlistByRoomid(int fd, ROOM_INFO* proominfo)
{
	int outseq = g_pNetProxy->GetCounter()->Get();
	BinaryWriteStream* outstream = StreamFactory::InstanceWriteStream();
	static const char* spname = {"DBMW_GetMultiAdmin_byroomid"};
	outstream->Write((short)CMD_CALLSP);
	outstream->Write(outseq);
	outstream->Write(spname,strlen(spname));
	outstream->Write((short)1);
	outstream->Write((char)PT_INPUT);
	outstream->Write((char)PDT_INT);
	outstream->Write((int)proominfo->roomid);
	outstream->Flush();

	char info[65535];
	size_t infolen;
	if(GetDBInfo(fd, outstream, info, sizeof(info), infolen) == -1)
	{
		AC_ERROR("NetProxy::GetManagerlistByRoomid():read info error");
		return -1;
	}

	BinaryReadStream infostream(info,infolen);
	short rownum;
	if(!infostream.Read(rownum))
	{
		AC_ERROR("NetProxy::GetManagerlistByRoomid():read row num error");
		return -1;
	}

	short colnum;
	if(!infostream.Read(colnum))
	{
		AC_ERROR("NetProxy::GetManagerlistByRoomid():read col num error");
		return -1;
	}

	proominfo->vjlist.clear();
	proominfo->vjlist_a.clear();
	proominfo->managerlist.clear();
	proominfo->userlistVIP.clear();

	AC_DEBUG("NetProxy::GetManagerlistByRoomid():rownum = %d colnum = %d", rownum, colnum);
	char infobuf[1024];
	size_t curlen;
	for(int i = 0;i < rownum;i++)
	{
		int idx = 0;
		for(int j = 0;j < colnum;j++)
		{
			if(!infostream.Read(infobuf,sizeof(infobuf),curlen))
			{
				AC_ERROR("NetProxy::GetManagerlistByRoomid():read ptr error");
				return -1;
			}
			infobuf[curlen] = 0;

			if(j == colnum - 2)
			{
				idx = atoi(infobuf);
			}

			if(j == colnum - 1)
			{
				int level = atoi(infobuf);

				proominfo->userlistVIP.insert(make_pair(idx, idx));
				
				ROOM_MANAGER roommanager;
				roommanager.m_idx=idx;
				roommanager.m_identity=(char)level;
				proominfo->managerlist.insert(make_pair(idx,roommanager));

				if(level == USER_ID_VJ)
				{
					proominfo->vjlist.insert(make_pair(idx, idx));
				}
				if(level == USER_ID_VJ_A)
				{
					proominfo->vjlist_a.insert(make_pair(idx, idx));	
				}
              	if(level == USER_ID_OWNER)
                {
                    proominfo->ownidx = idx;
                }
                if(level == USER_ID_OWNER_S)
                {
                    if(proominfo->secondownidx == 0)
                        proominfo->secondownidx = idx;	
                    else if(proominfo->secondownidx2 == 0)
                        proominfo->secondownidx2 = idx;
                }
						
			}
			AC_DEBUG("NetProxy::GetManagerlistByRoomid():rownum = %d colnum = %d roomid = %d,membernum=%d,vjnum=%d,vj_anum=%d,mangernum=%d",
				i,j,proominfo->roomid,proominfo->userlistVIP.size(),proominfo->vjlist.size(),proominfo->vjlist_a.size(),proominfo->managerlist.size());
		}
	}

	return 0;
}

int NetProxy::GetGMlist(int fd)
{
    int outseq = g_pNetProxy->GetCounter()->Get();
    BinaryWriteStream* outstream = StreamFactory::InstanceWriteStream();
    static const char* spname = {"DBMW_GetMultiAdmin_byroomid"};
    outstream->Write((short)CMD_CALLSP);
    outstream->Write(outseq);
    outstream->Write(spname,strlen(spname));
    outstream->Write((short)1);
    outstream->Write((char)PT_INPUT);
    outstream->Write((char)PDT_INT);
    outstream->Write((int)9999);
    outstream->Flush();

    char info[65535];
    size_t infolen;
    if(GetDBInfo(fd, outstream, info, sizeof(info), infolen) == -1)
    {
        AC_ERROR("NetProxy::GetGMlist():read info error");
        return -1;
    }

    BinaryReadStream infostream(info,infolen);
    short rownum;
    if(!infostream.Read(rownum))
    {
        AC_ERROR("NetProxy::GetGMlist():read row num error");
        return -1;
    }

    short colnum;
    if(!infostream.Read(colnum))
    {
        AC_ERROR("NetProxy::GetGMlist():read col num error");
        return -1;
    }

	m_GM.clear();

    AC_DEBUG("NetProxy::GetGMlist():rownum = %d colnum = %d", rownum, colnum);
    char infobuf[1024];
    size_t curlen;
    for(int i = 0;i < rownum;i++)
    {
        int idx = 0;
        for(int j = 0;j < colnum;j++)
        {
            if(!infostream.Read(infobuf,sizeof(infobuf),curlen))
            {
                AC_ERROR("NetProxy::GetGMlist():read ptr error");
                return -1;
            }
            infobuf[curlen] = 0;

            if(j == colnum - 2)
            {
                idx = atoi(infobuf);
            }

            if(j == colnum - 1)
            {
			int level = atoi(infobuf);

			if(level != USER_ID_GM)
				return -1;
				
			GM_INFO  gminfo;
			gminfo.idx=idx;
			gminfo.addflag=1;
			m_GM.insert(make_pair(idx,gminfo));
                
            }
            AC_DEBUG("NetProxy::GetGMlist():rownum = %d colnum = %d infobuf = %s",i,j,infobuf);
        }
    }

    return 0;
}

//modify by jinguanfu 2010/1/18 
/*
DB_RESULT_DATA* NetProxy::GetClientDBMap(int seq)
{
	map<int, DB_RESULT_DATA>::iterator it = m_DBMap.find(seq);
	if(it != m_DBMap.end())
		return (DB_RESULT_DATA*)(&((*it).second)); 

	return NULL;
}
*/
Dbresultdata* NetProxy::GetClientDBMap(int seq)
{
	map<int, Dbresultdata*>::iterator it = m_DBMap.find(seq);
	if(it != m_DBMap.end())
		return (Dbresultdata*)(((*it).second)); 

	return NULL;
}
//add by jinguanfu 2010/1/22
LobbyTimeout* NetProxy::GetLobbyTMMap(int seq)
{
	map<int, LobbyTimeout*>::iterator it = m_LobbyTMMap.find(seq);
	if(it != m_LobbyTMMap.end())
		return (LobbyTimeout*)(((*it).second)); 

	return NULL;
}
/*
void NetProxy::ClearFlowerResult(int seq)
{
	vector<Flowerresultdata*>::iterator it = m_flower.begin();
	for(;it != m_flower.end(); it++)
	{
		Flowerresultdata* fl=(Flowerresultdata*)(*it);
		if(fl->seq==seq)
		{
			m_flower.erase(it);
			return;
		}
	}
}
*/
/*modify by jinguanfu 2010/1/19
void NetProxy::ClearClientDBMap(int seq)
{
	map<int, DB_RESULT_DATA>::iterator it = m_DBMap.find(seq);
	if(it != m_DBMap.end())
		m_DBMap.erase(it);
}
*/
void NetProxy::ClearClientDBMap(int seq)
{
	map<int, Dbresultdata*>::iterator it = m_DBMap.find(seq);
	if(it != m_DBMap.end())
		m_DBMap.erase(it);
}
/*	modify by jinguanfu 2010/1/19
void NetProxy::ClearClientDBMap(ClientSocketBase* pClient)
{
    RoomClient* pRoomClient = (RoomClient*)pClient;
	map<int, DB_RESULT_DATA>::iterator it = m_DBMap.begin();
	for(;it != m_DBMap.end(); it++)
	{
		DB_RESULT_DATA* p = (DB_RESULT_DATA*)(&(*it).second);
		if(p->opidx == (int)pRoomClient->m_idx || p->bopidx == (int)pRoomClient->m_idx)
			m_DBMap.erase(it);
	}
}
*/
void NetProxy::ClearClientDBMap(ClientSocketBase* pClient)
{
	map<int,Dbresultdata*> clearlist;
	RoomClient* pRoomClient = (RoomClient*)pClient;
	map<int, Dbresultdata*>::iterator it = m_DBMap.begin();
	for(;it != m_DBMap.end(); it++)
	{
		Dbresultdata* p = (Dbresultdata*)(&(*it).second);
		/*
		if(p->opidx == (int)pRoomClient->m_idx || p->bopidx == (int)pRoomClient->m_idx)
		{
			//clearlist.insert(make_pair(it->first,it->second));
		}
		*/
		if(p->opidx == pRoomClient->m_idx )
		{
			p->opidx=0;
		}

		if(p->bopidx == pRoomClient->m_idx) 
		{
			p->bopidx=0;
		}
	}
	/*
	map<int,Dbresultdata*>::iterator  itc=clearlist.begin();
	for(;itc!=clearlist.end();itc++)
	{	
		int seq=itc->first;
		Dbresultdata* p= (Dbresultdata*)itc->second;
		map<int, Dbresultdata*>::iterator itDB = m_DBMap.find(seq);
		if(itDB!=m_DBMap.end())
		{
			m_DBMap.erase(itDB);
			DestroyDBResult(p);
			
		}
	}
	*/
}

void NetProxy::ClearLobbyTMMap(int outseq)
{
	map<int, LobbyTimeout*>::iterator it = m_LobbyTMMap.find(outseq);
	if(it != m_LobbyTMMap.end())
		m_LobbyTMMap.erase(it);

}
ROOM_INFO* NetProxy::GetRoomInfo(ClientSocketBase *pClient)
{
	RoomClient* pRoomClient = (RoomClient*)pClient;
	if(pRoomClient->m_roomid <= 0)
	{
		AC_ERROR("NetProxy::GetRoomInfo:client error");
		return NULL;
	}
	map<int, ROOM_INFO*>::iterator it = m_roomlistinfo.roommap.find(pRoomClient->m_roomid);
	if(it == m_roomlistinfo.roommap.end())
	{
		AC_ERROR("NetProxy::GetRoomInfo:roomid error");
		return NULL;
	}

	return (ROOM_INFO*)((*it).second);
}

ROOM_INFO* NetProxy::GetRoomInfo(int roomid)
{
    if(roomid <= 0)
    {
        AC_ERROR("NetProxy::GetRoomInfo:roomid error");
        return NULL;
    }
    map<int, ROOM_INFO*>::iterator it = m_roomlistinfo.roommap.find(roomid);
    if(it == m_roomlistinfo.roommap.end())
    {
        AC_ERROR("NetProxy::GetRoomInfo:roomid error");
        return NULL;
    }

    return (ROOM_INFO*)((*it).second);
}

int NetProxy::BroadcastSBCmd(ROOM_INFO* proominfo, BinaryWriteStream& stream, int bdirect)
{
// 	//map<int,RoomClient*> closelist;
// 	map<int, RoomClient*>::iterator itu = proominfo->userlist.begin();
// 	for(;itu != proominfo->userlist.end();itu++)
// 	{
// 		RoomClient* psend = (*itu).second;
// 		if(psend != NULL)
// 		{
// 			if(!psend->m_bclose)
// 			{
// 				if(SendToSrv(psend, stream, 1, bdirect) == -1)
// 	            		{
// 					AC_DEBUG("NetProxy::BroadcastSBCmd:psend->m_bclose = %d,idx=%d", psend->m_bclose,psend->m_idx);
// 					psend->m_bclose=1;
// 					continue;
// 					/*
// 					if(!psend->m_bclose)
// 					{
// 						closelist.insert(make_pair(psend->m_idx,psend));
// 					}
// 					*/
// 	            		}
// 			}
// 		}
// 	}
// 	/*
// 	itu=closelist.begin();
// 	for(;itu!=closelist.end();itu++)
// 	{
// 		RoomClient* pclient = (*itu).second;
// 		if(pclient != NULL)
// 		{
// 			pclient->Close();
// 		}
// 	}
// 	*/


    char outbuf[65535]={0};                     
    int outbuflen = sizeof(outbuf);                 
    if(!StreamCompress(stream.GetData(),(int)stream.GetSize(),outbuf,outbuflen))
    {
        AC_ERROR("streamcompress error");
        return -1;
    }

    map<int, RoomClient*>::iterator itsend = proominfo->userlist.begin();
    for(;itsend != proominfo->userlist.end();itsend++)
	{
		RoomClient* psend = itsend->second;
		if(psend != NULL)
        {
            if(bdirect)
            {
                if(psend->DirectSend(outbuf, outbuflen) != 0)
                {
                    //psend->ErrorClose();
					AC_ERROR("psend->AddBuf Error");
                    //return 0;
                }
            }
            else
            {
                if(psend->AddBuf(outbuf,outbuflen) != 0)                                                     
                {
                    //psend->ErrorClose();
					AC_ERROR("psend->AddBuf Error");
                    //return 0;
                }                            
            }
        }
    }
	return 0;
}

int NetProxy::SendToSrv(ClientSocketBase *pClient, BinaryWriteStream& stream, int btea, int bdirect)
{
	RoomClient* pRoomClient = (RoomClient*)pClient;

	if(!btea)
		return  pRoomClient->SendEncryptBuf(stream, btea, bdirect);
	else
	{
		if(stream.GetSize() < MAXBUFSIZETEA)
			return pRoomClient->SendEncryptBuf(stream, btea, bdirect);
		return pRoomClient->SendCompressBuf(stream, bdirect);
	}

	return 0;
}

int NetProxy::ReportMyStatus(HALL_SERVER_CMD _st)
{

	AC_DEBUG("NetProxy::ReportMyStatus: Report room status to lobby");
	char outbuf[65535] = {0};
	//BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
	BinaryWriteStream outstream(outbuf, sizeof(outbuf));
	outstream.Write((short)_st);
	int seq = g_pNetProxy->GetCounter()->Get();
	outstream.Write(seq);
	outstream.Write((char)ROOM_TYPE_ROOM);	//ADD BY JINGUANFU 2010/7/9	
	outstream.Write(g_pNetProxy->m_sIP,strlen(g_pNetProxy->m_sIP));
	outstream.Write((short)g_pNetProxy->m_port);
	outstream.Write((int)g_pNetProxy->m_roomlistinfo.roommap.size());//roomnum
	
	map<int, ROOM_INFO*>::iterator itr = g_pNetProxy->m_roomlistinfo.roommap.begin();
	for(;itr!=g_pNetProxy->m_roomlistinfo.roommap.end();itr++)
	{
		outstream.Write((int)itr->first);	//roomid
		ROOM_INFO* proominfo=(ROOM_INFO*)(itr->second);
		//add by jinguanfu 2010/12/14
		//重新连接大厅后，消除所有机器人用户
		if(proominfo)
		{
			map<int,RoomClient*>::iterator itvu=proominfo->vuserlist.begin();
			for(;itvu!=proominfo->vuserlist.end();itvu++)
			{
				RoomClient* pvClient=(RoomClient*)itvu->second;
				pvClient->VLeaveRoom();
			}
			proominfo->vuserlist.clear();

			outstream.Write((int)proominfo->userlistMale.size());	//mannum
			outstream.Write((int)proominfo->userlistFemale.size());	//womannum

			outstream.Write((int)proominfo->userlist.size());	//usernum
			map<int,RoomClient*>::iterator itc=proominfo->userlist.begin();
			for(;itc!=proominfo->userlist.end();itc++)
				outstream.Write((int)itc->first);	//useridx
		}
		else
		{
			AC_ERROR("proominfo == NULL");
		}

	}
	
	outstream.Flush();
	HallSvrClient *pHallSvr = g_pNetProxy->GetHallSvr();
	if(pHallSvr!=NULL)
		pHallSvr->AddBuf(outstream.GetData(), outstream.GetSize());			
	else
		AC_ERROR("NetProxy::ReportMyStatus:pHallSvr is NULL ");

	return 0;

}

//int NetProxy::PushBackFlower(FLOWER_RESULT_DATA fl)
int NetProxy::PushBackFlower(Flowerresultdata* fl)	
{
	//if (m_flower.size()>10)
	//{
	//	m_flower.erase(m_flower.begin());
	//}
	m_flower.push_back(fl);
	
	return 0;

}

/*
int NetProxy::FlowerStatusScan(struct timeval _time)
{
	//30MS送一朵花，现在定时器是3S，每扫描一次减100
	if (_time.tv_sec-m_time.tv_sec<3)
	{
		return 0;
	}
	int times=(_time.tv_sec-m_time.tv_sec)/3;
	//vector<FLOWER_RESULT_DATA>::iterator it=m_flower.begin();	
	vector<Flowerresultdata>::iterator it=m_flower.begin();	
	for (;it!=m_flower.end();it++)
	{
		//FLOWER_RESULT_DATA* temp=&(*it);		
		Flowerresultdata* temp=&(*it);	
		temp->havesend+=100*times;
		if (temp->number<=temp->havesend)
		{				
			m_flower.erase(it);
			it--;
		}				
	}
	//记录当前操作时间
	//m_time=_time;
	return 0;
	
}
*/
int NetProxy::ResultDataScan(timeval _time)
{
	//map<int, DB_RESULT_DATA>m_DBMap;
	int jj=0;//记录每次清理的数量
	//modify by jinguanfu 2010/1/19
	//map<int,DB_RESULT_DATA>::iterator it=m_DBMap.begin();
	map<int,Dbresultdata*>::iterator it=m_DBMap.begin();
	for (;it!=m_DBMap.end();)
	{
		jj++;
		if (jj>g_pNetProxy->m_Numberonescan)
		{
			return 0;
		}
		//DB_RESULT_DATA* temp=&(it->second);
		Dbresultdata* temp=(it->second);
		if (_time.tv_sec-temp->regtime.tv_sec>=m_timeout)
		{
			//map<int,DB_RESULT_DATA>::iterator tempit;
			map<int,Dbresultdata*>::iterator tempit;
			tempit=it;
			it++;
			m_DBMap.erase(tempit);	
		}
		else
			it++;
	}
	return 0;
}

int NetProxy::MonitorScan(timeval _time)
{
	//每5S报告一次自身的信息
	if (_time.tv_sec-m_time.tv_sec<5)
	{
		return 0;
	}
	//发送给dbagent
	//char outbuf[256] = {0};
	map<int, ROOM_INFO*>::iterator itroom=m_roomlistinfo.roommap.begin();
	for(;itroom!=m_roomlistinfo.roommap.end();itroom++)
	{
		ROOM_INFO* tempRoom=(itroom->second);
		if(tempRoom)
		{
			int outseq = g_pNetProxy->GetCounter()->Get();
			BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
			static const char* spname = {"SLM_Insert_ViewRoom"};//存储过程填充
			outstream->Write((short)CMD_CALLSP);
			outstream->Write(outseq);
			outstream->Write(spname,strlen(spname));
			outstream->Write((short)6);
			outstream->Write((char)PT_INPUT);
			outstream->Write((char)PDT_INT);
			outstream->Write(m_roomlistinfo.hallid);			
			outstream->Write((char)PT_INPUT);
			outstream->Write((char)PDT_INT);
			outstream->Write((int)tempRoom->roomid);
			outstream->Write((char)PT_INPUT);
			outstream->Write((char)PDT_INT);
			outstream->Write((int)tempRoom->userlist.size());
			outstream->Write((char)PT_INPUT);
			outstream->Write((char)PDT_INT);
			outstream->Write((int)tempRoom->miclist.size());
			outstream->Write((char)PT_INPUT);
			outstream->Write((char)PDT_INT);
			outstream->Write((int)tempRoom->vjonmic);
			outstream->Write((char)PT_INPUT);
			outstream->Write((char)PDT_INT);
			outstream->Write((int)0);//tempRoom->flower送花的现在不用
			outstream->Flush();
			BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBMonitor();
			pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize());
		}
		else
		{
			AC_ERROR("proominfo == NULL");
		}
	}	
	return 0;
}

int NetProxy::DeleteRoomByID(ROOM_INFO* roominfo)
{
    //delete room
    map<int, ROOM_INFO*>::iterator it = m_roomlistinfo.roommap.find(roominfo->roomid);
    if (it==m_roomlistinfo.roommap.end())
    {
        AC_ERROR("NetProxy::DeleteRoomByID():m_roomlistinfo.roommap.find error");
        return -1;

    }
	ROOM_INFO* proominfo = it->second;
	if(proominfo)
		delete proominfo;
    m_roomlistinfo.roommap.erase(it);
    m_roomlistinfo.roomnum--;
    return 0;
}

int NetProxy::GetApplylistByRoomid(int fd,ROOM_INFO* proominfo)
{

	int outseq = g_pNetProxy->GetCounter()->Get();
	BinaryWriteStream* outstream = StreamFactory::InstanceWriteStream();
	static const char* spname = {"DBMW_GetApplyList_HZSTAR"};//存储过程填充
	outstream->Write((short)CMD_CALLSP);
	outstream->Write(outseq);
	outstream->Write(spname,strlen(spname));
	outstream->Write((short)1);
	outstream->Write((char)PT_INPUT);
	outstream->Write((char)PDT_INT);
	outstream->Write((int)proominfo->roomid);			
	outstream->Flush();

	char info[65535];
	size_t infolen;
	if(GetDBInfo(fd, outstream, info, sizeof(info), infolen) == -1)
	{
		AC_ERROR("NetProxy::GetManagerlistByRoomid():read info error");
		return -1;
	}

	BinaryReadStream infostream(info,infolen);
	short rownum;
	if(!infostream.Read(rownum))
	{
		AC_ERROR("NetProxy::GetManagerlistByRoomid():read row num error");
		return -1;
	}

	short colnum;
	if(!infostream.Read(colnum))
	{
		AC_ERROR("NetProxy::GetManagerlistByRoomid():read col num error");
		return -1;
	}

	AC_DEBUG("NetProxy::GetManagerlistByRoomid():rownum = %d colnum = %d", rownum, colnum);
	char infobuf[1024];
	size_t curlen;
	for(int i = 0;i < rownum;i++)
		{		
			ROOM_APPLY roomapply;
			for(int j = 0;j < colnum;j++)
			{
				if(!infostream.Read(infobuf,sizeof(infobuf),curlen))
				{
					AC_ERROR("NetProxy::DoReturnRoomApplyList():read ptr error");
					return -1;
				}
				infobuf[curlen] = 0;
				if(j == 0){}
				if(j == 1)	//useridx
				{
					roomapply.m_idx= atoi(infobuf);
					//roomapply.m_id=rownum+1;
				}
				if(j == 2)//roomid
				{
					roomapply.m_roomid= atoi(infobuf);
					
				}
				if(j == 3)		//state,未处理的的state=0
				{
				}
				if(j == 4)
				{
					memset(roomapply.m_time,0,sizeof(roomapply.m_time));
					memcpy(roomapply.m_time,infobuf,sizeof(roomapply.m_time));
				}			
			}
			proominfo->userlistAPP.insert(make_pair(roomapply.m_idx,roomapply));
		}
	return 0;
}

int NetProxy::GetGiftInfo(int fd)
{
	int outseq = g_pNetProxy->GetCounter()->Get();
	BinaryWriteStream* outstream = StreamFactory::InstanceWriteStream();
	static const char* spname = {"DBMW_GetGiftInfo"};//存储过程填充
	outstream->Write((short)CMD_CALLSP);
	outstream->Write(outseq);
	outstream->Write(spname,strlen(spname));	
	outstream->Write((short)0);
	outstream->Flush();


	char info[65535];
	size_t infolen;
	if(GetDBInfo(fd, outstream, info, sizeof(info), infolen) == -1)
	{
		AC_ERROR("NetProxy::GetGiftInfo():read info error");
		return -1;
	}

	BinaryReadStream infostream(info,infolen);
	short rownum;
	if(!infostream.Read(rownum))
	{
		AC_ERROR("NetProxy::GetGiftInfo():read row num error");
		return -1;
	}

	short colnum;
	if(!infostream.Read(colnum))
	{
		AC_ERROR("NetProxy::GetGiftInfo():read col num error");
		return -1;
	}
	
	    AC_DEBUG("NetProxy::GetGiftInfo():rownum = %d colnum = %d", rownum, colnum);
	char infobuf[1024];
	size_t curlen;

	int giftid=0;
	int price=0;
	int extype=0;
	int vtime = 0;
	for(int i = 0;i < rownum;i++)
	{		
		for(int j = 0;j < colnum;j++)
		{
			if(!infostream.Read(infobuf,sizeof(infobuf),curlen))
			{
				AC_ERROR("NetProxy::GetGiftInfo():read ptr error");
				return -1;
			}
			infobuf[curlen] = 0;
			if(j == 0)	//礼物ID
			{
				giftid= atoi(infobuf);
			}
			if(j == 1)//礼物价格
			{
				price= atoi(infobuf);
			}		
			if(j == 2)//礼物处理类型
			{
				extype= atoi(infobuf);
			}
			if(j == 3)//礼物有效时间，仅对印章有效
				vtime = atoi(infobuf);
			
		}
		GIFT_INFO giftinfo;
		giftinfo.GiftID=giftid;
		giftinfo.price=price;
		giftinfo.type=extype;
		giftinfo.vtime=vtime;
		m_GiftInfo.insert(make_pair(giftid,giftinfo));
	}
	return 0;
}

MonitorSvrClient* NetProxy::GetMonitorSvr()
{
	return m_MonitorSrv;
}

int NetProxy::GetClientCount()
{
	int count=m_MapForClient.Size();

	return count;
}

void NetProxy::ReportStatusToMonitor()
{
	
	BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
	outstream->Write((short)MONITOR_CMD_REPORT);
	outstream->Write(0);
	outstream->Write(g_pNetProxy->m_sIP,strlen(g_pNetProxy->m_sIP));
	outstream->Write((short)g_pNetProxy->m_port);
	outstream->Write((int)MONITOR_CMD_REPLY);

	outstream->Flush();
	MonitorSvrClient *pMonitorSvr = g_pNetProxy->GetMonitorSvr();
	int ret=pMonitorSvr->AddBuf(outstream->GetData(), outstream->GetSize());			
	if(ret==-1)
	{
		AC_ERROR("NetProxy::ReportStatusToMonitor:pMonitorSvr->AddBuf error ");
		pMonitorSvr->Close();
	}

}


int NetProxy::LoadRightconfig(char* rightpath)
{
	/*
	RightConfig* pRconf=new RightConfig();
	
	pRconf->Init();
	if(pRconf->ReadCfg(rightpath) == -1)
	{
		AC_ERROR("read RightConfig error");
		delete pRconf;
		return -1;
	}
	if(m_pRightCfg!=NULL)
	{
		delete m_pRightCfg;	
	}
	
	m_pRightCfg=pRconf;
	*/
	
	RightConfigEx* pRconfEx=new RightConfigEx();
	pRconfEx->Init();
	if(pRconfEx->ReadCfg(rightpath) == -1)
	{
		AC_ERROR("read RightConfig error");
		delete pRconfEx;
		return -1;
	}

	m_pRightCfgEx=pRconfEx;

	return 0;


	//debug  view the xml
	FILE *gLogfile=NULL;
	char* xml="../etc/test.txt";
	char  temp[1024]={0};
	int writelen=0;
	
	gLogfile=fopen(xml,"a+");
	if(gLogfile==NULL)
	{
		AC_ERROR("log file open failed,errno:%d\n",errno);
		return -1;
	}
	
	int i,j,k;
	for(i=0;i<64;i++)
	{
		sprintf(temp,"type=%d,optionid=%d\r\n",m_pRightCfgEx->m_right[i].optiontype,i);
		writelen=fwrite(temp,1,strlen(temp),gLogfile);
		for(j=0;j<16;j++)
		{
			for(k=0;k<16;k++)
			{
				sprintf(temp,"%d,",m_pRightCfgEx->m_right[i].rightdetail[j][k]);
				writelen=fwrite(temp,1,strlen(temp),gLogfile);
			}
			sprintf(temp,"\r\n");
			writelen=fwrite(temp,1,strlen(temp),gLogfile);
		}
		fflush(gLogfile);
	}
	/*	
	sprintf(temp,"<?xml version=\"1.0\" ?>\r\n<roomright>\r\n");
	writelen=fwrite(temp,1,strlen(temp),gLogfile);

	map<int,int>::iterator it1;
	map<int,childcfg>::iterator it2;

	sprintf(temp,"<unpasswd>");
	writelen=fwrite(temp,1,strlen(temp),gLogfile);
	it1=m_pRightCfg->m_unpasswd.begin();
	for(;it1!=m_pRightCfg->m_unpasswd.end();it1++)
	{
		sprintf(temp,"<%d property=\"%d\" />",it1->first,it1->second);
		writelen=fwrite(temp,1,strlen(temp),gLogfile);
	}
	sprintf(temp,"</unpasswd>");
	writelen=fwrite(temp,1,strlen(temp),gLogfile);
	fflush(gLogfile);

	sprintf(temp,"<closed>");
	writelen=fwrite(temp,1,strlen(temp),gLogfile);
	it1=m_pRightCfg->m_closed.begin();
	for(;it1!=m_pRightCfg->m_closed.end();it1++)
	{
		sprintf(temp,"<%d property=\"%d\" />",it1->first,it1->second);
		writelen=fwrite(temp,1,strlen(temp),gLogfile);
	}
	sprintf(temp,"</closed>");
	writelen=fwrite(temp,1,strlen(temp),gLogfile);
	fflush(gLogfile);

	sprintf(temp,"<private>");
	writelen=fwrite(temp,1,strlen(temp),gLogfile);
	it1=m_pRightCfg->m_private.begin();
	for(;it1!=m_pRightCfg->m_private.end();it1++)
	{
		sprintf(temp,"<%d property=\"%d\" />",it1->first,it1->second);
		writelen=fwrite(temp,1,strlen(temp),gLogfile);
	}
	sprintf(temp,"</private>");
	writelen=fwrite(temp,1,strlen(temp),gLogfile);
	fflush(gLogfile);

	sprintf(temp,"<fulllimit>");
	writelen=fwrite(temp,1,strlen(temp),gLogfile);
	it1=m_pRightCfg->m_fulllimit.begin();
	for(;it1!=m_pRightCfg->m_fulllimit.end();it1++)
	{
		sprintf(temp,"<%d property=\"%d\" />",it1->first,it1->second);
		writelen=fwrite(temp,1,strlen(temp),gLogfile);
	}
	sprintf(temp,"</fulllimit>");
	writelen=fwrite(temp,1,strlen(temp),gLogfile);
	fflush(gLogfile);


	sprintf(temp,"<m_onvjmic>");
	writelen=fwrite(temp,1,strlen(temp),gLogfile);
	it1=m_pRightCfg->m_onvjmic.begin();
	for(;it1!=m_pRightCfg->m_onvjmic.end();it1++)
	{
		sprintf(temp,"<%d property=\"%d\" />",it1->first,it1->second);
		writelen=fwrite(temp,1,strlen(temp),gLogfile);
	}
	sprintf(temp,"</m_onvjmic>");
	writelen=fwrite(temp,1,strlen(temp),gLogfile);
	fflush(gLogfile);

	sprintf(temp,"<m_onvjmic>");
	writelen=fwrite(temp,1,strlen(temp),gLogfile);
	it1=m_pRightCfg->m_onvjmic.begin();
	for(;it1!=m_pRightCfg->m_onvjmic.end();it1++)
	{
		sprintf(temp,"<%d property=\"%d\" />",it1->first,it1->second);
		writelen=fwrite(temp,1,strlen(temp),gLogfile);
	}
	sprintf(temp,"</m_onvjmic>");
	writelen=fwrite(temp,1,strlen(temp),gLogfile);
	fflush(gLogfile);

	sprintf(temp,"<m_updownwaitmic>");
	writelen=fwrite(temp,1,strlen(temp),gLogfile);
	it1=m_pRightCfg->m_updownwaitmic.begin();
	for(;it1!=m_pRightCfg->m_updownwaitmic.end();it1++)
	{
		sprintf(temp,"<%d property=\"%d\" />",it1->first,it1->second);
		writelen=fwrite(temp,1,strlen(temp),gLogfile);
	}
	sprintf(temp,"</m_updownwaitmic>");
	writelen=fwrite(temp,1,strlen(temp),gLogfile);
	fflush(gLogfile);

	sprintf(temp,"<m_freewaitmic>");
	writelen=fwrite(temp,1,strlen(temp),gLogfile);
	it1=m_pRightCfg->m_freewaitmic.begin();
	for(;it1!=m_pRightCfg->m_freewaitmic.end();it1++)
	{
		sprintf(temp,"<%d property=\"%d\" />",it1->first,it1->second);
		writelen=fwrite(temp,1,strlen(temp),gLogfile);
	}
	sprintf(temp,"</m_freewaitmic>");
	writelen=fwrite(temp,1,strlen(temp),gLogfile);
	fflush(gLogfile);

	sprintf(temp,"<m_invitewaitmic>");
	writelen=fwrite(temp,1,strlen(temp),gLogfile);
	it1=m_pRightCfg->m_invitewaitmic.begin();
	for(;it1!=m_pRightCfg->m_invitewaitmic.end();it1++)
	{
		sprintf(temp,"<%d property=\"%d\" />",it1->first,it1->second);
		writelen=fwrite(temp,1,strlen(temp),gLogfile);
	}
	sprintf(temp,"</m_invitewaitmic>");
	writelen=fwrite(temp,1,strlen(temp),gLogfile);
	fflush(gLogfile);

	sprintf(temp,"<m_roomchat>");
	writelen=fwrite(temp,1,strlen(temp),gLogfile);
	it1=m_pRightCfg->m_roomchat.begin();
	for(;it1!=m_pRightCfg->m_roomchat.end();it1++)
	{
		sprintf(temp,"<%d property=\"%d\" />",it1->first,it1->second);
		writelen=fwrite(temp,1,strlen(temp),gLogfile);
	}
	sprintf(temp,"</m_roomchat>");
	writelen=fwrite(temp,1,strlen(temp),gLogfile);
	fflush(gLogfile);

	sprintf(temp,"<m_forbiden>");
	writelen=fwrite(temp,1,strlen(temp),gLogfile);
	it2=m_pRightCfg->m_forbiden.begin();
	for(;it2!=m_pRightCfg->m_forbiden.end();it2++)
	{
		childcfg cfg=it2->second;
		sprintf(temp,"<%d property=\"%d\" >",it1->first,cfg.property);
		writelen=fwrite(temp,1,strlen(temp),gLogfile);
		it1=cfg.boption.begin();
		for(;it1!=cfg.boption.end();it1++)
		{
			sprintf(temp,"<%d property=\"%d\" />",it1->first,it1->second);
			writelen=fwrite(temp,1,strlen(temp),gLogfile);
		}
		sprintf(temp,"</%d>",it2->first);
		writelen=fwrite(temp,1,strlen(temp),gLogfile);
	}
	sprintf(temp,"</m_forbiden>");
	writelen=fwrite(temp,1,strlen(temp),gLogfile);
	fflush(gLogfile);
	
	sprintf(temp,"<m_black>");
	writelen=fwrite(temp,1,strlen(temp),gLogfile);
	it2=m_pRightCfg->m_black.begin();
	for(;it2!=m_pRightCfg->m_black.end();it2++)
	{
		childcfg cfg=it2->second;
		sprintf(temp,"<%d property=\"%d\" >",it1->first,cfg.property);
		writelen=fwrite(temp,1,strlen(temp),gLogfile);
		it1=cfg.boption.begin();
		for(;it1!=cfg.boption.end();it1++)
		{
			sprintf(temp,"<%d property=\"%d\" />",it1->first,it1->second);
			writelen=fwrite(temp,1,strlen(temp),gLogfile);
		}
		sprintf(temp,"</%d>",it2->first);
		writelen=fwrite(temp,1,strlen(temp),gLogfile);
	}
	sprintf(temp,"</m_black>");
	writelen=fwrite(temp,1,strlen(temp),gLogfile);
	fflush(gLogfile);

	sprintf(temp,"<m_kick>");
	writelen=fwrite(temp,1,strlen(temp),gLogfile);
	it2=m_pRightCfg->m_kick.begin();
	for(;it2!=m_pRightCfg->m_kick.end();it2++)
	{
		childcfg cfg=it2->second;
		sprintf(temp,"<%d property=\"%d\" >",it1->first,cfg.property);
		writelen=fwrite(temp,1,strlen(temp),gLogfile);
		it1=cfg.boption.begin();
		for(;it1!=cfg.boption.end();it1++)
		{
			sprintf(temp,"<%d property=\"%d\" />",it1->first,it1->second);
			writelen=fwrite(temp,1,strlen(temp),gLogfile);
		}
		sprintf(temp,"</%d>",it2->first);
		writelen=fwrite(temp,1,strlen(temp),gLogfile);
	}
	sprintf(temp,"</m_kick>");
	writelen=fwrite(temp,1,strlen(temp),gLogfile);
	fflush(gLogfile);

	sprintf(temp,"<delmember>\r\n");
	writelen=fwrite(temp,1,strlen(temp),gLogfile);
	it2=m_pRightCfg->m_delmember.begin();
	for(;it2!=m_pRightCfg->m_delmember.end();it2++)
	{
		childcfg cfg=it2->second;
		sprintf(temp,"<%d property=\"%d\" >\r\n",it2->first,cfg.property);
		writelen=fwrite(temp,1,strlen(temp),gLogfile);
		it1=cfg.boption.begin();
		for(;it1!=cfg.boption.end();it1++)
		{
			sprintf(temp,"<%d property=\"%d\" />\r\n ",it1->first,it1->second);
			writelen=fwrite(temp,1,strlen(temp),gLogfile);
		}
		sprintf(temp,"</%d>\r\n",it2->first);
		writelen=fwrite(temp,1,strlen(temp),gLogfile);
	}
	sprintf(temp,"</delmember>\r\n");
	writelen=fwrite(temp,1,strlen(temp),gLogfile);
	fflush(gLogfile);
	
	sprintf(temp,"</roomright>\r\n");
	writelen=fwrite(temp,1,strlen(temp),gLogfile);
	fflush(gLogfile);

	fclose(gLogfile);
*/
	return 0;
}


int NetProxy::GetLuckConf(int fd)
{
	int outseq = g_pNetProxy->GetCounter()->Get();
	BinaryWriteStream* outstream = StreamFactory::InstanceWriteStream();
	static const char* spname = {"DBMW_GetLuckConf_HZSTAR"};//存储过程填充
	outstream->Write((short)CMD_CALLSP);
	outstream->Write(outseq);
	outstream->Write(spname,strlen(spname));	
	outstream->Write((short)0);
	outstream->Flush();


	char info[65535];
	size_t infolen;
	if(GetDBInfo(fd, outstream, info, sizeof(info), infolen) == -1)
	{
		AC_ERROR("NetProxy::GetLuckConf():read info error");
		return -1;
	}

	BinaryReadStream infostream(info,infolen);
	short rownum;
	if(!infostream.Read(rownum))
	{
		AC_ERROR("NetProxy::GetLuckConf():read row num error");
		return -1;
	}

	short colnum;
	if(!infostream.Read(colnum))
	{
		AC_ERROR("NetProxy::GetLuckConf():read col num error");
		return -1;
	}
	
	    AC_DEBUG("NetProxy::GetLuckConf():rownum = %d colnum = %d", rownum, colnum);
	char infobuf[1024];
	size_t curlen;

	int multiple=0;
	int probabitiy=0;
	for(int i = 0;i < rownum;i++)
	{		
		for(int j = 0;j < colnum;j++)
		{
			if(!infostream.Read(infobuf,sizeof(infobuf),curlen))
			{
				AC_ERROR("NetProxy::GetLuckConf():read ptr error");
				return -1;
			}
			infobuf[curlen] = 0;
			if(j == 0)	//中奖倍数
			{
				multiple= atoi(infobuf);
			}
			if(j == 1)//中奖概率
			{
				probabitiy= atoi(infobuf);
			}		
			if(j == 2)//概率基数
			{
				m_luckbase= atoi(infobuf);
			}
			
		}
		AC_DEBUG("NetProxy::GetLuckConf(): %d,%d,%d",multiple,probabitiy,m_luckbase);
		m_LuckConf.insert(make_pair(multiple,probabitiy));
	}
	return 0;
}


int NetProxy::GetRightConf()
{
	AC_DEBUG("NetProxy::GetRightConf():IN");
	/*
	int fd;
	if((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		AC_ERROR("NetProxy::GetRightConf():create socket error");
		return -1;
	}

	sockaddr_in addr;
	memset(&addr, 0, sizeof(sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(m_pDbagentRightCfg->m_ipmain.c_str());
	addr.sin_port = htons(m_pDbagentRightCfg->m_portmain);

	if(connect(fd, (sockaddr*)(&addr), sizeof(sockaddr_in)) == -1)
	{
		AC_ERROR("NetProxy::GetRightConf():connect maindbagent error");
		//return -1;
		
		sockaddr_in addrbak;
		memset(&addrbak, 0, sizeof(sockaddr_in));
		addrbak.sin_family = AF_INET;
		addrbak.sin_addr.s_addr = inet_addr(m_pDbagentCfg->m_ipbak.c_str());
		addrbak.sin_port = htons(m_pDbagentCfg->m_portbak);

		if(connect(fd, (sockaddr*)(&addrbak), sizeof(sockaddr_in)) == -1)
		{
			AC_ERROR("NetProxy::GetRoomInfomation():connect bakdbagent error");
			close(fd);
			return -1;
		}
		
		
	}
	
	int outseq = g_pNetProxy->GetCounter()->Get();
	BinaryWriteStream* outstream = StreamFactory::InstanceWriteStream();
	static const char* spname = {"DBMW_GetRightXML_HZSTAR"};//存储过程填充
	outstream->Write((short)CMD_CALLSP);
	outstream->Write(outseq);
	outstream->Write(spname,strlen(spname));	
	outstream->Write((short)0);
	outstream->Flush();


	char info[65535];
	size_t infolen;
	if(GetDBInfo(fd, outstream, info, sizeof(info), infolen) == -1)
	{
		AC_ERROR("NetProxy::GetRightConf:read info error");
		return -1;
	}

	BinaryReadStream infostream(info,infolen);
	short rownum;
	if(!infostream.Read(rownum))
	{
		AC_ERROR("NetProxy::GetRightConf:read row num error");
		return -1;
	}

	short colnum;
	if(!infostream.Read(colnum))
	{
		AC_ERROR("NetProxy::GetRightConf():read col num error");
		return -1;
	}
	
	AC_DEBUG("NetProxy::GetRightConf():rownum = %d colnum = %d", rownum, colnum);
	char infobuf[65535]={0};
	size_t curlen;

	for(int i=0;i<rownum;i++)
	{
		for(int j = 0;j < colnum;j++)
		{
			if(!infostream.Read(infobuf,sizeof(infobuf),curlen))
			{
				AC_ERROR("NetProxy::GetRightConf():read ptr error");
				return -1;
			}
			infobuf[curlen] = 0;
		}
	}

	if(strlen(infobuf)>0)
	{
		FILE *pXmlfile=NULL;
		size_t writelen=0;

		pXmlfile=fopen(g_rightpath,"a");
		if(pXmlfile==NULL)
		{
			AC_ERROR("NetProxy::GetRightConf():rightconfig xml file open failed,errno:%d\n",errno);
			return -1;
		}

		writelen=fwrite(infobuf,1,strlen(infobuf),pXmlfile);
		if(writelen!=strlen(infobuf))
		{
			AC_ERROR("NetProxy::GetRightConf():rightconfig xml file write failed,errno:%d\n",errno);
			return -1;
		}
		fflush(pXmlfile);
		fclose(pXmlfile);
	}
	*/
	//加载配置文件
	if(g_pNetProxy->LoadRightconfig(g_rightpath)==-1)
	{
		AC_ERROR("NetProxy::GetRightConf():LoadRightconfig error");
		return -1;
	}
	return 0;
}

//add by jinguanfu 2011/8/19
int NetProxy::GetChatBlacklist(int fd)
{

	g_pNetProxy->m_ChatBlacklist.clear();

	int outseq = g_pNetProxy->GetCounter()->Get();
	BinaryWriteStream* outstream = StreamFactory::InstanceWriteStream();
	static const char* spname = {"DBMW_GetChatBlacklist"};//存储过程填充
	outstream->Write((short)CMD_CALLSP);
	outstream->Write(outseq);
	outstream->Write(spname,strlen(spname));	
	outstream->Write((short)0);
	outstream->Flush();


	char info[65535];
	size_t infolen;
	if(GetDBInfo(fd, outstream, info, sizeof(info), infolen) == -1)
	{
		AC_ERROR("NetProxy::GetChatBlacklist:read info error");
		return -1;
	}

	BinaryReadStream infostream(info,infolen);
	short rownum;
	if(!infostream.Read(rownum))
	{
		AC_ERROR("NetProxy::GetChatBlacklist:read row num error");
		return -1;
	}

	short colnum;
	if(!infostream.Read(colnum))
	{
		AC_ERROR("NetProxy::GetChatBlacklist:read col num error");
		return -1;
	}
	
	AC_DEBUG("NetProxy::GetChatBlacklist:rownum = %d colnum = %d", rownum, colnum);
	char infobuf[1024];
	size_t curlen;

	int idx=0;
	for(int i = 0;i < rownum;i++)
	{
		for(int j = 0;j < colnum;j++)
		{
			if(!infostream.Read(infobuf,sizeof(infobuf),curlen))
			{
				AC_ERROR("NetProxy::GetChatBlacklist:read ptr error");
				return -1;
			}
			infobuf[curlen] = 0;

			if(j == colnum - 1)
			{
				idx= atoi(infobuf);
				g_pNetProxy->m_ChatBlacklist.insert(make_pair(idx, idx));
				AC_DEBUG("NetProxy::GetChatBlacklist:rownum = %d colnum = %d idx = %d",i,j,idx);
			}
		}
	}


	return 0;
}

//add by lihongwu 2011/9/20
int NetProxy::GetGiftConfig(int fd)
{
	int outseq = g_pNetProxy->GetCounter()->Get();
	BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
	static const char* spname = {"DBMW_GetGiftConfig"};//存储过程填充
	outstream->Write((short)CMD_CALLSP);
	outstream->Write(outseq);
	outstream->Write(spname,strlen(spname));
	outstream->Write((short)0);
	outstream->Flush();

	char info[65535];
	size_t infolen;
	if(GetDBInfo(fd, outstream, info, sizeof(info), infolen) == -1)
	{
		AC_ERROR("NetProxy::GetGiftConfig:read info error");
		return -1;
	}

	BinaryReadStream infostream(info,infolen);
	short rownum;
	if(!infostream.Read(rownum))
	{
		AC_ERROR("NetProxy::GetGiftConfig:read row num error");
		return -1;
	}

	short colnum;
	if(!infostream.Read(colnum))
	{
		AC_ERROR("NetProxy::GetGiftConfig:read col num error");
		return -1;
	}

	if (rownum <= 0 || colnum != 3)
	{
		AC_ERROR("NetProxy::GetGiftConfig:rownum=%d, colnum=%d error",rownum,colnum);
		return -1;
	}

	char infobuf[1024];
	size_t curlen;
	//add by lihongwu 2011/10/11
	int giftprice = 0;     //礼物可配置化价格配置
	int giftnum = 0;       //每次刷礼物个数
	int giftinterval = 0;  //刷礼物间隔时间

	for(int i=0;i<rownum;i++)
	{
		for(int j = 0;j < colnum;j++)
		{
			if(!infostream.Read(infobuf,sizeof(infobuf),curlen))
			{
				AC_ERROR("NetProxy::GetGiftConfig:read giftconfig error");
				return -1;
			}
			infobuf[curlen]=0;
			if (j == 0)
			{
				giftprice = atoi(infobuf);
			}
			if (j == 1)
			{
				giftnum = atoi(infobuf);
			}
			if (j == 2)
			{
				giftinterval = atoi(infobuf);
			}
		}
		GIFT_CONFIG giftconfig;
		giftconfig.price=giftprice;
		giftconfig.number=giftnum;
		giftconfig.interval=giftinterval;
		m_giftconflist.insert(make_pair(giftprice,giftconfig));
		AC_DEBUG("NetProxy::GetGiftConfig:price=%d,number=%d,interval=%d ",giftprice,giftnum,giftinterval);
	}

	//把礼物配置信息拼到礼物信息里 
	map<int,GIFT_CONFIG>::iterator itconfig = m_giftconflist.begin();
	map<int,GIFT_CONFIG>::iterator itconfig2 = itconfig;
	map<int,GIFT_INFO>::iterator itinfo = m_GiftInfo.begin();
	for (;itinfo != m_GiftInfo.end();itinfo++)
	{
		GIFT_INFO& giftinfo = itinfo->second;               //引用类型
		for (;itconfig != m_giftconflist.end();itconfig++)
		{
			itconfig2 = itconfig;
			itconfig2++;
			GIFT_CONFIG giftconf = itconfig->second;
			if (itconfig2 != m_giftconflist.end()) 
			{
				if (giftinfo.price>giftconf.price && giftinfo.price< itconfig2->first)  //价格在可配置区间内
				{
					giftinfo.priceconf = giftconf.price;
					giftinfo.number = giftconf.number;
					giftinfo.interval = giftconf.interval;
					break;
				}
			}
			else
			{
				giftinfo.priceconf = giftconf.price;
				giftinfo.number = giftconf.number;
				giftinfo.interval = giftconf.interval;
				break;
			} 
		}

		AC_DEBUG("NetProxy::GetGiftConfig: giftinfo..priceconf=%d,giftinfo.number =%d, giftinfo.interval = %d ",giftinfo.priceconf,giftinfo.number,giftinfo.interval);

	}
	AC_DEBUG("NetProxy::GetGiftConfig: giftinfo add giftconfig ");

	return 0;
}

