#pragma once
#ifndef _H_MUSICSQLOBJECT
#define _H_MUSICSQLOBJECT


#include <list>

using namespace std;
namespace XGSQLITE{

#ifdef XGSQLITE_EXPORTS
#	define XGSQLITE_API		__declspec( dllexport )
#else
#	define XGSQLITE_API		__declspec( dllimport )
#endif

#define XGSQLITE_SUCCESSD	0
#define XGSQLITE_FAILED			-1

//��������
enum SINGER_TYPE
{
	SINGER_TYPE_ALL=-1,					//���и���
};

//������������
enum LANG_TYPE
{
	LANG_TYPE_ALL=-1,
};

/*****************************
*������Ϣ��
*****************************/
class XGSQLITE_API CMusicInfo
{
public:
	CMusicInfo(void);
	~CMusicInfo(void);

	void ClearInfo();
public:
	int m_music_id;							//����id
	char m_music_name[300];			//������
	char m_music_filename[500];	//�����ļ���
	char m_music_singer[300];			//������
	char m_music_typeinfo[20];		//��������
	char m_music_namepy[50];		//������ƴ����д
	char m_music_singerpy[50];		//������ƴ����д
	int m_music_singertype;		//�������
	int m_music_langtype;			//�����������
	int m_music_namecount;		//����������
	char m_music_topinfo[20];			//���а���Ϣ
	int m_music_voicevalue;		//��������ֵ
	int m_music_lrbc;					//��������
	char m_music_musicpos[20];			//���׸߶�
	char m_music_timemark[100];	//������ʱ���
	int m_music_lighteffect;			//�����ƹ�Ч��
	int m_music_recommend;				//�Ƽ�����
	int m_music_pop;						//���Ÿ���
	int m_music_new;						//����
	int m_music_isScore;					//�Ƿ������
	int m_music_isPlay;				//Ĭ�ϲ��Ÿ���
	int m_music_isChorus;			//1�Ǻϳ���0���Ǻϳ�
};

/*****************************
*������Ϣ��
*****************************/
class XGSQLITE_API CSingerInfo
{
public :
	CSingerInfo()
	{
		memset(m_singername,0,sizeof(m_singername));
		m_songnum=0;
		m_singertype=0;
	};
	~CSingerInfo(){};
public :
	char	m_singername[100];		//������
	int	m_songnum;					//�������еĸ�����
	int	m_singertype;				//��������
} ;


/*****************************
*SQLite���ݿ������
*****************************/
class XGSQLITE_API CMusicSQLObject 
{
public:
	/*
	**����˵����dbname �������ݿ��ļ���
	**					trace	    �Ƿ��ڿ���̨�����������Ϣ��Ĭ��Ϊ�����
	*/
	CMusicSQLObject(std::string dbname,bool trace=false);
	~CMusicSQLObject(void);

	/*
	**�����������ݿ⣬������ݿ��Ѵ��ڣ���ɾ�����ؽ�
	**����ֵ���ɹ���XGSQLITE_SUCCESSD
	**			 ��ʧ�ܣ�XGSQLITE_FAILED ���Ƿ����ݿ�����
	*/
	int CreateDataBase();
public:
	/*
	**��ҳ�������
	**����˵����	pageindex	[in] ҳ������ȡ�ڼ�ҳ���ݣ���1��ʼ
	**					pagesize		[in] ҳ��С ��ÿҳ��ʾ��������
	**					list				[out] ������Ϣ�б�ȡ�������Ϣ������ͷ�CMusicInfo����
	**												����������ڴ�й©
	**����ֵ�� �ɹ��������������ҳ��  
	**				ʧ�ܣ�XGSQLITE_FAILED
	*/
	int GetMusicInfo(int pageindex,int pagesize,list<CMusicInfo*> &_list);

