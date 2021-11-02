#include "crc_transfer_file_server.h"

CrcTransFileServer::CrcTransFileServer()
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

CrcTransFileServer::~CrcTransFileServer()
{
    Close();
#ifdef _WIN32
	WSACleanup();
#endif
}

int
CrcTransFileServer::Initialize()
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

int
CrcTransFileServer::BindSocket(const char* ip, unsigned short port)
{
    if (INVALID_SOCKET == m_sock)
    {
        Initialize();
    }

    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(port);

#ifdef _WIN32
    if (ip)
    {
        _sin.sin_addr.S_un.S_addr = inet_addr(ip);
    }
    else
    {
        _sin.sin_addr.S_un.S_addr = INADDR_ANY;
    }
#else
    if (ip)
    {
        _sin.sin_addr.s_addr = inet_addr(ip);
    }
    else
    {
        _sin.sin_addr.s_addr = INADDR_ANY;
    }
#endif
    int ret = bind(m_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
    if (ret == SOCKET_ERROR)
    {
        std::cout << "socket bind failure because port used by other." << std::endl;
        return ret;
    }
    std::cout << "socket bind success." << std::endl;
    return ret;
}

void
CrcTransFileServer::Close()
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
CrcTransFileServer::Start()
{
    sockaddr_in clientAddr;
    int len = sizeof(sockaddr_in);
    char recvBuf[1024] = {0};

    //建立与客户端的连接
	recvfrom(m_sock, recvBuf, 1024, 0, (sockaddr*)&clientAddr, &len);
    std::cout << "Client Enter: " << recvBuf << std::endl;

    std::thread threadProc(std::mem_fn(&CrcTransFileServer::proc_recv), this);
	threadProc.detach();

    while(true)
    {
        char buf[1024] = {0};
        gets_s(buf, 1024);
        sendto(m_sock, buf, strlen(buf), 0, (sockaddr*)&clientAddr, sizeof(sockaddr));
    }
}

unsigned long
CrcTransFileServer::proc_recv()
{
    std::cout << "Waitting Recv Data..." << std::endl;

	//文件数据指针
	char * pRecvFileData = nullptr;
	//已经收到的文件的总的长度
	long recvedFileLen = 0;
	//fileSize：文件大小
	long fileSize = 0;
	//fileName：文件名
	char fileName[256] = { "d:/tmp/" };
	//文件用于保存接收到的数据
	FILE* pf = nullptr;

	while(true)
    {
        char recvBuf[1500] = {0};
        //阻塞，直到有数据
        int recvLen = recvfrom(m_sock, recvBuf, 1500, 0, 0, 0);
        //std::cout << "Client Talking:" << std::endl;
		//std::cout << recvBuf << std::endl;

		//接收到数据
		//需要区分接收到的数据是文件属性，还是文件数据
		int id = *(int*)recvBuf;						//获取数据属性id号

			//租借一个以仓库，准备装文件数据
		if (!pRecvFileData){
			pRecvFileData = new char[1024 * 1024 * 1024];
			if (!pRecvFileData) {
				//使用mmap
				std::cout << "File allocate 1G space failed!" << std::endl;
			}
		}

		//std::cout << "Recv data len : " << recvLen << std::endl;

		//根据id对数据做不同的处理
		switch (id)
		{
		case 1:												//运送文件数据的
		{
			Truck *tk = (Truck*)recvBuf;
			//将卡车上的数据放到仓库内指定的位置
			memcpy(pRecvFileData + tk->index * 1024, tk->fileData, recvLen - sizeof(int) - sizeof(long));
			//debug
			//std::cout << "Recv data: " << std::endl;
			//std::cout << "--------------------------------" << std::endl;
			//std::cout << tk->fileData << std::endl;
			//std::cout << "--------------------------------" << std::endl;
			recvedFileLen += (recvLen - sizeof(int) * 2);
		}
		break;
		case 2:												//运送文件属性的
		{
			//拷贝接收到的数据的前4个字节到fileSize，得到文件的大小
			memcpy(&fileSize, recvBuf + sizeof(int), sizeof(long));
			//拷贝文件名到fileName
			strcpy_s(fileName + 7, 256, recvBuf + sizeof(int) + sizeof(long));
			//建立文件
			if (pf) {
				fclose(pf);
			}
			fopen_s(&pf, fileName, "wb");
			//打印
			std::cout << "File Name:" << fileName << "; File Size:" << fileSize << "(byte)" << std::endl;
		}
		break;
		default:
		break;
		}

		//当文件接收完了，需要关闭文件并归还仓库
		if (recvedFileLen == fileSize && fileSize != 0) {	//文件已经接收完毕
			//将所有数据写入文件
			std::cout << "File has been recv completed!" << std::endl;
			fwrite(pRecvFileData, 1, recvedFileLen, pf);
			fclose(pf);
			recvedFileLen = 0;
			//归还仓库
			delete pRecvFileData;
		}


    }

    return 0;
}