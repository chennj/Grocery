#include "crc_transfer_file_client.h"

CrcTransFileClient::CrcTransFileClient()
{
#ifdef _WIN32
    WORD ver = MAKEWORD(2, 2);
    WSADATA data;
    WSAStartup(ver, &data);
#else
    // ignore exception signal
    signal(SIGPIPE, SIG_IGN);
#endif
}

CrcTransFileClient::~CrcTransFileClient()
{
    Close();
#ifdef _WIN32
	WSACleanup();
#endif
}

int
CrcTransFileClient::Initialize()
{
    //创建一个UDP套接字
    m_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (SOCKET_ERROR == m_sock)
    {
        std::cout << "create socket failure." << std::endl;
        return m_sock;
    }
    std::cout << "socket create success." << std::endl;
    return m_sock;
}


void
CrcTransFileClient::Close()
{
    if (m_sock == INVALID_SOCKET) return;

#ifdef _WIN32
    closesocket(m_sock);
#else
    close(m_sock);
#endif
    m_sock = INVALID_SOCKET;
}

void
CrcTransFileClient::StartTalking(const char* ip, unsigned short port)
{
    sockaddr_in serverAddr;
	
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.S_un.S_addr = inet_addr(ip);

    int len = sizeof(sockaddr_in);
    char recvBuf[1024] = {0};

	sendto(m_sock, "say hello", strlen("say hello"), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));

    std::thread threadProc(std::mem_fn(&CrcTransFileClient::proc_recv), this);
	threadProc.detach();

    while(true)
    {
        char buf[1024] = {0};
		std::cout << "Please Input File Name What's you need: ";
		//获得屏幕的输入
        gets_s(buf, 1024);
		//获取文件的大小
		long fileSize = get_file_len(buf);
		if (fileSize == -1) {
			std::cout << "File is invalid!" << std::endl;
			continue;
		}
		//获取文件名
		const char* fileName = get_file_name(buf);
		//定义一个发送缓冲区
		char sendBuf[1024] = { 0 };
		//表示下面发送的数据是文件的属性
		int flag = 2;
		//内存拷贝，拷贝数据类型的标号
		memcpy(sendBuf, &flag, sizeof(int));
		//将文件大小拷贝到发送缓冲区的前面4个字节：sizeof(unsigned long)=4
		memcpy(sendBuf + sizeof(int), &fileSize, sizeof(long));
		//拷贝文件名称到发送缓冲区
		memcpy(sendBuf + sizeof(int) + sizeof(long), fileName, strlen(fileName));
		
		//将文件大小和文件名称发送到服务器
        sendto(m_sock, sendBuf, sizeof(long) + sizeof(int) + strlen(fileName), 0, (sockaddr*)&serverAddr, sizeof(sockaddr));
		
		std::cout << "File Attribute had been send to server, Push any key to continue...";
		getchar();

		//发送文件中的数据
		//获取文件数据
		FILE* pf = nullptr;
		fopen_s(&pf, buf, "rb");	//以读取的方式打开文件

		//循环向服务器发送数据，直到文件的结尾
		int index = 0;				//文件数据块索引号
		while (!feof(pf))			//如果未到文件结尾，则一直循环
		{
			Truck tk;
			memset(&tk, 0, sizeof(Truck));
			tk.id = 1;
			//UDP传输是无序的，必须为每个数据块编制索引
			tk.index = index++;
			//读文件，一次1024字节，放入fileData
			int readSize = fread(tk.fileData, 1, 1024, pf);
			//debug
			//std::cout << "Read File len: " << readSize << std::endl;
			//std::cout << "File size: " << fileSize << std::endl;
			//std::cout << "Send data: " << std::endl;
			//std::cout << "--------------------------------" << std::endl;
			//std::cout << tk.fileData << std::endl;
			//std::cout << "--------------------------------" << std::endl;
			//将fileData发送到服务器
			sendto(m_sock, (char*)&tk, sizeof(int) + sizeof(long) + readSize, 0, (sockaddr*)&serverAddr, sizeof(sockaddr));
		}

		fclose(pf);
    }
}

unsigned long
CrcTransFileClient::proc_recv()
{
    //std::cout << "Waitting Recv Data..." << std::endl;
    while(true)
    {
        char recvBuf[1024] = {0};
        //阻塞，直到有数据
        recvfrom(m_sock, recvBuf, 1024, 0, 0, 0);
		std::cout << "Server Talking:" << std::endl;
		std::cout << recvBuf << std::endl;
    }

    return 0;
}

long
CrcTransFileClient::get_file_len(const char* path)
{
	FILE* pf = nullptr;
	
	fopen_s(&pf, path, "rb");
	if (!pf) {
		return -1;
	}

	fseek(pf, 0, SEEK_END);
	long size = ftell(pf);
	fclose(pf);
	return size;
}

const char*
CrcTransFileClient::get_file_name(const char* path)
{
	std::string spath = path;
	int pos = spath.find_last_of("/");
	if (pos == std::string::npos) {
		pos = spath.find_last_of("\\");
	}
	if (pos == std::string::npos) {
		return nullptr;
	}
	return path + pos + 1;
}