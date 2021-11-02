#ifndef _CRC_TOOLS_SERIAL_H_
#define _CRC_TOOLS_SERIAL_H_

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct serial_s serial_t;

    /**
     * 打开串口，返回串口句柄
     * dev      : 串口路径(串口设备名)
     * baud     : 波特率，1200/2400/4800/9600/14400/19200/38400
     * data     : 数据位
     * stop     : 停止位
     * parity   : 校验位 
     * 
     * */
    serial_t * serial_open(const char* dev, int baud, int data, int stop, char parity);

    /**
     * 串口读取函数，返回读取的字节数
     * 
     * */
    int serial_read(serial_t* serial, char* buf, int size, int us);

    /**
     * 串口发送函数，返回发送的字节数
     * 
     * */
    int serial_write(serial_t* serial, const char* buf, int size);

    /**
     * 串口释放函数，句柄serial内存未释放，需要外部free（serial）
     * 
     * */
    void serial_close(serial_t* serial);

#ifdef __cplusplus
}
#endif

#endif //!_CRC_TOOLS_SERIAL_H_