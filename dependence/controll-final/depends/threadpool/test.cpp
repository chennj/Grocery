#include "crc_thread_pool.h"
#include "crc_log.h"
#include <stdio.h>
#include <iostream>

class Test
{
public:
	int test(int i) {
		//std::cout << _name << ", i = " << i << std::endl;
		CRCLog_Info("%s, count<%d>", _name.c_str(), i);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000*15));
		return i;
	}
	void setName(std::string name) {
		_name = name;
	}
	std::string _name;
};

void func0()
{
	std::cout << "func0 is running..." << std::endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void func1(int a)
{
	std::cout << "func1 is running... a=" << a << std::endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void func2(int a, std::string b)
{
	std::cout << "func2 is running... a=" << a << ",b=" << b << std::endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

int func1_future(int a)
{
	std::cout << "func1 is running... a=" << a << std::endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	return a;
}

std::string func2_future(int a, std::string b)
{
	std::cout << "func2() is running... a=" << a << ",b=" << b << std::endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	return b;
}

//简单测试
void test1()
{
	//设置运行日志名称
	CRCLog::Instance().setLogPath("ThreadPoolLog", "w", false);

	CRCThreadPool threadpool;
	threadpool.init(1);
	threadpool.start();								//启动线程池

	//要执行的任务
	threadpool.exec(1000, func0);
	threadpool.exec(func1, 10);
	threadpool.exec(func2, 20, "dofen");

	//等待任务全部结束
	threadpool.waitForAllDone();
	threadpool.stop();
}

//测试异步返回值
void test2()
{
	CRCThreadPool threadpool;
	threadpool.init(4);
	threadpool.start();								//启动线程池

	//要执行的任务
	std::future<decltype(func1_future(0))>	result1 = threadpool.exec(func1_future, 10);
	std::future<std::string>				result2 = threadpool.exec(func2_future, 20, "dofen");

	//等待任务全部结束
	threadpool.waitForAllDone();
	CRCLog_Info("result1=%d",result1.get());
	CRCLog_Info("result2=%s",result2.get().c_str());

	threadpool.stop();
}

//测试对象函数的绑定
void test3()
{
	CRCThreadPool threadpool;
	threadpool.init(1);
	threadpool.start();								//启动线程池

	//要执行的任务
	Test t1, t2;
	t1.setName("test1");
	t2.setName("test2");

	auto f1 = threadpool.exec(std::bind(&Test::test, &t1, std::placeholders::_1), 10);
	auto f2 = threadpool.exec(std::bind(&Test::test, &t2, std::placeholders::_1), 20);

	//等待任务全部结束
	threadpool.waitForAllDone();
	CRCLog_Info("f1=%d",f1.get());
	CRCLog_Info("f2=%d",f2.get());

	threadpool.stop();
}

void test4()
{
	//设置运行日志名称
	CRCLog::Instance().setLogPath("ThreadPoolLog", "w", false);

	CRCThreadPool threadpool;
	threadpool.init(4);
	threadpool.start();								//启动线程池

	Test t;
	t.setName("test");
	for (int i=0; i<13; i++){
		threadpool.exec(std::bind(&Test::test, &t, std::placeholders::_1), i);
	}
	threadpool.waitForAllDone();
	threadpool.stop();
}

int main()
{
	/*
	CRCLog_Info("异步线程池测试：普通函数    -------------------");
	test1();
	CRCLog_Info("异步线程池测试：异步返回结果 -------------------");
	test2();
	CRCLog_Info("异步线程池测试：象函数的绑定 -------------------");
	test3();
	*/
	CRCLog_Info("异步线程池测试：多任务 -------------------");
	test4();
}