/**
 * 
 * author:  chenningjiang
 * desc:    字节流读写
 * 
 * */
#ifndef _CRC_MSG_STREAM_H_
#define _CRC_MSG_STREAM_H_

#include "crc_msg_header.h"
#include "crc_stream.h"

//消息数据字节流
class CRCReadStream : public CRCStream
{
public:
	CRCReadStream(CRCDataHeader* header)
		:CRCReadStream((char*)header, header->dataLength)
	{

	}

	CRCReadStream(char* pData, int nSize, bool bDelete = false)
		:CRCStream(pData, nSize, bDelete)
	{
		push(nSize);
		////预先读取消息长度
		//ReadInt16();
		////预先读取消息命令
		//getNetCmd();
	}

	uint16_t getNetCmd()
	{
		uint16_t cmd = CMD_ERROR;
		Read<uint16_t>(cmd);
		return cmd;
	}
};

//消息数据字节流
class CRCWriteStream :public CRCStream
{
public:
	CRCWriteStream(char* pData, int nSize, bool bDelete = false)
		:CRCStream(pData, nSize, bDelete)
	{
		//预先占领消息长度所需空间
		Write<uint16_t>(0);
	}

	CRCWriteStream(int nSize = 1024)
		:CRCStream(nSize)
	{
		//预先占领消息长度所需空间
		Write<uint16_t>(0);
	}

	void setNetCmd(uint16_t cmd)
	{
		Write<uint16_t>(cmd);
	}

	bool WriteString(const char* str, int len)
	{
		return WriteArray(str, len);
	}

	bool WriteString(const char* str)
	{
		return WriteArray(str, strlen(str));
	}

	bool WriteString(std::string& str)
	{
		return WriteArray(str.c_str(), str.length());
	}

	void finsh()
	{
		int pos = length();
		setWritePos(0);
		Write<uint16_t>(pos);
		setWritePos(pos);
	}
};


#endif // !_CRC_MSG_STREAM_H_
