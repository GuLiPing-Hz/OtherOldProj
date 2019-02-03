#ifndef AC_UTIL_DATA_BLOCK_H_
#define AC_UTIL_DATA_BLOCK_H_

#include <sys/types.h>

#include <ac/util/allocator.h>


namespace ac
{
enum {DEFAULT_BLOCK_SIZE = 40960/*1024*/,MAX_BLOCK_SIZE = 5242880/*5242880 5M*/};
class DataBlock
{
public:
	DataBlock(size_t size = DEFAULT_BLOCK_SIZE,size_t maxsize = MAX_BLOCK_SIZE,Allocator *pAllocator = Allocator::Instance())
		: m_pos(0),m_size(size),m_maxsize(maxsize),m_pAllocator(pAllocator)
	{
		m_buf = (char*)m_pAllocator->Allocate(m_size);	
	}
	virtual ~DataBlock()
	{
		m_pAllocator->Deallocate(m_buf);
	}
	int Append(const char *buf,size_t buflen)
	{
		return Copy(m_pos,buf,buflen);	
	}
	int Copy(size_t pos,const char *buf,size_t buflen);
	char* GetBuf() const {return m_buf;}
	size_t GetPos() const {return m_pos;}
	void InitPos() {m_pos = 0;}
	void SetMaxSize(size_t maxsize){m_maxsize=maxsize;}
private:
	char *m_buf;
	size_t m_pos;
	size_t m_size;
	size_t m_maxsize;
	Allocator *m_pAllocator;
};
}

#endif // AC_DATA_BLOCK_H_

