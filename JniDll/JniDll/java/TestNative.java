//�Զ���ṹ���൱�� c�����е�struct
class Base{
	public int id;
}

class  DiskInfo extends Base
  {
   // ���� 
    public  String name;
   // ���к� 
    public   int  serial;
}

public class TestNative
{
	//�ṩc++�ص�
	public void addInJava(int a,int b){
    		int c = a+b;
    		System.out.println("java add result : "+c);
    }
	//JNIEXPORT jint JNICALL Java_TestNative_add(
	//JNIEnv *env,        /* interface pointer */
     //jobject obj,        /* "this" pointer */
     //jint i,             /* argument #1 */
     //jstring s)          /* argument #2 */
     //�������
	private native int add(int a,int b);
	
	//�����ַ���
	private native int getName(String c);
	
	// ����һ������ 
      public  native  void  setArray(boolean[] blList);

     // ����һ���ַ������� 
      public  native String[] getStringArray();

     // ����һ���ṹ 
      public  native DiskInfo getStruct();

     // ����һ���ṹ���� 
      public  native DiskInfo[] getStructArray();
	
	//����dll��
	static
	{
		System.loadLibrary("JniDll");	
	}
	
	
	public static void main(String[] args)
	{
		TestNative hh = new TestNative();
		int r= hh.add(30,20);
		hh.getName("xiao��");
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