

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


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


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __iaudioswitch_h__
#define __iaudioswitch_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IOS_AudioSwitch_FWD_DEFINED__
#define __IOS_AudioSwitch_FWD_DEFINED__
typedef interface IOS_AudioSwitch IOS_AudioSwitch;
#endif 	/* __IOS_AudioSwitch_FWD_DEFINED__ */


#ifndef __IOS_ChangePitch_FWD_DEFINED__
#define __IOS_ChangePitch_FWD_DEFINED__
typedef interface IOS_ChangePitch IOS_ChangePitch;
#endif 	/* __IOS_ChangePitch_FWD_DEFINED__ */


#ifndef __AudioSwitch_FWD_DEFINED__
#define __AudioSwitch_FWD_DEFINED__

#ifdef __cplusplus
typedef class AudioSwitch AudioSwitch;
#else
typedef struct AudioSwitch AudioSwitch;
#endif /* __cplusplus */

#endif 	/* __AudioSwitch_FWD_DEFINED__ */


#ifndef __IOS_Saturation_FWD_DEFINED__
#define __IOS_Saturation_FWD_DEFINED__
typedef interface IOS_Saturation IOS_Saturation;
#endif 	/* __IOS_Saturation_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

/* interface __MIDL_itf_audioswitch_0000 */
/* [local] */ 

typedef 
enum _eSoundTrack
    {	Stereo	= 0,
	MonoL	= Stereo + 1,
	MonoR	= MonoL + 1
    } 	eSoundTrack;

typedef LONGLONG REFERENCE_TIME;



extern RPC_IF_HANDLE __MIDL_itf_audioswitch_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_audioswitch_0000_v0_0_s_ifspec;

#ifndef __IOS_AudioSwitch_INTERFACE_DEFINED__
#define __IOS_AudioSwitch_INTERFACE_DEFINED__

