#include "roomclient.h"
#include "netproxy.h"
#include <errno.h>
#include "StreamFactory.h"

#define FIRSTKEY "9158xingguang10"


// void RoomClient::ErrorClose()
// {
//     AC_ERROR("in Error Close idx = %d", m_idx);
//     ClientDataDecoder* pClientDecoder = (ClientDataDecoder*)GetDecoder();
// 
//     do{
//         //if(m_roomid == 0 ||m_btoken == 0)
//         if(m_roomid == 0)
//         {
//             AC_ERROR("RoomClient::ErrorClose:client error");
//             break;
//         }
//         ROOM_INFO* proominfo =g_pNetProxy->GetRoomInfo(m_roomid);
//         if(proominfo == NULL)
//         {
//             AC_ERROR("RoomClient::ErrorClose:roomid error");
//             break;
//         }
// 
// 	 //排麦列表清除 
// 	 pClientDecoder->ClearWaitmicList(proominfo, (int)m_idx);
// 
// 
// 	//持麦列表清除
// 	pClientDecoder->ClearOnmic(proominfo, (int)m_idx);
// 
// 	//DB对应map清除
// 	g_pNetProxy->ClearClientDBMap(this);
// 
//         map<int, RoomClient*>::iterator itu = proominfo->userlist.find(m_idx);
//         if(itu == proominfo->userlist.end())
//         {
//             AC_ERROR("RoomClient::ErrorClose:idx error");
//             break;
//         }
// 
//         //用户列表清除
//         proominfo->userlist.erase(itu);
// 
// //         //发送给房间内所有的人有人离开房间的消息
// //         int seq2 = 0;
// //         BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
// //         char type = 65;
// //         outstream->Write(type);
// //         outstream->Write((short)ROOM_CMD_SB_LEAVEROOM);
// //         outstream->Write(seq2);
// //         outstream->Write((int)m_idx);
// //         outstream->Flush();
// //         /*
// //         if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2) == -1)
// //         {
// //         AC_ERROR("ClientDataDecoder::DoLeaveRoom:g_pNetProxy->BroadcastSBCmd error");
// //         return -1;
// //         }
// //         */
// //         char outbuf[20];                     
// //         int outbuflen = sizeof(outbuf);  
// //         if(!StreamCompress(outstream->GetData(),(int)outstream->GetSize(),outbuf,outbuflen))
// //         {
// //             AC_ERROR("StreamCompress error");
// //             return;
// //         }
// //         map<int, RoomClient*>::iterator itsend = proominfo->userlist.begin();
// //         for(;itsend != proominfo->userlist.end();itsend++)
// //         {
// //             RoomClient* psend = itsend->second;
// //             if(psend != NULL)
// //             {
// //                 //if(StreamEncrypt(outstream->GetData(),(int)outstream->GetSize(),outbuf,outbuflen,psend->m_sessionkey,1))
// //                 {
// //                     if(psend->AddBuf(outbuf,outbuflen) == -1)
// //                     {
// //                         psend->ErrorClose();
// //                     }
// //                 }
// //             }
// //         }
// 
//         //
//         if(m_sex)
//         {
//             map<int,int>::iterator it_male=proominfo->userlistMale.find((int)m_idx);
//             if(it_male!=proominfo->userlistMale.end())
//                 proominfo->userlistMale.erase(it_male);
//         }else
//         {
//             map<int,int>::iterator it_female=proominfo->userlistFemale.find((int)m_idx);
//             if(it_female!=proominfo->userlistFemale.end())
//                 proominfo->userlistFemale.erase(it_female);
//         }
// 
//         //在线管理员清除
//         if(m_identity==USER_ID_OWNER||m_identity==USER_ID_OWNER_S
//             ||m_identity==USER_ID_VJ||m_identity==USER_ID_VJ_A)
//         {
//             map<int,ROOM_MANAGER>::iterator itMO=proominfo->managerlist_online.find((int)m_idx);
//             if(itMO!=proominfo->managerlist_online.end())
//                 proominfo->managerlist_online.erase(itMO);
// 
//         }
// 		
// 		//add by jinguanfu 2011/3/4
// 		//消除邀请列表
// 		map<int,int>::iterator itinvite=proominfo->invitelist.find(m_idx);
// 		if(itinvite!=proominfo->invitelist.end())
// 		{
// 			proominfo->invitelist.erase(itinvite);
// 		}
// 
// 
//         //发送给大厅有人离开房间的消息并通知当前房间内在线男女数
//         BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
//         outstream->Clear();
//         outstream->Write((short)HALL_CMD_LEAVEROOM);
//         int seq = g_pNetProxy->GetCounter()->Get();
//         outstream->Write(seq);
//         outstream->Write((char)ROOM_TYPE_ROOM);	//ADD BY JINGUANFU 2010/7/9
//         outstream->Write((int)m_idx);
//         outstream->Write((int)m_roomid);	
//         outstream->Write((short)proominfo->userlistFemale.size());
//         outstream->Write((short)proominfo->userlistMale.size());
// 		//add by jinguanfu 2011/6/30
// 		outstream->Write((short)proominfo->userlist.size());		//房间真实用户数
// 
//         outstream->Flush();
// 
//         HallSvrClient *pHallSvr = g_pNetProxy->GetHallSvr();
//         if(pHallSvr!=NULL)
//             pHallSvr->AddBuf(outstream->GetData(), outstream->GetSize());
//         else
//             AC_ERROR("RoomClient::ErrorClose:pHallSvr is NULL");
// 
// 		AC_DEBUG("RoomClient::ErrorClose:roomid=%d,usernum=%d",proominfo->roomid,proominfo->userlist.size());
// 		/*
// 		outstream->Clear();
// 		outstream->Write((short)FACTORY_CMD_LOGOUT_R2F);
// 		outstream->Write(0);
// 		outstream->Write(m_idx); //用户idx
// 		outstream->Write(m_roomid); //所在房间ID
// 		outstream->Flush();
// 		
// 		BackClientSocketBase *pFactocySvr = g_pNetProxy->GetFactoryServer();
// 		if(pFactocySvr!=NULL)
// 		{
// 			if(pFactocySvr->AddBuf(outstream->GetData(), outstream->GetSize())==-1) 
// 			{
// 				AC_ERROR("RoomClient::ErrorClose:pFactocySvr->AddBuf Error");
// 			}else
// 			{
// 				AC_DEBUG("RoomClient::ErrorClose:Send to pFactocySvr success,idx=%d roomid=%d ",m_idx,m_roomid);
// 			}
// 				
// 		}
// 		else
// 		{
// 			AC_ERROR("RoomClient::ErrorClose:pFactocySvr is NULL");
// 		}
// 		*/
// 		//add by jinguanfu 2011/4/27
// 		outstream->Clear();
// 		outstream->Write((short)CMD_ROOM_LEAVEROOM_R2LS);
// 		outstream->Write(0);
// 		outstream->Write(m_idx);		//用户idx
// 		outstream->Write(m_roomid);	//所在房间ID
// 		outstream->Flush();
// 		BackClientSocketBase *pLogSvr = g_pNetProxy->GetLogServer();
// 		if(pLogSvr!=NULL)
// 		{
// 			if(pLogSvr->AddBuf(outstream->GetData(), outstream->GetSize())==-1) 
// 			{
// 				AC_ERROR("RoomClient::Close:pLogSvr->AddBuf Error");
// 			}
// 		}
// 		else
// 		{
// 			AC_ERROR("RoomClient::Close:pLogSvr is NULL");
// 		}
// 		
// 
// 		//发送错误信息到日志服务器
// 		char errorinfo[1024]={0};
// 		sprintf(errorinfo,"ErrorClose");
// 		outstream->Clear();
// 		outstream->Write((short)CMD_ROOM_ERRORINFO_R2LS);
// 		outstream->Write(0);
// 		outstream->Write(m_idx); //用户idx
// 		outstream->Write(errorinfo,strlen(errorinfo));
// 		outstream->Flush();
// 
// 		//BackClientSocketBase *pLogSvr = g_pNetProxy->GetLogServer();
// 		if(pLogSvr!=NULL)
// 		{
// 			if(pLogSvr->AddBuf(outstream->GetData(), outstream->GetSize())==-1) 
// 			{
// 				AC_ERROR("RoomClient::ErrorClose:pLogSvr->AddBuf Error");
// 			}
// 		}
// 		else
// 		{
// 			AC_ERROR("RoomClient::ErrorClose:pLogSvr is NULL");
// 		}
// 
//         
//     }while(0);
// 	init(1);
// 	ClientSocket::Close();
// 
// }

