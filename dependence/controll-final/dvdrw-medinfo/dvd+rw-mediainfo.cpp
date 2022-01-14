/*
* DVD Media Info utility by Andy Polyakov <appro@fy.chalmers.se>.
*
* This code is in public domain.
*/

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "transport.h"
#include <dvd+rw-mediainfo.h>
#include <sys/vfs.h>
#include <blkid/blkid.h>
//#include "../xstorage/include/nlog.h"
//#include "../xstorage/include/nlog.h"
//#include "../xstorage/include/nlog.h"

#ifdef _WIN32
#define LLU "I64u"
#else
#define LLU "llu"
#endif

#define MAXPROFILE  37


static Profile_Desc  profile_desc[MAXPROFILE] = {
	{0x0, "No current profile"},
	{0x1, "Non-removable disk (obs)"},
	{0x2, "Removable disk"},
	{0x3, "Magneto optical erasable"},
	{0x4, "Optical write once"},
	{0x5, "AS-MO"},
	{0x8, "CD-ROM"},
	{0x9, "CD-R"},
	{0xa, "CD-RW"},
	{0x10, "DVD-ROM"},
	{0x11, "DVD-R sequential recording"},
	{0x12, "DVD-RAM"},
	{0x13, "DVD-RW restricted overwrite"},
	{0x14, "DVD-RW sequential recording"},
	{0x15, "DVD-R dual layer sequental recording"},
	{0x16, "DVD-R dual layer jump recording"},
	{0x17, "DVD-RW dual layer"},
	{0x18, "DVD-Download disc recording"},
	{0x1a, "DVD+RW"},
	{0x1b, "DVD+R"},
	{0x20, "DDCD-ROM"},
	{0x21, "DDCD-R"},
	{0x22, "DDCD-RW"},
	{0x2a, "DVD+RW dual layer"},
	{0x2b, "DVD+R dual layer"},
	{0x40, "BD-ROM"},
	{0x41, "BD-R SRM"},
	{0x42, "BD-R RRM"},
	{0x43, "BD-RE"},
	{0x50, "HD DVD-ROM"},
	{0x51, "HD DVD-R"},
	{0x52, "HD DVD-RAM"},
	{0x53, "HD DVD-RW"},
	{0x58, "HD DVD-R dual layer"},
	{0x5a, "HD DVD-RW dual layer"},
	{0xffff, "Non-conforming profile"},
	{-1, NULL}
};


char* _find_profile_str(int profile){
	int i = 0;
	for (i = 0;i<MAXPROFILE;i++)
	{
		if (profile_desc[i].profile_type == profile)
		{
			return strdup(profile_desc[i].desc_str);
		}
	}
	return NULL;
}

#define SPERROR(a,b) printf("%s,%s\n",a,b); sperror(a,b);

