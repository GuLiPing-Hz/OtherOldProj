#ifndef AC_DATAPROCESSOR_H_
#define AC_DATAPROCESSOR_H_

#include "ac/util/non_copyable.h"
#include "ac/log/log.h"

namespace ac
{

	class DataProcessor : public NonCopyable
	{
		public:
			virtual ~DataProcessor(){}
			virtual int Process(const char *inbuf,size_t inbuflen,char *outbuf,size_t &outbuflen) = 0;
	};

	class DataProcessor_Echo : public DataProcessor
	{
		public:
			virtual ~DataProcessor_Echo(){}
			virtual int Process(const char *inbuf,size_t inbuflen,char *outbuf,
size_t &outbuflen){
				AC_DEBUG("data arrived");
				size_t outlen = outbuflen;
				size_t len = 0;
				len += inbuflen;
				if(len < outlen)
				{
					memcpy(outbuf,inbuf,inbuflen);
					outbuflen = inbuflen;
					return 0;
				}
				return -1;
			}
	};

}

#endif