void RoomClient::errorClear(int idx)
{
	ClientDataDecoder* pClientDecoder = (ClientDataDecoder*)GetDecoder();
	map<int, ROOM_INFO*>::iterator it = g_pNetProxy->m_roomlistinfo.roommap.begin(); 
	for(; it != g_pNetProxy->m_roomlistinfo.roommap.end(); it++)
	{
		ROOM_INFO* proominfo = it->second;
		if(proominfo)
		{
			//用户列表
			map<int, RoomClient*>::iterator itu = proominfo->userlist.find(idx);
			//找到了
			if(itu != proominfo->userlist.end())
			{
				proominfo->userlist.erase(itu);
			}

			//排麦列表清除 
			pClientDecoder->ClearWaitmicList(proominfo, (int)idx);


			//持麦列表清除
			pClientDecoder->ClearOnmic(proominfo, (int)idx);

			//timeout 时清除
			//DB对应map清除
			//g_pNetProxy->ClearClientDBMap(this);

			if(m_sex)
			{
				map<int,int>::iterator it_male=proominfo->userlistMale.find((int)idx);
				if(it_male!=proominfo->userlistMale.end())
					proominfo->userlistMale.erase(it_male);
			}else
			{
				map<int,int>::iterator it_female=proominfo->userlistFemale.find((int)idx);
				if(it_female!=proominfo->userlistFemale.end())
					proominfo->userlistFemale.erase(it_female);
			}

			//在线管理员清除
			if(m_identity==USER_ID_OWNER||m_identity==USER_ID_OWNER_S
				||m_identity==USER_ID_VJ||m_identity==USER_ID_VJ_A)
			{
				map<int,ROOM_MANAGER>::iterator itMO=proominfo->managerlist_online.find((int)idx);
				if(itMO!=proominfo->managerlist_online.end())
					proominfo->managerlist_online.erase(itMO);

			}

			//消除邀请列表
			map<int,int>::iterator itinvite=proominfo->invitelist.find(idx);
			if(itinvite!=proominfo->invitelist.end())
			{
				proominfo->invitelist.erase(itinvite);
			}
		}
	}
}

