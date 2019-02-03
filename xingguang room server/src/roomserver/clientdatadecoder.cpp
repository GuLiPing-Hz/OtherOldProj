#include "netproxy.h"
#include "ac/log/log.h"
#include "ac/util/md5.h"
#include "ac/util/protocolstream.h"
#include "ac/util/crypt.h"
#include "roomtype.h"
#include "roomtimeout.h"
#include "zlib.h"
#include "HallSvrClient.h"
#include "StreamFactory.h"
#include "pthread.h"
#include <time.h>
//目前暂时没用
MIC_INFO MicInfoManager::micinfo;
void MicInfoManager::Clean()
{
	/*micinfo.idx=0;micinfo.musicid=0;micinfo.bk=0;micinfo.state=MIC_INFO_WAIT; micinfo.score=0; micinfo.pkidx=0;micinfo.pkmusicid=0;
		micinfo.pkbk=0; micinfo.pkstate=MIC_INFO_WAIT; micinfo.score=0; micinfo.level=0;*/
	micinfo.init();
}

int ClientDataDecoder::OnPackage(ClientSocketBase *pClient,const char* buf,size_t buflen)
{
	char ip[32];
	pClient->GetPeerIp(ip);
	//AC_DEBUG("%s",ip);

	RoomClient* pRoomClient = (RoomClient*)pClient;

	//AC_DEBUG("token = %d",pRoomClient->m_btoken);
	if(!pRoomClient->m_btoken) //token认证过了吗
	{
		//解包头
		BinaryReadStream instream(buf,buflen);	
		short cmd;
		if(!instream.Read(cmd))
		{
			AC_ERROR("ClientDataDecoder::OnPackage:read cmd error nologin");
			return -1;
		}

		if(cmd != ROOM_CMD_TOKEN)
		{
			AC_ERROR("ClientDataDecoder::OnPackage:cmd error nologin,cmd=%d",cmd);
			return -1;
		}

		int seq;
		if(!instream.Read(seq))
		{
			AC_ERROR("ClientDataDecoder::OnPackage:read seq error nologin");
			return -1;
		}

		//应用处理
		if(DoToken(pClient, cmd, seq, &instream) == -1)
		{
			AC_ERROR("ClientDataDecoder::OnPackage:DoToken error nologin");
			return -1;
		}
	}
	else
	{
		if(pRoomClient->m_idx == 0 || strlen(pRoomClient->m_sessionkey) == 0)
		{
			AC_ERROR("ClientDataDecoder::OnPackage:pClient error");
			return -1;
		}

		//tea解密
		char outbuf[65535];
		int outlen = sizeof(outbuf);
		//int offset = 3;

		if(buf[2] == TEAFLAG)
		{
			/*chg by jinguanfu 2011/7/1
			if(!TEADecrypt((unsigned char*)(buf+offset), buflen-offset, (unsigned char*)(outbuf+2), &outlen, (unsigned char*)pRoomClient->m_sessionkey))
			{
				AC_ERROR("ClientDataDecoder::OnPackage:TEADecrypt error");
				return -1;
			}
			outlen += 2;
			*/
			if(!StreamDecrypt(buf,buflen,outbuf,outlen,pRoomClient->m_sessionkey))
			{
				AC_ERROR("ClientDataDecoder::OnPackage:cann't decrypt,idx=%d,ip = %s sessionkey=%s",pRoomClient->m_idx,ip,pRoomClient->m_sessionkey);
				return -1;
			}
			
		}
		else if(buf[2] == COMPRESSFLAG)
		{
			/*chg by jinguanfu 2011/7/1
			if(uncompress((unsigned char*)(outbuf+2), (unsigned long*)&outlen, (unsigned char*)(buf+offset),
				(unsigned long)(buflen-offset)) != 0)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:uncompress error");
				return -1;
			}
			outlen += 2;
			*/
			if(!StreamUnCompress(buf,buflen,outbuf,outlen))
			{
				AC_ERROR("ClientDataDecoder::OnPackage:cann't uncompress,idx=%d,ip = %s",pRoomClient->m_idx,ip);
				return -1;
			}
		}
		else
		{
			AC_ERROR("ClientDataDecoder::OnPackage:flag error = %d", buf[2]);
			return -1;
		}
 	
		if(outlen <= 0 || outlen > (int)sizeof(outbuf))
		{
			AC_ERROR("ClientDataDecoder::OnPackage:Decrypt outlen error");
			return -1;
		}

		//解包头
		BinaryReadStream instream(outbuf, outlen);	
		//add by jinguanfu 2011/7/1 加密/压缩标志位
		char c;
		if(!instream.Read(c))
		{
			AC_ERROR("ClientDataDecoder::OnPackage:read FLAG error");
			return -1;
		}
		
		short cmd;
		if(!instream.Read(cmd))
		{
			AC_ERROR("ClientDataDecoder::OnPackage:read cmd error");
			return -1;
		}

		int seq;
		if(!instream.Read(seq))
		{
			AC_ERROR("ClientDataDecoder::OnPackage:read seq error");
			return -1;
		}

		//应用处理
		switch(cmd)
		{
		case ROOM_CMD_LOGIN:
			if(DoLogin(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoLogin error");
				return -1;
			}
			break;
		case ROOM_CMD_KEEPALIVE:
			if(DoKeepAlive(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoKeepAlive error");
				return -1;
			}
			break;
		case ROOM_CMD_LEAVEROOM:
			if(DoLeaveRoom(pClient) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoLeaveRoom error");
				return -1;
			}
			break;
		case ROOM_CMD_GET_ALLINFO:
			if(DoGetAllInfo(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoGetAllInfo error");
				return -1;
			}
			break;
		case ROOM_CMD_WAITMIC:
			if(DoWaitMic(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoWaitMic error");
				return -1;
			}
			break;
		case ROOM_CMD_CANCLE_WAITMIC:
			if(DoCancleWaitMic(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoCancleWaitMic error");
				return -1;
			}
			break;
		case ROOM_CMD_WAITMIC_PK:
			if(DoWaitMicPK(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoWaitMicPK error");
				return -1;
			}
			break;
		case ROOM_CMD_CANCLE_WAITMIC_PK:
			if(DoCancleWaitMicPK(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoCancleWaitMicPK error");
				return -1;
			}
			break;
		case ROOM_CMD_UP_WAITMIC:
			if(DoUpWaitMic(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoUpWaitMic error");
				return -1;
			}
			break;
		case ROOM_CMD_DOWN_WAITMIC:
			if(DoDownWaitMic(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoDownWaitMic error");
				return -1;
			}
			break;
		
		case ROOM_CMD_ONMIC_READYOK:
			if(DoOnMicReadyOK(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoGetMicReadyOK error");
				return -1;
			}
			break;
		case ROOM_CMD_ADD_TM:
			if(DoOnMicAddTM(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoOnMicAddTM error");
				return -1;
			}
			break;
		/*	
		case ROOM_CMD_OFFMIC_READY:
			if(DoOffMicReady(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoOffMicReady error");
				return -1;
			}
			break;
		*/
		case ROOM_CMD_SCORE:
			if(DoScore(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoScore error");
				return -1;
			}
			break;
		case ROOM_CMD_KICKOUT_SOMEONE:
			if(DoKickSb(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoKickOutSb error");
				return -1;
			}
			break;
		case ROOM_CMD_FORBIDEN_SOMEONE:
			if(DoForbidenSb(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoForbidenSb error");
				return -1;
			}
			break;
		case ROOM_CMD_UPDATE_BLACKLIST:
			if(DoUpdateBlacklist(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoUpdateBlacklist error");
				return -1;
			}
			break;
		/*  delete by jinguanfu 2010/8/27
		case ROOM_CMD_UPDATE_MGRLIST:
			if(DoUpdateMgr(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoUpdateMgr error");
				return -1;
			}
			break;
		case ROOM_CMD_PRIVATE_CHAT:
			if(DoPrivateChat(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoPrivateChat error");
				return -1;
			}
			break;
		*/
		case ROOM_CMD_PUBLIC_CHAT:
			if(DoPublicChat(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoPublicChat error");
				return -1;
			}
			break;
		case ROOM_CMD_ONVJ_MIC:
			if(DoOnVJMic(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoOnVJMic error");
				return -1;
			}
			break;
		case ROOM_CMD_OFFVJ_MIC:
			if(DoOffVJMic(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoOffVJMic error");
				return -1;
			}
			break;
		case ROOM_CMD_GIVE_VJ_MIC:
			if(DoGiveVJMic(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoGiveVJMic error");
				return -1;
			}
			break;
		case ROOM_CMD_GIVEOFF_VJ_MIC:
			if(DoGiveOffVJMic(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoGiveOffVJMic error");
				return -1;
			}
			break;
		case ROOM_CMD_GIVEOFF_MIC:
			if(DoGiveOffMic(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoGiveOffMic error");
				return -1;
			}
			break;
		case ROOM_CMD_UPDATE_CONTENT:
			if(DoUpdateContent(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoUpdateContent error");
				return -1;
			}
			break;
		case ROOM_CMD_SEND_GIFT:
			if(DoSendFlower(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoSendFlower error");
				return -1;
			}
			break;
		case ROOM_CMD_SEND_NOTICE_TEMP:
			if(DoSendNotice(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoSendNotice error");
				return -1;
			}
			break;
		case ROOM_CMD_INVITE_MIC:
			if(DoInviteMic(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoInviteMic error");
				return -1;
			}
			break;			
		/*case ROOM_CMD_BROAD_ALL:
			if(DoBroad(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoSendFlower error");
				return -1;
			}
			break;
		case ROOM_CMD_BROAD_LOBBY:
			if(DoBroad(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoSendFlower error");
				return -1;
			}
			break;*/
		case ROOM_CMD_USER_APP_JOINROOM:
			 if(DoJoinRoomApp(pClient,  cmd, seq) == -1)
			 {
				 AC_ERROR("ClientDataDecoder::OnPackage:DoJoinRoomApp error");
				 return -1;
			 }
			 break;
		case ROOM_CMD_VERIFY_USER_APP:
			 if(DoVerifyUserApp(pClient,  cmd, seq, &instream) == -1)
			 {
				 AC_ERROR("ClientDataDecoder::OnPackage:DoVerifyUserApp error");
				 return -1;
			 }
			 break;
		case ROOM_CMD_REMOVE_USER:
			if(DoRemoveUser(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoRemoveUser error");
				return -1;
			}
			break;
		case ROOM_CMD_GIVE_VJ_A:
			if(DoGiveVJA(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoGiveVJA error");
				return -1;
			}
			break;
		case ROOM_CMD_GIVE_VJ:
			if(DoGiveVJ(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoGiveVJ error");
				return -1;
			}
			break;
		case ROOM_CMD_GIVE_OUER_S:
			if(DoGiveOwnS(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoGiveOwnS error");
				return -1;
			}
			break;		
		case ROOM_CMD_SET_ROOM_PWD:
			if(DoSetRoomPwd(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoSetRoomPwd error");
				return -1;
			}
			break;
		case ROOM_CMD_SET_ROOM_LOCK:
			if(DoSetRoomLock(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoSetRoomLock error");
				return -1;
			}
			break;
		case ROOM_CMD_SET_USER_ONLY:
			if(DoSetUserOnly(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoSetUserOnly error");
				return -1;
			}
			break;
		case ROOM_CMD_SET_USER_INOUT:
			if(DoSetUserInOut(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoSetUserInOut error");
				return -1;
			}
			break;
		case ROOM_CMD_SET_MIC_UPDOWN:
			if(DoSetMicUpDown(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoSetMicUpDown error");
				return -1;
			}
			break;
		case ROOM_CMD_SET_ROOM_NAME:
			if(DoSetRoomName(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoSetRoomName error");
				return -1;
			}
			break;
		case ROOM_CMD_SET_CHAT_PUBLIC:
			if(DoSetChatPublic(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoSetChatPublic error");
				return -1;
			}
			break;
		case ROOM_CMD_SET_ROOM_WELCOME:
			if(DoSetRoomWelcome(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoSetRoomWelcome error");
				return -1;
			}
			break;
		case ROOM_CMD_SET_ROOM_LOGO:
			if(DoSetRoomLogo(pClient,  cmd, seq, &instream) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoSetRoomLogo error");
				return -1;
			}
			break;
		case ROOM_CMD_ROOMAPPLYLIST_C2R2C:
			if(DoReturnRoomApplyList(pClient,  cmd, seq) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoReturnRoomApplyList error");
				return -1;
			}
			break;	
		case ROOM_CMD_GETBLACKLIST:
			if(DoReturnRoomBlackList(pClient,  cmd, seq) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoReturnRoomBlackList error");
				return -1;
			}
			break;	
		case ROOM_CMD_GETROOMMEMBERLIST:
			if(DoReturnRoomMemberList(pClient,  cmd, seq) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoReturnRoomMemberList error");
				return -1;
			}
			break;	
		case ROOM_CMD_EXITMEMBER:
			if(DoExitRoomMember(pClient,  cmd,  seq) == -1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoExitRoomMember error");
				return -1;
			}
			break;
		case ROOM_CMD_GETPANELINFO:
			if(DoGetPanelInfo(pClient, cmd, seq)==-1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoGetPanelInfo error");
				return -1;
			}
			break;
		case ROOM_CMD_GIVE_MEMBER:	
			if(DoGiveMember(pClient, cmd, seq,   &instream)==-1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoGiveMember error");
				return -1;
			}
			break;
		case ROOM_CMD_REQUEST_GIVE_VJ_MIC:
			if(DoGiveVJMicRes(pClient, cmd, seq, &instream)==-1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoGiveVJMicRes error");
				return -1;
			}
			break;
		case ROOM_CMD_SEND_FIREWORKS:
			if(DoSendFireworks(pClient, cmd, seq, &instream)==-1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoSendFireworks error");
				return -1;
			}
			break;
		case ROOM_CMD_VIEW_INCOME:
			if(DoViewIncome(pClient,cmd, seq,&instream)==-1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoViewIncome error");
				return -1;
			}
			break;
		case ROOM_CMD_GET_INCOME:
			if(DoGetIncome(pClient, cmd, seq,  &instream)==-1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoGetIncome error");
				return -1;
			}
			break;
		case ROOM_CMD_VIEW_INCOME_LOG:
			if(DoViewIncomeLog(pClient, cmd, seq,  &instream)==-1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoViewIncomeLog error");
				return -1;
			}
			break;		
		case ROOM_CMD_UP_NETSTATUS:
			if(DoBroadCastNetstatus(pClient, cmd, seq,  &instream)==-1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoBroadCastNetstatus error");
				return -1;
			}
			break;
		case ROOM_CMD_INVITE_MEMBER:
			if(DoInviteMember(pClient, cmd, seq,  &instream)==-1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoInviteMember error");
				return -1;
			}
			break;
		case ROOM_CMD_INVITE_REPLY:
			if(DoReplyInvite(pClient, cmd, seq,  &instream)==-1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoReplyInvite error");
				return -1;
			}

			break;
		case ROOM_CMD_GET_GIFTSEND:
			if(DoGetGiftSend(pClient, cmd, seq)==-1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoGetGiftSend error");
				return -1;

			}
			break;
		case ROOM_CMD_GET_GIFTRECV:
			if(DoGetGiftRecv(pClient, cmd, seq)==-1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoGetGiftRecv error");
				return -1;

			}
			break;
		case ROOM_CMD_SET_AUTOONMIC:
			if(DoSetRoomAutoOnmic(pClient, cmd, seq, &instream)==-1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoSetRoomAutoOnmic error");
				return -1;

			}
			break;
		case ROOM_CMD_DISABLE_IPADDR:
			if(DoDisableIP(pClient, cmd, seq, &instream)==-1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoDisableIP error");
				return -1;

			}
			break;
		case ROOM_CMD_DISABLE_MACADDR:
			if(DoDisableMAC(pClient, cmd, seq, &instream)==-1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoDisableMAC error");
				return -1;

			}
			break;
		case ROOM_CMD_UPDATE_MUSICTIME:
			if(DoUpdateMusicTime(pClient, cmd, seq, &instream)==-1)
			{
				AC_ERROR("ClientDataDecoder::OnPackage:DoUpdateMusicTime error");
				return -1;
			}
			break;
		default:
			{
				AC_ERROR("ClientDataDecoder::OnPackage:cmd error tokenok,cmd=%d",cmd);
				return -1;
			}
			break;
		}
	}

	return 0;
}

int ClientDataDecoder::SendToHall(ClientSocketBase *pClient, short cmd,ROOM_INFO* pRoominfo)
{
	//return 0;//目前暂时不用
	RoomClient* pRoomClient = (RoomClient*)pClient;
	//char outbuf[256]128] = {0};
	BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
	//BinaryWriteStream outstream(outbuf, sizeof(outbuf));
	outstream->Write(cmd);
	int seq = g_pNetProxy->GetCounter()->Get();
	outstream->Write(seq);
	outstream->Write((char)ROOM_TYPE_ROOM);	//ADD BY JINGUANFU 2010/7/9
	outstream->Write((int)pRoomClient->m_idx);
	outstream->Write((int)pRoomClient->m_roomid);	
	outstream->Write((short)pRoominfo->userlistFemale.size());
	outstream->Write((short)pRoominfo->userlistMale.size());
	//add by jinguanfu 2011/6/30
	outstream->Write((short)pRoominfo->userlist.size());		//房间真实用户数
	outstream->Flush();

	HallSvrClient *pHallSvr = g_pNetProxy->GetHallSvr();
	if(pHallSvr!=NULL)
		pHallSvr->AddBuf(outstream->GetData(), outstream->GetSize());
	else
		AC_ERROR("ClientDataDecoder::SendToHall pHallSvr is NULL");

	AC_DEBUG("ClientDataDecoder::SendToHall:roomid=%d,usernum=%d",pRoominfo->roomid,pRoominfo->userlist.size());
	
	return 0;
}

/*
int ClientDataDecoder::SendToFactory(ClientSocketBase *pClient, short cmd)
{
	RoomClient* pRoomClient = (RoomClient*)pClient;
	BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();

	outstream->Write(cmd);
	outstream->Write(0);
	outstream->Write(pRoomClient->m_idx); //用户idx
	outstream->Write(pRoomClient->m_roomid); //所在房间ID
	outstream->Flush();

	
	BackClientSocketBase *pFactocySvr = g_pNetProxy->GetFactoryServer();
	if(pFactocySvr!=NULL)
	{
		if(pFactocySvr->AddBuf(outstream->GetData(), outstream->GetSize())==-1) 
		{
			AC_ERROR("ClientDataDecoder::SendToFactory:pFactocySvr->AddBuf Error");
		}else
		{
			AC_DEBUG("ClientDataDecoder::SendToFactory:Send to pFactocySvr success,idx=%d roomid=%d ",pRoomClient->m_idx,pRoomClient->m_roomid);
		}
			
	}
	else
	{
		AC_ERROR("ClientDataDecoder::SendToFactory:pFactocySvr is NULL");
	}
	
	
	return 0;
}
*/
int ClientDataDecoder::SendToLogServer(ClientSocketBase *pClient, short cmd,int idx,int roomid)
{
	RoomClient* pRoomClient = (RoomClient*)pClient;
	BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();

	outstream->Write(cmd);
	outstream->Write(0);
	outstream->Write(idx); 		//用户idx
	outstream->Write(roomid); 	//所在房间ID
	outstream->Flush();
		
	BackClientSocketBase *pLogSvr = g_pNetProxy->GetLogServer();
	if(pLogSvr!=NULL)
	{
		if(pLogSvr->AddBuf(outstream->GetData(), outstream->GetSize())==-1) 
		{
			AC_ERROR("ClientDataDecoder::SendToLogServer:pLogSvr->AddBuf Error");
		}else
		{
			AC_DEBUG("ClientDataDecoder::SendToLogServer:Send to pLogSvr success,idx=%d roomid=%d ",pRoomClient->m_idx,pRoomClient->m_roomid);
		}
				
	}
	else
	{
		AC_ERROR("ClientDataDecoder::SendToLogServer:pLogSvr is NULL");
	}

	return 0;
}

int ClientDataDecoder::DoToken(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoToken : cmd=%d,seq=%d, fd = %d",cmd,seq,pClient->GetFD());
	//读取包体信息
	char token[256] = {0};
	size_t tokenlen = 0;
	if(!pinstream->Read(token, sizeof(token), tokenlen))
	{
		AC_ERROR("ClientDataDecoder::DoToken:read token error");
		return -1;
	}

	if(tokenlen <= 0)
	{
		AC_ERROR("ClientDataDecoder::DoToken:tokenlen error");
		return -1;
	}

	char version[20] = {0};
	size_t versionlen = 0;
	if(!pinstream->Read(version, sizeof(version), versionlen))
	{
		AC_ERROR("ClientDataDecoder::DoToken:read version error");
		return -1;
	}

	if(versionlen <= 0)
	{
		AC_ERROR("ClientDataDecoder::DoToken:versionlen error");
		return -1;
	}
	version[versionlen] = 0; // add by wangpf 2010/07/30
	g_pNetProxy->m_version = atof(version);//atoi(version);

	//解析token
	char inbuf[256] = {0};
	int inlen = 0;
	if(!(inlen = CXTEA::Decrypt(token, tokenlen, inbuf, sizeof(inbuf)-1, g_pNetProxy->m_ServerKey)))
	{
		AC_ERROR("ClientDataDecoder::DoToken:Decrypt error");
		return -1;
	}

	if(inlen >= 256 || strlen(inbuf) == 0) // modify by wangpf 2010/07/30  
	{
		AC_ERROR("ClientDataDecoder::DoToken:token error inbuf empty");
		return -1;
	}

	const char* sfind = "&";
	int idx = 0; 
	char id[64] = {0};
	int ntime = 0;
	char sessionkey[17] = {0};
	char* sp = inbuf;
	char* sp1 = inbuf;
	char* p = strstr(sp, sfind);
	if(p == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoLogin:token error");
		return -1;
	}
	int count = 0;
	while(p != NULL)
	{
		if((count == 0) && (p-sp) < 64) // modify by wangpf 2010/07/30
		{
			char value[64] = {0};
			memcpy(value, sp, p-sp);
			idx  = atoi(value);
		}

		if((count == 1) && (p-sp) < 64) // modify by wangpf 2010/07/30
			memcpy(id, sp, p-sp);

		if((count == 2) && (p-sp) < 64) // modify by wangpf 2010/07/30
		{
			char value[64] = {0};
			memcpy(value, sp, p-sp);
			ntime  = atoi(value);
		}

		sp = p + strlen(sfind);
		p = strstr(sp, sfind);
		count ++;
	}

	if(count != 3)
	{
		AC_ERROR("ClientDataDecoder::DoLogin:token error");
		return -1;
	}

	int session_len = strlen(inbuf)+sp1-sp; // modify by wangpf 2010/07/30
	if( (count == 3) && session_len < 17)
		memcpy(sessionkey, sp, session_len);

	int ret = TOKENLOGIN_SUCCESS;	//add by jinguanfu 2011/9/27
	//验证token的合法性
	if(idx <= 0 || strlen(id) == 0 || ntime == 0 || strlen(sessionkey) == 0)
	{
		AC_ERROR("ClientDataDecoder::DoToken:user error,idx=%d,id=%s ,ntime=%d,sessionkey=%s",idx,id,ntime,sessionkey);
		return -1;
	}
	
	RoomClient* pRoomClient = (RoomClient*)pClient;
	
	do{
		//add by jinguanfu 2011/6/21
		//游客号不检查token超时
		if(!(idx>=900000000&&idx<901000000))
		{
			int timeout = 20*60 ; //20min
			int nowtime = (int)time(NULL);
			if(nowtime - ntime > timeout)
			{
				AC_ERROR("ClientDataDecoder::DoToken:timeout error,idx=%d",idx);
				ret = TOKENLOGIN_TIMEOUT;
				break;
			}
		}
		
		LobbyTimeout* pTime=g_pNetProxy->GetLobbyTMMap(idx);
		if(pTime!=NULL)
		{
			AC_ERROR("ClientDataDecoder::DoToken: idx=%d  is DoTokening",idx);
			return -1;
		}

		//向大厅取该用户信息
		HallSvrClient *pHallSvr = g_pNetProxy->GetHallSvr();
		if(pHallSvr!=NULL)
		{
			int outseq=g_pNetProxy->GetCounter()->Get();
			BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
			outstream->Write((short)HALL_CMD_GETUSERINFO);
			outstream->Write(outseq);
			outstream->Write((char)ROOM_TYPE_ROOM);	//ADD BY JINGUANFU 2010/7/9
			outstream->Write(idx);
			outstream->Flush();

			LobbyTimeout* pLobbytimeout=g_pNetProxy->CreateLobbyTimeout();
			if(pLobbytimeout!=NULL)
			{
				if(pHallSvr->AddBuf(outstream->GetData(), outstream->GetSize())==-1)
				{
					AC_ERROR("ClientDataDecoder::DoToken: AddBuf error");
					g_pNetProxy->DestroyLobbyTimeout(pLobbytimeout);
					return -1;
				}
			}
			else
			{
				AC_ERROR("ClientDataDecoder::DoToken: CreateLobbyTimeout error pLobbytimeout=%x",pLobbytimeout);
				return -1;
			}
			
			//向大厅取用户信息	
			
			pLobbytimeout->idx = idx;	
			pLobbytimeout->cmd = cmd;
			pLobbytimeout->seq=seq;
			pLobbytimeout->outseq=outseq;
			memset(pLobbytimeout->sessionkey,0,sizeof(pLobbytimeout->sessionkey));
			strncpy(pLobbytimeout->sessionkey,sessionkey,strlen(sessionkey));
			pLobbytimeout->clientID= pRoomClient->GetClientID();
			pLobbytimeout->SetReactor(g_pNetProxy->GetReactor());
			pLobbytimeout->RegisterTimer(3);	//超时3秒
			g_pNetProxy->m_LobbyTMMap.insert(make_pair(outseq, pLobbytimeout));
			//g_pNetProxy->m_LobbyTMMap.insert(make_pair(idx, pLobbytimeout));

			AC_DEBUG("ClientDataDecoder::DoToken:sessionkey=%s ",sessionkey);
			pRoomClient->m_idx = idx;
			strcpy(pRoomClient->m_sessionkey, sessionkey);
			pRoomClient->m_btoken = 0;


			return 0;
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoToken: GetHallSvr is NULL pHallSvr=%x",pHallSvr);
			return -1;
		}
	}while(0);

	//add by jinguanfu 2011/9/27
	BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
	char type = 65;
	outstream->Write(type);
	outstream->Write(cmd);
	outstream->Write(seq);
	outstream->Write(ret); 	
	outstream->Flush();

	if(g_pNetProxy->SendToSrv(pClient, *outstream)==-1)
	{
		AC_ERROR("ClientDataDecoder::DoToken: idx=%d,SendTo client error",idx);
		return -1;
	}

	
	return 0;
}

/*
int ClientDataDecoder::DoLogin(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoLogin fd = %d", pClient->GetFD());
	RoomClient* pRoomClient = (RoomClient*)pClient;
	//读取包体信息
	int roomid = 0;
	if(!pinstream->Read(roomid))
	{
		AC_ERROR("ClientDataDecoder::DoLogin:read roomid error");
		return -1;
	}

	if(roomid <= 0)
	{
		AC_ERROR("ClientDataDecoder::DoLogin:roomid error");
		return -1;
	}

	char passwd[64] = {0};
	size_t passwdlen = 0;
	if(!pinstream->Read(passwd, sizeof(passwd)-1, passwdlen))
	{
		AC_ERROR("ClientDataDecoder::DoLogin:read passwd error");
		return -1;
	}
	passwd[passwdlen]=0;

	AC_DEBUG("ClientDataDecoder::DoLogin:roomid = %d passwd = %s", roomid, passwd);

	LobbyTimeout* pTime=g_pNetProxy->GetLobbyTMMap(pRoomClient->m_idx);
	if(pTime!=NULL)
	{
		AC_ERROR("ClientDataDecoder::DoLogin: idx=%d is DoLogining ",pRoomClient->m_idx);
		return -1;
	}
	

	HallSvrClient *pHallSvr = g_pNetProxy->GetHallSvr();
	if(pHallSvr==NULL)
	{
		AC_ERROR("ClientDataDecoder::DoLogin:pHallSvr is NULL");
		return -1;
	}
	
	LobbyTimeout* pLobbytimeout=g_pNetProxy->CreateLobbyTimeout();
	if(pLobbytimeout!=NULL)
	{
		int outseq=g_pNetProxy->GetCounter()->Get();
		
		BinaryWriteStream* lobbystream=StreamFactory::InstanceWriteStream();
		lobbystream->Write((short)HALL_CMD_LOGIN);
		lobbystream->Write(outseq);
		lobbystream->Write(pRoomClient->m_idx);
		lobbystream->Flush();

		
		if(pHallSvr->AddBuf(lobbystream->GetData(), lobbystream->GetSize())==-1)
		{
			AC_ERROR("ClientDataDecoder::DoToken: AddBuf error");
			g_pNetProxy->DestroyLobbyTimeout(pLobbytimeout);
			return -1;
		}
		
		
		pLobbytimeout->init();
		pLobbytimeout->clientID=pRoomClient->GetClientID();
		pLobbytimeout->idx=pRoomClient->m_idx;
		pLobbytimeout->cmd=cmd;
		pLobbytimeout->seq=seq;
		pLobbytimeout->roomid=roomid;
		pLobbytimeout->outseq=outseq;
		if(strlen(passwd)>0)
			strcpy(pLobbytimeout->enterpwd,passwd);
		
		pLobbytimeout->SetReactor(g_pNetProxy->GetReactor());
		pLobbytimeout->RegisterTimer(5);	//超时3秒
		//g_pNetProxy->m_LobbyTMMap.insert(make_pair(outseq, pLobbytimeout));
		g_pNetProxy->m_LobbyTMMap.insert(make_pair(pLobbytimeout->idx, pLobbytimeout));
		
	}
	else
	{
		AC_ERROR("ClientDataDecoder::DoLogin:CreateLobbyTimeout error");
		return -1;
	}



	
	return 0;
	
}
*/

int ClientDataDecoder::DoLogin(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoLogin fd = %d proomclient = %x", pClient->GetFD(), pClient);
	RoomClient* pRoomClient = (RoomClient*)pClient;
	//读取包体信息
	int roomid = 0;
	if(!pinstream->Read(roomid))
	{
		AC_ERROR("ClientDataDecoder::DoLogin:read roomid error");
		return -1;
	}

	if(roomid <= 0)
	{
		AC_ERROR("ClientDataDecoder::DoLogin:roomid error");
		return -1;
	}

	char passwd[64] = {0};
	size_t passwdlen = 0;
	if(!pinstream->Read(passwd, sizeof(passwd)-1, passwdlen))
	{
		AC_ERROR("ClientDataDecoder::DoLogin:read passwd error");
		return -1;
	}
	passwd[passwdlen] = 0;

	//是否自动重连
	short blogin_again = 0;
	if(!pinstream->Read(blogin_again))
	{
		AC_ERROR("ClientDataDecoder::DoLogin:read blogin_again error");
		return -1;
	}

	AC_DEBUG("ClientDataDecoder::DoLogin:roomid = %d passwd = %s blogin_again = %d", roomid, passwd, blogin_again);

	//处理登录逻辑
	char ret = LOGIN_NONE;
		
	do{
		if(!blogin_again) //用户正常手动登陆
		{
			//查找所有的房间，是否有登陆过
			map<int, ROOM_INFO*>::iterator it = g_pNetProxy->m_roomlistinfo.roommap.begin(); 
			for(; it != g_pNetProxy->m_roomlistinfo.roommap.end(); it++)
			{
				ROOM_INFO* proominfo = it->second;
				if(proominfo)
				{
					map<int, RoomClient*>::iterator itu = proominfo->userlist.find(pRoomClient->m_idx);
					//找到了，有重复登陆的
					if(itu != proominfo->userlist.end())
					{
						ret = LOGIN_AGAIN;
						AC_DEBUG("login agin roomid = %d idx = %d", proominfo->roomid, pRoomClient->m_idx);
						//登陆失败回包给用户
						BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
						char type = 65;
						outstream->Write(type);
						outstream->Write(cmd);
						outstream->Write(seq);
						outstream->Write(ret);
						outstream->Flush();
						g_pNetProxy->SendToSrv(pClient,*outstream, 1, 1);
						return -1;
					}
				}
			}
		}
		else //房间掉线后自动重连
		{
			ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(roomid);
			if(proominfo == NULL)
			{
				AC_ERROR("ClientDataDecoder::DoLogin:roomid=%d not exist",roomid);
				ret = LOGIN_NOTEXIST;
				break;
			}
			map<int, RoomClient*>::iterator itu = proominfo->userlist.find(pRoomClient->m_idx);
			//找到了，有重复登陆的
			if(itu != proominfo->userlist.end())
			{
				RoomClient* pOldRoomClient = itu->second;
				if(pOldRoomClient && pOldRoomClient != pRoomClient)
					pOldRoomClient->Close(); //踢掉老的客户端
			}
		}
	
		ROOM_INFO* proominfo=g_pNetProxy->GetRoomInfo(roomid);
		if(proominfo==NULL)
		{
			AC_ERROR("ClientDataDecoder::DoLogin:roomid=%d not exist",roomid);
			ret = LOGIN_NOTEXIST;
			break;
		}

		//房间冻结
		if(proominfo->state == 1)
		{
			ret = LOGIN_LOCKED;
			break;
		}

		map<int, int>::iterator itb = proominfo->blacklist.find(pRoomClient->m_idx);
		if(itb != proominfo->blacklist.end())
		{
			ret = LOGIN_INBLACK;
			break;
		}
		

		map<int, int>::iterator ittb = proominfo->tblacklist.find(pRoomClient->m_idx);
		if(ittb != proominfo->tblacklist.end())
		{
			ret = LOGIN_INTBLACK;
			break;
		}
		
		map<int,int>::iterator itvj=proominfo->vjlist.find(pRoomClient->m_idx);
		map<int,int>::iterator itvj_a=proominfo->vjlist_a.find(pRoomClient->m_idx);
		map<int,int>::iterator itvip=proominfo->userlistVIP.find(pRoomClient->m_idx);
		map<int,GM_INFO>::iterator itmGM=g_pNetProxy->m_GM.find(pRoomClient->m_idx);

		if(pRoomClient->m_idx == proominfo->ownidx)
				pRoomClient->m_identity = USER_ID_OWNER;
		else if(pRoomClient->m_idx == proominfo->secondownidx || pRoomClient->m_idx == proominfo->secondownidx2)
				pRoomClient->m_identity = USER_ID_OWNER_S;
		else if(itvj != proominfo->vjlist.end())
				pRoomClient->m_identity = USER_ID_VJ;
		else if(itvj_a != proominfo->vjlist_a.end())
				pRoomClient->m_identity = USER_ID_VJ_A;
		else if(itvip != proominfo->userlistVIP.end())
				pRoomClient->m_identity = USER_ID_VIP;
		else if(itmGM !=g_pNetProxy->m_GM.end())
				pRoomClient->m_identity = USER_ID_GM;
		else 
				pRoomClient->m_identity = USER_ID_NONE;

		//add by jinguanfu 2011/5/20 增加游客身份
		if(pRoomClient->m_viplevel==USER_ID_GUEST)
			pRoomClient->m_viptype=USER_ID_GUEST;
		else if(pRoomClient->m_viplevel==VIP_LEVEL_NONE)
			pRoomClient->m_viptype=USER_ID_NONE;
		else if(pRoomClient->m_viplevel==VIP_LEVEL_VIP||pRoomClient->m_viplevel==VIP_LEVEL_LIFEVIP)
			pRoomClient->m_viptype=USER_ID_GVIP;
		else if((pRoomClient->m_viplevel%VIP_LEVEL_IMPERIAL)<60)
		{
			//皇冠过期，权限等同于VIP
			if(pRoomClient->m_vipflag==1)
				pRoomClient->m_viptype=USER_ID_IMPERIAL;
			else
				pRoomClient->m_viptype=USER_ID_GVIP;
		}
		else if((pRoomClient->m_viplevel%VIP_LEVEL_SUPER)<60)
		{
			//皇冠过期，权限等同于VIP
			if(pRoomClient->m_vipflag==1)
				pRoomClient->m_viptype=USER_ID_SUPER;
			else
				pRoomClient->m_viptype=USER_ID_GVIP;
		}
		else if((pRoomClient->m_viplevel%VIP_LEVEL_PURPLE)<60)
		{
			//皇冠过期，权限等同于VIP
			if(pRoomClient->m_vipflag==1)
				pRoomClient->m_viptype=USER_ID_PURPLE;
			else
				pRoomClient->m_viptype=USER_ID_GVIP;
		}
		else if((pRoomClient->m_viplevel%VIP_LEVEL_RED)<60)
		{
			//皇冠过期，权限等同于VIP
			if(pRoomClient->m_vipflag==1)
				pRoomClient->m_viptype=USER_ID_RED;
			else
				pRoomClient->m_viptype=USER_ID_GVIP;
		}
		else 
			pRoomClient->m_viptype=USER_ID_NONE;


		//副室主,室主,GM可以进入关闭的房间
		//if (LoginCheck(pRoomClient->m_identity,pRoomClient->m_viptype, 1))
		if (LoginCheckEx(pRoomClient->m_identity,pRoomClient->m_viptype, 1))
		{

        } 
		else if(proominfo->isClose)
		{
			ret = LOGIN_CLOSE;
			break;
		}
			

		//房间已到最大人数
		int usercount=proominfo->userlist.size()+proominfo->vuserlist.size();
		if(usercount >= proominfo->maxhuman)
		{
			ret = LOGIN_FULL;
			break;
		}

		//房间到最大允许人数
		//if (LoginCheck(pRoomClient->m_identity,pRoomClient->m_viptype, 3))
		if (LoginCheckEx(pRoomClient->m_identity,pRoomClient->m_viptype, 3))
		{

        } 
		else if(usercount  >= proominfo->allowhuman)
		{
			ret = LOGIN_IFULL;
			break;
		}

		//进入仅对会员开放房间
		//if (LoginCheck(pRoomClient->m_identity,pRoomClient->m_viptype, 2))
		if (LoginCheckEx(pRoomClient->m_identity,pRoomClient->m_viptype, 2))
		{

        } 
		else if (proominfo->isUserOnly)
		{
			ret =LOGIN_NOUIN;
			break;
		}
		
		//验证密码
		//if (LoginCheck(pRoomClient->m_identity,pRoomClient->m_viptype, 0))
		if (LoginCheckEx(pRoomClient->m_identity,pRoomClient->m_viptype, 0))
		{

        } 
		else if(strlen(proominfo->passwd)>0)
		{
			 if(strcmp(proominfo->passwd, passwd) )
			{
				ret = LOGIN_EPWD;
				break;
			}
		}
		
		AC_DEBUG("ClientDataDecoder::DoLogin:roomid=%d,passwd = %s proominfo->allowhuman = %d proominfo->maxhuman = %d login_ret= %d", 
			proominfo->roomid,passwd, proominfo->allowhuman, proominfo->maxhuman,ret);

		
		ret = LOGIN_SUCESS;
		pRoomClient->m_roomid = roomid;

		proominfo->userlist.insert(make_pair(pRoomClient->m_idx, pRoomClient));
		if(pRoomClient->m_sex)
			proominfo->userlistMale.insert(make_pair(pRoomClient->m_idx, pRoomClient->m_idx));
		else
			proominfo->userlistFemale.insert(make_pair(pRoomClient->m_idx, pRoomClient->m_idx));

		//在线管理员
		if(pRoomClient->m_identity==USER_ID_OWNER||pRoomClient->m_identity==USER_ID_OWNER_S
			||pRoomClient->m_identity==USER_ID_VJ||pRoomClient->m_identity==USER_ID_VJ_A)
		{
			ROOM_MANAGER manager;
			manager.m_idx=pRoomClient->m_idx;
			manager.m_identity=pRoomClient->m_identity;
			proominfo->managerlist_online.insert(make_pair(pRoomClient->m_idx, manager));
		}

		//add by jinguanfu 2011/1/25
		pRoomClient->m_onmicflag = USER_STATE_NONE;
		
		AC_DEBUG("ClientDataDecoder::DoLogin:ret = %d roomid = %d idx = %d identity = %d usersize = %d proominfo = %x &g_pNetProxy->m_roomlistinfo.roommap = %x fd = %d", 
				ret, pRoomClient->m_roomid, pRoomClient->m_idx, pRoomClient->m_identity, proominfo->userlist.size(),
				proominfo, &g_pNetProxy->m_roomlistinfo.roommap, pClient->GetFD());
	

		//回包给用户
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		char type = 65;
		outstream->Write(type);
		outstream->Write(cmd);
		outstream->Write(seq);
		outstream->Write(ret);
		outstream->Write(pRoomClient->m_identity);
		outstream->Flush();
		if(g_pNetProxy->SendToSrv(pClient, *outstream) == -1)
		{
			AC_ERROR("ClientDataDecoder::DoLogin:g_pNetProxy->SendToSrv error");
			return -1;
		}

		pRoomClient->SetTimeout(g_pNetProxy->m_Readtimeout);   //心跳
		pRoomClient->RegisterRead(g_pNetProxy->m_Readtimeout); 

		//发送给房间内所有的人有人进入房间的消息
		BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
		outstream2->Write(type);
		outstream2->Write((short)ROOM_CMD_SB_ENTERROOM);
		outstream2->Write((int)0);
		outstream2->Write((int)pRoomClient->m_idx);
		outstream2->Write(pRoomClient->m_identity);
		outstream2->Flush();
		/*
		if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2) == -1)
		{
			AC_ERROR("ClientDataDecoder::DoLogin:g_pNetProxy->BroadcastSBCmd error");
			return -1;
		}
		*/
		char outbuf[50];                     
		int outbuflen = sizeof(outbuf);  
        if(!StreamCompress(outstream2->GetData(),(int)outstream2->GetSize(),outbuf,outbuflen))
        {
            AC_ERROR("StreamCompress error");
            return -1;
        }

		map<int, RoomClient*>::iterator itsend = proominfo->userlist.begin();
		for(;itsend != proominfo->userlist.end();itsend++)
		{
			RoomClient* psend = itsend->second;
			if(psend != NULL)
			{
				//if(StreamEncrypt(outstream2->GetData(),(int)outstream2->GetSize(),outbuf,outbuflen,psend->m_sessionkey,1))
                {
                    if(psend->AddBuf(outbuf,outbuflen) == -1)
                    {
                        //psend->ErrorClose();
						AC_ERROR("psend->AddBuf Error");
                    }
                }
			}
		}
		AC_DEBUG("ClientDataDecoder::DoLogin:Success!");

		AC_DEBUG("ClientDataDecoder::DoLogin: sendgiftlist size =%d",proominfo->sendgiftlist.size());
		vector<SEND_GIFT_INFO*>::iterator itsendgift=proominfo->sendgiftlist.begin();
		for(;itsendgift!=proominfo->sendgiftlist.end();itsendgift++)
		{
			SEND_GIFT_INFO* pFdata=(SEND_GIFT_INFO*)(*itsendgift);	
			BinaryWriteStream* giftstream=StreamFactory::InstanceWriteStream();
			char type = 65;
			giftstream->Write(type);
			giftstream->Write((short)ROOM_CMD_SB_SEND_GIFT);
			giftstream->Write((int)0);
			giftstream->Write(pFdata->s_idx);		//送礼者
			giftstream->Write(pFdata->r_idx);		//接收礼物者
			giftstream->Write(pFdata->cate_idx);	//礼物id
			giftstream->Write(pFdata->number);		//礼物总数
			int leavings=pFdata->number-pFdata->havesend;
			giftstream->Write(leavings);				//礼物剩余数
			giftstream->Write(pFdata->interval);		//礼物刷新间隔时间
			giftstream->Write(pFdata->numpertime);		//礼物刷新个数
			giftstream->Flush();

			if(g_pNetProxy->SendToSrv(pClient, *giftstream) == -1)
			{
				AC_ERROR("ClientDataDecoder::DoLogin:gift info SendToSrv error");
				return -1;
			}
		}	
		
		//发送给大厅有人进入房间的消息
		if(SendToHall(pClient, HALL_CMD_ENTERROOM,proominfo) == -1)
		{
			AC_ERROR("ClientDataDecoder::DoLogin:SendToHall error");
			return -1;
		}
		//发送用户登陆到房间通知
		//SendToFactory(pClient,FACTORY_CMD_LOGIN_R2F);

		//发送用户登陆消息到日志服务器
		SendToLogServer( pClient, CMD_ROOM_ENTERROOM_R2LS,pRoomClient->m_idx,pRoomClient->m_roomid);
		
		return 0;
		
	}while(0);
	

	AC_DEBUG("ClientDataDecoder::DoLogin:roomid = %d ret = %d", roomid, ret);
	//登陆失败回包给用户
	BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
	char type = 65;
	outstream->Write(type);
	outstream->Write(cmd);
	outstream->Write(seq);
	outstream->Write(ret);
	outstream->Flush();
	g_pNetProxy->SendToSrv(pClient,*outstream, 1, 1);
	
	return -1;
	
}

int ClientDataDecoder::DoLeaveRoom(ClientSocketBase *pClient)
{
	RoomClient* pRoomClient = (RoomClient*)pClient;
	pRoomClient->LeaveRoom();
 
// 	NOT_USED(bclose);
// 
// // 	AC_DEBUG("ClientDataDecoder::DoLeaveRoom: roomid = %d btoken = %d", 
// // 		pRoomClient->m_roomid, pRoomClient->m_btoken);
// 	
// 	if(pRoomClient->m_roomid == 0 || pRoomClient->m_btoken == 0)
// 	{
// 		AC_ERROR("ClientDataDecoder::DoLeaveRoom:client error");
// 		return -1;
// 	}
// 	ROOM_INFO* proominfo =g_pNetProxy->GetRoomInfo(pRoomClient->m_roomid);
// 	if(proominfo == NULL)
// 	{
// 		AC_ERROR("ClientDataDecoder::DoLeaveRoom:roomid error");
// 		return -1;
// 	}
// 	
// 	AC_DEBUG("ClientDataDecoder::DoLeaveRoom:m_fd = %d roomid = %d idx = %d identity = %d usersize = %d proominfo = %x &g_pNetProxy->m_roomlistinfo.roommap = %x", 
// 		pRoomClient->GetFD(), pRoomClient->m_roomid, pRoomClient->m_idx, pRoomClient->m_identity, proominfo->userlist.size(), proominfo,
// 		&g_pNetProxy->m_roomlistinfo.roommap);
// 
// 	
// 	//排麦列表清除 
// 	ClearWaitmicList(proominfo, (int)pRoomClient->m_idx);
// 
// 
// 	//持麦列表清除
// 	ClearOnmic(proominfo, (int)pRoomClient->m_idx);
// 
// 	//timout 时消除
// 	//DB对应map清除
// 	g_pNetProxy->ClearClientDBMap(pClient);
// 	
// 	map<int, RoomClient*>::iterator itu = proominfo->userlist.find(pRoomClient->m_idx);
// 	if(itu == proominfo->userlist.end())
// 	{
// 		AC_ERROR("ClientDataDecoder::DoLeaveRoom:idx error");
// 		return -1;
// 	}
// 
// 
// 	//发送给房间内所有的人有人离开房间的消息
// 	int seq2 = 0;
// 	BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
// 	char type = 65;
// 	outstream2->Write(type);
// 	outstream2->Write((short)ROOM_CMD_SB_LEAVEROOM);
// 	outstream2->Write(seq2);
// 	outstream2->Write((int)pRoomClient->m_idx);
// 	outstream2->Flush();
// 	/*
// 	if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2) == -1)
// 	{
// 		AC_ERROR("ClientDataDecoder::DoLeaveRoom:g_pNetProxy->BroadcastSBCmd error");
// 		return -1;
// 	}
// 	*/
// 	char outbuf[50];                     
// 	int outbuflen = sizeof(outbuf);  
// 	if(!StreamCompress(outstream2->GetData(),(int)outstream2->GetSize(),outbuf,outbuflen))
// 	{
// 		AC_ERROR("StreamCompress error");
// 		return 0;
// 	}
// 	map<int,RoomClient*> pMap = proominfo->userlist;
// 	map<int, RoomClient*>::iterator itsend = pMap.begin();
// 	for(;itsend != pMap.end();itsend++)
// 	{
// 		RoomClient* psend = itsend->second;
// 		if(psend != NULL)
// 		{
// 			if(psend->AddBuf(outbuf,outbuflen) == -1)
// 			{
// 				//psend->ErrorClose();
// 				AC_ERROR("psend->AddBuf Error");
// 			} 
//         }
// 	}
// 
// 	//用户列表清除
// 	proominfo->userlist.erase(itu);
// 
// 	//
// 	if(pRoomClient->m_sex)
// 	{
// 		map<int,int>::iterator it_male=proominfo->userlistMale.find((int)pRoomClient->m_idx);
// 		if(it_male!=proominfo->userlistMale.end())
// 			proominfo->userlistMale.erase(it_male);
// 	}else
// 	{
// 		map<int,int>::iterator it_female=proominfo->userlistFemale.find((int)pRoomClient->m_idx);
// 		if(it_female!=proominfo->userlistFemale.end())
// 			proominfo->userlistFemale.erase(it_female);
// 	}
// 
// 	//在线管理员清除
// 	if(pRoomClient->m_identity==USER_ID_OWNER||pRoomClient->m_identity==USER_ID_OWNER_S
// 			||pRoomClient->m_identity==USER_ID_VJ||pRoomClient->m_identity==USER_ID_VJ_A)
// 	{
// 		map<int,ROOM_MANAGER>::iterator itMO=proominfo->managerlist_online.find((int)pRoomClient->m_idx);
// 		if(itMO!=proominfo->managerlist_online.end())
// 			proominfo->managerlist_online.erase(itMO);
// 
// 	 }
// 
// 	
// 	//add by jinguanfu 2011/3/4
// 	//消除邀请列表
// 	map<int,int>::iterator itinvite=proominfo->invitelist.find(pRoomClient->m_idx);
// 	if(itinvite!=proominfo->invitelist.end())
// 	{
// 		proominfo->invitelist.erase(itinvite);
// 	}
// 	
// 	//发送给大厅有人离开房间的消息并通知当前房间内在线男女数
// 	if(SendToHall(pClient, HALL_CMD_LEAVEROOM,proominfo) == -1)
// 	{
// 		AC_ERROR("ClientDataDecoder::DoLeaveRoom:SendToHall error");
// 		return -1;
// 	}
// 	//发送工厂服务器有人退出房间通知
// 	//SendToFactory(pClient,FACTORY_CMD_LOGOUT_R2F);
// 
// 	//发送用户登陆消息到日志服务器
// 	SendToLogServer( pClient, CMD_ROOM_LEAVEROOM_R2LS,pRoomClient->m_idx,pRoomClient->m_roomid);
// 
// 	pRoomClient->init(0);
// 	
 	return 0;
}

int ClientDataDecoder::DoGetAllInfo(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoGetAllInfo:pinstream = %x", pinstream);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	//BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
	static char outbuf[16384] = {0}; //16k
	BinaryWriteStream outstream(outbuf,sizeof(outbuf));

	char type = 65;
	outstream.Write(type);
	outstream.Write(cmd);
	outstream.Write(seq);

	//body
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoGetAllInfo:roominfo error");
		return -1;
	}

	//userlist
	short usernum = proominfo->userlist.size()+proominfo->vuserlist.size();
	outstream.Write(usernum);
	map<int, RoomClient*>::iterator itu = proominfo->userlist.begin();
	for(; itu != proominfo->userlist.end(); itu++)
	{
		RoomClient* p = (*itu).second;
		outstream.Write((int)p->m_idx);
		//outstream.Write(p->m_identity);
	}
	//虚拟用户列表
	map<int, RoomClient*>::iterator itvu = proominfo->vuserlist.begin();
	for(; itvu != proominfo->vuserlist.end(); itvu++)
	{
		RoomClient* p = (*itvu).second;
		outstream.Write((int)p->m_idx);
	}

	//waitmiclist
	short micnum = proominfo->miclist.size();
	outstream.Write(micnum);
	vector<MIC_INFO*>::iterator itm = proominfo->miclist.begin();
	for(; itm != proominfo->miclist.end(); itm++)
	{
		MIC_INFO* pmicinfo = (MIC_INFO*)(*itm);
		outstream.Write(pmicinfo->idx);
		outstream.Write(pmicinfo->musicid);
		outstream.Write(pmicinfo->bk);
		outstream.Write(pmicinfo->pkidx);
		outstream.Write(pmicinfo->pkmusicid);
		outstream.Write(pmicinfo->pkbk);
		outstream.Write(pmicinfo->level);
		//add by jinguanfu 2012/2/23  简化版
		outstream.Write(pmicinfo->musicname,strlen(pmicinfo->musicname));
		outstream.Write(pmicinfo->musicspeed);
		//add by jinguanfu 2012/4/5
		//outstream.Write(pmicinfo->reverseflag);
	}

	//vjonmic
	outstream.Write((int)proominfo->vjonmic);
	
	//onmic
	outstream.Write(proominfo->onmic.idx);
	outstream.Write(proominfo->onmic.musicid);
	outstream.Write(proominfo->onmic.bk);
	outstream.Write(proominfo->onmic.state);		//add by jinguanfu 2010/6/5  表演麦状态
	outstream.Write(proominfo->onmic.pkidx);
	outstream.Write(proominfo->onmic.pkmusicid);
	outstream.Write(proominfo->onmic.pkbk);
	outstream.Write(proominfo->onmic.pkstate);		//add by jinguanfu 2010/6/5  PK麦状态
	outstream.Write(proominfo->onmic.level);
	//add by jinguanfu 2012/2/23  简化版
	outstream.Write(proominfo->onmic.musicname,strlen(proominfo->onmic.musicname));
	outstream.Write(proominfo->onmic.musicspeed);
	//add by jinguanfu 2012/4/5
	//outstream.Write(proominfo->onmic.reverseflag);

	
	AC_DEBUG("ClientDataDecoder::DoGetAllInfo:roomid=%d,ONMIC idx=%d,musicid=%d,bk=%d,state=%d,pkidx=%d,pkmusicid=%d,pkbk=%d,pkstate=%d,level=%d",
		proominfo->roomid,proominfo->onmic.idx,proominfo->onmic.musicid,proominfo->onmic.bk,proominfo->onmic.state,
		proominfo->onmic.pkidx,proominfo->onmic.pkmusicid,proominfo->onmic.pkbk,proominfo->onmic.pkstate,
		proominfo->onmic.level);

	//managerllist
	short managernum=proominfo->managerlist.size();
	outstream.Write(managernum);
	map<int,ROOM_MANAGER>::iterator itman=proominfo->managerlist.begin();
	for(;itman!=proominfo->managerlist.end();itman++)
	{
		ROOM_MANAGER roommanager=itman->second;
		outstream.Write(roommanager.m_idx);
		outstream.Write(roommanager.m_identity);
	}
	
    	//content
	outstream.Write(proominfo->content, strlen(proominfo->content));
	//add by jinguanfu 2010/11/27
	//音视频服务器地址和端口信息
	outstream.Write(proominfo->AVServerIP_telcom, strlen(proominfo->AVServerIP_telcom));
	outstream.Write(proominfo->AVServerPort_telcom);
	outstream.Write(proominfo->AVServerIP_netcom, strlen(proominfo->AVServerIP_netcom));
	outstream.Write(proominfo->AVServerPort_netcom);
	outstream.Flush();
	
	if(g_pNetProxy->SendToSrv(pClient, outstream) == -1)
	{
		AC_ERROR("ClientDataDecoder::DoGetAllInfo:idx=%d g_pNetProxy->SendToSrv error",pRoomClient->m_idx);
		return -1;
	}
	AC_DEBUG("ClientDataDecoder::DoGetAllInfo:roomid=%d,idx=%d success",proominfo->roomid,pRoomClient->m_idx);

	return 0;
}

int ClientDataDecoder::DoWaitMic(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoWaitMic:cmd = %d seq = %d", cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoWaitMic:roominfo error");
		return -1;
	}
	
	AC_DEBUG("ClientDataDecoder::DoWaitMic:roomid =%d,idx=%d ",proominfo->roomid,pRoomClient->m_idx);

	//add by jinguanfu 2011/4/19 非自由排麦下的排麦
	if(!proominfo->isMicUpdown)
	{
		if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype, USER_ID_NONE, USER_ID_NONE, cmd)==0)
		{
			AC_ERROR("ClientDataDecoder::DoWaitMic:power error");
			return 0;
		}
	}

	//add by jinguanfu 2010/8/25 房间内上麦排序不能超过30
	if(proominfo->miclist.size()>=30)
	{
		AC_ERROR("ClientDataDecoder::DoWaitMic:miclist.size[%d] over than 30",proominfo->miclist.size());
		return 0;
	}

	
	//判断是否在麦上
	if(proominfo->onmic.idx == pRoomClient->m_idx || proominfo->onmic.pkidx ==pRoomClient->m_idx)
	{
		AC_ERROR("ClientDataDecoder::DoWaitMic:idx has onmic error");
		return 0;//双击误操作时不断开连接
	}
	//add by jinguanfu 2010/1/21 <begin>
	//判断是否在VJ麦上
	if(proominfo->vjonmic==pRoomClient->m_idx)
	{
		AC_ERROR("ClientDataDecoder::DoWaitMic:idx has vjonmic error");
		return 0;	//双击误操作时不断开连接
	}
	//add by jinguanfu 2010/1/21 <end>
	
	//判断是否在麦序列表中
	vector<MIC_INFO*>::iterator itm = SearchWaitMic(proominfo->miclist.begin(), proominfo->miclist.end(),pRoomClient->m_idx);
	//modify by jinguanfu 2010/6/8 <end>
    if(itm != proominfo->miclist.end())
   	{
		AC_ERROR("ClientDataDecoder::DoWaitMic:idx = %d has wait mic", pRoomClient->m_idx);
		return 0;//双击误操作时不断开连接
	}

	itm = SearchPKWaitMic(proominfo->miclist.begin(), proominfo->miclist.end(), pRoomClient->m_idx);
	if(itm != proominfo->miclist.end())
	{
		AC_ERROR("ClientDataDecoder::DoWaitMic:pkidx = %d has wait mic", pRoomClient->m_idx);
		return 0;//双击误操作时不断开连接
	}


	short musicid = 0;
	if(!pinstream->Read(musicid))
	{
		AC_ERROR("ClientDataDecoder::DoWaitMic:read musicid error");
		return -1;
	}

	//modify by lihongwu 2011-9-30
	if(musicid < 0)           //0是自由麦
	{
		AC_ERROR("ClientDataDecoder::DoWaitMic:musicid error");
		return -1;
	}

	short bk = 0;
	if(!pinstream->Read(bk))
	{
		AC_ERROR("ClientDataDecoder::DoWaitMic:read bk error");
		return -1;
	}

	if(bk == 0)
	{
		AC_ERROR("ClientDataDecoder::DoWaitMic:bk error");
		return -1;
	}

	char level = 0;
	if(!pinstream->Read(level))
	{
		AC_ERROR("ClientDataDecoder::DoWaitMic:read level error");
		return -1;
	}

	if(level == 0)
	{
		AC_ERROR("ClientDataDecoder::DoWaitMic:level error");
		return -1;
	}

	//add by jinguanfu 2012/2/23  对应简化版,原有版本无此字段
	char musicname[128];	//歌曲名
	size_t namelen=0;
	if(!pinstream->Read(musicname,sizeof(musicname)-1,namelen))
	{
		AC_ERROR("ClientDataDecoder::DoWaitMic:read music name  error");
		return -1;
	}
	musicname[namelen]=0;

	//add by jinguanfu 2012/2/27
	char musicspeed=0;		//歌曲速度
	if(!pinstream->Read(musicspeed))
	{
		AC_ERROR("ClientDataDecoder::DoOnMicReadyOK:read musicspeed  error");
		return -1;
	}

	//add by jinguanfu 2012/4/5
	/*
	char reverseflag=0;	//反调标志位，女声男唱或男声女唱
	if(!pinstream->Read(reverseflag))
	{
		AC_ERROR("ClientDataDecoder::DoOnMicReadyOK:read reverseflag  error");
		return -1;
	}
	*/
	//modify by jinguanfu 2010/3/30 <begin>
	MIC_INFO* pMicInfo=g_pNetProxy->CreateMicInfo();
	if(pMicInfo!=NULL)
	{
		pMicInfo->init();	//清空数据
		pMicInfo->idx = pRoomClient->m_idx;
		pMicInfo->bk=bk;
		pMicInfo->musicid=musicid;
		pMicInfo->level=level;
		//add by jinguanfu 2012/2/24  简化版
		memcpy(pMicInfo->musicname,musicname,namelen);
		pMicInfo->musicspeed=musicspeed;
		//add by jinguanfu 2012/4/5
		//pMicInfo->reverseflag=reverseflag;
	}
	else
	{
		AC_ERROR("ClientDataDecoder::DoWaitMic:CreateMicInfo error");
		return -1;
	}

	AC_DEBUG("ClientDataDecoder::DoWaitMic:MicInfo idx=%d ,musicid=%d,musicname=%s.",
							pMicInfo->idx,
							pMicInfo->musicid,
							pMicInfo->musicname);
	

	//add by jinguanfu 2011/1/25
	pRoomClient->m_onmicflag = USER_STATE_WAITMIC;

	AC_DEBUG("ClientDataDecoder::DoWaitMic:idx=%d Wait Mic Success",pRoomClient->m_idx);
	//modify by jinguanfu 2010/3/30 <end>
	//发送给房间内所有的人有人排麦成功的消息
	int seq2 = 0;
	BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
	char type = 65;
	outstream2->Write(type);
	outstream2->Write((short)ROOM_CMD_SB_WAITMIC);
	outstream2->Write(seq2);
	outstream2->Write((int)pRoomClient->m_idx);
	outstream2->Write(musicid);
	outstream2->Write(bk);
	outstream2->Write(level);
	//add by jinguanfu 2012/2/24  简化版
	outstream2->Write(musicname,namelen);
	outstream2->Write(musicspeed);
	//add by jinguanfu 2012/4/5
	//outstream2->Write(reverseflag);
	outstream2->Flush();
	if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2) == -1)
	{
		AC_ERROR("ClientDataDecoder::DoWaitMic:g_pNetProxy->BroadcastSBCmd WAITMIC error");
		return -1;
	}

	//add by jinguanfu 2010/12/29
	//向日志服务器发送音乐点击日志
	BinaryWriteStream* logstream=StreamFactory::InstanceWriteStream();
	logstream->Write((short)CMD_MUSIC_CHECK_R2LS);
	logstream->Write(0);
	logstream->Write((int)musicid);	//音乐ID
	logstream->Write(3);	//日志服务器上房间type
	logstream->Flush();

	BackClientSocketBase *pLogSvr = g_pNetProxy->GetLogServer();
	if(pLogSvr!=NULL)
	{
		if(pLogSvr->AddBuf(logstream->GetData(), logstream->GetSize())==-1) 
		{
			AC_ERROR("ClientDataDecoder::DoWaitMic:pLogSvr->AddBuf Error");
		}else
		{
			AC_DEBUG("ClientDataDecoder::DoWaitMic:Send to LogSvr success,musicid=%d",musicid);
		}

	}
	else
	{
		AC_ERROR("ClientDataDecoder::DoWaitMic:pLogSvr is NULL");
	}
	

	
	//如果没有人持麦直接上麦准备了
	proominfo->miclist.push_back(pMicInfo);

	//if(CheckMiclistOnmic(proominfo) == -1)
	if(CheckMiclistOnmic2(proominfo) == -1)
	{
		AC_ERROR("ClientDataDecoder::DoWaitMic:CheckMiclistOnmic error");
		return -1;
	}

	return 0;
}

int ClientDataDecoder::DoCancleWaitMic(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoCancleWaitMic:pinstream = %x cmd = %d seq = %d",pinstream, cmd ,seq);
	RoomClient* pRoomClient = (RoomClient*)pClient;
	
	int idx;
	if(!pinstream->Read(idx))
	{
		AC_ERROR("ClientDataDecoder::DoCancleWaitMic:read idx error");
        	return -1;
	}
	if(idx==0)
	{
		AC_ERROR("ClientDataDecoder::DoCancleWaitMic:idx error");
        return -1;
	}
	
	
	AC_DEBUG("ClientDataDecoder::DoCancleWaitMic:roomid=%d,pRoomClient->m_idx=%d,idx=%d ",
		pRoomClient->m_roomid,pRoomClient->m_idx,idx);
	
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoCancleWaitMic:roominfo error");
		return -1;
	}

	//自己取消排麦
	if(idx==pRoomClient->m_idx)
	{
		AC_DEBUG("ClientDataDecoder::DoCancleWaitMic:Cancel by self");
	}
	//被管理员踢出麦序
	else
	{
		AC_DEBUG("ClientDataDecoder::DoCancleWaitMic:Cancel by manger");

		map<int,RoomClient*>::iterator itbc = proominfo->userlist.find(idx);
		if(itbc==proominfo->userlist.end())
		{
			AC_ERROR("ClientDataDecoder::DoCancleWaitMic:idx=%d not in room",idx);
			return -1;
		}
		RoomClient* pbClient=itbc->second;

//		if(CheckRight(pRoomClient->m_identity, pRoomClient->m_viptype,pbClient->m_identity,pbClient->m_viptype, cmd)==0)
		if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,pbClient->m_identity,pbClient->m_viptype, cmd)==0)	
		{
			AC_ERROR("ClientDataDecoder::DoCancleWaitMic:idx = %d no option right",pRoomClient->m_idx);
	     		return -1;
		}

	}

	
	MIC_INFO micinfo;
	micinfo.idx = idx;

	//vector<MIC_INFO*>::iterator itm = find(proominfo->miclist.begin(), proominfo->miclist.end(), micinfo);
	vector<MIC_INFO*>::iterator itm = SearchWaitMic(proominfo->miclist.begin(), proominfo->miclist.end(), idx);
	if(itm != proominfo->miclist.end())
	{
		MIC_INFO* pmicinfo = (MIC_INFO*)(*itm);
		if(pmicinfo->pkidx == 0)
		{
			proominfo->miclist.erase(itm);
			//add by jinguanfu 2010/3/30  释放内存到缓存池
			g_pNetProxy->DestroyMicInfo(pmicinfo);
		}
		else 
		{
			pmicinfo->idx = 0;
			pmicinfo->musicid = 0;
			pmicinfo->bk = 0;
			pmicinfo->state = MIC_INFO_WAIT;
		}
	}
	else
	{
		AC_ERROR("ClientDataDecoder::DoCancleWaitMic:idx = %d has not wait mic",idx);
		return 0;//双击误操作时不断开连接
	}

	//add by jinguanfu 2011/1/25
	map<int,RoomClient*>::iterator itc = proominfo->userlist.find(idx);
	if(itc!=proominfo->userlist.end())
	{
		RoomClient* pRoomClient=(RoomClient*)itc->second;
		pRoomClient->m_onmicflag = USER_STATE_NONE;	
	}

	//发送给房间内所有的人有人取消排麦成功的消息
	int seq2 = 0;
	BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
	char type = 65;
	outstream2->Write(type);
	outstream2->Write((short)ROOM_CMD_SB_CANCLE_WAITMIC);
	outstream2->Write(seq2);
	outstream2->Write((int)pRoomClient->m_idx);	//操作者
	outstream2->Write(idx);					//被操作者
	outstream2->Flush();
	if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2) == -1)
	{
		AC_ERROR("ClientDataDecoder::DoCancleWaitMic:g_pNetProxy->BroadcastSBCmd CANCLEWAITMIC error");
		return -1;
	}

    return 0;
}

int ClientDataDecoder::DoWaitMicPK(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoWaitMicPK:cmd = %d seq = %d", cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoWaitMicPK:roominfo error");
		return -1;
	}

	if(proominfo->onmic.idx == pRoomClient->m_idx || proominfo->onmic.pkidx ==pRoomClient->m_idx)
    	{
		AC_ERROR("ClientDataDecoder::DoWaitMic:idx has onmic error");
		return 0;//双击误操作时不断开连接
    	}

	int idx = 0;
	if(!pinstream->Read(idx))  //pk对象的idx
	{
		AC_ERROR("ClientDataDecoder::DoWaitMicPK:read idx error");
		return -1;
	}

	if(idx == 0)
	{
		AC_ERROR("ClientDataDecoder::DoWaitMicPK:idx error");
		return -1;
	}

	short pkmusicid = 0;
	if(!pinstream->Read(pkmusicid))
	{
		AC_ERROR("ClientDataDecoder::DoWaitMicPK:read pkmusicid error");
		return -1;
	}

	//modify by lihongwu 2011/9/30
	/*if(pkmusicid == 0)
	{
		AC_ERROR("ClientDataDecoder::DoWaitMicPK:pkmusicid error");
		return -1;
	}*/

	short pkbk = 0;
	if(!pinstream->Read(pkbk))
	{
		AC_ERROR("ClientDataDecoder::DoWaitMicPK:read pkbk error");
		return -1;
	}

	if(pkbk == 0)
	{
		AC_ERROR("ClientDataDecoder::DoWaitMicPK:pkbk error");
		return -1;
	}

	MIC_INFO micinfo;
	micinfo.idx = idx;
	//vector<MIC_INFO*>::iterator itm = find(proominfo->miclist.begin(), proominfo->miclist.end(), micinfo);
	vector<MIC_INFO*>::iterator itm = SearchWaitMic(proominfo->miclist.begin(), proominfo->miclist.end(), idx);
	if(itm == proominfo->miclist.end())
	{
		AC_ERROR("ClientDataDecoder::DoWaitMicPK:idx error");
		return 0;//双击误操作时不断开连接
	}
	MIC_INFO* pmicinfo = (MIC_INFO*)(&(*itm));
	pmicinfo->pkidx = pRoomClient->m_idx;
	pmicinfo->pkmusicid = pkmusicid;
	pmicinfo->pkbk = pkbk;

	//发送给房间内所有的人PK成功的消息
	int seq2 = 0;
	BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
	char type = 65;
	outstream2->Write(type);
	outstream2->Write((short)ROOM_CMD_SB_WAITMIC_PK);
	outstream2->Write(seq2);
	outstream2->Write(pmicinfo->idx);  
    	outstream2->Write(pmicinfo->musicid);
	outstream2->Write(pmicinfo->bk);
	outstream2->Write(pmicinfo->pkidx); 
	outstream2->Write(pmicinfo->pkmusicid);  
	outstream2->Write(pmicinfo->pkbk);
	outstream2->Write(pmicinfo->level);
	outstream2->Flush();
	if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2) == -1)
	{
		AC_ERROR("ClientDataDecoder::DoWaitMicPK:g_pNetProxy->BroadcastSBCmd error");
		return -1;
	}
	return 0;
}


int ClientDataDecoder::DoCancleWaitMicPK(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoCancleWaitMicPK:pinstream = %x cmd = %d seq = %d", pinstream, cmd ,seq);

	int idx;
	if(!pinstream->Read(idx))
	{
		AC_ERROR("ClientDataDecoder::DoCancleWaitMic:read idx error");
        	return -1;
	}
	if(idx==0)
	{
		AC_ERROR("ClientDataDecoder::DoCancleWaitMic:idx error");
        	return -1;
	}
	
	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoCancleWaitMicPK:roominfo error");
		return -1;
	}

	//自己取消排麦
	if(idx==pRoomClient->m_idx)
	{
		AC_DEBUG("ClientDataDecoder::DoCancleWaitMic:Cancel by self");
	}
	//被管理员踢出麦序
	else
	{
		AC_DEBUG("ClientDataDecoder::DoCancleWaitMic:Cancel by manger");

		//主持人以上才能删除麦序
		map<int,GM_INFO>::iterator itGM=g_pNetProxy->m_GM.find(pRoomClient->m_idx);
		map<int,int>::iterator itVJ=proominfo->vjlist.find(pRoomClient->m_idx);

		if(itGM==g_pNetProxy->m_GM.end()&&
			pRoomClient->m_idx!=proominfo->ownidx&&
			pRoomClient->m_idx!=proominfo->secondownidx&&
			pRoomClient->m_idx!=proominfo->secondownidx2&&
			itVJ==proominfo->vjlist.end())
		{
	     		AC_ERROR("ClientDataDecoder::DoCancleWaitMic:idx = %d no option right",pRoomClient->m_idx);
	     		return -1;
		}
	}

	vector<MIC_INFO*>::iterator itm = SearchPKWaitMic(proominfo->miclist.begin(), proominfo->miclist.end(), idx);
	if(itm == proominfo->miclist.end())
	{
		AC_ERROR("ClientDataDecoder::DoCancleWaitMicPK:pkidx error");
		return 0;//双击误操作时不断开连接
	}
	MIC_INFO* pmicinfo = (MIC_INFO*)(*itm);
	pmicinfo->pkidx = 0;
	pmicinfo->pkmusicid = 0;
	pmicinfo->pkbk = 0;
	pmicinfo->pkstate = MIC_INFO_WAIT;

	//发送给房间内所有的人取消PK成功的消息
	int seq2 = 0;
	BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
	char type = 65;
	outstream2->Write(type);
	outstream2->Write((short)ROOM_CMD_SB_CANCLE_WAITMIC_PK);
	outstream2->Write(seq2);
	outstream2->Write((int)pRoomClient->m_idx);	//操作者
	outstream2->Write(idx);						//被操作者
	outstream2->Flush();
	if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2) == -1)
	{
		AC_ERROR("ClientDataDecoder::DoCancleWaitMicPK:g_pNetProxy->BroadcastSBCmd error");
		return -1;
	}
	return 0;
}


int ClientDataDecoder::DoUpWaitMic(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoUpWaitMic:cmd = %d seq = %d", cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	
	AC_DEBUG("ClientDataDecoder::DoUpWaitMic:roomid=%d,operatoidx=%d ",pRoomClient->m_roomid,pRoomClient->m_idx);

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoUpWaitMic:roominfo error");
		return -1;
	}


//	if(CheckRight(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
	if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
	{
		AC_ERROR("ClientDataDecoder::DoUpWaitMic:power error");
	 	return 0;
	}


	char tag = WAITMIC_UP_DOWN_IDX;
	if(!pinstream->Read(tag))    
	{
		AC_ERROR("ClientDataDecoder::DoUpWaitMic:read tag error");
		return -1;
	}

	int idx = 0;
	if(!pinstream->Read(idx))    //升麦对象的idx
	{
		AC_ERROR("ClientDataDecoder::DoUpWaitMic:read idx error");
		return -1;
	}

	if(idx == 0)
	{
		AC_ERROR("ClientDataDecoder::DoUpWaitMic:idx error");
		return -1;
	}

	char flag = MIC_UP_DOWN_ONE; //升麦的类型  
	if(!pinstream->Read(flag))
	{
		AC_ERROR("ClientDataDecoder::DoUpWaitMic:read flag error");
		return -1;
	}

	
	vector<MIC_INFO*>::iterator itm;
	if(tag == WAITMIC_UP_DOWN_IDX)
	{
		/*MIC_INFO micinfo;
		micinfo.idx = idx;*/
		itm = SearchWaitMic(proominfo->miclist.begin(), proominfo->miclist.end(), idx);
	}
	else if(tag == WAITMIC_UP_DOWN_PKIDX)
	{
		itm = SearchPKWaitMic(proominfo->miclist.begin(), proominfo->miclist.end(), idx);
	}
	else
	{
		AC_ERROR("ClientDataDecoder::DoUpWaitMic:tag error");
		return -1;
	}
    
	if(itm == proominfo->miclist.end())
	{
		AC_ERROR("ClientDataDecoder::DoUpWaitMic:idx error");
		return 0;//延时误操作时不断开连接
	}

	if(itm == proominfo->miclist.begin())
	{
		AC_ERROR("ClientDataDecoder::DoUpWaitMic:hastop error");
		return 0;//延时误操作时不断开连接
	}

#ifdef AC_HAS_DEBUG
	vector<MIC_INFO*>::iterator itmty = proominfo->miclist.begin();
	for(;itmty != proominfo->miclist.end(); itmty++)
	{
		MIC_INFO* pp = (*itmty);
		AC_DEBUG("ClientDataDecoder::DoUpWaitMic:before deal pp.idx = %d", pp->idx);
	}
#endif

	if(flag == MIC_UP_DOWN_ONE)
	{
		MIC_INFO* fmicinfo = (MIC_INFO*)(*itm);
		vector<MIC_INFO*>::iterator itm1 = itm;
		itm1--;
		MIC_INFO* imicinfo = (MIC_INFO*)(*itm1);
		proominfo->miclist.erase(itm);
		itm1 = SearchWaitMic(proominfo->miclist.begin(), proominfo->miclist.end(), imicinfo->idx);
		if(itm1 == proominfo->miclist.end())
		{
			AC_ERROR("ClientDataDecoder::DoUpWaitMic:itm1 error");
			return 0;//延时误操作时不断开连接
		}
		proominfo->miclist.insert(itm1, fmicinfo);
	}
	else if(flag == MIC_UP_DOWN_ALL)
	{
		MIC_INFO* fmicinfo = (MIC_INFO*)(*itm);
		proominfo->miclist.erase(itm);
		itm = proominfo->miclist.begin();
		proominfo->miclist.insert(itm, fmicinfo);
	}

#ifdef AC_HAS_DEBUG
	itmty = proominfo->miclist.begin();
	for(;itmty != proominfo->miclist.end(); itmty++)
	{
		MIC_INFO* pp =(MIC_INFO*) (*itmty);
		AC_DEBUG("ClientDataDecoder::DoUpWaitMic:deal after pp.idx = %d", pp->idx);
	}

	map<int, RoomClient*>::iterator itu = proominfo->userlist.begin();
	for(;itu != proominfo->userlist.end(); itu++)
	{
		RoomClient* psend = (*itu).second;
		if(psend != NULL)
		{
			AC_DEBUG("ClientDataDecoder::DoUpWaitMic:psend->m_idx = %d", psend->m_idx);
		}
	}
#endif

	//发送给房间内所有的人某人升麦成功的消息
	int seq2 = 0;
	BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
	char type = 65;
	outstream2->Write(type);
	outstream2->Write((short)ROOM_CMD_SB_UP_WAITMIC);
	outstream2->Write(seq2);
	outstream2->Write(tag);       //升麦idx或pkidx搜索
	outstream2->Write((int)idx);  //升麦对象的idx
	outstream2->Write((char)flag); //升麦的flag
	outstream2->Flush();
	if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2) == -1)
	{
		AC_ERROR("ClientDataDecoder::DoUpWaitMic:g_pNetProxy->BroadcastSBCmd error");
		return -1;
	}
	return 0;
}

int ClientDataDecoder::DoDownWaitMic(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoDownWaitMic:cmd = %d seq = %d", cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoDownWaitMic:roominfo error");
		return -1;
	}

	
	AC_DEBUG("ClientDataDecoder::DoDownWaitMic:roomid=%d,idx=%d ",pRoomClient->m_roomid,pRoomClient->m_idx);

	//if(CheckRight(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
	if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
	{
		AC_ERROR("ClientDataDecoder::DoDownWaitMic:power error");
	 	return -1;
	}


	char tag = WAITMIC_UP_DOWN_IDX;
	if(!pinstream->Read(tag))    
	{
		AC_ERROR("ClientDataDecoder::DoUpWaitMic:read tag error");
		return -1;
	}

	int idx = 0;
	if(!pinstream->Read(idx))    //降麦对象的idx
	{
		AC_ERROR("ClientDataDecoder::DoDownWaitMic:read idx error");
		return -1;
	}

	if(idx == 0)
	{
		AC_ERROR("ClientDataDecoder::DoDownWaitMic:idx error");
		return -1;
	}

	char flag = MIC_UP_DOWN_ONE; //降麦的类型  
	if(!pinstream->Read(flag))
	{
		AC_ERROR("ClientDataDecoder::DoDownWaitMic:read flag error");
		return -1;
	}

	vector<MIC_INFO*>::iterator itm;
	if(tag == WAITMIC_UP_DOWN_IDX)
	{
		MIC_INFO micinfo;
		micinfo.idx = idx;
        	itm = SearchWaitMic(proominfo->miclist.begin(), proominfo->miclist.end(), idx);
    	}
    	else if(tag == WAITMIC_UP_DOWN_PKIDX)
    	{
		itm = SearchPKWaitMic(proominfo->miclist.begin(), proominfo->miclist.end(), idx);
    	}
    	else
    	{
		AC_ERROR("ClientDataDecoder::DoDownWaitMic:tag error");
		return -1;
    	}
    
	if(itm == proominfo->miclist.end())
	{
		AC_ERROR("ClientDataDecoder::DoDownWaitMic:idx error");
		return 0;//延时误操作时不断开连接
	}

	vector<MIC_INFO*>::iterator itend = proominfo->miclist.end();
	itend --;
	if(itm == itend)
	{
		AC_ERROR("ClientDataDecoder::DoDownWaitMic:hasend error");
		return 0;//双击误操作时不断开连接
	}

    //---------------------------------------------------------------------------------------
#ifdef AC_HAS_DEBUG
	vector<MIC_INFO*>::iterator itmty = proominfo->miclist.begin();
	for(;itmty != proominfo->miclist.end(); itmty++)
	{
		MIC_INFO* pp = (MIC_INFO*) (*itmty);
		AC_DEBUG("ClientDataDecoder::DoDownWaitMic:before deal pp.idx = %d", pp->idx);
	}
#endif
    //---------------------------------------------------------------------------------------

	if(flag == MIC_UP_DOWN_ONE)
	{
		MIC_INFO* fmicinfo = (MIC_INFO*)(*itm);
		vector<MIC_INFO*>::iterator itm1 = itm;
		itm1 ++;
		MIC_INFO* imicinfo = (MIC_INFO*)(*itm1);
		proominfo->miclist.erase(itm);
		// erase后，调整了vector的迭代器，需要通过对比后一个数据来重新得到要插入的迭代器位置，
		// 不然++操作会导致崩溃 comment by wangpf 2010/07/31
		itm1 = SearchWaitMic(proominfo->miclist.begin(), proominfo->miclist.end(), imicinfo->idx);
		if(itm1 == proominfo->miclist.end())
		{
			AC_ERROR("ClientDataDecoder::DoDownWaitMic:itm1 error");
			return 0;
		}
		itm1 ++;
        if(itm1 == proominfo->miclist.end())
            proominfo->miclist.push_back(fmicinfo);
        else
		    proominfo->miclist.insert(itm1, fmicinfo);
	}
	else if(flag == MIC_UP_DOWN_ALL)
	{
		MIC_INFO* fmicinfo = (MIC_INFO*)(*itm);
		proominfo->miclist.erase(itm);
		proominfo->miclist.push_back(fmicinfo);
	}


    //---------------------------------------------------------------------------------------
#ifdef AC_HAS_DEBUG
	itmty = proominfo->miclist.begin();
	for(;itmty != proominfo->miclist.end(); itmty++)
	{
		MIC_INFO* pp = (MIC_INFO*)(*itmty);
		AC_DEBUG("ClientDataDecoder::DoDownWaitMic:deal after pp.idx = %d", pp->idx);
	}

	map<int, RoomClient*>::iterator itu = proominfo->userlist.begin();
	for(;itu != proominfo->userlist.end(); itu++)
	{
		RoomClient* psend = (*itu).second;
		if(psend != NULL)
		{
			AC_DEBUG("ClientDataDecoder::DoDownWaitMic:psend->m_idx = %d", psend->m_idx);
		}
	}
#endif
    //---------------------------------------------------------------------------------------

	//发送给房间内所有的人某人降麦成功的消息
	int seq2 = 0;
	//char outbuf[256]64] = {0};
	BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
	//BinaryWriteStream outstream2(outbuf2,sizeof(outbuf2));
	char type = 65;
	outstream2->Write(type);
	outstream2->Write((short)ROOM_CMD_SB_DOWN_WAITMIC);
	outstream2->Write(seq2);
	outstream2->Write(tag);       //升麦idx或pkidx搜索
	outstream2->Write((int)idx);  //升麦对象的idx
	outstream2->Write((char)flag); //升麦的flag
	outstream2->Flush();
	if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2) == -1)
	{
		AC_ERROR("ClientDataDecoder::DoDownWaitMic:g_pNetProxy->BroadcastSBCmd error");
		return -1;
	}
	return 0;
}

int ClientDataDecoder::DoOnMicReadyOK(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoOnMicReadyOK:pinstream = %x cmd = %d seq = %d", pinstream, cmd, seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	
	AC_DEBUG("ClientDataDecoder::DoOnMicReadyOK:roomid=%d,idx=%d ",pRoomClient->m_roomid,pRoomClient->m_idx);

	if(pRoomClient->m_idx == 0)
	{
		AC_ERROR("ClientDataDecoder::DoOnMicReadyOK:idx error");
		return -1;
	}

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoOnMicReadyOK:roominfo error");
		return -1;
	}

	if(proominfo->onmic.idx == 0 && proominfo->onmic.pkidx == 0)
	{
		AC_ERROR("ClientDataDecoder::DoOnMicReadyOK:proominfo->onmic error");
		return -1;
	}

	//add by jinguanfu 2011/8/10
	short musictime = 0;
	if(!pinstream->Read(musictime))
	{
		AC_ERROR("ClientDataDecoder::DoOnMicReadyOK:read musictime error");
		return -1;
	}
	
	AC_DEBUG("ClientDataDecoder::DoOnMicReadyOK:musictime=%d",musictime);
	
	//检查歌曲时间合法性(歌曲时长不超过10分钟)
	if(musictime <= 0 ||musictime>600)
	{
		//取消上麦准备超时
		if(proominfo->m_pOnMicTimeout!=NULL)
			proominfo->m_pOnMicTimeout->UnRegisterTimer();
		//下麦通知
		char type=65;
		BinaryWriteStream* outstream = StreamFactory::InstanceWriteStream();
		outstream->Clear();
		outstream->Write(type);
		outstream->Write((short)ROOM_CMD_SB_OFFMIC);
		outstream->Write(0);
		outstream->Write((char)0);	//haspk
		outstream->Write((int)proominfo->onmic.idx);					
		outstream->Write((int)0);	//新增经验值add by jinguanfu 2011/8/10
	    outstream->Write(0);			//打分信息体	
		outstream->Flush();
		g_pNetProxy->BroadcastSBCmd(proominfo, *outstream) ;

		proominfo->onmic.init();
	
		//下麦5秒后下一个人上麦
		roomtimeout* ptb = g_pNetProxy->CreateRoomTimeout();
		if(ptb!=NULL)
		{
	        ptb->m_roomid = pRoomClient->m_roomid;
			ptb->m_idx = pRoomClient->m_idx;
			ptb->m_time = OFFMIC_WAIT; //5s
			ptb->m_type = TIMEOUT_OFFMIC;
			ptb->SetReactor(g_pNetProxy->GetReactor());
			ptb->m_pClientDataDecoder=this;
			ptb->m_haspk = 0;
			ptb->RegisterTimer(ptb->m_time);
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoOnMicReadyOK: CreateRoomTimeout error");
			//CheckMiclistOnmic(proominfo);
			CheckMiclistOnmic2(proominfo);
		}

		return 0;
	}

	proominfo->onmic.musictime = musictime;

	
	if(pRoomClient->m_idx == proominfo->onmic.idx)
	{
		//PK 麦
		if(proominfo->onmic.pkidx != 0)
		{
			if(proominfo->onmic.pkstate == MIC_INFO_ONMIC_READY)
			{
				proominfo->onmic.state = MIC_INFO_ONMIC_READYOK;
			}
			else if(proominfo->onmic.pkstate == MIC_INFO_ONMIC_READYOK)
			{
				proominfo->onmic.state = MIC_INFO_ONMIC;
				proominfo->onmic.pkstate = MIC_INFO_ONMIC;
				//发送给房间内所有的人某人上麦成功的消息
				int seq2 = 0;
				BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
               	char type = 65;
                outstream2->Write(type);
				outstream2->Write((short)ROOM_CMD_SB_ONMIC);
				outstream2->Write(seq2);
				outstream2->Write((int)proominfo->onmic.idx);  
				outstream2->Flush();
				if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2) == -1)
				{
					AC_ERROR("ClientDataDecoder::DoOnMicReadyOK:g_pNetProxy->BroadcastSBCmd error");
					return -1;
				}
			}
			else
			{
				AC_ERROR("ClientDataDecoder::DoOnMicReadyOK:pkstate error");
				return -1;
			}
		}
		else
		{
			if(proominfo->onmic.state == MIC_INFO_ONMIC_READY)
			{
				proominfo->onmic.state = MIC_INFO_ONMIC;

				//上麦超时设置
				//add by jinguanfu 2011/8/10 <begin> 上麦超时
				if(proominfo->m_pOnMicTimeout!=NULL)
				{
					proominfo->m_pOnMicTimeout->UnRegisterTimer();
					proominfo->m_pOnMicTimeout->m_idx=proominfo->onmic.idx;
					proominfo->m_pOnMicTimeout->m_pClientDataDecoder=this;
					proominfo->m_pOnMicTimeout->m_type = TIMEOUT_ONMIC;
					int onmictimeout=proominfo->onmic.musictime+MUSIC_OFFSET;
					proominfo->m_pOnMicTimeout->RegisterTimer(onmictimeout);	
				}
				else
				{
					//初始化上麦超时对象
					proominfo->m_pOnMicTimeout=g_pNetProxy->CreateRoomTimeout();
					if(proominfo->m_pOnMicTimeout!=NULL)
					{
						proominfo->m_pOnMicTimeout->SetReactor(g_pNetProxy->GetReactor());
						proominfo->m_pOnMicTimeout->m_roomid=proominfo->roomid;

						proominfo->m_pOnMicTimeout->UnRegisterTimer();
						proominfo->m_pOnMicTimeout->m_idx=proominfo->onmic.idx;
						proominfo->m_pOnMicTimeout->m_pClientDataDecoder=this;
						proominfo->m_pOnMicTimeout->m_type = TIMEOUT_ONMIC;
						int onmictimeout=proominfo->onmic.musictime+MUSIC_OFFSET;	
						proominfo->m_pOnMicTimeout->RegisterTimer(onmictimeout);
					}
					else
					{
						AC_ERROR("ClientDataDecoder::DoOnMicReadyOK:CreateRoomTimeout error");
						return -1;
					}
				}
				//发送给房间内所有的人某人上麦成功的消息
				int seq2 = 0;
				BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
				char type = 65;
				outstream2->Write(type);
				outstream2->Write((short)ROOM_CMD_SB_ONMIC);
				outstream2->Write(seq2);
				outstream2->Write((int)proominfo->onmic.idx);  
				outstream2->Flush();
				if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2) == -1)
				{
					AC_ERROR("ClientDataDecoder::DoOnMicReadyOK:g_pNetProxy->BroadcastSBCmd error");
					return -1;
				}

				//add by jinguanfu 2011/1/25
				//发送状态到日志服务器
				BinaryWriteStream* logstream=StreamFactory::InstanceWriteStream();
				logstream->Write((short)CMD_ROOM_ONMIC_R2LS);
				logstream->Write(0);
				logstream->Write(proominfo->onmic.idx); 	//用户idx
				logstream->Write(proominfo->roomid); 		//所在房间ID
				logstream->Flush();
						
				BackClientSocketBase *pLogSvr = g_pNetProxy->GetLogServer();
				if(pLogSvr!=NULL)
				{
					if(pLogSvr->AddBuf(logstream->GetData(), logstream->GetSize())==-1) 
					{
						AC_ERROR("ClientDataDecoder::DoOnMicReadyOK:pLogSvr->AddBuf Error");
					}else
					{
						AC_DEBUG("ClientDataDecoder::DoOnMicReadyOK:Send to LogSvr success,idx=%d roomid=%d",proominfo->onmic.idx,proominfo->roomid);
					}
					
				}
				else
				{
					AC_ERROR("ClientDataDecoder::DoOnMicReadyOK:pLogSvr is NULL");
				}

				AC_DEBUG("ClientDataDecoder::DoOnMicReadyOK: idx=%d onmic",pRoomClient->m_idx);
				
			}
			else
			{
				AC_ERROR("ClientDataDecoder::DoOnMicReadyOK:state error");
				return -1;
			}

		}
	}
	else if(pRoomClient->m_idx == proominfo->onmic.pkidx)
	{
        if(proominfo->onmic.idx == 0)
        {
            proominfo->onmic.pkstate = MIC_INFO_ONMIC;
            //发送给房间内所有的人某人上麦成功的消息
			int seq2 = 0;
			BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
			char type = 65;
			outstream2->Write(type);
			outstream2->Write((short)ROOM_CMD_SB_ONMIC);
			outstream2->Write(seq2);
			outstream2->Write((int)proominfo->onmic.pkidx);  
			outstream2->Flush();
			if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2) == -1)
			{
				AC_ERROR("ClientDataDecoder::DoOnMicReadyOK:g_pNetProxy->BroadcastSBCmd error");
				return -1;
			}
        }
        else
        {
			if(proominfo->onmic.state == MIC_INFO_ONMIC_READY)
			{
				proominfo->onmic.pkstate = MIC_INFO_ONMIC_READYOK;
			}
			else if(proominfo->onmic.state == MIC_INFO_ONMIC_READYOK)
			{
				proominfo->onmic.state = MIC_INFO_ONMIC;
				proominfo->onmic.pkstate = MIC_INFO_ONMIC;
				//发送给房间内所有的人某人上麦成功的消息
				int seq2 = 0;
				//char outbuf[256]64] = {0};
				BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
				//BinaryWriteStream outstream2(outbuf2,sizeof(outbuf2));
                char type = 65;
                outstream2->Write(type);
				outstream2->Write((short)ROOM_CMD_SB_ONMIC);
				outstream2->Write(seq2);
				outstream2->Write((int)proominfo->onmic.idx);  
				outstream2->Flush();
				if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2) == -1)
				{
					AC_ERROR("ClientDataDecoder::DoOnMicReadyOK:g_pNetProxy->BroadcastSBCmd error");
					return -1;
				}
			}
			else
			{
				AC_ERROR("ClientDataDecoder::DoOnMicReadyOK:state error");
				return -1;
			}
        }
	}
	else
	{
		MIC_INFO*  pmicinfo;
		vector<MIC_INFO*>::iterator itm = proominfo->miclist.begin();
		if(itm!=proominfo->miclist.end())
		{
			pmicinfo=(MIC_INFO*)*itm;
			AC_DEBUG("ClientDataDecoder::DoOnMicReadyOK:next onmic idx=%d,roomid=%d",pmicinfo->idx,proominfo->roomid);
		}
		AC_ERROR("ClientDataDecoder::DoOnMicReadyOK:client idx error,clientidx=%d,readyidx=%d,roomid=%d",pRoomClient->m_idx,proominfo->onmic.idx,proominfo->roomid);
		return -1;
	}
	return 0;
}

//下麦准备流程已取消
int ClientDataDecoder::DoOffMicReady(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoOffMicReady:pinstream = %x cmd = %d seq = %d", pinstream, cmd, seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	
	AC_DEBUG("ClientDataDecoder::DoOffMicReady:idx=%d ",pRoomClient->m_idx);
	
	if(pRoomClient->m_idx == 0)
	{
		AC_ERROR("ClientDataDecoder::DoOffMicReady:idx error");
		return -1;
	}

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoOffMicReady:roominfo error");
		return -1;
	}

	if(proominfo->onmic.idx == 0 && proominfo->onmic.pkidx == 0)
	{
		AC_ERROR("ClientDataDecoder::DoOffMicReady:proominfo->onmic error");
		return -1;
	}

	if(pRoomClient->m_idx == proominfo->onmic.idx)
	{
		if(proominfo->onmic.pkidx != 0)
		{
			if(proominfo->onmic.pkstate == MIC_INFO_ONMIC)
			{
				proominfo->onmic.state = MIC_INFO_OFFMIC_READY;
			}
			else if(proominfo->onmic.pkstate == MIC_INFO_OFFMIC_READY)
			{
				proominfo->onmic.state = MIC_INFO_OFFMIC_READYOK;
				proominfo->onmic.pkstate = MIC_INFO_OFFMIC_READYOK;
				//发送给房间内所有的人下麦准备完成,可以给服务器发送打分结果了
				int seq2 = 0;
				BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
                		char type = 65;
                		outstream2->Write(type);
				outstream2->Write((short)ROOM_CMD_SB_OFFMIC_READYOK);
				outstream2->Write(seq2);
				outstream2->Write((int)proominfo->onmic.idx);  
				outstream2->Flush();
				if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2) == -1)
				{
					AC_ERROR("ClientDataDecoder::DoOffMicReady:g_pNetProxy->BroadcastSBCmd error");
					return -1;
				}
			}
			else 
			{
				AC_ERROR("ClientDataDecoder::DoOffMicReady:pkstate error");
				return -1;
			}
		}
		else 
		{
			proominfo->onmic.state = MIC_INFO_OFFMIC_READYOK;
			//发送给房间内所有的人下麦准备完成,可以给服务器发送打分结果了
			int seq2 = 0;
			BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
            		char type = 65;
            		outstream2->Write(type);
			outstream2->Write((short)ROOM_CMD_SB_OFFMIC_READYOK);
			outstream2->Write(seq2);
			outstream2->Write((int)proominfo->onmic.idx);  
			outstream2->Flush();
			if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2) == -1)
			{
				AC_ERROR("ClientDataDecoder::DoOffMicReady:g_pNetProxy->BroadcastSBCmd error");
				return -1;
			}
		}
	}
	else if(pRoomClient->m_idx == proominfo->onmic.pkidx)
	{
        if(proominfo->onmic.idx == 0)
        {
		proominfo->onmic.pkstate = MIC_INFO_OFFMIC_READYOK;
		//发送给房间内所有的人下麦准备完成,可以给服务器发送打分结果了
		int seq2 = 0;
		BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
		char type = 65;
		outstream2->Write(type);
		outstream2->Write((short)ROOM_CMD_SB_OFFMIC_READYOK);
		outstream2->Write(seq2);
		outstream2->Write((int)proominfo->onmic.pkidx);  
		outstream2->Flush();
		if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2) == -1)
		{
			AC_ERROR("ClientDataDecoder::DoOffMicReady:g_pNetProxy->BroadcastSBCmd error");
			return -1;
		}
        }
        else
        {
			if(proominfo->onmic.state == MIC_INFO_ONMIC)
			{
				proominfo->onmic.pkstate = MIC_INFO_OFFMIC_READY;
			}
			else if(proominfo->onmic.state == MIC_INFO_OFFMIC_READY)
			{
				proominfo->onmic.state = MIC_INFO_OFFMIC_READYOK;
				proominfo->onmic.pkstate = MIC_INFO_OFFMIC_READYOK;
				//发送给房间内所有的人下麦准备完成,可以给服务器发送打分结果了
				int seq2 = 0;
				//char outbuf[256]64] = {0};
				BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
				//BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
                		char type = 65;
                		outstream2->Write(type);
				outstream2->Write((short)ROOM_CMD_SB_OFFMIC_READYOK);
				outstream2->Write(seq2);
				outstream2->Write((int)proominfo->onmic.idx);  
				outstream2->Flush();
				if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2) == -1)
				{
					AC_ERROR("ClientDataDecoder::DoOffMicReady:g_pNetProxy->BroadcastSBCmd error");
					return -1;
				}
			}
			else
			{
				AC_ERROR("ClientDataDecoder::DoOffMicReady:state error");
				return -1;
			}
        }
	}
	else
	{
		AC_ERROR("ClientDataDecoder::DoOffMicReady:client idx error");
		return -1;
	}

	return 0;
}

int ClientDataDecoder::DoScore(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoScore:pinstream = %x cmd = %d seq = %d", pinstream, cmd, seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	
	AC_DEBUG("ClientDataDecoder::DoScore:roomid=%d,idx=%d ",pRoomClient->m_roomid,pRoomClient->m_idx);
	
	if(pRoomClient->m_idx == 0)
	{
		AC_ERROR("ClientDataDecoder::DoScore:idx error");
		return -1;
	}

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoScore:roominfo error");
		return -1;
	}

	if(proominfo->onmic.idx == 0 && proominfo->onmic.pkidx == 0)
	{
		AC_ERROR("ClientDataDecoder::DoScore:proominfo->onmic error");
		return 0;
	}

	if(proominfo->onmic.idx != pRoomClient->m_idx && proominfo->onmic.pkidx != pRoomClient->m_idx)
	{
		AC_ERROR("ClientDataDecoder::DoScore:pRoomClient->m_idx error");
		return 0;
	}


	//接收打分信息结构体
	char context[128] = {0};
	size_t len = 0;
	if(!pinstream->Read(context, sizeof(context)-1, len)) 
	{
		AC_ERROR("ClientDataDecoder::DoScore:read context error");
		return -1;
	}

	if(len != sizeof(NETSINGSCORE))
	{
		AC_ERROR("ClientDataDecoder::DoScore:context error,len=%d",len);
		return -1;
	}

	//发送给校验服务器校验
	if(proominfo->onmic.pkidx == 0)//如果是非PK麦
	{
		if(proominfo->onmic.idx != pRoomClient->m_idx)
		{
			AC_ERROR("ClientDataDecoder::DoScore:pRoomClient->m_idx error no pk");
			return -1;
		}

		proominfo->onmic.state = MIC_INFO_OFFMIC;
		
		strncpy(proominfo->onmic.scoreinfo, context, sizeof(context));	//add by jinguanfu 2010/6/5

		//演唱分数发送到数据库转换成经验
		NETSINGSCORE  singscore;

		memcpy(&singscore,context,sizeof(NETSINGSCORE));

		int expchange=0;					//获得经验值
		
		if(singscore.score!=0)
		{
			//add by jinguanfu 2011/8/11
			//计算获得经验值
			/*  经验值换算公式
			*  1、防沉迷用户
			*	   当天在线时长3小时内全额经验
			*	   当天在线时长超过3小时经验减半
			*	   当天在线时长超过5小时经验无
			*  2、非防沉迷用户
			*	   经验值全额计算，不受在线时长限制
			*/
			if(pRoomClient->m_status==USER_STATUS_JUST||pRoomClient->m_status==USER_STATUS_GOOD)
			{
				expchange=singscore.score;
			}
			else
			{
			
				int curtime=(int)time(NULL);		//当前时间
				//总在线时长= 登陆时在线时长+本次登陆在线时长
				int countonline=pRoomClient->m_onlinetime+(curtime-pRoomClient->m_logintime);

				AC_DEBUG("ClientDataDecoder::DoScore:Onlinetime=%d,curtime=%d,logintime=%d",
					pRoomClient->m_onlinetime,curtime,pRoomClient->m_logintime);
				
				if(countonline<10800)
					expchange=singscore.score;
				else if(countonline>=10800&&countonline<18000)
					expchange=singscore.score/2;
				else
					expchange =0;
				

			}
			//数据库同步打分信息和经验值
			BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
			if(pDBSvr!=NULL)
			{
				int outseq = g_pNetProxy->GetCounter()->Get();
				BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
				static const char* spname = {"DBMW_SCore_HZSTAR"};//存储过程填充
				outstream->Write((short)CMD_CALLSP);
				outstream->Write(outseq);
				outstream->Write(spname,strlen(spname));
				outstream->Write((short)3);
				outstream->Write((char)PT_INPUT);
				outstream->Write((char)PDT_INT);
				outstream->Write((int)pRoomClient->m_idx);	//演唱者idx
				outstream->Write((char)PT_INPUT);
				outstream->Write((char)PDT_INT);
				outstream->Write((int)proominfo->onmic.musicid);		//演唱曲目
				outstream->Write((char)PT_INPUT);
				outstream->Write((char)PDT_INT);
				outstream->Write((int)singscore.score);		//演唱得分
				outstream->Flush();

				Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
				if(data!=NULL)
				{
					if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1) 
					{
					
						data->roomid = pRoomClient->m_roomid;
						data->opidx = pRoomClient->m_idx;
						data->bopidx= pRoomClient->m_idx;
						data->number=pRoomClient->GetClientID();;
						data->cmd = cmd;
						data->seq=seq;
						data->outseq=outseq;
						data->SetReactor(g_pNetProxy->GetReactor());
						data->RegisterTimer(DB_TIMEOUT);
						g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
					}
					else
					{
						AC_ERROR("ClientDataDecoder::DoScore:pDBSvr->AddBuf() error");
						g_pNetProxy->DestroyDBResult(data);
					}
				}
				else
				{
					AC_ERROR("ClientDataDecoder::DoScore:CreateDBResultdata() error,data=%x",data);
					
				}
			}
			else
			{
				AC_ERROR("ClientDataDecoder::DoScore: pDBSvr=%x",pDBSvr);
			}
		}

		//发送给房间内所有的人下麦通知
		int seq2 = 0;
		char haspk = 0;
		char type=65;
		BinaryWriteStream* outstream = StreamFactory::InstanceWriteStream();
		outstream->Write(type);
		outstream->Write((short)ROOM_CMD_SB_OFFMIC);
		outstream->Write(seq2);
		outstream->Write(haspk);
		outstream->Write((int)proominfo->onmic.idx);					
		outstream->Write(expchange);				//新增经验值add by jinguanfu 2011/8/10
		outstream->Write(context,sizeof(NETSINGSCORE));	//打分信息体modify by lihongwu 2011/12/20	
		outstream->Flush();
		g_pNetProxy->BroadcastSBCmd(proominfo, *outstream);

		
		proominfo->onmic.init();
		AC_DEBUG("ClientDataDecoder::DoScore: roomid=%d,idx=%d is offmic",pRoomClient->m_roomid,pRoomClient->m_idx);
		
		//add by jinguanfu 2011/1/25
		pRoomClient->m_onmicflag = USER_STATE_NONE; 
		//发送状态到日志服务器
		BinaryWriteStream* logstream=StreamFactory::InstanceWriteStream();
		logstream->Write((short)CMD_ROOM_OFFMIC_R2LS);
		logstream->Write(0);
		logstream->Write(pRoomClient->m_idx); 	//用户idx
		logstream->Write(proominfo->roomid); 	//所在房间ID
		logstream->Flush();
						
		BackClientSocketBase *pLogSvr = g_pNetProxy->GetLogServer();
		if(pLogSvr!=NULL)
		{
			if(pLogSvr->AddBuf(logstream->GetData(), logstream->GetSize())==-1) 
			{
				AC_ERROR("ClientDataDecoder::DoScore:pLogSvr->AddBuf Error");
			}else
			{
				AC_DEBUG("ClientDataDecoder::DoScore:Send to LogSvr success,idx=%d roomid=%d",pRoomClient->m_idx,proominfo->roomid);
			}
					
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoScore:pLogSvr is NULL");
		}
	

		roomtimeout* ptb = g_pNetProxy->CreateRoomTimeout();
		if(ptb!=NULL)
		{
	        ptb->m_roomid = pRoomClient->m_roomid;
			ptb->m_idx = pRoomClient->m_idx;
			ptb->m_time = OFFMIC_WAIT; //5s
			ptb->m_type = TIMEOUT_OFFMIC;
			ptb->SetReactor(g_pNetProxy->GetReactor());
			ptb->m_pClientDataDecoder=this;
			ptb->m_haspk = haspk;
			ptb->RegisterTimer(ptb->m_time);
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoScore: CreateRoomTimeout error");

			//CheckMiclistOnmic(proominfo);
			CheckMiclistOnmic2(proominfo);
		}
	
    }
    else
    {
        //PK麦

    }

	return 0;
}

int ClientDataDecoder::DoGiveOffMic(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoGiveOffMic:cmd = %d seq = %d", cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoGiveOffMic:roominfo error");
		return -1;
	}

	int idx = 0;
	if(!pinstream->Read(idx))    //被操作对象的idx
	{
		AC_ERROR("ClientDataDecoder::DoGiveOffMic:read idx error");
		return -1;
	}

	if(idx == 0)
	{
		AC_ERROR("ClientDataDecoder::DoGiveOffMic:idx error");
		return -1;
	}

	AC_DEBUG("ClientDataDecoder::DoGiveOffMic:roomid=%d,pRoomClient->m_idx=%d ,idx=%d ",
		pRoomClient->m_roomid,pRoomClient->m_idx,idx);

	if(idx != proominfo->onmic.idx && idx != proominfo->onmic.pkidx)
    {
		AC_ERROR("ClientDataDecoder::DoGiveOffMic:idx=%d is not onmic ",idx);
		return 0;
	}

	//add by jinguanfu 2010/4/16 权限判断
	map<int,RoomClient*>::iterator itu=proominfo->userlist.find(idx);
	if(itu==proominfo->userlist.end())
	{
		AC_ERROR("ClientDataDecoder::DoGiveOffMic:idx=%d is not in roomid=%d",idx,proominfo->roomid);
        	return 0;
	}
	RoomClient* pbClient = itu->second;
	
	//if(CheckRight(pRoomClient->m_identity, pRoomClient->m_viptype,pbClient->m_identity,pbClient->m_viptype, cmd)==0)
	if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,pbClient->m_identity,pbClient->m_viptype, cmd)==0)
	{
		AC_ERROR("ClientDataDecoder::DoGiveOffMic:power error");
	 	return -1;
	}

	int seq3 = 0;
	BinaryWriteStream* outstream3=StreamFactory::InstanceWriteStream();
	char type = 65;
	outstream3->Write(type);
	outstream3->Write((short)ROOM_CMD_SB_GIVEOFF_MIC);
	outstream3->Write(seq3);
	outstream3->Write((int)idx);  
	outstream3->Flush();

	if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream3) == -1)
	{
		AC_ERROR("ClientDataDecoder::DoGiveOffMic:g_pNetProxy->BroadcastSBCmd error");
		return -1;
	}
	
	//add by jinguanfu 2011/1/25
	map<int,RoomClient*>::iterator itc = proominfo->userlist.find(idx);
	if(itc!=proominfo->userlist.end())
	{
		RoomClient* pRoomClient=(RoomClient*)itc->second;
		pRoomClient->m_onmicflag = USER_STATE_NONE;	
	}

	//发送状态到日志服务器
	BinaryWriteStream* logstream=StreamFactory::InstanceWriteStream();
	logstream->Write((short)CMD_ROOM_OFFMIC_R2LS);
	logstream->Write(0);
	logstream->Write(idx); 					//用户idx
	logstream->Write(proominfo->roomid); 	//所在房间ID
	logstream->Flush();
						
	BackClientSocketBase *pLogSvr = g_pNetProxy->GetLogServer();
	if(pLogSvr!=NULL)
	{
		if(pLogSvr->AddBuf(logstream->GetData(), logstream->GetSize())==-1) 
		{
			AC_ERROR("ClientDataDecoder::DoGiveOffMic:pLogSvr->AddBuf Error");
		}else
		{
			AC_DEBUG("ClientDataDecoder::DoGiveOffMic:Send to LogSvr success,idx=%d roomid=%d",idx,proominfo->roomid);
		}
					
	}
	else
	{
		AC_ERROR("ClientDataDecoder::DoGiveOffMic:pLogSvr is NULL");
	}
	

	if(idx == proominfo->onmic.idx)
    {
		if(proominfo->onmic.pkidx != 0)
		{
	            proominfo->onmic.idx = 0;
	            proominfo->onmic.musicid = 0;
	            proominfo->onmic.bk = 0;
	            proominfo->onmic.score = 0;
	            proominfo->onmic.state = MIC_INFO_WAIT;
		}
        else
        {
        		//add by jinguanfu 2011/8/12
        		//取消上麦超时
        		if(proominfo->m_pOnMicTimeout!=NULL)
        			proominfo->m_pOnMicTimeout->UnRegisterTimer();
				
	            //发送给房间内所有的人下麦消息
	            int seq2 = 0;
	            char haspk = 0;
	            BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
	            outstream->Write(type);
	            outstream->Write((short)ROOM_CMD_SB_OFFMIC);
	            outstream->Write(seq2);
	            outstream->Write(haspk);
	            outstream->Write((int)idx);  
	            outstream->Write((int)0);		//新增经验值add by jinguanfu 2011/8/10
	            outstream->Write(0);				//打分信息体
	            outstream->Flush();

	            if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream) == -1)
	            {
	                AC_ERROR("ClientDataDecoder::DoGiveOffMic:g_pNetProxy->BroadcastSBCmd error");
	                return -1;
	            }
				/* del by jinguanfu 2011/8/12
				//add by jinguanfu 2010/10/28
				outstream ->Clear();
				outstream->Write(type);
				outstream->Write((short)ROOM_CMD_SB_SCORE);
				outstream->Write(0);			//seq
				outstream->Write(0);			//当前经验增加值
				outstream->Write(0);			//当前银币增加值
				outstream->Flush();
				g_pNetProxy->BroadcastSBCmd(proominfo, *outstream);
				*/
		        proominfo->onmic.init();

				//5秒后下一个人上麦
				//roomtimeout* ptb = g_pNetProxy->GettoMgr()->CreateClient();
				roomtimeout* ptb = g_pNetProxy->CreateRoomTimeout();
				if(ptb!=NULL)
				{
					ptb->m_roomid = pRoomClient->m_roomid;
			        ptb->m_idx = pRoomClient->m_idx;
					ptb->m_time = OFFMIC_WAIT; //5s
					ptb->m_type = TIMEOUT_OFFMIC;
					ptb->SetReactor(g_pNetProxy->GetReactor());
					ptb->m_pClientDataDecoder=this;
					ptb->m_haspk = haspk;
					ptb->RegisterTimer(ptb->m_time);
				}
				else
				{
					AC_ERROR("ClientDataDecoder::DoGiveOffMic:CreateRoomTimeout error");
					//CheckMiclistOnmic(proominfo);
					CheckMiclistOnmic2(proominfo);
				}
		}
	}
    else if(idx == proominfo->onmic.pkidx)
	{
		if(proominfo->onmic.idx != 0)
		{
	            //发送给房间内所有的人下麦消息
	            int seq2 = 0;
	            //char outbuf[256]64] = {0};
	            char haspk = 0;
	            BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
	            outstream2->Write(type);
	            outstream2->Write((short)ROOM_CMD_SB_OFFMIC);
	            outstream2->Write(seq2);
	            outstream2->Write(haspk);
	            outstream2->Write((int)idx);  
	            outstream2->Write((int)0);		//新增经验值add by jinguanfu 2011/8/10
	            outstream2->Write(0);			//打分信息体
	            outstream2->Flush();
	            if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2) == -1)
	            {
	                AC_ERROR("ClientDataDecoder::DoGiveOffMic:g_pNetProxy->BroadcastSBCmd error");
	                return -1;
	            }
				//add by jinguanfu 2010/10/28
				outstream2 ->Clear();
				outstream2->Write(type);
				outstream2->Write((short)ROOM_CMD_SB_SCORE);
				outstream2->Write(0);			//seq
				outstream2->Write(0);			//当前经验增加值
				outstream2->Write(0);			//当前银币增加值
				outstream2->Flush();
				g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2);
			
	            proominfo->onmic.init();

	            //检查排麦列表有人就发生上麦ready的消息
	            //if(CheckMiclistOnmic(proominfo) == -1)
				if(CheckMiclistOnmic2(proominfo) == -1)
	            {
	                AC_ERROR("ClientDataDecoder::DoGiveOffMic:CheckMiclistOnmic error");
	                return -1;
	            }
        	}
		else
		{
	            proominfo->onmic.pkidx = 0;
	            proominfo->onmic.pkmusicid = 0;
	            proominfo->onmic.pkbk = 0;
	            proominfo->onmic.pkscore = 0;
	            proominfo->onmic.pkstate = MIC_INFO_WAIT;
		}
	}
	else
	{
		AC_ERROR("ClientDataDecoder::DoGiveOffMic:idx error");
		return 0;
	}

	return 0;
}

int ClientDataDecoder::DoKickSb(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
    AC_DEBUG("ClientDataDecoder::DoKickSb:cmd = %d seq = %d", cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoKickSb:roominfo error");
		return -1;
	}
	
	int idx = 0;
	if(!pinstream->Read(idx))    //对象的idx
	{
		AC_ERROR("ClientDataDecoder::DoKickSb:read idx error");
		return -1;
	}

	if(idx == 0)
	{
		AC_ERROR("ClientDataDecoder::DoKickSb:idx error");
		return -1;
	}
	
	AC_DEBUG("ClientDataDecoder::DoKickSb:roomid=%d,pRoomClient->m_idx=%d ,idx=%d ",
		pRoomClient->m_roomid,pRoomClient->m_idx,idx);

	map<int, RoomClient*>::iterator itu = proominfo->userlist.find(idx);
	if(itu != proominfo->userlist.end())
	{
		RoomClient* pbclient = (RoomClient*)((*itu).second);

		//if(CheckRight(pRoomClient->m_identity, pRoomClient->m_viptype,pbclient->m_identity,pbclient->m_viptype, cmd)==0)
		if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,pbclient->m_identity,pbclient->m_viptype, cmd)==0)
		{
			AC_ERROR("ClientDataDecoder::DoKickSb:power error");
		 	return -1;
		}

		//临时黑名单
		roomtimeout* ptb = g_pNetProxy->CreateRoomTimeout();
		if(ptb!=NULL)
		{
			ptb->m_roomid = proominfo->roomid;
			ptb->m_idx = idx;
			ptb->m_time = 10*60;//10 min modify by wangpf 2010/07/30
			ptb->m_type = TIMEOUT_TB;
			ptb->SetReactor(g_pNetProxy->GetReactor());
			ptb->m_pClientDataDecoder=this;
			ptb->RegisterTimer(ptb->m_time);
			proominfo->tblacklist.insert(make_pair(idx, idx));
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoKickSb:CreateRoomTimeout error");
			return -1;
		}
		//发送给房间内所有的人,有人被踢出房间
		int seq2 = 0;
		BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
		char type = 65;
		outstream2->Write(type);
		outstream2->Write((short)ROOM_CMD_SB_KICKOUT);
		outstream2->Write(seq2);
		outstream2->Write((int)idx);				//被踢者
		outstream2->Write((int)pRoomClient->m_idx); //操作者
		outstream2->Flush();
		if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2, 1) == -1)
		{
			AC_ERROR("ClientDataDecoder::DoKickSb:g_pNetProxy->BroadcastSBCmd error");
			return -1;
		}


		//踢出房间
		pbclient->LeaveRoom();
	}

	map<int, RoomClient*>::iterator itvu = proominfo->vuserlist.find(idx);
	if(itvu != proominfo->vuserlist.end())
	{
		RoomClient* pbclient = (RoomClient*)((*itvu).second);

		//if(CheckRight(pRoomClient->m_identity, pRoomClient->m_viptype,pbclient->m_identity,pbclient->m_viptype, cmd)==0)
		if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,pbclient->m_identity,pbclient->m_viptype, cmd)==0)
		{
			AC_ERROR("ClientDataDecoder::DoKickSb:power error");
		 	return -1;
		}


		//临时黑名单
		roomtimeout* ptb = g_pNetProxy->CreateRoomTimeout();
		if(ptb!=NULL)
		{
			ptb->m_roomid = proominfo->roomid;
			ptb->m_idx = idx;
			ptb->m_time = 10*60;//10 min modify by wangpf 2010/07/30
			ptb->m_type = TIMEOUT_TB;
			ptb->SetReactor(g_pNetProxy->GetReactor());
			ptb->m_pClientDataDecoder=this;
			ptb->RegisterTimer(ptb->m_time);
			proominfo->tblacklist.insert(make_pair(idx, idx));
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoKickSb:CreateRoomTimeout error");
			return -1;
		}
		//发送给房间内所有的人,有人被踢出房间
		int seq2 = 0;
		BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
		char type = 65;
		outstream2->Write(type);
		outstream2->Write((short)ROOM_CMD_SB_KICKOUT);
		outstream2->Write(seq2);
		outstream2->Write((int)idx);				//被踢者
		outstream2->Write((int)pRoomClient->m_idx); //操作者
		outstream2->Flush();
		if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2, 1) == -1)
		{
			AC_ERROR("ClientDataDecoder::DoKickSb:g_pNetProxy->BroadcastSBCmd error");
			return -1;
		}
				

		//踢出房间
		pbclient->VLeaveRoom();
		proominfo->vuserlist.erase(itvu);
	}

	return 0;
}

int ClientDataDecoder::DoForbidenSb(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoForbidenSb:cmd = %d seq = %d", cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoForbidenSb:roominfo error");
		return -1;
	}

	int idx = 0;
	if(!pinstream->Read(idx))    //对象的idx
	{
		AC_ERROR("ClientDataDecoder::DoForbidenSb:read idx error");
		return -1;
	}

	if(idx == 0)
	{
		AC_ERROR("ClientDataDecoder::DoForbidenSb:idx error");
		return -1;
	}

	AC_DEBUG("ClientDataDecoder::DoForbidenSb:roomid=%d,pRoomClient->m_idx=%d ,idx=%d ",
		pRoomClient->m_roomid,pRoomClient->m_idx,idx);

	char bforbiden = 0;
	if(!pinstream->Read(bforbiden)) 
	{
		AC_ERROR("ClientDataDecoder::DoForbidenSb:read bforbiden error");
		return -1;
	}

	RoomClient* pFobidenClient=NULL;
	//只能对房间内用户进行禁言
	map<int, RoomClient*>::iterator itu = proominfo->userlist.find(idx);
	map<int, RoomClient*>::iterator itvu = proominfo->vuserlist.find(idx);
	if(itu == proominfo->userlist.end()&&itvu==proominfo->vuserlist.end())
	{
		AC_ERROR("ClientDataDecoder::DoForbidenSb:idx=%d is not in room=%d",idx,proominfo->roomid);
		return 0;
	}
	else if(itu != proominfo->userlist.end())
	{
		pFobidenClient = (*itu).second; 
	}
	else if(itvu != proominfo->vuserlist.end())
	{
		pFobidenClient = (*itvu).second; 
	}


	//if(CheckRight(pRoomClient->m_identity, pRoomClient->m_viptype,pFobidenClient->m_identity,pFobidenClient->m_viptype, cmd)==0)
	if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,pFobidenClient->m_identity,pFobidenClient->m_viptype, cmd)==0)
	{
		AC_ERROR("ClientDataDecoder::DoForbidenSb:power error");
	 	return -1;
	}
	//设置禁言
	if(bforbiden)
	{
		map<int,roomtimeout*>::iterator ittimeout=proominfo->forbidenlist.find(idx);
		if(ittimeout==proominfo->forbidenlist.end())
		{
			//roomtimeout* ptb = g_pNetProxy->GettoMgr()->CreateClient();
			roomtimeout* ptb = g_pNetProxy->CreateRoomTimeout();
			if(ptb!=NULL)
			{
			   	ptb->m_roomid = pFobidenClient->m_roomid;
			   	ptb->m_idx = idx;
				ptb->m_time = 5*60;//禁言时效5分钟
				ptb->m_type = TIMEOUT_FORBIDEN;
				ptb->SetReactor(g_pNetProxy->GetReactor());
				ptb->m_pClientDataDecoder=this;
				ptb->RegisterTimer(ptb->m_time);
				proominfo->forbidenlist.insert(make_pair(idx, ptb));
			}
			else
			{
				AC_ERROR("ClientDataDecoder::DoForbidenSb:CreateRoomTimeout error");
			 	return -1;
			}
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoForbidenSb:idx[%d] will be forbidened");
			return 0;
		}
	}
	//取消禁言
	else
	{
		map<int, roomtimeout*>::iterator itr = proominfo->forbidenlist.find(idx);
		if(itr != proominfo->forbidenlist.end())
		{
			roomtimeout* ptb=(*itr).second;
			//禁言取消
			ptb->UnRegisterTimer();
			ptb->RegisterTimer(1);		// 1秒后消除禁言
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoForbidenSb:idx[%d] is not in forbidenlist",pFobidenClient->m_idx);
			return 0;
		}
	}
	//发送给房间内所有的人,有人被禁言/取消禁言
	//char outbuf[256]64] = {0};
	BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
	char type = 65;
	outstream2->Write(type);
	outstream2->Write((short)ROOM_CMD_SB_FORBIDEN);
	outstream2->Write((int)0);
	outstream2->Write((int)idx);  
	outstream2->Write(bforbiden);
	outstream2->Flush();
	if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2) == -1)
	{
		AC_ERROR("ClientDataDecoder::DoForbidenSb:g_pNetProxy->BroadcastSBCmd error");
		return -1;
	}

	return 0;
}

int ClientDataDecoder::DoUpdateBlacklist(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoUpdateBlacklist:cmd = %d seq = %d", cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoUpdateBlacklist:roominfo error");
		return -1;
	}

	int idx = 0;
	if(!pinstream->Read(idx))    //对象的idx
	{
		AC_ERROR("ClientDataDecoder::DoUpdateBlacklist:read idx error");
		return -1;
	}

	if(idx == 0)
	{
		AC_ERROR("ClientDataDecoder::DoUpdateBlacklist:idx error");
		return -1;
	}
		
	
	AC_DEBUG("ClientDataDecoder::DoUpdateBlacklist:roomid=%d,pRoomClient->m_idx=%d ,idx=%d ",
		pRoomClient->m_roomid,pRoomClient->m_idx,idx);

	map<int, RoomClient*>::iterator itu = proominfo->userlist.find(idx);
	//被操作者不在房间，则取得其身份
	if(itu==proominfo->userlist.end())
	{
		char b_identity=USER_ID_NONE;	//被操作者权限
			
		map<int, GM_INFO>::iterator itmGMU =g_pNetProxy->m_GM.find(idx);	
		map<int, int>::iterator itvj = proominfo->vjlist.find(idx);
		map<int, int>::iterator itvj_a = proominfo->vjlist_a.find(idx);
		map<int, int>::iterator itVIP = proominfo->userlistVIP.find(idx);
		if(itmGMU!=g_pNetProxy->m_GM.end())
				b_identity=USER_ID_GM;
		else if(idx==proominfo->ownidx)
				b_identity=USER_ID_OWNER;
		else if(idx==proominfo->secondownidx||idx==proominfo->secondownidx2)
				b_identity= USER_ID_OWNER_S;
		else if(itvj!=proominfo->vjlist.end())
				b_identity=USER_ID_VJ;
		else if(itvj_a!=proominfo->vjlist_a.end())
				b_identity = USER_ID_VJ_A;
		else if(itVIP!=proominfo->userlistVIP.end())
				b_identity = USER_ID_VIP;
		
		//if(CheckRight(pRoomClient->m_identity, pRoomClient->m_viptype,b_identity,VIP_LEVEL_NONE, cmd)==0)
		if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,b_identity,VIP_LEVEL_NONE, cmd)==0)
		{
			AC_ERROR("ClientDataDecoder::DoUpdateBlacklist:power error");
		 	return 0;
		}
		
	}
	else
	{
		RoomClient* pbClient=itu->second;

		//if(CheckRight(pRoomClient->m_identity, pRoomClient->m_viptype,pbClient->m_identity,pbClient->m_viptype, cmd)==0)
		if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,pbClient->m_identity,pbClient->m_viptype, cmd)==0)
		{
			AC_ERROR("ClientDataDecoder::DoUpdateBlacklist:power error");
		 	return 0;
		}
	}
	
	char badd = 0; //0 == delete, 1 == add
	if(!pinstream->Read(badd)) 
	{
		AC_ERROR("ClientDataDecoder::DoUpdateBlacklist:read badd error");
		return -1;
	}

	/****add by jinguanfu 2010/4/9 ****/
	//房间黑名单不能超过1000
	if(badd==1&&proominfo->blacklist.size()>=1000)
	{
		BinaryWriteStream* outstream1=StreamFactory::InstanceWriteStream();
		char type = 65;
		outstream1->Write(type);
		outstream1->Write(cmd);
		outstream1->Write(seq);
		outstream1->Write((int)LISTFULL);
		outstream1->Write(idx);
		outstream1->Write(badd);
		outstream1->Flush();
		if(g_pNetProxy->SendToSrv(pClient,*outstream1)==-1)
		{
			AC_ERROR("ClientDataDecoder::DoUpdateBlacklist:SendToSrv error");
			return -1;
		}
		return 0;
	}
	/****add by jinguanfu 2011/2/23 *****/
	map<int,int>::iterator itblack=proominfo->blacklist.find(idx);
	//添加黑名单时已存在或删除黑名单时不存在
	if((itblack!=proominfo->blacklist.end()&&badd==1)
		||((itblack==proominfo->blacklist.end()&&badd==0)))
	{
		BinaryWriteStream* outstream1=StreamFactory::InstanceWriteStream();
		char type = 65;
		outstream1->Write(type);
		outstream1->Write(cmd);
		outstream1->Write(seq);
		outstream1->Write((int)ALREADY);
		outstream1->Write(idx);
		outstream1->Write(badd);
		outstream1->Flush();
		if(g_pNetProxy->SendToSrv(pClient,*outstream1)==-1)
		{
			AC_ERROR("ClientDataDecoder::DoUpdateBlacklist:SendToSrv error");
			return -1;
		}
	}
	

	BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
	if(pDBSvr!=NULL)
	{
		//发送给dbagent
		int outseq = g_pNetProxy->GetCounter()->Get();
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		static const char* spname = {"DBMW_UpdateMblacklist_byroomid"};
		outstream->Write((short)CMD_CALLSP);
		outstream->Write(outseq);
		outstream->Write(spname,strlen(spname));
		outstream->Write((short)3);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)pRoomClient->m_roomid);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)idx);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)badd);
		outstream->Flush();

		Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
		if(data!=NULL)
		{
			if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1)
			{
				data->roomid = pRoomClient->m_roomid;
			    data->opidx = pRoomClient->m_idx;
			    data->bopidx = idx;
			    data->cmd = cmd;
			    data->badd = badd;
			    data->identity = 0;
				data->seq=seq;
				data->outseq=outseq;
				data->number = pRoomClient->GetClientID();
				data->SetReactor(g_pNetProxy->GetReactor());
				data->RegisterTimer(DB_TIMEOUT);	
				g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
			}
			else
			{
				AC_ERROR("ClientDataDecoder::DoUpdateBlacklist:pDBSvr->AddBuf() error");			
				g_pNetProxy->DestroyDBResult(data);
			}
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoUpdateBlacklist:CreateDBResultdata() error data =%x",data);
		}
	}
	else
	{
		AC_ERROR("ClientDataDecoder::DoUpdateBlacklist:pDBSvr =%x",pDBSvr);

	}
	return 0;
}
/* delete by jinguanfu 2010/8/27
int ClientDataDecoder::DoUpdateMgr(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoUpdateMgr:cmd = %d seq = %d", cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoUpdateMgr:roominfo error");
		return -1;
	}

	map<int, int>::iterator itvj = proominfo->vjlist.find(pRoomClient->m_idx);
	if(pRoomClient->m_idx != proominfo->ownidx && itvj == proominfo->vjlist.end())
	{
		AC_ERROR("ClientDataDecoder::DoUpdateMgr:power error");
		return -1;
	}

	int idx = 0;
	if(!pinstream->Read(idx))    //对象的idx
	{
		AC_ERROR("ClientDataDecoder::DoUpdateMgr:read idx error");
		return -1;
	}

	if(idx == 0)
	{
		AC_ERROR("ClientDataDecoder::DoUpdateMgr:idx error");
		return -1;
	}

	char badd = 0; //0 == delete, 1 == add
	if(!pinstream->Read(badd)) 
	{
		AC_ERROR("ClientDataDecoder::DoUpdateMgr:read badd error");
		return -1;
	}

	char identity = USER_ID_NONE;
	if(!pinstream->Read(identity)) 
	{
		AC_ERROR("ClientDataDecoder::DoUpdateMgr:read identity error");
		return -1;
	}

	//发送给dbagent
	//char outbuf[256]128] = {0};
    	int outseq = g_pNetProxy->GetCounter()->Get();
    	BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
    	static const char* spname = {"DBMW_UpdateMAdmin_byroomid"};
    	outstream->Write((short)CMD_CALLSP);
    	outstream->Write(outseq);
    	outstream->Write(spname,strlen(spname));
    	outstream->Write((short)4);
    	outstream->Write((char)PT_INPUT);
    	outstream->Write((char)PDT_INT);
    	outstream->Write((int)pRoomClient->m_roomid);
    	outstream->Write((char)PT_INPUT);
    	outstream->Write((char)PDT_INT);
   	outstream->Write((int)idx);
    	outstream->Write((char)PT_INPUT);
    	outstream->Write((char)PDT_INT);
    	outstream->Write((int)identity);
    	outstream->Write((char)PT_INPUT);
    	outstream->Write((char)PDT_INT);
    	outstream->Write((int)badd);
    	outstream->Flush();

    	BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
    	if(pDBSvr != NULL)
        	pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize());
	
	Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
	data->roomid = (int)pRoomClient->m_roomid;
   	data->opidx = (int)pRoomClient->m_idx;
    	data->bopidx = idx;
    	data->cmd = cmd;
	data->badd = badd;
	data->identity = identity;
	data->seq=outseq;
	data->SetReactor(g_pNetProxy->GetReactor());
	data->RegisterTimer(DB_TIMEOUT);	
    	g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));

	return 0;
}
*/
int ClientDataDecoder::DoUpdateContent(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoUpdateContent:cmd = %d seq = %d", cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoUpdateContent:roominfo error");
		return -1;
	}

	char content[301] = {0};
	size_t len = 0;
	if(!pinstream->Read(content, sizeof(content)-1, len))   
	{
		AC_ERROR("ClientDataDecoder::DoUpdateContent:read content error");
		return -1;
	}
	content[len]=0;
	AC_DEBUG("ClientDataDecoder::DoUpdateContent:roomid=%d,idx=%d, content len=%d",
		pRoomClient->m_roomid,pRoomClient->m_idx,len);
	/*
	if(len == 0)
	{
		AC_ERROR("ClientDataDecoder::DoUpdateContent:len error");
		return -1;
	}
	*/
	//if(CheckRight(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
	if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
	{
		AC_ERROR("ClientDataDecoder::DoUpdateContent:power error");
	 	return 0;
	}

	BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
	if(pDBSvr!=NULL)
	{
		//发送给dbagent
		int outseq = g_pNetProxy->GetCounter()->Get();
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		static const char* spname = {"DBMW_UpdateMRContent_byroomid"};
		outstream->Write((short)CMD_CALLSP);
		outstream->Write(outseq);
		outstream->Write(spname,strlen(spname));
		outstream->Write((short)2);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)pRoomClient->m_roomid);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_VARCHAR);
		outstream->Write(content, strlen(content));
		outstream->Flush();

		Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
		if(data != NULL)
		{
			if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1)
			{
				data->roomid = pRoomClient->m_roomid;
				data->opidx = pRoomClient->m_idx;
				data->bopidx = pRoomClient->m_idx;
				data->cmd = cmd;
				data->badd = 0;
				data->identity = 0;
				memset(data->content,0,sizeof(data->content));
				strncpy(data->content, content,strlen(content));
				data->seq=seq;
				data->outseq=outseq;
				data->number = pRoomClient->GetClientID();
				data->SetReactor(g_pNetProxy->GetReactor());
				data->RegisterTimer(DB_TIMEOUT);
				g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
			}
			else
			{
				AC_ERROR("ClientDataDecoder::DoUpdateContent:pDBSvr->AddBuf() error");
				g_pNetProxy->DestroyDBResult(data);
			}
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoUpdateContent:CreateDBResultdata data=%x",data);
		}
	}
	else
	{
		AC_ERROR("ClientDataDecoder::DoUpdateContent:pDBSvr=%x",pDBSvr);
	}
	return 0;
}

/* delete by jinguanfu 2010/8/27
int ClientDataDecoder::DoPrivateChat(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoPrivateChat:cmd = %d seq = %d", cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoPrivateChat:roominfo error");
		return -1;
	}

	int idx = 0;
	if(!pinstream->Read(idx))    //对象的idx
	{
		AC_ERROR("ClientDataDecoder::DoPrivateChat:read idx error");
		return -1;
	}

	if(idx == 0)
	{
		AC_ERROR("ClientDataDecoder::DoPrivateChat:idx error");
		return -1;
	}

	map<int, RoomClient*>::iterator itu = proominfo->userlist.find(idx);
	if(itu == proominfo->userlist.end())
	{
		AC_ERROR("ClientDataDecoder::DoPrivateChat:idx error");
		return -1;
	}

	char context[1024*100];
	size_t len = 0;
	if(!pinstream->Read(context, sizeof(context), len))   
	{
		AC_ERROR("ClientDataDecoder::DoPrivateChat:read context error");
		return -1;
	}

	if(len == 0)
	{
		AC_ERROR("ClientDataDecoder::DoPrivateChat:context error");
		return -1;
	}
	context[len] = 0;

	RoomClient* pSend = (*itu).second;
	char outbuf[1024*100+100];
	BinaryWriteStream outstream(outbuf,sizeof(outbuf));
	char type = 65;
	outstream.Write(type);
	outstream.Write((short)ROOM_CMD_PRIVATE_CHAT);
	outstream.Write(seq);	
	outstream.Write((int)pRoomClient->m_idx);
	outstream.Write(context, len);  
	outstream.Flush();
	
	if(g_pNetProxy->SendToSrv(pSend, outstream)==-1)
	{
		AC_ERROR("ClientDataDecoder::DoPrivateChat:send to %d error",idx);
		pSend->Close();
		return 0;
	}
	
	return 0;
}
*/
int ClientDataDecoder::DoPublicChat(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoPublicChat:cmd = %d seq = %d", cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoPublicChat:roominfo error");
		return -1;
	}

	AC_DEBUG("ClientDataDecoder::DoPublicChat:roomid=%d,idx = %d ",pRoomClient->m_roomid,pRoomClient->m_idx);

	//用户被禁言，则不转发聊天
	map<int,roomtimeout*>::iterator itf=proominfo->forbidenlist.find(pRoomClient->m_idx);
	if(itf!=proominfo->forbidenlist.end())
	{
		char temp[64];
		BinaryWriteStream outstream(temp,sizeof(temp));
    	char type = 65;
    	outstream.Write(type);
		outstream.Write((short)cmd);
		outstream.Write(seq);
		outstream.Write((int)0);		//被禁言标志
		outstream.Flush();

		if(g_pNetProxy->SendToSrv(pRoomClient,outstream)==-1)
		{
			AC_ERROR("ClientDataDecoder::DoPublicChat:SendToSrv error");
			return -1;
		}
		return 0;
	}

	if(!proominfo->isPublicChat)	//不允许公聊
	{
		//助理主持以下在禁止公聊时不能发公聊
		//if(CheckRight(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
		if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
		{
	
			char temp[64];
			BinaryWriteStream outstream(temp,sizeof(temp));
	    	char type = 65;
	    	outstream.Write(type);
			outstream.Write((short)cmd);
			outstream.Write(seq);
			outstream.Write(-1);		//不允许公聊
			outstream.Flush();

			if(g_pNetProxy->SendToSrv(pRoomClient,outstream)==-1)
			{
				AC_ERROR("ClientDataDecoder::DoPublicChat:SendToSrv error");
				return -1;
			}
			return 0;
		}
	}

	char context[10240];
	size_t len = 0;
	if(!pinstream->Read(context, sizeof(context), len)) 
	{
		AC_ERROR("ClientDataDecoder::DoPublicChat:read context error");
		return -1;
	}

	if(len == 0)
	{
		AC_ERROR("ClientDataDecoder::DoPublicChat:context error");
		return -1;
	}

	context[len] = 0;

    BinaryWriteStream* outstream = StreamFactory::InstanceWriteStream();
	char type = 65;
	outstream->Write(type);
	outstream->Write((short)ROOM_CMD_PUBLIC_CHAT);
	outstream->Write(seq);
	outstream->Write(1);
	outstream->Write((int)pRoomClient->m_idx);
	outstream->Write(context, len);  
	time_t chattime = time(0);
	outstream->Write((int)chattime);  
	outstream->Flush();
	/*
	if(g_pNetProxy->BroadcastSBCmd(proominfo,outstream) == -1)
	{
		AC_ERROR("ClientDataDecoder::DoPublicChat:g_pNetProxy->BroadcastSBCmd error");
		return -1;
	}
	*/
	char outbuf2[10240];
	int outbuflen2=sizeof(outbuf2);
    if(!StreamCompress(outstream->GetData(),(int)outstream->GetSize(),outbuf2,outbuflen2))
    {
        AC_ERROR("StreamCompress error");
        return 0;
    }

    map<int, RoomClient*>::iterator itsend = proominfo->userlist.begin();
    for(;itsend != proominfo->userlist.end();itsend++)
	{
		RoomClient* psend = itsend->second;
		if(psend != NULL)
		{
			if(psend->m_idx == 0)
			{
				AC_ERROR("psend m_idx == 0");
				//psend->ErrorClose();
			}
			else if(psend->AddBuf(outbuf2,outbuflen2) == -1)
			{
				AC_ERROR("AddBuf == -1");
				//psend->ErrorClose();
			}
		}
	}

	return 0;
}

int ClientDataDecoder::DoOnVJMic(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoOnVJMic:pinstream = %x cmd = %d seq = %d", pinstream, cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoOnVJMic:roominfo error");
		return -1;
	}
	
	AC_DEBUG("ClientDataDecoder::DoOnVJMic:roomid=%d,idx = %d ",pRoomClient->m_roomid,pRoomClient->m_idx);

	//请求上VJ的用户已经在表演麦上
	if(proominfo->onmic == pRoomClient->m_idx)
	{
		AC_ERROR("ClientDataDecoder::DoOnVJMic: idx=%d id onmic now",pRoomClient->m_idx);
		return 0;
	}
    
	//判断是否在麦序列表中,在排序列表中不能上VJ麦
	MIC_INFO micinfo;
	micinfo.idx = pRoomClient->m_idx;

	//vector<MIC_INFO*>::iterator itm = find(proominfo->miclist.begin(), proominfo->miclist.end(), micinfo);
	vector<MIC_INFO*>::iterator itm = SearchWaitMic(proominfo->miclist.begin(), proominfo->miclist.end(), pRoomClient->m_idx);
	if(itm != proominfo->miclist.end())
	{
		AC_ERROR("ClientDataDecoder::DoOnVJMic:idx = %d has wait mic", pRoomClient->m_idx);
		return 0;
	}

	//vj麦上有人也不能排，室主除外
	if(proominfo->vjonmic != 0 && proominfo->ownidx != pRoomClient->m_idx)
	{
		AC_ERROR("ClientDataDecoder::DoOnVJMic:proominfo->vjonmic = %d onvjmic", proominfo->vjonmic);
		return 0;
	}

	//if(CheckRight(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
	if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
	{
		AC_ERROR("ClientDataDecoder::DoOnVJMic:power error");
	 	return 0;
	}
    
	AC_DEBUG("ClientDataDecoder::DoOnVJMic:roomid = %d, vjlist size = %d ", proominfo->roomid, proominfo->vjlist.size());


	//如果是房主请求上麦时，则房主直接上麦且把原先的持VJ麦的人踢走
	AC_DEBUG("ClientDataDecoder::DoOnVJMic:roomid= %d proominfo->ownidx = %d pRoomClient->m_idx = %d ", proominfo->roomid,proominfo->ownidx, pRoomClient->m_idx);
	if (proominfo->ownidx == pRoomClient->m_idx)
	{
		if (proominfo->vjonmic != 0)//如果原先有人上VJ麦，则让其下VJ麦
		{
			//add by jinguanfu 2011/1/25
			map<int,RoomClient*>::iterator itc = proominfo->userlist.find(proominfo->vjonmic);
			if(itc!=proominfo->userlist.end())
			{
				RoomClient* pOnVJMic=(RoomClient*)itc->second;
				pOnVJMic->m_onmicflag = USER_STATE_NONE;	
			}
	
			int seq2 = 0;
			//char outbuf[256]64] = {0};
			BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
			char type = 65;
			outstream2->Write(type);
			outstream2->Write((short)ROOM_CMD_SB_OFFVJ_MIC);
			outstream2->Write(seq2);
			outstream2->Write((int)proominfo->vjonmic);  
			outstream2->Flush();
			if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2) == -1)
			{
				AC_ERROR("ClientDataDecoder::DoOnVJMic:g_pNetProxy->BroadcastSBCmd error");
				return -1;
			}

			//add by jinguanfu 2011/1/25
			//发送状态到日志服务器
			BinaryWriteStream* logstream=StreamFactory::InstanceWriteStream();
			logstream->Write((short)CMD_ROOM_OFFVJMIC_R2LS);
			logstream->Write(0);
			logstream->Write(proominfo->vjonmic); //用户idx
			logstream->Write(proominfo->roomid); //所在房间ID
			logstream->Flush();
				
			BackClientSocketBase *pLogSvr = g_pNetProxy->GetLogServer();
			if(pLogSvr!=NULL)
			{
				if(pLogSvr->AddBuf(logstream->GetData(), logstream->GetSize())==-1) 
				{
					AC_ERROR("ClientDataDecoder::DoOnVJMic:pLogSvr->AddBuf Error");
				}else
				{
					AC_DEBUG("ClientDataDecoder::DoOnVJMic:Send to LogSvr success,idx=%d roomid=%d",proominfo->vjonmic,proominfo->roomid);
				}
			
			}
			else
			{
				AC_ERROR("ClientDataDecoder::DoOnVJMic:pLogSvr is NULL");
			}

			
		}	
		proominfo->vjonmic = pRoomClient->m_idx;
		//add by jinguanfu 2011/1/25
		pRoomClient->m_onmicflag = USER_STATE_ONVJMIC;

		//发送给房间内所有的人,vj上麦了
		int seq2 = 0;
		//char outbuf[256]64] = {0};
		BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
		char type = 65;
		outstream2->Write(type);
		outstream2->Write((short)ROOM_CMD_SB_ONVJ_MIC);
		outstream2->Write(seq2);
		outstream2->Write((int)proominfo->vjonmic);  
		outstream2->Flush();
		if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2) == -1)
		{
			AC_ERROR("ClientDataDecoder::DoOnVJMic:g_pNetProxy->BroadcastSBCmd error");
			return -1;
		}

		//add by jinguanfu 2011/1/25
		//发送状态到日志服务器
		BinaryWriteStream* logstream=StreamFactory::InstanceWriteStream();
		logstream->Write((short)CMD_ROOM_ONVJMIC_R2LS);
		logstream->Write(0);
		logstream->Write(pRoomClient->m_idx); //用户idx
		logstream->Write(pRoomClient->m_roomid); //所在房间ID
		logstream->Flush();
				
		BackClientSocketBase *pLogSvr = g_pNetProxy->GetLogServer();
		if(pLogSvr!=NULL)
		{
			if(pLogSvr->AddBuf(logstream->GetData(), logstream->GetSize())==-1) 
			{
				AC_ERROR("ClientDataDecoder::DoOnVJMic:pLogSvr->AddBuf Error");
			}else
			{
				AC_DEBUG("ClientDataDecoder::DoOnVJMic:Send to LogSvr success,idx=%d roomid=%d",proominfo->vjonmic,proominfo->roomid);
			}
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoOnVJMic:pLogSvr is NULL");
		}
	}
	else
	{	
		if(proominfo->vjonmic == 0)
		{
			proominfo->vjonmic = pRoomClient->m_idx;

			//发送给房间内所有的人,vj上麦了
			int seq2 = 0;
			//char outbuf[256]64] = {0};
			BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
			char type = 65;
			outstream2->Write(type);
			outstream2->Write((short)ROOM_CMD_SB_ONVJ_MIC);
			outstream2->Write(seq2);
			outstream2->Write((int)proominfo->vjonmic);  
			outstream2->Flush();
			if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2) == -1)
			{
				AC_ERROR("ClientDataDecoder::DoOnVJMic:g_pNetProxy->BroadcastSBCmd error");
				return -1;
			}

			//add by jinguanfu 2011/1/25
			pRoomClient->m_onmicflag = USER_STATE_ONVJMIC;
			//发送状态到日志服务器
			BinaryWriteStream* logstream=StreamFactory::InstanceWriteStream();
			logstream->Write((short)CMD_ROOM_ONVJMIC_R2LS);
			logstream->Write(0);
			logstream->Write(proominfo->vjonmic); //用户idx
			logstream->Write(proominfo->roomid); //所在房间ID
			logstream->Flush();
					
			BackClientSocketBase *pLogSvr = g_pNetProxy->GetLogServer();
			if(pLogSvr!=NULL)
			{
				if(pLogSvr->AddBuf(logstream->GetData(), logstream->GetSize())==-1) 
				{
					AC_ERROR("ClientDataDecoder::DoOnVJMic:pLogSvr->AddBuf Error");
				}else
				{
					AC_DEBUG("ClientDataDecoder::DoOnVJMic:Send to LogSvr success,idx=%d roomid=%d",proominfo->vjonmic,proominfo->roomid);
				}
			}
			else
			{
				AC_ERROR("ClientDataDecoder::DoOnVJMic:pLogSvr is NULL");
			}
		}
	}

	return 0;
}

int ClientDataDecoder::DoOffVJMic(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoOffVJMic:pinstream = %x cmd = %d seq = %d", pinstream, cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoOffVJMic:roominfo error");
		return -1;
	}

	AC_DEBUG("ClientDataDecoder::DoOffVJMic:roomid=%d,idx = %d ",pRoomClient->m_roomid,pRoomClient->m_idx);

	if(proominfo->vjonmic == pRoomClient->m_idx)
	{
		proominfo->vjonmic = 0;
		//add by jinguanfu 2011/1/25
		pRoomClient->m_onmicflag = USER_STATE_NONE;

		//发送给房间内所有的人,vj下麦了
		int seq2 = 0;
		BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
		char type = 65;
		outstream2->Write(type);
		outstream2->Write((short)ROOM_CMD_SB_OFFVJ_MIC);
		outstream2->Write(seq2);
		outstream2->Write((int)pRoomClient->m_idx);  
		outstream2->Flush();
		if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2) == -1)
		{
			AC_ERROR("ClientDataDecoder::DoOffVJMic:g_pNetProxy->BroadcastSBCmd error");
			return -1;
		}

		//add by jinguanfu 2011/1/25
		//发送状态到日志服务器
		BinaryWriteStream* logstream=StreamFactory::InstanceWriteStream();
		logstream->Write((short)CMD_ROOM_OFFVJMIC_R2LS);
		logstream->Write(0);
		logstream->Write(pRoomClient->m_idx); //用户idx
		logstream->Write(pRoomClient->m_roomid); //所在房间ID
		logstream->Flush();
				
		BackClientSocketBase *pLogSvr = g_pNetProxy->GetLogServer();
		if(pLogSvr!=NULL)
		{
			if(pLogSvr->AddBuf(logstream->GetData(), logstream->GetSize())==-1) 
			{
				AC_ERROR("ClientDataDecoder::DoOffVJMic:pLogSvr->AddBuf Error");
			}else
			{
				AC_DEBUG("ClientDataDecoder::DoOffVJMic:Send to LogSvr success,idx=%d roomid=%d",pRoomClient->m_idx,pRoomClient->m_roomid);
			}
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoOffVJMic:pLogSvr is NULL");
		}

	}
	else
	{
		AC_ERROR("ClientDataDecoder::DoOffVJMic:idx=%d  not onvjmic",pRoomClient->m_idx);
		return 0;
	}

	return 0;
}



int ClientDataDecoder::DoGiveVJMic(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoGiveVJMic:cmd = %d seq = %d", cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoGiveVJMic:roominfo error");
		return -1;
	}

	int idx = 0;
	if(!pinstream->Read(idx))    //对象的idx
	{
		AC_ERROR("ClientDataDecoder::DoGiveVJMic:read idx error");
		return -1;
	}

	if(idx == 0)
	{
		AC_ERROR("ClientDataDecoder::DoGiveVJMic:idx error");
		return -1;
	}

	AC_DEBUG("ClientDataDecoder::DoGiveVJMic:roomid=%d,pRoomClient->m_idx=%d,idx = %d ",
		pRoomClient->m_roomid,pRoomClient->m_idx,idx);

	map<int,RoomClient*>::iterator itu=proominfo->userlist.find(idx);
	if(itu==proominfo->userlist.end())
	{
		AC_ERROR("ClientDataDecoder::DoGiveVJMic:idx=%d in not in roomid=%d",idx,proominfo->roomid);
		return 0;//延时操作时不断开连接
	}
	RoomClient* pClient2=itu->second;
	//if(CheckRight(pRoomClient->m_identity, pRoomClient->m_viptype,pClient2->m_identity,pClient2->m_viptype, cmd)==0)
	if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,pClient2->m_identity,pClient2->m_viptype, cmd)==0)
	{
		AC_ERROR("ClientDataDecoder::DoGiveVJMic:power error");
	 	return 0;
	}

	
	BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
	char type = 65;
	//add by jinguanfu 2011/3/21
	//检查被邀请者状态
	if(proominfo->vjonmic!=0)
	{
		outstream->Clear();
		outstream->Write(type);
		outstream->Write(cmd);
		outstream->Write(seq);
		outstream->Write(idx);
		outstream->Write((char)RESULT_GIVEVJMIC_ONVJMIC);//有人在VJ麦上
		outstream->Flush();	
		if(g_pNetProxy->SendToSrv(pRoomClient, *outstream) == -1)
      	{
       		AC_ERROR("ClientDataDecoder::DoGiveVJMic: SendToSrv error sendto=%d",pRoomClient->m_idx);
			pRoomClient->Close();
      	}
		return 0;
	}

	if(proominfo->onmic.idx==idx)
	{
		outstream->Clear();
		outstream->Write(type);
		outstream->Write(cmd);
		outstream->Write(seq);
		outstream->Write(idx);
		outstream->Write((char)RESULT_GIVEVJMIC_ONMIC);//被邀请者在表演麦上
		outstream->Flush();	
		if(g_pNetProxy->SendToSrv(pRoomClient, *outstream) == -1)
      	{
       		AC_ERROR("ClientDataDecoder::DoGiveVJMic: SendToSrv error sendto=%d",pRoomClient->m_idx);
			pRoomClient->Close();
      	}
		return 0;
	}


	//add by jinguanfu 2010/6/3 向被邀请者发送请求
	outstream->Clear();
	outstream->Write(type);
	outstream->Write((short)ROOM_CMD_REQUEST_GIVE_VJ_MIC);
	outstream->Write(0);
	outstream->Write((int)pRoomClient->m_idx);  
	outstream->Flush();
   	if(g_pNetProxy->SendToSrv(pClient2, *outstream) == -1)
      {
       	AC_ERROR("ClientDataDecoder::DoGiveVJMic: SendToSrv error sendto=%d",idx);
		pClient2->Close();
        	return 0;
      }
	

	return 0;
}

int ClientDataDecoder::DoGiveOffVJMic(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoGiveOffVJMic:cmd = %d seq = %d", cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoGiveOffVJMic:roominfo error");
		return -1;
	}

	int idx = 0;
	if(!pinstream->Read(idx))    //对象的idx
	{
		AC_ERROR("ClientDataDecoder::DoGiveOffVJMic:read idx error");
		return -1;
	}

	if(idx == 0)
	{
		AC_ERROR("ClientDataDecoder::DoGiveOffVJMic:idx error");
		return -1;
	}

	AC_DEBUG("ClientDataDecoder::DoGiveOffVJMic:roomid=%d,pRoomClient->m_idx=%d,idx = %d ",
		pRoomClient->m_roomid,pRoomClient->m_idx,idx);

	map<int,RoomClient*>::iterator itu=proominfo->userlist.find(idx);
	if(itu==proominfo->userlist.end())
	{

		AC_ERROR("ClientDataDecoder::DoGiveOffVJMic:idx=%d in not in roomid=%d",idx,proominfo->roomid);
        	return 0;//延时操作时不断开连接
	}
	RoomClient* pClient2=itu->second;
	//if(CheckRight(pRoomClient->m_identity, pRoomClient->m_viptype,pClient2->m_identity,pClient2->m_viptype, cmd)==0)
	if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,pClient2->m_identity,pClient2->m_viptype, cmd)==0)
	{
		AC_ERROR("ClientDataDecoder::DoGiveOffVJMic:power error");
	 	return 0;
	}
	
	if(proominfo->vjonmic == idx)
	{
		proominfo->vjonmic = 0;

		//add by jinguanfu 2011/1/25
		map<int,RoomClient*>::iterator itc = proominfo->userlist.find(idx);
		if(itc!=proominfo->userlist.end())
		{
			RoomClient* pOnVJMic=(RoomClient*)itc->second;
			pOnVJMic->m_onmicflag = USER_STATE_NONE;	
		}


		//发送给房间内所有的人,vj被提下麦了
		int seq2 = 0;
		//char outbuf[256]64] = {0};
		BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
		char type = 65;
		outstream2->Write(type);
		outstream2->Write((short)ROOM_CMD_SB_OFFVJ_MIC);
		outstream2->Write(seq2);
		outstream2->Write(idx);  
		outstream2->Flush();
		if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2) == -1)
		{
			AC_ERROR("ClientDataDecoder::DoGiveOffVJMic:g_pNetProxy->BroadcastSBCmd error");
			return -1;
		}

		//add by jinguanfu 2011/1/25
		//发送状态到日志服务器
		BinaryWriteStream* logstream=StreamFactory::InstanceWriteStream();
		logstream->Write((short)CMD_ROOM_OFFVJMIC_R2LS);
		logstream->Write(0);
		logstream->Write(idx); 					//用户idx
		logstream->Write(proominfo->roomid);	//所在房间ID
		logstream->Flush();
				
		BackClientSocketBase *pLogSvr = g_pNetProxy->GetLogServer();
		if(pLogSvr!=NULL)
		{
			if(pLogSvr->AddBuf(logstream->GetData(), logstream->GetSize())==-1) 
			{
				AC_ERROR("ClientDataDecoder::DoGiveOffVJMic:pLogSvr->AddBuf Error");
			}else
			{
				AC_DEBUG("ClientDataDecoder::DoGiveOffVJMic:Send to LogSvr success,idx=%d roomid=%d",idx,proominfo->roomid);
			}
			
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoGiveOffVJMic:pLogSvr is NULL");
		}
	}
	else
	{
		AC_ERROR("ClientDataDecoder::DoGiveOffVJMic:idx=%d not onvjmic",idx);
		return 0;
	}
	return 0;
}

//检查上麦列表，如果没有人就上排麦列表的第一个人
int ClientDataDecoder::CheckMiclistOnmic(ROOM_INFO* proominfo)
{
	AC_DEBUG("ClientDataDecoder::CheckMiclistOnmic:roomid=%d",proominfo->roomid);

	//add by jinguanfu 2011/4/19 暂停自动 上麦
	if(!proominfo->isAutoOnmic)
	{
		AC_DEBUG("ClientDataDecoder::CheckMiclistOnmic: not allow auto on mic");
		return 0;
	}

	if(proominfo->onmic.idx != 0 || proominfo->onmic.pkidx != 0)
	{
		AC_DEBUG("ClientDataDecoder::CheckMiclistOnmic:sb has on mic proominfo->onmic.idx = %d proominfo->onmic.pkidx = %d", 
		proominfo->onmic.idx, proominfo->onmic.pkidx);
		return 0;
	}

	vector<MIC_INFO*>::iterator itm = proominfo->miclist.begin();
	if(itm != proominfo->miclist.end())
	{
		MIC_INFO* pmicinfo = (MIC_INFO*)(*itm);
		proominfo->onmic.idx = pmicinfo->idx;
		proominfo->onmic.musicid = pmicinfo->musicid;
		proominfo->onmic.bk = pmicinfo->bk;
		proominfo->onmic.pkidx = pmicinfo->pkidx;
		proominfo->onmic.pkmusicid = pmicinfo->pkmusicid;
		proominfo->onmic.pkbk = pmicinfo->pkbk;
		proominfo->onmic.level = pmicinfo->level;
		proominfo->onmic.musictime = pmicinfo->musictime;
		proominfo->onmic.state = MIC_INFO_ONMIC_READY;
		if(proominfo->onmic.pkidx != 0)
			proominfo->onmic.pkstate = MIC_INFO_ONMIC_READY;

		//add by jinguanfu 2012/2/27
		strcpy(proominfo->onmic.musicname,pmicinfo->musicname);
		proominfo->onmic.musicspeed=pmicinfo->musicspeed;
		//add by jinguanfu 2012/4/5
		//proominfo->onmic.reverseflag=pmicinfo->reverseflag;

		//add by jinguanfu 2010/2/3 <begin> 上麦超时
		if(proominfo->m_pOnMicTimeout!=NULL)
		{
			proominfo->m_pOnMicTimeout->UnRegisterTimer();
			proominfo->m_pOnMicTimeout->m_idx=pmicinfo->idx;
			proominfo->m_pOnMicTimeout->m_pClientDataDecoder=this;
			proominfo->m_pOnMicTimeout->m_type = TIMEOUT_READY;
			proominfo->m_pOnMicTimeout->RegisterTimer(READY_WAIT);	
		}
		else
		{
			//初始化上麦超时对象
			proominfo->m_pOnMicTimeout=g_pNetProxy->CreateRoomTimeout();
			if(proominfo->m_pOnMicTimeout!=NULL)
			{
				proominfo->m_pOnMicTimeout->SetReactor(g_pNetProxy->GetReactor());
				proominfo->m_pOnMicTimeout->m_roomid=proominfo->roomid;

				proominfo->m_pOnMicTimeout->UnRegisterTimer();
				proominfo->m_pOnMicTimeout->m_idx=pmicinfo->idx;
				proominfo->m_pOnMicTimeout->m_pClientDataDecoder=this;
				proominfo->m_pOnMicTimeout->m_type = TIMEOUT_READY;
				proominfo->m_pOnMicTimeout->RegisterTimer(READY_WAIT);	
			}
			else
			{
				AC_ERROR("ClientDataDecoder::CheckMiclistOnmic:CreateRoomTimeout error");
				return -1;
			}
		}
		//add by jinguanfu 2010/2/3 <end> 上麦超时

		//add by jinguanfu 2011/1/25
		map<int,RoomClient*>::iterator itc = proominfo->userlist.find(proominfo->onmic.idx);
		if(itc!=proominfo->userlist.end())
		{
			RoomClient* pRoomClient=(RoomClient*)itc->second;
			pRoomClient->m_onmicflag = USER_STATE_ONMIC;	
		}
		
		map<int,RoomClient*>::iterator itcpk = proominfo->userlist.find(proominfo->onmic.pkidx);
		if(itcpk!=proominfo->userlist.end())
		{
			RoomClient* pRoomClient=(RoomClient*)itcpk->second;
			pRoomClient->m_onmicflag = USER_STATE_ONMIC;	
		}

		
		//发送给房间内所有的人有人上麦准备的消息
		int seq2 = 0;
		//char outbuf[256] = {0};
		BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
		char type = 65;
		outstream2->Write(type);
		outstream2->Write((short)ROOM_CMD_SB_ONMIC_READY);
		outstream2->Write(seq2);
		outstream2->Write(proominfo->onmic.idx);
		outstream2->Write(proominfo->onmic.musicid);
		outstream2->Write(proominfo->onmic.bk);
		outstream2->Write(proominfo->onmic.pkidx);
		outstream2->Write(proominfo->onmic.pkmusicid);
		outstream2->Write(proominfo->onmic.pkbk);
		outstream2->Write(proominfo->onmic.level);
		outstream2->Flush();
		if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2) == -1)
		{
			AC_ERROR("ClientDataDecoder::CheckMiclistOnmic:g_pNetProxy->BroadcastSBCmd ONMIC_READY error");
			return -1;
		}
		

		proominfo->miclist.erase(itm);

		AC_DEBUG("ClientDataDecoder::CheckMiclistOnmic:roomid=%d,onmic.idx=%d OnMicReady",proominfo->roomid,proominfo->onmic.idx);
		
		//add by jinguanfu 2010/3/30  释放内存到缓存池
		g_pNetProxy->DestroyMicInfo(pmicinfo);
	}
	else
	{
		AC_DEBUG("ClientDataDecoder::CheckMiclistOnmic:Nobody in the WaitMicList");		
	}
	return 0;
}

int ClientDataDecoder::CheckMiclistOnmic2(ROOM_INFO* proominfo)
{
	AC_DEBUG("ClientDataDecoder::CheckMiclistOnmic2:roomid=%d",proominfo->roomid);

	//add by jinguanfu 2011/4/19 暂停自动 上麦
	if(!proominfo->isAutoOnmic)
	{
		AC_DEBUG("ClientDataDecoder::CheckMiclistOnmic2: not allow auto on mic");
		return 0;
	}

	if(proominfo->onmic.idx != 0 || proominfo->onmic.pkidx != 0)
	{
		AC_DEBUG("ClientDataDecoder::CheckMiclistOnmic2:sb has on mic proominfo->onmic.idx = %d proominfo->onmic.pkidx = %d", 
		proominfo->onmic.idx, proominfo->onmic.pkidx);
		return 0;
	}

	vector<MIC_INFO*>::iterator itm = proominfo->miclist.begin();
	if(itm != proominfo->miclist.end())
	{
		MIC_INFO* pmicinfo = (MIC_INFO*)(*itm);
		proominfo->onmic.idx = pmicinfo->idx;
		proominfo->onmic.musicid = pmicinfo->musicid;
		proominfo->onmic.bk = pmicinfo->bk;
		proominfo->onmic.pkidx = pmicinfo->pkidx;
		proominfo->onmic.pkmusicid = pmicinfo->pkmusicid;
		proominfo->onmic.pkbk = pmicinfo->pkbk;
		proominfo->onmic.level = pmicinfo->level;
		proominfo->onmic.musictime = pmicinfo->musictime;
		proominfo->onmic.state = MIC_INFO_ONMIC;
		if(proominfo->onmic.pkidx != 0)
			proominfo->onmic.pkstate = MIC_INFO_ONMIC;

		strcpy(proominfo->onmic.musicname,pmicinfo->musicname);
		proominfo->onmic.musicspeed=pmicinfo->musicspeed;

		//上麦超时
		if(proominfo->m_pOnMicTimeout!=NULL)
		{
			proominfo->m_pOnMicTimeout->UnRegisterTimer();
			proominfo->m_pOnMicTimeout->m_idx=pmicinfo->idx;
			proominfo->m_pOnMicTimeout->m_pClientDataDecoder=this;
			proominfo->m_pOnMicTimeout->m_type = TIMEOUT_OFFMIC;
			proominfo->m_pOnMicTimeout->RegisterTimer(OFFMIC_WAIT);	
		}
		else
		{
			//初始化上麦超时对象
			proominfo->m_pOnMicTimeout=g_pNetProxy->CreateRoomTimeout();
			if(proominfo->m_pOnMicTimeout!=NULL)
			{
				proominfo->m_pOnMicTimeout->SetReactor(g_pNetProxy->GetReactor());
				proominfo->m_pOnMicTimeout->m_roomid=proominfo->roomid;

				proominfo->m_pOnMicTimeout->UnRegisterTimer();
				proominfo->m_pOnMicTimeout->m_idx=pmicinfo->idx;
				proominfo->m_pOnMicTimeout->m_pClientDataDecoder=this;
				proominfo->m_pOnMicTimeout->m_type = TIMEOUT_OFFMIC;
				proominfo->m_pOnMicTimeout->RegisterTimer(OFFMIC_WAIT);	
			}
			else
			{
				AC_ERROR("ClientDataDecoder::CheckMiclistOnmic2:CreateRoomTimeout error");
				return -1;
			}
		}

		map<int,RoomClient*>::iterator itc = proominfo->userlist.find(proominfo->onmic.idx);
		if(itc!=proominfo->userlist.end())
		{
			RoomClient* pRoomClient=(RoomClient*)itc->second;
			pRoomClient->m_onmicflag = USER_STATE_ONMIC;	
		}
		
		map<int,RoomClient*>::iterator itcpk = proominfo->userlist.find(proominfo->onmic.pkidx);
		if(itcpk!=proominfo->userlist.end())
		{
			RoomClient* pRoomClient=(RoomClient*)itcpk->second;
			pRoomClient->m_onmicflag = USER_STATE_ONMIC;	
		}

		
		//向房间广播有人上麦的消息
		int seq2 = 0;
		//char outbuf[256] = {0};
		BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
		char type = 65;
		outstream2->Write(type);
		outstream2->Write((short)ROOM_CMD_SB_ONMIC);
		outstream2->Write(seq2);
		outstream2->Write(proominfo->onmic.idx);
		outstream2->Write(proominfo->onmic.musicid);
		outstream2->Write(proominfo->onmic.bk);
		outstream2->Write(proominfo->onmic.pkidx);
		outstream2->Write(proominfo->onmic.pkmusicid);
		outstream2->Write(proominfo->onmic.pkbk);
		outstream2->Write(proominfo->onmic.level);
		outstream2->Write(proominfo->onmic.musicname,strlen(proominfo->onmic.musicname));
		outstream2->Write(proominfo->onmic.musicspeed);
		outstream2->Flush();
		if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2) == -1)
		{
			AC_ERROR("ClientDataDecoder::CheckMiclistOnmic2:g_pNetProxy->BroadcastSBCmd ONMIC_READY error");
			return -1;
		}

		proominfo->miclist.erase(itm);

		AC_DEBUG("ClientDataDecoder::CheckMiclistOnmic2:roomid=%d,onmic.idx=%d OnMicReady",proominfo->roomid,proominfo->onmic.idx);
		
		//add by jinguanfu 2010/3/30  释放内存到缓存池
		g_pNetProxy->DestroyMicInfo(pmicinfo);
	}
	else
	{
		AC_DEBUG("ClientDataDecoder::CheckMiclistOnmic2:Nobody in the WaitMicList");		
	}
	
	return 0;

}


//有人离开清除排麦列表
int ClientDataDecoder::ClearWaitmicList(ROOM_INFO* proominfo, int idx)
{
	AC_DEBUG("ClientDataDecoder::ClearWaitmicList:roomid=%d",proominfo->roomid);

	//MIC_INFO micinfo;
	//micinfo.idx = idx;
	vector<MIC_INFO*>::iterator itm = SearchWaitMic(proominfo->miclist.begin(), proominfo->miclist.end(), idx);
	if(itm != proominfo->miclist.end()) 
	{
		MIC_INFO* pmicinfo = (MIC_INFO*)(*itm);
		proominfo->miclist.erase(itm);
		g_pNetProxy->DestroyMicInfo(pmicinfo);
		
	}

	return 0;
}




//有人离开就清除持麦列表
int ClientDataDecoder::ClearOnmic(ROOM_INFO* proominfo, int idx)
{
	AC_DEBUG("ClientDataDecoder::ClearOnmic:roomid=%d, idx = %d onmicidx = %d",proominfo->roomid, idx, proominfo->onmic.idx);

	//如果是VJ持麦人，则VJ持麦人清空
	if ((int)proominfo->vjonmic == idx)
	{
		 proominfo->vjonmic = 0;
	}
     
	//判断离开房间的人是否onmic
	if(idx != proominfo->onmic.idx)
	{
		return 0;
	}

	//在麦上，清除
	proominfo->onmic.init();

	//取消上麦超时
	if(proominfo->m_pOnMicTimeout!=NULL)
		proominfo->m_pOnMicTimeout->UnRegisterTimer();
	
	//5s后让麦序里的人上麦
	roomtimeout* ptb = g_pNetProxy->CreateRoomTimeout();
	if(ptb!=NULL)
	{
		ptb->m_roomid = proominfo->roomid;
		ptb->m_idx=idx;
		ptb->m_time = OFFMIC_WAIT; //5s
		ptb->m_type = TIMEOUT_OFFMIC;
		ptb->SetReactor(g_pNetProxy->GetReactor());
		ptb->m_pClientDataDecoder=this;
		ptb->RegisterTimer(ptb->m_time);
	}
	else
	{
		AC_ERROR("ClientDataDecoder::ClearOnmic:CreateRoomTimeout error");
		//CheckMiclistOnmic(proominfo);
		CheckMiclistOnmic2(proominfo);
		return -1;
	}

	return 0;
}

int ClientDataDecoder::ClearOnmicIdxEqualIdx(ROOM_INFO* proominfo)
{
	AC_DEBUG("ClientDataDecoder::ClearOnmicIdxEqualIdx:roomid=%d",proominfo->roomid);

    if(proominfo->onmic.pkstate == MIC_INFO_ONMIC_READY)
    {
        proominfo->onmic.idx = 0;
        proominfo->onmic.musicid = 0;
        proominfo->onmic.bk = 0;
        proominfo->onmic.state = MIC_INFO_WAIT;
    }
    else if(proominfo->onmic.pkstate == MIC_INFO_ONMIC_READYOK)
    {
        proominfo->onmic.idx = 0;
        proominfo->onmic.musicid = 0;
        proominfo->onmic.bk = 0;
        proominfo->onmic.state = MIC_INFO_WAIT;
        proominfo->onmic.pkstate = MIC_INFO_ONMIC;
        //发送给房间内所有的人某人上麦成功的消息
        int seq2 = 0;
        //char outbuf[256]64] = {0};
        BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
        char type = 65;
        outstream2->Write(type);
        outstream2->Write((short)ROOM_CMD_SB_ONMIC);
        outstream2->Write(seq2);
        outstream2->Write((int)proominfo->onmic.pkidx);  
        outstream2->Flush();
        if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2) == -1)
        {
            AC_ERROR("ClientDataDecoder::ClearOnmicIdxEqualIdx:g_pNetProxy->BroadcastSBCmd error");
            return -1;
        }
    }
    else if(proominfo->onmic.pkstate == MIC_INFO_ONMIC)
    {
        proominfo->onmic.idx = 0;
        proominfo->onmic.musicid = 0;
        proominfo->onmic.bk = 0;
        proominfo->onmic.state = MIC_INFO_WAIT;
    }
    else if(proominfo->onmic.pkstate == MIC_INFO_OFFMIC_READY)
    {
        proominfo->onmic.idx = 0;
        proominfo->onmic.musicid = 0;
        proominfo->onmic.bk = 0;
        proominfo->onmic.state = MIC_INFO_WAIT;
        proominfo->onmic.pkstate = MIC_INFO_OFFMIC_READYOK;
        //发送给房间内所有的人下麦准备完成,可以给服务器发送打分结果了
        int seq2 = 0;
        //char outbuf[256]64] = {0};
        BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
        char type = 65;
        outstream2->Write(type);
        outstream2->Write((short)ROOM_CMD_SB_OFFMIC_READYOK);
        outstream2->Write(seq2);
        outstream2->Write((int)proominfo->onmic.pkidx);  
        outstream2->Flush();
        if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2) == -1)
        {
            AC_ERROR("ClientDataDecoder::ClearOnmicIdxEqualIdx:g_pNetProxy->BroadcastSBCmd error");
            return -1;
        }
    }
    else if(proominfo->onmic.pkstate == MIC_INFO_OFFMIC_READYOK)
    {
        proominfo->onmic.idx = 0;
        proominfo->onmic.musicid = 0;
        proominfo->onmic.bk = 0;
        proominfo->onmic.state = MIC_INFO_WAIT;
    }
    else if(proominfo->onmic.pkstate == MIC_INFO_SCORE)
    {
        //发送给房间内所有的人下麦消息
        int seq2 = 0;
        //char outbuf[256]64] = {0};
        char haspk = 0;
        BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
        char type = 65;
        outstream2->Write(type);
        outstream2->Write((short)ROOM_CMD_SB_OFFMIC);
        outstream2->Write(seq2);
        outstream2->Write(haspk);
        outstream2->Write((int)proominfo->onmic.pkidx);
        outstream2->Write(proominfo->onmic.pkscore);
		//add by jinguanfu 2011/8/10 
		outstream2->Write(proominfo->onmic.scoreinfo,sizeof(NETSINGSCORE));	//打分信息结构体modify by lihongwu 2011/12/20
        outstream2->Flush();
        if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2) == -1)
        {
            AC_ERROR("ClientDataDecoder::ClearOnmicIdxEqualIdx:g_pNetProxy->BroadcastSBCmd error");
            return -1;
        }
        proominfo->onmic.init();
    }
    return 0;
}

int ClientDataDecoder::ClearOnmicIdxEqualPKIdx(ROOM_INFO* proominfo)
{
	AC_DEBUG("ClientDataDecoder::ClearOnmicIdxEqualPKIdx:roomid=%d",proominfo->roomid);

    if(proominfo->onmic.state == MIC_INFO_ONMIC_READY)
    {
        proominfo->onmic.pkidx = 0;
        proominfo->onmic.pkmusicid = 0;
        proominfo->onmic.pkbk = 0;
        proominfo->onmic.pkstate = MIC_INFO_WAIT;
    }
    else if(proominfo->onmic.state == MIC_INFO_ONMIC_READYOK)
    {
        proominfo->onmic.pkidx = 0;
        proominfo->onmic.pkmusicid = 0;
        proominfo->onmic.pkbk = 0;
        proominfo->onmic.pkstate = MIC_INFO_WAIT;
        proominfo->onmic.state = MIC_INFO_ONMIC;
        //发送给房间内所有的人某人上麦成功的消息
        int seq2 = 0;
        //char outbuf[256]64] = {0};
        BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
        char type = 65;
        outstream2->Write(type);
        outstream2->Write((short)ROOM_CMD_SB_ONMIC);
        outstream2->Write(seq2);
        outstream2->Write((int)proominfo->onmic.idx);  
        outstream2->Flush();
        if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2) == -1)
        {
            AC_ERROR("ClientDataDecoder::ClearOnmicIdxEqualPKIdx:g_pNetProxy->BroadcastSBCmd error");
            return -1;
        }
    }
    else if(proominfo->onmic.state == MIC_INFO_ONMIC)
    {
        proominfo->onmic.pkidx = 0;
        proominfo->onmic.pkmusicid = 0;
        proominfo->onmic.pkbk = 0;
        proominfo->onmic.pkstate = MIC_INFO_WAIT;
    }
    else if(proominfo->onmic.state == MIC_INFO_OFFMIC_READY)
    {
        proominfo->onmic.pkidx = 0;
        proominfo->onmic.pkmusicid = 0;
        proominfo->onmic.pkbk = 0;
        proominfo->onmic.pkstate = MIC_INFO_WAIT;
        proominfo->onmic.state = MIC_INFO_OFFMIC_READYOK;
        //发送给房间内所有的人下麦准备完成,可以给服务器发送打分结果了
        int seq2 = 0;
        //char outbuf[256]64] = {0};
        BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
        char type = 65;
        outstream2->Write(type);
        outstream2->Write((short)ROOM_CMD_SB_OFFMIC_READYOK);
        outstream2->Write(seq2);
        outstream2->Write((int)proominfo->onmic.idx);  
        outstream2->Flush();
        if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2) == -1)
        {
            AC_ERROR("ClientDataDecoder::ClearOnmicIdxEqualPKIdx:g_pNetProxy->BroadcastSBCmd error");
            return -1;
        }
    }
    else if(proominfo->onmic.state == MIC_INFO_OFFMIC_READYOK)
    {
        proominfo->onmic.pkidx = 0;
        proominfo->onmic.pkmusicid = 0;
        proominfo->onmic.pkbk = 0;
        proominfo->onmic.pkstate = MIC_INFO_WAIT;
    }
    else if(proominfo->onmic.state == MIC_INFO_SCORE)
    {
        //发送给房间内所有的人下麦消息
        int seq2 = 0;
        //char outbuf[256]64] = {0};
        char haspk = 0;
        BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
        char type = 65;
        outstream2->Write(type);
        outstream2->Write((short)ROOM_CMD_SB_OFFMIC);
        outstream2->Write(seq2);
        outstream2->Write(haspk);
        outstream2->Write((int)proominfo->onmic.idx);
        outstream2->Write(proominfo->onmic.score);
		//add by jinguanfu 2011/8/10 
		outstream2->Write(proominfo->onmic.scoreinfo,sizeof(NETSINGSCORE));	//打分信息结构体modify by lihongwu 2011/12/20
        outstream2->Flush();
        if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2) == -1)
        {
            AC_ERROR("ClientDataDecoder::ClearOnmicIdxEqualPKIdx:g_pNetProxy->BroadcastSBCmd error");
            return -1;
        }
        proominfo->onmic.init();
    }
    return 0;
}


int ClientDataDecoder::DoSendFlower(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoSendFlower:pinstream = %x cmd = %d seq = %d", pinstream, cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoSendFlower:roominfo error");
		return -1;
	}		
	int r_idx = 0;
	if(!pinstream->Read(r_idx))    //礼物接收者idx
	{
		AC_ERROR("ClientDataDecoder::DoSendFlower:read r_idx error");
		return -1;
	}

	if(r_idx == 0)
	{
		AC_ERROR("ClientDataDecoder::DoSendFlower:r_idx error");
		return -1;
	}

	int cate_idx = 0;
	if(!pinstream->Read(cate_idx))    //礼物ID
	{
		AC_ERROR("ClientDataDecoder::DoSendFlower:read cate_idx error");
		return -1;
	}

	/*if(cate_idx == 0)
	{
		AC_ERROR("ClientDataDecoder::DoSendFlower:cate_idx error");
		return -1;
	}*/

	int number = 0;
	if(!pinstream->Read(number))    //送礼物个数
	{
		AC_ERROR("ClientDataDecoder::DoSendFlower:read number error");
		return -1;
	}

	if( number== 0)
	{
		AC_ERROR("ClientDataDecoder::DoSendFlower:number error");
		return -1;
	}	

	AC_DEBUG("ClientDataDecoder::DoSendFlower:roomid=%d ,idx = %d ,r_idx=%d,cate_idx=%d,number=%d",
		proominfo->roomid,pRoomClient->m_idx,r_idx,cate_idx,number);

	//add by jinguanfu 2010/5/12 送礼服务器校验<begin>
	//礼物接收者不在房间中
	map<int,RoomClient*>::iterator itrecv=proominfo->userlist.find(r_idx);
	map<int,RoomClient*>::iterator itVrecv=proominfo->vuserlist.find(r_idx);
	if(itrecv==proominfo->userlist.end()&&itVrecv==proominfo->vuserlist.end())
	{
		AC_DEBUG("ClientDataDecoder::DoSendFlower:r_idx=%d not in room",r_idx);
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		char type=65;
		outstream->Write(type);
		outstream->Write(cmd);
		outstream->Write(seq);
		outstream->Write(cate_idx);		//礼物ID
		outstream->Write(number);		//礼物数量
		outstream->Write(r_idx);		//收礼者idx
		outstream->Write((int)DBRESULT_GIFT_OFFLINE);		//礼物接收者不房间内
		outstream->Flush();

		if(g_pNetProxy->SendToSrv(pClient, *outstream) == -1)
		{
			AC_ERROR("ClientDataDecoder::DoSendFlower:SendToSrv error");
			return -1;
		}
		
		return 0;
	}
	
	//礼物不存在
	map<int,GIFT_INFO>::iterator itGift=g_pNetProxy->m_GiftInfo.find(cate_idx);
	if(itGift==g_pNetProxy->m_GiftInfo.end())
	{
		AC_DEBUG("ClientDataDecoder::DoSendFlower:cate_idx=%d not exist ",cate_idx);
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		char type=65;
		outstream->Write(type);
		outstream->Write(cmd);
		outstream->Write(seq);
		outstream->Write(cate_idx);	//礼物ID
		outstream->Write(number);		//礼物数量
		outstream->Write(r_idx);		//收礼者idx
		outstream->Write((int)DBRESULT_GIFT_NOGIFT);		//无此交易类型
		outstream->Flush();

		if(g_pNetProxy->SendToSrv(pClient, *outstream) == -1)
		{
			AC_ERROR("ClientDataDecoder::DoSendFlower:SendToSrv error");
			return -1;
		}
		
		return 0;
	}

	int price=(itGift->second).price;

	int total=price*number;
	//金币数不足
	if(pRoomClient->gold<total)
	{
		AC_DEBUG("ClientDataDecoder::DoSendFlower:gold lack gold=%d,total=%d",pRoomClient->gold,total);
				BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		char type=65;
		outstream->Write(type);
		outstream->Write(cmd);
		outstream->Write(seq);
		outstream->Write(cate_idx);	//礼物ID
		outstream->Write(number);		//礼物数量
		outstream->Write(r_idx);		//收礼者idx
		outstream->Write((int)DBRESULT_GIFT_NOMONEY);			//余额不足
		outstream->Flush();

		if(g_pNetProxy->SendToSrv(pClient, *outstream) == -1)
		{
			AC_ERROR("ClientDataDecoder::DoSendFlower:SendToSrv error");
			return -1;
		}
		
		return 0;
	}

	//add by jinguanfu 2010/5/12 送礼服务器校验<end>

	BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgentGift();
	if(pDBSvr!=NULL)
	{
		//发送给dbagent
		//char outbuf[256] = {0};
		int outseq = g_pNetProxy->GetCounter()->Get();
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		static const char* spname = {"DBMW_SendFlower"};//存储过程填充
		outstream->Write((short)CMD_CALLSP);
		outstream->Write(outseq);
		outstream->Write(spname,strlen(spname));
		outstream->Write((short)5);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)pRoomClient->m_idx);	//赠送者idx
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)r_idx);				//接收者idx
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)proominfo->roomid);	//房间id
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)cate_idx);			//礼物idx
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)number);				//礼物数量
		outstream->Flush();

		Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
		if(data != NULL) 
		{
			if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1)
			{
				data->roomid = pRoomClient->m_roomid;
				data->opidx = pRoomClient->m_idx;
				data->bopidx = r_idx;
				data->cate_idx = cate_idx;
				data->number=number;
				data->cmd = cmd;
				data->seq=seq;
				data->outseq=outseq;
				data->SetReactor(g_pNetProxy->GetReactor());
				data->RegisterTimer(DB_TIMEOUT);
				g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
			}
			else
			{
				AC_ERROR("ClientDataDecoder::DoSendFlower:pDBSvr->AddBuf() error");
				g_pNetProxy->DestroyDBResult(data);
			}
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoSendFlower:CreateDBResultdata error,data=%x",data);
		}
	}
	else
	{
		AC_ERROR("ClientDataDecoder::DoSendFlower:pDBSvr =%x",pDBSvr);
	}

	return 0;
}

int ClientDataDecoder::DoSendNotice(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoSendNotice:cmd = %d seq = %d", cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoSendNotice:roominfo error");
		return -1;
	}

	char context[1024*100] = {0};
	size_t len = 0;
	if(!pinstream->Read(context, sizeof(context), len)) 
	{
		AC_ERROR("ClientDataDecoder::DoSendNotice:read context error");
		return -1;
	}

	if(len == 0)
	{
		AC_ERROR("ClientDataDecoder::DoSendNotice:context error");
		return -1;
	}
	context[len] = 0;

	AC_DEBUG("ClientDataDecoder::DoSendNotice:roomid=%d,idx = %d ,contextlen=%d",
		pRoomClient->m_roomid,pRoomClient->m_idx,len);

	//add by jinguanfu 2010/6/2
	//if(CheckRight(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
	if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
	{
		AC_ERROR("ClientDataDecoder::DoSendNotice:power error");
	 	return 0;
	}

	char outbuf[1024*100+100];
	BinaryWriteStream outstream(outbuf,sizeof(outbuf));

	char type = 65;
	outstream.Write(type);
	outstream.Write((short)ROOM_CMD_SB_SEND_NOTICE_TEMP);
	outstream.Write(seq);
	outstream.Write((int)pRoomClient->m_idx);
	outstream.Write(context, len);  
	outstream.Flush();
	if(g_pNetProxy->BroadcastSBCmd(proominfo,outstream) == -1)
	{
		AC_ERROR("ClientDataDecoder::DoSendNotice:g_pNetProxy->BroadcastSBCmd error");
		return -1;
	}

	return 0;
}



int ClientDataDecoder::DoInviteMic(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoInviteMic:cmd = %d seq = %d", cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoInviteMic:roominfo error");
		return -1;
	}

	if (!proominfo->isMicUpdown)
	{
		AC_ERROR("ClientDataDecoder::DoInviteMic:isMicUPDown is true");
		return -1;
	}

	int idx = 0;
	if(!pinstream->Read(idx))    //对象的idx
	{
		AC_ERROR("ClientDataDecoder::DoInviteMic:read r_idx error");
		return -1;
	}

	if(idx == 0)
	{
		AC_ERROR("ClientDataDecoder::DoInviteMic:r_idx error");
		return -1;
	}

	map<int, GM_INFO>::iterator itmGM =g_pNetProxy->m_GM.find(pRoomClient->m_idx);	
	map<int, int>::iterator itvj = proominfo->vjlist.find(pRoomClient->m_idx);

	if(pRoomClient->m_idx != proominfo->ownidx && pRoomClient->m_idx != proominfo->secondownidx && pRoomClient->m_idx!=proominfo->secondownidx2
		&& itmGM != g_pNetProxy->m_GM.end() && itvj == proominfo->vjlist.end())
	{
		AC_ERROR("ClientDataDecoder::DoInviteMic:power error");
		return 0;
	}

	map<int, RoomClient*>::iterator itu = proominfo->userlist.find(idx);
	if(itu == proominfo->userlist.end())
	{
		AC_ERROR("ClientDataDecoder::DoInviteMic:idx error");
		return -1;
	}
	
	AC_DEBUG("ClientDataDecoder::DoInviteMic:roomid = %d ,optidx=%d,boptidx = %d", 
		proominfo->roomid,pRoomClient->m_idx,idx);


	RoomClient* pSend = (*itu).second;
	BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
	char type = 65;
	outstream->Write(type);
	outstream->Write((short)ROOM_CMD_SB_INVITE_MIC);
	outstream->Write(seq);
	//outstream->Write((int)pRoomClient->m_idx);
	//outstream->Write(context, len);  
	outstream->Flush();
	if(g_pNetProxy->SendToSrv(pSend, *outstream) == -1)
	{
		AC_ERROR("ClientDataDecoder::DoInviteMic:SendTo %d error",idx);
		pSend->Close();
		return 0;
	}

	return 0;
}


int ClientDataDecoder::DoJoinRoomApp(ClientSocketBase *pClient,  short cmd, int seq/*, BinaryReadStream* pinstream*/)
{
	AC_DEBUG("ClientDataDecoder::DoJoinRoomApp:cmd = %d seq = %d", cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoJoinRoomApp:roominfo error");
		return -1;
	}

	AC_DEBUG("ClientDataDecoder::DoJoinRoomApp:roomid=%d,idx = %d ",proominfo->roomid,pRoomClient->m_idx);

	//1，判断是否为其它房间会员 ;2，如果不是则向所有会员发送广播，该用户申请为该房间会员;3,如果是,则提醒该用户需要退出其它房间
	bool isUser=false;
	map<int, ROOM_INFO*>::iterator it=	g_pNetProxy->m_roomlistinfo.roommap.begin();
	for (;it!=g_pNetProxy->m_roomlistinfo.roommap.end();it++)
	{
		ROOM_INFO* temp=it->second;
		map<int,int>::iterator itu=temp->userlistVIP.find(pRoomClient->m_idx);
		if (temp && itu != temp->userlistVIP.end())
		{
			isUser=true;
			break;
		}
	}


	//重复申请
	map<int,ROOM_APPLY>::iterator itAPP=proominfo->userlistAPP.find(pRoomClient->m_idx);
	if(itAPP!=proominfo->userlistAPP.end())
	{
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		char type = 65;
		outstream->Write(type);
		outstream->Write((short)ROOM_CMD_USER_APP_JOINROOM);
		outstream->Write(seq);
		outstream->Write((int)IAGAIN);		
		outstream->Flush();
		if(g_pNetProxy->SendToSrv(pRoomClient, *outstream) == -1)
		{
			AC_ERROR("ClientDataDecoder::DoJoinRoomApp:IAGAIN SendToSrvs error");
			return -1;
		}
	}
	else if (isUser)
	{
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		char type = 65;
		outstream->Write(type);
		outstream->Write((short)ROOM_CMD_USER_APP_JOINROOM);
		outstream->Write(seq);
		outstream->Write((int)ALREADY);		
		outstream->Flush();
		if(g_pNetProxy->SendToSrv(pRoomClient, *outstream) == -1)
		{
			AC_ERROR("ClientDataDecoder::DoJoinRoomApp:ALREADY SendToSrvs error");
			return -1;
		}
	}
	//房间会员已满
	else if(proominfo->userlistVIP.size()>=300)
	{
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		char type = 65;
		outstream->Write(type);
		outstream->Write((short)ROOM_CMD_USER_APP_JOINROOM);
		outstream->Write(seq);
		outstream->Write((int)LISTFULL);		
		outstream->Flush();
		if(g_pNetProxy->SendToSrv(pRoomClient, *outstream) == -1)
		{
			AC_ERROR("ClientDataDecoder::DoJoinRoomApp: LISTFULL SendToSrvs error");
			return -1;
		}
	}
	else if(proominfo->userlistAPP.size()>=300)
	{
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		char type = 65;
		outstream->Write(type);
		outstream->Write((short)ROOM_CMD_USER_APP_JOINROOM);
		outstream->Write(seq);
		outstream->Write((int)APPLYFULL);		
		outstream->Flush();
		if(g_pNetProxy->SendToSrv(pRoomClient, *outstream) == -1)
		{
			AC_ERROR("ClientDataDecoder::DoJoinRoomApp: APPLYFULL SendToSrvs error");
			return -1;
		}
	}
	else
	{	

		BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
		if(pDBSvr!=NULL)
		{
			int outseq = g_pNetProxy->GetCounter()->Get();
			BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
			static const char* spname = {"DBMW_ApplyRoomUser_HZSTAR"};//存储过程填充
			outstream->Write((short)CMD_CALLSP);
			outstream->Write(outseq);
			outstream->Write(spname,strlen(spname));
			outstream->Write((short)2);
			outstream->Write((char)PT_INPUT);
			outstream->Write((char)PDT_INT);
			outstream->Write((int)proominfo->roomid);
			outstream->Write((char)PT_INPUT);
			outstream->Write((char)PDT_INT);
			outstream->Write((int)pRoomClient->m_idx);		
			outstream->Flush();
			
			Dbresultdata* data=g_pNetProxy->CreateDBResultdata();		
			if(data != NULL)
			{
				if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1)
				{
					data->roomid = proominfo->roomid;
					data->opidx = pRoomClient->m_idx;
					data->cmd = cmd;
					data->seq=seq;
					data->outseq=outseq;
					data->number = pRoomClient->GetClientID();
					data->SetReactor(g_pNetProxy->GetReactor());
					data->RegisterTimer(DB_TIMEOUT);
					g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
				}
				else
				{
					AC_ERROR("ClientDataDecoder::DoJoinRoomApp: pDBSvr->AddBuf () error");
					g_pNetProxy->DestroyDBResult(data);
				}
			}
			else
			{
				AC_ERROR("ClientDataDecoder::DoJoinRoomApp: CreateDBResultdata error,data=%x",data);
			}

		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoJoinRoomApp: pDBSvr=%x ",pDBSvr);
		}
	}
	return 0;
}

int ClientDataDecoder::DoVerifyUserApp(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoVerifyUserApp:cmd = %d seq = %d", cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoVerifyUserApp:roominfo error");
		return -1;
	}

	int idx = 0;
	if(!pinstream->Read(idx))    //被操作对象的idx
	{
		AC_ERROR("ClientDataDecoder::DoVerifyUserApp:read r_idx error");
		return -1;
	}

	int ref = 0;
	if(!pinstream->Read(ref))    //是通过还是拒绝
	{
		AC_ERROR("ClientDataDecoder::DoVerifyUserApp:read ref error");
		return -1;
	}
	
	AC_DEBUG("ClientDataDecoder::DoVerifyUserApp:roomid=%d,pRoomClient->m_idx=%d,idx = %d,ref=%d ",
		proominfo->roomid,pRoomClient->m_idx,idx,ref);

	if(idx == 0)
	{
		AC_ERROR("ClientDataDecoder::DoVerifyUserApp:r_idx error");
		return -1;
	}


	//if(CheckRight(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
	if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
	{
		AC_ERROR("ClientDataDecoder::DoVerifyUserApp:power error");
	 	return -1;
	}

	/**add by jinguanfu 2010/4/9 <begin>**/
	//房间会员已满
	if(ref==1&&proominfo->userlistVIP.size()>=1000)
	{
			BinaryWriteStream* outstream1=StreamFactory::InstanceWriteStream();
			char type = 65;
			outstream1->Write(type);
			outstream1->Write(cmd);
			outstream1->Write(seq);
			outstream1->Write(LISTFULL);
			outstream1->Write(idx);
			outstream1->Write(ref);
			outstream1->Flush();

			if(g_pNetProxy->SendToSrv(pClient, *outstream1)==-1)
			{
				AC_ERROR("ClientDataDecoder::DoVerifyUserApp: SendToSrv error");
				return -1;
			}
			return 0;
	}
	/**add by jinguanfu 2010/4/9 <end>**/
	// 1，判断是通过还是不通过；2，如果成功，则向数据库操作并向其它会员广播；3，如果失败，则操作本地内存，并向其它会员广播
	BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
 	if(pDBSvr!=NULL)
 	{
		int outseq = g_pNetProxy->GetCounter()->Get();
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		static const char* spname = {"DBMW_OpRoomUserAdd_HZSTAR"};//存储过程填充
		outstream->Write((short)CMD_CALLSP);
		outstream->Write(outseq);
		outstream->Write(spname,strlen(spname));
		outstream->Write((short)3);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)proominfo->roomid);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)idx);	
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)ref);			
		outstream->Flush();
		
		Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
		if(data != NULL)
		{
			if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1)
			{
				data->roomid = proominfo->roomid;
				data->opidx = pRoomClient->m_idx;
				data->bopidx = idx;		
				data->cmd = cmd;
				data->seq=seq;
				data->outseq=outseq;
				data->number = pRoomClient->GetClientID();
				data->badd = ref;
				data->SetReactor(g_pNetProxy->GetReactor());
				data->RegisterTimer(DB_TIMEOUT);
				g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
			}
			else
			{
				AC_ERROR("ClientDataDecoder::DoVerifyUserApp: pDBSvr->AddBuf() error");
				g_pNetProxy->DestroyDBResult(data);
			}
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoVerifyUserApp: CreateDBResultdata error,data=%x",data);
		}
 	}
	else
	{
		AC_ERROR("ClientDataDecoder::DoVerifyUserApp: pDBSvr=%x ",pDBSvr);
	}
	return 0;
}

int ClientDataDecoder::DoRemoveUser(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoRemoveUser:cmd = %d seq = %d", cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoRemoveUser:roominfo error");
		return -1;
	}

	int idx = 0;
	if(!pinstream->Read(idx))    //被操作对象的idx
	{
		AC_ERROR("ClientDataDecoder::DoRemoveUser:read idx error");
		return -1;
	}
	
	AC_DEBUG("ClientDataDecoder::DoRemoveUser:roomid=%d,pRoomClient->m_idx=%d,idx = %d ",
		proominfo->roomid,pRoomClient->m_idx,idx);

	if(idx == 0)
	{
		AC_ERROR("ClientDataDecoder::DoRemoveUser:idx error");
		return -1;
	}
	
	//add by jinguanfu 2010/6/2
	map<int, RoomClient*>::iterator itu = proominfo->userlist.find(idx);
	//被操作者不在房间，则取得其身份
	if(itu==proominfo->userlist.end())
	{
		char b_identity=USER_ID_NONE;	//被操作者权限
			
		map<int, GM_INFO>::iterator itmGMU =g_pNetProxy->m_GM.find(idx);	
		map<int, int>::iterator itvj = proominfo->vjlist.find(idx);
		map<int, int>::iterator itvj_a = proominfo->vjlist_a.find(idx);
		map<int, int>::iterator itVIP = proominfo->userlistVIP.find(idx);
		if(itmGMU!=g_pNetProxy->m_GM.end())
				b_identity=USER_ID_GM;
		else if(idx==(int)proominfo->ownidx)
				b_identity=USER_ID_OWNER;
		else if(idx==(int)proominfo->secondownidx||idx==(int)proominfo->secondownidx2)
				b_identity= USER_ID_OWNER_S;
		else if(itvj!=proominfo->vjlist.end())
				b_identity=USER_ID_VJ;
		else if(itvj_a!=proominfo->vjlist_a.end())
				b_identity = USER_ID_VJ_A;
		else if(itVIP!=proominfo->userlistVIP.end())
				b_identity = USER_ID_VIP;

		//if(CheckRight(pRoomClient->m_identity, pRoomClient->m_viptype,b_identity,VIP_LEVEL_NONE, cmd)==0)
		if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,b_identity,VIP_LEVEL_NONE, cmd)==0)
		{
			AC_ERROR("ClientDataDecoder::DoRemoveUser:power error");
		 	return 0;
		}
		
	}
	else
	{
		RoomClient* pbClient=itu->second;
		//if(CheckRight(pRoomClient->m_identity, pRoomClient->m_viptype,pbClient->m_identity,pbClient->m_viptype, cmd)==0)
		if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,pbClient->m_identity,pbClient->m_viptype, cmd)==0)
		{
			AC_ERROR("ClientDataDecoder::DoRemoveUser:power error");
		 	return 0;
		}
	}

	BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
	if(pDBSvr!=NULL)
	{
		int outseq = g_pNetProxy->GetCounter()->Get();
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		static const char* spname = {"DBMW_DelRoomUser"};//存储过程填充
		outstream->Write((short)CMD_CALLSP);
		outstream->Write(outseq);
		outstream->Write(spname,strlen(spname));
		outstream->Write((short)2);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)proominfo->roomid);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)idx);		
		outstream->Flush();
		
		Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
		if(data != NULL)
		{
			if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1)
			{
		    		data->roomid = proominfo->roomid;
				data->opidx = pRoomClient->m_idx;
				data->bopidx = idx;		
				data->cmd = cmd;
				data->seq=seq;
				data->outseq=outseq;
				data->number = pRoomClient->GetClientID();
				data->SetReactor(g_pNetProxy->GetReactor());
				data->RegisterTimer(DB_TIMEOUT);
				g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
			}
			else
			{
				AC_ERROR("ClientDataDecoder::DoRemoveUser:pDBSvr->AddBuf() error");
			}
		}else
		{
			AC_ERROR("ClientDataDecoder::DoRemoveUser:CreateDBResultdata error,data=%x ",data);
			g_pNetProxy->DestroyDBResult(data);
		}
	}
	else
	{
		AC_ERROR("ClientDataDecoder::DoRemoveUser:pDBSvr=%x ",pDBSvr);
	}
	return 0;
}

int ClientDataDecoder::DoGiveVJA(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoGiveVJA:cmd = %d seq = %d", cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoGiveVJA:roominfo error");
		return -1;
	}

	int idx = 0;
	if(!pinstream->Read(idx))    //被操作对象的idx
	{
		AC_ERROR("ClientDataDecoder::DoGiveVJA:read idx error");
		return -1;
	}
	
	AC_DEBUG("ClientDataDecoder::DoGiveVJA:roomid=%d,pRoomClient->m_idx=%d,idx = %d ",
		proominfo->roomid,pRoomClient->m_idx,idx);

	if(idx == 0)
	{
		AC_ERROR("ClientDataDecoder::DoGiveVJA:idx error");
		return -1;
	}
	
	//add by jinguanfu 2010/6/2
	map<int, RoomClient*>::iterator itu = proominfo->userlist.find(idx);
	//被操作者不在房间，则取得其身份
	if(itu==proominfo->userlist.end())
	{
		char b_identity=USER_ID_NONE;	//被操作者权限
			
		map<int, GM_INFO>::iterator itmGMU =g_pNetProxy->m_GM.find(idx);	
		map<int, int>::iterator itvj = proominfo->vjlist.find(idx);
		map<int, int>::iterator itvj_a = proominfo->vjlist_a.find(idx);
		map<int, int>::iterator itVIP = proominfo->userlistVIP.find(idx);
		if(itmGMU!=g_pNetProxy->m_GM.end())
				b_identity=USER_ID_GM;
		else if(idx==(int)proominfo->ownidx)
				b_identity=USER_ID_OWNER;
		else if(idx==(int)proominfo->secondownidx||idx==(int)proominfo->secondownidx2)
				b_identity= USER_ID_OWNER_S;
		else if(itvj!=proominfo->vjlist.end())
				b_identity=USER_ID_VJ;
		else if(itvj_a!=proominfo->vjlist_a.end())
				b_identity = USER_ID_VJ_A;
		else if(itVIP!=proominfo->userlistVIP.end())
				b_identity = USER_ID_VIP;

		//if(CheckRight(pRoomClient->m_identity, pRoomClient->m_viptype,b_identity,VIP_LEVEL_NONE, cmd)==0)
		if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,b_identity,VIP_LEVEL_NONE, cmd)==0)
		{
			AC_ERROR("ClientDataDecoder::DoGiveVJA:power error");
		 	return 0;
		}
		
	}
	else
	{
		RoomClient* pbClient=itu->second;
		//if(CheckRight(pRoomClient->m_identity, pRoomClient->m_viptype,pbClient->m_identity,pbClient->m_viptype, cmd)==0)
		if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,pbClient->m_identity,pbClient->m_viptype, cmd)==0)
		{
			AC_ERROR("ClientDataDecoder::DoGiveVJA:power error");
		 	return 0;
		}
	}

	//助理主持不能超过50人
	if(proominfo->vjlist_a.size()>50)
	{
		char context[64]={0};
		BinaryWriteStream onestream(context,sizeof(context));
		char type=65;
		onestream.Write(type);
		onestream.Write((short)cmd);
		onestream.Write(seq);
		onestream.Write(-1);
		onestream.Write(idx);
		onestream.Flush();

		if(g_pNetProxy->SendToSrv(pRoomClient, onestream)==-1)
		{
			AC_ERROR("ClientDataDecoder::DoGiveVJA:SendToSrv error");
			return -1;
		}
			
		return 0;	
	}
	
	BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
	if(pDBSvr!=NULL)
	{
		int outseq = g_pNetProxy->GetCounter()->Get();
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		static const char* spname = {"DBMW_AddRoomRole"};//存储过程填充
		outstream->Write((short)CMD_CALLSP);
		outstream->Write(outseq);
		outstream->Write(spname,strlen(spname));
		outstream->Write((short)3);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)proominfo->roomid);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)idx);	
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)USER_ID_VJ_A);	
		outstream->Flush();
		
		Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
		if(data != NULL)
		{
			if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1)
			{
				data->roomid = proominfo->roomid;
				data->opidx = pRoomClient->m_idx;
				data->bopidx = idx;		
				data->cmd = cmd;
				data->seq=seq;
				data->outseq=outseq;
				data->number = pRoomClient->GetClientID();
				data->SetReactor(g_pNetProxy->GetReactor());
				data->RegisterTimer(DB_TIMEOUT);
				g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
			}
			else
			{
				AC_ERROR("ClientDataDecoder::DoGiveVJA:pDBSvr->AddBuf() Error ");
				g_pNetProxy->DestroyDBResult(data);
			}
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoGiveVJA:CreateDBResultdata error ,data=%x ",data);
		}
	}
	else
	{
		AC_ERROR("ClientDataDecoder::DoGiveVJA:pDBSvr=%x ",pDBSvr);
	}
	return 0;
	
}

int ClientDataDecoder::DoGiveVJ(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoGiveVJ:cmd = %d seq = %d", cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoGiveVJ:roominfo error");
		return -1;
	}

	int idx = 0;
	if(!pinstream->Read(idx))    //被操作对象的idx
	{
		AC_ERROR("ClientDataDecoder::DoGiveVJ:read idx error");
		return -1;
	}
	
	AC_DEBUG("ClientDataDecoder::DoGiveVJA:roomid=%d,pRoomClient->m_idx=%d,idx = %d ",
		proominfo->roomid,pRoomClient->m_idx,idx);

	if(idx == 0)
	{
		AC_ERROR("ClientDataDecoder::DoGiveVJ:idx error");
		return -1;
	}

	//add by jinguanfu 2010/6/2
	map<int, RoomClient*>::iterator itu = proominfo->userlist.find(idx);
	//被操作者不在房间，则取得其身份
	if(itu==proominfo->userlist.end())
	{
		char b_identity=USER_ID_NONE;	//被操作者权限
			
		map<int, GM_INFO>::iterator itmGMU =g_pNetProxy->m_GM.find(idx);	
		map<int, int>::iterator itvj = proominfo->vjlist.find(idx);
		map<int, int>::iterator itvj_a = proominfo->vjlist_a.find(idx);
		map<int, int>::iterator itVIP = proominfo->userlistVIP.find(idx);
		if(itmGMU!=g_pNetProxy->m_GM.end())
				b_identity=USER_ID_GM;
		else if(idx==(int)proominfo->ownidx)
				b_identity=USER_ID_OWNER;
		else if(idx==(int)proominfo->secondownidx||idx==(int)proominfo->secondownidx2)
				b_identity= USER_ID_OWNER_S;
		else if(itvj!=proominfo->vjlist.end())
				b_identity=USER_ID_VJ;
		else if(itvj_a!=proominfo->vjlist_a.end())
				b_identity = USER_ID_VJ_A;
		else if(itVIP!=proominfo->userlistVIP.end())
				b_identity = USER_ID_VIP;
		
		//if(CheckRight(pRoomClient->m_identity, pRoomClient->m_viptype,b_identity,VIP_LEVEL_NONE, cmd)==0)
		if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,b_identity,VIP_LEVEL_NONE, cmd)==0)
		{
			AC_ERROR("ClientDataDecoder::DoGiveVJ:power error");
		 	return 0;
		}
		
	}
	else
	{
		RoomClient* pbClient=itu->second;
		//if(CheckRight(pRoomClient->m_identity, pRoomClient->m_viptype,pbClient->m_identity,pbClient->m_viptype, cmd)==0)
		if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,pbClient->m_identity,pbClient->m_viptype, cmd)==0)
		{
			AC_ERROR("ClientDataDecoder::DoGiveVJ:power error");
		 	return 0;
		}
	}
	
	//主持人不能超过50人
	if(proominfo->vjlist.size()>50)
	{
		char context[64]={0};
		BinaryWriteStream onestream(context,sizeof(context));
		char type=65;
		onestream.Write(type);
		onestream.Write((short)cmd);
		onestream.Write(seq);
		onestream.Write(-1);
		onestream.Write(idx);
		onestream.Flush();

		if(g_pNetProxy->SendToSrv(pRoomClient, onestream)==-1)
		{
			AC_ERROR("ClientDataDecoder::DoGiveVJ:SendToSrv error");
			return -1;
		}
			
		return 0;	
	}
	
	BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
	if(pDBSvr!=NULL)
	{
		int outseq = g_pNetProxy->GetCounter()->Get();
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		static const char* spname = {"DBMW_AddRoomRole"};//存储过程填充
		outstream->Write((short)CMD_CALLSP);
		outstream->Write(outseq);
		outstream->Write(spname,strlen(spname));
		outstream->Write((short)3);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)proominfo->roomid);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)idx);	
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)USER_ID_VJ);	
		outstream->Flush();
		
		Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
		if(data != NULL)
		{
			if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1)
			{
				data->roomid = proominfo->roomid;
				data->opidx = pRoomClient->m_idx;
				data->bopidx = idx;		
				data->cmd = cmd;
				data->seq=seq;
				data->outseq=outseq;
				data->number = pRoomClient->GetClientID();
				data->SetReactor(g_pNetProxy->GetReactor());
				data->RegisterTimer(DB_TIMEOUT);
				g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
			}
			else
			{

				AC_ERROR("ClientDataDecoder::DoGiveVJ:pDBSvr->AddBuf() Error");
				g_pNetProxy->DestroyDBResult(data);
			}
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoGiveVJ:CreateDBResultdata() Error,data=%x ",data);
		}
	}
	else
	{
		AC_ERROR("ClientDataDecoder::DoGiveVJ:pDBSvr=%x ",pDBSvr);
	}
	return 0;
}
int ClientDataDecoder::DoGiveOwnS(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoGiveOwnS:cmd = %d seq = %d", cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoGiveOwnS:roominfo error");
		return -1;
	}

	int idx = 0;
	if(!pinstream->Read(idx))    //被操作对象的idx
	{
		AC_ERROR("ClientDataDecoder::DoGiveOwnS:read idx error");
		return -1;
	}

	AC_DEBUG("ClientDataDecoder::DoGiveOwnS:roomid=%d,pRoomClient->m_idx=%d,idx = %d ",
		proominfo->roomid,pRoomClient->m_idx,idx);

	if(idx == 0)
	{
		AC_ERROR("ClientDataDecoder::DoGiveOwnS:idx error");
		return -1;
	}
	
	//add by jinguanfu 2010/6/2
	map<int, RoomClient*>::iterator itu = proominfo->userlist.find(idx);
	//被操作者不在房间，则取得其身份
	if(itu==proominfo->userlist.end())
	{
		char b_identity=USER_ID_NONE;	//被操作者权限
			
		map<int, GM_INFO>::iterator itmGMU =g_pNetProxy->m_GM.find(idx);	
		map<int, int>::iterator itvj = proominfo->vjlist.find(idx);
		map<int, int>::iterator itvj_a = proominfo->vjlist_a.find(idx);
		map<int, int>::iterator itVIP = proominfo->userlistVIP.find(idx);
		if(itmGMU!=g_pNetProxy->m_GM.end())
				b_identity=USER_ID_GM;
		else if(idx==(int)proominfo->ownidx)
				b_identity=USER_ID_OWNER;
		else if(idx==(int)proominfo->secondownidx||idx==(int)proominfo->secondownidx2)
				b_identity= USER_ID_OWNER_S;
		else if(itvj!=proominfo->vjlist.end())
				b_identity=USER_ID_VJ;
		else if(itvj_a!=proominfo->vjlist_a.end())
				b_identity = USER_ID_VJ_A;
		else if(itVIP!=proominfo->userlistVIP.end())
				b_identity = USER_ID_VIP;
		
		//if(CheckRight(pRoomClient->m_identity, pRoomClient->m_viptype,b_identity,VIP_LEVEL_NONE, cmd)==0)
		if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,b_identity,VIP_LEVEL_NONE, cmd)==0)
		{
			AC_ERROR("ClientDataDecoder::DoGiveOwnS:power error");
		 	return 0;
		}
		
	}
	else
	{
		RoomClient* pbClient=itu->second;
		//if(CheckRight(pRoomClient->m_identity, pRoomClient->m_viptype,pbClient->m_identity,pbClient->m_viptype, cmd)==0)
		if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,pbClient->m_identity,pbClient->m_viptype, cmd)==0)
		{
			AC_ERROR("ClientDataDecoder::DoGiveOwnS:power error");
		 	return 0;
		}
	}

	if(proominfo->secondownidx!=0&&proominfo->secondownidx2!=0)
	{
		char context[64]={0};
		BinaryWriteStream onestream(context,sizeof(context));
		char type=65;
		onestream.Write(type);
		onestream.Write((short)cmd);
		onestream.Write(seq);
		onestream.Write(-1);
		onestream.Write(idx);
		onestream.Flush();

		if(g_pNetProxy->SendToSrv(pRoomClient, onestream)==-1)
		{
			AC_ERROR("ClientDataDecoder::DoGiveOwnS:SendToSrv error");
			return -1;
		}
			
		return 0;	
	}

	BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
	if(pDBSvr!=NULL)
	{
		int outseq = g_pNetProxy->GetCounter()->Get();
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		static const char* spname = {"DBMW_AddRoomRole"};//存储过程填充
		outstream->Write((short)CMD_CALLSP);
		outstream->Write(outseq);
		outstream->Write(spname,strlen(spname));
		outstream->Write((short)3);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)proominfo->roomid);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)idx);	
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)USER_ID_OWNER_S);	
		outstream->Flush();
		Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
		if(data != NULL)
		{
			if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1)
			{
				data->roomid = proominfo->roomid;
				data->opidx = pRoomClient->m_idx;
				data->bopidx = idx;		
				data->cmd = cmd;
				data->seq=seq;
				data->outseq=outseq;
				data->number = pRoomClient->GetClientID();
				data->SetReactor(g_pNetProxy->GetReactor());
				data->RegisterTimer(DB_TIMEOUT);
				g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
			}
			else
			{
				AC_ERROR("ClientDataDecoder::DoGiveOwnS:pDBSvr->AddBuf() error ");
				g_pNetProxy->DestroyDBResult(data);
			}
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoGiveOwnS:CreateDBResultdata() error,data=%x ",data);
		}
	}	
	else
	{
		AC_ERROR("ClientDataDecoder::DoGiveOwnS:pDBSvr=%x ",pDBSvr);
	}
	return 0;
}


int ClientDataDecoder::DoSetRoomPwd(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoSetRoomPwd:cmd = %d seq = %d", cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoSetRoomPwd:roominfo error");
		return -1;
	}
	char pwd[100] = {0};
	size_t len = 0;
	if(!pinstream->Read(pwd, sizeof(pwd), len)) 
	{
		AC_ERROR("ClientDataDecoder::DoSetRoomPwd:read context error");
		return -1;
	}
	pwd[len]=0;

	AC_DEBUG("ClientDataDecoder::DoSetRoomPwd:roomid=%d,m_idx=%d,pwdlen = %d ",
		proominfo->roomid,pRoomClient->m_idx,len);

	//add by jinguanfu 2010/6/2
	//if(CheckRight(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
	if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
	{
		AC_ERROR("ClientDataDecoder::DoSetRoomPwd:power error");
	 	return 0;
	}
	
	BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
	if(pDBSvr!=NULL)
	{
		int outseq = g_pNetProxy->GetCounter()->Get();
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		static const char* spname = {"DBMW_UpdateRoomPWD"};//存储过程填充
		outstream->Write((short)CMD_CALLSP);
		outstream->Write(outseq);
		outstream->Write(spname,strlen(spname));
		outstream->Write((short)2);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)proominfo->roomid);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_VARCHAR);
		outstream->Write(pwd,len);		
		outstream->Flush();
		Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
		if(data != NULL)
		{
			if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1)
			{
				data->roomid = proominfo->roomid;
				data->opidx = pRoomClient->m_idx;	
				data->bopidx=pRoomClient->m_idx;
				data->cmd = cmd;
				data->seq=seq;
				data->outseq=outseq;
				if(len>0)
				{
					memcpy(data->content,pwd,strlen(pwd));
				}
				else
					memset(data->content,0,sizeof(data->content));
				data->number=pRoomClient->GetClientID();
				data->SetReactor(g_pNetProxy->GetReactor());
				data->RegisterTimer(DB_TIMEOUT);
				g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
			}
			else
			{
				AC_ERROR("ClientDataDecoder::DoSetRoomPwd:pDBSvr->AddBuf() error");
				g_pNetProxy->DestroyDBResult(data);
			}
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoSetRoomPwd:CreateDBResultdata() error,data=%x",data);
		}		
	}	
	else
	{
		AC_ERROR("ClientDataDecoder::DoSetRoomPwd:pDBSvr=%x ",pDBSvr);
	}
	return 0;
}

int ClientDataDecoder::DoSetRoomLock(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoSetRoomLock:cmd = %d seq = %d", cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoSetRoomLock:roominfo error");
		return -1;
	}	

	int ref = 0;
	if(!pinstream->Read(ref))    //锁定、开启flag
	{
		AC_ERROR("ClientDataDecoder::DoSetRoomLock:read ref error");
		return -1;
	}

	AC_DEBUG("ClientDataDecoder::DoSetRoomLock:roomid=%d,m_idx=%d,ref = %d ",
		proominfo->roomid,pRoomClient->m_idx,ref);

	//add by jinguanfu 2010/6/2
	//if(CheckRight(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
	if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
	{
		AC_ERROR("ClientDataDecoder::DoSetRoomLock:power error");
	 	return 0;
	}
	
	BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
	if(pDBSvr!=NULL)
	{
		int outseq = g_pNetProxy->GetCounter()->Get();
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		static const char* spname = {"DBMW_UpdateRoomLock"};//存储过程填充
		outstream->Write((short)CMD_CALLSP);
		outstream->Write(outseq);
		outstream->Write(spname,strlen(spname));
		outstream->Write((short)2);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)proominfo->roomid);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)ref);		
		outstream->Flush();
		
		Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
		if(data != NULL)
		{
			if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1)
			{
				data->roomid = proominfo->roomid;
				data->opidx = pRoomClient->m_idx;	
				data->bopidx=ref;
				data->cmd = cmd;
				data->seq=seq;
				data->outseq=outseq;
				data->number = pRoomClient->GetClientID();
				data->SetReactor(g_pNetProxy->GetReactor());
				data->RegisterTimer(DB_TIMEOUT);
				g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
			}
			else
			{
				AC_ERROR("ClientDataDecoder::DoSetRoomLock:pDBSvr->AddBuf() Error ");
				g_pNetProxy->DestroyDBResult(data);
			}
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoSetRoomLock:CreateDBResultdata() error,data=%x",data);
		}
	}
	else
	{
		AC_ERROR("ClientDataDecoder::DoSetRoomLock:pDBSvr=%x ",pDBSvr);
	}
	return 0;
}

int ClientDataDecoder::DoSetUserOnly(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoSetUserOnly:cmd = %d seq = %d", cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoSetUserOnly:roominfo error");
		return -1;
	}	

	int ref = 0;
	if(!pinstream->Read(ref))    //操作的参数
	{
		AC_ERROR("ClientDataDecoder::DoSetUserOnly:read ref error");
		return -1;
	}
	
	AC_DEBUG("ClientDataDecoder::DoSetUserOnly:roomid=%d,m_idx=%d,ref = %d ",
		proominfo->roomid,pRoomClient->m_idx,ref);

	//add by jinguanfu 2010/6/2
	//if(CheckRight(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
	if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
	{
		AC_ERROR("ClientDataDecoder::DoSetUserOnly:power error");
	 	return -1;
	}
	
	BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
	if(pDBSvr!=NULL)
	{
		int outseq = g_pNetProxy->GetCounter()->Get();
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		static const char* spname = {"DBMW_UpdateRoomUserOnly"};//存储过程填充
		outstream->Write((short)CMD_CALLSP);
		outstream->Write(outseq);
		outstream->Write(spname,strlen(spname));
		outstream->Write((short)2);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)proominfo->roomid);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)ref);		
		outstream->Flush();
		Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
		if(data != NULL)
		{
			if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1)
			{
				data->roomid = proominfo->roomid;
				data->opidx = pRoomClient->m_idx;	
				data->bopidx= ref;
				data->cmd = cmd;
				data->seq=seq;
				data->outseq=outseq;
				data->number = pRoomClient->GetClientID();
				data->SetReactor(g_pNetProxy->GetReactor());
				data->RegisterTimer(DB_TIMEOUT);
				g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
			}
			else
			{
				AC_ERROR("ClientDataDecoder::DoSetUserOnly:pDBSvr->AddBuf() error");
				g_pNetProxy->DestroyDBResult(data);
			}
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoSetUserOnly:CreateDBResultdata() error,data=%x",data);
		}
	}
	else
	{
		AC_ERROR("ClientDataDecoder::DoSetUserOnly:pDBSvr=%x",pDBSvr);

	}
	return 0;
}

int ClientDataDecoder::DoSetUserInOut(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoSetUserInOut:cmd = %d seq = %d", cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoSetUserInOut:roominfo error");
		return -1;
	}	

	int ref = 0;
	if(!pinstream->Read(ref))    //操作的参数
	{
		AC_ERROR("ClientDataDecoder::DoSetUserInOut:read ref error");
		return -1;
	}
	
	AC_DEBUG("ClientDataDecoder::DoSetUserInOut:roomid=%d,m_idx=%d,ref = %d ",
		proominfo->roomid,pRoomClient->m_idx,ref);

	//add by jinguanfu 2010/6/2
	//if(CheckRight(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
	if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
	{
		AC_ERROR("ClientDataDecoder::DoSetUserInOut:power error");
	 	return -1;
	}
	
	BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
	if(pDBSvr!=NULL)
	{
		int outseq = g_pNetProxy->GetCounter()->Get();
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		static const char* spname = {"DBMW_UpdateRoomInOut"};//存储过程填充
		outstream->Write((short)CMD_CALLSP);
		outstream->Write(outseq);
		outstream->Write(spname,strlen(spname));
		outstream->Write((short)2);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)proominfo->roomid);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)ref);		
		outstream->Flush();
		Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
		if(data != NULL)
		{
			if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1)
			{
				data->roomid = proominfo->roomid;
				data->opidx = pRoomClient->m_idx;	
				data->bopidx=ref;
				data->cmd = cmd;
				data->seq=seq;
				data->outseq=outseq;
				data->number = pRoomClient->GetClientID();
				data->SetReactor(g_pNetProxy->GetReactor());
				data->RegisterTimer(DB_TIMEOUT);
				g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
			}
			else
			{
				AC_ERROR("ClientDataDecoder::DoSetUserInOut:pDBSvr->AddBuf() Error");
				g_pNetProxy->DestroyDBResult(data);
			}	
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoSetUserInOut:CreateDBResultdata() Error,data=%x ",data);
		}
	}
	else
	{
		AC_ERROR("ClientDataDecoder::DoSetUserInOut:pDBSvr=%x ",pDBSvr);
	}
	
	return 0;
}

int ClientDataDecoder::DoSetMicUpDown(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoSetMicUpDown:cmd = %d seq = %d", cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoSetMicUpDown:roominfo error");
		return -1;
	}	

	char ref = 0;
	if(!pinstream->Read(ref))    //操作的参数
	{
		AC_ERROR("ClientDataDecoder::DoSetMicUpDown:read ref error");
		return -1;
	}

	AC_DEBUG("ClientDataDecoder::DoSetMicUpDown:roomid=%d,m_idx=%d,ref = %d ",
		proominfo->roomid,pRoomClient->m_idx,ref);

	//add by jinguanfu 2010/6/2
	//if(CheckRight(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
	if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
	{
		AC_ERROR("ClientDataDecoder::DoSetMicUpDown:power error");
	 	return -1;
	}

	BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
	if(pDBSvr!=NULL)
	{
		int outseq = g_pNetProxy->GetCounter()->Get();
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		static const char* spname = {"DBMW_UpdateRoomMicUpDown"};//存储过程填充
		outstream->Write((short)CMD_CALLSP);
		outstream->Write(outseq);
		outstream->Write(spname,strlen(spname));
		outstream->Write((short)2);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)proominfo->roomid);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)ref);		
		outstream->Flush();
		
		Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
		if(data != NULL)
		{
			if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1)
			{
				data->roomid = proominfo->roomid;
				data->opidx = pRoomClient->m_idx;	
				data->bopidx=pRoomClient->m_idx;
				data->badd=ref;
				data->cmd = cmd;
				data->seq=seq;
				data->outseq=outseq;
				data->number = pRoomClient->GetClientID();
				data->SetReactor(g_pNetProxy->GetReactor());
				data->RegisterTimer(DB_TIMEOUT);
				g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
			}
			else
			{
				AC_ERROR("ClientDataDecoder::DoSetMicUpDown:pDBSvr->AddBuf() error");
				g_pNetProxy->DestroyDBResult(data);
			}
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoSetMicUpDown:CreateDBResultdata error,data=%x",data);
		}
	}
	else
	{
		AC_ERROR("ClientDataDecoder::DoSetMicUpDown:pDBSvr=%x ",pDBSvr);
	}
	return 0;
}

int ClientDataDecoder::DoSetRoomName(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoSetRoomName:cmd = %d seq = %d", cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoSetRoomName:roominfo error");
		return -1;
	}
	char name[51] = {0};
	size_t len = 0;
	if(!pinstream->Read(name, sizeof(name)-1, len)) 
	{
		AC_ERROR("ClientDataDecoder::DoSetRoomName:read context error");
		return -1;
	}
	name[len]=0;
	
	AC_DEBUG("ClientDataDecoder::DoSetRoomName:roomid=%d,m_idx=%d,len = %d ",
		proominfo->roomid,pRoomClient->m_idx,len);

	if(len == 0)
	{
		AC_ERROR("ClientDataDecoder::DoSetRoomName:context error");
		return -1;
	}


	//add by jinguanfu 2010/6/2
	//if(CheckRight(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
	if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
	{
		AC_ERROR("ClientDataDecoder::DoSetRoomName:power error");
	 	return -1;
	}
	
	BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
	if(pDBSvr!=NULL)
	{
		int outseq = g_pNetProxy->GetCounter()->Get();
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		static const char* spname = {"DBMW_UpdateRoomName"};//存储过程填充
		outstream->Write((short)CMD_CALLSP);
		outstream->Write(outseq);
		outstream->Write(spname,strlen(spname));
		outstream->Write((short)2);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)proominfo->roomid);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_VARCHAR);
		outstream->Write(name,len);		
		outstream->Flush();
		Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
		if(data != NULL)
		{
			if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1)
			{
				data->roomid = proominfo->roomid;
				data->opidx = pRoomClient->m_idx;	
				data->bopidx = pRoomClient->m_idx;	
				data->cmd = cmd;
				memset(data->content,0,sizeof(data->content));
				strncpy(data->content,name,strlen(name));	
				data->seq=seq;
				data->outseq=outseq;
				data->number = pRoomClient->GetClientID();
				data->SetReactor(g_pNetProxy->GetReactor());
				data->RegisterTimer(DB_TIMEOUT);
				g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
			}
			else
			{
				AC_ERROR("ClientDataDecoder::DoSetRoomName:pDBSvr->AddBuf() Error");
				g_pNetProxy->DestroyDBResult(data);
			}
			
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoSetRoomName:CreateDBResultdata error,data=%x",data);
		}	
	}
	else
	{
		AC_ERROR("ClientDataDecoder::DoSetRoomName:pDBSvr=%x ",pDBSvr);
	}
	return 0;
}

int ClientDataDecoder::DoSetChatPublic(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoSetChatPublic:cmd = %d seq = %d", cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoSetChatPublic:roominfo error");
		return -1;
	}	

	int ref = 0;
	if(!pinstream->Read(ref))    //操作的参数
	{
		AC_ERROR("ClientDataDecoder::DoSetChatPublic:read ref error");
		return -1;
	}
	
	AC_DEBUG("ClientDataDecoder::DoSetChatPublic:roomid=%d,m_idx=%d,ref = %d ",
		proominfo->roomid,pRoomClient->m_idx,ref);

	//add by jinguanfu 2010/6/2
	//if(CheckRight(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
	if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
	{
		AC_ERROR("ClientDataDecoder::DoSetChatPublic:power error");
	 	return 0;
	}
	
	BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
	if(pDBSvr!=NULL)
	{
		int outseq = g_pNetProxy->GetCounter()->Get();
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		static const char* spname = {"DBMW_UpdateRoomChatPublic"};//存储过程填充
		outstream->Write((short)CMD_CALLSP);
		outstream->Write(outseq);
		outstream->Write(spname,strlen(spname));
		outstream->Write((short)2);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)proominfo->roomid);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)ref);		
		outstream->Flush();
		Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
		if(data != NULL)
		{
			if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1)
			{
				data->roomid = proominfo->roomid;
				data->opidx = pRoomClient->m_idx;
				data->bopidx=ref;
				data->cmd = cmd;
				data->seq=seq;
				data->outseq=outseq;
				data->number = pRoomClient->GetClientID();
				data->SetReactor(g_pNetProxy->GetReactor());
				data->RegisterTimer(DB_TIMEOUT);
				g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
			}
			else
			{
				AC_ERROR("ClientDataDecoder::DoSetChatPublic:pDBSvr->AddBuf() error");
				g_pNetProxy->DestroyDBResult(data);
			}
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoSetChatPublic:CreateDBResultdata error,data=%x",data);
		}
	}
	else
	{
		AC_ERROR("ClientDataDecoder::DoSetChatPublic:pDBSvr=%x ",pDBSvr);
	}
	return 0;
}

int ClientDataDecoder::DoSetRoomWelcome(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoSetRoomWelcome:cmd = %d seq = %d", cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoSetRoomPwd:roominfo error");
		return -1;
	}
	char welcome[201] = {0};
	size_t len = 0;
	if(!pinstream->Read(welcome, sizeof(welcome)-1, len)) 
	{
		AC_ERROR("ClientDataDecoder::DoSetRoomWelcome:read context error");
		return -1;
	}
	welcome[len]=0;
	
	AC_DEBUG("ClientDataDecoder::DoSetRoomWelcome:roomid=%d,idx=%d,context len =%d ",
		proominfo->roomid,pRoomClient->m_idx,len);
	/*
	if(len == 0)
	{
		AC_ERROR("ClientDataDecoder::DoSetRoomWelcome:context error");
		return -1;
	}
	*/
	//add by jinguanfu 2010/6/2
	//if(CheckRight(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
	if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
	{
		AC_ERROR("ClientDataDecoder::DoSetRoomWelcome:power error");
	 	return 0;
	}

	BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
	if(pDBSvr!=NULL)
	{
		int outseq = g_pNetProxy->GetCounter()->Get();
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		static const char* spname = {"DBMW_UpdateRoomWelcome"};//存储过程填充
		outstream->Write((short)CMD_CALLSP);
		outstream->Write(outseq);
		outstream->Write(spname,strlen(spname));
		outstream->Write((short)2);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)proominfo->roomid);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_VARCHAR);
		outstream->Write(welcome,len);		
		outstream->Flush();
		
		Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
		if(data != NULL)
		{
			if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1)
			{
				data->roomid = proominfo->roomid;
				data->opidx = pRoomClient->m_idx;			
				data->cmd = cmd;
				memset(data->content,0,sizeof(data->content));
				strncpy(data->content,welcome,strlen(welcome));
				data->seq=seq;
				data->outseq=outseq;
				data->number = pRoomClient->GetClientID();
				data->SetReactor(g_pNetProxy->GetReactor());
				data->RegisterTimer(DB_TIMEOUT);
				g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
			}
			else
			{
				AC_ERROR("ClientDataDecoder::DoSetRoomWelcome:pDBSvr->AddBuf() error");
				g_pNetProxy->DestroyDBResult(data);
			}
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoSetRoomWelcome:CreateDBResultdata error,data=%x",data);
		}
	}	
	else
	{
		AC_ERROR("ClientDataDecoder::DoSetRoomWelcome:pDBSvr=%x ",pDBSvr);
	}
	return 0;
}

int ClientDataDecoder::DoSetRoomLogo(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoSetRoomLogo:cmd = %d seq = %d", cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoSetRoomLogo:roominfo error");
		return -1;
	}
	char logo[201] = {0};
	size_t len = 0;
	if(!pinstream->Read(logo, sizeof(logo)-1, len)) 
	{
		AC_ERROR("ClientDataDecoder::DoSetRoomLogo:read context error");
		return -1;
	}
	logo[len]=0;
	AC_DEBUG("ClientDataDecoder::DoSetRoomLogo:roomid=%d, idx=%d,context len=%d",
		proominfo->roomid,pRoomClient->m_idx,len);
	/*
	if(len == 0)
	{
		AC_ERROR("ClientDataDecoder::DoSetRoomLogo:context error");
		return -1;
	}
	*/
	//add by jinguanfu 2010/6/2
	//if(CheckRight(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
	if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
	{
		AC_ERROR("ClientDataDecoder::DoSetRoomLogo:power error");
	 	return 0;
	}
	
	BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
	if(pDBSvr!=NULL)
	{
		int outseq = g_pNetProxy->GetCounter()->Get();
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		static const char* spname = {"DBMW_UpdateRoomLogo"};//存储过程填充
		outstream->Write((short)CMD_CALLSP);
		outstream->Write(outseq);
		outstream->Write(spname,strlen(spname));
		outstream->Write((short)2);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)proominfo->roomid);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_VARCHAR);
		outstream->Write(logo,len);		
		outstream->Flush();
		
		Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
		if(data != NULL)
		{
			if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1)
			{
				data->roomid = proominfo->roomid;
				data->opidx = pRoomClient->m_idx;	
				data->bopidx = pRoomClient->m_idx;
				data->cmd = cmd;
				memset(data->content,0,sizeof(data->content));
				strncpy(data->content,logo,strlen(logo));
				data->seq=seq;
				data->outseq=outseq;
				data->number = pRoomClient->GetClientID();
				data->SetReactor(g_pNetProxy->GetReactor());
				data->RegisterTimer(DB_TIMEOUT);
				g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
			}
			else
			{
				AC_ERROR("ClientDataDecoder::DoSetRoomLogo:pDBSvr->AddBuf() error");
				g_pNetProxy->DestroyDBResult(data);
			}
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoSetRoomLogo:CreateDBResultdata error,data=%x",data);
		}
	}
	else
	{
		AC_ERROR("ClientDataDecoder::DoSetRoomLogo:pDBSvr=%x ",pDBSvr);
	}
	return 0;
}

int ClientDataDecoder::DoReturnRoomApplyList(ClientSocketBase *pClient,  short cmd, int seq/*, BinaryReadStream* pinstream*/)
{
	AC_DEBUG("ClientDataDecoder::DoReturnRoomApplyList:cmd = %d seq = %d", cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoReturnRoomApplyList:roominfo error");
		return -1;
	}
	
	AC_DEBUG("ClientDataDecoder::DoReturnRoomApplyList: roomid=%d,idx=%d",
		proominfo->roomid,pRoomClient->m_idx);

	int roomid=pRoomClient->m_roomid;

	if (roomid==0)
	{
		AC_ERROR("ClientDataDecoder::DoReturnRoomApplyList:roomid error");
		return -1;
	}

	//add by jinguanfu 2010/6/2
	//if(CheckRight(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
	if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
	{
		AC_ERROR("ClientDataDecoder::DoReturnRoomApplyList:power error");
	 	return 0;
	}
	

	static char outbuf[16384] = {0}; //16k
	BinaryWriteStream outstream(outbuf,sizeof(outbuf));
	//BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();	
	char type = 65;
	outstream.Write(type);
	outstream.Write(cmd);
	outstream.Write(seq);		
	int usernum = proominfo->userlistAPP.size();
	outstream.Write(usernum);
	map<int, ROOM_APPLY>::iterator itAPP = proominfo->userlistAPP.begin();
	for(; itAPP != proominfo->userlistAPP.end(); itAPP++)
	{	
		ROOM_APPLY roomapply=itAPP->second;
		outstream.Write((int)roomapply.m_idx);	
		outstream.Write((int)roomapply.m_roomid);	
		outstream.Write((int)0);	//未处理的state为0
		outstream.Write(roomapply.m_time,strlen(roomapply.m_time));
	}
	outstream.Flush();

	if(g_pNetProxy->SendToSrv(pRoomClient,outstream)==-1)
	{
		AC_ERROR("ClientDataDecoder::DoReturnRoomApplyList:SendToSrv error ");
		return -1;
	}
	
	return 0;
}




int ClientDataDecoder::DoReturnRoomBlackList(ClientSocketBase *pClient,  short cmd, int seq/*, BinaryReadStream* pinstream*/)
{
	AC_DEBUG("ClientDataDecoder::DoReturnRoomBlackList:cmd = %d seq = %d", cmd ,seq);

	
	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoReturnRoomBlackList:roominfo error");
		return -1;
	}
	
	AC_DEBUG("ClientDataDecoder::DoReturnRoomBlackList:roomid=%d, idx=%d",proominfo->roomid,pRoomClient->m_idx);

	int roomid=pRoomClient->m_roomid;

	if (roomid==0)
	{
		AC_ERROR("ClientDataDecoder::DoReturnRoomBlackList:rooid error");
		return -1;
	}
	//add by jinguanfu 2010/6/2
	//if(CheckRight(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
	if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
	{
		AC_ERROR("ClientDataDecoder::DoReturnRoomBlackList:power error");
	 	return 0;
	}
	

	static char outbuf[16384] = {0}; //16k
	BinaryWriteStream outstream(outbuf,sizeof(outbuf));
	//BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
	char type=65;
	outstream.Write(type);
	outstream.Write((short)cmd);
	outstream.Write((int)seq);
	short usernum = proominfo->blacklist.size();
	outstream.Write(usernum);
	map<int, int>::iterator itu = proominfo->blacklist.begin();
	for(; itu != proominfo->blacklist.end(); itu++)
	{		
		outstream.Write((int)itu->second);		
	}
	outstream.Flush();
	AC_DEBUG("ClientDataDecoder::DoReturnRoomBlackList:Blacklist size:%d",usernum);
	
	if(g_pNetProxy->SendToSrv(pRoomClient,outstream)==-1)
	{
		AC_ERROR("ClientDataDecoder::DoReturnRoomBlackList:SendToSrv error ");
		return -1;
	}

	return 0;
}




int ClientDataDecoder::DoReturnRoomMemberList(ClientSocketBase *pClient,  short cmd, int seq/*, BinaryReadStream* pinstream*/)
{
	AC_DEBUG("ClientDataDecoder::DoReturnRoomMemberList:cmd = %d seq = %d", cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoReturnRoomMemberList:roominfo error");
		return -1;
	}

	AC_DEBUG("ClientDataDecoder::DoReturnRoomMemberList:roomid=%d, idx=%d",
		proominfo->roomid,pRoomClient->m_idx);

	int roomid=pRoomClient->m_roomid;

	if (roomid==0)
	{
		AC_ERROR("ClientDataDecoder::DoReturnRoomMemberList:roomid error");
		return -1;
	}

	//add by jinguanfu 2010/6/2
//	if(CheckRight(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
	if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
	{
		AC_ERROR("ClientDataDecoder::DoReturnRoomMemberList:power error");
	 	return 0;
	}

	static char outbuf[16384] = {0}; //16k
	BinaryWriteStream outstream(outbuf,sizeof(outbuf));
	//BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
	char type=65;
	outstream.Write(type);
	outstream.Write((short)cmd);
	outstream.Write((int)seq);
	short usernum = proominfo->userlistVIP.size();
	outstream.Write(usernum);
	map<int, int>::iterator itu = proominfo->userlistVIP.begin();
	for(; itu != proominfo->userlistVIP.end(); itu++)
	{		
		outstream.Write((int)itu->second);		
	}
	outstream.Flush();
	if(g_pNetProxy->SendToSrv(pRoomClient,outstream)==-1)
	{
		AC_ERROR("ClientDataDecoder::DoReturnRoomMemberList:SendToSrv error ");
		return -1;
	}

	return 0;
}

//add by jinguanfu 2010/4/11 用户退出房间会员
int ClientDataDecoder::DoExitRoomMember(ClientSocketBase *pClient,  short cmd, int seq/*,BinaryReadStream* pinstream*/)
{
	AC_DEBUG("ClientDataDecoder::DoExitRoomMember:cmd = %d seq = %d", cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoExitRoomMember:roominfo error");
		return -1;
	}

	AC_DEBUG("ClientDataDecoder::DoExitRoomMember: roomid=%d,idx=%d",
		proominfo->roomid,pRoomClient->m_idx);

	int roomid=pRoomClient->m_roomid;
	if (roomid==0)
	{
		AC_ERROR("ClientDataDecoder::DoExitRoomMember:rooid error");
		return -1;
	}
	//室主不能退出房间会员
	if(pRoomClient->m_idx==proominfo->ownidx)
	{
		char context[64] = {0};
		BinaryWriteStream outstream(context,sizeof(context));
		char type = 65;
		outstream.Write(type);
		outstream.Write((short)cmd);
		outstream.Write(seq);
		outstream.Write((int)0);	//0--失败(非房间会员)， 1--成功	
		outstream.Flush();
		if(g_pNetProxy->SendToSrv(pClient, outstream)==-1)
		{
			AC_ERROR("ClientDataDecoder::DoExitRoomMember:SendToSrv error");
			return -1;
		}
		return 0;
	}
	
	//检查是否是房间会员
	map<int,int>::iterator itVIP=proominfo->userlistVIP.find(pRoomClient->m_idx);
	if(itVIP==proominfo->userlistVIP.end())
	{
		char context[64] = {0};
		BinaryWriteStream outstream(context,sizeof(context));
		char type = 65;
		outstream.Write(type);
		outstream.Write((short)cmd);
		outstream.Write(seq);
		outstream.Write((int)0);	//0--失败(非房间会员)， 1--成功	
		outstream.Flush();
		if(g_pNetProxy->SendToSrv(pClient, outstream)==-1)
		{
			AC_ERROR("ClientDataDecoder::DoExitRoomMember:SendToSrv error");
			return -1;
		}
		return 0;
	}
	
	BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
	if(pDBSvr!=NULL)
	{
		int outseq = g_pNetProxy->GetCounter()->Get();
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		static const char* spname = {"DBMW_DelRoomUser"};//存储过程填充
		outstream->Write((short)CMD_CALLSP);
		outstream->Write(outseq);
		outstream->Write(spname,strlen(spname));
		outstream->Write((short)2);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)proominfo->roomid);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)pRoomClient->m_idx);		
		outstream->Flush();
		Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
		if(data != NULL)
		{
			if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1)
			{
				data->roomid = proominfo->roomid;
				data->opidx = pRoomClient->m_idx;
				data->bopidx = pRoomClient->m_idx;		
				data->cmd = cmd;
				data->seq=seq;
				data->outseq=outseq;
				data->number = pRoomClient->GetClientID();
				data->SetReactor(g_pNetProxy->GetReactor());
				data->RegisterTimer(DB_TIMEOUT);
				g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
			}
			else
			{
				AC_ERROR("ClientDataDecoder::DoExitRoomMember:pDBSvr->AddBuf() error ");
				g_pNetProxy->DestroyDBResult(data);
			}
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoExitRoomMember:CreateDBResultdata error, data=%x ",data);
		}
	}
	else
	{
		AC_ERROR("ClientDataDecoder::DoExitRoomMember:pDBSvr=%x ",pDBSvr);
	}
	return 0;
	
}
int ClientDataDecoder::DoGetPanelInfo(ClientSocketBase *pClient,  short cmd, int seq/*,BinaryReadStream* pinstream*/)
{
	AC_DEBUG("ClientDataDecoder::DoGetPanelInfo:cmd = %d seq = %d", cmd ,seq);
	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoGetPanelInfo:roominfo error");
		return -1;
	}
	
	AC_DEBUG("ClientDataDecoder::DoGetPanelInfo:roomid=%d, idx=%d",
		proominfo->roomid,pRoomClient->m_idx);

	int roomid=pRoomClient->m_roomid;
	if (roomid==0)
	{
		AC_ERROR("ClientDataDecoder::DoGetPanelInfo:roomid error");
		return -1;
	}

	//返回房间设置信息
	static char outbuf[16384] = {0}; //16k
	BinaryWriteStream outstream(outbuf,sizeof(outbuf));
	//BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
	char type=65;
	outstream.Write(type);
	outstream.Write((short)cmd);
	outstream.Write((int)seq);
	outstream.Write((int)0);		//ret
	if(strlen(proominfo->passwd)>0)
		outstream.Write((char)1);		//pwdflag
	else
		outstream.Write((char)0);		//pwdflag	
		
	//close
	if(proominfo->isClose)
		outstream.Write((char)1);			
	else
		outstream.Write((char)0);
	//useronly
	if(proominfo->isUserOnly)
		outstream.Write((char)1);			
	else
		outstream.Write((char)0);
	
	//userinout
	if(proominfo->isUserInOut)
		outstream.Write((char)1);			
	else
		outstream.Write((char)0);
		
	//freemic	
	if(proominfo->isMicUpdown)
		outstream.Write((char)1);			
	else
		outstream.Write((char)0);
	
	//房间名
	outstream.Write(proominfo->roomname,strlen(proominfo->roomname));	
	//房间公告
	//AC_DEBUG("ClientDataDecoder::DoGetPanelInfo:content len=%d",strlen(proominfo->content));
	outstream.Write(proominfo->content,strlen(proominfo->content));
	//chatflag
	if(proominfo->isPublicChat)	
		outstream.Write((char)1);			
	else
		outstream.Write((char)0);
	
	//欢迎词
	outstream.Write(proominfo->welcomeword,strlen(proominfo->welcomeword));	
	//LOGO
	outstream.Write(proominfo->roomlogo,strlen(proominfo->roomlogo));	

	//add by jinguanfu 2010/12/6
	//副室主以上都取得房间密码
	map<int,GM_INFO>::iterator itGM=g_pNetProxy->m_GM.find(pRoomClient->m_idx);
	if(itGM!=g_pNetProxy->m_GM.end()
		||pRoomClient->m_idx==proominfo->ownidx
		||pRoomClient->m_idx==proominfo->secondownidx
		||pRoomClient->m_idx==proominfo->secondownidx2)
	{
		outstream.Write(proominfo->passwd,strlen(proominfo->passwd));
	}
	else
	{
		char temp[25]={0};
		outstream.Write(temp,strlen(temp));
	}


	//auto onmic
	if(proominfo->isAutoOnmic)
		outstream.Write((char)1);	
	else
		outstream.Write((char)0);	
	outstream.Flush();

	if(g_pNetProxy->SendToSrv(pRoomClient,outstream)==-1)
	{
		AC_ERROR("ClientDataDecoder::DoGetPanelInfo:SendToSrv error ");
		return -1;
	}

	return 0;
	
}

int ClientDataDecoder::DoGiveMember(ClientSocketBase *pClient,  short cmd, int seq,BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoGiveMember:cmd = %d seq = %d", cmd ,seq);
	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoGiveMember:roominfo error");
		return -1;
	}

	int roomid=pRoomClient->m_roomid;
	if (roomid==0)
	{
		AC_ERROR("ClientDataDecoder::DoGiveMember:roomid error");
		return -1;
	}

	int idx = 0;
	if(!pinstream->Read(idx))    //被操作对象的idx
	{
		AC_ERROR("ClientDataDecoder::DoGiveMember:read idx error");
		return -1;
	}
	
	AC_DEBUG("ClientDataDecoder::DoGiveMember:roomid=%d,pRoomClient->m_idx=%d,idx=%d",
		proominfo->roomid,pRoomClient->m_idx,idx);

	if(idx == 0)
	{
		AC_ERROR("ClientDataDecoder::DoGiveMember:idx error");
		return -1;
	}

	//被操作者必须为房间会员
	map<int,int>::iterator itVIP = proominfo->userlistVIP.find(idx);
	if(itVIP==proominfo->userlistVIP.end())
	{
		AC_ERROR("ClientDataDecoder::DoGiveMember: idx=%d not member in room=%d",idx,proominfo->roomid);
		return -1;
	}

	char bop_identity=0;

	if(idx==(int)proominfo->ownidx)
		bop_identity=USER_ID_OWNER;
	else if(idx==(int)proominfo->secondownidx||idx==(int)proominfo->secondownidx2)
		bop_identity=USER_ID_OWNER_S;
	else
	{
		map<int,int>::iterator itVJ = proominfo->vjlist.find(idx);
		map<int,int>::iterator itVJ_A = proominfo->vjlist_a.find(idx);
		if(itVJ!=proominfo->vjlist.end())	
			bop_identity=USER_ID_VJ;
		else if(itVJ_A!=proominfo->vjlist_a.end())
			bop_identity=USER_ID_VJ_A;
		else
			bop_identity=USER_ID_VIP;
	}

	//if(CheckRight(pRoomClient->m_identity, pRoomClient->m_viptype,bop_identity,VIP_LEVEL_NONE, cmd)==0)
	if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,bop_identity,VIP_LEVEL_NONE, cmd)==0)
	{
		AC_ERROR("ClientDataDecoder::DoGiveMember:power error");
	 	return 0;
	}

	BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
	if(pDBSvr!=NULL)
	{
		int outseq = g_pNetProxy->GetCounter()->Get();
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		static const char* spname = {"DBMW_AddRoomRole"};//存储过程填充
		outstream->Write((short)CMD_CALLSP);
		outstream->Write(outseq);
		outstream->Write(spname,strlen(spname));
		outstream->Write((short)3);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)proominfo->roomid);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)idx);	
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)USER_ID_VIP);	
		outstream->Flush();
		Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
		if(data != NULL)
		{
			if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1)
			{
				data->roomid = proominfo->roomid;
				data->opidx = pRoomClient->m_idx;
				data->bopidx = idx;		
				data->cmd = cmd;
				data->seq=seq;
				data->outseq=outseq;
				data->number = pRoomClient->GetClientID();
				data->SetReactor(g_pNetProxy->GetReactor());
				data->RegisterTimer(DB_TIMEOUT);
				g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
			}
			else
			{
				AC_ERROR("ClientDataDecoder::DoGiveMember:pDBSvr->AddBuf() error");
				g_pNetProxy->DestroyDBResult(data);
			}
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoGiveMember:CreateDBResultdata error,data=%x",data);
		}
	}
	else
	{
		AC_ERROR("ClientDataDecoder::DoGiveMember:pDBSvr=%x",pDBSvr);
	}
	return 0;
}

int ClientDataDecoder::DoGiveVJMicRes(ClientSocketBase *pClient,  short cmd, int seq,BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoGiveVJMicRes :cmd = %d seq = %d", cmd ,seq);
	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoGiveVJMicRes:roominfo error");
		return -1;
	}

	int roomid=pRoomClient->m_roomid;
	if (roomid==0)
	{
		AC_ERROR("ClientDataDecoder::DoGiveVJMicRes:roomid error");
		return -1;
	}

	int from_idx = 0;
	if(!pinstream->Read(from_idx))    //邀请者idx
	{
		AC_ERROR("ClientDataDecoder::DoGiveVJMicRes:read from_idx error");
		return -1;
	}

	char flag;
	if(!pinstream->Read(flag))    //0:同意，1:不同意
	{
		AC_ERROR("ClientDataDecoder::DoGiveVJMicRes:read flag error");
		return -1;
	}

	AC_DEBUG("ClientDataDecoder::DoGiveVJMicRes:roomid=%d,pRoomClient->m_idx=%d,from_idx=%d,flag=%d",
		proominfo->roomid,pRoomClient->m_idx,from_idx,flag);

	if(from_idx == 0)
	{
		AC_ERROR("ClientDataDecoder::DoGiveVJMicRes:idx error");
		return -1;
	}

	//被邀请者不同意，则回复邀请者
	if(flag==RESULT_GIVEVJMIC_REFUSED)
	{
			BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
			char type=65;
			outstream->Write(type);
			outstream->Write((short)ROOM_CMD_GIVE_VJ_MIC);
			outstream->Write((int)seq);
			outstream->Write((int)pRoomClient->m_idx);
			outstream->Write(flag);
			outstream->Flush();

			map<int,RoomClient*>::iterator itfrom=proominfo->userlist.find(from_idx);
			if(itfrom!=proominfo->userlist.end())
			{
				RoomClient* pFromClient=itfrom->second;
				if(g_pNetProxy->SendToSrv(pFromClient,*outstream)==-1)
				{
					AC_ERROR("ClientDataDecoder::DoGiveVJMicRes:SendToSrv %d error ",from_idx);
					pFromClient->Close();
					return 0;
				}
			}
			
	}
	//被邀请者同意，则由客户端直接调用OnVjMic
	else if(flag==RESULT_GIVEVJMIC_ACCEPT)
	{}

	return 0;
	
}

int ClientDataDecoder::CheckRight(char op_iden/*操作者房间内身份*/,
									char op_vip,/*操作者VIP身份*/
									char bop_iden/*被操作者房间内身份*/,
									char bop_vip/*被操作者VIP身份*/,
									short cmd)
{

	int op_identity=op_iden;
	int op_viplevel=op_vip;
	int bop_identity=bop_iden;
	int bop_viplevel=bop_vip;

	AC_DEBUG("ClientDataDecoder::CheckRight: op_identity=%d op_viplevel=%d bop_identity=%d bop_viplevel=%d",op_identity,op_viplevel,bop_identity,bop_viplevel);

	switch(cmd)
	{

		/*房间日常功能*/
		case ROOM_CMD_GIVEOFF_MIC:		//踢表演麦
			{
				char right_iden=g_pNetProxy->m_pRightCfg->m_giveoffmic[op_identity][bop_identity];
				char right_vip=g_pNetProxy->m_pRightCfg->m_giveoffmic[op_identity][bop_viplevel];
				char rightvip_iden=g_pNetProxy->m_pRightCfg->m_giveoffmic[op_viplevel][bop_identity];
				char rightvip_vip=g_pNetProxy->m_pRightCfg->m_giveoffmic[op_viplevel][bop_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d,%d,%d",right_iden,right_vip,rightvip_iden,rightvip_vip);
				
				if(right_iden==1&&right_vip==1)
				{
					return 1;
				}

				if(rightvip_iden==1&&rightvip_vip==1)
				{
					return 1;
				}

				return 0;
								
			}
			break;
		case ROOM_CMD_GIVEOFF_VJ_MIC://踢主持麦
			{
				char right_iden=g_pNetProxy->m_pRightCfg->m_giveoffvjmic[op_identity][bop_identity];
				char right_vip=g_pNetProxy->m_pRightCfg->m_giveoffvjmic[op_identity][bop_viplevel];

				char rightvip_iden=g_pNetProxy->m_pRightCfg->m_giveoffvjmic[op_viplevel][bop_identity];
				char rightvip_vip=g_pNetProxy->m_pRightCfg->m_giveoffvjmic[op_viplevel][bop_viplevel];


				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d,%d,%d",right_iden,right_vip,rightvip_iden,rightvip_vip);
				
				if(right_iden==1&&right_vip==1)
				{
					return 1;
				}

				if(rightvip_iden==1&&rightvip_vip==1)
				{
					return 1;
				}

				return 0;
			}
			break;
		case ROOM_CMD_ONVJ_MIC:	//上主持麦
			{
				char right_iden=g_pNetProxy->m_pRightCfg->m_onvjmic[op_identity];
				char right_vip=g_pNetProxy->m_pRightCfg->m_onvjmic[op_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
			}
			break;
		case ROOM_CMD_KICKOUT_SOMEONE:	//踢人出房间
			{
				char right_iden=g_pNetProxy->m_pRightCfg->m_kick[op_identity][bop_identity];
				char right_vip=g_pNetProxy->m_pRightCfg->m_kick[op_identity][bop_viplevel];

				char rightvip_iden=g_pNetProxy->m_pRightCfg->m_kick[op_viplevel][bop_identity];
				char rightvip_vip=g_pNetProxy->m_pRightCfg->m_kick[op_viplevel][bop_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d,%d,%d",right_iden,right_vip,rightvip_iden,rightvip_vip);

				
				if(right_iden==1&&right_vip==1)
				{
					return 1;
				}

				if(rightvip_iden==1&&rightvip_vip==1)
				{
					return 1;
				}

				return 0;
				
			}
			break;
		case ROOM_CMD_UP_WAITMIC://调整麦序
		case ROOM_CMD_DOWN_WAITMIC://调整麦序
			{
				char right_iden=g_pNetProxy->m_pRightCfg->m_updownwaitmic[op_identity];
				char right_vip=g_pNetProxy->m_pRightCfg->m_updownwaitmic[op_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
				
			}
			break;
		case ROOM_CMD_FORBIDEN_SOMEONE://对用户禁言
			{
				char right_iden=g_pNetProxy->m_pRightCfg->m_forbiden[op_identity][bop_identity];
				char right_vip=g_pNetProxy->m_pRightCfg->m_forbiden[op_identity][bop_viplevel];

				char rightvip_iden=g_pNetProxy->m_pRightCfg->m_forbiden[op_viplevel][bop_identity];
				char rightvip_vip=g_pNetProxy->m_pRightCfg->m_forbiden[op_viplevel][bop_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d,%d,%d",right_iden,right_vip,rightvip_iden,rightvip_vip);
				
				if(right_iden==1&&right_vip==1)
				{
					return 1;
				}

				if(rightvip_iden==1&&rightvip_vip==1)
				{
					return 1;
				}

				return 0;
				
			}
			break;
		case ROOM_CMD_GIVE_VJ_MIC://邀请上VJ麦
			{
				char right_iden=g_pNetProxy->m_pRightCfg->m_giveonvjmic[op_identity][bop_identity];
				char right_vip=g_pNetProxy->m_pRightCfg->m_giveonvjmic[op_identity][bop_viplevel];

				char rightvip_iden=g_pNetProxy->m_pRightCfg->m_giveonvjmic[op_viplevel][bop_identity];
				char rightvip_vip=g_pNetProxy->m_pRightCfg->m_giveonvjmic[op_viplevel][bop_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d,%d,%d",right_iden,right_vip,rightvip_iden,rightvip_vip);

				
				if(right_iden==1||right_vip==1||rightvip_iden==1||rightvip_vip==1)
				{
					return 1;
				}
				

				return 0;

			}
			break;
		case  ROOM_CMD_WAITMIC:	//非自由排麦下的排麦
			{

				char right_iden=g_pNetProxy->m_pRightCfg->m_freewaitmic[op_identity];
				char right_vip=g_pNetProxy->m_pRightCfg->m_freewaitmic[op_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
				
			}
			break;	
		case ROOM_CMD_CANCLE_WAITMIC:	//把别人踢出麦序
			{
				char right_iden=g_pNetProxy->m_pRightCfg->m_delwaitmic[op_identity][bop_identity];
				char right_vip=g_pNetProxy->m_pRightCfg->m_delwaitmic[op_identity][bop_viplevel];

				char rightvip_iden=g_pNetProxy->m_pRightCfg->m_delwaitmic[op_viplevel][bop_identity];
				char rightvip_vip=g_pNetProxy->m_pRightCfg->m_delwaitmic[op_viplevel][bop_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d,%d,%d",right_iden,right_vip,rightvip_iden,rightvip_vip);
				
				if(right_iden==1&&right_vip==1)
				{
					return 1;
				}

				if(rightvip_iden==1&&rightvip_vip==1)
				{
					return 1;
				}

				return 0;
			}
			break;
		case ROOM_CMD_PUBLIC_CHAT:
			{
				
				char right_iden=g_pNetProxy->m_pRightCfg->m_roomchat[op_identity];
				char right_vip=g_pNetProxy->m_pRightCfg->m_roomchat[op_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
			}
			break;
		/*房间信息设置*/
		case ROOM_CMD_SET_ROOM_NAME://更改房间名
			{
				char right_iden=g_pNetProxy->m_pRightCfg->m_setroomname[op_identity];
				char right_vip=g_pNetProxy->m_pRightCfg->m_setroomname[op_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
				
			}
			break;
		case ROOM_CMD_SET_ROOM_PWD://设置房间密码
			{

				char right_iden=g_pNetProxy->m_pRightCfg->m_setpassword[op_identity];
				char right_vip=g_pNetProxy->m_pRightCfg->m_setpassword[op_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
			}
			break;
		case ROOM_CMD_SET_ROOM_LOCK://设置房间上锁
			{

				char right_iden=g_pNetProxy->m_pRightCfg->m_setroomclose[op_identity];
				char right_vip=g_pNetProxy->m_pRightCfg->m_setroomclose[op_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
			}
			break;
		case ROOM_CMD_SET_USER_ONLY://设置仅对会员开放
			{

				char right_iden=g_pNetProxy->m_pRightCfg->m_setroomprivate[op_identity];
				char right_vip=g_pNetProxy->m_pRightCfg->m_setroomprivate[op_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
				
			}
			break;
		case ROOM_CMD_SET_USER_INOUT://用户进出消息开启/关闭
			{
				char right_iden=g_pNetProxy->m_pRightCfg->m_setroominout[op_identity];
				char right_vip=g_pNetProxy->m_pRightCfg->m_setroominout[op_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
			}
			break;
		case ROOM_CMD_SET_MIC_UPDOWN://设置自由排麦
			{
				char right_iden=g_pNetProxy->m_pRightCfg->m_setfreewaitmic[op_identity];
				char right_vip=g_pNetProxy->m_pRightCfg->m_setfreewaitmic[op_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
			}
			break;
		case ROOM_CMD_UPDATE_CONTENT://更新房间公告
			{

				char right_iden=g_pNetProxy->m_pRightCfg->m_setroomaffiche[op_identity];
				char right_vip=g_pNetProxy->m_pRightCfg->m_setroomaffiche[op_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
			}
			break;
		case ROOM_CMD_SEND_NOTICE_TEMP://发送房间临时公告
			{
				char right_iden=g_pNetProxy->m_pRightCfg->m_sendnotice[op_identity];
				char right_vip=g_pNetProxy->m_pRightCfg->m_sendnotice[op_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
			}
			break;
		case ROOM_CMD_SET_CHAT_PUBLIC://允许房间公聊
			{
				
				char right_iden=g_pNetProxy->m_pRightCfg->m_setroomchat[op_identity];
				char right_vip=g_pNetProxy->m_pRightCfg->m_setroomchat[op_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
			}
			break;
		case ROOM_CMD_SET_ROOM_WELCOME://设置欢迎词
			{
				char right_iden=g_pNetProxy->m_pRightCfg->m_setroomwelcome[op_identity];
				char right_vip=g_pNetProxy->m_pRightCfg->m_setroomwelcome[op_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
			}
			break;
		case ROOM_CMD_SET_ROOM_LOGO://设置房间LOGO
			{

				char right_iden=g_pNetProxy->m_pRightCfg->m_setroomlogo[op_identity];
				char right_vip=g_pNetProxy->m_pRightCfg->m_setroomlogo[op_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
			}
			break;
		case ROOM_CMD_GETBLACKLIST:
			{
				char right_iden=g_pNetProxy->m_pRightCfg->m_getblacklist[op_identity];
				char right_vip=g_pNetProxy->m_pRightCfg->m_getblacklist[op_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
			}
			break;
		case ROOM_CMD_GETROOMMEMBERLIST:
			{

				char right_iden=g_pNetProxy->m_pRightCfg->m_getmemberlist[op_identity];
				char right_vip=g_pNetProxy->m_pRightCfg->m_getmemberlist[op_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
			}
			break;
		/*房间会员管理*/
		case ROOM_CMD_VERIFY_USER_APP:
			{				
				char right_iden=g_pNetProxy->m_pRightCfg->m_auditapply[op_identity];
				char right_vip=g_pNetProxy->m_pRightCfg->m_auditapply[op_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
			}
			break;
		case ROOM_CMD_ROOMAPPLYLIST_C2R2C:
			{
				char right_iden=g_pNetProxy->m_pRightCfg->m_getapplylist[op_identity];
				char right_vip=g_pNetProxy->m_pRightCfg->m_getapplylist[op_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
			}
			break;
		case ROOM_CMD_REMOVE_USER://删除会员
			{
				char right_iden=g_pNetProxy->m_pRightCfg->m_delmember[op_identity][bop_identity];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d",right_iden);
				
				if(right_iden==1)
				{
					return 1;
				}

				return 0;
				
			}
			break;
		case ROOM_CMD_GIVE_VJ:	//设置主持人
			{
				char right_iden=g_pNetProxy->m_pRightCfg->m_setvj[op_identity][bop_identity];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d",right_iden);
				
				if(right_iden==1)
				{
					return 1;
				}

				return 0;
				
			}
			break;
		case ROOM_CMD_GIVE_VJ_A:	//设置助理主持
			{
				char right_iden=g_pNetProxy->m_pRightCfg->m_setvja[op_identity][bop_identity];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d",right_iden);
				
				if(right_iden==1)
				{
					return 1;
				}

				return 0;
			}
			break;
		case ROOM_CMD_GIVE_OUER_S://设置副室主
			{
				char right_iden=g_pNetProxy->m_pRightCfg->m_setsubonwer[op_identity][bop_identity];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d",right_iden);
				
				if(right_iden==1)
				{
					return 1;
				}

				return 0;
				
			}
			break;
		case ROOM_CMD_GIVE_MEMBER://管理员设为会员
			{
				char right_iden=g_pNetProxy->m_pRightCfg->m_setmember[op_identity][bop_identity];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d",right_iden);
				
				if(right_iden==1)
				{
					return 1;
				}

				return 0;
				
			}
			break;
		case ROOM_CMD_UPDATE_BLACKLIST:	//添加黑名单
			{
				char right_iden=g_pNetProxy->m_pRightCfg->m_black[op_identity][bop_identity];
				char right_vip=g_pNetProxy->m_pRightCfg->m_black[op_identity][bop_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				
				if(right_iden==1&&right_vip==1)
				{
					return 1;
				}

				return 0;

			}
			break;
		default:
			{
				return 0;
			}
	}

	return 0;
}

//add by jinguanfu 2011/2/21  调整权限配置文件结构
int ClientDataDecoder::CheckRightEx(char op_iden/*操作者房间内身份*/,
									char op_vip,/*操作者VIP身份*/
									char bop_iden/*被操作者房间内身份*/,
									char bop_vip/*被操作者VIP身份*/,
									short cmd)
{

	int op_identity=op_iden;
	int op_viplevel=op_vip;
	int bop_identity=bop_iden;
	int bop_viplevel=bop_vip;

	AC_DEBUG("ClientDataDecoder::CheckRight: op_identity=%d op_viplevel=%d bop_identity=%d bop_viplevel=%d",op_identity,op_viplevel,bop_identity,bop_viplevel);

	switch(cmd)
	{

		/*房间日常功能*/
		case ROOM_CMD_GIVEOFF_MIC:		//踢表演麦
			{
				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_OFFMIC].rightdetail[op_identity][bop_identity];
				char right_vip= g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_OFFMIC].rightdetail[op_identity][bop_viplevel];
				char rightvip_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_OFFMIC].rightdetail[op_viplevel][bop_identity];
				char rightvip_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_OFFMIC].rightdetail[op_viplevel][bop_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d,%d,%d",right_iden,right_vip,rightvip_iden,rightvip_vip);
				
				if(right_iden==1&&right_vip==1)
				{
					return 1;
				}

				if(rightvip_iden==1&&rightvip_vip==1)
				{
					return 1;
				}

				return 0;
								
			}
			break;
		case ROOM_CMD_GIVEOFF_VJ_MIC://踢主持麦
			{
				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_OFFVJMIC].rightdetail[op_identity][bop_identity];
				char right_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_OFFVJMIC].rightdetail[op_identity][bop_viplevel];

				char rightvip_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_OFFVJMIC].rightdetail[op_viplevel][bop_identity];
				char rightvip_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_OFFVJMIC].rightdetail[op_viplevel][bop_viplevel];


				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d,%d,%d",right_iden,right_vip,rightvip_iden,rightvip_vip);
				
				if(right_iden==1&&right_vip==1)
				{
					return 1;
				}

				if(rightvip_iden==1&&rightvip_vip==1)
				{
					return 1;
				}

				return 0;
			}
			break;
		case ROOM_CMD_ONVJ_MIC:	//上主持麦
			{
				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_ONVJMIC].rightdetail[op_identity][bop_identity];
				char right_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_ONVJMIC].rightdetail[op_viplevel][bop_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
			}
			break;
		case ROOM_CMD_KICKOUT_SOMEONE:	//踢人出房间
			{
				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_KICKOUT].rightdetail[op_identity][bop_identity];
				char right_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_KICKOUT].rightdetail[op_identity][bop_viplevel];

				char rightvip_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_KICKOUT].rightdetail[op_viplevel][bop_identity];
				char rightvip_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_KICKOUT].rightdetail[op_viplevel][bop_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d,%d,%d",right_iden,right_vip,rightvip_iden,rightvip_vip);

				
				if(right_iden==1&&right_vip==1)
				{
					return 1;
				}

				if(rightvip_iden==1&&rightvip_vip==1)
				{
					return 1;
				}

				return 0;
				
			}
			break;
		case ROOM_CMD_UP_WAITMIC://调整麦序
		case ROOM_CMD_DOWN_WAITMIC://调整麦序
			{
				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_CHGWAITMIC].rightdetail[op_identity][bop_identity];
				char right_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_CHGWAITMIC].rightdetail[op_viplevel][bop_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
				
			}
			break;
			//add by lihongwu 2011-9-8
		case ROOM_CMD_ADD_TM://麦序时间延长
			{
				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_ADDTM].rightdetail[op_identity][bop_identity];
				char right_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_ADDTM].rightdetail[op_viplevel][bop_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;

			}
			break;
			//end by lihongwu 2011-9-13
		case ROOM_CMD_FORBIDEN_SOMEONE://对用户禁言
			{
				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_FORBIDEN].rightdetail[op_identity][bop_identity];
				char right_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_FORBIDEN].rightdetail[op_identity][bop_viplevel];

				char rightvip_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_FORBIDEN].rightdetail[op_viplevel][bop_identity];
				char rightvip_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_FORBIDEN].rightdetail[op_viplevel][bop_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d,%d,%d",right_iden,right_vip,rightvip_iden,rightvip_vip);
				
				if(right_iden==1&&right_vip==1)
				{
					return 1;
				}

				if(rightvip_iden==1&&rightvip_vip==1)
				{
					return 1;
				}

				return 0;
				
			}
			break;
		case ROOM_CMD_GIVE_VJ_MIC://邀请上VJ麦
			{
				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_GIVEVJMIC].rightdetail[op_identity][bop_identity];
				char right_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_GIVEVJMIC].rightdetail[op_identity][bop_viplevel];

				char rightvip_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_GIVEVJMIC].rightdetail[op_viplevel][bop_identity];
				char rightvip_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_GIVEVJMIC].rightdetail[op_viplevel][bop_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d,%d,%d",right_iden,right_vip,rightvip_iden,rightvip_vip);

				
				if(right_iden==1||right_vip==1||rightvip_iden==1||rightvip_vip==1)
				{
					return 1;
				}
				

				return 0;

			}
			break;
		case  ROOM_CMD_WAITMIC:	//非自由排麦下的排麦
			{

				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_FREEWAITMIC].rightdetail[op_identity][bop_identity];
				char right_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_FREEWAITMIC].rightdetail[op_viplevel][bop_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
				
			}
			break;	
		case ROOM_CMD_CANCLE_WAITMIC:	//把别人踢出麦序
			{
				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_DELWAITMIC].rightdetail[op_identity][bop_identity];
				char right_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_DELWAITMIC].rightdetail[op_identity][bop_viplevel];

				char rightvip_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_DELWAITMIC].rightdetail[op_viplevel][bop_identity];
				char rightvip_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_DELWAITMIC].rightdetail[op_viplevel][bop_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d,%d,%d",right_iden,right_vip,rightvip_iden,rightvip_vip);
				
				if(right_iden==1&&right_vip==1)
				{
					return 1;
				}

				if(rightvip_iden==1&&rightvip_vip==1)
				{
					return 1;
				}

				return 0;
			}
			break;
		case ROOM_CMD_PUBLIC_CHAT:
			{
				
				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_SENDCHAT].rightdetail[op_identity][bop_identity];
				char right_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_SENDCHAT].rightdetail[op_viplevel][bop_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
			}
			break;
		/*房间信息设置*/
		case ROOM_CMD_SET_ROOM_NAME://更改房间名
			{
				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_SETROOMNAME].rightdetail[op_identity][bop_identity];
				char right_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_SETROOMNAME].rightdetail[op_viplevel][bop_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
				
			}
			break;
		case ROOM_CMD_SET_ROOM_PWD://设置房间密码
			{

				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_SETROOMPWD].rightdetail[op_identity][bop_identity];
				char right_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_SETROOMPWD].rightdetail[op_viplevel][bop_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
			}
			break;
		case ROOM_CMD_SET_ROOM_LOCK://设置房间上锁
			{

				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_SETROOMCLOSE].rightdetail[op_identity][bop_identity];
				char right_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_SETROOMCLOSE].rightdetail[op_viplevel][bop_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
			}
			break;
		case ROOM_CMD_SET_USER_ONLY://设置仅对会员开放
			{

				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_SETROOMPRIVATE].rightdetail[op_identity][bop_identity];
				char right_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_SETROOMPRIVATE].rightdetail[op_viplevel][bop_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
				
			}
			break;
		case ROOM_CMD_SET_USER_INOUT://用户进出消息开启/关闭
			{
				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_SETROOMINOUT].rightdetail[op_identity][bop_identity];
				char right_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_SETROOMINOUT].rightdetail[op_viplevel][bop_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
			}
			break;
		case ROOM_CMD_SET_MIC_UPDOWN://设置自由排麦
			{
				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_SETFREEMIC].rightdetail[op_identity][bop_identity];
				char right_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_SETFREEMIC].rightdetail[op_viplevel][bop_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
			}
			break;
		case ROOM_CMD_UPDATE_CONTENT://更新房间公告
			{

				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_SETROOMCONTENT].rightdetail[op_identity][bop_identity];
				char right_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_SETROOMCONTENT].rightdetail[op_viplevel][bop_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
			}
			break;
		case ROOM_CMD_SEND_NOTICE_TEMP://发送房间临时公告
			{
				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_SENDCONTENT].rightdetail[op_identity][bop_identity];
				char right_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_SENDCONTENT].rightdetail[op_viplevel][bop_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
			}
			break;
		case ROOM_CMD_SET_CHAT_PUBLIC://允许房间公聊
			{
				
				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_SETROOMCHAT].rightdetail[op_identity][bop_identity];
				char right_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_SETROOMCHAT].rightdetail[op_viplevel][bop_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
			}
			break;
		case ROOM_CMD_SET_ROOM_WELCOME://设置欢迎词
			{
				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_SETWELCOMEWORD].rightdetail[op_identity][bop_identity];
				char right_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_SETWELCOMEWORD].rightdetail[op_viplevel][bop_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
			}
			break;
		case ROOM_CMD_SET_ROOM_LOGO://设置房间LOGO
			{

				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_SETROOMLOGO].rightdetail[op_identity][bop_identity];
				char right_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_SETROOMLOGO].rightdetail[op_viplevel][bop_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
			}
			break;
		case ROOM_CMD_GETBLACKLIST:
			{
				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_GETBLACK].rightdetail[op_identity][bop_identity];
				char right_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_GETBLACK].rightdetail[op_viplevel][bop_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
			}
			break;
		case ROOM_CMD_GETROOMMEMBERLIST:
			{

				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_MEMBERLIST].rightdetail[op_identity][bop_identity];
				char right_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_MEMBERLIST].rightdetail[op_viplevel][op_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
			}
			break;
		/*房间会员管理*/
		case ROOM_CMD_VERIFY_USER_APP:
			{				
				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_AUDITAPPLY].rightdetail[op_identity][bop_identity];
				char right_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_AUDITAPPLY].rightdetail[op_viplevel][bop_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
			}
			break;
		case ROOM_CMD_ROOMAPPLYLIST_C2R2C:
			{
				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_GETAPPLYLIST].rightdetail[op_identity][bop_identity];
				char right_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_GETAPPLYLIST].rightdetail[op_viplevel][bop_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
			}
			break;
		case ROOM_CMD_REMOVE_USER://删除会员
			{
				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_DELMEMBER].rightdetail[op_identity][bop_identity];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d",right_iden);
				
				if(right_iden==1)
				{
					return 1;
				}

				return 0;
				
			}
			break;
		case ROOM_CMD_GIVE_VJ:	//设置主持人
			{
				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_GIVEVJ].rightdetail[op_identity][bop_identity];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d",right_iden);
				
				if(right_iden==1)
				{
					return 1;
				}

				return 0;
				
			}
			break;
		case ROOM_CMD_GIVE_VJ_A:	//设置助理主持
			{
				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_GIVEVJA].rightdetail[op_identity][bop_identity];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d",right_iden);
				
				if(right_iden==1)
				{
					return 1;
				}

				return 0;
			}
			break;
		case ROOM_CMD_GIVE_OUER_S://设置副室主
			{
				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_GIVESUBOWNER].rightdetail[op_identity][bop_identity];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d",right_iden);
				
				if(right_iden==1)
				{
					return 1;
				}

				return 0;
				
			}
			break;
		case ROOM_CMD_GIVE_MEMBER://管理员设为会员
			{
				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_GIVEMEMBER].rightdetail[op_identity][bop_identity];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d",right_iden);
				
				if(right_iden==1)
				{
					return 1;
				}

				return 0;
				
			}
			break;
		case ROOM_CMD_UPDATE_BLACKLIST:	//添加黑名单
			{
				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_BLACK].rightdetail[op_identity][bop_identity];
				char right_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_BLACK].rightdetail[op_identity][bop_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				
				if(right_iden==1&&right_vip==1)
				{
					return 1;
				}

				return 0;

			}
			break;
		case ROOM_CMD_INVITE_MEMBER://邀请会员 
			{
				//权限判断等同于审核会员
				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_AUDITAPPLY].rightdetail[op_identity][bop_identity];
				char right_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_AUDITAPPLY].rightdetail[op_viplevel][bop_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;

			}
			break;
		case  ROOM_CMD_SET_AUTOONMIC:	//自动上麦
			{

				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_SETAUTOONMIC].rightdetail[op_identity][bop_identity];
				char right_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_SETAUTOONMIC].rightdetail[op_viplevel][bop_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d",right_iden,right_vip);
				if(right_iden==0&&right_vip==0)
					return 0;

				return 1;
				
			}
			break;
		case ROOM_CMD_DISABLE_IPADDR:
		case ROOM_CMD_DISABLE_MACADDR:
			{
				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTTON_DISABLEMACIP].rightdetail[op_identity][bop_identity];
				char right_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTTON_DISABLEMACIP].rightdetail[op_identity][bop_viplevel];

				char rightvip_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTTON_DISABLEMACIP].rightdetail[op_viplevel][bop_identity];
				char rightvip_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTTON_DISABLEMACIP].rightdetail[op_viplevel][bop_viplevel];

				AC_DEBUG("ClientDataDecoder::CheckRight: %d,%d,%d,%d",right_iden,right_vip,rightvip_iden,rightvip_vip);


				if(right_iden==1&&right_vip==1)
				{
					return 1;
				}

				if(rightvip_iden==1&&rightvip_vip==1)
				{
					return 1;
				}

				return 0;

			}
			break;
		default:
			{
				return 0;
			}
	}

	return 0;
}


vector<MIC_INFO*>::iterator  ClientDataDecoder::SearchWaitMic(vector<MIC_INFO*>::iterator itbegin,
															vector<MIC_INFO*>::iterator itend,int idx)
{
	vector<MIC_INFO*>::iterator ittemp=itbegin;
	while(ittemp!=itend)
	{
		MIC_INFO* pMicInfo=(MIC_INFO*)(*ittemp);
		if(pMicInfo->idx==idx)
			return ittemp;

		ittemp++;
		
	}
		
	return ittemp;
}


vector<MIC_INFO*>::iterator  ClientDataDecoder::SearchPKWaitMic(vector<MIC_INFO*>::iterator itbegin,
															vector<MIC_INFO*>::iterator itend,int idx)
{
	vector<MIC_INFO*>::iterator ittemp=itbegin;
	while(ittemp!=itend)
	{
		MIC_INFO* pMicInfo=(MIC_INFO*)(*ittemp);
		if(pMicInfo->pkidx==idx)
			return ittemp;

		ittemp++;
		
	}
		
	return ittemp;
}


int ClientDataDecoder::LoginCheck(char op_iden/*操作者房间内身份*/,
									char op_vip/*操作者VIP身份*/,
									char type/*0:密码房间 1:房间关闭 2:会员房间3:满员房*/)
{
	int op_identity=op_iden;
	int op_viplevel=op_vip;
	switch(type)
	{
		case 0:
			{
				char right_iden=g_pNetProxy->m_pRightCfg->m_unpasswd[op_identity];
				char right_vip=g_pNetProxy->m_pRightCfg->m_unpasswd[op_viplevel];

				AC_DEBUG("ClientDataDecoder::LoginCheck: unpasswd ,%d,%d",right_iden,right_vip);
				
				if(right_iden==0&&right_vip==0)
				{
					return 0;
				}

				return 1;
				
			}
			break;
		case 1:
			{
				char right_iden=g_pNetProxy->m_pRightCfg->m_closed[op_identity];
				char right_vip=g_pNetProxy->m_pRightCfg->m_closed[op_viplevel];

				AC_DEBUG("ClientDataDecoder::LoginCheck: closed ,%d,%d",right_iden,right_vip);
				
				if(right_iden==0&&right_vip==0)
				{
					return 0;
				}

				return 1;
				
			}
			break;
		case 2:
			{
				char right_iden=g_pNetProxy->m_pRightCfg->m_private[op_identity];
				char right_vip=g_pNetProxy->m_pRightCfg->m_private[op_viplevel];

				AC_DEBUG("ClientDataDecoder::LoginCheck: privated ,%d,%d",right_iden,right_vip);
				
				if(right_iden==0&&right_vip==0)
				{
					return 0;
				}

				return 1;
				
			}
			break;
		case 3:
			{
				char right_iden=g_pNetProxy->m_pRightCfg->m_fulllimit[op_identity];
				char right_vip=g_pNetProxy->m_pRightCfg->m_fulllimit[op_viplevel];

				AC_DEBUG("ClientDataDecoder::LoginCheck: fulllimit ,%d,%d",right_iden,right_vip);
				
				if(right_iden==0&&right_vip==0)
				{
					return 0;
				}

				return 1;

			}
			break;
		default:
			return 0;
	

	}

	return 0;
}

//add by jinguanfu 2011/2/22
int ClientDataDecoder::LoginCheckEx(char op_iden/*操作者房间内身份*/,
									char op_vip/*操作者VIP身份*/,
									char type/*0:密码房间 1:房间关闭 2:会员房间3:满员房*/)
{
	int op_identity=op_iden;
	int op_viplevel=op_vip;
	switch(type)
	{
		case 0:
			{
				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_CHKPWD].rightdetail[op_identity][op_identity];
				char right_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_CHKPWD].rightdetail[op_viplevel][op_viplevel];

				AC_DEBUG("ClientDataDecoder::LoginCheck: unpasswd ,%d,%d",right_iden,right_vip);
				
				if(right_iden==0&&right_vip==0)
				{
					return 0;
				}

				return 1;
				
			}
			break;
		case 1:
			{
				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_CHKCLOSE].rightdetail[op_identity][op_identity];
				char right_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_CHKCLOSE].rightdetail[op_viplevel][op_viplevel];

				AC_DEBUG("ClientDataDecoder::LoginCheck: closed ,%d,%d",right_iden,right_vip);
				
				if(right_iden==0&&right_vip==0)
				{
					return 0;
				}

				return 1;
				
			}
			break;
		case 2:
			{
				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_CHKPRIVATE].rightdetail[op_identity][op_identity];
				char right_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_CHKPRIVATE].rightdetail[op_viplevel][op_viplevel];

				AC_DEBUG("ClientDataDecoder::LoginCheck: privated ,%d,%d",right_iden,right_vip);
				
				if(right_iden==0&&right_vip==0)
				{
					return 0;
				}

				return 1;
				
			}
			break;
		case 3:
			{
				char right_iden=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_CHKFULL].rightdetail[op_identity][op_identity];
				char right_vip=g_pNetProxy->m_pRightCfgEx->m_right[RIGHT_OPTION_CHKFULL].rightdetail[op_viplevel][op_viplevel];

				AC_DEBUG("ClientDataDecoder::LoginCheck: fulllimit ,%d,%d",right_iden,right_vip);
				
				if(right_iden==0&&right_vip==0)
				{
					return 0;
				}

				return 1;

			}
			break;
		default:
			return 0;
	

	}

	return 0;
}


int ClientDataDecoder::DoSendFireworks(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoSendFireworks:pinstream = %x cmd = %d seq = %d", pinstream, cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoSendFireworks:roominfo error");
		return -1;
	}	

	int cate_idx = 0;
	if(!pinstream->Read(cate_idx))    //礼物ID
	{
		AC_ERROR("ClientDataDecoder::DoSendFireworks:read cate_idx error");
		return -1;
	}

	int number = 0;
	if(!pinstream->Read(number))    //送礼物个数
	{
		AC_ERROR("ClientDataDecoder::DoSendFireworks:read number error");
		return -1;
	}
	
	AC_DEBUG("ClientDataDecoder::DoSendFireworks:idx=%d,roomid=%d,cate_id=%d,number=%d",
		pRoomClient->m_idx,pRoomClient->m_roomid,cate_idx,number);

	if( number== 0)
	{
		AC_ERROR("ClientDataDecoder::DoSendFireworks:number error");
		return -1;
	}	

	//礼物不存在
	map<int,GIFT_INFO>::iterator itGift=g_pNetProxy->m_GiftInfo.find(cate_idx);
	if(itGift==g_pNetProxy->m_GiftInfo.end())
	{
		AC_DEBUG("ClientDataDecoder::DoSendFireworks:cate_idx=%d not exist ",cate_idx);
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		char type=65;
		outstream->Write(type);
		outstream->Write(cmd);
		outstream->Write(seq);
		outstream->Write(cate_idx);	//礼物ID
		outstream->Write(number);		//礼物数量
		outstream->Write((int)DBRESULT_GIFT_NOGIFT);		//无此交易类型
		outstream->Flush();

		if(g_pNetProxy->SendToSrv(pClient, *outstream) == -1)
		{
			AC_ERROR("ClientDataDecoder::DoSendFireworks:SendToSrv error");
			return -1;
		}
		
		return 0;
	}

	int price=(itGift->second).price;

	int total=price*number;
	//金币数不足
	if(pRoomClient->gold<total)
	{
		AC_DEBUG("ClientDataDecoder::DoSendFireworks:gold lack gold=%d,total=%d",pRoomClient->gold,total);
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		char type=65;
		outstream->Write(type);
		outstream->Write(cmd);
		outstream->Write(seq);
		outstream->Write(cate_idx);	//礼物ID
		outstream->Write(number);		//礼物数量
		outstream->Write((int)DBRESULT_GIFT_NOMONEY);			//余额不足
		outstream->Flush();

		if(g_pNetProxy->SendToSrv(pClient, *outstream) == -1)
		{
			AC_ERROR("ClientDataDecoder::DoSendFireworks:SendToSrv error");
			return -1;
		}
		
		return 0;
	}

	//add by jinguanfu 2010/5/12 送礼服务器校验<end>
	
	BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
	if(pDBSvr!=NULL)
	{
		//发送给dbagent
		//char outbuf[256] = {0};
		int outseq = g_pNetProxy->GetCounter()->Get();
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		static const char* spname = {"DBMW_SendFireworks_HZSTAR"};//存储过程填充
		outstream->Write((short)CMD_CALLSP);
		outstream->Write(outseq);
		outstream->Write(spname,strlen(spname));
		outstream->Write((short)4);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)pRoomClient->m_idx);	//赠送者idx
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)proominfo->roomid);	//房间id
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)cate_idx);			//礼物idx
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)number);				//礼物数量
		outstream->Flush();

		Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
		if(data != NULL) 
		{
			if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1)
			{
				data->roomid = pRoomClient->m_roomid;
				data->opidx = pRoomClient->m_idx;
				data->bopidx = 0;
				data->cate_idx = cate_idx;
				data->number=number;
				data->cmd = cmd;
				data->seq=seq;
				data->outseq=outseq;
				data->SetReactor(g_pNetProxy->GetReactor());
				data->RegisterTimer(DB_TIMEOUT);
				g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
			}
			else
			{
				AC_ERROR("ClientDataDecoder::DoSendFireworks:pDBSvr->AddBuf() error ");
				g_pNetProxy->DestroyDBResult(data);
			}
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoSendFireworks:CreateDBResultdata error,data=%x",data);
		}
	}
	else
	{
		AC_ERROR("ClientDataDecoder::DoSendFireworks:pDBSvr=%x ",pDBSvr);
	}
	return 0;
}

int ClientDataDecoder::DoViewIncome(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoViewIncome:pinstream = %x cmd = %d seq = %d", pinstream, cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoViewIncome:roominfo error");
		return -1;
	}	
	
	AC_DEBUG("ClientDataDecoder::DoViewIncome:idx=%d,roomid=%d",
		pRoomClient->m_idx,pRoomClient->m_roomid);

	if(pRoomClient->m_idx!=proominfo->ownidx)
	{
		AC_ERROR("ClientDataDecoder::DoViewIncome:power error");
		return 0;
	}

	BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
	if(pDBSvr!=NULL)
	{
		int outseq = g_pNetProxy->GetCounter()->Get();
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		static const char* spname = {"DBMW_GetIncome_ByRoomID"};//存储过程填充
		outstream->Write((short)CMD_CALLSP);
		outstream->Write(outseq);
		outstream->Write(spname,strlen(spname));
		outstream->Write((short)1);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)proominfo->roomid);
		outstream->Flush();
		
		Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
		if(data != NULL)
		{
			if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1)
			{
				data->roomid = proominfo->roomid;
				data->opidx = pRoomClient->m_idx;	
				data->bopidx=pRoomClient->m_idx;
				data->cmd = cmd;
				data->seq=seq;
				data->outseq=outseq;
				data->number=pRoomClient->GetClientID();
				data->SetReactor(g_pNetProxy->GetReactor());
				data->RegisterTimer(DB_TIMEOUT);
				g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
			}
			else
			{
				AC_ERROR("ClientDataDecoder::DoViewIncome:pDBSvr->AddBuf() error");
				g_pNetProxy->DestroyDBResult(data);
			}
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoViewIncome:CreateDBResultdata() error,data=%x",data);
		}		
	}	
	else
	{
		AC_ERROR("ClientDataDecoder::DoViewIncome:pDBSvr=%x ",pDBSvr);
	}
	
	return 0;
}

int ClientDataDecoder::DoViewIncomeLog(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoViewIncomeLog:pinstream = %x cmd = %d seq = %d", pinstream, cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoViewIncomeLog:roominfo error");
		return -1;
	}	
	
	AC_DEBUG("ClientDataDecoder::DoViewIncomeLog:idx=%d,roomid=%d",
		pRoomClient->m_idx,pRoomClient->m_roomid);

	if(pRoomClient->m_idx!=proominfo->ownidx)
	{
		AC_ERROR("ClientDataDecoder::DoViewIncomeLog:power error");
		return 0;
	}

	BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
	if(pDBSvr!=NULL)
	{
		int outseq = g_pNetProxy->GetCounter()->Get();
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		static const char* spname = {"DBMW_GetIncomeLog_ByRoomID"};//存储过程填充
		outstream->Write((short)CMD_CALLSP);
		outstream->Write(outseq);
		outstream->Write(spname,strlen(spname));
		outstream->Write((short)1);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)proominfo->roomid);
		outstream->Flush();
		
		Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
		if(data != NULL)
		{
			if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1)
			{
				data->roomid = proominfo->roomid;
				data->opidx = pRoomClient->m_idx;	
				data->bopidx=pRoomClient->m_idx;
				data->cmd = cmd;
				data->seq=seq;
				data->outseq=outseq;
				data->number=pRoomClient->GetClientID();
				data->SetReactor(g_pNetProxy->GetReactor());
				data->RegisterTimer(DB_TIMEOUT);
				g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
			}
			else
			{
				AC_ERROR("ClientDataDecoder::DoViewIncomeLog:pDBSvr->AddBuf() error");
				g_pNetProxy->DestroyDBResult(data);
			}
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoViewIncomeLog:CreateDBResultdata() error,data=%x",data);
		}		
	}	
	else
	{
		AC_ERROR("ClientDataDecoder::DoViewIncomeLog:pDBSvr=%x ",pDBSvr);
	}

	return 0;

}


int ClientDataDecoder::DoGetIncome(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoGetIncome:pinstream = %x cmd = %d seq = %d", pinstream, cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoGetIncome:roominfo error");
		return -1;
	}	

	AC_DEBUG("ClientDataDecoder::DoGetIncome:idx=%d,roomid=%d",
			pRoomClient->m_idx,pRoomClient->m_roomid);

	if(pRoomClient->m_idx!=proominfo->ownidx)
	{
		AC_ERROR("ClientDataDecoder::DoGetIncome:power error");
		return 0;
	}

	AC_DEBUG("ClientDataDecoder::DoGetIncome: roomid=%d,idx=%d",proominfo->roomid,pRoomClient->m_idx);
	
	BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
	if(pDBSvr!=NULL)
	{
		int outseq = g_pNetProxy->GetCounter()->Get();
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		static const char* spname = {"DBMW_GetMultiRoomIncome"};//存储过程填充
		outstream->Write((short)CMD_CALLSP);
		outstream->Write(outseq);
		outstream->Write(spname,strlen(spname));
		outstream->Write((short)2);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)proominfo->roomid);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write(pRoomClient->m_idx);
		outstream->Flush();
		
		Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
		if(data != NULL)
		{
			if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1)
			{
				data->roomid = proominfo->roomid;
				data->opidx = pRoomClient->m_idx;	
				data->bopidx=pRoomClient->m_idx;
				data->cmd = cmd;
				data->seq=seq;
				data->outseq=outseq;
				data->number=pRoomClient->GetClientID();
				data->SetReactor(g_pNetProxy->GetReactor());
				data->RegisterTimer(DB_TIMEOUT);
				g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
			}
			else
			{
				AC_ERROR("ClientDataDecoder::DoGetIncome:pDBSvr->AddBuf() error");
				g_pNetProxy->DestroyDBResult(data);
			}
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoGetIncome:CreateDBResultdata() error,data=%x",data);
		}		
	}	
	else
	{
		AC_ERROR("ClientDataDecoder::DoGetIncome:pDBSvr=%x ",pDBSvr);
	}

	return 0;

}


int ClientDataDecoder::DoKeepAlive(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	//AC_DEBUG("ClientDataDecoder::DoKeepAlive:pinstream = %x cmd = %d seq = %d", pinstream, cmd ,seq);

	NOT_USED(cmd);
	NOT_USED(seq);
	NOT_USED(pinstream);


	RoomClient* pRoomClient = (RoomClient*)pClient;

	//发送状态到日志服务器
	BinaryWriteStream* logstream=StreamFactory::InstanceWriteStream();
	logstream->Write((short)CMD_ROOM_KEEPALIVE_R2LS);
	logstream->Write(0);
	logstream->Write(pRoomClient->m_idx); //用户idx
	logstream->Write(pRoomClient->m_roomid); //所在房间ID
	logstream->Write(pRoomClient->m_onmicflag); //上麦状态
	logstream->Flush();
	
	BackClientSocketBase *pLogSvr = g_pNetProxy->GetLogServer();
	if(pLogSvr!=NULL)
	{
		if(pLogSvr->AddBuf(logstream->GetData(), logstream->GetSize())==-1) 
		{
			AC_ERROR("ClientDataDecoder::DoKeepAlive:pLogSvr->AddBuf Error");
		}else
		{
			AC_DEBUG("ClientDataDecoder::DoKeepAlive:Send to LogSvr success,idx=%d roomid=%d onmicflag=%d",pRoomClient->m_idx,pRoomClient->m_roomid,pRoomClient->m_onmicflag);
		}
	
	}
	else
	{
		AC_ERROR("ClientDataDecoder::DoKeepAlive:pLogSvr is NULL");
	}

	//发送状态到工厂服务器
	//SendToFactory(pClient,FACTORY_CMD_KEEPALIVE_R2F);
	
	return 0;
}

int ClientDataDecoder::DoBroadCastNetstatus(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{

	AC_DEBUG("ClientDataDecoder::DoBroadCastNetstatus:pinstream = %x cmd = %d seq = %d", pinstream, cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	if(pRoomClient->m_onmicflag!=2&&pRoomClient->m_onmicflag!=3)
	{
		AC_ERROR("ClientDataDecoder::DoBroadCastNetstatus:idx=%d is not onmic or onvjmic",pRoomClient->m_idx);
		return 0;
	}

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoBroadCastNetstatus:roominfo error");
		return -1;
	}
	char netinfo[65535];
	size_t netinfolen=0;

	if(!pinstream->Read(netinfo,sizeof(netinfo),netinfolen))
	{
		AC_ERROR("ClientDataDecoder::DoBroadCastNetstatus:read netinfo error");
		return -1;
	}
	

	BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
	char type=65;
	int outseq=0;
	outstream->Write(type);
	outstream->Write((short)ROOM_CMD_SB_NETSTATUS);
	outstream->Write(outseq);
	outstream->Write(pRoomClient->m_idx);		//用户IDX
	outstream->Write(netinfo,netinfolen);		//用户网络状态信息
	outstream->Flush();

	char outbuf2[65535];
	int outbuflen2=sizeof(outbuf2);
    if(!StreamCompress(outstream->GetData(),(int)outstream->GetSize(),outbuf2,outbuflen2))
    {
        AC_ERROR("ClientDataDecoder::DoBroadCastNetstatus:StreamCompress error");
        return 0;
    }

    map<int, RoomClient*>::iterator itsend = proominfo->userlist.begin();
    for(;itsend != proominfo->userlist.end();itsend++)
	{
		RoomClient* psend = itsend->second;
		if(psend != NULL)
		{
            if(psend->AddBuf(outbuf2,outbuflen2) == -1)
			{
                    //psend->ErrorClose();
				AC_ERROR("psend->AddBuf Error");
			}
		}
	}

	AC_DEBUG("ClientDataDecoder::DoBroadCastNetstatus:netinfo broadcast success from idx=%d ",pRoomClient->m_idx);

	//add by jinguanfu 2011/4/25
	//发送状态到日志服务器
	BinaryWriteStream* logstream=StreamFactory::InstanceWriteStream();
	logstream->Write((short)CMD_ROOM_USERSTATUS_R2LS);
	logstream->Write(0);
	logstream->Write(pRoomClient->m_idx); 	 //用户idx
	logstream->Write(pRoomClient->m_roomid); //所在房间ID
	logstream->Write(netinfo,netinfolen);	 //用户网络状态信息
	logstream->Flush();
	
	BackClientSocketBase *pLogSvr = g_pNetProxy->GetLogServer();
	if(pLogSvr!=NULL)
	{
		if(pLogSvr->AddBuf(logstream->GetData(), logstream->GetSize())==-1) 
		{
			AC_ERROR("ClientDataDecoder::DoBroadCastNetstatus:pLogSvr->AddBuf Error");
		}else
	    {
			AC_DEBUG("ClientDataDecoder::DoBroadCastNetstatus:Send to LogSvr success,idx=%d roomid=%d onmicflag=%d",pRoomClient->m_idx,pRoomClient->m_roomid,pRoomClient->m_onmicflag);
		}
	}
	else
	{
		AC_ERROR("ClientDataDecoder::DoBroadCastNetstatus:pLogSvr is NULL");
	}

	AC_DEBUG("ClientDataDecoder::DoBroadCastNetstatus:netstatus report success from idx=%d ",pRoomClient->m_idx);

	return 0;
}


int ClientDataDecoder::DoInviteMember(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{

	AC_DEBUG("ClientDataDecoder::DoInviteMember:pinstream = %x cmd = %d seq = %d", pinstream, cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	if(pRoomClient==NULL)
	{
		AC_ERROR("ClientDataDecoder::DoInviteMember:pClient is NULL");
		return -1;
	}


	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoInviteMember:roominfo error");
		return -1;
	}

	int idx=0;
	if(!pinstream->Read(idx))
	{
		AC_ERROR("ClientDataDecoder::DoInviteMember:read idx error");
		return -1;
	}
	//权限判断
	if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE, USER_ID_NONE, cmd)==0)
	{
		AC_ERROR("ClientDataDecoder::DoInviteMember:NO Option Right");
		return 0;
	}

	AC_DEBUG("ClientDataDecoder::DoInviteMember:roomid = %d, optidx= %d ,bopt = %d",
		proominfo->roomid, pRoomClient->m_idx,idx);

	BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
	char type=65;
	int ret=SUCCESS;
	do{
		//虚拟用户(后台打入的机器人)
		map<int,RoomClient*>::iterator itVU=proominfo->vuserlist.find(idx);
		if(itVU!=proominfo->vuserlist.end())
		{
			AC_DEBUG("ClientDataDecoder::DoInviteMember:idx=%d is virtual user",idx);
			ret=SUCCESS;
			break;
		}
		
		//被邀请用户是否在房间内
		map<int,RoomClient*>::iterator itidx=proominfo->userlist.find(idx);
		if(itidx==proominfo->userlist.end())
		{
			AC_ERROR("ClientDataDecoder::DoInviteMember:idx=%d not in room",idx);
			return 0;
		}

		//房间会员数已满
		if(proominfo->userlistVIP.size()>=300)
		{
		
			AC_ERROR("ClientDataDecoder::DoInviteMember:room=%d memberlist is full,[idx=%d]",proominfo->roomid,idx);
						
			ret=LISTFULL;
			break;
		}

		//重复邀请会员
		map<int,int>::iterator itinvite=proominfo->invitelist.find(idx);
		if(itinvite!=proominfo->invitelist.end())
		{
		
			AC_ERROR("ClientDataDecoder::DoInviteMember:idx=%d invite again in room=%d",idx,proominfo->roomid);

			ret=IAGAIN;
			break;
		}
		
		// 判断是否为其它房间会员 
		map<int, ROOM_INFO*>::iterator it=	g_pNetProxy->m_roomlistinfo.roommap.begin();
		for (;it!=g_pNetProxy->m_roomlistinfo.roommap.end();it++)
		{
			ROOM_INFO* temp = it->second;
			map<int,int>::iterator itu=temp->userlistVIP.find(pRoomClient->m_idx);
			if (temp && itu != temp->userlistVIP.end())
			{
				AC_ERROR("ClientDataDecoder::DoInviteMember:idx=%d is member in room=%d",idx,temp->roomid);
				
				ret=ALREADY;
				break;
			}
		}

		int outseq=0;
		outstream->Clear();
		outstream->Write(type);
		outstream->Write((short)ROOM_CMD_INVITE_NOTIFY);
		outstream->Write(outseq);
		outstream->Write(pRoomClient->m_idx);	//用户IDX
		outstream->Write(idx);					//被邀请者IDX
		outstream->Write(proominfo->roomid);	//房间ID
		outstream->Flush();

		RoomClient* pClient_idx=(RoomClient*)itidx->second;
		if(g_pNetProxy->SendToSrv(pClient_idx, *outstream)==-1)
		{
			AC_ERROR("ClientDataDecoder::DoInviteMember:SendTo idx=%d error",idx);
		}

		proominfo->invitelist.insert(make_pair(idx,pRoomClient->m_idx));
		
		ret=SUCCESS;
		
	}while(0);

	outstream->Clear();
	outstream->Write(type);
	outstream->Write(cmd);
	outstream->Write(seq);
	outstream->Write(ret);
	outstream->Flush();
	if(g_pNetProxy->SendToSrv(pRoomClient, *outstream)==-1)
	{
		AC_ERROR("ClientDataDecoder::DoInviteMember:SendTo idx=%d error",pRoomClient->m_idx);
		return -1;
	}

	return 0;
}


int ClientDataDecoder::DoReplyInvite(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoReplyInvite:pinstream = %x cmd = %d seq = %d", pinstream, cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	if(pRoomClient==NULL)
	{
		AC_ERROR("ClientDataDecoder::DoReplyInvite:pClient is NULL");
		return -1;
	}

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoReplyInvite:roominfo error");
		return -1;
	}

	char ret=0;
	if(!pinstream->Read(ret))
	{
		AC_ERROR("ClientDataDecoder::DoReplyInvite:read ret error,[idx=%d]",pRoomClient->m_idx);
		return -1;
	}

	map<int,int>::iterator itidx=proominfo->invitelist.find(pRoomClient->m_idx);
	if(itidx==proominfo->invitelist.end())
	{
		AC_ERROR("ClientDataDecoder::DoReplyInvite:[idx=%d] have invalid operate error",pRoomClient->m_idx);
		return -1;
	}
	int opidx=itidx->second;		//邀请者idx
	proominfo->invitelist.erase(itidx);
	
	//同意成为会员
	if(ret==1)
	{
		//房间会员数已满
		if(proominfo->userlistVIP.size()>=300)
		{
			BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
			char type=65;
			outstream->Write(type);
			outstream->Write(cmd);
			outstream->Write(seq);
			outstream->Write((int)LISTFULL);
			outstream->Flush();
			if(g_pNetProxy->SendToSrv(pRoomClient, *outstream)==-1)
			{
				AC_ERROR("ClientDataDecoder::DoReplyInvite:SendTo idx=%d error",pRoomClient->m_idx);
				return -1;
			}
						
			return 0;
		}
		
		// 判断是否为其它房间会员 
		map<int, ROOM_INFO*>::iterator it=	g_pNetProxy->m_roomlistinfo.roommap.begin();
		for (;it!=g_pNetProxy->m_roomlistinfo.roommap.end();it++)
		{
			ROOM_INFO* temp = it->second;
			map<int,int>::iterator itu=temp->userlistVIP.find(pRoomClient->m_idx);
			if (temp && itu != temp->userlistVIP.end())
			{
				AC_ERROR("ClientDataDecoder::DoReplyInvite:idx=%d is member in room=%d",pRoomClient->m_idx,temp->roomid);
				BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
				char type=65;
				outstream->Write(type);
				outstream->Write(cmd);
				outstream->Write(seq);
				outstream->Write((int)ALREADY);
				outstream->Flush();

				if(g_pNetProxy->SendToSrv(pRoomClient, *outstream)==-1)
				{
					AC_ERROR("ClientDataDecoder::DoReplyInvite:SendTo idx=%d error",pRoomClient->m_idx);
					return -1;
				}
			
				return 0;
			}
		}
		
		// 1，判断是通过还是不通过；2，如果成功，则向数据库操作并向其它会员广播；3，如果失败，则操作本地内存，并向其它会员广播
		BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
 		if(pDBSvr!=NULL)
 		{
			int outseq = g_pNetProxy->GetCounter()->Get();
			BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
			static const char* spname = {"DBMW_OpRoomUserAdd_HZSTAR"};//存储过程填充
			outstream->Write((short)CMD_CALLSP);
			outstream->Write(outseq);
			outstream->Write(spname,strlen(spname));
			outstream->Write((short)3);
			outstream->Write((char)PT_INPUT);
			outstream->Write((char)PDT_INT);
			outstream->Write((int)proominfo->roomid);
			outstream->Write((char)PT_INPUT);
			outstream->Write((char)PDT_INT);
			outstream->Write((int)pRoomClient->m_idx);	
			outstream->Write((char)PT_INPUT);
			outstream->Write((char)PDT_INT);
			outstream->Write((int)ret);			
			outstream->Flush();
			
			Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
			if(data != NULL)
			{
				if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1)
				{
					data->roomid = proominfo->roomid;
					data->opidx = opidx;
					data->bopidx = pRoomClient->m_idx;		
					data->cmd = cmd;
					data->seq=seq;
					data->outseq=outseq;
					data->number = pRoomClient->GetClientID();
					data->badd = ret;
					data->SetReactor(g_pNetProxy->GetReactor());
					data->RegisterTimer(DB_TIMEOUT);
					g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
				}
				else
				{
					AC_ERROR("ClientDataDecoder::DoReplyInvite: pDBSvr->AddBuf() error");
					g_pNetProxy->DestroyDBResult(data);
				}
			}
			else
			{
				AC_ERROR("ClientDataDecoder::DoReplyInvite: CreateDBResultdata error,data=%x",data);
			}
	 	}
		else
		{
			AC_ERROR("ClientDataDecoder::DoReplyInvite: pDBSvr=%x ",pDBSvr);
		}
		
	}


	return 0;
}

int ClientDataDecoder::DoGetGiftSend(ClientSocketBase *pClient,  short cmd, int seq)
{
	AC_DEBUG("ClientDataDecoder::DoGetGiftSend: cmd = %d seq = %d",  cmd ,seq);
	
	RoomClient* pRoomClient = (RoomClient*)pClient;
	if(pRoomClient==NULL)
	{
		AC_ERROR("ClientDataDecoder::DoGetGiftSend:pClient is NULL");
		return -1;
	}

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoGetGiftSend:roominfo error");
		return -1;
	}

	BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
	if(pDBSvr!=NULL)
	{
		int outseq = g_pNetProxy->GetCounter()->Get();
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		static const char* spname = {"DBMW_GetGiftSend_byIdx"};//存储过程填充
		outstream->Write((short)CMD_CALLSP);
		outstream->Write(outseq);
		outstream->Write(spname,strlen(spname));
		outstream->Write((short)1);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)pRoomClient->m_idx);
		outstream->Flush();
		
		Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
		if(data != NULL)
		{
			if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1)
			{
				data->roomid = proominfo->roomid;
				data->opidx = pRoomClient->m_idx;	
				data->bopidx=pRoomClient->m_idx;
				data->cmd = cmd;
				data->seq=seq;
				data->outseq=outseq;
				data->number=pRoomClient->GetClientID();
				data->SetReactor(g_pNetProxy->GetReactor());
				data->RegisterTimer(DB_TIMEOUT);
				g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
			}
			else
			{
				AC_ERROR("ClientDataDecoder::DoGetGiftSend:pDBSvr->AddBuf() error");
				g_pNetProxy->DestroyDBResult(data);
			}
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoGetGiftSend:CreateDBResultdata() error,data=%x",data);
		}		
	}	
	else
	{
		AC_ERROR("ClientDataDecoder::DoGetGiftSend:pDBSvr=%x ",pDBSvr);
	}


	return 0;
}

int ClientDataDecoder::DoGetGiftRecv(ClientSocketBase *pClient,  short cmd, int seq)
{
	AC_DEBUG("ClientDataDecoder::DoGetGiftRecv: cmd = %d seq = %d",  cmd ,seq);
	
	RoomClient* pRoomClient = (RoomClient*)pClient;
	if(pRoomClient==NULL)
	{
		AC_ERROR("ClientDataDecoder::DoGetGiftRecv:pClient is NULL");
		return -1;
	}

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoGetGiftRecv:roominfo error");
		return -1;
	}

	BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
	if(pDBSvr!=NULL)
	{
		int outseq = g_pNetProxy->GetCounter()->Get();
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		static const char* spname = {"DBMW_GetGiftRecv_byIdx"};//存储过程填充
		outstream->Write((short)CMD_CALLSP);
		outstream->Write(outseq);
		outstream->Write(spname,strlen(spname));
		outstream->Write((short)1);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)pRoomClient->m_idx);
		outstream->Flush();
		
		Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
		if(data != NULL)
		{
			if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1)
			{
				data->roomid = proominfo->roomid;
				data->opidx = pRoomClient->m_idx;	
				data->bopidx=pRoomClient->m_idx;
				data->cmd = cmd;
				data->seq=seq;
				data->outseq=outseq;
				data->number=pRoomClient->GetClientID();
				data->SetReactor(g_pNetProxy->GetReactor());
				data->RegisterTimer(DB_TIMEOUT);
				g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
			}
			else
			{
				AC_ERROR("ClientDataDecoder::DoGetGiftRecv:pDBSvr->AddBuf() error");
				g_pNetProxy->DestroyDBResult(data);
			}
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoGetGiftRecv:CreateDBResultdata() error,data=%x",data);
		}		
	}	
	else
	{
		AC_ERROR("ClientDataDecoder::DoGetGiftRecv:pDBSvr=%x ",pDBSvr);
	}


	return 0;
}

int ClientDataDecoder::DoSetRoomAutoOnmic(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{

	AC_DEBUG("ClientDataDecoder::DoSetRoomAutoOnmic: cmd = %d seq = %d",  cmd ,seq);
	
	RoomClient* pRoomClient = (RoomClient*)pClient;
	if(pRoomClient==NULL)
	{
		AC_ERROR("ClientDataDecoder::DoSetRoomAutoOnmic:pClient is NULL");
		return -1;
	}

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoSetRoomAutoOnmic:roominfo error");
		return -1;
	}

	if(CheckRightEx(pRoomClient->m_identity,pRoomClient->m_viptype, USER_ID_NONE, USER_ID_NONE, cmd)==0)
	{
		AC_ERROR("ClientDataDecoder::DoSetRoomAutoOnmic:power error");
		return 0;
	}

	char flag;
	if(!pinstream->Read(flag))
	{
		AC_ERROR("ClientDataDecoder::DoSetRoomAutoOnmic:read flag error");
		return -1;
	}

	AC_DEBUG("ClientDataDecoder::DoSetRoomAutoOnmic:roomid =%d,idx=%d,flag=%d",proominfo->roomid,pRoomClient->m_idx,flag);
	//暂停上麦
	if(flag==0)
	{
		proominfo->isAutoOnmic=false;

	}
	//开启上麦
	else if(flag==1)
	{
		proominfo->isAutoOnmic=true;

	}
	else
	{
		AC_ERROR("ClientDataDecoder::DoSetRoomAutoOnmic:flag invalid flag=%d",flag);
		return -1;

	}

	BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
	char type=65;

	//回应操作者
	outstream->Write(type);
	outstream->Write((short)cmd);
	outstream->Write(seq);
	outstream->Write((int)0);	//ret
	if(proominfo->isAutoOnmic)
		outstream->Write((char)1);
	else
		outstream->Write((char)0);
	outstream->Flush();	

	if(g_pNetProxy->SendToSrv(pClient, *outstream)==-1)
	{
		AC_ERROR("ClientDataDecoder::DoSetRoomAutoOnmic:g_pNetProxy->SendToSrv error");
	}

		
	//通知上麦状态
	outstream->Clear();
	outstream->Write(type);
	outstream->Write((short)ROOM_CMD_SB_SET_AUTOONMIC);
	outstream->Write(0);
	outstream->Write((int)0);	//ret
	outstream->Write(pRoomClient->m_idx); 
	if(proominfo->isAutoOnmic)
		outstream->Write((char)1);
	else
		outstream->Write((char)0);
	outstream->Flush();	
		
	if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream)==-1)
	{
		AC_ERROR("ClientDataDecoder::DoSetRoomAutoOnmic:BroadcastSBCmd error");
	}


	//if(CheckMiclistOnmic(proominfo)==-1)
	if(CheckMiclistOnmic2(proominfo)==-1)
	{
		AC_ERROR("ClientDataDecoder::DoSetRoomAutoOnmic:CheckMiclistOnmic error");
	}
	return 0;
	
}

//add by jinguanfu 2011/8/19
int ClientDataDecoder::DoDisableIP(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{

	AC_DEBUG("ClientDataDecoder::DoDisableIP: cmd = %d seq = %d",  cmd ,seq);
	
	RoomClient* pRoomClient = (RoomClient*)pClient;
	if(pRoomClient==NULL)
	{
		AC_ERROR("ClientDataDecoder::DoDisableIP:pClient is NULL");
		return -1;
	}

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoDisableIP:roominfo error");
		return -1;
	}
	
	int bopidx=0;//被禁者IDX
	if(!pinstream->Read(bopidx))
	{
		AC_ERROR("ClientDataDecoder::DoDisableIP:read bopidx error");
		return -1;
	}

	int flag=0;	//1:实际用户 2：机器人
	RoomClient* pDisableClient=NULL;
	//只能对房间内用户进行禁言
	map<int, RoomClient*>::iterator itu = proominfo->userlist.find(bopidx);
	map<int, RoomClient*>::iterator itvu = proominfo->vuserlist.find(bopidx);
	if(itu == proominfo->userlist.end()&&itvu==proominfo->vuserlist.end())
	{
		AC_ERROR("ClientDataDecoder::DoDisableMAC:idx=%d is not in room=%d",bopidx,proominfo->roomid);
		return 0;
	}
	else if(itu != proominfo->userlist.end())
	{
		flag=1;
		pDisableClient = (*itu).second; 
	}
	else if(itvu != proominfo->vuserlist.end())
	{
		flag=2;
		pDisableClient = (*itvu).second; 
	}

	if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,pDisableClient->m_identity,pDisableClient->m_viptype, cmd)==0)
	{
		AC_ERROR("ClientDataDecoder::DoDisableMAC:power error");
	 	return -1;
	}

	//被禁者为机器人,直接回应操作者
	if(flag==2)
	{
		//踢出机器人
		if(pDisableClient!=NULL)
			pDisableClient->VLeaveRoom();

		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		char type=65;

		//回应操作者
		outstream->Write(type);
		outstream->Write((short)cmd);
		outstream->Write(seq);
		outstream->Write((int)0);		//ret
		outstream->Write(bopidx);		//被禁者IDX
		outstream->Flush();	

		if(g_pNetProxy->SendToSrv(pClient, *outstream)==-1)
		{
			AC_ERROR("ClientDataDecoder::DoDisableMAC:g_pNetProxy->SendToSrv error");
		}

		return 0;
	}

	//
	BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
	if(pDBSvr!=NULL)
	{
		int outseq = g_pNetProxy->GetCounter()->Get();
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		static const char* spname = {"DBMW_DisableIP_byIDX"};//存储过程填充
		outstream->Write((short)CMD_CALLSP);
		outstream->Write(outseq);
		outstream->Write(spname,strlen(spname));
		outstream->Write((short)2);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)pRoomClient->m_idx);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)bopidx);
		outstream->Flush();
		
		Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
		if(data != NULL)
		{
			if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1)
			{
				data->roomid = proominfo->roomid;
				data->opidx = pRoomClient->m_idx;	
				data->bopidx=bopidx;
				data->cmd = cmd;
				data->seq=seq;
				data->outseq=outseq;
				data->number=pRoomClient->GetClientID();
				data->SetReactor(g_pNetProxy->GetReactor());
				data->RegisterTimer(DB_TIMEOUT);
				g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
			}
			else
			{
				AC_ERROR("ClientDataDecoder::DoDisableIP:pDBSvr->AddBuf() error");
				g_pNetProxy->DestroyDBResult(data);
			}
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoDisableIP:CreateDBResultdata() error,data=%x",data);
		}		
	}	
	else
	{
		AC_ERROR("ClientDataDecoder::DoDisableIP:pDBSvr=%x ",pDBSvr);
	}

	return 0;
}
//add by jinguanfu 2011/8/19
int ClientDataDecoder::DoDisableMAC(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{

	AC_DEBUG("ClientDataDecoder::DoDisableMAC: cmd = %d seq = %d",  cmd ,seq);
	
	RoomClient* pRoomClient = (RoomClient*)pClient;
	if(pRoomClient==NULL)
	{
		AC_ERROR("ClientDataDecoder::DoDisableMAC:pClient is NULL");
		return -1;
	}

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoDisableMAC:roominfo error");
		return -1;
	}
	
	int bopidx=0;//被禁者IDX
	if(!pinstream->Read(bopidx))
	{
		AC_ERROR("ClientDataDecoder::DoDisableMAC:read bopidx error");
		return -1;
	}

	int flag=0;	//1:实际用户 2：机器人
	RoomClient* pDisableClient=NULL;
	//只能对房间内用户进行禁言
	map<int, RoomClient*>::iterator itu = proominfo->userlist.find(bopidx);
	map<int, RoomClient*>::iterator itvu = proominfo->vuserlist.find(bopidx);
	if(itu == proominfo->userlist.end()&&itvu==proominfo->vuserlist.end())
	{
		AC_ERROR("ClientDataDecoder::DoDisableMAC:idx=%d is not in room=%d",bopidx,proominfo->roomid);
		return 0;
	}
	else if(itu != proominfo->userlist.end())
	{
		flag=1;
		pDisableClient = (*itu).second; 
	}
	else if(itvu != proominfo->vuserlist.end())
	{
		flag=2;
		pDisableClient = (*itvu).second; 
	}

	if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,pDisableClient->m_identity,pDisableClient->m_viptype, cmd)==0)
	{
		AC_ERROR("ClientDataDecoder::DoDisableMAC:power error");
	 	return -1;
	}

	//被禁者为机器人,直接回应操作者
	if(flag==2)
	{
		//踢出机器人
		if(pDisableClient!=NULL)
			pDisableClient->VLeaveRoom();

		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		char type=65;

		//回应操作者
		outstream->Write(type);
		outstream->Write((short)cmd);
		outstream->Write(seq);
		outstream->Write((int)0);		//ret
		outstream->Write(bopidx);		//被禁者IDX
		outstream->Flush();	

		if(g_pNetProxy->SendToSrv(pClient, *outstream)==-1)
		{
			AC_ERROR("ClientDataDecoder::DoDisableMAC:g_pNetProxy->SendToSrv error");
		}

		return 0;
	}

	//
	BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
	if(pDBSvr!=NULL)
	{
		int outseq = g_pNetProxy->GetCounter()->Get();
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		static const char* spname = {"DBMW_DisableMAC_byIDX"};//存储过程填充
		outstream->Write((short)CMD_CALLSP);
		outstream->Write(outseq);
		outstream->Write(spname,strlen(spname));
		outstream->Write((short)2);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)pRoomClient->m_idx);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write((int)bopidx);
		outstream->Flush();
		
		Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
		if(data != NULL)
		{
			if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1)
			{
				data->roomid = proominfo->roomid;
				data->opidx = pRoomClient->m_idx;	
				data->bopidx=bopidx;
				data->cmd = cmd;
				data->seq=seq;
				data->outseq=outseq;
				data->number=pRoomClient->GetClientID();
				data->SetReactor(g_pNetProxy->GetReactor());
				data->RegisterTimer(DB_TIMEOUT);
				g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
			}
			else
			{
				AC_ERROR("ClientDataDecoder::DoDisableMAC:pDBSvr->AddBuf() error");
				g_pNetProxy->DestroyDBResult(data);
			}
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoDisableMAC:CreateDBResultdata() error,data=%x",data);
		}		
	}	
	else
	{
		AC_ERROR("ClientDataDecoder::DoDisableMAC:pDBSvr=%x ",pDBSvr);
	}

	return 0;
}

int ClientDataDecoder::DoOnMicAddTM(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoOnMicAddTM:cmd = %d seq = %d", cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoOnMicAddTM:roominfo error");
		return -1;
	}

	AC_DEBUG("ClientDataDecoder::DoOnMicAddTM:roomid=%d,idx=%d ",pRoomClient->m_roomid,pRoomClient->m_idx);
	if(CheckRightEx(pRoomClient->m_identity, pRoomClient->m_viptype,USER_ID_NONE,VIP_LEVEL_NONE, cmd)==0)
	{
		AC_ERROR("ClientDataDecoder::DoOnMicAddTM:power error");
		return -1;
	}

	int idx = 0;
	if(!pinstream->Read(idx))    //操作对象的idx
	{
		AC_ERROR("ClientDataDecoder::DoOnMicAddTM:read idx error");
		return -1;
	}
    
	AC_DEBUG("ClientDataDecoder::DoOnMicAddTM:b_idx=%d ",idx); //add for debug lihongwu 2011/12/1

	/*if(idx == 0)
	{
		AC_ERROR("ClientDataDecoder::DoOnMicAddTM:idx error");
		return 0;
	}*/

	short remaintime = 0;        //剩余时间+延长时间
	if (!pinstream->Read(remaintime))
	{
		AC_ERROR("ClientDataDecoder::DoOnMicAddTM:read remaintime error");
		return -1;
	}

	//modify vy jinguanfu 2012/3/28 ,取消自由麦时间限制
	//if (remaintime <= 0 || remaintime > MIC_REMAINTM)
	if (remaintime <= 0)
	{
		AC_ERROR("ClientDataDecoder::DoOnMicAddTM:remaintime=%d error",remaintime);
		return -1;    
	}

	BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
	if (idx != 0 && idx == proominfo->onmic.idx && proominfo->onmic.musicid == 0)
	{
		AC_DEBUG("ClientDataDecoder::DoOnMicAddTM:idx =%d, proominfo->onmic.idx =%d, remaintime =%d",idx,proominfo->onmic.idx,remaintime);
		if (proominfo->m_pOnMicTimeout != NULL)
		{ 
			proominfo->m_pOnMicTimeout->UnRegisterTimer();
			proominfo->m_pOnMicTimeout->m_idx=proominfo->onmic.idx;
			proominfo->m_pOnMicTimeout->m_pClientDataDecoder=this;
			proominfo->m_pOnMicTimeout->m_type = TIMEOUT_ONMIC;
			int onmictimeout = remaintime + MUSIC_OFFSET;
			proominfo->m_pOnMicTimeout->RegisterTimer(onmictimeout);
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoOnMicAddTM:onmictimeout error");
			return -1; 
		}

		//回复操作者延麦成功
		char type = 65;
		outstream->Write(type);
		outstream->Write(cmd);
		outstream->Write(seq);
		outstream->Write((int)ADDTM_SUCESS);  //成功
		outstream->Write(idx);
		outstream->Flush();
		if(g_pNetProxy->SendToSrv(pClient, *outstream) == -1)
		{
			AC_ERROR("ClientDataDecoder::DoOnMicAddTM:g_pNetProxy->SendToSrv error");
		}

		//发送给房间内所有的人某人延麦时间成功的消息
			int seq2 = 0;
		BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
		short broadcasttime = remaintime;
		//char type = 65;
		outstream2->Write(type);
		outstream2->Write((short)ROOM_CMD_SB_ADDTM);
		outstream2->Write(seq2);
		outstream2->Write((int)proominfo->onmic.idx);
		outstream2->Write(broadcasttime);
		outstream2->Flush();
		if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2) == -1)
		{
			AC_ERROR("ClientDataDecoder::DoOnMicAddTM:g_pNetProxy->BroadcastSBCmd error");
		}
	}
	else
	{
		AC_ERROR("ClientDataDecoder::DoOnMicAddTM:idx onmic error");
		//延时误操作回复失败
		char type = 65;
		outstream->Write(type);
		outstream->Write(cmd);
		outstream->Write(seq);
		outstream->Write((int)ADDTM_FAILURE);    //失败
		outstream->Write(idx);
		outstream->Flush();
		if(g_pNetProxy->SendToSrv(pClient, *outstream) == -1)
		{
			AC_ERROR("ClientDataDecoder::DoOnMicAddTM:g_pNetProxy->SendToSrv error");
			return -1;
		}
		   //延时误操作时不断开连接
	}

	return 0;
}


int ClientDataDecoder::DoUpdateMusicTime(ClientSocketBase *pClient,  short cmd, int seq, BinaryReadStream* pinstream)
{
	AC_DEBUG("ClientDataDecoder::DoUpdateMusicTime:cmd = %d seq = %d", cmd ,seq);

	RoomClient* pRoomClient = (RoomClient*)pClient;

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pClient);
	if(proominfo == NULL)
	{
		AC_ERROR("ClientDataDecoder::DoUpdateMusicTime:roominfo error or client not in room,idx=%d",pRoomClient->m_idx);
		return -1;
	}

	//判断此人是否在麦上
	if(	pRoomClient->m_idx!=proominfo->onmic.idx)
	{
		AC_ERROR("ClientDataDecoder::DoUpdateMusicTime:client not onmic ,idx=%d,onmic=%d",pRoomClient->m_idx,proominfo->onmic.idx);
		return -1;
	}

	short musictime = 0;
	if(!pinstream->Read(musictime))
	{
		AC_ERROR("ClientDataDecoder::DoUpdateMusicTime:read musictime error");
		return -1;
	}
	
	AC_DEBUG("ClientDataDecoder::DoUpdateMusicTime:musictime=%d",musictime);
	
	//检查歌曲时间合法性(歌曲时长不超过10分钟)
	if(musictime <= 0 ||musictime>600)
	{
		//取消上麦超时计时
		if(proominfo->m_pOnMicTimeout!=NULL)
			proominfo->m_pOnMicTimeout->UnRegisterTimer();
		
		//下麦通知
		char type=65;
		BinaryWriteStream* outstream = StreamFactory::InstanceWriteStream();
		outstream->Clear();
		outstream->Write(type);
		outstream->Write((short)ROOM_CMD_SB_OFFMIC);
		outstream->Write(0);
		outstream->Write((char)0);	//haspk
		outstream->Write((int)proominfo->onmic.idx);					
		outstream->Write((int)0);	//新增经验值add by jinguanfu 2011/8/10
	    outstream->Write(0);			//打分信息体	
		outstream->Flush();
		g_pNetProxy->BroadcastSBCmd(proominfo, *outstream) ;

		proominfo->onmic.init();
	
		//下麦5秒后下一个人上麦
		roomtimeout* ptb = g_pNetProxy->CreateRoomTimeout();
		if(ptb!=NULL)
		{
	        ptb->m_roomid = pRoomClient->m_roomid;
			ptb->m_idx = pRoomClient->m_idx;
			ptb->m_time = OFFMIC_WAIT; //5s
			ptb->m_type = TIMEOUT_OFFMIC;
			ptb->SetReactor(g_pNetProxy->GetReactor());
			ptb->m_pClientDataDecoder=this;
			ptb->m_haspk = 0;
			ptb->RegisterTimer(ptb->m_time);
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoUpdateMusicTime: CreateRoomTimeout error");
			//CheckMiclistOnmic(proominfo);
			CheckMiclistOnmic2(proominfo);
		}

		return 0;
	}

	proominfo->onmic.musictime = musictime;

	proominfo->m_pOnMicTimeout->UnRegisterTimer();
	
	//上麦超时设置
	if(proominfo->m_pOnMicTimeout!=NULL)
	{
		proominfo->m_pOnMicTimeout->UnRegisterTimer();
		proominfo->m_pOnMicTimeout->m_idx=proominfo->onmic.idx;
		proominfo->m_pOnMicTimeout->m_pClientDataDecoder=this;
		proominfo->m_pOnMicTimeout->m_type = TIMEOUT_ONMIC;
		int onmictimeout=proominfo->onmic.musictime+MUSIC_OFFSET;
		proominfo->m_pOnMicTimeout->RegisterTimer(onmictimeout);	
	}
	else
	{
		//初始化上麦超时对象
		proominfo->m_pOnMicTimeout=g_pNetProxy->CreateRoomTimeout();
		if(proominfo->m_pOnMicTimeout!=NULL)
		{
			proominfo->m_pOnMicTimeout->SetReactor(g_pNetProxy->GetReactor());
			proominfo->m_pOnMicTimeout->m_roomid=proominfo->roomid;
			proominfo->m_pOnMicTimeout->UnRegisterTimer();
			proominfo->m_pOnMicTimeout->m_idx=proominfo->onmic.idx;
			proominfo->m_pOnMicTimeout->m_pClientDataDecoder=this;
			proominfo->m_pOnMicTimeout->m_type = TIMEOUT_ONMIC;
			int onmictimeout=proominfo->onmic.musictime+MUSIC_OFFSET;	
			proominfo->m_pOnMicTimeout->RegisterTimer(onmictimeout);
		}
		else
		{
			AC_ERROR("ClientDataDecoder::DoUpdateMusicTime:CreateRoomTimeout error");
			return -1;
		}
	}
		

	return 0;
}