int _GetMediaInfo(char* DevName, SDiskTypeInfo* pDiskTypeInfo)
{ 
	int argc = 2;
	
	Scsi_Command	cmd;
	unsigned char	header[48],inq[128],*page2A=NULL;
	char		cmdname[64];
	int		i,j,ntracks,err,dvd_dash=0,dvd_plus=0,
		plus_mediaid_printed=0,page2A_printed=0,dvd_ram_spare=0;
	unsigned short profile,
		dvd_0E=0,dvd_10=0,dvd_11=0,dvd_0A=0,dvd_C0=0,
		prf_23=0,prf_24=0,prf_38=0;
	int		_1x=1385,blu_ray=0;
	
    if (pDiskTypeInfo==NULL){
		fprintf (stdout,"usage: %s \n",DevName);
		return -1;
	}
	
	
    if (!cmd.associate(DevName)){
		fprintf (stdout,"%s: unable to open: ",DevName);
		perror (NULL);
		return -1;
	}
		
	
    cmd[0] = 0x12;	// INQUIRY
    cmd[4] = 36;
    cmd[5] = 0;
    if ((err=cmd.transport(READ,inq,36))){
		SPERROR ("INQUIRY",err);
		return -1;
	}
	
    if ((inq[0]&0x1F) != 5){
		fprintf (stdout,":-( not an MMC unit!\n");
		return -1;
	}
	
   /* if (argc>1)
		printf ("INQUIRY:                [%.8s][%.16s][%.4s]\n",
		inq+8,inq+16,inq+32);*/
	
#if 0
    wait_for_unit(cmd);
#else
    // consume sense data, most commonly "MEDIUM MAY HAVE CHANGED"...
    cmd[0] = 0;		// TEST UNIT READY
    cmd[5] = 0;
    if ((err=cmd.transport()) == -1){
		SPERROR ("TEST UNIT READY",err);
		return -1;
	}
#endif
	
    if (argc>2) do
    { 
		unsigned char *pages;
		int len,n;
		static int pagecode=0x3F;
		
	//	printf ("MODE SENSE[#%02Xh]:\n",pagecode);
		
		cmd[0] = 0x5A;		// MODE SENSE
		cmd[1] = 0x08;		// "Disable Block Descriptors"
		cmd[2] = pagecode;	// initially "All Pages"
		cmd[8] = 12;
		cmd[9] = 0;
		if ((err=cmd.transport(READ,header,12))){
			sprintf (cmdname,"MODE SENSE#%02X",pagecode),
			SPERROR (cmdname,err);
			return -1;
		}
		
		if (header[6]<<8|header[7]){
			fprintf (stdout,":-( \"DBD\" is not respected\n");
			return -1;
		}
		
		len=(header[0]<<8|header[1])+2;
		
		if (len < (8+2+header[9]))
		{   
			if (pagecode == 0x3F)
			{
				pagecode = 0x5;
				continue;
			}
			
			len = 8+2+header[9];
		}
		
		if (!(pages=(unsigned char *)malloc(len))) {
			fprintf (stdout,":-( no memery for %d\n", len);
			return -1;
		}
		
		cmd[0] = 0x5A;
		cmd[1] = 0x08;
		cmd[2] = pagecode;
		cmd[7] = len>>8;
		cmd[8] = len;
		cmd[9] = 0;
		cmd.transport(READ,pages,len);
		
		for(i=8+(pages[6]<<8|pages[7]);i<len;)
		{   
			if (pages[i] == 0)	break;
			
			if (pages[i] == 0x2A) page2A_printed=1;
		//	printf (" %02X:",pages[i++]);
			n=pages[i++];
			for (j=0;j<n;j++)
			{	
				//if (j && j%16==0) printf("\n");
				//printf ("%c%02x",(j%16)?' ':'\t',pages[i++]);
			}
			//printf("\n");
		}
		
		if (i<len)
		{  
			//printf (" RES:");
			for (len-=i,j=0;j<len;j++)
			{	
				//if (j && j%16==0) printf("\n");
				//printf ("%c%02x",(j%16)?' ':'\t',pages[i++]);
			}
			//printf ("\n");
		}   
		
		if(pages){ //V3.2.1 20180614 //segfault
			free(pages);
			pages=NULL;
		}
		
		break;
		
    } while (1);
	
    { 
		int len,n;
		
		page2A=header, len=36;
		
		while (1)
		{   
			cmd[0] = 0x5A;	// MODE SENSE(10)
			cmd[1] = 0x08;	// "Disable Block Descriptors"
			cmd[2] = 0x2A;	// "Capabilities and Mechanical Status"
			cmd[7] = len>>8;
			cmd[8] = len;
			cmd[9] = 0;
			if ((err=cmd.transport(READ,page2A,len))){
				SPERROR ("MODE SENSE#2A",err);
				return -1;
			}
			
			if (page2A[6]<<8|page2A[7])
				fprintf (stdout,":-( \"DBD\" is not respected\n");
			else if (len<((page2A[0]<<8|page2A[1])+2) || len<(8+2+page2A[9]))
			{	len=(page2A[0]<<8|page2A[1])+2;
			if (len < (8+2+page2A[9]))
				len = 8+2+page2A[9];
			page2A=(unsigned char *)malloc(len);
			continue;
			}
			
			if (!(page2A[8+(page2A[6]<<8|page2A[7])+2]&8)){
				fprintf (stdout,":-( not a DVD unit\n");
				return -1;
			}
			
			if (page2A==header) page2A=NULL;
			else if (argc>2 && !page2A_printed)
			{	
				//printf("MODE SENSE[#2A]:\n");
				for(i=8+(page2A[6]<<8|page2A[7]);i<len;)
				{   
					//printf (" %02X:",page2A[i++]);
					n=page2A[i++];
					for (j=0;j<n;j++)
					{	
						//if (j && j%16==0) printf("\n");
						//printf ("%c%02x",(j%16)?' ':'\t',page2A[i++]);
					}
					//printf("\n");
				}
			}
			
			break;
		}
    }
	
    cmd[0] = 0x46;	// GET CONFIGURATION
    cmd[1] = 1;
    cmd[8] = 8;
    cmd[9] = 0;
    if ((err=cmd.transport(READ,header,8)))
    {
		if (err!=0x52000){	// "IVALID COMMAND OPERATION CODE"
			SPERROR ("GET CONFIGURATION",err);
			perror (NULL);
			return -1;
		}
		profile=0x10;	// non MMC3 DVD-ROM unit?
		goto legacy;
    }
	
    profile=header[6]<<8|header[7];
	
    do
    { 
		unsigned char *profiles;
		int len,n;
		
		len=header[2]<<8|header[3];
		len+=4;
		
		if (!(profiles=(unsigned char *)malloc(len))) {
			fprintf (stdout,":-( no memery for %d\n", len);
			return -1;
		}
		
		cmd[0] = 0x46;
		cmd[1] = 1;
		cmd[6] = len>>16;
		cmd[7] = len>>8;
		cmd[8] = len;
		cmd[9] = 0;
		cmd.transport(READ,profiles,len);
		
		//printf ("GET [CURRENT] CONFIGURATION:\n");
		
		for(i=8;i<len;)
		{  
			unsigned short p=profiles[i]<<8|profiles[i+1];
			n=profiles[i+3]; i+=4;
			if      (p==0x23)	prf_23=n+4;
			else if (p==0x24)	prf_24=n+4;
			else if (p==0x38)	prf_38=n+4;
			if (argc<=2)
			{	
				i+=n; continue;   
			}
		//	printf (" %04X%c",p,':');
			for (j=0;j<n;j++)
			{
			//	if (j && j%16==0) printf("\n");
			//	printf ("%c%02x",(j%16)?' ':'\t',profiles[i++]);
			}
			//printf("\n");
		}//end for
		
			if(profiles){
				free(profiles);
				profiles=NULL;  ////V3.2.1 20180614 //segfault
			}
    } while(0);  //这个循环就执行一次
	
    if (profile==0){
		fprintf (stdout,":-( no media mounted, exiting...\n");
		return -1;
	}
	/* if ((profile&0xF0) != 0x10 && (profile&0xF0) != 0x20 &&
	(profile&0xF0) != 0x40)
	fprintf (stdout,":-( non-DVD media mounted, exiting...profile %x\n",profile),
	exit (FATAL_START(EMEDIUMTYPE));*/
	
	/* { 
	const char *str;
	
	 switch (profile)
	 {   case 0x10:	str="DVD-ROM";				break;
	 case 0x11:	str="DVD-R Sequential";			break;
	 case 0x12:	str="DVD-RAM";				break;
	 case 0x13:	str="DVD-RW Restricted Overwrite";	break;
	 case 0x14:	str="DVD-RW Sequential";		break;
	 case 0x15:	str="DVD-R Dual Layer Sequential";	break;
	 case 0x16:	str="DVD-R Dual Layer Jump";		break;
	 case 0x1A:	str="DVD+RW";				break;
	 case 0x1B:	str="DVD+R";				break;
	 case 0x2A:	str="DVD+RW Double Layer";		break;
	 case 0x2B:	str="DVD+R Double Layer";		break;
	 case 0x40:	str="BD-ROM";				break;
	 case 0x41:	str=prf_38?"BD-R SRM+POW":"BD-R SRM";	break;
	 case 0x42:	str="BD-R RRM";				break;
	 case 0x43:	str="BD-RE";				break;
	 default:	str="unknown";				break;
		}*/
	{
		const char *str;
		str = _find_profile_str(profile);		
		
		pDiskTypeInfo->media_type = profile;
		memset(pDiskTypeInfo->media_type_str,0,32);        //V3.2.1 20180614
		strncpy(pDiskTypeInfo->media_type_str,str,31);   //V3.2.1 20180614
		
	//	printf (" Mounted Media:         %Xh, %s\n",profile,str);
		if (str[3]=='-' && str[6]!='M')	dvd_dash=0x10;
		if (str[3]=='+')		dvd_plus=0x11;
    }
	
	
    if ((profile&0xF0) == 0x40)	_1x=4495, blu_ray=1;
	
    do
    { 
		unsigned int   len,j;
		unsigned char *p;
		
		cmd[0] = 0xAD;
		cmd[1] = blu_ray;	// Media Type
		cmd[7] = 0xFF;
		cmd[9] = 4;
		cmd[11] = 0;
		if ((err=cmd.transport(READ,header,4)))
		{   if (argc>2)
		SPERROR (blu_ray?"READ BD STRUCTURE#FF":"READ DVD STRUCTURE#FF",err);
		break;
		}
		
		len = (header[0]<<8|header[1]) + 2;
		if (!(p = (unsigned char *)malloc(len))) break;
		
		cmd[0] = 0xAD;
		cmd[1] = blu_ray;	// Media Type
		cmd[7] = 0xFF;
		cmd[8] = len>>8;
		cmd[9] = len;
		cmd[11] = 0;
		if ((err=cmd.transport(READ,p,len)))
			break;
		
		for (j=4;j<len;j+=4)
			switch (p[j])
		{	case 0x0A:  dvd_0A=(p[j+2]<<8|p[j+3])+2;    break;
		case 0x0E:  dvd_0E=(p[j+2]<<8|p[j+3])+2;    break;
		case 0x10:  dvd_10=(p[j+2]<<8|p[j+3])+2;    break;
		case 0x11:  dvd_11=(p[j+2]<<8|p[j+3])+2;    break;
		case 0xC0:  dvd_C0=(p[j+2]<<8|p[j+3])+2;    break;
		}
		
		if (argc<=2) break;
		
	//	printf ("READ %s STRUCTURE[#FF]:\n",blu_ray?"BD":"DVD");
	//	for (j=4;j<len;j+=4)
	//		printf(" STRUCTURE#%02x           %02x:%d\n",p[j],p[j+1],p[j+2]<<8|p[j+3]);
		
    } while (0);
	
    if (blu_ray) do
    {
		unsigned char di[128];
		unsigned int  len;
		unsigned char format=0;
		
		cmd[0] = 0xAD;
		cmd[1] = blu_ray;	// Media Type
		cmd[7] = format;
		cmd[9] = 4;
		cmd[11] = 0;
		if ((err=cmd.transport(READ,header,4)))
		{  
			if (argc>2)
				sprintf (cmdname,"READ BD STRUCTURE#%02x",format),
				SPERROR (cmdname,cmd.sense());
			break;
		}
		
		len = (header[0]<<8|header[1]) + 2;
		if (len>sizeof(di)) len=sizeof(di);
		
		cmd[0] = 0xAD;
		cmd[1] = blu_ray;	// Media Type
		cmd[7] = format;
		cmd[8] = len>>8;
		cmd[9] = len;
		cmd[11] = 0;
		if ((err=cmd.transport(READ,&di,len)))
			break;
		
		if (argc>2)
		{   
			/*printf ("READ BD STRUCTURE[#%02xh]:",format);
			for (j=0;j<(int)len;j++)
				printf("%c%02x",j?' ':'\t',di[j]);
			printf("\n"); */
		}
		/*
		if (di[4+0]=='D' && di[4+1]=='I')
			printf (" Media ID:              %6.6s/%-3.3s\n",di+4+100,di+4+106);
			*/
		sprintf(pDiskTypeInfo->media_id,"%6.6s/%-3.3s",di+4+100,di+4+106);
		
    } while (0);
    else if (dvd_0E || dvd_11) do
    { 
		union { unsigned char _e[4+40],_11[4+256]; } dvd;
		unsigned int  len;
		unsigned char format=dvd_plus?0x11:0x0E;
		
		cmd[0] = 0xAD;
		cmd[7] = format;
		cmd[9] = 4;
		cmd[11] = 0;
		if ((err=cmd.transport(READ,header,4)))
		{  
			if (argc>2)
				sprintf (cmdname,"READ DVD STRUCTURE#%02x",format),
				SPERROR (cmdname,cmd.sense());
			break;
		}
		
		len = (header[0]<<8|header[1]) + 2;
		if (len>sizeof(dvd)) len=sizeof(dvd);
		
		cmd[0] = 0xAD;
		cmd[7] = format;
		cmd[8] = len>>8;
		cmd[9] = len;
		cmd[11] = 0;
		if ((err=cmd.transport(READ,&dvd,len)))
			break;
		
		if (argc>2)
		{  
			/*printf ("READ DVD STRUCTURE[#%02xh]:",format);
			for (j=0,i=dvd_dash?sizeof(dvd._e):sizeof(dvd._11);j<i;j++)
				printf("%c%02x",j?' ':'\t',dvd._11[j]);
			printf("\n");*/
		}
		/*
		if (!dvd_plus && dvd_0E && dvd._e[4+16]==3 && dvd._e[4+24]==4)
			printf (" Media ID:              %6.6s%-6.6s\n",dvd._e+4+17,dvd._e+4+25);
		*/
		if (dvd_plus && dvd_11){
		//	printf (" Media ID:              %.8s/%.3s\n",dvd._11+23,dvd._11+31),
			plus_mediaid_printed=1;
		}
		
    } while (0);
	
    if (page2A) do
    { 
		int len,hlen,v,n=0;
		unsigned char *p;
		
		len  = (page2A[0]<<8|page2A[1])+2;
		hlen = 8+(page2A[6]<<8|page2A[7]);
		if (len<(hlen+30)) break;	// no "Current Write Speed"
		
		p = page2A+hlen;
		
		v = p[28]<<8|p[29];
		// Check Write Speed descriptors for sanity. Some DVD+units
		// return CD-R descriptors here, which has no meaning in the
		// context of interest.
		if (v<1352)	break;
		if (len>=(hlen+32))
		{  
			n = (p[30]<<8|p[31])*4;
			for (i=0;i<n;i+=4)
				if ((p[32+i+2]<<8|p[32+i+3]) < 1352) break;
				if (i<n) break;
		}
		
		//printf (" Current Write Speed:   %.1fx%d=%dKB/s\n",(double)v/_1x,_1x,v);
		
		if (len<(hlen+32)) break;	// no "Write Speed Descriptors"
		
		pDiskTypeInfo->media_writespeed_cnt = n/4;
		for (i=0;i<n;i+=4){
			v = p[32+i+2]<<8|p[32+i+3];
		//	printf (" Write Speed #%d:        %.1fx%d=%dKB/s\n",i/4,(double)v/_1x,_1x,v);
			if (i/4 < 8)
			{
				sprintf(pDiskTypeInfo->media_writespeed_str[i/4],"%.1fx%d=%dKB/s",(double)v/_1x,_1x,v);
				//fprintf(stdout,"%.1fx%d=%dKB/s\n",(double)v/_1x,_1x,v);
			}
		}
		
    } while (0);
	
    do
    { 
		unsigned char d[8+16],*p;
		unsigned int  len,rv,wv,i;
		
		cmd[0]=0xAC;		// GET PERFORMANCE
		cmd[1]=4;		// ask for "Overall Write Performance"
		cmd[9]=1;		// start with one descriptor
		cmd[10]=0;		// ask for descriptor in effect
		cmd[11]=0;
		if ((err=cmd.transport(READ,d,sizeof(d))))
		{   if (argc>2)
		SPERROR ("GET CURRENT PERFORMACE",err);
		break;
		}
		
		len = (d[0]<<24|d[1]<<16|d[2]<<8|d[3])-4;
		
		if (len%16)	// insane length
		{   if (argc>2)
		fprintf (stdout,":-( insane GET PERFORMANCE length %u\n",len);
		break;
		}
		
		len+=8;
		if (len < sizeof(d))
		{  
			if (argc>2)
				fprintf (stdout,":-( empty GET CURRENT PERFORMACE descriptor\n");
			break;
		}
		if (len == sizeof(d))
			p = d;
		else	// ever happens?
		{ unsigned int n=(len-8)/16;
		
		p = (unsigned char *)malloc(len);	// will leak...
		
		cmd[0]=0xAC;	// GET PERFORMANCE
		cmd[1]=4;		// ask for "Overall Write Performance"
		cmd[8]=n>>8;
		cmd[9]=n;		// real number of descriptors
		cmd[10]=0;		// ask for descriptor in effect
		cmd[11]=0;
		if ((err=cmd.transport(READ,p,len)))
		{
			SPERROR ("GET CURRENT PERFORMANCE",err);
			break;
		}
		}
		
		if (argc>2)
		{  
			/*
			printf ("GET CURRENT PERFORMANCE:\t");
			for (i=4;i<len;i++) printf ("%02x ",p[i]);
			printf ("\n");*/
		}
		
		if ((p[4]&2) == 0)
			fprintf (stdout,":-( reported write performance might be bogus\n");
		
		if (argc<=2) printf ("GET [CURRENT] PERFORMANCE:\n");
		
		for (p+=8,len-=8,i=0;len;p+=16,len-=16,i++)
		{  
			rv=p[4]<<24 |p[5]<<16 |p[6]<<8 |p[7],
				wv=p[12]<<24|p[13]<<16|p[14]<<8|p[15];
			/*if (rv==wv)
				printf (" %-23s%.1fx%d=%uKB/s@[%u -> %u]\n",
				i==0?"Write Performance:":"",
				(double)rv/_1x,_1x,rv,
				p[0]<<24|p[1]<<16|p[2]<<8 |p[3],
				p[8]<<24|p[9]<<16|p[10]<<8|p[11]);
			else
				printf (" %-23s%.1fx%d=%uKB/s@%u -> %.1fx%d=%uKB/s@%u\n",
				i==0?"Write Performance:":"",
				(double)rv/_1x,_1x,rv,p[0]<<24|p[1]<<16|p[2]<<8 |p[3],
				(double)wv/_1x,_1x,wv,p[8]<<24|p[9]<<16|p[10]<<8|p[11]);*/
		}
    } while (0);
	
    do
    { 
		unsigned char d[8+16],*p;
		unsigned int  len,rv,wv,i;
		
		cmd[0]=0xAC;		// GET PERFORMANCE
		cmd[9]=1;		// start with one descriptor
		cmd[10]=0x3;		// ask for "Write Speed Descriptor"
		cmd[11]=0;
		if ((err=cmd.transport(READ,d,sizeof(d))))
		{   if (argc>2)
		SPERROR ("GET PERFORMACE",err);
		break;
		}
		
		len = (d[0]<<24|d[1]<<16|d[2]<<8|d[3])-4;
		
		if (len%16)	// insage length
		{   
			if (argc>2)
				fprintf (stdout,":-( insane GET PERFORMANCE length %u\n",len);
			break;
		}
		
		len+=8;
		if (len < sizeof(d))
		{   
			fprintf (stdout,":-( empty GET PERFORMACE descriptor\n");
			break;
		}
		if (len == sizeof(d))
			p = d;
		else
		{ 
			unsigned int n=(len-8)/16;
			
			p = (unsigned char *)malloc(len);	// will leak...
			
			cmd[0]=0xAC;	// GET PERFORMANCE
			cmd[8]=n>>8;
			cmd[9]=n;		// real number of descriptors
			cmd[10]=0x3;	// ask for "Write Speed Descriptor"
			cmd[11]=0;
			if ((err=cmd.transport(READ,p,len)))
			{
				SPERROR ("GET PERFORMANCE",err);
				break;
			}
		}
		
		if (argc>2)
		{   
			/*printf ("GET PERFORMANCE:\t");
			for (i=8;i<len;i++) printf ("%02x ",p[i]);
			printf ("\n");*/
		}
		
		for (p+=8,len-=8,i=0;len;p+=16,len-=16,i++){
			rv=p[8]<<24 |p[9]<<16 |p[10]<<8|p[11];
			wv=p[12]<<24|p[13]<<16|p[14]<<8|p[15];

			/*printf (" Speed Descriptor#%d:    %02x/%u R@%.1fx%d=%uKB/s W@%.1fx%d=%uKB/s\n",
			i,p[0],p[4]<<24|p[5]<<16|p[6]<<8|p[7],
			(double)rv/_1x,_1x,rv,(double)wv/_1x,_1x,wv);*/
		}
    } while (0);
	
legacy:
	
    if (!blu_ray) do
    { 
		unsigned char book=header[4];
		const char *brand;
		unsigned int phys_start,phys_end;
		
		header[4]=0;
		cmd[0] = 0xAD;	// READ DVD STRUCTURE
		cmd[7] = dvd_dash;
		cmd[9] = 36;
		cmd[11] = 0;
		if ((err=cmd.transport(READ,header,36)))
		{ 
			if (dvd_dash) { dvd_dash=0; goto legacy; }
			if (err!=0x52400 || argc>2)
				sprintf (cmdname,"READ DVD STRUCTURE#%X",dvd_dash),
				SPERROR (cmdname,cmd.sense());
			break;
		}
		
	//	printf("READ DVD STRUCTURE[#%Xh]:",dvd_dash<0?0:dvd_dash);
		if (argc>2)
			//for (j=0;j<20-4;j++) printf("%c%02x",j?' ':'\t',header[4+j]);
			//printf("\n");
			switch(book&0xF0)
			{   case 0x00:	brand="-ROM";	break;
			case 0x10:	brand="-RAM";	break;
			case 0x20:	brand="-R";	break;
			case 0x30:	brand="-RW";	break;
			case 0x90:	brand="+RW";	break;
			case 0xA0:	brand="+R";	break;
			case 0xE0:	brand="+R DL";	break;
			default:	brand=NULL;	break;
			}
			/*printf (" Media Book Type:       %02Xh, ",book);
			if (brand)  printf ("DVD%s book [revision %d]\n",
				brand,book&0xF);
			else	    printf ("unrecognized value\n");
			*/
			if (header[4+1]&0xF0)	dvd_ram_spare=5120;
			else			dvd_ram_spare=12800;
			/*
			if (dvd_plus && !plus_mediaid_printed)
				printf (" Media ID:              %.8s/%.3s\n",header+23,header+31);
			*/
			phys_start  = header[4+5]<<16,
				phys_start |= header[4+6]<<8,
				phys_start |= header[4+7];
			if ((header[4+2]&0x60)==0)	// single-layer
				phys_end  = header[4+9]<<16,
				phys_end |= header[4+10]<<8,
				phys_end |= header[4+11];
			else
				phys_end  = header[4+13]<<16,
				phys_end |= header[4+14]<<8,
				phys_end |= header[4+15];
			if (phys_end>0)	phys_end -= phys_start;
			if (phys_end>0) phys_end += 1;
			/*
			printf (" %s    %u*2KB=%"LLU"\n",
				dvd_dash>=0?"Legacy lead-out at:":"Last border-out at:",
				phys_end,phys_end*2048LL);
			*/
			if (dvd_dash<=0) break;
			
			cmd[0] = 0xAD;
			cmd[7] = 0; dvd_dash=-1;
			cmd[9] = 20;
			cmd[11] = 0;
			if ((err=cmd.transport(READ,header,20)))
			{   SPERROR ("READ DVD-R STRUCTURE",err);
			break;
			}
    } while (1);
	
    if (profile==0x10 && (header[4]&0xF0)==0){
		printf ("DVD-ROM media detected, exiting...\n");
		return 0;
	}
	
    if (profile==0x2B) do
    { 
		unsigned char s[4+8];
		
		cmd[0] = 0xAD;	// READ DVD STRUCTURE
		cmd[7] = 0x20;	// "DVD+R Double Layer Boundary Information"
		cmd[9] = sizeof(s);
		cmd[11] = 0;
		if ((err=cmd.transport(READ,s,sizeof(s))))
		{  
			SPERROR ("READ LAYER BOUNDARY INFORMATION",err);
			break;
		}
		
		if ((s[0]<<8|s[1]) < 10)
		{  
			fprintf (stdout,":-( insane LBI structure length\n");
			break;
		}
		/*
		printf ("DVD+R DOUBLE LAYER BOUNDARY INFORMATION:\n");
		printf (" L0 Data Zone Capacity: %u*2KB, can %s be set\n",
			s[8]<<24|s[9]<<16|s[10]<<8|s[11],
			s[4]&0x80?"no longer":"still");*/
    } while (0);
	
    if (profile==0x16) do
    { 
		unsigned char s[4+8];
		
		// Layer Jump specific structures. Note that growisofs doesn't
		// perform Layer Jump recordings, see web-page for details...
		
		cmd[0] = 0xAD;	// READ DVD STRUCTURE
		cmd[7] = 0x21;	// "DVD-R DL Shifted Middle Area Start Address"
		cmd[9] = sizeof(s);
		cmd[11] = 0;
		if ((err=cmd.transport(READ,s,sizeof(s)))){
			if(argc > 2){
				SPERROR ("READ SHIFTED MIDDLE AREA START ADDRESS",err);
			}
//			argc>2 ? SPERROR ("READ SHIFTED MIDDLE AREA START ADDRESS",err) : (void)0;
		}
		else if ((s[0]<<8|s[1]) < 10)
			fprintf (stdout,":-( insane SMASA structure length\n");
		else {/*
			printf ("DVD-R DL SHIFTED MIDDLE AREA START ADDRESS:\n");
			printf (" Shifted Middle Area:   %u*2KB, can %s be set\n",
				s[8]<<24|s[9]<<16|s[10]<<8|s[11],
				s[4]&0x80?"no longer":"still");*/
		}
		
		cmd[0] = 0xAD;	// READ DVD STRUCTURE
		cmd[7] = 0x22;	// "DVD-R DL Jump Interval Size"
		cmd[9] = sizeof(s);
		cmd[11] = 0;
		if ((err=cmd.transport(READ,s,sizeof(s)))){
			if(argc > 2){
				SPERROR ("READ JUMP INTERVAL SIZE",err)
			}
//			argc>2 ? SPERROR ("READ JUMP INTERVAL SIZE",err) : (void)0;
		}
		else if ((s[0]<<8|s[1]) < 10)
			fprintf (stdout,":-( insane JIS structure length\n");
		else {/*
			printf ("DVD-R DL JUMP INTERVAL SIZE:\n");
			printf (" Jump Interval Size:    %u*2KB\n",
				s[8]<<24|s[9]<<16|s[10]<<8|s[11]);*/
		}
		
		cmd[0] = 0xAD;	// READ DVD STRUCTURE
		cmd[7] = 0x23;	// "DVD-R DL Manual Layer Jump Address"
		cmd[9] = sizeof(s);
		cmd[11] = 0;
		if ((err=cmd.transport(READ,s,sizeof(s)))){
			if(argc > 2){
				SPERROR ("READ MANUAL LAYER JUMP ADDRESS",err);
			}
//			argc>2 ? SPERROR ("READ MANUAL LAYER JUMP ADDRESS",err) : (void)0;
		}
		else if ((s[0]<<8|s[1]) < 10)
			fprintf (stdout,":-( insane MLJA structure length\n");
		else {/*
			printf ("DVD-R DL MANUAL LAYER JUMP ADDRESS:\n");
			printf (" Manual Layer Jump:     %u*2KB\n",
				s[8]<<24|s[9]<<16|s[10]<<8|s[11]);*/
		}
		
		cmd[0] = 0xAD;	// READ DVD STRUCTURE
		cmd[7] = 0x24;	// "DVD-R DL Remapping Address"
		cmd[9] = sizeof(s);
		cmd[11] = 0;
		if ((err=cmd.transport(READ,s,sizeof(s)))){
			if(argc > 2){
				SPERROR ("READ REMAPPING ADDRESS",err);
			}
//			argc>2 ? SPERROR ("READ REMAPPING ADDRESS",err) : (void)0;
		}
		else if ((s[0]<<8|s[1]) < 10)
			fprintf (stdout,":-( insane RA structure length\n");
		else {/*
			printf ("DVD-R DL REMAPPING ADDRESS:\n");
			printf (" Remapping Jump:        %u*2KB\n",
				s[8]<<24|s[9]<<16|s[10]<<8|s[11]);*/
		}
    } while (0);
	
    if (profile==0x12 && dvd_0A) do
    { unsigned char s[16];
	unsigned int  a,b;
	
	if (dvd_0A>sizeof(s))	dvd_0A=sizeof(s);
	
	cmd[0] = 0xAD;	// READ DVD STRUCTURE
	cmd[7] = 0x0A;	// "DVD-RAM Spare Area Information"
	cmd[8] = dvd_0A>>8;
	cmd[9] = dvd_0A;
	cmd[11] = 0;
	if ((err=cmd.transport(READ,s,dvd_0A)))
	{   SPERROR ("READ DVD-RAM SPARE INFORMATION",err);
	break;
	}
	
	if (dvd_0A<8)	break;
	//printf ("DVD-RAM SPARE AREA INFORMATION:\n");
	a = s[ 4]<<24|s[ 5]<<16|s[ 6]<<8|s[ 7];
	b = dvd_ram_spare;
//	printf (" Primary SA:            %u/%u=%.1f%% free\n",a,b,(a*100.0)/b);
	if (dvd_0A<16)	break;
	a = s[ 8]<<24|s[ 9]<<16|s[10]<<8|s[11];
	b = s[12]<<24|s[13]<<16|s[14]<<8|s[15];
	//printf (" Supplementary SA:      %u/%u=%.1f%% free\n",a,b,(a*100.0)/b);
    } while (0);
	
    if (profile==0x12 && dvd_C0) do
    { 
		unsigned char s[8];
		
		if (dvd_C0>sizeof(s))	dvd_C0=sizeof(s);
		
		cmd[0] = 0xAD;	// READ DVD STRUCTURE
		cmd[7] = 0xC0;	// "Write Protection Status"
		cmd[8] = dvd_C0>>8;
		cmd[9] = dvd_C0;
		cmd[11] = 0;
		if ((err=cmd.transport(READ,s,dvd_C0)))
		{   SPERROR ("READ WRITE PROTECTION STATUS",err);
		break;
		}
		
		if (dvd_C0<8)	break;
		//	printf ("DVD-RAM WRITE PROTECTION STATUS:\n");
		/*if (s[4]&0x04)
		printf (" Write protect tab on cartridge is set to protected\n");
		if (s[4]&0x01)
		printf (" Software Write Protection until Power down is on\n");
		printf (" Persistent Write Protection is %s\n",s[4]&0x02?"on":"off");*/
    } while (0);
	
    if (blu_ray && dvd_0A) do
    {
		unsigned char s[16];
		unsigned int  a,b;
		
		if (dvd_0A>sizeof(s))	dvd_0A=sizeof(s);
		
		cmd[0] = 0xAD;	// READ DVD STRUCTURE
		cmd[1] = blu_ray;
		cmd[7] = 0x0A;	// "BD Spare Area Information"
		cmd[8] = dvd_0A>>8;
		cmd[9] = dvd_0A;
		cmd[11] = 0;
		if ((err=cmd.transport(READ,s,dvd_0A)))
		{   //if (err!=0x33100 || argc>2) SPERROR ("READ BD SPARE INFORMATION",err);
		break;
		}
		
		a = s[ 8]<<24|s[ 9]<<16|s[10]<<8|s[11];
		b = s[12]<<24|s[13]<<16|s[14]<<8|s[15];
		if (dvd_0A<16 || b==0)	break;
		//printf ("BD SPARE AREA INFORMATION:\n");
		//	printf (" Spare Area:            %u/%u=%.1f%% free\n",a,b,(a*100.0)/b);
    } while (0);
	
    if (blu_ray && prf_38) do
    { 
		unsigned char pow[16];
		
		cmd[0] = 0x51;	// READ DISC INFORMATION
		cmd[1] = 0x10;	// "POW Resources Information"
		cmd[8] = sizeof(pow);
		cmd[9] = 0;
		if ((err=cmd.transport(READ,pow,sizeof(pow))))
		{   SPERROR ("READ ROW RESOURCES INFORMATION",err);
		break;
		}
		
	/*	printf ("POW RESOURCES INFORMATION:\n");
		printf (" Remaining Replacements:%u\n",pow[ 4]<<24|pow[ 5]<<16|pow[ 6]<<8|pow[ 7]);
		printf (" Remaining Map Entries: %u\n",pow[ 8]<<24|pow[ 9]<<16|pow[10]<<8|pow[11]);
		printf (" Remaining Updates:     %u\n",pow[12]<<24|pow[13]<<16|pow[14]<<8|pow[15]);*/
    } while(0);
	
    if (blu_ray && argc>2) do
    { 
		unsigned char *p,*pac,*s;
		unsigned int   len;
		
		cmd[0] = 0xAD;	// READ DVD STRUCTURE
		cmd[1] = blu_ray;
		cmd[7] = 0x30;	// "Physical Access Control (PAC)"
		cmd[9] = 4;
		cmd[11] = 0;
		if ((err=cmd.transport(READ,header,4)))
		{   if (err!=0x33100 || argc>2) SPERROR ("READ BD PAC LIST",err);
		break;
		}
		
		len = (header[0]<<8|header[1])+2;
		if (!(p = (unsigned char *)malloc(len))) break;
		
		cmd[0] = 0xAD;	// READ DVD STRUCTURE
		cmd[1] = blu_ray;
		cmd[7] = 0x30;	// "Physical Access Control (PAC)"
		cmd[8] = len>>8;
		cmd[9] = len;
		cmd[11] = 0;
		if ((err=cmd.transport(READ,p,len)))
		{   SPERROR ("READ BD PAC LIST",err);
		break;
		}
		/*
		printf ("BD WRITTEN PAC HEADERS:\n");
		for (pac=p+4;pac<p+len;pac+=384)
		{   printf (" %3.3s.%02x:                ",pac,pac[3]);
		for (i=4;i<16;i++) printf ("%02x ",pac[i]);
		for (i=0,s=pac+16;i<pac[15];i++,s+=8)
			printf ("%u:%u ",s[0]<<24|s[1]<<16|s[2]<<8|s[3],
			s[4]<<24|s[5]<<16|s[6]<<8|s[7]);
		printf ("\n");
		}*/
    } while (0);
	
    cmd[0] = 0x51;	// READ DISC INFORMATION
    cmd[8] = 32;
    cmd[9] = 0;
    if ((err=cmd.transport(READ,header,32))){
		SPERROR ("READ DISC INFORMATION",cmd.sense());
		return -1;
	}
	
    { static const char
		*session_state[]={	"empty",	"incomplete",
		"reserved/damaged","complete"	},
		*disc_status[]={	"blank",	"appendable",
		"complete",	"other"		},
		*bg_format[]={	"blank",	"suspended",
		"in progress",	"complete"	};
	
//	printf ("READ DISC INFORMATION:");
	//if (argc>2)
		//for (j=0;j<16;j++) printf("%c%02x",j?' ':'\t',header[j]);
		//printf ("\n");
	//	printf (" Disc status:           %s\n",disc_status[header[2]&3]);
		
		memset(pDiskTypeInfo->media_status_str,0,16);   //V3.2.1 20180614
		strncpy (pDiskTypeInfo->media_status_str,disc_status[header[2]&3],15);     //V3.2.1 20180614
		
	//	printf (" Number of Sessions:    %d\n",header[9]<<8|header[4]);

		pDiskTypeInfo->media_number_session = header[9]<<8|header[4];

	//	printf (" State of Last Session: %s\n",session_state[(header[2]>>2)&3]);
		
		memset(pDiskTypeInfo->media_last_session_status,0,16);      //V3.2.1 20180614
		strncpy(pDiskTypeInfo->media_last_session_status ,session_state[(header[2]>>2)&3],15);     //V3.2.1 20180614

	/*	if ((header[2]&3)!=2)
			printf (" \"Next\" Track:          %d\n",header[10]<<8|header[5]);
		printf (" Number of Tracks:      %d",ntracks=header[11]<<8|header[6]);*/
		ntracks=header[11]<<8|header[6];
		pDiskTypeInfo->media_number_tracks = ntracks;

		/*if (header[7]&3)
			printf ("\n BG Format Status:      %s",bg_format[header[7]&3]);*/
		if ((header[7]&3) == 2)
		{
			unsigned char sense[18];
			
			cmd[0] = 0x03;	// REQUEST SENSE
			cmd[4] = sizeof(sense);
			cmd[5] = 0;
			if (!cmd.transport (READ,sense,sizeof(sense)) && sense[15]&0x80)
				printf (", %.1f%% complete",(sense[16]<<8|sense[17])/655.36);
		}
		printf ("\n");
		
		while (prf_23 || profile==0x12 || (header[2]&0x10 && argc>2))
		{ 
			unsigned char formats[260];
			int len;
			unsigned long long  capacity,blocksize;
			static const char *type[] = {	"reserved",	"unformatted",
				"formatted",	"no media"	};
			
			//printf ("READ FORMAT CAPACITIES:\n");
			
			cmd[0] = 0x23;		// READ FORMAT CAPACITIES
			cmd[8] = 12;
			cmd[9] = 0;
			if ((err=cmd.transport(READ,formats,12)))
			{	SPERROR ("READ FORMAT CAPACITIES",err);
			break;
			}
			
			len = formats[3];
			if (len&7 || len<16)
			{	fprintf (stdout,":-( allocation length isn't sane %d\n",len);
			break;
			}
			
			cmd[0] = 0x23;		// READ FORMAT CAPACITIES
			cmd[7] = (4+len)>>8;	// now with real length...
			cmd[8] = (4+len)&0xFF;
			cmd[9] = 0;
			if ((err=cmd.transport(READ,formats,4+len)))
			{	SPERROR ("READ FORMAT CAPACITIES",err);
			break;
			}
			
			if (len != formats[3])
			{	fprintf (stdout,":-( parameter length inconsistency\n");
			break;
			}
			
			if (blu_ray)   
				blocksize=2048;
			else	    
				blocksize=formats[9]<<16|formats[10]<<8|formats[11];
			//printf("format 9  10 11 %x,%x,%x",formats[9],formats[10],formats[11]);			
			pDiskTypeInfo->media_block_size= blocksize;
			//printf("pDiskTypeInfo->media_block_size = %d\n",pDiskTypeInfo->media_block_size);
		//	printf(" $$$$ %s:\t\t%u*%u=",type[formats[8]&3],
		//		capacity=formats[4]<<24|formats[5]<<16|formats[6]<<8|formats[7],
		//		blocksize);
		
			memset(pDiskTypeInfo->media_format_status,0,16);   //V3.2.1 20180614
			strncpy(pDiskTypeInfo->media_format_status,type[formats[8]&3],15);   //V3.2.1 20180614
			
			pDiskTypeInfo->media_capacity = capacity*blocksize;
		//	printf("\n !!%u,%u,pDiskTypeInfo->media_capacity = %llu ,\n",capacity,blocksize,pDiskTypeInfo->media_capacity);
			//printf("%"LLU"\n",(unsigned long long)capacity*blocksize);
			/*
			for(i=12;i<len;i+=8)
			{
				printf(" %02Xh(%x):\t\t%u*%u=",formats[i+4]>>2,
					formats[i+5]<<16|formats[i+6]<<8|formats[i+7],
					capacity=formats[i]<<24|formats[i+1]<<16|formats[i+2]<<8|formats[i+3],
					blocksize);
				printf("%"LLU"\n",(unsigned long long)capacity*blocksize);
			}*/
			break;
		}
    }
	
    if (argc<=2 && profile==0x12) ntracks=0;
	
    for (i=1;i<=ntracks;i++)
    { static const char *dash_track_state[]={
		"complete",	"complete incremental",
			"blank",	"invisible incremental",
			"partial",	"partial incremental",
			"reserved",	"reserved incremental"};
		static const char *plus_track_state[]={
			"invisible",		"blank",
				"partial/complete",	"reserved" };
			int len=32;
			unsigned long long  freesize  =0;
			while (len)
			{   cmd[0] = 0x52;	// READ TRACK INFORMATION
			cmd[1] = 1;
			cmd[2] = i>>24;
			cmd[3] = i>>16;
			cmd[4] = i>>8;
			cmd[5] = i;
			cmd[8] = len;
			cmd[9] = 0;
			if ((err=cmd.transport(READ,header,len)))
			{	
				if (i<ntracks) { i=ntracks; continue; }
				SPERROR ("READ TRACK INFORMATION",err);
				return -1;
			}
			
			if ((profile==0x1B || profile==0x2B) &&
				(header[6]&0x80)==0 && (header[0]<<8|header[1])>=38)
			{	if (len==32)	{ len=40; continue; }
			else		{ break;            }
			}
			else if ((profile==0x15 || profile==0x16) &&
				(header[5]&0xC0) && (header[0]<<8|header[1])>=46)
			{	if (len==32)	{ len=48; continue; }
			else		{ break;	    }
			}
			
			len = 0;
			}
			
		//	printf ("READ TRACK INFORMATION[#%d]:",i);
			//if (argc>2)
				//for (j=0;j<16;j++) printf("%c%02x",j?' ':'\t',header[j]);
				//printf ("\n");
				if ((profile&0x0F)==0x0B)	// DVD+R
				{
//					printf (" Track State:           %s\n",
//						plus_track_state[header[6]>>6]);
					
					if (i<8)
					{
						// memset(pDiskTypeInfo->media_track_state[i-1],0,);
					//	strncpy(pDiskTypeInfo->media_track_state[i-1],plus_track_state[header[6]>>6],);
					}
					
				}
				else
				{
				//	printf (" Track State:           %s%s",
					//	((header[6]>>5)==1 && (header[7]&1))?"in":"",
					//	dash_track_state[header[6]>>5]);
					if (i<8)
					{
						//sprintf(pDiskTypeInfo->media_track_state[i-1],"%s%s",
						//	((header[6]>>5)==1 && (header[7]&1))?"in":"",
						//dash_track_state[header[6]>>5]);
					}
					if (header[5]&0x20)	printf("Track %d,damaged", i);
					printf ("\n");
				}
			//	printf (" Track Start Address:   %u*2KB\n",
				//	header[8]<<24|header[9]<<16|header[10]<<8|header[11]);
			//	if (header[7]&1)
				//	printf (" Next Writable Address: %u*2KB\n",
				//	header[12]<<24|header[13]<<16|header[14]<<8|header[15]);
			//	printf (" Free Blocks:           %u*2KB\n",
				//	header[16]<<24|header[17]<<16|header[18]<<8|header[19]);
					freesize = header[16]<<24|header[17]<<16|header[18]<<8|header[19];
					if (freesize > 0)
					{
						pDiskTypeInfo->media_track_freesize = freesize*2*1024;
					}else{
						freesize =  header[24]<<24|header[25]<<16|header[26]<<8|header[27];
						pDiskTypeInfo->media_track_freesize = freesize*2*1024;
					}
					
				//	printf("pDiskTypeInfo->media_track_freesize %llu,freesize %llu \n",pDiskTypeInfo->media_track_freesize,freesize);
				
/*
				if (header[6]&0x10)
					printf (" Fixed Packet Size:     %u*2KB\n",
					header[20]<<24|header[21]<<16|header[22]<<8|header[23]);
				printf (header[5]&0x40?		// check upon LJ bit
					" Zone End Address:      %u*2KB\n":
				" Track Size:            %u*2KB\n",
					header[24]<<24|header[25]<<16|header[26]<<8|header[27]);
				if (header[7]&2)
					printf (" Last Recorded Address: %u*2KB\n",
					header[28]<<24|header[29]<<16|header[30]<<8|header[31]);
				if (len>=40)
					printf (" ROM Compatibility LBA: %u\n",
					header[36]<<24|header[37]<<16|header[38]<<8|header[39]);
				if (len>=48 && header[5]&0xC0)	// check upon LJRS bits
					printf (" LJRS field:            %u\n",header[5]>>6),
					printf (" Next Layer Jump:       %u\n",
					header[40]<<24|header[41]<<16|header[42]<<8|header[43]),
					printf (" Last Layer Jump:       %u\n",
					header[44]<<24|header[45]<<16|header[46]<<8|header[47]);*/
    }//end for
	
    do
    {	unsigned char *toc,*p;
	int   len,sony;
	
	cmd[0] = 0x43;	// READ TOC
	cmd[6] = 1;
	cmd[8] = 12;
	cmd[9] = 0;
	if ((err=cmd.transport (READ,header,12)))
	{   if (argc>2)
	SPERROR ("READ TOC",err);
	break;
	}
	
	len = (header[0]<<8|header[1])+2;
	toc = (unsigned char *)malloc (len);
	
//	printf ("FABRICATED TOC:");
/*	if (argc>2)
		printf ("\t\t%u %x %x",header[0]<<8|header[1],header[2],header[3]);
	printf ("\n");
*/	
	cmd[0] = 0x43;		// READ TOC
	cmd[6] = header[2];	// "First Track Number"
	cmd[7] = len>>8;
	cmd[8] = len;
	cmd[9] = 0;
	if ((err=cmd.transport (READ,toc,len)))
	{   SPERROR ("READ TOC",err);
	break;
	}
	/*
	for (p=toc+4,i=4;i<len-8;i+=8,p+=8)
		printf (" Track#%-3u:             %02x@%u\n",
		p[2],p[1],p[4]<<24|p[5]<<16|p[6]<<8|p[7]);
	printf (" Track#%-2X :             %02x@%u\n",
		p[2],p[1],p[4]<<24|p[5]<<16|p[6]<<8|p[7]);
	*/
	cmd[0] = 0x43;	// READ TOC
	cmd[2] = 1;	// "Session info"
	cmd[8] = 12;
	cmd[9] = 0;
	if ((err=cmd.transport (READ,header,12)))
	{   if (argc>2)
	SPERROR ("GET SESSION INFO",err);
	break;
	}
	
	len = header[4+4]<<24|header[4+5]<<16|header[4+6]<<8|header[4+7];
//	printf (" Multi-session Info:    #%u@%u\n",header[4+2],len);
	
	cmd[0] = 0x43;	// READ TOC
	cmd[8] = 12;
	cmd[9] = 0x40;	// "SONY Vendor bit"
	if ((err=cmd.transport (READ,header,12)))
	{   if (argc>2)
	SPERROR ("GET SONY SESSION INFO",err);
	break;
	}
	
	sony = header[4+4]<<24|header[4+5]<<16|header[4+6]<<8|header[4+7];
	if (len != sony)
		printf (" SONY Session Info:     #%u@%u\n",header[4+2],sony);
	
    } while (0);
	
    do {
		unsigned int ccity,bsize;
		cmd[0] = 0x25;	// READ CAPACITY
		cmd[9] = 0;
		if ((err=cmd.transport (READ,header,8)))
		{   if (argc>2)
		SPERROR ("READ CAPACITY",err);
		break;
		}
		
		ccity = header[0]<<24|header[1]<<16|header[2]<<8|header[3];
		if (ccity) ccity++;
		bsize = header[4]<<24|header[5]<<16|header[6]<<8|header[7];
		
		/*printf ("READ CAPACITY:          %u*%u=%"LLU"\n",
			ccity,bsize,
			(unsigned long long)ccity*bsize);*/
    } while (0);
	
	fprintf (stdout,"get media ok\n");
	return 0;
}