void RoomClient::clearRoomStat()
{
	AC_DEBUG("clearRoomStat idx = %d roomid = %d btoken = %d fd = %d proomclient = %x", m_idx, m_roomid, m_btoken, GetFD(), this);
	ClientDataDecoder* pClientDecoder = (ClientDataDecoder*)GetDecoder();

	do{
		//这个客户端连接已经被清理过了
		if(m_idx == 0)
		{
			AC_ERROR("idx == 0");
			//做错误清理
			errorClear(m_idx);
			break;
		}

		//这个客户端有可能连接上来，认证都没有过去，需要关闭这个连接
		if(m_btoken == 0)
		{
			AC_ERROR("token == 0");
			break;
		}

		//这个客户端dologin失败
		if(m_roomid == 0)
		{
			AC_ERROR("roomid == 0");
			break;
		}

		ROOM_INFO* proominfo =g_pNetProxy->GetRoomInfo(m_roomid);
		if(proominfo == NULL)
		{
			AC_ERROR("RoomClient::Close:roomid error, m_roomid = %d", m_roomid);
			break;
		}

		map<int, RoomClient*>::iterator itu = proominfo->userlist.find(m_idx);
		if(itu == proominfo->userlist.end())
		{
			AC_ERROR("RoomClient:: idx error idx = %d", m_idx);
		}
		else
		{
			//用户列表清除
			proominfo->userlist.erase(itu);
		}

		//排麦列表清除 
		pClientDecoder->ClearWaitmicList(proominfo, (int)m_idx);


		//持麦列表清除
		pClientDecoder->ClearOnmic(proominfo, (int)m_idx);

		//timeout 时清除
		//DB对应map清除
		g_pNetProxy->ClearClientDBMap(this);

		if(m_sex)
		{
			map<int,int>::iterator it_male=proominfo->userlistMale.find((int)m_idx);
			if(it_male!=proominfo->userlistMale.end())
				proominfo->userlistMale.erase(it_male);
		}else
		{
			map<int,int>::iterator it_female=proominfo->userlistFemale.find((int)m_idx);
			if(it_female!=proominfo->userlistFemale.end())
				proominfo->userlistFemale.erase(it_female);
		}

		//在线管理员清除
		if(m_identity==USER_ID_OWNER||m_identity==USER_ID_OWNER_S
			||m_identity==USER_ID_VJ||m_identity==USER_ID_VJ_A)
		{
			map<int,ROOM_MANAGER>::iterator itMO=proominfo->managerlist_online.find((int)m_idx);
			if(itMO!=proominfo->managerlist_online.end())
				proominfo->managerlist_online.erase(itMO);

		}

		//消除邀请列表
		map<int,int>::iterator itinvite=proominfo->invitelist.find(m_idx);
		if(itinvite!=proominfo->invitelist.end())
		{
			proominfo->invitelist.erase(itinvite);
		}

		//发送给这个人离开房间的消息
		int seq2 = 0;
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		char type = 65;
		outstream->Write(type);
		outstream->Write((short)ROOM_CMD_SB_LEAVEROOM);
		outstream->Write(seq2);
		outstream->Write((int)m_idx);
		outstream->Flush();
		g_pNetProxy->SendToSrv(this,*outstream, 1, 1);

		/*
		if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2) == -1)
		{
		AC_ERROR("ClientDataDecoder::DoLeaveRoom:g_pNetProxy->BroadcastSBCmd error");
		return -1;
		}
		*/

		//发送给房间内所有的人有人离开房间的消息
		char outbuf[20];                     
		int outbuflen = sizeof(outbuf);  
		if(!StreamCompress(outstream->GetData(),(int)outstream->GetSize(),outbuf,outbuflen))
		{
			AC_ERROR("StreamCompress error");
			break;
		}

		map<int, RoomClient*>::iterator itsend = proominfo->userlist.begin();
		for(;itsend != proominfo->userlist.end();itsend++)
		{
			RoomClient* psend = itsend->second;
			if(psend != NULL)
			{
				//if(StreamEncrypt(outstream->GetData(),(int)outstream->GetSize(),outbuf,outbuflen,psend->m_sessionkey,1))
				{
					AC_DEBUG("send to client sb leaveroom idx = %d", psend->m_idx);
					if(psend->AddBuf(outbuf,outbuflen) == -1)
					{
						//psend->ErrorClose();
						AC_ERROR("addbuf error");
					}
				}
			}
			else
			{
				AC_ERROR("psend == NULL");
			}
		}

		//发送给大厅有人离开房间的消息并通知当前房间内在线男女数
		outstream->Clear();
		outstream->Write((short)HALL_CMD_LEAVEROOM);
		int seq = g_pNetProxy->GetCounter()->Get();
		outstream->Write(seq);
		outstream->Write((char)ROOM_TYPE_ROOM);	//ADD BY JINGUANFU 2010/7/9
		outstream->Write((int)m_idx);
		outstream->Write((int)m_roomid);	
		outstream->Write((short)proominfo->userlistFemale.size());
		outstream->Write((short)proominfo->userlistMale.size());
		//add by jinguanfu 2011/6/30
		outstream->Write((short)proominfo->userlist.size());		//房间真实用户数
		outstream->Flush();

		HallSvrClient *pHallSvr = g_pNetProxy->GetHallSvr();
		if(pHallSvr!=NULL)
			pHallSvr->AddBuf(outstream->GetData(), outstream->GetSize());
		else
			AC_ERROR("pHallSvr is NULL");

		AC_DEBUG("roomid=%d,usernum=%d",proominfo->roomid,proominfo->userlist.size());

		/*
		outstream->Clear();
		outstream->Write((short)FACTORY_CMD_LOGOUT_R2F);
		outstream->Write(0);
		outstream->Write(m_idx); //用户idx
		outstream->Write(m_roomid); //所在房间ID
		outstream->Flush();

		BackClientSocketBase *pFactocySvr = g_pNetProxy->GetFactoryServer();
		if(pFactocySvr!=NULL)
		{
		if(pFactocySvr->AddBuf(outstream->GetData(), outstream->GetSize())==-1) 
		{
		AC_ERROR("RoomClient::Close:pFactocySvr->AddBuf Error");
		}else
		{
		AC_DEBUG("RoomClient::Close:Send to pFactocySvr success,idx=%d roomid=%d ",m_idx,m_roomid);
		}

		}
		else
		{
		AC_ERROR("RoomClient::Close:pFactocySvr is NULL");
		}
		*/
		//发送状态到日志服务器
		outstream->Clear();
		outstream->Write((short)CMD_ROOM_OFFMIC_R2LS);
		outstream->Write(0);
		outstream->Write(m_idx); 					//用户idx
		outstream->Write(proominfo->roomid); 	//所在房间ID
		outstream->Flush();

		BackClientSocketBase *pLogSvr = g_pNetProxy->GetLogServer();
		if(pLogSvr!=NULL)
		{
			if(pLogSvr->AddBuf(outstream->GetData(), outstream->GetSize())==-1) 
			{
				AC_ERROR("pLogSvr->AddBuf Error");
			}

		}
		else
		{
			AC_ERROR("pLogSvr is NULL");
		}

		//add by jinguanfu 2011/4/27
		outstream->Clear();
		outstream->Write((short)CMD_ROOM_LEAVEROOM_R2LS);
		outstream->Write(0);
		outstream->Write(m_idx);		//用户idx
		outstream->Write(m_roomid);	//所在房间ID
		outstream->Flush();
		if(pLogSvr!=NULL)
		{
			if(pLogSvr->AddBuf(outstream->GetData(), outstream->GetSize())==-1) 
			{
				AC_ERROR("pLogSvr->AddBuf Error");
			}
		}
		else
		{
			AC_ERROR("pLogSvr is NULL");
		}

		if(strlen(m_ErrMsg)>0)
		{
			//发送错误信息到日志服务器
			outstream->Clear();
			outstream->Write((short)CMD_ROOM_ERRORINFO_R2LS);
			outstream->Write(0);
			outstream->Write(m_idx); //用户idx
			outstream->Write(m_ErrMsg,strlen(m_ErrMsg));
			outstream->Flush();

			if(pLogSvr!=NULL)
			{
				if(pLogSvr->AddBuf(outstream->GetData(), outstream->GetSize())==-1) 
				{
					AC_ERROR("RoomClient::Close:pLogSvr->AddBuf Error");
				}
			}
			else
			{
				AC_ERROR("RoomClient::Close:pLogSvr is NULL");
			}
		}


	}while(0);

	initAll();
	
	ClientSocket::Close();

}