/* interface IOS_AudioSwitch */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IOS_AudioSwitch;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("01454652-6904-4ffa-B00C-46538B6CA23A")
    IOS_AudioSwitch : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SwitchATrack( 
            /* [in] */ BOOL bFirstAudio) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSTrackCopy( 
            /* [in] */ BOOL bCopy) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSTrackCopy( 
            /* [out] */ BOOL *bCopy) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SwitchSTrack( 
            /* [in] */ eSoundTrack eType) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IOS_AudioSwitchVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOS_AudioSwitch * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOS_AudioSwitch * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOS_AudioSwitch * This);
        
        HRESULT ( STDMETHODCALLTYPE *SwitchATrack )( 
            IOS_AudioSwitch * This,
            /* [in] */ BOOL bFirstAudio);
        
        HRESULT ( STDMETHODCALLTYPE *SetSTrackCopy )( 
            IOS_AudioSwitch * This,
            /* [in] */ BOOL bCopy);
        
        HRESULT ( STDMETHODCALLTYPE *GetSTrackCopy )( 
            IOS_AudioSwitch * This,
            /* [out] */ BOOL *bCopy);
        
        HRESULT ( STDMETHODCALLTYPE *SwitchSTrack )( 
            IOS_AudioSwitch * This,
            /* [in] */ eSoundTrack eType);
        
        END_INTERFACE
    } IOS_AudioSwitchVtbl;

    interface IOS_AudioSwitch
    {
        CONST_VTBL struct IOS_AudioSwitchVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOS_AudioSwitch_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOS_AudioSwitch_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOS_AudioSwitch_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOS_AudioSwitch_SwitchATrack(This,bFirstAudio)	\
    (This)->lpVtbl -> SwitchATrack(This,bFirstAudio)

#define IOS_AudioSwitch_SetSTrackCopy(This,bCopy)	\
    (This)->lpVtbl -> SetSTrackCopy(This,bCopy)

#define IOS_AudioSwitch_GetSTrackCopy(This,bCopy)	\
    (This)->lpVtbl -> GetSTrackCopy(This,bCopy)

#define IOS_AudioSwitch_SwitchSTrack(This,eType)	\
    (This)->lpVtbl -> SwitchSTrack(This,eType)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IOS_AudioSwitch_SwitchATrack_Proxy( 
    IOS_AudioSwitch * This,
    /* [in] */ BOOL bFirstAudio);


void __RPC_STUB IOS_AudioSwitch_SwitchATrack_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOS_AudioSwitch_SetSTrackCopy_Proxy( 
    IOS_AudioSwitch * This,
    /* [in] */ BOOL bCopy);


void __RPC_STUB IOS_AudioSwitch_SetSTrackCopy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOS_AudioSwitch_GetSTrackCopy_Proxy( 
    IOS_AudioSwitch * This,
    /* [out] */ BOOL *bCopy);


void __RPC_STUB IOS_AudioSwitch_GetSTrackCopy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOS_AudioSwitch_SwitchSTrack_Proxy( 
    IOS_AudioSwitch * This,
    /* [in] */ eSoundTrack eType);


void __RPC_STUB IOS_AudioSwitch_SwitchSTrack_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IOS_AudioSwitch_INTERFACE_DEFINED__ */


#ifndef __IOS_ChangePitch_INTERFACE_DEFINED__
#define __IOS_ChangePitch_INTERFACE_DEFINED__

/* interface IOS_ChangePitch */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IOS_ChangePitch;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DBD365F1-1AAE-4d5e-BF9F-AAE12BFF3AEE")
    IOS_ChangePitch : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ChangeCurPitch( 
            /* [in] */ INT nPitch) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IOS_ChangePitchVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOS_ChangePitch * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOS_ChangePitch * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOS_ChangePitch * This);
        
        HRESULT ( STDMETHODCALLTYPE *ChangeCurPitch )( 
            IOS_ChangePitch * This,
            /* [in] */ INT nPitch);
        
        END_INTERFACE
    } IOS_ChangePitchVtbl;

    interface IOS_ChangePitch
    {
        CONST_VTBL struct IOS_ChangePitchVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOS_ChangePitch_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOS_ChangePitch_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOS_ChangePitch_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOS_ChangePitch_ChangeCurPitch(This,nPitch)	\
    (This)->lpVtbl -> ChangeCurPitch(This,nPitch)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IOS_ChangePitch_ChangeCurPitch_Proxy( 
    IOS_ChangePitch * This,
    /* [in] */ INT nPitch);


void __RPC_STUB IOS_ChangePitch_ChangeCurPitch_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IOS_ChangePitch_INTERFACE_DEFINED__ */


#ifndef __IOS_Saturation_INTERFACE_DEFINED__
#define __IOS_Saturation_INTERFACE_DEFINED__

/* interface IOS_Saturation */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IOS_Saturation;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("523DF7C3-E094-43fc-90FB-82765BBE6954")
    IOS_Saturation : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetSaturation( 
            long *plSat) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSaturation( 
            long plSat) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IOS_SaturationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOS_Saturation * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOS_Saturation * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOS_Saturation * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetSaturation )( 
            IOS_Saturation * This,
            long *plSat);
        
        HRESULT ( STDMETHODCALLTYPE *SetSaturation )( 
            IOS_Saturation * This,
            long plSat);
        
        END_INTERFACE
    } IOS_SaturationVtbl;

    interface IOS_Saturation
    {
        CONST_VTBL struct IOS_SaturationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOS_Saturation_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOS_Saturation_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOS_Saturation_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOS_Saturation_GetSaturation(This,plSat)	\
    (This)->lpVtbl -> GetSaturation(This,plSat)

#define IOS_Saturation_SetSaturation(This,plSat)	\
    (This)->lpVtbl -> SetSaturation(This,plSat)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IOS_Saturation_GetSaturation_Proxy( 
    IOS_Saturation * This,
    long *plSat);


void __RPC_STUB IOS_Saturation_GetSaturation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOS_Saturation_SetSaturation_Proxy( 
    IOS_Saturation * This,
    long plSat);


void __RPC_STUB IOS_Saturation_SetSaturation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IOS_Saturation_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


