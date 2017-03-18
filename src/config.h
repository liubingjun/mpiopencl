 
/*************************************************************************
	> File Name: config.cpp
	> Author: LiuBingjun
	> Mail: 278732708@qq.com 
	> Created Time: 2014年05月14日 星期三 14时53分01秒
 ************************************************************************/

class Config{
public:
    int num;
    int parameterType;
    int resultType;
    int totalLength;
    int unitLength;
    char* kernelname;
    char* kernelfile;
public:
    void  set();
    int**  init();
};