	/*
	**��������Ϣ���浽SQLite�������ݿ�
	**����˵����	pMusicInfo	[in] ������Ϣ����
	**����ֵ�� �ɹ���XGSQLITE_SUCCESSD  
	**				ʧ�ܣ�XGSQLITE_FAILED
	*/
	int PutMusicInfo(list<CMusicInfo*> &list);
	/*
	**�����������
	**����˵������
	**����ֵ�� ��  
	*/
	void ClearSearchCondition();
	//������������--����������������ʱ��Ч��
	void SetSearchConditionName(const char* value);
	//������������--������ƴ����д����������ʱ��Ч��
	void SetSearchConditionNamePY(const char* value);
	//������������--������
	void SetSearchConditionSinger(const char* value);
	//������������--������ƴ����д����������ʱ��Ч��
	void SetSearchConditionSingerPY(const char* value);
	//������������--�������ͣ���������ʱ��Ч��
	void SetSearchConditionSongType(int value);
	//������������--��������
	void SetSearchConditionSingerType(int value);
	//������������--��������������������ʱ��Ч��
	void SetSearchConditionNameCount(int value);
	//������������--�����������ͣ���������ʱ��Ч��
	void SetSearchConditionLangType(int value);
	//������������--�Ƿ�����֣���������ʱ��Ч��
	void SetSearchConditionIsScore(int value=1);
	//������������--���֡���������valueȡֵ�����ñ�
	void SetSearchConditionTopType(int value);
	//������������--����
	void SetSearchConditionNew(int value=1);
	//������������--Ĭ�ϲ��Ÿ���
	void SetSearchConditionIsPlay(int value=1);
	/*
	**������������������
	**����˵����	��
	**����ֵ�� �ɹ�������ʵ�ʸ�����  
	**				ʧ�ܣ�XGSQLITE_FAILED
	*/
	int  GetMusicCount();
	/*
	**�����������ֵ�����
	**����˵����	��
	**����ֵ�� �ɹ�������ʵ�ʸ�����  
	**				ʧ�ܣ�XGSQLITE_FAILED
	*/
	int GetSingerCount();
	/*
	**��ҳȡ�������ĸ�����Ϣ
	**����˵����	pageindex	[in] ҳ������ȡ�ڼ�ҳ���ݣ���1��ʼ
	**					pagesize		[in] ҳ��С ��ÿҳ��ʾ��������
	**					list				[out] ������Ϣ�б�ȡ�������Ϣ������ͷ�CSingerInfo����
	**												����������ڴ�й©
	**����ֵ�� �ɹ��������������ҳ��  
	**				ʧ�ܣ�XGSQLITE_FAILED
	*/
	int GetSingerList(int pageindex,int pagesize,list<CSingerInfo*> &_list);
	/*
	**��ҳȡ�������ĸ�����Ϣ
	**����˵����	_musicid		[in]    ����ID
	**					_musicinfo	[out]  ���ظ�������
	**����ֵ��	�ɹ���XGSQLITE_SUCCESS
	**				ʧ�ܣ�XGSQLITE_FAILED
	*/
	int GetMusicInfoById(int _musicid,CMusicInfo  &_musicinfo);

	/*
	**���ݸ���IDȡ�ö�Ӧ�ĸ�����Ϣ
	**����˵����	_musicid		[in]    ����ID
	**					_singerinfo	[out]  ���ظ�������
	**����ֵ��	�ɹ���XGSQLITE_SUCCESS
	**				ʧ�ܣ�XGSQLITE_FAILED
	*/
	int GetSingerByMusicID(int _musicid,CSingerInfo &_singerinfo);

	//int GetMusicInfoBySingerCount();
	/*
	**���ݸ�������ȷ���Ҹ�����Ϣ
	**����˵����	_musicid		[in]    ����ID
	**					_singerinfo	[out]  ���ظ�������
	**����ֵ��	�ɹ���XGSQLITE_SUCCESS
	**				ʧ�ܣ�XGSQLITE_FAILED
	*/
	//int GetMusicInfoBySinger(int _pageindex,int _pagesize,list<CMusicInfo*> &_list );

private:

	void CreateSearchMusicConditionStr(std::string& _condition);

	//_ByCount ��Ҫ����ȡcount������ʱ����Ϊ1
	void CreateSearchSingerConditionStr(std::string& _condition,int _ByCount=0);
	
	//��������������ͼ
	int CreateSearchView(std::string _searcher);

private:
	char		m_musicname[100];			//������
	char		m_musicsinger[50];			//������
	char		m_musictypeinfo[20];		//������Ϣ����
	char		m_musicnamepy[50];		//������ƴ����д
	char		m_musicsingerpy[50];		//������ƴ����д
	int		m_musicsingertype;			//�������
	int		m_musiclangtype;				//�����������
	int		m_musicnamecount;			//����������
	//int		m_musicsubtype;				//�Ƽ�����
	//int		m_musichot;						//���Ÿ���
	int		m_musicnew;					//����
	int		m_musicisscore;				//�Ƿ������
	char		m_musictopinfo[20];			//�������а���Ϣ
	int		m_musicplay;					//Ĭ�ϲ��Ÿ���

	//sqlite3xx::connection*   m_sqliteconn;	//sqlite���Ӷ���

	std::string m_dbname;		//�������ݿ���

	bool m_trace;					//�Ƿ���������־
	//result* m_presult;				//SQLiteִ�н��

	bool m_CreateView;
};
};
#endif