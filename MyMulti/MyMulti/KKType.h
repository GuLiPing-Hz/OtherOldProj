//
//  KKType.h
//  Karaoke
//
//  Created by 2012080303Imac on 12-8-23.
//  Copyright (c) 2012�� 9158. All rights reserved.
//
#ifndef __KKTYPE_H__
#define __KKTYPE_H__

#include "Opengl.h"
#include <string>

#ifdef __cplusplus
extern "C" {
#endif
	const float TIP_IMAGE_CX = 0;//39;
/*	const float TIP_IMAGE_Y1 = 0;*/
/*	const float TIP_IMAGE_Y2 = 42;*/

	const float MOVE_IMAGE_SPACE = 100;
	const float MOVE_IMAGE_Y1 = 3;
	const float MOVE_IMAGE_Y2 = 36;

	//glp
//	const float STAFF_HEIGHT = 400;//���׸߶�
//	const float SING_START_B = 200/1280.0f;//����ݳ���

	//-----------------------------------------------------------------------------
	typedef struct _KKColor
	{
		ulong c;

		_KKColor()
		{
			c=0xFFFFFFFF;
		}

		/*_KKColor( CGFloat _r, CGFloat _g, CGFloat _b, CGFloat _a )
		{
			r = _r;
			g = _g;
			b = _b;
			a = _a;
		}*/
		_KKColor(const ulong uargb)//add by glp
		{
			this->c = uargb;
		}
		_KKColor& operator=(const _KKColor a)
		{
			if (&a != this)
			{
				this->c = a.c;
			}
			return *this;
		}
		_KKColor(uchar r,uchar g,uchar b,uchar a)
		{
			this->c = ((a<<24)+(r<<16)+(g<<8)+b);
		}
		_KKColor(float r,float g,float b,float a)
		{
			this->c = ((int(a*255))<<24)+((int(r*255))<<16)+((int(g*255))<<8)+int(b*255);
		}
	}KKColor;

	//-----------------------------------------------------------------------------
	typedef struct _KKColorRect
	{
		KKColor left_top_color;
		KKColor left_bottom_color;
		KKColor right_top_color;
		KKColor right_bottom_color;

		_KKColorRect( ulong ltc, ulong lbc, ulong rtc, ulong rbc )
		{
			left_top_color.c = ltc;

			left_bottom_color.c = lbc;

			right_top_color.c = rtc;

			right_bottom_color.c = rbc;
		}

		_KKColorRect( const KKColor & c )
		{
			left_top_color = c;
			left_bottom_color = c;
			right_top_color = c;
			right_bottom_color = c;
		}
	}KKColorRect;

	static KKColorRect WHITE_RECT( 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF );

	//-----------------------------------------------------------------------------
	// �任����
	enum _eTansType
	{
		TRANS_NONE  = 0x00,
		TRANS_ALPHA = 0x01,
		TRANS_SCALE = 0x02,
		TRANS_ANGLE = 0x04
	};

	enum _eRank
	{
		RANK_NONE = 0,                              // ��
		RANK_RED,										 // ���
		RANK_YELLOW,                               // �ƽ�
		RANK_GREEN                                  // �̽�
	};

	enum _eQuadSplitMode
	{
		TopLeftToBottomRight = 0,
		BottomLeftToTopRight = 1
	};

	// �����ֵȼ�
	enum GLSentenceLevel
	{
		GLSENTENCEGRADELEVEL_NONE = 0,
		GLSENTENCEGRADELEVEL_GOOD,
		GLSENTENCEGRADELEVEL_NICE,
		GLSENTENCEGRADELEVEL_COOL,
		GLSENTENCEGRADELEVEL_PERFECT,
		GLSENTENCEGRADELEVEL_GODLIKE,
		GLSENTENCEGRADELEVEL_UNBELIEVABLE
	};

	// ʵʱ������Ϣ
	typedef struct _GLRealtimeGrade
	{
		unsigned int		cur_tm;						// ��ǰʱ��
		float						cur_pitch;					// ��ǰ�ݳ��߳�������
		int						sentence_index;				// ����ţ���һ�����Ϊ0��
		float						realtime_score;				// ��ǰ���ʵʱ�÷�
		float						accumulate_score;			// ��ǰ����ۼӵ÷�
		double					cur_db;
	}GLRealtimeGrade;

	// ��������Ϣ
	typedef struct _GLSentenceGrade
	{
		bool							sentence_switch;
		int							sentence_index;				// ����ţ���һ�����Ϊ0��
		float							sentence_score;				// ��÷�
		GLSentenceLevel	sentence_level;				// �����ֵȼ�
		float							combo_value;				// comboֵ
	}GLSentenceGrade;
	//-----------------------------------------------------------------------------
	// �����Ϣ
	struct _tGuiLyricInfo
	{
		std::wstring	lyric;
		int				width;
		int				height;
		float				pos;
		float				y;//���λ�ø߶�
		float				npass;//��ǰ������ʵİٷֱ� 0-1,��ʼΪ0
		_tGuiLyricInfo& operator=(const _tGuiLyricInfo& a)
		{
			if (this != &a)
			{
				this->lyric = a.lyric;
				this->width = a.width;
				this->pos = a.pos;
				this->y = a.y;
				this->npass = a.npass;
			}
			return *this;
		}
	};

	struct _tGuiParagraphInfo//������Ϣ
	{
		int             sentence_index;
		int             duration;
	};
	//-----------------------------------------------------------------------------
	// ����λ����Ϣ
	struct _tGuiWaveInfo
	{
		float l, r, y;		// �� �� y��ȫ������λ��
		unsigned int end_tm;
	};

	//-----------------------------------------------------------------------------
	//�����ַ��صĵ�
	typedef struct _RealTimePoint
	{
		float  pos;//���������Ķ�Ӧʱ��,�������λ��
		float   y;//��������yλ��
	}RealTimePoint;
	typedef struct _RealTimeDrawPoint//
	{
		float x;
		float y;
	}RealTimeDrawPoint;

	// �����Ϣ
	struct _tGuiCursorInfo
	{
		float 	x;
		float 	y;
		bool b;											// �Ƿ����ڱ�׼����Yλ�õ�ĳ��������
		_eRank r;

		_tGuiCursorInfo() : x( 0.0f ), y( 0.0f ), b( false ), r( RANK_NONE ) {}

		_tGuiCursorInfo( float _x, float _y, bool _b, _eRank _r )
			: x( _x ), y( _y ), b( _b ), r( _r ) {}

		_tGuiCursorInfo& operator = (_tGuiCursorInfo tmp)
		{
			if (this != &tmp)
			{
				this->x = tmp.x;
				this->y = tmp.y;
				this->b = tmp.b;
				this->r = tmp.r;
			}
			return *this;
		}
	};
#ifdef __cplusplus
}
#endif

#endif//__KKTYPE_H__