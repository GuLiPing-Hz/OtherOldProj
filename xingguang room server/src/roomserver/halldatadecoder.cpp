#include "netproxy.h"
#include "ac/log/log.h"
#include "ac/util/md5.h"
#include "ac/util/protocolstream.h"
#include "StreamFactory.h"

int HallDataDecoder::OnPackage(ClientSocketBase* pClient,const char* buf,size_t buflen)
{
	//char ip[32];
	//pClient->GetPeerIp(ip);
	//AC_DEBUG("%s%s%d",ip,buf,buflen);

	NOT_USED(pClient);

    BinaryReadStream instream(buf, buflen);
    short cmd;
    if(!instream.Read(cmd))
    {
        AC_ERROR("HallDataDecoder::OnPackage:read cmd error");
        return -1;
    }
    int seq;
    if(!instream.Read(seq))
    {
        AC_ERROR("HallDataDecoder::OnPackage:read seq error");
        return -1;
    }

    switch(cmd)
    {
	case HALL_CMD_CREATEROOM://Receive from lobby when a roomcreate by ym

		if(DoCreateRoom( cmd, seq, &instream) == -1)
		{
			AC_ERROR("HallDataDecoder::OnPackage:DoCreateRoom error");
			return 0;
		}			
		break;		
	case HALL_CMD_DELETEROOM:
		if(DoDeleteRoom(/*pClient,  cmd, seq,*/ &instream) == -1)
		{
			AC_ERROR("HallDataDecoder::OnPackage:DoDeleteRoom error");
			return 0;
		}
		break;
	case HALL_CMD_LOCKROOM://Receive from lobby when a roomcreate by ym
		if(DoBlockRoom( cmd, seq, &instream) == -1)
		{
			AC_ERROR("HallDataDecoder::OnPackage:DoBlockRoom error");
			return 0; 
		}
		break;
	case HALL_CMD_KICKUSER_L2R:
		if(DoKickUser(&instream)==-1)
		{
			AC_ERROR("HallDataDecoder::OnPackage:DoKickUser error");
			return 0; 
		}
		break;
	
	case HALL_CMD_UPDATEGOLD_L2R:
		if(DoUpdateCoin(&instream)==-1)
		{
			AC_ERROR("HallDataDecoder::OnPackage:DoUpdateCoin error");
			return 0; 
		}
		break;
	case HALL_CMD_CHANGEAVATAR_L2R:
		if(DoChangeAvatarNotice(&instream)==-1)
		{
			AC_ERROR("HallDataDecoder::OnPackage:DoChangeAvatarNotice error");
			return 0; 
		}
		break;

	case HALL_CMD_LEAVELOBBY_L2R:
		if(DoLeaveLobby(&instream)==-1)
		{
			AC_ERROR("HallDataDecoder::OnPackage:DoLeaveLobby error");
			return 0; 
		}
		break;
	case HALL_CMD_CHANGELEVEL:
		if(DoUpdateLevel(&instream)==-1)
		{
			AC_ERROR("HallDataDecoder::OnPackage:DoUpdateLevel error");
			return 0; 
		}
		break;
	/*
	case HALL_CMD_OVERDUE_ITEM_L2R:
		if(DoUpdateItem(&instream)==-1)
		{
			AC_ERROR("HallDataDecoder::OnPackage:DoUpdateItem error");
			return 0; 
		}
		break;
	*/
	case HALL_CMD_OVERDUE_VIP_L2R:
		if(DoUpdateVip(&instream)==-1)
		{
			AC_ERROR("HallDataDecoder::OnPackage:DoUpdateVip error");
			return 0; 
		}
		break;
	case HALL_CMD_GETUSERINFO:
		if(DoUpdateUserinfo(cmd,seq,&instream)==-1)
		{
			AC_ERROR("HallDataDecoder::OnPackage:DoUpdateUserinfo error");
			return 0; 
		}
		break;
	case HALL_CMD_UPDATEROOM:
		if(DoUpdateRoomInfo(cmd,seq,&instream)==-1)
		{
			AC_ERROR("HallDataDecoder::OnPackage:DoUpdateRoomInfo error");
			return 0; 
		}
		break;
	
	case HALL_CMD_ROOMMANAGERCHG:
		if(DoUpdateRoomManger(cmd,seq,&instream)==-1)
		{
			AC_ERROR("HallDataDecoder::OnPackage:DoUpdateRoomManger error");
			return -1; 
		}
		break;
 	case HALL_CMD_SYNC_ROOMINFO:
 		if(DoSyncRoomInfo(cmd, seq, &instream)==-1)
 		{
 			AC_ERROR("HallDataDecoder::OnPackage:DoSyncRoomInfo error");
			return 0; 
		}
		break;
	case HALL_CMD_RIGHT_CHANGE_L2R:
		if(DoRightConfChange(&instream)==-1)
		{
 			AC_ERROR("HallDataDecoder::OnPackage:DoRightConfChange error");
			return 0; 
		}
		break;
	case HALL_CMD_GIFTINFO_CHANGE_L2R:
		if(DoGiftInfoChange(&instream)==-1)
		{
 			AC_ERROR("HallDataDecoder::OnPackage:DoGiftInfoChange error");
			return 0; 
		}
		break;
	case HALL_CMD_LUCKCONF_CHANGE_L2R:
		if(DoLuckConfChange(&instream)==-1)
		{
 			AC_ERROR("HallDataDecoder::OnPackage:DoLuckConfChange error");
			return 0; 
		}
		break;		
	case HALL_CMD_MOVE_ROOM_L2R:
		if(DoMoveRoom(cmd, seq,&instream)==-1)
		{
 			AC_ERROR("HallDataDecoder::OnPackage:DoMoveRoom error");
			return 0;
		}
		break;
	case HALL_CMD_UPDATE_AVSERVER_L2R:
		if(DoUpdateAVServer(cmd, seq, &instream)==-1)
		{
		 	AC_ERROR("HallDataDecoder::OnPackage:DoUpdateAVServer error");
			return 0;
		}
		break;
	case HALL_CMD_VUSER_LOGIN_L2R:
		if(DoVUserLogin(cmd, seq, &instream)==-1)
		{
		 	AC_ERROR("HallDataDecoder::OnPackage:DoVUserLogin error");
			return 0;
		}
		break;
	case HALL_CMD_VUSER_LOGOUT_L2R:
		if(DoVUserLogout(cmd, seq, &instream)==-1)
		{
		 	AC_ERROR("HallDataDecoder::OnPackage:DoVUserLogout error");
			return 0;
		}
		break;
	case HALL_CMD_UPDATE_GM_L2R:
		if(DoUpDateGM(cmd, seq)==-1)
		{
		 	AC_ERROR("HallDataDecoder::OnPackage:DoUpDateGM error");
			return 0;
		}
		break;
	case HALL_CMD_REMOVE_SEEL_L2R:
		if(DoRemoveSeal(cmd, seq, &instream)==-1)
		{
			AC_ERROR("HallDataDecoder::OnPackage:DoRemoveSeal error");
			return 0;
		}

		break;
	case HALL_CMD_LOGIN:
		break;
	case HALL_CMD_ROOMBLACKMEMCHG_L2R:
		if(DoUpdateRoomBlacklist(cmd, seq, &instream)==-1)
		{
			AC_ERROR("HallDataDecoder::OnPackage:DoUpdateRoomBlacklist error");
			return 0;

		}
		break;
	case HALL_CMD_UPDATE_ROOMCHAT_BLACKLIST_L2R:
		if(DoUpdateChatBlacklist(cmd,seq, &instream)==-1)
		{
			AC_ERROR("HallDataDecoder::OnPackage:DoUpdateChatBlacklist error");
			return 0;
		}
		break;
	case HALL_CMD_UPDATE_ROOMGIFT_REFRESH_TIMES_L2R:
		if (DoUpdateGiftConfig(cmd, seq)==-1)
		{
			AC_ERROR("HallDataDecoder::OnPackage:DoUpdateGiftConfig error");
			return 0;
		}
		break;
	default:
		{            
            AC_ERROR("HallDataDecoder::OnPackage:cmd error,cmd=%d",cmd);
            return 0;
		}
	break;
    }

	return 0;
}

