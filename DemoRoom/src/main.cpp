#include "common/Base.h"

#include "algorithm/DP/Solution.h"
#include <Windows.h>

#include <ifnpub.h>

//display 16hex
void print16(const char* data, int num)
{
	int i;
	for (i = 0; i < num; i++) {
		printf("%02x ", *data++);
	}
	printf("\n");
}

// crc16
unsigned short crc16(const char* data, int num)
{
	int i;
	unsigned short crc = 0;
	for (i = 0; i < num; i++) {
		crc += (unsigned short)(*data++);
	}
	return crc;
}

// wchar_t to string
void Wchar_tToString(std::string& szDst, wchar_t *wchar)
{
	wchar_t * wText = wchar;
	DWORD dwNum = WideCharToMultiByte(CP_OEMCP, NULL, wText, -1, NULL, 0, NULL, FALSE);// WideCharToMultiByte������
	char *psText;  // psTextΪchar*����ʱ���飬��Ϊ��ֵ��std::string���м����
	psText = new char[dwNum];
	WideCharToMultiByte(CP_OEMCP, NULL, wText, -1, psText, dwNum, NULL, FALSE);// WideCharToMultiByte���ٴ�����
	szDst = psText;// std::string��ֵ
	delete[]psText;// psText�����
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

bool IsSame(unsigned int *array,int size)
{
	//��ǰ������������������Ƚϣ�һ��������ͬ��������true
	for (int i = 0; i < size; i++)
		for (int j = i + 1; j < size; j++)
			if (array[i] == array[j]) return true;
	return false;
}

bool IsContain(unsigned int se, unsigned int * array, int size)
{
	for (int i = 0; i < size; i++) {
		if (array[i] == se) {
			return true;
		}
	}
	return false;
}

void reverse_int(unsigned int v, unsigned int* ret)
{
	int i = 0;
	while (v)
	{
		//printf("%d", v % 10);//�����λ��
		ret[i++] = v % 10;
		v /= 10; //����һλ�����ƶ�����λ��
	}
}

int main()
{
#if 0 //�㷨ʵ��
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

#if 0 DLLʵ��
	typedef BOOL(*DLL_FUNC_1)(const MCHAR* s, BOOL quietErrors, FPValue* fpv);

	//wchar_t path[] = L"D:\\vs2015\\Grocery\\bin\\Debug-windows-x86_64\\CommonDll\\CommonDll.dll";
	//wchar_t path[] = L"D:\\vs2015\\WechatHookDll\\x64\\Debug\\EasyHook.dll";
	//wchar_t path[] = L"D:\\д�⹫˾\\3ds Max 2014 SDK\\lib\\x64\\Release\\";
	//wchar_t path[] = L"D:\\д�⹫˾\\3ds Max 2014 SDK\\lib\\x64\\Release\\Maxscrpt.dll";
	//����ͬһĿ¼���������·��
	wchar_t path[] = L"../CommonDll/CommonDll.dll";
	//HMODULE hmod = LoadLibraryEx(path, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
	HMODULE hmod = LoadLibraryW(path);
	if (!hmod) {
		std::cout << "����DLLʧ�ܣ��˳���" << std::endl;
		return 0;
	}
	//std::cout << "����ַ��" << static_cast <const  void  *>(hmod) << std::endl;

	DLL_FUNC_1 func_1 = (DLL_FUNC_1)::GetProcAddress(hmod, "ExecuteMAXScriptScript");
	if (!func_1) {
		std::cout << "û��exprot_test_str���������" << std::endl;
		FreeLibrary(hmod);
		system("PAUSE");
		return 0;
	}
	//std::cout << "func_1��ַ��" << func_1 << ";" << (DWORD)func_1 << std::endl;

	std::string szSrc = "";
	std::wstring wszDest;

	std::cout << "����һ���ַ�����(����֮ǰ��ʹ��Injector.exe����ע��)" << std::endl;
	std::cin >> szSrc;
	StringToWstring(wszDest, szSrc);
	int c = func_1(wszDest.c_str(), false, nullptr);
	std::cout << "�����" << c << std::endl;

	std::string cmd("");
	std::string exit("exit");
	std::cout << "���� exit �˳���" << std::endl;
	while (cmd.compare(exit)!=0) {
		std::cin >> cmd;
		std::cout << "������ǣ�" << cmd << std::endl;
	}

	std::cout << "���˳���" << std::endl;
	return 0;
#endif

#if 0 crc
	char order[15];
	memset(order, 0, sizeof(order));

	order[0]	= 0x48;
	order[1]	= 0x3a;
	order[2]	= 0x01;
	order[3]	= 0x54;
	order[4]	= atoi("26");
	order[5]	= 0x00;
	order[6]	= 0x01;
	order[7]	= 0x00;
	order[8]	= 0x00;
	order[9]	= 0x00;
	order[10]	= 0x00;
	order[11]	= 0x00;
	order[12]	= 0x00;
	order[13]	= 0x45;
	order[14]	= 0x44;

	unsigned short crc = crc16(order, 12);
	order[12] = ((char*)&crc)[0];
	print16(order,15);
	return 0;
#endif

#if 0
	char  asc[]= "zq 1 set 1 1 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 qz";
	const int size = sizeof(asc);
#endif

#if 0
	/*
	��̿��⣨����45��������ɣ�������Ĵ�д�ڿ����ϣ�
	��ͨ��������㣬�����һ����λ���������λ�����߸�����ͬ�ĸ�λ����ɣ������λ����1����2����3...��6��
	�õ���6�����ֻ�����λ��������6����λ����Ȼ���������λ�����߸�������ɣ�ֻ�����ֵ�������ϲ�ͬ���ѡ�
	�뽫�����λ�����������д�������ϡ�	
	*/
	int a, b, c, d, e, f, g;
	unsigned int result = 0;
	for (a = 1; a <= 9; a++)
	for (b = 0; b <= 9; b++)
	for (c = 0; c <= 9; c++)
	for (d = 0; d <= 9; d++)
	for (e = 0; e <= 9; e++)
	for (f = 0; f <= 9; f++)
	for (g = 0; g <= 9; g++)
	{
		unsigned int nums[7] = { a,b,c,d,e,f,g };
		if (IsSame(nums, 7)) {
			continue;
		}
		result = a * 1000000 + b * 100000 + c * 10000 + d * 1000 + e * 100 + f * 10 + g;

		bool isPass = true;
		unsigned int ses[7] = { 0 };
		unsigned int tmp = result * 2;
		if (tmp > 9999999) {
			continue;
		}
		reverse_int(tmp, ses);
		for (int i = 0; i < 7; i++) {
			if (!IsContain(ses[i], nums, 7)) {
				isPass = false;
				break;
			}
		}

		if (!isPass) { continue; }

		tmp = result * 3;
		if (tmp > 9999999) {
			continue;
		}
		reverse_int(tmp, ses);
		for (int i = 0; i < 7; i++) {
			if (!IsContain(ses[i], nums, 7)) {
				isPass = false;
				break;
			}
		}

		if (!isPass) { continue; }

		tmp = result * 4;
		if (tmp > 9999999) {
			continue;
		}
		reverse_int(tmp, ses);
		for (int i = 0; i < 7; i++) {
			if (!IsContain(ses[i], nums, 7)) {
				isPass = false;
				break;
			}
		}

		if (!isPass) { continue; }

		tmp = result * 5;
		if (tmp > 9999999) {
			continue;
		}
		reverse_int(tmp, ses);
		for (int i = 0; i < 7; i++) {
			if (!IsContain(ses[i], nums, 7)) {
				isPass = false;
				break;
			}
		}

		if (!isPass) { continue; }

		tmp = result * 6;
		if (tmp > 9999999) {
			continue;
		}
		reverse_int(tmp, ses);
		for (int i = 0; i < 7; i++) {
			if (!IsContain(ses[i], nums, 7)) {
				isPass = false;
				break;
			}
		}

		if (!isPass) { continue; }

		std::cout << "���ҽ����" << result << std::endl;
		std::cout << "��֤---------------" << std::endl;
		std::cout << "* 1 = " << result * 1 << std::endl;
		std::cout << "* 2 = " << result * 2 << std::endl;
		std::cout << "* 3 = " << result * 3 << std::endl;
		std::cout << "* 4 = " << result * 4 << std::endl;
		std::cout << "* 5 = " << result * 5 << std::endl;
		std::cout << "* 6 = " << result * 6 << std::endl;
	}

#endif

	struct _data {
		int a, b, c;
		char ary[0];
	};
	_data data;
	char str[] = "this is a test!";
	std::cout << "str len:" << sizeof(str) << std::endl;
	memcpy(data.ary, str, sizeof(str));
	std::cout << data.ary << ", size:" << sizeof(data) << std::endl;
	return 0;
}