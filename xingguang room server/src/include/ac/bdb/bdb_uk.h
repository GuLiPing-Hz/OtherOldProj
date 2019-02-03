#ifndef AC_BDB_UK_H
#define AC_BDB_UK_H

#include "ac/bdb/bdbenv.h"

namespace ac
{
	class Bdb_UK
	{
		public:
			Bdb_UK() : m_db(NULL){}
			int Init(BdbEnv *env,const char *bdbfilename,const char *database = NULL,DBTYPE dbtype = DB_BTREE,int type = DB_CREATE | DB_THREAD);
			void UnInit(bool closebdb = false);
			//0--成功 -1--bdb错误 -2--找不到
			int GetInfo(void *keybuf,int keylen,void *valuebuf,int &valuelen);
			int PutInfo(void *keybuf,int keylen,void *valuebuf,int valuelen);
			//0--成功 -1--bdb错误 -2--找不到
			int DelInfo(void *keybuf,int keylen);
			int SyncToFile();
			int Clear(u_int32_t *count);
			inline DB* GetDB(){return m_db;}
		private:
			char m_bdbfilename[64];
			char m_database[64];
			DB *m_db;
	};
}

#endif
