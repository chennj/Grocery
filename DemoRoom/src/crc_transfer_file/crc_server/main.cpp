#include "crc_transfer_file_server.h"

int main(int argc, char** argv)
{ 
    CrcTransFileServer server;

    if (SOCKET_ERROR == server.Initialize()){
        return 0;
    }

    if (SOCKET_ERROR == server.BindSocket(nullptr, 8090)){
        return 0;
    }
    
	server.Start();

    return 0;
}