int HallDataDecoder::DoCreateRoom(short cmd, int seq, BinaryReadStream *ppinstream)
{
	AC_DEBUG("HallDataDecoder::DoCreateRoom:cmd = %d seq = %d", cmd ,seq);	
	
	char info[1024];
	size_t infolen;
	if(!ppinstream->Read(info,sizeof(info),infolen))
	{
		AC_ERROR("HallDataDecoder::DoCreateRoom:read info error");
		return -1;
	}

	BinaryReadStream pinstream(info,infolen);

	short rownum;
	if(!pinstream.Read(rownum))
	{
		AC_ERROR("HallDataDecoder::DoCreateRoom:read row num error");
		return -1;
	}

	short colnum;
	if(!pinstream.Read(colnum))
	{
		AC_ERROR("HallDataDecoder::DoCreateRoom:read col num error");
		return -1;
	}
	
	AC_DEBUG("HallDataDecoder::DoCreateRoom:rownum = %d colnum = %d", rownum ,colnum);	

	char infobuf[1024] = {0};
	size_t curlen;

	char ip_telcom[32]={0};
	char ip_netcom[32]={0};
	int roomid = 0;
	ROOM_INFO* proominfo = new ROOM_INFO();
	for(int j = 0;j < colnum;j++)
	{
		if(!pinstream.Read(infobuf,sizeof(infobuf),curlen))
		{
			AC_ERROR("HallDataDecoder::DoCreateRoom:read ptr error,colnum=%d",j);
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
				AC_ERROR("HallDataDecoder::DoCreateRoom:hallid error,[%d],ServerHallID[%d]",hallid,g_pNetProxy->m_pHallsvrCfg->m_id);
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
			int ownidx=atoi(infobuf);
			proominfo->ownidx= ownidx;
			//室主加入管理员列表
			ROOM_MANAGER manager;
			manager.m_idx=ownidx;
			manager.m_identity=USER_ID_OWNER;
			proominfo->managerlist.insert(make_pair(ownidx,manager));
			//室主加入会员列表
			proominfo->userlistVIP.insert(make_pair(ownidx,ownidx));
			
 		}
		break;
		case 7:{}break;	//房间创建时间
		case 8:
		{
             	proominfo->isMicUpdown = atoi(infobuf);	//房间排序标志
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
				AC_ERROR("HallDataDecoder::DoCreateRoom:port error [%d],[%d]",port,g_pNetProxy->m_port);
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
				AC_ERROR("HallDataDecoder::DoCreateRoom:ip error ip_netcom=[%s],ip_telcom=[%s],server_IP:%s",ip_netcom,ip_telcom,g_pNetProxy->m_sIP);
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
				 	AC_ERROR("HallDataDecoder::DoCreateRoom:AVServerIP_telcom len=%d",curlen);
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
			 	AC_ERROR("HallDataDecoder::DoCreateRoom:AVServerIP_netcom len=%d",curlen);
				return -1;
			}
		}
		break;
		case 27://音视频服务器端口--网通
		{
				proominfo->AVServerPort_netcom=atoi(infobuf);
		}
		break;
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
			 	AC_ERROR("HallDataDecoder::DoCreateRoom:CreateRoomTimeout error");
				return -1;
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
			 	AC_ERROR("HallDataDecoder::DoCreateRoom:CreateFlowerResult error");
				return -1;
			}
					
			
			g_pNetProxy->m_roomlistinfo.roommap.insert(make_pair(roomid, proominfo));
			
			AC_DEBUG("HallDataDecoder::DoCreateRoom:proominfo->roomid = %d proominfo->roomname = %s proominfo->passwd = %s proominfo->type = %d proominfo->state = %d proominfo->ownidx = %d proominfo->maxhuman = %d proominfo->welcomeword = %s proominfo->content = %s",
			proominfo->roomid,proominfo->roomname,proominfo->passwd,proominfo->type, proominfo->state, proominfo->ownidx,
			proominfo->maxhuman,proominfo->welcomeword,proominfo->content);
		}
	}
	
	/*
	ROOM_INFO proominfo = new ROOM_INFO();
	int roomid=0;			//房间ID
	if(!ppinstream->Read(roomid))
	{
		AC_ERROR("HallDataDecoder::DoCreateRoom :read roomid error");
		return -1;
	}
	proominfo->roomid=roomid;

	int hallid = 0;				//大厅ID
	if(!ppinstream->Read(hallid))
	{
		AC_ERROR("HallDataDecoder::DoCreateRoom :read hallid error");
		return -1;
	}
	if(hallid!=g_pNetProxy->m_roomlistinfo.hallid)
	{
		AC_ERROR("HallDataDecoder::DoCreateRoom:hallid error,[%d],[%d]",hallid,g_pNetProxy->m_pHallsvrCfg->m_id);
		return -1;
	}

	size_t len;
	//房间名
	if(!ppinstream->Read(proominfo->roomname,sizeof(proominfo->roomname),len))
	{
		AC_ERROR("HallDataDecoder::DoCreateRoom :read roomname error");
		return -1;
	}
	//房间密码
	if(!ppinstream->Read(proominfo->passwd,sizeof(proominfo->passwd),len))
	{
		AC_ERROR("HallDataDecoder::DoCreateRoom :read passwd error");
		return -1;
	}
	//房间类型
	int type=0;
	if(!ppinstream->Read(type))
	{
		AC_ERROR("HallDataDecoder::DoCreateRoom :read type error");
		return -1;
	}
	proominfo->type=type;
	//房间状态
	int state=0;
	if(!ppinstream->Read(state))
	{
		AC_ERROR("HallDataDecoder::DoCreateRoom :read state error");
		return -1;
	}
	proominfo->state=state;
	//室主IDX
	if(!ppinstream->Read(proominfo->ownidx))
	{
		AC_ERROR("HallDataDecoder::DoCreateRoom :read ownidx error");
		return -1;
	}
	//房间创建时间
	char m_createdate[30];
	if(!ppinstream->Read(m_createdate,sizeof(m_createdate),len))
	{
		AC_ERROR("HallDataDecoder::DoCreateRoom :read m_createdate error");
		return -1;
	}
	//自由上麦flag	
	char m_sortflag[20];
	if(!ppinstream->Read(m_sortflag,sizeof(m_sortflag),len))
	{
		AC_ERROR("HallDataDecoder::DoCreateRoom :read m_sortflag error");
		return -1;
	}
	
	//房间最大人数
	if(!ppinstream->Read(proominfo->maxhuman)
	{
		AC_ERROR("HallDataDecoder::DoCreateRoom :read maxhuman error");
		return -1;
	}
	//房间冻结时间	
	char m_deadline[30];
	if(!ppinstream->Read(m_deadline,sizeof(m_deadline),len))
	{
		AC_ERROR("HallDataDecoder::DoCreateRoom :read m_deadline error");
		return -1;
	}
	//自动审核会员申请flag
	char m_ispassaudit[2];
	if(!ppinstream->Read(m_ispassaudit,sizeof(m_ispassaudit),len))
	{
		AC_ERROR("HallDataDecoder::DoCreateRoom :read m_ispassaudit error");
		return -1;
	}
		
	//房间欢迎词
	if(!ppinstream->Read(proominfo->welcomeword,sizeof(proominfo->welcomeword),len))
	{
		AC_ERROR("HallDataDecoder::DoCreateRoom :read welcomeword error");
		return -1;
	}
	//房间所在服务器的电信IP	
	char m_ip_telcom[21];
	if(!ppinstream->Read(m_ip_telcom,sizeof(m_ip_telcom),len))
	{
		AC_ERROR("HallDataDecoder::DoCreateRoom :read m_ip_telcom error");
		return -1;
	}
	m_ip_telnet[len]=0;
	
	//房间连接端口
	short m_port;
	if(!ppinstream->Read(m_port))
	{
		AC_ERROR("HallDataDecoder::DoCreateRoom :read m_port error");
		return -1;
	}
	//房间公告
	char m_content[256];
	if(!ppinstream->Read(proominfo->content,sizeof(proominfo->content),len))
	{
		AC_ERROR("HallDataDecoder::DoCreateRoom :read content error");
		return -1;
	}
	//房间更新时间
	char m_updatedate[30];
	if(!ppinstream->Read(m_updatedate,sizeof(m_updatedate),len))
	{
		AC_ERROR("HallDataDecoder::DoCreateRoom :read m_updatedate error");
		return -1;
	}
	//房间LOGO 地址	
	if(!ppinstream->Read(proominfo->roomlogo,sizeof(proominfo->roomlogo),len))
	{
		AC_ERROR("HallDataDecoder::DoCreateRoom :read roomlogo error");
		return -1;
	}
		
	short m_mannum;       //男人数
	if(!ppinstream->Read(m_mannum))
	{
		AC_ERROR("HallDataDecoder::DoCreateRoom :read m_mannum error");
		return -1;
	}
	short m_womannum;   //女人数
	if(!ppinstream->Read(m_womannum))
	{
		AC_ERROR("HallDataDecoder::DoCreateRoom :read m_womannum error");
		return -1;
	}
	
	if(!ppinstream->Read(proominfo->isPublicChat))
	{
		AC_ERROR("HallDataDecoder::DoCreateRoom :read isPublicChat error");
		return -1;
	}
		
	if(!ppinstream->Read(proominfo->isUserInOut))
	{
		AC_ERROR("HallDataDecoder::DoCreateRoom :read isUserInOut error");
		return -1;
	}

	if(!ppinstream->Read(proominfo->isUserOnly))
	{
		AC_ERROR("HallDataDecoder::DoCreateRoom :read isUserOnly error");
		return -1;
	}
	
	if(!ppinstream->Read(proominfo->isClose))
	{
		AC_ERROR("HallDataDecoder::DoCreateRoom :read isClose error");
		return -1;
	}
	
	if(!ppinstream->Read(proominfo->allowhuman))
	{
		AC_ERROR("HallDataDecoder::DoCreateRoom :read allowhuman error");
		return -1;
	}
	
	char m_ip_netcom[21];
	if(!ppinstream->Read(m_ip_netcom,sizeof(m_ip_netcom),len))
	{
		AC_ERROR("HallDataDecoder::DoCreateRoom :read m_ip_netcom error");
		return -1;
	}
	m_ip_netcom[len]=0;

	if(strcmp(g_pNetProxy->m_sIP,m_ip_telcom)&&strcmp(g_pNetProxy->m_sIP,m_ip_netcom))
	{
		AC_ERROR("HallDataDecoder::DoCreateRoom :ip error,room_telcom_ip=%s,room_netcom_ip=%s,server_IP=%s",m_ip_telcom,m_ip_netcom,g_pNetProxy->m_sIP);
		return -1;
	}

	if(m_port!=(short)g_pNetProxy->m_port)
	{
		AC_ERROR("HallDataDecoder::DoCreateRoom :port error,room_port=%d,server_port_%d",m_port,g_pNetProxy->m_port);
		return -1;
	}
	
		
	g_pNetProxy->m_roomlistinfo.roommap.insert(make_pair(roomid, roominfo));
	AC_DEBUG("HallDataDecoder::DoCreateRoom:proominfo->roomid = %d proominfo->roomname = %s proominfo->passwd = %s proominfo->type = %d proominfo->state = %d proominfo->ownidx = %d proominfo->maxhuman = %d proominfo->welcomeword = %s proominfo->content = %s",
			proominfo->roomid,proominfo->roomname, proominfo->passwd, proominfo->type, proominfo->state, proominfo->ownidx,
			proominfo->maxhuman, proominfo->welcomeword, proominfo->content);
	*/	
	return 0;
}

