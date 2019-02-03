#ifndef AC_DATADECODER_H_
#define AC_DATADECODER_H_

#include <sys/types.h>
#include <ac/util/queue.h>
#include <ac/util/objectmanager.h>
#include <ac/util/objectpool.h>
#include "network/package.h"

namespace ac
{

	enum
	{
		PROTOCOLTYPE_TEXT = 1,
		PROTOCOLTYPE_BINARY,
	};
	enum
	{
		HEADER_LEN_2 = 2,
		HEADER_LEN_4 = 4,
		HEADER_LEN_6 = 6,
	};
	class ClientSocketBase;
	class BackClientSocketBase;
	class DataDecoderBase
	{
		public:
			DataDecoderBase(){}
			virtual ~DataDecoderBase(){}
			virtual int Process(ClientSocketBase *pClient) = 0;
			virtual int OnPackage(ClientSocketBase *pClient,const char* buf,size_t buflen) = 0;
	};
	class DataDecoder_helloworld : public DataDecoderBase//直接把接到数据放入发送缓冲区
	{
		public:
			virtual ~DataDecoder_helloworld(){}
			virtual int Process(ClientSocketBase *pClient);
			virtual int OnPackage(ClientSocketBase *pClient,const char* buf,size_t buflen);
	};

	class DataDecoder : public DataDecoderBase
	{
		public:
			DataDecoder(int pttype,size_t hdlen) : m_pttype(pttype),m_hdlen(hdlen) {}
			virtual ~DataDecoder(){}
			virtual int Process(ClientSocketBase *pClient);
			virtual int OnPackage(ClientSocketBase *pClient,const char* buf,size_t buflen) = 0;
			size_t GetBuflen(char *buf);
		protected:
			int m_pttype;
			size_t m_hdlen;
	};
	
	class DataDecoder_Q : public DataDecoder,public ObjectManager<Package>//queue
	{
		public:
			DataDecoder_Q(int pttype,size_t hdlen,SyncQueue<Package*> *queue,ObjectPoolAllocator<Package> *pPackagePool) 				: DataDecoder(pttype,hdlen),ObjectManager<Package>(pPackagePool),m_queue_process(queue),m_PackagePool(pPackagePool) {}
			virtual ~DataDecoder_Q(){}
			virtual int OnPackage(ClientSocketBase *pClient,const char* buf,size_t buflen);
		protected:
			SyncQueue<Package*> *m_queue_process;
			ObjectPoolAllocator<Package> *m_PackagePool;	
	};

	class DataDecoder_Q_echo : public DataDecoder_Q
	{
		public:
			DataDecoder_Q_echo(int pttype,size_t hdlen,SyncQueue<Package*> *queue,ObjectPoolAllocator<Package> *pPackagePool) 				
				: DataDecoder_Q(pttype,hdlen,queue,pPackagePool) {}
			virtual ~DataDecoder_Q_echo(){}
			virtual int Process(ClientSocketBase *pClient);
		
	};
}
#endif
