/********************************************************************

purpose:	
*********************************************************************/

#pragma once

#if !defined BYTE
#define BYTE unsigned char
#endif
#if !defined UINT32
#define UINT32 unsigned int
#endif
#if !defined UINT
#define UINT unsigned int
#endif

#if !defined TRUE
#define TRUE true
#endif

#if !defined FALSE
#define FALSE false
#endif

#define TEAFLAG 'T'
#define XTEAFLAG 'X'
#define COMPRESSFLAG 'C'

namespace ac
{
	class CXTEA
	{
		public:
			static int Encrypt(char* pbyInBuffer, int nInBufferLength, char* pbyOutBuffer, int nOutBufferLength, char arrbyKey[16]);
			static int Decrypt(char* pbyInBuffer, int nInBufferLength, char* pbyOutBuffer, int nOutBufferLength, char arrbyKey[16]);
	};

    static BYTE DEF_KEY[16] = {'A','3','4','F','9','7','7','1','E','C','1','0','B','C','A','5'};

    /// TEA����
    /*
    @param	pInBuffer IN			Ҫ���ܵ�buffer
    @param	nInBufferLen IN			Ҫ���ܵ�buffer�ĳ���
    @param	pOutBuffer IN			��ż��ܽ����buffer
    @param	pnOutBufferLen IN OUT	��ż��ܽ����buffer�ĳ���(IN)�����ؼ��ܽ����ʵ�ʴ�С(OUT)
    @param	pKey IN					���ܵ�key,16byte
    @param	uRounds IN				TEA��������������Խ��ȫ��Խ�ߣ��������ٶ�Խ��
    @remark	���ܽ����ȼ���ǰ�ĳ��ȴ�1-8��byte,���Խ��� len(��ż��ܽ����buffer) = len(Ҫ���ܵ�buffer) + 8
    */
    bool TEAEncrypt(BYTE* pInBuffer, int nInBufferLen, BYTE* pOutBuffer, int* pnOutBufferLen, BYTE pKey[16] = DEF_KEY, UINT uRounds = 16);

    /// TEA����
    /*
    @param	pInBuffer		IN Ҫ���ܵ�buffer
    @param	nInBufferLen	IN Ҫ���ܵ�buffer�ĳ���
    @param	pOutBuffer		IN ��Ž��ܽ����buffer
    @param	pnOutBufferLen	IN OUT ��Ž��ܽ����buffer�ĳ���(IN)�����ؽ��ܽ����ʵ�ʴ�С(OUT)
    @param	pKey			IN ���ܵ�key,16byte,����ͼ���ʱ����ͬ
    @param	uRounds			IN ����,����ͼ���ʱ����ͬ
    @remark	��Ž��ܽ����buffer�ĳ��� ����Ҫ >= (Ҫ���ܵ�buffer�ĳ��� - 1)
    */
    bool TEADecrypt(BYTE* pInBuffer, int nInBufferLen, BYTE* pOutBuffer, int* pnOutBufferLen, BYTE pKey[16] = DEF_KEY, UINT uRounds = 16);
    int  TEAEncryptLen(int nInBufferLen);
    int TEADecryptLen(int nInBufferLen);

	bool StreamDecrypt(const char *inbuf,int inbuflen,char *outbuf,int &outbuflen,char key[16],int type = 1);

	bool StreamEncrypt(const char *inbuf,int inbuflen,char *outbuf,int &outbuflen,char key[16],int type = 1);

	bool StreamCompress(const char *inbuf,int inbuflen,char *outbuf,int &outbuflen);

	bool StreamUnCompress(const char *inbuf,int inbuflen,char *outbuf,int &outbuflen);

}
