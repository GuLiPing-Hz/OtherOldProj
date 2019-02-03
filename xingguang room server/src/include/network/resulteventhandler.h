#ifndef AC_RESULTEVENTHANDLER_H_
#define AC_RESULTEVENTHANDLER_H_

#include "network/eventhandler.h"
#include "ac/util/queue.h"
#include "ac/util/objectpool.h"
#include "network/package.h"
#include "network/counter.h"

namespace ac
{
	class ResultEventHandler : public FDEventHandler
	{
	public:
		ResultEventHandler(Reactor *pReactor,SyncQueue<ResultPackage*> *pResultQueue,ClientMap *pClientMap) 
			: FDEventHandler(pReactor),m_pResultQueue(pResultQueue),m_pClientMap(pClientMap) {
			SetReactor(pReactor);
		}
		~ResultEventHandler(){}
		virtual void OnFDRead();
		virtual void OnFDWrite(){};
		virtual void OnFDReadTimeOut(){};
		virtual void OnFDWriteTimeOut(){};
	public:
		SyncQueue<ResultPackage*> *m_pResultQueue;
		ClientMap *m_pClientMap;
	};
}

#endif
