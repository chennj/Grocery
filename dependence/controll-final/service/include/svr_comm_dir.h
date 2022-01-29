#ifndef _SVR_COMM_DIR_H_
#define _SVR_COMM_DIR_H_

#include <stdio.h>

class SvrDir
{
private:
    SvrDir(){}
    ~SvrDir(){}

public:
	static SvrDir& Inst()
	{
		static SvrDir inst;
		return inst;
	}

    //将源目录dir下的所有文件复制到目标目录ddir下，包括目录名
    int DuplicateDir(char *dir, char *ddir);
    //
    int ParseR(FILE *dfile, char*ddir, char*prefix, int splitcnt, char*labe_prefix);
    int ParseD(
        FILE* dfile,                        //目录文件
        char* ddir,                         //目的目录
        char* prefix                        //目录文件前缀
    );
    //返回光盘类型对应的尺寸
    unsigned long long ConvertMediaTypeToSplitSize(char *cdtype);
    //拆分目录
    int SplitDir(char *srcdir, char *destdir, char*label, char*cdtype);
    int SummaryDir(
        char *dir,                          //目的目录
        int  *fcnt,                         //输出文件数
        unsigned long long *bsize,          //总大小
        FILE *d_file,                       //目录名列表
        FILE *r_file,                       //文件名表
        unsigned long long splitsize,       //分割因子
        unsigned long long *splitcnt,       //分割盘数
        unsigned long long *split_base      //分割变量
    );

//private:
//    char g_track_not_closed[MAXSTATIONMEDIA];  //保存光盘在刻录后是否没有关闭轨道 ==1 就是没有关闭轨道

private:
    //在目标目录“ddir”中建立目录
    int smkdir(char *dirname,char *ddir);
    //移动源文件“rname”到目标目录“ddir”，目标可以有多个
    int smv(char *rname, char *ddir,char *prefix);
};

#endif //!_SVR_COMM_DIR_H_