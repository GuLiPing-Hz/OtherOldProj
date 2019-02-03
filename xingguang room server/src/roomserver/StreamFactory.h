#pragma once
#include "ac/util/protocolstream.h"
 using namespace ac;

class StreamFactory
{
public:
	StreamFactory(void){}
	~StreamFactory(void){}

	//static char bufR[1024];
	static char bufW[65535];
	
	//static BinaryReadStream sBinaryReadStream;
	static BinaryWriteStream sBinaryWriteStream;
	
	//static BinaryReadStream* InstanceReadStream();
	static BinaryWriteStream* InstanceWriteStream();
	//static void CleanReadChar();

	
};
