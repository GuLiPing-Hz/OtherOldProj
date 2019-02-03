#ifndef AC_UTIL_CIRCLE_QUEUE_4_TYPE_H_
#define AC_UTIL_CIRCLE_QUEUE_4_TYPE_H_

#include <sys/types.h>

#include <ac/util/queue.h>
#include <ac/util/circle_queue.h>
#include <ac/util/allocator.h>
#include <ac/util/mutex.h>


namespace ac {

template <class T>
class CircleQueue4Type : public Queue<T>
{
public:

	enum { DEFAULT_LIMIT = 1000 };
	
public:

	CircleQueue4Type(size_t limit = DEFAULT_LIMIT, const Mutex * writer_mutex=NullMutex::Instance(), const Mutex * reader_mutex=NullMutex::Instance(), Allocator * pAllocator=Allocator::Instance())
		: cqueue_(sizeof(T)*limit, writer_mutex, reader_mutex, pAllocator)
	{		
	}

	virtual size_t GetSize() const
	{
		return cqueue_.GetSize() / (sizeof(T) + cqueue_.GetQueueBlockSize());
	}


protected:	

	virtual int PushImp(const T& value)
	{
		return cqueue_.Push(&value, sizeof(value), false);
	}

	virtual int PopImp(T& value, bool block)
	{
		return cqueue_.Pop(&value, sizeof(value), block) < 0 ? -1 : 0;
	}
	
protected:

	CircleQueue cqueue_;
};

} // namespace ac


#endif // AC_UTIL_CIRCLE_QUEUE_4_TYPE_H_

