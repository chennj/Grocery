#include "common/Base.h"
#include "algorithm/DP/Solution.h"
#include <Windows.h>

#define THREADPOOL 0
#if		THREADPOOL	//线程池
#include "../../dependence/tools/threadpool/CRCThreadPool.h"

class Test
{
public:
	int test(int i) {
		std::cout << _name << ", i = " << i << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
	threadpool.init(1);
	threadpool.start();								//启动线程池

	//要执行的任务
	std::future<decltype(func1_future(0))>	result1 = threadpool.exec(func1_future, 10);
	std::future<std::string>				result2 = threadpool.exec(func2_future, 20, "dofen");

	//等待任务全部结束
	threadpool.waitForAllDone();
	std::cout << "result1 = " << result1.get() << std::endl;
	std::cout << "result2 = " << result2.get() << std::endl;

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
	std::cout << "t1 result: " << f1.get() << std::endl;
	std::cout << "t2 result: " << f2.get() << std::endl;

	threadpool.stop();
}
#endif


#if 0
#define MAGSLOTMAX              128                 //最大盘仓数，依赖于单片机上的gpio数目
#define MAILBOXMAX              1                   //最大邮箱数
#define RECORDERMAX             32                  //最大光驱数
#define BRIDGEMAX               1                   //未知，待定

#define MAXSTATIONMEDIA         MAGSLOTMAX*50       //最大盘槽数，每个盘仓50个槽，可放50张光盘

#define RFIDSTRMAX              16                  //盘仓电子标签长度
#define MAGITEMMAX              50                  //盘仓中盘槽数量

#define Q_PRT_STATUS			0x01
#define GO_PUT_DISC				0x02
#define GO_PRT_READY			0x03
#define CLEAN_HEAD				0x04
#define INITIALIZE				0x05
#define UPGRADE					0x06
#define SHUTDOWN				0x0A

/* LED3 POWER	LED2 PAPER	LED1 INK	LPC 2378
OFF	闪烁	闪烁	0x0C 字车错误
OFF	OFF	OFF	0x10 打印机关机
ON	ON	OFF	0x06 打印机缺纸
ON	闪烁	OFF	0x09 打印机夹纸
ON	OFF	闪烁	0x0A 墨水少
ON	OFF	ON	0x07 缺墨或墨盒出现故障
ON	闪烁	闪烁	0x0B打印机导板错误
ON	OFF	OFF	0x00打印机正常状态
闪烁	不变	不变	0x0D打印机自检
*/

#define COMBCMDOMAX				16

#define CMDLENMAX               64
#define CMDNUMS                 32

/////////////////////////////////////////////////////////////////////
//MidasAttr
/////////////////////////////////////////////////////////////////////
typedef struct _MidasAttr {
	/* box name */
	char                    box_name[32];
	/* max mag num */
	unsigned short          mag_max;
	/* max  recorder num */
	unsigned short          rec_max;
	/* max mail */
	unsigned short          mail_max;
}MidasAttr;

/////////////////////////////////////////////////////////////////////
//MidasNet
/////////////////////////////////////////////////////////////////////
typedef struct  _MidasCmd {
	/* 当前命令指针 */
	unsigned char           cmd_pi;
	/* cmd buff */
	char                    cmd_buff[CMDNUMS][CMDLENMAX];
	/* 执行结果  */
	char                    cmd_y2no[CMDNUMS];
}MidasCmd;

/////////////////////////////////////////////////////////////////////
//MidasNet
/////////////////////////////////////////////////////////////////////
//IP 息
typedef struct stIPInfo
{
	unsigned short		    IP1;
	unsigned short		    IP2;
	unsigned short		    IP3;
	unsigned short		    IP4;
}SIPInfo;

//Mac 信息
typedef struct stMacInfo
{
	unsigned short 	        mac1;
	unsigned short 	        mac2;
	unsigned short 	        mac3;
	unsigned short 	        mac4;
	unsigned short 	        mac5;
	unsigned short 	        mac6;
}SMacInfo;

/* 网络服务描述 */
typedef struct _MidasNet {
	/* mac address */
	SMacInfo                smacinfo;
	/* ip */
	SIPInfo                 sipinfo;
}MidasNet;

/////////////////////////////////////////////////////////////////////
//MidasDoor
/////////////////////////////////////////////////////////////////////
typedef struct _Door {
	char                    door_flag;
}MidasDoor;

/////////////////////////////////////////////////////////////////////
//MidasBridgeSlot
/////////////////////////////////////////////////////////////////////
/* 盘托 */
typedef struct _MidasMagTray {
	/*　存在标志　*/
	unsigned char           tray_presence_flag;
}MidasMagTray;

/* 蓝光盘 */
typedef struct _MidasBlueCd {
	/* 存在标志 */
	unsigned char           cd_presence_flag;
}MidasBlueCd;

/* 设备描述 */
typedef struct _MidasBridge {

	/* 中有无盘托 */
	MidasMagTray            mag_tray;
	/* 中有无光盘 */
	MidasBlueCd             blue_cd;
	/* 当有盘时，记录来源 ,启动时如有盘，按此源地址，搬运回去*/
	int cd_src_addr;
}MidasBridge;

/* 插槽设备描述 */
typedef struct _MidasBridgeSlot {
	/* 表时此slot上是否有桥 */
	char                    bridge_plug;
	/* 小车状态 */
	char                    cab_pos;
	MidasBridge             bridge;
}MidasBridgeSlot;

/////////////////////////////////////////////////////////////////////
//MidasCarrier
/////////////////////////////////////////////////////////////////////
/*  add */
typedef struct _MidasTtysDevice {
	char                    ttys_dev_name[32];
}MidasTtys;

typedef struct _ComboCmd {
	/* 命令源，传入打开的设备号 */
	int                     srcfd;
	char                    cmdtype;
	int                     arg;
}ComboCmd;

typedef struct _Carrier {
	/* 表明是否有机械手 */
	char carrier_plug;
	/* 如果有机械手，忙碌状态 */
	char                    carrier_busy;
	/* 与机械手相连的串口 */
	MidasTtys               tty_device;
	/* 盘槽 */
	MidasMagTray            mag_tray;
	/* 光盘 */
	MidasBlueCd             blue_cd;
	/* 当机械手上有盘时，记录来源 */
	int                     cd_src_address;
	/* 复合命令标志 */
	char                    combo_flag;
	/* 复合命令长度 */
	short                   combo_len;
	/* 当前命令位置 */
	short                   combo_p;
	ComboCmd                combo_cmd[COMBCMDOMAX];
}MidasCarrier;

/////////////////////////////////////////////////////////////////////
//MidasPrinter
/////////////////////////////////////////////////////////////////////
typedef struct prt_return
{
	unsigned char			printer_status;
	unsigned char			printer_hasdisc;
	unsigned char			printer_pos;
	unsigned char			printer_model[6];
	unsigned char			printer_serial[6];
	unsigned char			printer_errno;
	unsigned char			is_printed;
	unsigned char			reserved_b;
	unsigned char			crc_high;
	unsigned char			crc_low;
}SPrtReturn;

typedef struct _MidasPrint {
	/* 表时此slot上是否有光驱 */
	char                    print_plug;
	/* 如果有光驱，忙碌状态 */
	char                    print_busy;
	/* 当打印机中有盘时，记录来源 ,启动时如果打印机有盘，按此源地址，搬运回去*/
	int                     cdrom_src_address;
	MidasMagTray            mag_tray;
	MidasBlueCd             blue_cd;
	/* 打印机当前状态 */
	SPrtReturn              print_status;
}MidasPrinter;

/////////////////////////////////////////////////////////////////////
//MidasMagSlot
/////////////////////////////////////////////////////////////////////
/* 盘槽 */
typedef struct _MidasMagItem {
	/* 盘槽 */
	MidasMagTray            mag_tray;
	/* 光盘 */
	MidasBlueCd             blue_cd;
}MidasMagItem;

/* 盘仓 */
typedef struct _MidasMagazine {
	/* 盘仓电子标签 */
	char                    serial[RFIDSTRMAX];
	/* 盘槽数目 */
	MidasMagItem            mag_item[MAGITEMMAX];
}MidasMagazine;

/* 盘仓插槽设备 */
typedef struct _MidasMagSlot {
	/* 表时此slot上是否有盘仓 */
	char                    mag_plug;
	/* 插槽后射频读取设备号 */
	char                    rfid_devminor;
	char                    rfid_telno;
	/*  插槽中盘仓 */
	MidasMagazine           magazine;
}MidasMagSlot;

/////////////////////////////////////////////////////////////////////
//MidasMailBoxSlot
/////////////////////////////////////////////////////////////////////
/* 邮箱设备描述 */
typedef struct _MidasMailBox {

	/* 邮箱中有无盘托 */
	MidasMagTray            mag_tray;
	/* 邮箱中有无光盘 */
	MidasBlueCd             blue_cd;
	/* 当邮箱中有盘时，记录来源 ,启动时如果邮箱有盘，按此源地址，搬运回去*/
	int cd_src_addr;
}MidasMailBox;

/* 邮箱插槽设备描述 */
typedef struct _MailBoxrSlot {
	/* 表时此slot上是否有邮箱 */
	char                    mailbox_plug;
	/* 如果有光驱，忙碌状态 */
	char                    mailbox_busy;
	MidasMailBox            maibox;
}MidasMailBoxSlot;

/////////////////////////////////////////////////////////////////////
//MidasRecordeSlot
/////////////////////////////////////////////////////////////////////
/* 光驱设备描述 */
typedef struct _MidasRecorder {
	MidasMagTray            mag_tray;
	MidasBlueCd             blue_cd;
	/* 当光驱中有盘时，记录来源 逻辑地址*/
	int cd_src_address;
}MidasRecorder;

typedef struct _MidasRecorderSlot {
	/* 表时此slot上是否有光驱 */
	char                    recorder_plug;
	/* 如果有光驱，忙碌状态 */
	char                    recorder_busy;
	/* 设备描述光盘和托的状态 */
	MidasRecorder recorder;
}MidasRecordeSlot;

/////////////////////////////////////////////////////////////////////
//TotalCnt
/////////////////////////////////////////////////////////////////////
typedef struct _TotalCnt {
	unsigned int            prt_cnt;
	unsigned int            carr_cnt;
	unsigned int            mail_cnt;
	unsigned int            bridge_cnt;
	unsigned int            record_cnt[RECORDERMAX];
}TotalCnt;

/////////////////////////////////////////////////////////////////////
//MidasBox
/////////////////////////////////////////////////////////////////////
typedef struct _MidasBox {
	/* first start flag */
	char                    first_flag;
	/* not excute shutdown */
	char                    not_shutdown_flag;
	/* box attribute */
	MidasAttr	            midas_attr;
	/* cmd */
	MidasCmd                midas_cmd;
	/* 网卡 */
	MidasNet                net;
	/* 机箱门 */
	MidasDoor               door;
	/* 机械手 */
	MidasCarrier            carrier;
	/* 打印机 */
	MidasPrinter            printer;
	/* 设备盘仓列，用于描述安装了多少盘仓 */
	MidasMagSlot            mag_slotarray[MAGSLOTMAX];
	/* 邮箱设备列 ，用于描述安装了多少个邮箱 */
	MidasMailBoxSlot        mailbox_slotarray[MAILBOXMAX];
	/* 光驱设备列 ，用于描述了多少光驱 */
	MidasRecordeSlot        recorde_slotArray[RECORDERMAX];
	/* 桥设备列 ，用于描述了多少桥 */
	MidasBridgeSlot         bridge_slotArray[BRIDGEMAX];
	/* 统计使用数量 */
	TotalCnt		        total_cnt;
}MidasBox;

#endif

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

bool IsSame(unsigned int *array,int size)
{
	//在前面的数，与其后面的数比较，一旦发现相同，即返回true
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
		//printf("%d", v % 10);//输出个位。
		ret[i++] = v % 10;
		v /= 10; //将下一位数字移动到个位。
	}
}

