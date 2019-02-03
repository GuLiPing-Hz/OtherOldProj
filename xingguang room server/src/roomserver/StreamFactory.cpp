#include "StreamFactory.h"

//char StreamFactory::bufR[1024]={0};
char StreamFactory::bufW[65535]={0};

//BinaryReadStream StreamFactory::sBinaryReadStream(bufR,sizeof(bufR));
BinaryWriteStream StreamFactory::sBinaryWriteStream(bufW,sizeof(bufW));

//BinaryReadStream* StreamFactory::InstanceReadStream()
//{
//	return &sBinaryReadStream;
//}

BinaryWriteStream* StreamFactory::InstanceWriteStream()
{
	//memset(bufW,0,sizeof(bufW));	
	sBinaryWriteStream.Clear();    
	return &sBinaryWriteStream;
}


//void StreamFactory::CleanReadChar()
//{
//	memset(bufR,0,sizeof(bufR));	
//	sBinaryReadStream.Clear();
//}
