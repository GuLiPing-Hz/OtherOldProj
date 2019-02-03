#ifndef TBDB_QUEUE_H_
#define TBDB_QUEUE_H_

#include <db.h>
//#include "TBdbEnv.h"

class TBdbEnv;
class TBdb_Queue
{
public:
	TBdb_Queue();
	~TBdb_Queue();
	int Init(TBdbEnv *env,char *bdbfilename,int recordlen = 8);
	void UnInit();
	int GetInfo(/*void *keybuf,int &keylen,*/void *valuebuf,int &valuelen);//0����û������û���1���������ҵ���2����bdb����
	int PutInfo(/*void *keybuf,int keylen,*/void *valuebuf,int valuelen);
public:
	char m_bdbfilename[100];
	DB *m_db;
};

#endif
