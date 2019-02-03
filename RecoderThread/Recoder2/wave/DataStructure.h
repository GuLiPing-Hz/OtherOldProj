/*
	注释时间:2014-4-25
	author: glp
	录音结构数据
*/
#ifndef __WZ_DATASTRUCTURE_H__
#define __WZ_DATASTRUCTURE_H__

//#include "..\Evaluate\MSEvalDef.h"
#include <string>
//缺省的参数配置
const std::string DFLT_KTV_SERVER_IP = "127.0.0.1";
const int DFLT_KTV_SERVER_PORT = 6000;
const int DFLT_ROOM_SERVER_PORT = 2727;

//放大模式
const int NO_MAGNIFY_MODE = 0;
const int NORMAL_MAGNIFY_MODE= 1;
const int ADVANCED_MAGNIFY_MODE = 2;

//分数显示形式
const int HUNDRED_MODE = 0;
const int THOUSAND_MODE = 1;
const int STAR_MODE = 2;

//切歌的处理方式
const int CUT_NEXT_SONG = 0;
const int CUT_SHOW_HINT = 1;
const int CUT_SHOW_RESULT = 2;

//话筒声音过小的处理
const int NOVOICE_SHOW_HINT = 0;
const int NOVOICE_SHOW_RESULT = 1;

//级别
const int LEVEL_MAX_NUM = 6;

//--------------------------------------------------
//状态相关的数据结构和常量
//--------------------------------------------------
//client自身的状态
const int STATE_CLIENT_LISTENING = 0;
const int STATE_CLIENT_NO_LISTEN = 1;


//client与vod之间的状态
const int STATE_CLIENT_CV_CONNECT = 0;
const int STATE_CLIENT_CV_DISCONNECT = 1;
const int STATE_CLIENT_CV_SING_START = 2;
const int STATE_CLIENT_CV_SING_STOP = 3;
const int STATE_CLIENT_CV_SING_CUT = 4;
const int STATE_CLIENT_CV_SING_PAUSE = 5;
const int STATE_CLIENT_CV_SING_CONTINUE = 6;
const int STATE_CLIENT_CV_SING_RESTART = 7;

//client与server之间的状态
const int STATE_CLIENT_CS_CONNECT = 0;
const int STATE_CLIENT_CS_DISCONNECT = 1;

struct TWZ_StateClient
{
	int client; //client self
	int cv; //client - vod
	int cs; //client - server
};

//--------------------------------------------------
//歌曲基本信息和歌曲评判信息相关的数据结构
//--------------------------------------------------


//放大模式
struct TMagnifyMode
{
	int score;
	int rank;
};

//显示在屏幕上的分数
struct TSongScreenScore
{
	double difficulty;//难度系数[0,10]
	double brilliance;//精彩指数
	double charm; //魅力指数
	double FinalScore;//最后得分
};

//可能显示在屏幕上的级别
struct TSongScreenLevel
{
	int difficulty;
	int brilliance;
	int charm;
	int FinalScore;
};

//显示参数
//百分制，千分制，五星级等
//切歌，声音过小
struct TDisplayPara
{
	int final_score;
	int difficulty;
	int brilliance;
	int charm;
	int cut2next;
	int no_voice;
	int level_score[LEVEL_MAX_NUM];
};

//------------------------------
//歌曲的基本信息
struct TSongBaseInfo
{
	
	std::string track; //--bh 编号(歌曲号)
	std::string title; //--gm 歌名
	std::string artist; //--gx 歌星
	std::string language; //--yz 语种(如：国语，粤语，英语，韩语，日语)
	std::string style; //--qz 曲种(风格: 通俗，民族，戏剧)
	std::string version; //--bb 版本(如：MTV，演唱会，故事情节，风景，人物)
	
	std::string filename; //--wjm 歌曲对应的文件名
	std::string filefmt; //--wjgs 文件格式(如：mpg1, MPG4-1, mpg)
	std::string coderate; //--ml 码率(如：1.5, 2.5)
	
	//伴奏和原唱的音轨：
	//对mpg1或mpg且码率为1.5的是：左或者右；
	//对MPG2或mpg且码率为2.5以上的是：1, 2, 3；
	std::string accompany; //--bc 歌曲伴奏音轨
	std::string original;  //--yc 歌曲原唱音轨
};
//------------------------------

/*
//歌曲的基本信息（该结构既用于存储，也用于传送）
struct TSongBaseInfo
{
	std::string	title;//标题
	std::string	album;//专辑
	std::string	artist;//歌手
	std::string	year;//年代
	std::string	composer;//作者
	std::string	origartist;//原创
	std::string	style;//风格
	std::string	track;//歌曲号
	std::string	encodedby;//编码
	std::string	copyright;//版权
	std::string	url;//网址
	std::string	comment;//备注
};
*/

//评判库使用：
//歌曲的总得分信息（分值和时间）
// struct TSongScoreItem
// {
// 	int value;
// 	COleDateTime datetime;
// 	std::string location;
// 	std::string singer;
// };

//歌曲的总得分信息（包括id）（用于传送也用于存储）
// struct TSongEvalInfo
// {
// 	std::string id;
// 	TSongScoreItem score;
// };


#endif //__WZ_DATASTRUCTURE_H__