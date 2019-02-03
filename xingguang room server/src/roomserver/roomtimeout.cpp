#include "netproxy.h"
#include "StreamFactory.h"
#include "roomtimeout.h"

void roomtimeout::OnTimeOut()
{
    UnRegisterTimer();

    if(m_roomid <= 0)
    {
        AC_ERROR("roomtimeout::OnTimeOut:roomid error");
		g_pNetProxy->DestroyRoomtimeout(this);
        return;
    }

    if(m_idx == 0)
    {
        AC_ERROR("roomtimeout::OnTimeOut:idx error");
		g_pNetProxy->DestroyRoomtimeout(this);
        return;
    }

    ROOM_INFO* proominfo = g_pNetProxy->GetRoomInfo(m_roomid);
    if(proominfo==NULL)
    {
        AC_ERROR("roomtimeout::OnTimeOut:roomid=%d not exist",m_roomid);
		g_pNetProxy->DestroyRoomtimeout(this);
        return;
    }

    if(m_type == TIMEOUT_TB)
    {
        map<int, int>::iterator itr = proominfo->tblacklist.find(m_idx);
        if(itr == proominfo->tblacklist.end())
        {
            AC_ERROR("roomtimeout::OnTimeOut:idx error");
            return;
        }
        proominfo->tblacklist.erase(itr);

		g_pNetProxy->DestroyRoomtimeout(this);
    }
    //����5s��������ÿһ��������
	else if(m_type == TIMEOUT_OFFMIC)
	{
		
        //��������б����˾ͷ�������ready����Ϣ
		//if(m_pClientDataDecoder->CheckMiclistOnmic(proominfo) == -1)			
		if(m_pClientDataDecoder->CheckMiclistOnmic2(proominfo) == -1)
		{
			AC_ERROR("roomtimeout::TIMEOUT_OFFMIC:CheckMiclistOnmic error");
			return ;
		}

		g_pNetProxy->DestroyRoomtimeout(this);
	}
	//add by jinguanfu 2010/2/3 <begin>����ʱ���Զ�����
	else if(m_type==TIMEOUT_ONMIC)
	{

		if(m_pClientDataDecoder==NULL)
			return;

		if(proominfo->onmic.idx==0)
			return;

		if(proominfo->onmic.idx!=m_idx)
			return;
		
		BinaryWriteStream* outstream2 = StreamFactory::InstanceWriteStream();
		char type = 65;
		/*
		//add by jinguanfu 2010/10/28
		outstream2->Write(type);
		outstream2->Write((short)ROOM_CMD_SB_SCORE);
		outstream2->Write(0);			//seq
		outstream2->Write(0);			//��ǰ��������ֵ
		outstream2->Write(0);			//��ǰ��������ֵ
		outstream2->Flush();
		g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2);
		*/
		AC_DEBUG("roomtimeout::TIMEOUT_ONMIC: roomid=%d,onmicidx=%d",proominfo->roomid,proominfo->onmic.idx);
		
		outstream2 ->Clear();
		outstream2->Write(type);
		outstream2->Write((short)ROOM_CMD_SB_OFFMIC);
		outstream2->Write(0);
		outstream2->Write(m_haspk);
		outstream2->Write((int)proominfo->onmic.idx);  	
	    outstream2->Write((int)0);		//��������ֵadd by jinguanfu 2011/8/10
	    outstream2->Write(0);			//�����Ϣ��	
		outstream2->Flush();

		if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream2) == -1)
		{
			AC_ERROR("roomtimeout::OnTimeOut:g_pNetProxy->BroadcastSBCmd error,m_type=TIMEOUT_ONMIC");		
		}

		//add by jinguanfu 2011/1/25
		map<int,RoomClient*>::iterator itc = proominfo->userlist.find(proominfo->onmic.idx);
		if(itc!=proominfo->userlist.end())
		{
			RoomClient* pRoomClient=(RoomClient*)itc->second;
			pRoomClient->m_onmicflag = USER_STATE_NONE;	
		}

		//����״̬����־������
		BinaryWriteStream* logstream=StreamFactory::InstanceWriteStream();
		logstream->Write((short)CMD_ROOM_OFFMIC_R2LS);
		logstream->Write(0);
		logstream->Write(proominfo->onmic.idx); //�û�idx
		logstream->Write(proominfo->roomid); 	//���ڷ���ID
		logstream->Flush();
							
		BackClientSocketBase *pLogSvr = g_pNetProxy->GetLogServer();
		if(pLogSvr!=NULL)
		{
			if(pLogSvr->AddBuf(logstream->GetData(), logstream->GetSize())==-1) 
			{
				AC_ERROR("roomtimeout::OnTimeOut:pLogSvr->AddBuf Error");
			}else
			{
				AC_DEBUG("roomtimeout::OnTimeOut:Send to LogSvr success,idx=%d roomid=%d",proominfo->onmic.idx,proominfo->roomid);
			}
						
		}
		else
		{
			AC_ERROR("roomtimeout::OnTimeOut:pLogSvr is NULL");
		}


		proominfo->onmic.init();

		//����5�����һ��������
		roomtimeout* ptb = g_pNetProxy->CreateRoomTimeout();
		if(ptb!=NULL)
		{
	        ptb->m_roomid = m_roomid;
			ptb->m_idx = m_idx;
			ptb->m_time = OFFMIC_WAIT; 		//5s
			ptb->m_type = TIMEOUT_OFFMIC;
			ptb->SetReactor(g_pNetProxy->GetReactor());
			ptb->m_pClientDataDecoder=m_pClientDataDecoder;
			ptb->m_haspk = m_haspk;
			ptb->RegisterTimer(ptb->m_time);
		}
		else
		{
			AC_ERROR("roomtimeout::OnTimeOut:CreateRoomTimeout error");	
			//m_pClientDataDecoder->CheckMiclistOnmic(proominfo);
			m_pClientDataDecoder->CheckMiclistOnmic2(proominfo);

		}

		//����ʱ���󷴸�ʹ�ã�������Destroy();
	}
	else if(m_type==TIMEOUT_FORBIDEN)
	{
		map<int, roomtimeout*>::iterator itr = proominfo->forbidenlist.find(m_idx);
		if(itr == proominfo->forbidenlist.end())
		{
			AC_ERROR("roomtimeout::OnTimeOut:idx error");
		}
		else
		{
			proominfo->forbidenlist.erase(itr);
		}

		g_pNetProxy->DestroyRoomtimeout(this);
	}	
	else if(m_type==TIMEOUT_READY)
	{
		if(m_pClientDataDecoder==NULL)
			return;
		
		if(proominfo->onmic.idx==0)
			return;

		if(proominfo->onmic.idx!=m_idx)
			return;

		AC_DEBUG("roomtimeout::TIMEOUT_READY:proominfo->onmic.idx=%d,",proominfo->onmic.idx);

		char type = 65;
		BinaryWriteStream* outstream = StreamFactory::InstanceWriteStream();
		/*	
		outstream->Write(type);
		outstream->Write((short)ROOM_CMD_SB_SCORE);
		outstream->Write(0);			//seq
		outstream->Write(0);			//��ǰ��������ֵ
		outstream->Write(0);			//��ǰ��������ֵ
		outstream->Flush();
		g_pNetProxy->BroadcastSBCmd(proominfo, *outstream);
		*/
		outstream->Clear();
		outstream->Write(type);
		outstream->Write((short)ROOM_CMD_SB_OFFMIC);
		outstream->Write(0);
		outstream->Write(m_haspk);
		outstream->Write((int)proominfo->onmic.idx);					
		outstream->Write((int)0);	//��������ֵadd by jinguanfu 2011/8/10
	    outstream->Write(0);			//�����Ϣ��	
		outstream->Flush();
		g_pNetProxy->BroadcastSBCmd(proominfo, *outstream) ;

		proominfo->onmic.init();
	
		//����5�����һ��������
		roomtimeout* ptb = g_pNetProxy->CreateRoomTimeout();
		if(ptb!=NULL)
		{
			ptb->m_roomid = m_roomid;
			ptb->m_idx = m_idx;
			ptb->m_time = OFFMIC_WAIT;		//5s
			ptb->m_type = TIMEOUT_OFFMIC;
			ptb->SetReactor(g_pNetProxy->GetReactor());
			ptb->m_pClientDataDecoder=m_pClientDataDecoder;
			ptb->m_haspk = m_haspk;
			ptb->RegisterTimer(ptb->m_time);
		}
		else
		{
			AC_ERROR("roomtimeout::OnTimeOut:CreateRoomTimeout error"); 
			//m_pClientDataDecoder->CheckMiclistOnmic(proominfo);
			m_pClientDataDecoder->CheckMiclistOnmic2(proominfo);
		}

		//����ʱ���󷴸�ʹ�ã�������Destroy();
	}

}

