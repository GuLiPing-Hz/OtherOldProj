#ifndef AC_UTIL_QUEUE_H_
#define AC_UTIL_QUEUE_H_

#include <pthread.h>
#include <list>
#include <ac/util/notify.h>


namespace ac {


template<class T>
class Queue
{
public:
	Queue() : notify(0) {}

	virtual ~Queue() {}

	void SetNotification(Notification* notify) { this->notify = notify; }

	Notification* GetNotification() { return this->notify; }

	int Push(const T& value)
	{
		int ret = PushImp(value);
		if ( ret == 0 && GetNotification() ) {
			GetNotification()->notify();
		}
		return ret;
	}

	int Pop(T& value, bool block=false)
	{
		return PopImp(value, block);
	}
	
	virtual size_t GetSize() const = 0;	

protected:

	virtual int PushImp(const T& value) = 0;
	
	virtual int PopImp(T& value, bool block) = 0;

private:
	Notification* notify;
};


template<class T>
class SyncQueue : public Queue<T>
{
public:
	enum { DEFAULT_LIMIT = 1000 };

public:
	SyncQueue(size_t limit = DEFAULT_LIMIT) 
		: limit_(limit)
	{
		pthread_mutex_init(&mutex, NULL);
		pthread_cond_init(&cond, NULL);
	}

	virtual ~SyncQueue()
	{
		pthread_cond_destroy(&cond);
		pthread_mutex_destroy(&mutex);
	}

	virtual size_t GetSize() const
	{
		pthread_mutex_lock(&mutex);
		size_t size = queue.size();
		pthread_mutex_unlock(&mutex);
		return size;
	}
	
protected:

	virtual int PushImp(const T& value)
	{
		pthread_mutex_lock(&mutex);

		if ( queue.size() > limit_ ) {
			pthread_mutex_unlock(&mutex);			
			return -1;
		}
			
		queue.push_back(value);

		//pthread_cond_broadcast(&cond);
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&mutex);

		return 0;
	}

	virtual int PopImp(T& value, bool block)
	{
		pthread_mutex_lock(&mutex);
		while(true)
		{
			if (!queue.empty())
			{
				value = queue.front();
				queue.pop_front();
				pthread_mutex_unlock(&mutex);
				return 0;
			}

			if (!block)
			{
				pthread_mutex_unlock(&mutex);
				return -1;
			}

			pthread_cond_wait(&cond, &mutex);
		}
	}

private:

	mutable pthread_mutex_t mutex;
	mutable pthread_cond_t  cond;

	std::list<T> queue;
	size_t	limit_;	
	
};

} // namespace ac


#endif // AC_UTIL_QUEUE_H_

