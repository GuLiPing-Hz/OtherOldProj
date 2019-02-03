#ifndef AC_UTIL_THREAD_H_
#define AC_UTIL_THREAD_H_

#include <pthread.h>
#include <ac/util/non_copyable.h>

namespace ac {

class Thread : public NonCopyable
{
public:
	Thread();
	virtual ~Thread();
	int Start();
	void Stop();
	void Join();

	inline bool IsAlive() const { return alive; }
	inline pthread_t GetThreadID() const { return hdl; }

private:
	virtual void Run() = 0;	

private:
	pthread_t hdl;
	bool alive;

	friend void* __THREAD_FUNC(void* p);
};


} // namespace ac

#endif // AC_UTIL_THREAD_H_


