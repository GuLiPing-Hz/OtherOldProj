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
	int VLeaveRoom();	//�����û��˳�����

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
	char m_viptype;	//VIP����(��ͬm_identity�Ķ���)
	int gold;			//�����
	int silver;		//������
	char m_level;		//����ȼ�
	short m_viplevel;		//VIPֵ����������VIP���ʹڵȼ�
	char m_vipflag;		//VIP���ʹ��Ƿ���Ч
	char m_vipdate[32];	//VIP��Ч����
	char m_sex;		//�Ա�0:Ů1:��
	//void* gifttimeout;	//������Ч�ڼ�ʱ
	char m_onmicflag;	//0:δ���� 1:������ 2:������ 3:������
	short m_starnum;		//�û��ǹ���
	//add by jinguanfu 2011/8/10
	short m_status;		//�û�������״̬
	int m_onlinetime;		//�û���������ʱ��
	int m_logintime;		//�û����ε�½ʱ��

	
};

#endif // ROOMCLIENT_H_



