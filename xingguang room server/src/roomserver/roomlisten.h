#ifndef ROOMLISTEN_H_
#define ROOMLISTEN_H_

#include <string.h>
#include "roomclient.h"
#include "network/listensocket.h"
#include "ac/util/objectmanager.h"

using namespace ac;

class RoomListen : public ListenSocketBase2,public ObjectManager<RoomClient>
{
public:
	RoomListen(int port,Reactor *pReactor,ObjectAllocator<RoomClient> * pObjectAllocator,Counter *pCounter,DataDecoderBase *pDecoder,ClientMap *pClientMap,int timeout = 3) :
	  ListenSocketBase2(port,pReactor,pCounter,pDecoder,pClientMap,timeout),
		  ObjectManager<RoomClient>(pObjectAllocator)
	  {}
	  virtual RoomClient* CreateClient()
	  {
		RoomClient* pclient = Create();
		//pclient->GetRB()->SetMaxSize(40960);//40K 
		//pclient->GetWB()->SetMaxSize(40960);//40K 
		
        if(pclient == NULL)
        {
            AC_ERROR("pclient == NULL");
            return NULL;
        }

		pclient->initAll();
		return pclient;
	  }
	  virtual ~RoomListen(){}
};

#endif // ROOMLISTEN_H_