int GetLabel(char*DevName,SDiskTypeInfo* pDiskTypeInfo){
	blkid_probe pr;
	const char *name;

	pr = blkid_new_probe_from_filename(DevName);
	if (!pr) {
		printf("Failed to open %s", DevName);
		return -1;
	}
	
	blkid_do_probe(pr);
	if (blkid_probe_lookup_value(pr, "TYPE", &name, NULL) == 0)
	{
	//	printf("type=%s\n", name);
		memset(pDiskTypeInfo->fs_type,0,16);     //V3.2.1 20180614
		strncpy(pDiskTypeInfo->fs_type,name,15);   //V3.2.1 20180614
	}
	
	
	if (blkid_probe_lookup_value(pr, "LABEL", &name, NULL) == 0)
	{
		memset(pDiskTypeInfo->label_name,0,128);     //V3.2.1 20180614
		strncpy(pDiskTypeInfo->label_name,name,127);     //V3.2.1 20180614
	//	printf("label=%s\n", name);
	}
	
	
	blkid_free_probe(pr);
	return 0;
}
// 
// #include <linux/cdrom.h>
// /* 
// CDS_NO_INFO  Information not available.
// CDS_NO_DISC
// CDS_TRAY_OPEN
// CDS_DRIVE_NOT_READY
// CDS_DISC_OK
//      -1   error
// 	 */
// 
// /* return value 0 means trayopen,-1 error */
// int GetCdRomStatus(char *cdrom_dev) {
// 	int cdrom;
// 	int status=-1;
// 	
// 	if (! cdrom_dev ){
// 		return -1;
// 	}
// 	
// 	if ((cdrom = open(cdrom_dev,O_RDONLY | O_NONBLOCK)) < 0) {
// 		printf("Unable to open device %s. Provide a device name (/dev/sr0, /dev/cdrom) as a parameter.\n",cdrom_dev);
// 		return -1;
// 	}
// 	/* Check CD tray status */
// 	status = ioctl(cdrom,CDROM_DRIVE_STATUS);
// 	
// 	close(cdrom);
// 	
// 	return status;
// }
// 
// int CdromReady(char*cdrom_dev){
// 	int i,status;
// 	int ret = 0,try_num=0,ready=0;
// 	char cmd_buff[256];
// 	for(i=0;i<60;i++)
// 	{
// 		printf("Wait %s Ready !\n",cdrom_dev);
// 		status =  GetCdRomStatus(cdrom_dev);
// 		switch (status)
// 		{
// 		/* media maybe bad */
// 		case CDS_NO_INFO:
// 		case CDS_NO_DISC:
// 			printf("%s CDS_NOINFO or CDS_NO_DISC \n",cdrom_dev);
// 			ret = -2;
// 			i = 60;
// 			break;
// 			/* wait tray close */
// 		case CDS_TRAY_OPEN:
// 			printf("CDS_TRAY_OPEN \n");
// 		//	sprintf(cmd_buff,"eject -t %s",cdrom_dev);
// 		//	system(cmd_buff);
// 			ret = -1;
// 			i = 60;
// 			break;
// 		case CDS_DRIVE_NOT_READY:
// 			printf("CDS_DRIVE_NOT_READY \n");
// 			sleep(2);
// 			continue;
// 			break;
// 		case CDS_DISC_OK:
// 			printf("CDS_DISC_OK \n");
// 			ready = 1;
// 			ret = 1;
// 			break;
// 		case -1:
// 			printf("cdrom failure,errno %d \n",errno);
// 			sleep(10);
// 			continue;
// 			break;
// 		}
// 		sleep(1);
// 		if (ret==1)
// 		{
// 			i=60;
// 		}
// 	}
// 	return ret;
// }

