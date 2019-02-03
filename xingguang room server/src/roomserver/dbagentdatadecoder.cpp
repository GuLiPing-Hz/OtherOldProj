#include "netproxy.h"
#include "ac/log/log.h"
#include "ac/util/md5.h"
#include "ac/util/protocolstream.h"
#include "StreamFactory.h"

int DbagentDataDecoder::OnPackage(ClientSocketBase* pClient,const char* buf,size_t buflen)
{
	//char ip[32];
	//pClient->GetPeerIp(ip);
	//AC_DEBUG("%s %s  %d",ip,buf,buflen);

	NOT_USED(pClient);

	BinaryReadStream instream(buf, buflen);
	short cmd;
	if(!instream.Read(cmd))
	{
		AC_ERROR("DbagentDataDecoder::OnPackage:read cmd error");
		return -1;
	}
	int seq;
	if(!instream.Read(seq))
	{
		AC_ERROR("DbagentDataDecoder::OnPackage:read seq error");
		return -1;
	}

	Dbresultdata* pdata = g_pNetProxy->GetClientDBMap(seq);
	if(pdata == NULL)
	{
		AC_ERROR("DbagentDataDecoder::OnPackage:Dbresultdata not exist");
		return 0;
	}
	
	int ret;
	if(!instream.Read(ret))
	{
		AC_ERROR("DbagentDataDecoder::OnPackage:read ret error");
		return -1;
	}
	if(ret < 0)
	{
		AC_ERROR("DbagentDataDecoder::OnPackage:db ret error,ret=%d",ret);
		return -1;
	}
	short argnum;
	if(!instream.Read(argnum))
	{
		AC_ERROR("DbagentDataDecoder::OnPackage:read argnum error argnum=%d",argnum);
		return -1;
	}
	if(argnum != 0)//这个版本统一为0
	{
		AC_ERROR("DbagentDataDecoder::OnPackage:argnum error");	
		return -1;
	}

	char info[65535];
	size_t infolen;
	if(!instream.Read(info,sizeof(info),infolen))
	{
		AC_ERROR("DbagentDataDecoder::OnPackage:read info error");
		return -1;
	}

	BinaryReadStream infostream(info,infolen);
	short rownum;
	if(!infostream.Read(rownum))
	{
		AC_ERROR("DbagentDataDecoder::OnPackage:read row num error");
		return -1;
	}

	short colnum;
	if(!infostream.Read(colnum))
	{
		AC_ERROR("DbagentDataDecoder::OnPackage:read col num error");
		return -1;
	}

	//add by jinguanfu 2011/1/29
	if(pdata->opidx==0 &&pdata->bopidx==0)
	{
		AC_ERROR("DbagentDataDecoder::OnPackage:opidx=%d ,bopidx=%d",pdata->opidx,pdata->bopidx);
		pdata->UnRegisterTimer();
        g_pNetProxy->ClearClientDBMap(seq);
        g_pNetProxy->DestroyDBResult(pdata);
		return 0;
	}
	switch(pdata->cmd)
	{
	case ROOM_CMD_UPDATE_BLACKLIST:
		if(DoUpdateBlacklist(pdata,&infostream) == -1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoUpdateBlacklist error");
		}
		break;
	case ROOM_CMD_UPDATE_MGRLIST:
		if(DoUpdateMgr(pdata) == -1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoUpdateMgr error");
		}
		break;
	case ROOM_CMD_UPDATE_CONTENT:
		if(DoUpdateContent(pdata) == -1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoUpdateContent error");
		}
		 break;
	case ROOM_CMD_SEND_GIFT:       
		if(DoSendFlower(pdata,&infostream) == -1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoSendFlower error");
		}
		break;
	case ROOM_CMD_VERIFY_USER_APP:     
		if(DoVerifyUser(pdata,&infostream) == -1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoVerifyUser error");
		}
		break;
	case ROOM_CMD_REMOVE_USER:     
		if(DoRemoveUser(pdata,&infostream) == -1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoRemoveUser error");
		}
		break;
	case ROOM_CMD_GIVE_VJ_A:     
		if(DoGiveVJA(pdata,&infostream) == -1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoGiveVJA error");
		}
		break;
	case ROOM_CMD_GIVE_VJ:     
		if(DoGiveVJ(pdata,&infostream) == -1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoGiveVJ error");
		}
		break;
	case ROOM_CMD_GIVE_OUER_S:     
		if(DoGiveOwnS(pdata,&infostream) == -1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoGiveOwnS error");
		}
		break;
	case ROOM_CMD_SET_ROOM_LOCK:     
		if(DoSetRoomLock(pdata) == -1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoSetRoomLock error");
		}
		break;
	case ROOM_CMD_SET_USER_ONLY:     
		if(DoSetRoomUserOnly(pdata) == -1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoSetRoomUserOnly error");
		}
		break;
	case ROOM_CMD_SET_USER_INOUT:     
		if(DoSetRoomInOut(pdata) == -1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoSetRoomInOut error");
		}
		break;
	case ROOM_CMD_SET_ROOM_NAME:     
		if(DoSetRoomName(pdata) == -1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoSetRoomName error");
		}
		break;
	case ROOM_CMD_SET_CHAT_PUBLIC:     
		if(DoSetChatPublic(pdata) == -1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoSetChatPublic error");
		}
		break;
	case ROOM_CMD_SET_ROOM_WELCOME:     
		if(DoSetRoomWelcome(pdata) == -1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoSetRoomWelcome error");
		}
		break;
	case ROOM_CMD_SET_ROOM_LOGO:     
		if(DoSetRoomLogo(pdata) == -1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoSetRoomLogo error");
		}
		break;
	case ROOM_CMD_ROOMAPPLYLIST_C2R2C:     
		if(DoReturnRoomApplyList(pdata,&infostream,rownum,colnum,pdata->cmd) == -1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoReturnRoomApplyList error");
		}
		break;		
	case ROOM_CMD_GETBLACKLIST:     
		if(DoReturnRoomBlackList(pdata,&infostream,rownum,colnum,pdata->cmd) == -1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoReturnRoomApplyList error");
		}
		break;
	case ROOM_CMD_USER_APP_JOINROOM:     
		if(DoRoomUserApp(pdata,&infostream) == -1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoRoomUserApp error");
		}
		break;	
	case ROOM_CMD_GETROOMMEMBERLIST:     
		if(DoReturnRoomMemberList(pdata,&infostream,rownum,colnum,pdata->cmd) == -1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoReturnRoomMemberList error");
		}
		break;
	//add by jinguanfu 2010/4/7 设置房间密码回应
	case ROOM_CMD_SET_ROOM_PWD:
		if(DoSetRoomPwd(pdata) == -1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoSetRoomPwd error");
		}
		break;
	case ROOM_CMD_SET_MIC_UPDOWN:
		if(DoSetRoomMicUpDown(pdata)==-1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoSetRoomMicUpDown error");
		}
		break;
	case ROOM_CMD_EXITMEMBER:
		if(DoExitRoomMember(pdata, &infostream)== -1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoExitRoomMember error");
		}
		break;
	case ROOM_CMD_GIVE_MEMBER:
		if(DoGiveMember(pdata, &infostream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoGiveMember error");
		}
		break;
	case HALL_CMD_UPDATEROOM:
		if(DoUpdateRoomInfo(pdata, rownum, colnum, & infostream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoGiveMember error");
		}
		break;
	case HALL_CMD_ROOMMANAGERCHG:
		if(DoUpdateRoomMemberlist(pdata,& infostream, rownum, colnum)==-1)
		//if(DoUpdateRoomManger(pdata,& infostream, rownum, colnum)==-1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoUpdateRoomMemberlist error");
			return -1;
		}
		break;
	case ROOM_CMD_SEND_FIREWORKS:
		if(DoSendFireworks(pdata, &infostream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoSendFireworks error");
		}
		break;
	case ROOM_CMD_RECV_FIREWORKS:
		if(DoRecvFireworks(pdata,rownum,colnum,&infostream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoRecvFireworks error");
		}
		break;
	case ROOM_CMD_SB_LUCKY:
		/*
		if(DoRunLotte(pdata,&infostream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoRunLotte error");
			return -1;
		}
		*/
		if(DoLuckNotify(pdata,&infostream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoLuckNotify error");
		}
		break;
	case ROOM_CMD_VIEW_INCOME:
		if(DoViweIncome(pdata,  &infostream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoViweIncome error");
		}
		break;
	case ROOM_CMD_GET_INCOME:
		if(DoGetIncome(pdata,  &infostream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoGetIncome error");
		}
		break;
	case ROOM_CMD_VIEW_INCOME_LOG:
		if(DoViweIncomeLog(pdata, rownum,colnum,&infostream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoGetIncome error");
		}
		break;	
	case ROOM_CMD_SCORE:
		if(DoScore(pdata,&infostream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoScore error");
		}
		break;
	case ROOM_CMD_UPDATE_MUSICINFO:
		if(DoUpdateMusicInfo(pdata,&infostream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoUpdateMusicInfo error");
		}
		break;
	case ROOM_CMD_SB_GIFTINFO_CHANGE:
		if(DoUpdateGiftInfo(pdata, &infostream,rownum,colnum)==-1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoUpdateGiftInfo error");
		}
		break;
	case ROOM_CMD_SB_RIGHTCONF_CHANGE:
		if(DoUpdateRightConf(pdata, &infostream,rownum,colnum)==-1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoUpdateRightConf error");
		}
		break;
	case ROOM_CMD_DB_LUCKCONF_CHANGE:
		if(DoUpdateLuckConf(pdata, &infostream,rownum,colnum)==-1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoUpdateLuckConf error");
		}
		break;	
	case INNER_CMD_GETROOMINFO:
		if(DoGetRoomInfo(pdata, &infostream,rownum,colnum)==-1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoGetRoomInfo error");
		}
		break;
	case INNER_CMD_GETMEMBERLIST:
		if(DoGetRoomMemberList(pdata, &infostream,rownum,colnum)==-1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoGetRoomMemberList error");
		}
		break;
	case INNER_CMD_GETBLACKLIST:
		if(DoGetRoomBlackList(pdata, &infostream,rownum,colnum)==-1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoGetRoomBlackList error");
		}
		break;
	case INNER_CMD_GETAPPLYLIST:
		if(DoGetRoomApplyList(pdata, &infostream,rownum,colnum)==-1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoGetRoomApplyList error");
		}
		break;
	case ROOM_CMD_SB_AVSERVER_CHANGE:
		if(DoGetRoomAVServer(pdata, &infostream,rownum,colnum)==-1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoGetRoomAVServer error");
		}
		break;
	case ROOM_CMD_SB_UPDATE_GM:
		if(DoUpdateGM(pdata, &infostream,rownum,colnum)==-1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoUpdateGM error");
		}
		break;
	case ROOM_CMD_INVITE_REPLY:
		if(DoReplyInvite(pdata, &infostream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoReplyInvite error");
		}
		break;
	case ROOM_CMD_GET_GIFTSEND:
		if(DoGetGiftSend(pdata, &infostream,rownum,colnum)==-1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoGetGiftSend error");
		}
		break;
	case ROOM_CMD_GET_GIFTRECV:
		if(DoGetGiftRecv(pdata, &infostream,rownum,colnum)==-1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoGetGiftRecv error");
		}
		break;
	case HALL_CMD_ROOMBLACKMEMCHG_L2R:
		if(DoUpdateRoomBlacklist(pdata,&infostream,rownum,colnum)==-1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoUpdateRoomBlacklist error");
		}
		break;
	case ROOM_CMD_DISABLE_IPADDR:
		if(DoDisableIP(pdata,&infostream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoDisableIP error");
		}
		break;
	case ROOM_CMD_DISABLE_MACADDR:
		if(DoDisableMAC(pdata,&infostream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoDisableIP error");
		}
		break;
	case HALL_CMD_UPDATE_ROOMCHAT_BLACKLIST_L2R:
		if(DoUpdateChatBlacklist(pdata,&infostream,rownum,colnum)==-1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoUpdateChatBlacklist error");
		}
		break;
	case HALL_CMD_UPDATE_ROOMGIFT_REFRESH_TIMES_L2R:
		if(DoUpdateGiftConfig(pdata,&infostream,rownum,colnum)==-1)
		{
			AC_ERROR("DbagentDataDecoder::OnPackage:DoUpdateGiftConfig error");
		}
		break;
	default:
		AC_ERROR("DbagentDataDecoder::OnPackage:cmd error[%d] outseq=%d",pdata->cmd,pdata->outseq);
			
 	}

	pdata->UnRegisterTimer();
 	 g_pNetProxy->ClearClientDBMap(seq);
	 g_pNetProxy->DestroyDBResult(pdata);

	return 0;
}
//modify by jinguanfu 2010/1/19
//int DbagentDataDecoder::DoUpdateBlacklist(DB_RESULT_DATA* pdata)
int DbagentDataDecoder::DoUpdateBlacklist(Dbresultdata* pdata,BinaryReadStream* infostream)
{
	AC_DEBUG("DbagentDataDecoder::DoUpdateBlacklist:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);

	if(pdata->roomid == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoUpdateBlacklist:roomid error");
		return -1;
	}

	if(pdata->bopidx == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoUpdateBlacklist:idx error");
		return -1;
	}

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(proominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoUpdateBlacklist:proominfo error");
		return -1;
	}

	//数据库返回值
	char infobuf[1024];
	size_t clen;
	
	if(!infostream->Read(infobuf,sizeof(infobuf),clen))
	{
		AC_ERROR("DbagentDataDecoder::DoSendFlower:read result error");
		return -1;
	}
	infobuf[clen]=0;
	
	int result=atoi(infobuf);
	if(result==DBRESULT_BLACKLIST_FALIED)
	{
		//发送操作者回应
		BinaryWriteStream* outstream1=StreamFactory::InstanceWriteStream();
		char type = 65;
		outstream1->Write(type);
		outstream1->Write(pdata->cmd);
		outstream1->Write(pdata->seq);
		outstream1->Write((int)ALREADY);
		outstream1->Write(pdata->bopidx);  
		outstream1->Write(pdata->badd);
		outstream1->Flush();
		RoomClient* pClient = g_pNetProxy->GetClient(pdata->number);
		if(pClient!=NULL)
		{
			if(g_pNetProxy->SendToSrv(pClient,*outstream1))
			{
				AC_ERROR("DbagentDataDecoder::DoUpdateBlacklist:SendToSrv error");
				pClient->Close();
			}	
		}
		else
		{
			AC_ERROR("DbagentDataDecoder::DoUpdateBlacklist:%d is closed ",pdata->number);
		}
	
		return -1;
	}

	if(pdata->badd==1)
	{
		map<int, int>::iterator itb = proominfo->blacklist.find(pdata->bopidx);
		if(itb == proominfo->blacklist.end())
		{
			proominfo->blacklist.insert(make_pair(pdata->bopidx, pdata->bopidx));
			//用户在房间内，则踢出房间
			map<int, RoomClient*>::iterator itu=proominfo->userlist.find(pdata->bopidx);
			if(itu!=proominfo->userlist.end())
			{
				RoomClient* pbClient = itu->second;
				pbClient->LeaveRoom();
			}

			map<int, RoomClient*>::iterator itvu=proominfo->vuserlist.find(pdata->bopidx);
			if(itvu!=proominfo->vuserlist.end())
			{
				RoomClient* pbClient = itvu->second;
				pbClient->VLeaveRoom();
				proominfo->vuserlist.erase(itvu);
			}
			
		}
	}
	else
	{
		map<int, int>::iterator itb = proominfo->blacklist.find(pdata->bopidx);
		if(itb != proominfo->blacklist.end())
			proominfo->blacklist.erase(itb);
		else
		{
			AC_ERROR("DbagentDataDecoder::DoUpdateBlacklist:black bopidx error");
			return -1;
		}
	}

	//发送操作者回应
	BinaryWriteStream* outstream1=StreamFactory::InstanceWriteStream();
	char type = 65;
	outstream1->Write(type);
	outstream1->Write(pdata->cmd);
	outstream1->Write(pdata->seq);
	outstream1->Write((int)SUCCESS);
	outstream1->Write(pdata->bopidx);  
	outstream1->Write(pdata->badd);
	outstream1->Flush();
	RoomClient* pClient = g_pNetProxy->GetClient(pdata->number);
	if(pClient!=NULL)
	{
		if(g_pNetProxy->SendToSrv(pClient,*outstream1))
		{
			AC_ERROR("DbagentDataDecoder::DoUpdateBlacklist:SendToSrv error");
			pClient->Close();
		}	
	}
	else
	{
		AC_ERROR("DbagentDataDecoder::DoUpdateBlacklist:%d is closed ",pdata->number);
	}

	//发送给房间内所有的人,有人被添加/删除入黑名单
	//char outbuf[256]64] = {0};
	BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
	//char type = 65;
	outstream2->Write(type);
	outstream2->Write((short)ROOM_CMD_SB_BLACKLIST_UPDATE);
	outstream2->Write((int)0);
	outstream2->Write(pdata->bopidx);  
	outstream2->Write(pdata->badd);
	outstream2->Flush();
	if(g_pNetProxy->BroadcastSBCmd(proominfo,*outstream2) == -1)
	{
		AC_ERROR("DbagentDataDecoder::DoUpdateBlacklist:g_pNetProxy->BroadcastSBCmd error");
	}

	return 0;
}

//modify by jinguanfu 2010/1/19
//int DbagentDataDecoder::DoUpdateMgr(DB_RESULT_DATA* pdata)
int DbagentDataDecoder::DoUpdateMgr(Dbresultdata* pdata)
{
    AC_DEBUG("DbagentDataDecoder::DoUpdateMgr:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);

    if(pdata->roomid == 0)
    {
        AC_ERROR("DbagentDataDecoder::DoUpdateMgr:roomid error");
        return -1;
    }

    if(pdata->bopidx == 0)
    {
        AC_ERROR("DbagentDataDecoder::DoUpdateMgr:idx error");
        return -1;
    }

    ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pdata->roomid);
    if(proominfo == NULL)
    {
        AC_ERROR("DbagentDataDecoder::DoUpdateMgr:proominfo error");
        return -1;
    }

    if(pdata->badd)
    {
    /*
        if(pdata->identity == USER_ID_A_MANAGER)
        {
            map<int, int>::iterator ita = proominfo->managerlistA.find(pdata->bopidx);
            if(ita == proominfo->managerlistA.end())
                proominfo->managerlistA.insert(make_pair(pdata->bopidx, pdata->bopidx));
        }
        if(pdata->identity  == USER_ID_B_MANAGER)
        {
            map<int, int>::iterator itb = proominfo->managerlistB.find(pdata->bopidx);
            if(itb == proominfo->managerlistB.end())
                proominfo->managerlistB.insert(make_pair(pdata->bopidx, pdata->bopidx));
        }
        */
        if(pdata->identity  == USER_ID_VJ)
        {
            map<int, int>::iterator itvj = proominfo->vjlist.find(pdata->bopidx);
            if(itvj == proominfo->vjlist.end())
                proominfo->vjlist.insert(make_pair(pdata->bopidx, pdata->bopidx));
        }
    }
    else
    {
    	/*
        if(pdata->identity == USER_ID_A_MANAGER)
        {
            map<int, int>::iterator it = proominfo->managerlistA.find(pdata->bopidx);
            if(it != proominfo->managerlistA.end())
                proominfo->managerlistA.erase(it);
            else
            {
                AC_ERROR("DbagentDataDecoder::DoUpdateMgr:mgra delete bopidx error");
                return -1;
            }
        }
        if(pdata->identity  == USER_ID_B_MANAGER)
        {
            map<int, int>::iterator it = proominfo->managerlistB.find(pdata->bopidx);
            if(it != proominfo->managerlistB.end())
                proominfo->managerlistB.erase(it);
            else
            {
                AC_ERROR("DbagentDataDecoder::DoUpdateMgr:mgrb delete bopidx error");
                return -1;
            }
        }
        */
        if(pdata->identity  == USER_ID_VJ)
        {
            map<int, int>::iterator it = proominfo->vjlist.find(pdata->bopidx);
            if(it != proominfo->vjlist.end())
                proominfo->vjlist.erase(it);
            else
            {
                AC_ERROR("DbagentDataDecoder::DoUpdateMgr:vj delete bopidx error");
                return -1;
            }
        }
    }

    //发送给房间内所有的人,有人被添加/删除入管理员
    int seq2 = 0;
    //char outbuf[256]64] = {0};
    BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
    char type = 65;
    outstream2->Write(type);
    outstream2->Write((short)ROOM_CMD_SB_MGRLIST_UPDATE);
    outstream2->Write(seq2);
    outstream2->Write(pdata->bopidx);  
    outstream2->Write(pdata->badd);
    outstream2->Write(pdata->identity);
    outstream2->Flush();
    if(g_pNetProxy->BroadcastSBCmd(proominfo,*outstream2) == -1)
    {
        AC_ERROR("DbagentDataDecoder::DoUpdateMgr:g_pNetProxy->BroadcastSBCmd error");
        return -1;
    }

    return 0;
}

//modify by jinguanfu 2010/1/19
//int DbagentDataDecoder::DoUpdateContent(DB_RESULT_DATA* pdata)
int DbagentDataDecoder::DoUpdateContent(Dbresultdata* pdata)
{
	AC_DEBUG("DbagentDataDecoder::DoUpdateContent:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);

	if(pdata->roomid == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoUpdateContent:roomid error");
		return -1;
	}

	if(pdata->bopidx == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoUpdateContent:idx error");
		return -1;
	}

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(proominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoUpdateContent:proominfo error");
		return -1;
	}

	AC_DEBUG("DbagentDataDecoder::DoUpdateContent:pdata->content len=%d",strlen(pdata->content));
	if(strlen(pdata->content)>=sizeof(proominfo->content))
	{
		strncpy(proominfo->content, pdata->content,sizeof(proominfo->content)-1);	//更新房间信息
		proominfo->content[sizeof(proominfo->content)-1]=0;
	}
	else
	{
		strncpy(proominfo->content, pdata->content,strlen(pdata->content));
		proominfo->content[strlen(pdata->content)]=0;
	}

	char  temp[64] = {0};
	
	BinaryWriteStream onestream(temp,sizeof(temp));
	char type = 65;
	onestream.Write(type);
	onestream.Write((short)pdata->cmd);
	onestream.Write(pdata->seq);
	onestream.Write((int)0);	
	onestream.Flush();	
	RoomClient* pClient = g_pNetProxy->GetClient(pdata->number);
	if(pClient!=NULL)
	{
		if(g_pNetProxy->SendToSrv(pClient, onestream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::DoSetRoomInOut:g_pNetProxy->SendToSrv error");
			pClient->Close();
		}
	}
	else 
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomInOut:%d is closed ",pdata->number);
	}


    
    //发送给房间内所有的人,房主更新房间公告
    int seq2 = 0;
    //char outbuf[256]64] = {0};
    BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
   //char type = 65;
    outstream2->Write(type);
    outstream2->Write((short)ROOM_CMD_SB_CONTENT_UPDATE);
    outstream2->Write(seq2);
    outstream2->Write((int)0);	//ret
    //outstream2->Write(proominfo->content, strlen(proominfo->content));  
    outstream2->Flush();
    if(g_pNetProxy->BroadcastSBCmd(proominfo,*outstream2) == -1)
    {
        AC_ERROR("DbagentDataDecoder::DoUpdateContent:g_pNetProxy->BroadcastSBCmd error");
        return -1;
    }

    return 0;
}

//modify by jinguanfu 2010/1/19
//int DbagentDataDecoder::DoSendFlower(DB_RESULT_DATA* pdata)
int DbagentDataDecoder::DoSendFlower(Dbresultdata* pdata,BinaryReadStream* infostream)
{
	AC_DEBUG("DbagentDataDecoder::DoSendFlower::opidx=%d,boidx=%d,roomid=%d,outseq=%d,cate_id=%d,number=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq,pdata->cate_idx,pdata->number);

	if(pdata->roomid == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoSendFlower:roomid error");
		return -1;
	}
	/*
	if(pdata->opidx == 0 || pdata->bopidx == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoSendFlower:idx error");
		return -1;
	}
	*/
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(proominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoSendFlower:proominfo error");
		return 0;
	}

	//数据库返回值
	char infobuf[1024];
	size_t clen;
	
	if(!infostream->Read(infobuf,sizeof(infobuf),clen))
	{
		AC_ERROR("DbagentDataDecoder::DoSendFlower:read result error");
		return -1;
	}
	infobuf[clen]=0;

	char* saveptr=NULL;
	char* token=strtok_r(infobuf,"|",&saveptr);
	if(token==NULL||saveptr==NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoSendFlower:parse result error");
		return 0;
	}
	int result=atoi(token);
	int s_gold=0;		//发送者金币 更新
	int s_allgold=0;	//发送者金币 余额
	int s_silver=0;		//发送者银币 更新
	int s_allsilver=0;	//发送者银币  余额
	int r_gold=0;		//接收者金币 更新
	int r_allgold=0;	//接收者金币  余额
	int r_silver=0;		//接收者银币 更新
	int r_allsilver=0;	//接收者银币  余额
	int speakflag=0;  //发送广播标志位
	//	1--交易金额为空
      //	0--成功,后面跟"金币 更新|银币 更新|广播类型"
      //  -1--金额不足
      //	2--无此交易类型
	if(result==DBRESULT_GIFT_SUCCESS)
	{
		token=strtok_r(NULL,"|",&saveptr);
		if(token==NULL||saveptr==NULL)
		{
			AC_ERROR("DbagentDataDecoder::DoSendFlower:parse s_gold error");
			return 0;
		}
		s_gold=atoi(token);		//送礼者金币余额
		token=strtok_r(NULL,"|",&saveptr);
		if(token==NULL||saveptr==NULL)
		{
			AC_ERROR("DbagentDataDecoder::DoSendFlower:parse s_silver error");
			return 0;
		}
		s_silver=atoi(token);		//送礼者银币余额

		token=strtok_r(NULL,"|",&saveptr);
		if(token==NULL||saveptr==NULL)
		{
			AC_ERROR("DbagentDataDecoder::DoSendFlower:parse s_gold error");
			return 0;
		}
		r_gold=atoi(token);		//收礼者金币余额
		token=strtok_r(NULL,"|",&saveptr);
		if(token==NULL||saveptr==NULL)
		{
			AC_ERROR("DbagentDataDecoder::DoSendFlower:parse s_silver error");
			return 0;
		}
		r_silver=atoi(token);		//收礼者银币余额
		
		
		token=strtok_r(NULL,"|",&saveptr);
		if(token==NULL)
		{
			AC_ERROR("DbagentDataDecoder::DoSendFlower:parse speakflag error");
			return 0;
		}
		speakflag=atoi(token);


		map<int,GIFT_INFO>::iterator itGiftInfo=g_pNetProxy->m_GiftInfo.find(pdata->cate_idx);
		if(itGiftInfo==g_pNetProxy->m_GiftInfo.end())
		{
			AC_ERROR("DbagentDataDecoder::DoSendFlower:Gift=%d not exist ",pdata->cate_idx);
			return 0;
		}
		GIFT_INFO* pGiftinfo=(GIFT_INFO*)&(itGiftInfo->second);
		char type=65;

		map<int,RoomClient*>::iterator itR=proominfo->userlist.find(pdata->opidx);
		if(itR!=proominfo->userlist.end())
		{
			//更新发送的金币数和银币数
			RoomClient* pClient=itR->second;
			s_allgold=pClient->gold+=s_gold;
			s_allsilver=pClient->silver+=s_silver;
			BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
			outstream->Write(type);
			outstream->Write((short)pdata->cmd);
			outstream->Write(pdata->seq);
			outstream->Write(pdata->cate_idx);		//礼物ID
			outstream->Write(pdata->number);		//礼物数量
			outstream->Write(pdata->bopidx);		//收礼者idx
			outstream->Write(result);				//数据库操作结果
			outstream->Write(s_allgold);
			outstream->Write(s_allsilver);
			outstream->Flush();
			
			if(g_pNetProxy->SendToSrv(pClient, *outstream))
			{
				AC_ERROR("DbagentDataDecoder::DoSendFlower:SendToSrv error");
			}

		}

		RoomClient* pClient=NULL;
		//更新接收者的金币数和银币数add by jinguanfu 2010/5/19
		map<int,RoomClient*>::iterator itRR=proominfo->userlist.find(pdata->bopidx);
		map<int,RoomClient*>::iterator itRV=proominfo->vuserlist.find(pdata->bopidx);
		if(itRR!=proominfo->userlist.end())
		{
			pClient=itRR->second;

			r_allgold=pClient->gold+=r_gold;
			r_allsilver=pClient->silver+=r_silver;		
			
			BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
			outstream2->Write(type);
			outstream2->Write((short)ROOM_CMD_UPDATEMONEY);
			outstream2->Write(0);
			outstream2->Write(r_allgold);
			outstream2->Write(r_allsilver);
			outstream2->Flush();
			if(g_pNetProxy->SendToSrv(pClient, *outstream2))
			{
				AC_ERROR("DbagentDataDecoder::DoSendFlower:SendToSrv error");
			}
		}
		else if(itRV!=proominfo->vuserlist.end())
		{
			pClient=itRV->second;

			r_allgold=pClient->gold+=r_gold;
			r_allsilver=pClient->silver+=r_silver;		

		}
		//在服务器端保存送花状态 
		//送花数量少于100不放入列表
		//add by jinguanfu 2010/11/26 <begin>
		if(pdata->number > ((SENDGIFT_SETTM) * 1000 * pGiftinfo->number / pGiftinfo->interval))  //modify by lihongwu 2011/10/11
		{
			int giftseq=g_pNetProxy->GetCounter()->Get();
			SEND_GIFT_INFO* sendgift=g_pNetProxy->CreateSendGiftInfo();
			sendgift->s_idx=pdata->opidx;	
			sendgift->r_idx=pdata->bopidx;
			sendgift->cate_idx=pdata->cate_idx;
			sendgift->number=pdata->number;
			sendgift->havesend=0;
			sendgift->seq=giftseq;
			sendgift->price=pGiftinfo->price;
			//add by lihongwu 2011/10/11
			sendgift->priceconf=pGiftinfo->priceconf;        
			sendgift->numpertime=pGiftinfo->number;
			sendgift->interval=pGiftinfo->interval;

			//加入到送礼列表中
			proominfo->sendgiftlist.push_back(sendgift);
		}
		//add by jinguanfu 2010/11/26 <end>

		//发送给房间内所有的人,有用户给另外的用户送花了
		//char outbuf[256]64] = {0};
		AC_DEBUG("DbagentDataDecoder::DoSendFlower::pGiftinfo->number=%d,pGiftinfo->interval=%d ",pGiftinfo->number,pGiftinfo->interval);
		BinaryWriteStream* giftstream=StreamFactory::InstanceWriteStream();
		giftstream->Write(type);
		giftstream->Write((short)ROOM_CMD_SB_SEND_GIFT);
		giftstream->Write((int)0);
		giftstream->Write(pdata->opidx);		//送礼者
		giftstream->Write(pdata->bopidx);		//接收礼物者
		giftstream->Write(pdata->cate_idx);		//礼物id
		giftstream->Write(pdata->number);		//礼物总数
		giftstream->Write(pdata->number);		//礼物剩余数
		giftstream->Write(pGiftinfo->interval);		//礼物刷新间隔时间
		giftstream->Write(pGiftinfo->number);		//礼物刷新个数
		
		giftstream->Flush();
		/*
		if(g_pNetProxy->BroadcastSBCmd(proominfo,*giftstream) == -1)
		{
			AC_ERROR("DbagentDataDecoder::SendFlower:g_pNetProxy->BroadcastSBCmd error");
		}
		*/
		char outbuf[50];                     
		int outbuflen = sizeof(outbuf);  
        if(!StreamCompress(giftstream->GetData(),(int)giftstream->GetSize(),outbuf,outbuflen))
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
				//if(StreamEncrypt(giftstream->GetData(),(int)giftstream->GetSize(),outbuf,outbuflen,psend->m_sessionkey,1))
                {
					if(psend->AddBuf(outbuf,outbuflen) ==-1)
					{
						AC_ERROR("psend->AddBuf Error");
                        //psend->ErrorClose();
					}
                }
			}
		}

		//通知大厅发送喇叭消息
		BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
		outstream2->Write((short)HALL_CMD_FLOWER_ALL);
		outstream2->Write((int)0);
		outstream2->Write((char)ROOM_TYPE_ROOM);	//ADD BY JINGUANFU 2010/7/9
		outstream2->Write(pdata->opidx);		//送礼者
		outstream2->Write(pdata->bopidx);		//接收礼物者
		outstream2->Write(pdata->cate_idx);	//礼物id
		outstream2->Write(pdata->number);		//礼物总数
		outstream2->Write(s_allgold);			//送礼者金币余额
		outstream2->Write(s_allsilver);			//送礼者银币余额
		outstream2->Write(r_allgold);			//收礼者金币余额
		outstream2->Write(r_allsilver);			//收礼者银币余额
		outstream2->Write((char)speakflag);		// 1--不发广播
											// 2--大厅广播
											// 3--全区广播
		outstream2->Flush();
		
		AC_DEBUG("DbagentDataDecoder::SendFlower BroadToSver!");
		HallSvrClient *pHallSvr = g_pNetProxy->GetHallSvr();
		if(pHallSvr!=NULL)
			pHallSvr->AddBuf(outstream2->GetData(), outstream2->GetSize());	
		else
			AC_ERROR("DbagentDataDecoder::DoSendFlower:pHallSvr is NULL ");

		
		//幸运礼物抽奖add by jinguanfu 2010/9/6
		if(pGiftinfo->type==GIFT_TYPE_LUCK)
		{
			DoRunLottery(pdata);
		}
		else if(pGiftinfo->type==GIFT_TYPE_SEAL)	//印章
		{
			
				BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
				char type=65;
				outstream->Write(type);
				outstream->Write((short)ROOM_CMD_SB_GIFT_VALID);
				outstream->Write(0);					//seq
				outstream->Write(pdata->bopidx);		//收礼者idx
				outstream->Write(pdata->cate_idx);		//礼物ID
				outstream->Flush();

				g_pNetProxy->BroadcastSBCmd(proominfo, *outstream);


				BinaryWriteStream* lobbystream=StreamFactory::InstanceWriteStream();
				lobbystream->Write((short)HALL_CMD_ADD_SEEL_R2L);
				lobbystream->Write(0);
				lobbystream->Write(pdata->bopidx);
				lobbystream->Write(pGiftinfo->GiftID);
				lobbystream->Write(pGiftinfo->vtime);
				lobbystream->Flush();

				HallSvrClient *pHallSvr = g_pNetProxy->GetHallSvr();
				if(pHallSvr!=NULL)
					pHallSvr->AddBuf(lobbystream->GetData(), lobbystream->GetSize());
				else
					AC_ERROR("DbagentDataDecoder::DoSendFlower:pHallSvr is NULL");
				

		}
	}
	else
	{
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		char type=65;
		outstream->Write(type);
		outstream->Write((short)pdata->cmd);
		outstream->Write(pdata->seq);
		//add by jinguanfu 2010/4/30 送礼失败时被踢出房间BUG<begin>
		outstream->Write(pdata->cate_idx);		//礼物ID
		outstream->Write(pdata->number);		//礼物数量
		outstream->Write(pdata->bopidx);		//收礼者idx
		//add by jinguanfu 2010/4/30 送礼失败时被踢出房间BUG<end>
		outstream->Write(result);
		outstream->Flush();

		map<int,RoomClient*>::iterator itR=proominfo->userlist.find(pdata->opidx);
		if(itR != proominfo->userlist.end())
		{
			RoomClient* pClient=itR->second;
			if(pClient != NULL)
			{
				if(g_pNetProxy->SendToSrv(pClient, *outstream))
				{
					AC_ERROR("DbagentDataDecoder::DoSendFlower:SendToSrv error");
				}
			}
			else
				 AC_ERROR("DbagentDataDecoder::DoSendFlower:pClient is NULL");
		}
		else
			AC_ERROR("DbagentDataDecoder::DoSendFlower:idx=%d offline",pdata->opidx);
		
		return 0;
	}
	


	return 0;
}

//modify by jinguanfu 2010/1/19
//int DbagentDataDecoder::DoVerifyUser(DB_RESULT_DATA* pdata)
int DbagentDataDecoder::DoVerifyUser(Dbresultdata* pdata,BinaryReadStream* infostream)
{
	AC_DEBUG("DbagentDataDecoder::DoVerifyUser:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);

	if(pdata->roomid == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoVerifyUser:roomid error");
		return -1;
	}

	if(pdata->opidx == 0 || pdata->bopidx == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoVerifyUser:idx error");
		return -1;
	}

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(proominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoVerifyUser:proominfo error");
		return -1;
	}
	char infobuf[1024];
	size_t clen;
	if(!infostream->Read(infobuf,sizeof(infobuf),clen))
	{
		AC_ERROR("DbagentDataDecoder::DoVerifyUser:read result error");
		return -1;
	}
	infobuf[clen]=0;
	
	int result=atoi(infobuf);//0--成功, 1--已是会员, 2--超过会员最大限制
	//对操作者的回应
	char  temp[64] = {0};
	BinaryWriteStream onestream(temp,sizeof(temp));
	char type = 65;
	onestream.Write(type);
	onestream.Write((short)pdata->cmd);
	onestream.Write(pdata->seq);
	if(result==DBRESULT_VERIFY_ALREADY)	//已经是会员
		onestream.Write((int)ALREADY);				
	else if(result ==DBRESULT_VERIFY_SUCCESS)
		onestream.Write((int)SUCCESS);	
	else if(result == DBRESULT_VERIFY_FULL)
		onestream.Write((int)LISTFULL);
	onestream.Write(pdata->bopidx);
	onestream.Flush();	
	RoomClient* pClient = g_pNetProxy->GetClient(pdata->number);
	if(pClient!=NULL)
	{
		if(g_pNetProxy->SendToSrv(pClient, onestream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::DoVerifyUser:g_pNetProxy->SendToSrv error");
		}
	}
	else
	{
		AC_ERROR("DbagentDataDecoder::DoVerifyUser:pClient is NULL");
	}

	//申请处理后，从审核列表中删除
	map<int,ROOM_APPLY>::iterator itAPP=proominfo->userlistAPP.find(pdata->bopidx);
	if(itAPP!=proominfo->userlistAPP.end())
		proominfo->userlistAPP.erase(itAPP);

	//审核会员成功，更新会员列表及向大厅发送通知
	if(result==DBRESULT_VERIFY_SUCCESS)
	{
		if(pdata->badd)	//审核通知，加入会员列表
		{
			proominfo->userlistVIP.insert(make_pair(pdata->bopidx, pdata->bopidx));

			//被操作者在房间，则身份为房间会员
			map<int,RoomClient*>::iterator itu=proominfo->userlist.find(pdata->bopidx);
			if(itu!=proominfo->userlist.end())
			{
				RoomClient* pbClient = (*itu).second;
				if(pbClient)
					pbClient->m_identity = USER_ID_VIP;
			}
			//通知房间内在线用户
			char context2[64] = {0};
			BinaryWriteStream outstream2(context2,sizeof(context2));
			char type = 65;
			outstream2.Write(type);
			outstream2.Write((short)ROOM_CMD_SB_VERIFY_USER_APP);
			outstream2.Write(0);
			outstream2.Write((int)pdata->opidx);	//操作者idx
			outstream2.Write((int)pdata->bopidx);	//申请者idx	
			outstream2.Flush();	
			g_pNetProxy->BroadcastSBCmd(proominfo,outstream2) ;
			
		}
		char context[64] = {0};
		BinaryWriteStream outstream(context,sizeof(context));
		outstream.Write((short)HALL_CMD_APPLYMEMBER_R2L);
		outstream.Write((int)0);
		outstream.Write((char)ROOM_TYPE_ROOM);	//ADD BY JINGUANFU 2010/7/9
		outstream.Write(pdata->roomid);	//房间id
		outstream.Write(pdata->opidx);		//操作者idx
		outstream.Write(pdata->bopidx);	//申请者idx
		int ref=pdata->badd;
		outstream.Write(ref);				//审核结果0--拒绝1--通过
		outstream.Flush();

		HallSvrClient *pHallSvr = g_pNetProxy->GetHallSvr();
		if(pHallSvr!=NULL)
			pHallSvr->AddBuf(outstream.GetData(), outstream.GetSize());
		else
			AC_ERROR("DbagentDataDecoder::DoVerifyUser:pHallSvr is NULL");

	
	}

	return 0;
}

//modify by jinguanfu 2010/1/19 
//int DbagentDataDecoder::DoRemoveUser(DB_RESULT_DATA* pdata)
int DbagentDataDecoder::DoRemoveUser(Dbresultdata* pdata,BinaryReadStream* infostream)
{
	AC_DEBUG("DbagentDataDecoder::DoRemoveUser:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);

	if(pdata->roomid == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoRemoveUser:roomid error");
		return -1;
	}

	if(pdata->opidx == 0 || pdata->bopidx == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoRemoveUser:idx error");
		return -1;
	}

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(proominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoRemoveUser:proominfo error");
		return -1;
	}
	char infobuf[1024];
	size_t clen;
	if(!infostream->Read(infobuf,sizeof(infobuf),clen))
	{
		AC_ERROR("DbagentDataDecoder::DoRoomUserApp:read result error");
		return -1;
	}
	infobuf[clen]=0;
	
	int result=atoi(infobuf);
	
	//回应操作者
	char context[64] = {0};
	BinaryWriteStream outstream(context,sizeof(context));
	char type = 65;
	outstream.Write(type);
	outstream.Write((short)pdata->cmd);
	outstream.Write(pdata->seq);
	outstream.Write(result);				// 1--失败(非房间会员)， 0--成功
	outstream.Write(pdata->bopidx);	//被操作者idx
	outstream.Flush();	
		
	RoomClient* pClient = g_pNetProxy->GetClient(pdata->number);
	if(pClient!=NULL)
	{
		if(g_pNetProxy->SendToSrv(pClient, outstream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::DoRemoveUser:SendToSrv error");
		}
	}
	else
	{
		AC_ERROR("DbagentDataDecoder::DoRemoveUser:pClient is NULL ");
	}
	
	//删除成功处理
	if(result == DBRESULT_MEMBER_SUCCESS)
	{
		//更新房间信息
		map<int,int>::iterator itVIP= proominfo->userlistVIP.find(pdata->bopidx);
		map<int,int>::iterator itVJ= proominfo->vjlist.find(pdata->bopidx);
		map<int,int>::iterator itVJ_A= proominfo->vjlist_a.find(pdata->bopidx);
		map<int,ROOM_MANAGER>::iterator itM = proominfo->managerlist.find(pdata->bopidx);
		map<int,ROOM_MANAGER>::iterator itMO=proominfo->managerlist_online.find(pdata->bopidx);
		//从会员列表中删除
		if(itVIP!=proominfo->userlistVIP.end())
		{
			proominfo->userlistVIP.erase(itVIP);
		}
		//如果是管理员，从管理员中删除
		 if(itVJ!=proominfo->vjlist.end())
		{
			proominfo->vjlist.erase(itVJ);
		}
		else if(itVJ_A!=proominfo->vjlist_a.end())
		{
			proominfo->vjlist_a.erase(itVJ_A);
		}
		else if(pdata->bopidx==(int)proominfo->secondownidx)
		{
			proominfo->secondownidx=0;
		}
		else if(pdata->bopidx==(int)proominfo->secondownidx2)
		{
			proominfo->secondownidx2=0;
		}

		if(itM!=proominfo->managerlist.end())
		{
			proominfo->managerlist.erase(itM);
		}
		
		if(itMO!=proominfo->managerlist_online.end())
		{
			proominfo->managerlist_online.erase(itMO);
		}
		
		//删除会员后，在线的被删除者身份为普通用户
		map<int,RoomClient*>::iterator itu=proominfo->userlist.find(pdata->bopidx);
		if(itu!=proominfo->userlist.end())
		{
			RoomClient* pbClient = (*itu).second;
			if(pbClient)
				pbClient->m_identity = USER_ID_NONE;
		}
		
				
		//通知大厅删除会员，会员通知由大厅去发
		char context[64] = {0};
		BinaryWriteStream outstream(context,sizeof(context));

		outstream.Write((short)HALL_CMD_DELMEMBER_R2L);
		outstream.Write((int)0);
		outstream.Write((char)ROOM_TYPE_ROOM);	//ADD BY JINGUANFU 2010/7/9
		outstream.Write(pdata->roomid);
		outstream.Write(pdata->opidx);		//操作者idx
		outstream.Write(pdata->bopidx);	//被删除者idx
		outstream.Flush();

		HallSvrClient *pHallSvr = g_pNetProxy->GetHallSvr();
		if(pHallSvr!=NULL)
			pHallSvr->AddBuf(outstream.GetData(), outstream.GetSize());
		else
			AC_ERROR("DbagentDataDecoder::DoRemoveUser:pHallSvr is NULL");

		//通知房间内在线人员
		outstream.Clear();
		outstream.Write(type);
		outstream.Write((short)ROOM_CMD_SB_REMOVE_USER);
		outstream.Write(0);
		outstream.Write(pdata->opidx);		//操作者idx
		outstream.Write(pdata->bopidx);	//被删除者idx	
		outstream.Flush();	
		g_pNetProxy->BroadcastSBCmd(proominfo,outstream);
	
		
	}
	return 0;
}

//modify by jinguanfu 2010/1/19
//int DbagentDataDecoder::DoGiveVJA(DB_RESULT_DATA* pdata)
int DbagentDataDecoder::DoGiveVJA(Dbresultdata* pdata,BinaryReadStream* infostream)
{
	AC_DEBUG("DbagentDataDecoder::DoGiveVJA:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);

	if(pdata->roomid == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoGiveVJA:roomid error");
		return -1;
	}

	if(pdata->opidx == 0 || pdata->bopidx == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoGiveVJA:idx error");
		return -1;
	}

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(proominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoGiveVJA:proominfo error");
		return -1;
	}
	
	char infobuf[1024];
	size_t clen;
	if(!infostream->Read(infobuf,sizeof(infobuf),clen))
	{
		AC_ERROR("DbagentDataDecoder::DoGiveVJA:read result error");
		return -1;
	}
	infobuf[clen]=0;
	
	int result=atoi(infobuf);
	
	if(result==DBRESULT_MEMBER_SUCCESS)
	{
		//添加到房间助理主持列表
		proominfo->vjlist_a.insert(make_pair(pdata->bopidx,pdata->bopidx));

		//如果原来有其他权限，则从其他权限表中删除
		map<int,int>::iterator itVJ = proominfo->vjlist.find(pdata->bopidx);
		if(pdata->bopidx==(int)proominfo->secondownidx)
			proominfo->secondownidx=0;
		else if(pdata->bopidx==(int)proominfo->secondownidx2)
			proominfo->secondownidx2=0;
		else if(itVJ!=proominfo->vjlist.end())
			proominfo->vjlist.erase(itVJ);

		//被操作者身份为助理主持
		map<int,RoomClient*>::iterator itu=proominfo->userlist.find(pdata->bopidx);
		if(itu!=proominfo->userlist.end())
		{
			RoomClient* pbClient = (*itu).second;
			if(pbClient)
				pbClient->m_identity = USER_ID_VJ_A;
		}

		//add by jinguanfu 2010/6/17 <begin>
		//不在管理员列表，则加入到管理员列表中
		map<int,ROOM_MANAGER>::iterator itM=proominfo->managerlist.find(pdata->bopidx);
		if(itM==proominfo->managerlist.end())
		{
			ROOM_MANAGER manger;
			manger.m_idx=pdata->bopidx;
			manger.m_identity=USER_ID_VJ_A;
			proominfo->managerlist.insert(make_pair(pdata->bopidx,manger));
		}else
		{
			ROOM_MANAGER* pManger=(ROOM_MANAGER*)(&(itM->second));
			pManger->m_identity=USER_ID_VJ_A;
			
		}
		
		map<int,ROOM_MANAGER>::iterator itMO=proominfo->managerlist_online.find(pdata->bopidx);
		if(itMO==proominfo->managerlist_online.end())
		{
			ROOM_MANAGER manger;
			manger.m_idx=pdata->bopidx;
			manger.m_identity=USER_ID_VJ_A;
			proominfo->managerlist_online.insert(make_pair(pdata->bopidx,manger));
		}else
		{
			ROOM_MANAGER* pManger=(ROOM_MANAGER*)(&(itMO->second));
			pManger->m_identity=USER_ID_VJ_A;
			
		}
		//add by jinguanfu 2010/6/17 <end>
		char type = 65;
		//回复操作者
		char temp[64] ={0};
		BinaryWriteStream outstream1(temp,sizeof(temp));
		outstream1.Write(type);
		outstream1.Write((short)pdata->cmd);
		outstream1.Write(pdata->seq);	
		outstream1.Write(result);	//ret 0--成功-1--失败(人数已满)
		outstream1.Write(pdata->bopidx);
		outstream1.Flush();	
		RoomClient* pClient = g_pNetProxy->GetClient(pdata->number);
		if(pClient!=NULL)
		{
			if(g_pNetProxy->SendToSrv(pClient, outstream1)==-1)
			{
				AC_ERROR("DbagentDataDecoder::DoGiveVJA:SendToSrv error");
			}
		}
		else
		{
			AC_ERROR("DbagentDataDecoder::DoGiveVJA:pClient is NULL ");
		}
		
		//设置成功后房间广播
		char context[64] = {0};
		BinaryWriteStream outstream(context,sizeof(context));
		int seq = 0;
		outstream.Write(type);
		outstream.Write((short)ROOM_CMD_SB_GIVE_VJ_A);
		outstream.Write(seq);
		outstream.Write(pdata->opidx);		//操作者
		outstream.Write(pdata->bopidx);		//被操作者
		char level=USER_ID_VJ_A;		//权限
		outstream.Write(level);
		outstream.Flush();	
		if(g_pNetProxy->BroadcastSBCmd(proominfo,outstream) == -1)
		{
			AC_ERROR("DbagentDataDecoder::DoGiveVJA:g_pNetProxy->BroadcastSBCmd error");
			return -1;
		}
		

		//add by jinguanfu 2010/6/5 权限更新通知
		//发送大厅通知
		BinaryWriteStream* lobbystream=StreamFactory::InstanceWriteStream();	
		lobbystream->Write((short)HALL_CMD_CHANGERIGHT_R2L);
		lobbystream->Write(0);		
		lobbystream->Write((char)ROOM_TYPE_ROOM);	//ADD BY JINGUANFU 2010/7/9
		lobbystream->Write(pdata->opidx);		//操作者
		lobbystream->Write(pdata->bopidx);		//操作者
		lobbystream->Write(level);
		lobbystream->Flush();

		HallSvrClient *pHallSvr = g_pNetProxy->GetHallSvr();
		if(pHallSvr!=NULL)
			pHallSvr->AddBuf(lobbystream->GetData(), lobbystream->GetSize());
		else
			AC_ERROR("DbagentDataDecoder::DoGiveVJA:pHallSvr is NULL");
		

		
	}
	else
	{
		char type = 65;
		//回复操作者
		char temp[64] ={0};
		BinaryWriteStream outstream1(temp,sizeof(temp));
		outstream1.Write(type);
		outstream1.Write((short)pdata->cmd);
		outstream1.Write(pdata->seq);	
		outstream1.Write((int)DBRESULT_MEMBER_FULL);	//ret 0--成功-1--失败(人数已满)
		outstream1.Write(pdata->bopidx);
		outstream1.Flush();	
		RoomClient* pClient = g_pNetProxy->GetClient(pdata->number);
		if(pClient!=NULL)
		{
			if(g_pNetProxy->SendToSrv(pClient, outstream1)==-1)
			{
				AC_ERROR("DbagentDataDecoder::DoGiveVJA:SendToSrv error");
			}
		}
		else
		{
			AC_ERROR("DbagentDataDecoder::DoGiveVJA:pClient is NULL");
		}
	}
	return 0;
}

//modify by jinguanfu 2010/1/19
//int DbagentDataDecoder::DoGiveVJ(DB_RESULT_DATA* pdata)
int DbagentDataDecoder::DoGiveVJ(Dbresultdata* pdata,BinaryReadStream* infostream)
{
	AC_DEBUG("DbagentDataDecoder::DoGiveVJ:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);

	if(pdata->roomid == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoGiveVJ:roomid error");
		return -1;
	}

	if(pdata->opidx == 0 || pdata->bopidx == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoGiveVJ:idx error");
		return -1;
	}

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(proominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoGiveVJ:proominfo error");
		return -1;
	}

	char infobuf[1024];
	size_t clen;
	if(!infostream->Read(infobuf,sizeof(infobuf),clen))
	{
		AC_ERROR("DbagentDataDecoder::DoGiveVJ:read result error");
		return -1;
	}
	infobuf[clen]=0;
	
	int result=atoi(infobuf);
	
	if(result==DBRESULT_MEMBER_SUCCESS)
	{
		//添加到房间主持人列表
		proominfo->vjlist.insert(make_pair(pdata->bopidx,pdata->bopidx));

		//如果原来有其他权限，则从其他权限表中删除
		map<int,int>::iterator itVJA = proominfo->vjlist_a.find(pdata->bopidx);
		if(pdata->bopidx==(int)proominfo->secondownidx)
			proominfo->secondownidx=0;
		else if(pdata->bopidx==(int)proominfo->secondownidx2)
			proominfo->secondownidx2=0;
		else if(itVJA!=proominfo->vjlist_a.end())
			proominfo->vjlist_a.erase(itVJA);

		//被操作者身份为主持人
		map<int,RoomClient*>::iterator itu=proominfo->userlist.find(pdata->bopidx);
		if(itu!=proominfo->userlist.end())
		{
			RoomClient* pbClient = (*itu).second;
			if(pbClient)
				pbClient->m_identity = USER_ID_VJ;
		}

		//add by jinguanfu 2010/6/17 <begin>
		//不在管理员列表，则加入到管理员列表中
		map<int,ROOM_MANAGER>::iterator itM=proominfo->managerlist.find(pdata->bopidx);
		if(itM==proominfo->managerlist.end())
		{
			ROOM_MANAGER manger;
			manger.m_idx=pdata->bopidx;
			manger.m_identity=USER_ID_VJ;
			proominfo->managerlist.insert(make_pair(pdata->bopidx,manger));
		}else
		{
			ROOM_MANAGER* pManger=(ROOM_MANAGER*)(&(itM->second));
			pManger->m_identity=USER_ID_VJ;
		}

		map<int,ROOM_MANAGER>::iterator itMO=proominfo->managerlist_online.find(pdata->bopidx);
		if(itMO==proominfo->managerlist_online.end())
		{
			ROOM_MANAGER manger;
			manger.m_idx=pdata->bopidx;
			manger.m_identity=USER_ID_VJ_A;
			proominfo->managerlist_online.insert(make_pair(pdata->bopidx,manger));
		}else
		{
			ROOM_MANAGER* pManger=(ROOM_MANAGER*)(&(itMO->second));
			pManger->m_identity=USER_ID_VJ_A;
			
		}
		//add by jinguanfu 2010/6/17 <end>
		char type = 65;
		//回复操作者
		char temp[64] ={0};
		BinaryWriteStream outstream1(temp,sizeof(temp));
		outstream1.Write(type);
		outstream1.Write((short)pdata->cmd);
		outstream1.Write(pdata->seq);	
		outstream1.Write(result);	//ret 0--成功-1--失败(人数已满)
		outstream1.Write(pdata->bopidx);
		outstream1.Flush();	
		RoomClient* pClient = g_pNetProxy->GetClient(pdata->number);
		if(pClient!=NULL)
		{
			if(g_pNetProxy->SendToSrv(pClient, outstream1)==-1)
			{
				AC_ERROR("DbagentDataDecoder::DoGiveVJ:SendToSrv error");
			}
		}else
		{
			AC_ERROR("DbagentDataDecoder::DoGiveVJ:pClient is NULL ");
		}
		char context[64] = {0};
		BinaryWriteStream outstream(context,sizeof(context));
		int seq = 0;
		outstream.Write(type);
		outstream.Write((short)ROOM_CMD_SB_GIVE_VJ);
		outstream.Write(seq);
		outstream.Write(pdata->opidx);		//操作者
		outstream.Write(pdata->bopidx);	//被操作者
		char level=USER_ID_VJ;			//权限
		outstream.Write(level);
		outstream.Flush();	
		if(g_pNetProxy->BroadcastSBCmd(proominfo,outstream) == -1)
		{
			AC_ERROR("DbagentDataDecoder::DoGiveVJ:g_pNetProxy->BroadcastSBCmd error");
			return -1;
		}

		//add by jinguanfu 2010/6/5 权限更新通知
		//发送大厅通知
		BinaryWriteStream* lobbystream=StreamFactory::InstanceWriteStream();	
		lobbystream->Write((short)HALL_CMD_CHANGERIGHT_R2L);
		lobbystream->Write(0);		
		lobbystream->Write((char)ROOM_TYPE_ROOM);	//ADD BY JINGUANFU 2010/7/9
		lobbystream->Write(pdata->opidx);		//操作者
		lobbystream->Write(pdata->bopidx);		//操作者
		lobbystream->Write(level);
		lobbystream->Flush();

		HallSvrClient *pHallSvr = g_pNetProxy->GetHallSvr();
		if(pHallSvr!=NULL)
			pHallSvr->AddBuf(lobbystream->GetData(), lobbystream->GetSize());
		else
			AC_ERROR("DbagentDataDecoder::DoGiveVJ:pHallSvr is NULL");

		
	}
	else
	{
		char type = 65;
		//回复操作者
		char temp[64] ={0};
		BinaryWriteStream outstream1(temp,sizeof(temp));
		outstream1.Write(type);
		outstream1.Write((short)pdata->cmd);
		outstream1.Write(pdata->seq);	
		outstream1.Write((int)DBRESULT_MEMBER_FULL);	//ret 0--成功-1--失败(人数已满)
		outstream1.Write(pdata->bopidx);
		outstream1.Flush();	
		RoomClient* pClient = g_pNetProxy->GetClient(pdata->number);
		if(pClient!=NULL)
		{
			if(g_pNetProxy->SendToSrv(pClient, outstream1)==-1)
			{
				AC_ERROR("DbagentDataDecoder::DoGiveVJ:SendToSrv error");
			}
		}
		else
		{
			AC_ERROR("DbagentDataDecoder::DoGiveVJ:pClient is NULL ");
		}
	}
	
	return 0;
}

//modify by jinguanfu 2010/1/19
//int DbagentDataDecoder::DoGiveOwnS(DB_RESULT_DATA* pdata)
int DbagentDataDecoder::DoGiveOwnS(Dbresultdata* pdata,BinaryReadStream* infostream)
{
	AC_DEBUG("DbagentDataDecoder::DoGiveOwnS:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);

	if(pdata->roomid == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoGiveOwnS:roomid error");
		return -1;
	}

	if(pdata->opidx == 0 || pdata->bopidx == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoGiveOwnS:idx error");
		return -1;
	}

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(proominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoGiveOwnS:proominfo error");
		return -1;
	}

	char infobuf[1024];
	size_t clen;
	if(!infostream->Read(infobuf,sizeof(infobuf),clen))
	{
		AC_ERROR("DbagentDataDecoder::DoGiveOwnS:read result error");
		return -1;
	}
	infobuf[clen]=0;
	
	int result=atoi(infobuf);
	
	//数据库操作成功后，发送通知
	if(result==DBRESULT_MEMBER_SUCCESS)
	{
		//设置为房间副室主
		if(proominfo->secondownidx==0)
			proominfo->secondownidx=pdata->bopidx;
		else if(proominfo->secondownidx2==0)
			proominfo->secondownidx2=pdata->bopidx;

		//如果原来有其他权限，则从其他权限表中删除
		map<int,int>::iterator itVJ = proominfo->vjlist.find(pdata->bopidx);
		map<int,int>::iterator itVJA = proominfo->vjlist_a.find(pdata->bopidx);
		if(itVJ!=proominfo->vjlist.end())
			proominfo->vjlist.erase(itVJ);
		else if(itVJA!=proominfo->vjlist_a.end())
			proominfo->vjlist_a.erase(itVJA);

		//被操作者身份为主持人
		map<int,RoomClient*>::iterator itu=proominfo->userlist.find(pdata->bopidx);
		if(itu!=proominfo->userlist.end())
		{
			RoomClient* pbClient = (*itu).second;
			if(pbClient)
				pbClient->m_identity = USER_ID_OWNER_S;
		}
		//add by jinguanfu 2010/6/17 <begin>
		//不在管理员列表，则加入到管理员列表中
		map<int,ROOM_MANAGER>::iterator itM=proominfo->managerlist.find(pdata->bopidx);
		if(itM==proominfo->managerlist.end())
		{
			ROOM_MANAGER manger;
			manger.m_idx=pdata->bopidx;
			manger.m_identity=USER_ID_OWNER_S;
			proominfo->managerlist.insert(make_pair(pdata->bopidx,manger));
		}else
		{
			ROOM_MANAGER* pManger=(ROOM_MANAGER*)(&(itM->second));
			pManger->m_identity=USER_ID_OWNER_S;
		}

		map<int,ROOM_MANAGER>::iterator itMO=proominfo->managerlist_online.find(pdata->bopidx);
		if(itMO==proominfo->managerlist_online.end())
		{
			ROOM_MANAGER manger;
			manger.m_idx=pdata->bopidx;
			manger.m_identity=USER_ID_VJ_A;
			proominfo->managerlist_online.insert(make_pair(pdata->bopidx,manger));
		}else
		{
			ROOM_MANAGER* pManger=(ROOM_MANAGER*)(&(itMO->second));
			pManger->m_identity=USER_ID_VJ_A;
			
		}
		//add by jinguanfu 2010/6/17 <end>
		
		char type = 65;
		//回复操作者
		char temp[64] ={0};
		BinaryWriteStream outstream1(temp,sizeof(temp));
		outstream1.Write(type);
		outstream1.Write((short)pdata->cmd);
		outstream1.Write(pdata->seq);	
		outstream1.Write(result);	//ret 0--成功-1--失败(人数已满)
		outstream1.Write(pdata->bopidx);
		outstream1.Flush();	
		RoomClient* pClient = g_pNetProxy->GetClient(pdata->number);
		if(pClient!=NULL)
		{
			if(g_pNetProxy->SendToSrv(pClient, outstream1)==-1)
			{
				AC_ERROR("DbagentDataDecoder::DoGiveOwnS:SendToSrv error");
			}
		}
		else
		{
			AC_ERROR("DbagentDataDecoder::DoGiveOwnS:pClient is NULL ");
		}
		//房间内通知
		char context[64] = {0};
		BinaryWriteStream outstream(context,sizeof(context));
		int seq = 0;
		outstream.Write(type);
		outstream.Write((short)ROOM_CMD_SB_GIVE_OUER_S);
		outstream.Write(seq);	
		outstream.Write(pdata->opidx);		//操作者
		outstream.Write(pdata->bopidx);		//被操作者
		char level=USER_ID_OWNER_S;		//权限
		outstream.Write(level);
		outstream.Flush();	
		if(g_pNetProxy->BroadcastSBCmd(proominfo,outstream) == -1)
		{
			AC_ERROR("DbagentDataDecoder::DoGiveOwnS:g_pNetProxy->BroadcastSBCmd error");
			return -1;
		}

		//add by jinguanfu 2010/6/5 权限更新通知
		//发送大厅通知
		BinaryWriteStream* lobbystream=StreamFactory::InstanceWriteStream();	
		lobbystream->Write((short)HALL_CMD_CHANGERIGHT_R2L);
		lobbystream->Write(0);		
		lobbystream->Write((char)ROOM_TYPE_ROOM);	//ADD BY JINGUANFU 2010/7/9
		lobbystream->Write(pdata->opidx);		//操作者
		lobbystream->Write(pdata->bopidx);		//操作者
		lobbystream->Write(level);
		lobbystream->Flush();

		HallSvrClient *pHallSvr = g_pNetProxy->GetHallSvr();
		if(pHallSvr!=NULL)
			pHallSvr->AddBuf(lobbystream->GetData(), lobbystream->GetSize());
		else
			AC_ERROR("DbagentDataDecoder::DoGiveOwnS:pHallSvr is NULL");

		
	}
	else
	{
		char type = 65;
		//回复操作者
		char temp[64] ={0};
		BinaryWriteStream outstream1(temp,sizeof(temp));
		outstream1.Write(type);
		outstream1.Write((short)pdata->cmd);
		outstream1.Write(pdata->seq);	
		outstream1.Write((int)DBRESULT_MEMBER_FULL);	//ret 0--成功-1--失败(人数已满)
		outstream1.Write(pdata->bopidx);
		outstream1.Flush();	
		RoomClient* pClient = g_pNetProxy->GetClient(pdata->number);
		if(pClient!=NULL)
		{
			if(g_pNetProxy->SendToSrv(pClient, outstream1)==-1)
			{
				AC_ERROR("DbagentDataDecoder::DoGiveOwnS:SendToSrv error");
			}	
		}
		else
		{
			AC_ERROR("DbagentDataDecoder::DoGiveOwnS:pClient is NULL");
		}
	}

	return 0;
}

//modify by jinguanfu 2010/1/19
//int DbagentDataDecoder::DoSetRoomLock(DB_RESULT_DATA* pdata)
int DbagentDataDecoder::DoSetRoomLock(Dbresultdata* pdata)
{
	AC_DEBUG("DbagentDataDecoder::DoSetRoomLock:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);

	if(pdata->roomid == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomLock:roomid error");
		return -1;
	}

	//if(pdata->opidx == 0 || pdata->bopidx == 0)
	if(pdata->opidx == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomLock:idx error");
		return -1;
	}

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(proominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomLock:proominfo error");
		return -1;
	}

	//更新房间开启关闭标志
	if(pdata->bopidx==1)
		proominfo->isClose= true;
	else if(pdata->bopidx==0)
		proominfo->isClose = false;

	//回应操作用户
	char  temp[64] = {0};
	
	BinaryWriteStream onestream(temp,sizeof(temp));
	char type = 65;
	onestream.Write(type);
	onestream.Write((short)pdata->cmd);
	onestream.Write(pdata->seq);
	onestream.Write((int)0);	//ret	
	if(proominfo->isClose)
		onestream.Write((char)1); //关闭
	else
		onestream.Write((char)0); //开启
	onestream.Flush();	
	RoomClient* pClient = g_pNetProxy->GetClient(pdata->number);
	if(pClient!=NULL)
	{
		if(g_pNetProxy->SendToSrv(pClient, onestream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::DoSetRoomLock:g_pNetProxy->SendToSrv error");
		}
	}
	else
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomLock:pClient is NULL");
	}
	//通知所有房间内用户
	char context[64] = {0};
	BinaryWriteStream outstream(context,sizeof(context));
	//char type = 65;
	outstream.Write(type);
	outstream.Write((short)ROOM_CMD_SB_SET_ROOM_LOCK);
	outstream.Write((int)0);
	outstream.Write((int)0);	//ret
	outstream.Write((int)pdata->opidx);	//操作者idx
	if(proominfo->isClose)
		outstream.Write((char)1); //状态
	else
		outstream.Write((char)0);
	outstream.Flush();	

	//modify by jinguanfu 2010/5/12 直接发送设置通知
	//if(g_pNetProxy->BroadcastSBCmd(proominfo,outstream) == -1)
	if(g_pNetProxy->BroadcastSBCmd(proominfo,outstream,1) == -1)
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomLock:g_pNetProxy->BroadcastSBCmd error");
	}

	//add by jinguanfu 2010/4/22 通知大厅
	BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
	outstream2->Write((short)HALL_CMD_CHANGECLOSEFLAG_R2L);
	outstream2->Write((int)0);
	outstream2->Write((char)ROOM_TYPE_ROOM);	//ADD BY JINGUANFU 2010/7/9
	outstream2->Write((int)proominfo->roomid);
	if(proominfo->isClose)
		outstream2->Write((char)1);
	else
		outstream2->Write((char)0);
	outstream2->Flush();

	HallSvrClient *pHallSvr = g_pNetProxy->GetHallSvr();
	if(pHallSvr!=NULL)
		pHallSvr->AddBuf(outstream2->GetData(), outstream2->GetSize());
	else
		AC_ERROR("DbagentDataDecoder::DoSetRoomLock:pHallSvr is NULL");
		

	//add by jinguanfu 2010/5/12 房间关闭后，踢出所有在房间里的人
	map<int,RoomClient*> kicklist;			//待踢出人员列表 add by jinguanfu 2010/9/20
	map<int,RoomClient*>::iterator itu=proominfo->userlist.begin();
	for(;itu!=proominfo->userlist.end();itu++)
	{
		//GM、室主、副室主不踢出房间
		RoomClient* pRoomClient=itu->second;

		map<int,GM_INFO>::iterator itGM=g_pNetProxy->m_GM.find(pRoomClient->m_idx);
		if(	itGM==g_pNetProxy->m_GM.end()&&
			pRoomClient->m_idx!=proominfo->ownidx&&
			pRoomClient->m_idx!=proominfo->secondownidx&&
			pRoomClient->m_idx!=proominfo->secondownidx2)
		{
			//pRoomClient->LeaveRoom();
			kicklist.insert(make_pair(pRoomClient->m_idx,pRoomClient));
		}
	}

	map<int,RoomClient*>::iterator itk=kicklist.begin();
	for(;itk!=kicklist.end();itk++)
	{
		RoomClient* pRoomClient=itk->second;
		if(pRoomClient!=NULL)
			pRoomClient->LeaveRoom();
	}

	//add by jinguanfu 2010/11/27
	//清空虚拟用户
	map<int, RoomClient*>::iterator itvu = proominfo->vuserlist.begin();
	for(;itvu != proominfo->vuserlist.end();itvu++)
	{
		RoomClient*	pClient=(RoomClient*)itvu->second;
		if(pClient!=NULL)
			pClient->VLeaveRoom();
		
	}

	proominfo->vuserlist.clear();
	

	return 0;
}

//modify by jinguanfu 2010/1/19
//int DbagentDataDecoder::DoSetRoomUserOnly(DB_RESULT_DATA* pdata)
int DbagentDataDecoder::DoSetRoomUserOnly(Dbresultdata* pdata/*,BinaryReadStream* infostream*/)
{
	AC_DEBUG("DbagentDataDecoder::DoSetRoomUserOnly:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);

	if(pdata->roomid == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomUserOnly:roomid error");
		return -1;
	}

//	if(pdata->opidx == 0 || pdata->bopidx == 0)
	if(pdata->opidx == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomUserOnly:idx error");
		return -1;
	}

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(proominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomUserOnly:proominfo error");
		return -1;
	}

	//更新房间私有访问标志
	if(pdata->bopidx==0)
		proominfo->isUserOnly = false;
	else if(pdata->bopidx==1)
		proominfo->isUserOnly = true;
	
	char  temp[64] = {0};
	
	BinaryWriteStream onestream(temp,sizeof(temp));
	char type = 65;
	onestream.Write(type);
	onestream.Write((short)pdata->cmd);
	onestream.Write(pdata->seq);
	onestream.Write((int)0);	
	if(proominfo->isUserOnly)
		onestream.Write((char)1);
	else
		onestream.Write((char)0);
	onestream.Flush();	
	RoomClient* pClient = g_pNetProxy->GetClient(pdata->number);
	if(pClient!=NULL)
	{
		if(g_pNetProxy->SendToSrv(pClient, onestream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::DoSetRoomUserOnly:g_pNetProxy->SendToSrv error");
		}
	}
	else
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomUserOnly:pClient is NULL");
	}
	

	char context[64] = {0};
	BinaryWriteStream outstream(context,sizeof(context));
	//char type = 65;
	outstream.Write(type);
	outstream.Write((short)ROOM_CMD_SB_SET_USER_ONLY);
	outstream.Write((int)0);
	outstream.Write((int)0);	//ret
	outstream.Write((int)pdata->opidx);	
	if(proominfo->isUserOnly)
		outstream.Write((char)1);
	else
		outstream.Write((char)0);
	outstream.Flush();	
	if(g_pNetProxy->BroadcastSBCmd(proominfo,outstream) == -1)
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomUserOnly:g_pNetProxy->BroadcastSBCmd error");
		return -1;
	}

	//add by jinguanfu 2010/4/22 通知大厅
	BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
	outstream2->Write((short)HALL_CMD_CHANGEUSERONLYFLAG_R2L);
	outstream2->Write((int)0);
	outstream2->Write((char)ROOM_TYPE_ROOM);	//ADD BY JINGUANFU 2010/7/9
	outstream2->Write((int)proominfo->roomid);
	if(proominfo->isUserOnly)
		outstream2->Write((char)1);
	else
		outstream2->Write((char)0);
	outstream2->Flush();

	HallSvrClient *pHallSvr = g_pNetProxy->GetHallSvr();
	if(pHallSvr!=NULL)
		pHallSvr->AddBuf(outstream2->GetData(), outstream2->GetSize());
	else
		AC_ERROR("DbagentDataDecoder::DoSetRoomUserOnly:pHallSvr is NULL");
	
	return 0;
}

//modify by jinguanfu 2010/1/19
//int DbagentDataDecoder::DoSetRoomInOut(DB_RESULT_DATA* pdata)
int DbagentDataDecoder::DoSetRoomInOut(Dbresultdata* pdata/*,BinaryReadStream* infostream*/)
{
	AC_DEBUG("DbagentDataDecoder::DoSetRoomInOut:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);

	if(pdata->roomid == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomInOut:roomid error");
		return -1;
	}

	//if(pdata->opidx == 0 || pdata->bopidx == 0)
	if(pdata->opidx == 0 )
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomInOut:idx error");
		return -1;
	}

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(proominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomInOut:proominfo error");
		return -1;
	}

	//更新房间显示用户进出消息标志
	if(pdata->bopidx==0)
		proominfo->isUserInOut= false;
	else if(pdata->bopidx==1)
		proominfo->isUserInOut = true;
	
	char  temp[128] = {0};
	
	BinaryWriteStream onestream(temp,sizeof(temp));
	char type = 65;
	onestream.Write(type);
	onestream.Write((short)pdata->cmd);
	onestream.Write(pdata->seq);
	onestream.Write((int)0);	
	if(proominfo->isUserInOut)
		onestream.Write((char)1);
	else
		onestream.Write((char)0);
	onestream.Flush();	

	RoomClient* pClient = g_pNetProxy->GetClient(pdata->number);
	if(pClient!=NULL)
	{
		if(g_pNetProxy->SendToSrv(pClient, onestream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::DoSetRoomInOut:g_pNetProxy->SendToSrv error");
			//return -1;
		}
	}
	else{
		AC_ERROR("DbagentDataDecoder::DoSetRoomInOut:pClient is NULL");
		}
	
	char context[64] = {0};
	BinaryWriteStream outstream(context,sizeof(context));
	//char type = 65;
	outstream.Write(type);
	outstream.Write((short)ROOM_CMD_SB_SET_USER_INOUT);
	outstream.Write((int)0);
	outstream.Write((int)0);	//ret
	outstream.Write((int)pdata->opidx);	
	if(proominfo->isUserInOut)
		outstream.Write((char)1);
	else
		outstream.Write((char)0);
	outstream.Flush();	
	if(g_pNetProxy->BroadcastSBCmd(proominfo,outstream) == -1)
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomInOut:g_pNetProxy->BroadcastSBCmd error");
		return -1;
	}

	return 0;
}

//modify by jinguanfu 2010/1/19
//int DbagentDataDecoder::DoSetRoomMicUpDown(DB_RESULT_DATA* pdata)
int DbagentDataDecoder::DoSetRoomMicUpDown(Dbresultdata* pdata/*,BinaryReadStream* infostream*/)
{
	AC_DEBUG("DbagentDataDecoder::DoSetRoomMicUpDown:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);

	if(pdata->roomid == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomMicUpDown:roomid error");
		return -1;
	}

	//if(pdata->opidx == 0 || pdata->bopidx == 0)
	if(pdata->opidx == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomMicUpDown:idx error");
		return -1;
	}

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(proominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomMicUpDown:proominfo error");
		return -1;
	}

	//更新房间自由上麦标志
	if(pdata->badd==0)
		proominfo->isMicUpdown= false;
	else if(pdata->badd==1)
		proominfo->isMicUpdown = true;
	
	char  temp[64] = {0};
	BinaryWriteStream outstream(temp,sizeof(temp));
	char type = 65;
	outstream.Write(type);
	outstream.Write((short)pdata->cmd);
	outstream.Write(pdata->seq);
	outstream.Write((int)0);	
	if(proominfo->isMicUpdown)
		outstream.Write((char)1);
	else
		outstream.Write((char)0);
	outstream.Flush();	
	RoomClient* pClient = g_pNetProxy->GetClient(pdata->number);
	if(pClient!=NULL)
	{
		if(g_pNetProxy->SendToSrv(pClient, outstream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::DoSetRoomMicUpDown:g_pNetProxy->SendToSrv error");
		}
	}
	else
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomMicUpDown:pClient is NULL");	
	}
	
	outstream.Clear();
	outstream.Write(type);
	outstream.Write((short)ROOM_CMD_SB_SET_MIC_UPDOWN);
	outstream.Write((int)0);	//seq
	outstream.Write((int)0);	//ret
	outstream.Write((int)pdata->opidx);	
	if(proominfo->isMicUpdown)
		outstream.Write((char)1);
	else
		outstream.Write((char)0);
	outstream.Flush();	
	if(g_pNetProxy->BroadcastSBCmd(proominfo,outstream) == -1)
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomMicUpDown:g_pNetProxy->BroadcastSBCmd error");
		return -1;
	}

	//add by jinguanfu 2011/4/19
	//关闭自由排麦后，取消麦序里的排麦
	if(!proominfo->isMicUpdown)
	{
		vector<MIC_INFO*>::iterator itmic= proominfo->miclist.begin();
		for(;itmic!=proominfo->miclist.end();itmic++)
		{
			MIC_INFO* pmicinfo = (MIC_INFO*)(*itmic);
			if(pmicinfo==NULL)
			{
				AC_ERROR("DbagentDataDecoder::DoSetRoomMicUpDown:pmicinfo is NULL ");
				continue;
			}

			//设置用户状态
			map<int,RoomClient*>::iterator itc = proominfo->userlist.find(pmicinfo->idx);
			if(itc!=proominfo->userlist.end())
			{
				RoomClient* pRoomClient=(RoomClient*)itc->second;
				pRoomClient->m_onmicflag = USER_STATE_NONE; 
			}
		
			//通知取消排麦
			outstream.Clear();
			outstream.Write(type);
			outstream.Write((short)ROOM_CMD_SB_CANCLE_WAITMIC);
			outstream.Write(0);	//seq
			outstream.Write(pdata->opidx);//操作者
			outstream.Write(pmicinfo->idx);//被操作者
			outstream.Flush();
			if(g_pNetProxy->BroadcastSBCmd(proominfo, outstream) == -1)
			{
				AC_ERROR("DbagentDataDecoder::DoSetRoomMicUpDown:g_pNetProxy->BroadcastSBCmd CANCLEWAITMIC error");
			}

			AC_DEBUG("DbagentDataDecoder::DoSetRoomMicUpDown:idx=%d be cancel wait mic by admin.",pmicinfo->idx);
			//释放内存到缓存池
			g_pNetProxy->DestroyMicInfo(pmicinfo);
		}
		//清空麦序列表
		proominfo->miclist.clear();
	}
	

	return 0;
}

//modify by jinguanfu 2010/1/19
//int DbagentDataDecoder::DoSetRoomName(DB_RESULT_DATA* pdata)
int DbagentDataDecoder::DoSetRoomName(Dbresultdata* pdata/*,BinaryReadStream* infostream*/)
{
	AC_DEBUG("DbagentDataDecoder::DoSetRoomName:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);

	if(pdata->roomid == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomName:roomid error");
		return -1;
	}

	//if(pdata->opidx == 0 || pdata->bopidx == 0)
	if(pdata->opidx == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomName:idx error");
		return -1;
	}

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(proominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomName:proominfo error");
		return -1;
	}

	if(strlen(pdata->content)>=sizeof(proominfo->roomname))
	{
		strncpy(proominfo->roomname, pdata->content,sizeof(proominfo->roomname)-1);	//更新房间名
		proominfo->roomname[sizeof(proominfo->roomname)-1]=0;
	}
	else
	{
		strncpy(proominfo->roomname, pdata->content,strlen(pdata->content));	//更新房间名
		proominfo->roomname[strlen(pdata->content)]=0;
	}
	
	char  temp[64] = {0};
	
	BinaryWriteStream onestream(temp,sizeof(temp));
	char type = 65;
	onestream.Write(type);
	onestream.Write((short)pdata->cmd);
	onestream.Write(pdata->seq);
	onestream.Write((int)0);	
	onestream.Flush();	
	RoomClient* pClient = g_pNetProxy->GetClient(pdata->number);
	if(pClient!=NULL)
	{
		if(g_pNetProxy->SendToSrv(pClient, onestream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::DoSetRoomName:g_pNetProxy->SendToSrv error");
		}
	}
	else
		{
			AC_ERROR("DbagentDataDecoder::DoSetRoomName:pClient is NULL");
		}
	
	char context[1024] = {0};
	BinaryWriteStream outstream(context,sizeof(context));
	//char type = 65;
	outstream.Write(type);
	outstream.Write((short)ROOM_CMD_SB_SET_ROOM_NAME);
	outstream.Write((int)0);
	outstream.Write((int)0);	//ret
	outstream.Write(pdata->roomid);
	outstream.Write(pdata->content,strlen(pdata->content));	
	outstream.Flush();	
	if(g_pNetProxy->BroadcastSBCmd(proominfo,outstream) == -1)
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomName:g_pNetProxy->BroadcastSBCmd error");
		//return -1;
	}
	//add by jinguanfu 2010/4/9 通知大厅
	BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
	//BinaryWriteStream outstream(outbuf, sizeof(outbuf));
	outstream2->Write((short)HALL_CMD_CHANGENAME_R2L);
	outstream2->Write((int)0);
	outstream2->Write((char)ROOM_TYPE_ROOM);	//ADD BY JINGUANFU 2010/7/9
	outstream2->Write((int)proominfo->roomid);
	outstream2->Write(pdata->content,strlen(pdata->content));	
	outstream2->Flush();

	HallSvrClient *pHallSvr = g_pNetProxy->GetHallSvr();
	if(pHallSvr!=NULL)
		pHallSvr->AddBuf(outstream2->GetData(), outstream2->GetSize());
	else
		AC_ERROR("DbagentDataDecoder::DoSetRoomName:pHallSvr is NULL");

	return 0;
}

//modify by jinguanfu 2010/1/19
//int DbagentDataDecoder::DoSetChatPublic(DB_RESULT_DATA* pdata)
int DbagentDataDecoder::DoSetChatPublic(Dbresultdata* pdata/*,BinaryReadStream* infostream*/)
{
	AC_DEBUG("DbagentDataDecoder::DoSetChatPublic:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);

	if(pdata->roomid == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoSetChatPublic:roomid error");
		return -1;
	}

	//if(pdata->opidx == 0 || pdata->bopidx == 0)
	if(pdata->opidx == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoSetChatPublic:idx error");
		return -1;
	}

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(proominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoSetChatPublic:proominfo error");
		return -1;
	}

	//更新房间显示用户进出消息标志
	if(pdata->bopidx==0)
		proominfo->isPublicChat= false;
	else if(pdata->bopidx==1)
		proominfo->isPublicChat = true;
	
	char  temp[64] = {0};
	
	BinaryWriteStream onestream(temp,sizeof(temp));
	char type = 65;
	onestream.Write(type);
	onestream.Write((short)pdata->cmd);
	onestream.Write(pdata->seq);
	onestream.Write((int)0);	
	if(proominfo->isPublicChat)
		onestream.Write((char)1);
	else
		onestream.Write((char)0);
	onestream.Flush();	
	RoomClient* pClient = g_pNetProxy->GetClient(pdata->number);
	if(pClient!=NULL)
	{
		if(g_pNetProxy->SendToSrv(pClient, onestream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::DoSetChatPublic:g_pNetProxy->SendToSrv error");
		}
	}
	else
		{
			AC_ERROR("DbagentDataDecoder::DoSetChatPublic:pClient is NULL");
		}
	
	char context[64] = {0};
	BinaryWriteStream outstream(context,sizeof(context));
	//char type = 65;
	outstream.Write(type);
	outstream.Write((short)ROOM_CMD_SB_SET_CHAT_PUBLIC);
	outstream.Write((int)0);	//seq
	outstream.Write((int)0);	//ret
	outstream.Write((int)pdata->opidx);	
	if(proominfo->isPublicChat)
		outstream.Write((char)1);
	else
		outstream.Write((char)0);
	outstream.Flush();	
	if(g_pNetProxy->BroadcastSBCmd(proominfo,outstream) == -1)
	{
		AC_ERROR("DbagentDataDecoder::DoSetChatPublic:g_pNetProxy->BroadcastSBCmd error");
		return -1;
	}

	return 0;
}

//modify by jinguanfu 2010/1/19
//int DbagentDataDecoder::DoSetRoomWelcome(DB_RESULT_DATA* pdata)
int DbagentDataDecoder::DoSetRoomWelcome(Dbresultdata* pdata/*,BinaryReadStream* infostream*/)
{
	AC_DEBUG("DbagentDataDecoder::DoSetRoomWelcome:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);

	if(pdata->roomid == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomWelcome:roomid error");
		return -1;
	}

//	if(pdata->opidx == 0 || pdata->bopidx == 0)
	if(pdata->opidx == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomWelcome:idx error");
		return -1;
	}

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(proominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomWelcome:proominfo error");
		return -1;
	}

	if(strlen(pdata->content)>=sizeof(proominfo->welcomeword))
	{
		strncpy(proominfo->welcomeword, pdata->content,sizeof(proominfo->welcomeword)-1);		//更新房间欢迎词
		proominfo->welcomeword[sizeof(proominfo->welcomeword)-1]=0;
	}
	else
	{
		strncpy(proominfo->welcomeword, pdata->content,strlen(pdata->content));		//更新房间欢迎词
		proominfo->welcomeword[strlen(pdata->content)]=0;
	}
	
	char  temp[64] = {0};
	
	BinaryWriteStream onestream(temp,sizeof(temp));
	char type = 65;
	onestream.Write(type);
	onestream.Write((short)pdata->cmd);
	onestream.Write(pdata->seq);
	onestream.Write((int)0);	
	onestream.Flush();	
	RoomClient* pClient = g_pNetProxy->GetClient(pdata->number);
	if(pClient!=NULL)
	{
		if(g_pNetProxy->SendToSrv(pClient, onestream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::DoSetRoomWelcome:g_pNetProxy->SendToSrv error");
			//return -1;
		}
	}
	else
		{
			AC_ERROR("DbagentDataDecoder::DoSetRoomWelcome:pClient is NULL");
		}
	char context[1024] = {0};
	BinaryWriteStream outstream(context,sizeof(context));
	//char type = 65;
	outstream.Write(type);
	outstream.Write((short)ROOM_CMD_SB_SET_ROOM_WELCOME);
	outstream.Write((int)0);
	outstream.Write((int)0);
	outstream.Write((int)pdata->opidx);
	//outstream.Write(pdata->content,strlen(pdata->content));		
	outstream.Flush();	
	if(g_pNetProxy->BroadcastSBCmd(proominfo,outstream) == -1)
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomWelcome:g_pNetProxy->BroadcastSBCmd error");
		return -1;
	}

	return 0;
}

//modify by jinguanfu 2010/1/19
//int DbagentDataDecoder::DoSetRoomLogo(DB_RESULT_DATA* pdata)
int DbagentDataDecoder::DoSetRoomLogo(Dbresultdata* pdata/*,BinaryReadStream* infostream*/)
{
	AC_DEBUG("DbagentDataDecoder::DoSetRoomLogo:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);

	if(pdata->roomid == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomLogo:roomid error");
		return -1;
	}

//	if(pdata->opidx == 0 || pdata->bopidx == 0)
	if(pdata->opidx == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomLogo:idx error");
		return -1;
	}

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(proominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomLogo:proominfo error");
		return -1;
	}

	if(strlen(pdata->content)>=sizeof(proominfo->roomlogo))
	{
		strncpy(proominfo->roomlogo, pdata->content,sizeof(proominfo->roomlogo)-1);	//更新房间LOGO路径
		proominfo->roomlogo[sizeof(proominfo->roomlogo)-1]=0;
	}
	else
	{
		strncpy(proominfo->roomlogo, pdata->content,strlen(pdata->content));	//更新房间LOGO路径
	}
	
	char  temp[64] = {0};
	
	BinaryWriteStream onestream(temp,sizeof(temp));
	char type = 65;
	onestream.Write(type);
	onestream.Write((short)pdata->cmd);
	onestream.Write(pdata->seq);
	onestream.Write((int)0);	
	onestream.Flush();	
	RoomClient* pClient = g_pNetProxy->GetClient(pdata->number);
	if(pClient!=NULL)
	{
		if(g_pNetProxy->SendToSrv(pClient, onestream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::DoSetRoomLogo:g_pNetProxy->SendToSrv error");
			//return -1;
		}
	}
	else
		{
			AC_ERROR("DbagentDataDecoder::DoSetRoomLogo:pClient is NULL");
		}

	
	char context[1024] = {0};
	BinaryWriteStream outstream(context,sizeof(context));
	//char type = 65;
	outstream.Write(type);
	outstream.Write((short)ROOM_CMD_SB_SET_ROOM_LOGO);
	outstream.Write((int)0);
	outstream.Write((int)0);
	outstream.Write((int)pdata->opidx);
	//outstream.Write(pdata->content,sizeof(pdata->content));			
	outstream.Flush();	
	if(g_pNetProxy->BroadcastSBCmd(proominfo,outstream) == -1)
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomLogo:g_pNetProxy->BroadcastSBCmd error");
		return -1;
	}

	//add by jinguanfu 2011/3/22 通知大厅
	BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
	outstream2->Write((short)HALL_CMD_CHANGELOGO_R2L);
	outstream2->Write((int)0);
	outstream2->Write((char)ROOM_TYPE_ROOM);	
	outstream2->Write((int)proominfo->roomid);
	outstream2->Write(pdata->content,strlen(pdata->content));	
	outstream2->Flush();

	HallSvrClient *pHallSvr = g_pNetProxy->GetHallSvr();
	if(pHallSvr!=NULL)
		pHallSvr->AddBuf(outstream2->GetData(), outstream2->GetSize());
	else
		AC_ERROR("DbagentDataDecoder::DoSetRoomLogo:pHallSvr is NULL");

	return 0;
}

//modify by jinguanfu 2010/1/19
//int DbagentDataDecoder::DoRoomUserApp(DB_RESULT_DATA* pdata,BinaryReadStream* infostream)
int DbagentDataDecoder::DoRoomUserApp(Dbresultdata* pdata,
										BinaryReadStream* infostream)
{
	AC_DEBUG("DbagentDataDecoder::DoRoomUserApp:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);

	if(pdata->roomid == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoRoomUserApp:roomid error");
		return -1;
	}	

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(proominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoRoomUserApp:proominfo error");
		return -1;
	}
	
	char infobuf[1024];
	size_t clen;
	if(!infostream->Read(infobuf,sizeof(infobuf),clen))
	{
		AC_ERROR("DbagentDataDecoder::DoRoomUserApp:read result error");
		return -1;
	}
	infobuf[clen]=0;
	
	int result=atoi(infobuf);	//0--成功，2--已申请过,3--24小时内不能申请

	//对申请者的回应
	char  temp[64] = {0};
	BinaryWriteStream onestream(temp,sizeof(temp));
	char type = 65;
	onestream.Write(type);
	onestream.Write((short)pdata->cmd);
	onestream.Write(pdata->seq);
	if(result == DBRESULT_APPLY_REFUSE)	//服务器拒绝申请
		onestream.Write((int)REFUSE);				
	else if(result == DBRESULT_APPLY_SUCCESS)
		onestream.Write((int)SUCCESS);	
	else if(result == DBRESULT_APPLY_AGAGN)
		onestream.Write((int)IAGAIN);
	else if(result == DBRESULT_APPLY_ALREADY)
		onestream.Write((int)ALREADY);
	onestream.Flush();	
	RoomClient* pClient = g_pNetProxy->GetClient(pdata->number);
	if(pClient!=NULL)
	{
		if(g_pNetProxy->SendToSrv(pClient, onestream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::DoRoomUserApp:g_pNetProxy->SendToSrv error");
			return -1;
		}
	}
	else
		{
			AC_ERROR("DbagentDataDecoder::DoRoomUserApp:pClient is NULL");
		}
	if(result==DBRESULT_APPLY_SUCCESS)
	{
		//add by jinguanfu 2010/8/16
		if(!infostream->Read(infobuf,sizeof(infobuf),clen))
		{
			AC_ERROR("DbagentDataDecoder::DoRoomUserApp:read date error");
			return -1;
		}
		
		ROOM_APPLY roomapply;
		roomapply.m_roomid=pdata->roomid;
		roomapply.m_idx = pdata->opidx;
		strncpy(roomapply.m_time, infobuf,sizeof(roomapply.m_time));	//add by jinguanfu 2010/8/16
/* delete by jinguanfu 2010/8/16
		time_t ltime;
        	struct tm* tmNow;
		time(&ltime);
		tmNow = localtime(&ltime);
		sprintf(roomapply.m_time,"%04d-%02d-%02d %02d:%02d:%02d",
			tmNow->tm_year+1900,
			tmNow->tm_mon+1,
			tmNow->tm_mday,
			tmNow->tm_hour,
			tmNow->tm_min,
			tmNow->tm_sec);
*/		
		proominfo->userlistAPP.insert(make_pair(pdata->opidx,roomapply));
	}
	/*int seq=g_pNetProxy->GetCounter()->Get();
	proominfo->userlistAPP.insert(make_pair(pdata->opidx,pdata->opidx));			
	BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
	char type = 65;
	outstream->Write(type);
	outstream->Write((short)ROOM_CMD_SB_USER_APP_JOINROOM);
	outstream->Write(seq);
	outstream->Write((int)rel);		
	outstream->Flush();	
	if(g_pNetProxy->BroadcastSBCmd(proominfo,*outstream) == -1)
	{
		AC_ERROR("DbagentDataDecoder::DoRoomUserApp:g_pNetProxy->BroadcastSBCmd error");
		return -1;
	}*/

	return 0;
}


//modify by jinguanfu 2010/1/19
//int DbagentDataDecoder::DoReturnRoomApplyList(DB_RESULT_DATA* pdata,BinaryReadStream* infostream,short rownum,short colnum,short cmd)
int DbagentDataDecoder::DoReturnRoomApplyList(Dbresultdata* pdata,
												BinaryReadStream* infostream,
												short rownum,
												short colnum,
												short cmd)
{
	AC_DEBUG("DbagentDataDecoder::DoReturnRoomApplyList:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);
	
	RoomClient* pRoomClient = g_pNetProxy->GetClient(pdata->number);
	if(pRoomClient==NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoReturnRoomApplyList():pRoomClient is NULL");
		return 0;
	}
	char infobuf[1024] = {0};
	size_t curlen;
	int id,idx,state;
	char apptime[30];
	while(rownum>1000)
	{	
		rownum-=1000;
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();	
		//int seq = g_pNetProxy->GetCounter()->Get();
		char type = 65;
		outstream->Write(type);
		outstream->Write(cmd);
		outstream->Write(pdata->seq);		
		outstream->Write((int)rownum);		//剩余总数
		outstream->Write(1000);
		for(int i = 0;i < 1000;i++)
		{		
			for(int j = 0;j < colnum;j++)
			{
				if(!infostream->Read(infobuf,sizeof(infobuf),curlen))
				{
					AC_ERROR("DbagentDataDecoder::DoReturnRoomApplyList():read ptr error");
					return -1;
				}
				infobuf[curlen] = 0;
				if(j == 0)
				{
					id = atoi(infobuf);
					outstream->Write(id);
					
				}
				if(j == 1)
				{
					idx = atoi(infobuf);
					outstream->Write(idx);
					
				}
				if(j == 2)
				{
					state = atoi(infobuf);
					outstream->Write(state);
				}
				if(j == 3)
				{
					strcpy(apptime,infobuf);
					//outstream->Write((int)strlen(apptime));
					outstream->Write(apptime,strlen(apptime));
				}			
			}
		}
		outstream->Flush();
		if(g_pNetProxy->SendToSrv(pRoomClient, *outstream) == -1)
		{
			AC_ERROR(" DbagentDataDecoder::DoReturnRoomApplyList:g_pNetProxy->SendToSrv error");
			pRoomClient->Close();
			return 0;
		}
		rownum=rownum-1000;
	}
	if (rownum>=0)
	{
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();	
		char type = 65;
		outstream->Write(type);
		outstream->Write(cmd);
		outstream->Write(pdata->seq);		
		outstream->Write((int)0);		//剩余总数
		outstream->Write((int)rownum);
		for(int i = 0;i < rownum;i++)
		{		
			for(int j = 0;j < colnum;j++)
			{
				if(!infostream->Read(infobuf,sizeof(infobuf),curlen))
				{
					AC_ERROR("DbagentDataDecoder::DoReturnRoomApplyList():read ptr error");
					return -1;
				}
				infobuf[curlen] = 0;
				if(j == 0)
				{
					id = atoi(infobuf);
					outstream->Write(id);

				}
				if(j == 1)
				{
					idx = atoi(infobuf);
					outstream->Write(idx);

				}
				if(j == 2)
				{
					state = atoi(infobuf);
					outstream->Write(state);
				}
				if(j == 3)
				{
					strcpy(apptime,infobuf);
					//outstream->Write((int)strlen(apptime));
					outstream->Write(apptime,strlen(apptime));
				}			
			}
		}
		outstream->Flush();
		if(g_pNetProxy->SendToSrv(pRoomClient, *outstream) == -1)
		{
			AC_ERROR(" DbagentDataDecoder::DoReturnRoomApplyList:g_pNetProxy->SendToSrv error");
			pRoomClient->Close();
			return 0;
		}


	}
	return 0;
	
}

//modify by jinguanfu 2010/1/19
//int DbagentDataDecoder::DoReturnRoomBlackList(DB_RESULT_DATA* pdata,BinaryReadStream* infostream,short rownum,short colnum,short cmd)
int DbagentDataDecoder::DoReturnRoomBlackList(Dbresultdata* pdata,
										BinaryReadStream* infostream,
										short rownum,
										short colnum,
										short cmd)
{

	AC_DEBUG("DbagentDataDecoder::DoReturnRoomBlackList:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);

	RoomClient* pRoomClient = g_pNetProxy->GetClient(pdata->number);
	if(pRoomClient==NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoReturnRoomBlackList():pRoomClient is NULL");
		return 0;
	}
	char infobuf[1024] = {0};
	size_t curlen;
	int id,idx;
	char apptime[30];
	while(rownum>1000)
	{	
		rownum-=1000;
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();	
		int seq = g_pNetProxy->GetCounter()->Get();
		char type = 65;
		outstream->Write(type);
		outstream->Write(cmd);
		outstream->Write(seq);		
		outstream->Write(1);
		outstream->Write(1000);
		for(int i = 0;i < 1000;i++)
		{		
			for(int j = 0;j < colnum;j++)
			{
				if(!infostream->Read(infobuf,sizeof(infobuf),curlen))
				{
					AC_ERROR("NetProxy::DoReturnRoomBlackList():read ptr error");
					return -1;
				}
				infobuf[curlen] = 0;
				if(j == 0)
				{
					id = atoi(infobuf);
					outstream->Write(id);

				}
				if(j == 1)
				{
					idx = atoi(infobuf);
					outstream->Write(idx);

				}
				if(j == 2)
				{
					strcpy(apptime,infobuf);
					outstream->Write((int)strlen(apptime));
					outstream->Write(apptime,strlen(apptime));
				}			
			}
		}
		if(g_pNetProxy->SendToSrv(pRoomClient, *outstream) == -1)
		{
			AC_ERROR(" DbagentDataDecoder::DoReturnRoomBlackList:g_pNetProxy->SendToSrv error");
			pRoomClient->Close();
			return 0;
		}

	}
	if (rownum>0)
	{
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();	
		int seq = g_pNetProxy->GetCounter()->Get();
		char type = 65;
		outstream->Write(type);
		outstream->Write(cmd);
		outstream->Write(seq);		
		outstream->Write(1);
		outstream->Write(rownum);
		for(int i = 0;i < rownum;i++)
		{		
			for(int j = 0;j < colnum;j++)
			{
				if(!infostream->Read(infobuf,sizeof(infobuf),curlen))
				{
					AC_ERROR("NetProxy::DoReturnRoomBlackList():read ptr error");
					return -1;
				}
				infobuf[curlen] = 0;
				if(j == 0)
				{
					id = atoi(infobuf);
					outstream->Write(id);

				}
				if(j == 1)
				{
					idx = atoi(infobuf);
					outstream->Write(idx);

				}
				if(j == 2)
				{
					strcpy(apptime,infobuf);
					outstream->Write((int)strlen(apptime));
					outstream->Write(apptime,strlen(apptime));
				}			
			}
		}
		if(g_pNetProxy->SendToSrv(pRoomClient, *outstream) == -1)
		{
			AC_ERROR(" DbagentDataDecoder::DoReturnRoomBlackList:g_pNetProxy->SendToSrv error");
			pRoomClient->Close();
			return 0;
		}


	}
	return 0;

}

//modify by jinguanfu 2010/1/19
//int DbagentDataDecoder::DoReturnRoomMemberList(DB_RESULT_DATA* pdata,BinaryReadStream* infostream,short rownum,short colnum,short cmd)
int DbagentDataDecoder::DoReturnRoomMemberList(Dbresultdata* pdata,
												BinaryReadStream* infostream,
												short rownum,
												short colnum,
												short cmd)
{
	AC_DEBUG("DbagentDataDecoder::DoReturnRoomMemberList:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);

	RoomClient* pRoomClient = g_pNetProxy->GetClient(pdata->number);
	if(pRoomClient==NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoReturnRoomMemberList:pRoomClient is NULL");
		return 0;
	}
	char infobuf[1024] = {0};
	size_t curlen;
	int id,idx;
	char apptime[30];
	while(rownum>1000)
	{	
		rownum-=1000;
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();	
		int seq = g_pNetProxy->GetCounter()->Get();
		char type = 65;
		outstream->Write(type);
		outstream->Write(cmd);
		outstream->Write(seq);		
		outstream->Write(1);
		outstream->Write(1000);
		for(int i = 0;i < 1000;i++)
		{		
			for(int j = 0;j < colnum;j++)
			{
				if(!infostream->Read(infobuf,sizeof(infobuf),curlen))
				{
					AC_ERROR("NetProxy::DoReturnRoomBlackList():read ptr error");
					return -1;
				}
				infobuf[curlen] = 0;
				if(j == 0)
				{
					id = atoi(infobuf);
					outstream->Write(id);

				}
				if(j == 1)
				{
					idx = atoi(infobuf);
					outstream->Write(idx);

				}
				if(j == 2)
				{
					strcpy(apptime,infobuf);
					outstream->Write((int)strlen(apptime));
					outstream->Write(apptime,strlen(apptime));
				}			
			}
		}
		if(g_pNetProxy->SendToSrv(pRoomClient, *outstream) == -1)
		{
			AC_ERROR(" DbagentDataDecoder::DoReturnRoomBlackList:g_pNetProxy->SendToSrv error");
			pRoomClient->Close();
			return 0;
		}

	}
	if (rownum>0)
	{
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();	
		int seq = g_pNetProxy->GetCounter()->Get();
		char type = 65;
		outstream->Write(type);
		outstream->Write(cmd);
		outstream->Write(seq);		
		outstream->Write(1);
		outstream->Write(rownum);
		for(int i = 0;i < rownum;i++)
		{		
			for(int j = 0;j < colnum;j++)
			{
				if(!infostream->Read(infobuf,sizeof(infobuf),curlen))
				{
					AC_ERROR("NetProxy::DoReturnRoomBlackList():read ptr error");
					return -1;
				}
				infobuf[curlen] = 0;
				if(j == 0)
				{
					id = atoi(infobuf);
					outstream->Write(id);

				}
				if(j == 1)
				{
					idx = atoi(infobuf);
					outstream->Write(idx);

				}
				if(j == 2)
				{
					strcpy(apptime,infobuf);
					outstream->Write((int)strlen(apptime));
					outstream->Write(apptime,strlen(apptime));
				}			
			}
		}
		if(g_pNetProxy->SendToSrv(pRoomClient, *outstream) == -1)
		{
			AC_ERROR(" DbagentDataDecoder::DoReturnRoomBlackList:g_pNetProxy->SendToSrv error");
			pRoomClient->Close();
			return 0;
		}


	}
	return 0;

}

//add by jinguanfu 2010/4/7 房间密码设置回应
int DbagentDataDecoder::DoSetRoomPwd(Dbresultdata* pdata/*,BinaryReadStream* infostream*/)
{
	AC_DEBUG("DbagentDataDecoder::DoSetRoomPwd :opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);

	if(pdata->roomid == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomPwd:roomid error");
		return -1;
	}

	if(pdata->opidx == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomPwd:idx error");
		return -1;
	}

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(proominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomPwd:proominfo error");
		return -1;
	}
	
	if(strlen(pdata->content)>0&&strlen(pdata->content)<sizeof(proominfo->passwd))
	{
		strncpy(proominfo->passwd, pdata->content,strlen(pdata->content));
		proominfo->passwd[strlen(pdata->content)]=0;
	}
	else if(strlen(pdata->content)>=sizeof(proominfo->passwd))
	{
		strncpy(proominfo->passwd, pdata->content,sizeof(proominfo->passwd)-1);
		proominfo->passwd[sizeof(proominfo->passwd)-1]=0;
	}
	else
		memset(proominfo->passwd,0,sizeof(proominfo->passwd));

	//对操作者的回应
	char  temp[64] = {0};
	BinaryWriteStream onestream(temp,sizeof(temp));
	char type = 65;
	onestream.Write(type);
	onestream.Write((short)pdata->cmd);
	onestream.Write(pdata->seq);
	onestream.Write((int)0);				
	onestream.Flush();	
	RoomClient* pClient = g_pNetProxy->GetClient(pdata->number);
	if(pClient!=NULL)
	{
		if(g_pNetProxy->SendToSrv(pClient, onestream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::DoSetRoomPwd:g_pNetProxy->SendToSrv error");
		}
	}
	else
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomPwd:pClient is NULL");
	}
	//房间内用户广播
	char context[64] = {0};
	BinaryWriteStream outstream(context,sizeof(context));
	//char type = 65;
	outstream.Write(type);
	outstream.Write((short)ROOM_CMD_SB_SET_ROOM_PWD);
	outstream.Write((int)0);
	outstream.Write((int)0);	//ret
	outstream.Write((int)pdata->opidx);		
	if(strlen(proominfo->passwd)>0)
		outstream.Write((char)1);
	else
		outstream.Write((char)0);
	outstream.Flush();	
	if(g_pNetProxy->BroadcastSBCmd(proominfo,outstream) == -1)
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomPwd:g_pNetProxy->BroadcastSBCmd error");
		//return -1;
	}

	//add by jinguanfu 2010/4/9 通知大厅
	BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
	//BinaryWriteStream outstream(outbuf, sizeof(outbuf));
	outstream2->Write((short)HALL_CMD_CHANGEPWD_R2L);
	outstream2->Write((int)0);
	outstream2->Write((char)ROOM_TYPE_ROOM);	//ADD BY JINGUANFU 2010/7/9
	outstream2->Write((int)proominfo->roomid);
	if(strlen(proominfo->passwd)>0)
		outstream2->Write((char)1);
	else
		outstream2->Write((char)0);
	outstream2->Flush();

	HallSvrClient *pHallSvr = g_pNetProxy->GetHallSvr();
	if(pHallSvr!=NULL)
		pHallSvr->AddBuf(outstream2->GetData(), outstream2->GetSize());
	else
		AC_ERROR("DbagentDataDecoder::DoSetRoomPwd:pHallSvr is NULL");

	return 0;
}

//用户退出房间会员回应
int DbagentDataDecoder::DoExitRoomMember(Dbresultdata* pdata,BinaryReadStream* infostream)
{
	AC_DEBUG("DbagentDataDecoder::DoExitRoomMember:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);

	if(pdata->roomid == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoExitRoomMember:roomid error");
		return -1;
	}

	if(pdata->opidx == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoExitRoomMember:idx error");
		return -1;
	}

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(proominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoSetRoomPwd:proominfo error");
		return -1;
	}


	char infobuf[1024];
	size_t clen;
	if(!infostream->Read(infobuf,sizeof(infobuf),clen))
	{
		AC_ERROR("DbagentDataDecoder::DoRoomUserApp:read result error");
		return -1;
	}
	infobuf[clen]=0;
	
	int result=atoi(infobuf);
	
	//回应操作者
	char context[64] = {0};
	BinaryWriteStream outstream(context,sizeof(context));
	char type = 65;
	outstream.Write(type);
	outstream.Write((short)pdata->cmd);
	outstream.Write(pdata->seq);
	outstream.Write(result);				// 1--失败(非房间会员)，0--成功
	outstream.Flush();	
		
	RoomClient* pClient = g_pNetProxy->GetClient(pdata->number);
	if(pClient!=NULL)
	{
		if(g_pNetProxy->SendToSrv(pClient, outstream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::DoRemoveUser:SendToSrv error");
		}
	}
	else
	{
		AC_ERROR("DbagentDataDecoder::DoRemoveUser:pClient is NULL");
		//return 0;
	}
	
	if(result == DBRESULT_MEMBER_SUCCESS)
	{
		//更新房间信息
		map<int,int>::iterator itVIP= proominfo->userlistVIP.find(pdata->bopidx);
		map<int,int>::iterator itVJ= proominfo->vjlist.find(pdata->bopidx);
		map<int,int>::iterator itVJ_A= proominfo->vjlist_a.find(pdata->bopidx);
		map<int,ROOM_MANAGER>::iterator itM = proominfo->managerlist.find(pdata->bopidx);
		map<int,ROOM_MANAGER>::iterator itMO= proominfo->managerlist_online.find(pdata->bopidx);
		//从会员列表中删除
		if(itVIP!=proominfo->userlistVIP.end())
		{
			proominfo->userlistVIP.erase(itVIP);
		}
		//如果为管理员，则从管理员中删除
		if(itVJ!=proominfo->vjlist.end())
		{
			proominfo->vjlist.erase(itVJ);
		}
		else if(itVJ_A!=proominfo->vjlist_a.end())
		{
			proominfo->vjlist_a.erase(itVJ_A);
		}
		else if(pdata->bopidx==(int)proominfo->secondownidx)
		{
			proominfo->secondownidx=0;
		}
		else if(pdata->bopidx==(int)proominfo->secondownidx2)
		{
			proominfo->secondownidx2=0;
		}

		if(itM!=proominfo->managerlist.end())
		{
			proominfo->managerlist.erase(itM);
		}
	
		if(itMO!= proominfo->managerlist_online.end())
		{
			proominfo->managerlist_online.erase(itMO);
		}
		
		if(pClient!=NULL)
		{
			//退出会员后，身份为普通用户
			pClient->m_identity = USER_ID_NONE;
		}
		
		//通知大厅会员退出，通知由大厅去发
		char context[64] = {0};
		BinaryWriteStream outstream(context,sizeof(context));

		outstream.Write((short)HALL_CMD_EXITMEMBER_R2L);
		outstream.Write((int)0);
		outstream.Write((char)ROOM_TYPE_ROOM);	//ADD BY JINGUANFU 2010/7/9
		outstream.Write(pdata->roomid);
		outstream.Write(pdata->opidx);		//退出房间会员用户
		outstream.Flush();

		HallSvrClient *pHallSvr = g_pNetProxy->GetHallSvr();
		if(pHallSvr!=NULL)
			pHallSvr->AddBuf(outstream.GetData(), outstream.GetSize());
		else
			AC_ERROR("DbagentDataDecoder::DoRemoveUser:pHallSvr is NULL");

		outstream.Clear();
		outstream.Write(type);
		outstream.Write((short)ROOM_CMD_SB_EXITMEMBER);
		outstream.Write(0);
		outstream.Write(pdata->opidx);
		outstream.Flush();	

		g_pNetProxy->BroadcastSBCmd(proominfo,outstream);
		
	}

	return 0;
	
}

//室主、副室主将管理员降为一般会员
int DbagentDataDecoder::DoGiveMember(Dbresultdata* pdata,BinaryReadStream* infostream)
{
	AC_DEBUG("DbagentDataDecoder::DoGiveMember:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);

	if(pdata->roomid == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoGiveMember:roomid error");
		return -1;
	}

	if(pdata->opidx == 0 || pdata->bopidx == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoGiveMember:idx error");
		return -1;
	}

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(proominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoGiveMember:proominfo error");
		return -1;
	}

	char infobuf[1024];
	size_t clen;
	if(!infostream->Read(infobuf,sizeof(infobuf),clen))
	{
		AC_ERROR("DbagentDataDecoder::DoGiveMember:read result error");
		return -1;
	}
	infobuf[clen]=0;
	
	int result=atoi(infobuf);
	
	if(result==DBRESULT_MEMBER_SUCCESS)
	{
		//从原有权限中删除
		map<int,int>::iterator itVJ = proominfo->vjlist.find(pdata->bopidx);
		map<int,int>::iterator itVJA = proominfo->vjlist_a.find(pdata->bopidx);
		if(pdata->bopidx==(int)proominfo->secondownidx)
			proominfo->secondownidx=0;
		else if(pdata->bopidx==(int)proominfo->secondownidx2)
			proominfo->secondownidx2=0;
		else if(itVJ!=proominfo->vjlist.end())
			proominfo->vjlist.erase(itVJ);
		else if(itVJA!=proominfo->vjlist_a.end())
			proominfo->vjlist_a.erase(itVJA);

		//管理员中列表中删除
		map<int,ROOM_MANAGER>::iterator itM= proominfo->managerlist.find(pdata->bopidx);
		if(itM!= proominfo->managerlist.end())
			proominfo->managerlist.erase(itM);
		
		map<int,ROOM_MANAGER>::iterator itMO= proominfo->managerlist_online.find(pdata->bopidx);
		if(itMO!= proominfo->managerlist_online.end())
			proominfo->managerlist_online.erase(itMO);

		//被操作者身份设为一般会员
		map<int,RoomClient*>::iterator itu=proominfo->userlist.find(pdata->bopidx);
		if(itu!=proominfo->userlist.end())
		{
			RoomClient* pbClient = (*itu).second;
			if(pbClient)
				pbClient->m_identity = USER_ID_VIP;
		}
		
		char type = 65;
		//回复操作者
		char temp[64] ={0};
		BinaryWriteStream outstream1(temp,sizeof(temp));
		outstream1.Write(type);
		outstream1.Write((short)pdata->cmd);
		outstream1.Write(pdata->seq);	
		outstream1.Write(result);	//ret 0--成功-1--失败(人数已满)
		outstream1.Write(pdata->bopidx);
		outstream1.Flush();	
		RoomClient* pClient = g_pNetProxy->GetClient(pdata->number);
		if(pClient!=NULL)
		{
			if(g_pNetProxy->SendToSrv(pClient, outstream1)==-1)
			{
				AC_ERROR("DbagentDataDecoder::DoGiveMember:SendToSrv error");
			}
		}
		else
			{
				AC_ERROR("DbagentDataDecoder::DoGiveMember:pClient is NULL ");
			}
		char context[64] = {0};
		BinaryWriteStream outstream(context,sizeof(context));
		int seq = 0;
		outstream.Write(type);
		outstream.Write((short)ROOM_CMD_SB_GIVE_MEMBER);
		outstream.Write(seq);
		outstream.Write(pdata->opidx);		//操作者
		outstream.Write(pdata->bopidx);	//被操作者
		char level=USER_ID_VIP;			//权限
		outstream.Write(level);
		outstream.Flush();	
		if(g_pNetProxy->BroadcastSBCmd(proominfo,outstream) == -1)
		{
			AC_ERROR("DbagentDataDecoder::DoGiveMember:g_pNetProxy->BroadcastSBCmd error");
			return -1;
		}


		//add by jinguanfu 2010/6/5 权限更新通知
		//发送大厅通知
		BinaryWriteStream* lobbystream=StreamFactory::InstanceWriteStream();	
		lobbystream->Write((short)HALL_CMD_CHANGERIGHT_R2L);
		lobbystream->Write(0);	
		lobbystream->Write((char)ROOM_TYPE_ROOM);	//ADD BY JINGUANFU 2010/7/9
		lobbystream->Write(pdata->opidx);		//操作者
		lobbystream->Write(pdata->bopidx);		//操作者
		lobbystream->Write(level);
		lobbystream->Flush();

		HallSvrClient *pHallSvr = g_pNetProxy->GetHallSvr();
		if(pHallSvr!=NULL)
			pHallSvr->AddBuf(lobbystream->GetData(), lobbystream->GetSize());
		else 
			AC_ERROR("DbagentDataDecoder::DoGiveMember:pHallSvr is NULL");

	}
	else
	{
		char type = 65;
		//回复操作者
		char temp[64] ={0};
		BinaryWriteStream outstream1(temp,sizeof(temp));
		outstream1.Write(type);
		outstream1.Write((short)pdata->cmd);
		outstream1.Write(pdata->seq);	
		outstream1.Write((int)DBRESULT_MEMBER_FULL);	//ret 0--成功-1--失败(人数已满)
		outstream1.Write(pdata->bopidx);
		outstream1.Flush();	
		RoomClient* pClient = g_pNetProxy->GetClient(pdata->number);
		if(pClient!=NULL)
		{
			if(g_pNetProxy->SendToSrv(pClient, outstream1)==-1)
			{
				AC_ERROR("DbagentDataDecoder::DoGiveVJA:SendToSrv error");
			}
		}
		else
			{
			AC_ERROR("DbagentDataDecoder::DoGiveVJA:pClient is NULL ");
			}
	}
	
	return 0;
}

//数据库返回演唱经验值
int DbagentDataDecoder::DoScore(Dbresultdata* pdata,BinaryReadStream* infostream)
{
	AC_DEBUG("DbagentDataDecoder::DoScore:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);

	if(pdata->roomid == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoScore:roomid error");
		return -1;
	}

	if(pdata->opidx == 0 || pdata->bopidx == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoScore:idx error");
		return -1;
	}

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(proominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoScore:proominfo error");
		return -1;
	}

	char infobuf[1024];
	size_t clen;
	if(!infostream->Read(infobuf,sizeof(infobuf),clen))
	{
		AC_ERROR("DbagentDataDecoder::DoScore:read result error");
		return -1;
	}
	infobuf[clen]=0;
	
	char* saveptr=NULL;
	char* token=strtok_r(infobuf,"|",&saveptr);
	if(token==NULL||saveptr==NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoScore:parse result error");
		return 0;
	}
	int result=atoi(token);
	int level=0;			//经验等级
	int experience=0;		//经验值
	int epchange=0;			//经验更改值
	int silver=0;			//银币数
	int silverchange=0;	//银币更改值
	
	if(result==DBRESULT_SCORE_SUCCESS)
	{
		token=strtok_r(NULL,"|",&saveptr);
		if(token==NULL||saveptr==NULL)
		{
			AC_ERROR("DbagentDataDecoder::DoScore:parse experience error");
			return 0;
		}
		experience=atoi(token);		//用户经验值
		
		token=strtok_r(NULL,"|",&saveptr);
		if(token==NULL||saveptr==NULL)
		{
			AC_ERROR("DbagentDataDecoder::DoScore:parse level error");
			return 0;
		}
		level=atoi(token);		//用户经验等级

		token=strtok_r(NULL,"|",&saveptr);
		if(token==NULL||saveptr==NULL)
		{
			AC_ERROR("DbagentDataDecoder::DoScore:parse epchange error");
			return 0;
		}
		epchange=atoi(token);	//用户经验更改值

		token=strtok_r(NULL,"|",&saveptr);
		if(token==NULL||saveptr==NULL)
		{
			AC_ERROR("DbagentDataDecoder::DoScore:parse silver error");
			return 0;
		}
		silver=atoi(token);		//用户当前银币值

		token=strtok_r(NULL,"|",&saveptr);
		if(token==NULL||saveptr==NULL)
		{
			AC_ERROR("DbagentDataDecoder::DoScore:parse silverchange error");
			return 0;
		}
		silverchange=atoi(token);	//用户银币增加值
	
		RoomClient* pClient = g_pNetProxy->GetClient(pdata->number);
		if(pClient==NULL)
		{
			AC_ERROR("DbagentDataDecoder::DoScore:pClient is NULL.");
			return 0;
		}
		//当前评分经验及银币增加值
		char type=65;
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		/* del by jinguanfu 2011/8/11
		//add by jinguanfu 2010/10/28
		outstream->Write(type);
		outstream->Write((short)ROOM_CMD_SB_SCORE);
		outstream->Write(0);			//seq
		outstream->Write(epchange);			//当前经验增加值
		outstream->Write(silverchange);			//当前银币增加值
		outstream->Flush();
		g_pNetProxy->BroadcastSBCmd(proominfo, *outstream);
		
		
		pClient->silver=silver;		//更新银币数
		
		outstream->Clear();
		outstream->Write(type);
		outstream->Write((short)ROOM_CMD_UPDATEMONEY);
		outstream->Write(0);
		outstream->Write(pClient->gold);
		outstream->Write(pClient->silver);
		outstream->Flush();
		if(g_pNetProxy->SendToSrv(pClient, *outstream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::DoLuckNotify:idx=%d SendToSrv error",pClient->m_idx);
			pClient->LeaveRoom();
		}
		*/

		//经验改变通知
		outstream->Clear();
		outstream->Write((short)HALL_CMD_EXP_CHG_R2L);
		outstream->Write(0);
		outstream->Write((char)ROOM_TYPE_ROOM);
		outstream->Write((int)pClient->m_idx);	//用户idx
		outstream->Write(experience);			//经验值
		outstream->Write(level);				//经验等级
		outstream->Flush();
		
		HallSvrClient *pHallSvr = g_pNetProxy->GetHallSvr();
		if(pHallSvr!=NULL)
			pHallSvr->AddBuf(outstream->GetData(), outstream->GetSize());	
		else
			AC_ERROR("DbagentDataDecoder::DoScore:pHallSvr is NULL");

		
		//用户等级变化后通知房间内用户及大厅
		if(level!=pClient->m_level)
		{
			pClient->m_level=level;
			BinaryWriteStream* roomstream=StreamFactory::InstanceWriteStream();
			roomstream->Write(type);
			roomstream->Write((short)ROOM_CMD_SB_LEVEL);
			roomstream->Write(0);
			roomstream->Write((int)pClient->m_idx);		//用户idx
			roomstream->Write(pClient->m_level);		//经验等级
			roomstream->Flush();

			if(g_pNetProxy->BroadcastSBCmd(proominfo, *roomstream)==-1)
			{
			        AC_ERROR("DbagentDataDecoder::DoScore:g_pNetProxy->BroadcastSBCmd error");
			}
			
			BinaryWriteStream* lobbystream=StreamFactory::InstanceWriteStream();
			lobbystream->Write((short)HALL_CMD_CHANGELEVEL);
			lobbystream->Write(0);
			lobbystream->Write((char)ROOM_TYPE_ROOM);
			lobbystream->Write((int)pClient->m_idx);	//用户idx
			lobbystream->Write(pClient->m_level);	//经验等级
			lobbystream->Flush();

			HallSvrClient *pHallSvr = g_pNetProxy->GetHallSvr();
			if(pHallSvr!=NULL)
				pHallSvr->AddBuf(lobbystream->GetData(), lobbystream->GetSize());	
			else
				AC_ERROR("DbagentDataDecoder::DoScore:pHallSvr is NULL");
	
		}
	}
	else
	{
		AC_ERROR("DbagentDataDecoder::DoScore:result =%d",result);
		
	}
	
	return 0;
}

int DbagentDataDecoder::DoUpdateRoomInfo(Dbresultdata* pdata,short rownum,short colnum,BinaryReadStream* infostream)
{

	AC_DEBUG("DbagentDataDecoder::DoUpdateRoomInfo:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);
	

	int flag=0;	//室主更改标识
	
	ROOM_INFO* pRoominfo=g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(pRoominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoUpdateRoomInfo:roomid=%d not exist",pdata->roomid);
		return -1;
	}
	if(rownum!=1)
	{
		AC_ERROR("DbagentDataDecoder::DoUpdateRoomInfo:rownum=%d error",rownum);
		return -1;
	}

	char infobuf[1024];
	size_t curlen;
	for(int i = 0;i < rownum;i++)
	{
		int roomid = 0;
		for(int j = 0;j < colnum;j++)
		{
			if(!infostream->Read(infobuf,sizeof(infobuf),curlen))
			{
				AC_ERROR("DbagentDataDecoder::DoUpdateRoomInfo:read ptr error");
				return -1;
			}
			infobuf[curlen] = 0;
			if(j == 0)
			{
				roomid = atoi(infobuf);
				pRoominfo->roomid = roomid;
			}
			/*
			if(j == 1)
			{
				int hallid = atoi(infobuf);

			}
			*/
			if(j == 2)
           		 {
            			if(infobuf!=NULL&&curlen<sizeof(pRoominfo->roomname))
            			{
					strncpy(pRoominfo->roomname, infobuf,curlen);
					pRoominfo->roomname[curlen]=0;
            			}
				else if(infobuf!=NULL&&curlen>=sizeof(pRoominfo->roomname))
				{
					strncpy(pRoominfo->roomname, infobuf,sizeof(pRoominfo->roomname)-1);
					pRoominfo->roomname[sizeof(pRoominfo->roomname)-1]=0;
				}
				else 
					memset(pRoominfo->roomname,0,sizeof(pRoominfo->roomname));	
            		}
			if(j == 3)
	            {
	            		if(infobuf!=NULL&&curlen<sizeof(pRoominfo->passwd))
	            		{
					strncpy(pRoominfo->passwd, infobuf,curlen);
					pRoominfo->passwd[curlen]=0;
	            		}
				else 
					memset(pRoominfo->passwd,0,sizeof(pRoominfo->passwd));		
	            }
			if(j == 4)
				pRoominfo->type = atoi(infobuf);
			if(j == 5)
				pRoominfo->state = atoi(infobuf);
 			if(j == 6)
			{	
				int ownidx=atoi(infobuf);
				if(ownidx!=(int)pRoominfo->ownidx )
				{
					//原室主从管理员及会员列表中删除
					map<int,ROOM_MANAGER>::iterator itM_old = pRoominfo->managerlist.find(pRoominfo->ownidx);
					map<int,int>::iterator itVIP_old= pRoominfo->userlistVIP.find(pRoominfo->ownidx);
					map<int,RoomClient*>::iterator itU_old =	pRoominfo->userlist.find(pRoominfo->ownidx);
					if(itM_old!=pRoominfo->managerlist.end())
						pRoominfo->managerlist.erase(itM_old);
					if(itVIP_old!=pRoominfo->userlistVIP.end())
						pRoominfo->userlistVIP.erase(itVIP_old);
					//原室主在线更改其身份
					if(itU_old!=pRoominfo->userlist.end())
					{
						RoomClient* pClient_old=itU_old->second;
						pClient_old->m_identity=USER_ID_NONE;
					}

					//新室主添加到管理员及会员列表中
					map<int,ROOM_MANAGER>::iterator itM_new = pRoominfo->managerlist.find(ownidx);
					map<int,int>::iterator itVIP_new= pRoominfo->userlistVIP.find(ownidx);
					map<int,RoomClient*>::iterator itU_new =	pRoominfo->userlist.find(ownidx);
					if(itM_new!=pRoominfo->managerlist.end())
						pRoominfo->managerlist.erase(itM_old);
					if(itVIP_new!=pRoominfo->userlistVIP.end())
						pRoominfo->userlistVIP.erase(itVIP_old);
					//新室主在线更改其身份
					if(itU_new!=pRoominfo->userlist.end())
					{
						RoomClient* pClient_new=itU_new->second;
						pClient_new->m_identity=USER_ID_OWNER;
					}
				
					pRoominfo->ownidx=ownidx;
					flag = 1;
				}
 			}
//			if(j==7){}	//房间创建时间
			if(j==8)
                		pRoominfo->isMicUpdown = atoi(infobuf);	//房间排序标志
			if(j == 9)
				pRoominfo->maxhuman = atoi(infobuf);
//			if(j==10){}	//房间到期时间
//			if(j==11){}	//自动审核会员申请
			if(j == 12)	//欢迎词
          		  {
            			//modify by jinguanfu 2010/1/27 房间公告改欢迎词
				if(infobuf!=NULL&&curlen<sizeof(pRoominfo->welcomeword))
				{
					strncpy(pRoominfo->welcomeword,infobuf,curlen);
					pRoominfo->welcomeword[curlen]=0;
				}
				else if(infobuf!=NULL&&curlen>=sizeof(pRoominfo->welcomeword))
				{
					strncpy(pRoominfo->welcomeword,infobuf,sizeof(pRoominfo->welcomeword)-1);
					pRoominfo->welcomeword[sizeof(pRoominfo->welcomeword)-1]=0;
				}
				else
					memset(pRoominfo->welcomeword,0,sizeof(pRoominfo->welcomeword));	
         		   }
//			if(j==13){}	//房间IP地址
//			if(j==14){}	//房间端口
	            if(j == 15)	//房间公告
	            {
	            		if(infobuf!=NULL&&curlen<sizeof(pRoominfo->content))
	            		{
					strncpy(pRoominfo->content, infobuf,curlen);
					pRoominfo->content[curlen]=0;
	            		}	
				else if(infobuf!=NULL&&curlen>=sizeof(pRoominfo->content))
				{
					strncpy(pRoominfo->content, infobuf,sizeof(pRoominfo->content)-1);
					pRoominfo->content[sizeof(pRoominfo->content)-1]=0;
				}
				else
					memset(pRoominfo->content,0,sizeof(pRoominfo->content));
	            }
			if(j==16)	//房间更新时间
			{
			}	
			if(j== 17)
			{
				if(infobuf!=NULL&&curlen<sizeof(pRoominfo->roomlogo))
				{
				 	strncpy(pRoominfo->roomlogo,infobuf,curlen);
					pRoominfo->roomlogo[curlen]=0;
				}
				else if(infobuf!=NULL&&curlen>=sizeof(pRoominfo->roomlogo))
				{
					strncpy(pRoominfo->roomlogo, infobuf,sizeof(pRoominfo->roomlogo)-1);
					pRoominfo->roomlogo[sizeof(pRoominfo->roomlogo)-1]=0;
				}
				else
					memset(pRoominfo->roomlogo,0,sizeof(pRoominfo->roomlogo));	
			}

			if(j== 18)//是否允许公聊标志位
			{
				int  chatflag =atoi(infobuf);
				if(chatflag == 1)
					pRoominfo->isPublicChat=true;
				else if(chatflag == 0)
					pRoominfo->isPublicChat=false;
			}
			if(j== 19)//用户进出信息显示标志位
			{
				int inoutflag = atoi(infobuf);
				if(inoutflag ==1)
					pRoominfo->isUserInOut = true;
				else if(inoutflag == 0)
					pRoominfo->isUserInOut = false;
			}
			if(j==20)//房间是否公开标志位
			{
				int useronlyflag = atoi(infobuf);
				if(useronlyflag ==1)
					pRoominfo->isUserOnly = true;
				else if(useronlyflag == 0)
					pRoominfo->isUserOnly = false;
			}
			if(j==21)//房间是否关闭标志位
			{
				int closeflag = atoi(infobuf);
				if(closeflag ==1)
					pRoominfo->isClose = true;
				else if(closeflag == 0)
					pRoominfo->isClose = false;
			}
			if(j==22)//房间允许人数
				pRoominfo->allowhuman=atoi(infobuf);
	
			if(j == colnum - 1)
			{
				AC_DEBUG("DbagentDataDecoder::DoUpdateRoomInfo:proominfo->roomid = %d proominfo->roomname = %s proominfo->passwd = %s proominfo->type = %d proominfo->state = %d proominfo->ownidx = %d proominfo->maxhuman = %d proominfo->welcomeword = %s proominfo->content = %s",
				pRoominfo->roomid,pRoominfo->roomname,pRoominfo->passwd,pRoominfo->type, pRoominfo->state, pRoominfo->ownidx,
				pRoominfo->maxhuman,pRoominfo->welcomeword,pRoominfo->content);
			}
		}
	}

	//房间内通知
	char context[64] = {0};
	BinaryWriteStream outstream(context,sizeof(context));
	int seq = 0;
	char type=65;
	outstream.Write(type);
	outstream.Write((short)ROOM_CMD_SB_ROOMINFO_UPDATE);
	outstream.Write(seq);
	outstream.Flush();	
	if(g_pNetProxy->BroadcastSBCmd(pRoominfo,outstream) == -1)
	{
		AC_ERROR("DbagentDataDecoder::DoUpdateRoomInfo:: ROOMINFO_UPDATE BroadcastSBCmd error");
		return 0;
	}

	//室主更改通知
	if(flag==1)
	{
		outstream.Clear();
		outstream.Write(type);
		outstream.Write((short)ROOM_CMD_SB_GIVE_OWNER);
		outstream.Write(seq);
		outstream.Write((int)pRoominfo->ownidx);
		outstream.Flush();	
		if(g_pNetProxy->BroadcastSBCmd(pRoominfo,outstream) == -1)
		{
			AC_ERROR("DbagentDataDecoder::DoUpdateRoomInfo: GIVE_OWNER BroadcastSBCmd error");
			return 0;
		}
	}
	return 0;
}
/* delete by jinguanfu 2010/8/17 后台取消更新房间内用户权限功能
int DbagentDataDecoder::DoUpdateRoomManger(Dbresultdata* pdata,short rownum,short colnum,BinaryReadStream* infostream)
{

	ROOM_INFO* pRoominfo=g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(pRoominfo == NULL&&pdata->roomid!=9999)
	{
		AC_ERROR("DbagentDataDecoder::DoUpdateRoomManger:roomid=%d not exist",pdata->roomid);
		return 0;
	}

	char infobuf[1024];
	size_t curlen;
	if(!infostream->Read(infobuf,sizeof(infobuf),curlen))
	{
		AC_ERROR("DbagentDataDecoder::DoUpdateRoomManger:read ptr error");
		return -1;
	}
	int idx=pdata->bopidx;
	int level = atoi(infobuf);	
	short cmd=0;

	if(pdata->roomid==9999)
	{
		map<int,int>::iterator itGM=g_pNetProxy->m_GM.find(idx);
		if(level == USER_ID_GM)
		{
			if(itGM==g_pNetProxy->m_GM.end())
				g_pNetProxy->m_GM.insert(make_pair(idx,level));
		}
		else if(level == USER_ID_NONE)
      		{
			map<int,int>::iterator itGM=g_pNetProxy->m_GM.find(idx);
			if(itGM!=g_pNetProxy->m_GM.end())
				g_pNetProxy->m_GM.erase(itGM);
      				
		}else
		{
			AC_ERROR("DbagentDataDecoder::DoUpdateRoomManger:room[%d] invalid level[%d]",
					pRoominfo->roomid,level);
		}
	}
	else
	{	

		map<int,int>::iterator itVJ=pRoominfo->vjlist.find(idx);
		map<int,int>::iterator itVJ_A=pRoominfo->vjlist_a.find(idx);
		map<int,int>::iterator itVIP=pRoominfo->userlistVIP.find(idx);
		map<int,ROOM_MANAGER>::iterator itM=pRoominfo->managerlist.find(idx);
		
		if(level == USER_ID_OWNER) //不能对室主进行操作
	       {
	          	AC_ERROR("DbagentDataDecoder::DoUpdateRoomManger:room[%d] have OWNER:[%d]",
						pRoominfo->roomid,pRoominfo->ownidx);
		
	       }
	       else if(level == USER_ID_OWNER_S)
	     	{
	        	if(pRoominfo->secondownidx==0)
	           	{
		           	pRoominfo->secondownidx=idx;
	           	}
			else if(pRoominfo->secondownidx2==0)
			{
				pRoominfo->secondownidx2=idx;
			}
			else
			{
				AC_ERROR("DbagentDataDecoder::DoUpdateRoomManger:room[%d] have two OWNER_S:[%d],[%d]",
					pRoominfo->roomid,pRoominfo->secondownidx,pRoominfo->secondownidx2);
			}

			//新增会员
			if(itVIP==pRoominfo->userlistVIP.end())
			{
				pRoominfo->userlistVIP.insert(make_pair(idx,idx));
			}
			//原权限中删除
			else
			{
				if(itVJ!=pRoominfo->vjlist.end())
					pRoominfo->vjlist.erase(itVJ);
				else if(itVJ_A!=pRoominfo->vjlist_a.end())
					pRoominfo->vjlist_a.erase(itVJ_A);
			}

			//更新管理员列表
			if(itM!=pRoominfo->managerlist.end())
			{
					ROOM_MANAGER rm=itM->second;
					rm.m_identity=level;
			}
			else
			{
					ROOM_MANAGER rm;
					rm.m_idx=idx;
					rm.m_identity=level;

					pRoominfo->managerlist.insert(make_pair(idx,rm));
			}

			cmd=ROOM_CMD_SB_GIVE_OUER_S;
			
	      }
		else if(level == USER_ID_VJ)
		{

			//新增到主持人列表
			if(itVJ==pRoominfo->vjlist.end())
			{
				pRoominfo->vjlist.insert(make_pair(idx,idx));
			}


			//新增会员
			if(itVIP==pRoominfo->userlistVIP.end())
			{
				pRoominfo->userlistVIP.insert(make_pair(idx,idx));
			}
			else
			{
				if(pRoominfo->secondownidx==idx)
					pRoominfo->secondownidx=0;
				else if(pRoominfo->secondownidx2==idx)
					pRoominfo->secondownidx2=0;
				else if(itVJ_A!=pRoominfo->vjlist_a.end())
					pRoominfo->vjlist_a.erase(itVJ_A);
			}

			//更新管理员列表
			if(itM!=pRoominfo->managerlist.end())
			{
					ROOM_MANAGER rm=itM->second;
					rm.m_identity=level;
			}
			else
			{
					ROOM_MANAGER rm;
					rm.m_idx=idx;
					rm.m_identity=level;

					pRoominfo->managerlist.insert(make_pair(idx,rm));
			}

			cmd=ROOM_CMD_SB_GIVE_VJ;
			
		}
		else if(level == USER_ID_VJ_A)
		{

			//新增到助理主持列表
			if(itVJ_A==pRoominfo->vjlist_a.end())
			{
				pRoominfo->vjlist_a.insert(make_pair(idx,idx));
			}

			//新增会员
			if(itVIP==pRoominfo->userlistVIP.end())
			{
				pRoominfo->userlistVIP.insert(make_pair(idx,idx));
			}
			//原权限列表中删除
			else
			{
				if(pRoominfo->secondownidx==idx)
					pRoominfo->secondownidx=0;
				else if(pRoominfo->secondownidx2==idx)
					pRoominfo->secondownidx2=0;
				else if(itVJ!=pRoominfo->vjlist.end())
					pRoominfo->vjlist.erase(itVJ);
			}

			//更新管理员列表
			if(itM!=pRoominfo->managerlist.end())
			{
					ROOM_MANAGER rm=itM->second;
					rm.m_identity=level;
			}
			else
			{
					ROOM_MANAGER rm;
					rm.m_idx=idx;
					rm.m_identity=level;

					pRoominfo->managerlist.insert(make_pair(idx,rm));
			}
			
			cmd=ROOM_CMD_SB_GIVE_VJ_A;
			
		}
		else if(level == USER_ID_VIP)
		{
			//新增会员
			if(itVIP==pRoominfo->userlistVIP.end())
			{
				pRoominfo->userlistVIP.insert(make_pair(idx,idx));
			}
			//从管理员降为会员
			else
			{
				if(pRoominfo->secondownidx==idx)
					pRoominfo->secondownidx=0;
				else if(pRoominfo->secondownidx2==idx)
					pRoominfo->secondownidx2=0;
				else if(itVJ!=pRoominfo->vjlist.end())
					pRoominfo->vjlist.erase(itVJ);
				else if(itVJ_A!=pRoominfo->vjlist_a.end())
					pRoominfo->vjlist_a.erase(itVJ_A);
			}

			//从管理员列表中删除
			if(itM!=pRoominfo->managerlist.end())
			{
				pRoominfo->managerlist.erase(itM);
			}

			cmd=ROOM_CMD_SB_GIVE_MEMBER;
			
		}
		else if(level == USER_ID_NONE)
	      	{
	      			//从会员列表中删除
				if(itVIP!=pRoominfo->userlistVIP.end())
					pRoominfo->userlistVIP.erase(itVIP);

				//从对应管理员列表中删除
				if(pRoominfo->secondownidx==idx)
					pRoominfo->secondownidx=0;
				else if(pRoominfo->secondownidx2==idx)
					pRoominfo->secondownidx2=0;
				else if(itVJ!=pRoominfo->vjlist.end())
					pRoominfo->vjlist.erase(itVJ);
				else if(itVJ_A!=pRoominfo->vjlist_a.end())
					pRoominfo->vjlist_a.erase(itVJ_A);

				//从管理员列表中删除
				if(itM!=pRoominfo->managerlist.end())
				{
					pRoominfo->managerlist.erase(itM);
				}

				cmd=ROOM_CMD_SB_REMOVE_USER;
				
		}

		//用户在房间内发广播
		map<int,RoomClient*>::iterator itU=pRoominfo->userlist.find(idx);
		if(itU!=pRoominfo->userlist.end())
		{
			char context[64] = {0};
			BinaryWriteStream outstream(context,sizeof(context));
			int seq = 0;
			char type=65;
			
			outstream.Write(type);
			outstream.Write(cmd);
			outstream.Write(seq);
			outstream.Write(pdata->opidx);		//操作者
			outstream.Write(pdata->bopidx);	//被操作者
			outstream.Write((char)level);		//权限
			outstream.Flush();	
			if(g_pNetProxy->BroadcastSBCmd(pRoominfo,outstream) == -1)
			{
				AC_ERROR("DbagentDataDecoder::DoUpdateRoomManger::g_pNetProxy->BroadcastSBCmd error");
				return 0;
			}
		}
		
	}


	return 0;
}
*/

int DbagentDataDecoder::DoSendFireworks(Dbresultdata* pdata,BinaryReadStream* infostream)
{
	
	AC_DEBUG("DbagentDataDecoder::DoSendFireworks:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);

	if(pdata->roomid == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoSendFireworks:roomid error");
		return -1;
	}

	if(pdata->opidx == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoSendFireworks:idx error");
		return -1;
	}

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(proominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoSendFireworks:proominfo error");
		return -1;
	}

	//数据库返回值
	char infobuf[1024];
	size_t clen;
	
	if(!infostream->Read(infobuf,sizeof(infobuf),clen))
	{
		AC_ERROR("DbagentDataDecoder::DoSendFireworks:read result error");
		return -1;
	}
	infobuf[clen]=0;

	char* saveptr=NULL;
	char* token=strtok_r(infobuf,"|",&saveptr);
	if(token==NULL||saveptr==NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoSendFireworks:parse result error");
		return 0;
	}
	int result=atoi(token);
	int s_gold=0;		//发送者金币余额
	int s_silver=0;	//发送者银币余额 
	char speakflag=0;  //发送广播标志位
	char r_userinfo[1024]={0};
	//	1--交易金额为空
      //	0--成功,后面跟"金币余额|银币余额|广播类型"
      //  -1--金额不足
      //	2--无此交易类型
	if(result==DBRESULT_GIFT_SUCCESS)
	{
		token=strtok_r(NULL,"|",&saveptr);
		if(token==NULL||saveptr==NULL)
		{
			AC_ERROR("DbagentDataDecoder::DoSendFireworks:parse s_gold error");
			return 0;
		}
		s_gold=atoi(token);		//送礼者金币余额
		token=strtok_r(NULL,"|",&saveptr);
		if(token==NULL||saveptr==NULL)
		{
			AC_ERROR("DbagentDataDecoder::DoSendFireworks:parse s_silver error");
			return 0;
		}
		s_silver=atoi(token);		//送礼者银币余额
		
		token=strtok_r(NULL,"|",&saveptr);
		if(token==NULL)
		{
			AC_ERROR("DbagentDataDecoder::DoSendFireworks:parse speakflag error");
			return 0;
		}
		speakflag=atoi(token);	//送烟花不进行大厅以上广播

		
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		char type=65;
		outstream->Write(type);
		outstream->Write((short)pdata->cmd);
		outstream->Write(pdata->seq);
		outstream->Write(pdata->cate_idx);		//礼物ID
		outstream->Write(pdata->number);		//礼物数量
		outstream->Write(result);	//数据库操作结果
		outstream->Write(s_gold);
		outstream->Write(s_silver);
		outstream->Flush();

		map<int,RoomClient*>::iterator itR=proominfo->userlist.find(pdata->opidx);
		if(itR!=proominfo->userlist.end())
		{
			RoomClient* pClient=itR->second;
			if(g_pNetProxy->SendToSrv(pClient, *outstream))
			{
				AC_ERROR("DbagentDataDecoder::DoSendFireworks:SendToSrv error");
			}
			//更新发送的金币数和银币数
			pClient->gold=s_gold;
			pClient->silver=s_silver;
			
		}
		int outseq = 0;
		static const char* spname = {"DBMW_RecevieGift_HZSTAR"};//存储过程填充

		map<int,GIFT_INFO>::iterator itGiftInfo=g_pNetProxy->m_GiftInfo.find(pdata->cate_idx);
		if(itGiftInfo==g_pNetProxy->m_GiftInfo.end())
		{
			AC_ERROR("DbagentDataDecoder::DoSendFireworks:Gift not exist GiftID=%d",pdata->cate_idx);
			return 0;
		}
		GIFT_INFO* pGiftinfo=(GIFT_INFO*)&(itGiftInfo->second);
		//房间内在线管理员接收礼物
		if(pGiftinfo->type==GIFT_TYPE_SFIRE)
		{
			speakflag=2;		//大厅广播
			//一次返值的最大人数为50
			short mangernum=proominfo->managerlist_online.size();
			int count=0;
			map<int,ROOM_MANAGER>::iterator itM=proominfo->managerlist_online.begin();
			while(itM!=proominfo->managerlist_online.end())
			{
	
				char temp[16]={0};
				sprintf(temp,"%d,",itM->first);
				strncat(r_userinfo,temp,strlen(temp));
				itM++;
				count++;
				
				if(count==50)
				{
					AC_DEBUG("DbagentDataDecoder::DoSendFireworks:r_userinfo= %s",r_userinfo);

					BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
					if(pDBSvr!=NULL)
					{
						outseq=g_pNetProxy->GetCounter()->Get();
						outstream->Clear();
						outstream->Write((short)CMD_CALLSP);
						outstream->Write(outseq);
						outstream->Write(spname,strlen(spname));
						outstream->Write((short)6);
						outstream->Write((char)PT_INPUT);
						outstream->Write((char)PDT_INT);
						outstream->Write((int)pdata->opidx);			//赠送者idx
						outstream->Write((char)PT_INPUT);
						outstream->Write((char)PDT_VARCHAR);
						outstream->Write(r_userinfo,strlen(r_userinfo));	//接收者idx
						outstream->Write((char)PT_INPUT);
						outstream->Write((char)PDT_INT);
						outstream->Write((int)proominfo->roomid);		//房间id
						outstream->Write((char)PT_INPUT);
						outstream->Write((char)PDT_INT);
						outstream->Write((int)pdata->cate_idx);			//礼物idx
						outstream->Write((char)PT_INPUT);
						outstream->Write((char)PDT_INT);
						outstream->Write((int)pdata->number);			//礼物数量
						outstream->Write((char)PT_INPUT);
						outstream->Write((char)PDT_INT);
						outstream->Write((int)mangernum);				//均分基数
						outstream->Flush();

						Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
						if(data != NULL) 
						{
							if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1)
							{
								data->roomid = (int)proominfo->roomid;
								data->opidx = (int)pdata->opidx;
								data->bopidx = 0;
								data->cate_idx = pdata->cate_idx;
								data->number=pdata->number;
								data->cmd = (short)ROOM_CMD_RECV_FIREWORKS;
								data->seq=pdata->seq;
								data->outseq=outseq;
								data->SetReactor(g_pNetProxy->GetReactor());
								data->RegisterTimer(DB_TIMEOUT);
								g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
							}
							else
							{
								AC_ERROR("DbagentDataDecoder::DoSendFireworks:pDBSvr->AddBuf() error");
								g_pNetProxy->DestroyDBResult(data);
								return 0;
							}
						}
						else
						{
							AC_ERROR("DbagentDataDecoder::DoSendFireworks:CreateDBResultdata error,data=%x",data);
							return 0;
						}

						count=0;
						memset(r_userinfo,0,1024);
					}
					else
					{
						AC_ERROR("DbagentDataDecoder::DoSendFireworks:CreateDBResultdata error");
						return 0;
					}
				}
			}
			AC_DEBUG("DbagentDataDecoder::DoSendFireworks:r_userinfo= %s",r_userinfo);
			if(count<50&&count>0)
			{
				BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
				if(pDBSvr!=NULL)
				{
					outseq=g_pNetProxy->GetCounter()->Get();
					outstream->Clear();
					outstream->Write((short)CMD_CALLSP);
					outstream->Write(outseq);
					outstream->Write(spname,strlen(spname));
					outstream->Write((short)6);
					outstream->Write((char)PT_INPUT);
					outstream->Write((char)PDT_INT);
					outstream->Write((int)pdata->opidx);			//赠送者idx
					outstream->Write((char)PT_INPUT);
					outstream->Write((char)PDT_VARCHAR);
					outstream->Write(r_userinfo,strlen(r_userinfo));	//接收者idx
					outstream->Write((char)PT_INPUT);
					outstream->Write((char)PDT_INT);
					outstream->Write((int)proominfo->roomid);		//房间id
					outstream->Write((char)PT_INPUT);
					outstream->Write((char)PDT_INT);
					outstream->Write((int)pdata->cate_idx);			//礼物idx
					outstream->Write((char)PT_INPUT);
					outstream->Write((char)PDT_INT);
					outstream->Write((int)pdata->number);			//礼物数量
					outstream->Write((char)PT_INPUT);
					outstream->Write((char)PDT_INT);
					outstream->Write((int)mangernum);				//均分基数
					outstream->Flush();

					Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
					if(data != NULL) 
					{
						if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1)
						{
							data->roomid = (int)proominfo->roomid;
							data->opidx = (int)pdata->opidx;
							data->bopidx = 0;
							data->cate_idx = pdata->cate_idx;
							data->number=pdata->number;
							data->cmd =  (short)ROOM_CMD_RECV_FIREWORKS;
							data->seq=pdata->seq;
							data->outseq=outseq;
							data->SetReactor(g_pNetProxy->GetReactor());
							data->RegisterTimer(DB_TIMEOUT);
							g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
						}
						else
						{
							AC_ERROR("DbagentDataDecoder::DoSendFireworks:pDBSvr->AddBuf() error");
							g_pNetProxy->DestroyDBResult(data);
							return 0;
						}
					}
					else
					{
						AC_ERROR("DbagentDataDecoder::DoSendFireworks:CreateDBResultdata error,data=%x",data);
						return 0;
					}
				}
				else
				{
					AC_ERROR("DbagentDataDecoder::DoSendFireworks:pDBSvr=%x ",pDBSvr);
					return 0;
				}
			}
		}
		//房间内在线用户接收礼物
		else if(pGiftinfo->type==GIFT_TYPE_BFIRE)
		{
			speakflag=3;		//全区广播
			//一次返值的最大人数为50
			short usernum=proominfo->userlist.size()+proominfo->vuserlist.size();
			map<int,RoomClient*>::iterator itU=proominfo->userlist.begin();
			for(;itU!=proominfo->userlist.end();itU++)
			{
				RoomClient* pClient= itU->second;
				if(pClient->m_identity==USER_ID_GM)
					usernum--;
			}
			int count=0;
			itU=proominfo->userlist.begin();
			while(itU!=proominfo->userlist.end())
			{
				//GM 不在烟花收益名单内
				RoomClient* pClient= itU->second;
				if(pClient->m_identity==USER_ID_GM)
				{
					itU++;
					continue;
				}
				char temp[16]={0};
				sprintf(temp,"%d,",pClient->m_idx);
				strncat(r_userinfo,temp,strlen(temp));
				itU++;
				count++;

				if(count==50)
				{
					AC_DEBUG("DbagentDataDecoder::DoSendFireworks:r_userinfo= %s",r_userinfo);
					BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
					if(pDBSvr!=NULL)
					{
						outseq=g_pNetProxy->GetCounter()->Get();
						outstream->Clear();
						outstream->Write((short)CMD_CALLSP);
						outstream->Write(outseq);
						outstream->Write(spname,strlen(spname));
						outstream->Write((short)6);
						outstream->Write((char)PT_INPUT);
						outstream->Write((char)PDT_INT);
						outstream->Write((int)pdata->opidx);			//赠送者idx
						outstream->Write((char)PT_INPUT);
						outstream->Write((char)PDT_VARCHAR);
						outstream->Write(r_userinfo,strlen(r_userinfo));	//接收者idx
						outstream->Write((char)PT_INPUT);
						outstream->Write((char)PDT_INT);
						outstream->Write((int)proominfo->roomid);		//房间id
						outstream->Write((char)PT_INPUT);
						outstream->Write((char)PDT_INT);
						outstream->Write((int)pdata->cate_idx);			//礼物idx
						outstream->Write((char)PT_INPUT);
						outstream->Write((char)PDT_INT);
						outstream->Write((int)pdata->number);			//礼物数量
						outstream->Write((char)PT_INPUT);
						outstream->Write((char)PDT_INT);
						outstream->Write((int)usernum);				//均分基数
						outstream->Flush();

						Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
						if(data != NULL) 
						{
							if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1)
							{
								data->roomid = (int)proominfo->roomid;
								data->opidx = (int)pdata->opidx;
								data->bopidx = 0;
								data->cate_idx = pdata->cate_idx;
								data->number=pdata->number;
								data->cmd =  (short)ROOM_CMD_RECV_FIREWORKS;
								data->seq=pdata->seq;
								data->outseq=outseq;
								data->SetReactor(g_pNetProxy->GetReactor());
								data->RegisterTimer(DB_TIMEOUT);
								g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
							}
							else
							{
								AC_ERROR("DbagentDataDecoder::DoSendFireworks:pDBSvr->AddBuf() error");
								g_pNetProxy->DestroyDBResult(data);
								return 0;
							}
						}
						else
						{
							AC_ERROR("DbagentDataDecoder::DoSendFireworks:CreateDBResultdata error,data=%x",data);
							return 0;
						}

						count=0;
						memset(r_userinfo,0,1024);
					}
					else
					{
						AC_ERROR("DbagentDataDecoder::DoSendFireworks:pDBSvr=%x",pDBSvr);
						return 0;
					}
					
				}
			}

			if(count<50&&count>0)
			{
				AC_DEBUG("DbagentDataDecoder::DoSendFireworks:r_userinfo= %s",r_userinfo);
				BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
				if(pDBSvr!=NULL)
				{
					outseq=g_pNetProxy->GetCounter()->Get();
					outstream->Clear();
					outstream->Write((short)CMD_CALLSP);
					outstream->Write(outseq);
					outstream->Write(spname,strlen(spname));
					outstream->Write((short)6);
					outstream->Write((char)PT_INPUT);
					outstream->Write((char)PDT_INT);
					outstream->Write((int)pdata->opidx);			//赠送者idx
					outstream->Write((char)PT_INPUT);
					outstream->Write((char)PDT_VARCHAR);
					outstream->Write(r_userinfo,strlen(r_userinfo));	//接收者idx
					outstream->Write((char)PT_INPUT);
					outstream->Write((char)PDT_INT);
					outstream->Write((int)proominfo->roomid);		//房间id
					outstream->Write((char)PT_INPUT);
					outstream->Write((char)PDT_INT);
					outstream->Write((int)pdata->cate_idx);			//礼物idx
					outstream->Write((char)PT_INPUT);
					outstream->Write((char)PDT_INT);
					outstream->Write((int)pdata->number);			//礼物数量
					outstream->Write((char)PT_INPUT);
					outstream->Write((char)PDT_INT);
					outstream->Write((int)usernum);				//均分基数
					outstream->Flush();

					Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
					if(data != NULL) 
					{
						if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1)
						{
							data->roomid = (int)proominfo->roomid;
							data->opidx = (int)pdata->opidx;
							data->bopidx = 0;
							data->cate_idx = pdata->cate_idx;
							data->number=pdata->number;
							data->cmd =  (short)ROOM_CMD_RECV_FIREWORKS;
							data->seq=pdata->seq;
							data->outseq=outseq;
							data->SetReactor(g_pNetProxy->GetReactor());
							data->RegisterTimer(DB_TIMEOUT);
							g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
						}
						else
						{
							AC_ERROR("DbagentDataDecoder::DoSendFireworks:pDBSvr->AddBuf() error");
							g_pNetProxy->DestroyDBResult(data);
							return 0;
						}
					}
					else
					{
						AC_ERROR("DbagentDataDecoder::DoSendFireworks:CreateDBResultdata error,data=%x",data);
						return 0;
					}
				}
				else
				{
					AC_ERROR("DbagentDataDecoder::DoSendFireworks:pDBSvr=%x ",pDBSvr);
					return 0;
				}
			}
		}
		
	}
	else
	{
		AC_DEBUG("DbagentDataDecoder::DoSendFireworks:dbresult =%d",result);
		
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		char type=65;
		outstream->Write(type);
		outstream->Write((short)pdata->cmd);
		outstream->Write(pdata->seq);
		//add by jinguanfu 2010/4/30 送礼失败时被踢出房间BUG<begin>
		outstream->Write(pdata->cate_idx);		//礼物ID
		outstream->Write(pdata->number);		//礼物数量
		//outstream->Write(pdata->bopidx);		//收礼者idx
		//add by jinguanfu 2010/4/30 送礼失败时被踢出房间BUG<end>
		outstream->Write(result);
		outstream->Flush();

		map<int,RoomClient*>::iterator itR=proominfo->userlist.find(pdata->opidx);
		RoomClient* pClient=itR->second;
		if(g_pNetProxy->SendToSrv(pClient, *outstream))
		{
			AC_ERROR("DbagentDataDecoder::DoSendFireworks:SendToSrv error");
		}
		return 0;
	}

	//房间内发送送烟花通知
	//char outbuf[256]64] = {0};
	BinaryWriteStream* giftstream=StreamFactory::InstanceWriteStream();
	char type = 65;
	giftstream->Write(type);
	giftstream->Write((short)ROOM_CMD_SB_SEND_FIREWORKS);
	giftstream->Write((int)0);
	giftstream->Write(pdata->opidx);		//送礼者
	giftstream->Write(pdata->cate_idx);		//礼物id
	giftstream->Write(pdata->number);		//礼物总数
	giftstream->Flush();
	/*
	if(g_pNetProxy->BroadcastSBCmd(proominfo,*giftstream) == -1)
	{
		AC_ERROR("DbagentDataDecoder::SendFlower:g_pNetProxy->BroadcastSBCmd error");
	}
	*/
	char outbuf[50];                     
	int outbuflen = sizeof(outbuf);  
    if(!StreamCompress(giftstream->GetData(),(int)giftstream->GetSize(),outbuf,outbuflen))
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
			//if(StreamEncrypt(giftstream->GetData(),(int)giftstream->GetSize(),outbuf,outbuflen,psend->m_sessionkey,1))
            {
				if(psend->AddBuf(outbuf,outbuflen) == -1)
				{
                    //psend->ErrorClose();
					AC_ERROR("psend->AddBuf Error");
				}
            }
		}
	}

	//大厅广播
	giftstream->Clear();
	giftstream->Write((short)HALL_CMD_FIREWORKS_R2L);
	giftstream->Write((int)0);
	giftstream->Write(pdata->opidx);		//送礼者
	giftstream->Write(pdata->roomid);		//房间ID
	giftstream->Write(pdata->cate_idx);		//礼物id
	giftstream->Write(speakflag);
	giftstream->Flush();

	AC_DEBUG("DbagentDataDecoder::DoSendFireworks BroadToHALL!");
	HallSvrClient *pHallSvr = g_pNetProxy->GetHallSvr();
	if(pHallSvr!=NULL)
		pHallSvr->AddBuf(giftstream->GetData(), giftstream->GetSize());	
	else
		AC_ERROR("DbagentDataDecoder::DoSendFireworks:pHallSvr is NULL");
	

	return 0;
}


int DbagentDataDecoder::DoRecvFireworks(Dbresultdata* pdata,short rownum,short colnum,BinaryReadStream* infostream)
{

	AC_DEBUG("DbagentDataDecoder::DoRecvFireworks: rownum=%d,colnum=%d",rownum,colnum);

	ROOM_INFO* pRoominfo=g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(pRoominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoRecvFireworks:roomid=%d not exist",pdata->roomid);
		return -1;
	}

	char infobuf[1024];
	size_t curlen;
	for(int i = 0;i < rownum;i++)
	{
		int idx=0;
		int gold=0;
		int silver=0;
		for(int j = 0;j < colnum;j++)
		{
			if(!infostream->Read(infobuf,sizeof(infobuf),curlen))
			{
				AC_ERROR("DbagentDataDecoder::DoRecvFireworks:read ptr error");
				return -1;
			}
			infobuf[curlen] = 0;
			if(j == 0)		//idx
			{
				idx = atoi(infobuf);
			}
			
			if(j == 1)		//金币数
			{
				gold = atoi(infobuf);
			}
			
			if(j == 2)		//银币数
			{
         			silver = atoi(infobuf);
            		}
	
			if(j == colnum - 1)
			{
				AC_DEBUG("DbagentDataDecoder::DoRecvFireworks:idx=%d,gold=%d,silver=%d",
				idx,gold,silver);

				map<int,RoomClient*>::iterator itclient=pRoominfo->userlist.find(idx);
				if(itclient!=pRoominfo->userlist.end())
				{
					RoomClient*  pRoomClient=itclient->second;
					pRoomClient->gold=gold;
					pRoomClient->silver=silver;

					BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
					char type=65;
					outstream->Write(type);
					outstream->Write((short)ROOM_CMD_UPDATEMONEY);
					outstream->Write(0);
					outstream->Write(gold);
					outstream->Write(silver);
					outstream->Flush();
					if(g_pNetProxy->SendToSrv(pRoomClient, *outstream,1,1))
					{
						AC_ERROR("DbagentDataDecoder::DoRecvFireworks:SendToSrv error");
					}

					//通知大厅发送喇叭消息
					BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
					outstream2->Write((short)HALL_CMD_UPDATEGOLD_L2R);
					outstream2->Write((int)0);
					outstream2->Write((char)ROOM_TYPE_ROOM);	//ADD BY JINGUANFU 2010/7/9
					outstream2->Write(idx);		//接收烟花者
					outstream2->Write(gold);		//收礼者金币余额
					outstream2->Write(silver);		//收礼者银币余额
					outstream2->Flush();
	
					AC_DEBUG("DbagentDataDecoder::DoRecvFireworks BroadToSver!");
					HallSvrClient *pHallSvr = g_pNetProxy->GetHallSvr();
					if(pHallSvr!=NULL)
						pHallSvr->AddBuf(outstream2->GetData(), outstream2->GetSize());
					else
						AC_ERROR("DbagentDataDecoder::DoRecvFireworks:pHallSvr is NULL");

				}
			}

		}
	}
	return 0;
}

//幸运礼物抽奖反馈
int DbagentDataDecoder::DoRunLotte(Dbresultdata* pdata,BinaryReadStream* infostream)
{
	AC_DEBUG("DbagentDataDecoder::DoRunLotte:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);
	if(pdata->roomid == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoRunLotte:roomid error");
		return -1;
	}

	if(pdata->opidx == 0 || pdata->bopidx == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoRunLotte:idx error");
		return -1;
	}

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(proominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoRunLotte:proominfo error");
		return -1;
	}

      //	0--成功,后面跟"金币余额|银币余额|中奖倍率,中奖倍率..."
      //	2--失败,无此交易类型 "金币余额|银币余额"
	//数据库返回值
	char infobuf[1024];
	size_t clen;
	if(!infostream->Read(infobuf,sizeof(infobuf),clen))
	{
		AC_ERROR("DbagentDataDecoder::DoRunLotte:read result error");
		return -1;
	}
	infobuf[clen]=0;

	char* saveptr=NULL;
	char* token=strtok_r(infobuf,"|",&saveptr);
	if(token==NULL||saveptr==NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoRunLotte:parse result error");
		return 0;
	}
	int result=atoi(token);
	int s_gold=0;
	int s_silver=0;

	token=strtok_r(NULL,"|",&saveptr);
	if(token==NULL||saveptr==NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoRunLotte:parse s_gold error");
		return 0;
	}
	s_gold=atoi(token);		//送礼者金币余额
	token=strtok_r(NULL,"|",&saveptr);
	if(token==NULL||saveptr==NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoRunLotte:parse s_silver error");
		return 0;
	}
	s_silver=atoi(token);		//送礼者银币余额
	
	map<int,RoomClient*>::iterator itclient=proominfo->userlist.find(pdata->opidx);
	if(itclient!=proominfo->userlist.end())
	{
		RoomClient*  pRoomClient=itclient->second;
		pRoomClient->gold=s_gold;
		pRoomClient->silver=s_silver;
		
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		char type=65;
		outstream->Write(type);
		outstream->Write((short)ROOM_CMD_UPDATEMONEY);
		outstream->Write(0);
		outstream->Write(s_gold);
		outstream->Write(s_silver);
		outstream->Flush();
		if(g_pNetProxy->SendToSrv(pRoomClient, *outstream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::DoRunLotte:SendToSrv error");
			//pRoomClient->LeaveRoom();
		}

	}

	//同步用户金币与银币余额
	BinaryWriteStream* lobbystream=StreamFactory::InstanceWriteStream();
	lobbystream->Write((short)HALL_CMD_UPDATEGOLD_L2R);
	lobbystream->Write((int)0);
	lobbystream->Write((char)ROOM_TYPE_ROOM);	//ADD BY JINGUANFU 2010/7/9
	lobbystream->Write(pdata->opidx);			//中奖者
	lobbystream->Write(s_gold);				//中奖者金币余额
	lobbystream->Write(s_silver);				//中奖者银币余额
	lobbystream->Flush();
	AC_DEBUG("DbagentDataDecoder::DoRunLotte BroadToSver!");
	HallSvrClient *pHallSvr = g_pNetProxy->GetHallSvr();
	if(pHallSvr!=NULL)	
		pHallSvr->AddBuf(lobbystream->GetData(), lobbystream->GetSize());	
	else
		AC_ERROR("DbagentDataDecoder::DoRunLotte:pHallSvr is NULL");
	
	//中奖通知
	if(result==0)
	{
		/*
		map<int,GIFT_INFO>::iterator itGift=g_pNetProxy->m_GiftInfo.find(pdata->cate_idx);
		if(itGift==g_pNetProxy->m_GiftInfo.end())
		{
			AC_ERROR("DbagentDataDecoder::DoRunLotte:GiftID=%d not exist",pdata->cate_idx);
			return 0;
		}

		GIFT_INFO giftinfo=itGift->second;
		*/
		int lotte=0;
		token=strtok_r(NULL,"|",&saveptr);
		if(token==NULL||saveptr==NULL)
		{
			AC_ERROR("DbagentDataDecoder::DoRunLotte:parse tempbuf error");
			return 0;
		}

		AC_DEBUG("DbagentDataDecoder::DoRunLotte: tempbuf [%s]",token);
		
		token=strtok_r(token,",",&saveptr);
		for(;;)
		{
			if(token==NULL||saveptr==NULL)
			{
				AC_DEBUG("DbagentDataDecoder::DoRunLotte: tempbuf is null");
				break;
			}
			lotte=atoi(token);
			if(lotte!=0)
			{
				//房间内中奖广播
				BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
				char type=65;
				outstream->Write(type);
				outstream->Write((short)ROOM_CMD_SB_LUCKY);
				outstream->Write(0);
				outstream->Write(pdata->opidx);		//抽奖者
				outstream->Write(pdata->cate_idx);		//礼物ID
				outstream->Write(lotte);				//中奖倍率
				//outstream->Write(lotte*giftinfo.price);	//中奖金额
				outstream->Flush();
				if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream))
				{
					AC_ERROR("DbagentDataDecoder::DoRunLotte:SendToSrv error");
				}

				//大于50倍时，发送大厅通知
				if(lotte>=50)
				{
					BinaryWriteStream* lobbystream2=StreamFactory::InstanceWriteStream();
					lobbystream2->Write((short)HALL_CMD_LUCK_R2L);
					lobbystream2->Write((int)0);
					lobbystream2->Write((char)ROOM_TYPE_ROOM);	//ADD BY JINGUANFU 2010/7/9
					lobbystream2->Write(pdata->opidx);			//中奖者
					lobbystream2->Write(pdata->roomid);		//房间ID
					lobbystream2->Write(pdata->cate_idx);		//礼物ID
					lobbystream2->Write(lotte);					//中奖倍率
					lobbystream2->Flush();
					AC_DEBUG("DbagentDataDecoder::DoRunLotte BroadToSver!");
					HallSvrClient *pHallSvr = g_pNetProxy->GetHallSvr();
					if(pHallSvr!=NULL)
						pHallSvr->AddBuf(lobbystream2->GetData(), lobbystream2->GetSize());
					else
						AC_ERROR("DbagentDataDecoder::DoRunLotte:pHallSvr is NULL");
				}
			}
			token=strtok_r(NULL,",",&saveptr);
			
		}
	}

	AC_DEBUG("DbagentDataDecoder::DoRunLotte	:OUT");

	return 0;
}

//幸运礼物送完后的抽奖
int DbagentDataDecoder::DoRunLottery(Dbresultdata* pdata/*,BinaryReadStream* infostream*/)
{
	AC_DEBUG("DbagentDataDecoder::DoRunLottery:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);
	
	int multiple=0;
	char multistr[500]={0};
	int multilen=0;

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(proominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoRunLottery:proominfo error");
		return 0;
	}


	for(int i=0;i<pdata->number;i++)
	{
	
		srand((unsigned)(time(NULL)/g_pNetProxy->GetRandSeed())); //生成种子
		int r=rand()%g_pNetProxy->m_luckbase;	
				
		map<int,int>::iterator itLuck=g_pNetProxy->m_LuckConf.begin();
		for(;itLuck!=g_pNetProxy->m_LuckConf.end();itLuck++)
		{
			int prob=itLuck->second;
			
			if(r%(g_pNetProxy->m_luckbase/prob)==0)
			{
				multiple=itLuck->first;
				char temp[8]={0};
				sprintf(temp,"%d,",multiple);
				strcat(multistr,temp);
				multilen=multilen+strlen(temp);
				break;
			}
		}
	}

	AC_DEBUG("DbagentDataDecoder::DoRunLottery:multistr=%s",multistr);
	if(multilen>=(int)sizeof(multistr))
		multistr[sizeof(multistr)-1]=0;
	else
		multistr[multilen]=0;
	
	if(strlen(multistr)>0)
	{	
		BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgentGift();
		if(pDBSvr!=NULL)
		{
			int outseq = g_pNetProxy->GetCounter()->Get();
			static const char* spname = {"DBMW_LuckBack_HZSTAR"};//存储过程填充
			BinaryWriteStream* dbstream=StreamFactory::InstanceWriteStream();
			dbstream->Write((short)CMD_CALLSP);
			dbstream->Write(outseq);
			dbstream->Write(spname,strlen(spname));
			dbstream->Write((short)4);
			dbstream->Write((char)PT_INPUT);
			dbstream->Write((char)PDT_INT);
			dbstream->Write((int)pdata->opidx);			//赠送者idx
			dbstream->Write((char)PT_INPUT);
			dbstream->Write((char)PDT_INT);
			dbstream->Write((int)proominfo->roomid);		//房间id
			dbstream->Write((char)PT_INPUT);
			dbstream->Write((char)PDT_INT);
			dbstream->Write((int)pdata->cate_idx);		//礼物idx
			dbstream->Write((char)PT_INPUT);
			dbstream->Write((char)PDT_VARCHAR);
			dbstream->Write(multistr,strlen(multistr));		//中奖倍数
			dbstream->Flush();
			Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
			if(data != NULL) 
			{
				if(pDBSvr->AddBuf(dbstream->GetData(), dbstream->GetSize())!=-1)
				{
					data->roomid = (int)proominfo->roomid;
					data->opidx = (int)pdata->opidx;
					data->bopidx = (int)pdata->bopidx;
					data->cate_idx = pdata->cate_idx;
					data->number=pdata->number;
					data->cmd = (short)ROOM_CMD_SB_LUCKY;
					data->seq=0;
					data->outseq=outseq;
					strncpy(data->content,multistr,strlen(multistr));
					data->SetReactor(g_pNetProxy->GetReactor());
					data->RegisterTimer(DB_TIMEOUT);
					g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
				}
				else
				{
					AC_ERROR("DbagentDataDecoder::DoRunLottery:pDBSvr->AddBuf() error");
					g_pNetProxy->DestroyDBResult(data);
					return 0;
				}
			}
			else
			{
				AC_ERROR("DbagentDataDecoder::DoRunLottery:CreateDBResultdata error,data=%x",data);
				return 0;
			}
		}
		else
		{
			AC_ERROR("DbagentDataDecoder::DoRunLottery:pDBSvr=%x ",pDBSvr);
			return 0;
		}
	}

	
	return 0;
}

int DbagentDataDecoder::DoLuckNotify(Dbresultdata* pdata,BinaryReadStream* infostream)
{

	AC_DEBUG("DbagentDataDecoder::DoLuckNotify:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);
	
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(proominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoLuckNotify:proominfo error");
		return -1;
	}

	//数据库返回值
	char infobuf[1024];
	size_t clen;
	
	if(!infostream->Read(infobuf,sizeof(infobuf),clen))
	{
		AC_ERROR("DbagentDataDecoder::DoLuckNotify:read result error");
		return -1;
	}
	infobuf[clen]=0;

	AC_DEBUG("DbagentDataDecoder::DoLuckNotify:infobuf=%s",infobuf);
	
	char* saveptr=NULL;
	char* token=strtok_r(infobuf,"|",&saveptr);
	if(token==NULL||saveptr==NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoLuckNotify:parse result error");
		return 0;
	}

	int result=atoi(token);
	token=strtok_r(NULL,"|",&saveptr);
	if(token==NULL||saveptr==NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoLuckNotify:parse gold error");
		return 0;
	}
	int gold=atoi(token);		//中奖者金币余额
	token=strtok_r(NULL,"|",&saveptr);
	if(token==NULL||saveptr==NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoLuckNotify:parse silver error");
		return 0;
	}
	int silver=atoi(token);		//中奖者银币余额
	
	//	0--成功
      //	2--无此交易类型
	if(result==DBRESULT_GIFT_SUCCESS)
	{
		//客户端金币与银币同步
		map<int,RoomClient*>::iterator itclient=proominfo->userlist.find(pdata->opidx);
		if(itclient!=proominfo->userlist.end())
		{
			RoomClient*  pRoomClient=itclient->second;
			pRoomClient->gold=gold;
			pRoomClient->silver=silver;
			
			BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
			char type=65;
			outstream->Write(type);
			outstream->Write((short)ROOM_CMD_UPDATEMONEY);
			outstream->Write(0);
			outstream->Write(gold);
			outstream->Write(silver);
			outstream->Flush();
			if(g_pNetProxy->SendToSrv(pRoomClient, *outstream)==-1)
			{
				AC_ERROR("DbagentDataDecoder::DoLuckNotify:idx=%d SendToSrv error",pdata->opidx);
				//pRoomClient->LeaveRoom();
			}

		}

		//同步用户金币与银币余额
		BinaryWriteStream* lobbystream=StreamFactory::InstanceWriteStream();
		lobbystream->Write((short)HALL_CMD_UPDATEGOLD_L2R);
		lobbystream->Write((int)0);
		lobbystream->Write((char)ROOM_TYPE_ROOM);	//ADD BY JINGUANFU 2010/7/9
		lobbystream->Write(pdata->opidx);			//中奖者
		lobbystream->Write(gold);				//中奖者金币余额
		lobbystream->Write(silver);				//中奖者银币余额
		lobbystream->Flush();
		AC_DEBUG("DbagentDataDecoder::DoRunLotte BroadToSver!");
		HallSvrClient *pHallSvr = g_pNetProxy->GetHallSvr();
		if(pHallSvr!=NULL)
			pHallSvr->AddBuf(lobbystream->GetData(), lobbystream->GetSize());	
		else
			AC_ERROR("DbagentDataDecoder::DoLuckNotify:pHallSvr is NULL");

		//广播中奖通知
		char* saveptr=NULL;
		char* token=strtok_r(pdata->content,",",&saveptr);
		for(;;)
		{
			if(token==NULL||saveptr==NULL)
			{
				AC_ERROR("DbagentDataDecoder::DoLuckNotify:parse result error");
				return 0;
			}
			int	multiple=atoi(token);
			if(multiple==0)
				break;
			
			//房间内中奖广播
			BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
			char type=65;
			outstream->Write(type);
			outstream->Write((short)ROOM_CMD_SB_LUCKY);
			outstream->Write(0);
			outstream->Write(pdata->opidx);		//抽奖者
			outstream->Write(pdata->cate_idx);		//礼物ID
			outstream->Write(multiple);				//中奖倍率
			outstream->Flush();
			if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream))
			{
				AC_ERROR("DbagentDataDecoder::DoLuckNotify:SendToSrv error");
			}

			//大于50倍时，发送大厅通知
			if(multiple>=50&&multiple<500)              //modify by lihongwu 2011/10/25
			{
				outstream->Clear();
				outstream->Write((short)HALL_CMD_LUCK_R2L);
				outstream->Write((int)0);
				outstream->Write((char)ROOM_TYPE_ROOM);	//ADD BY JINGUANFU 2010/7/9
				outstream->Write(pdata->opidx);			//中奖者
				outstream->Write(pdata->roomid);		//房间ID
				outstream->Write(pdata->cate_idx);		//礼物ID
				outstream->Write(multiple);				//中奖倍率
				outstream->Write((short)LUCK_BUGLE);		    //喇叭广播
				outstream->Flush();
				AC_DEBUG("DbagentDataDecoder::DoLuckNotify BroadToSver!");
				HallSvrClient *pHallSvr = g_pNetProxy->GetHallSvr();
				if(pHallSvr!=NULL)
					pHallSvr->AddBuf(outstream->GetData(), outstream->GetSize());	
				else
					AC_ERROR("DbagentDataDecoder::DoLuckNotify:pHallSvr is NULL");
			}

			else if(multiple>=500)              //modify by lihongwu 2011/10/25
			{
				outstream->Clear();
				outstream->Write((short)HALL_CMD_LUCK_R2L);
				outstream->Write((int)0);
				outstream->Write((char)ROOM_TYPE_ROOM);	//ADD BY JINGUANFU 2010/7/9
				outstream->Write(pdata->opidx);			//中奖者
				outstream->Write(pdata->roomid);		//房间ID
				outstream->Write(pdata->cate_idx);		//礼物ID
				outstream->Write(multiple);				//中奖倍率
				outstream->Write((short)LUCK_CONTENT);		    //喇叭和公告广播
				outstream->Flush();
				AC_DEBUG("DbagentDataDecoder::DoLuckNotify BroadToSver!");
				HallSvrClient *pHallSvr = g_pNetProxy->GetHallSvr();
				if(pHallSvr!=NULL)
					pHallSvr->AddBuf(outstream->GetData(), outstream->GetSize());	
				else
					AC_ERROR("DbagentDataDecoder::DoLuckNotify:pHallSvr is NULL");
			}

			token=strtok_r(NULL,",",&saveptr);
		}
	}
	else
	{
		AC_ERROR("DbagentDataDecoder::DoLuckNotify result=%d!",result);
	}

	return 0;
}

int DbagentDataDecoder::DoViweIncome(Dbresultdata* pdata,BinaryReadStream* infostream)
{
	AC_DEBUG("DbagentDataDecoder::DoViweIncome:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);
	
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(proominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoViweIncome:proominfo error");
		return -1;
	}

	//数据库返回值
	char infobuf[1024];
	size_t clen;
	
	if(!infostream->Read(infobuf,sizeof(infobuf),clen))
	{
		AC_ERROR("DbagentDataDecoder::DoViweIncome:read result error");
		return -1;
	}
	infobuf[clen]=0;

	int income=atoi(infobuf);
	//回应操作者
	char context[64] = {0};
	BinaryWriteStream outstream(context,sizeof(context));
	char type = 65;
	outstream.Write(type);
	outstream.Write((short)pdata->cmd);
	outstream.Write(pdata->seq);
	outstream.Write(income);			// 可收益余额
	outstream.Flush();	
		
	RoomClient* pClient = g_pNetProxy->GetClient(pdata->number);
	if(pClient!=NULL)
	{
		if(g_pNetProxy->SendToSrv(pClient, outstream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::DoViweIncome:SendToSrv error");
		}
	}
	else
	{
		AC_ERROR("DbagentDataDecoder::DoViweIncome:pClient is NULL ");
	}
	
	
	return 0;
}

int DbagentDataDecoder::DoViweIncomeLog(Dbresultdata* pdata,short rownum,short colnum,BinaryReadStream* infostream)
{
	AC_DEBUG("DbagentDataDecoder::DoViweIncomeLog:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);
	
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(proominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoViweIncomeLog:proominfo error");
		return -1;
	}
	if(rownum>5)
	{
		AC_ERROR("DbagentDataDecoder::DoViweIncomeLog:DB result error,rownum=%d,colnum=%d",rownum,colnum);
		return -1;
	}

	BinaryWriteStream *outstream=StreamFactory::InstanceWriteStream();
	char type = 65;
	
	if(rownum==0)
	{
		outstream->Write(type);
		outstream->Write((short)pdata->cmd);
		outstream->Write(pdata->seq);
		outstream->Write((short)0);			//无记录
		outstream->Flush();	
	}
	else
	{

		outstream->Write(type);
		outstream->Write((short)pdata->cmd);
		outstream->Write(pdata->seq);
		outstream->Write(rownum);	//记录条数
		
		//数据库返回值
		char infobuf[1024];
		size_t clen;

		int income=0;
		for(int i=0;i<rownum;i++)
		{
			//已领取收益
			if(!infostream->Read(infobuf,sizeof(infobuf),clen))
			{
				AC_ERROR("DbagentDataDecoder::DoViweIncomeLog:read income error,rownum=%d",rownum);
				return -1;
			}
			infobuf[clen]=0;
			income=atoi(infobuf);
			outstream->Write(income);
			//领取日期
			if(!infostream->Read(infobuf,sizeof(infobuf),clen))
			{
				AC_ERROR("DbagentDataDecoder::DoViweIncomeLog:read date error,rownum=%d",rownum);
				return -1;
			}
			outstream->Write(infobuf,clen);
		}
		outstream->Flush();	

	}
	
	RoomClient* pClient = g_pNetProxy->GetClient(pdata->number);
	if(pClient!=NULL)
	{
		if(g_pNetProxy->SendToSrv(pClient, *outstream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::DoViweIncomeLog:SendToSrv error");
		}
	}
	else
	{
		AC_ERROR("DbagentDataDecoder::DoViweIncomeLog:pClient is NULL ");
	}
	
	
	return 0;
}

int DbagentDataDecoder::DoGetIncome(Dbresultdata* pdata,BinaryReadStream* infostream)
{
	AC_DEBUG("DbagentDataDecoder::DoGetIncome:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);
	
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(proominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoGetIncome:proominfo error");
		return -1;
	}

	//数据库返回值
	char infobuf[1024];
	size_t clen;
	
	if(!infostream->Read(infobuf,sizeof(infobuf),clen))
	{
		AC_ERROR("DbagentDataDecoder::DoViweIncome:read result error");
		return -1;
	}
	infobuf[clen]=0;

	char* saveptr=NULL;
	char* token=strtok_r(infobuf,"|",&saveptr);
	if(token==NULL||saveptr==NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoViweIncome:parse result error");
		return 0;
	}
	int result=atoi(token);
	int Gold=0;

	token=strtok_r(NULL,"|",&saveptr);
	if(token==NULL||saveptr==NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoViweIncome:parse Gold error");
		return 0;
	}
	Gold=atoi(token);	
		
	RoomClient* pClient = g_pNetProxy->GetClient(pdata->number);
	if(pClient!=NULL)
	{
		//回应操作者
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		char type = 65;
		outstream->Write(type);
		outstream->Write((short)pdata->cmd);
		outstream->Write(pdata->seq);
		outstream->Write(result);				// 0 :成功1:失败
		//outstream->Write(Gold-pClient->gold);	// 收益金额
		outstream->Flush();	
		if(g_pNetProxy->SendToSrv(pClient, *outstream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::DoViweIncome:idx=%d SendToSrv error",pClient->m_idx);
			//pClient->LeaveRoom();
			return 0;
		}

		//钱币更新
		pClient->gold=Gold;

		AC_DEBUG("DbagentDataDecoder::DoViweIncome: idx=%d,gold=%d,silver=%d",pClient->m_idx,pClient->gold,pClient->silver);
		outstream->Clear();
		outstream->Write(type);
		outstream->Write((short)ROOM_CMD_UPDATEMONEY);
		outstream->Write(0);
		outstream->Write(pClient->gold);
		outstream->Write(pClient->silver);
		outstream->Flush();
		if(g_pNetProxy->SendToSrv(pClient, *outstream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::DoViweIncome:idx=%d SendToSrv error",pClient->m_idx);
			//pClient->LeaveRoom();
			return 0;
		}
		
	}
	else
	{
		AC_ERROR("DbagentDataDecoder::DoViweIncome:pClient is NULL ");
	}
	
	return 0;
}

int DbagentDataDecoder::DoUpdateMusicInfo(Dbresultdata* pdata,BinaryReadStream* infostream)
{
	AC_DEBUG("DbagentDataDecoder::DoUpdateMusicInfo:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);
	//数据库返回值
	char infobuf[1024];
	size_t clen;
	
	if(!infostream->Read(infobuf,sizeof(infobuf),clen))
	{
		AC_ERROR("DbagentDataDecoder::DoUpdateMusicInfo:read result error");
		return -1;
	}
	infobuf[clen]=0;

	int result=atoi(infobuf);

	if(result==0)
	{
		AC_DEBUG("DbagentDataDecoder::DoUpdateMusicInfo: success");
	}
	else
	{
		AC_ERROR("DbagentDataDecoder::DoUpdateMusicInfo:error ,result[%d],outseq=%d",result,pdata->outseq);
	}

	return 0;
}

int DbagentDataDecoder::DoUpdateGiftInfo(Dbresultdata* pdata,
												BinaryReadStream* infostream,
												short rownum,
												short colnum)
{

	AC_DEBUG("DbagentDataDecoder::DoUpdateGiftInfo: rownum=%d, colnum=%d",rownum,colnum);

	char infobuf[1024] = {0};
	size_t curlen;
	int giftid=0;
	int price=0;
	int extype=0;
	int vtime = 0;
	
	for(int i=0;i<rownum;i++)
	{
		for(int j = 0;j < colnum;j++)
		{
			if(!infostream->Read(infobuf,sizeof(infobuf),curlen))
			{
				AC_ERROR("DbagentDataDecoder::DoUpdateGiftInfo():read ptr error");
				return -1;
			}
			infobuf[curlen] = 0;
			if(j == 0)	//礼物ID
			{
				giftid= atoi(infobuf);
			}
			if(j == 1)	//礼物价格
			{
				price= atoi(infobuf);
			}
			if(j == 2)	//礼物处理类型
			{
				extype= atoi(infobuf);
			}	
			if(j == 3)	//礼物有效时间，仅对印章有效
			{
				vtime= atoi(infobuf);
			}
		}
		map<int,GIFT_INFO>::iterator itGift=g_pNetProxy->m_GiftInfo.find(giftid);
		if(itGift!=g_pNetProxy->m_GiftInfo.end())
		{
			GIFT_INFO& giftinfo=itGift->second;
			giftinfo.GiftID=giftid;
			giftinfo.price=price;
			giftinfo.type=extype;
			giftinfo.vtime=vtime;
		}
		else
		{
			GIFT_INFO giftinfo;
			giftinfo.GiftID=giftid;
			giftinfo.price=price;
			giftinfo.type=extype;
			giftinfo.vtime=vtime;

			g_pNetProxy->m_GiftInfo.insert(make_pair(giftid,giftinfo));
		}
		
	}

	//add by lihongwu 2011/10/11
	//把礼物配置信息拼到礼物信息里 
	map<int,GIFT_CONFIG>::iterator itconfig = g_pNetProxy->m_giftconflist.begin();
	map<int,GIFT_CONFIG>::iterator itconfig2 = itconfig;
	map<int,GIFT_INFO>::iterator itinfo = g_pNetProxy->m_GiftInfo.begin();
	for (;itinfo != g_pNetProxy->m_GiftInfo.end();itinfo++)
	{
		GIFT_INFO& giftinfo = itinfo->second;               //引用类型
		for (;itconfig != g_pNetProxy->m_giftconflist.end();itconfig++)
		{
			itconfig2 = itconfig;
			itconfig2++;
			GIFT_CONFIG giftconf = itconfig->second;
			if (itconfig2 != g_pNetProxy->m_giftconflist.end()) 
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

	AC_DEBUG("DbagentDataDecoder::DoUpdateGiftInfo: giftinfo add giftconfig ");

	//向所有房间在线用户发送通知
	BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();	
	char type = 65;
	outstream->Write(type);
	outstream->Write(pdata->cmd);
	outstream->Write(0);		
	outstream->Flush();

	map<int, ROOM_INFO*>::iterator itroom = g_pNetProxy->m_roomlistinfo.roommap.begin();
	for(;itroom!=g_pNetProxy->m_roomlistinfo.roommap.end();itroom++)
	{
		ROOM_INFO* proominfo = itroom->second;
        if(proominfo)
			g_pNetProxy->BroadcastSBCmd(proominfo, *outstream);
		else
			AC_ERROR("proominfo == NULL");
	}
	
	return 0;
}

int DbagentDataDecoder::DoUpdateLuckConf(Dbresultdata* pdata,
												BinaryReadStream* infostream,
												short rownum,
												short colnum)
{
	AC_DEBUG("DbagentDataDecoder::DoUpdateGiftInfo: rownum=%d, colnum=%d,pdata=%x",rownum,colnum,pdata);

	char infobuf[1024];
	size_t curlen;

	int multiple=0;
	int probabitiy=0;
	for(int i = 0;i < rownum;i++)
	{		
		for(int j = 0;j < colnum;j++)
		{
			if(!infostream->Read(infobuf,sizeof(infobuf),curlen))
			{
				AC_ERROR("DbagentDataDecoder::DoUpdateLuckConf:read ptr error");
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
				g_pNetProxy->m_luckbase= atoi(infobuf);
			}
			
		}
		AC_DEBUG("DbagentDataDecoder::DoUpdateLuckConf: %d,%d,%d",multiple,probabitiy,g_pNetProxy->m_luckbase);

		map<int,int>::iterator itluck=g_pNetProxy->m_LuckConf.find(multiple);
		if(itluck!=g_pNetProxy->m_LuckConf.end())
		{
			itluck->second=probabitiy;
		}
		else
		{
			g_pNetProxy->m_LuckConf.insert(make_pair(multiple,probabitiy));
		}
		
	}

	return 0;
}

int DbagentDataDecoder::DoUpdateRightConf(Dbresultdata* pdata,
												BinaryReadStream* infostream,
												short rownum,
												short colnum)
{

	AC_DEBUG("DbagentDataDecoder::DoUpdateRightConf: rownum=%d, colnum=%d",rownum,colnum);


	char infobuf[65535];		//64K
	size_t curlen;
	char filetemp[128*1024];	//128K
	int filelen=0;

	filetemp[filelen]=0;

	for(int i=0;i<rownum;i++)
	{
		for(int j = 0;j < colnum;j++)
		{
			if(!infostream->Read(infobuf,sizeof(infobuf),curlen))
			{
				AC_ERROR("DbagentDataDecoder::DoUpdateRightConf():read ptr error");
				return -1;
			}
			infobuf[curlen] = 0;
			strncat(filetemp,infobuf,curlen);
			filelen+=curlen;
			filetemp[filelen]=0;
		}
	}
	filetemp[filelen]=0;
	if(strlen(filetemp)>0)
	{
		FILE *pXmlfile=NULL;
		size_t writelen=0;

		pXmlfile=fopen(g_rightpath,"wb");
		if(pXmlfile==NULL)
		{
			AC_ERROR("DbagentDataDecoder::DoUpdateRightConf:rightconfig xml file open failed,errno:%d\n",errno);
			return -1;
		}

		writelen=fwrite(filetemp,1,filelen,pXmlfile);
		if(writelen!=strlen(filetemp))
		{
			AC_ERROR("DbagentDataDecoder::DoUpdateRightConf:rightconfig xml file write failed,errno:%d\n",errno);
			return -1;
		}
		fflush(pXmlfile);
		fclose(pXmlfile);
		//加载配置文件
		if(g_pNetProxy->LoadRightconfig(g_rightpath)==-1)
		{
			AC_ERROR("DbagentDataDecoder::DoUpdateRightConf:LoadRightconfig error");
			return -1;
		}
		//向所有房间在线用户发送通知
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();	
		char type = 65;
		outstream->Write(type);
		outstream->Write(pdata->cmd);
		outstream->Write(0);		
		outstream->Flush();

		map<int, ROOM_INFO*>::iterator itroom = g_pNetProxy->m_roomlistinfo.roommap.begin();
		for(;itroom!=g_pNetProxy->m_roomlistinfo.roommap.end();itroom++)
		{
			ROOM_INFO* proominfo = itroom->second;
			if(proominfo)
				g_pNetProxy->BroadcastSBCmd(proominfo, *outstream);
			else
				AC_ERROR("proominfo == NULL");
		}
	}
	else
		AC_ERROR("DbagentDataDecoder::DoUpdateRightConf: infobuf length=%d",curlen);

	
	return 0;
}

int DbagentDataDecoder::DoGetRoomInfo(Dbresultdata* pdata,
												BinaryReadStream* infostream,
												short rownum,
												short colnum)
{
	AC_DEBUG("DbagentDataDecoder::DoGetRoomInfo: rownum=%d, colnum=%d,pdata=%x",rownum,colnum,pdata);

	if(rownum!=1)
	{
		AC_ERROR("DbagentDataDecoder::DoGetRoomInfo:rownum error");
		return -1;
	}

	char infobuf[1024];
	size_t curlen;

	char ip_telcom[32]={0};
	char ip_netcom[32]={0};
	int roomid = 0;
	ROOM_INFO* proominfo = new ROOM_INFO();
	for(int j = 0;j < colnum;j++)
	{
		if(!infostream->Read(infobuf,sizeof(infobuf),curlen))
		{
			AC_ERROR("DbagentDataDecoder::DoGetRoomInfo:read ptr error");
			return -1;
		}
		infobuf[curlen] = 0;
		switch(j)
		{
		case 0:
		{
			roomid = atoi(infobuf);
			proominfo->roomid = roomid;
		}
			break;
		case 1:
		{
			int hallid = atoi(infobuf);
			if(hallid!=g_pNetProxy->m_pHallsvrCfg->m_id)
			{
				AC_ERROR("DbagentDataDecoder::DoGetRoomInfo:hallid error,[%d],ServerHallID[%d]",hallid,g_pNetProxy->m_pHallsvrCfg->m_id);
				return -1;
			}

		}
			break;
		case 2:
           	{
               if(infobuf!=NULL&&curlen<sizeof(proominfo->roomname))
               {
                  strncpy(proominfo->roomname, infobuf,curlen);
                  proominfo->roomname[curlen]=0;
               }
               else if(infobuf!=NULL&&curlen>=sizeof(proominfo->roomname))
               {
				strncpy(proominfo->roomname, infobuf,sizeof(proominfo->roomname)-1);
				proominfo->roomname[sizeof(proominfo->roomname)-1]=0;
               }
               else 
				memset(proominfo->roomname,0,sizeof(proominfo->roomname));	

            }
			break;
		case 3:
	       {
               if(infobuf!=NULL&&curlen<sizeof(proominfo->passwd))
               {
                  strncpy(proominfo->passwd, infobuf,curlen);
                  proominfo->passwd[curlen]=0;
               }
			else 
				memset(proominfo->passwd,0,sizeof(proominfo->passwd));	
	       }
		break;
		case 4:
		{
			proominfo->type = atoi(infobuf);
		}
		break;
		case 5:
		{
			proominfo->state = atoi(infobuf);
		}
		break;
 		case 6:
		{	
			proominfo->ownidx= atoi(infobuf);
 		}
		break;
		case 7:{}break;	//房间创建时间
		case 8://房间排序标志
		{
             	int freeflag = atoi(infobuf);
				if(freeflag ==1)
					proominfo->isMicUpdown = true;
				else if(freeflag == 0)
					proominfo->isMicUpdown = false;
		}
        break;
		case 9:
		{
			proominfo->maxhuman = atoi(infobuf);
		}
		break;
		case 10:{}break;	//房间到期时间
		case 11:{}break;	//自动审核会员申请
		case 12:	//房间公告
          	{
          	 	if(infobuf!=NULL&&curlen<sizeof(proominfo->content))
	           	{
				strncpy(proominfo->content, infobuf,curlen);
				proominfo->content[curlen]=0;
	           	}	
			else if(infobuf!=NULL&&curlen>=sizeof(proominfo->content))
			{
				strncpy(proominfo->content, infobuf,sizeof(proominfo->content)-1);
				proominfo->content[sizeof(proominfo->content)-1]=0;
			}
			else
				memset(proominfo->content,0,sizeof(proominfo->content));
			
         	}
		break;
		case 13://房间IP地址--电信
		{
			strncpy(ip_telcom,infobuf,curlen);
			ip_telcom[curlen]=0;
		}
		break;	
		case 14://房间端口
		{
			int port=atoi(infobuf);
			if(port!=g_pNetProxy->m_port)
			{
				AC_ERROR("DbagentDataDecoder::DoGetRoomInfo:port error [%d],[%d]",port,g_pNetProxy->m_port);
				return -1;
			}
		}
		break;	
		case 15:	//欢迎词
		{

			if(infobuf!=NULL&&curlen<sizeof(proominfo->welcomeword))
			{
				strncpy(proominfo->welcomeword,infobuf,curlen);
				proominfo->welcomeword[curlen]=0;
			}
			else if(infobuf!=NULL&&curlen>=sizeof(proominfo->welcomeword))
			{
				strncpy(proominfo->welcomeword,infobuf,sizeof(proominfo->welcomeword)-1);
				proominfo->welcomeword[sizeof(proominfo->welcomeword)-1]=0;
			}
			else
				memset(proominfo->welcomeword,0,sizeof(proominfo->welcomeword));	
	       }
		break;
		case 16:	{}break;//房间更新时间
		case 17:
		{
			if(infobuf!=NULL&&curlen<sizeof(proominfo->roomlogo))
			{
			 	strncpy(proominfo->roomlogo,infobuf,curlen);
				proominfo->roomlogo[curlen]=0;
			}
			else if(infobuf!=NULL&&curlen>=sizeof(proominfo->roomlogo))
			{
				strncpy(proominfo->roomlogo, infobuf,sizeof(proominfo->roomlogo)-1);
				proominfo->roomlogo[sizeof(proominfo->roomlogo)-1]=0;
			}
			else
				memset(proominfo->roomlogo,0,sizeof(proominfo->roomlogo));	
		}
		break;
		case 18://是否允许公聊标志位
		{
			int  chatflag =atoi(infobuf);
			if(chatflag == 1)
				proominfo->isPublicChat=true;
			else if(chatflag == 0)
				proominfo->isPublicChat=false;
		}
		break;
		case 19://用户进出信息显示标志位
		{
			int inoutflag = atoi(infobuf);
			if(inoutflag ==1)
				proominfo->isUserInOut = true;
			else if(inoutflag == 0)
				proominfo->isUserInOut = false;
		}
		break;
		case 20://房间是否公开标志位
		{
			int useronlyflag = atoi(infobuf);
			if(useronlyflag ==1)
				proominfo->isUserOnly = true;
			else if(useronlyflag == 0)
				proominfo->isUserOnly = false;
		}
		break;
		case 21://房间是否关闭标志位
		{
			int closeflag = atoi(infobuf);
			if(closeflag ==1)
				proominfo->isClose = true;
			else if(closeflag == 0)
				proominfo->isClose = false;
		}
		break;
		case 22://房间允许人数
		{
			proominfo->allowhuman=atoi(infobuf);
		}
		break;
		case 23://服务器地址--网通
		{
			strncpy(ip_netcom,infobuf,curlen);
			ip_netcom[curlen]=0;

			if(strcmp(ip_netcom,g_pNetProxy->m_sIP)&&strcmp(ip_telcom,g_pNetProxy->m_sIP))
			{
				AC_ERROR("DbagentDataDecoder::DoGetRoomInfo:ip error ip_netcom=[%s],ip_telcom=[%s],server_IP:%s",ip_netcom,ip_telcom,g_pNetProxy->m_sIP);
				return -1;
			}
		}
		break;
		case 24:	//音视频服务器地址--电信
		{
			if(curlen<sizeof(proominfo->AVServerIP_telcom))
			{
				strncpy(proominfo->AVServerIP_telcom,infobuf,curlen);
				proominfo->AVServerIP_telcom[curlen]=0;
			}
			else
			{
			 	AC_ERROR("DbagentDataDecoder::DoGetRoomInfo:AVServerIP_telcom len=%d",curlen);
				return -1;
			}
			
		}
		break;
		case 25://音视频服务器端口--电信
		{
			proominfo->AVServerPort_telcom=atoi(infobuf);
		}
		break;
		case 26://音视频服务器地址--网通
		{
			if(curlen<sizeof(proominfo->AVServerIP_netcom))
			{
				strncpy(proominfo->AVServerIP_netcom,infobuf,curlen);
				proominfo->AVServerIP_netcom[curlen]=0;
			}
			else
			{
			 	AC_ERROR("DbagentDataDecoder::DoGetRoomInfo:AVServerIP_netcom len=%d",curlen);
				return -1;
			}
		}
		break;
		case 27://音视频服务器端口--网通
		{
			proominfo->AVServerPort_netcom=atoi(infobuf);
		}
		break;
		/*
		case 28://中继服务器地址--电信
		{
			if(curlen<sizeof(proominfo->RelayIP_telcom))
			{
				strncpy(proominfo->RelayIP_telcom,infobuf,curlen);
				proominfo->RelayIP_telcom[curlen]=0;
			}
			else
			{
			 	AC_ERROR("DbagentDataDecoder::DoGetRoomInfo:RelayIP_telcom len=%d",curlen);
				return -1;
			}
		}
		break;
		case 29://中继服务器端口--电信
		{
			proominfo->RelayPort_telcom=atoi(infobuf);
		}
		break;
		case 30://中继服务器地址--网通
		{
			if(curlen<sizeof(proominfo->RelayIP_netcom))
			{
				strncpy(proominfo->RelayIP_netcom,infobuf,curlen);
				proominfo->RelayIP_netcom[curlen]=0;
			}
			else
			{
			 	AC_ERROR("DbagentDataDecoder::DoGetRoomInfo:RelayIP_netcom len=%d",curlen);
				return -1;
			}
		}
		break;
		case 31://中继服务器端口--网通
		{
			proominfo->RelayPort_telcom=atoi(infobuf);
		}
		*/
		default: {}

		}
		if(j==colnum-1)
		{
			proominfo->isApplylistOk=0;
			proominfo->isBlacklistOk=0;
			proominfo->isMemberlistOk =0;
			proominfo->secondownidx=0;
			proominfo->secondownidx2=0;
			proominfo->vjonmic=0;

			proominfo->isAutoOnmic=true;

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
			 	AC_ERROR("DbagentDataDecoder::DoGetRoomInfo:CreateRoomTimeout error");
			}

			proominfo->FlowerTimer=g_pNetProxy->CreateFlowerResult();
			if(proominfo->FlowerTimer!=NULL)
			{
				proominfo->FlowerTimer->SetReactor(g_pNetProxy->GetReactor());
				proominfo->FlowerTimer->roomid=roomid;
				proominfo->FlowerTimer->RegisterTimer(SENDGIFT_SETTM);		// 3秒刷33个礼物
			}
			else
			{
				AC_ERROR("DbagentDataDecoder::DoGetRoomInfo:CreateFlowerResult error");
				return -1;
			}
					
			g_pNetProxy->m_roomlistinfo.roommap.insert(make_pair(roomid, proominfo));
			
			AC_DEBUG("DbagentDataDecoder::DoGetRoomInfo:proominfo->roomid = %d proominfo->roomname = %s proominfo->passwd = %s proominfo->type = %d proominfo->state = %d proominfo->ownidx = %d proominfo->maxhuman = %d proominfo->welcomeword = %s proominfo->content = %s",
				proominfo->roomid,proominfo->roomname,proominfo->passwd,proominfo->type, proominfo->state, proominfo->ownidx,
				proominfo->maxhuman,proominfo->welcomeword,proominfo->content);
		}
	}
	

	int outseq=0;
	BinaryWriteStream* outstream = StreamFactory::InstanceWriteStream();
	BackClientSocketBase *pDBSvr=NULL;
	//取房间会员列表
	pDBSvr = g_pNetProxy->GetDBAgent();
	if(pDBSvr!=NULL)
	{
		outstream->Clear();
		outseq = g_pNetProxy->GetCounter()->Get();
		static const char* spname = {"DBMW_GetMultiAdmin_byroomid"};
		outstream->Write((short)CMD_CALLSP);
		outstream->Write(outseq);
		outstream->Write(spname,strlen(spname));
		outstream->Write((short)1);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write(roomid);
		outstream->Flush();
		
		Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
		if(data != NULL) 
		{
			if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1)
			{
				data->roomid = roomid;
				data->opidx = 10000;
				data->bopidx = 10000;
				data->cmd = INNER_CMD_GETMEMBERLIST;
				data->seq=0;
				data->outseq=outseq;
				data->SetReactor(g_pNetProxy->GetReactor());
				data->RegisterTimer(DB_TIMEOUT);
				g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
			}
			else
			{
				AC_ERROR("DbagentDataDecoder::DoGetRoomInfo::pDBSvr->AddBuf() error ");
				g_pNetProxy->DestroyDBResult(data);
			}
		}
		else
		{
			AC_ERROR("DbagentDataDecoder::DoGetRoomInfo::CreateDBResultdata error,data=%x",data);
		}
	}
	else
	{
		AC_ERROR("DbagentDataDecoder::DoGetRoomInfo::pDBSvr=%x ",pDBSvr);
	}

	//取房间会员申请列表
	pDBSvr = g_pNetProxy->GetDBAgent();
	if(pDBSvr!=NULL)
	{
		outseq = g_pNetProxy->GetCounter()->Get();
		outstream->Clear();
		static const char* spname = {"DBMW_GetApplyList_HZSTAR"};
		outstream->Write((short)CMD_CALLSP);
		outstream->Write(outseq);
		outstream->Write(spname,strlen(spname));
		outstream->Write((short)1);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write(roomid);
		outstream->Flush();
		
		Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
		if(data != NULL) 
		{
			if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1)
			{
				data->roomid = roomid;
				data->opidx = 10000;
				data->bopidx = 10000;
				data->cmd = INNER_CMD_GETAPPLYLIST;
				data->seq=0;
				data->outseq=outseq;
				data->SetReactor(g_pNetProxy->GetReactor());
				data->RegisterTimer(DB_TIMEOUT);
				g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
			}
			else
			{
				AC_ERROR("DbagentDataDecoder::DoGetRoomInfo::pDBSvr->AddBuf() error ");
				g_pNetProxy->DestroyDBResult(data);
			}
		}
		else
		{
			AC_ERROR("DbagentDataDecoder::DoGetRoomInfo::CreateDBResultdata error,data=%x",data);
		}
	}
	else
	{
		AC_ERROR("DbagentDataDecoder::DoGetRoomInfo::pDBSvr=%x ",pDBSvr);
	}
	
	//取房间黑名单
	pDBSvr = g_pNetProxy->GetDBAgent();
	if(pDBSvr!=NULL)
	{
		outseq = g_pNetProxy->GetCounter()->Get();
		outstream->Clear();
		static const char* spname = {"DBMW_GetMultiblacklist_byroomid"};
		outstream->Write((short)CMD_CALLSP);
		outstream->Write(outseq);
		outstream->Write(spname,strlen(spname));
		outstream->Write((short)1);
		outstream->Write((char)PT_INPUT);
		outstream->Write((char)PDT_INT);
		outstream->Write(roomid);
		outstream->Flush();
		
		Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
		if(data != NULL) 
		{
			if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1)
			{
				data->roomid = roomid;
				data->opidx = 10000;
				data->bopidx = 10000;
				data->cmd = INNER_CMD_GETBLACKLIST;
				data->seq=0;
				data->outseq=outseq;
				data->SetReactor(g_pNetProxy->GetReactor());
				data->RegisterTimer(DB_TIMEOUT);
				g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
			}
			else
			{
				AC_ERROR("DbagentDataDecoder::DoGetRoomInfo::pDBSvr->AddBuf() error ");
				g_pNetProxy->DestroyDBResult(data);
			}
		}
		else
		{
			AC_ERROR("DbagentDataDecoder::DoGetRoomInfo::CreateDBResultdata error,data=%x",data);
		}
	}
	else
	{
		AC_ERROR("DbagentDataDecoder::DoGetRoomInfo::pDBSvr=%x ",pDBSvr);
	}
	
	return 0;
}

int DbagentDataDecoder::DoGetRoomApplyList(Dbresultdata* pdata,
												BinaryReadStream* infostream,
												short rownum,
												short colnum)
{

	AC_DEBUG("DbagentDataDecoder::DoGetRoomApplyList: rownum=%d, colnum=%d",rownum,colnum);

	ROOM_INFO* pRoominfo=g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(pRoominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoUpdateRoomInfo:roomid=%d not exist",pdata->roomid);
		return -1;
	}
	
	char infobuf[1024];
	size_t curlen;

	for(int i = 0;i < rownum;i++)
	{		
		ROOM_APPLY roomapply;
		for(int j = 0;j < colnum;j++)
		{
			if(!infostream->Read(infobuf,sizeof(infobuf),curlen))
			{
				AC_ERROR("DbagentDataDecoder::DoUpdateRoomInfo:read ptr error");
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
		pRoominfo->userlistAPP.insert(make_pair(roomapply.m_idx,roomapply));
	}
	pRoominfo->isApplylistOk=1;

	AC_DEBUG("DbagentDataDecoder::DoUpdateRoomInfo:isApplylistOk=%d,isMemberlistOk=%d,isBlacklistOk=%d",pRoominfo->isApplylistOk,pRoominfo->isMemberlistOk,pRoominfo->isBlacklistOk);
	//房间信息取完整后通知大厅
	if(pRoominfo->isMemberlistOk&&pRoominfo->isBlacklistOk&&pRoominfo->isApplylistOk)
	{
		BinaryWriteStream* lobbystream = StreamFactory::InstanceWriteStream();
		lobbystream->Write((short)HALL_CMD_MOVE_ROOM_L2R);
		lobbystream->Write(0);
		lobbystream->Write(pRoominfo->roomid);
		lobbystream->Flush();

		HallSvrClient *pHallSvr = g_pNetProxy->GetHallSvr();
		if(pHallSvr!=NULL)
			pHallSvr->AddBuf(lobbystream->GetData(), lobbystream->GetSize());	
		else
			AC_ERROR("DbagentDataDecoder::DoUpdateRoomInfo:pHallSvr is NULL");
	}
	
	return 0;
}

int DbagentDataDecoder::DoGetRoomBlackList(Dbresultdata* pdata,
												BinaryReadStream* infostream,
												short rownum,
												short colnum)
{
	AC_DEBUG("DbagentDataDecoder::DoGetRoomBlackList: rownum=%d, colnum=%d",rownum,colnum);

	ROOM_INFO* pRoominfo=g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(pRoominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoGetRoomBlackList:roomid=%d not exist",pdata->roomid);
		return -1;
	}
	
	char infobuf[1024];
	size_t curlen;
	int idx=0;
	for(int i = 0;i < rownum;i++)
	{
		for(int j = 0;j < colnum;j++)
		{
			if(!infostream->Read(infobuf,sizeof(infobuf),curlen))
			{
				AC_ERROR("DbagentDataDecoder::DoGetRoomBlackList:read ptr error");
				return -1;
			}
			infobuf[curlen] = 0;

			if(j == colnum - 1)
			{
				idx= atoi(infobuf);
				pRoominfo->blacklist.insert(make_pair(idx, idx));
				AC_DEBUG("DbagentDataDecoder::DoGetRoomBlackList:rownum = %d colnum = %d idx = %d",i,j,idx);
			}
		}
	}

	pRoominfo->isBlacklistOk=1;
	AC_DEBUG("DbagentDataDecoder::DoUpdateRoomInfo:isApplylistOk=%d,isMemberlistOk=%d,isBlacklistOk=%d",pRoominfo->isApplylistOk,pRoominfo->isMemberlistOk,pRoominfo->isBlacklistOk);

	//房间信息取完整后通知大厅
	if(pRoominfo->isMemberlistOk&&pRoominfo->isBlacklistOk&&pRoominfo->isApplylistOk)
	{
		BinaryWriteStream* lobbystream = StreamFactory::InstanceWriteStream();
		lobbystream->Write((short)HALL_CMD_MOVE_ROOM_L2R);
		lobbystream->Write(0);
		lobbystream->Write(pRoominfo->roomid);
		lobbystream->Flush();

		HallSvrClient *pHallSvr = g_pNetProxy->GetHallSvr();
		if(pHallSvr!=NULL)
			pHallSvr->AddBuf(lobbystream->GetData(), lobbystream->GetSize());	
		else
			AC_ERROR("DbagentDataDecoder::DoGetRoomBlackList:pHallSvr is NULL");
	}
	
	return 0;
}

int DbagentDataDecoder::DoGetRoomMemberList(Dbresultdata* pdata,
												BinaryReadStream* infostream,
												short rownum,
												short colnum)
{

	AC_DEBUG("DbagentDataDecoder::DoGetRoomMemberList: rownum=%d, colnum=%d",rownum,colnum);

	ROOM_INFO* pRoominfo=g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(pRoominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoGetRoomMemberList:roomid=%d not exist",pdata->roomid);
		return -1;
	}
	
	char infobuf[1024];
	size_t curlen;

	for(int i = 0;i < rownum;i++)
	{
		int idx = 0;
		for(int j = 0;j < colnum;j++)
		{
			if(!infostream->Read(infobuf,sizeof(infobuf),curlen))
			{
				AC_ERROR("DbagentDataDecoder::DoGetRoomMemberList::read ptr error");
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

				if(level == USER_ID_VJ)
				{
					pRoominfo->vjlist.insert(make_pair(idx, idx));
					ROOM_MANAGER roommanager;
					roommanager.m_idx=idx;
					roommanager.m_identity=USER_ID_VJ;
					pRoominfo->managerlist.insert(make_pair(idx,roommanager));
				}
				if(level == USER_ID_VJ_A)
				{
					pRoominfo->vjlist_a.insert(make_pair(idx, idx));	
					ROOM_MANAGER roommanager;
					roommanager.m_idx=idx;
					roommanager.m_identity=USER_ID_VJ_A;
					pRoominfo->managerlist.insert(make_pair(idx,roommanager));
				}
              		if(level == USER_ID_OWNER)
                		{
                    		pRoominfo->ownidx = idx;
					ROOM_MANAGER roommanager;
					roommanager.m_idx=idx;
					roommanager.m_identity=USER_ID_OWNER;
					pRoominfo->managerlist.insert(make_pair(idx,roommanager));
                		}
                		if(level == USER_ID_OWNER_S)
                		{
                    		if(pRoominfo->secondownidx == 0)
                        			pRoominfo->secondownidx = idx;	
                    		else if(pRoominfo->secondownidx2 == 0)
                        			pRoominfo->secondownidx2 = idx;

					ROOM_MANAGER roommanager;
					roommanager.m_idx=idx;
					roommanager.m_identity=USER_ID_OWNER_S;
					pRoominfo->managerlist.insert(make_pair(idx,roommanager));
                		}
				
                    	pRoominfo->userlistVIP.insert(make_pair(idx, idx));		
				AC_DEBUG("DbagentDataDecoder::DoGetRoomMemberList::rownum = %d colnum = %d level = %d idx = %d vjsize = %d proominfo = %x"
					,i,j,level, idx, pRoominfo->vjlist.size(), pRoominfo);
			}
		}
	}
	pRoominfo->isMemberlistOk=1;
	AC_DEBUG("DbagentDataDecoder::DoUpdateRoomInfo:isApplylistOk=%d,isMemberlistOk=%d,isBlacklistOk=%d",pRoominfo->isApplylistOk,pRoominfo->isMemberlistOk,pRoominfo->isBlacklistOk);

	//房间信息取完整后通知大厅
	if(pRoominfo->isMemberlistOk&&pRoominfo->isBlacklistOk&&pRoominfo->isApplylistOk)
	{
		BinaryWriteStream* lobbystream = StreamFactory::InstanceWriteStream();
		lobbystream->Write((short)HALL_CMD_MOVE_ROOM_L2R);
		lobbystream->Write(0);
		lobbystream->Write(pRoominfo->roomid);
		lobbystream->Flush();

		HallSvrClient *pHallSvr = g_pNetProxy->GetHallSvr();
		if(pHallSvr!=NULL)
			pHallSvr->AddBuf(lobbystream->GetData(), lobbystream->GetSize());	
		else
			AC_ERROR("DbagentDataDecoder::DoGetRoomMemberList:pHallSvr is NULL");
	}
	
	return 0;
}

int DbagentDataDecoder::DoGetRoomAVServer(Dbresultdata* pdata,
												BinaryReadStream* infostream,
												short rownum,
												short colnum)
{

	AC_DEBUG("DbagentDataDecoder::DoGetRoomAVServer: rownum=%d, colnum=%d",rownum,colnum);

	char infobuf[1024];
	size_t curlen;

	int roomid = 0;
	ROOM_INFO* proominfo;

	for(int i=0;i<rownum;i++)
	{
		for(int j=0;j<colnum;j++)
		{
			if(!infostream->Read(infobuf,sizeof(infobuf),curlen))
			{
				AC_ERROR("DbagentDataDecoder::DoGetRoomAVServer:read ptr error");
				return -1;
			}
			infobuf[curlen]=0;

			switch(j)
			{
				case 0:	//roomid
				{
					roomid=atoi(infobuf);
					proominfo=g_pNetProxy->GetRoomInfo(roomid);
					if(proominfo==NULL)
					{
						AC_ERROR("DbagentDataDecoder::DoGetRoomAVServer:get roominfo error ,roomid=%d",roomid);
						break; //读取下一条记录
					}
					
				}
				break;
				case 1:	//ip_telcom
				{
					if(proominfo!=NULL&&sizeof(proominfo->AVServerIP_telcom)>curlen)
					{
						memcpy(proominfo->AVServerIP_telcom,infobuf,sizeof(proominfo->AVServerIP_telcom));
						proominfo->AVServerIP_telcom[curlen]=0;
					}
					else
					{
						AC_ERROR("DbagentDataDecoder::DoGetRoomAVServer: proominfo=%x curlen=%d",proominfo,curlen);
						break; //读取下一条记录
					}
				}
				break;
				case 2:	//port_telcom
				{
					if(proominfo!=NULL)
					{
						proominfo->AVServerPort_telcom=atoi(infobuf);
					}
					else
					{
						AC_ERROR("DbagentDataDecoder::DoGetRoomAVServer: proominfo=%x ",proominfo);
						break; //读取下一条记录
					}
				}
				break;
				case 3:	//ip_netcom
				{
					if(proominfo!=NULL&&sizeof(proominfo->AVServerIP_netcom)>curlen)
					{
						memcpy(proominfo->AVServerIP_netcom,infobuf,sizeof(proominfo->AVServerIP_netcom));
						proominfo->AVServerIP_netcom[curlen]=0;
					}
					else
					{
						AC_ERROR("DbagentDataDecoder::DoGetRoomAVServer: proominfo=%x curlen=%d",proominfo,curlen);
						break; //读取下一条记录
					}
				}
				break;
				case 4:	//port_netcom
				{
					if(proominfo!=NULL)
					{
						proominfo->AVServerPort_netcom=atoi(infobuf);
					}
					else
					{
						AC_ERROR("DbagentDataDecoder::DoGetRoomAVServer: proominfo=%x ",proominfo);
						break; //读取下一条记录
					}
				}
				break;
				default:
					break;
				}	
			
				if(j==colnum-1&&proominfo!=NULL)
				{
					AC_DEBUG("DbagentDataDecoder::DoGetRoomAVServer: roomid=%d,ip_telcom=%s,port_telcom=%d,ip_netcom=%s,port_netcom=%d",
						proominfo->roomid,proominfo->AVServerIP_telcom,proominfo->AVServerPort_telcom,proominfo->AVServerIP_netcom,proominfo->AVServerPort_netcom);
					//向所有房间在线用户发送通知
					BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();	
					char type = 65;
					outstream->Write(type);
					outstream->Write(pdata->cmd);
					outstream->Write(0);
					outstream->Write(proominfo->AVServerIP_telcom,strlen(proominfo->AVServerIP_telcom));	
					outstream->Write(proominfo->AVServerPort_telcom);	
					outstream->Write(proominfo->AVServerIP_netcom,strlen(proominfo->AVServerIP_netcom));	
					outstream->Write(proominfo->AVServerPort_netcom);	
					outstream->Flush();
		
					g_pNetProxy->BroadcastSBCmd(proominfo, *outstream);


				}	
			
		}
	}
	
	return 0;
}

int DbagentDataDecoder::DoUpdateGM(Dbresultdata* pdata,
												BinaryReadStream* infostream,
												short rownum,
												short colnum)
{
	AC_DEBUG("DbagentDataDecoder::DoUpdateGM: rownum=%d, colnum=%d",rownum,colnum);

	char infobuf[1024];
	size_t curlen;
	int idx = 0;
	int level =0;
	BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();	
	char type = 65;
	//清空原有GM列表
	map<int,GM_INFO>::iterator itg=g_pNetProxy->m_GM.begin();
	for(;itg!=g_pNetProxy->m_GM.end();itg++)
	{
		GM_INFO* pGm=(GM_INFO*)&(itg->second);
		pGm->addflag=0;
	}
	
	for(int i = 0;i < rownum;i++)
	{
		
		for(int j = 0;j < colnum;j++)
		{
			if(!infostream->Read(infobuf,sizeof(infobuf),curlen))
	            {
	                AC_ERROR("DbagentDataDecoder::DoUpdateGM:read ptr error");
	                return -1;
	            }
	            infobuf[curlen] = 0;

	            if(j == 2)
	            {
	                idx = atoi(infobuf);
	            }

	            if(j == 3)
	            {
	                level = atoi(infobuf);

	                if(level != USER_ID_GM)
	                    return -1;
	            }
				
	            if(j == colnum - 1)
	            {
	            		itg=g_pNetProxy->m_GM.find(idx);
				if(itg!=g_pNetProxy->m_GM.end())
				{
					GM_INFO* pGm=(GM_INFO*)&(itg->second);
					pGm->addflag=1;
				}
				else
				{
					GM_INFO gminfo;
					gminfo.idx=idx;
					gminfo.addflag=1;
					g_pNetProxy->m_GM.insert(make_pair(idx,gminfo));
						
					AC_DEBUG("DbagentDataDecoder::DoUpdateGM:idx = %d",idx);

					map<int, ROOM_INFO*>::iterator itroom = g_pNetProxy->m_roomlistinfo.roommap.begin();
					for(;itroom!=g_pNetProxy->m_roomlistinfo.roommap.end();itroom++)
					{
						//向GM所在房间在线用户发送通知
						ROOM_INFO* proominfo = itroom->second;
						map<int,RoomClient*>::iterator itc=proominfo->userlist.find(idx);
						if(itc!=proominfo->userlist.end())
						{
							RoomClient* pGMClient=(RoomClient*)itc->second;
							if(pGMClient)
							{
								pGMClient->m_identity=USER_ID_GM;
								
								outstream->Clear();
								outstream->Write(type);
								outstream->Write(pdata->cmd);
								outstream->Write(0);
								outstream->Write(idx);	
								outstream->Write(gminfo.addflag);
								outstream->Flush();
								g_pNetProxy->BroadcastSBCmd(proominfo, *outstream);
							}
							break;
						}
					}
				}
	            }
		}
	}

	//取消GM通知
	map<int,GM_INFO> templist=g_pNetProxy->m_GM;
	map<int,GM_INFO>::iterator ittemp=templist.begin();
	for(;ittemp!=templist.end();ittemp++)
	{
		GM_INFO* pGm=(GM_INFO*)&(ittemp->second);
		if(pGm->addflag==0)
		{
			AC_DEBUG("DbagentDataDecoder::DoUpdateGM:cancel idx = %d",pGm->idx);
			//将已取消的GM从GM列表中删除
			itg=g_pNetProxy->m_GM.find(pGm->idx);
			if(itg!=g_pNetProxy->m_GM.end())
			{
				g_pNetProxy->m_GM.erase(itg);
			}
			
			//向GM所在房间在线用户发送通知
			map<int, ROOM_INFO*>::iterator itroom = g_pNetProxy->m_roomlistinfo.roommap.begin();
			for(;itroom!=g_pNetProxy->m_roomlistinfo.roommap.end();itroom++)
			{
				ROOM_INFO* proominfo= itroom->second;
				if(proominfo)
				{
					map<int,RoomClient*>::iterator itc=proominfo->userlist.find(pGm->idx);
					if(itc!=proominfo->userlist.end())
					{
						RoomClient* pGMClient=(RoomClient*)itc->second;
						if(pGMClient)
						{
							pGMClient->m_identity=USER_ID_NONE;
									
							outstream->Clear();
							outstream->Write(type);
							outstream->Write(pdata->cmd);
							outstream->Write(0);
							outstream->Write(pGm->idx);	
							outstream->Write(pGm->addflag);
							outstream->Flush();
							g_pNetProxy->BroadcastSBCmd(proominfo, *outstream);
						}
						break;
					}
				}
				else
				{
					AC_ERROR("proominfo == NULL");
				}
			}
		}
	}
	
	return 0;
}

int DbagentDataDecoder::DoReplyInvite(Dbresultdata* pdata,BinaryReadStream* infostream)
{
	AC_DEBUG("DbagentDataDecoder::DoReplyInvite:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);

	if(pdata->roomid == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoReplyInvite:roomid error");
		return -1;
	}

	if(pdata->opidx == 0 || pdata->bopidx == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoReplyInvite:idx error");
		return -1;
	}

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(proominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoReplyInvite:proominfo error");
		return -1;
	}
	char infobuf[1024];
	size_t clen;
	if(!infostream->Read(infobuf,sizeof(infobuf),clen))
	{
		AC_ERROR("DbagentDataDecoder::DoReplyInvite:read result error");
		return -1;
	}
	infobuf[clen]=0;
	
	int result=atoi(infobuf);//0--成功, 1--已是会员, 2--超过会员最大限制
	//对操作者的回应
	char  temp[64] = {0};
	BinaryWriteStream onestream(temp,sizeof(temp));
	char type = 65;
	onestream.Write(type);
	onestream.Write((short)pdata->cmd);
	onestream.Write(pdata->seq);
	if(result==DBRESULT_VERIFY_ALREADY)	//已经是会员
		onestream.Write((int)ALREADY);				
	else if(result ==DBRESULT_VERIFY_SUCCESS)
		onestream.Write((int)SUCCESS);	
	else if(result == DBRESULT_VERIFY_FULL)
		onestream.Write((int)LISTFULL);
	onestream.Write(pdata->bopidx);
	onestream.Flush();	
	RoomClient* pClient = g_pNetProxy->GetClient(pdata->number);
	if(pClient!=NULL)
	{
		if(g_pNetProxy->SendToSrv(pClient, onestream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::DoReplyInvite:g_pNetProxy->SendToSrv error");
		}
	}
	else
	{
		AC_ERROR("DbagentDataDecoder::DoReplyInvite:pClient is NULL");
	}


	//会员加入成功，更新会员列表及向大厅发送通知
	if(result==DBRESULT_VERIFY_SUCCESS)
	{
		if(pdata->badd)	//审核通知，加入会员列表
		{
			proominfo->userlistVIP.insert(make_pair(pdata->bopidx, pdata->bopidx));

			//被操作者在房间，则身份为房间会员
			map<int,RoomClient*>::iterator itu=proominfo->userlist.find(pdata->bopidx);
			if(itu!=proominfo->userlist.end())
			{
				RoomClient* pbClient = (*itu).second;
				if(pbClient)
					pbClient->m_identity = USER_ID_VIP;
			}
			//通知房间内在线用户
			char context2[64] = {0};
			BinaryWriteStream outstream2(context2,sizeof(context2));
			char type = 65;
			outstream2.Write(type);
			outstream2.Write((short)ROOM_CMD_SB_VERIFY_USER_APP);
			outstream2.Write(0);
			outstream2.Write((int)pdata->opidx);	//邀请者
			outstream2.Write((int)pdata->bopidx);	//被邀请者
			outstream2.Flush();	
			g_pNetProxy->BroadcastSBCmd(proominfo,outstream2) ;
			
		}
		char context[64] = {0};
		BinaryWriteStream outstream(context,sizeof(context));
		outstream.Write((short)HALL_CMD_APPLYMEMBER_R2L);
		outstream.Write((int)0);
		outstream.Write((char)ROOM_TYPE_ROOM);	//ADD BY JINGUANFU 2010/7/9
		outstream.Write(pdata->roomid);	//房间id
		outstream.Write(pdata->opidx);	//邀请者
		outstream.Write(pdata->bopidx);//被邀请者
		int ref=pdata->badd;
		outstream.Write(ref);				//审核结果0--拒绝1--通过
		outstream.Flush();

		HallSvrClient *pHallSvr = g_pNetProxy->GetHallSvr();
		if(pHallSvr!=NULL)
			pHallSvr->AddBuf(outstream.GetData(), outstream.GetSize());
		else
			AC_ERROR("DbagentDataDecoder::DoReplyInvite:pHallSvr is NULL");
	
	}

	return 0;
}

int DbagentDataDecoder::DoGetGiftSend(Dbresultdata* pdata,
									BinaryReadStream* infostream,
									short rownum,
									short colnum)
{

	AC_DEBUG("DbagentDataDecoder::DoGetGiftSend:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);
	
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(proominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoGetGiftSend:proominfo error");
		return -1;
	}
	if(colnum!=3)
	{
		AC_ERROR("DbagentDataDecoder::DoGetGiftSend:DB result error,rownum=%d,colnum=%d",rownum,colnum);
		return -1;
	}

	BinaryWriteStream *outstream=StreamFactory::InstanceWriteStream();
	char type = 65;
	
	if(rownum==0)
	{
		outstream->Write(type);
		outstream->Write((short)pdata->cmd);
		outstream->Write(pdata->seq);
		outstream->Write((short)0);			//无记录
		outstream->Flush();	
	}
	else
	{

		outstream->Write(type);
		outstream->Write((short)pdata->cmd);
		outstream->Write(pdata->seq);
		outstream->Write(rownum);	//记录条数
		
		//数据库返回值
		char infobuf[1024];
		size_t clen;

		int idx=0;
		int giftid=0;
		int giftcount=0;
		
		for(int i=0;i<rownum;i++)
		{
			idx=0;
			giftid=0;
			giftcount=0;
			//接收者IDX
			if(!infostream->Read(infobuf,sizeof(infobuf),clen))
			{
				AC_ERROR("DbagentDataDecoder::DoGetGiftSend:read income error,rownum=%d",rownum);
				return -1;
			}
			infobuf[clen]=0;
			idx=atoi(infobuf);
			outstream->Write(idx);
			//礼物ID
			if(!infostream->Read(infobuf,sizeof(infobuf),clen))
			{
				AC_ERROR("DbagentDataDecoder::DoGetGiftSend:read date error,rownum=%d",rownum);
				return -1;
			}
			infobuf[clen]=0;
			giftid=atoi(infobuf);
			outstream->Write(giftid);

			//礼物总数
			if(!infostream->Read(infobuf,sizeof(infobuf),clen))
			{
				AC_ERROR("DbagentDataDecoder::DoGetGiftSend:read date error,rownum=%d",rownum);
				return -1;
			}
			infobuf[clen]=0;
			giftcount=atoi(infobuf);
			outstream->Write(giftcount);
		}
		outstream->Flush();	

	}
	
	RoomClient* pClient = g_pNetProxy->GetClient(pdata->number);
	if(pClient!=NULL)
	{
		if(g_pNetProxy->SendToSrv(pClient, *outstream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::DoGetGiftSend:SendToSrv error");
		}
	}
	else
	{
		AC_ERROR("DbagentDataDecoder::DoGetGiftSend:pClient is NULL ");
	}
	
	

	return 0;
}

int DbagentDataDecoder::DoGetGiftRecv(Dbresultdata* pdata,
									BinaryReadStream* infostream,
									short rownum,
									short colnum)
{
	AC_DEBUG("DbagentDataDecoder::DoGetGiftRecv:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);
	
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(proominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoGetGiftRecv:proominfo error");
		return -1;
	}
	if(colnum!=3)
	{
		AC_ERROR("DbagentDataDecoder::DoGetGiftRecv:DB result error,rownum=%d,colnum=%d",rownum,colnum);
		return -1;
	}

	BinaryWriteStream *outstream=StreamFactory::InstanceWriteStream();
	char type = 65;
	
	if(rownum==0)
	{
		outstream->Write(type);
		outstream->Write((short)pdata->cmd);
		outstream->Write(pdata->seq);
		outstream->Write((short)0);			//无记录
		outstream->Flush();	
	}
	else
	{

		outstream->Write(type);
		outstream->Write((short)pdata->cmd);
		outstream->Write(pdata->seq);
		outstream->Write(rownum);	//记录条数
		
		//数据库返回值
		char infobuf[1024];
		size_t clen;

		int idx=0;
		int giftid=0;
		int giftcount=0;
		
		for(int i=0;i<rownum;i++)
		{
			idx=0;
			giftid=0;
			giftcount=0;
			//赠送者IDX
			if(!infostream->Read(infobuf,sizeof(infobuf),clen))
			{
				AC_ERROR("DbagentDataDecoder::DoGetGiftRecv:read income error,rownum=%d",rownum);
				return -1;
			}
			infobuf[clen]=0;
			idx=atoi(infobuf);
			outstream->Write(idx);
			//礼物ID
			if(!infostream->Read(infobuf,sizeof(infobuf),clen))
			{
				AC_ERROR("DbagentDataDecoder::DoGetGiftRecv:read date error,rownum=%d",rownum);
				return -1;
			}
			infobuf[clen]=0;
			giftid=atoi(infobuf);
			outstream->Write(giftid);

			//礼物总数
			if(!infostream->Read(infobuf,sizeof(infobuf),clen))
			{
				AC_ERROR("DbagentDataDecoder::DoGetGiftRecv:read date error,rownum=%d",rownum);
				return -1;
			}
			infobuf[clen]=0;
			giftcount=atoi(infobuf);
			outstream->Write(giftcount);
		}
		outstream->Flush();	

	}
	
	RoomClient* pClient = g_pNetProxy->GetClient(pdata->number);
	if(pClient!=NULL)
	{
		if(g_pNetProxy->SendToSrv(pClient, *outstream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::DoGetGiftRecv:SendToSrv error");
		}
	}
	else
	{
		AC_ERROR("DbagentDataDecoder::DoGetGiftRecv:pClient is NULL ");
	}

	return 0;
}

//后台未支持该功能，未完善
int DbagentDataDecoder::DoUpdateRoomManger(Dbresultdata* pdata,
									BinaryReadStream* infostream,
									short rownum,
									short colnum)
{

	AC_DEBUG("DbagentDataDecoder::DoUpdateRoomManger:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);
	

	ROOM_INFO* pRoominfo=g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(pRoominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoUpdateRoomManger:roomid=%d not exist",pdata->roomid);
		return 0;
	}
		
	char infobuf[1024];
	size_t curlen;

	map<int,ROOM_MANAGER> newmangerlist;
	map<int,int> newmemberlist;
	//map<int,int> delmangerlist;
	
	for(int i = 0;i < rownum;i++)
	{
		int idx = 0;
		for(int j = 0;j < colnum;j++)
		{
			if(!infostream->Read(infobuf,sizeof(infobuf),curlen))
			{
				AC_ERROR("DbagentDataDecoder::DoUpdateRoomManger::read ptr error");
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
				ROOM_MANAGER roommanager;
				roommanager.m_idx=idx;
				roommanager.m_identity=(char)level;
				newmangerlist.insert(make_pair(idx,roommanager));
				
				AC_DEBUG("DbagentDataDecoder::DoUpdateRoomManger::rownum = %d colnum = %d level = %d idx = %d "
						,i,j,level, idx);
			}
		}
	}

	char context[64] = {0};
	BinaryWriteStream outstream(context,sizeof(context));
	char type = 65;
	
	//房间管理员更新
	map<int,ROOM_MANAGER>::iterator itroommanger=pRoominfo->managerlist.begin();
	for(;itroommanger!=pRoominfo->managerlist.end();itroommanger++)
	{
		ROOM_MANAGER* pManger=(ROOM_MANAGER*)(&(itroommanger->second));
		int level = pManger->m_identity;
		int idx	= pManger->m_idx;

		map<int,ROOM_MANAGER>::iterator itmanger=newmangerlist.find(idx);
/*		
		if(itmanger==newmangerlist.end())
		{
			//在线会员状态更新
			map<int,RoomClient*>::iterator itUser=pRoominfo->userlist.find(idx);
			if(itUser!=pRoominfo->userlist.end())
			{
				RoomClient* pRoomClient=itUser->second;
				pRoomClient->m_identity=USER_ID_NONE;
			}

			//删除会员
			map<int,int>::iterator itVIP=pRoominfo->userlistVIP.find(idx);
			if(itVIP!=pRoominfo->userlistVIP.end())
				pRoominfo->userlistVIP.erase(itVIP); 

			delmangerlist.insert(make_pair(idx,idx));
			//通知房间内在线人员
			outstream.Clear();
			outstream.Write(type);
			outstream.Write((short)ROOM_CMD_SB_REMOVE_USER);
			outstream.Write(0);
			outstream.Write(pdata->opidx);		//操作者idx
			outstream.Write(idx);				//被删除者idx	
			outstream.Flush();	
			g_pNetProxy->BroadcastSBCmd(pRoominfo,outstream);
		}
*/
		ROOM_MANAGER newroommanager=itmanger->second;
		int newlevel=newroommanager.m_identity;

		pRoominfo->vjlist.clear();
		pRoominfo->vjlist_a.clear();
		pRoominfo->secondownidx=0;
		pRoominfo->secondownidx2=0;
		pRoominfo->ownidx=0;

		pManger->m_identity=newlevel;
		
		if(newlevel == USER_ID_VJ)
		{
			pRoominfo->vjlist.insert(make_pair(idx, idx));
			if(newlevel!=level)
			{
				//在线会员状态更新
				map<int,RoomClient*>::iterator itUser=pRoominfo->userlist.find(idx);
				if(itUser!=pRoominfo->userlist.end())
				{
					RoomClient* pRoomClient=itUser->second;
					pRoomClient->m_identity=USER_ID_VJ;
				}
				//通知房间内在线人员
				outstream.Clear();
				outstream.Write(type);
				outstream.Write((short)ROOM_CMD_SB_GIVE_VJ);
				outstream.Write(0);
				outstream.Write(pdata->opidx);		//操作者
				outstream.Write(pdata->bopidx); //被操作者
				char level=USER_ID_VJ;			//权限
				outstream.Write(level);
				outstream.Flush();	
				g_pNetProxy->BroadcastSBCmd(pRoominfo,outstream);
			}
		}
		else if(newlevel == USER_ID_VJ_A)
		{
			pRoominfo->vjlist_a.insert(make_pair(idx, idx));	
			if(newlevel!=level)
			{
				//在线会员状态更新
				map<int,RoomClient*>::iterator itUser=pRoominfo->userlist.find(idx);
				if(itUser!=pRoominfo->userlist.end())
				{
					RoomClient* pRoomClient=itUser->second;
					pRoomClient->m_identity=USER_ID_VJ_A;
				}
				//通知房间内在线人员
				outstream.Clear();
				outstream.Write(type);
				outstream.Write((short)ROOM_CMD_SB_GIVE_VJ);
				outstream.Write(0);
				outstream.Write(pdata->opidx);		//操作者
				outstream.Write(pdata->bopidx); //被操作者
				char level=USER_ID_VJ_A;			//权限
				outstream.Write(level);
				outstream.Flush();	
				g_pNetProxy->BroadcastSBCmd(pRoominfo,outstream);
			}
		}
		else if(newlevel == USER_ID_OWNER)
		{
			if(pRoominfo->ownidx !=0)
				continue;
			else
				pRoominfo->ownidx=idx;

			if(newlevel!=level)
			{
				//在线会员状态更新
				map<int,RoomClient*>::iterator itUser=pRoominfo->userlist.find(idx);
				if(itUser!=pRoominfo->userlist.end())
				{
					RoomClient* pRoomClient=itUser->second;
					pRoomClient->m_identity=USER_ID_OWNER;
				}
				//通知房间内在线人员
				outstream.Clear();
				outstream.Write(type);
				outstream.Write((short)ROOM_CMD_SB_GIVE_OWNER);
				outstream.Write(0);
				outstream.Write(pdata->opidx);		//操作者
				outstream.Write(pdata->bopidx); //被操作者
				char level=USER_ID_OWNER;			//权限
				outstream.Write(level);
				outstream.Flush();	
				g_pNetProxy->BroadcastSBCmd(pRoominfo,outstream);
			}
			
		}
		else if(newlevel == USER_ID_OWNER_S)
		{
			if(pRoominfo->secondownidx == 0)
				pRoominfo->secondownidx = idx;	
			else if(pRoominfo->secondownidx2 == 0)
				pRoominfo->secondownidx2 = idx;
			else
				continue;

			if(newlevel!=level)
			{
				//在线会员状态更新
				map<int,RoomClient*>::iterator itUser=pRoominfo->userlist.find(idx);
				if(itUser!=pRoominfo->userlist.end())
				{
					RoomClient* pRoomClient=itUser->second;
					pRoomClient->m_identity=USER_ID_OWNER_S;
				}
				//通知房间内在线人员
				outstream.Clear();
				outstream.Write(type);
				outstream.Write((short)ROOM_CMD_SB_GIVE_OUER_S);
				outstream.Write(0);
				outstream.Write(pdata->opidx);		//操作者
				outstream.Write(pdata->bopidx); //被操作者
				char level=USER_ID_OWNER_S;			//权限
				outstream.Write(level);
				outstream.Flush();	
				g_pNetProxy->BroadcastSBCmd(pRoominfo,outstream);
			}
	
		}	
	}
/*
	//从管理员列表中删除会员
	map<int,int>::iterator itdel=delmangerlist.begin();
	for(;itdel!=delmangerlist.end();itdel++)
	{
		int idx=itdel->first;

		map<int,ROOM_MANAGER>::iterator itdelmanger=pRoominfo->managerlist.find(idx);
		if(itdelmanger!=pRoominfo->managerlist.end())
			pRoominfo->managerlist.erase(itdelmanger);
	}

	delmangerlist.clear();
*/
	AC_DEBUG("DbagentDataDecoder::DoUpdateRoomManger:success end,roomid=%d",pdata->roomid);
			
	return 0;
}


int DbagentDataDecoder::DoUpdateRoomBlacklist(Dbresultdata* pdata,
									BinaryReadStream* infostream,
									short rownum,
									short colnum)
{

	AC_DEBUG("DbagentDataDecoder::DoUpdateRoomBlacklist:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);
	

	ROOM_INFO* pRoominfo=g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(pRoominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoUpdateRoomBlacklist:roomid=%d not exist",pdata->roomid);
		return 0;
	}

	map<int,int> newblacklist;
	map<int,int> delblacklist;
	
	char infobuf[1024];
	size_t curlen;
	int idx=0;
	for(int i = 0;i < rownum;i++)
	{
		for(int j = 0;j < colnum;j++)
		{
			if(!infostream->Read(infobuf,sizeof(infobuf),curlen))
			{
				AC_ERROR("DbagentDataDecoder::DoUpdateRoomBlacklist:read ptr error");
				return -1;
			}
			infobuf[curlen] = 0;

			if(j == colnum - 1)
			{
				idx= atoi(infobuf);
				newblacklist.insert(make_pair(idx, idx));
				AC_DEBUG("DbagentDataDecoder::DoUpdateRoomBlacklist:rownum = %d colnum = %d idx = %d",i,j,idx);
			}
		}
	}

	BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
	char type = 65;
	
	map<int,int>::iterator itnewblacklist=newblacklist.begin();

	//新增黑名单
	for(;itnewblacklist!=newblacklist.end();itnewblacklist++)
	{
		int idx_add = itnewblacklist->first;
		map<int,int>::iterator itblack= pRoominfo->blacklist.find(idx_add);
		if(itblack==pRoominfo->blacklist.end())
		{
			char badd = 1; //0 == delete, 1 == add
			//发送给房间内所有的人,有人被添加/删除入黑名单
			outstream->Write(type);
			outstream->Write((short)ROOM_CMD_SB_BLACKLIST_UPDATE);
			outstream->Write((int)0);
			outstream->Write(idx_add);  
			outstream->Write(badd);
			outstream->Flush();
			g_pNetProxy->BroadcastSBCmd(pRoominfo,*outstream);

			map<int,RoomClient*>::iterator itUser=pRoominfo->userlist.find(idx_add);

			if(itUser!=pRoominfo->userlist.end())
			{
				RoomClient* pClient=itUser->second;
				pClient->LeaveRoom();
			}

			map<int,RoomClient*>::iterator itVUser=pRoominfo->vuserlist.find(idx_add);
			if(itVUser!=pRoominfo->vuserlist.end())
			{
				RoomClient* pClient=itVUser->second;
				pClient->VLeaveRoom();
			}

		}
	}

	//删除黑名单
	map<int,int>::iterator itblacklist=pRoominfo->blacklist.begin();
	for(;itblacklist!=pRoominfo->blacklist.end();itblacklist++)
	{
		int idx_del = itblacklist->first;
		map<int,int>::iterator itnewblack= newblacklist.find(idx_del);
		if(itnewblack==newblacklist.end())
		{
			char badd = 0; //0 == delete, 1 == add
			//发送给房间内所有的人,有人被添加/删除入黑名单
			outstream->Write(type);
			outstream->Write((short)ROOM_CMD_SB_BLACKLIST_UPDATE);
			outstream->Write((int)0);
			outstream->Write(idx_del);  
			outstream->Write(badd);
			outstream->Flush();
			g_pNetProxy->BroadcastSBCmd(pRoominfo,*outstream);

		}
			
	}

	//更新房间内黑名单
	pRoominfo->blacklist.clear();
	itnewblacklist=newblacklist.begin();
	for(;itnewblacklist!=newblacklist.end();itnewblacklist++)
	{
		int idx_new = itnewblacklist->first;

		pRoominfo->blacklist.insert(make_pair(idx_new,idx_new));

	}

	newblacklist.clear();
	
	AC_DEBUG("DbagentDataDecoder::DoUpdateRoomBlacklist:success end,roomid=%d",pdata->roomid);
			
	return 0;
}

int DbagentDataDecoder::DoUpdateRoomMemberlist(Dbresultdata* pdata,
									BinaryReadStream* infostream,
									short rownum,
									short colnum)
{
	AC_DEBUG("DbagentDataDecoder::DoUpdateRoomMemberlist:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
			pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);
		
	
		ROOM_INFO* pRoominfo=g_pNetProxy->GetRoomInfo(pdata->roomid);
		if(pRoominfo == NULL)
		{
			AC_ERROR("DbagentDataDecoder::DoUpdateRoomMemberlist:roomid=%d not exist",pdata->roomid);
			return 0;
		}
			
		char infobuf[1024];
		size_t curlen;
	
		map<int,ROOM_MANAGER> newmangerlist;
		map<int,int> newmemberlist;
		
		for(int i = 0;i < rownum;i++)
		{
			int idx = 0;
			for(int j = 0;j < colnum;j++)
			{
				if(!infostream->Read(infobuf,sizeof(infobuf),curlen))
				{
					AC_ERROR("DbagentDataDecoder::DoUpdateRoomMemberlist::read ptr error");
					return -1;
				}
				infobuf[curlen] = 0;
				if(j == colnum - 2)
				{
					idx = atoi(infobuf);
				}
		
				if(j == colnum - 1)
				{
					newmemberlist.insert(make_pair(idx,idx));
					int level = atoi(infobuf);
					ROOM_MANAGER roommanager;
					roommanager.m_idx=idx;
					roommanager.m_identity=(char)level;
					newmangerlist.insert(make_pair(idx,roommanager));
					
					AC_DEBUG("DbagentDataDecoder::DoUpdateRoomMemberlist::rownum = %d colnum = %d level = %d idx = %d "
							,i,j,level, idx);
				}
			}
		}
	
		char context[64] = {0};
		BinaryWriteStream outstream(context,sizeof(context));
		char type = 65;

		map<int,int>::iterator itMember;
		map<int,int>::iterator itNewMember;

		//删除房间会员
		itMember=pRoominfo->userlistVIP.begin();
		for(;itMember!=pRoominfo->userlistVIP.end();itMember++)
		{
			int idx_del=itMember->first;

			itNewMember=newmemberlist.find(idx_del);
			if(itNewMember==newmemberlist.end())
			{
				//通知房间内在线人员
				outstream.Clear();
				outstream.Write(type);
				outstream.Write((short)ROOM_CMD_SB_REMOVE_USER);
				outstream.Write(0);
				outstream.Write(pdata->opidx);	//操作者idx
				outstream.Write(idx_del);		//被删除者idx	
				outstream.Flush();	
				g_pNetProxy->BroadcastSBCmd(pRoominfo,outstream);

				//在线会员状态更新
				map<int,RoomClient*>::iterator itUser=pRoominfo->userlist.find(idx_del);
				if(itUser!=pRoominfo->userlist.end())
				{
					RoomClient* pRoomClient=itUser->second;
					pRoomClient->m_identity=USER_ID_NONE;
				}
				
				map<int,ROOM_MANAGER>::iterator itmanger=pRoominfo->managerlist.find(idx_del);
				if(itmanger!=pRoominfo->managerlist.end())
				{
					pRoominfo->managerlist.erase(itmanger);
				}

				map<int,ROOM_MANAGER>::iterator itmanger_online=pRoominfo->managerlist_online.find(idx_del);
				if(itmanger_online!=pRoominfo->managerlist_online.end())
				{
					pRoominfo->managerlist_online.erase(itmanger_online);
				}
				map<int,int>::iterator itvj=pRoominfo->vjlist.find(idx_del);
				if(itvj!=pRoominfo->vjlist.end())
					pRoominfo->vjlist.erase(itvj);

				map<int,int>::iterator itvj_a=pRoominfo->vjlist_a.find(idx_del);
				if(itvj_a!=pRoominfo->vjlist_a.end())
					pRoominfo->vjlist_a.erase(itvj_a);

				if(idx_del==pRoominfo->ownidx)
					pRoominfo->ownidx=0;
				else if(idx_del==pRoominfo->secondownidx)
					pRoominfo->secondownidx=0;
				else if(idx_del==pRoominfo->secondownidx2)
					pRoominfo->secondownidx2=0;
				
			}
		}

		
		//新增房间会员
		itNewMember=newmemberlist.begin();
		for(;itNewMember!=newmemberlist.end();itNewMember++)
		{
			int idx_add=itNewMember->first;
			itMember=pRoominfo->userlistVIP.find(idx_add);
			if(itMember==pRoominfo->userlistVIP.end())
			{
				outstream.Write(type);
				outstream.Write((short)ROOM_CMD_SB_VERIFY_USER_APP);
				outstream.Write(0);
				outstream.Write((int)pdata->opidx);	//操作者idx
				outstream.Write((int)idx_add);		//申请者idx	
				outstream.Flush();	
				g_pNetProxy->BroadcastSBCmd(pRoominfo,outstream) ;
			}
		}

		//同步房间会员列表 
		pRoominfo->userlistVIP.clear();
		itNewMember=newmemberlist.begin();
		for(;itNewMember!=newmemberlist.end();itNewMember++)
		{
			int idx_add=itNewMember->first;
			pRoominfo->userlistVIP.insert(make_pair(idx_add,idx_add));
			//在线会员状态更新
			map<int,RoomClient*>::iterator itUser=pRoominfo->userlist.find(idx_add);
			if(itUser!=pRoominfo->userlist.end())
			{
				RoomClient* pRoomClient=itUser->second;
				pRoomClient->m_identity=USER_ID_VIP;
			}
		}

		AC_DEBUG("DbagentDataDecoder::DoUpdateRoomMemberlist::roomid= %d process success.",
							pRoominfo->roomid);

		

	return 0;
}

//add by jinguanfu 2011/8/19
int DbagentDataDecoder::DoDisableMAC(Dbresultdata* pdata,BinaryReadStream* infostream)
{

	AC_DEBUG("DbagentDataDecoder::DoDisableMAC:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);

	if(pdata->roomid == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoDisableMAC:roomid error");
		return -1;
	}

	if(pdata->opidx == 0 || pdata->bopidx == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoDisableMAC:idx error");
		return -1;
	}

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(proominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoDisableMAC:proominfo error");
		return -1;
	}

		char infobuf[1024];
	size_t clen;
	if(!infostream->Read(infobuf,sizeof(infobuf),clen))
	{
		AC_ERROR("DbagentDataDecoder::DoDisableMAC:read result error");
		return -1;
	}
	infobuf[clen]=0;

	int result = atoi(infobuf);
	int ret=0;
	if(result == 0)		
	{
		ret=0;			//成功
		ROOM_INFO* pRoomInfo=g_pNetProxy->GetRoomInfo(pdata->roomid);
		if(pRoomInfo!=NULL)
		{
			//踢被禁者出房间
			map<int,RoomClient*>::iterator itB=proominfo->userlist.find(pdata->bopidx);
			if(itB!=proominfo->userlist.end())
			{
				RoomClient* pBClient=itB->second;
				if(pBClient!=NULL)
					pBClient->Close();
			}
			// 通知大厅
			BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
			outstream2->Write((short)HALL_CMD_KICKUSER_R2L);
			outstream2->Write((int)0);
			outstream2->Write((char)ROOM_TYPE_ROOM);	//ADD BY JINGUANFU 2010/7/9
			outstream2->Write((int)pdata->bopidx);
			outstream2->Flush();

			HallSvrClient *pHallSvr = g_pNetProxy->GetHallSvr();
			if(pHallSvr!=NULL)
				pHallSvr->AddBuf(outstream2->GetData(), outstream2->GetSize());
			else
				AC_ERROR("DbagentDataDecoder::DoDisableMAC:pHallSvr is NULL");
		}
		else
		{
			AC_ERROR("DbagentDataDecoder::DoDisableMAC:roomid error roomid=%d",pdata->roomid);
		}		
	}
	else 
	{
		ret=-1;			//失败
	}
	//回应操作用户
	char  temp[64] = {0};
	
	BinaryWriteStream onestream(temp,sizeof(temp));
	char type = 65;
	onestream.Write(type);
	onestream.Write((short)pdata->cmd);
	onestream.Write(pdata->seq);
	onestream.Write(ret);	//ret	
	onestream.Write(pdata->bopidx);
	onestream.Flush();	

	RoomClient* pClient = g_pNetProxy->GetClient(pdata->number);
	if(pClient!=NULL)
	{
		if(g_pNetProxy->SendToSrv(pClient, onestream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::DoDisableMAC:g_pNetProxy->SendToSrv error");
		}
	}
	else
	{
		AC_ERROR("DbagentDataDecoder::DoDisableMAC:pClient is NULL");
	}


	return 0;
}

//add by jinguanfu 2011/8/19
int DbagentDataDecoder::DoDisableIP(Dbresultdata* pdata,BinaryReadStream* infostream)
{

	AC_DEBUG("DbagentDataDecoder::DoDisableIP:opidx=%d,boidx=%d,roomid=%d,outseq=%d",
		pdata->opidx,pdata->bopidx,pdata->roomid,pdata->outseq);

	if(pdata->roomid == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoDisableIP:roomid error");
		return -1;
	}

	if(pdata->opidx == 0 || pdata->bopidx == 0)
	{
		AC_ERROR("DbagentDataDecoder::DoDisableIP:idx error");
		return -1;
	}

	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(pdata->roomid);
	if(proominfo == NULL)
	{
		AC_ERROR("DbagentDataDecoder::DoDisableIP:proominfo error");
		return -1;
	}

		char infobuf[1024];
	size_t clen;
	if(!infostream->Read(infobuf,sizeof(infobuf),clen))
	{
		AC_ERROR("DbagentDataDecoder::DoDisableIP:read result error");
		return -1;
	}
	infobuf[clen]=0;

	int result = atoi(infobuf);
	int ret=0;
	if(result == 0)		
	{
		ret=0;			//成功
		ROOM_INFO* pRoomInfo=g_pNetProxy->GetRoomInfo(pdata->roomid);
		if(pRoomInfo!=NULL)
		{
			//踢被禁者出房间
			map<int,RoomClient*>::iterator itB=proominfo->userlist.find(pdata->bopidx);
			if(itB!=proominfo->userlist.end())
			{
				RoomClient* pBClient=itB->second;
				if(pBClient!=NULL)
					pBClient->Close();
			}
			// 通知大厅
			BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
			outstream2->Write((short)HALL_CMD_KICKUSER_R2L);
			outstream2->Write((int)0);
			outstream2->Write((char)ROOM_TYPE_ROOM);	//ADD BY JINGUANFU 2010/7/9
			outstream2->Write((int)pdata->bopidx);
			outstream2->Flush();

			HallSvrClient *pHallSvr = g_pNetProxy->GetHallSvr();
			if(pHallSvr!=NULL)
				pHallSvr->AddBuf(outstream2->GetData(), outstream2->GetSize());
			else
				AC_ERROR("DbagentDataDecoder::DoDisableIP:pHallSvr is NULL");
		}
		else
		{
			AC_ERROR("DbagentDataDecoder::DoDisableIP:roomid error roomid=%d",pdata->roomid);
		}		
	}
	else 
	{
		ret=-1;			//失败
	}

	//回应操作用户
	char  temp[64] = {0};
	
	BinaryWriteStream onestream(temp,sizeof(temp));
	char type = 65;
	onestream.Write(type);
	onestream.Write((short)pdata->cmd);
	onestream.Write(pdata->seq);
	onestream.Write(ret);	//ret	
	onestream.Write(pdata->bopidx);
	onestream.Flush();	

	RoomClient* pClient = g_pNetProxy->GetClient(pdata->number);
	if(pClient!=NULL)
	{
		if(g_pNetProxy->SendToSrv(pClient, onestream)==-1)
		{
			AC_ERROR("DbagentDataDecoder::DoDisableIP:g_pNetProxy->SendToSrv error");
		}
	}
	else
	{
		AC_ERROR("DbagentDataDecoder::DoDisableIP:pClient is NULL");
	}
	return 0;
}

//add by jinguanfu 2011/8/19
int DbagentDataDecoder::DoUpdateChatBlacklist(Dbresultdata *pdata,BinaryReadStream* infostream,short rownum,short colnum)
{
	AC_DEBUG("DbagentDataDecoder::DoUpdateChatBlacklist: rownum=%d, colnum=%d",rownum,colnum);

	NOT_USED(pdata);
	
	map<int,int> tempBlacklist;
	char infobuf[1024];
	size_t curlen;
	int idx=0;
	for(int i = 0;i < rownum;i++)
	{
		for(int j = 0;j < colnum;j++)
		{
			if(!infostream->Read(infobuf,sizeof(infobuf),curlen))
			{
				AC_ERROR("DbagentDataDecoder::DoUpdateChatBlacklist:read ptr error");
				return -1;
			}
			infobuf[curlen] = 0;

			if(j == colnum - 1)
			{
				idx= atoi(infobuf);
				tempBlacklist.insert(make_pair(idx, idx));
				AC_DEBUG("DbagentDataDecoder::DoUpdateChatBlacklist:rownum = %d colnum = %d idx = %d",i,j,idx);
			}
		}
	}

	g_pNetProxy->m_ChatBlacklist.clear();
	g_pNetProxy->m_ChatBlacklist=tempBlacklist;

	return 0;
}

int DbagentDataDecoder::DoUpdateGiftConfig(Dbresultdata *pdata,BinaryReadStream* infostream,short rownum,short colnum)
{
	AC_DEBUG("DbagentDataDecoder::DoUpdateGiftConfig:rownum=%d, colnum=%d",rownum,colnum);

	NOT_USED(pdata);

	char infobuf[1024];
	size_t curlen;
	//add by lihongwu 2011/10/11
	int giftprice = 0;     //礼物可配置化价格配置
	int giftnum = 0;       //每次刷礼物个数
	int giftinterval = 0;  //刷礼物间隔时间
	if (rownum<=0 || colnum!=3)
	{
		AC_ERROR("NetProxy::DoUpdateGiftConfig:rownum=%d,colnum=%d error",rownum,colnum);
		return -1;
	}

	g_pNetProxy->m_giftconflist.clear();
	for(int i=0;i<rownum;i++)
	{
		for(int j = 0;j < colnum;j++)
		{
			if(!infostream->Read(infobuf,sizeof(infobuf),curlen))
			{
				AC_ERROR("NetProxy::DoUpdateGiftConfig:read giftconfig error");
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
		g_pNetProxy->m_giftconflist.insert(make_pair(giftprice,giftconfig));
		AC_DEBUG("DbagentDataDecoder::DoUpdateGiftConfig:price=%d,number=%d,interval=%d ",giftprice,giftnum,giftinterval);

	}

	//把礼物配置信息拼到礼物信息里 
	map<int,GIFT_CONFIG>::iterator itconfig = g_pNetProxy->m_giftconflist.begin();
	map<int,GIFT_CONFIG>::iterator itconfig2 = itconfig;
	map<int,GIFT_INFO>::iterator itinfo = g_pNetProxy->m_GiftInfo.begin();
	for (;itinfo != g_pNetProxy->m_GiftInfo.end();itinfo++)
	{
		GIFT_INFO& giftinfo = itinfo->second;               //引用类型
		for (;itconfig != g_pNetProxy->m_giftconflist.end();itconfig++)
		{
			itconfig2 = itconfig;
			itconfig2++;
			GIFT_CONFIG giftconf = itconfig->second;
			if (itconfig2 != g_pNetProxy->m_giftconflist.end()) 
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

	AC_DEBUG("NetProxy::DoUpdateGiftConfig: giftinfo add giftconfig ");
	return 0;
}

