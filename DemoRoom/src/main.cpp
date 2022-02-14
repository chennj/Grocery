#include "common/Base.h"
#include "algorithm/DP/Solution.h"
#include <Windows.h>

#define THREADPOOL 0
#if		THREADPOOL	//�̳߳�
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

//�򵥲���
void test1()
{
	CRCThreadPool threadpool;
	threadpool.init(1);
	threadpool.start();								//�����̳߳�

	//Ҫִ�е�����
	threadpool.exec(1000, func0);
	threadpool.exec(func1, 10);
	threadpool.exec(func2, 20, "dofen");

	//�ȴ�����ȫ������
	threadpool.waitForAllDone();
	threadpool.stop();
}

//�����첽����ֵ
void test2()
{
	CRCThreadPool threadpool;
	threadpool.init(1);
	threadpool.start();								//�����̳߳�

	//Ҫִ�е�����
	std::future<decltype(func1_future(0))>	result1 = threadpool.exec(func1_future, 10);
	std::future<std::string>				result2 = threadpool.exec(func2_future, 20, "dofen");

	//�ȴ�����ȫ������
	threadpool.waitForAllDone();
	std::cout << "result1 = " << result1.get() << std::endl;
	std::cout << "result2 = " << result2.get() << std::endl;

	threadpool.stop();
}

//���Զ������İ�
void test3()
{
	CRCThreadPool threadpool;
	threadpool.init(1);
	threadpool.start();								//�����̳߳�

	//Ҫִ�е�����
	Test t1, t2;
	t1.setName("test1");
	t2.setName("test2");

	auto f1 = threadpool.exec(std::bind(&Test::test, &t1, std::placeholders::_1), 10);
	auto f2 = threadpool.exec(std::bind(&Test::test, &t2, std::placeholders::_1), 20);

	//�ȴ�����ȫ������
	threadpool.waitForAllDone();
	std::cout << "t1 result: " << f1.get() << std::endl;
	std::cout << "t2 result: " << f2.get() << std::endl;

	threadpool.stop();
}
#endif


#if 0
#define MAGSLOTMAX              128                 //����̲����������ڵ�Ƭ���ϵ�gpio��Ŀ
#define MAILBOXMAX              1                   //���������
#define RECORDERMAX             32                  //��������
#define BRIDGEMAX               1                   //δ֪������

#define MAXSTATIONMEDIA         MAGSLOTMAX*50       //����̲�����ÿ���̲�50���ۣ��ɷ�50�Ź���

#define RFIDSTRMAX              16                  //�ֵ̲��ӱ�ǩ����
#define MAGITEMMAX              50                  //�̲����̲�����

#define Q_PRT_STATUS			0x01
#define GO_PUT_DISC				0x02
#define GO_PRT_READY			0x03
#define CLEAN_HEAD				0x04
#define INITIALIZE				0x05
#define UPGRADE					0x06
#define SHUTDOWN				0x0A

/* LED3 POWER	LED2 PAPER	LED1 INK	LPC 2378
OFF	��˸	��˸	0x0C �ֳ�����
OFF	OFF	OFF	0x10 ��ӡ���ػ�
ON	ON	OFF	0x06 ��ӡ��ȱֽ
ON	��˸	OFF	0x09 ��ӡ����ֽ
ON	OFF	��˸	0x0A īˮ��
ON	OFF	ON	0x07 ȱī��ī�г��ֹ���
ON	��˸	��˸	0x0B��ӡ���������
ON	OFF	OFF	0x00��ӡ������״̬
��˸	����	����	0x0D��ӡ���Լ�
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
	/* ��ǰ����ָ�� */
	unsigned char           cmd_pi;
	/* cmd buff */
	char                    cmd_buff[CMDNUMS][CMDLENMAX];
	/* ִ�н��  */
	char                    cmd_y2no[CMDNUMS];
}MidasCmd;

