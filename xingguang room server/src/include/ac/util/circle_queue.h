#ifndef AC_UTIL_CIRCLE_QUEUE_H_
#define AC_UTIL_CIRCLE_QUEUE_H_

#include <sys/types.h>
#include <ac/util/mutex.h>
#include <ac/util/allocator.h>
#include <ac/util/non_copyable.h>

namespace ac {


class CircleQueue : public NonCopyable
{
public:

	CircleQueue(size_t queue_size,
		const Mutex * writer_mutex=NullMutex::Instance(), 
		const Mutex * reader_mutex=NullMutex::Instance(), 
		Allocator * allocator=Allocator::Instance());

	~CircleQueue();
	
	inline bool IsInit() const { return queue_; }

	int Push(const void * data, size_t len, bool block=true);
	
	ssize_t Pop(void * data, size_t len, bool block=true);

	size_t GetSize() const;


public:

	inline static size_t RoundupMem(size_t mem_size, size_t  mem_align=sizeof(size_t))
		{ return (mem_size+mem_align-1) & ~(mem_align - 1); }

	inline static size_t GetQueueBlockSize()
		{ return sizeof(QueueBlock); }
	
	
private:

	inline bool IsEmpty() const  { return queue_->wr_ptr==queue_->rd_ptr; }

	bool IsFull(size_t wr_len) const;

	int AtomicPush(const void * data, size_t len, bool block);

	ssize_t AtomicPop(void * data, size_t len, bool block);

private:

	typedef struct _QUEUE_Block {
		size_t len;
		char data[0];
	} QueueBlock;
		
	typedef struct _Queue {
		size_t block_size;
		volatile size_t wr_ptr;
		volatile size_t rd_ptr;
		char first_block_addr[0];
	} CQueue;

	Allocator *		allocator_;
	CQueue *		queue_;

	const Mutex * 	writer_mutex_;
	const Mutex *	reader_mutex_;
	
};

} // namespace ac

#endif // AC_UTIL_CIRCLE_QUEUE_H_
