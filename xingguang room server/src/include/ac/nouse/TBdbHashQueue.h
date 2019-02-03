#ifndef TBDB_HASH_QUEUE_H_
#define TBDB_HASH_QUEUE_H_

#include "TBdb_UK.h"
#include "TBdb_Queue.h"
#include "TBdbSeq.h"

class TBdbEnv;

class TBdbHashQueue
{
private:
	TBdb_UK		m_bdb;
	TBdb_UK		m_bdbseq;
	TBdb_Queue	m_queue;
	TBdbSeq		m_seq;

public:
	int Init(TBdbEnv* env, const char* dbfile);

	void UnInit(bool closebdb);

	int GetInfo(void* buf, int& len);

	int PutInfo(const void* buf, int len);
};

#endif // TBDB_HASH_QUEUE_H_

