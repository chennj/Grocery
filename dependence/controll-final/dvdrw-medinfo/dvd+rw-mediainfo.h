#ifndef __DVDMEDINFO_H_
#define __DVDMEDINFO_H_


#ifdef __cplusplus       
extern "C"{
#endif

#define MBLANK		0
#define MAPPENEN	1
#define MCOMPLET	2
#define MDAMAGE		3
#define MFORMAT		4
#define MUFORMAT	5



typedef struct _Profile_Desc{
	int profile_type;
	char desc_str[64];
}Profile_Desc;
/* 如果是BD空盘，大小根据media_capacity ,如果是DVD，默认是4G */
typedef struct tagDiskTypeInfo
{
	int			media_type;
	char 		media_type_str[32];          /*检测出来的类型*/
	char        media_id[32];
	char 		media_status_str[16];
	char		media_number_session;
	char		media_last_session_status[16];
	char		media_number_tracks;
	
unsigned int			media_block_size;
	char				media_format_status[16];
unsigned long long		media_capacity;
	
//	char				media_track_state[8][16];
unsigned long long		media_track_freesize;
	
	char		media_writespeed_cnt;
	char		media_writespeed_str[8][32];
	/* blkid get information */
	char		label_name[128];
	/* for mount */
	char	    fs_type[16];
	/*  analyse data  */
	char        media_abilty;

}SDiskTypeInfo;

extern int GetMediaInfo( char* DevName, SDiskTypeInfo* pDiskTypeInfo);
extern int WriteDiskType(char *filename,SDiskTypeInfo *pDiskTypeInfo);
#ifdef __cplusplus	
}
#endif


#endif