int HallDataDecoder::DoDeleteRoom( BinaryReadStream *pinstream)/*ClientSocketBase *pClient, short cmd, int seq,*/
{
	
	int iroomID;
	if(!pinstream->Read(iroomID))    //对象的idx
	{
		AC_ERROR("HallDataDecoder::DoDeleteRoom:read idx error");
		return -1;
	}

	AC_DEBUG("HallDataDecoder::DoDeleteRoom:iroomID=%d",iroomID);	
	
	ROOM_INFO* pRoominfo=g_pNetProxy->GetRoomInfo(iroomID);
	if(pRoominfo == NULL)
	{
		AC_ERROR("HallDataDecoder::DoDeleteRoom:roominfo error");
		return 0;
	}

	//通知房间内所有人房间删除
	BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
	char type = 65;
	outstream->Write(type);
	outstream->Write((short)ROOM_CMD_SB_DELETEROOM);
	outstream->Write(0);
	outstream->Write((int)pRoominfo->roomid);	//ROOMID
	outstream->Flush();

	g_pNetProxy->BroadcastSBCmd(pRoominfo, *outstream,1);
	/*
	//事先消除印章和排麦提高用户离开房间速度
	//印章信息在用户离开房间时已清除
	map<int,GiftTimeout*>::iterator itseal=pRoominfo->seallist.begin();
	for(;itseal!=pRoominfo->seallist.end();itseal++)
	{
		GiftTimeout* pGdata=(*itseal).second;
		g_pNetProxy->DestroyGifttimeout(pGdata);
	}
	pRoominfo->seallist.clear();
	*/
	//排麦信息在用户离开房间时已清除
	vector<MIC_INFO*>::iterator itMic=pRoominfo->miclist.begin();
	for(;itMic!=pRoominfo->miclist.end();itMic++)
	{
		MIC_INFO* pmicinfo = (MIC_INFO*)(*itMic);
		g_pNetProxy->DestroyMicInfo(pmicinfo);
	}
	pRoominfo->miclist.clear();

	//在线用户踢出房间
	AC_DEBUG("HallDataDecoder::DoDeleteRoom:iroomID=%d userlist.size=%d ",iroomID,pRoominfo->userlist.size());	
	map<int,RoomClient*> kicklist;
	map<int,RoomClient*>::iterator itc=pRoominfo->userlist.begin();
	for(;itc!=pRoominfo->userlist.end();itc++)
	{
		RoomClient* pClient=(RoomClient*) itc->second;
		if(pClient && pClient->m_btoken)
                   // pClient->LeaveRoom();
                   kicklist.insert(make_pair(pClient->m_idx,pClient));
	}

	map<int,RoomClient*>::iterator itk=kicklist.begin();
	for(;itk!=kicklist.end();itk++)
	{
		RoomClient* pRoomClient=itk->second;
		if(pRoomClient!=NULL)
			pRoomClient->LeaveRoom();
	}

	map<int,roomtimeout*>::iterator itforbiden=pRoominfo->forbidenlist.begin();
	for(;itforbiden!=pRoominfo->forbidenlist.end();itforbiden++)
	{
		roomtimeout* pRdata=(*itforbiden).second;
		g_pNetProxy->DestroyRoomtimeout(pRdata);
	}
	pRoominfo->forbidenlist.clear();
	//add by jinguanfu 2010/11/27
	//清空虚拟用户
	map<int, RoomClient*>::iterator itvu = pRoominfo->vuserlist.begin();
	for(;itvu != pRoominfo->vuserlist.end();itvu++)
	{
		RoomClient*	pClient=(RoomClient*)itvu->second;
		if(pClient==NULL)
		{

			AC_ERROR("HallDataDecoder::DoDeleteRoom:pClient==NULL ");
			return -1;
		}
		pClient->VLeaveRoom();
		//用户列表清除
		pRoominfo->vuserlist.erase(itvu);
	}
		
	g_pNetProxy->DestroyRoomtimeout(pRoominfo->m_pOnMicTimeout);

	g_pNetProxy->DestroyFlowerResult(pRoominfo->FlowerTimer);

	AC_DEBUG("HallDataDecoder::DoDeleteRoom: roomid=%d,delete success",iroomID);


	return g_pNetProxy->DeleteRoomByID(pRoominfo);	
	
}

int HallDataDecoder::DoBlockRoom(short cmd, int seq, BinaryReadStream *pinstream)
{

	AC_DEBUG("HallDataDecoder::DoBlockRoom: cmd=%d,seq=%d,pinstream=%x",cmd,seq,pinstream);
	int iroomID;
	if(!pinstream->Read(iroomID))    //对象的idx
	{
		AC_ERROR("ClientDataDecoder::DoBlockRoom:read iroomID error");
		return -1;
	}
	
	ROOM_INFO* pRoominfo=g_pNetProxy->GetRoomInfo(iroomID);
	if(pRoominfo == NULL)
	{
		AC_ERROR("HallDataDecoder::DoBlockRoom:roominfo error");
		return 0;
	}

	char state=0;
	if(!pinstream->Read(state))  
	{
		AC_ERROR("HallDataDecoder::DoBlockRoom:read state error");
		return 0;
	}

	AC_DEBUG("HallDataDecoder::DoBlockRoom:iroomID=%d state=%d",iroomID,state);
	
	pRoominfo->state =state;  //房间冻结标志
	
	if(state==1)	//冻结
	{
		//通知房间内所有人房间冻结
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
		//BinaryWriteStream outstream(outbuf,sizeof(outbuf));
		char type = 65;
		outstream->Write(type);
		outstream->Write((short)ROOM_CMD_SB_LOCKROOM);
		outstream->Write(0);
		outstream->Write((int)pRoominfo->roomid);	//ROOMID
		outstream->Flush();

		g_pNetProxy->BroadcastSBCmd(pRoominfo, *outstream,1);

		map<int,RoomClient*> kicklist;
		map<int,RoomClient*>::iterator itc=pRoominfo->userlist.begin();
		for(;itc!=pRoominfo->userlist.end();itc++)
		{
			RoomClient* pClient=(RoomClient*) itc->second;
			if(pClient && pClient->m_btoken)
	              //      pClient->LeaveRoom();
	              	 kicklist.insert(make_pair(pClient->m_idx,pClient));
		}

		map<int,RoomClient*>::iterator itk=kicklist.begin();
		for(;itk!=kicklist.end();itk++)
		{
			RoomClient* pRoomClient=itk->second;
			if(pRoomClient!=NULL)
				pRoomClient->LeaveRoom();
		}
		//清空虚拟用户
		map<int, RoomClient*>::iterator itvu = pRoominfo->vuserlist.begin();
		for(;itvu != pRoominfo->vuserlist.end();itvu++)
		{

			RoomClient*	pClient=(RoomClient*)itvu->second;
			if(pClient==NULL)
			{

				AC_ERROR("HallDataDecoder::DoDeleteRoom:pClient==NULL ");
				return -1;
			}
			pClient->VLeaveRoom();

		}
		
		pRoominfo->vuserlist.clear();


	
	}
	
	return 0;	
}

//add by jinguanfu 2010/4/12 大厅通知用户重复登录消息
int HallDataDecoder::DoKickUser(BinaryReadStream *pinstream)
{
	int roomid;
	if(!pinstream->Read(roomid))    //roomid
	{
		AC_ERROR("HallDataDecoder::DoKickUser:read roomid error");
		return -1;
	}

	int idx;
	if(!pinstream->Read(idx))    //idx
	{
		AC_ERROR("HallDataDecoder::DoKickUser:read idx error");
		return -1;
	}

	AC_DEBUG("DoKickUserIN idx = %d roomid = %d", idx, roomid);

	//用户是否在房间里在线，在线则关闭连接
	ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(roomid);
	if(proominfo == NULL)
	{
		AC_ERROR("proominfo == NULL");
		return 0;
	}

	map<int,RoomClient*>::iterator itu = proominfo->userlist.find(idx);
	if (itu != proominfo->userlist.end())
	{
		AC_DEBUG("DoKickUser:idx = %d,roomid = %d, client close",idx, proominfo->roomid);
		RoomClient* pClient = (*itu).second;
		pClient->Close();
	}

	//退出虚拟在线用户
	map<int, RoomClient*>::iterator itvu = proominfo->vuserlist.find(idx);
	if(itvu != proominfo->vuserlist.end())
	{
		RoomClient*	pClient = (RoomClient*)itvu->second;
		if(pClient==NULL)
		{
			AC_ERROR("HallDataDecoder::DoVUserLogout:pClient==NULL ");
			return -1;
		}
		pClient->VLeaveRoom();
		proominfo->vuserlist.erase(itvu);
	}

	return 0 ;
	
}

int HallDataDecoder::DoUpdateUserinfo(short cmd,int seq,BinaryReadStream *pinstream)
{


/*	
	int idx;
	if(!pinstream->Read(idx))    //idx
	{
		AC_ERROR("HallDataDecoder::DoUpdateUserinfo:read idx error");
		return -1;
	}


	int roomid;
	if(!pinstream->Read(roomid))    //roomid
	{
		AC_ERROR("HallDataDecoder::DoUpdateUserinfo:read roomid error");
		return -1;
	}
*/
	char sex=0;		//性别0:女1:男
	if(!pinstream->Read(sex)) 
	{
		AC_ERROR("HallDataDecoder::DoUpdateUserinfo:read sex error");
		return -1;
	}
		
	char level=0;		//经验等级
	if(!pinstream->Read(level)) 
	{
		AC_ERROR("HallDataDecoder::DoUpdateUserinfo:read level error");
		return -1;
	}

	short viplevel;		//VIP、皇冠等级
	if(!pinstream->Read(viplevel)) 
	{
		AC_ERROR("HallDataDecoder::DoUpdateUserinfo:read viplevel error");
		return -1;
	}

	char isVip;			//皇冠是否有效
	if(!pinstream->Read(isVip)) 
	{
		AC_ERROR("HallDataDecoder::DoUpdateUserinfo:read isVip error");
		return -1;
	}

	char vipdate[32]={0};		//VIP有效日期
	size_t len=0;
	if(!pinstream->Read(vipdate,sizeof(vipdate)-1,len)) 
	{
		AC_ERROR("HallDataDecoder::DoUpdateUserinfo:read vipdate error");
		return -1;
	}
	vipdate[len] = 0;// add by wangpf 2010/07/30
	
	int gold=0;
	if(!pinstream->Read(gold))    //金币
	{
		AC_ERROR("HallDataDecoder::DoUpdateUserinfo:read gold error");
		return -1;
	}

	int silver=0;
	if(!pinstream->Read(silver))    //银币
	{
		AC_ERROR("HallDataDecoder::DoUpdateUserinfo:read silver error");
		return -1;
	}

	//add by jinguanfu 2011/8/11
	short status=0;			//防沉迷状态
	if(!pinstream->Read(status))
	{
		AC_ERROR("HallDataDecoder::DoUpdateUserinfo:read status error");
		return -1;
	}

	int onlinetime=0;		//当天在线时长(非实时，用户最近一次登陆时长)
	if(!pinstream->Read(onlinetime)) 
	{
		AC_ERROR("HallDataDecoder::DoUpdateUserinfo:read status error");
		return -1;
	}

	int logintime = 0;		//最后一次登陆时间
	if(!pinstream->Read(logintime))  
	{
		AC_ERROR("HallDataDecoder::DoUpdateUserinfo:read status error");
		return -1;
	}
	
	LobbyTimeout* pLobbyTimeout = g_pNetProxy->GetLobbyTMMap(seq);
	if(pLobbyTimeout==NULL)
	{
		AC_ERROR("HallDataDecoder::DoUpdateUserinfo:pLobbyTimeout not find ,seq=%d",seq);
		return 0;
	}
	
	if(pLobbyTimeout->cmd!=ROOM_CMD_TOKEN)
	{
		AC_ERROR("HallDataDecoder::DoUpdateUserinfo:cmd error ,outseq=%d cmd=%d",seq,cmd);
		return 0;
	}
	

	RoomClient* pRoomClient=g_pNetProxy->GetClient(pLobbyTimeout->clientID);
	if(pRoomClient==NULL)
	{
		AC_ERROR("ClientDataDecoder::DoUpdateUserinfo:pRoomClient is NULL");
		return 0;
	}
	
	pRoomClient->m_btoken = 1;
	pRoomClient->m_sex=sex;
	pRoomClient->gold=gold;
	pRoomClient->silver=silver;
	pRoomClient->m_level=level;
	pRoomClient->m_vipflag=isVip;
	pRoomClient->m_viplevel=viplevel;
	//strcpy(pRoomClient->m_vipdate,vipdate);
	strncpy(pRoomClient->m_vipdate, vipdate, sizeof(pRoomClient->m_vipdate)); // add by wangpf 2010/07/30
	//add by jinguanfu 2011/8/11
	pRoomClient->m_status = status;
	pRoomClient->m_onlinetime = onlinetime;
	pRoomClient->m_logintime = logintime;

	AC_DEBUG("HallDataDecoder::DoUpdateUserinfo fd = %d, idx=%d,gold=%d ,silver=%d,level=%d,viplevel=%d,isVip=%d,vipdate=%s",
		pRoomClient->GetFD(), pRoomClient->m_idx,gold,silver,level,viplevel,isVip,vipdate);

	BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
	char type = 65;
	outstream->Write(type);
	outstream->Write(pLobbyTimeout->cmd);
	outstream->Write(pLobbyTimeout->seq);
	outstream->Write((int)TOKENLOGIN_SUCCESS); 	//add by jinguanfu 2011/9/27
	outstream->Write(pRoomClient->m_sessionkey, strlen(pRoomClient->m_sessionkey));	//sessionkey
	outstream->Flush();

	if(g_pNetProxy->SendToSrv(pRoomClient, *outstream,0,0) == -1)
	{
		AC_ERROR("ClientDataDecoder::DoUpdateUserinfo:g_pNetProxy->SendToSrv error");
		return 0;
	}
	pLobbyTimeout->UnRegisterTimer();
	g_pNetProxy->ClearLobbyTMMap(seq);
	g_pNetProxy->DestroyLobbyTimeout(pLobbyTimeout);
		
	return 0;
}

