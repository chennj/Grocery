#include <stdio.h>
#include <stdlib.h>
#include <dvd+rw-mediainfo.h>
#include <blkid/blkid.h>

int main(int argc,char**argv){
	int ret = 0,i;
	SDiskTypeInfo pDiskTypeInfo;

	blkid_probe pr;
	const char *uuid = NULL;


	memset((void*)&pDiskTypeInfo,0,sizeof(pDiskTypeInfo));
	ret = GetMediaInfo(argv[1],&pDiskTypeInfo);
	printf("sizeof %d\n",sizeof(pDiskTypeInfo));
	WriteDiskType("1.info",&pDiskTypeInfo);
	fprintf(stderr,"%d,",pDiskTypeInfo.media_type);
	fprintf(stderr,"%s,",pDiskTypeInfo.media_type_str);          /*检测出来的类型*/
	
	fprintf(stderr,"%s,",pDiskTypeInfo.media_status_str);
	fprintf(stderr,"%d,",pDiskTypeInfo.media_number_session);
	fprintf(stderr,"%s,",pDiskTypeInfo.media_last_session_status);
	fprintf(stderr,"%d,",pDiskTypeInfo.media_number_tracks);
	
	fprintf(stderr,"%d,",pDiskTypeInfo.media_block_size);
	fprintf(stderr,"%s,",pDiskTypeInfo.media_format_status);
	fprintf(stderr,"%llu,",pDiskTypeInfo.media_capacity);
	
	//	fprintf(file,"%s,",pDiskTypeInfo->media_track_state[i]);
	fprintf(stderr,"%llu,",pDiskTypeInfo.media_track_freesize);
	
	
	fprintf(stderr,"%d,",	pDiskTypeInfo.media_writespeed_cnt);
	for (i=0;i<8;i++)
	{
		fprintf(stderr,"%s,",pDiskTypeInfo.media_writespeed_str[i]);
	}

	pr = blkid_new_probe_from_filename(argv[1]);
	if (!pr) {
		err(2, "Failed to open %s", argv[1]);
	}
	
	blkid_do_probe(pr);
	if (blkid_probe_lookup_value(pr, "TYPE", &uuid, NULL) == 0)
	{
		if (uuid)
		{
			printf("type=%s\n", uuid);
		}
		
	}
	
	
	
	blkid_probe_lookup_value(pr, "LABEL", &uuid, NULL);
	
	printf("label=%s\n", uuid);
	
	blkid_free_probe(pr);
}
