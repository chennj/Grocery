#include "../../include/crc_stream.h"

CRCStream::CRCStream(int nSize)
{
    _nSize = nSize;
    _pBuff = new char[_nSize];
    _bDelete = true;
}

CRCStream::CRCStream(char* pData, int nSize, bool bDelete)
{
    _nSize = nSize;
    _pBuff = pData;
    _bDelete = bDelete;    
}

CRCStream::~CRCStream()
{
    if (_bDelete && _pBuff)
    {
        delete[] _pBuff;
        _pBuff = nullptr;
    }    
}

int8_t
CRCStream::ReadInt8(int8_t def)
{
    Read(def);
    return def;    
}

int16_t
CRCStream::ReadInt16(int16_t def)
{
    Read(def);
    return def;    
}

int32_t
CRCStream::ReadInt32(int32_t def)
{
    Read(def);
    return def;    
}

int64_t
CRCStream::ReadInt64(int64_t def)
{
    Read(def);
    return def;    
}

uint8_t
CRCStream::ReadUInt8(uint8_t def)
{
    Read(def);
    return def;    
}

uint16_t
CRCStream::ReadUInt16(uint16_t def)
{
    Read(def);
    return def;    
}

uint32_t
CRCStream::ReadUInt32(uint32_t def)
{
    Read(def);
    return def;    
}

uint64_t
CRCStream::ReadUInt64(uint64_t def)
{
    Read(def);
    return def;    
}

float
CRCStream::ReadFloat(float n)
{
    Read(n);
    return n;    
}

double
CRCStream::ReadDouble(double n)
{
    Read(n);
    return n;    
}

bool
CRCStream::ReadString(std::string& str)
{
    uint32_t nLen = 0;
    Read(nLen, false);
    if (nLen > 0)
    {
        //判断能不能读出
        if (canRead(nLen + sizeof(uint32_t)))
        {
            //计算已读位置+数组长度所占有空间
            pop(sizeof(uint32_t));
            //将要读取的数据 拷贝出来
            str.insert(0, _pBuff + _nReadPos, nLen);
            //计算已读数据位置
            pop(nLen);
            return true;
        }
    }
    return false;    
}

bool 
CRCStream::WriteInt8(int8_t n)
{
    return Write(n);
}

bool 
CRCStream::WriteInt16(int16_t n)
{
    return Write(n);
}

bool 
CRCStream::WriteInt32(int32_t n)
{
    return Write(n);
}

bool 
CRCStream::WriteFloat(float n)
{
    return Write(n);
}

bool 
CRCStream::WriteDouble(double n)
{
    return Write(n);
}