int HallDataDecoder::DoChangeAvatarNotice(BinaryReadStream *pinstream)
{
	AC_DEBUG("HallDataDecoder::DoChangeAvatarNotice : IN");

	int idx;
	if(!pinstream->Read(idx))    //idx
	{
		AC_ERROR("HallDataDecoder::DoChangeAvatarNotice:read idx error");
		return -1;
	}

	map<int, ROOM_INFO*>::iterator itroom = g_pNetProxy->m_roomlistinfo.roommap.begin();
	for(;itroom!= g_pNetProxy->m_roomlistinfo.roommap.end();itroom++)
	{
		ROOM_INFO* proominfo=itroom->second;
		if(proominfo)
		{
			map<int,RoomClient*>::iterator itU= proominfo->userlist.find(idx);

			if(itU != proominfo->userlist.end())
			{
				int seq = 0;
				BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
				char type = 65;
				outstream->Write(type);
				outstream->Write((short)ROOM_CMD_SB_CHANGEAVATAR);
				outstream->Write(seq);
				outstream->Write((int)idx);
				outstream->Flush();
				if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream) == -1)
				{
					AC_ERROR("ClientDataDecoder::DoChangeAvatarNotice:g_pNetProxy->BroadcastSBCmd error");
					return -1;
				}
				return 0;
			}
		}
		else
		{
			AC_ERROR("proominfo == NULL");
		}
	}

	return 0;
}

int HallDataDecoder::DoLeaveLobby(BinaryReadStream *pinstream)
{
	AC_DEBUG("HallDataDecoder::DoLeaveLobby: IN ");

	int idx;
	if(!pinstream->Read(idx))    //idx
	{
		AC_ERROR("HallDataDecoder::DoLeaveLobby:read idx error");
		return -1;
	}

	int roomid;
	if(!pinstream->Read(roomid))    //roomid
	{
		AC_ERROR("HallDataDecoder::DoLeaveLobby:read roomid error");
		return -1;
	}

	if(roomid==0||idx==0)
	{
		return 0;
	}

	ROOM_INFO* pRoominfo=g_pNetProxy->GetRoomInfo(roomid);
	if(pRoominfo==NULL)
	{
		AC_ERROR("HallDataDecoder::DoLeaveLobby: roomid=%d not exist",roomid);
		return 0;
	}

	map<int,RoomClient*>::iterator itU= pRoominfo->userlist.find(idx);
	if(itU!=pRoominfo->userlist.end())
	{
		RoomClient* pClient=itU->second;
		pClient->Close();
	}
	else
	{
		AC_ERROR("HallDataDecoder::DoLeaveLobby: idx=%d is not in roomid=%d ",idx,roomid);
	}
	

	return 0;
}

int HallDataDecoder::DoUpdateLevel(BinaryReadStream *pinstream)
{

	int roomid;
	if(!pinstream->Read(roomid))    //roomid
	{
		AC_ERROR("HallDataDecoder::DoUpdateLevel:read roomid error");
		return -1;
	}
	
	int idx;
	if(!pinstream->Read(idx))    //idx
	{
		AC_ERROR("HallDataDecoder::DoUpdateLevel:read idx error");
		return -1;
	}


	char level;
	if(!pinstream->Read(level))    //经验等级
	{
		AC_ERROR("HallDataDecoder::DoUpdateLevel:read level error");
		return -1;
	}
	AC_DEBUG("HallDataDecoder::DoUpdateLevel roomid=%d,idx=%d level=%d",roomid,idx,level);

	ROOM_INFO* proominfo=g_pNetProxy->GetRoomInfo(roomid);
	if(proominfo==NULL)
	{
		AC_DEBUG("HallDataDecoder::DoUpdateLevel:roomid error roomid=%d",roomid);
		return 0;	
	}

	map<int,RoomClient*>::iterator itclient=proominfo->userlist.find(idx);
	if(itclient==proominfo->userlist.end())
	{
		AC_ERROR("HallDataDecoder::DoUpdateLevel:idx=%d is not in room=%d",idx,roomid);
		return -1;
	}

	AC_DEBUG("HallDataDecoder::DoUpdateLevel: idx=%d  level=%d",idx,level);
	RoomClient* pRoomClient=itclient->second;
	pRoomClient->m_level=level;

	BinaryWriteStream* roomstream=StreamFactory::InstanceWriteStream();
	char type=65;
	roomstream->Write(type);
	roomstream->Write((short)ROOM_CMD_SB_LEVEL);
	roomstream->Write(0);
	roomstream->Write((int)pRoomClient->m_idx);		//用户idx
	roomstream->Write(pRoomClient->m_level);	//经验等级
	roomstream->Flush();

	if(g_pNetProxy->BroadcastSBCmd(proominfo, *roomstream)==-1)
	{
	        AC_ERROR("ClientDataDecoder::DoUpdateLevel:g_pNetProxy->BroadcastSBCmd error");
	}
	
	return 0;
}

int HallDataDecoder::DoUpdateVip(BinaryReadStream *pinstream)
{
	int roomid;
	if(!pinstream->Read(roomid))    //roomid
	{
		AC_ERROR("HallDataDecoder::DoUpdateVip:read roomid error");
		return -1;
	}
	
	int idx;
	if(!pinstream->Read(idx))    //idx
	{
		AC_ERROR("HallDataDecoder::DoUpdateVip:read idx error");
		return -1;
	}


	int viplevel;
	if(!pinstream->Read(viplevel))    //VIP、皇冠等级
	{
		AC_ERROR("HallDataDecoder::DoUpdateVip:read viplevel error");
		return -1;
	}

	char isVIP;
	if(!pinstream->Read(isVIP))    //皇冠是否有效
	{
		AC_ERROR("HallDataDecoder::DoUpdateVip:read isVIP error");
		return -1;
	}
	
	AC_DEBUG("HallDataDecoder::DoUpdateVip roomid=%d,idx=%d viplevel=%d isVIP=%d",roomid,idx,viplevel,isVIP);
	

	ROOM_INFO* proominfo=g_pNetProxy->GetRoomInfo(roomid);
	if(proominfo==NULL)
	{
		AC_DEBUG("HallDataDecoder::DoUpdateVip:roomid error roomid=%d",roomid);
		return 0;	
	}


	map<int,RoomClient*>::iterator itclient=proominfo->userlist.find(idx);
	if(itclient==proominfo->userlist.end())
	{
		AC_ERROR("HallDataDecoder::DoUpdateVip:idx=%d is not in room=%d",idx,roomid);
		return -1;
	}
	
	RoomClient* pRoomClient=itclient->second;
	pRoomClient->m_viplevel=viplevel;
	pRoomClient->m_vipflag = isVIP;

	BinaryWriteStream* roomstream=StreamFactory::InstanceWriteStream();
	char type=65;
	roomstream->Write(type);
	roomstream->Write((short)ROOM_CMD_SB_VIP);
	roomstream->Write(0);
	roomstream->Write((int)pRoomClient->m_idx);		//用户idx
	roomstream->Write(pRoomClient->m_viplevel);		//VIP、皇冠等级
	roomstream->Write(pRoomClient->m_vipflag);		//皇冠是否有效
	roomstream->Flush();

	if(g_pNetProxy->BroadcastSBCmd(proominfo, *roomstream)==-1)
	{
	        AC_ERROR("ClientDataDecoder::DoUpdateVip:g_pNetProxy->BroadcastSBCmd error");
	}

	
	return 0;
}
/*
int HallDataDecoder::DoUpdateItem(BinaryReadStream *pinstream)
{
	AC_DEBUG("HallDataDecoder::DoUpdateItem: IN");

	return 0;
}
*/
int HallDataDecoder::DoUpdateCoin(BinaryReadStream *pinstream)
{

	int idx;
	if(!pinstream->Read(idx))    //idx
	{
		AC_ERROR("HallDataDecoder::DoUpdateCoin:read idx error");
		return -1;
	}

	int roomid;
	if(!pinstream->Read(roomid))    //roomid
	{
		AC_ERROR("HallDataDecoder::DoUpdateCoin:read roomid error");
		return -1;
	}

	int gold;
	if(!pinstream->Read(gold))    //gold
	{
		AC_ERROR("HallDataDecoder::DoUpdateCoin:read gold error");
		return -1;
	}

	int silver;
	if(!pinstream->Read(silver))    //silver
	{
		AC_ERROR("HallDataDecoder::DoUpdateCoin:read silver error");
		return -1;
	}

	ROOM_INFO* proominfo=g_pNetProxy->GetRoomInfo(roomid);
	if(proominfo==NULL)
	{
		AC_ERROR("HallDataDecoder::DoUpdateCoin:roomid error roomid=%d",roomid);
		return 0;	
	}


	map<int,RoomClient*>::iterator itclient=proominfo->userlist.find(idx);
	if(itclient==proominfo->userlist.end())
	{
		AC_ERROR("HallDataDecoder::DoUpdateCoin:idx=%d is not in room=%d",idx,roomid);
		return 0;
	}
	
	AC_DEBUG("HallDataDecoder::DoUpdateCoin : idx=%d,roomid=%d,gold=%d,silver=%d",idx,roomid,gold,silver);


	RoomClient* pRoomClient=itclient->second;
	pRoomClient->gold=gold;
	pRoomClient->silver=silver;

	
	return 0;
	
}

