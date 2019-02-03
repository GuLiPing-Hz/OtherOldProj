#ifndef AC_NETWORK_REACTOR_H_
#define AC_NETWORK_REACTOR_H_
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <map>
#include <set>
#include "ac/util/non_copyable.h"
using namespace std;

namespace ac
{
	class TMEventHandler;
	class FDEventHandler;
	class IdleEventHandler;
	class Reactor : public NonCopyable
	{
		public:
			virtual ~Reactor(){}
			virtual void RegisterTimer(TMEventHandler *pHandler) = 0;
			virtual int RegisterReadEvent(FDEventHandler *pHandler,struct timeval *timeout) = 0;
			virtual int RegisterWriteEvent(FDEventHandler *pHandler,struct timeval *timeout) = 0;
			virtual void RegisterIdle(IdleEventHandler *pHandler) = 0;
			
			virtual void UnRegisterTimer(TMEventHandler *pHandler) = 0;
			virtual int UnRegisterEvent(FDEventHandler *pHandler) = 0;
			virtual int UnRegisterReadEvent(FDEventHandler *pHandler) = 0;
			virtual int UnRegisterWriteEvent(FDEventHandler *pHandler) = 0;
			virtual void UnRegisterIdle(IdleEventHandler *pHandler) = 0;			

			virtual int Init() = 0;
			virtual void Run() = 0;
			virtual void Stop() = 0;
	};
	class EventHandlerSet
	{
		typedef struct FDEHInfo
		{
			FDEventHandler *pHandler;
			struct timeval rto;
			struct timeval wto;
			struct timeval rregtime;
			struct timeval wregtime;
			int flag;
		}FDEHINFO;

		public:	
			void AddFDEventHandler(FDEventHandler *pHandler,int mask,struct timeval *timeout);
			void AddTMEventHandler(TMEventHandler *pHandler);
			void AddIdleEventHandler(IdleEventHandler *pHandler);
			void DelFDEventHandler(int fd,int mask);
			void DelTMEventHandler(TMEventHandler *pHandler);
			void DelIdleEventHandler(IdleEventHandler *pHandler);
			FDEventHandler* GetFDEventHandler(int fd,int *flag = NULL,int isupdatetime = 0);
			void GetInfo(char *str)
			{
				char sz[50] = {0};
				sprintf(sz,"fdeh count = %d , tmeh count = %d\n",m_FDEHMap.size(),m_TMEHList.size());
				strcat(str,sz);
			}

			void Scan();
			void Idle();
		private:
			map<int,FDEHINFO > m_FDEHMap;
			set<TMEventHandler*> m_TMEHList;
			set<IdleEventHandler*> m_IdleEHList;
	};
	class EPReactor : public Reactor
	{
		public:
			EPReactor() : m_running(true){}
			virtual ~EPReactor(){}
			virtual void RegisterTimer(TMEventHandler *pHandler);
			virtual int RegisterReadEvent(FDEventHandler *pHandler,struct timeval *timeout);
			virtual int RegisterWriteEvent(FDEventHandler *pHandler,struct timeval *timeout);
			virtual void RegisterIdle(IdleEventHandler *pHandler);
			virtual void UnRegisterTimer(TMEventHandler *pHandler);
			virtual int UnRegisterEvent(FDEventHandler *pHandler);
			virtual int UnRegisterReadEvent(FDEventHandler *pHandler);
			virtual int UnRegisterWriteEvent(FDEventHandler *pHandler);
			virtual void UnRegisterIdle(IdleEventHandler *pHandler);
			virtual int Init();
			virtual void Run();
			virtual void Stop();
			void GetInfo(char *str){m_Set.GetInfo(str);}
		private:
			EventHandlerSet m_Set;
			int m_epfd;
			bool m_running;
	};
}

#endif

