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
	SINGER_TYPE_HKTW_MAN=0,		//��̨�и���
	SINGER_TYPE_HKTW_WOMAN,		//��̨Ů����
	SINGER_TYPE_CHN_MAN,				//��½�и���
	SINGER_TYPE_CHN_WOMAN,		//��½Ů����
	SINGER_TYPE_CHN_GROUP,			//��½���
	SINGER_TYPE_FOREIGN_STAR,		//�������
	SINGER_TYPE_FOREIGN_GROUP,	//������
};

//��������
enum MUSIC_TYPE
{
	MUSIC_TYPE_ALL=-1,
	MUSIC_TYPE_DISCO=0,		//DISCO����
	MUSIC_TYPE_CHILD,			//��ͯ����
	MUSIC_TYPE_LOVER,			//���¶Գ�
	MUSIC_TYPE_RED,				//������������裩
	MUSIC_TYPE_ROCK,			//ҡ������
	MUSIC_TYPE_DANCE,		//����
	MUSIC_TYPE_HAPPY,		//ϲ�����
	MUSIC_TYPE_DRAMA,		//Ϸ��
	MUSIC_TYPE_CHORUS,		//Ⱥ�Ǻϳ�
};
//������������
enum LANG_TYPE
{
	LANG_TYPE_ALL=-1,
	LANG_TYPE_CN=0,	//���Ĺ���
	LANG_TYPE_CT,			//����
	LANG_TYPE_TN,		//������
	LANG_TYPE_EN,		//Ӣ��
	LANG_TYPE_JP,			//����
	LANG_TYPE_KR,			//����
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
	char m_music_name[100];			//������
	char m_music_filename[200];	//�����ļ���
	char m_music_singer[50];			//������
	int m_music_type;						//��������
	char m_music_namepy[50];		//������ƴ����д
	char m_music_singerpy[50];		//������ƴ����д
	int m_music_singertype;		//�������
	int m_music_langtype;			//�����������
	int m_music_namecount;		//����������
	int m_music_voicevalue;		//��������ֵ
	int m_music_lrbc;					//��������
	int m_music_musicpos;			//����ƫ��λ��
	char m_music_timemark[100];	//������ʱ���
	int m_music_lighteffect;			//�����ƹ�Ч��
	int m_music_subtype;				//�Ƽ�����
	int m_music_hot;						//���Ÿ���
	int m_music_new;						//�¸��Ƽ�
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
	};
	~CSingerInfo(){};
public :
	char	m_singername[100];		//������
	int	m_songnum;					//�������еĸ�����
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
	void SetSearchConditionName(char* value);
	//������������--������ƴ����д����������ʱ��Ч��
	void SetSearchConditionNamePY(char* value);
	//������������--������
	void SetSearchConditionSinger(char* value);
	//������������--������ƴ����д����������ʱ��Ч��
	void SetSearchConditionSingerPY(char* value);
	//������������--�������ͣ���������ʱ��Ч��
	void SetSearchConditionType(int value);
	//������������--�������ͣ���������ʱ��Ч��
	void SetSearchConditionSingerType(int value);
	//������������--��������������������ʱ��Ч��
	void SetSearchConditionNameCount(int value);
	//������������--�����������ͣ���������ʱ��Ч��
	void SetSearchConditionLangType(int value);
	//������������--�Ƽ���������������ʱ��Ч��
	void SetSearchConditionSubType(int value=1);
	//������������--���Ÿ�������������ʱ��Ч��
	void SetSearchConditionHot(int value=1);
	//������������--�¸��Ƽ�����������ʱ��Ч��
	void SetSearchConditionNew(int value=1);
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
private:

	void CreateSearchMusicConditionStr(std::string& _condition);

	void CreateSearchSingerConditionStr(std::string& _condition);
private:
	char		m_musicname[100];			//������
	char		m_musicsinger[50];			//������
	int		m_musictype;					//��������
	char		m_musicnamepy[50];		//������ƴ����д
	char		m_musicsingerpy[50];		//������ƴ����д
	int		m_musicsingertype;			//�������
	int		m_musiclangtype;				//�����������
	int		m_musicnamecount;			//����������
	int		m_musicsubtype;				//�Ƽ�����
	int		m_musichot;						//���Ÿ���
	int		m_musicnew;					//�¸��Ƽ�

	//sqlite3xx::connection*   m_sqliteconn;	//sqlite���Ӷ���

	std::string m_dbname;		//�������ݿ���

	bool m_trace;					//�Ƿ���������־
	//result* m_presult;				//SQLiteִ�н��
};
};
#endif