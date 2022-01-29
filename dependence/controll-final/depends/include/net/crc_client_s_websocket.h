#ifndef _CRC_CLIENT_S_WEBSOCKET_H_
#define _CRC_CLIENT_S_WEBSOCKET_H_

#include "crc_client_s_http.h"
#include "sha1.h"
#include "crc_msg_websocket.hpp"

class CRCClientSWebSocket : public CRCClientSHttp
{
public:
    CRCClientSWebSocket(SOCKET sockfd = INVALID_SOCKET, int sendSize = SEND_BUFF_SZIE, int recvSize = RECV_BUFF_SZIE);
    virtual ~CRCClientSWebSocket();

    bool handshake();

    virtual bool hasMsg() override;

    virtual void pop_front_msg() override;

    virtual bool hasMsgWS();

    char * fetch_data();

    int writeHeader(CRCIO::WebSocketOpcode opcode, uint64_t len);

    int writeText(const char* pData, int len);

    int ping();

    int pong();

    CRCIO::WebSocketHeader& WebsocketHeader();
private:
    CRCIO::WebSocketHeader _wsh = {};
};
#endif //!_CRC_CLIENT_S_WEBSOCKET_H_