1.����a��b��ֵ-��ʹ���м����
#include <iostream>
#include <string>
#include <sstream>

int main(void)
{
	int  a = 11, b = 2;
	a ^= b;
	b ^= a;
	a ^= b;

	std::cout << "a=" << a << ",b=" << b << std::endl;

	system("PAUSE");
	return 0;
}

2. C++�������ݣ��������ô��ݣ����������˻�Ϊָ��
���¶���͵õ�һ�����������
������ ��&��������[N]��
�ڽ��в����Ĵ���ʱ���������ÿ��԰������Ƿ�ֹ�����˻�Ϊָ�룬�����������ڱ���к���ע�⵽�����⡣
��������һ��ʵ����
#include <iostream>
void each(int int_ref[10])
{
      std::cout << sizeof(int_ref)<<std::endl;
      for(int i=0;i<10;i++)
            std::cout<<int_ref[i]<<" ";
       std::cout<<std::endl;
}

void each2(int (&int_ref)[10])
{
        std::cout<<sizeof(int_ref)<<std::endl;
        for(int i=0;i<10;i++)
               std::cout<<int_ref[i]<<" ";
       std::cout<<std::endl;
}
int main()
{
        int int_array[] = {1,2,3,4,5,6,7,8,9,10};
        each(int_array);//����1��sizeof()��ֵ��
         each2(int_array);//����2��sizeof()��ֵ��

         return 0;
}
��Ȼ�������ȥ���sizeof�����Ļ�������������������������κεĲ�ͬ�����Ƕ��ܹ���ȷ�����array[]�е�10��ֵ��
�������ǹ۲�һ��sizeof()��ֵ�ͻᷢ�ֺܴ�Ĳ�ͬ��
����1�����4
����2�����40
�������Ǿ��ܿ�������array[]��Ϊ�������ݹ�ȥ��������յĲ�����Ҳ��һ�����飬��ô���ͻ��˻�Ϊһ��ָ�룬Ҳ�������ǳ�˵�ġ��������һ��ָ�롱��
�����յĲ�����һ�����������ǣ��ͻᷢ�������Ǳ������Լ���ԭ��̬������������Ȼ��һ�����顱����ʱ���������þ�����һ�������Լ��˻�Ϊһ��ָ������á�

3. ��̬��ά����
c++��ʽ
����
int num = 3;
int** array = new int*[num];	// ����int*����������ڴ�ռ䣬��ֵ����int*��*���͵�array
for(int i=0; i<num; ++i) {
	array[i] = new int[num];	// �ֱ��int*���͵���������ռ䣬��һά����Ķ�̬���롣
}
�ͷ�
for(int i=0; i<num; i++) {
	delete[] array[i];
}
delete[] array;


һ������һ������ָ����ʵ��
//5��2�е�����
int **p = (int **)malloc(sizeof(int *) * 5);
for (int i = 0; i < 5; ++i)
{
	p[i] = (int *)malloc(sizeof(int) * 2);
}
for (int i = 0; i < 5; ++i)
{
	for (int j = 0; j < 2; ++j)
	{
		//�������ÿ��Ԫ�ص�ַ
		printf("%p\n", &p[i][j]);
	}
}
for (int i = 0; i < 5; ++i)
	free(p[i]);
free(p);
�ص㣺
ͬһ����Ԫ�ص�ַ�������ģ���ͬ����Ԫ�ص�ַ��һ���������ġ� 
�ͷ�����Ŀռ�Ĺ���Ҳ��Ҫע�⡣

������������ָ����ʵ��
//����һ��5��2�е���������
int(*p)[2] = (int(*)[2])malloc(sizeof(int) * 5 * 2);
for (int i = 0; i < 5; ++i)
{
	for (int j = 0; j < 2; ++j)
	{
		//�������ÿ��Ԫ�ص�ַ
		printf("%p\n", &p[i][j]);
	}
}
free(p);
return 0;
}
�ص㣺
����ĵ�ַ�ռ�ʼ���������ġ�
�ͷ�����ռ�ķ�ʽֵ��ע����бȽϡ� 

��������һά������ģ���ά����
int *p = (int *)malloc(sizeof(int) * 5 * 2);
for (int i = 0; i < 5; ++i)
{
	for (int j = 0; j < 2; ++j)
	{
		//�������ÿ��Ԫ�ص�ַ
		printf("%p\n", &p[i*2+j]);
	}
}
�ص㣺
����ĵ�ַ�������ġ�
�ͷ�������ռ�ķ�ʽֵ��ע�⡣