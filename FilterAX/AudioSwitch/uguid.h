

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 6.00.0366 */
/* at Tue Aug 06 14:52:16 2013
 */
/* Compiler settings for .\audioswitch.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, IID_IOS_AudioSwitch,0x01454652,0x6904,0x4ffa,0xB0,0x0C,0x46,0x53,0x8B,0x6C,0xA2,0x3A);


MIDL_DEFINE_GUID(IID, IID_IOS_ChangePitch,0xDBD365F1,0x1AAE,0x4d5e,0xBF,0x9F,0xAA,0xE1,0x2B,0xFF,0x3A,0xEE);


MIDL_DEFINE_GUID(CLSID, CLSID_AudioSwitch,0xD86F02C9,0x952A,0x4f15,0x9E,0x40,0xB2,0x8A,0x48,0x9C,0xD6,0x35);


MIDL_DEFINE_GUID(IID, IID_IOS_Saturation,0x523DF7C3,0xE094,0x43fc,0x90,0xFB,0x82,0x76,0x5B,0xBE,0x69,0x54);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



