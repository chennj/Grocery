#include "crc_easy_txt_client.h"
#include "crc_easy_websocket_client.h"

CRCEasyTxtClient::CRCEasyTxtClient()
{
    CRCNetWork::Init();
}

CRCClient* 
CRCEasyTxtClient::makeClientObj(SOCKET cSock, int sendSize, int recvSize)
{
    return new CRCClientCTxt(cSock, sendSize, recvSize);
}

void 
CRCEasyTxtClient::OnNetMsg(CRCDataHeader* header)
{
    CRCClientCTxt * pTxtClient = dynamic_cast<CRCClientCTxt*>(_pClient);

    if (!pTxtClient){
        CRCLog_Error("CRCEasyTxtClient::OnNetMsg pTxtClient is null");
        return;
    }

    if (clientState_join == pTxtClient->state()){

        if (!pTxtClient->getResponseInfo()){
            CRCLog_Error("CRCEasyTxtClient::OnNetMsg getResponseInfo failed");
            return;
        }

        if (auth())
        {
            CRCLog_Info("CRCEasyTxtClient::OnNetMsg, auth Good.");
            pTxtClient->state(clientState_run);
        }
        else {
            CRCLog_Warring("CRCEasyTxtClient::OnNetMsg auth, Bad.");
            pTxtClient->onClose();
        }
    }else if (clientState_run == pTxtClient->state()) {
        std::string & ss = pTxtClient->getContent();
        if (ss.compare("PING") == 0)
        {
            pTxtClient->pong();
        }
        else {
            //处理数据帧
            if (onmessage)
            {
                onmessage(pTxtClient);
            }
        }
    }
}

void 
CRCEasyTxtClient::send_buff_size(int n)
{
    _nSendBuffSize = n;
}

void 
CRCEasyTxtClient::recv_buff_size(int n)
{
    _nRecvBuffSize = n;
}

int 
CRCEasyTxtClient::writeText(const char* pData, int len)
{
    int ret = _pClient->SendData(pData, len);
    if (SOCKET_ERROR == ret)
    {
        CRCLog_Error("CRCEasyTxtClient::writeText<%s>-> SendData -> SOCKET_ERROR",pData);
    }
    return ret;
}

bool
CRCEasyTxtClient::auth()
{
    CRCClientCTxt * pTxtClient = dynamic_cast<CRCClientCTxt*>(_pClient);
    std::string & ss = pTxtClient->getContent();

    if (ss.compare("AUTH,0,\n") == 0){
        return true;
    } else {
        return false;
    }
}

bool 
CRCEasyTxtClient::connect(int af, const char* ip, unsigned short port)
{
    if (!ip)
        return false;

    if (INVALID_SOCKET == InitSocket(af, _nSendBuffSize, _nRecvBuffSize))
        return false;

    if (SOCKET_ERROR == Connect(ip, port))
        return false;

    CRCLog_Info("connet2ip(%s,%d)", ip, port);

    _pClient->state(clientState_join);

    writeText("RESP,\n", strlen("RESP,\n"));

    return true;
}

void 
CRCEasyTxtClient::setPairClient(CRCEasyWebSocketClient* client)
{
    _pPairClient = client;
}