//add by jinguanfu 2010/1/15
void Dbresultdata::OnTimeOut()
{

	AC_ERROR("Dbresultdata::OnTimeOut: cmd=%d,seq=%d,outseq=%d,roomid=%d,opidx=%d,bopidx=%d",cmd,seq,outseq,roomid,opidx,bopidx);

	UnRegisterTimer();
	g_pNetProxy->ClearClientDBMap(outseq);
	g_pNetProxy->DestroyDBResult(this);
	
	
}

void Flowerresultdata::OnTimeOut()
{
/*
	// 3������һ�Σ�30ms��һ�仨
	// ����һ����100��
	AC_DEBUG("Flowerresultdata::OnTimeOut ,number=%d, havesend=%d",number,havesend);
	//ʣ��������100
	if(havesend >=number-100)
	{
		havesend=number;
		number=0;
		UnRegisterTimer();
		//����������б���ɾ��
		ROOM_INFO* proominfo=g_pNetProxy->GetRoomInfo(roomid);
		if(proominfo!=NULL)
		{
			map<int,Flowerresultdata*>::iterator itf=proominfo->giftlist.find(seq);
			if(itf!=proominfo->giftlist.end())
				proominfo->giftlist.erase(itf);
			else
				AC_ERROR("Flowerresultdata::OnTimeOut: seq error");
		}


		g_pNetProxy->DestroyFlowerResult(this);
	}
	else
	{
		havesend+=100;
	}	
*/
	//��ʱ��Ƭ��ת�����㷨ִ����������
	ROOM_INFO* proominfo=g_pNetProxy->GetRoomInfo(roomid);
	if(proominfo!=NULL)
	{
		vector<SEND_GIFT_INFO*>::iterator itsendgift=proominfo->sendgiftlist.begin();
		if(itsendgift!=proominfo->sendgiftlist.end())
		{
			SEND_GIFT_INFO* pSendgift=(SEND_GIFT_INFO*)(*itsendgift);

			//����С��5000 ��ˢ�����ٶȳ�10��
			//modify by lihongwu 2011/10/11
			pSendgift->havesend+=(SENDGIFT_SETTM) * 1000 * pSendgift->numpertime / pSendgift->interval; 

			AC_DEBUG("Flowerresultdata::OnTimeOut ,roomid=%d,seq=%d,giftID=%d,number=%d, havesend=%d",
				proominfo->roomid,pSendgift->seq,pSendgift->cate_idx,pSendgift->number,pSendgift->havesend);
			
			//������ˢ��
			if(pSendgift->havesend >=pSendgift->number)
			{
				proominfo->sendgiftlist.erase(itsendgift);
				g_pNetProxy->DestroySendGift(pSendgift);
			}
			else
			{
				//�Ƶ��������
				proominfo->sendgiftlist.erase(itsendgift);	
				proominfo->sendgiftlist.push_back(pSendgift);	
			}
		}
	}
	
}

