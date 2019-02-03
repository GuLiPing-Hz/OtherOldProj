#ifndef AC_WORKTHREADPOOL_H_
#define AC_WORKTHREADPOOL_H_

#include "ac/util/threadpool.h"
#include "network/package.h"
#include "ac/util/queue.h"
#include "network/dataprocessor.h"
#include "ac/util/objectpool.h"
#include "ac/util/objectmanager.h"

namespace ac
{
	class WorkThreadPool : public ThreadPool,public ObjectManager<ResultPackage>
	{
	public:
		WorkThreadPool(ObjectPoolAllocator<ResultPackage> *pResultPackagePool,SyncQueue<Package*> *process_queue,SyncQueue<ResultPackage*> *result_queue,DataProcessor *pProcessor) 
			:ObjectManager<ResultPackage>(pResultPackagePool), m_process_queue(process_queue),m_result_queue(result_queue),m_pProcessor(pProcessor){}
		bool RunOnce();
	private:
		SyncQueue<Package*> *m_process_queue;
		SyncQueue<ResultPackage*> *m_result_queue;
		DataProcessor *m_pProcessor;
	};

}

#endif