int GetMediaInfo( char* DevName, SDiskTypeInfo* pDiskTypeInfo){
	int ret = -1;
	int i=0;
//	printf("GetMediaInfo dev_name %s\n",DevName);

	for (i =0;i<5;i++)
	{
		ret = _GetMediaInfo( DevName, pDiskTypeInfo);
		printf("after _GetMediaInfo( DevName, pDiskTypeInfo)\n");
		if (ret ==0)
		{
			break;
		}
		sleep(1);
	}
	if (ret != 0)
	{
		return -1;
	}
	/* some case cann't find label ,scsi error ,try again */
	if (strncmp(pDiskTypeInfo->media_status_str,"blank",5))
	{
		for (i=0;i<5;i++)
		{
			GetLabel(DevName,pDiskTypeInfo);
			if (strlen(pDiskTypeInfo->fs_type) > 0)
			{
				break;
			}
			sleep(1);
		}
	
	}
	
	if (!strcmp(pDiskTypeInfo->media_status_str,"blank"))
	{
		pDiskTypeInfo->media_capacity =  MBLANK;
	}
	if (!strcmp(pDiskTypeInfo->media_status_str,"appendable"))
	{
		if ((!strcmp(pDiskTypeInfo->media_last_session_status,"empty")) )
		{
			pDiskTypeInfo->media_capacity =  MAPPENEN;
		}
		if ((!strcmp(pDiskTypeInfo->media_last_session_status,"incomplete")) )
		{
			pDiskTypeInfo->media_capacity =  MDAMAGE;
		}
		if ((!strcmp(pDiskTypeInfo->media_last_session_status,"reserved/damaged")) )
		{
			pDiskTypeInfo->media_capacity =  MDAMAGE;
		}
		if ((!strcmp(pDiskTypeInfo->media_last_session_status,"complete")) )
		{
			pDiskTypeInfo->media_capacity =  MCOMPLET;
		}
	}
	if (!strcmp(pDiskTypeInfo->media_status_str,"other"))
	{
		if ((!strcmp(pDiskTypeInfo->media_format_status,"formatted")) )
		{
			struct statfs  d_statfs;
			pDiskTypeInfo->media_capacity =  MFORMAT;
			if (statfs(DevName,&d_statfs)!=0)
			{
				printf("\n	statfs error !\n");
			}else{
				pDiskTypeInfo->media_track_freesize = d_statfs.f_bfree*1024;
				printf("%ld\n",pDiskTypeInfo->media_track_freesize);
			}
			
		}
		if ((!strcmp(pDiskTypeInfo->media_last_session_status,"unformatted")) )
		{
			pDiskTypeInfo->media_capacity =  MUFORMAT;
		}
	}
	

	
	return ret ;
}


