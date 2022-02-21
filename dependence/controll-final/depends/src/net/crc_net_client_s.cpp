#include "crc_net_client_s.h"

CRCNetClientS::CRCNetClientS(SOCKET sockfd, int sendSize, int recvSize) 
    :
    CRCClientSWebSocket(sockfd, sendSize, recvSize)
{

}

bool
CRCNetClientS::transfer(CRCJson& msg)
{
    std::string retStr = msg.ToString();
    if (SOCKET_ERROR == this->writeText(retStr.c_str(), retStr.length()))
    {
        CRCLog_Error("CRCNetClientS::transfer::writeText SOCKET_ERROR.");
        return false;
    }
    return true;
}