/*
void GiftTimeout::OnTimeOut()
{

	AC_DEBUG("GiftTimeout::OnTimeOut :roomid =%d idx=%d giftID=%d is be invalid",roomid,idx,GiftID);

	ROOM_INFO* proominfo=g_pNetProxy->GetRoomInfo(roomid);
	
	if(proominfo!=NULL)
	{
		BinaryWriteStream* outstream = StreamFactory::InstanceWriteStream();
		int seq=0;
		char type = 65;
		outstream->Write(type);
		outstream->Write((short)ROOM_CMD_SB_GIFT_INVALID);
		outstream->Write(seq);
		outstream->Write(idx);
		outstream->Write(GiftID);  					
		outstream->Flush();

		if(g_pNetProxy->BroadcastSBCmd(proominfo, *outstream) == -1)
		{
			AC_ERROR("GiftTimeout::OnTimeOut:BroadcastSBCmd error");		
		}

		map<int,GiftTimeout*>::iterator itGift=proominfo->seallist.find(idx);
		if(itGift!=proominfo->seallist.end())
		{
			proominfo->seallist.erase(itGift);
		}
	
	}


	
	g_pNetProxy->DestroyGifttimeout(this);

}
*/
void LobbyTimeout::OnTimeOut()
{

	//�Ӵ���ȡ���û���Ϣ��ʱ
	if(cmd!= ROOM_CMD_TOKEN&&cmd!= ROOM_CMD_LOGIN)
	{
		AC_ERROR("LobbyTimeout::OnTimeOut: cmd[%d] error",cmd);
	}
	else
	{
		AC_ERROR("LobbyTimeout::OnTimeOut: get userinfo[idx=%d] timeout from lobby",idx);
		//modify by jinguanfu 2011/9/27
		BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();

		char type=65;
		outstream->Write(type);
		outstream->Write(cmd);
		outstream->Write(seq);
		outstream->Write((int)TOKENLOGIN_INVALID);
		outstream->Flush();

		RoomClient* pClient=g_pNetProxy->GetClient(clientID);
		if(pClient!=NULL)
		{
			if(g_pNetProxy->SendToSrv(pClient, *outstream)==-1)
			{
				AC_ERROR("LobbyTimeout::OnTimeOut:  idx=%d,SendTo client error",idx);
			}
		}
		else
		{
			AC_ERROR("LobbyTimeout::OnTimeOut: client not found, idx=%d",idx);
		}
		/*
		//�Ͽ��û�����
		RoomClient* pClient=g_pNetProxy->GetClient(clientID);
		if(pClient!=NULL)
			pClient->ErrorClose();
		*/
	}

	UnRegisterTimer();
	g_pNetProxy->ClearLobbyTMMap(outseq);
	g_pNetProxy->DestroyLobbyTimeout(this);
	
	
}

