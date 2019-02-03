#include "TestNative.h"
#include <Windows.h>
#include <string>

/*
输入javap -s DiskInfo可以查看某个类的具体Signatures。
*************Table 3-2 Java VM Type Signatures*********************
Type Signature                Java Type
Z                                       boolean
B                                       byte
C                                       char
S                                       short
I                                        int
J                                        long
F                                       float
D                                      double
L fully-qualified-class ;   fully-qualified-class 完全限定的，比如类中的字符串，不是全局的
[ type                               type[] 数组
( arg-types ) ret-type      method type
****************************************************************
For example, the Java method:

long f (int n, String s, int[] arr); 
has the following type signature:

(ILjava/lang/String;[I)J 
*/

JNIEXPORT jint JNICALL Java_TestNative_add
(JNIEnv *env, jobject obj, jint a, jint b)
{
	jclass objClass = env->FindClass("TestNative");
	jmethodID mAddInJava = env->GetMethodID(objClass,"addInJava","(II)V");
	char buf[260] = {0};
	sprintf_s(buf,"mAddInJava = %x",(int)mAddInJava);
	OutputDebugStringA(buf);
	jint ja = 10;
	jint jb = 20;
	env->CallVoidMethod(obj,mAddInJava,ja,jb);

	return a+b;
}

JNIEXPORT jint JNICALL Java_TestNative_getName
(JNIEnv *env, jobject obj, jstring s)
{
	/* Obtain a C-copy of the Java string */
	const jchar *str = env->GetStringChars(s, 0);//通过这种方式来传递字符串，获取到的是Unicode编码的字符串。下面会有详细解说在android底下
	const wchar_t* pws = (const wchar_t*)str;
	std::wstring wstr = pws;
	/* process the string */
	OutputDebugString(wstr.c_str());

	env->ReleaseStringChars(s,str);
	return 0;
}
//设置数组
JNIEXPORT void JNICALL Java_TestNative_setArray
  (JNIEnv *env, jobject, jbooleanArray aBool)
{
	int length = env->GetArrayLength(aBool);
	char buf[260] = {0};
	sprintf_s(buf,"length = %d",length);
	OutputDebugStringA(buf);
	std::string str;
	jboolean* pB =  env->GetBooleanArrayElements(aBool,0);
	for(int i = 0;i<length;i++)
	{
		
		if(pB[i] == JNI_TRUE)//if(pB[i])
		{
			sprintf_s(buf,"true : %d;",pB[i]);
			str += buf;
		}
		else
			str += "false ;";
		
	}
	env->ReleaseBooleanArrayElements(aBool,pB,0);
	
	OutputDebugStringA(str.c_str());
	//ReleaseBooleanArrayElements(jbooleanArray array, jboolean *elems, jint mode)
	//@param mode
	//0 提交java heap并释放copy内存 
	//JNI_COMMIT提交java heap 不释放copy内存 
	//JNI_ABORT放弃当前更改 释放copy内存
}

//获取字符串数组 参数传递统一用unicode编码 
JNIEXPORT jobjectArray JNICALL Java_TestNative_getStringArray
  (JNIEnv *env, jobject obj)
{
	jobjectArray ret = 0;
	jstring     str;
	jsize        len = 5;
	wchar_t*      sa[] = { L"Hello,", L"world!", L"JNI", L"is", L"雯" };//这里测试了下 在android底下，对字符串的长度
	//判断会有问题，所以觉得对代码中的字符串统一使用UTF-8编写，这样不至于出乱码，使用 env->NewStringUTF(const char*); 
	//但凡遇到字符串就这么处理吧
	int           i=0;
	ret = (env)->NewObjectArray(len,env->FindClass("java/lang/String"),0);
	for( i=0; i < len; i++ )
	{
		int len = wcslen(sa[i]);
		//虽然这个NewString创建的对象会在返回java之后被清理，
		//但是由于Loacl Reference Table比较小，它是不会被释放的直到程序返回java
		//所以最好还是对NewString对象进行清理，除非它是返回值，参考资料http://www.ibm.com/developerworks/cn/java/j-lo-jnileak/index.html
		str = env->NewString((jchar*)sa[i],len);//生成string
		env->SetObjectArrayElement(ret, i, str);//推入数组
		env->DeleteLocalRef(str);
	}
	return ret;
}
//获取自定义结构
JNIEXPORT jobject JNICALL Java_TestNative_getStruct
  (JNIEnv *env, jobject obj)
{
	jobject  ret = 0;
	/* 下面为获取到Java中对应的实例类中的变量*/

	//获取Java中的实例类
	jclass objectClass = env->FindClass("DiskInfo");//typedef sturct DiskInfo{};
	//获取类中每一个变量的定义
	//名字
	jfieldID str = env->GetFieldID(objectClass,"name","Ljava/lang/String;");
	//序列号
	jfieldID ival = env->GetFieldID(objectClass,"serial","I");
	//查找父类成员
	jfieldID id = env->GetFieldID(objectClass,"id","I");
	
	char buf[260] = {0};
	sprintf(buf,"jfieldID id = %x, name = %x, serial = %x",id,str,ival);
	OutputDebugStringA(buf);

	const wchar_t* pStr = L"my name is D: 雯";
	jsize length = wcslen(pStr);
	const jchar* pJc = (const jchar*) pStr;
	//给每一个实例的变量赋值
	env->SetObjectField(obj,str,(env)->NewString(pJc,length));
	env->SetShortField(obj,ival,10);
	env->SetIntField(obj,id,6);

	return obj;
}
//获取自定义结构数组
JNIEXPORT jobjectArray JNICALL Java_TestNative_getStructArray
  (JNIEnv *env, jobject obj)
{
	jobjectArray aObj;
	//获取Java中的实例类
	jclass objectClass = env->FindClass("DiskInfo");
	//获取类中每一个变量的定义
	//名字
	jfieldID str = env->GetFieldID(objectClass,"name","Ljava/lang/String;");
	//序列号
	jfieldID ival = env->GetFieldID(objectClass,"serial","I");
	//申请自定义结构数组
	aObj = env->NewObjectArray(3,objectClass,0);

	for(int i=0;i<3;i++)
	{
		const wchar_t* pStr = L"my name is D: 雯";
		jsize length = wcslen(pStr);

		jobject tmp = env->AllocObject(objectClass);
		//给每一个实例的变量赋值
		env->SetObjectField(tmp,str,(env)->NewString((const jchar*) pStr,length));
		env->SetShortField(tmp,ival,20+i);

		env->SetObjectArrayElement(aObj, i, tmp);//推入数组
	}

	return aObj;
}
