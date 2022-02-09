/**
 * 
 * author:  chenningjiang
 * desc:    字符串比较
 * 
 * */
#ifndef _CRC_KEYSTRING_HPP_
#define _CRC_KEYSTRING_HPP_

#include <cstring>

namespace CRCIO {

    class CRCKeyString
    {
    private:
        const char* _str = nullptr;
    public:
        CRCKeyString(const char* str)
        {
            set(str);
        }

        inline void set(const char* str)
        {
            _str = str;
        }

        inline const char* get()
        {
            return _str;
        }

        //bool operator < (const KeyString& right)
        //{
        //	return strcmp(this->_str, right._str) < 0;
        //}

        inline friend bool operator < (const CRCKeyString& left, const CRCKeyString& right);
    };

    bool operator < (const CRCKeyString& left, const CRCKeyString& right)
    {
        return strcmp(left._str, right._str) < 0;
    }
}

//bool operator < (const KeyString& left, const KeyString& right)
//{
//	return left < right;
//}
#endif //!_CRC_KEYSTRING_HPP_