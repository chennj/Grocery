#include "common/Base.h"

#include "algorithm/DP/Solution.h"
#include <Windows.h>

// wchar_t to string
void Wchar_tToString(std::string& szDst, wchar_t *wchar)
{
	wchar_t * wText = wchar;
	DWORD dwNum = WideCharToMultiByte(CP_OEMCP, NULL, wText, -1, NULL, 0, NULL, FALSE);// WideCharToMultiByte的运用
	char *psText;  // psText为char*的临时数组，作为赋值给std::string的中间变量
	psText = new char[dwNum];
	WideCharToMultiByte(CP_OEMCP, NULL, wText, -1, psText, dwNum, NULL, FALSE);// WideCharToMultiByte的再次运用
	szDst = psText;// std::string赋值
	delete[]psText;// psText的清除
}

// string to wstring
void StringToWstring(std::wstring& szDst, std::string str)
{
	std::string temp = str;
	int len = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)temp.c_str(), -1, NULL, 0);
	wchar_t * wszUtf8 = new wchar_t[len + 1];
	memset(wszUtf8, 0, len * 2 + 2);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)temp.c_str(), -1, (LPWSTR)wszUtf8, len);
	szDst = wszUtf8;
	std::wstring r = wszUtf8;
	delete[] wszUtf8;
}

int main()
{
#if 0 //算法实验
	Scope<DP::Fibonacci> fib = CreateScope< DP::Fibonacci>();
	int result = fib->fib_Optimal(10);
	std::cout << "fib 10 = " << result << std::endl;

	Scope<DP::Coins> coins = CreateScope< DP::Coins>();
	int c[] = { 1,2,5 };
	result = coins->CoinChange_DownTop(c, sizeof(c)/sizeof(int), 11);
	std::cout << "coins 11 = " << result << std::endl;

	Scope<DP::EditDistance> distance = CreateScope< DP::EditDistance>(std::string("intention"), std::string("execution"));
	result = distance->MinDistance_DownTop();
	std::cout << "min distance = " << result << std::endl;
#endif

	typedef int(*DLL_FUNC_1)(const WCHAR* s, BOOL quietErrors, void* fpv);

	wchar_t path[] = L"D:\\vs2015\\Grocery\\bin\\Debug-windows-x86_64\\CommonDll\\CommonDll.dll";
	//wchar_t path[] = L"D:\\vs2015\\WechatHookDll\\x64\\Debug\\EasyHook.dll";
	//必须同一目录，否则绝对路径
	//wchar_t path[] = L"CommonDll.dll";
	HMODULE hmod = LoadLibraryEx(path, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
	if (!hmod) {
		std::cout << "加载DLL失败，退出。" << std::endl;
		return 0;
	}
	//std::cout << "基地址：" << static_cast <const  void  *>(hmod) << std::endl;

	DLL_FUNC_1 func_1 = (DLL_FUNC_1)::GetProcAddress(hmod, "exprot_test_str");
	if (!func_1) {
		std::cout << "没有exprot_test_str这个函数。" << std::endl;
		FreeLibrary(hmod);
		return 0;
	}
	//std::cout << "func_1地址：" << func_1 << ";" << (DWORD)func_1 << std::endl;

	std::string szSrc = "";
	std::wstring wszDest;

	std::cout << "输入一个字符串。(输入之前先使用Injector.exe进行注入)" << std::endl;
	std::cin >> szSrc;
	StringToWstring(wszDest, szSrc);
	int c = func_1(wszDest.c_str(), false, nullptr);
	std::cout << "结果：" << c << std::endl;

	std::string cmd("");
	std::string exit("exit");
	std::cout << "输入 exit 退出。" << std::endl;
	while (cmd.compare(exit)!=0) {
		std::cin >> cmd;
		std::cout << "输入的是：" << cmd << std::endl;
	}

	std::cout << "已退出。" << std::endl;
	return 0;
}