void RoomClient::Close()
{
	AC_DEBUG("close roomclient idx = %d roomid =% d fd = %d", m_idx, m_roomid, GetFD());
	clearRoomStat();
}


void RoomClient::LeaveRoom()
{
	AC_DEBUG("LeaveRoom idx = %d roomid =% d fd = %d", m_idx, m_roomid, GetFD());
	clearRoomStat();
}


int RoomClient::DirectSend(const char* buf,size_t buflen)
{
	int len = 0;
	char* p  = (char*)buf;
	int maxsendcount = 3; //防止发送错误卡死线程
	while(buflen)
	{
		if(maxsendcount <= 0)
		{
			AC_ERROR("send error more than maxsend");
			return -1;
		}

		len = ::send(m_fd, p, buflen, 0);
		if(len < 0)
		{
			buflen = 0;
			if(errno != EINTR)
			{
				AC_ERROR("send error fd = %d errno = %d", m_fd, errno);
				return -1;
			}
			else
			{
				AC_INFO("send EINTR error fd = %d errno = %d", m_fd, errno);
				return 0;
			}
		}
		else if(len == 0)
		{
			buflen = 0;
			AC_ERROR("RoomClient::DirectSend:len == 0");
			return -1;
		}
		p += len;
		buflen -= len;
		maxsendcount --;
	}
	return 0;
}