int HallDataDecoder::DoUpdateRoomInfo(short cmd, int seq,BinaryReadStream *pinstream)
{
	AC_DEBUG("HallDataDecoder::DoUpdateRoomInfo : cmd=%d,seq=%d",cmd,seq);

	int flag=0;	//室主更改标识

	char info[1024];
	size_t infolen;
	if(!pinstream->Read(info,sizeof(info),infolen))
	{
		AC_ERROR("NetProxy::GetRoomDetailPrePage():read info error");	
		return -1;
	}

	BinaryReadStream instream(info,infolen);
		
	short rownum;
	if(!instream.Read(rownum))
	{
		AC_ERROR(" HallDataDecoder::DoUpdateRoomInfo:read row num error");
		return -1;
	}

	short colnum;
	if(!instream.Read(colnum))
	{
		AC_ERROR(" HallDataDecoder::DoUpdateRoomInfo:read col num error");
		return -1;
	}

	char infobuf[1024] = {0};
	size_t curlen;

	ROOM_INFO* pRoominfo;
	int roomid = 0;
	for(int i = 0;i < rownum; i++)
	{
		for(int j = 0;j < colnum;j++)
		{
			if(!instream.Read(infobuf,sizeof(infobuf),curlen))
			{
				AC_ERROR(" HallDataDecoder::DoUpdateRoomInfo:read ptr error,row=%d,col=%d",rownum,colnum);
				return -1;
			}
			infobuf[curlen] = 0;

			switch(j)
			{
				case 0:
				{
					roomid = atoi(infobuf);
					pRoominfo=g_pNetProxy->GetRoomInfo(roomid);
					if(pRoominfo==NULL)
					{
						AC_ERROR(" HallDataDecoder::DoUpdateRoomInfo:roomid=%d not exsit",roomid);
						return 0;
					}
					pRoominfo->roomid = roomid;	
				}
				break;
				case 1:
				{
					//int hallid = atoi(infobuf);
				}
				break;
				case 2:
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
				break;
				case 3:
			      {
			      		if(infobuf!=NULL&&curlen<sizeof(pRoominfo->passwd))
			      		{
						strncpy(pRoominfo->passwd, infobuf,curlen);
						pRoominfo->passwd[curlen]=0;
			            	}
					else 
						memset(pRoominfo->passwd,0,sizeof(pRoominfo->passwd));		
				}					
				break;
				case 4:
				{
					pRoominfo->type = atoi(infobuf);
				}
				break;
				case 5:
				{
					pRoominfo->state = atoi(infobuf);
				}
				break;
				case 6:
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
						map<int,int>::iterator itVJ_new = pRoominfo->vjlist.find(ownidx);
						map<int,int>::iterator itVJA_new = pRoominfo->vjlist_a.find(ownidx);
						map<int,GM_INFO>::iterator itGM_new = g_pNetProxy->m_GM.find(ownidx);
						//modify by jinguanfu 2011/9/14 <begin>
						/*
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
						*/
						//新室主为GM 时从GM列表中删除
						if(itGM_new!=g_pNetProxy->m_GM.end())
							g_pNetProxy->m_GM.erase(itGM_new);
						
						//新室主为副室主时，从副室主中删除
						if(ownidx==pRoominfo->secondownidx)
							pRoominfo->secondownidx=0;
						else if(ownidx==pRoominfo->secondownidx2)
							pRoominfo->secondownidx2=0;
	
						//新室主为房间主持人或助理主持时，从主持人列表或助理主持人列表中删除 
						if(itVJ_new!=pRoominfo->vjlist.end())
							pRoominfo->vjlist.erase(itVJ_new);
						else if(itVJA_new!=pRoominfo->vjlist_a.end())
							pRoominfo->vjlist_a.erase(itVJA_new);
						
						//新室主已在管理员列表里更新其身份
						if(itM_new!=pRoominfo->managerlist.end())
						{
							ROOM_MANAGER* pManager=(ROOM_MANAGER*)&(itM_new->second);
							pManager->m_identity=USER_ID_OWNER;
						}
						//不在管理员列表则添加到管理员列表
						else
						{
							ROOM_MANAGER Manager;
							Manager.m_idx=ownidx;
							Manager.m_identity=USER_ID_OWNER;
							pRoominfo->managerlist.insert(make_pair(ownidx,Manager));

						}
						//新室主不在会员列表则添加到会员列表
						if(itVIP_new==pRoominfo->userlistVIP.end())
							pRoominfo->userlistVIP.insert(make_pair(ownidx,ownidx));
						
						//新室主在线更改其身份
						if(itU_new!=pRoominfo->userlist.end())
						{
							RoomClient* pClient_new=itU_new->second;
							pClient_new->m_identity=USER_ID_OWNER;
						}
						//modify by jinguanfu 2011/9/14 <end>
						pRoominfo->ownidx=ownidx;
						flag = 1;
					}
				}
				break;
				case 7://房间创建时间
					{}break;
				case 8:
				{
						pRoominfo->isMicUpdown = atoi(infobuf);	//房间排序标志
				}
				break;
				case 9:
				{
						pRoominfo->maxhuman = atoi(infobuf);
				}
				break;
				case 10://房间到期时间
					{}break;
				case 11://自动审核会员申请
					{}break;
				case 12:
				{
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
				break;
				case 13://房间IP地址
					{}break;
				case 14://房间端口
					{}break;
				case 15://房间公告
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
				break;
				case 16://房间更新时间
				{}
				break;
				case 17:
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
				break;
				case 18://是否允许公聊标志位
				{
					int  chatflag =atoi(infobuf);
					if(chatflag == 1)
						pRoominfo->isPublicChat=true;
					else if(chatflag == 0)
						pRoominfo->isPublicChat=false;
				}
				break;
				case 19://用户进出信息显示标志位
				{
					int inoutflag = atoi(infobuf);
					if(inoutflag ==1)
						pRoominfo->isUserInOut = true;
					else if(inoutflag == 0)
						pRoominfo->isUserInOut = false;
				}
				break;
				case 20://房间是否公开标志位
				{
					int useronlyflag = atoi(infobuf);
					if(useronlyflag ==1)
						pRoominfo->isUserOnly = true;
					else if(useronlyflag == 0)
						pRoominfo->isUserOnly = false;
				}
				break;
				case 21://房间是否关闭标志位
				{
					int closeflag = atoi(infobuf);
					if(closeflag ==1)
						pRoominfo->isClose = true;
					else if(closeflag == 0)
						pRoominfo->isClose = false;
				}
				break;
				case 22://房间允许人数
				{
					pRoominfo->allowhuman=atoi(infobuf);
				}
				break;
										
			}

		}
	}
	AC_DEBUG("HallDataDecoder::DoUpdateRoomInfo:proominfo->roomid = %d proominfo->roomname = %s proominfo->passwd = %s proominfo->type = %d proominfo->state = %d proominfo->ownidx = %d proominfo->maxhuman = %d proominfo->allowhuman=%d proominfo->welcomeword = %s proominfo->content = %s",
						pRoominfo->roomid,pRoominfo->roomname,pRoominfo->passwd,pRoominfo->type, pRoominfo->state, pRoominfo->ownidx,
						pRoominfo->maxhuman,pRoominfo->allowhuman,pRoominfo->welcomeword,pRoominfo->content);

	AC_DEBUG("HallDataDecoder::DoUpdateRoomInfo:: room[%d] online:%d",pRoominfo->roomid,pRoominfo->userlist.size());
	//房间内通知
	char context[64] = {0};
	BinaryWriteStream outstream(context,sizeof(context));
	int outseq = 0;
	char type=65;
	outstream.Write(type);
	outstream.Write((short)ROOM_CMD_SB_ROOMINFO_UPDATE);
	outstream.Write(outseq);
	outstream.Flush();	
	if(g_pNetProxy->BroadcastSBCmd(pRoominfo,outstream) == -1)
	{
		AC_ERROR("HallDataDecoder::DoUpdateRoomInfo:: ROOMINFO_UPDATE BroadcastSBCmd error");
		return 0;
	}

	//室主更改通知
	if(flag==1)
	{
		outstream.Clear();
		outstream.Write(type);
		outstream.Write((short)ROOM_CMD_SB_GIVE_OWNER);
		outstream.Write(outseq);
		outstream.Write((int)pRoominfo->ownidx);
		outstream.Flush();	
		if(g_pNetProxy->BroadcastSBCmd(pRoominfo,outstream) == -1)
		{
			AC_ERROR("HallDataDecoder::DoUpdateRoomInfo: GIVE_OWNER BroadcastSBCmd error");
			return 0;
		}
	}
	
	return 0;
}


int HallDataDecoder::DoSyncRoomInfo(short cmd, int seq,BinaryReadStream *pinstream)
{
	AC_DEBUG("HallDataDecoder::DoSyncRoomInfo: IN");

	int total_roomnum=0;
	if(!pinstream->Read(total_roomnum))    
	{
		AC_ERROR("HallDataDecoder::DoSyncRoomInfo:read total_roomnum error");
		return -1;
	}
	int current_roomnum=0;
	if(!pinstream->Read(current_roomnum))    
	{
		AC_ERROR("HallDataDecoder::DoSyncRoomInfo:read current_roomnum error");
		return -1;
	}

	AC_DEBUG("HallDataDecoder::DoSyncRoomInfo:total_roomnum=%d current_roomnum=%d",total_roomnum,current_roomnum);
	
	char context[40960] = {0};
	BinaryWriteStream outstream(context,sizeof(context));
	outstream.Write(cmd);
	outstream.Write(seq);
	outstream.Write((char)ROOM_TYPE_ROOM);
	int count=0;
	char* curpos = outstream.GetCurrent();
	outstream.Write((int)0);
	
	for(int i =0;i<current_roomnum;i++)
	{
		int roomid=0;
		if(!pinstream->Read(roomid))    
		{
			AC_ERROR("HallDataDecoder::DoSyncRoomInfo:read roomid error");
			return -1;
		}

		ROOM_INFO* pRoominfo=g_pNetProxy->GetRoomInfo(roomid);
		if(pRoominfo!=NULL)
		{
			outstream.Write(roomid);	//roomid
			outstream.Write((int)pRoominfo->userlistMale.size());		//mannum
			outstream.Write((int)pRoominfo->userlistFemale.size());	//womannum
			outstream.Write((int)pRoominfo->userlist.size());			//usernum
			map<int,RoomClient*>::iterator itc=pRoominfo->userlist.begin();
			for(;itc!=pRoominfo->userlist.end();itc++)
				outstream.Write((int)itc->first);	//useridx

			count++;
		}
		
	}
	count=htonl(count);
	memcpy(curpos, &count, sizeof(int));
	outstream.Flush();

	HallSvrClient* pHallSvr=g_pNetProxy->GetHallSvr();
	if(pHallSvr!=NULL)
		pHallSvr->AddBuf(outstream.GetData(), outstream.GetSize());			
	else
		AC_ERROR("HallDataDecoder::DoSyncRoomInfo:pHallSvr is NULL ");
	
	return 0;
}

