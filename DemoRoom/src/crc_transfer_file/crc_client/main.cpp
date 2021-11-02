#include "crc_transfer_file_client.h"

int main(int argc, char** argv)
{ 
    CrcTransFileClient client;

    if (SOCKET_ERROR == client.Initialize()){
        return 0;
    }
    
	client.StartTalking("192.168.1.3", 8090);

    return 0;
}