/////////////////////////////////////////////////////////////////////
//MidasNet
/////////////////////////////////////////////////////////////////////
//IP Ϣ
typedef struct stIPInfo
{
	unsigned short		    IP1;
	unsigned short		    IP2;
	unsigned short		    IP3;
	unsigned short		    IP4;
}SIPInfo;

//Mac ��Ϣ
typedef struct stMacInfo
{
	unsigned short 	        mac1;
	unsigned short 	        mac2;
	unsigned short 	        mac3;
	unsigned short 	        mac4;
	unsigned short 	        mac5;
	unsigned short 	        mac6;
}SMacInfo;

/* ����������� */
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
/* ���� */
typedef struct _MidasMagTray {
	/*�����ڱ�־��*/
	unsigned char           tray_presence_flag;
}MidasMagTray;

/* ������ */
typedef struct _MidasBlueCd {
	/* ���ڱ�־ */
	unsigned char           cd_presence_flag;
}MidasBlueCd;

/* �豸���� */
typedef struct _MidasBridge {

	/* ���������� */
	MidasMagTray            mag_tray;
	/* �����޹��� */
	MidasBlueCd             blue_cd;
	/* ������ʱ����¼��Դ ,����ʱ�����̣�����Դ��ַ�����˻�ȥ*/
	int cd_src_addr;
}MidasBridge;

/* ����豸���� */
typedef struct _MidasBridgeSlot {
	/* ��ʱ��slot���Ƿ����� */
	char                    bridge_plug;
	/* С��״̬ */
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
	/* ����Դ������򿪵��豸�� */
	int                     srcfd;
	char                    cmdtype;
	int                     arg;
}ComboCmd;

typedef struct _Carrier {
	/* �����Ƿ��л�е�� */
	char carrier_plug;
	/* ����л�е�֣�æµ״̬ */
	char                    carrier_busy;
	/* ���е�������Ĵ��� */
	MidasTtys               tty_device;
	/* �̲� */
	MidasMagTray            mag_tray;
	/* ���� */
	MidasBlueCd             blue_cd;
	/* ����е��������ʱ����¼��Դ */
	int                     cd_src_address;
	/* ���������־ */
	char                    combo_flag;
	/* ��������� */
	short                   combo_len;
	/* ��ǰ����λ�� */
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
	/* ��ʱ��slot���Ƿ��й��� */
	char                    print_plug;
	/* ����й�����æµ״̬ */
	char                    print_busy;
	/* ����ӡ��������ʱ����¼��Դ ,����ʱ�����ӡ�����̣�����Դ��ַ�����˻�ȥ*/
	int                     cdrom_src_address;
	MidasMagTray            mag_tray;
	MidasBlueCd             blue_cd;
	/* ��ӡ����ǰ״̬ */
	SPrtReturn              print_status;
}MidasPrinter;

/////////////////////////////////////////////////////////////////////
//MidasMagSlot
/////////////////////////////////////////////////////////////////////
/* �̲� */
typedef struct _MidasMagItem {
	/* �̲� */
	MidasMagTray            mag_tray;
	/* ���� */
	MidasBlueCd             blue_cd;
}MidasMagItem;

/* �̲� */
typedef struct _MidasMagazine {
	/* �ֵ̲��ӱ�ǩ */
	char                    serial[RFIDSTRMAX];
	/* �̲���Ŀ */
	MidasMagItem            mag_item[MAGITEMMAX];
}MidasMagazine;

/* �ֲ̲���豸 */
typedef struct _MidasMagSlot {
	/* ��ʱ��slot���Ƿ����̲� */
	char                    mag_plug;
	/* ��ۺ���Ƶ��ȡ�豸�� */
	char                    rfid_devminor;
	char                    rfid_telno;
	/*  ������̲� */
	MidasMagazine           magazine;
}MidasMagSlot;