int HallDataDecoder::DoLuckConfChange(BinaryReadStream *pinstream)
{
	AC_DEBUG("HallDataDecoder::DoLuckConfChange: pinstream=%x",pinstream);

	BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
	if(pDBSvr!=NULL)
	{
		int outseq = g_pNetProxy->GetCounter()->Get();
		BinaryWriteStream* outstream = StreamFactory::InstanceWriteStream();
		static const char* spname = {"DBMW_GetLuckConf_HZSTAR"};//存储过程填充
		outstream->Write((short)CMD_CALLSP);
		outstream->Write(outseq);
		outstream->Write(spname,strlen(spname));	
		outstream->Write((short)0);
		outstream->Flush();

		Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
		if(data!=NULL)
		{
			if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1) 
			{
				data->roomid = 9999;
				data->opidx = 10000;
				data->bopidx= 10000;
				data->cmd = ROOM_CMD_DB_LUCKCONF_CHANGE;
				data->outseq=outseq;
				data->SetReactor(g_pNetProxy->GetReactor());
				data->RegisterTimer(DB_TIMEOUT);
				g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
			}
			else
			{
				AC_ERROR("HallDataDecoder::DoLuckConfChange:pDBSvr->AddBuf() error");
				g_pNetProxy->DestroyDBResult(data);
				return -1;
			}
		}
		else
		{
			AC_ERROR("HallDataDecoder::DoLuckConfChange:CreateDBResultdata() error,data=%x",data);			
			return -1;
		}
	}
	else
	{
		AC_ERROR("HallDataDecoder::DoLuckConfChange: pDBSvr=%x",pDBSvr);
		return -1;
	}
	return 0;
}

int HallDataDecoder::DoGiftInfoChange(BinaryReadStream *pinstream)
{
	AC_DEBUG("HallDataDecoder::DoGiftInfoChange: pinstream=%x",pinstream);

	BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
	if(pDBSvr!=NULL)
	{
		int outseq = g_pNetProxy->GetCounter()->Get();
		BinaryWriteStream* outstream = StreamFactory::InstanceWriteStream();
		static const char* spname = {"DBMW_GetGiftInfo"};//存储过程填充
		outstream->Write((short)CMD_CALLSP);
		outstream->Write(outseq);
		outstream->Write(spname,strlen(spname));	
		outstream->Write((short)0);
		outstream->Flush();

		Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
		if(data!=NULL)
		{
			if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1) 
			{
				data->roomid = 9999;
				data->opidx = 10000;
				data->bopidx= 10000;
				data->cmd = ROOM_CMD_SB_GIFTINFO_CHANGE;
				data->outseq=outseq;
				data->SetReactor(g_pNetProxy->GetReactor());
				data->RegisterTimer(DB_TIMEOUT);
				g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
			}
			else
			{
				AC_ERROR("HallDataDecoder::DoGiftInfoChange:pDBSvr->AddBuf() error");
				g_pNetProxy->DestroyDBResult(data);
				return -1;
			}
		}
		else
		{
			AC_ERROR("HallDataDecoder::DoGiftInfoChange:CreateDBResultdata() error,data=%x",data);			
			return -1;
		}
	}
	else
	{
		AC_ERROR("HallDataDecoder::DoGiftInfoChange: pDBSvr=%x",pDBSvr);
		return -1;
	}	
	
	return 0;
}

int HallDataDecoder::DoRightConfChange(BinaryReadStream *pinstream)
{
	AC_DEBUG("HallDataDecoder::DoRightConfChange: pinstream=%x",pinstream);

	BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgentRight();
	if(pDBSvr!=NULL)
	{
		int outseq = g_pNetProxy->GetCounter()->Get();
		BinaryWriteStream* outstream = StreamFactory::InstanceWriteStream();
		static const char* spname = {"DBMW_GetRightXML_HZSTAR"};//存储过程填充
		outstream->Write((short)CMD_CALLSP);
		outstream->Write(outseq);
		outstream->Write(spname,strlen(spname));	
		outstream->Write((short)0);
		outstream->Flush();

		Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
		if(data!=NULL)
		{
			if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1) 
			{
				data->roomid = 9999;
				data->opidx = 10000;
				data->bopidx= 10000;
				data->cmd = ROOM_CMD_SB_RIGHTCONF_CHANGE;
				data->outseq=outseq;
				data->SetReactor(g_pNetProxy->GetReactor());
				data->RegisterTimer(DB_TIMEOUT);
				g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
			}
			else
			{
				AC_ERROR("HallDataDecoder::DoRightConfChange:pDBSvr->AddBuf() error");
				g_pNetProxy->DestroyDBResult(data);
				return -1;
			}
		}
		else
		{
			AC_ERROR("HallDataDecoder::DoRightConfChange:CreateDBResultdata() error,data=%x",data);			
			return -1;
		}
	}
	else
	{
		AC_ERROR("HallDataDecoder::DoRightConfChange: pDBSvr=%x",pDBSvr);
		return -1;
	}
	
	return 0;
}

int HallDataDecoder::DoUpdateRoomManger(short cmd, int seq,BinaryReadStream *pinstream)
{
	AC_DEBUG("HallDataDecoder::DoUpdateRoomManger : cmd=%d,seq=%d",cmd,seq);
	int roomid;
	if(!pinstream->Read(roomid))    //roomid
	{
		AC_ERROR("HallDataDecoder::DoUpdateRoomManger:read roomid error");
		return -1;
	}
	
	ROOM_INFO* pRoominfo=g_pNetProxy->GetRoomInfo(roomid);
	if(pRoominfo == NULL)
	{
		AC_ERROR("HallDataDecoder::DoUpdateRoomManger:roomid=%d not exist",roomid);
		return 0;
	}
	
	AC_DEBUG("HallDataDecoder::DoUpdateRoomManger : roomid=%d",roomid);

	int outseq = g_pNetProxy->GetCounter()->Get();
	BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
	static const char* spname = {"DBMW_GetMultiAdmin_byroomid"};//存储过程填充
	outstream->Write((short)CMD_CALLSP);
	outstream->Write(outseq);
	outstream->Write(spname,strlen(spname));
	outstream->Write((short)1);
	outstream->Write((char)PT_INPUT);
	outstream->Write((char)PDT_INT);
	outstream->Write(roomid);	//房间idx
	outstream->Flush();

	BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
	if(pDBSvr != NULL) 
	    pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize());
	
	Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
	if (data == NULL)
	{
		AC_ERROR("HallDataDecoder::DoUpdateRoomManger:CreateDBResultdata() error,data=%x",data);
		return -1;
	}
	data->roomid = roomid;
	data->opidx = 10000;		//系统管理员
	data->bopidx= 0;
	data->number=0;
	data->cmd = HALL_CMD_ROOMMANAGERCHG;
	data->seq=seq;
	data->SetReactor(g_pNetProxy->GetReactor());
	data->RegisterTimer(DB_TIMEOUT);
	g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
		

	return 0;
}

int HallDataDecoder::DoMoveRoom(short cmd, int seq,BinaryReadStream *pinstream)
{

	AC_DEBUG("HallDataDecoder::DoMoveRoom:cmd = %d seq = %d", cmd ,seq);	
	int roomid=0;
	short delroomnum=0;
	if(!pinstream->Read(delroomnum))
	{
		AC_ERROR("HallDataDecoder::DoMoveRoom :read delroomnum error");
		return -1;
	}
	for(int i=0;i<delroomnum;i++)
	{
		if(!pinstream->Read(roomid))
		{
			AC_ERROR("HallDataDecoder::DoMoveRoom :read roomid error");
			return -1;
		}

		AC_DEBUG("HallDataDecoder::DoMoveRoom:roomid=%d will be delete",roomid);	

		ROOM_INFO* pRoominfo=g_pNetProxy->GetRoomInfo(roomid);
		if(pRoominfo == NULL)
		{
			AC_ERROR("HallDataDecoder::DoMoveRoom:roominfo error");
			return -1;
		}

		map<int,RoomClient*> kicklist;
		map<int,RoomClient*>::iterator itc=pRoominfo->userlist.begin();
		for(;itc!=pRoominfo->userlist.end();itc++)
		{
			RoomClient* pClient=(RoomClient*) itc->second;
			if(pClient && pClient->m_btoken)
	                   kicklist.insert(make_pair(pClient->m_idx,pClient));
		}

		map<int,RoomClient*>::iterator itk=kicklist.begin();
		for(;itk!=kicklist.end();itk++)
		{
			RoomClient* pRoomClient=itk->second;
			if(pRoomClient!=NULL)
				pRoomClient->LeaveRoom();
		}
		/*
		//事先消除印章和排麦提高用户离开房间速度
		//印章信息在用户离开房间时已清除
		map<int,GiftTimeout*>::iterator itseal=pRoominfo->seallist.begin();
		for(;itseal!=pRoominfo->seallist.end();itseal++)
		{
			GiftTimeout* pGdata=(*itseal).second;
			g_pNetProxy->DestroyGifttimeout(pGdata);
		}
		pRoominfo->seallist.clear();
		*/
		//排麦信息在用户离开房间时已清除
		vector<MIC_INFO*>::iterator itMic=pRoominfo->miclist.begin();
		for(;itMic!=pRoominfo->miclist.end();itMic++)
		{
			MIC_INFO* pmicinfo = (MIC_INFO*)(*itMic);
			g_pNetProxy->DestroyMicInfo(pmicinfo);
		}
		pRoominfo->miclist.clear();
		
		map<int,roomtimeout*>::iterator itforbiden=pRoominfo->forbidenlist.begin();
		for(;itforbiden!=pRoominfo->forbidenlist.end();itforbiden++)
		{
			roomtimeout* pRdata=(*itforbiden).second;
			g_pNetProxy->DestroyRoomtimeout(pRdata);
		}
		pRoominfo->forbidenlist.clear();

		//add by jinguanfu 2010/11/27
		//清空虚拟用户列表
		map<int,RoomClient*>::iterator itvu=pRoominfo->vuserlist.begin();
		for(;itvu!=pRoominfo->vuserlist.end();itvu++)
		{
			RoomClient* pRoomClient=itvu->second;
			if(pRoomClient!=NULL)
			{
				g_pNetProxy->DestroyClient(pRoomClient);
			}
		}
		pRoominfo->vuserlist.clear();

	
		g_pNetProxy->DestroyRoomtimeout(pRoominfo->m_pOnMicTimeout);

		g_pNetProxy->DestroyFlowerResult(pRoominfo->FlowerTimer);
		
		AC_DEBUG("HallDataDecoder::DoMoveRoom: roomid=%d,delete success",roomid);
		
		return g_pNetProxy->DeleteRoomByID(pRoominfo);
	
	}


	short addroomnum=0;
	if(!pinstream->Read(addroomnum))
	{
		AC_ERROR("HallDataDecoder::DoMoveRoom :read addroomnum error");
		return -1;
	}
	
	
	for(int i=0;i<addroomnum;i++)
	{
		if(!pinstream->Read(roomid))
		{
			AC_ERROR("HallDataDecoder::DoMoveRoom :read roomid error");
			return -1;
		}	

		AC_DEBUG("HallDataDecoder::DoMoveRoom:roomid=%d will be create",roomid);

		BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
		if(pDBSvr!=NULL)
		{
			int outseq = g_pNetProxy->GetCounter()->Get();
			BinaryWriteStream* outstream = StreamFactory::InstanceWriteStream();
			static const char* spname = {"DBMW_GetMultiRoomInfo_ByRoomID"};
			outstream->Write((short)CMD_CALLSP);
			outstream->Write(outseq);
			outstream->Write(spname,strlen(spname));
			outstream->Write((short)1);
			outstream->Write((char)PT_INPUT);
			outstream->Write((char)PDT_INT);
			outstream->Write(roomid);
			outstream->Flush();

			Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
			if(data!=NULL)
			{
				if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1) 
				{
					data->roomid = roomid;
					data->opidx = 10000;
					data->bopidx= 10000;
					data->cmd = INNER_CMD_GETROOMINFO;
					data->outseq=outseq;
					data->SetReactor(g_pNetProxy->GetReactor());
					data->RegisterTimer(DB_TIMEOUT);
					g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
				}
				else
				{
					AC_ERROR("HallDataDecoder::DoAddRoom:pDBSvr->AddBuf() error");
					g_pNetProxy->DestroyDBResult(data);
					return -1;
				}
			}
			else
			{
				AC_ERROR("HallDataDecoder::DoAddRoom:CreateDBResultdata() error,data=%x",data);			
				return -1;
			}
		}
		
	}
	
	
	return 0;
}

