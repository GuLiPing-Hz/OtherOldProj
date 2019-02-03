#include "netproxy.h"
#include "ac/log/log.h"
#include "ac/util/md5.h"
#include "ac/util/protocolstream.h"
#include "StreamFactory.h"

int FactoryDataDecoder::OnPackage(ClientSocketBase* pClient,const char* buf,size_t buflen)
{
	char ip[32];
	pClient->GetPeerIp(ip);

    BinaryReadStream instream(buf, buflen);
    short cmd;
    if(!instream.Read(cmd))
    {
        AC_ERROR("FactoryDataDecoder::OnPackage:read cmd error");
        return -1;
    }
    int seq;
    if(!instream.Read(seq))
    {
        AC_ERROR("FactoryDataDecoder::OnPackage:read seq error");
        return -1;
    }

	//AC_DEBUG("MonitorDataDecoder::OnPackage ip=%s cmd=%d,seq=%d",ip,cmd,seq);
    switch(cmd)
    {
	case FACTORY_CMD_STAR_NUM_F2R:
		if(DoUpdateStarNum(cmd,seq,&instream)==-1)
		{
            AC_ERROR("FactoryDataDecoder::OnPackage:DoUpdateStarNum error");
            return -1;
		}
		break;		
	
    default:
        {            
            AC_ERROR("FactoryDataDecoder::OnPackage:cmd error");
            return -1;
        }
        break;
    }

	return 0;
}

int FactoryDataDecoder::DoUpdateStarNum(short cmd, int seq,BinaryReadStream *pinstream)
{

	AC_DEBUG("FactoryDataDecoder::DoUpdateStarNum:cmd=%d,seq=%d,pinstream=%x",cmd,seq,pinstream);

	int idx=0;
	if(!pinstream->Read(idx))
	{
		AC_ERROR("FactoryDataDecoder::DoUpdateStarNum:read idx error");
		return -1;
	}

	int roomid=0;
	if(!pinstream->Read(roomid))
	{
		AC_ERROR("FactoryDataDecoder::DoUpdateStarNum:read roomid error");
		return -1;
	}

	short starnum=0;
	if(!pinstream->Read(starnum))
	{
		AC_ERROR("FactoryDataDecoder::DoUpdateStarNum:read starnum error");
		return -1;
	}

	ROOM_INFO* proominfo=g_pNetProxy->GetRoomInfo(roomid);
	if(proominfo==NULL)
	{
		AC_ERROR("FactoryDataDecoder::DoUpdateStarNum:has no room[roomid=%d] ",roomid);
		return 0;
	}

	map<int,RoomClient*>::iterator itu=proominfo->userlist.find(idx);
	if(itu==proominfo->userlist.end())
	{
		AC_ERROR("FactoryDataDecoder::DoUpdateStarNum:user[idx=%d] not in room[roomid=%d] ",idx,roomid);
		return 0;
	}

	RoomClient* pRoomClient=(RoomClient*)itu->second;
	if(pRoomClient!=NULL)
	{
		pRoomClient->m_starnum=starnum;

		//通知客户端更新星光道具数

	}
	else
	{
		AC_ERROR("FactoryDataDecoder::DoUpdateStarNum:pRoomClient=%x ",pRoomClient);
		return 0;
	}

	return 0;
}




