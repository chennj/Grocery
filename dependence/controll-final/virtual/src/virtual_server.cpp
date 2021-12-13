#include "virtual_server.h"

VirtualServer::VirtualServer()
{
    _bSendBack      = CRCConfig::Instance().hasKey("-sendback");
    _bSendFull      = CRCConfig::Instance().hasKey("-sendfull");
    _bCheckMsgID    = CRCConfig::Instance().hasKey("-checkMsgID");
}

void 
VirtualServer::OnNetJoin(CRCClient* pClient)
{
    CRCEasyTcpServer::OnNetJoin(pClient);
}

void 
VirtualServer::OnNetLeave(CRCClient* pClient)
{
    CRCEasyTcpServer::OnNetLeave(pClient);
}

void 
VirtualServer::OnNetMsg(CRCWorkServer* pServer, CRCClient* pClient, CRCDataHeader* header)
{
    CRCEasyTcpServer::OnNetMsg(pServer, pClient, header);
    switch (header->cmd)
    {
    case CMD_EQPT_MOVE:
    {
        CRCEqptMove* mmove = (CRCEqptMove*)header;
        CRCLog_Info("虚拟服务器 接收到命令：socket<%d> msgID<%d> MOVE discAddr<%d> FROM srcAddr<%d> TO destAddr<%d> test<%s> ",
            pClient->sockfd(), 
            mmove->msgId, 
            mmove->discAddr,
            mmove->srcAddr,
            mmove->destAddr,
            mmove->test
        );
        pClient->resetDTHeart();
        //检查消息ID
        if (_bCheckMsgID)
        {
            if (mmove->msgId != pClient->nRecvMsgID)
            {//当前消息ID和本地收消息次数不匹配
                CRCLog_Error("OnNetMsg socket<%d> msgID<%d> _nRecvMsgID<%d> %d", pClient->sockfd(), mmove->msgId, pClient->nRecvMsgID, mmove->msgId - pClient->nRecvMsgID);
            }
            ++pClient->nRecvMsgID;
        }
        //移动逻辑
        //......
        //回应消息
        if (_bSendBack)
        {
            CRCEqptMoveR ret;
            ret.sessionId   = mmove->sessionId;
            ret.msgId       = pClient->nSendMsgID;
            ret.srcAddr     = mmove->srcAddr;
            ret.destAddr    = mmove->destAddr;
            ret.discAddr    = mmove->discAddr;
            strncpy(ret.code, "88888888", 8);
            ret.code[8] = '\0';
            if (SOCKET_ERROR == pClient->SendData(&ret))
            {
                //发送缓冲区满了，消息没发出去,目前直接抛弃了
                //客户端消息太多，需要考虑应对策略
                //正常连接，业务客户端不会有这么多消息
                //模拟并发测试时是否发送频率过高
                if (_bSendFull)
                {
                    CRCLog_Warring("<Socket=%d> Send Full", pClient->sockfd());
                }
            }
            else {
                ++pClient->nSendMsgID;
            }
        }
    }
    break;
    case CMD_LOGIN:
    {
        pClient->resetDTHeart();
        CRCLogin* login = (CRCLogin*)header;
        //检查消息ID
        if (_bCheckMsgID)
        {
            if (login->msgId != pClient->nRecvMsgID)
            {//当前消息ID和本地收消息次数不匹配
                CRCLog_Error("OnNetMsg socket<%d> msgID<%d> _nRecvMsgID<%d> %d", pClient->sockfd(), login->msgId, pClient->nRecvMsgID, login->msgId - pClient->nRecvMsgID);
            }
            ++pClient->nRecvMsgID;
        }
        //登录逻辑
        //......
        //回应消息
        if (_bSendBack)
        {
            CRCLoginR ret;
            ret.msgId = pClient->nSendMsgID;
            if (SOCKET_ERROR == pClient->SendData(&ret))
            {
                //发送缓冲区满了，消息没发出去,目前直接抛弃了
                //客户端消息太多，需要考虑应对策略
                //正常连接，业务客户端不会有这么多消息
                //模拟并发测试时是否发送频率过高
                if (_bSendFull)
                {
                    CRCLog_Warring("<Socket=%d> Send Full", pClient->sockfd());
                }
            }
            else {
                ++pClient->nSendMsgID;
            }
        }

        //CRCLog_Info("recv <Socket=%d> msgType：CMD_LOGIN, dataLen：%d,userName=%s PassWord=%s", cSock, login->dataLength, login->userName, login->PassWord);
    }//接收 消息---处理 发送   生产者 数据缓冲区  消费者 
    break;
    case CMD_LOGOUT:
    {
        pClient->resetDTHeart();
        CRCReadStream r(header);
        //读取消息长度
        r.ReadInt16();
        //读取消息命令
        r.getNetCmd();
        auto n1 = r.ReadInt8();
        auto n2 = r.ReadInt16();
        auto n3 = r.ReadInt32();
        auto n4 = r.ReadFloat();
        auto n5 = r.ReadDouble();
        uint32_t n = 0;
        r.onlyRead(n);
        char name[32] = {};
        auto n6 = r.ReadArray(name, 32);
        char pw[32] = {};
        auto n7 = r.ReadArray(pw, 32);
        int ata[10] = {};
        auto n8 = r.ReadArray(ata, 10);
        ///
        CRCWriteStream s(128);
        s.setNetCmd(CMD_LOGOUT_RESULT);
        s.WriteInt8(n1);
        s.WriteInt16(n2);
        s.WriteInt32(n3);
        s.WriteFloat(n4);
        s.WriteDouble(n5);
        s.WriteArray(name, n6);
        s.WriteArray(pw, n7);
        s.WriteArray(ata, n8);
        s.finsh();
        pClient->SendData(s.data(), s.length());
    }
    break;
    case CMD_C2S_HEART:
    {
        pClient->resetDTHeart();
        CRCs2c_Heart ret;
        pClient->SendData(&ret);
    }
    default:
    {
        CRCLog_Info("recv <socket=%d> undefine msgType,dataLen：%d", pClient->sockfd(), header->dataLength);
    }
    break;
    }
}