#ifndef AC_BDBENV_H_
#define AC_BDBENV_H_

#include <db.h>

namespace ac
{
	class BdbEnv
	{
		public:
			BdbEnv() : m_Env(NULL){}
			int Init(const char *bdbenvfolder,const char *bdbfolder,int bdbcachesize_gb,int bdbcachesize_b,int flag = DB_CREATE | DB_INIT_LOCK | DB_INIT_LOG | DB_INIT_MPOOL | DB_THREAD,int lock_times = 1);
			void UnInit();
			inline DB_ENV* GetEnv() const {return m_Env;}
		private:
			char m_bdbenvfolder[64];
			char m_bdbfolder[64];
			DB_ENV *m_Env;
	};
}

#endif
