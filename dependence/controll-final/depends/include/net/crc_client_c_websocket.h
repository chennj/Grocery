#ifndef _CRC_CLIENT_C_WEBSOCKET_H_
#define _CRC_CLIENT_C_WEBSOCKET_H_

#include "crc_client_c_http.h"
#include "sha1.h"
#include "crc_msg_websocket.hpp"

class CRCClientCWebSocket :public CRCClientCHttp
{
public:
    CRCClientCWebSocket(SOCKET sockfd = INVALID_SOCKET, int sendSize = SEND_BUFF_SZIE, int recvSize = RECV_BUFF_SZIE);
    virtual ~CRCClientCWebSocket();

    virtual bool hasMsg() override;

    virtual void pop_front_msg() override;

    virtual bool hasMsgWS();

    char * fetch_data();

    void do_mask(int len);

    int writeHeader(CRCIO::WebSocketOpcode opcode, uint64_t len, bool mask, int32_t mask_key);

    int writeText(const char* pData, int len);

    int ping();

    int pong();

    CRCIO::WebSocketHeader& WebsocketHeader();
    
private:
    CRCIO::WebSocketHeader _wsh = {};
    int32_t _mask_key = rand();
};

#endif //!_CRC_CLIENT_C_WEBSOCKET_H_