void MusicInfoUpdate::OnTimeOut()
{
	//ÿ���賿06:00~07:00 ����¸����㳪��
	//��Ϣ���µ����ݿ�
	time_t timep;
    	struct tm *p;
	time(&timep);
    	p=localtime(&timep);  /* ��ȡ��ǰʱ�� */

	char musicinfo[1024*1024]={0};
	
	int count=g_pNetProxy->m_Music.size();

	//AC_DEBUG("MusicInfoUpdate::OnTimeOut:p->tm_hour=%d, m_Music.size=%d",p->tm_hour,count);
	if(p->tm_hour==6&&count!=0)
	{
		AC_DEBUG("MusicInfoUpdate::OnTimeOut:p->tm_hour=%d, m_Music.size=%d",p->tm_hour,count);
		map<int,int>::iterator itMusic=g_pNetProxy->m_Music.begin();
		for(; itMusic!=g_pNetProxy->m_Music.end();itMusic++)
		{
			char temp[64]={0};
			sprintf(temp,"%d,%d|",itMusic->first,itMusic->second);
			strcat(musicinfo,temp);
		}

		AC_DEBUG("MusicInfoUpdate::OnTimeOut: musicinfo=%s",musicinfo);
		BackClientSocketBase *pDBSvr = g_pNetProxy->GetDBAgent();
		if(pDBSvr!=NULL)
		{
			int outseq = g_pNetProxy->GetCounter()->Get();
			BinaryWriteStream* outstream=StreamFactory::InstanceWriteStream();
			static const char* spname = {"DBMW_UpdateMusicInfo_HZSTAR"};//�洢�������
			outstream->Write((short)CMD_CALLSP);
			outstream->Write(outseq);
			outstream->Write(spname,strlen(spname));
			outstream->Write((short)2);
			outstream->Write((char)PT_INPUT);
			outstream->Write((char)PDT_INT);
			outstream->Write((int)count);	
			outstream->Write((char)PT_INPUT);
			outstream->Write((char)PDT_VARCHAR);
			outstream->Write(musicinfo,strlen(musicinfo));		
			outstream->Flush();

			Dbresultdata* data=g_pNetProxy->CreateDBResultdata();
			if(data!=NULL)
			{
				if(pDBSvr->AddBuf(outstream->GetData(), outstream->GetSize())!=-1) 
				{
					
					data->roomid = 9999;	//ϵͳ����
					data->opidx = 10000;	//ϵͳIDX
					data->bopidx= 10000;	//ϵͳIDX
					data->cmd = ROOM_CMD_UPDATE_MUSICINFO;
					data->seq=0;
					data->outseq=outseq;
					data->SetReactor(g_pNetProxy->GetReactor());
					data->RegisterTimer(DB_TIMEOUT);
					g_pNetProxy->m_DBMap.insert(make_pair(outseq, data));
					
				}
				else
				{
					AC_ERROR("MusicInfoUpdate::OnTimeOut:pDBSvr->AddBuf() error");
					g_pNetProxy->DestroyDBResult(data);
				}
			}
			else
			{
				AC_ERROR("MusicInfoUpdate::OnTimeOut:CreateDBResultdata() error,data=%x",data);
				
			}
		}
		else
		{
			AC_ERROR("MusicInfoUpdate::OnTimeOut: pDBSvr=%x",pDBSvr);
		}

		//���º����ݿ����ձ��ص㳪��Ϣ
		g_pNetProxy->m_Music.clear();
	}
	
}