#include <type_traits>
template<typename It>
auto fcn(It beg)->decltype(*beg)
{
	return *beg;
};
template<typename It>
auto fcn(It beg)->typename std::remove_reference<decltype(*beg)>::type
{
	return *beg;
};
	
template<class T1, class T2>
void print_is_same() {
	std::cout << std::is_same<T1, T2>() << '\n';
};

#define CPP_INTERVIEW 1

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

#if 0 DLL实验
	typedef BOOL(*DLL_FUNC_1)(const MCHAR* s, BOOL quietErrors, FPValue* fpv);

	//wchar_t path[] = L"D:\\vs2015\\Grocery\\bin\\Debug-windows-x86_64\\CommonDll\\CommonDll.dll";
	//wchar_t path[] = L"D:\\vs2015\\WechatHookDll\\x64\\Debug\\EasyHook.dll";
	//wchar_t path[] = L"D:\\写意公司\\3ds Max 2014 SDK\\lib\\x64\\Release\\";
	//wchar_t path[] = L"D:\\写意公司\\3ds Max 2014 SDK\\lib\\x64\\Release\\Maxscrpt.dll";
	//必须同一目录，否则绝对路径
	wchar_t path[] = L"../CommonDll/CommonDll.dll";
	//HMODULE hmod = LoadLibraryEx(path, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
	HMODULE hmod = LoadLibraryW(path);
	if (!hmod) {
		std::cout << "加载DLL失败，退出。" << std::endl;
		return 0;
	}
	//std::cout << "基地址：" << static_cast <const  void  *>(hmod) << std::endl;

	DLL_FUNC_1 func_1 = (DLL_FUNC_1)::GetProcAddress(hmod, "ExecuteMAXScriptScript");
	if (!func_1) {
		std::cout << "没有exprot_test_str这个函数。" << std::endl;
		FreeLibrary(hmod);
		system("PAUSE");
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
	编程考题（请在45分钟内完成，将运算的答案写在考卷上）
	请通过编程运算，计算出一个七位数，这个七位数由七个不相同的个位数组成，这个七位数乘1、乘2、乘3...乘6，
	得到的6个数字还是七位数，且这6个七位数依然是由最初七位数的七个数字组成，只是数字的排列组合不同而已。
	请将这个七位数计算出来，写到考卷上。	
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

		std::cout << "查找结果：" << result << std::endl;
		std::cout << "验证---------------" << std::endl;
		std::cout << "* 1 = " << result * 1 << std::endl;
		std::cout << "* 2 = " << result * 2 << std::endl;
		std::cout << "* 3 = " << result * 3 << std::endl;
		std::cout << "* 4 = " << result * 4 << std::endl;
		std::cout << "* 5 = " << result * 5 << std::endl;
		std::cout << "* 6 = " << result * 6 << std::endl;
	}

