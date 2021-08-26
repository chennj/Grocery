#ifndef _CRC_STREAM_MESSAGE_H_
#define _CRC_STREAM_MESSAGE_H_

#include "crc_message_header.h"
#include "crc_stream.h"

//消息数据字节流
class CRCMessageReader :public CRCStream
{
public:
	CRCMessageReader(CRCDataHeader* header);

	CRCMessageReader(char* pData, int nSize, bool bDelete = false);

	uint16_t getNetCmd();
};

//消息数据字节流
class CRCMessageWriter :public CRCStream
{
public:
	CRCMessageWriter(char* pData, int nSize, bool bDelete = false);

	CRCMessageWriter(int nSize = 1024);

	void setNetCmd(uint16_t cmd);

	bool WriteString(const char* str, int len);

	bool WriteString(const char* str);

	bool WriteString(std::string& str);

	void finsh();
};

#endif //!_CRC_STREAM_MESSAGE_H_