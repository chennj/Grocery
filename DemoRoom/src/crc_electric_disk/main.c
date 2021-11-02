#include "crc_tools_serial.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//display 16hex
void print16(const char* data, int num)
{
	int i;
	for (i = 0; i < num; i++) {
		printf("%02x ", *data++);
	}
	printf("\n");
}

//crc16
unsigned short crc16(const char* data, int num)
{
	int i;
	unsigned short crc = 0;
	for (i = 0; i < num; i++) {
		crc += (unsigned short)(*data++);
	}
	return crc;
}

int main(int argc, char **argv)
{
    serial_t * serial;
    int nsend;
    int nrw;
    char order[15];
    char recv[128];

    if (argc < 2){
        return 0;
    }

    serial = serial_open(argv[1], 9600, 8, 1, 'n');

    if (!serial){
        printf("Can't Open Serial Port!\n");
        return 0;
    }
    
    if (argc == 2){             //集中控制
        printf("集中控制\n");
        memset(order, 0, sizeof(order));
        nsend = 15;

        order[0]	= 0x48;
        order[1]	= 0x3a;
        order[2]	= 0x01;
        order[3]	= 0x57;
        order[4]	= 0x00;
        order[5]	= 0x00;
        order[6]	= 0x00;
        order[7]	= 0x00;
        order[8]	= 0x00;
        order[9]	= 0x00;
        order[10]	= 0xff;
        order[11]	= 0xff;
        order[12]	= 0x00;
        order[13]	= 0x45;
        order[14]	= 0x44;

        unsigned short crc = crc16(order, 12);
        order[12] = ((char*)&crc)[0];
        print16(order,15);

        memset(recv, 0, sizeof(recv));
        nrw = serial_write(serial, order, nsend);
    }

    if (argc == 4){         //单独控制
        printf("单路控制\n");
        memset(order, 0, sizeof(order));
        nsend = 10;

        order[0]    = 0x48;
        order[1]    = 0x3a;
        order[2]    = 0x01;
        order[3]    = 0x70;
        order[4]    = atoi(argv[2]);
        order[5]    = atoi(argv[3]);
        order[6]    = 0x00;
        order[7]    = 0x00;
        order[8]    = 0x45;
        order[9]    = 0x44;
        memset(recv, 0, 128);
        nrw = serial_write(serial, order, nsend);
    }

    if (nrw != nsend){
        printf("\nERROR send failed %d\n",nrw);
        serial_close(serial);
        return 0;
    }

    nrw = serial_read(serial, recv, nsend, 1000000 * 10);

    printf("\n--------------------------------\n");
    printf("RET Len %d\n",nrw);
    print16(recv,nrw);

    serial_close(serial);
    return 0;
}