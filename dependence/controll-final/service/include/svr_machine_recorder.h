#ifndef _SVR_MACHINE_RECORDER_H_
#define _SVR_MACHINE_RECORDER_H_

#include <time.h> 

typedef struct _RecordAttr{
	char    is_pluging;
	char    is_have_media;
	int     media_address;
	char    is_busy;
	
	int     record_address;
	char    dev_name[16];
	char    is_mount;
	char    mnt_dir[256];
	int     lock_num;
	time_t  times;
    char    burning_label[256];
}RecordAttr;

#endif //!_SVR_MACHINE_RECORDER_H_