int RoomClient::SendEncryptBuf(BinaryWriteStream &stream, int xtea, int bdirect)
{
	char outbuf[65535];                     
	int outbuflen = sizeof(outbuf);    

	if(xtea)
	{
		if(StreamEncrypt(stream.GetData(),(int)stream.GetSize(),outbuf,outbuflen,m_sessionkey,xtea))
		{
// 	 		if(!IsConnect())
// 	 		{
// 	 			return false;
// 	 		}
			if(bdirect)
			{
				if(DirectSend(outbuf, outbuflen) != 0)
					return -1;
			}
			else
			{
				if(AddBuf(outbuf,outbuflen) != 0)                                                    
					return -1;                                   
			}  
			return 0;
		}
	}
	else
	{
		if(StreamEncrypt(stream.GetData(),(int)stream.GetSize(),outbuf,outbuflen,FIRSTKEY,xtea))
		{
// 			if(!IsConnect())
// 			{
// 				return false;
// 			}
			if(bdirect)
			{
				if(DirectSend(outbuf, outbuflen) != 0)
				return -1;
			}
			else
			{
				if(AddBuf(outbuf,outbuflen) != 0)                                                      
					return -1;                                   
			} 
			return 0;
		}
	}

	return -1;
}

int RoomClient::SendCompressBuf(BinaryWriteStream &stream, int bdirect)
{
	char outbuf[65535];                     
	int outbuflen = sizeof(outbuf);                 
	if(StreamCompress(stream.GetData(),(int)stream.GetSize(),outbuf,outbuflen))
	{
// 		if(!IsConnect())
// 		{
// 			return false;
// 		}

		if(bdirect)
		{
			if(DirectSend(outbuf, outbuflen) != 0)
			return -1;
		}
		else
		{
			if(AddBuf(outbuf,outbuflen) != 0)                                                     
				return -1;                                
		}
		return 0;
                                  
	}
	return -1;
}

