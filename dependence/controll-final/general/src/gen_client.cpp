#include "gen_client.h"
#include "crc_config.h"

GeneralClient::GeneralClient()
{
    _bCheckMsgID = CRCConfig::Instance().hasKey("-checkMsgID");
}

void 
GeneralClient::OnNetMsg(CRCDataHeader* header)
{
    _bSend = false;
    switch (header->cmd)
    {
    case CMD_EQPT_MOVE_RESULT:
    {
        CRCEqptMoveR* mmr = (CRCEqptMoveR*)header;
        if (_bCheckMsgID)
        {
            if (mmr->msgId != _nRecvMsgID)
            {//当前消息ID和本地收消息次数不匹配
                CRCLog_Error("OnNetMsg socket<%d> msgID<%d> _nRecvMsgID<%d> %d", _pClient->sockfd(), mmr->msgId, _nRecvMsgID, mmr->msgId - _nRecvMsgID);
            }
            ++_nRecvMsgID;
        }
        CRCLog_Info("<socket=%d> recv CMD_MOVE_RESULT code<%s> ", (int)_pClient->sockfd(), mmr->code);
    }
    break;
    case CMD_LOGOUT_RESULT:
    {
        CRCLogoutR* logout = (CRCLogoutR*)header;
        //CRCLog_Info("<socket=%d> recv msgType：CMD_LOGOUT_RESULT", (int)_pClient->sockfd());
    }
    break;
    case CMD_NEW_USER_JOIN:
    {
        CRCNewUserJoin* userJoin = (CRCNewUserJoin*)header;
        //CRCLog_Info("<socket=%d> recv msgType：CMD_NEW_USER_JOIN", (int)_pClient->sockfd());
    }
    break;
    case CMD_ERROR:
    {
        CRCLog_Info("<socket=%d> recv msgType：CMD_ERROR", (int)_pClient->sockfd());
    }
    break;
    default:
    {
        CRCLog_Info("error, <socket=%d> recv undefine msgType", (int)_pClient->sockfd());
    }
    }
}

int 
GeneralClient::SendTest(CRCEqptMove* mmove, bool bChekSendBack)
{
    int ret = 0;
    //如果剩余发送次数大于0
    if (_nSendCount > 0 && !_bSend)
    {
        mmove->msgId = _nSendMsgID;
        ret = SendData(mmove);
        //CRCLog_Info("%d", _nSendMsgID);
        if (SOCKET_ERROR != ret)
        {
            _bSend = bChekSendBack;
            ++_nSendMsgID;
            //如果剩余发送次数减少一次
            --_nSendCount;
        }
    }
    return ret;
}

bool 
GeneralClient::checkSend(time_t dt)
{
    _tRestTime += dt;
    //每经过nSendSleep毫秒
    if (_tRestTime >= _nSendSleep)
    {
        //重置计时
        _tRestTime -= _nSendSleep;
        //重置发送计数
        _nSendCount = _nMsg;
    }
    return _nSendCount > 0;
}

void 
GeneralClient::debug(int nSendSleep, int nMsg)
{
    _nSendSleep = nSendSleep;
    _nMsg = nMsg;
}

void
GeneralClient::Loop()
{
    _threadLoop.Start(
        nullptr, 
        [this](CRCThread* pThread) {
			while (pThread->isRun())
            {
                if (!OnRun(0)){
                    Close();
                    Connect(_ip, _port);
                } else {
                    CRCThread::Sleep(0);
                }
            }
		}
    );
}

void 
GeneralClient::PushMessage(SOCKET srcSock, CRCDataHeader* header)
{
    std::lock_guard<std::mutex> lock(_mutexSendMsg);
    header->sessionId = srcSock;
    _machineSendMessages.push(header);
}

void 
GeneralClient::Send2Stm32Loop()
{
    _threadSendCmd.Start(
        nullptr, 
        [this](CRCThread* pThread) {
			while (pThread->isRun())
            {
                if (!_machineSendMessages.empty())
                {
                    std::lock_guard<std::mutex> lock(_mutexSendMsg);
                    CRCDataHeader* header = _machineSendMessages.front();
                    _machineSendMessages.pop();
                    switch(header->cmd)
                    {
                        case CMD_EQPT_MOVE:{
                            CRCEqptMoveR* mmove = (CRCEqptMoveR*)header;
                            SendData(mmove);
                            break;
                        }
                    }
                } else {
                    CRCThread::Sleep(0);
                }
            }
		},
        nullptr
    );
}

CRCDataHeader* 
GeneralClient::popRecvMsg()
{
    if (!_machineRecvMessages.empty()){
        std::lock_guard<std::mutex> lock(_mutexRecvMsg);
        CRCDataHeader* header = _machineRecvMessages.front();
        _machineRecvMessages.pop();
        return header;
    }
    return nullptr;
}