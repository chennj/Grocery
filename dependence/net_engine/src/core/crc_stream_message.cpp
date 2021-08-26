#include "../../include/core/crc_stream_message.h"

///////////////////////////////////////////////////////////////////////////
// reader 
///////////////////////////////////////////////////////////////////////////
CRCMessageReader::CRCMessageReader(CRCDataHeader* header)
		:CRCMessageReader((char*)header, header->dataLength)
{}

CRCMessageReader::CRCMessageReader(char* pData, int nSize, bool bDelete)
		:CRCStream(pData, nSize, bDelete)
{
    push(nSize);
}

uint16_t 
CRCMessageReader::getNetCmd()
{
    uint16_t cmd = CMD_ERROR;
    Read<uint16_t>(cmd);
    return cmd;    
}

///////////////////////////////////////////////////////////////////////////
// writer 
///////////////////////////////////////////////////////////////////////////
CRCMessageWriter::CRCMessageWriter(char* pData, int nSize, bool bDelete)
		:CRCStream(pData, nSize, bDelete)
{
    //预先占领消息长度所需空间
    Write<uint16_t>(0);    
}

CRCMessageWriter::CRCMessageWriter(int nSize)
		:CRCStream(nSize)
{
    //预先占领消息长度所需空间
    Write<uint16_t>(0);    
}

void 
CRCMessageWriter::setNetCmd(uint16_t cmd)
{
    Write<uint16_t>(cmd);
}

bool 
CRCMessageWriter::WriteString(const char* str, int len)
{
    return WriteArray(str, len);
}

bool 
CRCMessageWriter::WriteString(const char* str)
{
    return WriteArray(str, strlen(str));
}

bool 
CRCMessageWriter::WriteString(std::string& str)
{
    return WriteArray(str.c_str(), str.length());
}

void 
CRCMessageWriter::finsh()
{
    int pos = length();
    setWritePos(0);
    Write<uint16_t>(pos);
    setWritePos(pos);
}