#include "netproxy.h"
#include "ac/log/log.h"
#include "ac/util/md5.h"
#include "ac/util/protocolstream.h"
#include "StreamFactory.h"

int MonitorDataDecoder::OnPackage(ClientSocketBase* pClient,const char* buf,size_t buflen)
{
	char ip[32];
	pClient->GetPeerIp(ip);

    BinaryReadStream instream(buf, buflen);
    short cmd;
    if(!instream.Read(cmd))
    {
        AC_ERROR("MonitorDataDecoder::OnPackage:read cmd error");
        return -1;
    }
    int seq;
    if(!instream.Read(seq))
    {
        AC_ERROR("MonitorDataDecoder::OnPackage:read seq error");
        return -1;
    }

	//AC_DEBUG("MonitorDataDecoder::OnPackage ip=%s cmd=%d,seq=%d",ip,cmd,seq);
    switch(cmd)
    {
	case MONITOR_CMD_REPLY:
		{
			BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
			outstream->Write((short)cmd);
			outstream->Write(seq);
			outstream->Write(g_pNetProxy->GetClientCount());		//房间服务器总在线人数
			outstream->Write((int)g_pNetProxy->m_roomlistinfo.roommap.size());//房间个数
			map<int, ROOM_INFO*>::iterator itroom = g_pNetProxy->m_roomlistinfo.roommap.begin();
			for(;itroom != g_pNetProxy->m_roomlistinfo.roommap.end();itroom++)
			{
				ROOM_INFO* proominfo=itroom->second;
				if(proominfo)
				{
					outstream->Write((int)proominfo->roomid);		//房间ID
					outstream->Write((int)proominfo->userlist.size());//房间内在线人数
				}
				else
				{
					AC_ERROR("proominfo == NULL");
				}
			}

			outstream->Flush();

			MonitorSvrClient* pMonitor=(MonitorSvrClient*)pClient;

			if(pMonitor->AddBuf(outstream->GetData(), outstream->GetSize())==-1)
			{
				AC_ERROR("MonitorDataDecoder::OnPackage: AddBuf error");
				return -1;
			}
		}
		break;		
	
    default:
        {            
            AC_ERROR("MonitorDataDecoder::OnPackage:cmd error");
            return -1;
        }
        break;
    }

	return 0;
}