int WriteDiskType(char *filename,SDiskTypeInfo *pDiskTypeInfo){
	FILE *file=NULL;
	if ((filename== NULL)||(pDiskTypeInfo==NULL))
	{
		return -1;
	}
	file= fopen(filename,"wb+");
	if (file)
	{
		int i = 0;
		fprintf(file,"%d,",pDiskTypeInfo->media_type);
		fprintf(file,"%s,",pDiskTypeInfo->media_type_str);          /* 检测出来的类型 */
		fprintf(file,"%s,",pDiskTypeInfo->media_id);          /* 光盘厂商标识 */
		fprintf(file,"%s,",pDiskTypeInfo->media_status_str);
		fprintf(file,"%d,",pDiskTypeInfo->media_number_session);
		fprintf(file,"%s,",pDiskTypeInfo->media_last_session_status);
		fprintf(file,"%d,",pDiskTypeInfo->media_number_tracks);
		
		fprintf(file,"%d,",pDiskTypeInfo->media_block_size);
		fprintf(file,"%s,",pDiskTypeInfo->media_format_status);
		fprintf(file,"%llu,",pDiskTypeInfo->media_capacity);
		
	//	fprintf(file,"%s,",pDiskTypeInfo->media_track_state[i]);
		fprintf(file,"%llu,",pDiskTypeInfo->media_track_freesize);
			
	
		fprintf(file,"%d,",	pDiskTypeInfo->media_writespeed_cnt);
		for (i=0;i<8;i++)
		{
			fprintf(file,"%s,",pDiskTypeInfo->media_writespeed_str[i]);
		}
		fprintf(file,"%s,",	pDiskTypeInfo->label_name);
		fprintf(file,"%s,",	pDiskTypeInfo->fs_type);
		fprintf(file,"\n");
		if(file){
			fclose(file);
			file=NULL;
		}
		return 0;
	}
	return -1;
}