int HallDataDecoder::DoUpdateAVServer(short cmd, int seq,BinaryReadStream *pinstream)
{
	AC_DEBUG("HallDataDecoder::DoUpdateAVServer:cmd = %d seq = %d", cmd ,seq);	

	int roomid=0;
	short roomnum=0;
	char roomstr[1024]={0};
	if(!pinstream->Read(roomnum))
	{
		AC_ERROR("HallDataDecoder::DoUpdateAVServer :read roomnum error");
		return -1;
	}
	for(int i=0;i<roomnum;i++)
	{
		if(!pinstream->Read(roomid))
		{
			AC_ERROR("HallDataDecoder::DoUpdateAVServer :read roomid error,roomnum=%d",roomnum);
			return -1;
		}

		ROOM_INFO* pRoominfo=g_pNetProxy->GetRoomInfo(roomid);
		if(pRoominfo == NULL)
		{
			AC_ERROR("HallDataDecoder::DoUpdateAVServer: get roominfo error,roomid=%d",roomid);
			continue;
		}
		char temp[16]={0};

		sprintf(temp,"%d,",roomid);

		strcat(roomstr,temp);

	}

	if(roomnum>0)
	{
		//向数据库批量取房间的AVServer 信息
		BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
		if(pDBSvr!=NULL)
		{
			int outseq = g_pNetProxy->GetCounter()->Get();
			BinaryWriteStream* outstream = StreamFactory::InstanceWriteStream();
			static const char* spname = {"DBMW_GetAVServer_ByRoomArray_HZSTAR"};
			outstream->Write((short)CMD_CALLSP);
			outstream->Write(outseq);
			outstream->Write(spname,strlen(spname));
			outstream->Write((short)1);
			outstream->Write((char)PT_INPUT);
			outstream->Write((char)PDT_VARCHAR);
			outstream->Write(roomstr,strlen(roomstr));
			outstream->Flush();

			Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
			if(data!=NULL)
			{
				if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1) 
				{
					data->roomid = 9999;
					data->opidx = 10000;
					data->bopidx= 10000;
					data->cmd = ROOM_CMD_SB_AVSERVER_CHANGE;
					data->outseq=outseq;
					data->SetReactor(g_pNetProxy->GetReactor());
					data->RegisterTimer(DB_TIMEOUT);
					g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
				}
				else
				{
					AC_ERROR("HallDataDecoder::DoUpdateAVServer:pDBSvr->AddBuf() error");
					g_pNetProxy->DestroyDBResult(data);
					return -1;
				}
			}
			else
			{
				AC_ERROR("HallDataDecoder::DoUpdateAVServer:CreateDBResultdata() error,data=%x",data);			
				return -1;
			}
		}
		else
		{

			AC_ERROR("HallDataDecoder::DoUpdateAVServer:pDBSvr=null error,data=%x",pDBSvr);			
			return -1;
		}
	}
	
	return 0;
}

int HallDataDecoder::DoVUserLogin(short cmd, int seq,BinaryReadStream *pinstream)
{
	AC_DEBUG("HallDataDecoder::DoVUserLogin:cmd = %d seq = %d", cmd ,seq);	

	int roomid=0;
	if(!pinstream->Read(roomid))
	{
		AC_ERROR("HallDataDecoder::DoVUserLogin :read roomid error");
		return -1;
	}
	
	int useridx=0;
	if(!pinstream->Read(useridx))
	{
		AC_ERROR("HallDataDecoder::DoVUserLogin :read useridx error");
		return -1;
	}
	char sex=0;		//性别0:女1:男
	if(!pinstream->Read(sex)) 
	{
		AC_ERROR("HallDataDecoder::DoUpdateUserinfo:read sex error");
		return -1;
	}
		
	char level=0;		//经验等级
	if(!pinstream->Read(level)) 
	{
		AC_ERROR("HallDataDecoder::DoUpdateUserinfo:read level error");
		return -1;
	}

	short viplevel;		//VIP、皇冠等级
	if(!pinstream->Read(viplevel)) 
	{
		AC_ERROR("HallDataDecoder::DoUpdateUserinfo:read viplevel error");
		return -1;
	}

	char isVip;			//皇冠是否有效
	if(!pinstream->Read(isVip)) 
	{
		AC_ERROR("HallDataDecoder::DoUpdateUserinfo:read isVip error");
		return -1;
	}

	char vipdate[32]={0};		//VIP有效日期
	size_t len=0;
	if(!pinstream->Read(vipdate,sizeof(vipdate)-1,len)) 
	{
		AC_ERROR("HallDataDecoder::DoUpdateUserinfo:read vipdate error");
		return -1;
	}
	vipdate[len] = 0;
	
	int gold;
	if(!pinstream->Read(gold))    //gold
	{
		AC_ERROR("HallDataDecoder::DoUpdateUserinfo:read gold error");
		return -1;
	}

	int silver;
	if(!pinstream->Read(silver))    //silver
	{
		AC_ERROR("HallDataDecoder::DoUpdateUserinfo:read silver error");
		return -1;
	}

	ROOM_INFO* proominfo=g_pNetProxy->GetRoomInfo(roomid);
	if(proominfo==NULL)
	{
		AC_ERROR("HallDataDecoder::DoVUserLogin :GetRoomInfo error, roomid=%d",roomid);
		return -1;

	}
	

	//房间冻结
	if(proominfo->state==1)
	{
		AC_ERROR("HallDataDecoder::DoVUserLogin :room locked");
		return -1;
	}

	if(proominfo->isClose)
	{
		AC_ERROR("HallDataDecoder::DoVUserLogin :room closed");
		return -1;
	}

	map<int, RoomClient*>::iterator itu = proominfo->userlist.find(useridx);
	if(itu != proominfo->userlist.end())
	{
		AC_ERROR("HallDataDecoder::DoVUserLogin :in userlist");
		return -1;
	}

	map<int, RoomClient*>::iterator itvu = proominfo->vuserlist.find(useridx);
	if(itvu != proominfo->vuserlist.end())
	{
		AC_ERROR("HallDataDecoder::DoVUserLogin : in vuserlist");
		return -1;
	}
		
	map<int, int>::iterator itb = proominfo->blacklist.find(useridx);
	if(itb != proominfo->blacklist.end())
	{
		AC_ERROR("HallDataDecoder::DoVUserLogin :in blacklist");
		return -1;
	}

	map<int, int>::iterator ittb = proominfo->tblacklist.find(useridx);
	if(ittb != proominfo->tblacklist.end())
	{
		AC_ERROR("HallDataDecoder::DoVUserLogin :in tblacklist");
		return -1;
	}

	short usercount=proominfo->userlist.size()+proominfo->vuserlist.size();
	//房间到最大允许人数
	if(usercount >= proominfo->allowhuman)
	{
		AC_ERROR("HallDataDecoder::DoVUserLogin :room full");
		return -1;
	}
	
	//房间已到最大人数
	if(usercount >= proominfo->maxhuman)
	{
		AC_ERROR("HallDataDecoder::DoVUserLogin :room maxhuman");
		return -1;
	}


	if (proominfo->isUserOnly)
	{
		AC_ERROR("HallDataDecoder::DoVUserLogin :room private");
		return -1;
	}
		

	if(strlen(proominfo->passwd)>0)
	{
		AC_ERROR("HallDataDecoder::DoVUserLogin :room has password");
		return -1;
	}

	RoomClient* pClient=g_pNetProxy->CreateClient();
	if(pClient==NULL)
	{
		AC_ERROR("HallDataDecoder::DoVUserLogin :CreateClient error");
		return -1;
	}

	pClient->m_idx=useridx;
	pClient->m_roomid=roomid;
	pClient->m_btoken=1;
	pClient->m_identity=USER_ID_NONE;
	pClient->m_sex=sex;
	pClient->m_level=level;
	pClient->m_vipflag=isVip;
	pClient->m_viplevel=viplevel;
	pClient->gold=gold;
	pClient->silver=silver;
	strncpy(pClient->m_vipdate, vipdate, sizeof(pClient->m_vipdate)); 

	proominfo->vuserlist.insert(make_pair(pClient->m_idx, pClient));
	if(pClient->m_sex)
		proominfo->userlistMale.insert(make_pair(pClient->m_idx, pClient->m_idx));
	else
		proominfo->userlistFemale.insert(make_pair(pClient->m_idx, pClient->m_idx));

		
	//发送给房间内所有的人有人进入房间的消息
	char type=65;
	BinaryWriteStream* outstream2=StreamFactory::InstanceWriteStream();
	outstream2->Write(type);
	outstream2->Write((short)ROOM_CMD_SB_ENTERROOM);
	outstream2->Write((int)0);
	outstream2->Write(pClient->m_idx);
	outstream2->Write(pClient->m_identity);
	outstream2->Flush();

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
			if(psend->AddBuf(outbuf,outbuflen) == -1)
			{
				//psend->ErrorClose();
				AC_ERROR("psend->AddBuf Error");
			}
		}
	}
	
	//向大厅发送登陆成功通知
	BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
	outstream->Write((short)HALL_CMD_ENTERROOM);
	outstream->Write(0);
	outstream->Write((char)ROOM_TYPE_ROOM);	
	outstream->Write((int)pClient->m_idx);
	outstream->Write((int)pClient->m_roomid);	
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
		AC_ERROR("HallDataDecoder::DoVUserLogin: pHallSvr is NULL");
		return -1;
	}

	AC_DEBUG("HallDataDecoder::DoVUserLogin :Success!");	

	return 0;

}

