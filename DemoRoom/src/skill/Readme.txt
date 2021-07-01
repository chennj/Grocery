1.交换a和b的值-不使用中间变量
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

2. C++参数传递，数组引用传递，保护数组退化为指针
如下定义就得到一个数组的引用
类型名 （&变量明）[N]；
在进行参数的传递时，数组引用可以帮助我们防止数组退化为指针，而这是我们在编程中很难注意到的问题。
下面来看一个实例：
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
        each(int_array);//问题1：sizeof()的值？
         each2(int_array);//问题2：sizeof()的值？

         return 0;
}
当然，如果不去理会sizeof（）的话，这两个函数的输出不会有任何的不同，他们都能够正确的输出array[]中的10个值，
但当我们观察一下sizeof()的值就会发现很大的不同。
问题1：输出4
问题2：输出40
从这我们就能看出，当array[]作为参数传递过去后，如果接收的参数是也是一个数组，那么它就会退化为一个指针，也就是我们常说的“数组就是一个指针”。
当接收的参数是一个数组引用是，就会发现它还是保持了自己的原生态，即“数组仍然是一个数组”。这时，数组引用就起到了一个保护自己退化为一个指针的作用。

3. 动态多维数组
c++方式
分配
int num = 3;
int** array = new int*[num];	// 申请int*类型数组的内存空间，赋值给（int*）*类型的array
for(int i=0; i<num; ++i) {
	array[i] = new int[num];	// 分别给int*类型的数组申请空间，即一维数组的动态申请。
}
释放
for(int i=0; i<num; i++) {
	delete[] array[i];
}
delete[] array;


一、利用一个二级指针来实现
//5行2列的数组
int **p = (int **)malloc(sizeof(int *) * 5);
for (int i = 0; i < 5; ++i)
{
	p[i] = (int *)malloc(sizeof(int) * 2);
}
for (int i = 0; i < 5; ++i)
{
	for (int j = 0; j < 2; ++j)
	{
		//输出数组每个元素地址
		printf("%p\n", &p[i][j]);
	}
}
for (int i = 0; i < 5; ++i)
	free(p[i]);
free(p);
特点：
同一行中元素地址是连续的，不同行中元素地址不一定是连续的。 
释放申请的空间的过程也需要注意。

二、利用数组指针来实现
//申请一个5行2列的整型数组
int(*p)[2] = (int(*)[2])malloc(sizeof(int) * 5 * 2);
for (int i = 0; i < 5; ++i)
{
	for (int j = 0; j < 2; ++j)
	{
		//输出数组每个元素地址
		printf("%p\n", &p[i][j]);
	}
}
free(p);
return 0;
}
特点：
申请的地址空间始终是连续的。
释放申请空间的方式值得注意进行比较。 

三、利用一维数组来模拟二维数组
int *p = (int *)malloc(sizeof(int) * 5 * 2);
for (int i = 0; i < 5; ++i)
{
	for (int j = 0; j < 2; ++j)
	{
		//输出数组每个元素地址
		printf("%p\n", &p[i*2+j]);
	}
}
特点：
申请的地址是连续的。
释放所申请空间的方式值得注意。