#endif

#if 0
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
#endif

#if 0
	//空指针访问类成员函数
	/*
	类的对象指针为空时，并不是指该对象就不存在，当我们的指针没有用来指向其他地方时，此时该指针都可以调用类中成员函数，
	但无法调用其虚函数。同样的道理，当我们用 delete 释放一个类的对象内存空间时，并不是将该对象内存空间进行清除，
	而是将这块内存的使用权释放了，但里面的东西可能还依然存在，所以在这块内存被其他对象占用之前里面的东西依然会存在，
	所以即使我们将对象指针指向空后，依然可以调用类中的函数。这里要强调的是，delete只是把指针所指的内存释放掉，
	并没有把指针本身给干掉。我们平常将指针所指内存释放掉之后会让内存指向空，这是为了防止产生野指针，
	为了下次调用指针的时候方便判断使用，并不是将该指针指向NULL后指针什么都没有了。
	*/
	class A {
	public:
		void foo() { std::cout << "foo" << std::endl; }
		void foo1() { std::cout << "a: " << a << std::endl; }
		virtual void foo2() { std::cout << "foo 2" << std::endl; }
	public:
		int a=1;
	};

	A* pa = new A();
	decltype(pa) p = pa;
	p->foo();

	delete pa;
	p->foo();
	p->foo1();
	//pa->foo2();
	pa = nullptr;
	p->foo();
	//pa->foo1();
	//pa->foo2();

	print_is_same<int, int>();
	print_is_same<int, int &>();
	print_is_same<int, int &&>();

	print_is_same<int, std::remove_reference<int>::type>();
	print_is_same<int, std::remove_reference<int &>::type>();
	print_is_same<int, std::remove_reference<int &&>::type>();

	return 0;
#endif

#if 0
	const int size = sizeof(MidasBox);
#endif

#if THREADPOOL	//线程池测试
	std::cout << "异步线程池测试：普通函数    -------------------" << std::endl;
	test1();
	std::cout << "异步线程池测试：异步返回结果 -------------------" << std::endl;
	test2();
	std::cout << "异步线程池测试：象函数的绑定 -------------------" << std::endl;
	test3();
#endif

#if CPP_INTERVIEW
	std::cout << 25u - 50 << std::endl;
	return 0;
#endif
}