int HallDataDecoder::DoVUserLogout(short cmd, int seq,BinaryReadStream *pinstream)
{
	AC_DEBUG("HallDataDecoder::DoVUserLogout:cmd = %d seq = %d", cmd ,seq);	
	
	int roomid=0;
	if(!pinstream->Read(roomid))
	{
		AC_ERROR("HallDataDecoder::DoVUserLogout :read roomid error");
		return -1;
	}
	
	int useridx=0;
	if(!pinstream->Read(useridx))
	{
		AC_ERROR("HallDataDecoder::DoVUserLogout :read useridx error");
		return -1;
	}

	ROOM_INFO* proominfo =g_pNetProxy->GetRoomInfo(roomid);
	if(proominfo == NULL)
	{
		AC_ERROR("HallDataDecoder::DoVUserLogout:GetRoomInfo error,roomid=%d",roomid);
		return -1;
	}

	map<int, RoomClient*>::iterator itvu = proominfo->vuserlist.find(useridx);
	if(itvu == proominfo->vuserlist.end())
	{
		AC_ERROR("HallDataDecoder::DoVUserLogout:useridx=%d not in room[%d]",useridx,roomid);
		return -1;
	}
	RoomClient*	pClient=(RoomClient*)itvu->second;
	if(pClient==NULL)
	{

		AC_ERROR("HallDataDecoder::DoVUserLogout:pClient==NULL ");
		return -1;
	}
	pClient->VLeaveRoom();
	proominfo->vuserlist.erase(itvu);
	
	AC_DEBUG("HallDataDecoder::DoVUserLogout: Success!");	
	
	return 0;

}

int HallDataDecoder::DoUpDateGM(short cmd, int seq/*,BinaryReadStream *pinstream*/)
{
	AC_DEBUG("HallDataDecoder::DoUpDateGM:cmd = %d seq = %d", cmd ,seq);	

	BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
	if(pDBSvr!=NULL)
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
		Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
		if(data!=NULL)
		{
			if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1) 
			{
				data->roomid = 9999;
				data->opidx = 10000;
				data->bopidx= 10000;
				data->cmd = ROOM_CMD_SB_UPDATE_GM;
				data->outseq=outseq;
				data->SetReactor(g_pNetProxy->GetReactor());
				data->RegisterTimer(DB_TIMEOUT);
				g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
			}
			else
			{
				AC_ERROR("HallDataDecoder::DoUpDateGM:pDBSvr->AddBuf() error");
				g_pNetProxy->DestroyDBResult(data);
				return -1;
			}
		}
		else
		{
			AC_ERROR("HallDataDecoder::DoUpDateGM:CreateDBResultdata() error,data=%x",data);			
			return -1;
		}	
	}		
	else
	{

		AC_ERROR("HallDataDecoder::DoUpDateGM:pDBSvr=null error,data=%x",pDBSvr);			
		return -1;
	}
	
	return 0;
}


int HallDataDecoder::DoRemoveSeal(short cmd, int seq,BinaryReadStream *pinstream)
{
	AC_DEBUG("HallDataDecoder::DoRemoveSeal:cmd = %d seq = %d", cmd ,seq);	

	int idx=0;
	if(!pinstream->Read(idx))
	{
		AC_ERROR("HallDataDecoder::DoRemoveSeal :read idx error");
		return -1;
	}

	int seal_id=0;
	if(!pinstream->Read(seal_id))
	{
		AC_ERROR("HallDataDecoder::DoRemoveSeal :read seal_id error");
		return -1;
	}

	map<int, ROOM_INFO*>::iterator itroom = g_pNetProxy->m_roomlistinfo.roommap.begin();
	for(;itroom!=g_pNetProxy->m_roomlistinfo.roommap.end();itroom++)
	{
		ROOM_INFO* proominfo = itroom->second;
		if(proominfo)
		{
			map<int,RoomClient*>::iterator itu = proominfo->userlist.find(idx);
			if (itu != proominfo->userlist.end())
			{
				AC_DEBUG("HallDataDecoder::DoRemoveSeal:idx=%d,roomid=%d,seal_id=%d ",idx,proominfo->roomid,seal_id);

				BinaryWriteStream* outstream = StreamFactory::InstanceWriteStream();
				int seq=0;
				char type = 65;
				outstream->Write(type);
				outstream->Write((short)ROOM_CMD_SB_GIFT_INVALID);
				outstream->Write(seq);
				outstream->Write(idx);
				outstream->Write(seal_id);					
				outstream->Flush();
				
				if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream) == -1)
				{
					AC_ERROR("HallDataDecoder::DoRemoveSeal:BroadcastSBCmd error");		
				}

				break;
			}
		}
		else
		{
			AC_ERROR("proominfo == NULL");
		}
	}

	
	return 0;
}

int HallDataDecoder::DoUpdateRoomBlacklist(short cmd, int seq,BinaryReadStream *pinstream)
{
	AC_DEBUG("HallDataDecoder::DoUpdateRoomBlacklist : cmd=%d",cmd);

	int roomid;
	if(!pinstream->Read(roomid))    //roomid
	{
		AC_ERROR("HallDataDecoder::DoUpdateRoomBlacklist:read roomid error");
		return -1;
	}
	
	ROOM_INFO* pRoominfo=g_pNetProxy->GetRoomInfo(roomid);
	if(pRoominfo == NULL)
	{
		AC_ERROR("HallDataDecoder::DoUpdateRoomBlacklist:roomid=%d not exist",roomid);
		return 0;
	}
	
	AC_DEBUG("HallDataDecoder::DoUpdateRoomBlacklist : roomid=%d",roomid);

	int outseq = g_pNetProxy->GetCounter()->Get();
	BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
	static const char* spname = {"DBMW_GetMultiblacklist_byroomid"};//存储过程填充
	outstream->Write((short)CMD_CALLSP);
	outstream->Write(outseq);
	outstream->Write(spname,strlen(spname));
	outstream->Write((short)1);
	outstream->Write((char)PT_INPUT);
	outstream->Write((char)PDT_INT);
	outstream->Write(roomid);	//房间idx
	outstream->Flush();

	BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
	if(pDBSvr != NULL) 
	    pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize());
	
	Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
	if (data == NULL)
	{
		AC_ERROR("HallDataDecoder::DoUpdateRoomBlacklist :CreateDBResultdata() error,data=%x",data);
		return -1;
	}
	data->roomid = roomid;
	data->opidx = 10000;		//系统管理员
	data->bopidx= 0;
	data->number=0;
	data->cmd = HALL_CMD_ROOMBLACKMEMCHG_L2R;
	data->seq=seq;
	data->SetReactor(g_pNetProxy->GetReactor());
	data->RegisterTimer(DB_TIMEOUT);
	g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
		

	return 0;
}

//add by jinguanfu 2011/8/19
int HallDataDecoder::DoUpdateChatBlacklist(short cmd, int seq,BinaryReadStream *pinstream)
{

	AC_DEBUG("HallDataDecoder::DoUpdateChatBlacklist:cmd=%d,pinstream=%x",cmd,pinstream);

	int outseq = g_pNetProxy->GetCounter()->Get();
	BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
	static const char* spname = {"DBMW_GetChatBlacklist"};//存储过程填充
	outstream->Write((short)CMD_CALLSP);
	outstream->Write(outseq);
	outstream->Write(spname,strlen(spname));
	outstream->Write((short)0);
	outstream->Flush();

	BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
	if(pDBSvr != NULL) 
	    pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize());
	
	Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
	if (data == NULL)
	{
		AC_ERROR("HallDataDecoder::DoUpdateChatBlacklist:CreateDBResultdata() error,data=%x",data);
		return -1;
	}
	data->roomid = 9999;
	data->opidx = 10000;		//系统管理员
	data->bopidx= 0;
	data->number=0;
	data->cmd = HALL_CMD_UPDATE_ROOMCHAT_BLACKLIST_L2R;
	data->seq=seq;
	data->SetReactor(g_pNetProxy->GetReactor());
	data->RegisterTimer(DB_TIMEOUT);
	g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));

	return 0;
}

int HallDataDecoder::DoUpdateGiftConfig(short cmd, int seq)
{
	AC_DEBUG("HallDataDecoder::DoUpdateGiftConfig: cmd=%d",cmd);
	int outseq = g_pNetProxy->GetCounter()->Get();
	BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
	static const char* spname = {"DBMW_GetGiftConfig"};//存储过程填充
	outstream->Write((short)CMD_CALLSP);
	outstream->Write(outseq);
	outstream->Write(spname,strlen(spname));
	outstream->Write((short)0);
	outstream->Flush();
	BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
	if(pDBSvr != NULL) 
		pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize());

	Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
	if (data == NULL)
	{
		AC_ERROR("HallDataDecoder::DoUpdateGiftConfig:CreateDBResultdata() error,data=%x",data);
		return -1;
	}
	data->roomid = 9999;
	data->opidx = 10000;		//系统管理员
	data->bopidx= 0;
	data->number=0;
	data->cmd = HALL_CMD_UPDATE_ROOMGIFT_REFRESH_TIMES_L2R;
	data->seq=seq;
	data->SetReactor(g_pNetProxy->GetReactor());
	data->RegisterTimer(DB_TIMEOUT);
	g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));

	return 0;
}

