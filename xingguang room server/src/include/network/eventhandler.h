#ifndef AC_NETWORK_EVENTHANDLER_H_
#define AC_NETWORK_EVENTHANDLER_H_

#include "ac/log/log.h"
#include "ac/util/non_copyable.h"
#include "ac/util/data_block.h"
#include "reactor.h"
#include <unistd.h>
#include <sys/time.h>

namespace ac
{
	class EventHandler : public NonCopyable
	{
		public:
			virtual ~EventHandler(){}
			EventHandler(){}
			inline void SetReactor(Reactor *pReactor){m_pReactor = pReactor;}
			inline Reactor* GetReactor(){return m_pReactor;}
			virtual void Close() = 0;
		protected:
			Reactor *m_pReactor;
	};
	class IdleEventHandler : virtual public EventHandler
	{
		public:
			IdleEventHandler(Reactor *pReactor)
			{
				SetReactor(pReactor);
			}
			virtual ~IdleEventHandler(){}
			virtual void OnRun() = 0;
			inline void RegisterIdle()
			{
				GetReactor()->RegisterIdle(this);
			}
			inline void UnRegisterIdle()
			{
				GetReactor()->UnRegisterIdle(this);
			}
			virtual void Close()
			{
				GetReactor()->UnRegisterIdle(this);
			}
	};
	class TMEventHandler : virtual public EventHandler
	{
		public:
			TMEventHandler()
			{
			}
			TMEventHandler(Reactor *pReactor)
			{
				SetReactor(pReactor);
			}
			virtual ~TMEventHandler()
			{
			}
			virtual void OnTimeOut() = 0;
			inline void RegisterTimer(struct timeval *timeout)
			{
				to.tv_sec = timeout->tv_sec;
				to.tv_usec = timeout->tv_usec;
				if( -1 == (gettimeofday(&regtime,NULL)))
				{
					AC_ERROR("get time error");
				}

				GetReactor()->RegisterTimer(this);
			}
			inline void RegisterTimer(int timeout)
			{
				to.tv_sec = timeout;
				to.tv_usec = 0;
				if(-1 == gettimeofday(&regtime,NULL))
				{
					AC_ERROR("get time error");
				}

				GetReactor()->RegisterTimer(this);
			}
			inline void UnRegisterTimer()
			{
				GetReactor()->UnRegisterTimer(this);
			}
			virtual void Close()
			{
				GetReactor()->UnRegisterTimer(this);
			}
		public:
			struct timeval to;
                        struct timeval regtime;
	};
/*
	class TimerTest : public TMEventHandler
	{
		public:
			TimerTest(Reactor *pReactor) : TMEventHandler(pReactor){}
			virtual void OnTimeOut()
			{
				AC_INFO("time out");
				struct timeval timeout = {3,0};
				Register(&timeout);
			}
	};
*/
/*
	class SNEventHandler : public EventHandler
	{
		public:
			SNEventHandler()
			{
			}
			SNEventHandler(Reactor *pReactor,int signaltype) : EventHandler(pReactor),m_signaltype(signaltype)
			{
			}
			virtual ~SNEventHandler()
			{
			}
			int Register();
			virtual void OnSignal() = 0;
			virtual void Close();
		protected:
			int m_signaltype;
	};
*/
	class FDEventHandler : virtual public EventHandler
	{
		public:
			virtual ~FDEventHandler()
			{
			}
			FDEventHandler()
			{
			}
			FDEventHandler(Reactor *pReactor)
			{
				SetReactor(pReactor);
			}
			inline void SetFD(int fd){m_fd = fd;}
			virtual void OnFDRead() = 0;
			virtual void OnFDWrite() = 0;
			virtual void OnFDReadTimeOut() = 0;
			virtual void OnFDWriteTimeOut() = 0;
			virtual void Close()
			{
				GetReactor()->UnRegisterEvent(this);
				::close(m_fd);
			}
			inline int RegisterRead(struct timeval *timeout)
			{
				Reactor *p = GetReactor();
				return p->RegisterReadEvent(this,timeout);
			}
			inline int RegisterRead(int timeout)
			{
				struct timeval timeout_;
				timeout_.tv_sec = timeout;
				timeout_.tv_usec = 0;

				return GetReactor()->RegisterReadEvent(this,&timeout_);
			}
			inline int RegisterWrite(struct timeval *timeout)
			{
				return GetReactor()->RegisterWriteEvent(this,timeout);
			}
			inline int RegisterWrite(int timeout)
			{
				struct timeval timeout_;
				timeout_.tv_sec = timeout;
				timeout_.tv_usec = 0;

				return GetReactor()->RegisterWriteEvent(this,&timeout_);
			}
			inline int UnRegisterRead()
			{
				return GetReactor()->UnRegisterReadEvent(this);
			}
			inline int UnRegisterWrite()
			{
				return GetReactor()->UnRegisterWriteEvent(this);
			}
			inline int GetFD() const {return m_fd;}
			int SetNonBlocking(bool blocking);                        
			inline void SetClientID(int id){m_id = id;}
                        inline int GetClientID(){return m_id;}
		protected:
			int m_id;
			int m_fd;
	};
}

#endif