/////////////////////////////////////////////////////////////////////
//MidasMailBoxSlot
/////////////////////////////////////////////////////////////////////
/* �����豸���� */
typedef struct _MidasMailBox {

	/* �������������� */
	MidasMagTray            mag_tray;
	/* ���������޹��� */
	MidasBlueCd             blue_cd;
	/* ������������ʱ����¼��Դ ,����ʱ����������̣�����Դ��ַ�����˻�ȥ*/
	int cd_src_addr;
}MidasMailBox;

/* �������豸���� */
typedef struct _MailBoxrSlot {
	/* ��ʱ��slot���Ƿ������� */
	char                    mailbox_plug;
	/* ����й�����æµ״̬ */
	char                    mailbox_busy;
	MidasMailBox            maibox;
}MidasMailBoxSlot;

/////////////////////////////////////////////////////////////////////
//MidasRecordeSlot
/////////////////////////////////////////////////////////////////////
/* �����豸���� */
typedef struct _MidasRecorder {
	MidasMagTray            mag_tray;
	MidasBlueCd             blue_cd;
	/* ������������ʱ����¼��Դ �߼���ַ*/
	int cd_src_address;
}MidasRecorder;

typedef struct _MidasRecorderSlot {
	/* ��ʱ��slot���Ƿ��й��� */
	char                    recorder_plug;
	/* ����й�����æµ״̬ */
	char                    recorder_busy;
	/* �豸�������̺��е�״̬ */
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
	/* ���� */
	MidasNet                net;
	/* ������ */
	MidasDoor               door;
	/* ��е�� */
	MidasCarrier            carrier;
	/* ��ӡ�� */
	MidasPrinter            printer;
	/* �豸�̲��У�����������װ�˶����̲� */
	MidasMagSlot            mag_slotarray[MAGSLOTMAX];
	/* �����豸�� ������������װ�˶��ٸ����� */
	MidasMailBoxSlot        mailbox_slotarray[MAILBOXMAX];
	/* �����豸�� �����������˶��ٹ��� */
	MidasRecordeSlot        recorde_slotArray[RECORDERMAX];
	/* ���豸�� �����������˶����� */
	MidasBridgeSlot         bridge_slotArray[BRIDGEMAX];
	/* ͳ��ʹ������ */
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
	//��ָ��������Ա����
	/*
	��Ķ���ָ��Ϊ��ʱ��������ָ�ö���Ͳ����ڣ������ǵ�ָ��û������ָ�������ط�ʱ����ʱ��ָ�붼���Ե������г�Ա������
	���޷��������麯����ͬ���ĵ����������� delete �ͷ�һ����Ķ����ڴ�ռ�ʱ�������ǽ��ö����ڴ�ռ���������
	���ǽ�����ڴ��ʹ��Ȩ�ͷ��ˣ�������Ķ������ܻ���Ȼ���ڣ�����������ڴ汻��������ռ��֮ǰ����Ķ�����Ȼ����ڣ�
	���Լ�ʹ���ǽ�����ָ��ָ��պ���Ȼ���Ե������еĺ���������Ҫǿ�����ǣ�deleteֻ�ǰ�ָ����ָ���ڴ��ͷŵ���
	��û�а�ָ�뱾����ɵ�������ƽ����ָ����ָ�ڴ��ͷŵ�֮������ڴ�ָ��գ�����Ϊ�˷�ֹ����Ұָ�룬
	Ϊ���´ε���ָ���ʱ�򷽱��ж�ʹ�ã������ǽ���ָ��ָ��NULL��ָ��ʲô��û���ˡ�
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

#if THREADPOOL	//�̳߳ز���
	std::cout << "�첽�̳߳ز��ԣ���ͨ����    -------------------" << std::endl;
	test1();
	std::cout << "�첽�̳߳ز��ԣ��첽���ؽ�� -------------------" << std::endl;
	test2();
	std::cout << "�첽�̳߳ز��ԣ������İ� -------------------" << std::endl;
	test3();
#endif

#if CPP_INTERVIEW
	std::cout << 25u - 50 << std::endl;
	return 0;
#endif
}