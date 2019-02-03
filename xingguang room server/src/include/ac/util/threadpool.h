#ifndef AC_UTIL_THREADPOOL_H_
#define AC_UTIL_THREADPOOL_H_

#include <ac/util/non_copyable.h>
#include <ac/util/thread.h>
#include <pthread.h>
#include <list>

namespace ac {

class JobThread;

// NOTICE: ThreadPool
class ThreadPool  : public NonCopyable
{
protected:
	typedef std::list<Thread*>	ThreadPtrContainer;
	
	ThreadPtrContainer	livethreads;
	ThreadPtrContainer	deadthreads;
	
	pthread_mutex_t		mutex;

	volatile size_t			stopcount;
	
public:

	ThreadPool();

	virtual ~ThreadPool();

	int Start(size_t count);

	void Stop(size_t count);

	void StopAll();

	size_t GetThreadNum();

protected:

	virtual bool RunOnce();
	
	virtual void StopThreads(size_t stopcount);


protected:
		
	Thread* CreateThread();

	void DestroyThread(Thread* thread);

	friend class JobThread;
};


} // namespace ac

#endif // AC_UTIL_THREADPOOL_H_

