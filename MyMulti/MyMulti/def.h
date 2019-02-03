#ifndef GLDEF__H__
#define GLDEF__H__

#include <objbase.h>
//#include "progress/processcommunicate.h"
//glp

static char LOGPATH[260] = {0}; //日志path

typedef struct _StructSound{
	int cur_mic;
	int min_mic;
	int max_mic;
	int cur_music;
	int min_music;
	int max_music;
	int cur_tone;
	int min_tone;
	int max_tone;
}StructSound;

enum eVideoType
{
	vtHost,
	vtSing,
};

enum eShowWave
{
	SHOW_NONE,
	SHOW_ONE,
	SHOW_TWO,
	SHOW_THREE,
};

enum eNotify
{
	NOTIFY_NONE=-1,
	//NOTIFY_MESSAGE
	NOTIFY_MUTE_OPEN,//Notify0
	NOTIFY_MUTE_CLOSE,//Notify1
	NOTIFY_PLAY_STOPPING,//Notify2
	NOTIFY_PLAY_RUNNING,//Notify3
	NOTIFY_ORIGIN_OPEN,//Notify4
	NOTIFY_ORIGIN_CLOSE,//Notify5
	NOTIFY_SINGLE_GRADE,//Notify6
	NOTIFY_DOUBLE_GRADE,//Notify7
	NOTIFY_CLOSE_GRADE,//Notify8

	NOTIFY_APPLAUD,//png序列 applaud
	NOTIFY_CHEER,//png序列 cheer

	NOTIFY_MICVOLUME_SET,//组合图
	NOTIFY_MUSICVOLUME_SET,//组合图
	NOTIFY_TONE_SET,//组合图
	//HEAD_MESSAGE
	MESSAGE_NOTIFY,
	MESSAGE_NEXTSONG,
};

#define SECONDWAVEHEIGHT 280

typedef struct _CGPoint
{
	float x;
	float y;
	float z;
	_CGPoint& operator+=(const _CGPoint& a)
	{
		this->x += a.x;
		this->y += a.y;
		this->z += a.z;
		return *this;
	}
	_CGPoint& operator=(const _CGPoint& a)
	{
		this->x = a.x;
		this->y = a.y;
		this->z = a.z;
		return *this;
	}
	bool operator==(const _CGPoint& a)
	{
		if ((this->x-a.x<0.01)&&(this->y-a.y)<0.01&&(this->z-a.z)<0.01)
		{
			return true;
		}
		return false;
	}
	_CGPoint(const float a=0.0f,const float b=0.0f,const float c=0.0f)
	{
		this->x = a;
		this->y = b;
		this->z = c;
	}
	_CGPoint(const _CGPoint& point)
	{
		this->x = point.x;
		this->y = point.y;
		this->z = point.z;
	}
}CGPoint;

typedef struct _CGSize 
{
	int width;
	int height;
	_CGSize& operator+=(const _CGSize& a)
	{
		this->width += a.width;
		this->height += a.height;
		return *this;
	}
	_CGSize& operator=(const _CGSize& a)
	{
		if ( this == &a )
			return *this;
		this->width = a.width;
		this->height = a.height;
		return *this;
	}
	bool operator==(const _CGSize& a)
	{
		if (this->width==a.width&&this->height==a.height)
		{
			return true;
		}
		return false;
	}
	_CGSize(const int w=0,const int h=0)
	{
		this->width = w;
		this->height = h;
	}
	_CGSize(const _CGSize& size)
	{
		this->width = size.width;
		this->height = size.height;
	}
}CGSize;

static CGSize CGSIZEZERO(0,0);

typedef struct _CGRect
{
	CGPoint origin;
	CGSize size;
	_CGRect& operator=(const _CGRect& a)
	{
		this->origin = a.origin;
		this->size = a.size;
		return *this;
	}
	bool operator==(const _CGRect& a)
	{
		if (this->origin==a.origin&&this->size==a.size)
		{
			return true;
		}
		return false;
	}
	_CGRect(const CGPoint& a=CGPoint(0.0f,0.0f),const CGSize& b=CGSize(0,0))
	{
		this->origin = a;
		this->size = b;
	}
	_CGRect(const float x,const float y,const int w,const int h)
	{
		this->origin = CGPoint(x,y);
		this->size = CGSize(w,h);
	}
	_CGRect(const _CGRect& rect)
	{
		this->origin = rect.origin;
		this->size = rect.size;
	}
}CGRect;

typedef struct _LINEVERTEX
{
	CGPoint point;
	unsigned long colour;
}LINEVERTEX;


typedef struct _CUSTOMVERTEX
{
	CGPoint point;
	unsigned long colour;
	float u;
	float v;
}CUSTOMVERTEX;

typedef struct _hgeInputEvent 
{
	int		type;			// event type
	int		key;			// key code
	int		flags;			// event flags
	int		chr;			// character code
	int		wheel;			// wheel shift
	float		x;				// mouse cursor x-coordinate
	float		y;				// mouse cursor y-coordinate
}hgeInputEvent;

typedef struct _CInputEventList
{
	hgeInputEvent		event;
	_CInputEventList*	next;
}CInputEventList;

typedef unsigned char uchar;
typedef float CGFloat;
typedef unsigned int  uint;
typedef unsigned long ulong;

static CGRect CGRECTZERO(0.0f,0.0f,0,0);

#define D3DFVF_CUSTOMVERTEX ( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 )
#define D3DFVF_LINEVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)

#define BATCH_BUFFER_SIZE 10000
#define BATCH_LINE_BUFFERSIZE 2000
#define WM_PLAYER_MESSAGE (WM_USER+200)
#define WM_GRAPHNOTIFY				(WM_PLAYER_MESSAGE+1)
#define WM_SWITCHSONG					(WM_PLAYER_MESSAGE+2)
#define WM_PLAY_CONTROL				(WM_PLAYER_MESSAGE+3)
#define WM_PLAY_GRADE					(WM_PLAYER_MESSAGE+4)
#define WM_PLAY_ACOMMPANY		(WM_PLAYER_MESSAGE+5)
#define WM_PLAY_END					    (WM_PLAYER_MESSAGE+6)
#define WM_DEBUG							(WM_PLAYER_MESSAGE+7)
#define WM_PLAY_SONGINFO			(WM_PLAYER_MESSAGE+8)
#define WM_PLAY_SINGERPIC			(WM_PLAYER_MESSAGE+9)
#define WM_PLAY_SONGSTARTEND	(WM_PLAYER_MESSAGE+10)

#define WM_PLAY_VOLUME_PUT		(WM_PLAYER_MESSAGE+11)
#define WM_PLAY_VOLUME_GET		(WM_PLAYER_MESSAGE+12)
//#define MSE_PITCHTRACK_ALGTYPE_YinACF	2

#define FAIL_RET(x) do { if( FAILED( hr = ( x  ) ) ) \
	return hr; } while(0)
#define FAIL_RET2(x) do { if( FAILED( hr = ( x  ) ) ) \
	return false; } while(0)
#define FAIL_RET3(x) do { if( FAILED( hr = ( x  ) ) ) \
	return; } while(0)
#define FAIL_GOTOFAILED(x) do { if( FAILED( hr = ( x  ) ) ) \
	goto failed; } while(0)

#ifndef FAILDE_RETURNNEGATIVE
#define FAILDE_RETURNNEGATIVE(exp) \
if(FAILED(exp))\
{\
	return -1;\
}
#endif

#endif//GLDEF__H__

