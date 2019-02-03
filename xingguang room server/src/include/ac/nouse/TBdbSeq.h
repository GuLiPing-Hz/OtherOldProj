#ifndef TBDBSEQ_H_
#define TBDBSEQ_H_

#include <db.h>

class TBdb_UK;

class TBdbSeq
{
public:
	int Init(TBdb_UK *db);
	void UnInit();
	int GetSeq(db_seq_t* seq);
public:
	DB_SEQUENCE *m_dbseq;
};
#endif