//add by jinguanfu 2010/11/27 <begin>
//虚拟用户退出房间
int RoomClient::VLeaveRoom()
{
	ROOM_INFO* proominfo=g_pNetProxy->GetRoomInfo(m_roomid);
	if(proominfo==NULL)
	{
		AC_ERROR("RoomClient::VLeaveRoom:GetRoomInfo error,roomid=%d",m_roomid);
		return -1;
	}

	//发送给房间内真实用户有人离开房间的消息
	BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
	char type = 65;
	outstream2->Write(type);
	outstream2->Write((short)ROOM_CMD_SB_LEAVEROOM);
	outstream2->Write(0);
	outstream2->Write(m_idx);
	outstream2->Flush();

	char outbuf[50];                     
	int outbuflen = sizeof(outbuf);  
	if(!StreamCompress(outstream2->GetData(),(int)outstream2->GetSize(),outbuf,outbuflen))
	{
		AC_ERROR("RoomClient::VLeaveRoom:StreamCompress error");
		return -1;
	}

	map<int, RoomClient*>::iterator itsend = proominfo->userlist.begin();
	for(;itsend != proominfo->userlist.end();itsend++)
	{
		RoomClient* psend = itsend->second;
		if(psend != NULL)
		{
			if(psend->AddBuf(outbuf,outbuflen) == -1)
			{
				//psend->ErrorClose();
				AC_ERROR("AddBuf == -1");
			} 
        }
	}

	//用户在线人数更新
	if(m_sex)
	{
		map<int,int>::iterator it_male=proominfo->userlistMale.find(m_idx);
		if(it_male!=proominfo->userlistMale.end())
			proominfo->userlistMale.erase(it_male);
	}else
	{
		map<int,int>::iterator it_female=proominfo->userlistFemale.find(m_idx);
		if(it_female!=proominfo->userlistFemale.end())
			proominfo->userlistFemale.erase(it_female);
	}
	
	//add by jinguanfu 2011/3/4
	//消除邀请列表
	map<int,int>::iterator itinvite=proominfo->invitelist.find(m_idx);
	if(itinvite!=proominfo->invitelist.end())
	{
		proominfo->invitelist.erase(itinvite);
	}


	//向大厅发送退出房间通知
	BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
	outstream->Write((short)HALL_CMD_LEAVEROOM);
	outstream->Write(0);
	outstream->Write((char)ROOM_TYPE_ROOM);	
	outstream->Write(m_idx);
	outstream->Write((int)m_roomid);	
	outstream->Write((short)proominfo->userlistFemale.size());
	outstream->Write((short)proominfo->userlistMale.size());
	//add by jinguanfu 2011/6/30
	outstream->Write((short)proominfo->userlist.size());		//房间真实用户数
	outstream->Flush();

	HallSvrClient *pHallSvr = g_pNetProxy->GetHallSvr();
	if(pHallSvr!=NULL)
	{
		pHallSvr->AddBuf(outstream->GetData(), outstream->GetSize());
	}
	else
	{
		AC_ERROR("RoomClient::VLeaveRoom: pHallSvr is NULL");
		return -1;
	}
	
	AC_DEBUG("RoomClient::VLeaveRoom:roomid=%d,usernum=%d",proominfo->roomid,proominfo->userlist.size());

	g_pNetProxy->DestroyClient(this);
	
	return 0;
}
//add by jinguanfu 2010/11/27 <end>






