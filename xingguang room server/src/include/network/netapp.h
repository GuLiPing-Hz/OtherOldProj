#ifndef AC_NETAPP_H_
#define AC_NETAPP_H_

#include "network/listensocket.h"
#include "network/clientsocket.h"
#include "network/reactor.h"
#include "network/counter.h"
#include "ac/util/objectpool.h"
#include "network/datadecoder.h"
#include "network/package.h"
#include "ac/util/queue.h"
#include "ac/util/notify.h"
#include "ac/util/mutex.h"
#include "network/dataprocessor.h"
#include "network/workthreadpool.h"
#include "network/resulteventhandler.h"

namespace ac
{
	class NetApp
	{
		public:
			NetApp(int port,size_t maxclient,size_t maxqueuecount,size_t threadcount,DataProcessor *pDataProcessor,int pttype,size_t hdlen,int udpport,int timeout = 3) : 
				m_ClientSocketPool(maxclient),
				m_ProcessQueue(maxqueuecount),
				m_ResultQueue(maxqueuecount),
				m_ProcessPackagePool(maxqueuecount,0,0,&m_processmutex),
				m_ResultPackagePool(maxqueuecount,0,0,&m_resultmutex),
				m_DataDecoder(pttype,hdlen,&m_ProcessQueue,&m_ProcessPackagePool),
				m_pDataProcessor(pDataProcessor),
				m_WorkThreadPool(&m_ResultPackagePool,&m_ProcessQueue,&m_ResultQueue,m_pDataProcessor),
				m_threadcount(threadcount),
				m_listener(port,&m_Reactor,&m_ClientSocketPool,&m_counter,&m_DataDecoder,&m_MapForClient,timeout),
				m_udplistener(&m_Reactor,udpport),
				m_resulteventhandler(&m_Reactor,&m_ResultQueue,&m_MapForClient)
			{
				m_ResultQueue.SetNotification(&m_SockNotification);	
			}
			virtual ~NetApp(){}
			virtual int Start();
			virtual int Stop();
			void GetInfo(char* str)
			{
				m_Reactor.GetInfo(str);
				char sz[50] = {0};
				sprintf(sz,"process queue count = %d , result queue count = %d\n",m_ProcessQueue.GetSize(),m_ResultQueue.GetSize());
				strcat(str,sz);
			}
		protected:
			EPReactor m_Reactor;
			ObjectPoolAllocator<ClientSocket> m_ClientSocketPool;
                        SyncQueue<Package*> m_ProcessQueue;
                        SyncQueue<ResultPackage*> m_ResultQueue;
			ObjectPoolAllocator<Package> m_ProcessPackagePool;
			ObjectPoolAllocator<ResultPackage> m_ResultPackagePool;
			DataDecoder_Q m_DataDecoder;
			//DataDecoder_Q_echo m_DataDecoder;
			//DataDecoder_helloworld m_DataDecoder;
			DataProcessor *m_pDataProcessor;
			SocketNotification m_SockNotification;
			WorkThreadPool m_WorkThreadPool;
			size_t m_threadcount;
			ListenSocket m_listener;
			UdpListenSocket m_udplistener;
			Counter m_counter;
			ClientMap m_MapForClient;
			ThreadMutex m_processmutex;
			ThreadMutex m_resultmutex;

			ResultEventHandler m_resulteventhandler;
	};
}

#endif
