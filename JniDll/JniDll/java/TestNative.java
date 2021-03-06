//自定义结构，相当于 c语言中的struct
class Base{
	public int id;
}

class  DiskInfo extends Base
  {
   // 名字 
    public  String name;
   // 序列号 
    public   int  serial;
}

public class TestNative
{
	//提供c++回调
	public void addInJava(int a,int b){
    		int c = a+b;
    		System.out.println("java add result : "+c);
    }
	//JNIEXPORT jint JNICALL Java_TestNative_add(
	//JNIEnv *env,        /* interface pointer */
     //jobject obj,        /* "this" pointer */
     //jint i,             /* argument #1 */
     //jstring s)          /* argument #2 */
     //两数相加
	private native int add(int a,int b);
	
	//传递字符串
	private native int getName(String c);
	
	// 输入一个数组 
      public  native  void  setArray(boolean[] blList);

     // 返回一个字符串数组 
      public  native String[] getStringArray();

     // 返回一个结构 
      public  native DiskInfo getStruct();

     // 返回一个结构数组 
      public  native DiskInfo[] getStructArray();
	
	//调用dll库
	static
	{
		System.loadLibrary("JniDll");	
	}
	
	
	public static void main(String[] args)
	{
		TestNative hh = new TestNative();
		int r= hh.add(30,20);
		hh.getName("xiao雯");
		boolean aB[] = { true,false,true,true,false};
		hh.setArray(aB);
		System.out.println("result="+r);
		
		String rS[] = hh.getStringArray();
		
		for(int i=0;i<rS.length;i++)
		{
			System.out.println(rS[i]);
		}
		
		DiskInfo di = hh.getStruct();
		
		System.out.println(di.name+":"+di.serial+";"+di.id);
		System.out.println("*********************************************");
		DiskInfo aDi[] = hh.getStructArray();
		System.out.println(aDi.length);
		for(int i=0;i<aDi.length;i++)
		{
			DiskInfo tmp = aDi[i];
			System.out.println(tmp.name+":"+tmp.serial);
		}
	}
}